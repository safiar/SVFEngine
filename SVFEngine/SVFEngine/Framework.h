// ----------------------------------------------------------------------- //
//
// MODULE  : Framework.h
//
// PURPOSE : Определяет среду разработки
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _FRAMEWORK_H
#define _FRAMEWORK_H

#include "WindowMain.h"
#include "WindowProc.h"
#include "InputManager.h"
#include "Render.h"

using namespace SAVFGAME;

namespace SAVFGAME
{
	enum eProcArch
	{
		EPA_AMD64,
		EPA_INTEL,
		EPA_MIPS,
		EPA_PPC,
		EPA_ARM,
		EPA_IA64,
		EPA_IA32,	// EPA_IA32_ON_WIN64
		EPA_UNKNOWN
	};

	struct SYSINFO
	{
		eProcArch	processor_architecture;
		uint32		page_size;				// page size and the granularity of page protection and commitment
		void *		minimum_app_addr;		// pointer to the lowest memory address accessible to applications and dynamic-link libraries
		void *		maximum_app_addr;		// pointer to the highest memory address accessible to applications and DLLs
		uint32		active_processor_mask;	// mask representing the set of processors : bit 0 is processor 0; bit 31 is processor 31
		uint32		num_of_processors;
		uint32		processor_type;
		uint32		allocation_granularity; // granularity for the starting address at which virtual memory can be allocated
		uint16		processor_level;
		uint16		processor_revision;
		///////////////////////////////
		void Printf()
		{
			printf("\nSYSTEM INFORMATION");
			printf("\n- proc arch  = ");
			switch (processor_architecture)
			{
			case EPA_AMD64:   printf("AMD64");   break;
			case EPA_INTEL:   printf("INTEL");   break;
			case EPA_MIPS:    printf("MIPS");    break;
			case EPA_PPC:     printf("PPC");     break;
			case EPA_ARM:     printf("ARM");     break;
			case EPA_IA64:    printf("IA64");    break;
			case EPA_IA32:    printf("IA32");    break;
			case EPA_UNKNOWN: printf("unknown"); break;
			}
			printf("\n- page size  = 0x%X", page_size);
			printf("\n- alloc gran = 0x%X", allocation_granularity);
			printf("\n- min addr   = 0x%X", (int)minimum_app_addr);
			printf("\n- max addr   = 0x%X", (int)maximum_app_addr);
			printf("\n- num procs  = %i", num_of_processors);
			printf("\n- proc type  = %X", processor_type);
			printf("\n- proc level = %X", processor_level);
			printf("\n- proc rev   = %X", processor_revision);
			for (uint32 i=0; i<num_of_processors; i++)
			{
				printf("\n- proc %2i    = ", i);
				if ((active_processor_mask >> i) & 1)
					 printf("active");
				else printf("inactive");
			}
			
		}
	};

	class CFramework
	{
		SYSINFO				system_info; // [DEBUG]
	protected:
		HWND                hwnd          { nullptr };
		CStatusWindow *		state         { nullptr };
		CWindowMain *		window        { nullptr };
		CInputManager *		imanager      { nullptr };
		CRender *			render        { nullptr };
		bool				show_cursor   { true  };
		bool				isInit        { false };
	public:
		CFramework(const CFramework& src)				= delete;
		CFramework(CFramework&& src)					= delete;
		CFramework& operator=(const CFramework& src)	= delete;
		CFramework& operator=(CFramework&& src)			= delete;
	public:
		CFramework(){};
		~CFramework(){};
		bool Init(CRender * pRender)
		{
			if (isInit) return true;

			GetSystemInfo();

			if (pRender == nullptr) { _MBM(ERROR_PointerNone); return false; }

			window   = new CWindowMain();
			imanager = new CInputManager();
			render   = pRender;

			if (!window || !imanager)
				{ _MBM(ERROR_MEMORY); return false; }

			if (!window->Create())                  { Close(); return false; }
			if (!window->SetInputManager(imanager)) { Close(); return false; }

			hwnd  = window->GetHWND();
			state = window->GetWindowStatus();

			window->UpdateInputManager();

			if (!render->Init(hwnd, state))
				{ Close(); return false; }

			return isInit = true;
		}
		void Close()
		{
			_CLOSE(render);
			_CLOSE(imanager);
			_CLOSE(window);
			isInit = false;		
		}
		bool Run()
		{
			if (!isInit) { _MBM(ERROR_InitNone); return false; }

			while (WorkOnFrame()) { };

			return true;
		}
		
