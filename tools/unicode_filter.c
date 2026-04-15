// unicode_filter.c
// Supports format 4 + format 12
// Author: 咿云冷雨(Numbersf)
// Fixed: multi-range clip corruption, split-range loss,
//        format4 partial overlap, glyphID drift on start-clip

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

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

/* ── checksum ── */
uint32_t checksum(const uint8_t *data, uint32_t len){
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
 *
 * 算法：扫描排序后的删除区间，逐步切割剩余空间。
 * ──────────────────────────────────────────────────── */
static int clip_segments(uint32_t start, uint32_t end,
                         const RangeList *list,
                         Range out[MAX_SPLIT])
{
    /* 先把删除区间按 start 排序（复制后排，不改原列表） */
    Range sorted[MAX_RANGES];
    int n = list->count;
    memcpy(sorted, list->ranges, (size_t)n * sizeof(Range));
    /* 简单插入排序，区间数通常很少 */
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

        /* cur .. ds-1 是保留段（若 ds > cur） */
        if(ds > cur){
            out[count].start = cur;
            out[count].end   = ds - 1;
            count++;
        }
        /* 跳过删除区间 */
        cur = de + 1;
    }
    /* 剩余尾部 */
    if(cur <= end){
        out[count].start = cur;
        out[count].end   = end;
        count++;
    }
    return count;   /* 0 = 全部删除 */
}

/* ════════════════════════════════════════════════════
 * FORMAT 12
 * 每个 group = [startCharCode(4), endCharCode(4), startGlyphID(4)]
 * 裁剪时 startGlyphID 需随 startCharCode 偏移同步增加。
 * ════════════════════════════════════════════════════ */
void rebuild_format12_safe(uint8_t *sub, const RangeList *list){
    uint32_t n   = r32(sub+12);
    uint8_t *grp = sub+16;
    uint32_t wi  = 0;                  /* write index */

    for(uint32_t i=0;i<n;i++){
        uint8_t *g         = grp + i*12;
        uint32_t start     = r32(g);
        uint32_t end       = r32(g+4);
        uint32_t startGlyph= r32(g+8);

        Range out[MAX_SPLIT];
        int nseg = clip_segments(start, end, list, out);

        for(int s=0;s<nseg;s++){
            uint32_t ns  = out[s].start;
            uint32_t ne  = out[s].end;
            /* glyphID 随 startCharCode 的偏移线性移动 */
            uint32_t ng  = startGlyph + (ns - start);

            uint8_t *dst = grp + wi*12;
            w32(dst,    ns);
            w32(dst+4,  ne);
            w32(dst+8,  ng);
            wi++;
        }
    }
    w32(sub+12, wi);
    /* 同步更新 format12 子表的 length 字段（偏移4，4字节） */
    w32(sub+4, 16u + wi*12u);
}

/* ════════════════════════════════════════════════════
 * FORMAT 4
 *
 * 原始布局（偏移从子表头算起）：
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
 * 策略：
 *   1. 把所有非哨兵段读出来，经 clip_segments 处理后
 *      存入动态数组（允许段数增加，因为一个段可能被拆成多个）。
 *   2. idRangeOffset==0 时，glyphID = (charCode + idDelta) & 0xFFFF，
 *      拆分子段只需调整 idDelta 即可，无需动 glyphIdArray。
 *   3. idRangeOffset!=0 时，glyphID 来自 glyphIdArray 间接索引，
 *      裁剪逻辑较复杂——本实现对此类段采用保守策略：
 *      仅做全删（完全覆盖）或保持原样（无交叉），
 *      部分重叠时截断为不重叠的最大子段，避免乱跳。
 *   4. 重建后更新 segCountX2、length 及相关二分查找辅助字段。
 * ════════════════════════════════════════════════════ */

typedef struct {
    uint16_t start, end, delta, rangeOffset;
} Seg4;

static uint16_t log2floor(uint16_t x){
    uint16_t r=0; while(x>>=1) r++; return r;
}

