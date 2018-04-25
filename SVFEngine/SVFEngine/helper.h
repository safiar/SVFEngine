// ----------------------------------------------------------------------- //
//
// MODULE  : helper.h
//
// PURPOSE : Вспомогательные классы и функции
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _HELPER_H
#define _HELPER_H

#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <bitset>
#include <cmath>
#include <queue>
#include <share.h>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <io.h>
#include <memory>
#include <future>
#include <functional>
#include "defines.h"

#define mem_relax   std::memory_order_relaxed
#define mem_seq_cst std::memory_order_seq_cst
#define mem_acquire std::memory_order_acquire
#define mem_consume std::memory_order_consume
#define mem_release std::memory_order_release
#define mem_acq_rel std::memory_order_acq_rel

#define _TO_ALL(cv,lk)  std::notify_all_at_thread_exit(cv,std::move(lk))
#define _ATM_ADD(a,x)	a.fetch_add(x, mem_seq_cst)
#define _ATM_SUB(a,x)	a.fetch_sub(x, mem_seq_cst)
#define _ATM_LD(a)      a.load(mem_seq_cst)
#define _ATM_ST(a,x)    a.store(x, mem_seq_cst)
#define _ATM_0(a)       a.store(false, mem_seq_cst)
#define _ATM_1(a)       a.store(true, mem_seq_cst)
#define _CV_ALL(cv)     cv.notify_all()
#define _CV_ONE(cv)     cv.notify_one()
#define _CV_WAIT(cv,lk) cv.wait(lk)
#define _RLX_ST(a,x);   a.store(x, mem_relax)
#define _RLX_LD(a)      a.load(mem_relax)
#define _RLX_0(a)       a.store(false, mem_relax)
#define _RLX_1(a)       a.store(true, mem_relax)
#define _RLX_ADD(a,x)	a.fetch_add(x, mem_relax)
#define _RLX_SUB(a,x)	a.fetch_sub(x, mem_relax)

typedef std::unique_lock <std::mutex>  ulock;
typedef std::lock_guard  <std::mutex>  glock;

typedef std::unique_lock <std::recursive_mutex>  rulock;
typedef std::lock_guard  <std::recursive_mutex>  rglock;

//std::this_thread::sleep_for(std::chrono::microseconds(1));
//nanoseconds, microseconds. milliseconds, seconds, minutes, hours
//std::chrono::high_resolution_clock::now();
//recursive_mutex, timed_mutex, recursive_timed_mutex

typedef  unsigned __int8      uint8;
typedef    signed __int8       int8;
typedef  unsigned __int16    uint16;
typedef	   signed __int16     int16;
typedef  unsigned __int32    uint32;
typedef	   signed __int32     int32;
typedef  unsigned __int64    uint64;
typedef	   signed __int64     int64;

using std::size_t;

namespace SAVFGAME
{
	enum eThreadCode // THREAD/DATA/POOL returning codes
	{
		TC_NONE,
		////////////////
		TC_SUCCESS,			// успешное исполнение
		TC_ALREADY,			// THREADDATA уже закрыта/открыта
		TC_CLOSED,			// THREADDATA закрыта
		TC_IN_PROGRESS,		// поток ещё выполняется
		TC_FREE_PLACE,		// поток уже свободен
		TC_ERROR,			// аварийный выход
		TC_NO_PLACE,		// не удалось запустить поток т.к. место занято
		TC_RESULT_NONE,		// никаких результатов не предоставляется
		TC_RESULT_WAIT,		// результат ещё не готов
		TC_RESULT_LOCK,		// результат больше недоступен (готовится удаление)
		TC_WAIT_NONE,		// нет причин для ожидания
		TC_WAIT_OPEN,		// произведено ожидание открытия
		TC_WAIT_RESULT,		// произведено ожидание результата
		TC_WAIT_THREAD,		// произведено ожидание окончания потока
		TC_WAIT_GRAB,		// ожидание окончания потока не произведено - необходимо FreeResult()
		////////////////
		TC_POOL_UID_MISMATCH,	// несоответствие экземляра THREADPOOL : это не тот POOL, принцесса в другом замке !
		TC_POOL_ID_MISMATCH,	// несоответствие итерации  THREADPOOL : производился вызов Close() - все задачи были сброшены
		TC_POOL_TASK_NOT_FOUND,	// не найдена соответствующая задача - вероятно, она уже была удалена
		TC_POOL_TASK_NO_REMOVE,	// нельзя удалить задачу - она уже выполняется или уже выполнена
		TC_POOL_TASK_REMOVED,   // задачу уже отметили на удаление
		////////////////
		TC_ENUM_MAX
	};

	//>> [Посредник в управлении потоками | Инструкция по использованию]
	//>> 
	//>>    THREADDATA  myThread;
	//>>    MYCLASS     myClass;     // MYCLASS содержит некоторую функцию ::MYFUNC(), которую надо исполнить в поток
	//>>
	//>>    myThread (	ID,                     // метка int32 (не обязательно)
	//>>                & MYCLASS::MYFUNC,		// ptr на исполняемую функцию
	//>>				& myClass,				// ptr на объект класса
	//>>				...  );					// ... (если есть) аргументы исполняемой функции
	//>>				
	//>>	В случае успешного запуска потока, возвращает TC_SUCCESS_RUN
	//>>    
	
	enum eThreadRunMethod
	{
		TRM_THREAD,		// run via std::thread
		TRM_ASYNC,		// run via std::async  (std::launch::async)

		TRM_DEFAULT = TRM_THREAD // run this if you don't care about
	};
	
	class THREADDATA
	{
	////////////////////////////////////////////////////////////////////////////////////////////
		class CFutureHolderT // virtual base class for holding <future>
		{
		public:
			CFutureHolderT() {};
			virtual ~CFutureHolderT() {};
		};
		template<class RESULT>
		class CFutureHolder : public CFutureHolderT // actual class holding <future>
		{
		public:
			CFutureHolder() : CFutureHolderT() {};
			~CFutureHolder() { if (result.valid()) result.get(); };
			std::future<RESULT> result;
		};
	////////////////////////////////////////////////////////////////////////////////////////////
	public:
		THREADDATA(const THREADDATA& src)            = delete;
		THREADDATA(THREADDATA&& src)                 = delete;
		THREADDATA& operator=(THREADDATA&& src)      = delete;
		THREADDATA& operator=(const THREADDATA& src) = delete;
	public:
		THREADDATA()                { Init(true);       }     // Ловится довольно странный баг, если detach() поток и``
		THREADDATA(bool use_detach) { Init(use_detach); }     // ``очень быстро пересоздавать THREADDATA.
		~THREADDATA()               { Close(); Cooldown(); }  // Причины не ясны. Лечится малой задержкой деструктора.
	private:
		//>> Запуск
		void Init(bool use_detach)
		{
			Default();
			_ATM_ST(_detach, use_detach);
			_ATM_ST(_closed, false);
			_ATM_ST(working_via_result, false);
			_ATM_ST(waiting_for_result, false);
			_ATM_ST(waiting_for_grab, false);
			mem_run_method = TRM_THREAD;
			future_holder  = nullptr;
		}

	public:
		//>> Закрытие THREADDATA :: return TC_SUCCESS / TC_ALREADY :: auto FreeResult()
		eThreadCode Close()
		{
			glock stop_others_if_any (global_block);

			if (_ATM_LD(_closed)) return TC_ALREADY;

			FreeResultProc();
			WaitThread(false);
			future_holder = nullptr; // free <future> from previous TRM_ASYNC call (if any)

			if (!_ATM_LD(_detach) && !_ATM_LD(_free))
			{
				if (IsThreadJoinable())
				{
					JoinThread();	// объединяем
					Default();		// освобождаем место
				}
				else _MBM(ERROR_THREAD); // Something going wrong... check the code!
			}
			_ATM_1(_closed);

			return TC_SUCCESS;
		}
		//>> Открытие THREADDATA :: return TC_SUCCESS / TC_ALREADY
		eThreadCode Open(bool use_detach)
		{
			glock stop_others_if_any (global_block);

			bool is_closed;

			{
				glock block_who_wait_for_opening (opening_block);
				is_closed = _ATM_LD(_closed);

				if (is_closed)
				{
					_ATM_ST(_detach, use_detach);
					_ATM_0(_closed);
				}
			}

			if (is_closed)
			{
				_CV_ALL(opening_cv); // сообщаем в WaitOpen() что открыто
				return TC_SUCCESS;
			}
			else
			{
				_CV_ALL(opening_cv); // сообщаем в WaitOpen() что открыто
				return TC_ALREADY;
			}
		}
		//>> Открытие THREADDATA :: return TC_SUCCESS / TC_ALREADY
		eThreadCode Open()
		{
			return Open(_ATM_LD(_detach));
		}

