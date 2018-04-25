// ----------------------------------------------------------------------- //
//
// MODULE  : Window.h
//
// PURPOSE : Базовый класс окна
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _WINDOW_H
#define _WINDOW_H

#include "stdafx.h"
#include "title.h"
#include "StatusIO.h"

//	--------------------------------- <- window system rectangle
//	|                             X |
//	|   -------------------------   | <- window client rectangle
//	|   |         RENDER        |   |
//	|   |         RENDER        |   |
//	|   |         RENDER        |   |
//	|   |         RENDER        |   |
//	|   -------------------------   |
//	|                               |
//	---------------------------------

namespace SAVFGAME
{
	struct WInfo // параметры создания окна
	{
			WInfo() : title(PRODUCT_TITLE) { state = new CStatusWindow; };
			~WInfo() { _DELETE(state); };

			CStatusWindow *	state;
			bool			resize		{ true };			// is resizable ?
			DWORD			dwStyle		{ GAMEWINDOWED };
			DWORD			dwExStyle	{ GAMEWINDOWED_EX };
			wstring			title;
			HWND			hWnd		{ nullptr };
			HWND			hWndParent	{ nullptr };
			HMENU			hMenu		{ nullptr };
			LPVOID			lpParam		{ nullptr };
			WNDCLASSEX		wc;		
	public:
		WInfo(const WInfo& src)				= delete;
		WInfo(WInfo&& src)					= delete;
		WInfo& operator=(const WInfo& src)	= delete;
		WInfo& operator=(WInfo&& src)		= delete;
	};
	
	class CWindow // Базовый класс окна
	{
	protected:
		WInfo	wInfo;
	public:
		CWindow(const CWindow& src)				= delete;
		CWindow(CWindow&& src)					= delete;
		CWindow& operator=(const CWindow& src)	= delete;
		CWindow& operator=(CWindow&& src)		= delete;
	public:
		CWindow()
		{
			memset(&wInfo.wc, 0, sizeof(WNDCLASSEX));
			wInfo.wc.cbSize = sizeof(WNDCLASSEX);
			wInfo.wc.cbClsExtra = NULL;
			wInfo.wc.cbWndExtra = NULL;
			wInfo.wc.style = CS_HREDRAW | CS_VREDRAW;
			wInfo.wc.lpfnWndProc = nullptr;
			wInfo.wc.hInstance = GetModuleHandle(NULL);
			wInfo.wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wInfo.wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
			wInfo.wc.hIconSm = wInfo.wc.hIcon;
			wInfo.wc.hbrBackground = NULL;
			wInfo.wc.lpszMenuName = nullptr;
			wInfo.wc.lpszClassName = L"DefaultWindowClass";

			GetDesktopResolution();
		};
		~CWindow(){};

		HWND            GetHWND()         { return wInfo.hWnd;  }
		CStatusWindow * GetWindowStatus() { return wInfo.state; }

		void SetWindowResizable(bool isResizable)
		{
			wInfo.resize = isResizable;
		}
	protected:
		void GetDesktopResolution()
		{
			RECT desktop;
			HWND hDesktop = GetDesktopWindow();
			GetWindowRect(hDesktop, &desktop);
			wInfo.state->desktop_horizontal = desktop.right;
			wInfo.state->desktop_vertical   = desktop.bottom;
		}
	};
}

#endif // _WINDOW_H