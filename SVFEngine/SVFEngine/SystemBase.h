// ----------------------------------------------------------------------- //
//
// MODULE  : SystemBase.h
//
// PURPOSE : Прототип класса сборщика сведений о системе
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2019)
//
// ----------------------------------------------------------------------- //

#ifndef _SYSTEMBASE_H
#define _SYSTEMBASE_H

#include "types.h"
#include "defines.h"
#include "thread.h"

#define IntelVendorStrID     "GenuineIntel" // Intel
#define AMDVendorStrID       "AuthenticAMD" // AMD
#define CyrixVendorStrID     "CyrixInstead" // Cyrix
#define CentaurVendorStrId   "CentaurHauls" // Centaur
#define SiSVendorStrId       "SiS SiS SiS " // SiS
#define NexGenVendorStrId    "NexGenDriven" // NexGen
#define TransmetaVendorStrId "GenuineTMx86" // Transmeta
#define RiseVendorStrId      "RiseRiseRise" // Rise
#define UMCVendorStrId       "UMC UMC UMC " // UMC
#define NSemicondVendorStrId "Geode by NSC" // National Semiconductor
#define DMPVendorStrId       "Vortex86 SoC" // DM&P Electronics
#define bhyveVendorStrId     "bhyve bhyve " // virtual machine bhyve
#define KVMVendorStrId       "KVM KVM KVM " // virtual machine KVM
#define MHVVendorStrId       "Microsoft Hv" // virtual machine Microsoft Hyper-V or Windows Virtual PC
#define ParallelsVendorStrId " lrpepyh vr " // virtual machine Parallels
#define VMwareVendorStrId    "VMwareVMware" // virtual machine VMware
#define XenHVMVendorStrId    "XenVMMXenVMM" // virtual machine Xen HVM

#define HCExtensionID  0x80000000

#define SYSUPDATEDTIME 5 // ms relax time between updates

#define SYS_NO_ERRORS 0 // eSystemError NONE

// https://en.wikipedia.org/wiki/CPUID
// EAX = 0: Highest Function Parameter and Manufacturer ID
// EAX = 1 : Processor Info and Feature Bits
// EAX = 2 : Cache and TLB Descriptor information
// EAX = 3 : Processor Serial Number
// EAX = 4 and EAX = Bh : Intel thread / core and cache topology
// EAX = 7, ECX = 0 : Extended Features
// EAX = 80000000h : Get Highest Extended Function Implemented
// EAX = 80000001h : Extended Processor Info and Feature Bits
// EAX = 80000002h, 80000003h, 80000004h : Processor Brand String
// EAX = 80000005h : L1 Cache and TLB Identifiers
// EAX = 80000006h : Extended L2 Cache Features
// EAX = 80000007h : Advanced Power Management Information
// EAX = 80000008h : Virtual and Physical address Sizes
// EAX = 8FFFFFFFh : AMD Easter Egg
//
// EAX=1: Processor Info and Feature Bits
// 
// Processor Version Information - EAX
//
// 31 - 28   27 - 20             19 - 16            15 - 14   13 - 12	      11 - 8	 7 - 4  3 - 0
// Reserved	 Extended Family ID	 Extended Model ID	Reserved  Processor Type  Family ID	 Model	Stepping ID
//
// Additional Information - EBX
//
// 7:0	 Brand Index
// 15:8	 CLFLUSH line size (Value . 8 = cache line size in bytes)
// 23:16 Maximum number of addressable IDs for logical processors in this physical package
// 31:24 Local APIC ID
//
// Intel® Architecture Instruction Set Extensions and Future Features Programming Reference
// https://software.intel.com/sites/default/files/managed/c5/15/architecture-instruction-set-extensions-programming-reference.pdf
// AMD CPUID Specification
// https://www.amd.com/system/files/TechDocs/25481.pdf

namespace SAVFGAME
{
	//>> ID архитектуры процессора
	enum eProcArch
	{
		EPA_UNKNOWN,
		EPA_AMD64,  // x64 (AMD or Intel)
		EPA_INTEL,
		EPA_MIPS,
		EPA_PPC,
		EPA_ARM,
		EPA_IA64,
		EPA_IA32	// EPA_IA32_ON_WIN64	
	};

	//>> ID типа архитектуры процессора
	enum eProcArchType
	{
		EPAT_UNKNOWN,
		EPAT_INTEL_386,
		EPAT_INTEL_486,
		EPAT_INTEL_PENTIUM,
		EPAT_INTEL_IA64,
		EPAT_AMD_X8664,
		EPAT_MIPS_R4000,
		EPAT_ALPHA_21064,
		EPAT_PPC_601,
		EPAT_PPC_603,
		EPAT_PPC_604,
		EPAT_PPC_620,
		EPAT_HITACHI_SH3,
		EPAT_HITACHI_SH3E,
		EPAT_HITACHI_SH4,
		EPAT_MOTOROLA_821,
		EPAT_SHx_SH3,
		EPAT_SHx_SH4,
		EPAT_STRONGARM,
		EPAT_ARM720,
		EPAT_ARM820,
		EPAT_ARM920,
		EPAT_ARM_7TDMI,
		EPAT_OPTIL
	};

	//>> ID возможностей CPU
	enum eCPUCaps
	{
		eCPUCap_HTT,	// hyper threaded proc
		eCPUCap_SSE,
		eCPUCap_SSE2,
		eCPUCap_SSE3,
		eCPUCap_SSSE3,
		eCPUCap_SSE41,
		eCPUCap_SSE42,
		eCPUCap_SSE4a,
		eCPUCap_MMX,
		eCPUCap_AVX,
		eCPUCap_AVX2,
		eCPUCap_AVX512F,
		eCPUCap_AVX512BW,
		eCPUCap_AVX512CD,
		eCPUCap_AVX512DQ,
		eCPUCap_AVX512ER,
		eCPUCap_AVX512IFMA52,
		eCPUCap_AVX512PF,
		eCPUCap_AVX512VL,
		eCPUCap_AVX512VPOPCNTDQ,
		eCPUCap_AVX512_4FMAPS,
		eCPUCap_AVX512_4VNNIW,
		eCPUCap_AVX512_BITALG,
		eCPUCap_AVX512_VBMI,
		eCPUCap_AVX512_VBMI2,
		eCPUCap_AVX512_VNNI,
		eCPUCap_FMA,
		eCPUCap_KNC,
		eCPUCap_SVML,

