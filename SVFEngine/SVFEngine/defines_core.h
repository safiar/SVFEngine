// ----------------------------------------------------------------------- //
//
// MODULE  : defines_core.h
//
// PURPOSE : Корневые вспомогательные объявления
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2019)
//
// ----------------------------------------------------------------------- //

#ifndef _DEFINES_CORE_H
#define _DEFINES_CORE_H

// ### Checking OS (platform) ###
// Linux                  : __linux__
// Android                : __ANDROID__
// Linux-non-Android      : __linux__ && !__ANDROID__
// Darwin (Mac OS X, iOS) : __APPLE__
// Akaros (akaros.org)    : __ros__
// Windows 32 bit         : _WIN32
// Windows 64 bit         : _WIN64
// NaCL                   : __native_client__
// AsmJS                  : __asmjs__
// Fuschia                : __Fuchsia__
//
// ### Checking compiler ###
// Visual Studio          : _MSC_VER
// gcc                    : __GNUC__
// clang                  : __clang__
// emscripten             : __EMSCRIPTEN__    (asm.js, webassembly)
// MinGW 32               : __MINGW32__
// MinGW - w64 32bit      : __MINGW32__
// MinGW - w64 64bit      : __MINGW64__
// 
// ### Checking compiler version ### 
// ### gcc ###
// __GNUC__(e.g. 5) and __GNUC_MINOR__(e.g. 1).
// To check that this is gcc compiler version 5.1 or greater :
//   #if defined(__GNUC__) && (__GNUC___ > 5 || (__GNUC__ == 5 && __GNUC_MINOR__ >= 1))
//      this is gcc 5.1 or greater
//   #endif
// Notice the check has to be : major > 5 || (major == 5 && minor >= 1)
// If you only do : major == 5 && minor >= 1 , it won’t work for version 6.0.
// ### clang  ###
// __clang_major__, __clang_minor__, __clang_patchlevel__
// ### MinGW ###
// MinGW (aka MinGW32) and MinGW-w64 32bit: __MINGW32_MAJOR_VERSION and __MINGW32_MINOR_VERSION
// MinGW - w64 64bit: __MINGW64_VERSION_MAJOR and __MINGW64_VERSION_MINOR
// ### Visual Studio ###
// VS                     _MSC_VER    _MSC_FULL_VER
// 1                      800
// 3                      900
// 4                      1000
// 4                      1020
// 5                      1100
// 6                      1200
// 6 SP6                  1200        12008804
// 7                      1300        13009466
// 7.1 (2003)             1310        13103077
// 8 (2005)               1400        140050727
// 9 (2008)               1500        150021022
// 9 SP1                  1500        150030729
// 10 (2010)              1600        160030319
// 10 (2010) SP1          1600        160040219
// 11 (2012)              1700        170050727
// 12 (2013)              1800        180021005
// 14 (2015)              1900        190023026
// 14 (2015 Upd 1)        1900        190023506
// 14 (2015 Upd 2)        1900        190023918
// 14 (2015 Upd 3)        1900        190024210
// 15 (2017 Upd 1 & 2)    1910        191025017
// 15 (2017 Upd 3 & 4)    1911
// 15 (2017 Upd 5)        1912
//
// Checking processor architecture
// ### gcc ###
// __i386__
// __x86_64__
// __arm__ && (__ARM_ARCH_5T__ || __ARM_ARCH_7A__)
// __powerpc64__
// __aarch64__

#ifdef _MSC_VER
	#if (_MSC_VER <= 1800) // VS 2013 or earlier
		#define __thisfunc__ __FUNCTION__
	#else // VS 2015 or elder
		#define __thisfunc__ __func__
	#endif
#else // not VS compiler
	#define __thisfunc__ __func__
#endif

#define __thisfile__ __FILE__
#define __thisline__ __LINE__

#if _WIN32 || _WIN64
	#define _OS_WINDOWS 1
	#if _WIN64
		#define BUILDx64
		#define SVFENGINEx64 1
	#else
		#define BUILDx32
		#define SVFENGINEx64 0
	#endif
#elif
	#define _OS_WINDOWS 0
#endif

//#if __GNUC__
//	#if __x86_64__ || __ppc64__
//		#define BUILDx64
//		#define SVFENGINEx64 1
//	#else
//		#define BUILDx32
//		#define SVFENGINEx64 0
//	#endif
//#endif

// https://software.intel.com/sites/landingpage/IntrinsicsGuide/