	protected:
		//>> Сброс основного состояния
		void Default()
		{
			_ATM_1(_free);
			_ATM_1(_finished);
			_ATM_ST(_ID, MISSING);
		}
		//>> Добавление нового потока
		bool Add()
		{
			if (!_ATM_LD(_detach) &&	// Завершившийся неоткрепленный поток
				!_ATM_LD(_free)   &&	// .
				 _ATM_LD(_finished))	// .
			{
				if (_thread.joinable())
				{
					_thread.join(); // объединяем
					Default();	    // освобождаем место
				}
				else _MBM(ERROR_THREAD); // Something going wrong... check the code!
			}

			if (_ATM_LD(_free))		// место свободно -> записываем
			{
				_ATM_0(_finished);
				_ATM_0(_free);
				 return true;		// сообщаем об успешной записи
			}
			else return false;		// сообщаем, что место ещё занято
		}
		//>> Сообщение о завершении потока
		void Finish()
		{
			_ATM_1(_finished);
		}
		//>> Освобождает поток от ожидания, что результат заберут :: тело процедуры без global_block
		void FreeResultProc()
		{
			if (_ATM_LD(working_via_result))
			{
				{
					glock time_to_tell_to_thread_be_free (waiting_for_grab_block);
					_ATM_0(waiting_for_grab);
				}
				_CV_ALL(waiting_for_grab_cv);
			}
		}
		//>> test joinable()
		bool IsThreadJoinable()
		{
			switch (mem_run_method)
			{
			case TRM_ASYNC:  return true;
			case TRM_THREAD: break;
			}

			return _thread.joinable();
		}
		//>> wait & join()
		void JoinThread()
		{
			switch (mem_run_method)
			{
			case TRM_THREAD: _thread.join();    return;
			case TRM_ASYNC:  WaitThread(false); return;
			}
		}
		//>> 
		void Cooldown()
		{
			std::this_thread::yield();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

	public:
		//>> Возврат потока :: if (wait) будет ждать окончания (даже для _detached) :: if (wait) -> auto FreeResult() 
		eThreadCode Join(bool wait)
		{
			glock stop_others_if_any (global_block);

			if (_ATM_LD(_closed)) return TC_CLOSED;

			if (_ATM_LD(_detach) && !_ATM_LD(_free))
			{
				if (!wait)				      return TC_IN_PROGRESS;
				else { FreeResultProc();
					   WaitThread(false);	  return TC_SUCCESS; }
			}
			if (_ATM_LD(_free))               return TC_FREE_PLACE;
			if (!_ATM_LD(_finished) && !wait) return TC_IN_PROGRESS;
			if (IsThreadJoinable())
			{
				FreeResultProc();
				JoinThread();     Default();  return TC_SUCCESS;
			}
			else _MBM(ERROR_THREAD); // Something going wrong... check the code!

			return TC_ERROR;
		}
		//>> Проверка ID != MISSING (-1)
		bool CheckID()
		{
			return (_NOMISS(_ATM_LD(_ID)));
		}
		//>> Взятие метки потока (значение default: MISSING (-1))
		int32 GetID()
		{
			return _ATM_LD(_ID);
		}
		//>> Сообщает свободно ли место
		bool IsFree()
		{
			return _ATM_LD(_free);
		}
		//>> Сообщает завершён ли поток
		bool IsFinished()
		{
			return _ATM_LD(_finished);
		}
		//>> Сообщает открепляет ли THREADDATA потоки
		bool IsDetach()
		{
			return _ATM_LD(_detach);
		}
		//>> Сообщает закрыта ли THREADDATA
		bool IsClosed()
		{
			return _ATM_LD(_closed);
		}
		//>> Сообщает ведётся ли обработка возвращаемого значения
		bool IsExpectAnyResult()
		{
			return _ATM_LD(working_via_result);
		}
		//>> Сообщает статус ожидания результата (true если ещё не получен)
		bool IsWaitingForResult()
		{
			return _ATM_LD(waiting_for_result);
		}

	public:
		template<class RESULT>
		//>> Копирует результат работы функции на адрес <save_out_location>
		eThreadCode GrabResult(RESULT & ref_to_save_out_location)
		{
			glock get_place_in_queue (grab_block);

			if (!_ATM_LD(working_via_result)) return TC_RESULT_NONE;
			if (_ATM_LD(waiting_for_result))  return TC_RESULT_WAIT;
			if (!_ATM_LD(waiting_for_grab))   return TC_RESULT_LOCK;

			ref_to_save_out_location = *(reinterpret_cast<RESULT*>(ptr_to_result));
			return TC_SUCCESS;
		}
		template<class RESULT>
		//>> Копирует результат работы функции на адрес <save_out_location>
		eThreadCode GrabResult(RESULT * ptr_to_save_out_location)
		{
			glock get_place_in_queue (grab_block);

			if (!_ATM_LD(working_via_result)) return TC_RESULT_NONE;
			if (_ATM_LD(waiting_for_result))  return TC_RESULT_WAIT;
			if (!_ATM_LD(waiting_for_grab))   return TC_RESULT_LOCK;

			(*ptr_to_save_out_location) = *(reinterpret_cast<RESULT*>(ptr_to_result));
			return TC_SUCCESS;
		}
		//>> Освобождает поток от ожидания, что результат заберут
		void FreeResult()
		{
			glock stop_others_if_any (global_block);

			FreeResultProc();
		}
		//>> Отправляет вызвавшего в ожидание, пока функция потока не вернёт результат
		bool WaitResult()
		{
			if (_ATM_LD(waiting_for_result))
			{
				ulock lock(waiting_for_result_block);
				while (_ATM_LD(waiting_for_result))
					{ _CV_WAIT(waiting_for_result_cv, lock); }
				return true;
			}
			return false;
		}
		//>> Отправляет вызвавшего в ожидание до окончания потока (_finished) :: NOTE _ if you forget to FreeResult() you may stuck here
		bool WaitThread(bool notify_me_in_queue)
		{
			if (!_ATM_LD(_finished))
			{
				if (notify_me_in_queue)
				{
					ulock lock(finish_working_block_one);
					while (!_ATM_LD(_finished))
						{ _CV_WAIT(finish_working_cv_one, lock); }	
				}
				else
				{
					ulock lock(finish_working_block_all);
					while (!_ATM_LD(_finished))
						{ _CV_WAIT(finish_working_cv_all, lock); }
				}
				return true;
			}
			return false;
		}
		//>> Отправляет вызвавшего в ожидание, пока THREADDATA не станет открыта
		bool WaitOpen()
		{
			if (_ATM_LD(_closed))
			{
				ulock lock(opening_block);
				while (_ATM_LD(_closed))
					{ _CV_WAIT(opening_cv, lock); }
				return true;
			}
			return false;
		}
		//>> Автоожидание [ WaitResult / WaitThread / WaitOpen ] :: NOTE _ here is no stuck with waiting result (like at WaitThread())
		eThreadCode Wait(bool notify_me_in_queue)
		{
			if (_ATM_LD(_closed))
			{
				WaitOpen();
				return TC_WAIT_OPEN;
			}
			else if (_ATM_LD(waiting_for_result))
			{
				WaitResult();
				return TC_WAIT_RESULT;
			}
			else if (!_ATM_LD(_finished))
			{
				if (_ATM_LD(waiting_for_grab)) return TC_WAIT_GRAB;	// fixing waiting result stuck
				else WaitThread(notify_me_in_queue);
				return TC_WAIT_THREAD;
			}

			return TC_WAIT_NONE;
		}

	protected:
		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Оболочка с авто-уведомлением о завершении потока (игнорирует возвращаемое значение)
		void ThreadShell(TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			{
				glock block_who_wait_separate  (finish_working_block_all);
				glock block_who_wait_in_queue  (finish_working_block_one);
				glock wait_detaching_by_runner (detach_block);

				(class_ptr->*class_function)(args...);

				if (_ATM_LD(_detach))
					 Default();
				else Finish();
			}

			_CV_ONE (finish_working_cv_one); // сообщаем в WaitThread()
			_CV_ALL (finish_working_cv_all); // сообщаем в WaitThread()
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Оболочка с авто-уведомлением о завершении потока (для не возвращающей ничего процедуры, с проверкой)
		void ThreadShellVoid(TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			{
				glock block_who_wait_separate  (finish_working_block_all);
				glock block_who_wait_in_queue  (finish_working_block_one);
				glock wait_detaching_by_runner (detach_block);

				auto func = std::mem_fn(class_function);							// проверка на ошибку времени компиляции
				static_assert (														// 
					std::is_void < decltype(func(*class_ptr, args...)) > ::value,	//
					"ThreadShellVoid : TFUNC returning type is not VOID" );			//

			//	func(*class_ptr, std::forward<TARGS>(args)...);
				(class_ptr->*class_function)(args...);

				if (_ATM_LD(_detach))
					 Default();
				else Finish();
			}

			_CV_ONE (finish_working_cv_one); // сообщаем в WaitThread()
			_CV_ALL (finish_working_cv_all); // сообщаем в WaitThread()
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Оболочка с авто-уведомлением о завершении потока (для возвращающей значение функции, с проверкой)
		void ThreadShellRet(TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			auto func = std::mem_fn(class_function);							// проверка на ошибку времени компиляции
			static_assert (														// .
				!std::is_void < decltype(func(*class_ptr, args...)) > ::value,	// .
				"ThreadShellRet : TFUNC must return any value");				// .	
			{
				decltype(func(*class_ptr, args...)) value;						// объявляем возвращаемую переменную

				glock block_who_wait_separate (finish_working_block_all);
				glock block_who_wait_in_queue (finish_working_block_one);
				{
					glock block_who_wait_for_result (waiting_for_result_block);
					glock wait_detaching_by_runner  (detach_block);

					auto //std::future < std::result_of<decltype(func)(decltype(*class_ptr), TARGS...)>::type  >
						ret = std::async (
								std::launch::deferred,
								func,
								std::ref(*class_ptr),
								args... );

					value = ret.get();				// ждём и готовим результат к передаче
					ptr_to_result = (void*)&value;	// .
					_ATM_0(waiting_for_result);		// сообщаем в WaitResult() что результат готов
				}									// .
				_CV_ALL(waiting_for_result_cv);		// .

				if (_ATM_LD(waiting_for_grab))	// ждём, пока заберут результат и освободят нас
				{
					ulock lock(waiting_for_grab_block);
					while (_ATM_LD(waiting_for_grab))
						{ _CV_WAIT(waiting_for_grab_cv, lock); }
				}
				glock wait_if_any_grabber_still_in_process (grab_block); // нас могли отпустить, но кто-то ещё в процессе

				//_ATM_0(working_via_result);

				if (_ATM_LD(_detach))
					 Default();
				else Finish();
			}
			_CV_ONE (finish_working_cv_one); // сообщаем в WaitThread()
			_CV_ALL (finish_working_cv_all); // сообщаем в WaitThread()
		}
		
		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск оболочки в поток
		eThreadCode RunThreadFunc(bool have_ret, int32 ID, eThreadRunMethod method, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			glock stop_others_if_any (global_block);

			if (_ATM_LD(_closed)) return TC_CLOSED;
			if (!Add())           return TC_NO_PLACE;

			_ATM_ST(_ID, ID);

			future_holder = nullptr;	 // free <future> from previous TRM_ASYNC call (if any)
			mem_run_method = method;

			if (have_ret) { _ATM_1(working_via_result);
							_ATM_1(waiting_for_result);
							_ATM_1(waiting_for_grab);   }
			else		  { _ATM_0(working_via_result);
							_ATM_0(waiting_for_result);
							_ATM_0(waiting_for_grab);   }

			if (method == TRM_THREAD)
			{
				glock stop_thread_for_detaching (detach_block);

				_thread = std::thread ( class_function,
										class_ptr,
										std::forward<TARGS>(args)... );

				if (_ATM_LD(_detach)) _thread.detach();
			}
			else // TRM_ASYNC :: We need to hold <future> from std::async until next <Run> or <Close> call
			{
				auto func = std::mem_fn(class_function);

				std::shared_ptr<               CFutureHolder< decltype(func(*class_ptr, args...)) >> future_ptr;
				future_ptr = std::make_shared< CFutureHolder< decltype(func(*class_ptr, args...)) >> ();

				(*future_ptr).result = std::async ( std::launch::async,
													func,
													std::ref(*class_ptr),
													std::forward<TARGS>(args)... );
				future_holder = future_ptr;
			}

			return TC_SUCCESS;
		}

	public:
		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск функции в поток с игнорированием возвращаемого значения
		eThreadCode RunThreadIgnore(eThreadRunMethod method, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThreadFunc( false,
				MISSING,
				method,
			  & THREADDATA::ThreadShell < TFUNC, TCLASS, TARGS... >,
				this,
				class_function,
				class_ptr,
				args... );
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск функции в поток (с ID) с игнорированием возвращаемого значения
		eThreadCode RunThreadIgnore(int32 ID, eThreadRunMethod method, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThreadFunc( false,
				ID,
				method,
			  & THREADDATA::ThreadShell < TFUNC, TCLASS, TARGS... >,
				this,
				class_function,
				class_ptr,
				args... );
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск функции в поток с получением возвращаемого значения :: do not forget to FreeResult()
		eThreadCode RunThreadRet(eThreadRunMethod method, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThreadFunc ( true,
				MISSING,
				method,
			  & THREADDATA::ThreadShellRet < TFUNC, TCLASS, TARGS... >,
				this,
				class_function,
				class_ptr,
				args... );
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск функции в поток (с ID) с получением возвращаемого значения :: do not forget to FreeResult()
		eThreadCode RunThreadRet(int32 ID, eThreadRunMethod method, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThreadFunc ( true,
				ID,
				method,
			  & THREADDATA::ThreadShellRet < TFUNC, TCLASS, TARGS... >,
				this,
				class_function,
				class_ptr,
				args... );
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск функции в поток
		eThreadCode RunThread(eThreadRunMethod method, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThreadFunc ( false,
				  MISSING,
				  method,
				& THREADDATA::ThreadShellVoid < TFUNC, TCLASS, TARGS... >,
				  this,
				  class_function,
				  class_ptr,
				  args... );
		}
		
		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск функции в поток (с ID)
		eThreadCode RunThread(int32 ID, eThreadRunMethod method, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThreadFunc ( false,
				  ID,
				  method,
				& THREADDATA::ThreadShellVoid < TFUNC, TCLASS, TARGS... >,
				  this,
				  class_function,
				  class_ptr,
				  args... );
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> RunThread() operator()
		eThreadCode operator()(TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{		
			return RunThread ( TRM_DEFAULT,
							   class_function,
							   class_ptr,
							   args... );
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> RunThread() operator() с идентификатором
		eThreadCode operator()(int32 ID, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThread ( ID,
							   TRM_DEFAULT,
							   class_function,
							   class_ptr,
							   args... );
		}

	protected:
		std::thread			_thread;	// поток исполнения
		std::atomic<bool>	_free;		// статус свободности THREADDATA
		std::atomic<bool>	_finished;  // статус завершённости потока
		std::atomic<bool>	_closed;	// THREADDATA статус: закрыт / открыт для пользования
		std::atomic<int32>	_ID;		// можно поставить вспомогательную метку
		std::atomic<bool>	_detach;	// настройка : будем ли мы откреплять поток ?
	/////////
		std::mutex			global_block;	// ставит в ожидание другие любые параллельные вызовы
		std::mutex			detach_block;	// ставит в ожидание начало исполнения потока до открепления
		std::mutex			grab_block;		// ставит в ожидание пока кто-то берёт результат
	/////////
		std::condition_variable	 opening_cv;				// организует ожидание до открытия THREADDATA
		std::mutex				 opening_block;				// организует ожидание до открытия THREADDATA
		std::condition_variable  finish_working_cv_all;		// организует ожидание до окончания потока - для всех по любой причине
		std::condition_variable  finish_working_cv_one;		// организует ожидание до окончания потока - для одного желающего занять место
		std::mutex				 finish_working_block_all;	// организует ожидание до окончания потока - для всех по любой причине
		std::mutex				 finish_working_block_one;	// организует ожидание до окончания потока - для одного желающего занять место
		std::condition_variable  waiting_for_result_cv;		// организует ожидание до получения результата от потока
		std::mutex				 waiting_for_result_block;  // организует ожидание до получения результата от потока
		std::condition_variable  waiting_for_grab_cv;		// организует ожидание потока до освобождения результата grabber'ом
		std::mutex				 waiting_for_grab_block;	// организует ожидание потока до освобождения результата grabber'ом
	/////////
		std::atomic<bool>		 working_via_result;	// метка схемы работы (работаем над результатом или нет)
		std::atomic<bool>		 waiting_for_result;	// флаг ожидания результата
		std::atomic<bool>		 waiting_for_grab;		// флаг ожидания, что результат заберут и отпустят поток
		void *					 ptr_to_result;			// указатель на объект с результатом
	/////////
		eThreadRunMethod				 mem_run_method;	// какой конкретно функцией запускался поток
		std::shared_ptr<CFutureHolderT>	 future_holder;		// содержит future от запуска оболочки методом TRM_ASYNC
	};

	/////////////////////////////////////////////////////////////////////

	template <class T, typename SZ>
	struct TBUFFER // Оболочка для содержания простых буферов
	{
		TBUFFER() : buf(nullptr), count(0) {};
		~TBUFFER() { Close(); }	
		//>> <false> если buf == nullptr ; иначе <true>
		bool Check()
		{
			if (buf == nullptr) return false;
			return true;
		}
		//>> стирает старый и создаёт новый буфер в количестве num элементов
		void Create(SZ num)
		{
			Close();
			count = num;
			buf = new T [count];
		}
		//>> memset(NULL)
		void Clear()
		{
			ZeroMemory(buf, sizeof(T)*count);
		}
		//>> memset(val)
		void Clear(byte val)
		{
			memset(buf, val, sizeof(T)*count);
		}
		//>> удаляет всё
		void Close()
		{
			_DELETEN(buf);
			count = 0;
		}
		/////////
		T& operator[](size_t idx) { return buf[idx]; }
		T* operator()()           { return buf; }
		/////////
		T *	buf;		// буфер данных
		SZ	count;		// количество данных
	};

	template <class T, typename SZ>
	struct TBUFFERNEXT : public TBUFFER < T, SZ >
	{
		TBUFFERNEXT() { };
		~TBUFFERNEXT() { };

		SZ cur;
		SZ prev;

		void SetNext(const T & value)
		{
			if (count < 2) { _MBM(ERROR_BUFLIMIT); return; }

			SZ current = cur + 1;
			if (current >= count)
				current = 0;

			prev = cur;
			cur  = current;

			buf[current] = value;
		}
		void Reset(const T & value)
		{
			for (SZ i=0; i<count; i++)
				buf[i] = value;
		}
		
		bool Equal()   { return ( buf[cur] == buf[prev] ); }
		T & Current()  { return buf[cur];  }
		T & Previous() { return buf[prev]; }
	};

/*
	template <class T>
	struct DATAP // Вспомогательная оболочка к shared_ptr
	{
		DATAP() {};
		DATAP(bool create)            { if (create) Create(); };
		DATAP(std::shared_ptr<T> ptr) { data = ptr; };
		/////////
		T * get() { return data.get(); }
		/////////
		//>> <true> если есть *data ; <false> иначе
		bool Check()
		{
			if (data == nullptr) return false;
			return true;
		}
		//>> исполняет data->Close()
		void Close(bool delete_data)
		{
			if (data != nullptr) data->Close();
			if (delete_data) Delete();
		}
		//>> исполняет data = nullptr
		void Delete()
		{
			data = nullptr;
		}
		//>> исполняет data = make_shared <T> () , но только если data == nullptr
		bool Create()
		{
			if (data != nullptr) return false;
			data = std::make_shared <T> ();
			return true;
		}
		//>> delete + create 
		void ReCreate(bool close_data)
		{
			if (close_data) Close(1);
			else Delete();
			data = std::make_shared <T> ();
		}
		////
		                T* operator->() { return data.get(); }
		std::shared_ptr<T> operator()() { return data; }
		bool operator==(std::shared_ptr<T> ptr) { return (data == ptr); }
		bool operator!=(std::shared_ptr<T> ptr) { return (data != ptr); }
		////
		std::shared_ptr<T> data;
	};

	template <class T>
	struct VECDATAP // Вспомогательная оболочка для управления массивом shared_ptr
	{
		std::vector<DATAP<T>> v;

		VECDATAP()  { };
		~VECDATAP() { Delete(1); };
		size_t size()              { return v.size(); }
		void emplace_back()        { Add(1); }
		void emplace_back_create() { AddCreate(1); }
		void pop_back()            { v.pop_back(); }
		void reserve(size_t count) { v.reserve(count); }
		DATAP<T>& last()           { return v[v.size()-1]; }
		size_t last_id()           { return   v.size()-1 ; }

		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data)
		{
			// Deleting
			for (auto & i : v) i.Delete();
			if (erase_data)
				v.erase(v.begin(), v.end());
		}
		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data, size_t from_id)
		{
			size_t i, size = v.size();	if (!size) return;
										if (from_id >= size) return;
			// Deleting
			for (i=from_id; i<size; i++) v[i].Delete();
			if (erase_data)
				v.erase(v.begin() + from_id, v.end());
		}
		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data, size_t from_id, size_t to_id)
		{
			size_t i, size = v.size();	if (!size) return;
										if (from_id >= size || to_id > size || to_id <= from_id) return;
			// Deleting
			for (i=from_id; i<to_id; i++) v[i].Delete();
			if (erase_data)
				v.erase(v.begin() + from_id, v.end() + to_id);
		}
		//>> Задаёт новый вектор в count мест (все указатели пустые)
		void New(size_t count)
		{
			// Deleting
			Delete(1);

			// Reserving
			v.reserve(count);
			for (size_t i = 0; i<count; i++) v.emplace_back(DATAP<T>());
		}
		//>> Задаёт новый вектор в count мест (все указатели ведут к новым объектам) :: abstract classes restricted via this function
		void Create(size_t count)
		{
			// Deleting + Reserving
			New(count);

			// Creating
			for (auto & i : v) i.Create();
		}		
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data)
		{
			// Closing
			for (auto & i : v) i.Close(0);
			if (delete_data)
				Delete(erase_data);
		}
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data, size_t from_id)
		{
			size_t i, size = v.size();	if (!size) return;
										if (from_id >= size) return;
			// Closing
			for (i=from_id; i<size; i++) v[i].Close(0);
			if (delete_data)
				Delete(erase_data, from_id);
		}
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data, size_t from_id, size_t to_id)
		{
			size_t i, size = v.size();	if (!size) return;
										if (from_id >= size || to_id > size || to_id <= from_id) return;
			// Closing
			for (i=from_id; i<to_id; i++) v[i].Close(0);
			if (delete_data)
				Delete(erase_data, from_id, to_id);
		}
		//>> Добавляет count новых мест в конец (указатели пустые)
		void Add(size_t count)
		{
			for (size_t i=0; i<count; i++) v.emplace_back(DATAP<T>());
		}
		//>> Добавляет count новых мест в конец (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void AddCreate(size_t count)
		{	
			size_t size   = v.size();
			size_t target = size + count;
			Add(count);
			for (size_t i = size; i<target; i++)
				v[i].Create();
		}
		//>> Увеличивает до count новых мест (указатели пустые)
		void Set(size_t count)
		{
			size_t size = v.size();
			if (count > size) Add(count - size);
		}
		//>> Увеличивает до count новых мест (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void SetCreate(size_t count)
		{
			size_t size = v.size();
			if (count > size) AddCreate(count - size);
		}

		      DATAP<T>& operator[](size_t idx)       { return v[idx]; }
		const DATAP<T>& operator[](size_t idx) const { return v[idx]; }
		void operator()(size_t num_to_create)  { Create(num_to_create); }
	};
//*/

	template <class T>
	struct VECDATAP // Вспомогательная оболочка для управления массивом shared ptr
	{
	protected:
		std::vector<std::shared_ptr<T>> v;
	public:
		VECDATAP()  { };
		~VECDATAP() { Delete(1); };
		size_t size()              { return v.size(); }
		void emplace_back()        { Add(1); }
		void emplace_back_create() { AddCreate(1); }
		void pop_back()            { v.pop_back(); }
		void reserve(size_t count) { v.reserve(count); }
		std::shared_ptr<T> last()  { return v[v.size() - 1]; }
		size_t last_id()           { return   v.size() - 1;  }

		auto begin() -> decltype(v.begin())
		{
			return v.begin();
		}
		auto end() -> decltype(v.end())
		{
			return v.end();
		}

		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data)
		{
			// Deleting
			for (auto & i : v) i = nullptr;
			if (erase_data)
				v.erase(v.begin(), v.end());
		}
		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data, size_t from_id)
		{
			size_t i, size = v.size();	if (!size) return;
										if (from_id >= size) return;
			// Deleting
			for (i=from_id; i<size; i++) v[i] = nullptr;
			if (erase_data)
				v.erase(v.begin() + from_id, v.end());
		}
		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data, size_t from_id, size_t to_id)
		{
			size_t i, size = v.size();	if (!size) return;
										if (from_id >= size || to_id > size || to_id <= from_id) return;
			// Deleting
			for (i=from_id; i<to_id; i++) v[i] = nullptr;
			if (erase_data)
				v.erase(v.begin() + from_id, v.begin() + to_id);
		}
		//>> Задаёт новый вектор в count мест (все указатели пустые)
		void New(size_t count)
		{
			// Deleting
			Delete(1);

			// Reserving
			v.reserve(count);
			for (size_t i = 0; i<count; i++) v.emplace_back(nullptr);
		}
		//>> Задаёт новый вектор в count мест (все указатели ведут к новым объектам) :: abstract classes restricted via this function
		void Create(size_t count)
		{
			// Deleting + Reserving
			New(count);

			// Creating
			for (auto & i : v) i = std::make_shared<T>();
		}		
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data)
		{
			// Closing
			for (auto & i : v) if (i != nullptr) i->Close();
			if (delete_data)
				Delete(erase_data);
		}
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data, size_t from_id)
		{
			size_t i, size = v.size();	if (!size) return;
										if (from_id >= size) return;
			// Closing
			for (i=from_id; i<size; i++) if (v[i] != nullptr) v[i]->Close();
			if (delete_data)
				Delete(erase_data, from_id);
		}
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data, size_t from_id, size_t to_id)
		{
			size_t i, size = v.size();	if (!size) return;
										if (from_id >= size || to_id > size || to_id <= from_id) return;
			// Closing
			for (i=from_id; i<to_id; i++) if (v[i] != nullptr) v[i]->Close();
			if (delete_data)
				Delete(erase_data, from_id, to_id);
		}
		//>> Добавляет count новых мест в конец (указатели пустые)
		void Add(size_t count)
		{
			for (size_t i=0; i<count; i++) v.emplace_back(nullptr);
		}
		//>> Добавляет count новых мест в конец (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void AddCreate(size_t count)
		{	
			size_t size   = v.size();
			size_t target = size + count;
			Add(count);
			for (size_t i=size; i<target; i++)
				v[i] = std::make_shared<T>();
		}
		//>> Увеличивает до count новых мест (указатели пустые)
		void Set(size_t count)
		{
			size_t size = v.size();
			if (count > size) Add(count - size);
		}
		//>> Увеличивает до count новых мест (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void SetCreate(size_t count)
		{
			size_t size = v.size();
			if (count > size) AddCreate(count - size);
		}
		//>> Устанавливает указатель в позицию (другой объект будет удалён)
		void Reset(std::shared_ptr<T> ptr, size_t id)
		{
			if (id >= v.size()) { _MBM(ERROR_IDLIMIT); return; }

			v[id] = ptr;
		}
		//>> Меняет местами 2 указателя
		void Swap(size_t id_0, size_t id_1)
		{
			size_t size = v.size();
			if (id_0 >= size || id_1 >= size) { _MBM(ERROR_IDLIMIT); return; }

			_SWAP(v[id_0], v[id_1]);
		}

		std::shared_ptr<T> operator[](size_t idx)  { return v[idx]; }
		void operator()(size_t num_to_create) { Create(num_to_create); }
	};

	template <class T>
	struct VECDATAPP // Вспомогательная оболочка для гибридного управления массивом обычных + shared ptr
	{
	protected:
		std::vector<std::shared_ptr<T>> vs;
		std::vector<T*>                 v;
	public:
		VECDATAPP()  { };
		~VECDATAPP() { Delete(1); };
		size_t size()              { return vs.size(); }
		void emplace_back()        { Add(1); }
		void emplace_back_create() { AddCreate(1); }
		void pop_back()            { v.pop_back();     vs.pop_back();     }
		void reserve(size_t count) { v.reserve(count); vs.reserve(count); }
		T *    last()              { return v [vs.size() - 1];  }
		size_t last_id()           { return    vs.size() - 1;   }

		auto begin() -> decltype(v.begin())
		{
			return v.begin();
		}
		auto end() -> decltype(v.end())
		{
			return v.end();
		}
		
		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data)
		{
			size_t i, size = vs.size();

			// Deleting
			for (i = 0; i < size; i++)	{
				vs[i] = nullptr;
				v[i]  = nullptr;		}			
			if (erase_data)								{
				vs.erase ( vs.begin(), vs.end() );
				v.erase  ( v.begin(),  v.end()  );		}
		}
		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data, size_t from_id)
		{
			size_t i, size = vs.size();			if (!size) return;
			if (from_id >= size) return;

			// Deleting
			for (i = from_id; i < size; i++)	{
				vs[i] = nullptr;
				v[i]  = nullptr;				}
			if (erase_data)										{
				vs.erase ( vs.begin() + from_id, vs.end() );
				v.erase  ( v.begin()  + from_id, v.end()  );	}
		}
		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data, size_t from_id, size_t to_id)
		{
			size_t i, size = vs.size();					  if (!size) return;
			if (from_id >= size || to_id > size || to_id <= from_id) return;
			
			// Deleting
			for (i = from_id; i < to_id; i++)	{
				vs[i] = nullptr;
				v[i]  = nullptr;				}
			if (erase_data)												{
				vs.erase ( vs.begin() + from_id, vs.begin() + to_id );
				v.erase  (  v.begin() + from_id,  v.begin() + to_id );	}
		}
		//>> Задаёт новый вектор в count мест (все указатели пустые)
		void New(size_t count)
		{
			// Deleting
			Delete(1);

			// Reserving
			vs.reserve(count);
			v.reserve(count);
			for (size_t i = 0; i < count; i++)
			{
				vs.emplace_back(nullptr);
				v.emplace_back(nullptr);
			}
		}
		//>> Задаёт новый вектор в count мест (все указатели ведут к новым объектам) :: abstract classes restricted via this function
		void Create(size_t count)
		{
			// Deleting + Reserving
			New(count);

			// Creating
			size_t i, size = vs.size();
			for (i=0; i<size; i++)
			{
				vs[i] = std::make_shared<T>();
				v[i]  = vs[i].get();
			}
		}
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data)
		{
			// Closing
			for (auto & i : v) if (i != nullptr) i->Close();
			if (delete_data)
				Delete(erase_data);
		}
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data, size_t from_id)
		{
			size_t i, size = vs.size();					if (!size) return;
			if (from_id >= size) return;
			// Closing
			for (i = from_id; i<size; i++) if (v[i] != nullptr) v[i]->Close();
			if (delete_data)
				Delete(erase_data, from_id);
		}
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data, size_t from_id, size_t to_id)
		{
			size_t i, size = vs.size();					  if (!size) return;
			if (from_id >= size || to_id > size || to_id <= from_id) return;
			// Closing
			for (i = from_id; i<to_id; i++) if (v[i] != nullptr) v[i]->Close();
			if (delete_data)
				Delete(erase_data, from_id, to_id);
		}
		//>> Добавляет count новых мест в конец (указатели пустые)
		void Add(size_t count)
		{
			for (size_t i = 0; i < count; i++)
			{
				vs.emplace_back(nullptr);
				v.emplace_back(nullptr);
			}
		}
		//>> Добавляет count новых мест в конец (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void AddCreate(size_t count)
		{
			size_t size = vs.size();
			size_t target = size + count;
			Add(count);
			for (size_t i = size; i<target; i++)
			{
				vs[i] = std::make_shared<T>();
				v[i]  = vs[i].get();
			}
		}
		//>> Увеличивает до count новых мест (указатели пустые)
		void Set(size_t count)
		{
			size_t size = vs.size();
			if (count > size) Add(count - size);
		}
		//>> Увеличивает до count новых мест (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void SetCreate(size_t count)
		{
			size_t size = vs.size();
			if (count > size) AddCreate(count - size);
		}
		//>> Устанавливает указатель в позицию (другой объект будет удалён)
		void Reset(std::shared_ptr<T> ptr, size_t id)
		{
			if (id >= vs.size()) { _MBM(ERROR_IDLIMIT); return; }

			vs[id] = ptr;
			v[id]  = ptr.get();
		}
		//>> Меняет местами 2 указателя
		void Swap(size_t id_0, size_t id_1)
		{
			size_t size = vs.size();
			if (id_0 >= size || id_1 >= size) { _MBM(ERROR_IDLIMIT); return; }

			_SWAP(vs[id_0], vs[id_1]);
			_SWAP(v [id_0], v [id_1]);
		}

		T *  operator[](size_t idx)           { return v[idx];  }
		void operator()(size_t num_to_create) { Create(num_to_create); }

		std::shared_ptr<T> shared(size_t idx) { return vs[idx]; }
	};

	template <class T>
	struct VECPDATA // Вспомогательная оболочка для управления массивом обычных указателей
	{
	protected:
		std::vector<T*> v;
	public:
		VECPDATA()  { };
		~VECPDATA() { Delete(1); };
		size_t size()              { return v.size(); }
		void emplace_back()        { Add(1); }
		void emplace_back_create() { AddCreate(1); }
		void pop_back()            { v.pop_back(); }
		void reserve(size_t count) { v.reserve(count); }
		T *    last()              { return v[v.size() - 1]; }
		size_t last_id()           { return   v.size() - 1;  }

		auto begin() -> decltype(v.begin())
		{
			return v.begin();
		}
		auto end() -> decltype(v.end())
		{
			return v.end();
		}

		//>> Удаляет объекты и сбрасывает указатели в nullptr
		void Delete(bool erase_data)
		{
			// Deleting
			for (auto & i : v) _DELETE(i);
			if (erase_data)
				v.erase(v.begin(), v.end());
		}
		//>> Удаляет объекты и сбрасывает указатели в nullptr
		void Delete(bool erase_data, size_t from_id)
		{
			size_t i, size = v.size();	if (!size) return;
			if (from_id >= size) return;
			// Deleting
			for (i = from_id; i<size; i++) _DELETE(v[i]);
			if (erase_data)
				v.erase(v.begin() + from_id, v.end());
		}
		//>> Удаляет объекты и сбрасывает указатели в nullptr
		void Delete(bool erase_data, size_t from_id, size_t to_id)
		{
			size_t i, size = v.size();	if (!size) return;
			if (from_id >= size || to_id > size || to_id <= from_id) return;
			// Deleting
			for (i = from_id; i<to_id; i++) _DELETE(v[i]);
			if (erase_data)
				v.erase(v.begin() + from_id, v.begin() + to_id);
		}
		//>> Задаёт новый вектор в count мест (все указатели пустые)
		void New(size_t count)
		{
			// Deleting
			Delete(1);

			// Reserving
			v.reserve(count);
			for (size_t i = 0; i<count; i++) v.emplace_back(nullptr);
		}
		//>> Задаёт новый вектор в count мест (все указатели ведут к новым объектам) :: abstract classes restricted via this function
		void Create(size_t count)
		{
			// Deleting + Reserving
			New(count);

			// Creating
			for (auto & i : v) i = new T;
		}
		//>> Вызывает метод закрытия объекта
		void Close(bool delete_data, bool erase_data)
		{
			// Closing
			for (auto & i : v) if (i != nullptr) i->Close();
			if (delete_data)
				Delete(erase_data);
		}
		//>> Вызывает метод закрытия объекта
		void Close(bool delete_data, bool erase_data, size_t from_id)
		{
			size_t i, size = v.size();	if (!size) return;
			if (from_id >= size) return;
			// Closing
			for (i = from_id; i<size; i++) if (v[i] != nullptr) v[i]->Close();
			if (delete_data)
				Delete(erase_data, from_id);
		}
		//>> Вызывает метод закрытия объекта
		void Close(bool delete_data, bool erase_data, size_t from_id, size_t to_id)
		{
			size_t i, size = v.size();	if (!size) return;
			if (from_id >= size || to_id > size || to_id <= from_id) return;
			// Closing
			for (i = from_id; i<to_id; i++) if (v[i] != nullptr) v[i]->Close();
			if (delete_data)
				Delete(erase_data, from_id, to_id);
		}
		//>> Добавляет count новых мест в конец (указатели пустые)
		void Add(size_t count)
		{
			for (size_t i = 0; i<count; i++) v.emplace_back(nullptr);
		}
		//>> Добавляет count новых мест в конец (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void AddCreate(size_t count)
		{
			size_t size = v.size();
			size_t target = size + count;
			Add(count);
			for (size_t i = size; i<target; i++)
				v[i] = new T;
		}
		//>> Увеличивает до count новых мест (указатели пустые)
		void Set(size_t count)
		{
			size_t size = v.size();
			if (count > size) Add(count - size);
		}
		//>> Увеличивает до count новых мест (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void SetCreate(size_t count)
		{
			size_t size = v.size();
			if (count > size) AddCreate(count - size);
		}
		//>> Устанавливает указатель в позицию (другой объект будет удалён)
		void Reset(T * ptr, size_t id)
		{
			if (id >= v.size()) { _MBM(ERROR_IDLIMIT); return; }

			_DELETE(v[id]);
			v[id] = ptr;
		}
		//>> Меняет местами 2 указателя
		void Swap(size_t id_0, size_t id_1)
		{
			size_t size = v.size();
			if (id_0 >= size || id_1 >= size) { _MBM(ERROR_IDLIMIT); return; }

			_SWAP(v[id_0], v[id_1]);
		}

		T *  operator[](size_t idx)            { return v[idx]; }
		void operator()(size_t num_to_create)  { Create(num_to_create); }
	};

	/////////////////////////////////////////////////////////////////////