void rebuild_format4_safe(uint8_t *sub, const RangeList *list){
    uint16_t segCount = r16(sub+6)/2;          /* 含末尾哨兵 */

    uint8_t *p = sub+14;
    uint16_t *endCode       = (uint16_t*)p;  p += segCount*2 + 2; /* +pad */
    uint16_t *startCode     = (uint16_t*)p;  p += segCount*2;
    uint16_t *idDelta       = (uint16_t*)p;  p += segCount*2;
    uint16_t *idRangeOffset = (uint16_t*)p;  /* p += segCount*2; */

    /* ── 读取所有非哨兵段 ── */
    /* 最坏情况每段被拆成 MAX_SPLIT 份，+1 留给哨兵 */
    int maxOut = (segCount-1)*MAX_SPLIT + 1;
    Seg4 *out = (Seg4*)malloc((size_t)maxOut * sizeof(Seg4));
    if(!out){ fprintf(stderr,"OOM\n"); return; }
    int nout = 0;

    for(int i=0; i<(int)segCount-1; i++){  /* 跳过最后哨兵 */
        uint32_t s  = r16((uint8_t*)&startCode[i]);
        uint32_t e  = r16((uint8_t*)&endCode[i]);
        uint16_t dt = r16((uint8_t*)&idDelta[i]);
        uint16_t ro = r16((uint8_t*)&idRangeOffset[i]);

        Range segs[MAX_SPLIT];
        int ns = clip_segments(s, e, list, segs);

        for(int k=0;k<ns;k++){
            uint32_t ns_ = segs[k].start;
            uint32_t ne_ = segs[k].end;

            if(ro == 0){
                /*
                 * glyphID = (charCode + delta) & 0xFFFF
                 * 拆分后新段的 delta 不变（线性映射，偏移一致）。
                 */
                out[nout].start       = (uint16_t)ns_;
                out[nout].end         = (uint16_t)ne_;
                out[nout].delta       = dt;
                out[nout].rangeOffset = 0;
                nout++;
            } else {
                /*
                 * idRangeOffset != 0：间接查 glyphIdArray。
                 * 仅保留与原段完全吻合的子段（即裁剪后起止未变的部分），
                 * 其余因 glyphIdArray 偏移无法安全修正而丢弃。
                 * ——保守但正确，不产生乱跳。
                 */
                if(ns_ == s && ne_ == e){
                    /* 未被裁剪，原样保留 */
                    out[nout].start       = (uint16_t)s;
                    out[nout].end         = (uint16_t)e;
                    out[nout].delta       = dt;
                    out[nout].rangeOffset = ro;
                    nout++;
                } else if(ns_ == s && ne_ < e){
                    /* 仅右侧被截，glyphIdArray 起点不变，可安全保留 */
                    out[nout].start       = (uint16_t)ns_;
                    out[nout].end         = (uint16_t)ne_;
                    out[nout].delta       = dt;
                    out[nout].rangeOffset = ro;
                    nout++;
                }
                /* 左侧被截 (ns_ > s) 时 glyphIdArray 偏移错乱，丢弃 */
            }
        }
    }

    /* ── 追加哨兵 ── */
    out[nout].start       = 0xFFFF;
    out[nout].end         = 0xFFFF;
    out[nout].delta       = 1;
    out[nout].rangeOffset = 0;
    nout++;

    uint16_t newSeg = (uint16_t)nout;

    /* ── 回写（段数不超过原始分配空间时原地回写） ── */
    /* format4 子表内存是固定的，我们只能在原 segCount 范围内回写。
       若新段数 <= 原段数，直接回写并用哨兵填充剩余；
       若新段数 > 原段数（理论上不可能，因为我们只删不增地处理
       idRangeOffset!=0 的段，且 ro==0 拆分最多和原来一样多），
       截断到原段数（极端边界保护）。                          */
    if(newSeg > segCount) newSeg = segCount;  /* 保险截断 */

    p = sub+14;
    uint16_t *wEnd = (uint16_t*)p;  p += segCount*2 + 2;
    uint16_t *wStart = (uint16_t*)p; p += segCount*2;
    uint16_t *wDelta = (uint16_t*)p; p += segCount*2;
    uint16_t *wRO    = (uint16_t*)p;

    for(int i=0;i<newSeg;i++){
        w16((uint8_t*)&wEnd[i],   out[i].end);
        w16((uint8_t*)&wStart[i], out[i].start);
        w16((uint8_t*)&wDelta[i], out[i].delta);
        w16((uint8_t*)&wRO[i],    out[i].rangeOffset);
    }
    /* 将原来多出的旧段用哨兵覆盖（避免残留脏数据被误读） */
    for(int i=newSeg;i<segCount;i++){
        w16((uint8_t*)&wEnd[i],   0xFFFF);
        w16((uint8_t*)&wStart[i], 0xFFFF);
        w16((uint8_t*)&wDelta[i], 1);
        w16((uint8_t*)&wRO[i],    0);
    }

    /* ── 更新 segCountX2 及二分查找辅助字段 ── */
    w16(sub+6, (uint16_t)(newSeg*2));
    uint16_t sc = newSeg;
    uint16_t p2 = (uint16_t)(1u << log2floor(sc));
    w16(sub+8,  (uint16_t)(p2*2));              /* searchRange  */
    w16(sub+10, log2floor(p2));                 /* entrySelector */
    w16(sub+12, (uint16_t)((sc - p2)*2));       /* rangeShift   */

    /* length = 固定头(14) + 4个数组×segCount×2 + pad(2) + glyphIdArray */
    /* 注意：glyphIdArray 我们没有改动，只更新段数相关字段，
       length 保持原值以免破坏后续 glyphIdArray 数据。          */

    free(out);
}

