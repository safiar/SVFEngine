﻿// ----------------------------------------------------------------------- //
//
// MODULE  : defines.h
//
// PURPOSE : Вспомогательные объявления
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _DEFINES_H
#define _DEFINES_H

#include "defines_core.h"

#if _OS_WINDOWS
	#include <windows.h>
	#include <windowsx.h>
#endif

#define DATAMAKEBY   " Generated by SVF Engine "
#define DATAMAKEBYW L" Generated by SVF Engine "

/* default testing directory */																	/**/
#define DIRECTORY_GAME			L"C:\\TheGame"													/*
                                                                                                /**/
#define DIRECTORY_DATAFOLDER	L"data"
#define DIRECTORY_FONTS			L"data\\fonts"
#define DIRECTORY_MAPS          L"data\\maps"
#define DIRECTORY_MODELS		L"data\\models"
#define DIRECTORY_SHADERS		L"data\\shaders"
#define DIRECTORY_SOUNDS		L"data\\sounds"
#define DIRECTORY_TEXTURES		L"data\\textures"
#define DIRECTORY_TEXTURESMDL	L"data\\textures\\models"
#define DIRECTORY_TEXTURESSKY	L"data\\textures\\skybox"
#define DIRECTORY_SPRITES       L"data\\textures\\sprites"

#define BASE_SETTINGS_FILENAME  L"settings.ini"

#define DIRECTORY_BASE_UI_SPRITES L"sprites/ui/menu.ini"

#ifndef TRUE
	#define TRUE  1
#endif
#ifndef FALSE
	#define FALSE 0
#endif
#ifndef NULL
	#define NULL  0
#endif

#define ONE        1
#define TWO        2
#define THREE      3

#ifndef MAX_PATH
	#define MAX_PATH 260
#endif

#define  MISSING		-1
#define _ISMISS(x)		x == (decltype(x))MISSING
#define _NOMISS(x)		x != (decltype(x))MISSING
#define __ISMISS(x)		x == (std::decay_t<decltype(x)>)MISSING
#define __NOMISS(x)		x != (std::decay_t<decltype(x)>)MISSING

#define CULL_DEFCCW	 TRUE	// [GLOBAL SETTING] default cullmode : CCW (TRUE) or CW (FALSE) 

#define IGNORE_MB_ERRORS  FALSE

#define _MB(a,b,c,d)		{ if (IGNORE_MB_ERRORS == FALSE) MessageBox(a,b,c,d); }
#define _MBM(message)		{ if (IGNORE_MB_ERRORS == FALSE) MessageBox(NULL, message, ERROR_Warning, ERROR_MB); }
#define _MBS(a,b,c,d)		{ MessageBoxA(a,(char*)b->GetBufferPointer(),c,d); _RELEASE(b); } // shader compiling error

