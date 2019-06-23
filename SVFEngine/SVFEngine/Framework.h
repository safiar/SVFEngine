// ----------------------------------------------------------------------- //
//
// MODULE  : Framework.h
//
// PURPOSE : Определяет среду разработки
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _FRAMEWORK_H
#define _FRAMEWORK_H

#include "WindowMain.h"
#include "WindowProc.h"
#include "InputManager.h"
#include "Render.h"

using namespace SAVFGAME;

namespace SAVFGAME
{
	class CFramework
	{
	protected:
		HWND                hwnd          { nullptr };
		CStatusWindow *		state         { nullptr }; // get exist
		CWindowMain *		window        { nullptr };
		CInputManager *		imanager      { nullptr };
		CRender *			render        { nullptr }; // get exist
		bool				show_cursor   { true  };
		bool				isInit        { false };
	public:
		CFramework(const CFramework& src)				= delete;
		CFramework(CFramework&& src)					= delete;
		CFramework& operator=(const CFramework& src)	= delete;
		CFramework& operator=(CFramework&& src)			= delete;
	public:
		CFramework()  { };
		~CFramework() { Close(); };
		void Close()
		{
			_DELETE(imanager);
			_DELETE(window);
			render = nullptr;

			isInit = false;
		}
		bool Init(CRender * pRender)
		{
			if (isInit) return true;

			if (pRender == nullptr) { _MBM(ERROR_PointerNone); return false; }

			window   = new CWindowMain;
			imanager = new CInputManager;
			render   = pRender;

			if (!window || !imanager)
			{
				Close(); //_MBM(ERROR_MEMORY);
				return false;
			}

			if (!window->Create())                  { Close(); return false; }
			if (!window->SetInputManager(imanager)) { Close(); return false; }

			hwnd  = window->GetHWND();
			state = window->GetWindowStatus();

			window->UpdateInputManager();

			if (!render->Init(hwnd, state))
				{ Close(); return false; }

			return isInit = true;
		}
		bool Run()
		{
			if (!isInit) { _MBM(ERROR_InitNone); return false; }

			while (WorkOnFrame()) { };

			return true;
		}
		
