#define _CRT_SECURE_NO_WARNINGS

#include <windowsx.h>
#include "resource.h"
#include <stdio.h>
#include <share.h>
#include "View.h"


void DelCBorders(HWND hwnd)
{
	LONG lExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	lExStyle &= ~(WS_EX_CLIENTEDGE);
	SetWindowLong(hwnd, GWL_EXSTYLE, lExStyle);
}

OPENFILENAME InitOFN(HWND hwnd,  char *szOpenedFileName)
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

OPENFILENAME InitSFN(HWND hwnd, char *szSavedFileName)
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
	errno_t err = fopen_s((FILE**)stream, szOpenedFileName, mode);

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

	free(buf);
}

void getCaretPos(HWND hwndEdit, PCOORD pos)
{	
	DWORD endSel = 0;
	LONG firstCharacter;
	
	// получение строки, на которой находится каретка (параметр wParam = -1, чтобы получить текущую позицию каретки)				
	pos->Y = (LONG)SendMessage(hwndEdit, EM_LINEFROMCHAR, (WPARAM)-1, 0) + 1;
	
	// получение столбца, в котором находится каретка		
	SendMessage(hwndEdit, EM_GETSEL, 0, (LPARAM)&endSel);

	firstCharacter = SendMessage(hwndEdit, EM_LINEINDEX, (WPARAM)-1, 0);
	pos->X = endSel - firstCharacter;
}

BOOL getSaveWarningDialogResult(HWND hwnd, HWND hwndEdit, _In_opt_ DLGPROC lpDialogFunc, BOOL isFileSaved)
{
	BOOL flag = FALSE;

	if (isFileSaved == FALSE)
	{
		int res = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SAVEWARNING), hwnd, lpDialogFunc);

		switch (res)
		{
			case IDD_SAVEWARNING_SAVE:
			{
				SendMessage(hwnd, WM_COMMAND, (WPARAM)ID_FILE_SAVE, (LPARAM)NULL);
				flag = FALSE;				
			}
			break;
			case IDD_SAVEWARNING_NOTSAVE:
			{
				flag = TRUE;
			}
			break;
			case IDD_SAVEWARNING_CANCEL:
			{
				flag = FALSE;
			}
			break;
		}
	}
	else
	{
		flag = TRUE;
	}

	return flag;
}

char* stristr(const char *haystack, const char *needle) {
	int c = tolower((unsigned char)*needle);

	if (c == '\0')
		return (char *)haystack;

	for (; *haystack; haystack++) 
	{
		if (tolower((unsigned char)*haystack) == c) 
		{
			for (unsigned int i = 1; ; i++) 
			{
				if (needle[i] == '\0')
					return (char *)haystack;

				if (tolower((unsigned char)haystack[i]) != tolower((unsigned char)needle[i]))
					break;
			}
		}
	}

	return NULL;
}

char* strReplace(char *text, char *search, char *replace, BOOL withCaseSensitive) {
	char *buffer = NULL;
	char *p = text;
	int len;	

	if (!withCaseSensitive)
	{		
		while ((p = stristr(p, search)))
		{
			len = strlen(text) - strlen(p) + 1 + strlen(replace) + strlen((p + strlen(search)));

			if (buffer != NULL)
				buffer = realloc(buffer, len * sizeof(char));
			else
				buffer = calloc(len, sizeof(char));

			strncpy(buffer, text, strlen(text) - strlen(p));
			buffer[strlen(text) - strlen(p)] = '\0';
			strcat(buffer, replace);
			strcat(buffer, p + strlen(search));

			if (strlen(buffer) > strlen(text))
				text = realloc(text, len * sizeof(char));

			strcpy(text, buffer);

			if (strlen(replace) != 0)
				p++;
		}
	}
	else
	{
		while ((p = strstr(p, search)))
		{
			len = strlen(text) - strlen(p) + 1 + strlen(replace) + strlen((p + strlen(search)));

			if (buffer != NULL)
				buffer = realloc(buffer, len * sizeof(char));
			else
				buffer = calloc(len, sizeof(char));

			strncpy(buffer, text, strlen(text) - strlen(p));
			buffer[strlen(text) - strlen(p)] = '\0';
			strcat(buffer, replace);
			strcat(buffer, p + strlen(search));

			if (strlen(buffer) > strlen(text))
				text = realloc(text, len * sizeof(char));

			strcpy(text, buffer);

			if (strlen(replace) != 0)
				p++;
		}
	}

	free(buffer);

	return text;
}