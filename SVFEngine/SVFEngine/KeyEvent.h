// ----------------------------------------------------------------------- //
//
// MODULE  : KeyEvent.h
//
// PURPOSE : Система привязки действий к управлению
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _KEYEVENT_H
#define _KEYEVENT_H

#include "stdafx.h"
#include "InputCodes.h"

#define  MISS_WCHAR  MISSING

namespace SAVFGAME
{
	enum eKeyEvent // Действия для привязки
	{
		EKE_MOVE_FORWARD,	// default : W
		EKE_MOVE_BACK,		// default : S
		EKE_MOVE_LEFT,		// default : A
		EKE_MOVE_RIGHT,		// default : D
		EKE_MOVE_UP,		// default : SPACE
		EKE_MOVE_DOWN,		// default : CTRL
		EKE_ROLL_LEFT,		// default : Q
		EKE_ROLL_RIGHT,		// default : E

		EKE_CAMERA_NO_CONTROL,	// default : 0
		EKE_CAMERA_1ST_PERSON,	// default : 1
		EKE_CAMERA_3RD_PERSON,	// default : 3
		EKE_CAMERA_FLY_MODE,	// default : F
		EKE_CAMERA_XZ_FIX,		// default : G

		EKE_CAMERA_ADD_RADIUS,	// default : mouse wheel DOWN
		EKE_CAMERA_SUB_RADIUS,	// default : mouse wheel UP

		EKE_INTERACT_ROTATE_MODEL,	// default : mouse LB
		EKE_INTERACT_MOVE_MODEL,	// default : mouse RB

		EKE_INCREASE_PARALLAX_SCALE,	// default : Z = 0x5a   wchar
		EKE_INCREASE_PARALLAX_SCALE_RU,	// default : Я = 0x42f  wchar
		EKE_DECREASE_PARALLAX_SCALE,	// default : z = 0x7a   wchar
		EKE_DECREASE_PARALLAX_SCALE_RU, // default : я = 0x44f  wchar

		EKE_ESCAPE_MENU, // default : ESC

		EKE_CHAT_MENU,				// default : TAB
		EKE_CHAT_MENU_DELETE_CHAR,	// default : BACKSPACE

		EKE_HELP_SHOW_STATUS, // default : H

		EKE_CLIPBOARD_CTRL,	// default : [ctrl + c/v] copy-paste /clipboard/
		EKE_CLIPBOARD_V,	// default : [ctrl + c/v] copy-paste /clipboard/
		EKE_CLIPBOARD_C,	// default : [ctrl + c/v] copy-paste /clipboard/

		EKE_SYSTEM_CTRL,    // ctrl + alt + del combo
		EKE_SYSTEM_ALT,     // ctrl + alt + del combo
		EKE_SYSTEM_DELETE,  // ctrl + alt + del combo

		EKE_ENUM_MAX
	};

	class CKeyEvent
	{
		struct EventBound
		{
			eKeyBoard		key   { K_ENUM_MAX }; // default K_ENUM_MAX
			wchar_t			wchar { MISS_WCHAR }; // default MISS_WCHAR
		};
	public:
		vector<EventBound>	ev;	// usage : keyboard.key[ ev[eKeyEvent].key ].<status_property>
	private:
		CKeyEvent(const CKeyEvent& src);
		CKeyEvent(CKeyEvent&& src);
		CKeyEvent& operator=(CKeyEvent&& src);
		CKeyEvent& operator=(const CKeyEvent& src);
	public:
		CKeyEvent() : ev(EKE_ENUM_MAX) { Reset(); };
		~CKeyEvent(){};
		void Close()
		{
			Reset();
		}
		void Reset()
		{
			ev[EKE_MOVE_FORWARD].key = eKeyBoard::K_W;
			ev[EKE_MOVE_BACK].key    = eKeyBoard::K_S;
			ev[EKE_MOVE_LEFT].key    = eKeyBoard::K_A;
			ev[EKE_MOVE_RIGHT].key   = eKeyBoard::K_D;
			ev[EKE_MOVE_UP].key      = eKeyBoard::K_SPACE;
			ev[EKE_MOVE_DOWN].key    = eKeyBoard::K_CTRL;
			ev[EKE_ROLL_LEFT].key    = eKeyBoard::K_Q;
			ev[EKE_ROLL_RIGHT].key   = eKeyBoard::K_E;

			ev[EKE_CAMERA_NO_CONTROL].key = eKeyBoard::K_0;
			ev[EKE_CAMERA_1ST_PERSON].key = eKeyBoard::K_1;
			ev[EKE_CAMERA_3RD_PERSON].key = eKeyBoard::K_3;
			ev[EKE_CAMERA_FLY_MODE].key   = eKeyBoard::K_F;
			ev[EKE_CAMERA_XZ_FIX].key     = eKeyBoard::K_G;

			ev[EKE_CAMERA_ADD_RADIUS].key = eKeyBoard::K_WHEEL_DOWN;
			ev[EKE_CAMERA_SUB_RADIUS].key = eKeyBoard::K_WHEEL_UP;

			ev[EKE_INTERACT_ROTATE_MODEL].key = eKeyBoard::K_LBUTTON;
			ev[EKE_INTERACT_MOVE_MODEL].key   = eKeyBoard::K_RBUTTON;

			ev[EKE_INCREASE_PARALLAX_SCALE].key   = eKeyBoard::K_Z;
			ev[EKE_INCREASE_PARALLAX_SCALE].wchar = 0x5a;				// Z
			ev[EKE_DECREASE_PARALLAX_SCALE].key   = eKeyBoard::K_Z;
			ev[EKE_DECREASE_PARALLAX_SCALE].wchar = 0x7a;				// z

			ev[EKE_INCREASE_PARALLAX_SCALE_RU].key   = eKeyBoard::K_Z;
			ev[EKE_INCREASE_PARALLAX_SCALE_RU].wchar = 0x42f;			// Я
			ev[EKE_DECREASE_PARALLAX_SCALE_RU].key   = eKeyBoard::K_Z;
			ev[EKE_DECREASE_PARALLAX_SCALE_RU].wchar = 0x44f;			// я

			ev[EKE_ESCAPE_MENU].key = eKeyBoard::K_ESCAPE;

			ev[EKE_CHAT_MENU].key             = eKeyBoard::K_TAB;
			ev[EKE_CHAT_MENU_DELETE_CHAR].key = eKeyBoard::K_BACKSPACE;

			ev[EKE_HELP_SHOW_STATUS].key = eKeyBoard::K_H;

			ev[EKE_CLIPBOARD_CTRL].key = eKeyBoard::K_CTRL;
			ev[EKE_CLIPBOARD_V].key    = eKeyBoard::K_V;
			ev[EKE_CLIPBOARD_C].key    = eKeyBoard::K_C;
		}
	};
}

#endif // _KEYEVENT_H