#define ERROR_MB				MB_ICONEXCLAMATION | MB_OK
#define ERROR_Warning			L"Внимание!"
#define ERROR_UnknownDataType	L"Неизвестный тип данных"
#define ERROR_Unknown			L"Неизвестная ошибка"
#define ERROR_FormatNoSupport	L"Неподдерживаемый формат"
#define ERROR_FormatRestricted	L"Недопустимый формат"
#define ERROR_NUMZERO			L"Значение не может быть NULL"
#define ERROR_IDLIMIT			L"ID вне диапазона"
#define ERROR_IDRANGE			L"Ошибка диапазона в значении ID"
#define ERROR_NETBUFLIMIT		L"Превышен размер сетевого сообщения"
#define ERROR_BUFLIMIT			L"Некорректный размер буфера"
#define ERROR_BUFLOAD			L"Не удалось загрузить буфер"
#define ERROR_PTRNOTNULLPTR		L"Указатель не nullptr"
#define ERROR_VARIABLE			L"Некорректное значение переменной"
#define ERROR_THREAD			L"Ошибка работы с потоком"
#define ERROR_MEMORY			L"Ошибка работы с памятью"
#define ERROR_WINDOW			L"Ошибка работы с окном"
#define ERROR_InitNone			L"Инициализация не произведена"
#define ERROR_InitAlready		L"Инициализация уже произведена"
#define ERROR_PointerNone		L"Пропущен указатель на объект"
#define ERROR_PointerAlready	L"Указатель на объект уже задан"
#define ERROR_OpenFile			L"Не удалось открыть файл"
#define ERROR_ReadFile			L"Ошибка чтения файла"
#define ERROR_RegClipboardType  L"Не удалось зарегистрировать тип данных буфера обмена"
#define ERROR_StreamANotSup		L"Устройство вывода не поддерживает аудиопоток"
#define ERROR_CrtVertexBuf		L"Не удалось CreateVertexBuffer()"
#define ERROR_CrtIndexBuf		L"Не удалось CreateIndexBuffer()"
#define ERROR_CrtTextureFFIM	L"Не удалось CreateTextureFromFileInMemory()"
#define ERROR_CrtTextureFF		L"Не удалось CreateTextureFromFile()"
#define ERROR_FntLoadFailed		L"Не удалось загрузить шрифт"
#define ERROR_MdlSetCullmode    L"Модель уже создана! Выгрузите модель для изменения режима и загрузите снова"
#define ERROR_MdlNoMaterial     L"Не указан материал для набора вершин"
#define ERROR_ResetDevice		L"Не удалось Reset() девайса"
#define ERROR_ResetDeviceINV	L"Не удалось Reset() девайса: D3DERR_INVALIDCALL. Освободите ресурсы POOL_DEFAULT"
#define ERROR_ObjVertexNum		L"OBJ-Reader: Ошибка количества вертексов в структуре поверхности"
#define ERROR_ObjVertexFormat	L"OBJ-Reader: Ошибка структуры вертексов"
#define ERROR_ObjVertexMax		L"OBJ-Reader: Ошибка максимального вертекса"
#define ERROR_ShaderCompile		L"Не удалось скомпилировать шейдер"
#define ERROR_ShaderCompile_F	L"Не удалось скомпилировать эффект"
#define ERROR_ShaderCompile_2	L"Не удалось установить шейдер после компиляции"
#define ERROR_ShaderCompiler     "Компилятор шейдера"
#define ERROR_VertexDecl		L"Не удалось задать описание вершин"
#define ERROR_ShaderNotExists	L"Шейдер не скомпилирован"
#define ERROR_ShaderCollision	L"Другой шейдер вершинного типа уже был включен"
#define ERROR_ShaderCollision2	L"Другой шейдер пиксельного типа уже был включен"
#define ERROR_ShaderCollision3	L"Другой эффект уже был включен"
#define ERROR_ShaderEffNoHndl	L"HANDLE ещё не установлены, для установки включите шейдер методом State()"
#define ERROR_ShaderMTLDATA		L"Ошибка установки сведений о материале в пиксельный шейдер"
#define ERROR_MatrixDetNull		L"Нулевой определитель матрицы (?)"
#define ERROR_OnLostDevice		L"Не удалось выполнить OnLostDevice()"
#define ERROR_OnResetDevice		L"Не удалось выполнить OnResetDevice()"
#define ERROR_DepthStencilFrmt  L"Не удалось выбрать формат буфера глубины/трафарета"
#define ERROR_IndexBuf32		L"Отсутствует поддержка 32bit index buffer"
#define ERROR_PresentInterv0	L"Отсутствует поддержка presentation interval IMMEDIATE"
#define ERROR_PresentInterv1	L"Отсутствует поддержка presentation interval 1"
#define ERROR_PresentInterv2	L"Отсутствует поддержка presentation interval 2"
#define ERROR_PresentInterv3	L"Отсутствует поддержка presentation interval 3"
#define ERROR_PresentInterv4	L"Отсутствует поддержка presentation interval 4"
#define ERROR_ShaderEffTchMiss	L"Не выбрана шейдерная техника для эффекта"

#define GAMEWINDOWEDNOBORDER_  (WS_EX_TOPMOST | WS_POPUP)				// нет границ
#define GAMEWINDOWEDTITLE_     (WS_BORDER | WS_CLIPCHILDREN)			// только название
#define GAMEWINDOWED_          (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN)  // стандартные кнопки: свернуть, раскрыть, закрыть
#define GAMEFULLSCREEN_        (WS_EX_TOPMOST | WS_POPUP)				// нет границ

#define GAMEWINDOWED			GAMEWINDOWED_		// using this
#define GAMEFULLSCREEN			GAMEFULLSCREEN_		// using this

#define GAMEWINDOWED_EX_		NULL // пока ничего
#define GAMEFULLSCREEN_EX_		NULL // пока ничего

#define GAMEWINDOWED_EX			GAMEWINDOWED_EX_	// using this
#define GAMEFULLSCREEN_EX		GAMEFULLSCREEN_EX_	// using this

#define _DEPRECATED         __declspec(deprecated)  // для функций на удаление

