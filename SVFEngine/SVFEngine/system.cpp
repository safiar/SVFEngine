// ----------------------------------------------------------------------- //
//
// MODULE  : system.cpp
//
// PURPOSE : Сообщает информацию об OS, HDD, RAM и CPU
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2019)
//
// ----------------------------------------------------------------------- //

#include "system.h"

namespace SAVFGAME
{
	CSystemBase * CSystem::sys = nullptr; // источник данных

	//>> Сообщает о возможности CPU : (ret > 0) 1 да, (ret == 0) 0 нет, (ret < 0) -1 MISSING (debug)
	char CSystem::Get(eCPUCaps CPUCap)
	{
		// данные никогда не меняются, можно не изолировать вызов на чтение

		char ret = MISSING;

		if (sys != nullptr)
		if (sys->main.isInit)
		switch (CPUCap)
		{
		case eCPUCap_HTT:	          ret = sys->caps.main.HTT;              break;
		case eCPUCap_SSE:             ret = sys->caps.main.SSE;              break;
		case eCPUCap_SSE2:            ret = sys->caps.main.SSE2;             break;
		case eCPUCap_SSE3:            ret = sys->caps.main.SSE3;             break;
		case eCPUCap_SSSE3:           ret = sys->caps.main.SSSE3;            break;
		case eCPUCap_SSE41:           ret = sys->caps.main.SSE41;            break;
		case eCPUCap_SSE42:           ret = sys->caps.main.SSE42;            break;
		case eCPUCap_SSE4a:           ret = sys->caps.main.SSE4a;            break;
		case eCPUCap_MMX:             ret = sys->caps.main.MMX;              break;
		case eCPUCap_AVX:             ret = sys->caps.main.AVX;              break;
		case eCPUCap_AVX2:            ret = sys->caps.main.AVX2;             break;
		case eCPUCap_AVX512F:         ret = sys->caps.main.AVX512F;          break;
		case eCPUCap_AVX512BW:        ret = sys->caps.main.AVX512BW;         break;
		case eCPUCap_AVX512CD:        ret = sys->caps.main.AVX512CD;         break;
		case eCPUCap_AVX512DQ:        ret = sys->caps.main.AVX512DQ;         break;
		case eCPUCap_AVX512ER:        ret = sys->caps.main.AVX512ER;         break;
		case eCPUCap_AVX512IFMA52:    ret = sys->caps.main.AVX512IFMA52;     break;
		case eCPUCap_AVX512PF:        ret = sys->caps.main.AVX512PF;         break;
		case eCPUCap_AVX512VL:        ret = sys->caps.main.AVX512VL;         break;
		case eCPUCap_AVX512VPOPCNTDQ: ret = sys->caps.main.AVX512VPOPCNTDQ;  break;
		case eCPUCap_AVX512_4FMAPS:   ret = sys->caps.main.AVX512_4FMAPS;    break;
		case eCPUCap_AVX512_4VNNIW:   ret = sys->caps.main.AVX512_4VNNIW;    break;
		case eCPUCap_AVX512_BITALG:   ret = sys->caps.main.AVX512_BITALG;    break;
		case eCPUCap_AVX512_VBMI:     ret = sys->caps.main.AVX512_VBMI;      break;
		case eCPUCap_AVX512_VBMI2:    ret = sys->caps.main.AVX512_VBMI2;     break;
		case eCPUCap_AVX512_VNNI:     ret = sys->caps.main.AVX512_VNNI;      break;
		case eCPUCap_FMA:             ret = sys->caps.main.FMA;              break;
		case eCPUCap_KNC:             ret = sys->caps.main.KNC;              break;
		case eCPUCap_SVML:            ret = sys->caps.main.SVML;             break;

		///////////////////////////////////////////////

		case eCPUCap_ADX:         ret = sys->caps.other.ADX;         break;
		case eCPUCap_AES:         ret = sys->caps.other.AES;         break;
		case eCPUCap_BMI1:        ret = sys->caps.other.BMI1;        break;
		case eCPUCap_BMI2:        ret = sys->caps.other.BMI2;        break;
		case eCPUCap_CLDEMOTE:    ret = sys->caps.other.CLDEMOTE;    break;
		case eCPUCap_CLFLUSHOPT:  ret = sys->caps.other.CLFLUSHOPT;  break;
		case eCPUCap_CLWB:        ret = sys->caps.other.CLWB;        break;
		case eCPUCap_F16C:        ret = sys->caps.other.F16C;        break;
		case eCPUCap_FSGSBASE:    ret = sys->caps.other.FSGSBASE;    break;
		case eCPUCap_FXSR:        ret = sys->caps.other.FXSR;        break;
		case eCPUCap_GFNI:        ret = sys->caps.other.GFNI;        break;
		case eCPUCap_INVPCID:     ret = sys->caps.other.INVPCID;     break;
		case eCPUCap_LZCNT:       ret = sys->caps.other.LZCNT;       break;
		case eCPUCap_MONITOR:     ret = sys->caps.other.MONITOR;     break;
		case eCPUCap_MOVBE:       ret = sys->caps.other.MOVBE;       break;
		case eCPUCap_MOVDIR64B:   ret = sys->caps.other.MOVDIR64B;   break;
		case eCPUCap_MOVDIRI:     ret = sys->caps.other.MOVDIRI;     break;
		case eCPUCap_MPX:         ret = sys->caps.other.MPX;         break;
		case eCPUCap_PCLMULQDQ:   ret = sys->caps.other.PCLMULQDQ;   break;
		case eCPUCap_PCONFIG:     ret = sys->caps.other.PCONFIG;     break;
		case eCPUCap_POPCNT:      ret = sys->caps.other.POPCNT;      break;
		case eCPUCap_PREFETCHWT1: ret = sys->caps.other.PREFETCHWT1; break;
		case eCPUCap_RDPID:       ret = sys->caps.other.RDPID;       break;
		case eCPUCap_RDRAND:      ret = sys->caps.other.RDRAND;      break;
		case eCPUCap_RDSEED:      ret = sys->caps.other.RDSEED;      break;
		case eCPUCap_RDTSCP:      ret = sys->caps.other.RDTSCP;      break;
		case eCPUCap_RTM:         ret = sys->caps.other.RTM;         break;
		case eCPUCap_SHA:         ret = sys->caps.other.SHA;         break;
	//	case eCPUCap_TSC:         ret = sys->caps.other.TSC;         break;
		case eCPUCap_VAES:        ret = sys->caps.other.VAES;        break;
		case eCPUCap_VPCLMULQDQ:  ret = sys->caps.other.VPCLMULQDQ;  break;
		case eCPUCap_WAITPKG:     ret = sys->caps.other.WAITPKG;     break;
		case eCPUCap_WBNOINVD:    ret = sys->caps.other.WBNOINVD;    break;
		case eCPUCap_XSAVE:       ret = sys->caps.other.XSAVE;       break;
		case eCPUCap_XSAVEC:      ret = sys->caps.other.XSAVEC;      break;
		case eCPUCap_XSAVEOPT:    ret = sys->caps.other.XSAVEOPT;    break;
		case eCPUCap_XSS:         ret = sys->caps.other.XSS;         break;

		///////////////////////////////////////////////
		
		case eCPUCap_ABM:        ret = sys->caps.other.ABM;        break;
		case eCPUCap_CLFSH:      ret = sys->caps.other.CLFSH;      break;
		case eCPUCap_CMOV:       ret = sys->caps.other.CMOV;       break;
		case eCPUCap_CMPXCHG16B: ret = sys->caps.other.CMPXCHG16B; break;
		case eCPUCap_CX8:        ret = sys->caps.other.CX8;        break;
		case eCPUCap_ERMS:       ret = sys->caps.other.ERMS;       break;
		case eCPUCap_HLE:        ret = sys->caps.other.HLE;        break;
		case eCPUCap_LAHF:       ret = sys->caps.other.LAHF;       break;
		case eCPUCap_MMXEXT:     ret = sys->caps.other.MMXEXT;     break;
		case eCPUCap_MSR:        ret = sys->caps.other.MSR;        break;
		case eCPUCap_OSXSAVE:    ret = sys->caps.other.OSXSAVE;    break;
		case eCPUCap_SEP:        ret = sys->caps.other.SEP;        break;
		case eCPUCap_SYSCALL:    ret = sys->caps.other.SYSCALL;    break;
		case eCPUCap_TBM:        ret = sys->caps.other.TBM;        break;
		case eCPUCap_XOP:        ret = sys->caps.other.XOP;        break;
		case eCPUCap__3DNOWEXT:  ret = sys->caps.other._3DNOWEXT;  break;
		case eCPUCap__3DNOW:     ret = sys->caps.other._3DNOW;     break;
		}

		return ret;
	}

