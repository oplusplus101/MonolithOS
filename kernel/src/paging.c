
// Code inspired by: https://github.com/Absurdponcho/PonchoOS

#include <paging.h>
#include <idt.h>
#include <screen.h>
#include <memory.h>
#include <panic.h>

sPageTable *g_pCurrentPageDirectory, *g_pKernelPageDirectory;
sBitmap g_sPageBitmap;
QWORD g_qwMemorySize = 0, g_qwFreeMemory = 0;
QWORD g_qwPageBitmapIndex = 0;

// Returns a page of memory cleared filled with zeroes
PVOID AllocatePage()
{
    for (; g_qwPageBitmapIndex < g_sPageBitmap.qwLength * 8; g_qwPageBitmapIndex++)
    {
        if (GetBitmap(&g_sPageBitmap, g_qwPageBitmapIndex)) continue;

        ReservePage((PVOID) _PAGE_TO_ADDRESS(g_qwPageBitmapIndex));
        // Clear the page
        ZeroMemory((PVOID) _PAGE_TO_ADDRESS(g_qwPageBitmapIndex), PAGE_SIZE);

        return (PVOID) _PAGE_TO_ADDRESS(g_qwPageBitmapIndex);
    }

    _KERNEL_PANIC("Out of pages");
    
    return NULL;
}

PVOID AllocateContinousPages(QWORD qwPages)
{
    for (; g_qwPageBitmapIndex < g_sPageBitmap.qwLength * 8; g_qwPageBitmapIndex++)
    {
        BOOL bAllFree = true;
        for (QWORD i = 0; i < qwPages; i++)
            if (GetBitmap(&g_sPageBitmap, g_qwPageBitmapIndex + i))
            {
                g_qwPageBitmapIndex += i;
                bAllFree = false;
                break;
            }
        
        if (!bAllFree) continue;
        

        ReservePages((PVOID) _PAGE_TO_ADDRESS(g_qwPageBitmapIndex), qwPages);
        // Clear the page
        ZeroMemory((PVOID) _PAGE_TO_ADDRESS(g_qwPageBitmapIndex), PAGE_SIZE * qwPages);

        return (PVOID) _PAGE_TO_ADDRESS(g_qwPageBitmapIndex);
    }

    
    _KERNEL_PANIC("Out of pages");
    return NULL;
}

void FreeContinousPages(PVOID pAddress, QWORD qwPages)
{
    if (pAddress == NULL) return;
    MapPageRangeToIdentity(pAddress, qwPages, PF_WRITEABLE);
    ReturnPages(pAddress, qwPages);
    g_qwPageBitmapIndex = _ADDRESS_TO_PAGE(pAddress);
}

void FreePage(PVOID pAddress)
{
    if (pAddress == NULL) return;
    MapPageToIdentity(pAddress, PF_WRITEABLE);
    ReturnPage(pAddress);
    g_qwPageBitmapIndex = _ADDRESS_TO_PAGE(pAddress);
}

void ReservePage(PVOID pAddress)
{
    QWORD qwIndex = (QWORD) pAddress / PAGE_SIZE;
    
    if (GetBitmap(&g_sPageBitmap, qwIndex)) return;

    if (SetBitmap(&g_sPageBitmap, qwIndex, true))
        g_qwFreeMemory -= PAGE_SIZE;
}

void ReturnPage(PVOID pAddress)
{
    QWORD qwIndex = (QWORD) pAddress / PAGE_SIZE;
    if (!GetBitmap(&g_sPageBitmap, qwIndex)) return;
    if (SetBitmap(&g_sPageBitmap, qwIndex, false))
    {
        g_qwFreeMemory += PAGE_SIZE;
        if (g_qwPageBitmapIndex > qwIndex) g_qwPageBitmapIndex = qwIndex;
    }
}

void ReservePages(PVOID pAddress, QWORD nPages)
{
    for (QWORD i = 0; i < nPages * PAGE_SIZE; i += PAGE_SIZE)
        ReservePage((PVOID) ((QWORD) pAddress + i));
}

void ReturnPages(PVOID pAddress, QWORD nPages)
{
    for (QWORD i = 0; i < nPages * PAGE_SIZE; i += PAGE_SIZE)
        ReturnPage((PVOID) ((QWORD) pAddress + i));
}

