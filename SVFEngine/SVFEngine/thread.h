// ----------------------------------------------------------------------- //
//
// MODULE  : thread.h
//
// PURPOSE : Классы для решения многопоточных задач
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _THREAD_HELPER_H
#define _THREAD_HELPER_H

#include "defines.h"
#include "types.h"
#include "vecdata.h"

#define mem_relax   std::memory_order_relaxed // NO BARRIER                   - no specific critical store/load logic
#define mem_seq_cst std::memory_order_seq_cst // ABSOLUTE BARRIER             - global - logic relative to many threads
#define mem_acq_rel std::memory_order_acq_rel // LOAD/STORE BARRIER           - local  - logic relative to 1 thread
#define mem_acquire std::memory_order_acquire // LOAD BARRIER : more stronger - local  - logic relative to 1 thread
#define mem_consume std::memory_order_consume // LOAD BARRIER : less stronger - local  - logic relative to 1 thread
#define mem_release std::memory_order_release // STORE BARRIER                - local  - logic relative to 1 thread

// DIFF BETWEEN  mem_acquire  AND  mem_consume
//
// on Intel x86/x64 : mem_consume == mem_acquire (NO DIFF, STRONG ONLY)
// 
//    MACHINE    : ACQUIRE - CONSUME    https://preshing.com/20140709/the-purpose-of-memory_order_consume-in-cpp11/
// Intel x86/x64 : 0.81 ns - 0.73 ns
//    PowerPC    : 16.3 ns - 3.09 ns
//     ARMv7     : 16.9 ns - 10.7 ns
//
// THREAD 1                                 <------     THREAD 2
// g = Guard.load(memory_order_consume);    <------     Payload = 42;
// if (g != nullptr)                        <------     Guard.store(&Payload, memory_order_release);
//     p = *g;
//
// memory_order_consume : (g != nullptr) NO GUARANTEED
// memory_order_acquire : (g != nullptr) GUARANTEED

// DIFF BETWEEN  mem_seq_cst  AND  mem_acq_rel
// 
// bool x = false;
// bool y = false;
// int z = 0;
//
// a() { x = true; }               // thread 1
// b() { y = true; }               // thread 2
// c() { while (!x); if (y) z++; } // thread 3 linked to thread 1,2
// d() { while (!y); if (x) z++; } // thread 4 linked to thread 1,2
//
// kick off a, b, c, d, join all threads
// assert(z != 0);
// 
// Operations on z are guarded by two atomic variables, not one,
// so you can't use acquire-release semantics to enforce that z is always incremented.
//
// ---------------------------
//
// Release-Acquire ordering guarantees everything that happened-before a store in one
// thread becomes a visible side effect in the thread that did a load. But in our example,
// nothing happens before store in both thread0 and thread1.
// 
// x.store(true, std::memory_order_release); // thread0
// 
// y.store(true, std::memory_order_release); // thread1
// 
// Further more, without memory_order_seq_cst, the sequential ordering of thread2
// and thread3 are not guaranteed.You can imagine they becomes :
// 
// if (y.load(std::memory_order_acquire)) { ++z; } // thread2, load y first
// while (!x.load(std::memory_order_acquire)); // and then, load x
// 
// if (x.load(std::memory_order_acquire)) { ++z; } // thread3, load x first
// while (!y.load(std::memory_order_acquire)); // and then, load y
//
// So, if thread2 and thread3 are executed before thread0 and thread1, that means
// both x and y stay false, thus, ++z is never touched, z stay 0 and the assert fires.

#define _TO_ALL(cv,lk)      std::notify_all_at_thread_exit(cv,std::move(lk))
#define _CV_ALL(cv)         cv.notify_all()
#define _CV_ONE(cv)         cv.notify_one()
#define _CV_WAIT(cv,lk)     cv.wait(lk)
#define _CV_FOR(cv,lk,time) cv.wait_for(lk,time) // timems time = 10 ; // 10 ms

#define _ATM_ST(a,x)        a.store(x, mem_seq_cst)     // ABSOLUTE BARRIER
#define _ATM_LD(a)          a.load(mem_seq_cst)         // ABSOLUTE BARRIER
#define _ATM_0(a)           a.store(false, mem_seq_cst) // ABSOLUTE BARRIER
#define _ATM_1(a)           a.store(true, mem_seq_cst)  // ABSOLUTE BARRIER
#define _ATM_ADD(a,x)	    a.fetch_add(x, mem_seq_cst) // ABSOLUTE BARRIER
#define _ATM_SUB(a,x)	    a.fetch_sub(x, mem_seq_cst) // ABSOLUTE BARRIER

#define _RLX_ST(a,x)        a.store(x, mem_relax)       // NO BARRIER
#define _RLX_LD(a)          a.load(mem_relax)           // NO BARRIER
#define _RLX_0(a)           a.store(false, mem_relax)   // NO BARRIER
#define _RLX_1(a)           a.store(true, mem_relax)    // NO BARRIER
#define _RLX_ADD(a,x)	    a.fetch_add(x, mem_relax)   // NO BARRIER
#define _RLX_SUB(a,x)	    a.fetch_sub(x, mem_relax)   // NO BARRIER

///// Для простой логики где нет сложной многопоточной зависимости ( если нет уверенности, использовать ATM ! ) /////

