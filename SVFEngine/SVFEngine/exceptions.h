// ----------------------------------------------------------------------- //
//
// MODULE  : exceptions.h
//
// PURPOSE : Обработка исключений
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2019)
//
// ----------------------------------------------------------------------- //

#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

//#include <iostream>
//#include <sstream>
//#include <string>

#include "defines_core.h"
#include "types.h"
#include "version.h"

#define ENGINE_EXCEPTION_INFOSIZE 0x1000

namespace SAVFGAME
{
	// SVFEngine v.%i.%i.%i.%i - time = <TIME>\r\n
	// EXCEPTION: <WHAT>\r\n
	// - fun <FUNCTION>() - line <LINE> - file <FILE>\r\n

	#define STR_VER    "SVFEngine v."
	#define STR_TIME   " - time = "
	#define STR_LINE   "\r\n"
	#define STR_E      "EXCEPTION: "
	#define STR_F      "- fun "
	#define STR_AT     "() - line "
	#define STR_IN     " - file "
	#define STR_VER_L  12 // STR_VER  "SVFEngine v."
	#define STR_TIME_L 10 // STR_TIME " - time = "
	#define STR_E_L    11 // STR_E    "EXCEPTION: "
	#define STR_F_L    6  // STR_F    "- fun "
	#define STR_AT_L   10 // STR_AT   "() - line "
	#define STR_IN_L   8  // STR_IN   " - file "
	#define STR_LINE_L 2  // STR_LINE "\r\n"
	#define TEMP_MAX   256

	#define EXCEPTION_LOG_TO_FILE true

	struct STATIC_EXCEPTION_DATA
	{
		struct _EXCEPTION_DATA
		{
			friend struct STATIC_EXCEPTION_DATA;
		public:
			char info [ENGINE_EXCEPTION_INFOSIZE];
			int  size { 0 };
			FILE * file;
		private:
			char temp [TEMP_MAX]; // temp buffer
			bool file_opened { false };
			bool first_time  { true };
		};

		static _EXCEPTION_DATA log;

	private:
		virtual void __DO_NOT_CREATE_OBJECT_OF_THIS_CLASS() = 0;

		//>> strlen()
		static int __fastcall getlen(char* p)
		{
			//return (int) strlen(p);

			int i = 0;
			while (*p) { i++; p++; }
			return i;
		}
		//>> ltoa()
		static void __fastcall i2char(int val)
		{
			//ltoa(val, temp, 10);
			_snprintf_s(log.temp, TEMP_MAX, "%i", val);
		}
		//>> memcpy()
		static void __fastcall copy(char* to, const char* from, int len)
		{
			//memcpy(to, from, len);

			for (int i=0; i<len; i++)
				to[i] = from[i];
		}
		//>> лог версии ПО и времени
		static void logfirsttime()
		{
			char * p = log.info;
			int len;

			copy(p, STR_VER, STR_VER_L);        // "SVFEngine v."
			p        += STR_VER_L;
			log.size += STR_VER_L;

			len = getlen(PRODUCT_VERSION_STR);  // "%i.%i.%i.%i"
			copy(p, PRODUCT_VERSION_STR, len);
			p        += len;
			log.size += len;

			copy(p, STR_TIME, STR_TIME_L);      // " - time "
			p        += STR_TIME_L;
			log.size += STR_TIME_L;

			auto time = std::chrono::system_clock::to_time_t
				( std::chrono::system_clock::now() );

			log.temp[0] = 0; // очистка
			ctime_s( log.temp, TEMP_MAX, &time );

			len = getlen(log.temp);             // "<TIME>\r\n"
			copy(p, log.temp, len);
			p        += len;
			log.size += len;

			if (EXCEPTION_LOG_TO_FILE) // open file 
			{
				//... open file

				if (len)
				{
					//... time name  (проверка допустимых символов) !
				}
				else
				{
					//... random name
				}
				
			}

			// ... make dump file ...
		}

