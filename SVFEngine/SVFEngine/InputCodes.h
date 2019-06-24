// ----------------------------------------------------------------------- //
//
// MODULE  : InputCodes.h
//
// PURPOSE : Коды событий ввода
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _INPUTCODES_H
#define _INPUTCODES_H

namespace SAVFGAME
{
	enum eMouseButton
	{
		M_LEFT,
		M_MIDDLE,
		M_RIGHT,

		M_ENUM_MAX
	};
	enum eKeyBoard	// Virtual-Key Codes https://msdn.microsoft.com/ru-ru/library/windows/desktop/dd375731(v=vs.85).aspx
	{
		K_0x00        = 0x00, // __Reserved__

		K_LBUTTON     = 0x01, // Левая кнопка мыши
		K_RBUTTON     = 0x02, // Правая кнопка мыши
		K_CANCEL      = 0x03,
		K_MBUTTON     = 0x04, // Средняя кнопка мыши
		K_XBUTTON1    = 0x05, // Боковая кнопка мыши (ближняя)
		K_XBUTTON2    = 0x06, // Боковая кнопка мыши (дальняя)
		K_0x07        = 0x07, // __Undefined__
		K_BACKSPACE   = 0x08,
		K_TAB         = 0x09, //
		K_0x0A		  = 0x0A, // __Reserved__
		K_0x0B        = 0x0B, // __Reserved__
		K_CLEAR       = 0x0C,
		K_ENTER       = 0x0D, //
		K_0x0E        = 0x0E, // __Undefined__
		K_0x0F        = 0x0F, // __Undefined__
		K_SHIFT       = 0x10,
		K_CTRL        = 0x11,
		K_ALT	      = 0x12,
		K_PAUSE       = 0x13,
		K_CAPSLOCK    = 0x14,
		K_KANA        = 0x15, // IME Kana mode
		K_HANGUEL     = 0x15, // IME Hanguel mode
		K_HANGUL      = 0x15, // IME Hangul mode
		K_0x16        = 0x16, // __Undefined__
		K_JUNJA       = 0x17, // IME Junja mode
		K_FINAL       = 0x18, // IME final mode
		K_HANJA       = 0x19, // IME Hanja mode
		K_KANJI       = 0x19, // IME Kanji mode
		K_0x1A        = 0x1A, // __Undefined__

		K_ESCAPE      = 0x1B,
		K_CONV        = 0x1C, // IME convert
		K_NONCONV     = 0x1D, // IME nonconvert
		K_ACCEPT      = 0x1E, // IME accept
		K_MODECHANGE  = 0x1F, // IME mode change request
		K_SPACE       = 0x20,
		K_PAGEUP      = 0x21,
		K_PAGEDOWN    = 0x22,
		K_END         = 0x23,
		K_HOME        = 0x24,
		K_LEFT        = 0x25, // стрелка влево
		K_UP          = 0x26, // стрелка вверх
		K_RIGHT       = 0x27, // стрелка вправо
		K_DOWN        = 0x28, // стрелка вниз
		K_SELECT      = 0x29,
		K_EXE         = 0x2B, // execute key
		K_SNAPSHOT    = 0x2C,
		K_INSERT      = 0x2D,
		K_DELETE      = 0x2E,
		K_HELP        = 0x2F,

		K_0           = 0x30,
		K_1           = 0x31,
		K_2           = 0x32,
		K_3           = 0x33,
		K_4           = 0x34,
		K_5           = 0x35,
		K_6           = 0x36,
		K_7           = 0x37,
		K_8           = 0x38,
		K_9           = 0x39,

		K_0x3A        = 0x3A, // __Undefined__
		K_0x3B        = 0x3B, // __Undefined__
		K_0x3C        = 0x3C, // __Undefined__
		K_0x3D        = 0x3D, // __Undefined__
		K_0x3E        = 0x3E, // __Undefined__
		K_0x3F        = 0x3F, // __Undefined__
		K_0x40        = 0x40, // __Undefined__

