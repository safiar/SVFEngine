// ----------------------------------------------------------------------- //
//
// MODULE  : WindowMain.h
//
// PURPOSE : Основное окно
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _WINDOWMAIN_H
#define _WINDOWMAIN_H

#include "Window.h"
#include "WindowProc.h"
#include "InputManager.h"

using namespace SAVFGAME;

namespace SAVFGAME
{
	class CWindowMain : public CWindow	// Основное окно рендера
	{
		friend class CWindowProc;
	protected:
		bool userCloseEvent;
		bool isWindowActive;
		bool isWindowResized;
		bool isWindowMoved;
		bool isWindowMinimized;
		bool isWindowMaximized;
		bool isWindowFullscreen;
		CInputManager *		imanager;
		WPARAM				wParam;
		LPARAM				lParam;
	public:
		CWindowMain(const CWindowMain& src)				= delete;
		CWindowMain(CWindowMain&& src)					= delete;
		CWindowMain& operator=(const CWindowMain& src)	= delete;
		CWindowMain& operator=(CWindowMain&& src)		= delete;
	public:
		CWindowMain() : CWindow()
		{
			SetDefaultParams();
		}
		CWindowMain(UINT width, UINT height, UINT posx, UINT posy, DWORD dwStyle, DWORD dwExStyle, bool resize) : CWindow()
		{
			SetDefaultParams();
			wInfo.state->sys_width  = width;
			wInfo.state->sys_height = height;
			wInfo.state->position_x = posx;
			wInfo.state->position_y = posy;
			wInfo.dwStyle = dwStyle;
			wInfo.dwExStyle = dwExStyle;
			wInfo.resize = resize;
		}
		CWindowMain(bool isFullScreen) : CWindow()
		{
			SetDefaultParams();
			if (isFullScreen)
			{
				wInfo.dwStyle = GAMEFULLSCREEN;
				wInfo.dwExStyle = GAMEFULLSCREEN_EX;
				wInfo.state->position_x = 0;
				wInfo.state->position_y = 0;
				wInfo.state->sys_width  = GetSystemMetrics(SM_CXSCREEN);
				wInfo.state->sys_height = GetSystemMetrics(SM_CYSCREEN);
				wInfo.resize = false;
			}
		};
		virtual ~CWindowMain()
		{
			Close();
		};
		bool Close()
		{	
			if (wInfo.hWnd)
			{
				if (!DestroyWindow(wInfo.hWnd))
					{ _MBM(ERROR_WINDOW); return false; }

				if (!UnregisterClass(wInfo.wc.lpszClassName,wInfo.wc.hInstance))
					{ _MBM(ERROR_WINDOW); return false; }

				wInfo.hWnd = nullptr;
				wInfo.state->hwnd.Set(nullptr); // reset shared copy of window handler
			}

			DropWindowProc(); // unbind proc

			return true;
		};
		bool BindWindowProc(CWindowMain * ptr); // bind proc
		void DropWindowProc();					// unbind proc
	protected:
		bool SetWindowProc();
		bool SetDefaultParams()
		{
			if (!SetWindowProc())
				return false;
		//	wInfo.wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
			wInfo.state->sys_width  = 800;
			wInfo.state->sys_height = 600;
		//	wInfo.state.mode = WM_WINDOWED;
		//	wInfo.dwStyle = GAMEWINDOWED;
		//	wInfo.dwExStyle = GAMEWINDOWED_EX;
			wInfo.state->position_x = 300;
			wInfo.state->position_y = 200;
			wInfo.wc.lpszClassName = L"GameMainWindowClass";
			userCloseEvent = false;
			isWindowActive = true;
			isWindowResized = false;
			isWindowMinimized = false;
			isWindowMaximized = false;
			imanager = nullptr;
			return true;
		};
	public:
		//>> Создание окна
		bool Create()
		{
			if (!RegisterClassEx(&wInfo.wc))
				{ _MBM(ERROR_WINDOW); return false; }

			if (!(wInfo.hWnd = CreateWindowEx(wInfo.dwExStyle,
							wInfo.wc.lpszClassName,
							wInfo.title.c_str(),
							wInfo.dwStyle,
							wInfo.state->position_x,
							wInfo.state->position_y,
							wInfo.state->sys_width,
							wInfo.state->sys_height,
							wInfo.hWndParent,
							wInfo.hMenu,
							wInfo.wc.hInstance,
							wInfo.lpParam)))
				{ _MBM(ERROR_WINDOW); return false; }

			wInfo.state->hwnd.Set(wInfo.hWnd); // set up shared copy of window handler

			if (wInfo.dwStyle == GAMEFULLSCREEN)
			{
				ShowWindow(wInfo.hWnd, SW_SHOWMAXIMIZED);
				//ShowWindow(FindWindow(L"Shell_TrayWnd", NULL), SW_HIDE);
			}
			else ShowWindow(wInfo.hWnd, SW_SHOW);
			UpdateWindow(wInfo.hWnd);

			return true;
		};
		//>> Показ окна
		bool Proceed()
		{
			MSG msg;
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			return true;
		}
	
