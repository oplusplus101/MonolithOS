
#ifndef __SCREEN_H
#define __SCREEN_H

#include <types.h>

typedef struct
{
    BYTE r, g, b;
} __attribute__((packed)) sColour;

#define _RGB(r, g, b) ((sColour) { (r), (g), (b) })

void InitScreen(int nWidth, int nHeight, DWORD *pScreenBuffer);
void DrawPixel(int x, int y, sColour c);
void PrintChar(char c);

void PrintString(const PCHAR sz);
void PrintStringW(const PWCHAR wsz);
void PrintDec(QWORD qw);
void PrintHex(QWORD qw, BYTE nDigits, BOOL bUppercase);
void PrintFormat(const PCHAR sFormat, ...);
void PrintBytes(PVOID pBuffer, QWORD qwLength, WORD wBytesPerLine, BOOL bASCII);
void FillRectangle(int x, int y, int w, int h, sColour c);

void ClearScreen();
void SetCursor(int x, int y);
int GetCursorX();
int GetCursorY();
void SetFGColor(sColour c);
void SetBGColor(sColour c);

#endif // __SCREEN_H