/*	template <typename T>
	class AtomicSharedPtr
	{
	/////////////////////////////////////////////////////////////////////
		struct SharedLocker
		{
			SharedLocker() : locker(block, std::defer_lock) { }
			~SharedLocker() { locker.unlock(); }

			void Lock()
			{
				locker.lock();
			}
			void Unlock()
			{
				locker.unlock();
			}

		private:
			std::mutex                    block;
			std::unique_lock<std::mutex>  locker;
		};
	/////////////////////////////////////////////////////////////////////
		friend class AtomicSharedPtr<T>;
	public:
		AtomicSharedPtr()
		{
			locker = new SharedLocker;
			owners = nullptr;
			ptr    = nullptr;
		}
		AtomicSharedPtr(std::shared_ptr<T> & this_ptr_will_be_nulled)
		{
			locker = new SharedLocker;
			owners = new uint64;
			(*owners) = 1;
			ptr = this_ptr_will_be_nulled;
			this_ptr_will_be_nulled = nullptr;
		}
		~AtomicSharedPtr()
		{
			Close(true);
			_DELETE(locker);	
		}
	protected:
		void Close(bool lock)
		{
			if (lock) Lock();
			if (owners != nullptr) // we have something
			{
				ptr = nullptr;
				(*owners)--;

				bool is_last = ((*owners) == 0);

				if (is_last) _DELETE(owners);
				else          owners = nullptr;

				if (!is_last) // somebody still hold data, so we create and swap own lock
				{
					SharedLocker * new_locker = new SharedLocker;

					new_locker->Lock();				// new lock()
					std::swap(new_locker, locker);	// new <-> old
					new_locker->Unlock();			// old unlock()
				}	
			}		
			Unlock();
		}

	public:
		void operator=(const void * _nullptr_)
		{
			if (_nullptr_ != nullptr) return;

			Close(true);
		}
		void operator=(const AtomicSharedPtr & other)
		{
			Lock();
			other.Lock();

			if (other.owners == nullptr)
			{
				other.Unlock();
				Close(false);
			}
			else
			{
				if (owners == nullptr) // we have nothing & free for new data
				{
					SharedLocker * old_locker = locker;
					locker = other.locker;
					ptr    = other.ptr;
					owners = other.owners;
					(*owners)++;
					other.Unlock();
					old_locker.Unlock();
					delete old_locker;
				}
				else // exchange old data to new one
				{
					bool is_last = ((*owners) == 1);

					if (is_last)
					{

					}
					else
					{

					}
				}
			}
		}

	protected:
		void Lock()
		{
			locker->Lock();
		}
		void Unlock()
		{
			locker->Unlock();
		}

		std::shared_ptr<T> ptr;
		SharedLocker *     locker;
		uint64 *           owners;
	}; //*/

	/////////////////////////////////////////////////////////////////////

	enum eTaskPriority // tasks priorities in waiting queue
	{
		TASK_PRIOR_0,
		TASK_PRIOR_1,
		TASK_PRIOR_2,
		TASK_PRIOR_3,
		TASK_PRIOR_4,
		TASK_PRIOR_5,
		TASK_PRIOR_6,
		TASK_PRIOR_7,
		TASK_PRIOR_8,
		TASK_PRIOR_9,
		TASK_PRIOR_10,
		TASK_PRIOR_11,
		TASK_PRIOR_12,
		TASK_PRIOR_13,
		TASK_PRIOR_14,
		TASK_PRIOR_15,

		TASK_PRIOR_LIMIT,

		TASK_PRIOR_HIGH   = TASK_PRIOR_0, // <TASK_PRIOR_0> : task will be done quckly, before others
		TASK_PRIOR_NORMAL = TASK_PRIOR_1, // <TASK_PRIOR_1> : common task queue
		TASK_PRIOR_LOW    = TASK_PRIOR_2, // <TASK_PRIOR_2> : task can wait until others done

		TASK_PRIOR_DEFAULT = TASK_PRIOR_NORMAL // if you don't care about
	};

	class THREADPOOL // _NOTE_ выдаваемый пулом ticket никогда не NULL
	{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
		class CThreadTaskT // virtual base class for any type of task
		{
		public:
			CThreadTaskT(const CThreadTaskT& src)            = delete;
			CThreadTaskT(CThreadTaskT&& src)                 = delete;
			CThreadTaskT& operator=(CThreadTaskT&& src)      = delete;
			CThreadTaskT& operator=(const CThreadTaskT& src) = delete;
		public:
			CThreadTaskT()
			{
				//is_void = ? unknown from here
				//task_id   = MISSING;  no matter right now
				have_worker = false;
				drop_result = false;
				removed     = false;
				ptr_to_result = nullptr;
				_ATM_ST(done_by_worker, false);
				_ATM_ST(ready_to_remove, false);
			};
			virtual ~CThreadTaskT(){};
			virtual void Run() = 0;
		/////////
			uint32  task_id;		// unique task's ID
			 bool   have_worker;	// hey, is there any worker ?
			 bool   is_void;		// true if task return nothing
			 bool   drop_result;	// manager don't wait grabbers and remove *this after worker's done
			 bool   removed;		// user call to delete it from queue & don't run
			 void * ptr_to_result;	// provide to task result (if result isn't void)
		/////////	
			std::atomic<bool>  done_by_worker;	// 
			std::atomic<bool>  ready_to_remove; //
		};
		template<class RESULT, class ...TARGS>
		class CThreadTask : public CThreadTaskT // actual class contained implemented task
		{
		public:
			typedef typename std::packaged_task < RESULT(TARGS...) >   TFUNC;
			typedef typename std::tuple         <        TARGS...  >   TARGS;
			typedef typename std::decay<TARGS>::type                  _TARGS;
			typedef typename std::tuple_size<_TARGS>                  nTARGS;

			template <class RESULT, bool ISVOID>
			// if result isn't void -> hold result in this
			struct RHOLD
			{
				std::future<RESULT> future_result;
				RESULT result;
				void   GET() { result = future_result.get();       }
				void * PTR() { return static_cast<void*>(&result); }
			};

			template <class RESULT>
			// if result is void -> just create this dummy thing
			struct RHOLD <RESULT, true> 
			{
				std::future<RESULT> future_result;
				void   GET() { future_result.get();  }
				void * PTR() { return nullptr;       }
			};

			////////////////////

			TFUNC  func;
			TARGS  args;
			RHOLD  < RESULT, std::is_void<RESULT>::value >  hold;

			CThreadTask() : CThreadTaskT() { is_void = std::is_void<RESULT>::value; };
			~CThreadTask() { };

			////////////////////

			template <class F, class T, bool DONE, int TOTAL, int... ARGS_NUM>
			struct RUNNER
			{
				static void Run(F & f, T & t)
				{
					RUNNER < F, T, TOTAL == 1 + sizeof...(ARGS_NUM), TOTAL, ARGS_NUM..., sizeof...(ARGS_NUM) > 
						:: Run ( std::move(f), std::move(t) ) ;
				}
			};

			template <class F, class T,            int TOTAL, int... ARGS_NUM>
			struct RUNNER < F, T, true, TOTAL, ARGS_NUM...>
			{
				static void Run(F & f, T & t)
				{
					f ( std::get <ARGS_NUM> (t)... );
				}
			};

			void Run() override final
			{
				hold.future_result = func.get_future();

				RUNNER < TFUNC, TARGS, 0 == nTARGS::value, nTARGS::value >
					:: Run ( std::move(func), std::move(args) ) ;

				hold.GET();
				ptr_to_result = hold.PTR();
			}
		};
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
		template <bool STATIC, class TFUNC, class ...TARGS>
		struct RunTaskProcWrapper 
		{	///////////// if we run non-static function member of some class /////////////
			static auto Get() -> decltype(std::mem_fn(&THREADPOOL::RunTaskProc<TFUNC, TARGS...>))
			{
				return std::mem_fn(&THREADPOOL::RunTaskProc<TFUNC, TARGS...>);
			}
		};
		template <             class TFUNC, class ...TARGS>
		struct RunTaskProcWrapper <true, TFUNC, TARGS...> 
		{	///////////// if we run static function (static member or not member or lambda) /////////////
			static auto Get() -> decltype(std::mem_fn(&THREADPOOL::RunTaskProcStatic<TFUNC, TARGS...>))
			{
				return std::mem_fn(&THREADPOOL::RunTaskProcStatic<TFUNC, TARGS...>);
			}
		};
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
		template <uint16 MAX, class T>
		//>> При более 100000 элементов deque забивает память и слишком медленно потом её освобождает 
		//>> <balanced_queue> разбивает на малые <MAX> сегменты и удаляет их по ходу дела
		//>> TODO: подумать над условием в pop_front()
		class balanced_queue
		{
			typedef typename balanced_queue<MAX,T> _THIS;
			typedef typename std::vector<T> chunk;
			typedef void (_THIS::*pFreeAtBackgroundProc)(std::vector<chunk> &);

		public:
			balanced_queue(const balanced_queue& src)            = delete;
			balanced_queue& operator=(const balanced_queue& src) = delete;
			balanced_queue(balanced_queue&& src)                 = delete;
			balanced_queue& operator=(balanced_queue&& src)      = delete;
		
		private:
			size_t SZ;		// real count of T elements
			size_t START;	// virtual start position of first [0] available T element
			size_t TODEL;	// num of chunks prepared for deleting
			std::vector<chunk> data;

		private:
			std::atomic<uint64>		 free_mem_wait;			// simply block ~balanced_queue() until FreeAtBackgroundProc() done 
			std::mutex				 free_mem_wait_block;	// simply block ~balanced_queue() until FreeAtBackgroundProc() done
			std::condition_variable	 free_mem_wait_cv;		// simply block ~balanced_queue() until FreeAtBackgroundProc() done
			pFreeAtBackgroundProc    pFree;					//

		private:
			const int TRIGGER_FOR_BACKGROUND_DELETE = 10;  // min chunks limit to del in cur thread
			const int TRIGGER_FOR_FORCE_DELETE      = 100; // max chunks limit to awake auto delete

		public:
			balanced_queue() : START(0), SZ(0), TODEL(0), free_mem_wait(0), pFree(&_THIS::FreeAtBackgroundProc)
			{
				make_chunk(true);
			};
			~balanced_queue()
			{
				ulock lock(free_mem_wait_block);
				while (_ATM_LD(free_mem_wait))
					{ _CV_WAIT(free_mem_wait_cv, lock); }
			};

			T & operator[](uint64 element_id)
			{
				return data [GetQueueNum(element_id)] [GetLocalID(element_id)] ;
			}
			size_t size()
			{
				return (SZ - START);
			}
			void clear()
			{
				//clock_t time = clock();
				if (TODEL < TRIGGER_FOR_BACKGROUND_DELETE) // it should be fast, so just clear in current thread
				{
					data.clear();
					make_chunk(true);
					START = 0;
					SZ    = 0;
					TODEL = 0;
				}
				else
				{			
					//printf("\nUsing FreeAtBackground()...");
					FreeAtBackground();
				}
				//printf("\n ...done free %i ms", clock() - time);
			}
			template<class _Ty>
			void emplace_back(_Ty&& val)
			{
				static_assert (
					!std::is_same<_Ty, T>::value,
					"balanced_queue : Type _Ty should be T"
				);

				size_t Q  = SZ / MAX;	// queue id
				size_t ID = SZ % MAX;	// element id in queue
				if (ID == 0 && Q > 0)
					make_chunk(true);
				data[Q].emplace_back(forward<_Ty>(val));
				SZ++;
			};
			void pop_front()
			{
				if (!size()) return;

				START++;

				if (START == SZ && TODEL) // no tasks, let's some clearing!
				{
					//printf("\nqueue.clear();  chnks %i", TODEL);
					clear();
				} else
				if (START == (MAX * (TODEL + 1)))
				{
					TODEL++;
				
					// TODO: подумать ещё над условием

					if ( TODEL > TRIGGER_FOR_FORCE_DELETE &&					// we have been triggered to spy what's going on
						 (SZ - START) < (MAX * TRIGGER_FOR_BACKGROUND_DELETE)	// clear at (tasks in work < triggered limit)
					   )
					{
						std::vector<chunk> empty;			//clock_t time = clock();
						std::swap(empty, data);

						size_t i, n=0, sz=empty.size();
						data.reserve(sz - TODEL);
						for (i=TODEL; i<sz; i++)	  make_chunk(false);			// fill with dummy
						for (i=TODEL; i<sz; i++, n++) std::swap(data[n], empty[i]);	// exchange dummy to real

						FreeAtBackground(empty); // push to delete

						SZ -= MAX * TODEL;
						START = 0;
						TODEL = 0;

						//printf("\n ... %i ms (Force clearing)", clock() - time);
					}
				}
			}
			size_t chunks()
			{
				return TODEL;
			}

		private:
			size_t GetQueueNum(size_t location)
			{
				return (START + location) / MAX;
			}
			size_t GetLocalID(size_t location)
			{
				return (START + location) % MAX;
			}
			void make_chunk(bool reserve)
			{
				auto N = data.size();
				data.emplace_back(chunk());
				if (reserve) data[N].reserve(MAX);
			};
			void FreeAtBackground()
			{
				_ATM_ADD(free_mem_wait, 1); // block destructor - this will prevent programm closing until memory freeing normally

				auto background = std::thread(pFree, this, std::move(data)); // after move <data> become empty in this thread
				background.detach();

				make_chunk(true);
				START = 0;
				SZ    = 0;
				TODEL = 0;
			}
			void FreeAtBackground(std::vector<chunk> & data)
			{
				_ATM_ADD(free_mem_wait, 1);

				auto background = std::thread(pFree, this, std::move(data));
				background.detach();
			}
			void FreeAtBackgroundProc(std::vector<chunk> & data)
			{
				//auto time = _TIME;
				data.clear();
				//printf("\nFreeAtBackgroundProc ENDED! %i ms", _TIMER(time));
				{
					glock lock(free_mem_wait_block);
					_ATM_SUB(free_mem_wait, 1);
				}
				_CV_ALL(free_mem_wait_cv);
			}
		};
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
		//>> TODO: возможность сразу задавать рабочему пару тысяч задач в собственную <queue>
		struct WORKER
		{
			WORKER()  { };
			~WORKER() { Close(); };
			void Close()
			{
				thread.Close();
			}

			THREADDATA thread;
			//TBUFFER<CThreadTaskT*, uint64> queue;
			//balanced_queue <50, CThreadTaskT*> queue;
		};
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
		static uint16 pool_unique_id_counter; // счётчик экземпляров THREADPOOL
	protected:
		THREADDATA											_manager;	// manage tasks & workers
		VECDATAP<WORKER>									_worker;	// threads that do stuff
		std::atomic<int32>									_available;	// num of workers without task
		balanced_queue <50, std::shared_ptr<CThreadTaskT>>	_task_queue[TASK_PRIOR_LIMIT]; // queues via priorities
		//////////
		std::mutex               task_block;		// ставит в ожидание работу с задачами < _task >
		std::recursive_mutex	 thread_block;		// ставит в ожидание работу с потоками < _worker >
		std::mutex				 manager_block;		// ставит в ожидание до окончания работы менеджера
		//////////
		std::atomic<bool>		 exit_signal;		// signal to manager & workers for exit
		std::atomic<bool>		 stop_signal;		// user can stop manager (prevent run_signal)
		//////////
		bool					 run_signal;		// signal to manager for run
		std::mutex				 run_signal_block;	// block manager until signal
		std::condition_variable	 run_signal_cv;		// block manager until signal
		//////////			
		bool					 work_signal;		// signal to worker for run
		std::mutex				 work_signal_block;	// block worker until signal
		std::condition_variable	 work_signal_cv;	// block worker until signal
		//////////
		std::mutex				 done_signal_block;	// block waiters/grabbers until work will be done
		std::condition_variable	 done_signal_cv;	// block waiters/grabbers until work will be done
		//////////
		uint32	task_id; // счётчик номеров задач
		uint16	pool_id; // счётчик перезагрузок пула (никогда не 0)
		uint16  pool_unique_id; // id текущего экземпляра THREADPOOL
		//////////
		eTaskPriority			 next_task_priority;	// manager set data for worker
		size_t					 next_task_N;			// manager set data for worker
		bool					 worker_grab;		// manager waiting for worker while he grabbing data
		std::mutex				 worker_grab_block;	// manager waiting for worker while he grabbing data
		std::condition_variable	 worker_grab_cv;	// manager waiting for worker while he grabbing data
		//////////
		uint64					 tasks_run[TASK_PRIOR_LIMIT];	// определяет номер задачи в очереди, на которой остановились
		std::atomic<uint64>		 tasks_done[TASK_PRIOR_LIMIT];	// количество выполненных и ещё не удалённых из очереди задач
		std::atomic<uint64>		 cooldown_time;					// manager cooldown in nanoseconds
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	public:
		THREADPOOL(const THREADPOOL& src)            = delete;
		THREADPOOL(THREADPOOL&& src)                 = delete;
		THREADPOOL& operator=(THREADPOOL&& src)      = delete;
		THREADPOOL& operator=(const THREADPOOL& src) = delete;
	public:
		THREADPOOL()             : pool_id(1) { pool_unique_id = 0x0FFF & pool_unique_id_counter++; Init(1);     }
		THREADPOOL(uint16 count) : pool_id(1) { pool_unique_id = 0x0FFF & pool_unique_id_counter++; Init(count); }
		~THREADPOOL()						  { Free(false); }

	protected:
		//>> Задаёт начальные параметры
		void Init(uint16 count)
		{	
			_ATM_ST(_available, 0);	
			_ATM_ST(exit_signal, false);
			_ATM_ST(stop_signal, false);
			worker_grab = false;
			run_signal  = false;
			work_signal = false;
			task_id = 0;
			for (int i=0; i<TASK_PRIOR_LIMIT; i++)	{
				tasks_run[i] = 0;
				_ATM_ST(tasks_done[i], 0);			}
			_ATM_ST(cooldown_time, 1000000);
			WorkersSet(count);
			RunManagerThread();
		}
		//>> Производит сброс данных
		void Free(bool reinit)
		{
			 ulock lock_manager (manager_block); // wait & block manager

			 glock stop_other_working_with_tasks   (task_block);
			rglock stop_other_working_with_threads (thread_block);

			ExitSignal();

			lock_manager.unlock();	// exit signal has given now -> unlock manager

			_manager.Close();
			_worker.Close(1,1);

			for (int i = 0; i<TASK_PRIOR_LIMIT; i++)
				_task_queue[i].clear();

			pool_id++;
			pool_id &= 0x0FFF;
			if (pool_id == 0) pool_id++;

			if (reinit) Init(1);
		}
		//>> Запускает работу менеджера
		void RunManagerThread()
		{
			_manager(&THREADPOOL::ManagerProc, this);
		}
		//>> Запускает работу <id>-потока
		void RunWorkerThread(int32 own_id)
		{
			_worker[own_id]->thread(&THREADPOOL::WorkerProc, this, own_id);
		}

	public:
		//>> Закрывает/сбрасывает THREADPOOL, удаляя все задачи и доп.потоки
		void Close()
		{
			Free(true);
		}	
		//>> Добавляет рабочих потоков в количестве <count>
		void WorkersAdd(uint8 count)
		{
			rglock stop_other_working_with_threads (thread_block);

			size_t old_size = _worker.size();
			_worker.AddCreate(count);
			size_t new_size = _worker.size();

			for (size_t i=old_size; i<new_size; i++)
				RunWorkerThread(static_cast<int32>(i));

			_ATM_ADD(_available, (int32)(new_size - old_size));
		}
		//>> Добавляет рабочих потоков до размера <count>
		void WorkersSet(uint16 count)
		{
			rglock stop_other_working_with_threads (thread_block);

			size_t old_size = _worker.size();
			_worker.SetCreate(count);
			size_t new_size = _worker.size();

			for (size_t i=old_size; i<new_size; i++)
				RunWorkerThread(static_cast<int32>(i));

			_ATM_ADD(_available, (int32)(new_size - old_size));
		}
		//>> Сообщает текущее количество ожидающих задач
		size_t GetTasks(eTaskPriority priority)
		{
			glock stop_other_working_with_tasks (task_block);

			if (priority == TASK_PRIOR_LIMIT) priority = (eTaskPriority)(TASK_PRIOR_LIMIT - 1);

			return _task_queue[priority].size();
		}
		//>> Сообщает текущее количество ожидающих задач
		size_t GetTasks()
		{
			glock stop_other_working_with_tasks (task_block);

			size_t ret = 0;
			for (uint32 i=0; i<TASK_PRIOR_LIMIT; i++)
				ret += _task_queue[i].size();

			return ret;
		}
		//>> Сообщает текущее количество рабочих потоков
		size_t GetWorkers()
		{
			rglock stop_other_working_with_threads (thread_block);

			return _worker.size();
		}

	public:
		//>> Ставит в ожидание, пока задачу не выполнят
		eThreadCode Wait(uint64 ticket)
		{
			eTaskPriority eTP = TicketGetPriority (ticket); // target task priority
			uint16   POOL_UID = TicketGetUID      (ticket); // target pool unique id
			uint16   POOL_ID  = TicketGetID       (ticket); // target pool id
			uint32   TASK_ID  = TicketGetTaskID   (ticket); // target task id

			if (POOL_UID != pool_unique_id) return TC_POOL_UID_MISMATCH;
			if (POOL_ID  != pool_id       ) return TC_POOL_ID_MISMATCH;

			auto FindTaskProc = [this, TASK_ID, eTP] () -> std::shared_ptr < CThreadTaskT >
			{
				auto & queue = _task_queue[eTP];
				if (queue.size() > 0)
				{
					uint64 N;
					uint32 TASK_0 = queue[0]->task_id;
					if (TASK_ID >= TASK_0)
						 N = (uint64)(TASK_ID - TASK_0);
					else N = (uint64)(0x100000000 | TASK_ID - TASK_0);
					if (N > queue.size()) return nullptr;
					if (queue[N]->task_id == TASK_ID)
						return queue[N];
				}
				return nullptr;
			};

			std::shared_ptr < CThreadTaskT > task; // <shared_ptr> prevent deleting task object while we are waiting

			{
				glock stop_other_working_with_tasks (task_block);
				task = FindTaskProc();
				if (task == nullptr) return TC_POOL_TASK_NOT_FOUND;
				if (_ATM_LD(task->done_by_worker))
					{ task = nullptr; return TC_SUCCESS; }
			}
			
			{
				ulock lock(done_signal_block);				// Даже если менеджер удалит задачу из своих списоков,
				while (!_ATM_LD(task->done_by_worker))		// сам объект задачи сохранится, пока мы владеем <shared_ptr>
					{ _CV_WAIT(done_signal_cv, lock); }		// .
			}

			{
				glock stop_other_working_with_tasks (task_block);
				task = nullptr;
			}

			return TC_SUCCESS;
		}
		//>> Освобождает от ожидания результата, если такой есть и его ожидают (задача будет удалена)
		eThreadCode FreeResult(uint64 ticket)
		{
			eTaskPriority eTP = TicketGetPriority (ticket); // target task priority
			uint16   POOL_UID = TicketGetUID      (ticket); // target pool unique id
			uint16   POOL_ID  = TicketGetID       (ticket); // target pool id
			uint32   TASK_ID  = TicketGetTaskID   (ticket); // target task id

			if (POOL_UID != pool_unique_id) return TC_POOL_UID_MISMATCH;
			if (POOL_ID  != pool_id       ) return TC_POOL_ID_MISMATCH;

			auto FreeResultProc = [this, TASK_ID, eTP] () -> bool
			{
				auto & queue = _task_queue[eTP];
				if (queue.size() > 0)
				{
					uint64 N;
					uint32 TASK_0 = queue[0]->task_id;
					if (TASK_ID >= TASK_0)
						 N = (uint64)(TASK_ID - TASK_0);
					else N = (uint64)(0x100000000 | TASK_ID - TASK_0);
					if (N > queue.size()) return nullptr;
					if (queue[N]->task_id == TASK_ID)			{
						_RLX_1(queue[N]->ready_to_remove);
						return true;							}
				}
				return false;
			};

			glock stop_other_working_with_tasks (task_block);
			if (FreeResultProc()) { RunSignal(); return TC_SUCCESS; }
			return TC_POOL_TASK_NOT_FOUND;
		}
		template<class RESULT>
		//>> Возвращает результат на <save_out_location>, если он готов
		eThreadCode GrabResult(RESULT & ref_to_save_out_location, uint64 ticket, bool wait, bool free_result_if_success)
		{
			eTaskPriority eTP = TicketGetPriority (ticket); // target task priority
			uint16   POOL_UID = TicketGetUID      (ticket); // target pool unique id
			uint16   POOL_ID  = TicketGetID       (ticket); // target pool id
			uint32   TASK_ID  = TicketGetTaskID   (ticket); // target task id

			if (POOL_UID != pool_unique_id) return TC_POOL_UID_MISMATCH;
			if (POOL_ID  != pool_id       ) return TC_POOL_ID_MISMATCH;

			auto FindTaskProc = [this, TASK_ID, eTP] () -> CThreadTaskT *
			{
				auto & queue = _task_queue[eTP];
				if (queue.size() > 0)
				{ 
					uint64 N;
					uint32 TASK_0 = queue[0]->task_id;
					if (TASK_ID >= TASK_0)
						 N = (uint64)(TASK_ID - TASK_0);
					else N = (uint64)(0x100000000 | TASK_ID - TASK_0);
					if (N > queue.size()) return nullptr;
					if (queue[N]->task_id == TASK_ID)
						return queue[N].get();
				}
				return nullptr;
			};

			{
				ulock stop_other_working_with_tasks (task_block);

				auto task = FindTaskProc();

				if (task == nullptr)                    return TC_POOL_TASK_NOT_FOUND;
				if (task->removed)                      return TC_POOL_TASK_REMOVED;
				if (task->is_void || task->drop_result) return TC_RESULT_NONE;
				if (!_ATM_LD(task->done_by_worker)) 
				{
					if (!wait) return TC_RESULT_WAIT;
					else
					{
						stop_other_working_with_tasks.unlock();
						{
							ulock lock(done_signal_block);
							while (!_ATM_LD(task->done_by_worker))
								{ _CV_WAIT(done_signal_cv, lock); }
						}
						stop_other_working_with_tasks.lock();			// Task may had been removed while we have waited
																		// (если её не успели начать, а мы уже стали ждать)
						if (task->removed) return TC_POOL_TASK_REMOVED; // .
					}
				}
				if (_RLX_LD(task->ready_to_remove)) return TC_RESULT_LOCK;
				
				RESULT * result = reinterpret_cast<RESULT*>(task->ptr_to_result);
				ref_to_save_out_location = (*result);
				if (free_result_if_success) { _RLX_ST(task->ready_to_remove, true); RunSignal(); }
			}

			return TC_SUCCESS;
		}		
		template<class RESULT>
		//>> Возвращает результат на <save_out_location>, если он готов
		eThreadCode GrabResult(RESULT * ptr_to_save_out_location, uint64 ticket, bool wait, bool free_result_if_success)
		{
			eTaskPriority eTP = TicketGetPriority (ticket); // target task priority
			uint16   POOL_UID = TicketGetUID      (ticket); // target pool unique id
			uint16   POOL_ID  = TicketGetID       (ticket); // target pool id
			uint32   TASK_ID  = TicketGetTaskID   (ticket); // target task id

			if (POOL_UID != pool_unique_id) return TC_POOL_UID_MISMATCH;
			if (POOL_ID  != pool_id       ) return TC_POOL_ID_MISMATCH;

			auto FindTaskProc = [this, TASK_ID, eTP] () -> CThreadTaskT *
			{
				auto & queue = _task_queue[eTP];
				if (queue.size() > 0)
				{ 
					uint64 N;
					uint32 TASK_0 = queue[0]->task_id;
					if (TASK_ID >= TASK_0)
						 N = (uint64)(TASK_ID - TASK_0);
					else N = (uint64)(0x100000000 | TASK_ID - TASK_0);
					if (N > queue.size()) return nullptr;
					if (queue[N]->task_id == TASK_ID)
						return queue[N].get();
				}
				return nullptr;
			};

			{
				glock stop_other_working_with_tasks (task_block);

				auto task = FindTaskProc();

				if (task == nullptr)                    return TC_POOL_TASK_NOT_FOUND;
				if (task->removed)                      return TC_POOL_TASK_REMOVED;
				if (task->is_void || task->drop_result) return TC_RESULT_NONE;
				if (!_ATM_LD(task->done_by_worker)) 
				{
					if (!wait) return TC_RESULT_WAIT;
					else
					{
						stop_other_working_with_tasks.unlock();
						{
							ulock lock(done_signal_block);
							while (!_ATM_LD(task->done_by_worker))
								{ _CV_WAIT(done_signal_cv, lock); }
						}
						stop_other_working_with_tasks.lock();			// Task may had been removed while we have waited
																		// (если её не успели начать, а мы уже стали ждать)
						if (task->removed) return TC_POOL_TASK_REMOVED; // .
					}
				}
				if (_RLX_LD(task->ready_to_remove)) return TC_RESULT_LOCK;
				
				RESULT * result = reinterpret_cast<RESULT*>(task->ptr_to_result);
				(*ptr_to_save_out_location) = (*result);
				if (free_result_if_success) { _RLX_ST(task->ready_to_remove, true); RunSignal(); }
			}

			return TC_SUCCESS;
		}
		//>> Удаляет задачу из списка задач на выполнение, если это ещё возможно
		eThreadCode RemoveTask(uint64 ticket)
		{
			eTaskPriority eTP = TicketGetPriority (ticket); // target task priority
			uint16   POOL_UID = TicketGetUID      (ticket); // target pool unique id
			uint16   POOL_ID  = TicketGetID       (ticket); // target pool id
			uint32   TASK_ID  = TicketGetTaskID   (ticket); // target task id

			if (POOL_UID != pool_unique_id) return TC_POOL_UID_MISMATCH;
			if (POOL_ID  != pool_id       ) return TC_POOL_ID_MISMATCH;

			auto FindTaskProc = [this, TASK_ID, eTP] () -> CThreadTaskT *
			{
				auto & queue = _task_queue[eTP];
				if (queue.size() > 0)
				{
					uint64 N;
					uint32 TASK_0 = queue[0]->task_id;
					if (TASK_ID >= TASK_0)
						 N = (uint64)(TASK_ID - TASK_0);
					else N = (uint64)(0x100000000 | TASK_ID - TASK_0);
					if (N > queue.size()) return nullptr;
					if (queue[N]->task_id == TASK_ID)
						return queue[N].get();
				}
				return nullptr;
			};

			{
				glock stop_other_working_with_tasks (task_block);

				auto task = FindTaskProc();

				if (task == nullptr)   return TC_POOL_TASK_NOT_FOUND;
				if (task->removed)     return TC_POOL_TASK_REMOVED;
				if (task->have_worker) return TC_POOL_TASK_NO_REMOVE;

				task->have_worker = true;
				task->drop_result = true;
				task->removed     = true;
				_ATM_ST(task->done_by_worker,  true);
				_RLX_ST(task->ready_to_remove, true);

				_CV_ALL(done_signal_cv); // if somebody wait for end of work
				RunSignal();
			}

			return TC_SUCCESS;
		}

	protected:
		//>> Ожидание вызова от менеджера
		void WorkerWaitingProc()
		{
			ulock lock(work_signal_block);
			while (work_signal == false)
				 { _CV_WAIT(work_signal_cv, lock); }
			work_signal = false;
		}
		//>> Проверка вызова на выход
		bool WorkerCheckExit(/*int32 own_id*/)
		{
			if (_ATM_LD(exit_signal))
			{
				// notify next worker //
				{
					glock lock(work_signal_block);
					work_signal = true;
				}
				_CV_ONE(work_signal_cv);
				//printf("\nWorker exiting... [%i]", (int32)own_id);
				return true;
			}
			return false;
		}
		//>> Взятие следующей задачи
		void WorkerGetTask(CThreadTaskT * &task, eTaskPriority & mem_priority)
		{
			// Get data and response back to manager //

			mem_priority = next_task_priority;
			task = _task_queue[mem_priority][next_task_N].get();
			task->have_worker = true;
			{
				glock stop_manager_for_a_moment (worker_grab_block);
				worker_grab = true;
			}
			_CV_ALL(worker_grab_cv);
		}
		//>> Исполнение
		void WorkerRunTask(CThreadTaskT * &task, eTaskPriority   mem_priority)
		{
			task->Run();

			{
				glock stop_waiters_for_a_moment (done_signal_block);

				_ATM_ST(task->done_by_worker, true);			// set info that we are done
				if (task->is_void || task->drop_result)			// set to remove if it's needed
					{ _RLX_ST(task->ready_to_remove, true); }	// .
			}
			_CV_ALL(done_signal_cv);

			_ATM_ADD(_available,               +1);	// strong tip to manager
			_RLX_ADD(tasks_done[mem_priority], +1);	// relax tip

			task = nullptr;

			RunSignal(); // call manager that worker done
		}
		//>> Тело рабочего потока
		void WorkerProc(int32 own_id)
		{
			CThreadTaskT * task;
			eTaskPriority  mem_priority;

			WorkerWaitingProc();							// START WAITING POINT
			if (WorkerCheckExit(/*own_id*/)) return;		// EXIT POINT (1)
			WorkerGetTask(task, mem_priority);				// ...

			for (;;)
			{
				//printf("\nWorker in process... [%i]", own_id);

				if (WorkerCheckExit(/*own_id*/)) return;	// EXIT POINT (2)

				WorkerRunTask(task, mem_priority);			// WORK

				if (WorkerCheckExit(/*own_id*/)) return;	// EXIT POINT (3)

				//printf("\nWorker waiting... [%i]", own_id);

				WorkerWaitingProc();						// WAITING POINT
				if (WorkerCheckExit(/*own_id*/)) return;	// EXIT POINT (4)
				WorkerGetTask(task, mem_priority);			// ...		
			}
		}
		
	public:
		//>> Указывает обязательное время отдыха между проверкой очереди задач :: default is 1.000.000 == one (1) millisecond
		void ManagerSetCooldownTime(uint64 nanoseconds)
		{
			_RLX_ST(cooldown_time, nanoseconds);
		}
		//>> Зовёт проверить очереди задач :: additional manual run call
		void ManagerCall()
		{
			StopSignal(2);
		}
		//>> Приостанавливает работу по запуску задач из очередей :: additional manual stop call
		void ManagerStop()
		{
			StopSignal(0);
		}

	protected:
		//>> prevent spamming
		void ManagerCooldown()
		{
			std::this_thread::yield();
			std::this_thread::sleep_for(std::chrono::nanoseconds(_RLX_LD(cooldown_time)));
		}
		//>> Ожидание вызова
		void ManagerWaitingProc()
		{
			ulock lock(run_signal_block);
			while (run_signal == false)
				 { _CV_WAIT(run_signal_cv, lock); }
			run_signal = false;
		}
		//>> Проверка вызова на выход
		bool ManagerCheckExit()
		{
			if (_ATM_LD(exit_signal))
			{
				//printf("\nManager exiting...");
				return true;
			}
			return false;
		}
		//>> Обработка очереди менеджером
		void ManagerWork(eTaskPriority task_priority)
		{
			auto & queue = _task_queue[task_priority];	// current priority queue 
			auto & N_run =  tasks_run[task_priority];	// num of tasks has been run already
			
			// Drop old tasks // FIFO : First In - First Out //
			{
				uint64 limit, done = _RLX_LD(tasks_done[task_priority]);
				uint64 size  = queue.size();
				uint64 del   = 0;

				limit = (size >> 3) & 0x1FFFFFFFFFFFFFFF; // limit = 1/8 queue size

				if (done >= limit)
				{
					//printf("\ndone %i limit %i size %i", done, limit, size);		clock_t time = clock();
					for (uint64 i=0; i<size; i++)
					{
						if (_ATM_LD(queue[0]->done_by_worker) &&
							_RLX_LD(queue[0]->ready_to_remove))
							{ queue.pop_front(); del++; }
						else break;
					}
					if (del)
					{
						//printf("\ndeleted %i in %i ms", del, clock() - time);
						N_run -= del;
						_RLX_SUB(tasks_done[task_priority], del);
					}
				}
			}

			if (_ATM_LD(_available))
			{
				uint64 size = queue.size();
				for (; N_run < size ;)
				{
					next_task_priority = task_priority;
					next_task_N        = N_run;
					{
						glock stop_workers_for_a_moment (work_signal_block);
						work_signal = true;
					}
					_CV_ONE(work_signal_cv);

					// Wait response from any worker //

					ulock lock(worker_grab_block);
					while (worker_grab == false)
						{ _CV_WAIT(worker_grab_cv, lock); }
					worker_grab = false;

					N_run++;
					_ATM_ADD(_available, -1);
					if (!_ATM_LD(_available)) return;
				}
			}
		}
		//>> Тело менеджера
		void ManagerProc()
		{
			ManagerWaitingProc(); // START WAITING POINT

			for(;;)
			{
				{
					glock lock_while_working (manager_block); // блокируем возможный вызов Close() / Free()

					if (ManagerCheckExit()) return; // EXIT POINT (1)

					{
						 glock stop_other_working_with_tasks   (task_block);
						rglock stop_other_working_with_threads (thread_block);
					
						for (int i=0; i<TASK_PRIOR_LIMIT; i++)				// WORKING POINT
							if (_task_queue[i].size())						// .
								ManagerWork(static_cast<eTaskPriority>(i));	// .
					}
				}

				if (ManagerCheckExit()) return; // EXIT POINT (2)

				ManagerCooldown();	  // WAITING POINT
				ManagerWaitingProc(); // .

				//printf("\nManager waking up...");
			}
		}
		
	protected:
		//>> Останавливает (или убирает останов) все сигналы менеджеру на запуск
		void StopSignal(byte remove_stop)
		{
			_RLX_ST(stop_signal, !_BOOL(remove_stop));
			if (remove_stop > 1) RunSignal();
		}
		//>> Отправляет менеджеру сигнал на работу
		void RunSignal()
		{
			if (_RLX_LD(stop_signal)) return; // user has blocked manager
			{
				glock stop_manager_for_a_moment (run_signal_block);
				run_signal = true;
			}
			_CV_ALL(run_signal_cv);
		}
		//>> Отправляет менеджеру и рабочим потокам сигнал на выход
		void ExitSignal()
		{
			_ATM_ST(exit_signal, true);

			{
				glock stop_manager_for_a_moment (run_signal_block);
				run_signal = true;
			}
			_CV_ALL(run_signal_cv);

			{
				glock stor_workers_for_a_moment (work_signal_block);
				work_signal = true;
			}
			_CV_ONE(work_signal_cv);			
		}

	protected:
		//>> Создаёт ticket задачи
		uint64 TicketCreate(eTaskPriority priority)
		{
			uint64 ret  = ++task_id                                   & 0x00000000FFFFFFFF;
			       ret |= (((uint64)pool_unique_id) << (32 + 16 + 4)) & 0xFFF0000000000000;
				   ret |= (((uint64)pool_id)        << (32 + 8     )) & 0x000FFF0000000000;
				   ret |= (((uint64)priority)       << (32         )) & 0x000000FF00000000;

			return ret;
		}
		//>> Возвращает pool_unique_id & 0x0FFF
		uint16 TicketGetUID(uint64 ticket)
		{
			return static_cast <uint16> ((ticket >> (32 + 16 + 4)) & 0x0FFF);
		}
		//>> Возвращает pool_id & 0x0FFF
		uint16 TicketGetID(uint64 ticket)
		{
			return static_cast <uint16> ((ticket >> (32 + 8)) & 0x0FFF);
		}
		//>> Возвращает приоритет записанной в ticket задачи
		eTaskPriority TicketGetPriority(uint64 ticket)
		{
			return static_cast <eTaskPriority> ((ticket >> 32) & 0xFF);
		}
		//>> Возвращает ID записанной в ticket задачи
		uint32 TicketGetTaskID(uint64 ticket)
		{
			return static_cast <uint32> (ticket & 0x00000000FFFFFFFF);
		}

		//TODO: сделать через std::bind, избавиться от std::tuple и RunTaskProcWrapper
		//TODO: подумать, что ещё может ускорить запуск (выделять заранее память ? / rvalue ? / ???)

	protected:
		template<class TFUNC, class ...TARGS>
		//>> Запуск задачи :: возвращает ticket задачи :: prepare non-static functions
		uint64 RunTaskProc(eTaskPriority priority, bool ignore_result, TFUNC function, TARGS... args)
		{
			glock stop_other_working_with_tasks (task_block);

			auto func = std::mem_fn(function);
			std::packaged_task<              decltype(func(args...)) (TARGS...) >  funcpack(func);
			std::shared_ptr<    CThreadTask< decltype(func(args...)), TARGS... >>  task;
			task = make_shared< CThreadTask< decltype(func(args...)), TARGS... >>();
			auto &TASK = *task;

			if (priority == TASK_PRIOR_LIMIT) priority = (eTaskPriority)(TASK_PRIOR_LIMIT - 1);

			uint64 ticket = TicketCreate(priority);
			
			TASK.func        = std::move(funcpack);
			TASK.args        = std::move(std::tuple<TARGS...>(args...));
			TASK.task_id     = std::move(TicketGetTaskID(ticket));
			TASK.drop_result = std::move(ignore_result);

			_task_queue[priority].emplace_back(std::move(task));

			RunSignal();

			return ticket;
		}

		template<class TFUNC, class ...TARGS>
		//>> Запуск задачи :: возвращает ticket задачи :: prepare static functions
		uint64 RunTaskProcStatic(eTaskPriority priority, bool ignore_result, TFUNC function, TARGS... args)
		{
			glock stop_other_working_with_tasks(task_block);

			std::packaged_task<              decltype(function(args...)) (TARGS...) >  funcpack(function);
			std::shared_ptr<    CThreadTask< decltype(function(args...)), TARGS... >>  task;
			task = make_shared< CThreadTask< decltype(function(args...)), TARGS... >>();
			auto &TASK = *task;

			if (priority == TASK_PRIOR_LIMIT) priority = (eTaskPriority)(TASK_PRIOR_LIMIT - 1);

			uint64 ticket = TicketCreate(priority);

			TASK.func        = std::move(funcpack);
			TASK.args        = std::move(std::tuple<TARGS...>(args...));
			TASK.task_id     = std::move(TicketGetTaskID(ticket));
			TASK.drop_result = std::move(ignore_result);

			_task_queue[priority].emplace_back(std::move(task));

			RunSignal();

			return ticket;
		}

	public:
		template<class TFUNC, class ...TARGS>
		//>> Запуск задачи :: возвращает ticket задачи :: будет ожидаться FreeResult() если <function> возврашает что-либо
		uint64 RunTaskRet(eTaskPriority priority, TFUNC function, TARGS... args)
		{
			return RunTaskProcWrapper < !std::is_member_function_pointer<TFUNC>::value, TFUNC, TARGS... > :: Get()
				( this, priority, false, function, args... );
		}

		template<class TFUNC, class ...TARGS>
		//>> Запуск задачи :: возвращает ticket задачи :: будет ожидаться FreeResult() если <function> возврашает что-либо
		uint64 RunTaskRet(TFUNC function, TARGS... args)
		{
			return RunTaskProcWrapper < !std::is_member_function_pointer<TFUNC>::value, TFUNC, TARGS... > :: Get()
				( this, TASK_PRIOR_DEFAULT, false, function, args... );
		}

		template<class TFUNC, class ...TARGS>
		//>> Запуск задачи :: возвращает ticket задачи :: игнорирует возвращаемый <function> результат [ нет ожидания FreeResult() ]
		uint64 RunTask(eTaskPriority priority, TFUNC function, TARGS... args)
		{
			return RunTaskProcWrapper < !std::is_member_function_pointer<TFUNC>::value, TFUNC, TARGS... > :: Get()
				( this, priority, true, function, args... );
		}

		template<class TFUNC, class ...TARGS>
		//>> Запуск задачи :: возвращает ticket задачи :: игнорирует возвращаемый <function> результат [ нет ожидания FreeResult() ]
		uint64 RunTask(TFUNC function, TARGS... args)
		{
			return RunTaskProcWrapper < !std::is_member_function_pointer<TFUNC>::value, TFUNC, TARGS... > :: Get()
				( this, TASK_PRIOR_DEFAULT, true, function, args... );
		}

		template<class TFUNC, class ...TARGS>
		//>> Запуск задачи :: возвращает ticket задачи :: игнорирует возвращаемый <function> результат [ нет ожидания FreeResult() ]
		uint64 operator()(eTaskPriority priority, TFUNC function, TARGS... args)
		{
			return RunTask ( priority, function, args... );
		}

		template<class TFUNC, class ...TARGS>
		//>> Запуск задачи :: возвращает ticket задачи :: игнорирует возвращаемый <function> результат [ нет ожидания FreeResult() ]
		uint64 operator()(TFUNC function, TARGS... args)
		{
			return RunTask ( function, args... );
		}
	};

	///////////////////////////////////////////////////////

	// DELETED : use std's as in example comparing floats
	// std::vector<std::pair<float, uint64>> map;
	// std::sort(map.begin(), map.end(), std::less_equal<>());
	// OR
	// std::map<float, uint64> map;
	// map.insert(std::pair<float, uint64>(...));

