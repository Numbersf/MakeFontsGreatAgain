// unicode_filter.c
// Supports format 4 + format 12
// Author: 咿云冷雨(Numbersf)
//
// v2 rewrite: instead of patching the cmap in place (which could silently
// drop kept glyphs when a deletion split a format-4 segment and the segment
// count outgrew the original table, and which left stale glyphIdArray
// offsets after segment compaction), the tool now rebuilds the whole cmap
// table into an exact-size buffer and writes the font to <file>.tmp,
// atomically renamed over the original.
//
// Exit codes:
//   0 = success
//   1 = usage / open / parse error
//   2 = no valid range parsed

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_RANGES 128
#define MAX_SPLIT  (MAX_RANGES * 2 + 1)   // worst-case output segments per input segment

typedef struct { uint32_t start, end; } Range;
typedef struct { Range ranges[MAX_RANGES]; int count; } RangeList;

/* ── BE helpers ── */
static uint16_t r16(const uint8_t *b){ return (uint16_t)((b[0]<<8)|b[1]); }
static uint32_t r32(const uint8_t *b){
    return ((uint32_t)b[0]<<24)|((uint32_t)b[1]<<16)|((uint32_t)b[2]<<8)|b[3];
}
static void w16(uint8_t *b,uint16_t v){ b[0]=(v>>8)&0xFF; b[1]=v&0xFF; }
static void w32(uint8_t *b,uint32_t v){
    b[0]=(v>>24)&0xFF; b[1]=(v>>16)&0xFF; b[2]=(v>>8)&0xFF; b[3]=v&0xFF;
}

/* ── checksum (padded to 4 bytes) ── */
static uint32_t checksum(const uint8_t *data, uint32_t len){
    uint32_t sum=0, n=(len+3)&~3u;
    for(uint32_t i=0;i<n;i+=4){
        if(i+3<len){ sum+=r32(data+i); }
        else{
            uint8_t tmp[4]={0,0,0,0};
            for(int j=0;j<4;j++) if(i+j<len) tmp[j]=data[i+j];
            sum+=r32(tmp);
        }
    }
    return sum;
}

/* ────────────────────────────────────────────────────
 * clip_segments
 *
 * 输入一个 [start,end] 区段，从中减去 RangeList 所有删除区间，
 * 输出剩余的若干子区段写入 out[]，返回子区段数量。
 * ──────────────────────────────────────────────────── */
static int clip_segments(uint32_t start, uint32_t end,
                         const RangeList *list,
                         Range out[MAX_SPLIT])
{
    /* 先把删除区间按 start 排序（复制后排，不改原列表） */
    Range sorted[MAX_RANGES];
    int n = list->count;
    memcpy(sorted, list->ranges, (size_t)n * sizeof(Range));
    for(int i=1;i<n;i++){
        Range key=sorted[i]; int j=i-1;
        while(j>=0 && sorted[j].start > key.start){ sorted[j+1]=sorted[j]; j--; }
        sorted[j+1]=key;
    }

    int count = 0;
    uint32_t cur = start;

    for(int i=0;i<n && cur<=end;i++){
        uint32_t ds = sorted[i].start;
        uint32_t de = sorted[i].end;
        if(de < cur) continue;   /* 删除区间完全在左侧，已跳过 */
        if(ds > end) break;      /* 删除区间完全在右侧，后面也不用看 */

        if(ds > cur){
            out[count].start = cur;
            out[count].end   = ds - 1;
            count++;
        }
        cur = de + 1;
    }
    if(cur <= end){
        out[count].start = cur;
        out[count].end   = end;
        count++;
    }
    return count;   /* 0 = 全部删除 */
}

/* ════════════════════════════════════════════════════
 * FORMAT 12 rebuild (exact size)
 * 每个 group = [startCharCode(4), endCharCode(4), startGlyphID(4)]
 * 裁剪时 startGlyphID 随 startCharCode 偏移同步增加。
 * 返回新子表缓冲（malloc），*out_len 为长度。失败返回 NULL。
 * ════════════════════════════════════════════════════ */