	protected:
		//>> 
		bool WorkOnFrame()
		{
			window->Proceed(); // Обработка событий окна	
		//	if (!window->IsActive()) return true;		// Свёрнутое окно не обрабатываем
			if (window->IsCloseEvent()) return false;	// Окно закрыли, выход из игры

			if (render->to_framework.WindowResetCheck()) // Рендер запрашивает обновить окно
			{	
			//	tagRECT winRc;
			//	GetWindowRect(handle, &winRc);
			//	cWindowMain->UpdateInputManager(winRc);

				int	 width  = state->sys_width;
				int	 height = state->sys_height;
				int	 pos_x  = state->position_x;
				int	 pos_y  = state->position_y;
				auto mode   = state->mode;

				if (mode == WM_FULLWINDOW || mode == WM_FULLSCREEN)
				{
					UINT params = SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW;
					width  = GetSystemMetrics(SM_CXSCREEN);
					height = GetSystemMetrics(SM_CYSCREEN);
					window->SetWindowResizable(false);
					SetWindowLongPtr(hwnd, GWL_STYLE,   GAMEFULLSCREEN);
					SetWindowLongPtr(hwnd, GWL_EXSTYLE, GAMEFULLSCREEN_EX);

					SetWindowPos(hwnd, HWND_TOP, 0, 0, width, height, params);
					window->UpdateInputManager(width, height, 0, 0, mode);
				}
				else // WINDOWED
				{
					UINT params = SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW; // | SWP_NOMOVE; // no move pos_x / pos_y
					if (mode == WM_NOBORDERS)										{
						window->SetWindowResizable(false);
						SetWindowLongPtr(hwnd, GWL_STYLE, GAMEFULLSCREEN);
						SetWindowLongPtr(hwnd, GWL_EXSTYLE, GAMEFULLSCREEN_EX);	}
					else															{
						window->SetWindowResizable(true);
						SetWindowLongPtr(hwnd, GWL_STYLE, GAMEWINDOWED);
						SetWindowLongPtr(hwnd, GWL_EXSTYLE, GAMEWINDOWED_EX);		}

					SetWindowPos(hwnd, HWND_TOP, pos_x, pos_y, width, height, params);
					window->UpdateInputManager(width, height, MISSING, MISSING, mode);
				}
				//printf("\nWindow resized by Render Call");
			}

			if (window->IsMoved()) render->Update();

			if (window->IsResized())
			{
				render->Reset(hwnd, state);

				printf("\nW %i H %i", state->sys_width, state->sys_height);
				switch (state->mode)
				{
				case WM_WINDOWED:	printf(" window mode");                 break;
				case WM_FULLWINDOW: printf(" full window with no borders"); break;
				case WM_FULLSCREEN: printf(" fullscreen mode");             break;
				case WM_NOBORDERS:  printf(" window with no borders");      break;
				}
				printf("\nX %i Y %i (render client size)", state->client_width, state->client_height);
			}

			// Проверка центирования курсора //

			if (render->to_framework.centered_cursor)
				imanager->SetCenterCursor();

			// Проверка сдвига курсора в X, Y //

			if (render->to_framework.CursorMoveCheck())
				imanager->SetCursorAt(render->to_framework.x, render->to_framework.y);

			// Проверка показа курсора //

			if ( show_cursor && !render->to_framework.show_cursor)	{ 
				show_cursor = false;
				imanager->SetShowCursor(false);						}
			else
			if (!show_cursor &&  render->to_framework.show_cursor)	{
				show_cursor = true;
				imanager->SetShowCursor(true);						}

			if (!render->Run()) return false; // выход из игры
		
			return true; // продолжение игры
		}
		
	protected:
		//>> 
		void GetSystemInfo()
		{
			SYSTEM_INFO sysinfo;

			GetNativeSystemInfo(&sysinfo);

			switch (sysinfo.wProcessorArchitecture)
			{
			case PROCESSOR_ARCHITECTURE_AMD64: system_info.processor_architecture = EPA_AMD64; break;
			case PROCESSOR_ARCHITECTURE_INTEL: system_info.processor_architecture = EPA_INTEL; break;
			case PROCESSOR_ARCHITECTURE_MIPS:  system_info.processor_architecture = EPA_MIPS;  break;
			case PROCESSOR_ARCHITECTURE_PPC:   system_info.processor_architecture = EPA_PPC;   break;
			case PROCESSOR_ARCHITECTURE_ARM:   system_info.processor_architecture = EPA_ARM;   break;
			case PROCESSOR_ARCHITECTURE_IA64:  system_info.processor_architecture = EPA_IA64;  break;
			case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64: system_info.processor_architecture = EPA_IA32; break;
			default: system_info.processor_architecture = EPA_UNKNOWN;
			}

			system_info.page_size              = sysinfo.dwPageSize;
			system_info.minimum_app_addr       = sysinfo.lpMinimumApplicationAddress;
			system_info.maximum_app_addr       = sysinfo.lpMaximumApplicationAddress;
			system_info.active_processor_mask  = (uint32)sysinfo.dwActiveProcessorMask;
			system_info.num_of_processors      = sysinfo.dwNumberOfProcessors;
			system_info.processor_type         = sysinfo.dwProcessorType;
			system_info.processor_level        = sysinfo.wProcessorLevel;
			system_info.processor_revision     = sysinfo.wProcessorRevision;
			system_info.allocation_granularity = sysinfo.dwAllocationGranularity;

			if (system_info.num_of_processors > 32)
				system_info.num_of_processors = 32;

			//system_info.Printf();
		}

	public:
		//>> Добавление ответных методов на события окна
		bool AddAnswer(CInputAnswers * pAnswers)
		{
			if (imanager)
				imanager->AddAnswer(pAnswers);
			return true;
		}
	};
}

#endif // _FRAMEWORK_H