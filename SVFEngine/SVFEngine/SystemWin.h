// ----------------------------------------------------------------------- //
//
// MODULE  : SystemBase.h
//
// PURPOSE : Сборщик сведений о системе (вариант для OS Windows)
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2019)
//
// ----------------------------------------------------------------------- //

#ifndef _SYSTEMWIN_H
#define _SYSTEMWIN_H

#include "SystemBase.h"

#pragma comment(lib, "version.lib" ) // GetOSInfo() -> GetFileVersionInfoEx()

namespace SAVFGAME
{
	#if !_WIN32 && !_WIN64
		#define PULARGE_INTEGER uint64 *
	#endif

	// Типы указателей на функции, получаемые из kernel32.dll
	typedef int           (__stdcall *pIsWow64Process)      (void * hProcess, int * Wow64Process);
	typedef unsigned long (__stdcall *pGetLogicalDrives)    (void);
	typedef int           (__stdcall *pGetDiskFreeSpaceExW) (const wchar_t *, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);

	//#if !_WIN32 && !_WIN64
	//	inline void __cpuidex(int * r, int level, int sub_level)
	//	{
	//		//asm
	//		//	("cpuid" : "=a" (r[0]), "=b" (r[1]), "=c" (r[2]), "=d" (r[3]) 
	//		//	         : "a" (level), "c" (sub_level));
	//
	//		__asm
	//		{
	//			mov    esi, r
	//			mov    eax, level
	//			mov    ecx, sub_level
	//			cpuid
	//			mov    dword ptr[esi + 0], eax
	//			mov    dword ptr[esi + 4], ebx
	//			mov    dword ptr[esi + 8], ecx
	//			mov    dword ptr[esi + 12], edx
	//		}
	//	}
	//#else

	class CSystemWin : public CSystemBase
	{
	public:
		CSystemWin() : CSystemBase() {};
		~CSystemWin() override final {};

	private:
		//>> Определение id, idEx, архитектуры AMD / Intel, CPU строки, количества ядер
		void GetMain()
		{
			std::string vendor;	// intel ? amd
			int data[4];		// cpuid out (EAX, EBX, ECX, EDX)
			int & EAX = data[0]; // data 0
			int & EBX = data[1]; // data 1
			int & ECX = data[2]; // data 2
			int & EDX = data[3]; // data 3

			// Get id

			__cpuid(data, 0);
			main.id = EAX;

			// Get intel ? amd

			{
				char text[13];
				memcpy(text + 0, & EBX, 4);
				memcpy(text + 4, & EDX, 4);
				memcpy(text + 8, & ECX, 4);
				text[12] = 0;

				vendor = text;
			}

				 if (!vendor.compare(IntelVendorStrID)) main.intel = true;
			else if (!vendor.compare(AMDVendorStrID))   main.amd   = true;
			else system_error |= eSystemError_0xF00_manufacturer;

			// Get extension id

			__cpuid(data, HCExtensionID);
			main.idEx = EAX;

			// Get brand string

			if (main.idEx >= HCExtensionID + 4)
			{
				char text[49];
				text[48] = 0;

				for (int i=0; i<3; i++)
				{
					__cpuidex(data, HCExtensionID + i + 2, 0);
					memcpy(text + i * 16, data, 16);
				}

				main.brand = text;
			}

			// Get num of CPUs

			if (main.intel)
			{
				if (main.id >= 11)
				{
					for (int lvl = 0; lvl < 4; ++lvl)
					{
						__cpuidex(data, 0x0B, lvl);
						uint32_t curlvl = (0x0000FF00 & ECX) >> 8;
						switch (curlvl)
						{
							case 1: main.nSMT   = 0x0000FFFF & EBX; break;
							case 2: main.nLogic = 0x0000FFFF & EBX; break;
						}
					}

					if (main.nLogic && main.nSMT)
						 main.nCores = main.nLogic / main.nSMT;
					else main.nCores = 0;

					if (main.nCores)
					{
						__cpuidex(data, 1, 0);
						caps.main.HTT = _BOOL(EDX & MASK_BIT_28);
					}
				}
				                     // TRY ANOTHER INFO OF nCores
				if (main.id < 11 || (main.id >= 11 && main.nCores == 0))
				{
					if (main.id >= 1) 
					{
						__cpuidex(data, 1, 0);
						main.nLogic    = (EBX >> 16) & 0xFF;
						caps.main.HTT  = _BOOL(EDX & MASK_BIT_28);

						if (main.id >= 4)
						{
							__cpuidex(data, 4, 0);
							main.nCores = 1 + ((EAX >> 26) & 0x3F);
						}
					}

					if (caps.main.HTT)
					{
						if (!(main.nCores > 1))
						{
							main.nCores = 1;
							main.nLogic = (main.nLogic >= 2) ? main.nLogic : 2;
						}
					}
					else
					{
						main.nCores = 1;
						main.nLogic = 1;
					}
				}
			}
			else if (main.amd)
			{
				if (main.id >= 1) 
				{
					__cpuidex(data, 1, 0);
					main.nLogic    = (EBX >> 16) & 0xFF;
					caps.main.HTT  = _BOOL(EDX & MASK_BIT_28);

					if (main.idEx >= 8)
					{
						__cpuidex(data, HCExtensionID + 8, 0);
						main.nCores = 1 + (ECX & 0xFF);
					}
				}
			
				if (caps.main.HTT)
				{
					if (!(main.nCores > 1))
					{
						main.nCores = 1;
						main.nLogic = (main.nLogic >= 2) ? main.nLogic : 2;
					}
				}
				else
				{
					main.nCores = 1;
					main.nLogic = 1;
				}
			}
		}

