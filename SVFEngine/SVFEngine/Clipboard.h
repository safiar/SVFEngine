// ----------------------------------------------------------------------- //
//
// MODULE  : Clipboard.h
//
// PURPOSE : Работа с буфером обмена
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _CLIPBOARD_H
#define _CLIPBOARD_H

#include "stdafx.h"

#define SVFEngineChatTextFormatName L"SVF engine chat text"

enum eClipboardAction
{
	ECLIPB_PASTE_TOCONSOLE,	// paste buffer to console
	ECLIPB_PASTE_TOCHAT,	// paste buffer to in-game chat window

	ECLIPB_COPY_FROMCHAT,	// copy text from in-game chat window to buffer
	ECLIPB_COPY_TEST,		// /test/

	ECLIPB_EMPTY_BUFFER,	// clearing buffer

	ECLIPB_ACTION_ENUM_MAX
};

enum eClipboardError
{
	ECLIPB_ERROR_NONE        = 0,		// success returning
	ECLIPB_ERROR_UNSPECIFIED = 1 << 1,	// unregistred type

	ECLIPB_ERROR_NULLHWND    = 1 << 2,	// missing window handler

	ECLIPB_ERROR_REGFORMAT   = 1 << 3,
	ECLIPB_ERROR_OPEN        = 1 << 4,
	ECLIPB_ERROR_CLOSE       = 1 << 5,
	ECLIPB_ERROR_ENUM        = 1 << 6,
	ECLIPB_ERROR_GETFNAME    = 1 << 7,
	ECLIPB_ERROR_EMPTY       = 1 << 8,
	ECLIPB_ERROR_GETDATA     = 1 << 9,
	ECLIPB_ERROR_SETDATA     = 1 << 10,
	ECLIPB_ERROR_GLOCK       = 1 << 11,
	ECLIPB_ERROR_GUNLOCK     = 1 << 12,
	ECLIPB_ERROR_GALLOC      = 1 << 13,

	ECLIPB_ERROR_ENUM_MAX = 0xFFFFFFFF
};

struct ClipboardFormat
{
	uint32 chatText {NULL};	// SVFEngineChatTextFormatName
};

/////////////////////////////////////////////////////////////////////////

enum eClipboardChatTextStatus
{
	ECCTEXT_NONE,
	ECCTEXT_CHAR,
	ECCTEXT_WCHAR
};

struct ClipboardChatText
{
	ClipboardChatText() : status(ECCTEXT_NONE), sz_text(0) {};

	std::string      text;
	std::wstring    wtext;
	size_t        sz_text;

	eClipboardChatTextStatus status;

	//>> Setting up text, status and count of text characters
	void Setup(void * buffer, eClipboardChatTextStatus ccts)
	{
		status = ccts;
		switch (status)
		{
		case ECCTEXT_CHAR:
			text = reinterpret_cast<char*>(buffer);
			sz_text = text.size();
			break;
		case ECCTEXT_WCHAR:
			wtext = reinterpret_cast<wchar_t*>(buffer);
			sz_text = wtext.size();
			break;
		}
	}
};

/////////////////////////////////////////////////////////////////////////

class CClipboard
{
	protected:
		HWND				hwnd   { nullptr };		// дескриптор окна
		bool				opened { false };		// текущий статус доступа
		ClipboardFormat		format;					// содержит ID собственных зарегистрированных форматов
		uint32				error;					// кворумный результат/ошибка последнего запуска
		uint32				error_mem;				// кворумная ошибка последнего неудачного запуска
	public:
		CClipboard(const CClipboard& src)            = delete;
		CClipboard(CClipboard&& src)                 = delete;
		CClipboard& operator=(CClipboard&& src)      = delete;
		CClipboard& operator=(const CClipboard& src) = delete;
	public:
		CClipboard() : error(ECLIPB_ERROR_NONE), error_mem(ECLIPB_ERROR_NONE)
		{
			format.chatText = RegisterClipboardFormat(SVFEngineChatTextFormatName);
			if (!format.chatText)
			{
				error = error_mem = HandleError(ECLIPB_ERROR_REGFORMAT);
			}
		};
		~CClipboard() { };