	public:
		//>> 
		static void AddLogInfo(char* what, char* in_func, char* in_file, int at_line)
		{
			int log_size_add         = 0;
			int log_size_add_what    = 0;
			int log_size_add_in_func = 0;
			int log_size_add_in_file = 0;
			int log_size_add_at_line = 0;

			if (log.first_time)
			{
				log.first_time = false;
				logfirsttime();		
			}

			if (in_func) i2char(at_line); // <at_line> to <temp> str

			if (what)    log_size_add_what    = getlen(what);
			if (in_func) log_size_add_in_func = getlen(in_func);
			if (in_file) log_size_add_in_file = getlen(in_file);
			if (in_func) log_size_add_at_line = getlen(log.temp); // <at_line> only if <in_func>
			
			// пропуск namespace "SAVFGAME::<FUNCTION>"
			if (in_func)
			{
				char * p     = in_func;
				char * p_end = in_func + log_size_add_in_func - 2;
				int i = 0;

				while (p < p_end)
				{
					char a = *  p      ;
					char b = * (p + 1) ;

					if (a == ':' && b == ':')
					{
						in_func               = p + 2;
						log_size_add_in_func -= i + 2;
						break;
					}

					p++;
					i++;
				}
			}

			// сбросить путь, оставить только имя файла !
			if (in_file)
			{
				char * p = in_file + log_size_add_in_file ;
				int i = 0;

				while (p != in_file)
				{
					char a = *p;

					if (a == '\\' || a == '/')
					{
						in_file              = in_file + log_size_add_in_file - i + 1;
						log_size_add_in_file = i - 1;
						break;
					}

					i++;
					p--;
				}
			}

						             // EXCEPTION: <WHAT>\r\n
			if (what) log_size_add += STR_E_L + log_size_add_what + STR_LINE_L ;

			                         // - fun <FUNCTION>() - line <LINE> - file <FILE>\r\n
			if (in_func) log_size_add += STR_F_L + log_size_add_in_func + STR_AT_L +
                                                   log_size_add_at_line + STR_LINE_L ;

			if (in_func && in_file) log_size_add += STR_IN_L + log_size_add_in_file;

			if (log_size_add)        // + null-terminate
			if (log_size_add + log.size + 1 < ENGINE_EXCEPTION_INFOSIZE)
			{
				char * p = log.info + log.size;

				if (what)
				{
					copy(p, STR_E, STR_E_L);
					p += STR_E_L;

					copy(p, what, log_size_add_what);
					p += log_size_add_what;

					copy(p, STR_LINE, STR_LINE_L);
					p += STR_LINE_L;
				}

				if (in_func)
				{
					copy(p, STR_F, STR_F_L);
					p += STR_F_L;

					copy(p, in_func, log_size_add_in_func);
					p += log_size_add_in_func;

					copy(p, STR_AT, STR_AT_L);
					p += STR_AT_L;

					copy(p, log.temp, log_size_add_at_line);
					p += log_size_add_at_line;

					if (in_file)
					{
						copy(p, STR_IN, STR_IN_L);
						p += STR_IN_L;

						copy(p, in_file, log_size_add_in_file);
						p += log_size_add_in_file;
					}

					copy(p, STR_LINE, STR_LINE_L);
					p += STR_LINE_L;
				}

				(*(++p)) = 0; // null-terminate

				if (log.file_opened)
				{
					//... log to file
				}

				// new size
				log.size += log_size_add ;
			}
		};
		//>> 
		static void AddLogInfo(char* in_func, char* in_file, int at_line)
		{
			AddLogInfo(nullptr, std::move(in_func), std::move(in_file), std::move(at_line));
		};
		//>>
		static void CloseLogFile()
		{
			if (log.file_opened)
			{
				//... close file
			}
		}