	/////////////////////////////////////

	//>> ! ! !  Первичная загрузка информации ! ! ! возвращает коды ошибок eSystemError
	uint32 CSystem::Init()
	{
		if (sys == nullptr)
		{
			sys = new CSystemF;

			if (sys == nullptr)
				return SYS_ALLOC_ERROR;
		}

		return sys->Init();
	}

	//>> Освобождение занятых ресурсов и сброс данных
	void   CSystem::Release()
	{
		_DELETE(sys);
	}

	//>> Сообщает заказанную информацию :: ret MISSING if error
	uint64 CSystem::Get(eSystemRet type, uint64 extra_input)
	{
		uint64 ret = MISSING;

		if (sys != nullptr)
		if (sys->main.isInit)
		switch (type)
		{
		case eSystemRet_RAM_installed: ret = sys->system_info.system_memory * 1024; break;
		case eSystemRet_RAM_available: ret = sys->Update(type, 0);                  break;
		case eSystemRet_OS_64_bit:     ret = sys->system_os.system64;               break;
		case eSystemRet_OS_type:       ret = sys->system_os.type;                   break;
		case eSystemRet_HDD_free:
			{			
				if (extra_input >= (uint64)((wchar_t)'A') &&
					extra_input <= (uint64)((wchar_t)'Z'))
				{
					wchar_t chDisk = (wchar_t) extra_input;
					int iDisk = sys->system_disk.GetDisk(chDisk);
					if (sys->system_disk.disk[iDisk].type == eSystemDiskFixed)
					{
						ret = sys->Update(type, iDisk);
					}
				}
				break;
			}
		}

		return ret;
	}

