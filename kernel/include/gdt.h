
#ifndef __GDT_H
#define __GDT_H

#include <types.h>

#define KERNEL_CODE_SEGMENT 0x10
#define KERNEL_DATA_SEGMENT 0x20
#define USER_CODE_SEGMENT   0x30
#define USER_DATA_SEGMENT   0x40
#define TASK_STATE_SEGMENT  0x50


typedef struct
{
    WORD  wLimitLow;
    DWORD nBaseLow    : 24;
    BYTE  nAccessByte;
    BYTE  nLimitHigh  : 4;
    BYTE  nFlags      : 4;
    QWORD nBaseHigh   : 40;
    DWORD dwReserved;
} __attribute__((packed)) sSegmentDescriptor;

typedef struct
{
    sSegmentDescriptor nullSegment;
    sSegmentDescriptor kernelCodeSegment;
    sSegmentDescriptor kernelDataSegment;
    sSegmentDescriptor userCodeSegment;
    sSegmentDescriptor userDataSegment;
    sSegmentDescriptor tssSegment;
} sGlobalDescriptorTable;

void MakeSegmentDescriptor(QWORD qwBase, DWORD nLimit, BYTE nAccessByte, BYTE nFlags, sSegmentDescriptor *pSegment);

void InitGDT();
extern void WriteGDT();

#endif // __GDT_H
