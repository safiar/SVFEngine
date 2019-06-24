// ----------------------------------------------------------------------- //
//
// MODULE  : StatusIO.h
//
// PURPOSE : Содержит состояния устройств ввода-ввывода
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _STATUSIO_H
#define _STATUSIO_H

#include "stdafx.h"
#include "InputCodes.h"

namespace SAVFGAME
{
	enum eWindowMode
	{
		WM_WINDOWED   = 0,
		WM_FULLWINDOW = 1,
		WM_FULLSCREEN = 2,
		WM_NOBORDERS  = 3,

	//	WM_MINIMIZED  = 4,
	};

	enum eMouseResets
	{
		MR_POSITION = 1 << 0,	// ResetPosition()
		MR_WHEEL    = 1 << 1,	// ResetWheel()
		MR_BUTTONS  = 1 << 2	// ResetButtons()
	};

	struct CStatusMouse
	{
		int32 sys_x    { 0 }; // cursor position x in system rectangle (0 at sys rect)
		int32 sys_y    { 0 }; // cursor position y in system rectangle (0 at sys rect)
		int32 client_x { 0 }; // cursor position x in client rectangle (0 at client rect)
		int32 client_y { 0 }; // cursor position y in client rectangle (0 at client rect)
		char  wheel    { 0 }; // -1 (down) / 0 / +1 (up)
		bool rb        { 0 }; // right  button
		bool mb        { 0 }; // middle button
		bool lb        { 0 }; // left   button
		bool show      { 1 }; // state of showing
		bool center    { 0 }; // state of centering

		void Reset()
		{
			ResetPosition();
			ResetWheel();
			ResetButtons();

			show     = true;
			center   = false;
		}
		void Reset(char eMouseResets)
		{
			if (eMouseResets & MR_POSITION) ResetPosition();
			if (eMouseResets & MR_WHEEL)    ResetWheel();
			if (eMouseResets & MR_BUTTONS)  ResetButtons();

			show = true;
			center = false;
		}
		void Reset(char eMouseResets, bool show_state, bool center_state)
		{
			if (eMouseResets & MR_POSITION) ResetPosition();
			if (eMouseResets & MR_WHEEL)    ResetWheel();
			if (eMouseResets & MR_BUTTONS)  ResetButtons();

			show   = show_state;
			center = center_state;
		}
		void ResetPosition()
		{
			sys_x = 0;
			sys_y = 0;
			client_x = 0;
			client_y = 0;
		}
		void ResetWheel()
		{
			wheel = 0;
		}
		void ResetButtons()
		{
			rb = false;
			mb = false;
			lb = false;
		}
	};

	class CWindowMain;
	struct CStatusWindow
	{
		THOLDER <HWND, CWindowMain>	hwnd; // shared copy of window handler ; original manipulated located at struct WInfo

		int  sys_width          { 800 };
		int  sys_height         { 600 };
		int  client_width       { 800 };
		int  client_height      { 600 };
		int  border_left        { 0 };
		int  border_right       { 0 };
		int  border_top         { 0 };
		int  border_bottom      { 0 };
		int  center_x           { 0 };	// (local) center of client rectangle :: from pos 0 in sys rectangle
		int  center_y           { 0 };	// (local) center of client rectangle :: from pos 0 in sys rectangle
		int  position_x         { 300 };
		int  position_y         { 200 };
		int  desktop_horizontal { 800 };
		int	 desktop_vertical   { 600 };
		eWindowMode	mode        { WM_WINDOWED };	
		bool Updated            { 0 };  // окно меняется редко, отметим факт изменения отдельно
		//>> Сброс в состояние по умолчанию		
		void Reset()
		{
			sys_width     = 800;
			sys_height    = 600;
			client_width  = 800;
			client_height = 600;
			border_left   = 0;
			border_right  = 0;
			border_top    = 0;
			border_bottom = 0;
			center_x      = 0;
			center_y      = 0;
			position_x    = 300;
			position_y    = 200;
			mode          = WM_WINDOWED;
		}
		void Print()
		{
			printf("\n- sys_width      %i"
				   "\n- sys_height     %i"
				   "\n- client_width   %i"
				   "\n- client_height  %i"
				   "\n- border_left    %i"
				   "\n- border_right   %i"
				   "\n- border_top     %i"
				   "\n- border_bottom  %i"
				   "\n- center_x       %i"
				   "\n- center_y       %i"
				   "\n- position_x     %i"
				   "\n- position_y     %i"		   
				   "\n- mode           %i"
				   "\n- desktop_horizontal %i"
				   "\n- desktop_vertical   %i",
					   sys_width,
					   sys_height,
					   client_width,
					   client_height,
					   border_left,
					   border_right,
					   border_top,
					   border_bottom,
					   center_x,
					   center_y,
					   position_x,
					   position_y,
					   mode,
					   desktop_horizontal,
					   desktop_vertical
			);
		}
		void UpdateCenter()
		{
			center_x = border_left + client_width / 2;
			center_y = border_top + client_height / 2;
		}
	};

	struct CStatusKey
	{
		wchar_t	wchar    { 0 };	// associated symbol status
		bool	pressed  { 0 }; // push status
		bool	timeout  { 1 }; // prevent spamming
		bool	stuck    { 0 }; // check stucking in pushed condition
		//>> Сброс в состояние по умолчанию
		void Reset()
		{
			wchar    = 0;
			pressed  = 0;
			timeout  = 1;
			stuck    = 0;
		}
		//>> pressed=YES && timeout=NO -> return TRUE
		bool is_active()
		{
			return (pressed && !timeout);
		}
		//>> pressed=YES && timeout=NO && stuck=NO -> return TRUE
		bool is_active_s()
		{
			return (pressed && !timeout && !stuck);
		}
	};

	struct CStatusKeyboard
	{
		CStatusKey	key[eKeyBoard::K_ENUM_MAX];
		//>> Сброс всех клавиш в <timeout = true>
		void ResetTimeout()
		{
			for (auto & k : key)
				k.timeout = true;
		}
		//>> СБрос в состояние по умолчанию
		void Reset()
		{
			for (auto & k : key)
				k.Reset();
		}
	};

	class CStatusIO
	{
	public:
		CStatusMouse		mouse;
		CStatusWindow		window;
		CStatusKeyboard		keyboard;
	public:
		CStatusIO(){};
		~CStatusIO(){};
		void Close()
		{
			mouse.Reset();
			window.Reset();
			keyboard.Reset();
		}
	public:
		CStatusIO(const CStatusIO& src)            = delete;
		CStatusIO(CStatusIO&& src)                 = delete;
		CStatusIO& operator=(CStatusIO&& src)      = delete;
		CStatusIO& operator=(const CStatusIO& src) = delete;
	};
}

#endif // _STATUSIO_H