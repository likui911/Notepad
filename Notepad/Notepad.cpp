#include "stdafx.h"
#include "Notepad.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//设置标题栏和WindowClass名称
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_NOTEPAD, szWindowClass, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_NO_NAME, szNoName, MAX_LOADSTRING);
	//注册窗口类
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(NULL, MAKEINTRESOURCE(IDC_NOTEPAD));
	if (hAccelTable == NULL) {
		MessageBox(NULL, TEXT("Accelerator Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	MSG msg;
	// 主消息循环: 
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(hWnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

void InitOpenFileName(HWND hwnd)
{
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = TEXT("文本文档(*txt)\0*.txt\0所有文件(*.*)\0*.*\0\0");
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrDefExt = TEXT("txt");
}

void InitFindReplace(HWND hwnd)
{
	ZeroMemory(&fr, sizeof(fr));
	fr.lStructSize = sizeof(fr);
	fr.hwndOwner = hwnd;
	fr.lpstrFindWhat = szFindWhat;
	fr.wFindWhatLen = 80;
	fr.lpstrReplaceWith = szReplaceWith;
	fr.wReplaceWithLen = 80;
	fr.Flags = FR_FINDNEXT | FR_DOWN | FR_HIDEWHOLEWORD;
}

void InitChooseFont(HWND hwnd)
{
	ZeroMemory(&cf, sizeof(cf));
	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = hwnd;
	cf.lpLogFont = &logFont;
	cf.Flags = CF_SCREENFONTS | CF_NOVERTFONTS | CF_INITTOLOGFONTSTRUCT;
}

BOOL PopFileOpenDialog()
{
	ofn.lpstrFile[0] = '\0';
	return GetOpenFileName(&ofn);
}

BOOL PopFileSaveDialog()
{
	return GetSaveFileName(&ofn);
}

BOOL PopFontChooseDialog()
{
	return ChooseFont(&cf);
}

int AskForSave()
{
	TCHAR szInfo[MAX_PATH + 64];
	wsprintf(szInfo, TEXT("是否保存文件到%s "), szFile[0] ? szFile : TEXT(" 无标题"));
	int i = MessageBox(hEdit, szInfo, szTitle, MB_YESNOCANCEL);
	if (i == IDYES)
	{
		if (!SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_SAVE, 0), 0))
		{
			return IDCANCEL;
		}
	}
	return i;
}

void PopFindWarn(HWND hDlg)
{
	TCHAR pInfo[100];
	wsprintf(pInfo, TEXT("找不到 \"%s\""), fr.lpstrFindWhat);
	MessageBox(hDlg, pInfo, TEXT("查找"), MB_OK | MB_ICONINFORMATION);
}

Encode DetectEncode(const PBYTE pB, DWORD sz)
{
	if (sz < 3)
		return Encode::ANSI;

	if (pB[0] == 0xFF && pB[1] == 0xFE)
	{
		return Encode::UNICODE_LE;
	}
	else if (pB[0] == 0xFE && pB[1] == 0xFF)
	{
		return Encode::UNICODE_BE;
	}
	else if (pB[0] == 0xEF && pB[1] == 0xBB && pB[2] == 0xBF)
	{
		return Encode::UTF_8;
	}
	return Encode::ANSI;
}

BOOL ReadText(HWND hEdit, LPCWSTR szFileName)
{
	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE != hFile)
	{
		DWORD dwFileSize = GetFileSize(hFile, NULL);
		PBYTE pBuffer = (PBYTE)malloc(dwFileSize + 2);
		PBYTE pText;
		PBYTE pConv = nullptr;
		int iLen = 0;
		if (!ReadFile(hFile, pBuffer, dwFileSize, NULL, NULL))
		{
			free(pBuffer);
			return FALSE;
		}
		CloseHandle(hFile);
		pBuffer[dwFileSize] = '\0';
		pBuffer[dwFileSize + 1] = '\0';

		fileEncode = DetectEncode(pBuffer, dwFileSize);
		switch (fileEncode)
		{
		case Encode::UNICODE_BE:
			for (int i = 0; i < (int)dwFileSize - 1; )
			{
				std::swap(pBuffer[i], pBuffer[i + 1]);
				i += 2;
			}
			pText = pBuffer + 2;
			SetWindowText(hEdit, (LPWSTR)pBuffer);
			break;
		case Encode::UNICODE_LE:
			pText = pBuffer + 2;
			SetWindowText(hEdit, (LPWSTR)pText);
			break;
		case Encode::UTF_8:
			iLen = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pBuffer, -1, NULL, 0);
			pConv = (PBYTE)malloc(iLen * sizeof(TCHAR));
			MultiByteToWideChar(
				CP_UTF8, MB_PRECOMPOSED, (LPCCH)(pBuffer), -1, (LPWSTR)pConv, iLen);
			SetWindowText(hEdit, (LPCWSTR)pConv);
			free(pConv);
			break;
		case Encode::ANSI:
			pConv = (PBYTE)malloc(dwFileSize * 2 + 1);
			MultiByteToWideChar(
				CP_ACP, MB_PRECOMPOSED, (LPCCH)pBuffer, -1, (LPWSTR)pConv, dwFileSize * 2 + 1);
			SetWindowText(hEdit, (LPCWSTR)pConv);
			free(pConv);
			break;
		}
		free(pBuffer);
		return TRUE;
	}
	return FALSE;
}

