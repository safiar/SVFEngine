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

#include <windows.h>
#include <windowsx.h>
#include <clocale>
#include <ctime>
#include <string>
#include <list>
#include <vector>
#include <deque>
#include <array>
#include <initializer_list>
#include <io.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <memory>
#include <map>

#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <comdef.h>
#include <share.h>

using std::wstring;
using std::list;
using std::vector;
using std::pair;
using std::deque;
using std::initializer_list;
using std::stof;
using std::runtime_error;
using std::move;
using std::forward;
using std::swap;
using std::thread;
using std::mutex;
using std::recursive_mutex;
using std::condition_variable;
using std::unique_lock;
using std::lock_guard;
using std::atomic;
using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;
using std::make_shared;
using std::static_pointer_cast;
using std::const_pointer_cast;
using std::dynamic_pointer_cast;

typedef  std::chrono::steady_clock                 monoclock;
typedef  std::chrono::system_clock::time_point     timepoint;

// -------------------------------------------- SAVFGAME PROJECT BASE INCLUDES

#include "color.h"
#include "defines.h"
#include "helper.h"
#include "mathematics.h"
#include "hashs.h"
#include "shared.h"

#endif // _STDAFX_H