// Можно использовать по принципу REL -> AQR -> ACQ т.е. в посреднеческом потоке 
// 
// std::vector<int> data;
// std::atomic<int> flag = { 0 };
// 
// void thread_1()
// {
// 	data.push_back(42);
// 	flag.store(1, std::memory_order_release);
// }
// 
// void thread_2()
// {
// 	int expected = 1;
// 	while (!flag.compare_exchange_strong(expected, 2, std::memory_order_acq_rel)) {
// 		expected = 1;
// 	}
// }
// 
// void thread_3()
// {
// 	while (flag.load(std::memory_order_acquire) < 2)
// 		;
// 	assert(data.at(0) == 42); // равенство будет выполняться всегда
// }

#define _AQR_ADD(a,x)	    a.fetch_add(x, mem_acq_rel) // LOAD/STORE BARRIER
#define _AQR_SUB(a,x)	    a.fetch_sub(x, mem_acq_rel) // LOAD/STORE BARRIER
#define _AQR_LD(a)          a.load(mem_acq_rel)         // LOAD/STORE BARRIER
#define _AQR_ST(a,x)        a.store(x, mem_acq_rel)     // LOAD/STORE BARRIER
#define _AQR_0(a)           a.store(false, mem_acq_rel) // LOAD/STORE BARRIER
#define _AQR_1(a)           a.store(true, mem_acq_rel)  // LOAD/STORE BARRIER

///// Для простой логики (использовать только в местах с четко прослеживаемым кодом) /////

#define _REL_0(a)			a.store(false, mem_release) // STORE BARRIER
#define _REL_1(a)			a.store(true, mem_release)  // STORE BARRIER
#define _REL_ST(a,x)		a.store(x, mem_release)     // STORE BARRIER
#define _ACQ_LD(a)			a.load(mem_acquire)         // LOAD BARRIER
//#define _CON_LD(a)		a.load(mem_consume)         // LOAD BARRIER  (less stronger)

// THREAD 1                                 <------     THREAD 2
// g = Guard.load(memory_order_acquire);    <------     Payload = 42;
// if (g != 0)                              <------     Guard.store(1, memory_order_release);
//     p = Payload;

typedef std::unique_lock <std::mutex>  ulock;
typedef std::lock_guard  <std::mutex>  glock;

typedef std::unique_lock <std::recursive_mutex>  rulock;
typedef std::lock_guard  <std::recursive_mutex>  rglock;

typedef std::condition_variable cvariable;
typedef std::recursive_mutex    rmutex;

typedef std::_Cv_status cvstate;

#define _CV_NO_TIMEOUT  cvstate::no_timeout
#define _CV_TIMEOUT     cvstate::timeout

//std::this_thread::sleep_for(std::chrono::microseconds(1));
//nanoseconds, microseconds. milliseconds, seconds, minutes, hours
//std::chrono::high_resolution_clock::now();
//recursive_mutex, timed_mutex, recursive_timed_mutex

// Использование установки на конкретный процессор - реализовать?
// SetThreadIdealProcessor(thread.native_handle(), core_num)

namespace SAVFGAME
{
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
		TC_RESULT_NONE_IN,	// никаких результатов не предоставляется (выполнение в процессе)
		TC_RESULT_NONE_OUT, // никаких результатов не предоставляется (выполнение завершено)
		TC_RESULT_NONE,     // никаких результатов не предоставляется
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
		// https://www.acodersjourney.com/top-20-cplusplus-multithreading-mistakes/
		// If you just need some code executed asynchronously i.e. without blocking execution of Main thread,
		// your best bet is to use the std::async functionality to execute the code. The same could be achieved
		// by creating a thread and passing the executable code to the thread via a function pointer or lambda
		// parameter. However, in the later case, you're responsible for managing creation and joining/detaching of
		// them thread , as well as handling any exceptions that might happen in the thread. If you use std::async ,
		// you just get rid of all these hassels and also dramatically reduce chances of getting into a deadlock scenario.
		
		TRM_THREAD,              // run via std::thread	
		TRM_ASYNC,               // run via std::async  (std::launch::async)

