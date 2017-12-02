#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include "resource.h"
#include "View.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//#pragma comment(lib, "comctl32.lib")

const char mainWindowClassName[] = "MainWindowClass";

char appConfigFileName[MAX_PATH] = "AppConfig.txt";

char *windowText;

HWND hwndMenu;

HWND hwndEdit;
WNDPROC lpEditProc;

OPENFILENAME ofn;
char fileName[MAX_PATH];
OPENFILENAME sfn;

FILE *stream;
BOOL isFileSaved = TRUE;

HWND hwndTB;
HWND hwndSB;

TBBUTTON tbB[10];
TBADDBITMAP tbAB;

FINDREPLACE fr;
char findWhat[80];
char replaceWith[80];
HWND hwndF;
HWND hwndR;
UINT uFindReplaceMsg;
char frBuff[80];
int offset;

HFONT hfFont;

typedef struct
{
	LOGFONT lgFont;	
	COLORREF rgbText;
	COLORREF rgbBackground;
	COLORREF rgbCustomBackground[16];
	BOOL isSBShown;
	BOOL isSoundKeysEnabled;
} appInformation;

appInformation *appInfo;

BOOL CALLBACK SaveWarningDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			char text[50 + MAX_PATH];

			if (strlen(fileName) != 0)
				wsprintf(text, "Вы хотите сохранить изменения в файле %s?", fileName);
			else
				wsprintf(text, "Вы хотите сохранить изменения в файле?", fileName);

			SetDlgItemText(hwnd, IDD_SAVEWARNING_TEXT, text);

			return TRUE;
		}		
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDD_SAVEWARNING_SAVE:
					EndDialog(hwnd, IDD_SAVEWARNING_SAVE);
					break;
				case IDD_SAVEWARNING_NOTSAVE:
					EndDialog(hwnd, IDD_SAVEWARNING_NOTSAVE);
					break;
				case IDD_SAVEWARNING_CANCEL:
					EndDialog(hwnd, IDD_SAVEWARNING_CANCEL);
					break;
			}
		}
		break;
		case WM_CLOSE:
			EndDialog(hwnd, IDD_SAVEWARNING_CANCEL);
		break;
		default:
			return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{		
		case WM_LBUTTONUP:
		{
			if (hwndSB != NULL)
			{
				char text[50];
				PCOORD pos = malloc(sizeof(PCOORD));

				getCaretPos(hwndEdit, pos);
				wsprintf(text, "Строка %d, столбец %d", pos->Y, pos->X + 1);

				SendMessage(hwndSB, SB_SETTEXT, 0, (LPARAM)text);

				free(pos);
			}
		}
		break;
		case WM_KEYUP:
		{
			switch (wParam)
			{
				case VK_RIGHT:
				case VK_LEFT:
				case VK_UP:
				case VK_DOWN:
				case VK_HOME:
				case VK_END:
				{
					if (hwndSB != NULL)
					{
						char text[50];
						PCOORD pos = malloc(sizeof(PCOORD));

						getCaretPos(hwndEdit, pos);
						wsprintf(text, "Строка %d, столбец %d", pos->Y, pos->X + 1);

						SendMessage(hwndSB, SB_SETTEXT, 0, (LPARAM)text);

						free(pos);
					}
				}
				break;
			}				
		}
		break;
		case WM_CHAR:
		{
			if (wParam == 14 || wParam == 15 || wParam == 19 || wParam == 6 || wParam == 18)
				hwnd = FindWindow(mainWindowClassName, NULL);
			switch (wParam)
			{
			case 1: // CTRL + A
			{
				SendMessage(hwndEdit, EM_SETSEL, 0, -1);
				SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);

				if (hwndSB != NULL)
				{
					char text[50];
					PCOORD pos = malloc(sizeof(PCOORD));

					getCaretPos(hwndEdit, pos);
					wsprintf(text, "Строка %d, столбец %d", pos->Y, pos->X + 1);

					SendMessage(hwndSB, SB_SETTEXT, 0, (LPARAM)text);

					free(pos);
				}

				return 0;
			}
			break;
			case 14: // CTRL + N
			{
				SendMessage(hwnd, WM_COMMAND, ID_FILE_NEW, lParam);
			}
			break;
			case 15: // CTRL + O
			{
				SendMessage(hwnd, WM_COMMAND, ID_FILE_OPEN, lParam);
			}
			break;
			case 19: // CTRL + S
			{
				SendMessage(hwnd, WM_COMMAND, ID_FILE_SAVE, lParam);
			}
			break;
			case 6: // CTRL + F
			{
				SendMessage(hwnd, WM_COMMAND, ID_EDIT_FIND, lParam);
			}
			break;
			case 18: // CTRL + R
			{
				SendMessage(hwnd, WM_COMMAND, ID_EDIT_REPLACE, lParam);
			}
			break;
			}
		}
		break;
	}
	return CallWindowProc(lpEditProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	if (msg == uFindReplaceMsg)
	{		
		// Получение указателя на структуру FINDREPLACE из lParam
		LPFINDREPLACE lpfr = (LPFINDREPLACE)lParam;			
		
		// Сообщение закрытия 
		if (lpfr->Flags & FR_DIALOGTERM)
		{
			lpfr->Flags ^= FR_DIALOGTERM;
			offset = 0;

			return 0;
		}

		// Cообщение на поиск
		if (lpfr->Flags & FR_FINDNEXT)
		{
			int iSingleLength, length, startPos;
			char *editText, *subStr;

			if ((strlen(frBuff) != 0 && strcmp(frBuff, lpfr->lpstrFindWhat) != 0) || strlen(lpfr->lpstrReplaceWith) == 0)
				offset = 0;

			strcpy_s(frBuff, 80, lpfr->lpstrFindWhat);

			iSingleLength = strlen(lpfr->lpstrFindWhat);

			// Получение длины текста, по которому осуществляется поиск
			length = GetWindowTextLength(hwndEdit);

			editText = malloc((length + 1) * sizeof(char));
			
			GetWindowText(hwndEdit, editText, length + 1);

			// Приравнивание регистров, если не установлена галочка "С учётом регистра"
			if (!(lpfr->Flags & FR_MATCHCASE))
			{
				for (int i = 0; i < length; i++) 
					editText[i] = tolower(editText[i]);				

				for(int i = 0; i < iSingleLength; i++)
					lpfr->lpstrFindWhat[i] = tolower(lpfr->lpstrFindWhat[i]);
			}

			// Функция _tcsstr, в случае обнаружения текста для поиска,
			// возвращает текст из текста в котором осуществлялся поиск,
			// который начинается с позиции найденного текста
			subStr = _tcsstr(editText + offset, lpfr->lpstrFindWhat);
			
			free(editText);

			// Если текст, по которому осуществляется поиск, не содержит строку поиска 
			if (subStr == NULL)
			{
				char buff[140];
								
				offset = 0;

				wsprintf(buff, "Не удалось найти \"%s\".", lpfr->lpstrFindWhat);
				MessageBox(hwnd, buff, "Текстовый редактор", MB_OK | MB_ICONINFORMATION);

				return FALSE;
			}

			// Получение позиций для выделения найденного текста
			startPos = subStr - editText;
			offset = startPos + strlen(lpfr->lpstrFindWhat);

			// Выполнение активации главного окна, 
			// выделение в элементе для управления найденного текста
			SetActiveWindow(hwnd);

			SendMessage(hwndEdit, EM_SETSEL, startPos, offset);
			SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);	

			if (hwndSB != NULL)
			{
				char text[50];
				PCOORD pos = malloc(sizeof(PCOORD));

				getCaretPos(hwndEdit, pos);
				wsprintf(text, "Строка %d, столбец %d", pos->Y, pos->X + 1);

				SendMessage(hwndSB, SB_SETTEXT, 0, (LPARAM)text);

				free(pos);
			}
		}

		// Cообщение на замену
		if (lpfr->Flags & FR_REPLACE)
		{
			DWORD startSel, endSel;			

			SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&startSel, (LPARAM)&endSel);

			//Если есть выделение
			if (startSel != endSel)
			{
				char *editText, *selText;
				int editTextLen;
				
				editTextLen = GetWindowTextLength(hwndEdit);
				editText = malloc((editTextLen + 1) * sizeof(char));
				GetWindowText(hwndEdit, editText, editTextLen + 1);

				selText = calloc(endSel - startSel + 1, sizeof(char));

				strncpy_s(selText, (endSel - startSel + 1) * sizeof(char), editText + startSel, endSel - startSel);

				// Приравнивание регистров, если не установлена галочка "С учётом регистра"
				if (!(lpfr->Flags & FR_MATCHCASE))
				{
					for (int i = 0; i < strlen(selText); i++)
						selText[i] = tolower(selText[i]);

					for (int i = 0; i < strlen(lpfr->lpstrFindWhat); i++)
						lpfr->lpstrFindWhat[i] = tolower(lpfr->lpstrFindWhat[i]);
				}

				if (strcmp(selText, lpfr->lpstrFindWhat) == 0)
					SendMessage(hwndEdit, EM_REPLACESEL, TRUE, lpfr->lpstrReplaceWith);								

				free(editText);
				free(selText);
			}

			lpfr->Flags ^= FR_REPLACE;
			lpfr->Flags += FR_FINDNEXT;

			SendMessage(hwnd, uFindReplaceMsg, 0, (LPARAM)lpfr);
		}

		// Cообщение на замену всего
		if (lpfr->Flags & FR_REPLACEALL)
		{
			char *editText;
			int editTextLen;

			editTextLen = GetWindowTextLength(hwndEdit);
			editText = malloc((editTextLen + 1) * sizeof(char));
			GetWindowText(hwndEdit, editText, editTextLen + 1);

			editText = strReplace(editText, lpfr->lpstrFindWhat, lpfr->lpstrReplaceWith, lpfr->Flags & FR_MATCHCASE);

			SendMessage(hwndEdit, EM_SETSEL, 0, -1);
			SendMessage(hwndEdit, EM_REPLACESEL, TRUE, editText);
			SendMessage(hwndEdit, EM_SETSEL, 0, 0);
			SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);

			if (hwndSB != NULL)
				SendMessage(hwndSB, SB_SETTEXT, 0, (LPARAM)"Строка 1, столбец 1");			

			free(editText);
		}

		return 0;
	}

	switch (msg)
	{
		case WM_CREATE:
		{
			hwndMenu = GetMenu(hwnd);

			hwndTB = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT, 0, 0, 0, 0,
				hwnd, (HMENU)IDC_TOOLBAR, GetModuleHandle(NULL), NULL);

			if (hwndTB == NULL)
				MessageBox(hwnd, "Не удалось создать панель инструментов.", "Ошибка", MB_OK | MB_ICONERROR);

			SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

			tbAB.hInst = HINST_COMMCTRL;
			tbAB.nID = IDB_STD_SMALL_COLOR;

			SendMessage(hwndTB, TB_ADDBITMAP, 0, (LPARAM)&tbAB);
			ZeroMemory(tbB, sizeof(tbB));

			tbB[0].iBitmap = STD_FILENEW;
			tbB[0].fsState = TBSTATE_ENABLED;
			tbB[0].fsStyle = TBSTYLE_BUTTON;
			tbB[0].idCommand = ID_FILE_NEW;

			tbB[1].iBitmap = STD_FILEOPEN;
			tbB[1].fsState = TBSTATE_ENABLED;
			tbB[1].fsStyle = TBSTYLE_BUTTON;
			tbB[1].idCommand = ID_FILE_OPEN;

			tbB[2].iBitmap = STD_FILESAVE;
			tbB[2].fsState = TBSTATE_ENABLED;
			tbB[2].fsStyle = TBSTYLE_BUTTON;
			tbB[2].idCommand = ID_FILE_SAVE;

			tbB[3].iBitmap = STD_UNDO;
			tbB[3].fsState = TBSTATE_ENABLED;
			tbB[3].fsStyle = TBSTYLE_BUTTON;
			tbB[3].idCommand = ID_EDIT_UNDO;

			tbB[4].iBitmap = STD_CUT;
			tbB[4].fsState = TBSTATE_ENABLED;
			tbB[4].fsStyle = TBSTYLE_BUTTON;
			tbB[4].idCommand = ID_EDIT_CUT;

			tbB[5].iBitmap = STD_COPY;
			tbB[5].fsState = TBSTATE_ENABLED;
			tbB[5].fsStyle = TBSTYLE_BUTTON;
			tbB[5].idCommand = ID_EDIT_COPY;

			tbB[6].iBitmap = STD_PASTE;
			tbB[6].fsState = TBSTATE_ENABLED;
			tbB[6].fsStyle = TBSTYLE_BUTTON;
			tbB[6].idCommand = ID_EDIT_INSERT;

			tbB[7].iBitmap = STD_DELETE;
			tbB[7].fsState = TBSTATE_ENABLED;
			tbB[7].fsStyle = TBSTYLE_BUTTON;
			tbB[7].idCommand = ID_EDIT_DELETE;

			tbB[8].iBitmap = STD_FIND;
			tbB[8].fsState = TBSTATE_ENABLED;
			tbB[8].fsStyle = TBSTYLE_BUTTON;
			tbB[8].idCommand = ID_EDIT_FIND;

			tbB[9].iBitmap = STD_REPLACE;
			tbB[9].fsState = TBSTATE_ENABLED;
			tbB[9].fsStyle = TBSTYLE_BUTTON;
			tbB[9].idCommand = ID_EDIT_REPLACE;

			SendMessage(hwndTB, TB_ADDBUTTONS, sizeof(tbB) / sizeof(TBBUTTON), (LPARAM)&tbB);

			hwndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
				WS_CHILD | WS_VISIBLE | ES_NOHIDESEL | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
				0, 0, 100, 100, hwnd, (HMENU)IDC_EDIT, GetModuleHandle(NULL), NULL);

			if (hwndEdit == NULL)
				MessageBox(hwnd, "Не удалось создать окно редактирования.", "Ошибка", MB_OK | MB_ICONERROR);

			lpEditProc = (WNDPROC)SetWindowLongPtr(hwndEdit, GWL_WNDPROC, (LONG_PTR)&EditProc);

			ZeroMemory(&fr, sizeof(FINDREPLACE));
			fr.lStructSize = sizeof(FINDREPLACE);
			fr.hwndOwner = hwnd;
			fr.lpstrFindWhat = findWhat;
			fr.wFindWhatLen = 80;
			fr.lpstrReplaceWith = replaceWith;
			fr.wReplaceWithLen = 80;
			fr.Flags = FR_HIDEWHOLEWORD | FR_HIDEUPDOWN;
			
			uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);

			appInfo = malloc(sizeof(appInformation));					

			//Инициализация значениями по умолчанию
			hfFont = GetStockObject(DEFAULT_GUI_FONT);
			GetObject(hfFont, sizeof(LOGFONT), &appInfo->lgFont);

			appInfo->rgbText = RGB(0, 0, 0);
			appInfo->rgbBackground = RGB(255, 255, 255);
			appInfo->isSBShown = FALSE;
			appInfo->isSoundKeysEnabled = FALSE;
		
			char currentDirectory[MAX_PATH];
			char buff[MAX_PATH];

			GetCurrentDirectory(MAX_PATH, currentDirectory);
			wsprintf(buff, "%s\\%s", currentDirectory, appConfigFileName);		
			strcpy_s(appConfigFileName, sizeof(buff), buff);
			
			errno_t err = fopen_s(&stream, appConfigFileName, "r");

			if (!err)
			{
				if (fread(appInfo, sizeof(appInformation), 1, stream) == 1)
				{
					hfFont = CreateFontIndirect(&appInfo->lgFont);

					SendMessage(hwnd, WM_CTLCOLOREDIT, GetDC(hwndEdit), hwndEdit);
					InvalidateRect(hwnd, NULL, FALSE);
				}

				fclose(stream);
			}

			SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hfFont, MAKELPARAM(FALSE, 0));			

			if (appInfo->isSBShown)
				SendMessage(hwnd, WM_COMMAND, ID_VIEW_STATUSBAR, NULL);
			if (appInfo->isSoundKeysEnabled)
				SendMessage(hwnd, WM_COMMAND, ID_SOUND_KEYS, NULL);			

			DelCBorders(hwnd);
			DelCBorders(hwndEdit);
			ofn = InitOFN(hwnd, fileName);
			sfn = InitSFN(hwnd, fileName);				
		}
		break;		
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				SetFocus(hwndEdit);
			}
		}
		break;
		case WM_CTLCOLOREDIT:
		{
			HDC hdc = (HDC)wParam;
			SetTextColor(hdc, appInfo->rgbText);
			SetBkColor(hdc, appInfo->rgbBackground);	
			return (INT_PTR)CreateSolidBrush(appInfo->rgbBackground);
		}
		break;		
		case WM_COMMAND:
		{			
			switch (LOWORD(wParam))
			{				
				case IDC_EDIT:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						if (hwndSB != NULL)
						{
							char text[50];
							PCOORD pos = malloc(sizeof(PCOORD));

							getCaretPos(hwndEdit, pos);
							wsprintf(text, "Строка %d, столбец %d", pos->Y, pos->X + 1);

							SendMessage(hwndSB, SB_SETTEXT, 0, (LPARAM)text);

							free(pos);
						}

						if (isFileSaved)
						{
							if (strlen(fileName) == 0)
							{
								windowText = (char*)calloc(strlen("Безымянный* – Текстовый редактор") + 1, sizeof(char));
								wsprintf(windowText, "Безымянный* – Текстовый редактор", fileName);
							}
							else
							{
								windowText = (char*)calloc(strlen(fileName) + strlen("* – Текстовый редактор") + 1, sizeof(char));
								wsprintf(windowText, "%s* – Текстовый редактор", fileName);
							}

							SetWindowText(hwnd, windowText);

							free(windowText);
						}

						isFileSaved = FALSE;

						MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
						mii.fMask = MIIM_STATE;
						GetMenuItemInfo(hwndMenu, ID_SOUND_KEYS, FALSE, &mii);
						
						if (mii.fState == MFS_CHECKED)
							Beep(30, 2);
					}
				}
				break;
				case ID_FILE_NEW:
				{
					if (getSaveWarningDialogResult(hwnd, hwndEdit, SaveWarningDlgProc, isFileSaved))
					{
						SetWindowText(hwnd, "Безымянный – Текстовый редактор");
						SetDlgItemText(hwnd, IDC_EDIT, "");

						wsprintf(fileName, "", fileName);
						isFileSaved = TRUE;
					}
				}
				break;
				case ID_FILE_OPEN:
				{
					if (getSaveWarningDialogResult(hwnd, hwndEdit, SaveWarningDlgProc, isFileSaved))
					{					
						if (GetOpenFileName(&ofn)) //Действие, после выбора в диалоговом окне файла для открытия
						{
							char *fileText;
							char paragraph[512];

							windowText = (char*)calloc(strlen(fileName) + strlen(" – Текстовый редактор") + 1, sizeof(char));
							wsprintf(windowText, "%s – Текстовый редактор", fileName);

							SetWindowText(hwnd, windowText);

							free(windowText);

							if (openFile(hwnd, &stream, fileName, "r"))
							{
								for (unsigned i = 0; fgets(paragraph, 512, stream) != NULL; i++)
								{
									if (i == 0)
									{
										// Выделение памяти на 2 байт больше, так как необходимо хранить 
										// терминальный символ и добавленный символ "\r"
										fileText = (char*)calloc(strlen(paragraph) + 2, sizeof(char));
										strcpy_s(fileText, strlen(paragraph) + 1, paragraph);
										append(fileText, "\r", strlen(fileText));
									}
									else
									{
										// Выделение памяти на 2 байта больше, так как необходимо хранить
										// терминальный символ и добавленный символ "\r"
										fileText = (char*)realloc(fileText, strlen(fileText) + strlen(paragraph) + 2);
										append(paragraph, "\r", strlen(paragraph) + 1);
										append(fileText, paragraph, strlen(fileText));
									}
								}

								SetDlgItemText(hwnd, IDC_EDIT, fileText);

								isFileSaved = TRUE;

								free(fileText);
								fclose(stream);								
							}
						}
					}
				}
				break;
				case ID_FILE_SAVE:
				{
					if (strlen(fileName) != 0)
					{
						DWORD dwTextLength;

						dwTextLength = GetWindowTextLength(hwndEdit);
						//Если есть, что сохранять
						if (dwTextLength > 0)
						{
							if (openFile(hwnd, &stream, fileName, "w"))
							{
								char *editText;

								editText = (char*)calloc(dwTextLength + 1, 1);
								GetDlgItemText(hwnd, IDC_EDIT, editText, dwTextLength + 1);
								fwrite(editText, 1, strlen(editText), stream);

								free(editText);
								fclose(stream);

								isFileSaved = TRUE;
								windowText = (char*)calloc(strlen(fileName) + strlen(" – Текстовый редактор") + 1, sizeof(char));
								wsprintf(windowText, "%s – Текстовый редактор", fileName);									

								SetWindowText(hwnd, windowText);

								free(windowText);								
							}
						}
					}
					else
					{
						SendMessage(hwnd, WM_COMMAND, ID_FILE_SAVEAS, lParam);
					}
				}
				break;
				case ID_FILE_SAVEAS:
				{
					if (GetSaveFileName(&sfn))
					{
						DWORD dwTextLength;

						dwTextLength = GetWindowTextLength(hwndEdit);
						//Если есть, что сохранять
						if (dwTextLength > 0)
						{
							if (openFile(hwnd, &stream, fileName, "w"))
							{
								char *editText;

								editText = (char*)calloc(dwTextLength + 1, 1);
								GetDlgItemText(hwnd, IDC_EDIT, editText, dwTextLength + 1);
								fwrite(editText, 1, strlen(editText), stream);

								free(editText);
								fclose(stream);

								isFileSaved = TRUE;
								windowText = (char*)calloc(strlen(fileName) + strlen(" – Текстовый редактор") + 1, sizeof(char));
								wsprintf(windowText, "%s – Текстовый редактор", fileName);

								SetWindowText(hwnd, windowText);

								free(windowText);
							}
						}
					}
				}
				break;
				case ID_EDIT_UNDO:
				{
					SendMessage(hwndEdit, EM_UNDO, wParam, lParam);
				}
				break;
				case ID_EDIT_CUT:
				{
					SendMessage(hwndEdit, WM_CUT, wParam, lParam);
				}
				break;
				case ID_EDIT_COPY:
				{
					SendMessage(hwndEdit, WM_COPY, wParam, lParam);
				}
				break;
				case ID_EDIT_INSERT:
				{
					SendMessage(hwndEdit, WM_PASTE, wParam, lParam);
				}
				break;
				case ID_EDIT_DELETE:
				{
					SendMessage(hwndEdit, WM_CLEAR, wParam, lParam);
				}
				break;
				case ID_EDIT_SELECTALL:
				{
					SendMessage(hwndEdit, EM_SETSEL, 0, -1);
				}
				break;
				case ID_EDIT_LOWERREGISTER:
				{					
					char *editText;
					int caretPos, editTextLen;
				
					SendMessage(hwndEdit, EM_GETSEL, &caretPos, 0);

					editTextLen = GetWindowTextLength(hwndEdit);

					editText = (char*)calloc(editTextLen + 1, 1);
					GetDlgItemText(hwnd, IDC_EDIT, editText, editTextLen + 1);					

					for (int i = 0; i < editTextLen; i++)
						editText[i] = tolower(editText[i]);

					SendMessage(hwndEdit, EM_SETSEL, 0, -1);
					SendMessage(hwndEdit, EM_REPLACESEL, TRUE, editText);
					SendMessage(hwndEdit, EM_SETSEL, caretPos, caretPos);
					SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);

					free(editText);					
				}
				break;
				case ID_EDIT_UPPERREGISTER:
				{
					char *editText;
					int caretPos, editTextLen;

					SendMessage(hwndEdit, EM_GETSEL, &caretPos, 0);

					editTextLen = GetWindowTextLength(hwndEdit);

					editText = (char*)calloc(editTextLen + 1, 1);
					GetDlgItemText(hwnd, IDC_EDIT, editText, editTextLen + 1);

					for (int i = 0; i < editTextLen; i++)
						editText[i] = toupper(editText[i]);

					SendMessage(hwndEdit, EM_SETSEL, 0, -1);
					SendMessage(hwndEdit, EM_REPLACESEL, TRUE, editText);
					SendMessage(hwndEdit, EM_SETSEL, caretPos, caretPos);
					SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);

					free(editText);
				}
				break;
				case ID_EDIT_FIND:
				{
					DestroyWindow(hwndR);
					hwndR = NULL;
					hwndF = FindText(&fr);
				}
				break;
				case ID_EDIT_REPLACE:
				{
					DestroyWindow(hwndF);
					hwndF = NULL;
					hwndR = ReplaceText(&fr);
				}
				break;
				case ID_FORMAT_FONT:
				{			
					CHOOSEFONT cf = { sizeof(CHOOSEFONT) };

					cf.Flags = CF_EFFECTS | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
					cf.hwndOwner = hwnd;
					cf.lpLogFont = &appInfo->lgFont;
					cf.rgbColors = appInfo->rgbText;				

					if (ChooseFont(&cf))
					{
						hfFont = CreateFontIndirect(&appInfo->lgFont);

						if (hfFont)
							SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hfFont, MAKELPARAM(TRUE, 0));
						else						
							MessageBox(hwnd, "Не удалось применить выбранный шрифт.", "Ошибка", MB_OK | MB_ICONEXCLAMATION);
														
						appInfo->rgbText = cf.rgbColors;
						SendMessage(hwnd, WM_CTLCOLOREDIT, GetDC(hwndEdit), hwndEdit);
					}
				}
				break;
				case ID_FORMAT_BACKGROUNDCOLOR:
				{
					CHOOSECOLOR cc = { sizeof(CHOOSECOLOR) };

					cc.Flags = CC_RGBINIT | CC_FULLOPEN | CC_ANYCOLOR;
					cc.hwndOwner = hwnd;
					cc.rgbResult = appInfo->rgbBackground;
					cc.lpCustColors = appInfo->rgbCustomBackground;

					if (ChooseColor(&cc))
					{
						appInfo->rgbBackground = cc.rgbResult;
						SendMessage(hwnd, WM_CTLCOLOREDIT, GetDC(hwndEdit), hwndEdit);
					}
				}
				break;
				case ID_VIEW_STATUSBAR:
				{
					MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
					mii.fMask = MIIM_STATE;
					GetMenuItemInfo(hwndMenu, ID_VIEW_STATUSBAR, FALSE, &mii);
					mii.fState ^= MFS_CHECKED;
					SetMenuItemInfo(hwndMenu, ID_VIEW_STATUSBAR, FALSE, &mii);

					if (mii.fState == MFS_CHECKED)
					{
						appInfo->isSBShown = TRUE;

						hwndSB = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
												hwnd, (HMENU)IDC_STATUSBAR, GetModuleHandle(NULL), NULL);

						if (hwndSB == NULL)
							MessageBox(hwnd, "Не удалось создать строку состояния.", "Ошибка", MB_OK | MB_ICONERROR);

						int statwidths[] = { 230, -1 };

						SendMessage(hwndSB, SB_SETPARTS, sizeof(statwidths) / sizeof(int), (LPARAM)statwidths);

						SendMessage(hwnd, WM_SIZE, 0, 0);
						SendMessage(hwndEdit, WM_LBUTTONUP, 0, 0);
					}
					else
					{
						appInfo->isSBShown = FALSE;

						DestroyWindow(hwndSB);
						hwndSB = NULL;
						SendMessage(hwnd, WM_SIZE, 0, 0);
					}
				}
				break;
				case ID_SOUND_KEYS:
				{
					MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
					mii.fMask = MIIM_STATE;
					GetMenuItemInfo(hwndMenu, ID_SOUND_KEYS, FALSE, &mii);
					mii.fState ^= MFS_CHECKED;
					SetMenuItemInfo(hwndMenu, ID_SOUND_KEYS, FALSE, &mii);	

					if (mii.fState == MFS_CHECKED)
						appInfo->isSoundKeysEnabled = TRUE;
					else
						appInfo->isSoundKeysEnabled = FALSE;
				}
				break;
			}
		}
		break;				
		case WM_SIZE:
		{
			RECT rcTB;
			int tbHeight;
			
			RECT rcSB;
			int sbHeight;

			int editHeight;
			RECT rcClient;
			
			SendMessage(hwndTB, TB_AUTOSIZE, 0, 0);

			GetWindowRect(hwndTB, &rcTB);
			tbHeight = rcTB.bottom - rcTB.top;
			
			SendMessage(hwndSB, WM_SIZE, 0, 0);

			GetWindowRect(hwndSB, &rcSB);
			sbHeight = rcSB.bottom - rcSB.top;

			// Вычисление высоты edit
			GetClientRect(hwnd, &rcClient);

			editHeight = rcClient.bottom - tbHeight - sbHeight;
			SetWindowPos(hwndEdit, NULL, 0, tbHeight, rcClient.right, editHeight, SWP_NOZORDER);
		}
		break;
		case WM_CLOSE:
		{		
			if (getSaveWarningDialogResult(hwnd, hwndEdit, SaveWarningDlgProc, isFileSaved))
			{					
				if (openFile(hwnd, &stream, appConfigFileName, "w"))
				{
					fwrite(appInfo, sizeof(appInformation), 1, stream);

					fclose(stream);
				}			

				free(appInfo);

				DestroyWindow(hwnd);
			}
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	// Регистрация класса Window
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HICON));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
	wc.lpszClassName = mainWindowClassName;
	wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HICONSM));
	
	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Не удалось зарегистрировать класс окна!", "Ошибка!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// Создание окна
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		mainWindowClassName,
		"Безымянный – Текстовый редактор",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 700, 500,
		NULL, NULL, hInstance, NULL);

	if (hwnd == NULL)
	{
		MessageBox(NULL, "Не удалось создать окно!", "Ошибка!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);	

	// Поток сообщений
	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;	
}