		//>> Определение возможностей железа
		void GetCaps()
		{
			int data[4]; // cpuid out (EAX, EBX, ECX, EDX)
			int & EAX = data[0];
			int & EBX = data[1];
			int & ECX = data[2];
			int & EDX = data[3];

			// Highest CPUID source operand for KNC processors is 4
			caps.main.KNC = (main.id == 4);

			if (main.id >= 1)
			{
				__cpuidex(data, 1, 0);

				caps.main.SSE3        = _BOOL(ECX & MASK_BIT_00);
				caps.other.PCLMULQDQ  = _BOOL(ECX & MASK_BIT_01);
				caps.other.MONITOR    = _BOOL(ECX & MASK_BIT_03);
				caps.main.SSSE3       = _BOOL(ECX & MASK_BIT_09);
				caps.main.FMA         = _BOOL(ECX & MASK_BIT_12);
				caps.other.CMPXCHG16B = _BOOL(ECX & MASK_BIT_13);
				caps.main.SSE41       = _BOOL(ECX & MASK_BIT_19);
				caps.main.SSE42       = _BOOL(ECX & MASK_BIT_20);
				caps.other.MOVBE      = _BOOL(ECX & MASK_BIT_22);
				caps.other.POPCNT     = _BOOL(ECX & MASK_BIT_23);
				caps.other.AES        = _BOOL(ECX & MASK_BIT_25);
				caps.other.XSAVE      = _BOOL(ECX & MASK_BIT_26);
				caps.other.OSXSAVE    = _BOOL(ECX & MASK_BIT_27);
				caps.main.AVX         = _BOOL(ECX & MASK_BIT_28);
				caps.other.F16C       = _BOOL(ECX & MASK_BIT_29);
				caps.other.RDRAND     = _BOOL(ECX & MASK_BIT_30);

				caps.other.MSR   = _BOOL(EDX & MASK_BIT_05);
				caps.other.CX8   = _BOOL(EDX & MASK_BIT_08);
				caps.other.SEP   = _BOOL(EDX & MASK_BIT_11);
				caps.other.CMOV  = _BOOL(EDX & MASK_BIT_15);
				caps.other.CLFSH = _BOOL(EDX & MASK_BIT_19);
				caps.main.MMX    = _BOOL(EDX & MASK_BIT_23);
				caps.other.FXSR  = _BOOL(EDX & MASK_BIT_24);
				caps.main.SSE    = _BOOL(EDX & MASK_BIT_25);
				caps.main.SSE2   = _BOOL(EDX & MASK_BIT_26);
				caps.main.HTT    = _BOOL(EDX & MASK_BIT_28);
			}

			if (main.id >= 7)
			{
				__cpuidex(data, 7, 0);

				caps.other.FSGSBASE    = _BOOL(EBX & MASK_BIT_00);
				caps.other.BMI1        = _BOOL(EBX & MASK_BIT_03);
				caps.other.HLE         = ((_BOOL(EBX & MASK_BIT_04)) && (main.intel));
				caps.main.AVX2         = _BOOL(EBX & MASK_BIT_05);
				caps.other.BMI2        = _BOOL(EBX & MASK_BIT_08);
				caps.other.ERMS        = _BOOL(EBX & MASK_BIT_09);
				caps.other.INVPCID     = _BOOL(EBX & MASK_BIT_10);
				caps.other.RTM         = ((_BOOL(EBX & MASK_BIT_11)) && (main.intel));
				caps.other.MPX         = _BOOL(EBX & MASK_BIT_14);
				caps.main.AVX512F      = _BOOL(EBX & MASK_BIT_16);
				caps.main.AVX512DQ     = _BOOL(EBX & MASK_BIT_17);
				caps.other.RDSEED      = _BOOL(EBX & MASK_BIT_18);
				caps.other.ADX         = _BOOL(EBX & MASK_BIT_19);
				caps.main.AVX512IFMA52 = _BOOL(EBX & MASK_BIT_21);
				caps.other.CLFLUSHOPT  = _BOOL(EBX & MASK_BIT_23);
				caps.other.CLWB        = _BOOL(EBX & MASK_BIT_24);
				caps.main.AVX512PF     = _BOOL(EBX & MASK_BIT_26);
				caps.main.AVX512ER     = _BOOL(EBX & MASK_BIT_27);
				caps.main.AVX512CD     = _BOOL(EBX & MASK_BIT_28);
				caps.other.SHA         = _BOOL(EBX & MASK_BIT_29);
				caps.main.AVX512BW     = _BOOL(EBX & MASK_BIT_30);
				caps.main.AVX512VL     = _BOOL(EBX & MASK_BIT_31);

				caps.other.PREFETCHWT1    = _BOOL(ECX & MASK_BIT_00);
				caps.main.AVX512_VBMI     = _BOOL(ECX & MASK_BIT_01);
				caps.other.WAITPKG        = _BOOL(ECX & MASK_BIT_05);
				caps.main.AVX512_VBMI2    = _BOOL(ECX & MASK_BIT_06);
				caps.other.GFNI           = _BOOL(ECX & MASK_BIT_08);
				caps.other.VAES           = _BOOL(ECX & MASK_BIT_09);
				caps.other.VPCLMULQDQ     = _BOOL(ECX & MASK_BIT_10);
				caps.main.AVX512_VNNI     = _BOOL(ECX & MASK_BIT_11);
				caps.main.AVX512_BITALG   = _BOOL(ECX & MASK_BIT_12);
				caps.main.AVX512VPOPCNTDQ = _BOOL(ECX & MASK_BIT_14);	
				caps.other.RDPID          = _BOOL(ECX & MASK_BIT_22);
				caps.other.CLDEMOTE       = _BOOL(ECX & MASK_BIT_25);
				caps.other.MOVDIRI        = _BOOL(ECX & MASK_BIT_27);
				caps.other.MOVDIR64B      = _BOOL(ECX & MASK_BIT_28);

				caps.main.AVX512_4VNNIW   = _BOOL(EDX & MASK_BIT_02);
				caps.main.AVX512_4FMAPS   = _BOOL(EDX & MASK_BIT_03);
				caps.other.PCONFIG        = _BOOL(EDX & MASK_BIT_18);		
			}

			if (main.id >= 0xd)
			{
				__cpuidex(data, 0xd, 1);

				caps.other.XSAVEOPT = _BOOL(EAX & MASK_BIT_00);
				caps.other.XSAVEC   = _BOOL(EAX & MASK_BIT_01);
				caps.other.XSS      = _BOOL(EAX & MASK_BIT_03);
			}

			if (main.idEx >= HCExtensionID + 1)
			{
				__cpuidex(data, HCExtensionID + 1, 0);

				caps.other.LAHF  = _BOOL(ECX & MASK_BIT_00);
				caps.other.LZCNT = ((_BOOL(ECX & MASK_BIT_05)) && (main.intel));
				caps.other.ABM   = ((_BOOL(ECX & MASK_BIT_05)) && (main.amd));
				caps.main.SSE4a  = ((_BOOL(ECX & MASK_BIT_06)) && (main.amd));
				caps.other.XOP   = ((_BOOL(ECX & MASK_BIT_11)) && (main.amd));
				caps.other.TBM   = ((_BOOL(ECX & MASK_BIT_21)) && (main.amd));

				caps.other.SYSCALL   = ((_BOOL(EDX & MASK_BIT_11)) && (main.intel));
				caps.other.MMXEXT    = ((_BOOL(EDX & MASK_BIT_22)) && (main.amd));
				caps.other.RDTSCP    = ((_BOOL(EDX & MASK_BIT_27)) && (main.intel));
				caps.other._3DNOWEXT = ((_BOOL(EDX & MASK_BIT_30)) && (main.amd));
				caps.other._3DNOW    = ((_BOOL(EDX & MASK_BIT_31)) && (main.amd));
			}

			if (main.idEx >= HCExtensionID + 8)
			{
				__cpuidex(data, HCExtensionID + 8, 0);

				caps.other.WBNOINVD = _BOOL(EBX & MASK_BIT_09);
			}

			if (main.idEx >= HCExtensionID + 0xA)
			{
				__cpuidex(data, HCExtensionID + 0xA, 0);

				caps.main.SVML = _BOOL(EDX & MASK_BIT_02); // && main.amd ?  для интела тут же или иначе?
			}		
		}