static uint8_t *rebuild_format12(const uint8_t *sub, uint32_t subLen,
                                 const RangeList *list, uint32_t *out_len)
{
    if (subLen < 16) return NULL;
    uint32_t lenField = r32(sub+4);          /* length 字段应与传入长度一致 */
    if (lenField != subLen || lenField < 16) return NULL;
    uint32_t n   = r32(sub+12);
    if (16u + n*12u > subLen) return NULL;   /* 越界，拒绝 */

    const uint8_t *grp = sub+16;
    Range segs[MAX_SPLIT];
    uint32_t maxOut = n * MAX_SPLIT;
    uint32_t wi = 0;

    for(uint32_t i=0;i<n;i++){
        const uint8_t *g = grp + i*12;
        uint32_t start = r32(g), end = r32(g+4);
        if (start > end) return NULL;
        int ns = clip_segments(start, end, list, segs);
        wi += (uint32_t)ns;
        if (wi > maxOut) return NULL;
    }

    uint32_t newLen = 16u + wi*12u;
    uint8_t *out = (uint8_t *)calloc(1, newLen);
    if (!out) return NULL;
    w16(out, 12);
    w32(out+4, newLen);                      /* length 字段 */
    w16(out+8, 0);                           /* language = 0 */
    w32(out+12, wi);                         /* numGroups */

    wi = 0;
    for(uint32_t i=0;i<n;i++){
        const uint8_t *g = grp + i*12;
        uint32_t start = r32(g), end = r32(g+4);
        uint32_t startGlyph = r32(g+8);
        int ns = clip_segments(start, end, list, segs);
        for(int s=0;s<ns;s++){
            uint32_t ns_ = segs[s].start, ne_ = segs[s].end;
            uint32_t ng = startGlyph + (ns_ - start);   /* glyphID 随偏移线性移动 */
            uint8_t *dst = out + 16 + wi*12;
            w32(dst,    ns_);
            w32(dst+4,  ne_);
            w32(dst+8,  ng);
            wi++;
        }
    }
    *out_len = newLen;
    return out;
}

/* ════════════════════════════════════════════════════
 * FORMAT 4 rebuild (exact size)
 *
 * 布局（偏移从子表头算起）：
 *   0  format(2)
 *   2  length(2)
 *   4  language(2)
 *   6  segCountX2(2)
 *   8  searchRange(2)
 *  10  entrySelector(2)
 *  12  rangeShift(2)
 *  14  endCode[segCount]      ← 含末尾 0xFFFF 哨兵
 *  14+segCount*2  reservedPad(2)
 *  16+segCount*2  startCode[segCount]
 *  16+segCount*4  idDelta[segCount]
 *  16+segCount*6  idRangeOffset[segCount]
 *  16+segCount*8  glyphIdArray[...]
 *
 * 关键修正（相对旧版原地裁剪）：
 *  1. 精确重建：输出段数可以超过原始段数（删除中间一段会把一个
 *     ro==0 段拆成两段），不再“截断丢弃”保留字形。
 *  2. idRangeOffset 是相对 idRangeOffset[i] 字位置的偏移；段被压缩后
 *     i/j 变化，必须重算：newRO = oldRO - 2 * (i 之后被删除的段数)。
 *     glyphIdArray 原样搬移。
 *  3. ro!=0 的段沿用保守策略：仅当左边界未被裁剪时才保留。
 * ════════════════════════════════════════════════════ */