		TRM_DEFAULT = TRM_THREAD // run this if you don't care about [TRM_THREAD is default]
	};

	// TODO: переброс исключений от созданного потока обратно вверх в основной !

	// TODO: настройка критичности задачи в потоке : bool critical_task;

	// TODO: лог без завершения main() при <critical_task> == false

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
		THREADDATA()                { Init(true); }           // Ловится баг, если detach() поток и быстро пересоздавать THREADDATA
		THREADDATA(bool use_detach) { Init(use_detach); }     // Причины не ясны (проблемы OS ?)
		~THREADDATA()               { Close(); Cooldown(); }  // Лечится малой задержкой деструктора
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
			future_holder = nullptr;
		}

	public:
		//>> Закрытие THREADDATA :: return TC_SUCCESS / TC_ALREADY :: auto FreeResult()
		eThreadCode Close()
		{
			glock stop_others_if_any (global_block);

			if (_ATM_LD(_closed)) return TC_ALREADY;

			FreeResultProc();
			WaitThread();
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
			glock stop_others_if_any(global_block);

			bool is_closed;

			{
				glock block_who_wait_for_opening (opening_block);

				is_closed = _ATM_LD(_closed);

				if (is_closed)
				{
					_ATM_ST(_detach, use_detach);
					_ATM_0(_closed);
				}

				_CV_ALL(opening_cv); // сообщаем в WaitOpen() что открыто
			}

			if (is_closed) return TC_SUCCESS;
			else           return TC_ALREADY;
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
			if (!_ATM_LD(_detach) && // Завершившийся неоткрепленный поток
				!_ATM_LD(_free) &&   // .
				_ATM_LD(_finished))  // .
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
					_CV_ALL(waiting_for_grab_cv);
				}			
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
			case TRM_THREAD: _thread.join(); return;
			case TRM_ASYNC:  WaitThread();   return;
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
			glock stop_others_if_any(global_block);

			if (_ATM_LD(_closed)) return TC_CLOSED;

			if (_ATM_LD(_detach) && !_ATM_LD(_free))
			{
				if (!wait)				      return TC_IN_PROGRESS;
				else {
					FreeResultProc();
					WaitThread();	          return TC_SUCCESS;
				}
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
			glock get_place_in_queue(grab_block);

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
		void WaitResult()
		{
			ulock lock (waiting_for_result_block);
			while (_ATM_LD(waiting_for_result))
				{ _CV_WAIT(waiting_for_result_cv, lock); }
		}
		//>> Отправляет вызвавшего в ожидание до окончания потока (_finished) :: NOTE _ if you forget to FreeResult() you may stuck here
		void WaitThread()
		{
			ulock lock (finish_working_block_all);
			while (!_ATM_LD(_finished))
				{ _CV_WAIT(finish_working_cv_all, lock); }
		}
		//>> Отправляет вызвавшего в ожидание, пока THREADDATA не станет открыта
		void WaitOpen()
		{
			ulock lock (opening_block);
			while (_ATM_LD(_closed))
				{ _CV_WAIT(opening_cv, lock); }
		}
		//>> Автоожидание [ WaitResult / WaitThread / WaitOpen ] :: NOTE _ here is no stuck with waiting result (like at WaitThread())
		eThreadCode Wait()
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
				else WaitThread();
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
				glock wait_detaching_by_runner (detach_block);

				(class_ptr->*class_function)(args...);

				if (_ATM_LD(_detach))
						Default();
				else Finish();
			}

			// сообщаем в WaitThread()
			{
				glock block_who_wait_separate(finish_working_block_all);
				_CV_ONE(finish_working_cv_one);
			}		
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Оболочка с авто-уведомлением о завершении потока (для не возвращающей ничего процедуры, с проверкой)
		void ThreadShellVoid(TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			{
				glock wait_detaching_by_runner (detach_block);

				auto func = std::mem_fn(class_function);							// проверка на ошибку времени компиляции
				static_assert (														// 
					std::is_void < decltype(func(*class_ptr, args...)) > ::value,	//
					"ThreadShellVoid : TFUNC returning type is not VOID");			//

				//	func(*class_ptr, std::forward<TARGS>(args)...);
				(class_ptr->*class_function)(args...);

				if (_ATM_LD(_detach))
					 Default();
				else Finish();
			}

			// сообщаем в WaitThread()
			{
				glock block_who_wait_separate(finish_working_block_all);
				_CV_ALL(finish_working_cv_all);
			}
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

				{
					glock wait_detaching_by_runner (detach_block);

					auto //std::future < std::result_of<decltype(func)(decltype(*class_ptr), TARGS...)>::type  >
						ret = std::async(
						std::launch::deferred,
						func,
						std::ref(*class_ptr),
						args...);

					value = ret.get();				// ждём и готовим результат к передаче
					ptr_to_result = (void*)&value;	// .
					_ATM_0(waiting_for_result);     // .
				}

				// зовём к выдаче
				{
					glock block_who_wait_for_result (waiting_for_result_block);
					_CV_ALL(waiting_for_result_cv);
				}

				if (_ATM_LD(waiting_for_grab)) // ждём, пока заберут результат и освободят нас
				{
					ulock lock (waiting_for_grab_block);
					while (_ATM_LD(waiting_for_grab))
						{ _CV_WAIT(waiting_for_grab_cv, lock); }
				}

				{
					glock wait_if_any_grabber_still_in_process (grab_block); // нас могли отпустить, но кто-то ещё в процессе

					//_ATM_0(working_via_result);

					ptr_to_result = nullptr;

					if (_ATM_LD(_detach))
						 Default();
					else Finish();
				}
			}

			// сообщаем в WaitThread()
			{
				glock block_who_wait_separate (finish_working_block_all);
				_CV_ALL(finish_working_cv_all);
			}
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск оболочки в поток
		eThreadCode RunThreadFunc(bool have_ret, int32 ID, eThreadRunMethod method, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			glock stop_others_if_any(global_block);

			if (_ATM_LD(_closed)) return TC_CLOSED;
			if (!Add())           return TC_NO_PLACE;

			_ATM_ST(_ID, ID);

			future_holder = nullptr;	 // free <future> from previous TRM_ASYNC call (if any)
			mem_run_method = method;

			if (have_ret) {
				_ATM_1(working_via_result);
				_ATM_1(waiting_for_result);
				_ATM_1(waiting_for_grab);
			}
			else		  {
				_ATM_0(working_via_result);
				_ATM_0(waiting_for_result);
				_ATM_0(waiting_for_grab);
			}

			if (method == TRM_THREAD)
			{
				glock stop_thread_for_detaching (detach_block);

				_thread = std::thread(class_function,
					class_ptr,
					std::forward<TARGS>(args)...);

				//printf("\nRUN THREAD ID 0x%x", _thread.get_id().hash());

				if (_ATM_LD(_detach)) _thread.detach();
			}
			else // TRM_ASYNC :: We need to hold <future> from std::async until next <Run> or <Close> call
			{
				auto func = std::mem_fn(class_function);

				std::shared_ptr<               CFutureHolder< decltype(func(*class_ptr, args...)) >> future_ptr;
				future_ptr = std::make_shared< CFutureHolder< decltype(func(*class_ptr, args...)) >>();

				(*future_ptr).result = std::async(std::launch::async,
					func,
					std::ref(*class_ptr),
					std::forward<TARGS>(args)...);
				future_holder = future_ptr;
			}

			return TC_SUCCESS;
		}

	public:
		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск функции в поток с игнорированием возвращаемого значения
		eThreadCode RunThreadIgnore(eThreadRunMethod method, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThreadFunc(false,
				MISSING,
				method,
				&THREADDATA::ThreadShell < TFUNC, TCLASS, TARGS... >,
				this,
				class_function,
				class_ptr,
				args...);
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск функции в поток (с ID) с игнорированием возвращаемого значения
		eThreadCode RunThreadIgnore(int32 ID, eThreadRunMethod method, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThreadFunc(false,
				ID,
				method,
				&THREADDATA::ThreadShell < TFUNC, TCLASS, TARGS... >,
				this,
				class_function,
				class_ptr,
				args...);
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск функции в поток с получением возвращаемого значения :: do not forget to FreeResult()
		eThreadCode RunThreadRet(eThreadRunMethod method, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThreadFunc(true,
				MISSING,
				method,
				&THREADDATA::ThreadShellRet < TFUNC, TCLASS, TARGS... >,
				this,
				class_function,
				class_ptr,
				args...);
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск функции в поток (с ID) с получением возвращаемого значения :: do not forget to FreeResult()
		eThreadCode RunThreadRet(int32 ID, eThreadRunMethod method, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThreadFunc(true,
				ID,
				method,
				&THREADDATA::ThreadShellRet < TFUNC, TCLASS, TARGS... >,
				this,
				class_function,
				class_ptr,
				args...);
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск функции в поток
		eThreadCode RunThread(eThreadRunMethod method, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThreadFunc(false,
				MISSING,
				method,
				&THREADDATA::ThreadShellVoid < TFUNC, TCLASS, TARGS... >,
				this,
				class_function,
				class_ptr,
				args...);
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск функции в поток (с ID)
		eThreadCode RunThread(int32 ID, eThreadRunMethod method, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThreadFunc(false,
				ID,
				method,
				&THREADDATA::ThreadShellVoid < TFUNC, TCLASS, TARGS... >,
				this,
				class_function,
				class_ptr,
				args...);
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> RunThread() operator()
		eThreadCode operator()(TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThread(TRM_DEFAULT,
				class_function,
				class_ptr,
				args...);
		}

		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> RunThread() operator() с идентификатором
		eThreadCode operator()(int32 ID, TFUNC class_function, TCLASS class_ptr, TARGS&&... args)
		{
			return RunThread(ID,
				TRM_DEFAULT,
				class_function,
				class_ptr,
				args...);
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
		std::condition_variable  finish_working_cv_all;		// организует ожидание до окончания потока
		std::mutex				 finish_working_block_all;	// организует ожидание до окончания потока
		std::condition_variable  waiting_for_result_cv;		// организует ожидание до получения результата от потока
		std::mutex				 waiting_for_result_block;  // организует ожидание до получения результата от потока
		std::condition_variable  waiting_for_grab_cv;		// организует ожидание потока до освобождения результата grabber'ом
		std::mutex				 waiting_for_grab_block;	// организует ожидание потока до освобождения результата grabber'ом
		/////////
		std::atomic<bool>		 working_via_result;	// метка схемы работы (работаем над результатом или нет)
		std::atomic<bool>		 waiting_for_result;	// флаг ожидания результата
		std::atomic<bool>		 waiting_for_grab;		// флаг ожидания, что результат заберут и отпустят поток
		std::atomic<void*>		 ptr_to_result;			// указатель на объект с результатом
		/////////
		eThreadRunMethod				 mem_run_method;	// какой конкретно функцией запускался поток
		std::shared_ptr<CFutureHolderT>	 future_holder;		// содержит future от запуска оболочки методом TRM_ASYNC
	};

	/////////////////////////////////////////////////////////////////////

	enum eTaskPriority : unsigned int // tasks priorities in waiting queue
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

	// TODO: возможность сразу задавать рабочему пару тысяч задач в собственную <queue>
	// TODO: подумать над условием в pop_front() balanced_queue
	// TODO: <expired_time> для задач с результатом - для автоудаления, если забыли забрать
	// TODO: сделать через std::bind, избавиться от std::tuple и RunTaskProcWrapper
	// TODO: подумать, что ещё может ускорить запуск (выделять заранее память ? / rvalue ? / ???)

	// _NOTE_ Выдаваемый пулом ticket никогда не NULL
	// _NOTE_ Долгосрочные и краткосрочные задачи лучше держать в разных очередях
	class THREADPOOL 
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
			 bool   drop_result;	// manager don't wait grabbers and remove (*this) after worker's done
			 bool   removed;		// user call to delete it from queue & don't run
			 void * ptr_to_result;	// provide to task result (if result isn't void)
		/////////	
			std::atomic<bool>  done_by_worker;	// task done by worker
			std::atomic<bool>  ready_to_remove; // task done by worker AND ready to removed (RESULT RELEASED or NO RESULT) 
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
		class balanced_queue
		{
			typedef typename balanced_queue<MAX,T> _THIS;
			typedef typename std::vector<T> chunk;
			typedef void (_THIS::*pFreeAtBackgroundProc)(std::vector<chunk> &);

		public:
			balanced_queue(const balanced_queue& src)            = delete;
			balanced_queue(balanced_queue&& src)                 = delete;
			balanced_queue& operator=(const balanced_queue& src) = delete;
			balanced_queue& operator=(balanced_queue&& src)      = delete;
		
		private:
			uint64 SZ;		// real count of T elements
			uint64 START;	// virtual start position of first [0] available T element
			uint64 TODEL;	// num of chunks prepared for deleting
			std::vector<chunk> data;

		private:
			std::atomic<uint64>		 free_mem_wait;			// simply block ~balanced_queue() until FreeAtBackgroundProc() done 
			std::mutex				 free_mem_wait_block;	// simply block ~balanced_queue() until FreeAtBackgroundProc() done
			std::condition_variable	 free_mem_wait_cv;		// simply block ~balanced_queue() until FreeAtBackgroundProc() done
			pFreeAtBackgroundProc    pFree;					//

		private:
			const uint64 TRIGGER_FOR_BACKGROUND_DELETE = 10;  // min chunks limit to del in cur thread
			const uint64 TRIGGER_FOR_FORCE_DELETE      = 100; // max chunks limit to awake auto delete

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
				return data [(size_t)GetQueueNum(element_id)] [(size_t)GetLocalID(element_id)];
			}
			uint64 size()
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

				uint64 Q  = SZ / MAX;	// queue id
				uint64 ID = SZ % MAX;	// element id in queue
				if (ID == 0 && Q > 0)
					make_chunk(true);
				data[(size_t)Q].emplace_back(forward<_Ty>(val));
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

						uint64 i, n=0, sz=(uint64)empty.size();
						data.reserve((size_t)(sz - TODEL));
						for (i=TODEL; i<sz; i++)	  make_chunk(false);							// fill with dummy
						for (i=TODEL; i<sz; i++, n++) std::swap(data[(size_t)n], empty[(size_t)i]);	// exchange dummy to real

						FreeAtBackground(empty); // push to delete

						SZ -= MAX * TODEL;
						START = 0;
						TODEL = 0;

						//printf("\n ... %i ms (Force clearing)", clock() - time);
					}
				}
			}
			uint64 chunks()
			{
				return TODEL;
			}

		private:
			uint64 GetQueueNum(uint64 location)
			{
				return (START + location) / MAX;
			}
			uint64 GetLocalID(uint64 location)
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
					_CV_ALL(free_mem_wait_cv);
				}
				//_CV_ALL(free_mem_wait_cv);
			}
		};
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
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
		struct POOLTICKET
		{
			POOLTICKET            (const uint64 &  src) { __raw = src; }
			POOLTICKET            (      uint64 && src) { __raw = src; }
			POOLTICKET & operator=(const uint64 &  src) { __raw = src; return *this; }
			POOLTICKET & operator=(      uint64 && src) { __raw = src; return *this; }

			operator uint64() { return __raw; }

			POOLTICKET() { __raw = 0; }

			POOLTICKET(uint32 _task_id, eTaskPriority _priority, uint16 _pool_id, uint16 _pool_unique_id)
			{
				task_id        = _task_id;
				priority       = _priority;
				pool_id        = _pool_id;
				pool_unique_id = _pool_unique_id;
			}

			union
			{
				uint64 __raw;
				struct
				{
					unsigned task_id        : 32; // 00000000FFFFFFFF
					unsigned priority       : 8;  // 000000FF00000000
					unsigned pool_id        : 12; // 000FFF0000000000
					unsigned pool_unique_id : 12; // FFF0000000000000
				};
			};
		};
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
		static uint16 pool_unique_id_counter; // счётчик экземпляров THREADPOOL
	protected:
		THREADDATA											_manager;	  // manage tasks & workers
		VECDATAP<WORKER>									_worker;	  // threads that do stuff
		std::atomic<int32>									_available;	  // num of workers without task
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
		uint64					 next_task_N;			// manager set data for worker
		bool					 worker_grab;		// manager waiting for worker while he grabbing data
		std::mutex				 worker_grab_block;	// manager waiting for worker while he grabbing data
		std::condition_variable	 worker_grab_cv;	// manager waiting for worker while he grabbing data
		//////////
		uint64					 tasks_run[TASK_PRIOR_LIMIT];	// определяет номер задачи в очереди, на которой остановились (used by manager only)
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

			uint64 old_size = _worker.size();
			_worker.AddCreate(count);
			uint64 new_size = _worker.size();

			for (uint64 i=old_size; i<new_size; i++)
				RunWorkerThread(static_cast<int32>(i));

			_ATM_ADD(_available, (int32)(new_size - old_size));
		}
		//>> Добавляет рабочих потоков до размера <count>
		void WorkersSet(uint16 count)
		{
			rglock stop_other_working_with_threads (thread_block);

			uint64 old_size = _worker.size();
			_worker.SetCreate(count);
			uint64 new_size = _worker.size();

			for (uint64 i=old_size; i<new_size; i++)
				RunWorkerThread(static_cast<int32>(i));

			_ATM_ADD(_available, (int32)(new_size - old_size));
		}
		//>> Сообщает количество задач на выполнении
		uint64 GetTasks(eTaskPriority priority)
		{
			glock stop_other_working_with_tasks (task_block);

			if (priority == TASK_PRIOR_LIMIT)
				priority = (eTaskPriority)(TASK_PRIOR_LIMIT - 1);

			uint64 ret  = 0;
			uint64 full = _task_queue[priority].size();

			if (full) ret = full - _ATM_LD(tasks_done[priority]);

			return ret;
		}
		//>> Сообщает количество задач на выполнении
		uint64 GetTasks()
		{
			glock stop_other_working_with_tasks (task_block);

			uint64 ret = 0;

			for (uint32 i=0; i<TASK_PRIOR_LIMIT; i++)
			{
				uint64 full = _task_queue[i].size();

				if (full) ret += full - _ATM_LD(tasks_done[i]);
			}

			return ret;
		}
		//>> Сообщает количество рабочих потоков
		uint64 GetWorkers()
		{
			rglock stop_other_working_with_threads (thread_block);

			return _worker.size();
		}

	/* public: //*/ private:
		//>> DEBUG : пользователю некорректно знать свободное число потоков - можно говорить только их полное число
		void _GetWorkers(uint64 & total, uint64 & free)
		{
			rglock stop_other_working_with_threads (thread_block);

			total = _worker.size();
			free  = _ATM_LD(_available);
		}
		//>> DEBUG : пользователю некорректно знать заполненность очереди - можно говорить только число ещё не выполненных задач
		uint64 _GetTasks(eTaskPriority priority)
		{
			glock stop_other_working_with_tasks (task_block);

			if (priority == TASK_PRIOR_LIMIT)
				priority = (eTaskPriority)(TASK_PRIOR_LIMIT - 1);

			return _task_queue[priority].size();
		}
		//>> DEBUG : пользователю некорректно знать заполненность очереди - можно говорить только число ещё не выполненных задач
		uint64 _GetTasks()
		{
			glock stop_other_working_with_tasks (task_block);

			uint64 ret = 0;
			for (uint32 i=0; i<TASK_PRIOR_LIMIT; i++)
				ret += _task_queue[i].size();

			return ret;
		}

	private:
		auto FindTaskProc   (uint32 TASK_ID, uint32 eTP) -> CThreadTaskT *
		{
			auto & queue = _task_queue [ (eTaskPriority) eTP ]; // MAX TASK ID == 0xFFFFFFFF
			uint64 queue_size = queue.size();					// if [3] tasks = { 0xFFFFFFFF, 0, 1 } --> N = 0x100000001 - 0x0FFFFFFFF = 2
			if (queue_size > 0)
			{ 
				uint64 N;
				uint32 TASK_0 = queue[0]->task_id;
				if (TASK_ID >= TASK_0)
					 N = (uint64)(TASK_ID - TASK_0);
				else N = (uint64)((0x100000000 | TASK_ID) - TASK_0);
				if ((N+1) > queue_size) return nullptr;
				if (queue[N]->task_id == TASK_ID)
					return queue[N].get();
			}
			return nullptr;
		}
		auto FindTaskProcS  (uint32 TASK_ID, uint32 eTP) -> std::shared_ptr < CThreadTaskT >
		{
			auto & queue = _task_queue [ (eTaskPriority) eTP ];
			uint64 queue_size = queue.size();
			if (queue_size > 0)
			{
				uint64 N;
				uint32 TASK_0 = queue[0]->task_id;
				if (TASK_ID >= TASK_0)
					 N = (uint64)(TASK_ID - TASK_0);
				else N = (uint64)((0x100000000 | TASK_ID) - TASK_0);
				if ((N+1) > queue_size) return nullptr;
				if (queue[N]->task_id == TASK_ID)
					return queue[N];
			}
			return nullptr;
		};
		auto FreeResultProc (uint32 TASK_ID, uint32 eTP) -> bool
		{
			auto & queue = _task_queue[eTP];
			uint64 queue_size = queue.size();
			if (queue_size > 0)
			{
				uint64 N;
				uint32 TASK_0 = queue[0]->task_id;
				if (TASK_ID >= TASK_0)
					 N = (uint64)(TASK_ID - TASK_0);
				else N = (uint64)((0x100000000 | TASK_ID) - TASK_0);
				if ((N+1) > queue_size) return false;
				if (queue[N]->task_id == TASK_ID)			{
					_ATM_1(queue[N]->ready_to_remove);
					return true;							}
			}
			return false;
		};

	public:
		//>> Ставит в ожидание, пока задачу не выполнят
		eThreadCode Wait(POOLTICKET ticket)
		{
			if (ticket.pool_unique_id != pool_unique_id) return TC_POOL_UID_MISMATCH;
			if (ticket.pool_id        != pool_id       ) return TC_POOL_ID_MISMATCH;

			std::shared_ptr < CThreadTaskT > task; // <shared_ptr> prevent deleting task object while we are waiting

			{
				glock stop_other_working_with_tasks (task_block);
				task = FindTaskProcS (ticket.task_id, ticket.priority);
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
		//>> Проверяет статус выполнения задачи без ожидания
		eThreadCode WaitCheck(POOLTICKET ticket)
		{
			if (ticket.pool_unique_id != pool_unique_id) return TC_POOL_UID_MISMATCH;
			if (ticket.pool_id        != pool_id       ) return TC_POOL_ID_MISMATCH;

			{
				glock stop_other_working_with_tasks (task_block);

				CThreadTaskT * task = FindTaskProc (ticket.task_id, ticket.priority);

				if (task == nullptr) return TC_POOL_TASK_NOT_FOUND;

				if (_ATM_LD(task->done_by_worker)) return TC_SUCCESS;
			}

			return TC_IN_PROGRESS;
		}	
		//>> Освобождает от ожидания результата, если такой есть и его ожидают (задача будет удалена)
		eThreadCode FreeResult(POOLTICKET ticket)
		{
			if (ticket.pool_unique_id != pool_unique_id) return TC_POOL_UID_MISMATCH;
			if (ticket.pool_id        != pool_id       ) return TC_POOL_ID_MISMATCH;

			{
				glock stop_other_working_with_tasks (task_block);

				if (FreeResultProc(ticket.task_id, ticket.priority)) // call manager & exit
					{ RunSignal(); return TC_SUCCESS; } 
			}

			return TC_POOL_TASK_NOT_FOUND;
		}
		template<class RESULT>
		//>> Возвращает результат на <save_out_location>, если он готов
		eThreadCode GrabResult(RESULT & ref_to_save_out_location, POOLTICKET ticket, bool wait, bool free_result_if_success)
		{
			if (ticket.pool_unique_id != pool_unique_id) return TC_POOL_UID_MISMATCH;
			if (ticket.pool_id        != pool_id       ) return TC_POOL_ID_MISMATCH;

			{
				ulock stop_other_working_with_tasks (task_block);

				std::shared_ptr < CThreadTaskT > task_shr = FindTaskProcS (ticket.task_id, ticket.priority);
				                  CThreadTaskT * task     = task_shr.get();

				if (task == nullptr)                    return TC_POOL_TASK_NOT_FOUND;
				if (task->removed)                      return TC_POOL_TASK_REMOVED;
				if (task->is_void || task->drop_result)
				{
					if (_ATM_LD(task->done_by_worker))  return TC_RESULT_NONE_OUT;
					else                                return TC_RESULT_NONE_IN;
				}
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
				if (_ATM_LD(task->ready_to_remove)) return TC_RESULT_LOCK;
				
				RESULT * result = reinterpret_cast<RESULT*>(task->ptr_to_result);
				ref_to_save_out_location = (*result);
				if (free_result_if_success)
					{ _ATM_ST(task->ready_to_remove, true); RunSignal(); } // call manager

				// if ((uint32)ref_to_save_out_location == 0xBAADF001) ERROR type
			}

			return TC_SUCCESS;
		}		
		template<class RESULT>
		//>> Возвращает результат на <save_out_location>, если он готов
		eThreadCode GrabResult(RESULT * ptr_to_save_out_location, POOLTICKET ticket, bool wait, bool free_result_if_success)
		{
			if (ticket.pool_unique_id != pool_unique_id) return TC_POOL_UID_MISMATCH;
			if (ticket.pool_id        != pool_id       ) return TC_POOL_ID_MISMATCH;

			{
				ulock stop_other_working_with_tasks (task_block);

				std::shared_ptr < CThreadTaskT > task_shr = FindTaskProcS (ticket.task_id, ticket.priority);
				                  CThreadTaskT * task     = task_shr.get();

				if (task == nullptr)                    return TC_POOL_TASK_NOT_FOUND;
				if (task->removed)                      return TC_POOL_TASK_REMOVED;
				if (task->is_void || task->drop_result)
				{
					if (_ATM_LD(task->done_by_worker))  return TC_RESULT_NONE_OUT;
					else                                return TC_RESULT_NONE_IN;
				}
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
				if (_ATM_LD(task->ready_to_remove)) return TC_RESULT_LOCK;
				
				RESULT * result = reinterpret_cast<RESULT*>(task->ptr_to_result);
				(*ptr_to_save_out_location) = (*result);
				if (free_result_if_success)
					{ _ATM_ST(task->ready_to_remove, true); RunSignal(); } // call manager

				// if ((uint32)(*ptr_to_save_out_location) == 0xBAADF001) ERROR type
			}

			return TC_SUCCESS;
		}
		//>> Удаляет задачу из списка задач на выполнение, если это ещё возможно
		eThreadCode RemoveTask(POOLTICKET ticket)
		{
			if (ticket.pool_unique_id != pool_unique_id) return TC_POOL_UID_MISMATCH;
			if (ticket.pool_id        != pool_id       ) return TC_POOL_ID_MISMATCH;

			{
				glock stop_other_working_with_tasks (task_block);

				CThreadTaskT * task = FindTaskProc (ticket.task_id, ticket.priority);

				if (task == nullptr)   return TC_POOL_TASK_NOT_FOUND;
				if (task->removed)     return TC_POOL_TASK_REMOVED;
				if (task->have_worker) return TC_POOL_TASK_NO_REMOVE;

				task->have_worker = true;
				task->drop_result = true;
				task->removed     = true;
				_ATM_ST(task->done_by_worker,  true);
				_ATM_ST(task->ready_to_remove, true);			
			}

			_ATM_ADD(tasks_done[ (eTaskPriority) ticket.priority ], 1); // tip to manager for delete task

			{
				glock lock(done_signal_block); // if somebody wait for end of work
				_CV_ALL(done_signal_cv);       // .
			}

			RunSignal(); // call manager

			return TC_SUCCESS;
		}

	protected:
		//>> Ожидание вызова от менеджера
		void WorkerWaitingProc()
		{
			ulock lock (work_signal_block);
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
					glock lock (work_signal_block);
					work_signal = true;
					_CV_ONE(work_signal_cv);
				}
				//printf("\nWorker exiting... [%i]", (int32)own_id);
				return true;
			}
			return false;
		}
		//>> Взятие следующей задачи
		void WorkerGetTask(CThreadTaskT * &task, eTaskPriority & mem_priority)
		{
			// Get data and response back to manager //

			{
				glock stop_other_working_with_tasks (task_block);
				mem_priority = next_task_priority;
				task = _task_queue[mem_priority][next_task_N].get();
				if (task->removed) // task can be removed between manager call and worker grab data
						task = nullptr;
				else task->have_worker = true;
			}

			{
				glock stop_manager_for_a_moment (worker_grab_block);
				worker_grab = true;
				_CV_ALL(worker_grab_cv); // wake up manager
			}

		}
		//>> Исполнение
		void WorkerRunTask(CThreadTaskT * &task, eTaskPriority   mem_priority)
		{
			if (task != nullptr) // task can be removed between manager call and worker grab data
			{
				task->Run();

				{
					glock stop_other_working_with_tasks (task_block);
					_ATM_ST(task->done_by_worker, true);			// set info that we are done
					if (task->is_void || task->drop_result)			// set to remove if it's needed
						{ _ATM_ST(task->ready_to_remove, true); }	// .
				}

				{
					glock stop_waiters_for_a_moment (done_signal_block);
					_CV_ALL(done_signal_cv);
				}

				_ATM_ADD(tasks_done[mem_priority], 1); // tip to manager
			}

			_ATM_ADD(_available, 1); // tip to manager

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
			_REL_ST(cooldown_time, nanoseconds);
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
			std::this_thread::sleep_for(std::chrono::nanoseconds(_ACQ_LD(cooldown_time)));
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
		void ManagerWork(eTaskPriority task_priority, ulock & stop_other_working_with_tasks)
		{
			auto & queue = _task_queue [task_priority];	// current priority queue 
			auto & N_run =  tasks_run  [task_priority];	// num of tasks has been run already
			
			// Drop old tasks // FIFO : First In - First Out //
			{
				uint64 limit, done = _ATM_LD(tasks_done[task_priority]);
				uint64 size  = queue.size();
				uint64 del   = 0;

				limit = (size >> 3) & 0x1FFFFFFFFFFFFFFF; // limit = 1/8 queue size

				if (done >= limit)
				{
					//printf("\ndone %i limit %i size %i", done, limit, size);		auto time = _TIME;
					for (uint64 i=0; i<size; i++)
					{
						if (_ATM_LD(queue[0]->done_by_worker) &&
							_ATM_LD(queue[0]->ready_to_remove))
							{ queue.pop_front(); del++; }
						else break;
					}
					if (del)
					{
						//printf("\ndeleted %i in %i ms", del, _TIMER(time));
						N_run -= del;
						_ATM_SUB(tasks_done[task_priority], del);
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
						// ! already synch. by <task_block> in WorkerGetTask()
						//{
						//	glock stop_workers_for_a_moment2(worker_grab_block); // synch. non-atomic data
						//	next_task_priority = task_priority;
						//	next_task_N = N_run;
						//}
						work_signal = true;
						_CV_ONE(work_signal_cv);
					}

					// Wait response from any worker //
					{
						stop_other_working_with_tasks.unlock(); // unlock <task_block> for worker, so he can grab data

						{
							ulock lock (worker_grab_block);
							while (worker_grab == false)
								{ _CV_WAIT(worker_grab_cv, lock); }
							worker_grab = false;
						}

						stop_other_working_with_tasks.lock();   // lock <task_block> back for continue working

						N_run++;
						_ATM_SUB(_available, 1); // -1 worker avail.

						if (!_ATM_LD(_available)) // if 0 avail.
							return;
					}
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
						 ulock stop_other_working_with_tasks   (task_block);
						rglock stop_other_working_with_threads (thread_block);
					
						for (int i=0; i<TASK_PRIOR_LIMIT; i++)				// WORKING POINT
							if (_task_queue[i].size())						// .
								ManagerWork(static_cast<eTaskPriority>(i),  // .
									stop_other_working_with_tasks);	        // .
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
			_ATM_ST(stop_signal, !_BOOL(remove_stop));
			if (remove_stop > 1) RunSignal();
		}
		//>> Отправляет менеджеру сигнал на работу
		void RunSignal()
		{
			if (_ATM_LD(stop_signal)) return; // user has blocked manager
			
			{
				glock stop_manager_for_a_moment (run_signal_block);
				run_signal = true;
				_CV_ALL(run_signal_cv);
			}
		}
		//>> Отправляет менеджеру и рабочим потокам сигнал на выход
		void ExitSignal()
		{
			_ATM_ST(exit_signal, true);

			{
				glock stop_manager_for_a_moment (run_signal_block);
				run_signal = true;
				_CV_ALL(run_signal_cv);
			}

			{
				glock stor_workers_for_a_moment (work_signal_block);
				work_signal = true;
				_CV_ONE(work_signal_cv);
			}
		}

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

			if (priority == TASK_PRIOR_LIMIT) priority = static_cast <eTaskPriority> (TASK_PRIOR_LIMIT - 1);

			POOLTICKET ticket ( (++task_id), priority, pool_id, pool_unique_id );
			
			TASK.func        = std::move(funcpack);
			TASK.args        = std::move(std::tuple<TARGS...>(args...));
			TASK.task_id     = ticket.task_id;
			TASK.drop_result = std::move(ignore_result);

			_task_queue[priority].emplace_back(std::move(task));

			RunSignal();

			return ticket;
		}

		template<class TFUNC, class ...TARGS>
		//>> Запуск задачи :: возвращает ticket задачи :: prepare static functions
		uint64 RunTaskProcStatic(eTaskPriority priority, bool ignore_result, TFUNC function, TARGS... args)
		{
			glock stop_other_working_with_tasks (task_block);

			std::packaged_task<              decltype(function(args...)) (TARGS...) >  funcpack(function);
			std::shared_ptr<    CThreadTask< decltype(function(args...)), TARGS... >>  task;
			task = make_shared< CThreadTask< decltype(function(args...)), TARGS... >>();
			auto &TASK = *task;

			if (priority == TASK_PRIOR_LIMIT) priority = static_cast <eTaskPriority> (TASK_PRIOR_LIMIT - 1);

			POOLTICKET ticket ( (++task_id), priority, pool_id, pool_unique_id );

			TASK.func        = std::move(funcpack);
			TASK.args        = std::move(std::tuple<TARGS...>(args...));
			TASK.task_id     = ticket.task_id;
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
}

#endif // _THREAD_HELPER_H