		//>> Получение другой информации о системе
		void GetSystemInfo()
		{
			SYSTEM_INFO sysinfo;

			GetNativeSystemInfo(&sysinfo);

			system_info.processor_architecture_raw = sysinfo.wProcessorArchitecture;
			system_info.processor_type_raw         = sysinfo.dwProcessorType;

			switch (sysinfo.wProcessorArchitecture)
			{
			case PROCESSOR_ARCHITECTURE_INTEL: system_info.processor_architecture = EPA_INTEL; break;
			case PROCESSOR_ARCHITECTURE_AMD64: system_info.processor_architecture = EPA_AMD64; break;
			case PROCESSOR_ARCHITECTURE_MIPS:  system_info.processor_architecture = EPA_MIPS;  break;
			case PROCESSOR_ARCHITECTURE_PPC:   system_info.processor_architecture = EPA_PPC;   break;
			case PROCESSOR_ARCHITECTURE_ARM:   system_info.processor_architecture = EPA_ARM;   break;
			case PROCESSOR_ARCHITECTURE_IA64:  system_info.processor_architecture = EPA_IA64;  break;
			case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64: system_info.processor_architecture = EPA_IA32; break;
			case PROCESSOR_ARCHITECTURE_NEUTRAL:
			case PROCESSOR_ARCHITECTURE_UNKNOWN:
			default: system_info.processor_architecture = EPA_UNKNOWN;
			}

			switch (sysinfo.dwProcessorType)
			{
			case PROCESSOR_INTEL_386:     system_info.processor_type = EPAT_INTEL_386;     break;
			case PROCESSOR_INTEL_486:     system_info.processor_type = EPAT_INTEL_486;     break;
			case PROCESSOR_INTEL_PENTIUM: system_info.processor_type = EPAT_INTEL_PENTIUM; break;
			case PROCESSOR_INTEL_IA64:    system_info.processor_type = EPAT_INTEL_IA64;    break;
			case PROCESSOR_AMD_X8664:     system_info.processor_type = EPAT_AMD_X8664;     break;
			case PROCESSOR_MIPS_R4000:    system_info.processor_type = EPAT_MIPS_R4000;    break;
			case PROCESSOR_ALPHA_21064:   system_info.processor_type = EPAT_ALPHA_21064;   break;
			case PROCESSOR_PPC_601:       system_info.processor_type = EPAT_PPC_601;       break;
			case PROCESSOR_PPC_603:       system_info.processor_type = EPAT_PPC_603;       break;
			case PROCESSOR_PPC_604:       system_info.processor_type = EPAT_PPC_604;       break;
			case PROCESSOR_PPC_620:       system_info.processor_type = EPAT_PPC_620;       break;
			case PROCESSOR_HITACHI_SH3:   system_info.processor_type = EPAT_HITACHI_SH3;   break;
			case PROCESSOR_HITACHI_SH3E:  system_info.processor_type = EPAT_HITACHI_SH3E;  break;
			case PROCESSOR_HITACHI_SH4:   system_info.processor_type = EPAT_HITACHI_SH4;   break;
			case PROCESSOR_MOTOROLA_821:  system_info.processor_type = EPAT_MOTOROLA_821;  break;
			case PROCESSOR_SHx_SH3:       system_info.processor_type = EPAT_SHx_SH3;       break;
			case PROCESSOR_SHx_SH4:       system_info.processor_type = EPAT_SHx_SH4;       break;
			case PROCESSOR_STRONGARM:     system_info.processor_type = EPAT_STRONGARM;     break;
			case PROCESSOR_ARM720:        system_info.processor_type = EPAT_ARM720;        break;
			case PROCESSOR_ARM820:        system_info.processor_type = EPAT_ARM820;        break;
			case PROCESSOR_ARM920:        system_info.processor_type = EPAT_ARM920;        break;
			case PROCESSOR_ARM_7TDMI:     system_info.processor_type = EPAT_ARM_7TDMI;     break;
			case PROCESSOR_OPTIL:         system_info.processor_type = EPAT_OPTIL;         break;
			default: system_info.processor_type = EPAT_UNKNOWN;
			}

			system_info.page_size              = sysinfo.dwPageSize;
			system_info.minimum_app_addr       = sysinfo.lpMinimumApplicationAddress;
			system_info.maximum_app_addr       = sysinfo.lpMaximumApplicationAddress;
			system_info.active_processor_mask  = (uint32)sysinfo.dwActiveProcessorMask;
			system_info.num_of_processors      = sysinfo.dwNumberOfProcessors;
			system_info.processor_level        = sysinfo.wProcessorLevel;
			system_info.processor_revision     = sysinfo.wProcessorRevision;
			system_info.allocation_granularity = sysinfo.dwAllocationGranularity;

			if (system_info.num_of_processors > 32)
				system_info.num_of_processors = 32;

			ULONGLONG sysmem;
			if (GetPhysicallyInstalledSystemMemory(&sysmem))
				 system_info.system_memory = (uint64)sysmem;
			else system_error |= eSystemError_0xF000_RAM;
		}