static uint8_t *rebuild_format4(const uint8_t *sub, uint32_t subLen,
                                const RangeList *list, uint32_t *out_len)
{
    if (subLen < 16) return NULL;
    uint16_t segCount = r16(sub+6)/2;        /* 含末尾哨兵 */
    if (segCount == 0 || 16u + (uint32_t)segCount*8u > subLen) return NULL;

    const uint8_t *p = sub+14;
    const uint8_t *endCode       = p;  p += (size_t)segCount*2 + 2; /* +pad */
    const uint8_t *startCode     = p;  p += (size_t)segCount*2;
    const uint8_t *idDelta       = p;  p += (size_t)segCount*2;
    const uint8_t *idRangeOffset = p;
    const uint8_t *glyphArray    = sub + 16u + (uint32_t)segCount*8u;
    uint32_t glyphArrayLen = subLen - 16u - (uint32_t)segCount*8u;

    Range segs[MAX_SPLIT];
    uint32_t maxOut = (uint32_t)segCount * MAX_SPLIT;
    uint32_t newSeg = 0;
    for (int i=0; i<(int)segCount-1; i++){   /* 跳过最后哨兵 */
        uint32_t s  = r16(startCode + i*2);
        uint32_t e  = r16(endCode   + i*2);
        if (s > e) return NULL;
        int ns = clip_segments(s, e, list, segs);
        newSeg += (uint32_t)ns;
        if (newSeg > maxOut) return NULL;
    }
    newSeg += 1;                              /* 哨兵 */
    if (newSeg == 0 || newSeg > 0xFFFF) return NULL;

    uint32_t newLen = 16u + newSeg*8u + glyphArrayLen;
    uint8_t *out = (uint8_t *)calloc(1, newLen);
    if (!out) return NULL;

    w16(out, 4);
    w16(out+2, (uint16_t)newLen);             /* length 字段（format4 为 2 字节） */
    w16(out+4, 0);                            /* language */
    w16(out+6, (uint16_t)(newSeg*2));         /* segCountX2 */

    uint8_t *wEnd   = out + 14;
    uint8_t *wStart = wEnd + (size_t)newSeg*2 + 2;
    uint8_t *wDelta = wStart + (size_t)newSeg*2;
    uint8_t *wRO    = wDelta + (size_t)newSeg*2;
    uint8_t *wGlyph = wRO    + (size_t)newSeg*2;

    uint32_t total_removed = 0;
    for (int i=0; i<(int)segCount-1; i++){
        uint32_t s = r16(startCode + i*2), e = r16(endCode + i*2);
        if (clip_segments(s, e, list, segs) == 0) total_removed++;
    }

    uint32_t removed_before = 0;              /* 本段之前被整段删除的段数 */
    uint32_t j = 0;                           /* 输出段下标 */
    for (int i=0; i<(int)segCount-1; i++){
        uint32_t s  = r16(startCode + i*2);
        uint32_t e  = r16(endCode   + i*2);
        uint16_t dt = r16(idDelta   + i*2);
        uint16_t ro = r16(idRangeOffset + i*2);

        int ns = clip_segments(s, e, list, segs);
        if (ns == 0){ removed_before++; continue; }

        if (ro == 0){
            /* glyphID = (charCode + delta) & 0xFFFF，拆分后 delta 不变 */
            for (int k=0;k<ns;k++){
                w16(wEnd   + j*2, (uint16_t)segs[k].end);
                w16(wStart + j*2, (uint16_t)segs[k].start);
                w16(wDelta + j*2, dt);
                w16(wRO    + j*2, 0);
                j++;
            }
        } else {
            /* 仅保留左边界未被裁剪的子段（segs[k].start==s），其余丢弃 */
            int32_t removed_after = (int32_t)(total_removed - removed_before);
            int32_t newRO = (int32_t)ro - 2 * removed_after;
            if (newRO < 0 || newRO > 0xFFFF){ free(out); return NULL; }
            for (int k=0;k<ns;k++){
                if (segs[k].start != s) continue;
                w16(wEnd   + j*2, (uint16_t)segs[k].end);
                w16(wStart + j*2, (uint16_t)segs[k].start);
                w16(wDelta + j*2, dt);
                w16(wRO    + j*2, (uint16_t)newRO);
                j++;
            }
        }
    }

    /* ── 哨兵 ── */
    w16(wEnd   + j*2, 0xFFFF);
    w16(wStart + j*2, 0xFFFF);
    w16(wDelta + j*2, 1);
    w16(wRO    + j*2, 0);
    j++;

    memcpy(wGlyph, glyphArray, glyphArrayLen);

    /* ── searchRange / entrySelector / rangeShift ── */
    uint16_t sc = (uint16_t)j;
    uint16_t p2 = 1;
    while ((uint16_t)(p2*2) <= sc && p2 < 0x8000) p2 = (uint16_t)(p2*2);
    uint16_t es = 0, t = p2; while (t > 1){ t >>= 1; es++; }
    w16(out+8,  (uint16_t)(p2*2));            /* searchRange  */
    w16(out+10, es);                          /* entrySelector */
    w16(out+12, (uint16_t)((sc - p2)*2));     /* rangeShift   */

    *out_len = newLen;
    return out;
}

/* 子表自身长度：format 4/6/8/10/13/14 的 length@+2（2字节），
   format 12 的 length@+4（4字节），format 0 固定 262。 */