	public:
		//>> Устанавливает InputManager
		bool SetInputManager(CInputManager* ptr) 
		{
			if (ptr == nullptr) { _MBM(ERROR_PointerNone); return false; }

			imanager = ptr;
			UpdateInputManager();
			return true;
		}
		//>> Передатчик от WindowProc в InputManager
		void GoInputManager(const UINT& msg, const WPARAM& wParam, const LPARAM& lParam)
		{
			if (imanager == nullptr) { return; }

			imanager->Run(msg, wParam, lParam);
		}
		//>> Отправка состояния окна в InputManager
		bool UpdateInputManager()
		{
			if (imanager == nullptr) { return false; }

			tagRECT cliRc;
			GetClientRect(wInfo.hWnd, &cliRc);

			wInfo.state->client_width  = cliRc.right;
			wInfo.state->client_height = cliRc.bottom;

			wInfo.state->border_left   = (wInfo.state->sys_width - wInfo.state->client_width) / 2;
			wInfo.state->border_right  = wInfo.state->border_left;
			wInfo.state->border_bottom = wInfo.state->border_left;
			wInfo.state->border_top    = (wInfo.state->sys_height - wInfo.state->client_height - wInfo.state->border_bottom);

			wInfo.state->UpdateCenter();

			wInfo.state->Updated = true;

			//wInfo.state->border_left = GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXFIXEDFRAME);
			//wInfo.state->border_top  = GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYMENU);
			//printf("\nBORDER: X=%4i Y=%4i", wInfo.border_dx, wInfo.border_dy);

			imanager->SetRWindow(wInfo.state);

			return true;
		}
		//>> Отправка состояния окна в InputManager (вариант для WindowProc)
		bool UpdateInputManager(bool XY, const LPARAM& lParam)
		{
			tagRECT winRc;
			GetWindowRect(wInfo.hWnd, &winRc);
			if (XY)
			{
			//	wInfo.state->position_x = LOWORD(lParam);
			//	wInfo.state->position_y = HIWORD(lParam);
				wInfo.state->position_x = winRc.left;
				wInfo.state->position_y = winRc.top;
				//printf("\nPOS: %4i %4i -- %4i %4i", LOWORD(lParam), HIWORD(lParam), winRc.left, winRc.top);
				isWindowMoved = true;
			}
			else
			{
			//	wInfo.state->sys_width  = LOWORD(lParam);
			//	wInfo.state->sys_height = HIWORD(lParam);
				wInfo.state->sys_width  = winRc.right  - winRc.left;
				wInfo.state->sys_height = winRc.bottom - winRc.top;
				isWindowResized = true;
			}
			UpdateInputManager();
			return true;
		}
		//>> Отправка состояния окна в InputManager (альт. вариант)
		bool UpdateInputManager(int sys_width, int sys_height, int pos_x, int pos_y, int window_mode)
		{
			if (_NOMISS(sys_width))   wInfo.state->sys_width  = sys_width;
			if (_NOMISS(sys_height))  wInfo.state->sys_height = sys_height;
			if (_NOMISS(pos_x))       wInfo.state->position_x = pos_x;
			if (_NOMISS(pos_y))       wInfo.state->position_y = pos_y;
			if (_NOMISS(window_mode)) wInfo.state->mode = (eWindowMode)window_mode;
			isWindowResized = true;
			isWindowMoved   = true;

			UpdateInputManager();
			return true;
		}
		//>> Отправка состояния окна в InputManager (альт. вариант)
		bool UpdateInputManager(const tagRECT& winRc)
		{
			wInfo.state->position_x = winRc.left;
			wInfo.state->position_y = winRc.top;
			wInfo.state->sys_width  = winRc.right - winRc.left;
			wInfo.state->sys_height = winRc.bottom - winRc.top;
			isWindowResized = true;
			isWindowMoved   = true;

			UpdateInputManager();
			return true;
		}

	public:
		//>> Устанавливает, что окно свёрнуто
		void SetMinimized()
		{
			isWindowActive    = false;
			isWindowMinimized = true;
			isWindowMaximized = false;
			UpdateInputManager();
		}
		//>> Устанавливает, что окно развёрнуто
		void SetMaximized()
		{
			isWindowActive    = true;
			isWindowMinimized = false;
			isWindowMaximized = true;
			UpdateInputManager();
		}
		//>> Устанавливает, что окно восстановлено
		void SetRestore()
		{
			if (isWindowMinimized)				{
				isWindowActive    = true;
				isWindowMinimized = false;		} else
			if (isWindowMaximized)				{
				isWindowActive    = true;
				isWindowMaximized = false;		}
			UpdateInputManager();
		}
	
	public:
		bool IsResized()       { _RETBOOL(isWindowResized); }
		bool IsMoved()         { _RETBOOL(isWindowMoved);   }
		bool IsActive()        { return isWindowActive;     }
		bool IsCloseEvent()    { return userCloseEvent;     }
	};
}

#endif // _WINDOWMAIN_H