#define _NOTHROW            (std::nothrow)
#define _NEW_NOTHROW        new _NOTHROW

#define _CEIL32(f)          ( (static_cast<float>(static_cast<int32>((f))) == (f)) ? static_cast<int32>((f)) : static_cast<int32>((f)) + (((f) > 0) ? 1 : 0) )

#define _FREE(p)			{ if ((p)!=nullptr) {free (p);                   (p)=nullptr;} }
#define _DELETE(p)			{ if ((p)!=nullptr) {delete (p);                 (p)=nullptr;} }
#define _DELETEN(p)			{ if ((p)!=nullptr) {delete[] (p);               (p)=nullptr;} }
#define _RELEASE(p)			{ if ((p)!=nullptr) {(p)->Release();             (p)=nullptr;} }
#define _CLOSE(p)			{ if ((p)!=nullptr) {(p)->Close();   delete (p); (p)=nullptr;} }
#define _CLOSESHARED(p)		{ static_assert (														\
                              !std::is_same < decltype(p), std::shared_ptr<decltype(*p)> > ::value,	\
                              "_CLOSESHARED : This is accept only shared_ptr type" );				\
							  if ((p)!=nullptr) {(p)->Close();               (p)=nullptr;} }

#define _CHECKBIT(var,pos)	((var) &   (1ULL<<(pos)))
#define _CLEARBIT(var,pos)	((var) &= ~(1ULL<<(pos)))
#define _SETBIT(var,pos)	((var) |=  (1ULL<<(pos)))
#define _FLIPBIT(var,pos)	((var) ^=  (1ULL<<(pos)))

#define _SETMASK(var,mask)		((var)  |= (mask))
#define _CLEARMASK(var,mask)	((var)  &= (~(mask)))
#define _FLIPMASK(var,mask)		((var)  ^= (mask))
#define _CHECKALLMASK(var,mask)	(((var) &  (mask)) == (mask))
#define _CHECKANYMASK(var,mask)	((var)  &  (mask))

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define _EQUAL(a,b)			(!(a^b))
#define _EQUAL2(a,b,c)		(!(a^b) || !(a^c))
#define _EQUAL3(a,b,c,d)	(!(a^b) || !(a^c) || !(a^d))
#define _NOTEQUAL(a,b)		(a^b)

#define _SZTP(x)			sizeof(decltype(x))
#define _ELEMENTS(array_t)	sizeof(array_t) / sizeof(std::decay_t<decltype(array_t[0])>)

#define _CLAMP(x,MIN,MAX)   min(max(x,MIN),MAX)

#define _SWAP(A,B)			std::swap(A,B) // { auto C = A; A = B; B = C; }

#define _BOOL(x)			((x)!=0) //((x)!=0)?true:false
#define _SLEN(s)			s, strlen(s)
#define _RETBOOL(x)			bool b=x; x=false; return b;
#define _BOOLYESNO(x)		((x)!=0)?("yes"):("no")

#define _TIME				std::chrono::steady_clock::now() // текущее время _TIME = monoclock::now()
#define _TIMEMS(start,end)  std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()   // время между start и end (в мс.)
#define _TIMER(start)		std::chrono::duration_cast<std::chrono::milliseconds>(_TIME-start).count() // прошло времени с момента _TIME = start
#define T_TIME				decltype(_TIME)

#define _SLEEP(ms)			std::this_thread::sleep_for(std::chrono::milliseconds(ms))

#define _ABORT				std::abort()
#define _EXIT(code)         std::exit((int)code)

#define _BYTESTOKB(x)       x/0x400
#define _BYTESTOMB(x)       x/0x100000
#define _BYTESTOGB(x)       x/0x40000000
#define _KBTOMB(x)          x/0x400
#define _KBTOGB(x)          x/0x100000
#define _MBTOGB(x)          x/0x400

