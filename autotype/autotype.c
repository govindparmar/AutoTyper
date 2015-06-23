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


const TCHAR g_szClassName[] = _T("AutoTyperWndClass1");

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SysLinkHandler(HWND hWnd, INT nSrcCtrl, LPNMHDR pNMHdr);
BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);

VOID CALLBACK TimerProc(HWND hwSrc, UINT Msg, UINT_PTR puTID, DWORD dwTime);
ATOM RegisterWCEX(HINSTANCE hInstance);
VOID SendKeys(TCHAR *szTypeKeys, INT cchTypeKeys);


HWND hWnd, hInterval, hEditTxt, hRandMS, hRandChk, hDropChk, hStatic1, hStatic2, hStartBtn, hStopBtn, hDonateLnk;

VOID CALLBACK TimerProc(HWND hwSrc, UINT Msg, UINT_PTR puTID, DWORD dwTime)
{
	TCHAR *szWndTxt;
	HANDLE hHeap = GetProcessHeap();
	INT nTLen = GetWindowTextLength(hEditTxt);
	szWndTxt = (TCHAR*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, (nTLen+2)*sizeof(TCHAR));
	GetWindowText(hEditTxt, szWndTxt, nTLen+1);
	//MessageBox(0, _T("In TimerProc"), _T("Info"), MB_OK);
	if (!((GetWindowLong(hRandMS, GWL_STYLE)&WS_DISABLED) == WS_DISABLED))
	{
		TCHAR *szRMS;
		INT nRLen = GetWindowTextLength(hRandMS), nSleepMS;
		srand(GetTickCount());
		szRMS = (TCHAR*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, (nRLen + 2)*sizeof(TCHAR));
		GetWindowText(hRandMS, szRMS, nRLen+1);
		_stscanf_s(szRMS, _T("%d"), &nSleepMS);
		if (nSleepMS < 0 || nSleepMS > 5000) nSleepMS = 1000;
		HeapFree(hHeap, 0, szRMS);
		Sleep(rand()%nSleepMS);
	}
	
	SendKeys(szWndTxt, nTLen);
	HeapFree(hHeap, 0, szWndTxt);
	CloseHandle(hHeap);
}

__forceinline void CreateWindows(HINSTANCE hInstance)
{
	INITCOMMONCONTROLSEX iccx;
	BOOL bRet;
	iccx.dwICC =
	 ICC_STANDARD_CLASSES | ICC_LINK_CLASS;
	iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	bRet = InitCommonControlsEx(&iccx);
	hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, g_szClassName, _T("AutoTyper"), WS_VISIBLE | WS_SYSMENU, 100, 100, 400, 235, NULL, NULL, hInstance, NULL);
	hStatic1 = CreateWindow(_T("Static"), _T("Enter the desired text to auto-type:"), WS_VISIBLE | WS_CHILD | SS_LEFTNOWORDWRAP, 10, 10, 360, 20, hWnd, NULL, hInstance, NULL);
	hEditTxt = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, _T("Edit"), _T("e.g. \"buying rune 2 hander\" "), WS_VISIBLE | WS_CHILD | ES_LEFT, 10, 30, 360, 20, hWnd, NULL, hInstance, NULL);
	hStatic2 = CreateWindow(_T("Static"), _T("Enter the base delay between sending text (1-60 seconds): "), WS_VISIBLE | WS_CHILD | SS_LEFTNOWORDWRAP, 10, 50, 280, 20, hWnd, NULL, hInstance, NULL);
	hInterval = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, _T("Edit"), _T("5"), WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER, 305, 50, 65, 20, hWnd, NULL, hInstance, NULL);
	hRandChk = CreateWindow(_T("Button"), _T("Add additional random delay to base interval? (Max. Milliseconds): "), WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | BS_LEFTTEXT | BST_UNCHECKED, 10, 70, 360, 20, hWnd, NULL, hInstance, NULL);
	hRandMS = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, _T("Edit"), _T("50"), WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER  | WS_DISABLED, 30, 90, 110, 20, hWnd, NULL, hInstance, NULL);
	hStartBtn = CreateWindow(_T("Button"), _T("Start"), WS_VISIBLE | WS_CHILD | BS_LEFTTEXT, 10, 110, 180, 30, hWnd, NULL, hInstance, NULL);
	hStopBtn = CreateWindow(_T("Button"), _T("Stop"), WS_VISIBLE | WS_CHILD | BS_LEFTTEXT | WS_DISABLED, 190, 110, 180, 30, hWnd, NULL, hInstance, NULL);
	hDonateLnk = CreateWindowEx(0, _T("SysLink"), _T("This program is 100% free.\r\nIf you want to donate to me via PayPal, please <A HREF=\"https://www.paypal.com/cgi-bin/webscr/?cmd=_s-xclick&hosted_button_id=X8EGYQNZ7VBPJ\">follow this link</A>."), WS_VISIBLE | WS_CHILD, 10, 140, 360, 40, hWnd, (HMENU)IDC_SYSLINK, hInstance, NULL); 
}