static uint32_t subtable_self_len(const uint8_t *sub, uint32_t avail){
    uint16_t fmt = r16(sub);
    if (avail < 4) return 0;
    if (fmt == 0) return 262u;
    if (fmt == 12){
        if (avail < 8) return 0;
        uint32_t l = r32(sub+4);
        return (l >= 16 && l <= avail) ? l : 0;
    }
    uint32_t l = r16(sub+2);
    return (l >= 4 && l <= avail) ? l : 0;
}

static void free_all_sub(uint8_t **subs, int n){
    for (int i=0;i<n;i++) free(subs[i]);
}

/* ────────────────────────────────────────────────────
 * 重建整个 cmap 表
 * 返回新缓冲（malloc），*out_len 为新长度；失败返回 NULL。
 * 未改动的子表（format 0/2/6/8/10/13/14）原样搬移。
 * ──────────────────────────────────────────────────── */
static uint8_t *rebuild_cmap(const uint8_t *cmap, uint32_t cmapLen,
                             const RangeList *list, uint32_t *out_len)
{
    if (cmapLen < 4) return NULL;
    uint16_t numSub = r16(cmap+2);
    if (4u + (uint32_t)numSub*8u > cmapLen) return NULL;
    if (numSub == 0) return NULL;

    uint32_t *oldOffsets = (uint32_t *)malloc(sizeof(uint32_t)*numSub);
    uint8_t  **newSub    = (uint8_t **)calloc(numSub, sizeof(uint8_t*));
    uint32_t *newSubLen  = (uint32_t *)malloc(sizeof(uint32_t)*numSub);
    uint32_t *newOffsets = (uint32_t *)malloc(sizeof(uint32_t)*numSub);
    int      *recMap     = (int *)malloc(sizeof(int)*numSub);
    if (!oldOffsets || !newSub || !newSubLen || !newOffsets || !recMap){
        free(oldOffsets); free(newSub); free(newSubLen); free(newOffsets); free(recMap);
        return NULL;
    }

    int unique = 0;
    for (int i=0;i<numSub;i++){
        const uint8_t *rec = cmap + 4 + i*8;
        uint32_t off = r32(rec+4);
        if (off >= cmapLen){ free_all_sub(newSub, unique); free(oldOffsets); free(newSub); free(newSubLen); free(newOffsets); free(recMap); return NULL; }

        int k = -1;
        for (int t=0;t<unique;t++) if (oldOffsets[t]==off){ k=t; break; }
        if (k >= 0){ recMap[i] = k; continue; }

        const uint8_t *sub = cmap + off;
        uint32_t avail = cmapLen - off;
        if (avail < 2){ free_all_sub(newSub, unique); free(oldOffsets); free(newSub); free(newSubLen); free(newOffsets); free(recMap); return NULL; }
        uint16_t fmt = r16(sub);

        uint32_t selfLen = subtable_self_len(sub, avail);
        if (!selfLen){ free_all_sub(newSub, unique); free(oldOffsets); free(newSub); free(newSubLen); free(newOffsets); free(recMap); return NULL; }

        uint8_t *nb = NULL; uint32_t nlen = 0;
        if (fmt==12) nb = rebuild_format12(sub, selfLen, list, &nlen);
        else if (fmt==4) nb = rebuild_format4 (sub, selfLen, list, &nlen);
        else {
            nb = (uint8_t *)malloc(selfLen);
            if (nb){ memcpy(nb, sub, selfLen); nlen = selfLen; }
        }
        if (!nb){ free_all_sub(newSub, unique); free(oldOffsets); free(newSub); free(newSubLen); free(newOffsets); free(recMap); return NULL; }

        oldOffsets[unique] = off;
        newSub[unique]     = nb;
        newSubLen[unique]  = nlen;
        recMap[i]          = unique;
        unique++;
    }

    /* 摆放：记录偏移，先累加 */
    uint32_t headerLen = 4u + (uint32_t)numSub*8u;
    uint32_t cursor = headerLen;
    for (int k=0;k<unique;k++){
        newOffsets[k] = cursor;
        cursor += (newSubLen[k] + 3u) & ~3u;
    }
    if (cursor < headerLen){ /* 溢出保护 */ free_all_sub(newSub, unique); free(oldOffsets); free(newSub); free(newSubLen); free(newOffsets); free(recMap); return NULL; }

    uint8_t *out = (uint8_t *)calloc(1, cursor);
    if (!out){ free_all_sub(newSub, unique); free(oldOffsets); free(newSub); free(newSubLen); free(newOffsets); free(recMap); return NULL; }

    memcpy(out, cmap, 4);                     /* version + numTables */
    for (int i=0;i<numSub;i++){
        const uint8_t *rec = cmap + 4 + i*8;
        uint8_t *dst = out + 4 + i*8;
        memcpy(dst, rec, 4);                  /* platformID + encodingID */
        w32(dst+4, newOffsets[recMap[i]]);    /* 编码记录偏移字段在 +4（不是 +6） */
    }
    for (int k=0;k<unique;k++){
        memcpy(out + newOffsets[k], newSub[k], newSubLen[k]);
    }

    free_all_sub(newSub, unique);
    free(oldOffsets); free(newSub); free(newSubLen); free(newOffsets); free(recMap);
    *out_len = cursor;
    return out;
}

