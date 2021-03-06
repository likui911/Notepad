#pragma once

#include "stdafx.h"
#include "resource.h"
#include <algorithm>
#include <vector>

#define MAX_LOADSTRING 100

enum class Encode
{
	ANSI = 1, UNICODE_LE, UNICODE_BE, UTF_8
};

HINSTANCE hInst;
HWND hEdit;
HWND hWnd;
HWND hFindDlg;
UINT uFindReplaceMsg;

TCHAR szTitle[MAX_LOADSTRING];
TCHAR szNoName[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
TCHAR szFile[MAX_PATH];
TCHAR szFileTitle[MAX_PATH];
TCHAR szFindWhat[80];
TCHAR szReplaceWith[80];

OPENFILENAME ofn;
FINDREPLACE fr;
Encode fileEncode;
CHOOSEFONT cf;
LOGFONT logFont;
HFONT hfont;
PRINTDLG pd;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int);
void InitOpenFileName(HWND);
void InitFindReplace(HWND);
void InitChooseFont(HWND);
void InitPrintDlg(HWND);
BOOL PopFileOpenDialog();
BOOL PopFileSaveDialog();
BOOL PopFontChooseDialog();
int AskForSave();
void PopFindWarn(HWND hDlg);

Encode DetectEncode(const PBYTE pB, DWORD sz);
BOOL ReadText(HWND, LPCWSTR);
BOOL WriteText();
int FindNext();
void SetCaption();