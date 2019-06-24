// ----------------------------------------------------------------------- //
//
// MODULE  : WindowProc.cpp
//
// PURPOSE : Обработчик событий окна
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#include "WindowProc.h"

using namespace SAVFGAME;

namespace SAVFGAME
{
	CWindowMain* CWindowProc::pWindowMain = nullptr;

	//>> Устанавливает указатель на класс главного окна
	bool CWindowProc::SetWindowMain(CWindowMain* ptr)
	{ 
		if (ptr == nullptr) { _MBM(ERROR_PointerNone); return false; }
				
		CWindowProc::pWindowMain = ptr;
		return true;
	}

	//>> Возвращает указатель на класс главного окна
	CWindowMain* const CWindowProc::GetWindowMain()
	{
		return CWindowProc::pWindowMain;
	}

	//>> Удаляет указатель на класс главного окна
	void CWindowProc::DropWindowMain()
	{
		CWindowProc::pWindowMain = nullptr;
	}

	//>> Обработка событий
	LRESULT CALLBACK CWindowProc::WindowProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch(message)
		{
			case WM_CLOSE:							{			// Окно закрыто
				CWindowProc::pWindowMain->
					userCloseEvent = true;
				return 0;							} break;

			case WM_CREATE:							{			// Окно создано
				return 0;							} break;

			case WM_MOVE:							{			// Окно переместилось
				CWindowProc::pWindowMain->
					UpdateInputManager(true,lParam);
				return 0;							} break;

			case WM_ACTIVATE:						{			// Окно в фокусе?
				if (LOWORD(wParam) == WA_INACTIVE)
					CWindowProc::pWindowMain->
						isWindowActive = false;
				else
					CWindowProc::pWindowMain->
						isWindowActive = true;
				return 0;							} break;

			case WM_SIZE:							{			// Окно изменило размеры...
				if (CWindowProc::pWindowMain->
						wInfo.resize == false)
					return 0;
				CWindowProc::pWindowMain->
					UpdateInputManager(FALSE,lParam);
				if( wParam == SIZE_MINIMIZED )					// ...свернувшись
				{
					CWindowProc::pWindowMain->
						SetMinimized();
				}
				else if( wParam == SIZE_MAXIMIZED )				// ...развернувшись
				{
					CWindowProc::pWindowMain->
						SetMaximized();
				}
				else if( wParam == SIZE_RESTORED )				// ...восстановившись
				{
					CWindowProc::pWindowMain->
						SetRestore();
				}
				return 0;							} break;

			case WM_MOUSEMOVE:									// События мыши и клавиатуры
			case WM_MOUSEWHEEL:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_LBUTTONUP:
			case WM_LBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:		
			case WM_KEYDOWN:
			case WM_KEYUP:
				CWindowProc::pWindowMain->
					GoInputManager(message,wParam,lParam);
		}
		return DefWindowProc(hWnd,message,wParam,lParam);
	}
}