	//>> Сообщает комплексный (итоговый) результат возможности использования <CPUCap>
	bool   CSystem::GetCap(eCPUCaps CPUCap)
	{
		// данные никогда не меняются, можно не изолировать вызов на чтение

		bool ret = false;

		if (sys != nullptr)
		if (sys->main.isInit)
		if (SVFENGINEx64) // && system_os.system64
		{
			switch (CPUCap)
			{
			case eCPUCap_SSE:     ret = sys->caps.main.SSE     && ENGINE_SSE_ENABLED;     break;
			case eCPUCap_SSE2:    ret = sys->caps.main.SSE2    && ENGINE_SSE2_ENABLED;    break;
			case eCPUCap_SSE3:    ret = sys->caps.main.SSE3    && ENGINE_SSE3_ENABLED;    break;
			case eCPUCap_SSSE3:   ret = sys->caps.main.SSSE3   && ENGINE_SSSE3_ENABLED;   break;
			case eCPUCap_SSE41:   ret = sys->caps.main.SSE41   && ENGINE_SSE41_ENABLED;   break;
			case eCPUCap_SSE42:   ret = sys->caps.main.SSE42   && ENGINE_SSE42_ENABLED;   break;
		//	case eCPUCap_SSE4a:                                                           break;
			case eCPUCap_AVX:     ret = sys->caps.main.AVX     && ENGINE_AVX_ENABLED;     break;
			case eCPUCap_AVX2:    ret = sys->caps.main.AVX2    && ENGINE_AVX2_ENABLED;    break;
			case eCPUCap_AVX512F: ret = sys->caps.main.AVX512F && ENGINE_AVX512F_ENABLED; break;
			}
		}

		return ret;
	}

	//>> [DEBUG] Вывод информации в консоль
	void   CSystem::Printf()
	{	
		if (sys == nullptr)
		{
			printf("\nSYS IS MISSING");
			return;
		}

		printf("\nSYS_ERR_VALUE = 0x%08X", sys->system_error);
		printf("\nENGINE BUILD  = %s\n", SVFENGINEx64 ? "x64" : "x32");

		sys->system_info.Printf();
		sys->system_memory.Printf();
		sys->system_os.Printf();
		sys->system_disk.Printf();
		sys->main.Printf();
		sys->caps.Printf();

		//	for (uint32 i = 0; i < eCPUCap___ENUM_MAX; i++)
		//		printf("\neCPUCap %i = %i", i, Get((eCPUCaps)i));
	}