BOOL WriteText()
{
	HANDLE hFile = CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE != hFile)
	{
		BYTE byteOfMark[3];
		int iLen = GetWindowTextLength(hEdit);
		int iDest;
		PBYTE pBuffer = (PBYTE)malloc((iLen + 1) * sizeof(TCHAR));
		PBYTE pConv;
		GetWindowText(hEdit, (LPWSTR)pBuffer, iLen + 1);

		switch (fileEncode)
		{
		case Encode::UNICODE_LE:
			byteOfMark[0] = 0xFF;
			byteOfMark[1] = 0xFE;
			WriteFile(hFile, &byteOfMark, 2, NULL, NULL);
			WriteFile(hFile, pBuffer, (iLen + 1) * sizeof(TCHAR), NULL, NULL);
			break;
		case Encode::UNICODE_BE:
			byteOfMark[0] = 0xFE;
			byteOfMark[1] = 0xFF;
			for (int i = 0; i < int((iLen + 1) * sizeof(TCHAR) - 1); )
			{
				std::swap(pBuffer[i], pBuffer[i + 1]);
				i += 2;
			}
			WriteFile(hFile, &byteOfMark, 2, NULL, NULL);
			WriteFile(hFile, pBuffer, (iLen + 1) * sizeof(TCHAR), NULL, NULL);
			break;
		case Encode::UTF_8:
		{
			byteOfMark[0] = 0xEF;
			byteOfMark[1] = 0xBB;
			byteOfMark[2] = 0xBF;

			WriteFile(hFile, &byteOfMark, 3, NULL, NULL);
			iDest = WideCharToMultiByte(CP_UTF8, WC_COMPOSITECHECK, (LPCWCH)pBuffer, iLen + 1,
				NULL, 0, NULL, NULL);
			pConv = (PBYTE)malloc(iDest);
			WideCharToMultiByte(CP_UTF8, WC_COMPOSITECHECK, (LPCWCH)pBuffer, iLen + 1,
				(LPSTR)pConv, iDest, NULL, NULL);
			WriteFile(hFile, pConv, iDest, NULL, NULL);
			free(pConv);
			break;
		}
		default://ANSI
			iDest = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (LPCWCH)pBuffer, iLen + 1,
				NULL, 0, NULL, NULL);
			pConv = (PBYTE)malloc(iDest);
			WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (LPCWCH)pBuffer, iLen + 1,
				(LPSTR)pConv, iDest, NULL, NULL);
			WriteFile(hFile, pConv, iDest, NULL, NULL);
			free(pConv);
			break;
		}
		CloseHandle(hFile);
		free(pBuffer);
		SendMessage(hEdit, EM_SETMODIFY, FALSE, 0);
	}
	return 0;
}