	//////////////////////////////////

		eCPUCap_ADX,
		eCPUCap_AES,
		eCPUCap_BMI1,
		eCPUCap_BMI2,
		eCPUCap_CLDEMOTE,
		eCPUCap_CLFLUSHOPT,
		eCPUCap_CLWB,
		eCPUCap_F16C,
		eCPUCap_FSGSBASE,
		eCPUCap_FXSR,
		eCPUCap_GFNI,
		eCPUCap_INVPCID,
		eCPUCap_LZCNT,
		eCPUCap_MONITOR,
		eCPUCap_MOVBE,
		eCPUCap_MOVDIR64B,
		eCPUCap_MOVDIRI,
		eCPUCap_MPX,
		eCPUCap_PCLMULQDQ,
		eCPUCap_PCONFIG,
		eCPUCap_POPCNT,
		eCPUCap_PREFETCHWT1,
		eCPUCap_RDPID,
		eCPUCap_RDRAND,
		eCPUCap_RDSEED,
		eCPUCap_RDTSCP,
		eCPUCap_RTM,
		eCPUCap_SHA,
	//	eCPUCap_TSC,
		eCPUCap_VAES,
		eCPUCap_VPCLMULQDQ,
		eCPUCap_WAITPKG,
		eCPUCap_WBNOINVD,
		eCPUCap_XSAVE,
		eCPUCap_XSAVEC,
		eCPUCap_XSAVEOPT,
		eCPUCap_XSS,

	//////////////////////////////////

		eCPUCap_ABM,
		eCPUCap_CLFSH,
		eCPUCap_CMOV,
		eCPUCap_CMPXCHG16B,
		eCPUCap_CX8,
		eCPUCap_ERMS,
		eCPUCap_HLE,
		eCPUCap_LAHF,
		eCPUCap_MMXEXT,
		eCPUCap_MSR,
		eCPUCap_OSXSAVE,
		eCPUCap_SEP,
		eCPUCap_SYSCALL,
		eCPUCap_TBM,
		eCPUCap_XOP,
		eCPUCap__3DNOWEXT,
		eCPUCap__3DNOW,

		eCPUCap___ENUM_MAX
	};

	//>> Коды возможных ошибок (вариант для OS Windows)
	enum eSystemError : unsigned int
	{
		eSystemError_NONE = SYS_NO_ERRORS,
		/////////////////
		eSystemError_0xF                           = 0x0000000F, // GetOSInfo() - Get OS version mask of errors
		eSystemError_0xF_GetSystemDirectory        = 0x00000001, // GetOSInfo() - Get OS version error - failed GetSystemDirectory()
		eSystemError_0xF_GetFileVersionInfoSizeEx  = 0x00000002, // GetOSInfo() - Get OS version error - failed GetFileVersionInfoSizeEx()
		eSystemError_0xF_mallocBufferEx            = 0x00000003, // GetOSInfo() - Get OS version error - failed malloc() memory
		eSystemError_0xF_GetFileVersionInfoEx      = 0x00000004, // GetOSInfo() - Get OS version error - failed GetFileVersionInfoEx()
		eSystemError_0xF_VerQueryValue             = 0x00000005, // GetOSInfo() - Get OS version error - failed VerQueryValue()
		eSystemError_0xF_VerQueryValueInfoSize     = 0x00000006, // GetOSInfo() - Get OS version error - info size is NULL
		/////////////////
		eSystemError_0xF0            = 0x000000F0, // GetOSInfo() - Get OS mask of errors
		eSystemError_0xF0_kernel32   = 0x00000010, // GetOSInfo() - Get OS error - failed to find kernel32.dll
		eSystemError_0xF0_IsWow64    = 0x00000020, // GetOSInfo() - Get OS error - failed IsWow64Process()
		/////////////////
		eSystemError_0xF00              = 0x00000F00, // GetMain() - Get main CPU info mask of errors
		eSystemError_0xF00_manufacturer = 0x00000100, // GetMain() - Get main CPU info error - unknown manufacturer (not AMD/Intel)
		/////////////////
		eSystemError_0xF000     = 0x0000F000, // GetSystemInfo() - Get sys info mask of errors
		eSystemError_0xF000_RAM = 0x00001000, // GetSystemInfo() - Get sys info error - failed GetPhysicallyInstalledSystemMemory()
		/////////////////
		eSystemError_0xF0000             = 0x000F0000, // GetUpdate() - Get update mask of errors
		eSystemError_0xF0000_MemStatus   = 0x00010000, // GetUpdate() - Get update error - failed GlobalMemoryStatusEx()
		/////////////////
		eSystemError_0xF00000                    = 0x00F00000, // GetDisksInfo() - Get disks mask of errors
		eSystemError_0xF00000_kernel32           = 0x00100000, // GetDisksInfo() - Get disks error - failed to find kernel32.dll
		eSystemError_0xF00000_pGetLogicalDrives  = 0x00200000, // GetDisksInfo() - Get disks error - failed to find ptr GetLogicalDrives()
		eSystemError_0xF00000_GetLogicalDrives   = 0x00300000, // GetDisksInfo() - Get disks error - failed GetLogicalDrives()
		eSystemError_0xF00000_NoFixedDisks       = 0x00400000, // GetDisksInfo() - Get disks error - can't find any fixed disk
		eSystemError_0xF00000_GetDiskFreeSpaceEx = 0x00500000, // GetDisksInfo() - Get disks error - failed GetDiskFreeSpaceExW()
		/////////////////
		eSystemError_0xF000000 = 0x0F000000, // RESERVED mask of errors
		/////////////////
		eSystemError_0xF0000000             = 0xF0000000, // Mask of special returning codes
		eSystemError_0xF0000000_InitAlready = 0x10000000, // Init() - returning value if CSystem is already inited
	};

