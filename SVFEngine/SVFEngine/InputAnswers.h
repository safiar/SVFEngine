// ----------------------------------------------------------------------- //
//
// MODULE  : InputAnswers.h
//
// PURPOSE : Интерфейс класса ответов на события окна
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _INPUTANSWERS_H
#define _INPUTANSWERS_H

#include "stdafx.h"
#include "StatusIO.h"

namespace SAVFGAME
{	
	struct MousePos // координаты мыши
	{
		MousePos(const int _x, const int _y, CStatusWindow * _w) : x(_x), y(_y), w(_w) {}

		int x;				
		int y;
		CStatusWindow * w;
	};
	struct MouseButton : public MousePos // кнопка мыши + координаты мыши
	{
		MouseButton(const eMouseButton _mButton, const int _x, const int _y, CStatusWindow * _w)
			: MousePos(_x, _y, _w), mButton(_mButton) {}

		eMouseButton mButton;
	};
	struct MouseWheel : public MousePos // колёсико мыши + координаты мыши
	{
		MouseWheel(const char _wheel, const int _x, const int _y, CStatusWindow * _w)
			: MousePos(_x, _y, _w), wheel(_wheel) {}

		char wheel;
	};
	struct KeyboardButton // клавиши клавиатуры
	{
		KeyboardButton(const wchar_t _wchar, const eKeyBoard _key) : wchar(_wchar), key(_key) {}

		wchar_t wchar;
		eKeyBoard key;
	};
	
	class CInputAnswers // virtual base class : определяет ответы на события окна
	{
	public:
		CInputAnswers(){};
		virtual ~CInputAnswers(){};
		virtual bool MouseMoved    (const MousePos        & arg)	{ return false; }
		virtual bool MousePressed  (const MouseButton     & arg)	{ return false; }
		virtual bool MouseReleased (const MouseButton     & arg)	{ return false; }
		virtual bool MouseWheeled  (const MouseWheel      & arg)	{ return false; }
		virtual bool KeyPressed    (const KeyboardButton  & arg)	{ return false; }
		virtual bool KeyReleased   (const KeyboardButton  & arg)	{ return false; }
	};
}

#endif // _INPUTANSWERS_H