int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
{
	ATOM aRes = RegisterWCEX(hInstance);
	MSG Msg;
	if (aRes <= 0)
	{
		MessageBox(0, _T("Window registration failed"), _T("Error"), MB_OK | MB_ICONSTOP);
		return -1;
	}
	
	CreateWindows(hInstance);
	
	if (hWnd <= (HWND)0)
	{
		MessageBox(0, _T("Window creation failed"), _T("Error"), MB_OK | MB_ICONSTOP);
		return -1;
	}
	ShowWindow(hWnd, SW_SHOW);
	EnumChildWindows(hWnd, EnumChildProc, 0L);
	UpdateWindow(hWnd);

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}

LRESULT CALLBACK SysLinkHandler(HWND hWnd, INT nSrcCtrl, LPNMHDR pNMHdr)
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
				if (lItem.iLink == 0)
				{
					ShellExecute(NULL, _T("open"), lItem.szUrl, NULL, NULL, SW_SHOW);
				}
			}
		}
	}
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	HANDLE_MSG(hWnd, WM_NOTIFY, SysLinkHandler);
	case WM_COMMAND:
	{
		HWND hwSrc = (HWND)lParam;
		if (hwSrc == hRandChk)
		{
			if ((GetWindowLong(hRandMS, GWL_STYLE)&WS_DISABLED) == WS_DISABLED)
			{
				EnableWindow(hRandMS, TRUE);
			}
			else
			{
				EnableWindow(hRandMS, FALSE);
			}
		}
		else if (hwSrc == hStartBtn)
		{
			TCHAR *szBaseIV;
			HANDLE hHeap = GetProcessHeap();
			INT nBLen = GetWindowTextLength(hInterval), secInt;
			szBaseIV = (TCHAR*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, (nBLen + 2)*sizeof(TCHAR));
			GetWindowText(hInterval, szBaseIV, nBLen+1);
			_stscanf_s(szBaseIV, _T("%d"), &secInt);
			HeapFree(hHeap, 0, szBaseIV);
			CloseHandle(hHeap);
			MessageBox(0, _T("Program will begin in 5 seconds after pressing OK..."), _T("Information"), MB_OK | MB_ICONASTERISK);
			Sleep(5000);
			
			SetTimer(hWnd, IDT_TIMER1, secInt*1000, TimerProc);
			EnableWindow(hStartBtn, FALSE);
			EnableWindow(hStopBtn, TRUE);
		}
		else if (hwSrc == hStopBtn)
		{
			EnableWindow(hStartBtn, TRUE);
			EnableWindow(hStopBtn, FALSE);
			KillTimer(hWnd, IDT_TIMER1);
		}
	}
	break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
	HFONT hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hfDefault, 0L);
	return TRUE;
}

ATOM RegisterWCEX(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hInstance = hInstance;
	wcex.lpfnWndProc = WindowProc;
	wcex.lpszClassName = g_szClassName;

	return RegisterClassEx(&wcex);
}