	//>> Коды типа OS
	enum eSystemOStype : unsigned int
	{
		eSystemOS_NO_INFO = 0,
		//
		eSystemOS_Win2000_or_greater  = MASK_BIT_00, // MASK_BIT_00
		eSystemOS_WinXP_or_greater    = MASK_BIT_01, // MASK_BIT_01
		eSystemOS_WinVista_or_greater = MASK_BIT_02, // MASK_BIT_02
		eSystemOS_Win7_or_greater     = MASK_BIT_03, // MASK_BIT_03
		eSystemOS_Win8_or_greater     = MASK_BIT_04, // MASK_BIT_04
		eSystemOS_Win81_or_greater    = MASK_BIT_05, // MASK_BIT_05
		eSystemOS_Win10_or_greater    = MASK_BIT_06, // MASK_BIT_06
		//
		eSystemOS_Win2000  = 0x10000000 | eSystemOS_Win2000_or_greater,									// 0x10000001
		eSystemOS_WinXP    = 0x20000000 | eSystemOS_Win2000_or_greater | eSystemOS_WinXP_or_greater,	// 0x20000003
		eSystemOS_WinXPx64 = 0x30000000 | eSystemOS_Win2000_or_greater | eSystemOS_WinXP_or_greater,	// 0x30000003
		eSystemOS_WinVista = 0x40000000 | eSystemOS_Win2000_or_greater | eSystemOS_WinXP_or_greater		// 0x40000007
		                                | eSystemOS_WinVista_or_greater,
		eSystemOS_Win7     = 0x50000000 | eSystemOS_Win2000_or_greater  | eSystemOS_WinXP_or_greater	// 0x5000000F
		                                | eSystemOS_WinVista_or_greater | eSystemOS_Win7_or_greater,
		eSystemOS_Win8     = 0x60000000 | eSystemOS_Win2000_or_greater  | eSystemOS_WinXP_or_greater	// 0x6000001F
		                                | eSystemOS_WinVista_or_greater | eSystemOS_Win7_or_greater
										| eSystemOS_Win8_or_greater,
		eSystemOS_Win81    = 0x70000000 | eSystemOS_Win2000_or_greater  | eSystemOS_WinXP_or_greater	// 0x7000003F
		                                | eSystemOS_WinVista_or_greater | eSystemOS_Win7_or_greater
										| eSystemOS_Win8_or_greater     | eSystemOS_Win81_or_greater,
		eSystemOS_Win10    = 0x80000000 | eSystemOS_Win2000_or_greater  | eSystemOS_WinXP_or_greater	// 0x8000007F
		                                | eSystemOS_WinVista_or_greater | eSystemOS_Win7_or_greater
										| eSystemOS_Win8_or_greater     | eSystemOS_Win81_or_greater
										| eSystemOS_Win10_or_greater,
	};

	//>> Коды типов дисков в системе
	enum eSystemDiskType
	{
		eSystemDiskNotPresented,
		eSystemDiskUnknown,			// DRIVE_UNKNOWN   Cannot be determined
		eSystemDiskFixed,			// DRIVE_FIXED     Fixed media (hard disk drive, flash drive)
		eSystemDiskRemovable,		// DRIVE_REMOVABLE Removable media (floppy drive, thumb drive, flash card reader)
		eSystemDiskCDROM,			// DRIVE_CDROM     The drive is a CD-ROM drive
		eSystemDiskRemote,			// DRIVE_REMOTE    The drive is a remote (network) drive
		eSystemDiskRAM,				// DRIVE_RAMDISK   The drive is a RAM disk

		eSystemDisk_ENUM_MAX
	};

	//>> Соответсвие имени диска и его номера
	enum eSystemDiskNameValue
	{
		eSystemDiskName_A = 0,
		eSystemDiskName_B,
		eSystemDiskName_C,
		eSystemDiskName_D,
		eSystemDiskName_E,
		eSystemDiskName_F,
		eSystemDiskName_G,
		eSystemDiskName_H,
		eSystemDiskName_I,
		eSystemDiskName_J,
		eSystemDiskName_K,
		eSystemDiskName_L,
		eSystemDiskName_M,
		eSystemDiskName_N,
		eSystemDiskName_O,
		eSystemDiskName_P,
		eSystemDiskName_Q,
		eSystemDiskName_R,
		eSystemDiskName_S,
		eSystemDiskName_T,
		eSystemDiskName_U,
		eSystemDiskName_V,
		eSystemDiskName_W,
		eSystemDiskName_X,
		eSystemDiskName_Y,
		eSystemDiskName_Z,

		eSystemDiskName_ENUM_MAX // max value names (invalid name ret value)
	};

	//>> Типы выдаваемой пользователю информации
	enum eSystemRet
	{
		eSystemRet_RAM_available,	// (in bytes) доступное кол-во оперативной памяти в системе : ret MISSING (-1) if error
		eSystemRet_RAM_installed,	// (in bytes) установленное кол-во оперативной памяти в системе : ret MISSING (-1) if error
		eSystemRet_OS_64_bit,		// (bool) сообщает тип OS : x32 (false) или x64 (true) : ret MISSING (-1) if error
		eSystemRet_OS_type,			// (eSystemOStype) тип OS (uint32 combined bitmask) : ret MISSING (-1) if error
		eSystemRet_HDD_free,		// (in bytes) HDD free space : (передать в extra_input букву диска wchar_t) : ret MISSING (-1) if error
	};

	//>> Тип реализации класса
	enum eSystemClassType
	{
		eSystemClassVirtual,   // class CSystem
		eSystemClassWin,       // class CSystemWin : public CSystemBase
	};

