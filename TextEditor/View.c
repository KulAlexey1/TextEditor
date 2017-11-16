#define _CRT_SECURE_NO_WARNINGS

#include <windowsx.h>
#include <stdio.h>
#include <share.h>
#include "View.h"

void DelCBorders(HWND hwnd)
{
	LONG lExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	lExStyle &= ~(WS_EX_CLIENTEDGE);
	SetWindowLong(hwnd, GWL_EXSTYLE, lExStyle);
}

OPENFILENAME InitOFN(HWND hwnd,  char *szOpenedFileName[MAX_PATH])
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "Текстовые документы (*.txt)\0*.txt\0Все файлы (*.*)\0*.*\0";
	ofn.lpstrFile = szOpenedFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "txt";
	
	return ofn;
}

OPENFILENAME InitSFN(HWND hwnd, char *szSavedFileName[MAX_PATH])
{
	OPENFILENAME sfn;

	ZeroMemory(&sfn, sizeof(sfn));

	sfn.lStructSize = sizeof(sfn);
	sfn.hwndOwner = hwnd;
	sfn.lpstrFilter = "Текстовые документы (*.txt)\0*.txt\0Все файлы (*.*)\0*.*\0";
	sfn.lpstrFile = szSavedFileName;
	sfn.nMaxFile = MAX_PATH;
	sfn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	sfn.lpstrDefExt = "txt";

	return sfn;
}

BOOLEAN openFile(HWND hwnd, FILE *stream, const char *szOpenedFileName, const char *mode)
{
	errno_t err = fopen_s(stream, szOpenedFileName, mode);

	if (err)
	{
		char text[512];
		sprintf_s(text, 512, "Не удалось открыть файл \"%s\".", szOpenedFileName);
		MessageBox(hwnd, text, "Ошибка", MB_OK | MB_ICONERROR);
		
		return FALSE;
	}

	return TRUE;
}

void append(char *subject, const char *insert, int pos) {
	// Выделение памяти на 2 байт больше, так как необходимо хранить терминальный символ двух строк
	char *buf = (char*)calloc(strlen(subject) + strlen(insert) + 2, sizeof(char));
	int len;

	strncpy(buf, subject, pos); // Копируем в buf из subject pos-символов
	len = strlen(buf);
	strcpy(buf + len, insert); // Копируем в конец buf все символы из insert
	len += strlen(insert); 
	strcpy(buf + len, subject + pos); // Копируем в конец buf оставшиеся в subject символы

	strcpy(subject, buf); // Копируем из buf в subject 
}