PVOID GetPhysicalAddress(PVOID pVirtualAddress)
{
    // Page Directory Pointer
    sPageTableEntry *pEntry = &g_pCurrentPageDirectory->arrEntries[_ADDRESS_TO_PDP_INDEX(pVirtualAddress)];
    if (!(pEntry->qwFlags & PF_PRESENT)) return NULL;
    sPageTable *pPageDirectoryPointer = (sPageTable *) _PAGE_TO_ADDRESS(pEntry->qwAddress);

    // Page Directory
    pEntry = &pPageDirectoryPointer->arrEntries[_ADDRESS_TO_PD_INDEX(pVirtualAddress)];
    if (!(pEntry->qwFlags & PF_PRESENT)) return NULL;
    sPageTable *pPageDirectory = (sPageTable *) _PAGE_TO_ADDRESS(pEntry->qwAddress);

    // Page Table
    pEntry = &pPageDirectory->arrEntries[_ADDRESS_TO_PT_INDEX(pVirtualAddress)];
    if (!(pEntry->qwFlags & PF_PRESENT)) return NULL;
    sPageTable *pPageTable = (sPageTable *) _PAGE_TO_ADDRESS(pEntry->qwAddress);

    // Page Entry
    pEntry = &pPageTable->arrEntries[_ADDRESS_TO_PE_INDEX(pVirtualAddress)];
    return (pEntry->qwFlags & PF_PRESENT) ? (PVOID) _PAGE_TO_ADDRESS(pEntry->qwAddress) : NULL;
}

void MapPage(PVOID pVirtualAddress, PVOID pPhysicalAddress, WORD wFlags)
{
    // Page Directory Pointer
    sPageTableEntry *pEntry = &g_pCurrentPageDirectory->arrEntries[_ADDRESS_TO_PDP_INDEX(pVirtualAddress)];
    sPageTable *pPageDirectoryPointer;
    if (!(pEntry->qwFlags & PF_PRESENT))
    {
        pPageDirectoryPointer = AllocatePage();
        pEntry->qwAddress = _ADDRESS_TO_PAGE(pPageDirectoryPointer);
        pEntry->qwFlags   = PF_PRESENT | PF_WRITEABLE;
        g_pCurrentPageDirectory->arrEntries[_ADDRESS_TO_PDP_INDEX(pVirtualAddress)] = *pEntry;
    }
    else
        pPageDirectoryPointer = (sPageTable *) _PAGE_TO_ADDRESS(pEntry->qwAddress);


    // Page Directory
    pEntry = &pPageDirectoryPointer->arrEntries[_ADDRESS_TO_PD_INDEX(pVirtualAddress)];

    sPageTable *pPageDirectory;
    if (!(pEntry->qwFlags & PF_PRESENT))
    {
        pPageDirectory = AllocatePage();
        pEntry->qwAddress = _ADDRESS_TO_PAGE(pPageDirectory);
        pEntry->qwFlags   = PF_PRESENT | PF_WRITEABLE;
        pPageDirectoryPointer->arrEntries[_ADDRESS_TO_PD_INDEX(pVirtualAddress)] = *pEntry;
    }
    else
        pPageDirectory = (sPageTable *) _PAGE_TO_ADDRESS(pEntry->qwAddress);

    // Page Table
    pEntry = &pPageDirectory->arrEntries[_ADDRESS_TO_PT_INDEX(pVirtualAddress)];

    sPageTable *pPageTable;
    if (!(pEntry->qwFlags & PF_PRESENT))
    {
        pPageTable = AllocatePage();
        pEntry->qwAddress = _ADDRESS_TO_PAGE(pPageTable);
        pEntry->qwFlags   = PF_PRESENT | PF_WRITEABLE;
        pPageDirectory->arrEntries[_ADDRESS_TO_PT_INDEX(pVirtualAddress)] = *pEntry;
    }
    else
        pPageTable = (sPageTable *) _PAGE_TO_ADDRESS(pEntry->qwAddress);

    // Page Entry
    pEntry = &pPageTable->arrEntries[_ADDRESS_TO_PE_INDEX(pVirtualAddress)];
    pEntry->qwAddress = _ADDRESS_TO_PAGE(pPhysicalAddress);
    pEntry->qwFlags   = PF_PRESENT | (wFlags & 0x0FFF);
    
    pPageTable->arrEntries[_ADDRESS_TO_PE_INDEX(pVirtualAddress)] = *pEntry;
}