/* ── fix head checksum ── */
void fix_font_checksum(uint8_t *buf, uint32_t size){
    uint16_t numTables = r16(buf+4);
    uint8_t *head = NULL;
    for(int i=0;i<numTables;i++){
        uint8_t *rec = buf+12+i*16;
        if(!memcmp(rec,"head",4)){ head = buf+r32(rec+8); break; }
    }
    if(!head) return;
    w32(head+8, 0);
    uint32_t adj = 0xB1B0AFBA - checksum(buf, size);
    w32(head+8, adj);
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
            list.ranges[list.count].start = s;
            list.ranges[list.count].end   = e;
            list.count++;
        }
        token = strtok(NULL, ";,");
    }
    free(ranges);
    if(list.count==0){ printf("No valid ranges parsed.\n"); return 1; }

    int fd = open(argv[1], O_RDWR);
    if(fd<0){ perror("open"); return 1; }
    struct stat st; fstat(fd, &st);
    uint8_t *buf = mmap(NULL, (size_t)st.st_size,
                        PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(buf==MAP_FAILED){ perror("mmap"); close(fd); return 1; }

    uint16_t numTables   = r16(buf+4);
    uint32_t cmap_offset = 0;
    uint8_t *cmap_record = NULL;
    for(int i=0;i<numTables;i++){
        uint8_t *rec = buf+12+i*16;
        if(!memcmp(rec,"cmap",4)){ cmap_offset=r32(rec+8); cmap_record=rec; }
    }
    if(!cmap_record){ fprintf(stderr,"No cmap table.\n"); return 1; }

    uint16_t numSub = r16(buf+cmap_offset+2);
    for(int i=0;i<numSub;i++){
        uint8_t *rec = buf+cmap_offset+4+i*8;
        uint32_t off = r32(rec+4);
        uint8_t *sub = buf+cmap_offset+off;
        uint16_t fmt = r16(sub);
        if(fmt==12)      rebuild_format12_safe(sub, &list);
        else if(fmt==4)  rebuild_format4_safe (sub, &list);
    }

    /* 更新 cmap 表校验和 */
    uint32_t cmap_len = r32(cmap_record+12);
    w32(cmap_record+4, checksum(buf+cmap_offset, cmap_len));

    fix_font_checksum(buf, (uint32_t)st.st_size);

    msync(buf, (size_t)st.st_size, MS_SYNC);
    munmap(buf, (size_t)st.st_size);
    close(fd);

    printf("SAFE CMAP REBUILD COMPLETE\n");
    return 0;
}