// ----------------------------------------------------------------------- //
//
// MODULE  : types.h
//
// PURPOSE : Определение базовых типов
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _TYPES_BASE_H
#define _TYPES_BASE_H

#if _WIN32 || _WIN64
	#include <windows.h>
	#include <windowsx.h>
#endif

#include <new>
#include <clocale>
#include <ctime>
#include <string>
#include <list>
#include <vector>
#include <queue>
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

//#include <string.h>
#include <direct.h>
#include <bitset>
#include <cmath>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <functional>

#include <exception>
#include <stdexcept>

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
typedef  std::chrono::hours                        timeh;		// hours
typedef  std::chrono::minutes                      timem;		// minutes
typedef  std::chrono::seconds                      times;		// seconds
typedef  std::chrono::milliseconds                 timems;		// milliseconds
typedef  std::chrono::microseconds                 timemcs;		// microseconds
typedef  std::chrono::nanoseconds                  timens;		// nanoseconds

typedef  unsigned __int8      uint8;
typedef    signed __int8       int8;
typedef  unsigned __int16    uint16;
typedef	   signed __int16     int16;
typedef  unsigned __int32    uint32;
typedef	   signed __int32     int32;
typedef  unsigned __int64    uint64;
typedef	   signed __int64     int64;

typedef  unsigned __int32   error_t; // returning errors codes

using std::size_t;

#endif // _TYPES_BASE_H