void MapPageRange(PVOID pVirtualAddress, PVOID pPhysicalAddress, QWORD qwPages, WORD wFlags)
{
    for (QWORD i = 0; i < qwPages * PAGE_SIZE; i += PAGE_SIZE)
        MapPage((PVOID) ((QWORD) pVirtualAddress + i), (PVOID) ((QWORD) pPhysicalAddress + i), wFlags);
}

void MapPageToIdentity(PVOID pPage, WORD wFlags)
{
    MapPage(pPage, pPage, wFlags);
}

void MapPageRangeToIdentity(PVOID pPages, QWORD qwPages, WORD wFlags)
{
    MapPageRange(pPages, pPages, qwPages, wFlags);
}

void InitPaging(sEFIMemoryDescriptor *pMemoryDescriptor,
                QWORD nMemoryMapSize, QWORD nMemoryDescriptorSize,
                QWORD nLoaderStart, QWORD nLoaderEnd)
{

    PVOID pLargestSegment = NULL;
    QWORD nLargestSegmentSize = 0, nMemorySize = 0;

    for (sEFIMemoryDescriptor *pEntry = pMemoryDescriptor;
         (BYTE *) pEntry < (BYTE *) pMemoryDescriptor + nMemoryMapSize;
         pEntry = (sEFIMemoryDescriptor *) ((BYTE *) pEntry + nMemoryDescriptorSize))
    {
        nMemorySize += pEntry->nNumberOfPages * PAGE_SIZE;
        if (pEntry->nType == 7 && pEntry->nNumberOfPages * PAGE_SIZE > nLargestSegmentSize)
        {
            pLargestSegment     = (PVOID) pEntry->nPhysicalStart;
            nLargestSegmentSize = pEntry->nNumberOfPages * PAGE_SIZE;
        }
    }

    g_qwMemorySize = nMemorySize;
    g_qwFreeMemory = nMemorySize;
    
    g_sPageBitmap.pData   = pLargestSegment;
    g_sPageBitmap.qwLength = nMemorySize / PAGE_SIZE / 8 + 1;
    ZeroMemory(g_sPageBitmap.pData, g_sPageBitmap.qwLength);
    
    ReservePages(0, g_qwMemorySize / PAGE_SIZE + 1);
    
    for (QWORD i = 0; i < nMemoryMapSize; i += nMemoryDescriptorSize)
    {
        sEFIMemoryDescriptor *pDesc = (sEFIMemoryDescriptor *) ((QWORD) pMemoryDescriptor + i);
        if (pDesc->nType == 7) ReturnPages((PVOID) pDesc->nPhysicalStart, pDesc->nNumberOfPages);
    }
    
    ReservePages(0, 256); // Reserve the first 1MiB.
    ReservePages(g_sPageBitmap.pData, g_sPageBitmap.qwLength / PAGE_SIZE + 1); // Reserve the bitmap's pages.
    
    // Reserve the bootloader's pages.
    ReservePages((PVOID) nLoaderStart, (nLoaderEnd - nLoaderStart) / PAGE_SIZE + 1);
    
    g_pKernelPageDirectory = AllocatePage();
    g_pCurrentPageDirectory = g_pKernelPageDirectory;

    // Identity map the whole memory.
    MapPageRangeToIdentity(NULL, nMemorySize / PAGE_SIZE + 1, PF_WRITEABLE);

    LoadPageDirectory(g_pCurrentPageDirectory);
}

sPageTable *GetCurrentPageDirectory()
{
    return g_pCurrentPageDirectory;
}

void LoadPageDirectory(sPageTable *pTable)
{
    g_pCurrentPageDirectory = pTable;
    __asm__ volatile ("mov %0, %%cr3" : : "r" (g_pCurrentPageDirectory));
}