//	#define ENGINE_AVX512F	// enable AVX512F and older intrinsics source code : Visual Studio 2017 required
//	#define ENGINE_AVX2		// enable AVX2    and older intrinsics source code
//	#define ENGINE_AVX		// enable AVX     and older intrinsics source code
//	#define ENGINE_SSE42	// enable SSE42   and older intrinsics source code
	#define ENGINE_SSE41	// enable SSE41   and older intrinsics source code
//	#define ENGINE_SSSE3	// enable SSSE3   and older intrinsics source code
//	#define ENGINE_SSE3		// enable SSE3    and older intrinsics source code
//	#define ENGINE_SSE2		// enable SSE2    and older intrinsics source code
//	#define ENGINE_SSE		// enable SSE     and older intrinsics source code

// GO FROM TOP TO LOW

///////////////////////////////////////////////////////////////////  AVX512

#ifdef ENGINE_AVX512F
	#define ENGINE_AVX512F_ENABLED 1
	#ifndef ENGINE_AVX2
		#define ENGINE_AVX2
	#endif
#else
	#define ENGINE_AVX512F_ENABLED 0
#endif

///////////////////////////////////////////////////////////////////  AVX2

#ifdef ENGINE_AVX2
	#define ENGINE_AVX2_ENABLED 1
	#ifndef ENGINE_AVX
		#define ENGINE_AVX
	#endif
#else
	#define ENGINE_AVX2_ENABLED 0
#endif

///////////////////////////////////////////////////////////////////  AVX

#ifdef ENGINE_AVX
	#define ENGINE_AVX_ENABLED 1
	#ifndef ENGINE_SSE42
		#define ENGINE_SSE42
	#endif
#else
	#define ENGINE_AVX_ENABLED 0
#endif

///////////////////////////////////////////////////////////////////  SSE42

#ifdef ENGINE_SSE42
	#define ENGINE_SSE42_ENABLED 1
	#ifndef ENGINE_SSE41
		#define ENGINE_SSE41
	#endif
#else
	#define ENGINE_SSE42_ENABLED 0
#endif

///////////////////////////////////////////////////////////////////  SSE41

#ifdef ENGINE_SSE41
	#define ENGINE_SSE41_ENABLED 1
	#ifndef ENGINE_SSSE3
		#define ENGINE_SSSE3
	#endif
#else
	#define ENGINE_SSE41_ENABLED 0
#endif

///////////////////////////////////////////////////////////////////  SSSE3

#ifdef ENGINE_SSSE3
	#define ENGINE_SSSE3_ENABLED 1
	#ifndef ENGINE_SSE3
		#define ENGINE_SSE3
	#endif
#else
	#define ENGINE_SSSE3_ENABLED 0
#endif

///////////////////////////////////////////////////////////////////  SSE3

#ifdef ENGINE_SSE3
	#define ENGINE_SSE3_ENABLED 1
	#ifndef ENGINE_SSE2
		#define ENGINE_SSE2
	#endif
#else
	#define ENGINE_SSE3_ENABLED 0
#endif

///////////////////////////////////////////////////////////////////  SSE2

#ifdef ENGINE_SSE2
	#define ENGINE_SSE2_ENABLED 1
	#ifndef ENGINE_SSE
		#define ENGINE_SSE
	#endif
#else
	#define ENGINE_SSE2_ENABLED 0
#endif

///////////////////////////////////////////////////////////////////  SSE

#ifdef ENGINE_SSE
	#define ENGINE_SSE_ENABLED 1
#else
	#define ENGINE_SSE_ENABLED 0
#endif

// Align memory for SSE    : __declspec(align(16)) : 16 bytes
// Align memory for AVX    : __declspec(align(32)) : 32 bytes
// Align memory for AVX512 : __declspec(align(64)) : 64 bytes

#if SVFENGINEx64
	#if ENGINE_AVX512F_ENABLED
		#define _ALIGNOPTSZ 64 // AVX512F memory alignment (512-bit)
	#elif ENGINE_AVX_ENABLED
		#define _ALIGNOPTSZ 32 // AVX memory alignment (256-bit)
	#elif ENGINE_SSE_ENABLED
		#define _ALIGNOPTSZ 16 // SSE memory alignment (128-bit)
	#elif
		#define _ALIGNOPTSZ  8 // default memory alignment (64-bit)
	#endif
#else
	#define _ALIGNOPTSZ  4 // default memory alignment (32-bit)
#endif

#define _ALIGNOPT          __declspec(align(_ALIGNOPTSZ)) // align memory for SSE/AVX/AVX512 optimization
#define _ALIGNMALLOC(size) _aligned_malloc(size, _ALIGNOPTSZ)
#define _ALIGNFREE(ptr)    _aligned_free(ptr)


#endif // _DEFINES_CORE_H