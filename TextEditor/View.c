#define _CRT_SECURE_NO_WARNINGS

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

	ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "��������� ��������� (*.txt)\0*.txt\0��� ����� (*.*)\0*.*\0";
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

	sfn.lStructSize = sizeof(sfn); // SEE NOTE BELOW
	sfn.hwndOwner = hwnd;
	sfn.lpstrFilter = "��������� ��������� (*.txt)\0*.txt\0��� ����� (*.*)\0*.*\0";
	sfn.lpstrFile = szSavedFileName;
	sfn.nMaxFile = MAX_PATH;
	sfn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	sfn.lpstrDefExt = "txt";

	return sfn;
}

void append(char* subject, const char* insert, int pos) {
	// ��������� ������ �� 2 ���� ������, ��� ��� ���������� ������� ������������ ������ ���� �����
	char *buf = (char*)calloc(strlen(subject) + strlen(insert) + 2, sizeof(char));
	int len;

	strncpy(buf, subject, pos); // �������� � buf �� subject pos-��������
	len = strlen(buf);
	strcpy(buf + len, insert); // �������� � ����� buf ��� ������� �� insert
	len += strlen(insert); 
	strcpy(buf + len, subject + pos); // �������� � ����� buf ���������� � subject �������

	strcpy(subject, buf); // �������� �� buf � subject 
}