// recolor_glyph.c
// Author: 咿云冷雨(Numbersf)
// * 退出码：
// 0 = 成功（无论是上色/还原/换色，都算成功）
// 1 = 参数错误
// 2 = 文件打开/读取失败
// 3 = 这个字体的 cmap 里没有这个码位（非致命，调用方可以继续处理别的文件）
// 4 = 字体内部结构解析失败（非预期的 sfnt/cmap/COLR/CPAL 格式）
// 5 = 写文件失败

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

/* ---------- 大端序读写 ---------- */
static uint16_t ru16(const uint8_t *p) { return (uint16_t)((p[0] << 8) | p[1]); }
static uint32_t ru32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}
static void wu16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)(v >> 8); p[1] = (uint8_t)v; }
static void wu32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24); p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);  p[3] = (uint8_t)v;
}

/* ---------- sfnt 表目录条目 ---------- */
typedef struct {
    char tag[5];
    uint32_t checksum;
    uint32_t offset;
    uint32_t length;
} TableEntry;

/* ---------- COLR 内存表示 ---------- */
typedef struct {
    uint16_t gid;
    uint16_t paletteIndex;
} ColrLayer;

typedef struct {
    uint16_t gid;
    int layerStart;   /* layers 数组里的起始下标 */
    int layerCount;
} ColrBase;

/* ---------- CPAL 颜色 (BGRA 存储顺序，spec如此) ---------- */
typedef struct { uint8_t b, g, r, a; } CpalColor;

