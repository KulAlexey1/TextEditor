#pragma once
#include <windows.h>

void DelCBorders(HWND);

OPENFILENAME InitOFN(HWND, char*);
OPENFILENAME InitSFN(HWND, char*);

void append(char*, const char*, int);