	public:
		//>> Установка дескриптора окна
		void SetHWND(const HWND window_handler)
		{
			if (window_handler == nullptr)
				{ _MBM(ERROR_PointerNone); return; }

			hwnd = window_handler;
		}

	protected:
		//>> Обработка ошибки
		eClipboardError HandleError(eClipboardError errtype)
		{
			wchar_t exmes[128];
			wchar_t * lpBuffer;
			DWORD errcode = GetLastError();	//HRESULT_FROM_WIN32(errcode);

			if (errcode == ERROR_SUCCESS) return ECLIPB_ERROR_NONE; // fake error

			const DWORD langID = 0; // 0 auto
			const DWORD minSz  = 0; // min lpBuffer sz
			const int va_list  = 0; // NONE va_list * 
			const void * lpSource = nullptr; // NONE source frmt

			switch (errtype)
			{
			case ECLIPB_ERROR_UNSPECIFIED: swprintf_s(exmes,L""); break;
			case ECLIPB_ERROR_REGFORMAT:   swprintf_s(exmes,L"<RegisterClipboardFormat err>"); break;
			case ECLIPB_ERROR_OPEN:        swprintf_s(exmes,L"<OpenClipboard err>");           break;
			case ECLIPB_ERROR_CLOSE:       swprintf_s(exmes,L"<CloseClipboard err>");          break;
			case ECLIPB_ERROR_ENUM:        swprintf_s(exmes,L"<EnumClipboardFormats err>");    break;
			case ECLIPB_ERROR_GETFNAME:    swprintf_s(exmes,L"<GetClipboardFormatName err>");  break;
			case ECLIPB_ERROR_EMPTY:       swprintf_s(exmes,L"<EmptyClipboard err>");          break;
			case ECLIPB_ERROR_GETDATA:     swprintf_s(exmes,L"<GetClipboardData err>");        break;
			case ECLIPB_ERROR_SETDATA:     swprintf_s(exmes,L"<SetClipboardData err>");        break;
			case ECLIPB_ERROR_GLOCK:       swprintf_s(exmes,L"<GlobalLock err>");              break;
			case ECLIPB_ERROR_GUNLOCK:     swprintf_s(exmes,L"<GlobalUnlock err>");            break;
			case ECLIPB_ERROR_GALLOC:      swprintf_s(exmes,L"<GlobalAlloc err>");             break;
			}
			
			wprintf(L"\nClipboard ERRCODE = %X %s", errcode, exmes);

			if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				              FORMAT_MESSAGE_FROM_SYSTEM |
							  FORMAT_MESSAGE_IGNORE_INSERTS,
							  lpSource, errcode, langID, (LPWSTR)&lpBuffer, minSz, va_list))
			{
				wprintf(L"\nClipboard ERRDESC = %s", lpBuffer);
				LocalFree(lpBuffer);
			}
			else
			{
				wprintf(L"\nClipboard ERRDESC = NULL (DESC FAIL ERR %X)", GetLastError());
			}

			if (errtype == ECLIPB_ERROR_REGFORMAT) _MBM(ERROR_RegClipboardType);

			return errtype;
		};

		//>> debug
		void PrintfFormatsInfo()
		{
			printf("\nAvailable formats: %i", CountClipboardFormats());

			const byte fnameSize = 64;
			wchar_t fname[fnameSize];
			uint32 format = EnumClipboardFormats(NULL);
			while (format)
			{
				switch (format)
				{
				case CF_TEXT:            printf("\n- CF_TEXT");            break;
				case CF_BITMAP:          printf("\n- CF_BITMAP");          break;
				case CF_METAFILEPICT:    printf("\n- CF_METAFILEPICT");    break;
				case CF_SYLK:            printf("\n- CF_SYLK");            break;
				case CF_DIF:             printf("\n- CF_DIF");             break;
				case CF_TIFF:            printf("\n- CF_TIFF");            break;
				case CF_OEMTEXT:         printf("\n- CF_OEMTEXT");         break;
				case CF_DIB:             printf("\n- CF_DIB");             break;
				case CF_PALETTE:         printf("\n- CF_PALETTE");         break;
				case CF_PENDATA:         printf("\n- CF_PENDATA");         break;
				case CF_RIFF:            printf("\n- CF_RIFF");            break;
				case CF_WAVE:            printf("\n- CF_WAVE");            break;
				case CF_UNICODETEXT:     printf("\n- CF_UNICODETEXT");     break;
				case CF_ENHMETAFILE:     printf("\n- CF_ENHMETAFILE");     break;
				case CF_HDROP:           printf("\n- CF_HDROP");           break;
				case CF_LOCALE:          printf("\n- CF_LOCALE");          break;
				case CF_DIBV5:           printf("\n- CF_DIBV5");           break;  // CF_MAX
				case CF_OWNERDISPLAY:    printf("\n- CF_OWNERDISPLAY");    break;
				case CF_DSPTEXT:         printf("\n- CF_DSPTEXT");         break;
				case CF_DSPBITMAP:       printf("\n- CF_DSPBITMAP");       break;
				case CF_DSPMETAFILEPICT: printf("\n- CF_DSPMETAFILEPICT"); break;
				case CF_DSPENHMETAFILE:  printf("\n- CF_DSPENHMETAFILE");  break;
				case CF_PRIVATEFIRST:    printf("\n- CF_PRIVATEFIRST");    break;
				case CF_PRIVATELAST:     printf("\n- CF_PRIVATELAST");     break;
				case CF_GDIOBJFIRST:     printf("\n- CF_GDIOBJFIRST");     break;
				case CF_GDIOBJLAST:      printf("\n- CF_GDIOBJLAST");      break;
				default:
					printf("\n- 0x%X", format);
					if (!GetClipboardFormatName(format, fname, fnameSize))
						 HandleError(ECLIPB_ERROR_GETFNAME);
					else wprintf(L" %s", fname);
				}
				format = EnumClipboardFormats(format);
			}
			printf("\n<End of enum formats>");

			if (GetLastError() != ERROR_SUCCESS)
				HandleError(ECLIPB_ERROR_ENUM);
			
		}
	
	public:
		//>> Запуск работы с буфером обмена :: return ECLIPB_ERROR_NONE if success
		uint32 Run(eClipboardAction action, void * pData)
		{	
			error = ECLIPB_ERROR_NONE; // сброс

			if (hwnd == nullptr)
			{
				_MBM(ERROR_PointerNone);
				return error = error_mem = ECLIPB_ERROR_NULLHWND;
			}

			opened = _BOOL(OpenClipboard(hwnd)); // ((HWND)0x77777); // test error

			if (opened)
			{
				switch (action)
				{
				case ECLIPB_PASTE_TOCONSOLE:
					PasteToConsole();
					break;
				case ECLIPB_PASTE_TOCHAT:
					PasteToChat((ClipboardChatText*)pData);
					break;
				case ECLIPB_COPY_FROMCHAT:
					// todo
					break;
				case ECLIPB_COPY_TEST:
					CopyTest((wchar_t*)pData);
					break;
				case ECLIPB_EMPTY_BUFFER:
					if (!EmptyClipboard())
						error |= HandleError(ECLIPB_ERROR_EMPTY);
					break;			
				default:;
				}

				opened = false;

				if (!CloseClipboard()) 
					error |= HandleError(ECLIPB_ERROR_CLOSE);
			}
			else
				error |= HandleError(ECLIPB_ERROR_OPEN);

			if (error != ECLIPB_ERROR_NONE)
				error_mem = error;

			return error;
		}

		//>> Возвращает кворумный набор ошибок <eClipboardError> результата последнего запуска
		uint32 GetLastRunResult()
		{
			return error;
		}

		//>> Возвращает кворумный набор ошибок <eClipboardError> имевших место в последний неудачный запуск
		uint32 GetLastClipboardError()
		{
			return error_mem;
		}

	protected:
		//>> Возвращает текст для вставки в игровой чат
		void PasteToChat(ClipboardChatText * cctext)
		{
			uint32 type = 0;
			if (IsClipboardFormatAvailable(CF_TEXT))         type = CF_TEXT;
			if (IsClipboardFormatAvailable(CF_UNICODETEXT))  type = CF_UNICODETEXT;
			if (IsClipboardFormatAvailable(format.chatText)) type = format.chatText;

			if (!type) return;

			HANDLE data = GetClipboardData(type);
			if (!data) error |= HandleError(ECLIPB_ERROR_GETDATA);
			else
			{
				void * p_text = GlobalLock(data);
				if (!p_text) error |= HandleError(ECLIPB_ERROR_GLOCK);
				else
				{
					switch (type)
					{
					case CF_TEXT:
						cctext->Setup(p_text, ECCTEXT_CHAR);
						break;
					case CF_UNICODETEXT:
						cctext->Setup(p_text, ECCTEXT_WCHAR);
						break;
					default:
						if (type == format.chatText)
							cctext->Setup(p_text, ECCTEXT_WCHAR);
						break;
					}

					if (!GlobalUnlock(data))
						error |= HandleError(ECLIPB_ERROR_GUNLOCK);
				}
			}
		}

		//>> (test) Вставка из буфера в консоль
		void PasteToConsole()
		{	
			// PrintfFormatsInfo();

			uint32 type = 0;
			if (IsClipboardFormatAvailable(CF_TEXT))         type = CF_TEXT;
			if (IsClipboardFormatAvailable(CF_UNICODETEXT))  type = CF_UNICODETEXT;
			if (IsClipboardFormatAvailable(format.chatText)) type = format.chatText;

			if (!type) {
				printf("\n[CLIPBOARD] No available text to paste\n");
				return; }

			HANDLE data = GetClipboardData(type);
			if (!data) error |= HandleError(ECLIPB_ERROR_GETDATA);
			else
			{
				void * p_text = GlobalLock(data);
				if (!p_text) error |= HandleError(ECLIPB_ERROR_GLOCK);
				else
				{
					switch (type)
					{
					case CF_TEXT:          printf( "\n%s", (char*)    p_text); break;
					case CF_UNICODETEXT:  wprintf(L"\n%s", (wchar_t*) p_text); break;
					default:
						if (type == format.chatText)
							wprintf(L"\nCHAT_TEXT\n%s", (wchar_t*) p_text);
						break;
					}
					if (!GlobalUnlock(data))
						error |= HandleError(ECLIPB_ERROR_GUNLOCK);
				}
			}
		}

		//>> (test) Копирование в буфер
		void CopyTest(wchar_t * pText)
		{
			wstring text = L"Test text for testing clipboard.\nNo more comments.\nP.S. 1+1=2";
			if (pText != nullptr)
				text = pText;
			size_t textsz = (text.size() + 1) * _SZTP(*text.c_str());

			if (!EmptyClipboard())
				{ error |= HandleError(ECLIPB_ERROR_EMPTY); return; }

			HANDLE hGlobMem = GlobalAlloc(GMEM_MOVEABLE, textsz);
			if (!hGlobMem) error |= HandleError(ECLIPB_ERROR_GALLOC);
			else
			{
				void * buffer = GlobalLock(hGlobMem);
				if (!buffer) error |= HandleError(ECLIPB_ERROR_GLOCK);
				else
				{
					memcpy(buffer, text.c_str(), textsz);

					if (!GlobalUnlock(hGlobMem))
						error |= HandleError(ECLIPB_ERROR_GUNLOCK);

					if (error == ECLIPB_ERROR_NONE) // check if there are any errors
					{
						if (format.chatText != NULL)
							if (!SetClipboardData(format.chatText, hGlobMem))
								error |= HandleError(ECLIPB_ERROR_SETDATA);

						if (!SetClipboardData(CF_UNICODETEXT, hGlobMem))
							error |= HandleError(ECLIPB_ERROR_SETDATA);
					}
				}
			}
		}
};

#endif // _CLIPBOARD_H