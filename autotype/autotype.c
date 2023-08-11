#include <Windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdlib.h>

#include <CommCtrl.h>
#pragma comment(lib,"Comctl32.lib")

#define IDC_SYSLINK 0x4000
#define IDT_TIMER1 1001


#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


const WCHAR g_wszClassName[] = L"AutoTyperWndClass1";

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OnNotify(HWND hWnd, INT nSrcCtrl, LPNMHDR pNMHdr);
BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);

VOID CALLBACK TimerProc(HWND hwSrc, UINT Msg, UINT_PTR puTID, DWORD dwTime);
ATOM RegisterWCEX(HINSTANCE hInstance);
VOID SendKeys(WCHAR *szTypeKeys, INT cchTypeKeys);

VOID CALLBACK TimerProc(HWND hWnd, UINT Msg, UINT_PTR puTID, DWORD dwTime)
{
	HWND hEditTxt, hRandMS;
	WCHAR *szWndTxt;
	HANDLE hHeap = GetProcessHeap();
	INT nTLen;
	
	hEditTxt = FindWindowExW(hWnd, NULL, L"Edit", NULL);
	hRandMS = FindWindowExW(hWnd, hEditTxt, L"Edit", NULL);
	hRandMS = FindWindowExW(hWnd, hRandMS, L"Edit", NULL);
	nTLen = GetWindowTextLengthW(hEditTxt);
	szWndTxt = (WCHAR*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, (nTLen+2)*sizeof(WCHAR));
	if (NULL == szWndTxt)
	{
		MessageBoxW(NULL, L"Your system is out of memory.", L"Error", MB_OK | MB_ICONSTOP);
		ExitProcess(ERROR_OUTOFMEMORY);
	}
	GetWindowTextW(hEditTxt, szWndTxt, nTLen+1);
	//MessageBox(0, _T("In TimerProc"), _T("Info"), MB_OK);
	if (!((GetWindowLongW(hRandMS, GWL_STYLE)&WS_DISABLED) == WS_DISABLED))
	{
		WCHAR *szRMS;
		INT nRLen = GetWindowTextLengthW(hRandMS), nSleepMS;
		srand((UINT)GetTickCount64());
		szRMS = (WCHAR*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, (nRLen + 2)*sizeof(WCHAR));
		if(NULL == szRMS)
		{
			MessageBoxW(NULL, L"Your system is out of memory.", L"Error", MB_OK | MB_ICONSTOP);
			ExitProcess(ERROR_OUTOFMEMORY);
		}
		GetWindowTextW(hRandMS, szRMS, nRLen+1);
		swscanf_s(szRMS, L"%d", &nSleepMS);
		if (nSleepMS < 0 || nSleepMS > 5000)
		{
			nSleepMS = 1000;
		}
		HeapFree(hHeap, 0, szRMS);
		Sleep(rand()%nSleepMS);
	}
	
	SendKeys(szWndTxt, nTLen);
	HeapFree(hHeap, 0, szWndTxt);
	CloseHandle(hHeap);
}

INT APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ LPWSTR lpCmdLine,
	_In_ INT nShowCmd)
{
	HWND hWnd;
	NONCLIENTMETRICSW ncm;
	HFONT hfDefault;
	MSG Msg;

	ZeroMemory(&ncm, sizeof(NONCLIENTMETRICSW));
	ncm.cbSize = sizeof(NONCLIENTMETRICSW);
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, FALSE);
	hfDefault = CreateFontIndirectW(&ncm.lfMessageFont);

	if (RegisterWCEX(hInstance)==(ATOM)0)
	{
		MessageBoxW(0, L"Window registration failed", L"Error", MB_OK | MB_ICONSTOP);
		return -1;
	}
		
	hWnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, g_wszClassName, L"AutoTyper", WS_VISIBLE | WS_SYSMENU, 100, 100, 400, 235, NULL, NULL, hInstance, NULL);
	if (NULL == hWnd)
	{
		MessageBoxW(0, L"Window creation failed", L"Error", MB_OK | MB_ICONSTOP);
		return -1;
	}
	ShowWindow(hWnd, SW_SHOW);
	EnumChildWindows(hWnd, EnumChildProc, (LPARAM)&hfDefault);
	UpdateWindow(hWnd);

	while (GetMessageW(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessageW(&Msg);
	}
	return (INT) Msg.wParam;
}

