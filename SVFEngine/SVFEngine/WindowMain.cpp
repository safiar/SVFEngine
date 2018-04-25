// ----------------------------------------------------------------------- //
//
// MODULE  : WindowMain.cpp
//
// PURPOSE : Основное окно
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#include "WindowMain.h"

using namespace SAVFGAME;

namespace SAVFGAME
{
	bool CWindowMain::BindWindowProc(CWindowMain * ptr)
	{
		return CWindowProc::SetWindowMain(ptr);
	}
	void CWindowMain::DropWindowProc()
	{
		CWindowProc::DropWindowMain();
	}
	bool CWindowMain::SetWindowProc()
	{
		if (!BindWindowProc(this))
			{ _MBM(ERROR_PointerAlready); return false; }
		wInfo.wc.lpfnWndProc = CWindowProc::WindowProcMain;
		return true;
	}
}