	//>> [DEBUG] Вывод ошибок в консоль
	void   CSystem::Printf(uint32 syserr)
	{
		if (syserr == SYS_ALLOC_ERROR || sys == nullptr)
			printf("\nSystem error : using pure virtual class (memory not allocated)");
		else if (syserr == 0)
			printf("\nSystem error : NONE");
		else if (sys->implementation == eSystemClassWin)
		{
			uint32 xF0000000 = syserr & eSystemError_0xF0000000;
		//	uint32 x0F000000 = syserr & eSystemError_0xF000000;
			uint32 x00F00000 = syserr & eSystemError_0xF00000;
			uint32 x000F0000 = syserr & eSystemError_0xF0000;
			uint32 x0000F000 = syserr & eSystemError_0xF000;
			uint32 x00000F00 = syserr & eSystemError_0xF00;
			uint32 x000000F0 = syserr & eSystemError_0xF0;
			uint32 x0000000F = syserr & eSystemError_0xF;

			switch (xF0000000)
			{
			case eSystemError_0xF0000000_InitAlready:
				printf("\nSystem error : sys data already inited"); break;
			}

		//	switch (x0F000000)
		//	{
		//	}

			switch (x00F00000)
			{
			case eSystemError_0xF00000_kernel32:
				printf("\nSystem error : Get disks error - failed to find kernel32.dll"); break;
			case eSystemError_0xF00000_pGetLogicalDrives:
				printf("\nSystem error : Get disks error - failed to find ptr GetLogicalDrives()"); break;
			case eSystemError_0xF00000_GetLogicalDrives:
				printf("\nSystem error : Get disks error - failed GetLogicalDrives()"); break;
			case eSystemError_0xF00000_NoFixedDisks:
				printf("\nSystem error : Get disks error - can't find any fixed disk"); break;
			case eSystemError_0xF00000_GetDiskFreeSpaceEx:
				printf("\nSystem error : Get disks error - failed GetDiskFreeSpaceExW()"); break;
			}

			switch (x000F0000)
			{
			case eSystemError_0xF0000_MemStatus:
				printf("\nSystem error : Get update error - failed GlobalMemoryStatusEx()"); break;
			}

			switch (x0000F000)
			{
			case eSystemError_0xF000_RAM:
				printf("\nSystem error : Get sys info error - failed GetPhysicallyInstalledSystemMemory()"); break;
			}

			switch (x00000F00)
			{
			case eSystemError_0xF00_manufacturer:
				printf("\nSystem error : Get main CPU info error - unknown manufacturer (not AMD/Intel)"); break;
			}

			switch (x000000F0)
			{
			case eSystemError_0xF0_kernel32:
				printf("\nSystem error : Get OS x32/x64 bit error - failed to find kernel32.dll"); break;
			case eSystemError_0xF0_IsWow64:
				printf("\nSystem error : Get OS x32/x64 bit error - failed IsWow64Process()"); break;
			}

			switch (x0000000F)
			{
			case eSystemError_0xF_GetSystemDirectory:
				printf("\nSystem error : Get OS version error - failed GetSystemDirectory()"); break;
			case eSystemError_0xF_GetFileVersionInfoSizeEx:
				printf("\nSystem error : Get OS version error - failed GetFileVersionInfoSizeEx()"); break;
			case eSystemError_0xF_mallocBufferEx:
				printf("\nSystem error : Get OS version error - failed malloc() memory"); break;
			case eSystemError_0xF_GetFileVersionInfoEx:
				printf("\nSystem error : Get OS version error - failed GetFileVersionInfoEx()"); break;
			case eSystemError_0xF_VerQueryValue:
				printf("\nSystem error : Get OS version error - failed VerQueryValue()"); break;
			case eSystemError_0xF_VerQueryValueInfoSize:
				printf("\nSystem error : Get OS version error - info size is NULL"); break;
			}
		}
	}

}