int FindNext()
{
	int iLen = GetWindowTextLength(hEdit);
	LPWSTR pBuffer = (LPWSTR)malloc((iLen + 1) * sizeof(TCHAR));
	pBuffer[iLen] = '\0';
	GetWindowText(hEdit, (LPWSTR)pBuffer, iLen + 1);
	LPWSTR pFind = NULL;
	if (!(fr.Flags&FR_MATCHCASE))
	{
		for (int i = 0; i < (iLen + 1); ++i)
		{
			pBuffer[i] = towlower(pBuffer[i]);
		}
		for (int j = 0; j < (int)_tcsclen(szFindWhat); ++j)
		{
			szFindWhat[j] = towlower(szFindWhat[j]);
		}
	}

	int nIndex;
	if (fr.Flags&FR_DOWN)
	{
		nIndex = HIWORD(SendMessage(hEdit, EM_GETSEL, 0, 0));
		pFind = _tcsstr(pBuffer + nIndex, szFindWhat);
	}
	else
	{
		nIndex = LOWORD(SendMessage(hEdit, EM_GETSEL, 0, 0));

		LPWSTR pLast = pBuffer + nIndex - 1;
		for (; pLast >= pBuffer; --pLast)
		{
			if (*pLast == *szFindWhat)
			{
				if (_tcsncmp(pLast, szFindWhat, _tcslen(szFindWhat)) == 0)
				{
					pFind = pLast;
					break;
				}
			}
		}
	}
	int idx = (pFind == NULL ? -1 : pFind - pBuffer);
	free(pBuffer);
	return idx;
}

void SetCaption()
{
	TCHAR szCaption[MAX_PATH + 64];
	wsprintf(szCaption, TEXT("%s - %s"), szFile[0] == '\0' ? szNoName : szFileTitle, szTitle);
	SetWindowText(hWnd, szCaption);
}

