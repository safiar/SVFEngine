// ----------------------------------------------------------------------- //
//
// MODULE  : InputAnswersMain.h
//
// PURPOSE : Обработка событий окна
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _INPUTANSWERSMAIN_H
#define _INPUTANSWERSMAIN_H

#include "stdafx.h"
#include "link_video.h"
#include "InputAnswers.h"

using namespace SAVFGAME;

namespace SAVFGAME
{
	class CMainInputAnswers final : public CInputAnswers
	{
	protected:
		CRenderT *	render;
		CWorld *	world;
	private:
	public:
		CMainInputAnswers() : CInputAnswers() { };
		~CMainInputAnswers(){};
		bool KeyPressed(const KeyboardButton &arg)  override final
		{
			//wprintf(L"\nkey pressed: %c %4x (wchar) %2x (key)", arg.wchar, arg.wchar, arg.key);

			world->UpdateKey(arg.key, arg.wchar, true);

			return true;
		}
		bool KeyReleased(const KeyboardButton &arg)	override final
		{
			//wprintf(L"\nkey released: %c %4x (wchar) %2x (key)", arg.wchar, arg.wchar, arg.key);

			world->UpdateKey(arg.key, arg.wchar, false);

			return true;
		}
		bool MouseMoved(const MousePos &arg)		override final
		{
			//wprintf(L"\nmouse pos (x,y): %4d - %4d", arg.x, arg.y);

			world->UpdateMouse(arg.x, arg.y);

			return true;
		}
		bool MousePressed(const MouseButton &arg)	override final
		{
			//wprintf(L"\nmouse pressed: %2d (%4d - %4d)", arg.mButton, arg.x, arg.y);

			world->UpdateMouse(arg.mButton, true);

			return true;
		}
		bool MouseReleased(const MouseButton &arg)	override final
		{
			//wprintf(L"\nmouse pressed: %2d (%4d - %4d)", arg.mButton, arg.x, arg.y);

			world->UpdateMouse(arg.mButton, false);

			return true;
		}
		bool MouseWheeled(const MouseWheel &arg)	override final
		{
			//wprintf(L"\nmouse wheeled: %3d (%4d - %4d)", arg.wheel, arg.x, arg.y);

			world->UpdateMouse(arg.wheel);

			return true;
		}
		
		void SetRender(const CRenderT * pRender)
		{
			render = const_cast<CRenderT*>(pRender);
			world = & render->world;
		};
	};
}

#endif // _INPUTANSWERSMAIN_H