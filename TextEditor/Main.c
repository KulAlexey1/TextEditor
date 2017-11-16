#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include "resource.h"
#include "View.h"

#pragma comment(lib, "comctl32.lib")

const char mainWindowClassName[] = "MainWindowClass";
const char closeButtonClassName[] = "CloseButtonClassName";

HWND hwndEdit;
WNDPROC lpEditProc;

OPENFILENAME ofn;
char szOpenedFileName[MAX_PATH] = "";
OPENFILENAME sfn;
char szSavedFileName[MAX_PATH] = "";

FILE *stream;

HWND hwndTB;
HWND hwndSB;

TBBUTTON tbB[10];
TBADDBITMAP tbAB;

LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CHAR:
		{
			switch (wParam)
			{
				case 1: // CTRL + A
				{
					SendMessage(hwnd, EM_SETSEL, 0, -1);
					return 1;
				}
				break;
				case 14: // CTRL + N
				{
					//Doesn't work
					SendMessage(hwnd, WM_COMMAND, ID_FILE_NEW, lParam);
					return 14;
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
	switch (msg)
	{
		case WM_CREATE:
		{
			/*INITCOMMONCONTROLSEX icce;
			icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
			icce.dwICC = ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_USEREX_CLASSES;
			InitCommonControlsEx(&icce);*/

			hwndTB = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT, 0, 0, 0, 0,
									hwnd, (HMENU)IDC_TOOLBAR, GetModuleHandle(NULL), NULL);

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

			hwndSB = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
									hwnd, (HMENU)IDC_STATUSBAR, GetModuleHandle(NULL), NULL);

			int statwidths[] = { 230, -1 };

			SendMessage(hwndSB, SB_SETPARTS, sizeof(statwidths) / sizeof(int), (LPARAM)statwidths);
			SendMessage(hwndSB, SB_SETTEXT, 0, (LPARAM)"Hi there :)");

			/*hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_NEWFILEBITMAP), IMAGE_BITMAP, 20, 20, LR_DEFAULTCOLOR);

			if (hBitmap == NULL)
				MessageBox(hwnd, "Could not load IDB_BALL!", "Error", MB_OK | MB_ICONEXCLAMATION);
			
			hwndButton = CreateWindowEx(0, "Button", "", WS_VISIBLE | WS_CHILD | BS_BITMAP, 4, 2, 20, 20, hwnd, (HMENU)IDC_CLOSE, GetModuleHandle(NULL), 0);

			*/			

			hwndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | WS_EX_ACCEPTFILES | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
				0, 0, 100, 100, hwnd, (HMENU)IDC_EDIT, GetModuleHandle(NULL), NULL);

			if (hwndEdit == NULL)
				MessageBox(hwnd, "Не удалось создать окно редактирования.", "Ошибка", MB_OK | MB_ICONERROR);
			
			lpEditProc = (WNDPROC)SetWindowLongPtr(hwndEdit, GWL_WNDPROC, (LONG_PTR)&EditProc);	
			
			HFONT hfDefault;

			hfDefault = GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));			
			
			DelCBorders(hwnd);
			DelCBorders(hwndEdit);
			ofn = InitOFN(hwnd, szOpenedFileName);
			sfn = InitSFN(hwnd, szSavedFileName);

			SetFocus(hwndEdit);

			/*
			RegisterHotKey(hwnd, 0, MOD_CONTROL, 'N'); // Ctrl-N
			RegisterHotKey(hwnd, 1, MOD_CONTROL, 'O'); // Ctrl-O									   
			RegisterHotKey(hwnd, 2, MOD_CONTROL, 'S'); // Ctrl-S
			RegisterHotKey(hwnd, 3, MOD_CONTROL, 'Z'); // Ctrl-Z	
			RegisterHotKey(hwnd, 4, MOD_CONTROL, 'X'); // Ctrl-X
			RegisterHotKey(hwnd, 5, MOD_CONTROL, 'C'); // Ctrl-C
			RegisterHotKey(hwnd, 6, MOD_CONTROL, 'V'); // Ctrl-V
			RegisterHotKey(hwnd, 7, NULL, VK_DELETE);  // DELETE
			RegisterHotKey(hwnd, 8, MOD_CONTROL, 'A'); // Ctrl-A
			RegisterHotKey(hwnd, 9, MOD_CONTROL, 'F'); // Ctrl-F
			RegisterHotKey(hwnd, 10, MOD_CONTROL, 'H'); // Ctrl-H
			*/
		}
		break;		
		case WM_COMMAND:
		{		
			if (IDC_EDIT == LOWORD(wParam) && EN_CHANGE == HIWORD(wParam))
			{	
				POINT p;												
				char text[17];

				GetCaretPos(&p);
				_itoa_s(p.x, &text, 17, 16);
				
				SendMessage(hwndSB, SB_SETTEXT, 0, (LPARAM)text);
			}
			switch (LOWORD(wParam))
			{					
				case ID_FILE_NEW:
				{
					SetWindowText(hwnd, "Безымянный – Текстовый редактор");
					SetDlgItemText(hwnd, IDC_EDIT, "");
				}
				break;
				case ID_FILE_OPEN:
				{
					if (GetOpenFileName(&ofn)) //Действие, после выбора в диалоговом окне файла для открытия
					{
						char *fileName;
						int fileNameLen;
						char *fileText;
						char paragraph[512];						

						fileNameLen = strlen(szOpenedFileName) + strlen(" – Текстовый редактор") + 1;
						fileName = (char*)calloc(fileNameLen, sizeof(char));						
						strcat_s(fileName, fileNameLen, szOpenedFileName);
						strcat_s(fileName, fileNameLen, " – Текстовый редактор");

						SetWindowText(hwnd, fileName);

						if (openFile(hwnd, &stream, szOpenedFileName, "r"))
						{
							for (unsigned i = 0; fgets(paragraph, 512, stream) != NULL; i++)
							{
								if (i == 0)
								{
									// Выделение памяти на 3 байт больше, так как необходимо хранить терминальный символ
									// и добавленные символы "\r"
									fileText = (char*)calloc(strlen(paragraph) + 3, sizeof(char));
									strcpy_s(fileText, strlen(paragraph) + 1, paragraph);
									append(fileText, "\r", strlen(fileText) - 1);
								}
								else
								{
									// Выделение памяти на 4 байта больше, так как необходимо хранить
									// терминальные символы 2-х строк и добавленные символы "\r"
									fileText = (char*)realloc(fileText, strlen(fileText) + strlen(paragraph) + 4);
									append(paragraph, "\r", strlen(paragraph) - 1);
									append(fileText, paragraph, strlen(fileText));
								}
							}

							SetDlgItemText(hwnd, IDC_EDIT, fileText);
							SetFocus(hwndEdit);

							free(fileText);
							fclose(stream);
						}
					}
				}
				break;
				case ID_FILE_SAVE:
				{
					if (strlen(szOpenedFileName) != 0)
					{
						DWORD dwTextLength;

						dwTextLength = GetWindowTextLength(hwndEdit);
						//Если есть, что сохранять
						if (dwTextLength > 0)
						{
							if (openFile(hwnd, &stream, szOpenedFileName, "w"))
							{
								char *editText;

								editText = (char*)calloc(dwTextLength + 1, 1);
								GetDlgItemText(hwnd, IDC_EDIT, editText, dwTextLength + 1);
								fwrite(editText, 1, strlen(editText), stream);

								free(editText);
								fclose(stream);
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
							if (openFile(hwnd, &stream, szSavedFileName, "w"))
							{
								char *editText;

								editText = (char*)calloc(dwTextLength + 1, 1);
								GetDlgItemText(hwnd, IDC_EDIT, editText, dwTextLength + 1);
								fwrite(editText, 1, strlen(editText), stream);

								free(editText);
								fclose(stream);
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
				case ID_EDIT_FIND:
				{
				}
				break;
				case ID_EDIT_REPLACE:
				{
				}
				break;
				case ID_VIEW_STATUSBAR:
				{
				}
				break;
			}
		}
		break;		
		/*case WM_HOTKEY:
		{
			switch (wParam)
			{
				case 0: // Ctrl-N
					SendMessage(hwnd, WM_COMMAND, ID_FILE_NEW, 0);
					break;
				case 1: // Ctrl-O
					SendMessage(hwnd, WM_COMMAND, ID_FILE_OPEN, 0);
					break;
				case 2: // Ctrl-S
					SendMessage(hwnd, WM_COMMAND, ID_FILE_SAVE, 0);
					break;
				case 3: // Ctrl-Z
					SendMessage(hwnd, WM_COMMAND, ID_EDIT_UNDO, 0);
					break;
				case 4: // Ctrl-X
					SendMessage(hwnd, WM_COMMAND, ID_EDIT_CUT, 0);
					break;
				case 5: // Ctrl-C
					SendMessage(hwnd, WM_COMMAND, ID_EDIT_COPY, 0);
					break;
				case 6: // Ctrl-V
					SendMessage(hwnd, WM_COMMAND, ID_EDIT_INSERT, 0);
					break;
				case 7: // DELETE
					SendMessage(hwnd, WM_COMMAND, ID_EDIT_DELETE, 0);
					break;
				case 8: // Ctrl-A
					SendMessage(hwnd, WM_COMMAND, ID_EDIT_SELECTALL, 0);
					break;
				case 9: // Ctrl-F
					SendMessage(hwnd, WM_COMMAND, ID_EDIT_FIND, 0);
					break;
				case 10: // Ctrl-H
					SendMessage(hwnd, WM_COMMAND, ID_EDIT_REPLACE, 0);
					break;					
			}
		}
		break;*/
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
			/*
			UnregisterHotKey(hwnd, 0); // Ctrl-N
			UnregisterHotKey(hwnd, 1); // Ctrl-O			
			UnregisterHotKey(hwnd, 2); // Ctrl-S
			UnregisterHotKey(hwnd, 3); // Ctrl-Z
			UnregisterHotKey(hwnd, 4); // Ctrl-X
			UnregisterHotKey(hwnd, 5); // Ctrl-C
			UnregisterHotKey(hwnd, 6); // Ctrl-V
			UnregisterHotKey(hwnd, 7); // DELETE
			UnregisterHotKey(hwnd, 8); // Ctrl-A
			UnregisterHotKey(hwnd, 9); // Ctrl-F
			UnregisterHotKey(hwnd, 10); // Ctrl-H
			*/
			DestroyWindow(hwnd);			
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