		//>> Получение информации об ОС
		void GetOSInfo()
		{
			// Get OS version

			wchar_t sysdir[MAX_PATH];
			byte * bufferEx = nullptr;

			if (GetSystemDirectory(sysdir, MAX_PATH))
			{
				system_os.sysdir = sysdir;
				wcscat_s(sysdir, L"\\kernel32.dll");

				DWORD handle;
				DWORD sizeEx = GetFileVersionInfoSizeEx(FILE_VER_GET_NEUTRAL, sysdir, &handle);

				if (sizeEx)
				{
					bufferEx = (byte*) malloc(sizeEx);
					if (bufferEx != nullptr)
					{
						if (GetFileVersionInfoEx(FILE_VER_GET_NEUTRAL, sysdir, 0, sizeEx, bufferEx))
						{
							VS_FIXEDFILEINFO * fileInfo = nullptr;
							UINT infoSize;

							if (VerQueryValue(bufferEx, L"\\", (void**)&fileInfo, &infoSize))
							{
								if (infoSize) { 
									system_os.version = fileInfo->dwProductVersionMS;
			} else system_error |= eSystemError_0xF_VerQueryValueInfoSize;
			} else system_error |= eSystemError_0xF_VerQueryValue;
			} else system_error |= eSystemError_0xF_GetFileVersionInfoEx;
			} else system_error |= eSystemError_0xF_mallocBufferEx;
			} else system_error |= eSystemError_0xF_GetFileVersionInfoSizeEx;
			} else system_error |= eSystemError_0xF_GetSystemDirectory;

			if (bufferEx != nullptr) free(bufferEx);

			if ((system_error & eSystemError_0xF) == eSystemError_NONE)
			{
				system_os._windows._5_0_Win2000      = (system_os.version == 0x00050000);
				system_os._windows._5_1_WinXP        = (system_os.version == 0x00050001);
				system_os._windows._5_2_WinXPx64     = (system_os.version == 0x00050002);
				system_os._windows._6_0_WinVista     = (system_os.version == 0x00060000);
				system_os._windows._6_1_Win7         = (system_os.version == 0x00060001);
				system_os._windows._6_2_Win8         = (system_os.version == 0x00060002); 
				system_os._windows._6_3_Win81        = (system_os.version == 0x00060003);
				system_os._windows._10_0_Win10       = (system_os.version == 0x000a0000);

				system_os._windows.is_Win2000_or_greater  = (system_os.version >= 0x00050000);
				system_os._windows.is_WinXP_or_greater    = (system_os.version >= 0x00050001);
				system_os._windows.is_WinVista_or_greater = (system_os.version >= 0x00060000);
				system_os._windows.is_Win7_or_greater     = (system_os.version >= 0x00060001);
				system_os._windows.is_Win8_or_greater     = (system_os.version >= 0x00060002);
				system_os._windows.is_Win81_or_greater    = (system_os.version >= 0x00060003);
				system_os._windows.is_Win10_or_greater    = (system_os.version >= 0x000a0000);

				system_os.UpdateType();
			}

			// Check OS x32 or x64
			// Check using WOW64
		
			if (SVFENGINEx64)
			{
				system_os.engine3264 = false; // engine 64-bit : WOW64 is false
				system_os.system64   = true;  // system 64-bit : only 64-bit OS can run 64-bit build
			}
			else // engine build is x32, so (WOW64==true) if running x32 build on x64 OS
			{
				// IsWow64Process is not available on all supported versions of Windows ---> so let's check it
				HMODULE handle = GetModuleHandle(L"kernel32");
				if (handle)
				{
					pIsWow64Process addr = (pIsWow64Process) GetProcAddress(handle, "IsWow64Process");
					if (addr)
					{
						//If the process is running under 32-bit Windows, the value is set to FALSE.
						//If the process is a 32-bit application running under 64-bit Windows 10 on ARM, the value is set to FALSE.
						//If the process is a 64-bit application running under 64-bit Windows, the value is also set to FALSE.

						int WOW64;

						//if (IsWow64Process(GetCurrentProcess(), &WOW64))
						if ((addr)(GetCurrentProcess(), &WOW64))
						{
							system_os.engine3264 = _BOOL(WOW64);
							system_os.system64   = _BOOL(WOW64);
						}
						else
							system_error |= eSystemError_0xF0_IsWow64;
					}
					else // can't find function IsWow64Process(), so WOW64 is false
					{
						system_os.engine3264 = false; // engine 32-bit build
						system_os.system64   = false; // system 32-bit
					}
				}
				else // can't find kernel32.dll
					system_error |= eSystemError_0xF0_kernel32;
			}
		}