	//>> Прототип сборщика сведений о системе
	class CSystemBase
	{
		friend class CSystem;
	protected:
		uint32           system_error;   // eSystemError : content initial errors
		mutex            m_Update;       // update blocker
		decltype(_TIME)  update_time;    // update timer
		eSystemClassType implementation; // type of system class
	protected:
		struct SYSINFO
		{
			SYSINFO() { Reset(); }
			void Reset()
			{
				processor_architecture_raw = MISSING;
				processor_type_raw         = MISSING;
				processor_architecture = EPA_UNKNOWN;
				processor_type         = EPAT_UNKNOWN;
				page_size              = 0;
				minimum_app_addr       = 0;
				maximum_app_addr       = 0;
				active_processor_mask  = 0;
				num_of_processors      = 0;			
				allocation_granularity = 0;
				processor_level        = 0;
				processor_revision     = 0;
				system_memory          = 0;
			}
			uint32 processor_architecture_raw;
			uint32 processor_type_raw;
			eProcArch	  processor_architecture;
			eProcArchType processor_type;
			uint32	page_size;				// page size and the granularity of page protection and commitment
			void *	minimum_app_addr;		// pointer to the lowest memory address accessible to applications and dynamic-link libraries
			void *	maximum_app_addr;		// pointer to the highest memory address accessible to applications and DLLs
			uint32	active_processor_mask;	// mask representing the set of processors : bit 0 is processor 0; bit 31 is processor 31
			uint32	num_of_processors;	
			uint32	allocation_granularity; // granularity for the starting address at which virtual memory can be allocated
			uint16	processor_level;
			uint16	processor_revision;
			uint64  system_memory;			// installed RAM size (in KB)
			void Printf()
			{
				printf("\nSYSTEM INFORMATION");
				printf("\n- proc arch  = ");
				switch (processor_architecture)
				{
				case EPA_AMD64:   printf("x64 (AMD/Intel)"); break;
				case EPA_INTEL:   printf("Intel");   break;
				case EPA_MIPS:    printf("MIPS");    break;
				case EPA_PPC:     printf("PPC");     break;
				case EPA_ARM:     printf("ARM");     break;
				case EPA_IA64:    printf("IA64");    break;
				case EPA_IA32:    printf("IA32");    break;
				case EPA_UNKNOWN: printf("unknown"); break;
				}
				printf(" [0x%X]", processor_architecture_raw);
				printf("\n- proc type  = ");
				switch (processor_type)
				{
				case EPAT_INTEL_386:     printf("INTEL_386");     break;
				case EPAT_INTEL_486:     printf("INTEL_486");     break;
				case EPAT_INTEL_PENTIUM: printf("INTEL_PENTIUM"); break;
				case EPAT_INTEL_IA64:    printf("INTEL_IA64");    break;
				case EPAT_AMD_X8664:     printf("AMD_X8664");     break;
				case EPAT_MIPS_R4000:    printf("MIPS_R4000");    break;
				case EPAT_ALPHA_21064:   printf("ALPHA_21064");   break;
				case EPAT_PPC_601:       printf("PPC_601");       break;
				case EPAT_PPC_603:       printf("PPC_603");       break;
				case EPAT_PPC_604:       printf("PPC_604");       break;
				case EPAT_PPC_620:       printf("PPC_620");       break;
				case EPAT_HITACHI_SH3:   printf("HITACHI_SH3");   break;
				case EPAT_HITACHI_SH3E:  printf("HITACHI_SH3E");  break;
				case EPAT_HITACHI_SH4:   printf("HITACHI_SH4");   break;
				case EPAT_MOTOROLA_821:  printf("MOTOROLA_821");  break;
				case EPAT_SHx_SH3:       printf("SHx_SH3");       break;
				case EPAT_SHx_SH4:       printf("SHx_SH4");       break;
				case EPAT_STRONGARM:     printf("STRONGARM");     break;
				case EPAT_ARM720:        printf("ARM720");        break;
				case EPAT_ARM820:        printf("ARM820");        break;
				case EPAT_ARM920:        printf("ARM920");        break;
				case EPAT_ARM_7TDMI:     printf("ARM_7TDMI");     break;
				case EPAT_OPTIL:         printf("OPTIL");         break;
				case EPAT_UNKNOWN:       printf("unknown");       break;
				}
				printf(" [0x%X]", processor_type_raw);
				printf("\n- page size  = 0x%X", page_size);
				printf("\n- alloc gran = 0x%X", allocation_granularity);
				printf("\n- min addr   = 0x%X", (int)minimum_app_addr);
				printf("\n- max addr   = 0x%X", (int)maximum_app_addr);
				printf("\n- num procs  = %i", num_of_processors);
				printf("\n- proc level = 0x%X", processor_level);
				printf("\n- proc rev   = 0x%X", processor_revision);
				for (uint32 i = 0; i<num_of_processors; i++)
				{
					printf("\n- proc %2i    = ", i);
					if ((active_processor_mask >> i) & 1)
						 printf("active");
					else printf("inactive");
				}
				printf("\n- sys memory = %i MB", _KBTOMB(system_memory));
			}
		} system_info;   // system data
		struct SYSMEM
		{
			SYSMEM() { Reset(); }
			void Reset()
			{
				ZeroMemory(this, sizeof(SYSMEM));
			}
			uint32  percent_in_use; // in percents
			uint64  total_phys;		// in bytes
			uint64  avail_phys;		// in bytes
			uint64  total_page;		// in bytes
			uint64  avail_page;		// in bytes
			uint64  total_virt;		// address space
			uint64  avail_virt;		// address space
			uint64  avail_virt_ext; // address space (extended) always 0 (reserved value)
			void Printf()
			{
				printf("\nSYSTEM MEMORY INFORMATION");
				printf("\n- Memory in use     = %3u %%",  percent_in_use);
				printf("\n- Total phys. mem   = %5llu MB (%llu KB)", _BYTESTOMB(total_phys), _BYTESTOKB(total_phys));
				printf("\n- Free  phys. mem   = %5llu MB (%llu KB)", _BYTESTOMB(avail_phys), _BYTESTOKB(avail_phys));
				printf("\n- Total paging file = %5llu MB (%llu KB)", _BYTESTOMB(total_page), _BYTESTOKB(total_page));
				printf("\n- Free  paging file = %5llu MB (%llu KB)", _BYTESTOMB(avail_page), _BYTESTOKB(avail_page));
				printf("\n- Total virtual mem = 0x%llX", total_virt);
				printf("\n- Free  virtual mem = 0x%llX", avail_virt);
				printf("\n- Free extended mem = 0x%llX", _BYTESTOMB(avail_virt_ext), _BYTESTOKB(avail_virt_ext));
			}
		} system_memory;  // actual RAM info
		struct SYSOS
		{
			SYSOS()
			{
				Reset();
			}
			void Reset()
			{
				engine3264 = 0;
				system64   = 0;
				version    = 0;
				type       = eSystemOS_NO_INFO;
				sysdir     = L"";
			}
			bool			engine3264; // is OS using WOW64 to run x32 engine build on x64 machine
			bool			system64;   // is OS an 64-bit OS
			uint32			version;    // example : 0x00060001 = 6.1 = Win7
			uint32			type;		// OS type bit mask (user out info)
			std::wstring	sysdir;     // system directory
			struct VERSION
			{
				VERSION() { ZeroMemory(this, sizeof(VERSION)); }
				//
				unsigned _5_0_Win2000      : 1;
				unsigned _5_1_WinXP        : 1;
				unsigned _5_2_WinXPx64     : 1;
				unsigned _6_0_WinVista     : 1;
				unsigned _6_1_Win7         : 1;
				unsigned _6_2_Win8         : 1;
				unsigned _6_3_Win81        : 1;
				unsigned _10_0_Win10       : 1;
				//
				unsigned is_Win2000_or_greater  : 1;
				unsigned is_WinXP_or_greater    : 1;
				unsigned is_WinVista_or_greater : 1;
				unsigned is_Win7_or_greater     : 1;
				unsigned is_Win8_or_greater     : 1;
				unsigned is_Win81_or_greater    : 1;
				unsigned is_Win10_or_greater    : 1;
				//
			} _windows;
			void UpdateType()
			{
					 if (_windows._5_0_Win2000)  type = eSystemOS_Win2000;
				else if (_windows._5_1_WinXP)    type = eSystemOS_WinXP;
				else if (_windows._5_2_WinXPx64) type = eSystemOS_WinXPx64;
				else if (_windows._6_0_WinVista) type = eSystemOS_WinVista;
				else if (_windows._6_1_Win7)     type = eSystemOS_Win7;
				else if (_windows._6_2_Win8)     type = eSystemOS_Win8;
				else if (_windows._6_3_Win81)    type = eSystemOS_Win81;
				else if (_windows._10_0_Win10)   type = eSystemOS_Win10;
			}
			void Printf()
			{
				printf("\nOS INFORMATION");
				printf("\n- version = ");
					 if (_windows._5_0_Win2000)  printf("Windows 2000 ");
				else if (_windows._5_1_WinXP)    printf("Windows XP ");
				else if (_windows._5_2_WinXPx64) printf("Windows XP 64-bit ");
				else if (_windows._6_0_WinVista) printf("Windows Vista ");
				else if (_windows._6_1_Win7)     printf("Windows 7 ");
				else if (_windows._6_2_Win8)     printf("Windows 8 ");
				else if (_windows._6_3_Win81)    printf("Windows 8.1 ");
				else if (_windows._10_0_Win10)   printf("Windows 10 ");

				 printf( "\n- verbits = %s", system64 ? "64 bit" : "32 bit");
				 printf( "\n- WOW64   = %s", _BOOLYESNO(engine3264));
				wprintf(L"\n- sysdir  = %s", sysdir.c_str());
			}
		} system_os;       // OS info
		struct SYSDISK
		{
			struct SysDiskData
			{
				friend struct SYSDISK;
			private:
				void Reset()
				{
					presented           = false;
					type                = eSystemDiskNotPresented;
					size_total          = 0;
					size_free           = 0;
					size_free_to_caller = 0;
					name                = 0;
					ZeroMemory(root, sizeof(root));
				}
			public:
				bool            presented;
				eSystemDiskType type;
				wchar_t         name;
				wchar_t         root[8];
				uint64          size_total;			 // in bytes
				uint64          size_free;           // in bytes
				uint64          size_free_to_caller; // in bytes
			};
			SYSDISK() { Reset(); }
			void Reset()
			{
				for (int i = 0; i < eSystemDiskName_ENUM_MAX; i++)
				{
					disk[i].Reset();
					disk[i].name = GetDisk((eSystemDiskNameValue)i);
					wsprintf(disk[i].root, L"%c:\\", disk[i].name);
				}
				updater = 0;
			}
			////////////////////////////////////////
			SysDiskData disk[eSystemDiskName_ENUM_MAX];
			int updater; // type 0 default, 1 other (метод получения инфы)
			//>> Возвращает порядковый номер диска eSystemDiskNameValue из имени
			uint32 GetDisk(wchar_t ch_disk)
			{
				uint32 ret;

				switch (ch_disk)
				{
				case (wchar_t)'A': ret = eSystemDiskName_A; break;
				case (wchar_t)'B': ret = eSystemDiskName_B; break;
				case (wchar_t)'C': ret = eSystemDiskName_C; break;
				case (wchar_t)'D': ret = eSystemDiskName_D; break;
				case (wchar_t)'E': ret = eSystemDiskName_E; break;
				case (wchar_t)'F': ret = eSystemDiskName_F; break;
				case (wchar_t)'G': ret = eSystemDiskName_G; break;
				case (wchar_t)'H': ret = eSystemDiskName_H; break;
				case (wchar_t)'I': ret = eSystemDiskName_I; break;
				case (wchar_t)'J': ret = eSystemDiskName_J; break;
				case (wchar_t)'K': ret = eSystemDiskName_K; break;
				case (wchar_t)'L': ret = eSystemDiskName_L; break;
				case (wchar_t)'M': ret = eSystemDiskName_M; break;
				case (wchar_t)'N': ret = eSystemDiskName_N; break;
				case (wchar_t)'O': ret = eSystemDiskName_O; break;
				case (wchar_t)'P': ret = eSystemDiskName_P; break;
				case (wchar_t)'Q': ret = eSystemDiskName_Q; break;
				case (wchar_t)'R': ret = eSystemDiskName_R; break;
				case (wchar_t)'S': ret = eSystemDiskName_S; break;
				case (wchar_t)'T': ret = eSystemDiskName_T; break;
				case (wchar_t)'U': ret = eSystemDiskName_U; break;
				case (wchar_t)'V': ret = eSystemDiskName_V; break;
				case (wchar_t)'W': ret = eSystemDiskName_W; break;
				case (wchar_t)'X': ret = eSystemDiskName_X; break;
				case (wchar_t)'Y': ret = eSystemDiskName_Y; break;
				case (wchar_t)'Z': ret = eSystemDiskName_Z; break;
				default:           ret = eSystemDiskName_ENUM_MAX; break;
				}

				return ret;
			}
			//>> Возвращает имя диска из порядкового номера eSystemDiskNameValue
			wchar_t GetDisk(eSystemDiskNameValue i_disk)
			{
				wchar_t ret;

				switch (i_disk)
				{
				case eSystemDiskName_A: ret = 'A'; break;
				case eSystemDiskName_B: ret = 'B'; break;
				case eSystemDiskName_C: ret = 'C'; break;
				case eSystemDiskName_D: ret = 'D'; break;
				case eSystemDiskName_E: ret = 'E'; break;
				case eSystemDiskName_F: ret = 'F'; break;
				case eSystemDiskName_G: ret = 'G'; break;
				case eSystemDiskName_H: ret = 'H'; break;
				case eSystemDiskName_I: ret = 'I'; break;
				case eSystemDiskName_J: ret = 'J'; break;
				case eSystemDiskName_K: ret = 'K'; break;
				case eSystemDiskName_L: ret = 'L'; break;
				case eSystemDiskName_M: ret = 'M'; break;
				case eSystemDiskName_N: ret = 'N'; break;
				case eSystemDiskName_O: ret = 'O'; break;
				case eSystemDiskName_P: ret = 'P'; break;
				case eSystemDiskName_Q: ret = 'Q'; break;
				case eSystemDiskName_R: ret = 'R'; break;
				case eSystemDiskName_S: ret = 'S'; break;
				case eSystemDiskName_T: ret = 'T'; break;
				case eSystemDiskName_U: ret = 'U'; break;
				case eSystemDiskName_V: ret = 'V'; break;
				case eSystemDiskName_W: ret = 'W'; break;
				case eSystemDiskName_X: ret = 'X'; break;
				case eSystemDiskName_Y: ret = 'Y'; break;
				case eSystemDiskName_Z: ret = 'Z'; break;
				default:                ret = 0;   break;
				}

				return ret;
			}
			//>> 
			void Printf()
			{
				printf("\nDISKS INFORMATION");
				for (int i = 0; i < eSystemDiskName_ENUM_MAX; i++)
				if (disk[i].presented)
				{
					printf("\nDrive %s - ", disk[i].root);
					switch (disk[i].type)
					{
					case eSystemDiskNotPresented: printf("not presented  "); break;
					case eSystemDiskUnknown:      printf("uknown type    "); break;
					case eSystemDiskFixed:        printf("fixed disk     "); break;
					case eSystemDiskRemovable:    printf("removable disk "); break;
					case eSystemDiskCDROM:        printf("CD-ROM         "); break;
					case eSystemDiskRemote:       printf("remote disk    "); break;
					case eSystemDiskRAM:          printf("RAM disk       "); break;
					default:                      printf("INVALID INPUT  ");
					}
					float free_mb  = (float) _BYTESTOMB(disk[i].size_free);
					float total_mb = (float) _BYTESTOMB(disk[i].size_total);
					if (disk[i].type == eSystemDiskFixed)
					printf("%6.2f / %6.2f GB available", _MBTOGB(free_mb), _MBTOGB(total_mb));
				}
				printf("\nUpdater - type %i", updater);
			}
		} system_disk; // disks drives info
		struct MAIN
		{
			MAIN() { Reset(); }
			void Reset()
			{
				id     = 0;
				idEx   = 0;
				nCores = 0;
				nLogic = 0;
				nSMT   = 0;
				intel  = false;
				amd    = false;
				isInit = false;
				brand  = "";
			}
			int  id;     // num of the highest valid function ID
			int  idEx;   // num of the highest valid extended ID
			int  nCores; // num of cores
			int  nLogic; // num of logical cores
			int  nSMT;	 // num of SMT
			bool intel;  // vendor string == "GenuineIntel"
			bool amd;    // vendor string == "AuthenticAMD"
			bool isInit; //
			std::string brand; // CPU description
			void Printf()
			{
				printf( "\nMAIN CPU INFORMATION" \
						"\nCPU   = %s" \
						"\nIntel = %s" \
						"\nAMD   = %s" \
						"\nCores = %i" \
						"\nCores = %i (logical)" \
						"\nSMT   = %i" \
						,
					brand.c_str(),
					_BOOLYESNO(intel),
					_BOOLYESNO(amd),
					nCores,
					nLogic,
					nSMT
				);
			}
		} main; // main CPU data
		struct CAPS {
			CAPS() { Reset(); }
			void Reset()
			{
				ZeroMemory(this, sizeof(CAPS));
			}
			struct {
				unsigned HTT             : 1; // hyper threaded proc
				unsigned SSE             : 1;
				unsigned SSE2            : 1;
				unsigned SSE3            : 1;
				unsigned SSSE3           : 1; // S-SSE3 : supplemental SSE3 instructions
				unsigned SSE41           : 1;
				unsigned SSE42           : 1;
				unsigned SSE4a           : 1; // AMD SSE 4
				unsigned MMX             : 1;
				unsigned AVX             : 1; // AVX : advanced vector extensions
				unsigned AVX2            : 1;
				unsigned AVX512F         : 1; // AVX-512 foundation instructions
				unsigned AVX512BW        : 1; // AVX-512 byte and word instructions
				unsigned AVX512CD        : 1;
				unsigned AVX512DQ        : 1; // AVX-512 doubleword and quadword instructions
				unsigned AVX512ER        : 1;
				unsigned AVX512IFMA52    : 1; // AVX-512 integer fused multiply-add instructions (avx512ifma)
				unsigned AVX512PF        : 1;
				unsigned AVX512VL        : 1; // AVX-512 vector length extensions
				unsigned AVX512VPOPCNTDQ : 1; // AVX-512 vector population count double and quad-word
				unsigned AVX512_4FMAPS   : 1; // AVX-512 4-register multiply accumulation single precision
				unsigned AVX512_4VNNIW   : 1; // AVX-512 4-register neural network instructions
				unsigned AVX512_BITALG   : 1; // AVX-512 BITALG instructions
				unsigned AVX512_VBMI     : 1; // AVX-512 vector bit manipulation instructions
				unsigned AVX512_VBMI2    : 1; // AVX-512 vector bit manipulation instructions 2
				unsigned AVX512_VNNI     : 1; // AVX-512 vector neural network instructions		
				unsigned FMA             : 1; // FMA : fused multiply-add
				unsigned KNC             : 1; // Knights Corner
				unsigned SVML            : 1; // SVML : short vector math library			

