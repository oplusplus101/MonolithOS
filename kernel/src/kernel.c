
#include <bootstructs.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

void KernelMain(sBootData sHeader)
{
    // Test Pattern from: https://youtu.be/hxOw_p0kLfI?t=42
    for (int y = 0; y < sHeader.sGOP.nHeight; y++)
    {
        for (int x = 0; x < sHeader.sGOP.nWidth; x++)
        {
            int l = min(0x1FF >> min(min(min(min(x, y), sHeader.sGOP.nWidth - 1 - x), sHeader.sGOP.nHeight - 1 - y), 31u), 255);
            int d = 50;
            ((DWORD *) sHeader.sGOP.pFramebuffer)[x + y * sHeader.sGOP.nWidth] = 65536 * min(max((int) ((~x & ~y) & 0xFF) - d, l), 255) +
                                                                          256   * min(max((int) (( x & ~y) & 0xFF) - d, l), 255) +
                                                                                  min(max((int) ((~x &  y) & 0xFF) - d, l), 255);
        }
    }
}
