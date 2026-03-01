// unicode_filter.c
// Supports format 4 + format 12

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_RANGES 128

typedef struct {
    uint32_t start;
    uint32_t end;
} Range;

typedef struct {
    Range ranges[MAX_RANGES];
    int count;
} RangeList;

// BE helpers
static uint16_t r16(const uint8_t *b){ return (b[0]<<8)|b[1]; }
static uint32_t r32(const uint8_t *b){ return (b[0]<<24)|(b[1]<<16)|(b[2]<<8)|b[3]; }
static void w16(uint8_t *b,uint16_t v){ b[0]=(v>>8)&0xFF; b[1]=v&0xFF; }
static void w32(uint8_t *b,uint32_t v){ b[0]=(v>>24)&0xFF; b[1]=(v>>16)&0xFF; b[2]=(v>>8)&0xFF; b[3]=v&0xFF; }

// checksum
uint32_t checksum(uint8_t *data,uint32_t len){
    uint32_t sum=0;
    uint32_t n=(len+3)&~3;

    for(uint32_t i=0;i<n;i+=4){
        uint32_t v=0;
        if(i+3 < len){
            v=r32(data+i);
        }else{
            uint8_t tmp[4]={0,0,0,0};
            for(int j=0;j<4;j++)
                if(i+j < len) tmp[j]=data[i+j];
            v=r32(tmp);
        }
        sum+=v;
    }
    return sum;
}

// check if this range overlaps any delete range
int should_delete(uint32_t start,uint32_t end,RangeList *list){
    for(int i=0;i<list->count;i++){
        if(!(end < list->ranges[i].start || start > list->ranges[i].end))
            return 1;
    }
    return 0;
}

// format12 safe rebuild
void rebuild_format12_safe(uint8_t *sub,RangeList *list){
    uint32_t n = r32(sub+12);
    uint8_t *grp = sub+16;
    uint32_t writeIndex=0;

    for(uint32_t i=0;i<n;i++){
        uint8_t *g = grp + i*12;
        uint32_t start = r32(g);
        uint32_t end   = r32(g+4);

        if(should_delete(start,end,list)){
            continue;
        }

        if(writeIndex != i)
            memcpy(grp+writeIndex*12,g,12);

        writeIndex++;
    }

    w32(sub+12,writeIndex);
}

// format4 rebuild
void rebuild_format4_safe(uint8_t *sub,RangeList *list){
    uint16_t segCount = r16(sub+6)/2;
    uint8_t *p = sub+14;

    uint16_t *endCode = (uint16_t*)p;
    p += segCount*2;

    p += 2; // reservedPad

    uint16_t *startCode = (uint16_t*)p;
    p += segCount*2;

    uint16_t *idDelta = (uint16_t*)p;
    p += segCount*2;

    uint16_t *idRangeOffset = (uint16_t*)p;
    // glyphIdArray follows but we don't modify it

    for(int i=0;i<segCount;i++){
        uint32_t start = r16((uint8_t*)&startCode[i]);
        uint32_t end   = r16((uint8_t*)&endCode[i]);

        if(start == 0xFFFF) continue;

        if(should_delete(start,end,list)){
            w16((uint8_t*)&idDelta[i],0);
            w16((uint8_t*)&idRangeOffset[i],0);
        }
    }
}

// fix checksumAdjustment in head
void fix_font_checksum(uint8_t *buf,uint32_t size){
    uint16_t numTables=r16(buf+4);
    uint8_t *head=NULL;

    for(int i=0;i<numTables;i++){
        uint8_t *rec=buf+12+i*16;
        if(!memcmp(rec,"head",4)){
            head=buf+r32(rec+8);
            break;
        }
    }

    if(!head) return;

    w32(head+8,0);
    uint32_t sum=checksum(buf,size);
    uint32_t adj=0xB1B0AFBA - sum;
    w32(head+8,adj);
}

int main(int argc,char **argv){
    if(argc<3){
        printf("Usage: %s font.ttf U+XXXX-U+YYYY\n",argv[0]);
        return 1;
    }

    RangeList list;
    list.count=1;
    sscanf(argv[2],"U+%x-U+%x",&list.ranges[0].start,&list.ranges[0].end);

    int fd=open(argv[1],O_RDWR);
    struct stat st;
    fstat(fd,&st);

    uint8_t *buf=mmap(NULL,st.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);

    uint16_t numTables=r16(buf+4);
    uint32_t cmap_offset=0;
    uint8_t *cmap_record=NULL;

    for(int i=0;i<numTables;i++){
        uint8_t *rec=buf+12+i*16;
        if(!memcmp(rec,"cmap",4)){
            cmap_offset=r32(rec+8);
            cmap_record=rec;
        }
    }

    uint16_t numSub=r16(buf+cmap_offset+2);
    for(int i=0;i<numSub;i++){
        uint8_t *rec=buf+cmap_offset+4+i*8;
        uint32_t off=r32(rec+4);
        uint8_t *sub=buf+cmap_offset+off;
        uint16_t format=r16(sub);

        if(format==12)
            rebuild_format12_safe(sub,&list);
        if(format==4)
            rebuild_format4_safe(sub,&list);
    }

    uint32_t cmap_len=r32(cmap_record+12);
    uint32_t sum=checksum(buf+cmap_offset,cmap_len);
    w32(cmap_record+4,sum);

    fix_font_checksum(buf,st.st_size);

    msync(buf,st.st_size,MS_SYNC);
    munmap(buf,st.st_size);
    close(fd);

    printf("SAFE CMAP REBUILD COMPLETE\n");
    return 0;
}