		//>> Получение информации о дисках
		void GetDisksInfo()
		{
			// Получить имена дисков можно и через GetLogicalDriveStrings()

			HMODULE handle = GetModuleHandle(L"kernel32");
			if (handle)
			{
				// Get disk drives and disk types
				{
					pGetLogicalDrives addr = (pGetLogicalDrives) GetProcAddress(handle, "GetLogicalDrives");
					if (addr)
					{
						unsigned long mask = (addr)();
						if (mask)
						{
							uint32 iDrive = 0;
							while (mask)
							{
								if (iDrive == eSystemDiskName_ENUM_MAX)
									break;
								if (mask & 1)
								{
									auto & disk = system_disk.disk[iDrive];
									disk.presented = true;
									uint32 dtype = GetDriveType(disk.root); // считаем, что функция есть
									switch (dtype)
									{
									case DRIVE_FIXED:       disk.type = eSystemDiskFixed;      break;
									case DRIVE_REMOVABLE:   disk.type = eSystemDiskRemovable;  break;
									case DRIVE_CDROM:       disk.type = eSystemDiskCDROM;      break;
									case DRIVE_REMOTE:      disk.type = eSystemDiskRemote;     break;
									case DRIVE_RAMDISK:     disk.type = eSystemDiskRAM;        break;
									case DRIVE_UNKNOWN:
									case DRIVE_NO_ROOT_DIR:
									default: disk.type = eSystemDiskUnknown;
									}
								}
								iDrive++;
								mask >>= 1;					
							}

							bool error = true;
							for (int i = 0; i < eSystemDiskName_ENUM_MAX; i++)
							if (system_disk.disk[i].presented)
							if (system_disk.disk[i].type == eSystemDiskFixed)
								{ error = false; break; }

							if (error) // no fixed disks ? something going wrong...
								system_error |= eSystemError_0xF00000_NoFixedDisks;
						}
						else // failed GetLogicalDrives()
							system_error |= eSystemError_0xF00000_GetLogicalDrives;
					}
					else // can't find GetLogicalDrives()
						system_error |= eSystemError_0xF00000_pGetLogicalDrives;
				}

				// Get disks' space (total and free)
				if ((system_error & eSystemError_0xF00000) == 0) // if no errors before...
				{
					ULARGE_INTEGER FreeBytesToCaller;
					ULARGE_INTEGER TotalBytes;
					ULARGE_INTEGER FreeBytes;

					pGetDiskFreeSpaceExW addr = (pGetDiskFreeSpaceExW) GetProcAddress(handle, "GetDiskFreeSpaceExW");
					if (addr)
					{
						for (int i = 0; i < eSystemDiskName_ENUM_MAX; i++)
						if (system_disk.disk[i].type == eSystemDiskFixed)
						{
							auto & disk = system_disk.disk[i];
							if ((addr)(disk.root, & FreeBytesToCaller, & TotalBytes, & FreeBytes))
							{
								disk.size_free_to_caller = FreeBytesToCaller.QuadPart;
								disk.size_free           = FreeBytes.QuadPart;
								disk.size_total          = TotalBytes.QuadPart;
							}
							else // failed GetDiskFreeSpaceExW()
							{
								system_error |= eSystemError_0xF00000_GetDiskFreeSpaceEx;
								break;
							}
						}

						system_disk.updater = 0;
					}
					else // считаем, что GetDiskFreeSpace есть всегда
					{
						unsigned long dwSectPerClust;
						unsigned long dwBytesPerSect;
						unsigned long dwFreeClusters;
						unsigned long dwTotalClusters;

						for (int i = 0; i < eSystemDiskName_ENUM_MAX; i++)
						if (system_disk.disk[i].type == eSystemDiskFixed)
						{
							auto & disk = system_disk.disk[i];

							GetDiskFreeSpace ( disk.root,
								& dwSectPerClust,
								& dwBytesPerSect,
								& dwFreeClusters,
								& dwTotalClusters );

							disk.size_total = (uint64) ((int64)dwTotalClusters * dwSectPerClust * dwBytesPerSect);
							disk.size_free  = (uint64) ((int64)dwFreeClusters  * dwSectPerClust * dwBytesPerSect);
							disk.size_free_to_caller = disk.size_free;
						}

						system_disk.updater = 1;
					}
				}
			}
			else // can't find kernel32.dll
				system_error |= eSystemError_0xF00000_kernel32;
		}

