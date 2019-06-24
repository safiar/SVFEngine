#ifndef _STDAFX_H
#define _STDAFX_H

//#define VC_EXTRALEAN

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef noexcept
#define _NOEXCEPT_ throw()
#else
#define _NOEXCEPT_ noexcept
#endif

//#ifndef _CRT_SECURE_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS
//#endif

//#ifndef _CRT_NON_CONFORMING_SWPRINTFS
//#define _CRT_NON_CONFORMING_SWPRINTFS
//#endif

#include "color.h"
#include "helper.h"
#include "mathematics.h"
#include "hashs.h"
#include "shared.h"

#endif // _STDAFX_H