//注册窗口类
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NOTEPAD));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_NOTEPAD);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//初始化实例
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	hWnd = CreateWindowW(szWindowClass, NULL, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//  目的:    处理主窗口的消息。
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DWORD dwSel;
	switch (message)
	{
	case WM_CREATE:
		hEdit = CreateWindow(TEXT("EDIT"), nullptr,
			WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE |
			ES_LEFT | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_NOHIDESEL,
			0, 0, 0, 0, hWnd, (HMENU)1, hInst, NULL);
		szFile[0] = '\0';
		InitOpenFileName(hWnd);
		InitFindReplace(hWnd);
		InitChooseFont(hWnd);
		uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);
		break;
	case WM_SETFOCUS:
		SetFocus(hEdit);
		break;
	case WM_SIZE:
		//设置文本区大小
		MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), true);
		SetCaption();
		//设置默认字体
		logFont = { -21,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,GB2312_CHARSET ,OUT_STROKE_PRECIS ,
			OUT_CHARACTER_PRECIS ,OUT_STRING_PRECIS ,FIXED_PITCH ,TEXT("宋体") };
		hfont = CreateFontIndirect(&logFont);
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hfont, 0);
		break;
	case WM_INITMENUPOPUP:
		switch (LOWORD(lParam))
		{
		case 1://EDIT Menu
			EnableMenuItem((HMENU)wParam, IDM_UNDO,
				SendMessage(hEdit, EM_CANUNDO, 0, 0) ? MF_ENABLED : MF_GRAYED);
			dwSel = SendMessage(hEdit, EM_GETSEL, 0, 0);
			bool bSel = LOWORD(dwSel) < HIWORD(dwSel) ? MF_ENABLED : MF_GRAYED;
			EnableMenuItem((HMENU)wParam, IDM_CUT, bSel);
			EnableMenuItem((HMENU)wParam, IDM_COPY, bSel);
			EnableMenuItem((HMENU)wParam, IDM_PASTE,
				IsClipboardFormatAvailable(CF_TEXT) ? MF_ENABLED : MF_GRAYED);
			EnableMenuItem((HMENU)wParam, IDM_DEL, bSel);
			break;
		}
		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		BOOL bNeedSave = SendMessage(hEdit, EM_GETMODIFY, 0, 0);
		switch (wmId)
		{
		case IDM_NEW:
			if (bNeedSave&&IDCANCEL == AskForSave())
			{
				return 0;
			}
			szFile[0] = '\0';
			SetWindowText(hEdit, TEXT(""));
			SetCaption();
			break;
		case IDM_OPEN:
			if (bNeedSave&&IDCANCEL == AskForSave())
			{
				return 0;
			}
			if (PopFileOpenDialog())
			{
				if (!ReadText(hEdit, szFile))
				{
					MessageBox(NULL, TEXT("文件打开失败\n请确认文件是否存在，然后重试"),
						TEXT("打开"), MB_OK | MB_ICONWARNING);
					return 0;
				}
				SetCaption();
			}
			break;
		case IDM_SAVE:
			if (!szFile[0])
			{
				SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_SAVEAS, 0), 0);
			}
			WriteText();
			break;
		case IDM_SAVEAS:
			if (PopFileSaveDialog())
			{
				WriteText();
				SetCaption();
			}
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_PROPETYSHEET:
			break;
		case IDM_PRINT:
		{
			break;
		}
		case IDM_UNDO:
			SendMessage(hEdit, WM_UNDO, 0, 0);
			break;
		case IDM_CUT:
			SendMessage(hEdit, WM_CUT, 0, 0);
			break;
		case IDM_COPY:
			SendMessage(hEdit, WM_COPY, 0, 0);
			break;
		case IDM_PASTE:
			SendMessage(hEdit, WM_PASTE, 0, 0);
			break;
		case IDM_DEL:
			SendMessage(hEdit, WM_CLEAR, 0, 0);
			break;
		case IDM_FIND:
			hFindDlg = FindText(&fr);
			break;
		case IDM_FINDNEXT:
		{
			int idx = FindNext();
			if (idx == -1)
			{
				PopFindWarn(hEdit);
				return 0;
			}
			SendMessage(hEdit, EM_SETSEL, idx, idx + _tcsclen(fr.lpstrFindWhat));
			break;
		}
		case IDM_REPLACE:
			hFindDlg = ReplaceText(&fr);
			break;
		case IDM_ALL:
			SendMessage(hEdit, EM_SETSEL, 0, GetWindowTextLength(hEdit));
			break;
		case IDM_FONT:
			if (PopFontChooseDialog())
			{
				DeleteObject(hfont);
				hfont = CreateFontIndirect(&logFont);
				SendMessage(hEdit, WM_SETFONT, (WPARAM)hfont, 0);

				RECT rect;
				GetClientRect(hEdit, &rect);
				InvalidateRect(hEdit, &rect, TRUE);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	if (message == uFindReplaceMsg)
	{
		LPFINDREPLACE lpfr = (LPFINDREPLACE)lParam;
		fr = *lpfr;
		if (lpfr->Flags&FR_DIALOGTERM)
		{
			return 0;
		}
		if (lpfr->Flags&FR_FINDNEXT)
		{
			int idx = FindNext();
			if (idx == -1)
			{
				PopFindWarn(hFindDlg);
				return 0;
			}
			SendMessage(hEdit, EM_SETSEL, idx, idx + _tcsclen(szFindWhat));
			return 0;
		}
		if (lpfr->Flags&FR_REPLACE)
		{
			DWORD dw = SendMessage(hEdit, EM_GETSEL, 0, 0);
			int bg = LOWORD(dw);
			int ed = HIWORD(dw);

			int iLen = GetWindowTextLength(hEdit);
			LPWSTR pBuffer = (LPWSTR)malloc((iLen + 1) * sizeof(TCHAR));
			pBuffer[iLen] = '\0';
			GetWindowText(hEdit, (LPWSTR)pBuffer, iLen + 1);
			LPWSTR pText = pBuffer + bg;

			if ((bg != ed) && _tcsncmp(szFindWhat, pText, lstrlen(szFindWhat)) == 0)
			{
				SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)szReplaceWith);
			}
			free(pBuffer);
			int idx = FindNext();
			if (idx == -1)
			{
				PopFindWarn(hFindDlg);
				return 0;
			}
			SendMessage(hEdit, EM_SETSEL, idx, idx + _tcsclen(szFindWhat));
			return 0;
		}
		if (lpfr->Flags&FR_REPLACEALL)
		{
			SendMessage(hEdit, EM_SETSEL, 0, 0);

			for (int index = FindNext(); index != -1;)
			{
				SendMessage(hEdit, EM_SETSEL, index, index + _tcsclen(szFindWhat));
				SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)szReplaceWith);
				index = FindNext();
			}
			return 0;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