		//>> Обновление данных (занятая сис.память, свободное место на ЖД и проч.)
		void GetUpdate()
		{
			system_error = eSystemError_NONE;

			MEMORYSTATUSEX mem_state;
			mem_state.dwLength = (decltype(mem_state.dwLength)) sizeof(mem_state);
		
			if (GlobalMemoryStatusEx(&mem_state))
			{
				system_memory.percent_in_use = (uint32) mem_state.dwMemoryLoad;
				system_memory.total_phys     = (uint64) mem_state.ullTotalPhys;
				system_memory.avail_phys     = (uint64) mem_state.ullAvailPhys;
				system_memory.total_page     = (uint64) mem_state.ullTotalPageFile;
				system_memory.avail_page     = (uint64) mem_state.ullAvailPageFile;
				system_memory.total_virt     = (uint64) mem_state.ullTotalVirtual;
				system_memory.avail_virt     = (uint64) mem_state.ullAvailVirtual;
				system_memory.avail_virt_ext = (uint64) mem_state.ullAvailExtendedVirtual;
			}
			else system_error |= eSystemError_0xF0000_MemStatus;

			if (system_disk.updater == 0) // GetDiskFreeSpaceEx()
			{
				ULARGE_INTEGER FreeBytesToCaller;
				ULARGE_INTEGER TotalBytes;
				ULARGE_INTEGER FreeBytes;

				for (int i = 0; i < eSystemDiskName_ENUM_MAX; i++)
				if (system_disk.disk[i].type == eSystemDiskFixed)
				{
					auto & disk = system_disk.disk[i];
					if (GetDiskFreeSpaceEx(disk.root, & FreeBytesToCaller, & TotalBytes, & FreeBytes))
					{
						disk.size_free_to_caller = FreeBytesToCaller.QuadPart;
						disk.size_free           = FreeBytes.QuadPart;
						disk.size_total          = TotalBytes.QuadPart;
					}
					else // failed GetDiskFreeSpaceExW()
					{
						system_error |= eSystemError_0xF00000_GetDiskFreeSpaceEx;
						break;
					}
				}
			}
			else // GetDiskFreeSpace()
			{
				unsigned long dwSectPerClust;
				unsigned long dwBytesPerSect;
				unsigned long dwFreeClusters;
				unsigned long dwTotalClusters;

				for (int i = 0; i < eSystemDiskName_ENUM_MAX; i++)
				if (system_disk.disk[i].type == eSystemDiskFixed)
				{
					auto & disk = system_disk.disk[i];

					GetDiskFreeSpace ( disk.root,
						& dwSectPerClust,
						& dwBytesPerSect,
						& dwFreeClusters,
						& dwTotalClusters );

					disk.size_total = (uint64) ((int64)dwTotalClusters * dwSectPerClust * dwBytesPerSect);
					disk.size_free  = (uint64) ((int64)dwFreeClusters  * dwSectPerClust * dwBytesPerSect);
					disk.size_free_to_caller = disk.size_free;
				}
			}

			update_time = _TIME;
		}

		//>> Обновление данных и возврат заказанного значения
		uint64 Update(eSystemRet type, uint64 extra_input) override final
		{
			uint64 ret = MISSING;

			{
				glock others_will_wait (m_Update); // изолируем вызов

				if (_TIMER(update_time) >= SYSUPDATEDTIME)
					GetUpdate();

				switch (type)
				{
				case eSystemRet_RAM_available:
					ret = system_memory.avail_phys;
					break;
				case eSystemRet_HDD_free:
					if (extra_input < eSystemDiskName_ENUM_MAX)
						ret = system_disk.disk[extra_input].size_free_to_caller;
					break;
				}
			}

			return ret;
		}

		//>> Первичная загрузка информации :: возвращает коды ошибок eSystemError
		uint32 Init()
		{
			if (!main.isInit)
			{
				implementation = eSystemClassWin;

				GetMain();
				GetCaps();
				GetSystemInfo();
				GetOSInfo();
				GetDisksInfo();
				GetUpdate();

				if (system_error == eSystemError_NONE)
					main.isInit = true;

				return system_error;
			}

			return eSystemError_0xF0000000_InitAlready;
		}
	};
}

#endif