LRESULT CALLBACK OnNotify(HWND hWnd, INT nSrcCtrl, LPNMHDR pNMHdr)
{
	if (nSrcCtrl != IDC_SYSLINK)
	{
		return 0;
	}
	else
	{
		switch (pNMHdr->code)
		{
			case NM_CLICK:
			case NM_RETURN:
			{
				PNMLINK pNMLink = (PNMLINK)pNMHdr;
				LITEM lItem = pNMLink->item;

				if (0 == lItem.iLink)
				{
					ShellExecuteW(NULL, L"open", lItem.szUrl, NULL, NULL, SW_SHOW);
				}
			}
		}
	}
	return 0;
}

VOID WINAPI OnCommand(_In_ HWND hWnd, _In_ INT nID, _In_ HWND hwSource, _In_ UINT uNotify)
{
	HWND hRandChk, hRandMS, hStartBtn, hInterval, hStopBtn;

	hInterval = FindWindowExW(hWnd, NULL, L"Edit", NULL);
	hInterval = FindWindowExW(hWnd, hInterval, L"Edit", NULL);
	hRandChk = FindWindowExW(hWnd, NULL, L"Button", NULL);
	hRandMS = FindWindowExW(hWnd, hInterval, L"Edit", NULL);
	hStartBtn = FindWindowExW(hWnd, hRandChk, L"Button", NULL);
	hStopBtn = FindWindowExW(hWnd, hStartBtn, L"Button", NULL);
	
	if (hwSource == hRandChk)
	{
		if ((GetWindowLongW(hRandMS, GWL_STYLE) & WS_DISABLED) == WS_DISABLED)
		{
			EnableWindow(hRandMS, TRUE);
		}
		else
		{
			EnableWindow(hRandMS, FALSE);
		}
	}
	else if (hwSource == hStartBtn)
	{
		WCHAR *szBaseIV;
		HANDLE hHeap = GetProcessHeap();
		INT nBLen = GetWindowTextLengthW(hInterval), secInt;
		szBaseIV = (WCHAR *) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, (nBLen + 2) * sizeof(WCHAR));
		if (NULL == szBaseIV)
		{
			MessageBoxW(NULL, L"Your system is out of memory.", L"Error", MB_OK | MB_ICONSTOP);
			ExitProcess(ERROR_OUTOFMEMORY);
		}
		GetWindowTextW(hInterval, szBaseIV, nBLen + 1);
		swscanf_s(szBaseIV, L"%d", &secInt);
		if (secInt < 0 || secInt > 15) secInt = 5;
		HeapFree(hHeap, 0, szBaseIV);
		szBaseIV = NULL;

		MessageBoxW(0, L"Program will begin in 5 seconds after pressing OK...", L"Information", MB_OK | MB_ICONASTERISK);
		Sleep(5000);

		SetTimer(hWnd, IDT_TIMER1, secInt * 1000, TimerProc);
		EnableWindow(hStartBtn, FALSE);
		EnableWindow(hStopBtn, TRUE);
	}
	else if (hwSource == hStopBtn)
	{
		EnableWindow(hStartBtn, TRUE);
		EnableWindow(hStopBtn, FALSE);
		KillTimer(hWnd, IDT_TIMER1);
	}
}

VOID WINAPI OnClose(_In_ HWND hWnd)
{
	DestroyWindow(hWnd);
}

VOID WINAPI OnDestroy(_In_ HWND hWnd)
{
	PostQuitMessage(ERROR_SUCCESS);
}

VOID WINAPI OnPaint(_In_ HWND hWnd)
{
	PAINTSTRUCT ps;

	BeginPaint(hWnd, &ps);
	EndPaint(hWnd, &ps);
}

