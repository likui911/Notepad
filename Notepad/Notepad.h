#pragma once

#include "stdafx.h"
#include "resource.h"
#include <algorithm>

#define MAX_LOADSTRING 100

enum class Encode
{
	ANSI = 1, UNICODE_LE, UNICODE_BE, UTF_8
};

// 全局变量: 
HINSTANCE hInst;                                
HWND hEdit;									
HWND hWnd;				
HWND hFindDlg;
UINT uFindReplaceMsg;
TCHAR szNewTitle[MAX_PATH];			            //默认的标题
TCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
TCHAR szFile[MAX_PATH];
TCHAR szFileTitle[MAX_PATH];
TCHAR szFindWhat[80];
TCHAR szReplaceWith[80];
OPENFILENAME ofn;
FINDREPLACE fr;
Encode fileEncode;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int);
void InitOpenFileName(HWND);
void InitFindReplace(HWND);
BOOL PopFileOpenDialog();
BOOL PopFileSaveDialog();
BOOL AskForSave();

Encode DetectEncode(const PBYTE pB, DWORD sz);
BOOL ReadText(HWND, LPCWSTR);
BOOL WriteText();
int FindNext();


