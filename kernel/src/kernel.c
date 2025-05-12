
#include <bootstructs.h>
#include <exceptions.h>
#include <paging.h>
#include <screen.h>
#include <drive.h>
#include <ahci.h>
#include <gdt.h>
#include <idt.h>
#include <pci.h>

void KernelMain(sBootData sHeader)
{
    DisableInterrupts();

    // Load the GDT
    InitGDT();
    WriteGDT();

    // Load the IDT
    InitIDT();
    RegisterExceptions();

    // Display
    InitScreen(sHeader.sGOP.nWidth, sHeader.sGOP.nHeight, sHeader.sGOP.pFramebuffer);
    SetFGColor(_RGB(255, 255, 255));
    SetBGColor(_RGB(0, 0, 0));
    ClearScreen();
    PrintString("Kernel loaded!\n");
    
    // Paging
    InitPaging(sHeader.pMemoryDescriptor,
               sHeader.nMemoryMapSize, sHeader.nMemoryDescriptorSize,
               sHeader.nLoaderStart, sHeader.nLoaderEnd);
    ReservePages(sHeader.sGOP.pFramebuffer, sHeader.sGOP.nBufferSize / PAGE_SIZE + 1);
    MapPageRangeToIdentity(sHeader.sGOP.pFramebuffer, sHeader.sGOP.nBufferSize / PAGE_SIZE + 1, PF_WRITEABLE);
    PrintString("Paging loaded!\n");
    
    // Devices
    PrintString("Loading devices...\n");
    ScanPCIDevices();
    InitDrives();

    
    for (;;);
}
