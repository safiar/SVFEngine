// ----------------------------------------------------------------------- //
//
// MODULE  : system.h
//
// PURPOSE : Сообщает информацию об OS, HDD, RAM и CPU
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2019)
//
// ----------------------------------------------------------------------- //

#ifndef _INFO_SYSTEM_H
#define _INFO_SYSTEM_H

#include "defines_core.h"

#if _OS_WINDOWS
	#include "SystemWin.h"
	#define CSystemF CSystemWin
#endif

#define SYS_ALLOC_ERROR 0xFFFFFFFF // CSystemBase * sys = nullptr 

namespace SAVFGAME
{
	//>> Информация о системе
	class CSystem
	{
		virtual void __DO_NOT_CREATE_OBJECT_OF_THIS_CLASS() = 0;

	private:
		static CSystemBase * sys; // источник данных

		//>> Сообщает о возможности CPU : (ret > 0) 1 да, (ret == 0) 0 нет, (ret < 0) -1 MISSING (debug)
		static char Get(eCPUCaps CPUCap);

	public:
		//>> ! ! !  Первичная загрузка информации ! ! ! возвращает коды ошибок eSystemError
		static uint32 Init();

		//>> Освобождение занятых ресурсов и сброс данных
		static void Release();
		
		//>> Сообщает заказанную информацию
		static uint64 Get(eSystemRet type, uint64 extra_input);

		//>> Сообщает комплексный (итоговый) результат возможности использования <CPUCap>
		static bool GetCap(eCPUCaps CPUCap);

		//>> [DEBUG] Вывод общей информации в консоль
		static void Printf();

		//>> [DEBUG] Вывод ошибок в консоль
		static void Printf(uint32 syserr);
	};
}

#endif // _INFO_SYSTEM_H