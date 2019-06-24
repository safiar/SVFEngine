// ----------------------------------------------------------------------- //
//
// MODULE  : InputManager.h
//
// PURPOSE : Управление обработкой событий окна
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _INPUTMANAGER_H
#define _INPUTMANAGER_H

#include "InputAnswers.h"

using namespace SAVFGAME;

namespace SAVFGAME
{
	class CInputAnswers;

	class CInputManager
	{
	protected:
		list<CInputAnswers*> answers;
		CStatusWindow * rwindow;
		int position_to_centred_cursor_x;
		int position_to_centred_cursor_y;
		int mouse_x;
		int mouse_y;
		char mouse_wheel;
	public:
		CInputManager(const CInputManager& src)				= delete;
		CInputManager(CInputManager&& src)					= delete;
		CInputManager& operator=(const CInputManager& src)	= delete;
		CInputManager& operator=(CInputManager&& src)		= delete;
	public:
		CInputManager() : rwindow(nullptr) { Close(); }
		~CInputManager() { Close(); }
		void Close()
		{
			rwindow = nullptr;
			Clear();
		}
		void Clear()
		{
			mouse_x = NULL;
			mouse_y = NULL;
			mouse_wheel = NULL;
			//memset(&rwindow,0,sizeof(WINRECT));
			if (!answers.empty()) answers.clear();
		}
	public:
		//>> Регистрация методов ответа на событие
		void AddAnswer(const CInputAnswers* pAMethod)
		{
			if (pAMethod == nullptr) { _MBM(ERROR_PointerNone); return; }

			answers.push_back(const_cast<CInputAnswers*>(pAMethod));
		}
		//>> Регистрация параметров окна
		void SetRWindow(const CStatusWindow * rect)
		{
			if (rect == nullptr) { _MBM(ERROR_PointerNone); return; }

			rwindow = const_cast<CStatusWindow*>(rect);
		//	position_to_centred_cursor_x = (rect->position_x + rect->border_left) + (rect->client_width  / 2);
		//	position_to_centred_cursor_y = (rect->position_y + rect->border_top)  + (rect->client_height / 2);
			position_to_centred_cursor_x = rwindow->position_x + rwindow->center_x;
			position_to_centred_cursor_y = rwindow->position_y + rwindow->center_y;
		}
		//>> Установка курсора по центру окна [client window rectangle center]
		void SetCenterCursor()
		{
			SetCursorPos ( position_to_centred_cursor_x,
						   position_to_centred_cursor_y );
		}
		//>> Установка курсора по координатам [client window rectangle X, Y]
		void SetCursorAt(const int x, const int y)
		{
			SetCursorPos ( rwindow->position_x + rwindow->border_left + x,
						   rwindow->position_y + rwindow->border_top  + y );
		}
		//>> Установка видимости курсора
		void SetShowCursor(bool show)
		{
			ShowCursor(show);
		}
	public:
		//>> Запуск в случае события [WindowProc -> Window -> Manager::Run]
		void Run(const UINT msg, const WPARAM wParam, const LPARAM lParam)
		{
			if (answers.empty()) return; // Нет методов ответной реакции

			eKeyBoard key;
			BYTE state[256];
			wchar_t wchar[ONE];

			EventMouseMove();

			switch(msg)
			{
			case WM_KEYDOWN:
				key = static_cast<eKeyBoard>(wParam);	// преобразуем в наш enum
				GetKeyboardState(state);
				ToUnicode((UINT)wParam, HIWORD(lParam) & 0xFF, state, wchar, ONE, NULL);  // переводит код-сигнал wParam в Unicode-символ wchar
				EventKeyboard(FALSE,key,wchar[0]);
				break;
			case WM_KEYUP:
				key = static_cast<eKeyBoard>(wParam);
				GetKeyboardState(state);
				ToUnicode((UINT)wParam, HIWORD(lParam) & 0xFF, state, wchar, ONE, NULL);
				EventKeyboard(TRUE,key,wchar[0]);
				break;
			case WM_LBUTTONDOWN:
				EventMouseClick(FALSE, M_LEFT);
				break;
			case WM_LBUTTONUP:
				EventMouseClick(TRUE, M_LEFT);
				break;
			case WM_RBUTTONDOWN:
				EventMouseClick(FALSE, M_RIGHT);
				break;
			case WM_RBUTTONUP:
				EventMouseClick(TRUE, M_RIGHT);
				break;
			case WM_MBUTTONDOWN:
				EventMouseClick(FALSE, M_MIDDLE);
				break;
			case WM_MBUTTONUP:
				EventMouseClick(TRUE, M_MIDDLE);
				break;
			case WM_MOUSEWHEEL:
				EventMouseWheel( (short)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA );
				break;
			}
		}
	protected:
		//>> Запуск ответа на события клавиатуры
		void EventKeyboard(const bool isKeyUp, const eKeyBoard& key, const wchar_t wchar)
		{
			for(auto answer : answers)
			{
				if (answer == nullptr) { continue; } else

				if (!isKeyUp)														{
					if (answer->KeyPressed(KeyboardButton(wchar,key)) == true)
						return;														} else {
					if (answer->KeyReleased(KeyboardButton(wchar,key)) == true)
						return;														}
			}
		}
		//>> Запуск ответа на событие клика мышью
		void EventMouseClick(const bool isKeyUp, const eMouseButton& key)
		{
			for (auto answer : answers)
			{
				if (answer == nullptr) { continue; } else

				if (!isKeyUp)																		{
					if (answer->MousePressed(MouseButton(key,mouse_x,mouse_y,rwindow)) == true)
						return;																		} else {
					if (answer->MouseReleased(MouseButton(key,mouse_x,mouse_y,rwindow)) == true)
						return;																		}
			}
		}
		//>> Запуск ответа на событие вращения колёсика мыши
		void EventMouseWheel(const int wheel)
		{
			//if (mouse_wheel == wheel) return; else 
			mouse_wheel = (char)wheel;

			for (auto answer : answers)
			{
				if (answer == nullptr) { continue; } else
			
				if (answer->MouseWheeled(MouseWheel(mouse_wheel,mouse_x,mouse_y,rwindow))==true)
					return;
			}
		}
		//>> Запуск ответа на событие движения мыши
		void EventMouseMove()
		{
			POINT pos;
			GetCursorPos(&pos);

			pos.x -= rwindow->position_x; // Локальные [x,y] в окне
			pos.y -= rwindow->position_y; // 

			if (mouse_x==pos.x && mouse_y==pos.y) return;

			mouse_x = pos.x;
			mouse_y = pos.y;

			for (auto answer : answers)
			{
				if (answer == nullptr) { continue; } else
			
				if (answer->MouseMoved(MousePos(mouse_x,mouse_y,rwindow))==true)
					/*break;*/ return;
			} //SetCenterCursor();
		}
	};
}

#endif // _INPUTMANAGER_H