BOOL ShiftNeeded(TCHAR key)
{
	BOOL retVal = FALSE;
	switch (key)
	{
	case _T('\"'):
	case _T('\?'):
	case _T('|'):
	case _T(':'):
	case _T('{'):
	case _T('}'):
	case _T('!'):
	case _T('@'):
	case _T('#'):
	case _T('$'):
	case _T('%'):
	case _T('^'):
	case _T('&'):
	case _T('*'):
	case _T('('):
	case _T(')'):
	case _T('_'):
	case _T('+'):
	case _T('~'):
		retVal = TRUE;
		break;
	}

	return retVal;
}

VOID SendKeys(TCHAR *szTypeKeys, INT cchTypeKeys)
{
	int i;
	srand(GetTickCount());
	for (i = 0; i < cchTypeKeys; i++)
	{
		BOOL bShft = ShiftNeeded(szTypeKeys[i]);
	
		Sleep(rand() % 100);

		if (bShft)
		{
			keybd_event(VK_SHIFT, MapVirtualKey(VK_SHIFT, MAPVK_VK_TO_VSC), 0, 0);
		}

		keybd_event((BYTE)VkKeyScan(szTypeKeys[i]), MapVirtualKey((BYTE)VkKeyScan(szTypeKeys[i]), MAPVK_VK_TO_VSC), 0, 0);
		keybd_event((BYTE)VkKeyScan(szTypeKeys[i]), MapVirtualKey((BYTE)VkKeyScan(szTypeKeys[i]), MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);

		if (bShft)
		{
			keybd_event(VK_SHIFT, MapVirtualKey(VK_SHIFT, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
		}

	}
	keybd_event(VK_RETURN, MapVirtualKey(VK_RETURN, MAPVK_VK_TO_VSC), 0, 0);
	keybd_event(VK_RETURN, MapVirtualKey(VK_RETURN, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);

	/*HANDLE hHeap = GetProcessHeap();
	INPUT *inputs = (INPUT*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 2*((cchTypeKeys)*sizeof(INPUT)));
	INPUT rtrn[2] = { 0 };
	INT i, count = 0;
	for (i = 0; i < 2*cchTypeKeys; i+=2)
	{
		inputs[i].type = INPUT_KEYBOARD;
		inputs[i].ki.wVk = VkKeyScan(szTypeKeys[count]);
		inputs[i].ki.wScan = MapVirtualKey(szTypeKeys[count], MAPVK_VK_TO_VSC);
		inputs[i].ki.dwFlags = 0;
		inputs[i].ki.dwExtraInfo = 0;

		inputs[i+1].type = INPUT_KEYBOARD;
		inputs[i+1].ki.wVk = VkKeyScan(szTypeKeys[count]);
		inputs[i+1].ki.wScan = MapVirtualKey(szTypeKeys[count], MAPVK_VK_TO_VSC);
		inputs[i+1].ki.dwFlags = KEYEVENTF_KEYUP;
		inputs[i+1].ki.dwExtraInfo = 0;
		count++;
	}
	 
	rtrn[0].type = INPUT_KEYBOARD;
	rtrn[0].ki.wVk = VK_RETURN;
	rtrn[0].ki.wScan = MapVirtualKey(VK_RETURN, MAPVK_VK_TO_VSC);
	rtrn[0].ki.dwFlags = 0;
	rtrn[0].ki.dwExtraInfo = 0;

	rtrn[1].type = INPUT_KEYBOARD;
	rtrn[1].ki.wVk = VK_RETURN;
	rtrn[1].ki.wScan = MapVirtualKey(VK_RETURN, MAPVK_VK_TO_VSC);
	rtrn[1].ki.dwFlags = KEYEVENTF_KEYUP;
	rtrn[1].ki.dwExtraInfo = 0;

	SendInput(cchTypeKeys*2, inputs, sizeof(INPUT));
	SendInput(2, rtrn, sizeof(INPUT));*/
}