				void Printf()
				{
					printf( "\nMAIN CPU CAPABILITIES" \
							"\nHTT             = %s" \
							"\nSSE             = %s" \
							"\nSSE2            = %s" \
							"\nSSE3            = %s" \
							"\nSSSE3           = %s" \
							"\nSSE41           = %s" \
							"\nSSE42           = %s" \
							"\nSSE4a           = %s" \
							"\nMMX             = %s" \
							"\nAVX             = %s" \
							"\nAVX2            = %s" \
							"\nAVX512F         = %s" \
							"\nAVX512BW        = %s" \
							"\nAVX512CD        = %s" \
							"\nAVX512DQ        = %s" \
							"\nAVX512ER        = %s" \
							"\nAVX512IFMA52    = %s" \
							"\nAVX512PF        = %s" \
							"\nAVX512VL        = %s" \
							"\nAVX512VPOPCNTDQ = %s" \
							"\nAVX512_4FMAPS   = %s" \
							"\nAVX512_4VNNIW   = %s" \
							"\nAVX512_BITALG   = %s" \
							"\nAVX512_VBMI     = %s" \
							"\nAVX512_VBMI2    = %s" \
							"\nAVX512_VNNI     = %s" \
							"\nFMA             = %s" \
							"\nKNC             = %s" \
							"\nSVML            = %s" \
							,
						_BOOLYESNO(HTT),
						_BOOLYESNO(SSE),
						_BOOLYESNO(SSE2),
						_BOOLYESNO(SSE3),
						_BOOLYESNO(SSSE3),
						_BOOLYESNO(SSE41),
						_BOOLYESNO(SSE42),
						_BOOLYESNO(SSE4a),
						_BOOLYESNO(MMX),
						_BOOLYESNO(AVX),
						_BOOLYESNO(AVX2),
						_BOOLYESNO(AVX512F),
						_BOOLYESNO(AVX512BW),
						_BOOLYESNO(AVX512CD),
						_BOOLYESNO(AVX512DQ),
						_BOOLYESNO(AVX512ER),
						_BOOLYESNO(AVX512IFMA52),
						_BOOLYESNO(AVX512PF),
						_BOOLYESNO(AVX512VL),
						_BOOLYESNO(AVX512VPOPCNTDQ),
						_BOOLYESNO(AVX512_4FMAPS),
						_BOOLYESNO(AVX512_4VNNIW),
						_BOOLYESNO(AVX512_BITALG),
						_BOOLYESNO(AVX512_VBMI),
						_BOOLYESNO(AVX512_VBMI2),
						_BOOLYESNO(AVX512_VNNI),		
						_BOOLYESNO(FMA),
						_BOOLYESNO(KNC),
						_BOOLYESNO(SVML),
						"end"
					);
				}
			
			} main;
			struct {