/* ---------- 整体读取缓冲区 ---------- */
static uint8_t *read_whole_file(const char *path, long *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long len = ftell(f);
    if (len < 0) { fclose(f); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
    uint8_t *buf = (uint8_t *)malloc((size_t)len);
    if (!buf) { fclose(f); return NULL; }
    size_t got = fread(buf, 1, (size_t)len, f);
    fclose(f);
    if (got != (size_t)len) { free(buf); return NULL; }
    *out_len = len;
    return buf;
}

/* checksum */
static uint32_t calc_checksum(const uint8_t *data, long len) {
    uint32_t sum = 0;
    long i = 0;
    while (i + 4 <= len) {
        sum += ru32(data + i);
        i += 4;
    }
    if (i < len) {
        uint8_t tail[4] = {0, 0, 0, 0};
        long rem = len - i;
        memcpy(tail, data + i, (size_t)rem);
        sum += ru32(tail);
    }
    return sum;
}

/* 解析 sfnt 表目录*/
static int parse_table_directory(const uint8_t *buf, long len,
                                  TableEntry **out_tables, int *out_count) {
    if (len < 12) return -1;
    uint16_t numTables = ru16(buf + 4);
    if (12 + (long)numTables * 16 > len) return -1;

    TableEntry *tables = (TableEntry *)malloc(sizeof(TableEntry) * numTables);
    if (!tables) return -1;

    for (int i = 0; i < numTables; i++) {
        const uint8_t *rec = buf + 12 + i * 16;
        memcpy(tables[i].tag, rec, 4);
        tables[i].tag[4] = 0;
        tables[i].checksum = ru32(rec + 4);
        tables[i].offset = ru32(rec + 8);
        tables[i].length = ru32(rec + 12);
        if ((long)tables[i].offset + (long)tables[i].length > len) {
            free(tables);
            return -1;
        }
    }
    *out_tables = tables;
    *out_count = numTables;
    return 0;
}

static TableEntry *find_table(TableEntry *tables, int n, const char *tag) {
    for (int i = 0; i < n; i++) {
        if (memcmp(tables[i].tag, tag, 4) == 0) return &tables[i];
    }
    return NULL;
}

/* cmap：查码位对应的 glyph ID，优先 format 12，找不到再试 format 4 */
static int cmap_lookup_format4(const uint8_t *sub, uint32_t cp, uint16_t *gid_out) {
    if (cp > 0xFFFF) return -1;
    uint16_t segCountX2 = ru16(sub + 6);
    int segCount = segCountX2 / 2;
    const uint8_t *endCodes = sub + 14;
    const uint8_t *startCodes = endCodes + segCountX2 + 2;
    const uint8_t *idDeltas = startCodes + segCountX2;
    const uint8_t *idRangeOffsets = idDeltas + segCountX2;

    for (int i = 0; i < segCount; i++) {
        uint16_t endCode = ru16(endCodes + i * 2);
        if (cp > endCode) continue;
        uint16_t startCode = ru16(startCodes + i * 2);
        if (cp < startCode) return -1; /* 落在两段之间的空隙 */
        uint16_t idDelta = ru16(idDeltas + i * 2);
        uint16_t idRangeOffset = ru16(idRangeOffsets + i * 2);
        if (idRangeOffset == 0) {
            *gid_out = (uint16_t)((cp + idDelta) & 0xFFFF);
        } else {
            const uint8_t *glyphIdAddr = idRangeOffsets + i * 2 + idRangeOffset
                                          + (uint32_t)(cp - startCode) * 2;
            uint16_t g = ru16(glyphIdAddr);
            if (g == 0) return -1;
            *gid_out = (uint16_t)((g + idDelta) & 0xFFFF);
        }
        return (*gid_out == 0) ? -1 : 0;
    }
    return -1;
}

static int cmap_lookup_format12(const uint8_t *sub, uint32_t cp, uint32_t *gid_out) {
    uint32_t numGroups = ru32(sub + 12);
    const uint8_t *groups = sub + 16;
    /* 分段有序，二分查找 */
    long lo = 0, hi = (long)numGroups - 1;
    while (lo <= hi) {
        long mid = (lo + hi) / 2;
        const uint8_t *g = groups + mid * 12;
        uint32_t startCode = ru32(g);
        uint32_t endCode = ru32(g + 4);
        uint32_t startGid = ru32(g + 8);
        if (cp < startCode) { hi = mid - 1; }
        else if (cp > endCode) { lo = mid + 1; }
        else { *gid_out = startGid + (cp - startCode); return 0; }
    }
    return -1;
}

static int find_cmap_glyph(const uint8_t *buf, TableEntry *tables, int ntables,
                            uint32_t cp, uint32_t *gid_out) {
    TableEntry *cmapTab = find_table(tables, ntables, "cmap");
    if (!cmapTab) return -1;
    const uint8_t *cmap = buf + cmapTab->offset;
    uint16_t numSubtables = ru16(cmap + 2);

    long best_format12_offset = -1, best_format4_offset = -1;
    for (int i = 0; i < numSubtables; i++) {
        const uint8_t *rec = cmap + 4 + i * 8;
        uint16_t platformID = ru16(rec);
        uint16_t encodingID = ru16(rec + 2);
        uint32_t offset = ru32(rec + 4);
        const uint8_t *sub = cmap + offset;
        uint16_t format = ru16(sub);
        if (format == 12 &&
            ((platformID == 3 && encodingID == 10) || platformID == 0)) {
            best_format12_offset = offset;
        }
        if (format == 4 &&
            ((platformID == 3 && encodingID == 1) || platformID == 0)) {
            best_format4_offset = offset;
        }
    }

    if (best_format12_offset >= 0) {
        uint32_t gid;
        if (cmap_lookup_format12(cmap + best_format12_offset, cp, &gid) == 0) {
            *gid_out = gid;
            return 0;
        }
    }
    if (best_format4_offset >= 0) {
        uint16_t gid;
        if (cmap_lookup_format4(cmap + best_format4_offset, cp, &gid) == 0) {
            *gid_out = gid;
            return 0;
        }
    }
    return -1;
}

/* 解析已有 COLR / CPAL*/
static int parse_colr(const uint8_t *buf, TableEntry *colrTab,
                       ColrBase **out_bases, int *out_nbases,
                       ColrLayer **out_layers, int *out_nlayers) {
    if (!colrTab) { *out_nbases = 0; *out_nlayers = 0; *out_bases = NULL; *out_layers = NULL; return 0; }
    const uint8_t *t = buf + colrTab->offset;
    uint16_t version = ru16(t);
    if (version != 0) return -1; /* 只处理 COLR v0 */
    uint16_t numBase = ru16(t + 2);
    uint32_t baseOff = ru32(t + 4);
    uint32_t layerOff = ru32(t + 8);
    uint16_t numLayer = ru16(t + 12);

    ColrBase *bases = (ColrBase *)malloc(sizeof(ColrBase) * (numBase + 1));
    ColrLayer *layers = (ColrLayer *)malloc(sizeof(ColrLayer) * (numLayer + 8));
    if (!bases || !layers) { free(bases); free(layers); return -1; }

    for (int i = 0; i < numLayer; i++) {
        const uint8_t *rec = t + layerOff + i * 4;
        layers[i].gid = ru16(rec);
        layers[i].paletteIndex = ru16(rec + 2);
    }
    for (int i = 0; i < numBase; i++) {
        const uint8_t *rec = t + baseOff + i * 6;
        bases[i].gid = ru16(rec);
        bases[i].layerStart = ru16(rec + 2);
        bases[i].layerCount = ru16(rec + 4);
    }
    *out_bases = bases; *out_nbases = numBase;
    *out_layers = layers; *out_nlayers = numLayer;
    return 0;
}

static int parse_cpal(const uint8_t *buf, TableEntry *cpalTab,
                       CpalColor **out_colors, int *out_ncolors,
                       uint16_t *out_numPalettes) {
    if (!cpalTab) { *out_ncolors = 0; *out_colors = NULL; *out_numPalettes = 1; return 0; }
    const uint8_t *t = buf + cpalTab->offset;
    uint16_t numColorRecords = ru16(t + 6);
    uint32_t colorRecordsOffset = ru32(t + 8);
    uint16_t numPalettes = ru16(t + 4);

    CpalColor *colors = (CpalColor *)malloc(sizeof(CpalColor) * (numColorRecords + 8));
    if (!colors) return -1;
    for (int i = 0; i < numColorRecords; i++) {
        const uint8_t *rec = t + colorRecordsOffset + i * 4;
        colors[i].b = rec[0]; colors[i].g = rec[1]; colors[i].r = rec[2]; colors[i].a = rec[3];
    }
    *out_colors = colors; *out_ncolors = numColorRecords; *out_numPalettes = numPalettes;
    return 0;
}

/* 序列化新的 COLR / CPAL */
static uint8_t *build_colr(ColrBase *bases, int nbases, ColrLayer *layers, int nlayers,
                            long *out_len) {
    /* 按 gid 升序排序 baseGlyphRecords —— spec 要求有序，便于二分查找 */
    for (int i = 0; i < nbases - 1; i++)
        for (int j = 0; j < nbases - 1 - i; j++)
            if (bases[j].gid > bases[j + 1].gid) {
                ColrBase tmp = bases[j]; bases[j] = bases[j + 1]; bases[j + 1] = tmp;
            }

    long len = 14 + (long)nbases * 6 + (long)nlayers * 4;
    uint8_t *out = (uint8_t *)calloc(1, (size_t)len);
    if (!out) return NULL;

    wu16(out + 0, 0);                /* version */
    wu16(out + 2, (uint16_t)nbases);
    wu32(out + 4, 14);               /* baseGlyphRecordsOffset，紧跟在14字节头之后 */
    wu32(out + 8, 14 + (uint32_t)nbases * 6); /* layerRecordsOffset */
    wu16(out + 12, (uint16_t)nlayers);

    uint8_t *p = out + 14;
    for (int i = 0; i < nbases; i++) {
        wu16(p, bases[i].gid); p += 2;
        wu16(p, (uint16_t)bases[i].layerStart); p += 2;
        wu16(p, (uint16_t)bases[i].layerCount); p += 2;
    }
    for (int i = 0; i < nlayers; i++) {
        wu16(p, layers[i].gid); p += 2;
        wu16(p, layers[i].paletteIndex); p += 2;
    }
    *out_len = len;
    return out;
}

static uint8_t *build_cpal(CpalColor *colors, int ncolors, uint16_t numPalettes, long *out_len) {
    long headerLen = 12 + (long)numPalettes * 2;
    long len = headerLen + (long)ncolors * 4;
    uint8_t *out = (uint8_t *)calloc(1, (size_t)len);
    if (!out) return NULL;

    wu16(out + 0, 0);                       /* version */
    wu16(out + 2, (uint16_t)ncolors);        /* numPaletteEntries (每个调色板的颜色数) */
    wu16(out + 4, numPalettes);              /* numPalettes */
    wu16(out + 6, (uint16_t)ncolors);        /* numColorRecords (单调色板情形=numPaletteEntries) */
    wu32(out + 8, (uint32_t)headerLen);      /* colorRecordsArrayOffset */
    for (int i = 0; i < numPalettes; i++) {
        wu16(out + 12 + i * 2, 0);           /* colorRecordIndices[i]，单调色板都是0 */
    }
    uint8_t *p = out + headerLen;
    for (int i = 0; i < ncolors; i++) {
        p[0] = colors[i].b; p[1] = colors[i].g; p[2] = colors[i].r; p[3] = colors[i].a;
        p += 4;
    }
    *out_len = len;
    return out;
}

/* 重建整份 sfnt 文件并写出 */
typedef struct { char tag[5]; const uint8_t *data; long len; int is_head; } OutTable;

static int tag_cmp(const void *a, const void *b) {
    return memcmp(((const OutTable *)a)->tag, ((const OutTable *)b)->tag, 4);
}

static int write_font(const char *path, const uint8_t *origBuf, long origLen,
                       TableEntry *tables, int ntables,
                       const uint8_t *newColr, long newColrLen,
                       const uint8_t *newCpal, long newCpalLen) {
    (void)origLen;
    OutTable *outs = (OutTable *)malloc(sizeof(OutTable) * (ntables + 2));
    if (!outs) return -1;
    int n = 0;
    for (int i = 0; i < ntables; i++) {
        if (memcmp(tables[i].tag, "COLR", 4) == 0) continue;
        if (memcmp(tables[i].tag, "CPAL", 4) == 0) continue;
        memcpy(outs[n].tag, tables[i].tag, 5);
        outs[n].data = origBuf + tables[i].offset;
        outs[n].len = tables[i].length;
        outs[n].is_head = (memcmp(tables[i].tag, "head", 4) == 0);
        n++;
    }
    memcpy(outs[n].tag, "COLR", 5); outs[n].data = newColr; outs[n].len = newColrLen; outs[n].is_head = 0; n++;
    memcpy(outs[n].tag, "CPAL", 5); outs[n].data = newCpal; outs[n].len = newCpalLen; outs[n].is_head = 0; n++;

    qsort(outs, n, sizeof(OutTable), tag_cmp);

    /* head 表需要单独可写的副本，因为要把 checkSumAdjustment 清零再重算 */
    uint8_t *headCopy = NULL;
    long headLen = 0;
    for (int i = 0; i < n; i++) {
        if (outs[i].is_head) {
            headLen = outs[i].len;
            headCopy = (uint8_t *)malloc((size_t)headLen);
            memcpy(headCopy, outs[i].data, (size_t)headLen);
            if (headLen >= 12) wu32(headCopy + 8, 0); /* checkSumAdjustment = 0 */
            outs[i].data = headCopy;
            break;
        }
    }

    /* 计算每个表的 offset（4字节对齐）和 checksum */
    uint16_t numTables = (uint16_t)n;
    uint16_t searchRange = 1, entrySelector = 0, rangeShift;
    while ((uint32_t)(searchRange * 2) <= numTables) { searchRange *= 2; entrySelector++; }
    searchRange *= 16;
    rangeShift = (uint16_t)(numTables * 16 - searchRange);

    long dirLen = 12 + (long)numTables * 16;
    long cursor = dirLen;
    uint32_t *offsets = (uint32_t *)malloc(sizeof(uint32_t) * numTables);
    uint32_t *checksums = (uint32_t *)malloc(sizeof(uint32_t) * numTables);

    for (int i = 0; i < n; i++) {
        offsets[i] = (uint32_t)cursor;
        checksums[i] = calc_checksum(outs[i].data, outs[i].len);
        long padded = ((outs[i].len + 3) / 4) * 4;
        cursor += padded;
    }

    long totalLen = cursor;
    uint8_t *out = (uint8_t *)calloc(1, (size_t)totalLen);
    if (!out) { free(outs); free(offsets); free(checksums); free(headCopy); return -1; }

    /* sfnt header：（支持 0x00010000 / OTTO 等） */
    memcpy(out, origBuf, 4);
    wu16(out + 4, numTables);
    wu16(out + 6, searchRange);
    wu16(out + 8, entrySelector);
    wu16(out + 10, rangeShift);

    for (int i = 0; i < n; i++) {
        uint8_t *rec = out + 12 + i * 16;
        memcpy(rec, outs[i].tag, 4);
        wu32(rec + 4, checksums[i]);
        wu32(rec + 8, offsets[i]);
        wu32(rec + 12, (uint32_t)outs[i].len);
        memcpy(out + offsets[i], outs[i].data, (size_t)outs[i].len);
    }

    /* 整份文件 checksum -> head.checkSumAdjustment */
    uint32_t fileSum = calc_checksum(out, totalLen);
    uint32_t adjustment = 0xB1B0AFBAu - fileSum;
    for (int i = 0; i < n; i++) {
        if (outs[i].is_head) {
            wu32(out + offsets[i] + 8, adjustment);
            break;
        }
    }

    struct stat st;
    mode_t orig_mode = 0644;
    uid_t orig_uid = 0;
    gid_t orig_gid = 0;

    if (stat(path, &st) == 0) {
        orig_mode = st.st_mode & 0777;
        orig_uid = st.st_uid;
        orig_gid = st.st_gid;
    }

    char tmpPath[4096];
    snprintf(tmpPath, sizeof(tmpPath), "%s.recolor_tmp", path);

    FILE *f = fopen(tmpPath, "wb");
    if (!f) { free(outs); free(offsets); free(checksums); free(headCopy); free(out); return -1; }

    size_t written = fwrite(out, 1, (size_t)totalLen, f);
    fflush(f);
    fclose(f);

    if (written != (size_t)totalLen) {
        unlink(tmpPath);
        free(outs); free(offsets); free(checksums); free(headCopy); free(out);
        return -1;
    }

    chmod(tmpPath, orig_mode);
    chown(tmpPath, orig_uid, orig_gid);

    if (rename(tmpPath, path) != 0) {
        unlink(tmpPath);
        free(outs); free(offsets); free(checksums); free(headCopy); free(out);
        return -1;
    }

    free(outs); free(offsets); free(checksums); free(headCopy); free(out);
    return 0;
}

/* 主流程 */
static int hex_to_byte(const char *s) {
    int v = 0;
    for (int i = 0; i < 2; i++) {
        char c = s[i];
        v <<= 4;
        if (c >= '0' && c <= '9') v |= (c - '0');
        else if (c >= 'a' && c <= 'f') v |= (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') v |= (c - 'A' + 10);
        else return -1;
    }
    return v;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "用法: %s <字体路径> <codepoint十六进制> <颜色RRGGBB>\n", argv[0]);
        return 1;
    }
    const char *fontPath = argv[1];
    char *endptr = NULL;
    unsigned long cp_ul = strtoul(argv[2], &endptr, 16);
    if (endptr == argv[2] || *endptr != '\0') {
        fprintf(stderr, "[✗] codepoint 不是合法十六进制: %s\n", argv[2]);
        return 1;
    }
    uint32_t cp = (uint32_t)cp_ul;

    if (strlen(argv[3]) != 6) {
        fprintf(stderr, "[✗] 颜色必须是6位十六进制 RRGGBB: %s\n", argv[3]);
        return 1;
    }
    int r = hex_to_byte(argv[3]);
    int g = hex_to_byte(argv[3] + 2);
    int b = hex_to_byte(argv[3] + 4);
    if (r < 0 || g < 0 || b < 0) {
        fprintf(stderr, "[✗] 颜色不是合法十六进制: %s\n", argv[3]);
        return 1;
    }

    long len;
    uint8_t *buf = read_whole_file(fontPath, &len);
    if (!buf) {
        fprintf(stderr, "[✗] 无法读取文件: %s\n", fontPath);
        return 2;
    }

    TableEntry *tables = NULL;
    int ntables = 0;
    if (parse_table_directory(buf, len, &tables, &ntables) != 0) {
        fprintf(stderr, "[✗] sfnt 表目录解析失败，文件可能已损坏: %s\n", fontPath);
        free(buf);
        return 4;
    }

    uint32_t gid;
    if (find_cmap_glyph(buf, tables, ntables, cp, &gid) != 0) {
        fprintf(stdout, "[SKIP] U+%04X 不在这个字体的 cmap 里: %s\n", cp, fontPath);
        free(tables); free(buf);
        return 3;
    }
    if (gid > 0xFFFF) {
        fprintf(stderr, "[✗] glyph ID 超出 COLR v0 支持范围(uint16): %u\n", gid);
        free(tables); free(buf);
        return 4;
    }

    TableEntry *colrTab = find_table(tables, ntables, "COLR");
    TableEntry *cpalTab = find_table(tables, ntables, "CPAL");

    ColrBase *bases; int nbases;
    ColrLayer *layers; int nlayers;
    if (parse_colr(buf, colrTab, &bases, &nbases, &layers, &nlayers) != 0) {
        fprintf(stderr, "[✗] COLR 表解析失败（不是 v0 或格式异常）: %s\n", fontPath);
        free(tables); free(buf);
        return 4;
    }
    CpalColor *colors; int ncolors; uint16_t numPalettes;
    if (parse_cpal(buf, cpalTab, &colors, &ncolors, &numPalettes) != 0) {
        fprintf(stderr, "[✗] CPAL 表解析失败: %s\n", fontPath);
        free(tables); free(bases); free(layers); free(buf);
        return 4;
    }
    if (numPalettes == 0) numPalettes = 1;

    /* 在调色板里找完全一致的颜色，没有就追加一个 */
    int colorIdx = -1;
    for (int i = 0; i < ncolors; i++) {
        if (colors[i].r == r && colors[i].g == g && colors[i].b == b && colors[i].a == 255) {
            colorIdx = i; break;
        }
    }
    int addedNewColor = 0;
    if (colorIdx < 0) {
        colors = (CpalColor *)realloc(colors, sizeof(CpalColor) * (ncolors + 1));
        colors[ncolors].r = (uint8_t)r; colors[ncolors].g = (uint8_t)g;
        colors[ncolors].b = (uint8_t)b; colors[ncolors].a = 255;
        colorIdx = ncolors;
        ncolors++;
        addedNewColor = 1;
    }

    /* 在现有 COLR base 记录里找这个 glyph */
    int baseIdx = -1;
    for (int i = 0; i < nbases; i++) {
        if (bases[i].gid == gid) { baseIdx = i; break; }
    }

    const char *action;
    if (baseIdx >= 0 && bases[baseIdx].layerCount == 1) {
        int removedLayerIdx = bases[baseIdx].layerStart;
        ColrLayer *existingLayer = &layers[removedLayerIdx];
        if (existingLayer->gid == gid && existingLayer->paletteIndex == colorIdx) {
            for (int i = removedLayerIdx; i < nlayers - 1; i++) layers[i] = layers[i + 1];
            nlayers--;
            for (int i = 0; i < nbases; i++) {
                if (bases[i].layerStart > removedLayerIdx) bases[i].layerStart--;
            }
            for (int i = baseIdx; i < nbases - 1; i++) bases[i] = bases[i + 1];
            nbases--;
            action = "REVERT";
        } else {
            existingLayer->paletteIndex = (uint16_t)colorIdx;
            action = "REPLACE";
        }
    } else {
        bases = (ColrBase *)realloc(bases, sizeof(ColrBase) * (nbases + 1));
        layers = (ColrLayer *)realloc(layers, sizeof(ColrLayer) * (nlayers + 1));
        layers[nlayers].gid = (uint16_t)gid;
        layers[nlayers].paletteIndex = (uint16_t)colorIdx;
        bases[nbases].gid = (uint16_t)gid;
        bases[nbases].layerStart = nlayers;
        bases[nbases].layerCount = 1;
        nlayers++;
        nbases++;
        action = "APPLY";
    }

    long newColrLen = 0, newCpalLen = 0;

    /* 调色板垃圾回收：只保留 layers[] 实际引用到的颜色，重新映射下标，
     * 避免反复 apply/revert/换色之后 CPAL 越堆越大、留一堆没人用的颜色。 */
    {
        int *remap = (int *)malloc(sizeof(int) * (size_t)ncolors);
        int *used = (int *)calloc((size_t)ncolors, sizeof(int));
        for (int i = 0; i < nlayers; i++) used[layers[i].paletteIndex] = 1;

        CpalColor *compact = (CpalColor *)malloc(sizeof(CpalColor) * (size_t)(ncolors > 0 ? ncolors : 1));
        int nc = 0;
        for (int i = 0; i < ncolors; i++) {
            if (used[i]) { compact[nc] = colors[i]; remap[i] = nc; nc++; }
            else { remap[i] = -1; }
        }
        for (int i = 0; i < nlayers; i++) {
            layers[i].paletteIndex = (uint16_t)remap[layers[i].paletteIndex];
        }
        free(colors); colors = compact; ncolors = nc;
        free(used); free(remap);
    }

    uint8_t *newColr = build_colr(bases, nbases, layers, nlayers, &newColrLen);
    uint8_t *newCpal = build_cpal(colors, ncolors, numPalettes, &newCpalLen);
    if (!newColr || !newCpal) {
        fprintf(stderr, "[✗] 内存分配失败\n");
        free(tables); free(bases); free(layers); free(colors); free(buf);
        return 4;
    }

    int wr = write_font(fontPath, buf, len, tables, ntables, newColr, newColrLen, newCpal, newCpalLen);

    free(tables); free(bases); free(layers); free(colors); free(buf);
    free(newColr); free(newCpal);

    if (wr != 0) {
        fprintf(stderr, "[✗] 写文件失败: %s\n", fontPath);
        return 5;
    }

    fprintf(stdout, "[%s] U+%04X -> #%02X%02X%02X (%s)\n",
            action, cp, r, g, b, fontPath);
    (void)addedNewColor;
    return 0;
}