		K_A           = 0x41,
		K_B           = 0x42,
		K_C           = 0x43,
		K_D           = 0x44,
		K_E           = 0x45,
		K_F           = 0x46,
		K_G           = 0x47,
		K_H           = 0x48,
		K_I           = 0x49,
		K_J           = 0x4A,
		K_K           = 0x4B,
		K_L           = 0x4C,
		K_M           = 0x4D,
		K_N           = 0x4E,
		K_O           = 0x4F,
		K_P           = 0x50,
		K_Q           = 0x51,
		K_R           = 0x52,
		K_S           = 0x53,
		K_T           = 0x54,
		K_U           = 0x55,
		K_V           = 0x56,
		K_W           = 0x57,
		K_X           = 0x58,
		K_Y           = 0x59,
		K_Z           = 0x5A,

		K_WINLEFT     = 0x5B,
		K_WINRIGHT    = 0x5C,
		K_APPS        = 0x5D,
		K_0x5E        = 0x5E, // __Reserved__

		K_SLEEP       = 0x5F, // Computer Sleep key
		K_NUMPAD0     = 0x60,
		K_NUMPAD1     = 0x61,
		K_NUMPAD2     = 0x62,
		K_NUMPAD3     = 0x63,
		K_NUMPAD4     = 0x64,
		K_NUMPAD5     = 0x65,
		K_NUMPAD6     = 0x66,
		K_NUMPAD7     = 0x67,
		K_NUMPAD8     = 0x68,
		K_NUMPAD9     = 0x69,

		K_MULTIPLY    = 0x6A,
		K_ADD         = 0x6B,
		K_SEPARATOR   = 0x6C,
		K_SUBTRACT    = 0x6D,
		K_DECIMAL     = 0x6E,
		K_DIVIDE      = 0x6F,

		K_F1          = 0x70,
		K_F2          = 0x71,
		K_F3          = 0x72,
		K_F4          = 0x73,
		K_F5          = 0x74,
		K_F6          = 0x75,
		K_F7          = 0x76,
		K_F8          = 0x77,
		K_F9          = 0x78,
		K_F10         = 0x79,
		K_F11         = 0x7A,
		K_F12         = 0x7B,
		K_F13         = 0x7C,
		K_F14         = 0x7D,
		K_F15         = 0x7E,
		K_F16         = 0x7F,
		K_F17         = 0x80,
		K_F18         = 0x81,
		K_F19         = 0x82,
		K_F20         = 0x83,
		K_F21         = 0x84,
		K_F22         = 0x85,
		K_F23         = 0x86,
		K_F24         = 0x87,

		// 0x88 .. 0x8F  skipped

		K_NUMLOCK     = 0x90,
		K_SCROLL      = 0x91,

		// 0x92 .. 0x9F  skipped

		K_LSHIFT      = 0xA0,
		K_RSHIFT      = 0xA1,
		K_LCONTROL    = 0xA2,
		K_RCONTROL    = 0xA3,
		K_LALT        = 0xA4,
		K_RALT        = 0xA5,

		// 0xA6 .. 0xBA  skipped

		K_PLUS        = 0xBB,
		K_COMMA       = 0xBC,
		K_MINUS       = 0xBD,
		K_PERIOD      = 0xBE,

		// 0xBF .. 0xDB  skipped

		K_EXPONENT    = 0xDC,

		// 0xDD .. 0xF5  skipped

		K_ATTN        = 0xF6,
		K_CRSEL       = 0xF7,
		K_EXSEL       = 0xF8,
		K_EREOF       = 0xF9,
		K_PLAY        = 0xFA,
		K_ZOOM        = 0xFB,
		K_NONAME      = 0xFC,
		K_PA1         = 0xFD,
		K_OEMCLEAR    = 0xFE,
		K_0xFF        = 0xFF, // __Undefined__

		// SPECIAL : MOUSE WHEEL //

		K_WHEEL_UP   = 0x100, // mouse wheel UP status
		K_WHEEL_DOWN = 0x101, // mouse wheel DOWN status

		K_ENUM_MAX   = 0x102
	};
}

#endif // _INPUTCODES_H