				unsigned ADX        : 1;
				unsigned AES        : 1;
				unsigned BMI1       : 1;
				unsigned BMI2       : 1;
				unsigned CLDEMOTE   : 1; // cache line demote
				unsigned CLFLUSHOPT : 1;
				unsigned CLWB       : 1;
				unsigned F16C       : 1;
				unsigned FSGSBASE   : 1;
				unsigned FXSR       : 1;
				unsigned GFNI       : 1; // galois field instructions
				unsigned INVPCID    : 1;
				unsigned LZCNT      : 1;
				unsigned MONITOR    : 1;
				unsigned MOVBE      : 1;
				unsigned MOVDIR64B  : 1; // move 64 bytes as direct store
				unsigned MOVDIRI    : 1; // move doubleword as direct store
				unsigned MPX        : 1; // memory protection extensions
				unsigned PCLMULQDQ  : 1; // carry-less multiplication quadword
				unsigned PCONFIG    : 1; // platform configuration
				unsigned POPCNT     : 1; // return the count of number of bits set to 1 in BYTE/WORD/DWORD/QWORD
				unsigned PREFETCHWT1: 1;
				unsigned RDPID      : 1; // read processor ID
				unsigned RDRAND     : 1;
				unsigned RDSEED     : 1;
				unsigned RDTSCP     : 1;
				unsigned RTM        : 1;
				unsigned SHA        : 1;
			//	unsigned TSC        : 1; // ?
				unsigned VAES       : 1; // vector AES instruction set (VEX-256/EVEX)
				unsigned VPCLMULQDQ : 1; // CLMUL instruction set (VEX-256/EVEX)
				unsigned WAITPKG    : 1; // 
				unsigned WBNOINVD   : 1; // write back and do not invalidate cache
				unsigned XSAVE      : 1; // save processor extended states to memory
				unsigned XSAVEC     : 1; // save processor extended states with compaction to memory + XRSTOR compacted form
				unsigned XSAVEOPT   : 1; // save processor extended states to memory, optimized
				unsigned XSS        : 1; // IA32_XSS + XSAVES/XRSTORS

