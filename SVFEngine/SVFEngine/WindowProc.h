// ----------------------------------------------------------------------- //
//
// MODULE  : WindowProc.h
//
// PURPOSE : Обработчик событий окна
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _WINDOWPROC_H
#define _WINDOWPROC_H

#include "stdafx.h"
#include "WindowMain.h"

namespace SAVFGAME
{
	class CWindowMain;

	class CWindowProc
	{
	protected:
		static CWindowMain* pWindowMain;
	public:		
		static LRESULT CALLBACK WindowProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static bool SetWindowMain(CWindowMain*);
		static CWindowMain* const GetWindowMain();
		static void DropWindowMain();
	};
}

#endif // _WINDOWPROC_H