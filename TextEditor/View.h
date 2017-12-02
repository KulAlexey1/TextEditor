#pragma once
#include <windows.h>     


void DelCBorders(HWND);

OPENFILENAME InitOFN(HWND, char*);
OPENFILENAME InitSFN(HWND, char*);

BOOLEAN openFile(HWND, FILE*, const char*, const char*);

void append(char*, const char*, int);

void getCaretPos(HWND, PCOORD);

BOOL getSaveWarningDialogResult(HWND, HWND, _In_opt_ DLGPROC, BOOL);

char* strReplace(char*, char*, char*, BOOL);