#define MASK_BIT_00 0x00000001 // 1 << 0
#define MASK_BIT_01 0x00000002 // 1 << 1
#define MASK_BIT_02 0x00000004 // 1 << 2
#define MASK_BIT_03 0x00000008 // 1 << 3
#define MASK_BIT_04 0x00000010 // 1 << 4
#define MASK_BIT_05 0x00000020 // 1 << 5
#define MASK_BIT_06 0x00000040 // 1 << 6
#define MASK_BIT_07 0x00000080 // 1 << 7
#define MASK_BIT_08 0x00000100 // 1 << 8
#define MASK_BIT_09 0x00000200 // 1 << 9
#define MASK_BIT_10 0x00000400 // 1 << 10
#define MASK_BIT_11 0x00000800 // 1 << 11
#define MASK_BIT_12 0x00001000 // 1 << 12
#define MASK_BIT_13 0x00002000 // 1 << 13
#define MASK_BIT_14 0x00004000 // 1 << 14
#define MASK_BIT_15 0x00008000 // 1 << 15
#define MASK_BIT_16 0x00010000 // 1 << 16
#define MASK_BIT_17 0x00020000 // 1 << 17
#define MASK_BIT_18 0x00040000 // 1 << 18
#define MASK_BIT_19 0x00080000 // 1 << 19
#define MASK_BIT_20 0x00100000 // 1 << 20
#define MASK_BIT_21 0x00200000 // 1 << 21
#define MASK_BIT_22 0x00400000 // 1 << 22
#define MASK_BIT_23 0x00800000 // 1 << 23
#define MASK_BIT_24 0x01000000 // 1 << 24
#define MASK_BIT_25 0x02000000 // 1 << 25
#define MASK_BIT_26 0x04000000 // 1 << 26
#define MASK_BIT_27 0x08000000 // 1 << 27
#define MASK_BIT_28 0x10000000 // 1 << 28
#define MASK_BIT_29 0x20000000 // 1 << 29
#define MASK_BIT_30 0x40000000 // 1 << 30
#define MASK_BIT_31 0x80000000 // 1 << 31

#define MASK_INT32     0xFFFFFFFF
#define MASK_INT32_MAX 0x7FFFFFFF
#define MASK_INT16     0xFFFF
#define MASK_INT16_MAX 0x7FFF
#define MASK_INT8      0xFF
#define MASK_INT8_MAX  0x7F

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// RAW CODE //////////// RAW CODE //////////// RAW CODE //////////// RAW CODE //////////// RAW CODE ////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CODESKIPSTRING			{                         for (;; fpbuf++) if (_EQUAL(*fpbuf, 0x0A))  break; }
#define CODESKIPSPACE			{ if (_EQUAL(*fpbuf,' ')) for (;; fpbuf++) if (_NOTEQUAL(*fpbuf,' ')) break; }

#define CODESKIPCOMMENTS        if (_EQUAL3(*fpbuf, '#', 0x0A, 0x0D)) CODESKIPSTRING

#define CODEREADSTRINGVALUE		for (n=0;; n++, fpbuf++)									\
								{															\
										 if (_EQUAL(*fpbuf, '#'))   break; /* comments */	\
									else if ((byte)(*fpbuf) < 0x20) break; /* system */		\
									str[n] = *fpbuf;										\
								}	str[n] = NULL;

#define CODEREADSTRINGWTEXT		if (_EQUAL(*fpbuf,'"'))													\
								for (fpbuf++, n=0;; n+=2, fpbuf++)										\
								{																		\
										 if (_EQUAL2(*fpbuf, '#', '"')) break; /* comments, strend */	\
									else if ((byte)(*fpbuf) < 0x20)	    break; /* system */				\
									str[n] = *fpbuf;													\
									str[n + 1] = NULL;													\
								}																		\
								else for (n=0;; n+=2, fpbuf++)											\
								{																		\
										 if (_EQUAL2(*fpbuf, '#', ' ')) break; /* comments, space */	\
									else if ((byte)(*fpbuf) < 0x20)	    break; /* system */				\
									str[n] = *fpbuf;													\
									str[n + 1] = NULL;													\
								}	str[n] = NULL; str[n + 1] = NULL;

#define CODEREADSTRINGTEXT		if (_EQUAL(*fpbuf,'"'))													\
								for (fpbuf++, n=0;; n++, fpbuf++)										\
								{																		\
										 if (_EQUAL2(*fpbuf, '#', '"')) break; /* comments, strend */	\
									else if ((byte)(*fpbuf) < 0x20)	    break; /* system */				\
									str[n] = *fpbuf;													\
								}																		\
								else for (n=0;; n++, fpbuf++)											\
								{																		\
										 if (_EQUAL2(*fpbuf, '#', ' ')) break; /* comments, space */	\
									else if ((byte)(*fpbuf) < 0x20)     break; /* system */				\
									str[n] = *fpbuf;													\
								}	str[n] = NULL;

#define CODECOMPARESETTING(s,S)		else if (Compare(fpbuf, _SLEN(s))) { fpbuf += strlen(s); LoadSettings(fpbuf, S); CODESKIPSTRING }

#endif // _DEFINES_H