/* ────────────────────────────────────────────────────
 * 写新字体：sfnt 头 + 表目录（cmap 替换，其余原样），
 * 写 <path>.tmp 后原子 rename。
 * ──────────────────────────────────────────────────── */
static int write_font(const char *path, const uint8_t *orig, uint32_t origLen,
                      const uint8_t *newCmap, uint32_t newCmapLen)
{
    if (origLen < 12) return -1;
    uint16_t numTables = r16(orig+4);
    if (12u + (uint32_t)numTables*16u > origLen || numTables == 0) return -1;

    uint32_t headOff = 0, headLen = 0;
    for (int i=0;i<numTables;i++){
        const uint8_t *rec = orig + 12 + i*16;
        if (!memcmp(rec,"head",4)){ headOff = r32(rec+8); headLen = r32(rec+12); }
    }
    if (!headOff || headOff + headLen > origLen || headLen < 12) return -1;

    uint32_t dirLen = 12u + (uint32_t)numTables*16u;
    uint32_t *offs = (uint32_t *)malloc(sizeof(uint32_t)*numTables);
    uint32_t *lens = (uint32_t *)malloc(sizeof(uint32_t)*numTables);
    uint32_t *sums = (uint32_t *)malloc(sizeof(uint32_t)*numTables);
    if (!offs || !lens || !sums){ free(offs); free(lens); free(sums); return -1; }

    uint32_t cursor = dirLen;
    for (int i=0;i<numTables;i++){
        const uint8_t *rec = orig + 12 + i*16;
        uint32_t o, l;
        if (!memcmp(rec,"cmap",4)){ o = cursor; l = newCmapLen; }
        else {
            o = r32(rec+8); l = r32(rec+12);
            if (o + l > origLen){ free(offs); free(lens); free(sums); return -1; }
        }
        offs[i] = cursor; lens[i] = l;
        cursor += (l + 3u) & ~3u;
        if (cursor < offs[i]){ free(offs); free(lens); free(sums); return -1; } /* 溢出保护 */
    }

    for (int i=0;i<numTables;i++){
        const uint8_t *rec = orig + 12 + i*16;
        if (!memcmp(rec,"cmap",4)){
            sums[i] = checksum(newCmap, newCmapLen);
        } else if (!memcmp(rec,"head",4)){
            uint8_t *headCopy = (uint8_t *)malloc(headLen);
            if (!headCopy){ free(offs); free(lens); free(sums); return -1; }
            memcpy(headCopy, orig+headOff, headLen);
            w32(headCopy+8, 0);               /* checkSumAdjustment = 0 后重算 */
            sums[i] = checksum(headCopy, headLen);
            free(headCopy);
        } else {
            sums[i] = r32(rec+4);             /* 其余表数据未变，校验和不变 */
        }
    }

    uint8_t *out = (uint8_t *)calloc(1, cursor);
    if (!out){ free(offs); free(lens); free(sums); return -1; }

    memcpy(out, orig, 4);                     /* sfnt 版本 */
    w16(out+4, numTables);
    /* 二进制搜索参数按规范计算 */
    uint16_t sr = 1, es = 0;
    while ((uint16_t)(sr*2) <= numTables){ sr = (uint16_t)(sr*2); es++; }
    w16(out+6,  (uint16_t)(sr*16));
    w16(out+8,  es);
    w16(out+10, (uint16_t)(numTables*16 - sr*16));

    for (int i=0;i<numTables;i++){
        uint8_t *rec = out + 12 + i*16;
        memcpy(rec, orig + 12 + i*16, 16);
        w32(rec+4, sums[i]);
        w32(rec+8, offs[i]);
        w32(rec+12, lens[i]);

        uint8_t *dst = out + offs[i];
        if (!memcmp(rec,"cmap",4)) memcpy(dst, newCmap, newCmapLen);
        else memcpy(dst, orig + r32(orig+12+i*16+8), lens[i]);
    }

    /* 整字校验和 → head.checkSumAdjustment */
    uint32_t total = checksum(out, cursor);
    for (int i=0;i<numTables;i++){
        if (!memcmp(out+12+i*16,"head",4)){
            w32(out + offs[i] + 8, 0xB1B0AFBAu - total);
            break;
        }
    }
    free(offs); free(lens); free(sums);

    /* 写临时文件 + rename */
    size_t plen = strlen(path);
    char *tmp = (char *)malloc(plen + 8);
    if (!tmp){ free(out); return -1; }
    snprintf(tmp, plen + 8, "%s.tmp", path);

    int fd = open(tmp, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if (fd < 0){ free(out); free(tmp); return -1; }
    size_t written = 0;
    while (written < cursor){
        ssize_t w = write(fd, out + written, cursor - written);
        if (w <= 0){ close(fd); unlink(tmp); free(out); free(tmp); return -1; }
        written += (size_t)w;
    }
    if (fsync(fd) != 0){ close(fd); unlink(tmp); free(out); free(tmp); return -1; }
    close(fd);
    if (rename(tmp, path) != 0){ unlink(tmp); free(out); free(tmp); return -1; }
    free(tmp); free(out);
    return 0;
}

int main(int argc, char **argv){
    if(argc<3){
        printf("Usage: %s font.ttf U+XXXX-U+YYYY[;U+XXXX-U+YYYY...]\n",argv[0]);
        return 1;
    }

    RangeList list; list.count=0;
    char *ranges = strdup(argv[2]);
    char *token  = strtok(ranges, ";,");
    while(token && list.count < MAX_RANGES){
        uint32_t s, e;
        if(sscanf(token,"U+%x-U+%x",&s,&e)==2){
            if (s > e){ uint32_t t=s; s=e; e=t; }   /* 容忍倒序 */
            if (s > 0x10FFFFu) s = 0x10FFFFu;
            if (e > 0x10FFFFu) e = 0x10FFFFu;
            list.ranges[list.count].start = s;
            list.ranges[list.count].end   = e;
            list.count++;
        }
        token = strtok(NULL, ";,");
    }
    free(ranges);
    if(list.count==0){ printf("No valid ranges parsed.\n"); return 2; }

    int fd = open(argv[1], O_RDONLY);
    if(fd<0){ perror("open"); return 1; }
    struct stat st; if (fstat(fd, &st) != 0 || st.st_size <= 0 || st.st_size > 0x7FFFFFFFu){ close(fd); return 1; }
    uint8_t *buf = (uint8_t *)malloc((size_t)st.st_size);
    if (!buf){ close(fd); return 1; }
    size_t rd = 0;
    while (rd < (size_t)st.st_size){
        ssize_t r = read(fd, buf + rd, (size_t)st.st_size - rd);
        if (r <= 0){ free(buf); close(fd); return 1; }
        rd += (size_t)r;
    }
    close(fd);
    uint32_t origLen = (uint32_t)st.st_size;

    uint16_t numTables = r16(buf+4);
    uint32_t cmapOff = 0, cmapLen = 0;
    for(int i=0;i<numTables;i++){
        const uint8_t *rec = buf+12+i*16;
        if(!memcmp(rec,"cmap",4)){ cmapOff=r32(rec+8); cmapLen=r32(rec+12); }
    }
    if(!cmapOff || cmapOff + cmapLen > origLen){
        fprintf(stderr,"No valid cmap table.\n");
        free(buf);
        return 1;
    }

    uint32_t newCmapLen = 0;
    uint8_t *newCmap = rebuild_cmap(buf + cmapOff, cmapLen, &list, &newCmapLen);
    if (!newCmap){
        fprintf(stderr,"cmap rebuild failed.\n");
        free(buf);
        return 1;
    }

    int ret = write_font(argv[1], buf, origLen, newCmap, newCmapLen);
    free(newCmap);
    free(buf);

    if (ret != 0){
        fprintf(stderr,"write failed.\n");
        return 1;
    }
    printf("SAFE CMAP REBUILD COMPLETE\n");
    return 0;
}