		#undef STR_VER
		#undef STR_TIME
		#undef STR_LINE
		#undef STR_E
		#undef STR_F
		#undef STR_AT
		#undef STR_IN
		#undef STR_VER_L
		#undef STR_TIME_L
		#undef STR_LINE_L
		#undef STR_E_L
		#undef STR_F_L
		#undef STR_AT_L
		#undef STR_IN_L	
		#undef TEMP_MAX

	} ;

	//>> Rethrow type of exceptions
	class CExceptionRethrow : public std::runtime_error
	{
	public:
		explicit CExceptionRethrow(const std::string & message)
			: std::runtime_error(message.c_str())
		{ }

		explicit CExceptionRethrow(const char * message)
			: std::runtime_error(message)
		{ }
	};

	//>> Диагностика/лог исключения и переброс выше
	inline void EngineExceptionRethrow(char* in_func, char* in_file, int at_line )
	{
		try { std::rethrow_exception( std::current_exception() ); }

		catch (const CExceptionRethrow & e)
		{
			char * e_what = (char*) e.what();
			STATIC_EXCEPTION_DATA::AddLogInfo(in_func, in_file, at_line);
			throw;
		}

		catch (const std::exception & e)
		{
			char * e_what = (char*) e.what();
			STATIC_EXCEPTION_DATA::AddLogInfo(e_what, in_func, in_file, at_line);
			throw CExceptionRethrow(e_what);
		}
	};

	//>> Оконечная обработка исключения
	inline void EngineExceptionHandle(std::exception_ptr & e_ptr)
	{
		// try to re-throw exception if any
		try { if (e_ptr) { std::rethrow_exception(e_ptr); } }

		// catching SVFEngine type
		catch (const CExceptionRethrow & e)
		{
			auto e_what = e.what();
			printf("\n[[__HANDLE EXCEPTION__]]\n[[__LOG__]]\n%s", STATIC_EXCEPTION_DATA::log.info);
			
		}

		/*//////////////////////////////////////////

		// catching <runtime_error> type
		catch (const std::range_error & e) {}
		// catching <runtime_error> type
		catch (const std::overflow_error & e) {}
		// catching <runtime_error> type
		catch (const std::underflow_error & e) {}
		// catching <runtime_error> type
		catch (const std::ios_base::failure & e) {}
		// catching <runtime_error> type
		catch (const std::system_error & e) {}
		// catching <runtime_error> itself
		catch (const std::runtime_error & e) {}

		////////////////////////////////////////////

		// catching <logic_error> type
		catch (const std::invalid_argument & e) { }
		// catching <logic_error> type
		catch (const std::domain_error & e) {}
		// catching <logic_error> type
		catch (const std::length_error & e) {}
		// catching <logic_error> type
		catch (const std::out_of_range & e) {}
		// catching <logic_error> type
		catch (const std::future_error & e) {}
		// catching <logic_error> itself
		catch (const std::logic_error & e) {}

		////////////////////////////////////////////

		// catching <bad_typeid> itself
		catch (const std::bad_typeid & e) {}
		// catching <bad_cast> itself
		catch (const std::bad_cast & e) {}
		// catching <bad_weak_ptr> itself
		catch (const std::bad_weak_ptr & e) {}
		// catching <bad_function_call> itself
		catch (const std::bad_function_call & e) {}
		// catching <bad_alloc> type
		catch (const std::bad_array_new_length & e) {}
		// catching <bad_alloc> itself
		catch (const std::bad_alloc & e) {}
		// catching <bad_exception> itself
		catch (const std::bad_exception & e) {}

		//////////////////////////////////////////*/

		// catching BASE type
		catch (const std::exception & e)
		{
			auto e_what = e.what();
			printf("\n[[__HANDLE EXCEPTION__]]\n[[__WHAT__]]\n%s\n[[__LOG__]]\n%s", e_what, STATIC_EXCEPTION_DATA::log.info);
		}
	};

	#define __TRY__   try {
	#define __CATCH__ } catch (...) { EngineExceptionRethrow(__thisfunc__, __thisfile__, __thisline__); };

}

#endif // _EXCEPTIONS_H