/*	template < typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type >
	//>> type T should be arithmetic, overwise it will not compile
	struct sortable_data
	{
		struct DATAKEY
		{
			     T d;
			uint64 k;
		};
		struct IDKEY
		{
			uint64 id;
			uint64 k;
		};

		sortable_data() : _size(0) {};
		~sortable_data(){};
		void create(uint64 capacity)
		{
			_data.Create(capacity);
			_ID.Close();
			_size = 0;
		}
		uint64 capacity()
		{
			return _data.count;
		}
		uint64 size()
		{
			return _size;
		}
		template <typename _Ty, typename _Key>
		bool put(_Ty && data, _Key && key)
		{
			static_assert (
				!std::is_same<_Ty, T>::value,
				"sortable_data : Type _Ty should be same as T"
			);
			static_assert (
				!std::is_same<_Key, uint64>::value,
				"sortable_data : Type _Key should be uint64"
			);

			if (_size == _data.count) return false;
			_data[_size].d = std::forward<_Ty>(data);
			_data[_size].k = std::forward<_Key>(key);
			_size++;
			return true;
		}
		void sort(bool from_min_to_max)
		{
			TBUFFER <uint64, uint64> _XX;
			_ID.Create(_size);
			_XX.Create(_size);
			_XX.Clear();

			if (from_min_to_max)					{
				for(uint64 i=0;   i<_size; i++)
				for(uint64 n=i+1; n<_size; n++)
					if (_data[i].d > _data[n].d)
						_XX[i]++; else _XX[n]++;	}
			else									{
				for(uint64 i=0;   i<_size; i++)
				for(uint64 n=i+1; n<_size; n++)
					if (_data[i].d < _data[n].d)
						_XX[i]++; else _XX[n]++;	}

			for(uint64 i=0; i<_size; i++)
			{ 
				_ID[_XX[i]].id = i;				// so _ID[0].id  will send us to _data[id] which is first (min or max)
				_ID[_XX[i]].k  = _data[i].k;	// copy keys from <_data> in correct order for quick grabbing
			}
		}
		bool is_sorted()
		{
			if (!_ID.count) return false;
			return (_size == _ID.count);
		}

		     T& operator[](uint64 get_data_at_sorted_id) { return _data[_ID[get_data_at_sorted_id].id].d; }
		uint64& operator()(uint64 get_key_at_sorted_id)  { return       _ID[get_key_at_sorted_id].k;      }

	private:
		TBUFFER <DATAKEY, uint64> _data;	// RAW original
		TBUFFER <IDKEY,   uint64> _ID;		// sorted id / keys
		uint64                    _size;	// count of presented RAW blocks
	}; //*/

	///////////////////////////////////////////////////////

	//>> Проверка доступности файла по одному из путей
	inline wchar_t* LoadFileCheck(uint8 num, wchar_t* p, ...)
	{
		wchar_t* ret = nullptr;
		wchar_t** pp = &p;
		for (uint8 i=0; i<num; i++)
		{
			FILE *fp = _wfsopen(*pp, L"rb", _SH_DENYNO);
			if (fp) {fclose(fp); ret=*pp; break;}
			pp++;
		}
		return ret;
	}

	template<typename... T64>
	//>> Проверка доступности файла по одному из путей
	inline wchar_t* LoadFileCheck64(T64&&... args)
	{
		std::vector< int64 > && argvec = initializer_list < int64 > { args... };
		for (auto & cur : argvec)
		{
			FILE *fp = _wfsopen(reinterpret_cast<wchar_t*>(cur), L"rb", _SH_DENYNO);
			if (fp) { fclose(fp); return reinterpret_cast<wchar_t*>(cur); }
		}
		return nullptr;
	}

	template<typename T1, typename T2>
	//>> Вспомогательная функция сверки 2х массивов	:: возвращает <true> если одинаковые
	inline bool Compare(const T1* array1, const T2* array2, const uint64 size)
	{
		static_assert (
			sizeof(T1) == sizeof(T2),
			"Compare<T1,T2>() : Types T1,T2 are not size-identical"
		);

		for (uint64 i=0;; array1++, array2++)	{
			if (*array1 ^ *array2) return 0;
			if (!(++i^size))       break;		}
		return 1;
	}

	//>> Взятие пути без имени файла
	inline std::wstring GetPrenamePath(const wchar_t* source)
	{
		std::wstring path(source);
		size_t last = path.find_last_of(L"/\\");
		return path.substr(0, last);
	}

	//>> Взятие имени файла из пути
	inline std::wstring GetFilename(const wchar_t* source)
	{
		std::wstring path(source);
		size_t last = path.find_last_of(L"/\\");
		return path.substr(last+1, path.size());
	}

	template <typename SZ>
	//>> Копирование файла с HDD в RAM
	inline bool ReadFileInMemory(const wchar_t* in_filepath, TBUFFER <byte, SZ> & out, bool show_error)
	{
		static_assert (
			std::is_integral<SZ>::value,
			"ReadFileInMemory<SZ>() : Type SZ should be integral"
		);

		wchar_t error[256];
		out.Close();

		wsprintf(error, L"%s %s", ERROR_OpenFile, in_filepath);
		FILE *fp = _wfsopen(in_filepath, L"rb", _SH_DENYNO);		if (!fp) { if (show_error) _MBM(error); return false; }

		out.Create(static_cast<SZ>(_filelengthi64(_fileno(fp))));
		fread(out.buf, sizeof(byte), out.count, fp);
		fclose(fp);

		return true;
	}

	///////////////////////////////////////////////////

	//>>
	inline void Makedirs(char* path)
	{
		char temp[0xff];
		size_t length = strlen(path);
		for (size_t x = 0; x<length; x++)
		{
			if (path[x]=='/'||path[x]=='\\')
			{
				memset(temp,0,sizeof(temp));
				strncat_s(temp,path,x);
				_mkdir(temp);
			};
		};
	}

	//>> 
	inline void Makedump(char* buf, long size, char* name)
	{
		Makedirs(name);
		FILE *fp = _fsopen(name, "wb", _SH_DENYNO);
		fwrite(buf, size, 1, fp);
		fclose(fp);
	}
}

#endif // _HELPER_H