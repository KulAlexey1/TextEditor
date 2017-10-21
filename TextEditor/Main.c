#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "resource.h"
#include "View.h"


const char mainWindowClassName[] = "MainWindowClass";

HWND hEdit;
WNDPROC lpEditProc;

OPENFILENAME ofn;
char szOpenedFileName[MAX_PATH] = "";
OPENFILENAME sfn;
char szSavedFileName[MAX_PATH] = "";

FILE *stream;

LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYDOWN:
		{
			switch (LOWORD(wParam))
			{
				case 'O': // Ctrl-O
				{
					if (GetAsyncKeyState(VK_CONTROL)) 
						SendMessage(GetActiveWindow(), WM_COMMAND, ID_FILE_OPEN, 0);					
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
			HFONT hfDefault;
			
			hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_EX_ACCEPTFILES | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
				0, 0, 100, 100, hwnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);

			if (hEdit == NULL)
				MessageBox(hwnd, "Не удалось создать окно редактирования.", "Ошибка", MB_OK | MB_ICONERROR);

			lpEditProc = (WNDPROC)SetWindowLongPtr(hEdit, GWL_WNDPROC, (LONG_PTR)&EditProc);

			hfDefault = GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

			DelCBorders(hwnd);
			DelCBorders(hEdit);
			ofn = InitOFN(hwnd, szOpenedFileName);
			sfn = InitSFN(hwnd, szSavedFileName);

			SetFocus(hEdit);
		}
		break;
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case ID_FILE_CREATE:

					break;
				case ID_FILE_OPEN:
					if (GetOpenFileName(&ofn)) //Действие, после выбора в диалоговом окне файла для открытия
					{						
						char *fileText;
						char paragraph[512];						

						errno_t err = fopen_s(&stream, szOpenedFileName, "r");
						if (err)
							printf_s("Файл %s не был открыт\n", szOpenedFileName);
						else
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

							SetDlgItemText(hwnd, IDC_MAIN_EDIT, fileText);													
							SetFocus(hEdit);

							free(fileText);
							fclose(stream);
						}						
					}
					break;
				case ID_FILE_SAVE:
					if (GetSaveFileName(&sfn))
					{
						// Do something usefull with the filename stored in szFileName 
					}
					break;
			}
		}
		break;		
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case 'O': // Ctrl-O
				{
					if (GetAsyncKeyState(VK_CONTROL)) {
						SendMessage(hwnd, WM_COMMAND, ID_FILE_OPEN, 0);
					}
				}
				break;
			}
		}
		break;
		case WM_SIZE:
		{
			RECT rcClient;

			GetClientRect(hwnd, &rcClient);

			hEdit = GetDlgItem(hwnd, IDC_MAIN_EDIT);
			SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
		}
		break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
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
		"Текстовый редактор",
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