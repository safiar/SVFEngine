// ----------------------------------------------------------------------- //
//
// MODULE  : Render.h
//
// PURPOSE : Интерфейс класса рендера
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "StatusIO.h"

#ifndef _RENDER_H
#define _RENDER_H

namespace SAVFGAME
{
	class CRender // virtual base class
	{
		struct RenderToFramework
		{
			int                 x, y;						// move cursor to x, y  [client window rectangle]
			bool				centered_cursor   { false };
			bool				show_cursor       { true  };
		protected:		
			bool				reset_render_call { false };
			bool				reset_window_call { false };
			bool                minimize_call     { false };
			bool                move_cursor       { false };
		public:
			bool ResetRenderCheck()    { _RETBOOL(reset_render_call); }
			bool CursorMoveCheck()     { _RETBOOL(move_cursor);       }
			bool WindowMinimizeCheck() { _RETBOOL(minimize_call);     }
			bool WindowResetCheck()    { _RETBOOL(reset_window_call); }
			void ResetRenderCall()     { reset_render_call = true;    }
			void WindowResetCall()     { reset_window_call = true;    }
			void WindowMinimizeCall()  { minimize_call = true;        }
			void CursorMoveCall()      { move_cursor = true;          }

		};
	public:
		RenderToFramework	to_framework;
	protected:
		CStatusWindow *		state { nullptr };
		HWND				hwnd  { nullptr };
	public:
		CRender(const CRender& src)				= delete;
		CRender(CRender&& src)					= delete;
		CRender& operator=(CRender&& src)		= delete;
		CRender& operator=(const CRender& src)	= delete;
	public:
		CRender(){};
		virtual ~CRender() {};
		virtual bool Init(const HWND hWnd, const CStatusWindow * pState) = 0;
		virtual bool Reset(const HWND hWnd, const CStatusWindow * pState) = 0;
		virtual bool Update() = 0;
		virtual bool Run() = 0;
		virtual void Close() = 0;
	};
}

#endif // _RENDER_H