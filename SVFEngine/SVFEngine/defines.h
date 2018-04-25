// ----------------------------------------------------------------------- //
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

#include <windows.h>
#include <windowsx.h>

#define DIRECTORY_GAME			L"C:\\TheGame"				// default testing directory
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

#define _EQUAL(a,b)			(!(a^b))
#define _EQUAL2(a,b,c)		(!(a^b) || !(a^c))
#define _EQUAL3(a,b,c,d)	(!(a^b) || !(a^c) || !(a^d))
#define _NOTEQUAL(a,b)		(a^b)

#define _SZTP(x)			sizeof(decltype(x))

#define _CLAMP(x,MIN,MAX)   min(max(x,MIN),MAX)

#define _SWAP(A,B)			{ auto C = A; A = B; B = C; }

#define _BOOL(x)			(x!=0)?true:false
#define _SLEN(s)			s, strlen(s)
#define _RETBOOL(x)			bool b=x; x=false; return b;

#define _TIME				monoclock::now()
#define _TIMER(start)		std::chrono::duration_cast<std::chrono::milliseconds>(_TIME-start).count()

#define _SLEEP(ms)			std::this_thread::sleep_for(std::chrono::milliseconds(ms))

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