BOOL WINAPI OnCreate(_In_ HWND hWnd, _In_ LPCREATESTRUCTW lpCreateStruct)
{
	HINSTANCE hInstance = lpCreateStruct->hInstance;
	HWND hInterval, hEditTxt, hRandMS, hRandChk, hStatic1, hStatic2, hStartBtn, hStopBtn, hDonateLnk;
	INITCOMMONCONTROLSEX iccx;
	//BOOL bRet;
	iccx.dwICC =
		ICC_STANDARD_CLASSES | ICC_LINK_CLASS;
	iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	/*bRet = */ InitCommonControlsEx(&iccx);
	hStatic1 = CreateWindowW(L"Static", L"Enter the desired text to auto-type:", WS_VISIBLE | WS_CHILD | SS_LEFTNOWORDWRAP, 10, 10, 360, 20, hWnd, NULL, hInstance, NULL);
	hEditTxt = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, L"Edit", L"e.g. \"buying rune 2 hander\" ", WS_VISIBLE | WS_CHILD | ES_LEFT, 10, 30, 360, 20, hWnd, NULL, hInstance, NULL);
	hStatic2 = CreateWindowW(L"Static", L"Enter the base delay between sending text (1-60 seconds): ", WS_VISIBLE | WS_CHILD | SS_LEFTNOWORDWRAP, 10, 50, 280, 20, hWnd, NULL, hInstance, NULL);
	hInterval = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, L"Edit", L"5", WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER, 305, 50, 65, 20, hWnd, NULL, hInstance, NULL);
	hRandChk = CreateWindowW(L"Button", L"Add additional random delay to base interval? (Max. Milliseconds): ", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | BS_LEFTTEXT | BST_UNCHECKED, 10, 70, 360, 20, hWnd, NULL, hInstance, NULL);
	hRandMS = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, L"Edit", L"50", WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | WS_DISABLED, 30, 90, 110, 20, hWnd, NULL, hInstance, NULL);
	hStartBtn = CreateWindowW(L"Button", L"Start", WS_VISIBLE | WS_CHILD | BS_LEFTTEXT, 10, 110, 180, 30, hWnd, NULL, hInstance, NULL);
	hStopBtn = CreateWindowW(L"Button", L"Stop", WS_VISIBLE | WS_CHILD | BS_LEFTTEXT | WS_DISABLED, 190, 110, 180, 30, hWnd, NULL, hInstance, NULL);
	hDonateLnk = CreateWindowW(L"SysLink", L"This program is 100% free.\r\nIf you want to donate to me via PayPal, please <A HREF=\"https://www.paypal.me/govind\">follow this link</A>.", WS_VISIBLE | WS_CHILD, 10, 140, 360, 40, hWnd, (HMENU) IDC_SYSLINK, hInstance, NULL);
	// Old link: <A HREF=\"https://www.paypal.com/cgi-bin/webscr/?cmd=_s-xclick&hosted_button_id=X8EGYQNZ7VBPJ\">follow this link</A>

	return TRUE;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
	HANDLE_MSG(hWnd, WM_CLOSE, OnClose);
	HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
	HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
	HANDLE_MSG(hWnd, WM_NOTIFY, OnNotify);
	HANDLE_MSG(hWnd, WM_PAINT, OnPaint);

	default:
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
	HFONT hfDefault = *(HFONT *) lParam;
	SendMessageW(hWnd, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(TRUE, 0));
	return TRUE;
}

ATOM RegisterWCEX(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.hIcon = 
	wcex.hIconSm = LoadIconW(NULL, IDI_APPLICATION);
	wcex.hInstance = hInstance;
	wcex.lpfnWndProc = WindowProc;
	wcex.lpszClassName = g_wszClassName;

	return RegisterClassExW(&wcex);
}

BOOL ShiftNeeded(TCHAR key)
{
	switch (key)
	{
	case L'\"':
	case L'\?':
	case L'|':
	case L':':
	case L'{':
	case L'}':
	case L'!':
	case L'@':
	case L'#':
	case L'$':
	case L'%':
	case L'^':
	case L'&':
	case L'*':
	case L'(':
	case L')':
	case L'_':
	case L'+':
	case L'~':
		return TRUE;
	default:
		return FALSE;
	}
}

VOID SendKeys(WCHAR *szTypeKeys, INT cchTypeKeys)
{
	int i;
	srand((UINT)GetTickCount64());
	for (i = 0; i < cchTypeKeys; i++)
	{
		BOOL bShft = ShiftNeeded(szTypeKeys[i]);
	
		Sleep(rand() % 100);

		if (bShft)
		{
			keybd_event(VK_SHIFT, MapVirtualKeyW(VK_SHIFT, MAPVK_VK_TO_VSC), 0, 0);
		}

		keybd_event((BYTE)VkKeyScanW(szTypeKeys[i]), MapVirtualKeyW((BYTE)VkKeyScanW(szTypeKeys[i]), MAPVK_VK_TO_VSC), 0, 0);
		keybd_event((BYTE)VkKeyScanW(szTypeKeys[i]), MapVirtualKeyW((BYTE)VkKeyScanW(szTypeKeys[i]), MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);

		if (bShft)
		{
			keybd_event(VK_SHIFT, MapVirtualKeyW(VK_SHIFT, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
		}

	}
	keybd_event(VK_RETURN, MapVirtualKeyW(VK_RETURN, MAPVK_VK_TO_VSC), 0, 0);
	keybd_event(VK_RETURN, MapVirtualKeyW(VK_RETURN, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);

}