			///////////////////////////////

				unsigned ABM        : 1;
				unsigned CLFSH      : 1; // CLFLUSH instruction
				unsigned CMOV       : 1; // conditional move
				unsigned CMPXCHG16B : 1;
				unsigned CX8        : 1;
				unsigned ERMS       : 1;
				unsigned HLE        : 1;
				unsigned LAHF       : 1; // LAHF/SAHF available in 64-bit mode
				unsigned MMXEXT     : 1;
				unsigned MSR        : 1; // model specific registers RDMSR and WRMSR instructions
				unsigned OSXSAVE    : 1; // enable XSETBV/XGETBV to access XCR0 and support proc ext. state management using XSAVE / XRSTOR
				unsigned SEP        : 1; // SYSENTER and SYSEXIT
				unsigned SYSCALL    : 1; // SYSCALL/SYSRET available (when in 64-bit mode)
				unsigned TBM        : 1;
				unsigned XOP        : 1;
				unsigned _3DNOWEXT  : 1; // AMD extensions to 3DNow! instructions
				unsigned _3DNOW     : 1; // AMD 3DNow! instructions

				void Printf()
				{
					printf( "\nOTHER CPU CAPABILITIES - PART 1" \
							"\nADX         = %s" \
							"\nAES         = %s" \
							"\nBMI1        = %s" \
							"\nBMI2        = %s" \
							"\nCLDEMOTE    = %s" \
							"\nCLFLUSHOPT  = %s" \
							"\nCLWB        = %s" \
							"\nF16C        = %s" \
							"\nFSGSBASE    = %s" \
							"\nFXSR        = %s" \
							"\nGFNI        = %s" \
							"\nINVPCID     = %s" \
							"\nLZCNT       = %s" \
							"\nMONITOR     = %s" \
							"\nMOVBE       = %s" \
							"\nMOVDIR64B   = %s" \
							"\nMOVDIRI     = %s" \
							"\nMPX         = %s" \
							"\nPCLMULQDQ   = %s" \
							"\nPCONFIG     = %s" \
							"\nPOPCNT      = %s" \
							"\nPREFETCHWT1 = %s" \
							"\nRDPID       = %s" \
							"\nRDRAND      = %s" \
							"\nRDSEED      = %s" \
							"\nRDTSCP      = %s" \
							"\nRTM         = %s" \
							"\nSHA         = %s" \
							"\nTSC         = %s" \
							"\nVAES        = %s" \
							"\nVPCLMULQDQ  = %s" \
							"\nWAITPKG     = %s" \
							"\nWBNOINVD    = %s" \
							"\nXSAVE       = %s" \
							"\nXSAVEC      = %s" \
							"\nXSAVEOPT    = %s" \
							"\nXSS         = %s" \
							"\nOTHER CPU CAPABILITIES - PART 2" \
							"\nABM         = %s" \
							"\nCLFSH       = %s" \
							"\nCMOV        = %s" \
							"\nCMPXCHG16B  = %s" \
							"\nCX8         = %s" \
							"\nERMS        = %s" \
							"\nHLE         = %s" \
							"\nLAHF        = %s" \
							"\nMMXEXT      = %s" \
							"\nMSR         = %s" \
							"\nOSXSAVE     = %s" \
							"\nSEP         = %s" \
							"\nSYSCALL     = %s" \
							"\nTBM         = %s" \
							"\nXOP         = %s" \
							"\n_3DNOWEXT   = %s" \
							"\n_3DNOW      = %s" \
							,
						_BOOLYESNO(ADX),
						_BOOLYESNO(AES),
						_BOOLYESNO(BMI1),
						_BOOLYESNO(BMI2),
						_BOOLYESNO(CLDEMOTE),
						_BOOLYESNO(CLFLUSHOPT),
						_BOOLYESNO(CLWB),
						_BOOLYESNO(F16C),
						_BOOLYESNO(FSGSBASE),
						_BOOLYESNO(FXSR),
						_BOOLYESNO(GFNI),
						_BOOLYESNO(INVPCID),
						_BOOLYESNO(LZCNT),
						_BOOLYESNO(MONITOR),
						_BOOLYESNO(MOVBE),
						_BOOLYESNO(MOVDIR64B),
						_BOOLYESNO(MOVDIRI),
						_BOOLYESNO(MPX),
						_BOOLYESNO(PCLMULQDQ),
						_BOOLYESNO(PCONFIG),
						_BOOLYESNO(POPCNT),
						_BOOLYESNO(PREFETCHWT1),
						_BOOLYESNO(RDPID),
						_BOOLYESNO(RDRAND),
						_BOOLYESNO(RDSEED),
						_BOOLYESNO(RDTSCP),
						_BOOLYESNO(RTM),
						_BOOLYESNO(SHA),
						"???", // _BOOLYESNO(TSC),
						_BOOLYESNO(VAES),
						_BOOLYESNO(VPCLMULQDQ),
						_BOOLYESNO(WAITPKG),
						_BOOLYESNO(WBNOINVD),
						_BOOLYESNO(XSAVE),
						_BOOLYESNO(XSAVEC),
						_BOOLYESNO(XSAVEOPT),
						_BOOLYESNO(XSS),
						
						_BOOLYESNO(ABM),
						_BOOLYESNO(CLFSH),
						_BOOLYESNO(CMOV),
						_BOOLYESNO(CMPXCHG16B),
						_BOOLYESNO(CX8),
						_BOOLYESNO(ERMS),
						_BOOLYESNO(HLE),
						_BOOLYESNO(LAHF),
						_BOOLYESNO(MMXEXT),
						_BOOLYESNO(MSR),
						_BOOLYESNO(OSXSAVE),
						_BOOLYESNO(SEP),
						_BOOLYESNO(SYSCALL),
						_BOOLYESNO(TBM),
						_BOOLYESNO(XOP),
						_BOOLYESNO(_3DNOWEXT),
						_BOOLYESNO(_3DNOW),
						"end"
					);
				}

			} other;
			void Printf()
			{
				main.Printf();
				other.Printf();
			}
		} caps; // CPU capabilities

		CSystemBase() : system_error(0), implementation(eSystemClassVirtual) {};
		virtual ~CSystemBase() {};

	protected:
		virtual uint32 Init() = 0;
		virtual uint64 Update(eSystemRet type, uint64 extra_input) = 0;
	};
}

#endif // _SYSTEMBASE_H