	protected:
		//>> 
		bool WorkOnFrame()
		{
			bool reset_render = render->to_framework.ResetRenderCheck();

			window->Proceed(); // Обработка событий окна	
		//	if (!window->IsActive()) return true;		// Свёрнутое окно не обрабатываем
			if (window->IsCloseEvent()) return false;	// Окно закрыли, выход из игры

			if (render->to_framework.WindowResetCheck()) // Рендер запрашивает обновить окно
			{	
			//	tagRECT winRc;
			//	GetWindowRect(handle, &winRc);
			//	cWindowMain->UpdateInputManager(winRc);

				int	 width  = state->sys_width;
				int	 height = state->sys_height;
				int	 pos_x  = state->position_x;
				int	 pos_y  = state->position_y;
				auto mode   = state->mode;

				if (mode == WM_FULLWINDOW || mode == WM_FULLSCREEN)
				{
					UINT params = SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW;
					width  = GetSystemMetricsOS(SM_CXSCREEN);
					height = GetSystemMetricsOS(SM_CYSCREEN);
					window->SetWindowResizable(false);
					SetWindowLongPtr(hwnd, GWL_STYLE,   GAMEFULLSCREEN);
					SetWindowLongPtr(hwnd, GWL_EXSTYLE, GAMEFULLSCREEN_EX);

					SetWindowPosOS(hwnd, HWND_TOP, 0, 0, width, height, params);
					window->UpdateInputManager(width, height, 0, 0, mode);
				}
				else // WINDOWED
				{
					UINT params = SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW; // | SWP_NOMOVE; // no move pos_x / pos_y
					if (mode == WM_NOBORDERS)										{
						window->SetWindowResizable(false);
						SetWindowLongPtr(hwnd, GWL_STYLE, GAMEFULLSCREEN);
						SetWindowLongPtr(hwnd, GWL_EXSTYLE, GAMEFULLSCREEN_EX);		}
					else															{
						window->SetWindowResizable(true);
						SetWindowLongPtr(hwnd, GWL_STYLE, GAMEWINDOWED);
						SetWindowLongPtr(hwnd, GWL_EXSTYLE, GAMEWINDOWED_EX);		}

					SetWindowPosOS(hwnd, HWND_TOP, pos_x, pos_y, width, height, params);
					window->UpdateInputManager(width, height, MISSING, MISSING, mode);
				}
				//printf("\nWindow resized by Render Call");
			}
		
			if (render->to_framework.WindowMinimizeCheck()) // запрос свернуть окно
			{
			//	SW_HIDE - Скрывает окно и активизирует другое окно.
			//	SW_MAXIMIZE - Развертывает определяемое окно.
			//	SW_MINIMIZE - Свертывает определяемое окно и активизирует
			//	              следующее окно верхнего уровня в Z - последовательности.
			//	SW_RESTORE - Активизирует и отображает окно.Если окно свернуто или развернуто,
			//	             Windows восстанавливает в его первоначальных размерах и позиции.
			//	             Прикладная программа должна установить этот флажок при восстановлении свернутого окна.
			//	SW_SHOW - Активизирует окно и отображает его текущие размеры и позицию.
			//	SW_SHOWDEFAULT - Устанавливает состояние показа, основанное на флажке SW_,
			//	                 определенном в структуре STARTUPINFO, переданной в функцию
			//					 CreateProcess программой, которая запустила прикладную программу.
			//	SW_SHOWMAXIMIZED - Активизирует окно и отображает его как развернутое окно.
			//	SW_SHOWMINIMIZED - Активизирует окно и отображает его как свернутое окно.
			//	SW_SHOWMINNOACTIVE - Отображает окно как свернутое окно.Активное окно остается активным.
			//	SW_SHOWNA - Отображает окно в его текущем состоянии.Активное окно остается активным.
			//	SW_SHOWNOACTIVATE - Отображает окно в его самом современном размере и позиции.Активное окно остается активным.
			//	SW_SHOWNORMAL - Активизирует и отображает окно.Если окно свернуто или развернуто,
			//	                Windows восстанавливает его в первоначальном размере и позиции.
			//					Прикладная программа должна установить этот флажок при отображении окна впервые.

				if (ShowWindowOS(hwnd, SW_MINIMIZE))
					window->SetMinimized();
			}

			if (window->IsMoved()) render->Update();

			if (window->IsResized())
			{
				reset_render = true;

				printf("\nW %i H %i", state->sys_width, state->sys_height);
				switch (state->mode)
				{
				case WM_WINDOWED:	printf(" window mode");                 break;
				case WM_FULLWINDOW: printf(" full window with no borders"); break;
				case WM_FULLSCREEN: printf(" fullscreen mode");             break;
				case WM_NOBORDERS:  printf(" window with no borders");      break;
				}
				printf("\nX %i Y %i (render client size)", state->client_width, state->client_height);
			}

			if (reset_render)
				render->Reset(hwnd, state);

			// Проверка центирования курсора //

			if (render->to_framework.centered_cursor)
				imanager->SetCenterCursor();

			// Проверка сдвига курсора в X, Y //

			if (render->to_framework.CursorMoveCheck())
				imanager->SetCursorAt(render->to_framework.x, render->to_framework.y);

			// Проверка показа курсора //

			if ( show_cursor && !render->to_framework.show_cursor)	{ 
				show_cursor = false;
				imanager->SetShowCursor(false);						}
			else
			if (!show_cursor &&  render->to_framework.show_cursor)	{
				show_cursor = true;
				imanager->SetShowCursor(true);						}

			if (!render->Run())
				return false; // выход из игры
		
			return true; // продолжение игры
		}
		
	public:
		//>> Добавление ответных методов на события окна
		bool AddAnswer(CInputAnswers * pAnswers)
		{
			if (imanager)
				imanager->AddAnswer(pAnswers);
			return true;
		}
	};
}

#endif // _FRAMEWORK_H