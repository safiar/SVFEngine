// ----------------------------------------------------------------------- //
//
// MODULE  : Sound.h
//
// PURPOSE : Базовый звуковой класс
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _SOUND_H
#define _SOUND_H

#include "stdafx.h"
#include "types.h"
#include "link_defines.h"
#include "WAVE.h"
#include "thread.h"

#include "SoundDevice.h"

#include "pcm.h"
#include "mp3.h"

#define CONTROL_THREADS     1                        // исполнитель от основного потока управления (всегда 1, нельзя больше 1)
#define CAPTURE_THREADS     1                        // потоков захвата звука (всегда 1)
#define RENDER_MIX_THREADS  1                        // потоков воспроизведения микшера (всегда 1)
#define RENDER_THREADS_MAX  60 + RENDER_MIX_THREADS  // максимум потоков вывода для заказа у аудиокарты (реальное число может быть меньше)
#define RENDER_THREADS_MIN  20 + RENDER_MIX_THREADS  // минимум потоков вывода для нормальной работы    (гарантированный)
#define MIXPOOL_THREADS     1 + RENDER_MIX_THREADS   // поток микшера (1) + воспроизведения (RENDER_MIX_THREADS)

#define MIXER_PERIODICITY     5 // мс, частота самопробуждений для проверки состояния
#define MIXER_PLAYBUFTIME    40 // мс, виртуальный размер рабочей области в конечном классе
#define MIXER_BUFTIME      1000 // мс, размер передаточного буфера микшера

#define MIXER_PLAYTHREAD_PERIODICITY (MIXER_PLAYBUFTIME / 8)                            // мс, частота пробуждений потока воспроизведения микшера
#define MIXER_PLAYTHREAD_WAIT_TIME   (MIXER_PLAYBUFTIME - MIXER_PLAYTHREAD_PERIODICITY) // мс, примерное время вступления изменений в силу

// EXAMPLE : PLAYBUFTIME = 200, MIXER_PLAYTHREAD_PERIODICITY = 100
// Поток воспр. держит наготове не менее PLAYBUFTIME данных (200 мс.)
// Просыпается раз в [PLAYBUFTIME / 2] (100 мс.) проверить, осталось ли в аудиокарте менее PLAYBUFTIME (200 мс.)
// Если да, то забирает у микшера ещё PLAYBUFTIME данных : [0..200] + 200 = [200..400] мс. имеет наготове в результате
//                                                
// !    !    !    !    !    !    !    !    !      Микшер должен успевать обеспечивать PLAYBUFTIME данных (200 мс.)
// +---------+---------+---------+---------+      с частотой PERIODICITY <= 0.5 * [PLAYBUFTIME / 2] == (50 мс.)
// 0    50   100       200       300       400
//
// Тестовый процессор : DualCore E8400 3.00 GHz
// При MIXER_PERIODICITY = 1 мс : слишком высокая частота проверок, нестабильная работа !
// При MIXER_PERIODICITY = 5 мс : стабильно 20 mp3-источников (PLAYBUFTIME 40 мс) (RELEASE билд, 7-11 мс время конвертации 20 источников) (DEBUG: 5 источников, 15-19 мс)

// макс. семплов/блоков может быть в передаточном буфере
#define MIXER_MAX_BUFTIME_SAMPLES  (uint32)( ((float)MIXER_BUFTIME / 1000) * PCM_MAXSAMPLESPERSEC )

#define SOUND_ALL            MISSING  // заказ всех ID
#define DEFAULT_SOUND_GROUP  0        // номер user-группы по умолчанию

#define SOUND_QUEUE_NOT_LOOP TASK_PRIOR_NORMAL // очередь в пуле потоков для незацикленных звуков
#define SOUND_QUEUE_LOOP     TASK_PRIOR_LOW    // очередь в пуле потоков для   зацикленных звуков

// TODO допилить SetVolumeChannels() для группы и для SOUND_ALL
// TODO ошибки выделения памяти? не везде всё под контролем, перепроверить позже
 
namespace SAVFGAME
{
	enum eSoundError : unsigned int
	{
		eSoundError_FALSE   = 0,  // BASIC FAIL         :: work failed and aborted
		eSoundError_TRUE    = 1,  // BASIC SUCCESS      :: work has been done normally

		eSoundError__NORMAL_BEGIN,	// begin of normal exit codes section

		eSoundError_Normal_1,		// played and exited with no errors
		eSoundError_Normal_2,		// played and exited with interrupt call
		eSoundError_Normal_End,		// no sound data to convert & play
		eSoundError_Normal_None,    // aborted by (!async) lock guard

		eSoundError__NORMAL_END,	// end of normal exit codes section

		////////////////////////

		eSoundError__COMMON_BEGIN, // begin of common errors section

		eSoundError_Unknown,       // [DEBUG] undefined exit code
		eSoundError_NoInit,        // CSound -basic [BASE CLASS IS NOT INITED]
		eSoundError_NoInit_F,      // CSound -final [FINAL CLASS IS NOT INITED]
		eSoundError_NoInit_R,      // CSound -final [RENDER MANAGER FAILED]
		eSoundError_NoInit_C,      // CSound -final [CAPTURE MANAGER FAILED]
		eSoundError_MaxThreads,    // reached threads limit
		eSoundError_BadID_G,       // incorrect sound id (bad group)
		eSoundError_BadID_GE,      // incorrect sound id (bad group -> element)
		eSoundError_BadID_E,       // incorrect sound id (bad group -> element -> content)
		eSoundError_BadSndPTR,     // snd[id] is nullptr
		eSoundError_BadSndDescPTR, // SNDDESC is nullptr
		eSoundError_BadFilePTR,    // sound buffer is nullptr
		eSoundError_BadFileSZ,     // sound buffer size is 0
		eSoundError_BadAlloc,      // malloc error
		eSoundError_BadFormat,     // unsupported sound type
		eSoundError_BadTaskID,     // incorrect task id (invalid threadpool id)
		eSoundError_NoneTask,      // task not found
		eSoundError_pAudioData,	   // [INTERFACE ERROR] pAudioData is nullptr
		eSoundError_pCaller,	   // [INTERFACE ERROR] pCaller is nullptr
		eSoundError_pClient,       // [INTERFACE ERROR] pClient is nullptr
		eSoundError_pRender,       // [INTERFACE ERROR] pRender is nullptr
		eSoundError_pVolume,       // [INTERFACE ERROR] pVolume is nullptr
		eSoundError_wFormatTag,    // target mode format is NOT a PCM / IEEE_FLOAT (can't convert others)
		eSoundError_noConvPtrs,    // failed to get converters ptrs
		eSoundError_soundType,	   // unknown/unmanaged type of sound data
		eSoundError_convBuf,	   // failed to alloc memory for working window
		eSoundError_convAAC,       // aac converter Run() error
		eSoundError_convOGG,       // ogg converter Run() error
		eSoundError_convMP3,       // mp3 converter Run() error
		eSoundError_convPCM,       // pcm converter Run() error
		eSoundError_BadBufSz,      // real buffer size returned by audiocard is too small
		eSoundError_BadBufSzMixer, // real buffer size returned by audiocard < virtual buffer size requested by mixer
		eSoundError_duration,	   // playback duration is <= 0 (internal error)
		eSoundError_diffChNum,     // unexpected difference in channels count (internal safe checking error)
		eSoundError_BadPath,       // [LOAD ERROR] string ptr or string size is null
		eSoundError_BadBits,       // [LOAD ERROR] unexpected format (incorrect bits per sample)
		eSoundError_BadChannels,   // [LOAD ERROR] unexpected format (incorrect num of channels)
		eSoundError_BadFreq,       // [LOAD ERROR] unexpected format (incorrect samples per sec)
		eSoundError_BadWaveTag,    // [LOAD ERROR] unexpected format (incorrect audio format tag)
		eSoundError_ReadFileWAV,   // [LOAD ERROR] error reading *.wav file
		eSoundError_ReadFileMP3,   // [LOAD ERROR] error reading *.mp3 file	
		eSoundError_ReadFileOGG,   // [LOAD ERROR] error reading *.ogg file	
		eSoundError_ReadFileAAC,   // [LOAD ERROR] error reading *.aac file	
		eSoundError_UnkFileType,   // [LOAD ERROR] unknown type of input file
		eSoundError_FileNotFound,  // [LOAD ERROR] can't open file or there is no file at all
		eSoundError_MaxNumFiles,   // [LOAD ERROR] reached maximum of uint32 [0xFFFFFFFF]
		eSoundError_MaxGroupID,    // [LOAD ERROR] group ID limit : num of user's groups limited by value <eSoundGroup_USER_MAX>
		eSoundError_MaxGroup,      // [LOAD ERROR] too much elements in group : limited by value <eSoundGroup_ELEMENT_MAX>
		eSoundError_NoneFiles,     // Sound engine content nothing
		eSoundError_Corrupted,     // One or more IDs in group corrupted and skipped
		eSoundError_NoMixer,       // mixer hasn't been actived
		eSoundError_DeviceChanged, // RunThreadPlay() blocked by DefaulDeviceChanged()
		eSoundError_MixerNoThr,    // mixer detected that playthread has never been runned
		eSoundError_MixerLostThr,  // mixer detected that playthread has been shuted down and never returned
		eSoundError_SequenceFail,  // ОШИБКА ПОСЛЕДОВАТЕЛЬНОСТИ ВЫЗОВОВ	

		eSoundError__COMMON_END,   // end of common errors section

		////////////////////////

		eSoundError__CALL_BEGIN, // begin of calling interfaces errors section

		eSoundError_getBufSz,   // failed to pClient->GetBufferSize()
		eSoundError_getBuf,		// failed to pRender->GetBuffer()
		eSoundError_releaseBuf, // failed to pRender->ReleaseBuffer()
		eSoundError_getChNum,   // failed to pVolume->GetChannelCount()	
		eSoundError_setChVol,	// failed to pVolume->SetChannelVolume()
		eSoundError_start,		// failed to pClient->Start()
		eSoundError_stop,		// failed to pClient->Stop()
		eSoundError_padding,    // failed to pClient->GetCurrentPadding()

		eSoundError__CALL_END,	// end of calling interfaces errors section

		////////////////////////

		eSoundError__MANAGER_BEGIN,	// begin of manager errors section

		eSoundError_RENDERREINIT,	// render  ManageSession ReInit()
		eSoundError_CAPTUREREINIT,	// capture ManageSession ReInit()
		eSoundError_RMAXCLIENTS,	// [DEBUG] render  ManageSession MAX NUM OF CLIENTS REACHED
		eSoundError_CMAXCLIENTS,	// [DEBUG] capture ManageSession MAX NUM OF CLIENTS REACHED

		eSoundError__MANAGER_END,	// end of manager errors section

		////////////////////////

		eSoundError__MASK_PLAYER_ERROR    = 0x000000FF, // 000000FF : mask to get basic <eSoundPlayError> error code
		eSoundError__MASK_CONVERTER_ERROR = 0xFF000000, // FF000000 : mask to get aac/ogg/mp3/pcm converter error code
		eSoundError__MASK_CONVERTER_SHIFT = 24,			// FF000000 : bits to shift for set/get converter error code

		eSoundError__MISSING = (unsigned int) MISSING
	};
	enum eSoundType : unsigned int
	{
		eSoundType_UNKNOWN,	// unknown data

		eSoundType_WAVE,	// PCM/IEEE wave riff audio
		eSoundType_MP3,		// MPEG-1 Layer-3 format
		eSoundType_OGG,		// Ogg Vorbis format
		eSoundType_AAC,		// Advanced Audio Coding format

		eSoundType_ENUM_MAX
	};
	enum eSoundCallReason : unsigned int
	{
		eSoundCallReason_NONE             = 0,

		eSoundCallReason_Interrupt        = MASK_BIT_00, // прерывание
		eSoundCallReason_Pause            = MASK_BIT_01, // пауза
		eSoundCallReason_Rewind_NO_WAITER = MASK_BIT_02 | 0,           // перемотка - основной бит        (нет ожидающего)
		eSoundCallReason_Rewind_WAITER    = MASK_BIT_02 | MASK_BIT_03, // перемотка - основной и доп. бит (есть ожидающий)
		eSoundCallReason_MasterVolume     = MASK_BIT_04, // общая громкость
		eSoundCallReason_VolumeMultiplier = MASK_BIT_05, // громкость для конкретного звука
		eSoundCallReason_ChannelVolume    = MASK_BIT_06, // громкость на каналы для конкретного звука

		eSoundCallReason_Resume = MASK_BIT_07, // спец.флаг ожидания выхода из паузы

		eSoundCallReason_SelfBack  = MASK_BIT_08, // спец.флаг самовозврата
		eSoundCallReason_SelfPause = MASK_BIT_09, // спец.флаг самопаузы

		eSoundCallReason_MixerRun     = MASK_BIT_10, // новый поток микшера
		eSoundCallReason_MixerPause   = MASK_BIT_11, // пауза работы микшера
		eSoundCallReason_MixerResume  = MASK_BIT_12, // продолжение работы микшера
		eSoundCallReason_MixerAbort   = MASK_BIT_13, // завершает поток микшера
		eSoundCallReason_MixerNewTask = MASK_BIT_14, // передана новая задача в микшер
		eSoundCallReason_MixerUpdate  = MASK_BIT_15, // пользователь внёс или изменил задачу - требуется внеочередное обновление
		eSoundCallReason_MixerERR     = MASK_BIT_16, // ошибка работы микшера : нет потока воспроизведения

		// групповой признак перемотки (основной и доп. биты)
		eSoundCallReason_Rewind = eSoundCallReason_Rewind_NO_WAITER |
		                          eSoundCallReason_Rewind_WAITER,

		// групповой признак перемотки (основной бит) + паузы
		eSoundCallReason_Stop = eSoundCallReason_Rewind_NO_WAITER |
								eSoundCallReason_Pause,

		// групповой признак смены громкости
		eSoundCallReason_VolumeAll = eSoundCallReason_MasterVolume     |
		                             eSoundCallReason_VolumeMultiplier |
									 eSoundCallReason_ChannelVolume,
	};
	enum eSoundRunSettings : unsigned int
	{
		eSoundRunNONE        = 0,           // простой вызов по умолчанию (NO ASYNC, NO LOOP, NO STOP_START/END, NO MIX)
		eSoundRunAsync       = MASK_BIT_00, // многопоточный запуск (иначе - другие новые запуски блокируются)
		eSoundRunLoop        = MASK_BIT_01, // зацикленный звук
		eSoundRunStopAtStart = MASK_BIT_02, // запуск не сразу, а по вызову Resume()
		eSoundRunStopAtEnd   = MASK_BIT_03, // не закрывать поток после окончания (TIP: недействительно при loop)
		eSoundRunMix         = MASK_BIT_04, // запуск через свой микшер (не требует доп. интерфейса у аудиокарты) 
	};
	enum eSoundGroup : unsigned int
	{
		// группы внутреннего пользования

		eSoundGroup_MIX      = 0, // для воспроизведения с собственным микшированием
		eSoundGroup_CAPTURE  = 1, // для записанного с микрофона
		
		// группы пользователя
		
		eSoundGroup_USER        = 2,       // общий доступ начиная с этого ID и выше
		eSoundGroup_USER_MAX    = 1000,    // максимальное число групп пользователя 1000 = 0x3E8
		eSoundGroup_ELEMENT_MAX = 1000000, // максимальное число элементов в группе 1000000 = 0xF4240
		eSoundGroup_CAPACITY    = 100,     // резервировать по N мест
	};
	enum eSoundDelete
	{
		eSoundDelete_simple,		// удаляет только сам звуковой файл для данного ID [FAST, DEFAULT]
		eSoundDelete_hex,			// удаляет всю ячейку
		eSoundDelete_reconstruct,	// удаляет всю ячейку и проводит уменьшение массива указателей
	};

	// Загрузочный список
	struct CSoundLoadList
	{
		struct ___
		{
			___ ()
			{
				file        = nullptr;
				GID.group   = eSoundGroup_USER_MAX;    //DEFAULT_SOUND_GROUP;
				GID.element = eSoundGroup_ELEMENT_MAX; //MISSING;
				linked_GID  = nullptr;
				result      = 0;
			}

			TBUFFERS <byte, int32> * file; // in     : loading from memory
			wstring       filename;        // in     : loading from <subpath>\<filename>
			wstring       objname;         // in     : formal name
			uint64_gid    GID;             // in/out : group_ID in, element_ID / GID out
			uint64_gid *  linked_GID;      // in     : loading from other GID to load from [nullptr if not used]
			error_t       result;          // out    : content <eSoundError_TRUE> if success
		};

		wstring        gamepath; // in     : <mainpath>
		VECPDATA <___> info;     // in/out : various
	};

	// Сводка статуса
	struct CSoundState
	{
		uint64 GID;                  // / СВОДКА / GID данного звука
		float  time       { 0.f };   // / СВОДКА / полное время воспроизведения
		float  time_t     { 0.f };   // / СВОДКА / текущая точка воспроизведения (в секундах)
		float  time_p     { 0.f };   // / СВОДКА / текущая точка воспроизведения (в процентах, от 0.f до 1.f)
		bool   auto_pause { false }; // / СВОДКА / задача в собственной автопаузе (в начале или конце)
		bool   user_pause { false }; // / СВОДКА / задача в паузе по вызову Control*() функции пользователем
		bool   in_mixer   { false }; // / СВОДКА / задача запущена через микшер
	};

	// Базовый класс звукового движка
	class CSound
	{	
		#define ALLOWED_NUM_OF_COMMON_INTERFACES (actual_render_interfaces - RENDER_MIX_THREADS) // поправка с учётом резерва

		#define _SoundID(gid) (*snd_g[(int32)gid.group + eSoundGroup_USER])[gid.element] // извлекает фактический ID звука

		#define MASTER_VOLUME_DEFAULT 0.5f // общая громкость, значение для конструктора класса

	public:
		//>> Дополнительные статусы состояния для регулировки специфических ситуаций в критических секциях
		struct SNDSTAT
		{
			// базовый блок для работы в критических секциях

			bool  runned      { false }; // <false> = начальная пауза        (исп. блок m_pause) : main_thread <-- sound_thread
			bool  in_pause    { false }; // собственный статус паузы потока  (исп. блок m_pause) : main_thread <-- sound_thread
			bool  awakened    { false }; // признак разбуженности из паузы   (исп. блок m_pause) : main_thread --> sound_thread
			bool  rewinded    { false }; // признак разбуженности перемоткой (исп. блок m_pause) : main_thread --> sound_thread

			// второстепенная информация пользователю (внешняя сводка)
		public:
			mutex             m_STATSELF; // замок для заполнения информации STATSELF либо через функции UpdateInfo***()
			CSoundState         STATSELF; // собственная информация
		private:
			CSoundState         STATOUT;
			CSoundState * const STATOUT_P { & STATOUT };

		public:
			//>> Сброс дополнительных статусов состояния
			void Close()
			{
				runned   = false;
				in_pause = false;
				awakened = false;
				rewinded = false;

				glock lock (m_STATSELF);

				STATSELF.GID        = MISSING;
				STATSELF.time_t     = 0.f;
				STATSELF.time_p     = 0.f;
				STATSELF.auto_pause = false;
				STATSELF.user_pause = false;
			}

			//>> Сообщает текущий статус 
			CSoundState * UpdateInfoGet()
			{
				{
					glock lock (m_STATSELF);

					STATOUT = STATSELF;
				}

				return STATOUT_P;
			}

			void UpdateInfoTime(float _time_t, float _time_p)
			{
				glock lock (m_STATSELF);

				STATSELF.time_t = _time_t;
				STATSELF.time_p = _time_p;
			}
			void UpdateInfoPause(bool _auto_pause, bool _user_pause)
			{
				glock lock (m_STATSELF);

				STATSELF.auto_pause = _auto_pause;
				STATSELF.user_pause = _user_pause;
			}
		};
		//>> Обеспечивает ожидание главного потока
		struct SNDWAIT
		{
			mutex				m_wait;			// блок на работу со счетчиком
			cvariable			cv_wait;		// ожидание счетчика
			uint32				counter { 0 };	// счетчик
			uint32				target  { 0 };	// целевое число ожидаемых потоков
		};
		//>> Зовущий потока (посредник между основным потоком и потоком воспроизведения)
		struct SNDCALL
		{
			//>> Управление ожиданиями
			struct SNDWAIT_MANAGER {
				#define i_interrupt  0
				#define i_interrupt2 1
				#define i_pause      2
				#define i_rewind     3
				#define i_resume     4
				#define i_MAX        5
					private:
						mutex m_wait_manager; // блок на работу с указателями SNDWAIT *			
						union
						{
							SNDWAIT	* __union [i_MAX]; // ожидания одним массивом
							struct
							{
								SNDWAIT	* interrupt;	// [0] ожидает прерывание : основной поток или поток DefaulDeviceChanged()
								SNDWAIT	* interrupt_2;	// [1] ожидает прерывание : основной поток или поток DefaulDeviceChanged()
								SNDWAIT	* pause;		// [2] ожидает вход в паузу (pause или stop)
								SNDWAIT	* rewind;		// [3] ожидает окончание перемотки
								SNDWAIT	* resume;		// [4] ожидает выход из паузы
							};
						};
					public:
						SNDWAIT_MANAGER()  { _nullptr_clear(); };
						~SNDWAIT_MANAGER() { Close(); }
						//>> Быстрая очистка (после полных перезапусков)
						void _nullptr_clear()
						{
							for (auto & p : __union)
								p = nullptr;
						}
						//>> Закрывает все ожидания (из потока)
						void Close()
						{
							glock lock (m_wait_manager);

							for (auto & p : __union)
								if (p != nullptr)
								{
									{
										glock lock (p->m_wait);
										p->counter++;
										_CV_ALL(p->cv_wait);
									}
									p = nullptr;
								}
						}
						//>> Сообщает указатель (в поток)
						void Store(SNDWAIT * ptr, eSoundCallReason type)
						{
							glock lock (m_wait_manager);

							switch (type)
							{
							case eSoundCallReason_Interrupt:
								if (interrupt == nullptr)
									 interrupt   = ptr; // основной поток или поток DefaulDeviceChanged()
								else interrupt_2 = ptr; // .
								break;
							case eSoundCallReason_Pause:
							case eSoundCallReason_Stop:
								pause = ptr;
								break;
							case eSoundCallReason_Rewind_WAITER:
							case eSoundCallReason_Rewind_NO_WAITER:
						//	case eSoundCallReason_Rewind:
								rewind = ptr;
								break;
							case eSoundCallReason_Resume:
								resume = ptr;
								break;
							default:
								_MBM(L"ERR = Store(SNDWAIT * ptr, eSoundCallReason type)");
								return;
							}
						}
						//>> Закрывает ожидание (из потока)
						bool Close(eSoundCallReason type)
						{
							glock lock (m_wait_manager);

							bool closed = false;

							if (type == eSoundCallReason_Interrupt)
							{
								for (int i = i_interrupt; i <= i_interrupt2; i++)
								{
									auto & p = __union[i];
									if (p != nullptr)
									{
										{
											glock lock(p->m_wait);
											p->counter++;
											_CV_ALL(p->cv_wait);
										}
										p = nullptr;
										closed = true;
									}
								}
							}
							else
							{
								uint32 N = 0;

								switch (type)
								{
									case eSoundCallReason_Stop:
									case eSoundCallReason_Pause:
										N = i_pause;
										break;
									case eSoundCallReason_Rewind_WAITER:
									case eSoundCallReason_Rewind_NO_WAITER:
								//	case eSoundCallReason_Rewind:
										N = i_rewind;
										break;
									case eSoundCallReason_Resume:
										N = i_resume;
										break;
									default:
										_MBM(L"ERR = Close(eSoundCallReason type)");
										return closed;
								}

								auto & p = __union[N];
								if (p != nullptr)
								{
									{
										glock lock (p->m_wait);
										p->counter++;
										_CV_ALL(p->cv_wait);
									}
									p = nullptr;
									closed = true;
								}
							}

							return closed;
						}
				#undef i_interrupt
				#undef i_interrupt2
				#undef i_pause
				#undef i_rewind
				#undef i_resume
				#undef i_MAX
			}; 

			mutex		m_call;       // останов для cv_call
			cvariable	cv_call;      // обеспечивает пробуждение по вызову
			bool		call   { 0 }; // признак вызова
			uint32		reason { eSoundCallReason_NONE }; // причины вызова <eSoundCallReason>

			SNDWAIT_MANAGER         wait;				 // обратная связь на ожидание
			SNDWAIT_MANAGER * const wait_ptr { & wait }; // обратная связь на ожидание (ptr)

			SNDSTAT status;

		public:
			float  rewind_time      { 0.f };   // время к перемотке
			bool   rewind_new_order { false }; // признак нового заказа

		public:

			//>> [SOUND THREAD] Сброс информации о вызове !! исп. блок <m_call> "сверху"
			void Close()
			{
				call   = false;
				reason = eSoundCallReason_NONE;
			}
			
			//>> [MAIN THREAD] Пробуждает поток (если спит) и оставляет заметки с указанием причины
			void Awake(eSoundCallReason thread_call_reason)
			{
				glock wait_a_moment_my_dear_thread (m_call);

				reason |= thread_call_reason; // установка битов вызова
				call    = true;
				_CV_ALL(cv_call);
			}

			//>> [MAIN THREAD] Пробуждает поток (если спит) и оставляет заметки с указанием причины (вариант для перемотки)
			void Awake(eSoundCallReason thread_call_reason, float rewind_time_f)
			{
				glock wait_a_moment_my_dear_thread (m_call);

				rewind_time = rewind_time_f;
				rewind_new_order = true;

				reason |= thread_call_reason; // установка битов вызова
				call = true;
				_CV_ALL(cv_call);
			}

			//>> [SOUND THREAD] Сообщает время к перемотке, сбрасывает признак нового заказа !! исп. блок <m_call> "сверху"
			float GetRewindTime()
			{
				rewind_new_order = false;

				return rewind_time;
			}

			//>> Для перезапуков при смене задачи воспроизведения (вызов из SNDCHUNK)
			void ResetRewindStatus()
			{
				glock lock (m_call);

				rewind_time      = 0.f;
				rewind_new_order = false;
			}
		};
		//>> Данные к запуску потока воспроизведения
		struct SNDCHUNK
		{
			struct {
				uint64_gid GID             { MISSING }; // комплексный идентификатор
				void *     mixer_p         { nullptr }; // совместные данные с микшером (для потока воспр. микшера)
				void *     actual_sound    { nullptr }; // непосредственный указатель на структуру звука в памяти
				bool       loop            { false };	// настройка цикличности
				bool       stop_at_start   { false };   // пауза перед запуском
				bool       stop_at_end     { false };   // пауза после окончания (не сбрасывать поток)
				bool       mixer           { false };   // запускаемый поток является интерфейсом для микшера
			};	
			SNDCALL * const caller { &_ }; // зовущий
		private:
			SNDCALL _ ;
		public:
			//>> Производит подготовку на основе принятых настроек запуска
			void Init()
			{
				//_quick_clear();

				if (stop_at_start)
				{
					// <false> как признак что мы ждём, что нас вытолкнут из первичной паузы
					_.status.runned = false;

					// сменится на <true> если нас запустят
					_.status.awakened = false;
					_.status.rewinded = false;
				}
				else
				{
					// воспроизведение сразу
					_.status.runned = true;
				}

				// поскольку потока ещё не существует, mutex <m_STATSELF> не нужен
				_.status.STATSELF.GID      = GID;
				_.status.STATSELF.time     = reinterpret_cast<SNDDATA*>(actual_sound) -> duration;
				_.status.STATSELF.in_mixer = false;
			}

			//>> Сброс настроек запуска
			void Close()
			{
				GID           = MISSING;
				mixer_p       = nullptr;
				actual_sound  = nullptr;
				loop          = false;
				stop_at_start = false;
				stop_at_end   = false;
				mixer         = false;
			}

			//>> Быстрое закрытие всего (для выхода потока)
			void _quick_close()
			{
				Close();
				_.Close();
				_.ResetRewindStatus();
				_.status.Close();
				_.wait.Close();			
			}

			//>> Быстрая очистка всего (для полных перезапусков)
			void _quick_clear()
			{
				Close();
				_.Close();
				_.ResetRewindStatus();
				_.status.Close();
				_.wait._nullptr_clear();
			}
		};
	public:
		//>> Физ. данные звука
		struct SNDDATA
		{
		public:
			SNDDATA(const SNDDATA& src)				= delete;
			SNDDATA(SNDDATA&& src)					= delete;
			SNDDATA& operator=(SNDDATA&& src)       = delete;
			SNDDATA& operator=(const SNDDATA& src)	= delete;
		public:
			SNDDATA() : snddesc(&_) { Close(); };
			~SNDDATA() { Close(); }
			void Close()
			{
				file.Close();
				filename.clear();
				objname.clear();
				snddesc->Reset();

				file_offset       = 0;
				file_size         = 0;
				type              = eSoundType_UNKNOWN;
				duration          = 0;
				players           = 0;
				players_mixer     = 0;
				pause             = false;
			//	modified          = false;
			//	modified_clr_wav  = false;
				GID               = MISSING;
				baddat_err        = eSoundError_Unknown;

				_ATM_ST(a_volume_multiplier, 1.f);
				_ATM_ST(a_interrupt,         false);
				_ATM_ST(a_playblock,         false);

				ResetChannelsVolume();
			}
			error_t CheckFile()
			{
				error_t ret = eSoundError_TRUE;

					 if (file.buf   == nullptr) ret = eSoundError_BadFilePTR;
				else if (file.count == 0)       ret = eSoundError_BadFileSZ;

				return ret;
			}
			////////////
			void ResetChannelsVolume()
			{
				glock others_will_wait (m_volume_ch);

				static_assert (
					8 == PCM_MAXCHANNELS,
					"FIX HERE IF MAXCHANNELS CHANGES !"
				);

				static const float ___ [ PCM_MAXCHANNELS ] = { 1, 1, 1, 1, 1, 1, 1, 1 };
				//std::fill_n(___, PCM_MAXCHANNELS, 1);

				memcpy(volume_ch, ___, PCM_MAXCHANNELS * sizeof(float));
			}
			////////////
			void GetChannelsVolume(const float * volume_PCM_MAXCHANNELS_floats)
			{
				glock others_will_wait (m_volume_ch);

				memcpy((void*)volume_PCM_MAXCHANNELS_floats, volume_ch, PCM_MAXCHANNELS * sizeof(float));
			}
			void GetChannelsVolumeStereo(float & left, float & right)
			{
				glock others_will_wait (m_volume_ch);

				left  = volume_ch[0];
				right = volume_ch[1];
			}
			void GetChannelsVolumeMono(float & mono)
			{
				glock others_will_wait (m_volume_ch);

				mono  = volume_ch[0];
			}
			////////////
			void SetChannelsVolume(float volume)
			{
				glock others_will_wait (m_volume_ch);

				for (int i = 0; i < PCM_MAXCHANNELS; i++)
					volume_ch[i] = _CLAMP(volume, 0.f, 1.f);
			}
			void SetChannelsVolume(const float * volume_PCM_MAXCHANNELS_floats)
			{
				glock others_will_wait(m_volume_ch);

				for (int i = 0; i < PCM_MAXCHANNELS; i++)
					volume_ch[i] = _CLAMP(volume_PCM_MAXCHANNELS_floats[i], 0.f, 1.f);
			}
			void SetChannelsVolumeStereo(float left, float right)
			{
				glock others_will_wait (m_volume_ch);

				volume_ch[0] = _CLAMP(left,  0.f, 1.f);
				volume_ch[1] = _CLAMP(right, 0.f, 1.f);
			}
			void SetChannelsVolumeMono(float mono)
			{
				glock others_will_wait (m_volume_ch);

				volume_ch[0] = _CLAMP(mono, 0.f, 1.f);
				volume_ch[1] = volume_ch[0];
			}

			////////////
			void Link(SNDDATA & other)
			{
				file.Link(other.file);

				other.file_offset = file_offset;
				other.file_size   = file_size;
				other.filename    = filename;
				other.duration    = duration;
				other.type        = type;
				other._           = _;  // свойства WAV
				other.baddat_err  = eSoundError_TRUE;
			}
			////////////
			TBUFFERS <byte, int32>	file;				 // полнозагруженный файл данных звука
			uint32					file_offset;		 // смещение относительно начала файла на фактические данные
			uint32					file_size;		     // размер фактических данных
			wstring					filename;			 // имя файла-источника звука
			wstring                 objname;             // формальное имя звука
			float					duration;			 // (в секундах) доп. информация
			atomic <float>			a_volume_multiplier; // out = master * multiplier * channel
			eSoundType				type;				 // исходный формат файла (wav, mp3, ogg и т.п.)
			SNDDESC	* const			snddesc;			 // свойства WAV
			mutex					m_playblock;         // блокировщик воспроизведения в (!asynch) режиме
			atomic <bool>           a_playblock;         // блокировщик воспроизведения в (!asynch) режиме [MIXER-related]
			atomic <bool>			a_interrupt;		 // прерыватель потока
			bool					pause;				 // пауза потока                   (исп. блок m_pause)
			mutex					m_pause;			 // обеспечивает ожидание в паузе
			cvariable				cv_pause;			 // обеспечивает ожидание в паузе
			uint32					players;			 // количество текущих потоков     (исп. блок m_listRenderID)
			uint32                  players_mixer;       // количество текущих в микшере   (исп. блок m_mixer из данных микшера)
			uint64                  GID;                 // обратная связь на группу		
			error_t                 baddat_err;          // запомненная ошибка (выход воспроизведения из-за бракованных данных)
		public:
	//	TODO : когда и где проставлять ? как организовать захват блока извне ?
	//		bool					modified;			 // изменяемый <file.buf> (например, запись с микрофона)
	//		bool                    modified_clr_wav;    // авто-очистка считанного "modified" .WAVE
	//		mutex					m_modified;          // блок чтения-записи <file.buf>
		private:
			SNDDESC _;									        // 
			float					volume_ch[PCM_MAXCHANNELS]; // громкость по каналам
			mutex					m_volume_ch;                // останов при работе с громкостью по каналам
		public:
		//	atomic <uint32>			debug_num_in_pause { 0 };
		};
	public:
		CSound(const CSound& src)			 = delete;
		CSound(CSound&& src)				 = delete;
		CSound& operator=(const CSound& src) = delete;
		CSound& operator=(CSound&& src)		 = delete;
	protected:
		uint32 actual_render_interfaces  { 0 }; // доступно интерфейсов воспроизведения (исп. блок <m_init_close>)
		uint32 actual_capture_interfaces { 0 }; // доступно интерфейсов записи          (исп. блок <m_init_close>)
	protected:
//	public:
		uint32                    snd_count; // актуальное число звуковых файлов
		VECPDATA <SNDDATA>	      snd;       // данные загруженных звуковых файлов
		VECPDATA <vector<uint32>> snd_g;     // данные о группах звуковых файлов
		atomic <float>		a_master_volume; // общая громкость
	   rmutex				m_init_close;	 // [приоритет блока 0] контроль доступа Init() / Close() в конечном классе
		mutex				m_load_delete;   // [приоритет блока 1] останов при загрузке/удалении данных (блок касательно DefaulDeviceChanged())
	//	mutex               m_run_play;      // [приоритет блока 2] останов на запусках воспроизведения
		mutex               m_control;       // [приоритет блока 3] автозащита от многопоточного вызова public Control-функций
	protected:
		THREADPOOL			playpool;		 // содержит задачи и потоки воспроизведения (отдельные, без микшера)
		THREADPOOL			recpool;		 // содержит задачи и потоки записи
		THREADPOOL			controlpool;     // содержит очередь задач управления для работы в стиле "дал задачу - вернусь за результатом потом"
		THREADPOOL			mixpool;		 // содержит поток микшера и поток воспроизведения для микшера
	private:
		VECPDATA <SNDCHUNK> listRenderID;    // набор всех ID, проигрываемых в данный момент (max RENDER_THREADS_MAX)
		uint32 listRenderID_count;           // актуальное количество в списке listRenderID  (max RENDER_THREADS_MAX)
		mutex  m_listRenderID;               // блок на работу со списком listRenderID
	public:
		struct MIXERDATA {
			// MIXLIST      : данные и статус касательно отдельной задачи микшера
			// THREADDATA   : данные статуса потока воспроизведения
			// BUFFERDATA   : данные касательно работы по наполнению буфера воспроизведения
			// EXTRADATA    : вспомогательные данные для работы
			// THREADSHARED : данные потока воспр. по позиции чтения в буфере микширования
			struct MIXLIST {
				struct MIXSET
				{
					void Reset(uint32 eSoundRunSet)
					{
						async         = _BOOL(eSoundRunSet & eSoundRunAsync);
						loop          = _BOOL(eSoundRunSet & eSoundRunLoop);
						stop_at_start = _BOOL(eSoundRunSet & eSoundRunStopAtStart);
						stop_at_end   = _BOOL(eSoundRunSet & eSoundRunStopAtEnd);
					}

					bool async;         // запуск в асинхр. режиме
					bool loop;          // зацикленное воспр.
					bool stop_at_start; // пауза в начале
					bool stop_at_end;   // пауза в конце		
				};
				struct MIXRULE
				{
					void Reset()
					{
						pause        = false;
						rewind       = false;
						rewind_pause = false;
						interrupt    = false;
						volume       = false;

						rewind_time  = 0.f;
					}

					bool  pause;        // пауза
					bool  rewind;       // перемотка
					bool  rewind_pause; // пауза после перемотки
					bool  interrupt;    // завершение звука
					bool  volume;       // изменение громкости

					float rewind_time;  // время к перемотке				
				};
				struct MIXCONV
				{
					void Reset()
					{
						ConverterMP3 = nullptr;
						ConverterOGG = nullptr;
						ConverterAAC = nullptr;
						first_run    = true;
						storage_cur_ptr  = nullptr;
						storage_cur_size = 0;
						storage_update   = true;
					}

					CMP3Converter * ConverterMP3; // декодер *.mp3
					void *          ConverterOGG; // reserved
					void *          ConverterAAC; // reserved

					bool first_run; // статус первого прохода

					TBUFFER <byte, uint32> storage; // хранилище сконвертированного
					byte * storage_cur_ptr;  // текущая позиция в хранилище
					uint32 storage_cur_size; // размер готового в хранилище
					bool   storage_update;   // перепроверка размера хранилища при смене задачи

					SNDDESC	        snddesc;                // настройки параметров, лежащего в хранилище
					SNDDESC	* const snddesc_ptr {&snddesc}; // настройки параметров, лежащего в хранилище
				};
				struct MIXPOS
				{
					float  time;  // [позиция] текущее время в секундах
					uint32 frame; // [позиция] текущий кадр или семпл
					byte * ptr;   // [позиция] текущий адрес в буфере-источнике звука

					uint32 _f2b;  // множитель для перевода кадров в байты
					float  _t2p;  // множитель для перевода секунд в проценты
				};
				struct MIXVOL
				{
					void Reset(float master_volume)
					{
						multiplier = 1.f;
						for (auto & ch_value : value)
							ch_value = 1.f;
						for (auto & ch_result : result)
							ch_result = master_volume;
					}

					float multiplier; // общий множитель
					float value[8];   // громкости по каждому из 8 каналов
					float result[8];  // громкости по каждому из 8 каналов (итоговый подсчёт)
				};
				uint64_gid             GID;       // GID звука к воспроизведению
				SNDDATA *              sound;     // фактические данные звука
				SoundMixSource         mixdata;                    // (настройка) данные к смешиванию
				SoundMixSource * const mixdata_ptr { & mixdata } ; // (настройка) данные к смешиванию
				MIXPOS                 POS;       // позиции задачи
				MIXSET                 SET;       // настройки запуска
				MIXRULE                RULE;      // управляющие команды
				MIXCONV                CONV;      // конверторы, статус первого прохода
				MIXVOL                 VOL;       // настройки громкости
				error_t                ERR;       // код выхода
				CSoundState            STATSELF;                // собственная постоянно обновляемая информация
				CSoundState            STATOUT;                 // заполняется только при выдаче UpdateInfoGet()
				CSoundState * const    STATOUT_P { & STATOUT }; // адрес на выдачу
				bool  b_throwed;  // основной поток передал задачу в микшер
				bool  b_accepted; // микшер принял задачу к выполнению
				bool  b_done;     // выполнена (на удаление из списка)

				//>> Сообщает текущий статус 
				CSoundState * UpdateInfoGet()
				{
					STATOUT = STATSELF;
					return STATOUT_P;
				}
			};
			struct THREADDATA
			{
				bool     thread_run  {0}; // статус запуска потока воспроизведения
				bool     thread_exit {0}; // статус выхода  потока воспроизведения
				bool     thread_snd  {0}; // статус готовности звуковых данных для потока воспроизведения
				error_t  thread_ERR;   // код выхода потока воспроизведения
				mutex    m_thread;     // блок для работы с <thread> данными
			};
			struct BUFFERDATA
			{
				byte * pos_mixer;  // позиция микшера в буфере
				byte * pos_player; // позиция потока воспр. в буфере
				byte * pos_start;  // позиция начала
				byte * pos_end;    // позиция конца
			};
			struct EXTRADATA
			{
				uint32 frame_size;    // размер кадра (блока) в байтах (nBlockAlign)
				uint32 second_size;   // размер одной секунды в байтах
				float  second_size_f; // множитель для конвертации размера буфера в секунды = 1.f / second_size
				float  buffer_time;   // размер буфера данных в секундах
				uint32 buffer_size;   // размер буфера данных в байтах
				int    null_sample;   // значение пустого DUMMY семпла
				
				float  min_time;    // минимальное кол-во времени, которое должен обеспечить микшер
				uint32 min_frames;  // минимальное кол-во кадров, которое должен обеспечить микшер
				uint32 min_buf_sz;  // минимальное размер буфера, который должен обеспечить микшер

				uint32 done_buf_sz; // последний раз было подготовлено для потока воспр.

				T_TIME sleep_timepoint; // точка отсчета выхода из сна
			};
			struct THREADSHARED
			{
				mutex	m_ReadWrite; // блок совместной работы с микшером
				uint32	rw_pos;      // точка чтения (номер кадра/блока) буфера потоком воспроизведения
				uint32  rw_count;    // счетчик количества прочитанных кадров/блоков  !! не более чем MIXER_MAX_BUFTIME_SAMPLES !!
			};
		//	VECPDATA <COGGConverter> ConverterOGG; // (m_mixer) набор декодеров *.ogg
		//	VECPDATA <CAACConverter> ConverterAAC; // (m_mixer) набор декодеров *.aac	
			VECPDATA <CMP3Converter> ConverterMP3; // (m_mixer) набор декодеров *.mp3
			uint32                   ConverterMP3_tasks; // (m_mixer) количество рабочего в списке
			uint32                   ConverterOGG_tasks; // (m_mixer) количество рабочего в списке
			uint32                   ConverterAAC_tasks; // (m_mixer) количество рабочего в списке
			CPCMMixer                MixerPCM;     // смешиватель PCM
			vector < SoundMixSource * > mixdata;   // набор данных к смешиванию
			float                    master_volume { MASTER_VOLUME_DEFAULT }; // общая громкость (исп. блок <m_mixer>)
			SNDCHUNK *               list {0};     // отдельный "list" вместо использования общего из набора listRenderID[]
			THREADDATA               THR;          // данные статуса потока воспроизведения, воспроизводящего звук от микшера
			THREADSHARED             THRS;         // совместные данные микшера и потока воспроизведения
			eSoundCallReason         THR_status;   // статус управления микшером потока воспроизведения
			T_TIME                   THR_timer;    // счетчик до сообщения о критическом сбое
			error_t                  THR_ERR;      // запомненный статус ошибки для вывода при критическом сбое	
			BUFFERDATA               BFD;          // данные по позициям в буфере воспроизведения
			EXTRADATA                EX;           // доп. запомненные данные
			VECPDATA <MIXLIST>       task;         // (m_mixer) содержит текущий список к работе
			uint32                   task_num {0}; // (m_mixer) количество рабочего в списке (включая ещё не принятое микшером)
			uint32                   task_num_accepted {0};  // количество рабочего в списке, принятого микшером к работе в данный момент
			mutex                    m_mixer;      // [DATA] останов работы с общими данными микшера (данными задач к смешиванию и меткой вызова <call_r>)
			cvariable                cv_mixer;     // [DATA] пробуждает микшер из собственного периодического сна, отправляет работать с данными
			mutex                    m_wait;       // [WAIT] останов на ожидание действий микшера (запуск/окончание, пауза/продолжение)
			cvariable                cv_wait;      // [WAIT] микшер пробуждает ожидающего после выполнения действий (запуск/окончание, пауза/продолжение)
			mutex                    m_pause;      // [PAUSE] останов при работе с меткой паузы микшера <pause_end>
			cvariable                cv_pause;     // [PAUSE] пробуждает микшер из паузы
			uint32                   call_r_local; // (-------) содержит биты признаков вызова (собственная внутренняя копия микшера)
			uint32                   call_r {0};   // (m_mixer) содержит биты признаков вызова 
			bool                     state_runned {0}; // [ВНЕШНЯЯ МЕТКА УПРАВЛЕНИЯ МИКШЕРОМ] статус запуска микшера
			bool                     state_paused {0}; // [ВНЕШНЯЯ МЕТКА УПРАВЛЕНИЯ МИКШЕРОМ] статус паузы   микшера	
			bool                     wait_exit;    // (m_wait) признак выxода (внешнее уведомление)
			bool                     wait_run;     // (m_wait) признак входа  (внешнее уведомление)
			bool                     wait_pause;   // (m_wait) признак входа  в  паузу (внешнее уведомление)
			bool                     wait_resume;  // (m_wait) признак выхода из паузы (внешнее уведомление)
			bool                     pause_end;    // (m_pause) признак на выход из паузы для микшера
			bool                     exit;         // собственный признак завершения работы потока микшера
			bool                     ERR_once;     // признак установки ERR (для однократной установки)
			atomic <error_t>         ERR { eSoundError_TRUE }; // содержит код ошибки / код выхода  микшера
		};
	private:
		MIXERDATA mixerdata;
	private:
		typedef error_t (CSound::*pSoundPlayFunc) (SNDCHUNK * list, bool async);
		typedef error_t (CSound::*pSoundRecFunc)  (uint64 id);
		pSoundPlayFunc const	pPlayFunc;		// Play()
		pSoundRecFunc  const    pRecFunc;		// Record()
	private:
		bool  isInit;
	public:
		CSound() : 
			isInit(false),
			snd_count(0),
			listRenderID_count(0),
			a_master_volume(MASTER_VOLUME_DEFAULT),
			pPlayFunc(&CSound::Play),
			pRecFunc(&CSound::Record)
		{
			// Проверка корректности пред-настроек

			static_assert (
				CONTROL_THREADS == 1,
				"CSound() : Only one control thread allowed"
			);

			static_assert (
				RENDER_MIX_THREADS == 1,
				"CSound() : Only one mix thread allowed"
			);

			static_assert (
				CAPTURE_THREADS == 1,
				"CSound() : Only one capture thread allowed"
			);

			static_assert (
				MIXPOOL_THREADS == 2,
				"CSound() : mixpool threads should be 2 in total"
			);

			static_assert (
				(float)MIXER_BUFTIME > ((float)MIXER_PLAYBUFTIME * 2.2),
				"CSound() : MIXER_BUFTIME should be > MIXER_PLAYBUFTIME * 2.2"
			);

			static_assert (
				MIXER_PERIODICITY <= (MIXER_PLAYBUFTIME / 4),
				"CSound() : MIXER_PERIODICITY should be <= MIXER_PLAYBUFTIME / 4"
			);

			static_assert (
				MIXER_PERIODICITY >= 5,
				"CSound() : MIXER_PERIODICITY should be >= 5 ms"
			);
			
		};
		virtual ~CSound() { Close(); }

	protected:
		virtual void DefaulDeviceChanged(eSoundDevType type, uint32 iteration) = 0;

	protected:
		virtual error_t RunPlayback(SNDCHUNK * list) = 0;
		virtual error_t RunRecord(uint64 id) = 0;
		virtual error_t ActivateMixer()   = 0;
		virtual error_t DeActivateMixer() = 0;
	public:
		virtual error_t ReActivateMixer()   = 0;
		virtual error_t ReActivateRender()  = 0;
		virtual error_t ReActivateCapture() = 0;

	public:
		virtual bool GetStateInit()       = 0;
		virtual bool GetStateMicrophone() = 0;
		virtual bool GetStateSpeakers()   = 0;
		virtual byte GetNumChannels()     = 0;

	private: // malloc error ??? FIX LATER !!!
		bool ResetList()
		{
			// выставим на максимальное число (потолок)
			listRenderID.SetCreate(RENDER_THREADS_MAX);
			for (auto cur : listRenderID)
				cur->_quick_clear();
			listRenderID_count = 0;

			return true;
		}
	
	public:
		virtual bool Init()
		{
			if (isInit) return true;

			// malloc error ??? FIX LATER !!!
			snd_g.SetCreate(eSoundGroup_USER);

			// malloc error ??? FIX LATER !!!
			if (!ResetList()) return false;

			controlpool.WorkersSet(CONTROL_THREADS); // 1   контроллер
			mixpool.WorkersSet(MIXPOOL_THREADS);     // 2   микшер + воспр. микшера
			recpool.WorkersSet(CAPTURE_THREADS);     // 1   запись
			//playpool на саморегуляции

			return isInit = true;
		};
		virtual void Close()
		{
			if (isInit)
			{
				Delete();

				playpool.Close();
				recpool.Close();
				controlpool.Close();
				mixpool.Close();

				_ATM_ST(a_master_volume, 0.5f);

				isInit = false;
			}
		};

	//////////////// Mixer block ////////////////

	private:
		error_t ActivateMixerPlayDataPRIV(SNDDESC * snddesc)
		{
			// в данном блоке создаём SNDDATA и группу eSoundGroup_MIX (если нет)

			error_t hRes = eSoundError_TRUE;

			glock lk_thread (mixerdata.THR.m_thread);

			if (mixerdata.THR.thread_snd) // уже существует
				return hRes;              // .

			//////////////////////////////

			if (snd_count == snd.size())   // Добавим
				snd.emplace_back_create(); // .

			uint64 id = snd_count++;       // ID текущего

			if (snd[id] == nullptr)        // Создадим
				snd.ReCreate(id);          // .

			//////////////////////////////

			SNDDATA & sound = *snd[id];

			mixerdata.EX.frame_size    = snddesc->nChannels * (snddesc->wBitsPerSample >> 3);
			mixerdata.EX.second_size   = snddesc->nSamplesPerSec * mixerdata.EX.frame_size;
			mixerdata.EX.second_size_f = 1.f / mixerdata.EX.second_size;
			mixerdata.EX.buffer_time   = (float)MIXER_BUFTIME / 1000;
			mixerdata.EX.buffer_size   = (uint32) ceil(mixerdata.EX.buffer_time * mixerdata.EX.second_size);
			mixerdata.EX.null_sample   = (snddesc->wBitsPerSample <= 8) ? PCM_NULL8BIT : 0;

			// создание целевого буфера
			if (sound.file.Create(mixerdata.EX.buffer_size))
			{
				uint64_gid GID;
				GID.element = 0; // всегда 1 в группе
				GID.group   = (uint32) (eSoundGroup_MIX - eSoundGroup_USER) ;

				*sound.snddesc    = *snddesc; // запоминаем формат (копия данных)			
				sound.duration    = mixerdata.EX.buffer_time;
				sound.GID         = GID;
				sound.baddat_err  = eSoundError_TRUE;
				sound.type        = eSoundType_WAVE;
				sound.players     = 1; // всегда 1 поток воспроизводит
				sound.filename    = L"MIXER_PLAYDATA";
				sound.file_offset = 0;
				sound.file_size   = sound.file.count;

				// передаём в рабочую часть микшера профиль выходного формата
				mixerdata.MixerPCM.target = sound.snddesc;

				// расчет доп. данных
				mixerdata.EX.min_time     = (float)MIXER_PLAYBUFTIME / 1000;
				mixerdata.EX.min_buf_sz   = (uint32) ceil(mixerdata.EX.min_time * mixerdata.EX.second_size);
				mixerdata.EX.min_frames   = mixerdata.EX.min_buf_sz / mixerdata.EX.frame_size;
				mixerdata.EX.done_buf_sz  = 0;

				// обратные поправки на случай некратности
				mixerdata.EX.min_buf_sz = mixerdata.EX.min_frames * mixerdata.EX.frame_size;
				mixerdata.EX.min_time   = (float) mixerdata.EX.min_frames / snddesc->nSamplesPerSec;

				// + 2 кадра для состыковки при микро-расхождениях (фикс "потрескивания" звука)
				mixerdata.EX.min_buf_sz += 2 * mixerdata.EX.frame_size;

				// настройка позиций
				mixerdata.BFD.pos_start  = sound.file.buf;                    // sound.file_offset = 0;
				mixerdata.BFD.pos_end    = sound.file.buf + sound.file.count; // sound.file_size   = sound.file.count;
				mixerdata.BFD.pos_mixer  = sound.file.buf;
				mixerdata.BFD.pos_player = sound.file.buf;

				// сброс общих данных микшера с потоком
				mixerdata.THRS.rw_count = 0;
				mixerdata.THRS.rw_pos   = 0;

				// предварительно очистить целевой буфер
				memset(sound.file.buf, mixerdata.EX.null_sample, sound.file.count);
			}
			else // ошибка выделения памяти
			{
				sound.Close();
				return hRes = eSoundError_BadAlloc;
			}

			//////////////////////////////

			if (eSoundGroup_USER > snd_g.size())   // создаём группы, если нет
				snd_g.SetCreate(eSoundGroup_USER); // .

			auto & group = *snd_g[eSoundGroup_MIX];

			group.resize(1);                         // запомним настоящий ID
			if (group.size() < 1)                    // .
			{                                        // .
				sound.Close();                       // .
				return hRes = eSoundError_BadAlloc;  // .
			}			                             // .
			group[0] = (uint32) id;                  // .

			mixerdata.THR.thread_snd = true; // отметим, что существует

			return hRes;
		}
		error_t ActivateMixerPlayThreadPRIV()
		{
			// в данном блоке создаём SNDCHUNK (list) для потока и запускаем сам поток

			error_t hRes = eSoundError_TRUE;

			glock lk_thread (mixerdata.THR.m_thread);

			if (mixerdata.THR.thread_run) // уже запущено
				return hRes;              // .

			if (!mixerdata.THR.thread_snd)              // не выполнено ActivateMixerPlayData()
				return hRes = eSoundError_SequenceFail; // .

			//////////////////////////////

			if ((eSoundGroup_MIX + 1) > snd_g.size()) // нет группы
				return hRes = eSoundError_BadID_G;    // .

			auto & group = *snd_g[eSoundGroup_MIX];

			if (group.size() < 1)                   // нет элемента группы
				return hRes = eSoundError_BadID_GE; // .

			uint32 ID = group[0];

			if (ID >= snd_count)                   // данные отсутствуют
				return hRes = eSoundError_BadID_E; // .

			//////////////////////////////

			_DELETE(mixerdata.list);
			mixerdata.list = new /*_NEW_NOTHROW*/ SNDCHUNK;

			if (mixerdata.list)
			{
				mixerdata.list->loop          = true;
				mixerdata.list->stop_at_start = true;
				mixerdata.list->stop_at_end   = false;
				mixerdata.list->GID           = snd[ID]->GID;
				mixerdata.list->actual_sound  = snd[ID];
				mixerdata.list->mixer_p       = & mixerdata.THRS;
				mixerdata.list->mixer         = true;

				mixerdata.list->Init();
			}
			else // list == nullptr
				return hRes = eSoundError_BadAlloc;

			if (mixpool.GetWorkers() < MIXPOOL_THREADS)
				mixpool.WorkersSet(MIXPOOL_THREADS);

			// запуск потока воспроизведения для микшера
			mixpool(&CSound::MixerPlayThreadPRIV, this, mixerdata.list);

			mixerdata.THR.thread_run  = true;  // отметим, что поток запущен
			mixerdata.THR.thread_exit = false; // сбросим признак выхода потока

			return hRes;
		}
		error_t MixerPlayThreadPRIV(SNDCHUNK * list)
		{
			// в данном блоке - поток воспроизведения того, что микшируется

			error_t hRes = RunPlayback(list);

			{
				glock lk_thread (mixerdata.THR.m_thread);

				SNDDATA & sound = *reinterpret_cast<SNDDATA*>(list->actual_sound);

				sound._ATM_0(a_interrupt);
				list->_quick_close();

				mixerdata.THR.thread_exit = true;
				mixerdata.THR.thread_ERR  = hRes;

				if (hRes >= eSoundError__NORMAL_END || hRes <= eSoundError__NORMAL_BEGIN)
				{
					wchar_t error[MAX_PATH * 2];
					wsprintf(error, L"\nSoundMixer playthread exited with ERROR 0x%X", hRes);

					wprintf(L"%s", error);
					//_MBM(error);
				}				
			}

			return hRes;
		}
		error_t DeActivateMixerPlayThreadPRIV()
		{
			// в данном блоке удаляем SNDCHUNK (list) потока и завершаем сам поток

			error_t hRes = eSoundError_TRUE;

			ulock lk_thread (mixerdata.THR.m_thread);

			if (!mixerdata.THR.thread_run) // уже закрыто
				return hRes;               // .

			//////////////////////////////

			if (mixerdata.THR.thread_exit) // поток завершился самостоятельно (с ошибкой)
			{
				// ...
			}
			else // поток работает, требуется команда на завершение
			{
				SNDWAIT   waiter;
				SNDWAIT * waiter_ptr = & waiter;

				waiter.target = 1; // поток микшера всегда один

				SNDDATA & sound  = *reinterpret_cast<SNDDATA*>(mixerdata.list->actual_sound);
				auto    & caller = *mixerdata.list->caller;

				sound._ATM_1(a_interrupt);

				caller.wait.Store(waiter_ptr, eSoundCallReason_Interrupt);
				caller.Awake(eSoundCallReason_Interrupt);

				{
					glock stop_while_working_with_pause (sound.m_pause);
					sound.pause = false;
					sound._CV_ALL(cv_pause);
				}

				lk_thread.unlock();
				ControlWait(waiter_ptr, waiter.target);
				lk_thread.lock();
			}

			//////////////////////////////

			_DELETE(mixerdata.list);

			mixerdata.THR.thread_run  = false; // отметим, что закрыли
			mixerdata.THR.thread_exit = false; // 

			return hRes;
		}
		error_t DeActivateMixerPlayDataPRIV()
		{
			// в данном блоке удаляем SNDDATA и отмечаем в группе eSoundGroup_MIX, что инф. MISSING

			error_t hRes = eSoundError_TRUE;

			glock lk_thread (mixerdata.THR.m_thread);

			if (!mixerdata.THR.thread_snd) // уже закрыто
				return hRes;               // .

			if (mixerdata.THR.thread_run)               // не выполнено DeActivateMixerPlayThread()
				return hRes = eSoundError_SequenceFail; // .

			//////////////////////////////

			if ((eSoundGroup_MIX + 1) > snd_g.size()) // нет группы
				return hRes = eSoundError_BadID_G;    // .

			// группа микшера
			auto & group = *snd_g[eSoundGroup_MIX];

			if (group.size() < 1)                   // нет элемента группы (ID)
				return hRes = eSoundError_BadID_GE; // .

			// ID данных микшера
			uint32 ID = group[0];

			if (ID >= snd_count)                   // данные отсутствуют
				return hRes = eSoundError_BadID_E; // .

			// данные микшера
			SNDDATA & sound = *snd[ID];

			//////////////////////////////

			uint64      ID_last = snd_count - 1;
			uint64_gid GID_last = snd[ID_last]->GID;

			sound.Close();      // сброс данных микшера
			group[0] = MISSING; // в группе оставляем метку, что нет данных
			snd_count--;        // уменьшаем общее число

			if (ID != ID_last) // нужна перестановка
			{
				snd.Swap(ID, ID_last);
				_SoundID(GID_last) = (uint32) ID;
			}

			mixerdata.THR.thread_snd = false; // отметим, что закрыли

			return hRes;
		}
	protected:
		//>> Запуск или останов всей системы микшера
		error_t Mixer(SNDDESC * snddesc, bool activate)
		{
			error_t hRes = eSoundError_TRUE;

			if (activate)
			{
				if (snddesc == nullptr)
					return hRes = eSoundError_BadSndDescPTR;

				if (eSoundError_TRUE == (hRes = ActivateMixerPlayDataPRIV(snddesc)))
				if (eSoundError_TRUE == (hRes = ActivateMixerPlayThreadPRIV()))
				if (eSoundError_TRUE == (hRes = ControlMixerPRIV(eSoundCallReason_MixerRun)))
				{ 
					//printf("\nMixer activated normally");
				}

				if (hRes != eSoundError_TRUE)
					printf("\nMixer activate result : %i", hRes);
			}
			else // de-activate
			{
				if (eSoundError_TRUE == (hRes = ControlMixerPRIV(eSoundCallReason_MixerAbort)))
				if (eSoundError_TRUE == (hRes = DeActivateMixerPlayThreadPRIV()))
				if (eSoundError_TRUE == (hRes = DeActivateMixerPlayDataPRIV()))
				{
					//printf("\nMixer de-activated normally"); //getchar();
				}

				if (hRes != eSoundError_TRUE)
					printf("\nMixer de-activate result : %i", hRes);
			}

			return hRes;
		}

	private:
		void MixerLoopThread_ClearDone()
		{
			// USE IT WITH  glock data_lock (mixerdata.m_mixer);

			// 1111111111122222222333333  task.size()       == MAX            [accepted + throwed + dummy]
			// 1111111111122222222------  task_num          <= task.size()    [accepted + throwed        ]
			// 11111111111--------------  task_num_accepted <= task_num       [accepted                  ]

			bool b_step_2 = (mixerdata.task_num != mixerdata.task_num_accepted); // throwed > accepted

			for (uint32 i = 0; i < mixerdata.task_num_accepted; i++)
			{
				auto & task = *mixerdata.task[i];
				if (task.b_done)
				{
					// сброс основных состояний
					task.b_throwed  = false;
					task.b_accepted = false;
					task.b_done     = false;

					// сброс блокировки (если это мы блокировали)
					if (!task.SET.async)
						task.sound->_ATM_ST(a_playblock, false);

					// переброс в конец списка в 1 или 2 шага
					{
						//      A    B       C                 A    B       C                 A    B       C
						// 11111_1111122222222333333  ->  1111111111_22222222333333  ->  111111111122222222_333333  [accepted + throwed + dummy]
						// 11111_1111122222222------  ->  1111111111_22222222------  ->  111111111122222222-------  [accepted + throwed        ]
						// 11111_11111--------------  ->  1111111111---------------  ->  1111111111---------------  [accepted                  ]

						uint32 last_accepted = mixerdata.task_num_accepted - 1;

						// STEP 1
						if (i != last_accepted) // push to "throwed" (or "dummy") segment
						{
							mixerdata.task.Swap(i, last_accepted); // swap A to B (or to С)
						}

						// STEP 2
						if (b_step_2) // if "throwed" segment exists : push to "dummy" segment
						{
							uint32 last_throwed = mixerdata.task_num - 1;

							mixerdata.task.Swap(last_accepted, last_throwed); // swap B to C
						}
					}				

					// уменьшить счетчики
					task.sound->players_mixer--;
					mixerdata.task_num_accepted--;
					mixerdata.task_num--;
				}
			}

			//////////////// ДОБАВЛЕНО : проверка <throwed> сегмента ////////////////

			if (mixerdata.task_num != mixerdata.task_num_accepted)
			{
				for (uint32 i = mixerdata.task_num_accepted; i < mixerdata.task_num; i++)
				{
					auto & task = *mixerdata.task[i];
					if (task.b_done)
					{
						// сброс основных состояний
						task.b_throwed  = false;
						task.b_accepted = false;
						task.b_done     = false;

						// сброс блокировки (если это мы блокировали)
						if (!task.SET.async)
							task.sound->_ATM_ST(a_playblock, false);

						// переброс в конец списка
						{
							uint32 last_throwed = mixerdata.task_num - 1;

							if (i != last_throwed)
								mixerdata.task.Swap(i, last_throwed);
						}

						// уменьшить счетчики
						task.sound->players_mixer--;
						//mixerdata.task_num_accepted--;
						mixerdata.task_num--;
					}
				}
			}
		}

		//>> Постоянная задача по смешиванию и воспроизведению :: эта функция выполняется в другом потоке
		error_t MixerLoopThread()
		{
			#define DEBUG_TIME_MIN   7000 // ms
			#define DEBUG_TIME_MAX  17000 // ms

			auto & _ = mixerdata;

			// Н.У.
			_.task_num              = 0;
			_.task_num_accepted     = 0;
			_.ConverterMP3_tasks    = 0;
			_.ConverterOGG_tasks    = 0;
			_.ConverterAAC_tasks    = 0;
			_.THR_status            = eSoundCallReason_Pause;
			_.EX.sleep_timepoint    = _TIME;
			_.call_r                = eSoundCallReason_NONE;
			_.call_r_local          = eSoundCallReason_NONE;
			_.exit                  = false;
			_.ERR_once              = false;
			_.ERR                   = eSoundError_Unknown;
			// ERR = eSoundError_Unknown - нет данных / нет ошибок (микшер работает нормально)
			// ERR = eSoundError_TRUE - микшер не работает / оканчивает работу
			// ERR = eSoundError_MixerLostThr - потеря потока воспроизведения (микшер работает вхолостую)

			_.MixerPCM.settings.allocate       = true;
			_.MixerPCM.settings.target.usetime = true;

			// 1. DONE блок на работу с данными тут и там
			// 2. DONE проверка возврата ERR от потока воспроизведения
			// 3. получить каретку, где остановилось считывание - чтобы узнать, сколько ещё смикшировать
			// 4. STOP если ничего не воспроиздводит
			// 5. сброс задачи (interrupt) ?
			// 6. пауза задачи ? продолжение ?   
			// 7. перемотка задачи ?
			// 8. пауза потока воспроизведения
			// 9. продолжение потока воспроизведения - каретку в ноль !
			// Если поток воспроизведения упал :
			// - не пытаться контролировать его
			// - прочую работу делать как будто всё в порядке
			// - нас, вероятно, вызовут на выход - при нормальной работе
			//   упасть может только если происходит смена профиля девайса
			// - если на выход так и не позвали будет DEBUG ERROR сообщение
			// не забыть проверить корректность работы счетчика : task.sound->players_mixer
						
			// уведомление о входе
			{
				glock stop_run_waiter (_.m_wait);
				_.wait_run = true;
				_CV_ALL(_.cv_wait);
			}

			auto ClearDone  = [this] () -> void
			{
				MixerLoopThread_ClearDone();
			};
			auto ClearAll   = [&_] () -> void
			{
				// очистка задач при закрытии микшера

				glock data_lock (_.m_mixer);

				uint32 i_max = (uint32) _.task.size();

				for (uint32 i = 0; i < i_max; i++)
				{
					auto & task = *_.task[i];

					if (task.b_throwed)
						task.sound->players_mixer = 0;

					// сброс основных состояний
					task.b_throwed  = false;
					task.b_accepted = false;
					task.b_done     = false;

					// сброс блокировки (если это мы блокировали)
					if (!task.SET.async)
						task.sound->_ATM_ST(a_playblock, false);
				}

				_.task_num_accepted = 0;
				_.task_num          = 0;
			};

			//printf("\nMixer start...");

			while (!_.exit)
			{
				auto MakeSleep     = [&_] () -> void
				{
					// ОЖИДАНИЕ ПО ВРЕМЕНИ //

					bool break_sleep = false;

					// сброс своей копии
					_.call_r_local = eSoundCallReason_NONE;

					while (!break_sleep)
					{
						ulock data_lock (_.m_mixer);

						if (_.call_r != eSoundCallReason_NONE)
						{
							_.call_r_local = _.call_r;
							_.call_r       = eSoundCallReason_NONE;
							break_sleep    = true;
						}
						else
						{
							int32 sleep_time = MIXER_PERIODICITY - (int32)_TIMER(_.EX.sleep_timepoint);
							if (sleep_time > MIXER_PERIODICITY)
								sleep_time = MIXER_PERIODICITY;
							//printf("\nMIXER SLEEP TIME : %3i ms", sleep_time);
							if (sleep_time > 0)
							{
								for (;;) // sleep loop
								{
									cvstate timeout;
									timems sleep_time_ms (sleep_time);

									timeout = _._CV_FOR(cv_mixer, data_lock, sleep_time_ms);

									if (timeout == _CV_NO_TIMEOUT)
									{
										if (_.call_r != eSoundCallReason_NONE)
										{
											_.call_r_local = _.call_r;
											_.call_r       = eSoundCallReason_NONE;
											break_sleep    = true;
											break;
										}
										else // fake awakening, try continue sleep
										{
											sleep_time = MIXER_PERIODICITY - (int32)_TIMER(_.EX.sleep_timepoint);
											if (sleep_time > MIXER_PERIODICITY)
												sleep_time = MIXER_PERIODICITY;
											if (sleep_time <= 0)
											{
												break_sleep = true;
												break;
											}
										}
									}
									else // _CV_TIMEOUT
									{
										break_sleep = true;
										break;
									}
								}
								// exit sleep loop
							}
							else // no time to sleep
								break_sleep = true;
						}
						// break_sleep exit, data_lock unlock
					}

					// обновление
					_.EX.sleep_timepoint = _TIME;
				};
				auto CheckAbort    = [&_] () -> void
				{
					// ПРОВЕРКА ВЫЗОВА на ВЫХОД //

					if (_.call_r_local & eSoundCallReason_MixerAbort)
					{
						_.exit = true;
						_.ERR = eSoundError_TRUE;
					}			
				};
				auto CheckPause    = [&_] () -> void
				{
					// ПРОВЕРКА ЗАКАЗА К ПАУЗЕ //     ! FOR DEBUG ONLY !

					if (_.call_r_local & eSoundCallReason_MixerPause)
					{
						{
							ulock pause_lock (_.m_pause);

							_.pause_end = false;

							// уведомление о входе в паузу
							{
								glock stop_pause_waiter (_.m_wait);
								_.wait_pause = true;
								_CV_ALL(_.cv_wait);
							}

							while (!_.pause_end)
							{
								_CV_WAIT(_.cv_pause, pause_lock);
							}
						}
						
						// уведомление о выходе из паузы
						{
							glock stop_pause_waiter (_.m_wait);
							_.wait_resume = true;
							_CV_ALL(_.cv_wait);
						}
					}

					// ЗАМЕТКА : Поток воспроизведения продолжает работать
				};
				auto CheckTasks    = [&_] () -> void
				{
					// ПРОВЕРКА ПОСТУПЛЕНИЯ НОВЫХ ЗАДАЧ //

					if (_.call_r_local & eSoundCallReason_MixerNewTask)
					{
						//         accepted   throwed    dummy
						// (start) --->>---  --->>---  --->>--- (end of task's list)

						// пройдёмся по задачам с конца
						for (int64 i = _.task_num - 1; i >= 0; i--)
						{
							auto & task = *_.task[i];

							if (!task.b_throwed) // незадействованный DUMMY слот в конце массива - пропускаем
								continue;
							else
							{
								if (task.b_accepted) // эта и последующие уже приняты - выходим
									break;
								else // принимаем новую задачу (переводим 'throwed' в 'accepted')
								{
									task.b_accepted = true;
									_.task_num_accepted++;
								}
							}
						}
					}				
				};
				auto MixSounds     = [&_, ClearDone] () -> void
				{
					// ГЛАВНАЯ ПРОЦЕДУРА МИКШИРОВАНИЯ . КОНТРОЛЬ ЗАДАЧ //

					// Содержим в буфере MIXER_BUFTIME минимум MIXER_PLAYBUFTIME данных
					// Если текущих данных меньше (проверяем текущую точку чтения от потока воспроизведения) - микшируем ещё
					// При поступлении новых задач или продолжении из паузы старых - форсированно добавляем/микшируем новое (спец.признак для этого?)
					//
					// если все задачи поставили в паузу, потом продолжили
					// поток перемотается на начало и часть, что уже залили в него ранее - пропадёт
					// Как поправить ? контроль, чего не успели воспр.? или не ставить в паузу поток (воспр. пустышки)?
					// РЕШЕНИЕ : 
					// поток воспрв. ставить в паузу только если совсем нет задач
					// если задачи все в паузе, поток продолжит работать, но воспр. пустышки (поток теперь стирает ранее прочитанное сам)
					//
					// ОБЩАЯ СХЕМА (на примере .mp3)
					// 1. Заказать у mp3-декодера (delta_task) в секундах. Результат он сохранит в собственном спец. буфере.
					//    Скорее всего результат выйдет больше заказанного из-за frame-структуры *.mp3
					// 2. Часть результата из спец. буфера хранения указать как входные данные к микшированию <task.mixdata>
					// 3. Результат микшера копировать в целевой (общий с потоком воспр.) буфер (BFD)

					if (_.THR_status == eSoundCallReason_MixerERR) // сбой в работе
						return;                                    // .

					if (_.task_num_accepted)
					{
						SNDDATA & sound  = *reinterpret_cast<SNDDATA*>(_.list->actual_sound);

						{
							glock lock_read_write (_.THRS.m_ReadWrite); // останов работы с буфером в потоке воспроизведения

							if (_.THR_status == eSoundCallReason_Pause) // если поток в паузе т.к. не было задач,          
							{                                           // мы позже отмотаем его на начало,
								_.THRS.rw_pos   = 0;                    // а пока самостоятельно проставим
								_.THRS.rw_count = 0;                    // текущую позицию как начальную

								// очистить целевой буфер
								memset(sound.file.buf,  _.EX.null_sample, sound.file.count);
							//	memset(_.BFD.pos_start, _.EX.null_sample, _.BFD.pos_end - _.BFD.pos_start);
							}
							
							// обновление позиции потока воспроизведения
							_.BFD.pos_player = _.BFD.pos_start + _.THRS.rw_pos * _.EX.frame_size;
							//printf("\n%08x - %08x - %08x", _.BFD.pos_start, _.BFD.pos_player, _.BFD.pos_end);
							//printf("\n%6i, %i", _.THRS.rw_pos, _.THRS.rw_count);

							uint32 task_done     = MISSING;                            // сколько задач помечены как завершённые
							uint32 task_paused   = 0;                                  // сколько задач находятся в паузе
							uint32 task_possible = 0;                                  // сколько возможно (на случай malloc error с mixdata)
							uint32 task_i        = 0;                                  // счетчик задач к выполнению
							uint32 task_num      = _.task_num_accepted;                // задач всего к работе (== task_num_accepted)
						//	 int32 delta_pos     = _.BFD.pos_mixer - _.BFD.pos_player; // разница между позициями микшера и потока воспр.
							uint32 delta_task    = 0;                                  // нужно сделать (в байтах)
							float  delta_task_t  = 0;                                  // нужно сделать (в секундах)
							float  delta_task_t_ = 0;                                  // нужно сделать (в секундах) [увеличенный заказ для декодеров]

							auto CalcTaskPortion = [&_, &delta_task]                () -> void
							{
								// РАСЧЁТ РАЗМЕРА БУФЕРА К МИКШИРОВАНИЮ //

								uint32 THRS_rw_readed = _.THRS.rw_count * _.EX.frame_size;         // количество прочитанного потоком (в байтах)
	
								uint32 buf_sz_not_readed = (THRS_rw_readed >= _.EX.done_buf_sz)    // осталось непрочитанного потоком (в байтах)
									?  0 : (_.EX.done_buf_sz - THRS_rw_readed);                    // .

							//	if (THRS_rw_readed > _.EX.done_buf_sz)
							//	printf("\n%6i %6i : %6i", _.EX.done_buf_sz, THRS_rw_readed, buf_sz_not_readed);
							//	if (THRS_rw_readed > _.EX.done_buf_sz)
							//		printf(" !!!");

							//	if (_.call_r_local & eSoundCallReason_MixerUpdate) // полное обновление
							//	{
							//		// сюда попадаем если были вызовы по функциям ControlMixerTask[Interrupt/Pause/Rewind]
							//
								//	uint32 size_to_full_update = (buf_sz_not_readed > _.EX.min_buf_sz) // размер для полного обновления (в байтах)
								//		?  buf_sz_not_readed : _.EX.min_buf_sz;                        // .
								//
								//	delta_task       = size_to_full_update;  // >= _.EX.min_buf_sz
								//	_.BFD.pos_mixer  = _.BFD.pos_player;     // пройти на позицию потока
								//	_.EX.done_buf_sz = 0;                    // сброс (старые данные перезаписываются, если были)
								//	_.THRS.rw_count  = 0;
							//	
									// ЗАПРЕТ ПОЛНОГО ОБНОВЛЕНИЯ
									// Позиции по задачам ставятся исходя из размеров уже прочитанного, сконвертированного и
									// смикшированного из источника в целевой буфер хранения микшированных данных.
									// А значит, если просто сбросить ранее смикшированное, эта часть воспроизведения будет
									// утрачена, т.к. текущие позиции "убежали" вперёд.
									// Чтобы сделать полное обновления, для возврата придётся для каждой задачи содержать ряд 
									// контрольных точек по выполненным данным, но ещё не прочитанным потоком воспроизведения,
									// контролировать в этих контрольных точках какие задачи были в тот момент уже готовы к микшированию,
									// а какие ещё нет и прочие статусы... 
									// Всё это слишком усложнит и запутает код, а выгода будет небольшая - пораньше внести изменения.
									// Поэтому, вместо полного обновления, установим высокую частоту работы микшера, чтобы делал часто и понемногу,
									// тогда и изменения будут проявлятся как можно быстрее.
							//
							//	}
							//	else // до-обновление "сверху" по необходимости
							//	{
									if (THRS_rw_readed >= _.EX.done_buf_sz)  // если поток "убежал" вперёд
									{
										delta_task       = _.EX.min_buf_sz;
										_.BFD.pos_mixer  = _.BFD.pos_player;    // пройти на позицию потока
										_.EX.done_buf_sz = 0;                   // сброс (нет старых данных)
										_.THRS.rw_count  = 0;
									}
									else // иначе - поток не догнал нас ещё
									{
										if (buf_sz_not_readed < _.EX.min_buf_sz)
										{
											//delta_task = _.EX.min_buf_sz - buf_sz_not_readed;
											delta_task = _.EX.min_buf_sz;

											// старое остаётся, новое добавится "сверху" позже
											_.EX.done_buf_sz = buf_sz_not_readed;
											_.THRS.rw_count  = 0;

										} // else delta_task = 0;
									}
							//	}
							};
							auto CalcTaskDone    = [&_, &task_done, &task_paused]   (uint32 i) -> bool
							{
								// ОЦЕНКА ЗАДАЧ К УДАЛЕНИЮ , ПЕРЕМОТКА , ПАУЗА , ГРОМКОСТЬ //

								auto & task = *_.task[i];
								auto & sound = *task.sound;

								bool skip = false; // пропуск секции декодинга

								if (task.b_done) // задача уже отмечена завершённой
								{
									task_done++;
									skip = true;
									wprintf(L"\nDEBUG WARNING : unexpected task.b_done (mixer thread)");
								}
								else // задача ещё выполняется
								{
									if (task.RULE.interrupt) // запрос прервать
									{
										task.ERR = eSoundError_Normal_2;
										task.b_done = true;
										task_done++;
										skip = true;
									}
									else // нет запроса на прерывание
									{
										// запрос изменения позиции
										if (task.RULE.rewind) 
										{
											task.RULE.rewind = false;

											task.POS.time  = task.RULE.rewind_time;
											task.POS.frame = 0;                                  // сброс
											task.POS.ptr   = sound.file.buf + sound.file_offset; // сброс

											if (task.POS.time == task.sound->duration)
												task.POS.ptr = sound.file.buf + sound.file_offset + sound.file_size;

											task.STATSELF.time_p = task.POS.time * task.POS._t2p;
											task.STATSELF.time_t = task.POS.time;

											task.RULE.pause = task.RULE.rewind_pause;

											// сброс данных хранилища (для декодеров MP3, OGG, AAC)
											task.CONV.storage_cur_ptr  = task.CONV.storage.buf;
											task.CONV.storage_cur_size = 0;

											// перезапуск последовательности
											task.CONV.first_run = true;
										}

										// не зациклено
										if (!task.SET.loop)
										{
											// находимся в конце источника
											if (task.POS.ptr >= (sound.file.buf + sound.file_offset + sound.file_size))
											{
												if (task.SET.stop_at_end) // в начало и на паузу
												{
													task.RULE.pause          = true;
													task.STATSELF.auto_pause = true;

													task.POS.frame = 0;                       // сброс
													task.POS.time  = 0.f;                     // .
													task.POS.ptr   = task.sound->file.buf +   // .
														             task.sound->file_offset; // .

													task.STATSELF.time_p = task.POS.time * task.POS._t2p;
													task.STATSELF.time_t = task.POS.time;

													// сброс данных хранилища (для декодеров MP3, OGG, AAC)
													task.CONV.storage_cur_ptr  = task.CONV.storage.buf;
													task.CONV.storage_cur_size = 0;

													// перезапуск последовательности
													task.CONV.first_run = true;
												}
												else // прервать
												{
													task.ERR = eSoundError_Normal_1;
													task.RULE.pause     = false;
													task.RULE.interrupt = true;
													task.b_done = true;
													task_done++;
												}
												skip = true;
											}
										}

										// запрос изменения громкости
										if (task.RULE.volume && !task.b_done)
										{
											task.RULE.volume = false;

											float multiplier = task.VOL.multiplier * _.master_volume ;
											      multiplier = _CLAMP(multiplier, 0.f, 1.f);

											for (uint32 i = 0; i < 8; i++)
											{
												task.VOL.result[i]  = multiplier * task.VOL.value[i];
												task.VOL.result[i]  = _CLAMP(task.VOL.result[i], 0.f, 1.f);
											}
										}
									}
								}

								// ОЦЕНКА ЗАДАЧ В ПАУЗЕ //

								if (task.RULE.pause)
								{
									if (!task.STATSELF.auto_pause)
										 task.STATSELF.user_pause = true;

									task_paused++;
									skip = true;
								}
								else
								{
									task.STATSELF.auto_pause = false;
									task.STATSELF.user_pause = false;
								}

								return skip;
							};
							auto CalcTaskMP3     = [&_, &task_done, &delta_task_t, &delta_task_t_] (uint32 i) -> void
							{
								// ДЕКОДИРОВАНИЕ MP3 ИСТОЧНИКА //

								auto & task  = *_.task[i];
								auto & sound = *task.sound;

								error_t hRes;
								bool convert_more   =  task.CONV.first_run; // всегда <true> при первом проходе

								auto & ConverterMP3 = *task.CONV.ConverterMP3;

								//////////// Запуск декодера ////////////

								if (task.CONV.first_run)
								{
									ConverterMP3.settings.in.data        = sound.file.buf + sound.file_offset;
									ConverterMP3.settings.in.size        = sound.file_size;
									ConverterMP3.settings.in.loop        = task.SET.loop;
									ConverterMP3.settings.in.start_time  = task.POS.time; // 0.f или время перемотки
									ConverterMP3.settings.in.start_frame = 0;
									ConverterMP3.settings.in.use_frame   = false;
									ConverterMP3.settings.in.time        = delta_task_t_ ; // исп. увеличенный заказ (для гарантии получения выхода не меншье заказанного)
									ConverterMP3.settings.in.maximize    = true;

									hRes = ConverterMP3.Run();

									ConverterMP3.settings.in.use_frame   = true;
								}
								else // first_run == false
								{
									// Оценка необходимости конвертации (исходя из "delta_task_t", а не "delta_task_t_")
									{
										uint32 delta_task_t_as_storage_size = task.CONV.snddesc.nBlockAlign +  // 1 block + ceil(...) blocks
											(uint32) ceil(delta_task_t * (task.CONV.snddesc.nSamplesPerSec * task.CONV.snddesc.nBlockAlign));

										convert_more = (task.CONV.storage_cur_size < delta_task_t_as_storage_size);
									}

									if (convert_more)
									{
									//	ConverterMP3.settings.in.start_frame = ConverterMP3.settings.out.end_frame; // task.POS.frame
										ConverterMP3.settings.in.time        = delta_task_t_ ;

										hRes = ConverterMP3.Continue();
									}
									else
									{
										hRes = eMP3ConvStatus_Normal;
									}								
								}
									
								//////////// Контроль результата декодера ////////////

								if (hRes != eMP3ConvStatus_Normal)
								{
									if (hRes == eMP3ConvStatus_BufferEnd)
									{
										//printf("\nMixer Task %i : eMP3ConvStatus_BufferEnd", i);
										task.POS.frame = ConverterMP3.settings.out.total_frames;
										task.POS.ptr   = sound.file.buf + sound.file_offset + sound.file_size;
										task.POS.time  = sound.duration;
										task.STATSELF.time_p = 1.f;
										task.STATSELF.time_t = sound.duration;
									}
									else // bad mp3 source
									{
										//printf("\nMixer Task %i : ConverterMP3->Run() error %i", i, hRes);
										task.ERR  = eSoundError_convMP3;
										task.ERR |= hRes << eSoundError__MASK_CONVERTER_SHIFT;
										task.b_done = true;
										task_done++;
									}
								}
								else // hRes == eMP3ConvStatus_Normal
								{
									task.POS.frame = ConverterMP3.settings.out.end_frame;
									task.POS.ptr   = ConverterMP3.settings.out._data;
									task.POS.time  = task.POS.frame * ConverterMP3.settings.out.fsz_time;
									task.STATSELF.time_p = task.POS.time * task.POS._t2p;
									task.STATSELF.time_t = task.POS.time;

									//printf("\nMixer Task %i : PTR %X frame %i time %f", i, task.pos_ptr, task.pos_frame, task.pos_time);
								}

								//////////// Перенос результата в хранилище ////////////

								if (!task.b_done) 
								{
									bool error = false;

									//////////// Создание хранилища ////////////

									if (task.CONV.storage_update)
									{
										task.CONV.storage_update = false;

										auto & snddesc = task.CONV.snddesc;

										snddesc.wFormatTag     = WAVE_FORMAT_PCM;
										snddesc.nSamplesPerSec = ConverterMP3.settings.out.nSamplesPerSec;
										snddesc.nChannels      = ConverterMP3.settings.out.nChannels;
										snddesc.wBitsPerSample = ConverterMP3.settings.out.wBitsPerSample;
										snddesc.nBlockAlign    = snddesc.nChannels * (snddesc.wBitsPerSample >> 3);
										snddesc.cbSize         = 0;

										uint32 size_of_storage = (uint32) (((float)MIXER_BUFTIME / 1000) * snddesc.nSamplesPerSec * snddesc.nBlockAlign);

										if (task.CONV.storage.buf   == nullptr ||      // хранилища не было в принципе (впервые задача) или
											task.CONV.storage.count < size_of_storage) // другая задача с mp3-источником с большим выходом
										{
											if (!task.CONV.storage.Create(size_of_storage))
											{
												error       = true;
												task.ERR    = eSoundError_BadAlloc;
												task.b_done = true;
												task_done++;
											}
										}

										if (!error)
										{
											task.CONV.storage_cur_ptr  = task.CONV.storage.buf;
											task.CONV.storage_cur_size = 0;
										}
									}

									if (!error) // хранилище валидно
									{
										//////////// Перенос результата ////////////

										if (convert_more) // было конвертирование
										{
											byte * storage_start = task.CONV.storage.buf;
											byte * storage_end   = task.CONV.storage.buf + task.CONV.storage.count;
											byte * source_ptr    = ConverterMP3.settings.out.data_out;
											uint32 source_size   = ConverterMP3.settings.out.size_out;

											byte * storage_cur_ptr_end = task.CONV.storage_cur_ptr + task.CONV.storage_cur_size;

											// + ------ cur ------ cur_end ---- +  if (...)
											// + ------ cur_end ------ cur ---- +  else

											if (storage_cur_ptr_end < storage_end)
											{
												uint32 available_until_end = (uint32) (storage_end - storage_cur_ptr_end);

												if (available_until_end > source_size)
												{
													memcpy(storage_cur_ptr_end, source_ptr, source_size);	

													task.CONV.storage_cur_size += source_size;
												}
												else // source_size >= available_size 
												{
													uint32 p1sz = available_until_end;
													uint32 p2sz = source_size - available_until_end;

													memcpy(storage_cur_ptr_end, source_ptr,        p1sz);
													memcpy(storage_start,       source_ptr + p1sz, p2sz);

													task.CONV.storage_cur_size += source_size;
												}		
											}
											else // storage_cur_ptr_end >= storage_end
											{
												storage_cur_ptr_end = storage_start + (storage_cur_ptr_end - storage_end);

												memcpy(storage_cur_ptr_end, source_ptr, source_size);

												task.CONV.storage_cur_size += source_size;
											}

											// DEBUG check
											if (task.CONV.storage_cur_size >= task.CONV.storage.count)
												wprintf(L"\nMixer WARNING : storage_cur_size > storage_count !");
										}
									
										//////////// Подготовка задания микшеру ////////////

										if (task.CONV.first_run)
										{
											task.mixdata.origin        = task.CONV.snddesc_ptr;
											task.mixdata.settings_loop = true; // хранилище используется по кругу

											task.mixdata.settings_origin.buffer      = task.CONV.storage.buf;
											task.mixdata.settings_origin.size        = task.CONV.storage.count;
											task.mixdata.settings_origin.usetime     = false;
											task.mixdata.settings_origin.auto_signed = true;
											task.mixdata.settings_origin.user_signed = true;
											task.mixdata.settings_origin.BE          = false;
											task.mixdata.settings_origin.range       = eSoundDevRange::eSDRangeDirectOut;
											task.mixdata.settings_origin.volume      = task.VOL.result;
										}

										task.mixdata.settings_origin.samples = (uint32)                                         (
											(task.CONV.storage_cur_ptr - task.CONV.storage.buf) / task.CONV.snddesc.nBlockAlign ) ;
									}
								}

								// сброс
								task.CONV.first_run = false;
							};
							auto CalcTaskOGG     = [&_, &task_done, &delta_task_t, &delta_task_t_] (uint32 i) -> void
							{
								// ДЕКОДИРОВАНИЕ OGG ИСТОЧНИКА //

								auto & task  = *_.task[i];
								auto & sound = *task.sound;

								// ЗАГЛУШКА (временно)
								task.b_done = true;
								task_done++;
							};
							auto CalcTaskAAC     = [&_, &task_done, &delta_task_t, &delta_task_t_] (uint32 i) -> void
							{
								// ДЕКОДИРОВАНИЕ AAC ИСТОЧНИКА //

								auto & task  = *_.task[i];
								auto & sound = *task.sound;

								// ЗАГЛУШКА (временно)
								task.b_done = true;
								task_done++;
							};
							auto CalcTaskWAV     = [&_, &task_done, &delta_task_t]  (uint32 i) -> void
							{
								auto & task  = *_.task[i];
								auto & sound = *task.sound;

								//////////// Подготовка задания микшеру ////////////

								if (task.CONV.first_run)
								{
									task.CONV.first_run = false;

									task.mixdata.origin        = sound.snddesc;
									task.mixdata.settings_loop = task.SET.loop;

									task.mixdata.settings_origin.buffer      = sound.file.buf + sound.file_offset;
									task.mixdata.settings_origin.size        = sound.file_size;
									task.mixdata.settings_origin.usetime     = true;
									task.mixdata.settings_origin.time        = task.POS.time;
									task.mixdata.settings_origin.auto_signed = true;
									task.mixdata.settings_origin.user_signed = true;
									task.mixdata.settings_origin.BE          = false;
									task.mixdata.settings_origin.range       = eSoundDevRange::eSDRangeDirectOut;
									task.mixdata.settings_origin.volume      = task.VOL.result;
								}
								else
								{
									task.mixdata.settings_origin.usetime     = false;
									task.mixdata.settings_origin.samples     = task.POS.frame;
								}
							};
							auto CalcMixData     = [&_, &task_done, &task_paused, &task_num, &task_i, &task_possible] () -> void
							{
								// ПОДГОТОВКА ВХОДНОГО МАССИВА МИКШЕРА //

								task_possible = (uint32) _.mixdata.size();

								// подготовить массив смешивания под размер
								{
									uint32 task_actual = task_num - (task_done + task_paused);
									if (task_actual > task_possible)
									{
										_.mixdata.resize(task_actual);
										task_possible = (uint32) _.mixdata.size();
									}
								}

								// заполнить массив смешивания
								for (uint32 i = 0; i < task_num; i++)
								{
									auto & task  = *_.task[i];
									auto & sound = *task.sound;

									if (task.b_done || task.RULE.pause)
										continue;

									if (task_i == task_possible) // произошёл 'malloc error' при mixdata.resize()
									{
										task.b_done = true; // сброс этой задачи и последующих
										task_done++;        // .
									}
									else
									{
										// запомним номер отправленной задачи task[i]
										task.mixdata.ID = i;

										_.mixdata[task_i] = task.mixdata_ptr; // передаём и идём дальше
										task_i++;                             // .
									}
								}
							};
							auto RunMixData      = [&_, &task_done, &task_i, &delta_task_t] () -> bool
							{
								// ЗАПУСК МИКШЕРА . КОНТРОЛЬ ОШИБОК МИКШИРОВАНИЯ //

								bool mixer_success = false;

								if (task_i)
								{
									uint32 try_once_more = 100;

									// сообщаем размер задачи микшеру
									_.MixerPCM.settings.target.time = delta_task_t;

									while (try_once_more)
									{
										// ЗАПУСК
										auto hRes = _.MixerPCM.AutoRun(_.mixdata, task_i);

										// КОНТРОЛЬ РЕЗУЛЬТАТА
										if (hRes == ePCMConvStatus_Normal ||
											hRes == ePCMConvStatus_EndOfOrigin)
										{
											try_once_more = 0;
											mixer_success = true;
										}
										else // error occured
										{
											if (hRes == ePCMConvStatus_BadInputs)
											{
												wprintf(L"\nMixerPCM registered <BadInputs> error");

												for (uint32 i = 0; i < task_i; i++)
												{
													auto & mixertask = *_.mixdata[i];
													auto & task      = *_.task[mixertask.ID];

													task.ERR    = hRes;
													task.b_done = true;
													task_done++;
												}

												try_once_more = 0;
											}
											else if (hRes > ePCMConvStatus_AllocErr_START &&
												     hRes < ePCMConvStatus_AllocErr_END)
											{
												wprintf(L"\nMixerPCM registered MALLOC error = %i", hRes);
												try_once_more = 0;
											}
											else // сбросим задачи с ошибками и попробуем без них
											{
												try_once_more--;

												wprintf(L"\nMixerPCM registered error = %i", hRes);

												// DEBUG check
												if (!try_once_more)
													wprintf(L"\nMixerPCM WARNING : try limit reached");

												for (uint32 i = 0; i < task_i; i++)
												{
													auto & mixertask = *_.mixdata[i];
													auto & task      = *_.task[mixertask.ID];

													if (!(mixertask.ERR == ePCMConvStatus_Normal ||
														  mixertask.ERR == ePCMConvStatus_EndOfOrigin))
													{
														task.ERR    = mixertask.ERR;
														task.b_done = true;
														task_done++;

														if (i != (task_i-1)) // уберём в конец задачу с ошибкой
															_SWAP(_.mixdata[i], _.mixdata[task_i-1]);

														task_i--; // уменьшим число рабочих
													}
												}

												if (!task_i)
													try_once_more = 0;
											}
										}
									} // end <try_once_more>

									if (mixer_success)
									{
										for (uint32 i = 0; i < task_i; i++)
										{
											auto & mixertask = *_.mixdata[i];
											auto & task      = *_.task[mixertask.ID];

											if (mixertask.ERR == ePCMConvStatus_Normal ||
												mixertask.ERR == ePCMConvStatus_EndOfOrigin)
											{
												// обновлений позиций для успешно завершённых задач

												switch (task.sound->type)
												{
												case eSoundType_WAVE: // WAV не использует хранилище, обновить позиции соотв. источнику
													{
														task.POS.frame =         mixertask.settings_outInfo.loop_end_sample;
														task.POS.time  = (float) mixertask.settings_outInfo.loop_end_time_o;
														task.POS.ptr   = task.sound->file.buf + 
																		 task.sound->file_offset +
																		 task.POS.frame * task.POS._f2b;

														task.STATSELF.time_p = task.POS.time * task.POS._t2p;
														task.STATSELF.time_t = task.POS.time;

														break;
													}
												case eSoundType_MP3: // MP3 использует посредник-хранилище, обновить позиции хранилища
													{
														uint32 size_readed = mixertask.settings_outInfo.origin_samples * task.CONV.snddesc.nBlockAlign;

														byte * storage_cur_ptr_new = task.CONV.storage.buf +
															(mixertask.settings_outInfo.loop_end_sample * task.CONV.snddesc.nBlockAlign);

														// DEBUG check
														if (size_readed > task.CONV.storage_cur_size)
															wprintf(L"\nMixer WARNING : [MP3] size_readed > storage_cur_size");

														task.CONV.storage_cur_ptr   = storage_cur_ptr_new;
														task.CONV.storage_cur_size -= size_readed;

														// DEBUG fix
														if ((int)task.CONV.storage_cur_size < 0)
															task.CONV.storage_cur_size = 0;

														break;
													}
												case eSoundType_AAC:
												case eSoundType_OGG:
													break;
												}
											}
											else // задача завершилась с ошибкой - сбросить
											{
												task.ERR    = mixertask.ERR;
												task.b_done = true;
												task_done++;
											}
										}
									}
								}

								return mixer_success;
							};
							auto MoveMixData     = [&_] () -> void
							{
								// ПЕРЕДАЧА РЕЗУЛЬТАТОВ МИКШЕРА В ЦЕЛЕВОЙ БУФЕР //

								bool mixer_forward = (_.BFD.pos_mixer >= _.BFD.pos_player);

								// DEBUG check
								{
									// + ---- player -------------- mixer --- +   if (mixer_forward)
									// + ---- mixer -------------- player --- +   else

									uint32 available_size;

									if (mixer_forward)
										 available_size = (uint32) ( (_.BFD.pos_end - _.BFD.pos_mixer)  + (_.BFD.pos_player - _.BFD.pos_start) );
									else available_size = (uint32) ( (_.BFD.pos_end - _.BFD.pos_player) + (_.BFD.pos_mixer  - _.BFD.pos_start) );

									if (available_size < _.MixerPCM.settings.target.size)
										wprintf(L"\nMIXER: Unexpected ERROR (available_size < mixer target size)");
								}

								if (mixer_forward) // + ---- player ------------------------- mixer --- +
								{
									uint32 available_until_end  = (uint32) (_.BFD.pos_end    - _.BFD.pos_mixer);
									uint32 available_from_start = (uint32) (_.BFD.pos_player - _.BFD.pos_start);
									uint32 source_size = _.MixerPCM.settings.target.size;
									byte * source_buf  = (byte*) _.MixerPCM.settings.target.buffer;

									if (available_until_end >= source_size)
									{
										memcpy(_.BFD.pos_mixer, source_buf, source_size);
										_.BFD.pos_mixer  += source_size;
										_.EX.done_buf_sz += source_size;
									}
									else
									{
										uint32 p1sz = available_until_end;
										uint32 p2sz = source_size - available_until_end;

										memcpy(_.BFD.pos_mixer, source_buf,        p1sz);
										memcpy(_.BFD.pos_start, source_buf + p1sz, p2sz);

										_.BFD.pos_mixer   = _.BFD.pos_start + p2sz;
										_.EX.done_buf_sz += source_size;
									}
								}
								else // + ---- mixer ------------------------- player --- +
								{
									uint32 source_size = _.MixerPCM.settings.target.size;
									byte * source_buf  = (byte*) _.MixerPCM.settings.target.buffer;

									memcpy(_.BFD.pos_mixer, source_buf, source_size);
									_.BFD.pos_mixer  += source_size;
									_.EX.done_buf_sz += source_size;
								}
							};

							CalcTaskPortion();

							// заметки

							// 6. volume контроль ! (где как что изменять?)

							if (delta_task)
							{
								task_done = 0; // н.у. счетчика оконченных задач

								// преобразование в количество секунд к заказу
								delta_task_t  = delta_task * _.EX.second_size_f;
								delta_task_t_ = delta_task_t * 2;

								// увеличенный заказ для декодеров не более 2х минимальных
								if (delta_task_t_ > (_.EX.min_time * 2))
									delta_task_t_ = (_.EX.min_time * 2);

								for (uint32 i = 0; i < task_num; i++)
								if (!CalcTaskDone(i))
								{
									eSoundType sound_type = _.task[i]->sound->type;

										 if (sound_type == eSoundType_MP3) CalcTaskMP3(i); // decode mp3
									else if (sound_type == eSoundType_OGG) CalcTaskOGG(i); // decode ogg
									else if (sound_type == eSoundType_AAC) CalcTaskAAC(i); // decode aac
									else                                   CalcTaskWAV(i);		
								}
					
								CalcMixData();

								if (RunMixData()) 
									MoveMixData();
							}

							if (task_done != 0) // MISSING (не микшировали), либо > 0 (была задача на микширование)
								ClearDone();
						}
					}
				};
				auto ControlPlayer = [&_, this] () -> void
				{
					if (_.THR_status == eSoundCallReason_MixerERR) // сбой в работе
						return;                                    // .

					// останов, если нет задач
					if (!_.task_num_accepted)
					{
						if (_.THR_status != eSoundCallReason_Pause)
						{
							SNDWAIT   waiter;
							SNDWAIT * waiter_ptr = & waiter;

							{
								glock lk_thread (_.THR.m_thread); // блокируем выход потока

								if ( !_.THR.thread_run || // поток не запущен (ошибка в коде?)
									  _.THR.thread_exit ) // поток вышел сам (внутренняя ошибка, возможно изменение профиля девайса)
								{
									if (!_.THR.thread_run)
										 _.THR_ERR = eSoundError_MixerNoThr;
									else _.THR_ERR = _.THR.thread_ERR;
									_.THR_timer  = _TIME;
									_.THR_status = eSoundCallReason_MixerERR; // запомнить сбой в работе
									return;                                   // выход
								}

								{
									SNDDATA & sound  = *reinterpret_cast<SNDDATA*>(_.list->actual_sound);
									auto    & caller = *_.list->caller;

									glock stop_while_working_with_pause (sound.m_pause);

									sound.pause = true; // открыли вход в паузу

									if (caller.status.in_pause) // поток не успел выйти из старой паузы
									{
										// но выходит из-за Rewind()
										if (caller.status.rewinded)
										{
											// добавляем его к ожиданию на паузу после перемотки
											caller.wait.Store(waiter_ptr, eSoundCallReason_Pause);
											caller.Awake(eSoundCallReason_Pause);
										}
										//else {} // не выходит, либо выходит из-за Resume()							

										caller.status.awakened = false; // сбросить последствия Resume(), но не Rewind()
									}
									else // поток вне паузы
									{
										caller.wait.Store(waiter_ptr, eSoundCallReason_Pause);
										caller.Awake(eSoundCallReason_Pause);
									}

								} // pause-lock unlock()

							} // lk_thread unlock()

							waiter.target = 1;
							ControlWait(waiter_ptr, waiter.target);

							// обновляем статус управления
							_.THR_status = eSoundCallReason_Pause;
						}
					}
					else // есть задачи - выбить из паузы и перемотать в начальную позицию
					{
						if (_.THR_status == eSoundCallReason_Pause)
						{
							SNDWAIT   waiter;
							SNDWAIT * waiter_ptr = & waiter;

							{
								glock lk_thread (mixerdata.THR.m_thread); // блокируем выход потока

								if ( !_.THR.thread_run || // поток не запущен (ошибка в коде?)
									  _.THR.thread_exit ) // поток вышел сам (внутренняя ошибка, возможно изменение профиля девайса)
								{
									if (!_.THR.thread_run)
										 _.THR_ERR = eSoundError_MixerNoThr;
									else _.THR_ERR = _.THR.thread_ERR;
									_.THR_timer  = _TIME;
									_.THR_status = eSoundCallReason_MixerERR; // запомнить сбой в работе
									return;                                   // выход
								}

								{
									SNDDATA & sound  = *reinterpret_cast<SNDDATA*>(_.list->actual_sound);
									auto    & caller = *_.list->caller;

									// заказанное время перемотки
									//sound._REL_ST(a_rewind_time, 0.f);

									glock stop_while_working_with_pause (sound.m_pause);

									sound.pause = false; // закрыли вход в паузу
								//	caller.status.awakened = true;
									caller.status.rewinded = true;
									caller.wait.Store(waiter_ptr, eSoundCallReason_Rewind_WAITER);
									caller.Awake(eSoundCallReason_Rewind_WAITER, 0.f);
									sound._CV_ALL(cv_pause);

								} // pause-lock unlock()

							} // lk_thread unlock()

							waiter.target = 1;
							ControlWait(waiter_ptr, waiter.target);

							// обновляем статус управления
							_.THR_status = eSoundCallReason_NONE;
						}
					}
				};
				auto ErrorCheck    = [&_] () -> void
				{
					if (_.THR_status == eSoundCallReason_MixerERR) // сбой в работе
					{
						// сбой может быть вызван перезапуском конечного класса из-за изменений в звуковой системе
						// в этом случае конечный звуковой класс должен нас перезапустить
						// если этого не произошло в течении DEBUG_TIME_MIN, запоминаем ERR для контроля через GetStateMixer()
						// и дальнейшие действия оставляем на усмотрение выше
						// ВОЗМОЖНЫЕ ДЕЙСТВИЯ ВЫШЕ
						// Попробовать ReActivateMixer() и если не удалось 
						// вар.1. завершить работу приложения  ;  вар.2. провести реконфигурацию без использования микшера

						if (!_.ERR_once)
						{
							auto debug_time = _TIMER(_.THR_timer);

							// DEBUG : catch error situation after TIME_MIN sec.
						//	if (debug_time > DEBUG_TIME_MIN && debug_time < DEBUG_TIME_MAX)
						//	{
						//		wchar_t error[MAX_PATH * 2];
						//		wsprintf(error, 
						//			L"SoundMixer can't work cause of critical error :"
						//			L"\n- playthread shuted down (ERROR 0x%X)"
						//			L"\n- mixer's reset never happened"
						//			L"\n\nTHIS IS DEBUG ERROR : CHECK THE CODE !",
						//			_.THR_ERR);
						//		_MBM(error);
						//
						//		// will repeat error message
						//		//_.THR_timer = _TIME;
						//	}

							if (debug_time > DEBUG_TIME_MIN)
							{
								wprintf(L"\nERROR : SoundMixer lost MixerPlayThread! (%i ms past)", debug_time);
								_.ERR = eSoundError_MixerLostThr;
								_.ERR_once = true;
							}
						}
					}
					else // _.THR_status != eSoundCallReason_MixerERR   // проверка на сбой в работе
					{
						glock lk_thread (_.THR.m_thread);

						if ( !_.THR.thread_run || // поток не запущен (ошибка в коде?)
							  _.THR.thread_exit ) // поток вышел сам (внутренняя ошибка, возможно изменение профиля девайса)
						{
							if (!_.THR.thread_run)
								 _.THR_ERR = eSoundError_MixerNoThr;
							else _.THR_ERR = _.THR.thread_ERR;
							_.THR_timer  = _TIME;
							_.THR_status = eSoundCallReason_MixerERR; // запомнить сбой в работе
						}
					}
				};

				MakeSleep();
				CheckAbort();

				if (_.exit) break;
				else
				{
					CheckPause(); 

					//auto time = _TIME;
					{
						glock data_lock (_.m_mixer);

						CheckTasks();
						MixSounds();
					}
					//printf("%3i ", _TIMER(time)); // оценка времени микширования

					ControlPlayer();
					ErrorCheck();
				}
			}

			//printf("\nMixer exit...");

			// Нет смысла ставить паузу, если мы выходим, его завершат следом
			// StopPlayer();

			// Микшер выходит в случаях :
			// 1. Полное закрытие CSound
			// 2. Перезагрузка профилей DefaulDeviceChanged() - берётся блок запуска < m_run_play >
			// Во всех случаях запуск новых задач остановлен, можно безопасно всё удалять
			ClearAll();

			error_t mixer_exit_code = _.ERR;

			// уведомление о выходе
			{
				glock stop_exit_waiter (_.m_wait);
				_.wait_exit = true;
				_CV_ALL(_.cv_wait);
			}

			return mixer_exit_code;

			#undef DEBUG_TIME_MIN
			#undef DEBUG_TIME_MAX
		}

		//>> Запускает новый звук в поток смешивания
		error_t RunInMixer(uint64_gid GID, uint32 eSoundRunSet)
		{
			error_t hRes = eSoundError_TRUE;

			SNDDATA & sound = *snd[_SoundID(GID)];

			// проверка возможности запуска при откл. асинхр.
			if (!_BOOL(eSoundRunSet & eSoundRunAsync))
			{
				ulock guard (sound.m_playblock, std::try_to_lock);
				if (guard.owns_lock())
				{
					if (!sound._ATM_LD(a_playblock))
					{    // захват блокировки
						 sound._ATM_ST(a_playblock, true);
					}
					else // заблокировано RunInMixer()
						hRes = eSoundError_Normal_None;
				}
				else // заблокировано Play()
					hRes = eSoundError_Normal_None;
			}

			// успешный запуск
			if (hRes == eSoundError_TRUE)
			{
				glock stop_mixer (mixerdata.m_mixer); // блокируем микшер

				if (mixerdata.task.size() == mixerdata.task_num) // проверка переполнения
					mixerdata.task.emplace_back_create();        // .
				
				uint32 cur_task_id = mixerdata.task_num++;

				if (mixerdata.task[cur_task_id] == nullptr) // проверка фактического наличия
					mixerdata.task.ReCreate(cur_task_id);   //.

				auto & cur_task = *mixerdata.task[cur_task_id];

				cur_task.GID        = GID;
				cur_task.sound      = snd[_SoundID(GID)];
				cur_task.POS.frame  = 0;
				cur_task.POS.time   = 0.f;
				cur_task.POS.ptr    = cur_task.sound->file.buf + 
					                  cur_task.sound->file_offset;
				cur_task.POS._f2b   = 0;
				cur_task.POS._t2p   = 1.f / cur_task.sound->duration;
				cur_task.ERR        = eSoundError_Unknown;
				cur_task.b_throwed  = true;  // задача передана
				cur_task.b_accepted = false; // микшер ещё не ознакомился с ней
				cur_task.b_done     = false;

				cur_task.sound->players_mixer++; // отметка о количестве запущенного в микшер

				// заполнение данных для внешних опросов
				cur_task.STATSELF.GID        = GID;
				cur_task.STATSELF.time       = cur_task.sound->duration;
				cur_task.STATSELF.auto_pause = false;
				cur_task.STATSELF.user_pause = false;
				cur_task.STATSELF.time_p     = 0.f;
				cur_task.STATSELF.time_t     = 0.f;
				cur_task.STATSELF.in_mixer   = true;

				cur_task.SET.Reset(eSoundRunSet);
				cur_task.RULE.Reset();
				cur_task.CONV.Reset();
				cur_task.VOL.Reset(mixerdata.master_volume);

				// обработка заказанной паузы
				if (cur_task.SET.stop_at_start)
				{
					cur_task.RULE.pause = true;
					cur_task.STATSELF.auto_pause = true;
				}				

				switch (sound.type) // сделать malloc error check !
				{
				case eSoundType_MP3:
					{
						uint32 cur_id;

						// определить MP3 конвертер к работе

						if (mixerdata.ConverterMP3.size() == mixerdata.ConverterMP3_tasks) // проверка переполнения
							mixerdata.ConverterMP3.emplace_back_create();                  // .

						cur_id = mixerdata.ConverterMP3_tasks++;

						cur_task.CONV.ConverterMP3 = mixerdata.ConverterMP3[cur_id]; // запомнить конвертер

						break;
					}
				case eSoundType_OGG:
				case eSoundType_AAC:
						break;
				case eSoundType_WAVE:
					{
						cur_task.POS._f2b = sound.snddesc->nChannels * (sound.snddesc->wBitsPerSample >> 3); //  wBitsPerSample / 8
						break;
					}
				default:
					break;
				}

				mixerdata.call_r |= eSoundCallReason_MixerNewTask;

				// вызовем микшер из сна
				_CV_ALL(mixerdata.cv_mixer);
			}

			return hRes;
		}

	private:
		error_t ControlMixerPRIV(eSoundCallReason reason)
		{
			error_t hRes = eSoundError_TRUE;

			switch (reason)
			{
			case eSoundCallReason_MixerRun:
				{
					if (!mixerdata.state_runned) // микшер не запущен?
					{
						{
							ulock lk_wait_mixer_run (mixerdata.m_wait);

							mixerdata.wait_run = false;

							if (mixpool.GetWorkers() < MIXPOOL_THREADS)
								mixpool.WorkersSet(MIXPOOL_THREADS);

							// запуск потока микшера
							mixpool(&CSound::MixerLoopThread, this);

							// ждём достоверности запуска
							while (!mixerdata.wait_run)
							{
								_CV_WAIT(mixerdata.cv_wait, lk_wait_mixer_run);
							}
						}

						mixerdata.state_runned = true;
					}
					break;
				}
			case eSoundCallReason_MixerAbort:
				{
					if (mixerdata.state_runned) // микшер запущен?
					{
						{
							ulock lk_wait_mixer_exit (mixerdata.m_wait);

							mixerdata.wait_exit = false;

							// вызов на закрытие
							{
								glock data_lock (mixerdata.m_mixer);
								mixerdata.call_r |= eSoundCallReason_MixerAbort;
								_CV_ALL(mixerdata.cv_mixer);
							}

							// ждём достоверности закрытия
							while (!mixerdata.wait_exit)
							{
								_CV_WAIT(mixerdata.cv_wait, lk_wait_mixer_exit);
							}
						}

						mixerdata.state_runned = false;
						hRes = mixerdata.ERR;
					}
					break;
				}			
			case eSoundCallReason_MixerPause:  // ! FOR DEBUG ONLY !
				{
					if (!mixerdata.state_runned) // микшер не запущен?
						break;

					if (!mixerdata.state_paused) // микшер вне паузы?
					{
						{
							ulock lk_wait_mixer_pause (mixerdata.m_wait);

							mixerdata.wait_pause = false;

							// вызов на паузу
							{
								glock data_lock (mixerdata.m_mixer);
								mixerdata.call_r |= eSoundCallReason_MixerPause;
								_CV_ALL(mixerdata.cv_mixer);
							}

							// ждём достоверности паузы
							while (!mixerdata.wait_pause)
							{
								_CV_WAIT(mixerdata.cv_wait, lk_wait_mixer_pause);
							}
						}

						mixerdata.state_paused = true;
					}
					break;
				}
			case eSoundCallReason_MixerResume:  // ! FOR DEBUG ONLY !
				{
					if (!mixerdata.state_runned) // микшер не запущен?
						break;

					if (mixerdata.state_paused) // микшер в паузе?
					{
						{
							ulock lk_wait_mixer_resume (mixerdata.m_wait);

							mixerdata.wait_resume = false;

							// вызов выхода из паузы
							{
								glock pause_lock (mixerdata.m_pause);
								mixerdata.pause_end = true;
								_CV_ALL(mixerdata.cv_pause);
							}

							// ждём достоверности выхода из паузы
							while (!mixerdata.wait_resume)
							{
								_CV_WAIT(mixerdata.cv_wait, lk_wait_mixer_resume);
							}
						}

						mixerdata.state_paused = false;
					}
					break;
				}
			default:
				hRes = eSoundError_Unknown;
				break;
			}

			return hRes;
		}
	private:
		error_t ControlMixerTaskInterruptPRIV(uint64 id, bool group, bool wait)
		{
			// При (wait == true) сами сразу всё очищаем, иначе - микшер позже почистит

			error_t hRes = eSoundError_FALSE; // вернуть <false>, если задач в микшере не найдено

			const bool all = _ISMISS(id);

			if (!all)
			{
				if (group)
				{
					glock stop_mixer (mixerdata.m_mixer); // блокируем микшер
				
					uint32 task_num = mixerdata.task_num;
					
					if (task_num)
					{ 
						uint32 target_group = (uint32) id;
						auto & task         = mixerdata.task;

						for (uint32 i = 0; i < task_num; i++)
						if (target_group == task[i]->GID.group)
						{
							auto & RULE = task[i]->RULE;

							RULE.interrupt    = true;
							RULE.pause        = false;
							RULE.rewind       = false;
							RULE.rewind_pause = false;
							RULE.volume       = false;

							if (wait) 
							{
								task[i]->ERR    = eSoundError_Normal_2;
								task[i]->b_done = true;
							}

							hRes = eSoundError_TRUE;
						}

						if (hRes == eSoundError_TRUE)
						{
							if (wait)
								MixerLoopThread_ClearDone();
							else
							{
								mixerdata.call_r |= eSoundCallReason_MixerUpdate;
								_CV_ALL(mixerdata.cv_mixer);
							}
						}
					}
				}
				else // target sound id
				{
					glock stop_mixer (mixerdata.m_mixer); // блокируем микшер

					uint32 p     = 0;                      // счетчик проигрывающихся
					uint32 p_max = snd[id]->players_mixer; // всего проигрывающихся

					if (p_max)
					{
						uint64 GID      = snd[id]->GID;
						uint32 task_num = mixerdata.task_num;
						auto & task     = mixerdata.task;

						for (uint32 i = 0; i < task_num; i++)
						if (GID == task[i]->GID)
						{
							auto & RULE = task[i]->RULE;

							RULE.interrupt    = true;
							RULE.pause        = false;
							RULE.rewind       = false;
							RULE.rewind_pause = false;
							RULE.volume       = false;

							if (wait)
							{
								task[i]->ERR    = eSoundError_Normal_2;
								task[i]->b_done = true;
							}							

							if ((++p) == p_max) break; // выходим, если обзвонили всех
						}

						hRes = eSoundError_TRUE;

						if (wait)
							MixerLoopThread_ClearDone();
						else
						{
							mixerdata.call_r |= eSoundCallReason_MixerUpdate;
							_CV_ALL(mixerdata.cv_mixer);
						}
					}
				}
			}
			else // all tasks
			{
				glock stop_mixer (mixerdata.m_mixer); // блокируем микшер

				uint32 task_num = mixerdata.task_num;
				
				if (task_num)
				{
					auto & task = mixerdata.task;

					for (uint32 i = 0; i < task_num; i++)
					{
						auto & RULE = task[i]->RULE;

						RULE.interrupt    = true;
						RULE.pause        = false;
						RULE.rewind       = false;
						RULE.rewind_pause = false;
						RULE.volume       = false;

						if (wait)
						{
							task[i]->ERR    = eSoundError_Normal_2;
							task[i]->b_done = true;
						}							
					}

					hRes = eSoundError_TRUE;

					if (wait)
						MixerLoopThread_ClearDone();
					else
					{
						mixerdata.call_r |= eSoundCallReason_MixerUpdate;
						_CV_ALL(mixerdata.cv_mixer);
					}
				}
			}

			return hRes;
		}
		error_t ControlMixerTaskPausePRIV(uint64 id, bool group, bool resume)
		{
			// Нет прямой "wait"-реализации через condition_variable.
			// Использовать вместо этого задержку MIXER_PLAYTHREAD_WAIT_TIME на вызове "сверху".

			error_t hRes = eSoundError_FALSE; // вернуть <false>, если задач в микшере не найдено

			const bool all = _ISMISS(id);
			
			if (!all)
			{
				if (group)
				{
					glock stop_mixer (mixerdata.m_mixer); // блокируем микшер

					uint32 task_num = mixerdata.task_num;
					
					if (task_num)
					{
						uint32 target_group = (uint32) id;
						auto & task         = mixerdata.task;

						for (uint32 i = 0; i < task_num; i++)
						if (target_group == task[i]->GID.group)
						{
							auto & RULE = task[i]->RULE;

							if (RULE.interrupt)
								continue;
							else
								hRes = eSoundError_TRUE;

							RULE.pause = !resume;
						}

						if (hRes == eSoundError_TRUE)
						{ 
							mixerdata.call_r |= eSoundCallReason_MixerUpdate;
							_CV_ALL(mixerdata.cv_mixer);
						}
					}
				}
				else // target sound id
				{
					glock stop_mixer (mixerdata.m_mixer); // блокируем микшер

					uint32 p     = 0;                      // счетчик проигрывающихся
					uint32 p_max = snd[id]->players_mixer; // всего проигрывающихся

					if (p_max)
					{
						uint64 GID      = snd[id]->GID;
						uint32 task_num = mixerdata.task_num;
						auto & task     = mixerdata.task;

						for (uint32 i = 0; i < task_num; i++)
						if (GID == task[i]->GID)
						{
							auto & RULE = task[i]->RULE;

							if (RULE.interrupt)
							{
								if ((++p) == p_max) break;
								else continue;
							}
							else
								hRes = eSoundError_TRUE;

							RULE.pause = !resume;

							if ((++p) == p_max) break; // выходим, если обзвонили всех
						}

						if (hRes == eSoundError_TRUE)
						{
							mixerdata.call_r |= eSoundCallReason_MixerUpdate;
							_CV_ALL(mixerdata.cv_mixer);
						}
					}
				}
			}
			else // all tasks
			{
				glock stop_mixer (mixerdata.m_mixer); // блокируем микшер

				uint32 task_num = mixerdata.task_num;
				
				if (task_num)
				{
					auto & task = mixerdata.task;

					for (uint32 i = 0; i < task_num; i++)
					{
						auto & RULE = task[i]->RULE;

						if (RULE.interrupt)
							continue;
						else
							hRes = eSoundError_TRUE;

						RULE.pause = !resume;
					}

					if (hRes == eSoundError_TRUE)
					{
						mixerdata.call_r |= eSoundCallReason_MixerUpdate;
						_CV_ALL(mixerdata.cv_mixer);
					}
				}
			}

			return hRes;
		}
		error_t ControlMixerTaskRewindPRIV(uint64 id, float time_in_seconds, bool pause_after, bool group)
		{
			// Нет прямой "wait"-реализации через condition_variable.
			// Использовать вместо этого задержку MIXER_PLAYTHREAD_WAIT_TIME на вызове "сверху".

			#define _CHECK_TIME_                                 \
				float requested_time;                            \
				float duration = task[i]->sound->duration;       \
				if (duration < time_in_seconds)                  \
					 requested_time = duration;                  \
				else requested_time = time_in_seconds;           \
				RULE.rewind_time = requested_time;               \

			//////////////////////////////////////////////////////

			error_t hRes = eSoundError_FALSE; // вернуть <false>, если задач в микшере не найдено

			const bool all = _ISMISS(id);

			time_in_seconds = abs(time_in_seconds);
			
			if (!all)
			{
				if (group)
				{
					glock stop_mixer (mixerdata.m_mixer); // блокируем микшер

					uint32 task_num = mixerdata.task_num;
					
					if (task_num)
					{
						uint32 target_group = (uint32) id;
						auto & task         = mixerdata.task;

						for (uint32 i = 0; i < task_num; i++)
						if (target_group == task[i]->GID.group)
						{
							auto & RULE = task[i]->RULE;

							if (RULE.interrupt)
								continue;
							else
								hRes = eSoundError_TRUE;

							RULE.rewind       = true;
							RULE.pause        = false;
							RULE.rewind_pause = pause_after;

							_CHECK_TIME_
						}

						if (hRes == eSoundError_TRUE)
						{
							mixerdata.call_r |= eSoundCallReason_MixerUpdate;
							_CV_ALL(mixerdata.cv_mixer);
						}
					}
				}
				else // target sound id
				{
					glock stop_mixer (mixerdata.m_mixer); // блокируем микшер

					uint32 p     = 0;                      // счетчик проигрывающихся
					uint32 p_max = snd[id]->players_mixer; // всего проигрывающихся

					if (p_max)
					{
						uint64 GID      = snd[id]->GID;
						uint32 task_num = mixerdata.task_num;
						auto & task     = mixerdata.task;

						for (uint32 i = 0; i < task_num; i++)
						if (GID == task[i]->GID)
						{
							auto & RULE = task[i]->RULE;

							if (RULE.interrupt)
							{
								if ((++p) == p_max) break;
								else continue;
							}
							else
								hRes = eSoundError_TRUE;

							RULE.rewind       = true;
							RULE.pause        = false;
							RULE.rewind_pause = pause_after;

							_CHECK_TIME_

							if ((++p) == p_max) break; // выход, если обзвонили всех
						}

						if (hRes == eSoundError_TRUE)
						{
							mixerdata.call_r |= eSoundCallReason_MixerUpdate;
							_CV_ALL(mixerdata.cv_mixer);
						}
					}
				}
			}
			else // all tasks
			{
				glock stop_mixer (mixerdata.m_mixer); // блокируем микшер

				uint32 task_num = mixerdata.task_num;
	
				if (task_num)
				{
					auto & task = mixerdata.task;
	
					for (uint32 i = 0; i < task_num; i++)
					{
						auto & RULE = task[i]->RULE;

						if (RULE.interrupt)
							continue;
						else
							hRes = eSoundError_TRUE;

						RULE.rewind       = true;
						RULE.pause        = false;
						RULE.rewind_pause = pause_after;

						_CHECK_TIME_
					}

					if (hRes == eSoundError_TRUE)
					{
						mixerdata.call_r |= eSoundCallReason_MixerUpdate;
						_CV_ALL(mixerdata.cv_mixer);
					}
				}
			}

			return hRes;

			#undef _CHECK_TIME_
		}
	private:
		error_t ControlMixerSetMasterVolumePRIV(float volume)
		{
			error_t hRes = eSoundError_FALSE; // вернуть <false>, если задач в микшере не найдено

			{
				glock stop_mixer (mixerdata.m_mixer);

				mixerdata.master_volume = volume;

				uint32 task_num = mixerdata.task_num;

				if (task_num)
				{
					auto & task = mixerdata.task;

					for (uint32 i = 0; i < task_num; i++)
					{
						auto & t = *task[i];

						if (t.RULE.interrupt)
							continue;
						else
							hRes = eSoundError_TRUE;

						t.RULE.volume = true;
					}

					if (hRes == eSoundError_TRUE)
					{
						mixerdata.call_r |= eSoundCallReason_MixerUpdate;
						_CV_ALL(mixerdata.cv_mixer);
					}
				}
			}

			return hRes;
		}
		error_t ControlMixerSetVolumePRIV(uint64 id, float volume, bool group)
		{
			error_t hRes = eSoundError_FALSE; // вернуть <false>, если задач в микшере не найдено

			const bool all = _ISMISS(id);

			if (!all)
			{
				if (group)
				{
					glock stop_mixer (mixerdata.m_mixer); // блокируем микшер

					uint32 task_num = mixerdata.task_num;
					
					if (task_num)
					{
						uint32 target_group = (uint32) id;
						auto & task         = mixerdata.task;

						for (uint32 i = 0; i < task_num; i++)
						if (target_group == task[i]->GID.group)
						{
							auto & t = *task[i];

							if (t.RULE.interrupt)
								continue;
							else
								hRes = eSoundError_TRUE;

							t.RULE.volume    = true;
							t.VOL.multiplier = volume;
						}

						if (hRes == eSoundError_TRUE)
						{
							mixerdata.call_r |= eSoundCallReason_MixerUpdate;
							_CV_ALL(mixerdata.cv_mixer);
						}
					}
				}
				else // target sound id
				{
					glock stop_mixer (mixerdata.m_mixer); // блокируем микшер

					uint32 p     = 0;                      // счетчик проигрывающихся
					uint32 p_max = snd[id]->players_mixer; // всего проигрывающихся

					if (p_max)
					{
						uint64 GID      = snd[id]->GID;
						uint32 task_num = mixerdata.task_num;
						auto & task     = mixerdata.task;

						for (uint32 i = 0; i < task_num; i++)
						if (GID == task[i]->GID)
						{
							auto & t = *task[i];

							if (t.RULE.interrupt)
							{
								if ((++p) == p_max) break;
								else continue;
							}
							else
								hRes = eSoundError_TRUE;

							t.RULE.volume    = true;
							t.VOL.multiplier = volume;

							if ((++p) == p_max) break; // выход, если обзвонили всех
						}

						if (hRes == eSoundError_TRUE)
						{
							mixerdata.call_r |= eSoundCallReason_MixerUpdate;
							_CV_ALL(mixerdata.cv_mixer);
						}
					}
				}
			}
			else // all tasks
			{
				glock stop_mixer (mixerdata.m_mixer); // блокируем микшер

				uint32 task_num = mixerdata.task_num;

				if (task_num)
				{
					auto & task = mixerdata.task;

					for (uint32 i = 0; i < task_num; i++)
					{
						auto & t = *task[i];

						if (t.RULE.interrupt)
							continue;
						else
							hRes = eSoundError_TRUE;

						t.RULE.volume    = true;
						t.VOL.multiplier = volume;
					}

					if (hRes == eSoundError_TRUE)
					{
						mixerdata.call_r |= eSoundCallReason_MixerUpdate;
						_CV_ALL(mixerdata.cv_mixer);
					}				
				}
			}

			return hRes;
		}
		error_t ControlMixerSetVolumeChannelsPRIV(uint64 id, float * volume, uint32 numChannels)
		{
			error_t hRes = eSoundError_FALSE; // вернуть <false>, если задач в микшере не найдено

			const bool all   = _ISMISS(id);
			const bool group = false;

			static_assert (
				8 == PCM_MAXCHANNELS,
				"FIX HERE IF MAXCHANNELS CHANGES ! [2]"
			);

			static const float ___ [PCM_MAXCHANNELS] = { 1, 1, 1, 1, 1, 1, 1, 1 };
			float * vptr = nullptr;
			bool reset = (volume == nullptr) ;

			if (reset) {	vptr = const_cast <float*> (___) ;
							numChannels = PCM_MAXCHANNELS ;			                }
			else       {    vptr = volume;
				            numChannels = _CLAMP(numChannels, 0, PCM_MAXCHANNELS);  }
		
			if (!all)       // только для конкретного sound_id (пока что)
			{               // .
				if (!group) // .
				{
					glock stop_mixer (mixerdata.m_mixer); // блокируем микшер

					uint32 p     = 0;                      // счетчик проигрывающихся
					uint32 p_max = snd[id]->players_mixer; // всего проигрывающихся

					if (p_max)
					{
						uint64 GID      = snd[id]->GID;
						uint32 task_num = mixerdata.task_num;
						auto & task     = mixerdata.task;

						for (uint32 i = 0; i < task_num; i++)
						if (GID == task[i]->GID)
						{
							auto & t = *task[i];

							if (t.RULE.interrupt)
							{
								if ((++p) == p_max) break;
								else continue;
							}
							else
								hRes = eSoundError_TRUE;

							t.RULE.volume = true;

							//memcpy(t.VOL.value, vptr, numChannels * sizeof(float));

							for (uint32 ch = 0; ch < numChannels; ch++)
								t.VOL.value[ch] = _CLAMP(vptr[ch], 0.f, 1.f);

							if ((++p) == p_max) break; // выход, если обзвонили всех
						}

						if (hRes == eSoundError_TRUE)
						{
							mixerdata.call_r |= eSoundCallReason_MixerUpdate;
							_CV_ALL(mixerdata.cv_mixer);
						}
					}
				}
			}

			return hRes;
		}
		error_t ControlMixerSetVolumeChannelsPRIV(uint64 id, float   volume, uint32 numChannels)
		{
			error_t hRes = eSoundError_FALSE; // вернуть <false>, если задач в микшере не найдено

			const bool all   = _ISMISS(id);
			const bool group = false;

			numChannels = _CLAMP(numChannels, 0, PCM_MAXCHANNELS);

			if (!all)       // только для конкретного sound_id (пока что)
			{               // .
				if (!group) // .
				{
					glock stop_mixer (mixerdata.m_mixer); // блокируем микшер

					uint32 p     = 0;                      // счетчик проигрывающихся
					uint32 p_max = snd[id]->players_mixer; // всего проигрывающихся

					if (p_max)
					{
						uint64 GID      = snd[id]->GID;
						uint32 task_num = mixerdata.task_num;
						auto & task     = mixerdata.task;

						for (uint32 i = 0; i < task_num; i++)
						if (GID == task[i]->GID)
						{
							auto & t = *task[i];

							if (t.RULE.interrupt)
							{
								if ((++p) == p_max) break;
								else continue;
							}
							else
								hRes = eSoundError_TRUE;

							t.RULE.volume = true;

							for (uint32 ch = 0; ch < numChannels; ch++)
								t.VOL.value[ch] = _CLAMP(volume, 0.f, 1.f);

							if ((++p) == p_max) break; // выход, если обзвонили всех
						}

						if (hRes == eSoundError_TRUE)
						{
							mixerdata.call_r |= eSoundCallReason_MixerUpdate;
							_CV_ALL(mixerdata.cv_mixer);
						}
					}
				}
			}

			return hRes;
		}
	public:
		//>> Возвращает <false>, если микшер застрял - тогда требуется ReActivateMixer()
		bool GetStateMixer()
		{
			// микшер зависает в простое, если потерян поток воспроизведения
			if (mixerdata.ERR == eSoundError_MixerLostThr)
				return false;

			return true;
		}

	//////////////// Data load/open block ////////////////

	private:
		//>> Описание данных .wav
		error_t ConvertToDesc(WAVERIFF & wav, SNDDESC * snddesc)
		{
			error_t hRes = eSoundError_TRUE;

			switch (wav.audioFormat)
			{
			case WAVE_FORMAT_PCM:
			case WAVE_FORMAT_EXTENSIBLE:
				wav.audioFormat = WAVE_FORMAT_PCM;
				break;
			default:
				hRes = eSoundError_BadWaveTag;
				break;
			}

			// Write "encoded" params

			snddesc->wFormatTag      = wav.audioFormat;
			snddesc->nChannels       = wav.numChannels;
			snddesc->nSamplesPerSec  = wav.sampleRate;
			snddesc->nAvgBytesPerSec = wav.byteRate;
			snddesc->nBlockAlign     = wav.blockAlign;
			snddesc->wBitsPerSample  = wav.bitsPerSample;
			snddesc->cbSize          = 0;

			// Write "decoded" params

			snddesc->iecAverageBytesPerSec   = snddesc->nAvgBytesPerSec;
			snddesc->iecEncodedChannelCount  = snddesc->nChannels;
			snddesc->iecEncodedSamplesPerSec = snddesc->nSamplesPerSec;

			if (hRes != eSoundError_TRUE)
			{
				return hRes;
			}
			else if (snddesc->wBitsPerSample % 8)
			{
				hRes = eSoundError_BadBits;
			}
			else if (snddesc->nChannels > PCM_MAXCHANNELS)
			{
				hRes = eSoundError_BadChannels;
			}
			else if (snddesc->nSamplesPerSec > PCM_MAXSAMPLESPERSEC)
			{
				hRes = eSoundError_BadFreq;
			}

			return hRes;
		}

		//>> Считывание файла .wav
		error_t OpenSoundWAV(FILE *fp, TBUFFERS <byte, int32> * file, const wchar_t* filename)
		{
			#define _RESET_RETURN sound.filename.clear();           \
							      sound.file.Close();               \
								  sound.file_offset = 0;            \
								  sound.file_size = 0;              \
							      sound.type = eSoundType_UNKNOWN;  \
							      sound.duration = 0.f;             \
							      snd_count--;                      \
							      return hRes;                      \

			error_t hRes = eSoundError_TRUE;

			WAVERIFF wav;
			int wav_data_pos;

			if (file == nullptr)
				 hRes = wav._Read(fp);
			else hRes = wav._Read(file->buf, file->count, file->count, wav_data_pos);

			switch (hRes)
			{
			case eWaveErr_NONE: // no errors = SUCCESS
				hRes = eSoundError_TRUE;
				break;
			default: // some errors = FAILURE
				wprintf(L"\nWAVE READ ERROR %i (0x%X) : %s", hRes, hRes, filename);
				hRes = eSoundError_ReadFileWAV;
				break;
			}

			if (hRes != eSoundError_TRUE) // can't read the file
				return hRes;              // .

			/////////////////////////////
		
			if (snd_count == snd.size())   // Добавим
				snd.emplace_back_create(); // .
					
			uint64 id = snd_count++;       // ID текущего

			if (snd[id] == nullptr)        // Создадим
				snd.ReCreate(id);          // .

			auto & sound = *snd[id];
			sound.type = eSoundType_WAVE;

			hRes = ConvertToDesc(wav, sound.snddesc);
		
			if (hRes != eSoundError_TRUE) { _RESET_RETURN ; }

			/////////////////////////////

			sound.filename = filename;		
			if (file == nullptr)
			{
			//	if (sound.file.Create(wav.subchunk2Size))
			//	{
			//		fread(sound.file.buf, sizeof(byte), sound.file.count, fp); // fp уже fseek'нут на wav_data_pos !
			//	}
			//	else hRes = eSoundError_BadAlloc;

				sound.file_offset = (uint32) _ftelli64(fp); // поправка на RAW-данные
				sound.file_size   = wav.subchunk2Size;      // .
				rewind(fp);

				uint32 fsize = (uint32) _filelengthi64(_fileno(fp));
				if (sound.file.Create(fsize))
				{
					fread(sound.file.buf, sizeof(byte), fsize, fp);
				}
				else hRes = eSoundError_BadAlloc;
			}
			else // from memory
			{
			//	if (sound.file.Create(wav.subchunk2Size))
			//	{
			//		memcpy(sound.file.buf, file->buf + wav_data_pos, wav.subchunk2Size);
			//	}
			//	else hRes = eSoundError_BadAlloc;

				sound.file_offset = wav_data_pos;      // поправка на RAW-данные
				sound.file_size   = wav.subchunk2Size; // .

				file->Link(sound.file);
			}
			//PrintWaveRiff(wav);

			if (hRes != eSoundError_TRUE) { _RESET_RETURN ; }

			sound.duration  = (float)wav.subchunk2Size;
			sound.duration /= (float)wav.bitsPerSample / 8;
			sound.duration /= wav.numChannels;
			sound.duration /= wav.sampleRate;

			// сброс запомненной ошибки и выход
			return sound.baddat_err = hRes;

			#undef _RESET_RETURN
		}

		//>> Считывание файла .mp3
		error_t OpenSoundMP3(FILE *fp, TBUFFERS <byte, int32> * file, const wchar_t* filename)
		{
			#define _RESET_RETURN sound.filename.clear();           \
							      sound.file.Close();               \
								  sound.file_offset = 0;            \
								  sound.file_size = 0;              \
							      sound.type = eSoundType_UNKNOWN;  \
							      sound.duration = 0.f;             \
							      snd_count--;                      \
							      return hRes;                      \

			error_t hRes = eSoundError_TRUE;

			if (snd_count == snd.size())   // Добавим
				snd.emplace_back_create(); // .

			uint64 id = snd_count++;       // ID текущего

			if (snd[id] == nullptr)        // Создадим
				snd.ReCreate(id);          // .

			auto & sound = *snd[id];
			sound.type        = eSoundType_MP3;
			sound.filename    = filename;
			
			if (file == nullptr)
			{
				rewind(fp);
				if (sound.file.Create(_filelength(_fileno(fp))))
				{
					fread(sound.file.buf, sizeof(byte), sound.file.count, fp);
				}
				else return hRes = eSoundError_BadAlloc;
			}
			else // from memory
			{
				file->Link(sound.file);
			}

			if (hRes != eSoundError_TRUE) { _RESET_RETURN ; }

			// Оставим позицию пустой
			if (!CMP3Converter::_Duration(sound.file.buf, sound.file.count, sound.duration))
			{
				wchar_t error [MAX_PATH * 2];
				wsprintf(error, L"\n%s :\nMP3 duration analyze error\n", filename);
				//_MBM(error);

				hRes = eSoundError_ReadFileMP3;

				_RESET_RETURN ;
			}

			// для mp3 без поправок
			sound.file_offset = 0;
			sound.file_size   = sound.file.count;

			// сброс запомненной ошибки и выход
			return sound.baddat_err = hRes;

			#undef _RESET_RETURN
		}

		//>> Определение формата и вызов соотв. функции
		error_t OpenSoundFormat(const wchar_t* filepath, const wchar_t* filename)
		{
			error_t hRes = eSoundError_TRUE;

			eSoundType snd_type;

			FILE *fp = _wfsopen(filepath, L"rb", _SH_DENYNO);

				 if (WAVERIFF      :: _Check(fp)) snd_type = eSoundType_WAVE;
			else if (CMP3Converter :: _Check(fp)) snd_type = eSoundType_MP3;
			else                                  snd_type = eSoundType_UNKNOWN;

			switch (snd_type)
			{
			case eSoundType_WAVE:    hRes = OpenSoundWAV(fp, nullptr, filename); break;
			case eSoundType_MP3:     hRes = OpenSoundMP3(fp, nullptr, filename); break;
			case eSoundType_OGG:
			case eSoundType_AAC:
			case eSoundType_UNKNOWN:
			default:
				hRes = eSoundError_UnkFileType;
				//_MBM(ERROR_UnknownDataType);
			};

			fclose(fp);

			return hRes;
		};

		//>> Определение формата и вызов соотв. функции
		error_t OpenSoundFormat(TBUFFERS <byte, int32> * file, const wchar_t* filename)
		{
			error_t hRes = eSoundError_TRUE;

			eSoundType snd_type;

				 if (WAVERIFF      :: _Check((__int32 *) file->buf, file->count)) snd_type = eSoundType_WAVE;
			else if (CMP3Converter :: _Check(            file->buf, file->count)) snd_type = eSoundType_MP3;
			else                                                                  snd_type = eSoundType_UNKNOWN;

			switch (snd_type)
			{
			case eSoundType_WAVE:    hRes = OpenSoundWAV(nullptr, file, filename); break;
			case eSoundType_MP3:     hRes = OpenSoundMP3(nullptr, file, filename); break;
			case eSoundType_OGG:
			case eSoundType_AAC:
			case eSoundType_UNKNOWN:
			default:
				hRes = eSoundError_UnkFileType;
				//_MBM(ERROR_UnknownDataType);
			};

			return hRes;
		};

		//>> Открытие файла с проверкой доступности
		error_t OpenSoundProc(const wchar_t* gamePath, const wchar_t* filename,
			                  TBUFFERS <byte, int32> * file)
		{
			error_t hRes = eSoundError_TRUE;

			if (file == nullptr)
			{
				if (gamePath == nullptr || filename == nullptr)
					hRes = eSoundError_BadPath;
				else
				{
					wchar_t syspath[MAX_PATH], error[MAX_PATH * 2], p1[MAX_PATH], p2[MAX_PATH];
					GetCurrentDirectory(MAX_PATH, syspath);

					wchar_t *p = nullptr;
				
					wsprintf(p1,L"%s\\%s\\%s",gamePath,DIRECTORY_SOUNDS,filename);
					wsprintf(p2,L"%s\\%s\\%s",syspath,DIRECTORY_SOUNDS,filename);
				
				//	if (p = LoadFileCheck(2, p1, p2)) 
					if (p = LoadFileCheck( { p1, p2 } ))
						hRes = OpenSoundFormat(p, filename);
					else
					{
						wsprintf(error, L"%s %s", ERROR_OpenFile, filename);
						//_MBM(error);
						wprintf(L"\n%s", error);
						hRes = eSoundError_FileNotFound;
					}
				}
			}
			else // loading from memory
			{
				hRes = OpenSoundFormat(file, filename);
			}

			return hRes;
		};

	private:
		error_t LoadPRIV(const wchar_t* gamePath, const wchar_t* filename,
			             TBUFFERS <byte, int32> * file, uint32 & ID)
		{
			error_t hRes = eSoundError_TRUE;

			if (!isInit) return hRes = eSoundError_NoInit;

			hRes = OpenSoundProc(gamePath, filename, file);

			if (hRes != eSoundError_TRUE)
			{
				wprintf(L"\nError 0x%X loading audio \"%s\"", hRes, filename);
			}
			else
			{
				ID = snd_count - 1;
			}

			return hRes;
		}
		error_t LoadLinkedPRIV(uint32 linked_ID, uint32 & ID)
		{
			error_t hRes = eSoundError_TRUE;

			if (snd_count == snd.size())   // Добавим
				snd.emplace_back_create(); // .
					
			uint64 id = snd_count++;       // ID текущего

			if (snd[id] == nullptr)        // Создадим
				snd.ReCreate(id);          // .

			//////////////////////////////

			snd[linked_ID]->Link(*snd[id]);

			ID = (uint32) id;

			return hRes;
		}

	private:
		//>> Загрузка файла
		error_t Load(const wchar_t* gamePath, const wchar_t* filename,
			         TBUFFERS <byte, int32> * file,
					 uint32 target_group, uint64 & OUT_GID)
		{
			glock safe_lock (m_load_delete); // защита на время манипуляций со списками snd[] и snd_g[]
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (target_group >= eSoundGroup_USER_MAX) // ERROR bad group ID (LIMIT REACHED)
				return hRes = eSoundError_MaxGroupID; // .

			{
				if (snd_count == MASK_INT32)               // ERROR insane situation
					return hRes = eSoundError_MaxNumFiles; // .

				uint64_gid GID;    // out ID to user
				uint32     snd_id; // real ID in snd list
				uint32     grp_id; // real ID of group

				// GID = группа + порядковый номер в группе
				// в ячейке по порядковому номеру указан настоящий ID

				grp_id = target_group + eSoundGroup_USER;

				if ((grp_id + 1) > snd_g.size()) // создаём группу, если нет
					snd_g.SetCreate(grp_id + 1); // .

				auto & group = * snd_g [grp_id];

				if (group.size() == eSoundGroup_ELEMENT_MAX) // ERROR group limit
					return hRes = eSoundError_MaxGroup;      // .

				{
					hRes = LoadPRIV(gamePath, filename, file, snd_id);

					if (hRes == eSoundError_TRUE)
					{
						uint32 group_size     = (uint32) group.size();
						uint32 group_capacity = (uint32) group.capacity();

						GID.element = group_size;   // данные будут в конце списка
						GID.group   = target_group; // номер группы, указанный пользователем
						OUT_GID     = GID;          // .

						if (group_size + 1 >= group_capacity)                     // резервировать места
							group.reserve(group_capacity + eSoundGroup_CAPACITY); // .

						group.resize(group_size + 1); // поправляем размер группы
						group [group_size] = snd_id;  // сохраняем в ячейку у группы настоящий ID
						snd [snd_id]->GID = GID;      // для обратной связи
					}
					else
					{
						wprintf(L"\nCSound::Load failed, ERR=0x%X (%s)", hRes, filename);
					}
				}
			}

			return hRes;
		}

	public:
		//>> Загрузка файла по пути (номер группы на выбор)
		error_t Load(const wchar_t* gamePath, const wchar_t* filename, uint32 target_group, uint64 & OUT_GID)
		{
			return Load(std::move(gamePath), std::move(filename), 
				        std::move(nullptr),
						std::move(target_group), OUT_GID);
		}

		//>> Загрузка файла по пути (в группу DEFAULT_SOUND_GROUP)
		error_t Load(const wchar_t* gamePath, const wchar_t* filename, uint64 & OUT_GID)
		{
			return Load(std::move(gamePath), std::move(filename), 
				        std::move(nullptr),
						std::move(DEFAULT_SOUND_GROUP), OUT_GID);
		}

		//>> Загрузка из памяти (номер группы на выбор)
		error_t Load(TBUFFERS <byte, int32> * file, const wchar_t* filename, uint32 target_group, uint64 & OUT_GID)
		{
			return Load(std::move(nullptr),  std::move(filename),
				        std::move(file),
						std::move(target_group), OUT_GID);
		}

		//>> Загрузка из памяти (в группу DEFAULT_SOUND_GROUP)
		error_t Load(TBUFFERS <byte, int32> * file, const wchar_t* filename, uint64 & OUT_GID)
		{
			return Load(std::move(nullptr),  std::move(filename),
				        std::move(file),
						std::move(DEFAULT_SOUND_GROUP), OUT_GID);
		}

		//>> Связка с уже загруженным
		error_t Load(uint64_gid linked_GID, uint32 target_group, uint64 & OUT_GID)
		{
			glock safe_lock (m_load_delete); // защита на время манипуляций со списками snd[] и snd_g[]
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (eSoundError_TRUE != (hRes = CheckSound(linked_GID))) return hRes;  // ERROR bad sound

			if (target_group >= eSoundGroup_USER_MAX) // ERROR bad group ID (LIMIT REACHED)
				return eSoundError_MaxGroupID;        // .

			{
				if (snd_count == MASK_INT32)               // ERROR insane situation
					return hRes = eSoundError_MaxNumFiles; // .

				uint64_gid GID;    // out ID to user
				uint32     snd_id; // real ID in snd list
				uint32     grp_id; // real ID of group

				grp_id = target_group + eSoundGroup_USER;

				if ((grp_id + 1) > snd_g.size()) // создаём группу, если нет
					snd_g.SetCreate(grp_id + 1); // .

				auto & group = * snd_g [grp_id];

				if (group.size() == eSoundGroup_ELEMENT_MAX) // ERROR group limit
					return hRes = eSoundError_MaxGroup;      // .

				{
					hRes = LoadLinkedPRIV((uint32)_SoundID(linked_GID), snd_id);

					if (hRes == eSoundError_TRUE)
					{
						uint32 group_size     = (uint32) group.size();
						uint32 group_capacity = (uint32) group.capacity();

						GID.element = group_size;   // данные будут в конце списка
						GID.group   = target_group; // номер группы, указанный пользователем
						OUT_GID     = GID;          // .

						if (group_size + 1 >= group_capacity)                     // резервировать места
							group.reserve(group_capacity + eSoundGroup_CAPACITY); // .

						group.resize(group_size + 1); // поправляем размер группы
						group [group_size] = snd_id;  // сохраняем в ячейку у группы настоящий ID
						snd [snd_id]->GID = GID;      // для обратной связи
					}
					else
					{
						wprintf(L"\nCSound::Load failed, ERR=0x%X (0x%016llX)", hRes, linked_GID._u64);
					}
				}
			}

			return hRes;
		}

		//>> Загрузчик по списку
		error_t Load(CSoundLoadList & list)
		{
			glock safe_lock (m_load_delete); // защита на время манипуляций со списками snd[] и snd_g[]
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			uint64 num_to_load = list.info.size();

			if (num_to_load >= MASK_INT32)             // ERROR insane situation
				return hRes = eSoundError_MaxNumFiles; // .

			const wchar_t* gamePath   = list.gamepath.c_str();
			uint64         gamePathSz = list.gamepath.size();

			{
				for (uint64 i=0; i<num_to_load; i++)
				{
					error_t err;
					auto & info = *list.info[i];

					if (snd_count == MASK_INT32)               // ERROR insane situation
					{                                          // .
						info.result = eSoundError_MaxNumFiles; // .
						continue;                              // .
					}                                          // .
					
					if (info.GID.group >= eSoundGroup_USER_MAX) // ERROR bad group ID (LIMIT REACHED)
					{                                           // .
						info.result = eSoundError_MaxGroupID;   // .
						continue;                               // .
					}                                           // .

					if (info.linked_GID != nullptr)
					{
						if (eSoundError_TRUE != (err = CheckSound(*info.linked_GID))) // ERROR bad linked sound
						{                                                             // .
							info.result = err;                                        // .
							continue;                                                 // .
						}                                                             // .
					}
					
					uint64_gid GID;    // out ID to user
					uint32     snd_id; // real ID in snd list
					uint32     grp_id; // real ID of group

					grp_id = info.GID.group + eSoundGroup_USER;

					if ((grp_id + 1) > snd_g.size()) // создаём группу, если нет
						snd_g.SetCreate(grp_id + 1); // .

					auto & group = *snd_g[grp_id];

					if (group.size() == eSoundGroup_ELEMENT_MAX) // ERROR group limit
					{                                            // .
						info.result = eSoundError_MaxGroup;      // .
						continue;                                // .
					}                                            // .
					
					{
						if (info.linked_GID != nullptr)
						{
							uint64_gid LGID = (*info.linked_GID);
							err = LoadLinkedPRIV((uint32)_SoundID(LGID), snd_id);
						}
						else // original
						{
							if (info.file == nullptr)
							{
								if (!gamePathSz || !(info.filename.size())) // ERROR invalid filepath
									 err = eSoundError_BadPath;
								else err = LoadPRIV(gamePath, info.filename.c_str(), nullptr, snd_id);
							}
							else // from memory
							{
								err = LoadPRIV(nullptr, info.filename.c_str(), info.file, snd_id);
							}
						}

						if (err == eSoundError_TRUE)
						{
							uint32 group_size     = (uint32) group.size();
							uint32 group_capacity = (uint32) group.capacity();

							GID.element = group_size;     // данные будут в конце списка
							GID.group   = info.GID.group; // номер группы, указанный пользователем
							info.GID    = GID;            // .

							if (group_size + 1 >= group_capacity)                     // резервировать места
								group.reserve(group_capacity + eSoundGroup_CAPACITY); // .

							group.resize(group_size + 1); // поправляем размер группы
							group [group_size] = snd_id;  // сохраняем в ячейку у группы настоящий ID
							snd [snd_id]->GID = GID;      // для обратной связи
							snd [snd_id]->objname = info.objname;
						}
						else
						{
							wprintf(L"\nERROR 0x%x load listed sound \"%s\"", err, info.filename.c_str());
						}

						info.result = err;
					}
					// next i
				}
			}		

			return hRes;
		}

		//>> [TODO] Создание пустого WAV источника (болванка)
		error_t Load(const SNDDESC	* snddesc, float time, uint32 target_group, uint64 & OUT_GID)
		{
			glock safe_lock (m_load_delete);
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			// ... 

			return hRes;
		}

	public:
		//>> Меняет группу указанного GID :: изменяет GID
		error_t ChangeGroup(uint64 & GID, uint32 target_group)
		{
			glock safe_lock (m_load_delete); // защита на время манипуляций со списками snd[] и snd_g[]
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (target_group >= eSoundGroup_USER_MAX) // ERROR bad group ID (LIMIT REACHED)
				return eSoundError_MaxGroupID;        // .

			{
				if (!snd_count) return hRes = eSoundError_NoneFiles;              // ERROR none files
				if (eSoundError_TRUE != (hRes = CheckSoundID(GID))) return hRes;  // ERROR bad sound id

				uint64_gid _GID   = GID;
				uint32     grp_id = target_group + eSoundGroup_USER;
				uint32     snd_id = _SoundID(_GID);
			
				if ((grp_id + 1) > snd_g.size()) // создаём группу, если нет
					snd_g.SetCreate(grp_id + 1); // .

				auto & group = *snd_g[grp_id];

				if (group.size() == eSoundGroup_ELEMENT_MAX) // ERROR group limit
					return hRes = eSoundError_MaxGroup;      // .

				// обязательно завершаем т.к. меняется GID звука
				ControlInterrupt(_GID, true);

				_SoundID(_GID) = MISSING; // сброс в старой ячейке

				{
					uint32 group_size     = (uint32) group.size();
					uint32 group_capacity = (uint32) group.capacity();

					_GID.element = group_size;   // данные будут в конце списка
					_GID.group   = target_group; // номер группы, указанный пользователем
					 GID         = _GID;         // .

					if (group_size + 1 >= group_capacity)                     // резервировать места
						group.reserve(group_capacity + eSoundGroup_CAPACITY); // .

					group.resize(group_size + 1); // поправляем размер группы
					group[group_size] = snd_id;   // сохраняем в ячейку у группы настоящий ID
					snd[snd_id]->GID = _GID;      // для обратной связи
				}

				// safe_lock unlock()
			}

			return hRes;
		}

		//>> [TODO] Меняет группы по списку
		error_t ChangeGroup()
		{
			glock safe_lock (m_load_delete);
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			// ...

			return hRes;
		}

	public:
		//>> Устанавливает формальное имя объекта звука
		error_t FormalNameSet(uint64_gid GID, wstring & object_name)
		{
			glock control_guard(m_control);

			error_t hRes = eSoundError_TRUE;

			if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;

			snd[_SoundID(GID)]->objname = object_name;

			return hRes;
		};
		
		//>> Сообщает формальное имя объекта звука
		error_t FormalNameGet(uint64_gid GID, wstring & object_name)
		{
			glock control_guard(m_control);

			error_t hRes = eSoundError_TRUE;

			if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;

			object_name = snd[_SoundID(GID)]->objname;
		};
		
		//>> Ищет звук по формальному имени (первый соответствующий)
		error_t FormalNameFind(uint64_gid & GID, wstring & object_name)
		{
			glock control_guard(m_control);

			error_t hRes = eSoundError_FALSE;

			for (auto sound : snd)
			{
				if (!sound->objname.compare(object_name)) // при совпадении имён
				{
					GID  = sound->GID;
					hRes = eSoundError_TRUE;
					break;
				}
			}

			return hRes;
		};
		
		//>> Ищет звук по формальному имени (все соответствующие)
		error_t FormalNameFind(vector <uint64_gid> & GID, uint32 GID_count, wstring & object_name)
		{
			glock control_guard(m_control);

			error_t hRes = eSoundError_FALSE;

			uint32 num_found    = 0;
			uint32 num_prealloc = (uint32) GID.size();

			for (auto sound : snd)
			{
				if (!sound->objname.compare(object_name)) // при совпадении имён
				{
					num_found++;
					hRes = eSoundError_TRUE;

					if (num_found >= num_prealloc)
					{
						num_prealloc += 4;
						GID.resize(num_prealloc);

						num_prealloc = (uint32) GID.size();
						if (num_found >= num_prealloc)
						{
							num_found = 0;
							hRes = eSoundError_BadAlloc;
							break;
						}
					}
					
					GID[num_found-1] = sound->GID;
				}
			}

			GID_count = num_found;

			return hRes;
		};

	//////////////// Data delete block ////////////////

	public:
		//>> Удаляет звуковые данные (полная очистка)
		error_t Delete()
		{
			// удалить всё, что от пользователя, остальное оставить

			error_t hRes = eSoundError_TRUE;

			{
				glock safe_lock (m_load_delete); // защита на время манипуляций со списками snd[] и snd_g[]

				// ControlInterrupt() берёт <m_control> блок
				ControlInterrupt(SOUND_ALL, true);

				uint32 mixer_id; // ID данных микшера (которые оставляем)

				// теперь можно самим захватить <m_control> блок
				{
					glock control_guard (m_control); // блок от других Control-функций

					{
						glock lk_thread (mixerdata.THR.m_thread);

						if (mixerdata.THR.thread_snd)
						{
							auto & mixer_group = *snd_g[eSoundGroup_MIX];
								   mixer_id    = mixer_group[0];

							if (mixer_id != 0) // переставить в начало списка
							{
								snd.Swap(mixer_id, 0);
								mixer_id       = 0;
								mixer_group[0] = mixer_id;
							}
						}
						else
							mixer_id = MISSING;
					}

					if (_ISMISS(mixer_id))
						 mixer_id  = 0; // удаление всего с 0
					else mixer_id += 1; // удаление со след. после микшера

					snd_g.Delete(true, eSoundGroup_USER);
					snd.Delete(true, mixer_id);

				//	if (mixer_id == 0)
				//		 snd_count = 0;        // ничего нет
				//	else snd_count = mixer_id; // есть микшер

					snd_count = (uint32) snd.size();
				}
			}

			return hRes;
		}

		//>> Удаляет звуковые данные (указанный GID)
		error_t DeleteOne(uint64_gid GID, eSoundDelete deltype)
		{
			glock safe_lock (m_load_delete); // защита на время манипуляций со списками snd[] и snd_g[]
			ulock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (!snd_count) return hRes = eSoundError_NoneFiles;              // ERROR none files
			if (eSoundError_TRUE != (hRes = CheckSoundID(GID))) return hRes;  // ERROR bad sound id

			//////////////////////////////////////

			    uint64  ID      = _SoundID(GID);
			    uint64  ID_last = snd_count - 1;
			uint64_gid GID_last = snd[ID_last]->GID;
			
			if (snd[ID] != nullptr)
			{
				control_guard.unlock();      // ControlInterrupt() берёт <m_control> блок
				ControlInterrupt(GID, true); // .
				control_guard.lock();        // .

				if (deltype == eSoundDelete_simple) // {VAR.1} сброс данных ячейки
					snd[ID]->Close();
				else                                // {VAR.2,3} удаление ячейки
					snd.Delete(false, ID, ID + 1);
			}

			if (ID != ID_last)         // Пустое в конец
				snd.Swap(ID, ID_last); // .

			snd_count--;

			_SoundID(GID)      = MISSING;     // Поправка данных у группы
			_SoundID(GID_last) = (uint32) ID; // .		

			if (deltype == eSoundDelete_reconstruct) // {VAR.3} уменьшение длины массива
			{
				snd.Delete(true, snd_count);
			}

			return hRes;
		}

		//>> Удаляет звуковые данные (указанную группу)
		error_t DeleteGroup(uint32 target_group, eSoundDelete deltype)
		{
			glock safe_lock (m_load_delete); // защита на время манипуляций со списками snd[] и snd_g[]
			ulock control_guard (m_control);

			if (target_group >= eSoundGroup_USER_MAX) // ERROR bad group ID (LIMIT REACHED)
				return eSoundError_MaxGroupID;        // .

			error_t hRes = eSoundError_TRUE;

			if (!snd_count)                           // ERROR none files
				return hRes = eSoundError_NoneFiles;  // .

			uint32 group_id = target_group + eSoundGroup_USER;

			if (group_id >= snd_g.size())          // ERROR bad ID of group
				return hRes = eSoundError_BadID_G; // .

			//////////////////////////////////////

			control_guard.unlock();                    // ControlInterruptGroup() берёт <m_control> блок
			ControlInterruptGroup(target_group, true); // .
			control_guard.lock();                      // .

			auto & group = *snd_g[group_id];
			for (auto & ID : group)
			{
				if (__ISMISS(ID)) continue; // already removed

				    uint64  ID_last = snd_count - 1;
				uint64_gid GID_last = snd[ID_last]->GID;

				if ( (decltype(snd_count)) ID >= snd_count ) // ERROR bad ID
				{
					ID = MISSING;
					hRes = eSoundError_Corrupted;
					continue;
				}

				if (snd[ID] != nullptr)
				{
					if (deltype == eSoundDelete_simple) // {VAR.1} сброс данных ячейки
						snd[ID]->Close();
					else                                // {VAR.2,3} удаление ячейки
						snd.Delete(false, ID, ID + 1);
				}

				if (ID != ID_last)         // Пустое в конец
					snd.Swap(ID, ID_last); // .

				snd_count--;

				_SoundID(GID_last) = ID; // Поправка данных у группы
				ID = MISSING;            // .
			}

			// Сброс массива группы
			group.erase(group.begin(), group.end());

			if (deltype == eSoundDelete_reconstruct) // {VAR.3} уменьшение длины массива
			{
				snd.Delete(true, snd_count);
			}

			return hRes;
		}

	//////////////// Playback / Record block ////////////////

	private:
		//>> Воспроизведение :: эта функция выполняется в другом потоке
		error_t Play(SNDCHUNK * list, bool async)
		{
			SNDDATA & sound = *reinterpret_cast<SNDDATA*>(list->actual_sound);
			error_t   hRes  = MISSING;

			if (!async)
			{
				ulock guard(sound.m_playblock, std::try_to_lock);
				if (guard.owns_lock())
				{
					if (!sound._ATM_LD(a_playblock))  // захват <a_playblock> не требуется, <m_playblock> остаётся всё время захваченным
					{
						//sound.a_playblock = true;
						hRes = RunPlayback(list);
						//sound.a_playblock = false;
					}
					else // заблокировано RunInMixer()
						hRes = eSoundError_Normal_None;
				}
				else // заблокировано Play()
					hRes = eSoundError_Normal_None;
			}
			else
			{
				hRes = RunPlayback(list);
			}

			///////////// ОБРАБОТКА ВЫХОДА /////////////

			{
				glock stop_others_working_with_list (m_listRenderID);

				// Поменяемся местами с последним из занятых
				uint64 pos_self, pos_last;

				for (pos_self = 0; pos_self < listRenderID_count; pos_self++)
				{
					// Найдём себя в списке (нас могли переставить)
					if (list == listRenderID[pos_self])
						break;
				}

				if (pos_self == listRenderID_count) // debug check
					_MBM(L"\nlistRenderID_count CRITICAL ERROR");

				pos_last = listRenderID_count - 1;

				if (pos_self != pos_last)
					listRenderID.Swap(pos_self, pos_last);

				listRenderID_count--; // уменьшим количество занятых

				//printf("\nEND : snd_id %i, pl %i, listRenderID_count %i, self/last %i/%i",
				//	id, snd[id]->players - 1, listRenderID_count, pos_self, pos_last);

				if ((--(sound.players)) == 0) // сброс статуса прерывания, если больше никого нет
				{
					sound._ATM_0(a_interrupt);
				}

				list->_quick_close();

				if (hRes >= eSoundError__NORMAL_END || hRes <= eSoundError__NORMAL_BEGIN)
					wprintf(L"\nSound GID=0x%X (%s) exited with code 0x%X", 
						sound.GID, sound.filename.c_str(), hRes);
			}

			return hRes;
		}

		//>> Захват :: эта функция выполняется в другом потоке
		error_t Record(uint64 id)
		{
			uint32 hRes = RunRecord(id);

			return hRes;
		}

	public:
		//>> Воспроизведение : GID звука, eSoundRunSet настройки запуска <eSoundRunSettings>
		error_t RunThreadPlay(uint64_gid GID, uint32 eSoundRunSet)
		{
			rglock guard (m_init_close); // теперь <m_run_play> не нужен

		//	ulock running_guard (m_run_play, std::try_to_lock);
		//	if (!running_guard.owns_lock())       // заблокировано из DefaulDeviceChanged()
		//		return eSoundError_DeviceChanged; // .
		
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (!isInit) return hRes = eSoundError_NoInit;                     // ERROR нет инициализации
			if (!actual_render_interfaces) return hRes = eSoundError_NoInit_R; // ERROR нет render-интерфейсов
			if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;     // ERROR некорректный ID

			uint32 id = _SoundID(GID); // реальный ID

			hRes = snd[id]->baddat_err;                // некая запомненная ошибка
			if (hRes != eSoundError_TRUE) return hRes; // .

			// запуск через общее смешивание
			if (eSoundRunSet & eSoundRunMix)
			{
				if (mixerdata.state_runned)
					 hRes = RunInMixer(GID, eSoundRunSet);
				else // микшер не включен
					 hRes = eSoundError_NoMixer;

				return hRes;
			}
			// else : стандартный запуск далее

			uint64 T = playpool.GetTasks();
			uint64 W = playpool.GetWorkers();			

			if (T > W) _MBM(L"DEBUG WARNING : T > W : unexpected");

			if (T >= W)
			{
				if (W < ALLOWED_NUM_OF_COMMON_INTERFACES) // можно создать ещё поток
				{
					playpool.WorkersAdd(1);
					//++W;
				}
				else // нет больше мест - попробуем запустить в микшер
				{
					if (mixerdata.state_runned)
					{
						//eSoundRunSet |= eSoundRunMix;
						hRes = RunInMixer(GID, eSoundRunSet);
					}
					else // микшер не включен (???)
						hRes = eSoundError_NoMixer;

					return hRes;
				}
			}

			SNDCHUNK * list = nullptr;

			{
				glock stop_others_working_with_list (m_listRenderID);

				// 1. Прерывание касается только уже проигрываемых, новые запуски не буду трогать
				// 2  Структура ожидания передаётся зовущему, у каждого потока он свой, значит проблем со счетчиком на ожидание нет
				// 3. DefaulDeviceChanged() имеет собственный блок a_RenderChanged от новых
				//
				//if (snd[id]->_ATM_LD(a_interrupt))
				//	return false;

				if (listRenderID_count == ALLOWED_NUM_OF_COMMON_INTERFACES) // DEBUG этого не может быть
				{
					_MBM(L"DEBUG : listRenderID_count ERROR - MAX LIMIT");
					return hRes = eSoundError_FALSE;
				}

				list = listRenderID[listRenderID_count]; // получить указатель
				list->loop            = _BOOL(eSoundRunSet & eSoundRunLoop);        // сохранить настройки вызова
				list->stop_at_start   = _BOOL(eSoundRunSet & eSoundRunStopAtStart); // .
				list->stop_at_end     = _BOOL(eSoundRunSet & eSoundRunStopAtEnd);   // .
				list->GID             = GID;                                        // .
				list->actual_sound    = snd[_SoundID(GID)];                         // .
				list->mixer_p         = nullptr;                                    // .
				list->mixer           = false;                                      // .
				listRenderID_count++;                    // увеличить число занятых
		
				list->Init(); // подготовка

				snd[id]->players++; // увеличить счетчик проигрывателей

				//printf("\nRUNNING : snd_id %i, pl %i, listRenderID_count %i",
				//	id, snd[id]->players, listRenderID_count);
			}

			// очередь в пуле очищается по первому вышедшему --> лучше отделить долгие задачи в другую очередь
			eTaskPriority eTP = _BOOL(eSoundRunSet & eSoundRunLoop) ? SOUND_QUEUE_LOOP : SOUND_QUEUE_NOT_LOOP ;

			playpool(eTP, pPlayFunc, this, list, _BOOL(eSoundRunSet & eSoundRunAsync));

			return hRes;
		}

/*	private:
		//>> Захват
		void RunThreadRecord(uint64_gid id)
		{
			uint64 T = recpool.GetTasks();
			uint64 W = recpool.GetWorkers();

			if (T >= W && W < actual_capture_threads) recpool.WorkersAdd(1);

		//	cap[id]->_RLX_ADD(a_players, 1);

			recpool(pRecFunc, this, id);
		}

	public:
		//>> Захват
		void RunThreadRecord()
		{
			RunThreadRecord(0);
		} //*/

	//////////////// Volume control block ////////////////

	public:
		//>> Устанавливает общую громкость [+0.f..+1.f]
		error_t SetVolumeMaster(float volume)
		{
			glock control_guard (m_control);

			volume = _CLAMP(volume, 0.f, 1.f);

			// сообщаем микшеру
			ControlMixerSetMasterVolumePRIV(volume);

			// изменим для отдельных потоков
			_ATM_ST(a_master_volume, volume);

			{
				glock stop_others_working_with_list (m_listRenderID);

				for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
				{
					listRenderID[i]->caller->Awake(eSoundCallReason_MasterVolume);
				}
			}

			return eSoundError_TRUE;
		}

		//>> Сообщает текущую общую громкость
		error_t GetVolumeMaster(float & volume)
		{
			glock control_guard (m_control);

			volume = _ATM_LD(a_master_volume);

			return eSoundError_TRUE;
		}

		//>> Устанавливает множитель громкости [+0.f..+1.f] :: GID = SOUND_ALL для всех текущих
		error_t SetVolume(uint64_gid GID, float volume)
		{
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			const bool all = _ISMISS(GID._u64);

			if (!all) if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;

			volume = _CLAMP(volume, 0.f, 1.f);

			if (!all)
			{	
				uint64 sndID = _SoundID(GID);

				// сообщаем микшеру
				ControlMixerSetVolumePRIV(sndID, volume, false);

				SNDDATA & sound = *snd[sndID];              // изменим для отдельных потоков
				sound._ATM_ST(a_volume_multiplier, volume); // .

				{
					glock stop_others_working_with_list (m_listRenderID);

					uint32 p     = 0;             // счетчик потоков
					uint32 p_max = sound.players; // всего потоков играющих этот звук сейчас активно

					if (p_max)
					{
						for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
						if (GID == listRenderID[i]->GID)
						{
							listRenderID[i]->caller->Awake(eSoundCallReason_VolumeMultiplier); // позвали
				
							if ((++p) == p_max) break; // сразу выйдем, если обзвонили всех
						}
					}
				}
			}
			else // во всех текущих
			{
				// сообщаем микшеру
				ControlMixerSetVolumePRIV(SOUND_ALL, volume, false);

				{
					glock stop_others_working_with_list (m_listRenderID);

				//	if (!listRenderID_count) return hRes; // none threads

					for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
					{
						reinterpret_cast <SNDDATA*> // изменим
							(listRenderID[i]->actual_sound)->_ATM_ST(a_volume_multiplier, volume); 

						listRenderID[i]->caller->Awake(eSoundCallReason_VolumeMultiplier); // позвали
					}
				}
			}

			return hRes;
		}

		//>> Сообщает множитель громкости [+0.f..+1.f]
		error_t GetVolume(uint64_gid GID, float & volume)
		{
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;

			volume = snd[_SoundID(GID)]->_ATM_LD(a_volume_multiplier);

			return hRes;
		}

		//>> Устанавливает множитель громкости [+0.f..+1.f]
		error_t SetVolumeGroup(uint32 target_group, float volume)
		{
			glock control_guard (m_control);

			if (target_group >= eSoundGroup_USER_MAX) // ERROR bad group ID (LIMIT REACHED)
				return eSoundError_MaxGroupID;        // .

			uint32 group_id = target_group + eSoundGroup_USER; // fix
		
			if (group_id >= snd_g.size())   // ERROR bad ID of group
				return eSoundError_BadID_G; // .

			////////////////////////////////

			error_t hRes = eSoundError_TRUE;
			auto & group = *snd_g[group_id];

			if (group.size())
			{
				volume = _CLAMP(volume, 0.f, 1.f);

				// сообщаем микшеру
				ControlMixerSetVolumePRIV(target_group, volume, true);

				for (auto ID : group)
				{
					if (_ISMISS(ID)) continue;
					snd[ID]->_ATM_ST(a_volume_multiplier, volume);
				}

				{
					glock stop_others_working_with_list (m_listRenderID);

					for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
					if (target_group == listRenderID[i]->GID.group)
					{
						listRenderID[i]->caller->Awake(eSoundCallReason_VolumeMultiplier); // позвали
					}
				}
			}

			return hRes;
		}

	public:
		//>> Сбрасывает множители громкости по каналам (1.f по умолчанию)
		error_t SetVolumeChannelsDefault(uint64_gid GID)
		{
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;

			uint32 id = (uint32)_SoundID(GID);
			snd[id]->ResetChannelsVolume();
			ControlMixerSetVolumeChannelsPRIV(id, nullptr, 0);
			SetVolumeChannelsCALL(GID, id);

			return hRes;
		}

		//>> Устанавливает множители громкости по каналам (8 каналов макс.) - все каналы одинаковый <volume>
		error_t SetVolumeChannels(uint64_gid GID, float volume)
		{
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;

			uint32 id = (uint32)_SoundID(GID);
			snd[id]->SetChannelsVolume(volume);
			ControlMixerSetVolumeChannelsPRIV(id, volume, PCM_MAXCHANNELS);
			SetVolumeChannelsCALL(GID, id);

			return hRes;
		}

		//>> Устанавливает множители громкости по каналам (8 каналов макс.) - каждый канал своё значение из массива
		error_t SetVolumeChannels(uint64_gid GID, float * volume_PCM_MAXCHANNELS_floats)
		{
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;

			if (volume_PCM_MAXCHANNELS_floats != nullptr)
			{
				uint32 id = (uint32)_SoundID(GID);
				snd[id]->SetChannelsVolume(volume_PCM_MAXCHANNELS_floats);
				ControlMixerSetVolumeChannelsPRIV(id, volume_PCM_MAXCHANNELS_floats, PCM_MAXCHANNELS);
				SetVolumeChannelsCALL(GID, id);
			}
			else hRes = eSoundError_FALSE;

			return hRes;
		}

		//>> Устанавливает множители громкости по каналам (left , right)
		error_t SetVolumeChannelsStereo(uint64_gid GID, float left, float right)
		{
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;

			float volume[] = { left, right };
			uint32 id = (uint32)_SoundID(GID);

			snd[id]->SetChannelsVolumeStereo(left, right);			
			ControlMixerSetVolumeChannelsPRIV(id, volume, 2);
			SetVolumeChannelsCALL(GID, id);

			return hRes;
		}

		//>> Устанавливает множители громкости по каналам (left = right)
		error_t SetVolumeChannelsMono(uint64_gid GID, float mono)
		{
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;

			float volume[] = { mono, mono };
			uint32 id = (uint32)_SoundID(GID);

			snd[id]->SetChannelsVolumeMono(mono);
			ControlMixerSetVolumeChannelsPRIV(id, volume, 2);
			SetVolumeChannelsCALL(GID, id);

			return hRes;
		}

		//>> Сообщает множители громкости по каналам (8 каналов макс.)
		error_t GetVolumeChannels(uint64_gid GID, float * volume_PCM_MAXCHANNELS_floats)
		{
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;

			snd[_SoundID(GID)]->GetChannelsVolume(volume_PCM_MAXCHANNELS_floats);

			return hRes;
		}

		//>> Сообщает множители громкости по каналам (стерео)
		error_t GetVolumeChannelsStereo(uint64_gid GID, float & left, float & right)
		{
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;

			snd[_SoundID(GID)]->GetChannelsVolumeStereo(left, right);

			return hRes;
		}

		//>> Сообщает множители громкости по каналам (моно)
		error_t GetVolumeChannelsMono(uint64_gid GID, float & mono)
		{
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;

			snd[_SoundID(GID)]->GetChannelsVolumeMono(mono);

			return hRes;
		}

	private:
		//>> Обзвон при изменении настроек громкости каналов
		void SetVolumeChannelsCALL(uint64_gid GID, uint32 id)
		{
			glock stop_others_working_with_list (m_listRenderID);

			uint32 p     = 0;                // счетчик потоков
			uint32 p_max = snd[id]->players; // всего потоков играющих этот звук сейчас активно

			if (!p_max) return; // none threads

			for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
			if (GID == listRenderID[i]->GID)
			{
				listRenderID[i]->caller->Awake(eSoundCallReason_ChannelVolume); // позвали

				if ((++p) == p_max) break; // сразу выйдем, если обзвонили всех
			}
		}

	//////////////// Support block ////////////////

	private:
		//>> Проверка корректности ID
		error_t CheckSoundID(uint64_gid GID)
		{
			// Поскольку DefaulDeviceChanged() отправляет SOUND_ALL в качестве GID
			// эта проверка не используется, а значит нет необходимости захватывать <m_load_delete>

			error_t ret = eSoundError_TRUE;

			if ( GID.group + eSoundGroup_USER >= snd_g.size() ) // ERROR bad ID of group
				ret = eSoundError_BadID_G;                      // .
			else			
			if ( GID.element >= (*snd_g[GID.group + eSoundGroup_USER]).size() ) // ERROR bad ID of group's element
				ret = eSoundError_BadID_GE;                                     // .
			else
			if ( ((decltype(snd_count)) _SoundID(GID)) >= snd_count ) // ERROR bad target sound ID
				ret = eSoundError_BadID_E;                            // .

			return ret;
		}

		//>> Проверка наличия звуковых данных
		error_t CheckSoundFile(uint64_gid GID)
		{
			error_t ret = eSoundError_TRUE;

			uint64 ID = _SoundID(GID);

			if (snd[ID] == nullptr)
				 ret = eSoundError_BadSndPTR;
			else ret = snd[ID]->CheckFile();

			return ret;
		}

		//>> Проверка корректности ID и наличия звуковых данных
		error_t CheckSound(uint64_gid GID)
		{
			error_t ret = eSoundError_TRUE;

			if (eSoundError_TRUE == (ret = CheckSoundID(GID)))
			{
				ret = CheckSoundFile(GID);
			}

			return ret;
		}

	public:
		//>> Проверка корректности ID и наличия звуковых данных
		error_t IsValid(uint64_gid GID)
		{
			glock control_guard (m_control);

			return CheckSound(GID);
		}

		//>> Сообщает длительность
		error_t GetDuration(uint64_gid GID, float & time)
		{
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;

			time = snd[_SoundID(GID)]->duration;

			return hRes;
		}

		//>> Сообщает макс. число потоков воспроизведения (не микшер!) (корректно после инициализации)
		error_t GetNumThreadsMax(uint32 & threads_max)
		{
			// glock control_guard(m_control);

			{
				rglock guard (m_init_close);

				if (isInit)
					 threads_max = ALLOWED_NUM_OF_COMMON_INTERFACES ;
				else threads_max = 0;
			}

			return eSoundError_TRUE;
		}

		//>> Сообщает текущее число потоков воспроизведения в работе (не микшер)
		error_t GetNumThreadsInUse(uint32 & threads_current)
		{
			// glock control_guard(m_control);

			// одному потоку соответствует одна задача
			// задач не запускается больше, чем потоков

			threads_current = (uint32) playpool.GetTasks();

			return eSoundError_TRUE;
		}

		//>> Сообщает текущее число потоков воспроизведения в работе (не микшер) - alt. вариант для DEBUG тестов
		error_t _GetNumThreadsInUse_alt(uint32 & tasks_current_debug)
		{
			// glock control_guard(m_control);

			// Для проверок  listRenderID_count == playpool.GetTasks()  в тестах
			{
				glock stop_others_working_with_list (m_listRenderID);

				tasks_current_debug = listRenderID_count;
			}

			return eSoundError_TRUE;
		}

		//>> Сообщает текущее число задач воспроизведения в микшере
		error_t GetNumTasksInMixer(uint32 & tasks_current)
		{		
			// glock control_guard (m_control);

			{
				glock stop_mixer (mixerdata.m_mixer);

				tasks_current = mixerdata.task_num;
			}
			
			return eSoundError_TRUE;
		}

		//>> Выдаёт сводку статуса (по всем запускам данного ID)
		error_t GetStateSound(vector <CSoundState*> & blank, uint32 & num_tasks, uint64_gid GID)
		{
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			if (eSoundError_TRUE != (hRes = CheckSound(GID))) return hRes;

			SNDDATA & sound = *snd[_SoundID(GID)];

			{
				glock stop_others_working_with_list (m_listRenderID);
				glock stop_mixer (mixerdata.m_mixer);

				uint32 p     = 0;
				uint32 p_max = sound.players + sound.players_mixer;

				num_tasks = p_max;

				if (!p_max) return hRes;

				if (p_max > blank.size())
				{
					blank.resize(p_max);

					if (p_max > blank.size())
						return hRes = eSoundError_BadAlloc;
				}

				if (sound.players)
				for (uint32 i = 0; i < listRenderID_count; i++)
				if (GID == listRenderID[i]->GID)
				{
					blank [p] = listRenderID[i]->caller->status.UpdateInfoGet();

					if ((++p) == sound.players) break;
				}

				if (sound.players_mixer)
				for (uint32 i = 0; i < mixerdata.task_num; i++)
				if (GID == mixerdata.task[i]->GID)
				{
					blank [p] = mixerdata.task[i]->UpdateInfoGet();

					if ((++p) == p_max) break;
				}

				num_tasks = p;
			}

			return hRes;
		}

		//>> Выдаёт сводку статуса (по всем запускам всех ID)
		error_t GetStateSound(vector <CSoundState*> & blank, uint32 & num_tasks)
		{
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			{
				glock stop_others_working_with_list (m_listRenderID);
				glock stop_mixer (mixerdata.m_mixer);

				uint32 p     = 0;
				uint32 p_max = listRenderID_count + mixerdata.task_num;

				num_tasks = p_max;

				if (!p_max) return hRes;

				if (p_max > blank.size())
				{
					blank.resize(p_max);

					if (p_max > blank.size())
						return hRes = eSoundError_BadAlloc;
				}

				//if (listRenderID_count)
				for (uint32 i = 0; i < listRenderID_count; i++)
				{
					blank [p++] = listRenderID[i]->caller->status.UpdateInfoGet();
				}

				//if (mixerdata.task_num)
				for (uint32 i = 0; i < mixerdata.task_num; i++)
				{
					blank [p++] = mixerdata.task[i]->UpdateInfoGet();
				}
			}

			return hRes;
		}

		//>> Выдаёт сводку статуса (по всем запускам данной группы)
		error_t GetStateSoundGroup(vector <CSoundState*> & blank, uint32 & num_tasks, uint32 target_group)
		{
			glock control_guard (m_control);

			if (target_group >= eSoundGroup_USER_MAX) // ERROR bad group ID (LIMIT REACHED)
				return eSoundError_MaxGroupID;        // .

			uint32 group_id = target_group + eSoundGroup_USER; // fix
		
			if (group_id >= snd_g.size())   // ERROR bad ID of group
				return eSoundError_BadID_G; // .

			////////////////////////////////

			error_t hRes = eSoundError_TRUE;
			auto & group = *snd_g[group_id];

			if (group.size())
			{
				glock stop_others_working_with_list (m_listRenderID);

				uint32 p               = 0;
				uint32 p_max           = 0;
				uint32 p_max_mixer     = 0;
				uint32 p_max_not_mixer = 0;

				for (uint32 i = 0; i < listRenderID_count; i++)
				if (target_group == listRenderID[i]->GID.group)
					p_max_not_mixer++;

				for (uint32 i = 0; i < mixerdata.task_num; i++)
				if (target_group == mixerdata.task[i]->GID.group)
					p_max_mixer++;

				p_max = p_max_mixer + p_max_not_mixer;
				num_tasks = p_max;

				if (p_max)
				{
					if (p_max > blank.size())
					{
						blank.resize(p_max);

						if (p_max > blank.size())
							return hRes = eSoundError_BadAlloc;
					}

					if (p_max_not_mixer)
					for (uint32 i = 0; i < listRenderID_count; i++) 
					if (target_group == listRenderID[i]->GID.group)
					{
						blank [p] = listRenderID[i]->caller->status.UpdateInfoGet();

						if ((++p) == p_max_not_mixer) break;
					}

					if (p_max_mixer)
					for (uint32 i = 0; i < mixerdata.task_num; i++) 
					if (target_group == mixerdata.task[i]->GID.group)
					{
						blank [p] = mixerdata.task[i]->UpdateInfoGet();

						if ((++p) == p_max) break;
					}

					num_tasks = p;
				}
			}
			else // p_max == 0
			{
				num_tasks = 0;
			}

			return hRes;
		}

		//>> [TODO] Выдаёт общую сводку касательно количества загруженных файлов и количества групп
		error_t GetState(void * out_data)
		{
			glock control_guard (m_control);

			error_t hRes = eSoundError_TRUE;

			// ...

			return hRes;
		}

	//////////////// Threads control block ////////////////

	private:
		error_t ControlInterruptPRIV(uint64 id, bool wait, bool group)
		{
			error_t hRes = eSoundError_TRUE;

			const bool all = _ISMISS(id);

			// сначала в микшере
			error_t hRes_mixer = ControlMixerTaskInterruptPRIV(id, group, wait);

			if (!wait) // don't wait
			{
				if (!all) // don't wait : only one id
				{
					////////////////////////////////////////////////////////////////////////
					if (group) // don't wait : group
					{
						glock stop_others_working_with_list (m_listRenderID);

						uint32 target_group = (uint32) id;

						for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
						if (target_group == listRenderID[i]->GID.group)
						{
							SNDDATA & sound = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);

							sound._ATM_1(a_interrupt);                                  // установили запрос
							listRenderID[i]->caller->Awake(eSoundCallReason_Interrupt); // позвали

							// сбросим паузу
							{
								glock stop_while_working_with_pause (sound.m_pause);
								sound.pause = false;
								sound._CV_ALL(cv_pause);
							}
						}
					}
					////////////////////////////////////////////////////////////////////////
					else // don't wait : not group
					{				
						glock stop_others_working_with_list (m_listRenderID);

						SNDDATA & sound = *snd[id];
						uint64    GID   = sound.GID;

						uint32 p     = 0;			  // счетчик потоков
						uint32 p_max = sound.players; // всего потоков играющих этот звук сейчас активно

						if (p_max)
						{
							sound._ATM_1(a_interrupt); // установили запрос

							for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
							if (GID == listRenderID[i]->GID)
							{
								listRenderID[i]->caller->Awake(eSoundCallReason_Interrupt);		
								if ((++p) == p_max) break; // сразу выйдем, если обзвонили всех
							}
							// сбросим паузу
							{
								glock stop_while_working_with_pause (sound.m_pause);
								sound.pause = false;
								sound._CV_ALL(cv_pause);
							}
						}
					}
				}
				////////////////////////////////////////////////////////////////////////
				else // don't wait : everybody
				{
					glock stop_others_working_with_list (m_listRenderID);

					for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
					{
						SNDDATA & sound = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);

						sound._ATM_1(a_interrupt);                                  // установили запрос
						listRenderID[i]->caller->Awake(eSoundCallReason_Interrupt); // позвали

						// сбросим паузу
						{
							glock stop_while_working_with_pause (sound.m_pause);
							sound.pause = false;
							sound._CV_ALL(cv_pause);
						}
					}
				}
			}
			else // wait
			{
				SNDWAIT   waiter;
				SNDWAIT * waiter_ptr = & waiter;

				if (!all) // wait : only one id
				{
					////////////////////////////////////////////////////////////////////////
					if (group) // wait : group
					{
						glock stop_others_working_with_list (m_listRenderID);

						uint32 target_group = (uint32)id;
						uint32 p = 0; // счетчик потоков

						for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
						if (target_group == listRenderID[i]->GID.group)
						{
							SNDDATA & sound  = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);
							auto    & caller = *listRenderID[i]->caller;

							sound._ATM_1(a_interrupt); // установили запрос

							caller.wait.Store(waiter_ptr, eSoundCallReason_Interrupt);
							caller.Awake(eSoundCallReason_Interrupt);
							// сбросим паузу
							{
								glock stop_while_working_with_pause (sound.m_pause);
								sound.pause = false;
								sound._CV_ALL(cv_pause);
							}

							p++;
						}

						waiter.target = p;
					}
					////////////////////////////////////////////////////////////////////////
					else // wait : not group
					{
						glock stop_others_working_with_list (m_listRenderID);

						SNDDATA & sound = *snd[id];
						uint64    GID   = sound.GID;

						uint32 p     = 0;             // счетчик потоков
						uint32 p_max = sound.players; // всего потоков играющих этот звук сейчас активно

						if (p_max)
						{
							sound._ATM_1(a_interrupt); // установили запрос

							for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
							if (GID == listRenderID[i]->GID)
							{
								auto & caller = *listRenderID[i]->caller;

								caller.wait.Store(waiter_ptr, eSoundCallReason_Interrupt);
								caller.Awake(eSoundCallReason_Interrupt);

								if ((++p) == p_max) break; // сразу выйдем, если обзвонили всех
							}
							// сбросим паузу
							{
								glock stop_while_working_with_pause (sound.m_pause);
								sound.pause = false;
								sound._CV_ALL(cv_pause);
							}
						}

						waiter.target = p;
					}
				}
				////////////////////////////////////////////////////////////////////////
				else // wait : everybody
				{
					glock stop_others_working_with_list (m_listRenderID);

					for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
					{
						SNDDATA & sound  = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);
						auto    & caller = *listRenderID[i]->caller;

						sound._ATM_1(a_interrupt); // установили запрос

						caller.wait.Store(waiter_ptr, eSoundCallReason_Interrupt);
						caller.Awake(eSoundCallReason_Interrupt);
						// сбросим паузу
						{
							glock stop_while_working_with_pause (sound.m_pause);
							sound.pause = false;
							sound._CV_ALL(cv_pause);
						}
					}

					waiter.target = listRenderID_count;
				}

				///// ВСЕ БЛОКИРОВКИ СНЯТЫ /////

				if (waiter.target)
					ControlWait(waiter_ptr, waiter.target);
			}

			return hRes;
		}
		error_t ControlPausePRIV(uint64 id, bool wait, bool group)
		{
			// сбросить возможные последствия от предыдущего Resume(), когда поток начал просыпаться, но ещё не вышел
			// если это последствия от Rewind() - делать проверку заказа перемотки <status.rewinded> в теле паузы для выхода
			// если выходит из-за Rewind(), а мы Pause() с ожиданием - будем ждать его после перемотки

			error_t hRes = eSoundError_TRUE;

			const bool all = _ISMISS(id);

			// сначала в микшере
			error_t hRes_mixer = ControlMixerTaskPausePRIV(id, group, false);
			T_TIME  time_mixer; // счетчик задержки при ожидании

			if (hRes_mixer == eSoundError_TRUE)
				time_mixer = _TIME;

			if (!wait) // don't wait
			{
				if (!all) // don't wait : only one id
				{	
					////////////////////////////////////////////////////////////////////////
					if (group) // don't wait : group
					{
						glock stop_others_working_with_list (m_listRenderID);

						uint32 target_group = (uint32)id;

						for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
						if (target_group == listRenderID[i]->GID.group)
						{
							SNDDATA & sound = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);
							auto    & caller = *listRenderID[i]->caller;

							// установим запрос
							{
								glock stop_while_working_with_pause (sound.m_pause);

								sound.pause = true; // открыли вход в паузу
								caller.status.awakened = false; // сбросили последствия Resume(), но не Rewind()							

								// Неважен статус т.к. без ожиданий
								//if (caller.status.in_pause)
								//{
								//	if (caller.status.rewinded) {}
								//	else {}
								//}
								//else {}
							}

							caller.Awake(eSoundCallReason_Pause);
						}
					}
					////////////////////////////////////////////////////////////////////////
					else // don't wait : not group
					{
						glock stop_others_working_with_list (m_listRenderID);

						SNDDATA & sound = *snd[id];
						uint64    GID   = sound.GID;

						uint32 p     = 0;             // счетчик потоков
						uint32 p_max = sound.players; // всего потоков играющих этот звук сейчас активно

						if (p_max)
						{
							glock stop_while_working_with_pause (sound.m_pause);

							sound.pause = true; // открыли вход в паузу

							for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
							if (GID == listRenderID[i]->GID)
							{
								auto & caller = *listRenderID[i]->caller;

								caller.status.awakened = false; // сбросить последствия Resume(), но не Rewind()
								caller.Awake(eSoundCallReason_Pause);

								// Неважен статус т.к. без ожиданий
								//if (caller.status.in_pause)
								//{
								//	if (caller.status.rewinded) {}
								//	else {}
								//}
								//else {}

								// сразу выйдем, если обзвонили всех
								if ((++p) == p_max) break;
							}
						}
					}
				}
				////////////////////////////////////////////////////////////////////////
				else // don't wait : everybody
				{
					glock stop_others_working_with_list (m_listRenderID);

					for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
					{
						SNDDATA & sound = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);
						auto    & caller = *listRenderID[i]->caller;

						// установим запрос
						{
							glock stop_while_working_with_pause (sound.m_pause);

							sound.pause = true; // открыли вход в паузу
							caller.status.awakened = false; // сбросили последствия Resume(), но не Rewind()							

							// Неважен статус т.к. без ожиданий
							//if (caller.status.in_pause)
							//{
							//	if (caller.status.rewinded) {}
							//	else {}
							//}
							//else {}
						}

						caller.Awake(eSoundCallReason_Pause);
					}
				}
			}
			else // wait
			{
				SNDWAIT   waiter;
				SNDWAIT * waiter_ptr = & waiter;

				if (!all) // wait : only one id
				{	
					////////////////////////////////////////////////////////////////////////
					if (group) // wait : group
					{
						glock stop_others_working_with_list (m_listRenderID);

						uint32 target_group = (uint32)id;
						uint32 p = 0;

						for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
						if (target_group == listRenderID[i]->GID.group)
						{
							SNDDATA & sound  = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);
							auto    & caller = *listRenderID[i]->caller;

							{
								glock stop_while_working_with_pause_status (sound.m_pause);

								sound.pause = true; // открыли вход в паузу

								if (caller.status.in_pause) // поток не успел выйти из старой паузы
								{
									// но выходит из-за Rewind()
									if (caller.status.rewinded)
									{
										// добавляем его к ожиданию на паузу после перемотки
										caller.wait.Store(waiter_ptr, eSoundCallReason_Pause);
										caller.Awake(eSoundCallReason_Pause);
										p++;
									}
									//else // не выходит, либо выходит из-за Resume()
									//{}

									caller.status.awakened = false; // сбросить последствия Resume(), но не Rewind()
								}
								else // поток вне паузы
								{
									caller.wait.Store(waiter_ptr, eSoundCallReason_Pause);
									caller.Awake(eSoundCallReason_Pause);
									p++;
								}
							}
						}

						waiter.target = p;
					}
					////////////////////////////////////////////////////////////////////////
					else // wait : not group
					{
						glock stop_others_working_with_list (m_listRenderID);

						SNDDATA & sound = *snd[id];
						uint64    GID   = sound.GID;

						uint32 p     = 0;             // счетчик потоков
						uint32 p_max = sound.players; // всего потоков играющих этот звук сейчас активно

						if (p_max)
						{
							glock stop_while_working_with_pause_status (sound.m_pause);

							sound.pause = true; // открыли вход в паузу

							for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
							if (GID == listRenderID[i]->GID)
							{
								auto & caller = *listRenderID[i]->caller;

								if (caller.status.in_pause) // поток не успел выйти из старой паузы
								{
									// но выходит из-за Rewind()
									if (caller.status.rewinded)
									{
										// добавляем его к ожиданию на паузу после перемотки
										caller.wait.Store(waiter_ptr, eSoundCallReason_Pause);
										caller.Awake(eSoundCallReason_Pause);
										++p; 
									}
									else // не выходит, либо выходит из-за Resume()
									{
										--p_max; // уменьшаем список на обзвон
									}
			
									caller.status.awakened = false; // сбросить последствия Resume(), но не Rewind()

									// сразу выйдем, если обзвонили всех
									if (p == p_max) break;
								}
								else // поток вне паузы
								{
									caller.wait.Store(waiter_ptr, eSoundCallReason_Pause);
									caller.Awake(eSoundCallReason_Pause);

									// сразу выйдем, если обзвонили всех
									if ((++p) == p_max) break;
								}
							}
						}

						waiter.target = p;
					}
				}
				////////////////////////////////////////////////////////////////////////
				else // wait : everybody
				{
					glock stop_others_working_with_list (m_listRenderID);

					uint32 p = listRenderID_count;

					for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
					{
						SNDDATA & sound  = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);
						auto    & caller = *listRenderID[i]->caller;

						{
							glock stop_while_working_with_pause_status (sound.m_pause);

							sound.pause = true; // открыли вход в паузу

							if (caller.status.in_pause) // поток не успел выйти из старой паузы
							{
								// но выходит из-за Rewind()
								if (caller.status.rewinded)
								{
									// добавляем его к ожиданию на паузу после перемотки
									caller.wait.Store(waiter_ptr, eSoundCallReason_Pause);
									caller.Awake(eSoundCallReason_Pause);
								}
								else // не выходит, либо выходит из-за Resume()
								{
									p--; // уменьшить количество к ожиданию
								}

								caller.status.awakened = false; // сбросить последствия Resume(), но не Rewind()
							}
							else // поток вне паузы
							{
								caller.wait.Store(waiter_ptr, eSoundCallReason_Pause);
								caller.Awake(eSoundCallReason_Pause);
							}
						}
					}

					waiter.target = p;
				}

				///// ВСЕ БЛОКИРОВКИ СНЯТЫ /////

				if (waiter.target)
					ControlWait(waiter_ptr, waiter.target);

				// ожидаем некоторое время до фактического вступления в силу изменений в потоке воспроизведеняи микшера
				if (hRes_mixer == eSoundError_TRUE)
				{
					int64 time_delta   = (int64) _TIMER(time_mixer);
					int64 time_to_wait = (int64) MIXER_PLAYTHREAD_WAIT_TIME ;

					if (time_delta >= 0)
						if (time_delta < time_to_wait)
							_SLEEP(time_to_wait - time_delta);
				}
			}

			return hRes;
		}
		error_t ControlResumePRIV(uint64 id, bool wait, bool group)
		{
			// <pause> регулирует возможность входа в паузу - общий регулятор для всех копий воспр.
			// <awakened> у каждого свой, регулирует личный выход из паузы
			// при <status.runned = false> может и не быть в паузе, но будет в ней (pause_at_start)

			error_t hRes = eSoundError_TRUE;

			const bool all = _ISMISS(id);

			// сначала в микшере
			error_t hRes_mixer = ControlMixerTaskPausePRIV(id, group, true);
			T_TIME  time_mixer; // счетчик задержки при ожидании

			if (hRes_mixer == eSoundError_TRUE)
				time_mixer = _TIME;

			if (!wait) // don't wait
			{		
				if (!all) // don't wait : only one id
				{	
					////////////////////////////////////////////////////////////////////////
					if (group) // don't wait : group
					{
						glock stop_others_working_with_list (m_listRenderID);

						uint32 target_group = (uint32)id;

						for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
						if (target_group == listRenderID[i]->GID.group)
						{
							SNDDATA & sound = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);
							auto    & caller = *listRenderID[i]->caller;

							{
								glock stop_while_working_with_pause (sound.m_pause);

								sound.pause = false; // закрыли вход в паузу
								caller.status.awakened = true;

								sound._CV_ALL(cv_pause); // позвали
							}
						}
					}
					////////////////////////////////////////////////////////////////////////
					else // don't wait : not group
					{
						glock stop_others_working_with_list (m_listRenderID);

						SNDDATA & sound = *snd[id];
						uint64    GID   = sound.GID;

						uint32 p = 0;				  // счетчик потоков
						uint32 p_max = sound.players; // всего потоков играющих этот звук сейчас активно

						if (p_max)
						{
							glock stop_while_working_with_pause (sound.m_pause);

							sound.pause = false; // закрыли вход в паузу

							for (uint32 i = 0; i < listRenderID_count; i++)
							if (GID == listRenderID[i]->GID)
							{
								auto & caller = *listRenderID[i]->caller;

								caller.status.awakened = true;

								if ((++p) == p_max) break;
							}

							sound._CV_ALL(cv_pause); // позвали
						}
					}
				}
				////////////////////////////////////////////////////////////////////////
				else // don't wait : everybody
				{
					glock stop_others_working_with_list (m_listRenderID);

					for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
					{
						SNDDATA & sound = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);
						auto    & caller = *listRenderID[i]->caller;

						{
							glock stop_while_working_with_pause (sound.m_pause);

							sound.pause = false; // закрыли вход в паузу
							caller.status.awakened = true;

							sound._CV_ALL(cv_pause); // позвали
						}
					}
				}
			}
			else // wait
			{
				SNDWAIT   waiter;
				SNDWAIT * waiter_ptr = &waiter;

				if (!all) // wait : only one id
				{
					////////////////////////////////////////////////////////////////////////
					if (group) // wait : group
					{
						glock stop_others_working_with_list (m_listRenderID);

						uint32 target_group = (uint32)id;
						uint32 p = 0;

						for (uint32 i = 0; i < listRenderID_count; i++)
						if (target_group == listRenderID[i]->GID.group)
						{
							SNDDATA & sound  = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);
							auto    & caller = *listRenderID[i]->caller;

							{
								glock stop_while_working_with_pause (sound.m_pause);

								sound.pause = false; // закрыли вход в паузу

								if ( caller.status.in_pause || // поток в паузе
									!caller.status.runned )    // или нужен первичный запуск
								{
									caller.wait.Store(waiter_ptr, eSoundCallReason_Resume);
									caller.status.awakened = true;
									sound._CV_ALL(cv_pause); // позвали
									p++;
								}
							}
						}

						waiter.target = p;
					}
					////////////////////////////////////////////////////////////////////////
					else // wait : not group
					{
						glock stop_others_working_with_list (m_listRenderID);

						SNDDATA & sound = *snd[id];
						uint64    GID   = sound.GID;

						uint32 p     = 0;             // счетчик потоков
						uint32 p_max = sound.players; // всего потоков играющих этот звук сейчас активно

						if (p_max)
						{
							glock stop_while_working_with_pause (sound.m_pause);

							sound.pause = false; // закрыли вход в паузу

							for (uint32 i = 0; i < listRenderID_count; i++)
							if (GID == listRenderID[i]->GID)
							{
								auto & caller = *listRenderID[i]->caller;

								if ( caller.status.in_pause || // поток в паузе
									!caller.status.runned )    // или нужен первичный запуск
								{
									caller.wait.Store(waiter_ptr, eSoundCallReason_Resume);
									caller.status.awakened = true;

									if ((++p) == p_max) // сразу выйдем, если обзвонили всех
										break;						
								}
								else // поток вне паузы
								{
									if (p == (--p_max)) // уменьшаем список на обзвон
										break;
								}
							}

							sound._CV_ALL(cv_pause); // позвали
						}

						waiter.target = p;
					}
				}
				////////////////////////////////////////////////////////////////////////
				else // wait : everybody
				{
					glock stop_others_working_with_list (m_listRenderID);

					uint32 p = 0;

					for (uint32 i = 0; i < listRenderID_count; i++)
					{
						SNDDATA & sound  = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);
						auto    & caller = *listRenderID[i]->caller;

						{
							glock stop_while_working_with_pause (sound.m_pause);

							sound.pause = false; // закрыли вход в паузу

							if ( caller.status.in_pause || // поток в паузе
								!caller.status.runned )    // или нужен первичный запуск
							{
								caller.wait.Store(waiter_ptr, eSoundCallReason_Resume);
								caller.status.awakened = true;
								sound._CV_ALL(cv_pause); // позвали
								p++;

								// Если воспроизводится сразу много с одним ID при зове первого проснутся
								// и прочие и попытаются выйти. Кого-то мы успеем поймать и обвесить waiter_ptr,
								// а кого-то и нет - неважно - нам же меньше ждать
							}
						}
					}

					waiter.target = p;
				}

				///// ВСЕ БЛОКИРОВКИ СНЯТЫ /////

				if (waiter.target)
					ControlWait(waiter_ptr, waiter.target);

				// ожидаем некоторое время до фактического вступления в силу изменений в потоке воспроизведеняи микшера
				if (hRes_mixer == eSoundError_TRUE)
				{
					int64 time_delta   = (int64) _TIMER(time_mixer);
					int64 time_to_wait = (int64) MIXER_PLAYTHREAD_WAIT_TIME ;

					if (time_delta >= 0)
						if (time_delta < time_to_wait)
							_SLEEP(time_to_wait - time_delta);
				}
			}

			return hRes;
		}
		error_t ControlRewindPRIV(uint64 id, float time_in_seconds, bool pause_after, bool wait, bool group)
		{
			#define _CHECK_TIME_(requested_time)                 \
				if (sound.duration < time_in_seconds)            \
					 requested_time = sound.duration;            \
				else requested_time = time_in_seconds;           \

			//////////////////////////////////////////////////////

			error_t hRes = eSoundError_TRUE;

			const bool all = _ISMISS(id);

			// сначала в микшере
			error_t hRes_mixer = ControlMixerTaskRewindPRIV(id, time_in_seconds, pause_after, group);
			T_TIME  time_mixer; // счетчик задержки при ожидании

			if (hRes_mixer == eSoundError_TRUE)
				time_mixer = _TIME;

			eSoundCallReason call_reason; // stop (rewind + pause) or rewind-only reason

			if (pause_after)
				 call_reason = eSoundCallReason_Stop;
			else // rewind without pause
			{
				if (wait)
					 call_reason = eSoundCallReason_Rewind_WAITER;
				else call_reason = eSoundCallReason_Rewind_NO_WAITER;
			}

			time_in_seconds = abs(time_in_seconds);

			// ЗАМЕТКА
			// При отсутствии ожидания необходимо взять дополнительно (caller.status.m_rewind) до взятия (sound.m_pause)
			// на случай "вклинивания" нового заказа во время выполнения старого

			if (!wait) // don't wait
			{
				if (!all) // don't wait : only one id
				{	
					////////////////////////////////////////////////////////////////////////
					if (group) // don't wait : group
					{
						glock stop_others_working_with_list (m_listRenderID);

						uint32 target_group = (uint32)id;

						for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
						if (target_group == listRenderID[i]->GID.group)
						{
							SNDDATA & sound  = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);
							auto    & caller = *listRenderID[i]->caller;

							{
								float requested_time;
								_CHECK_TIME_ (requested_time) ;

								{
									glock stop_while_working_with_pause (sound.m_pause);

									if (pause_after) 
										 sound.pause = true;  // открыли вход в паузу т.к. заказано, иначе...
									else sound.pause = false; // ...перемотка по умолчанию сбрасывает паузу

								//	caller.status.awakened = true; // выбьем из паузы
									caller.status.rewinded = true; // .
									caller.Awake(call_reason, requested_time);

									// зовём из паузы
									sound._CV_ALL(cv_pause);
								}
							}
						}
					}
					////////////////////////////////////////////////////////////////////////
					else // don't wait : not group
					{
						glock stop_others_working_with_list (m_listRenderID);

						SNDDATA & sound = *snd[id];
						uint64    GID   = sound.GID;

						uint32 p     = 0;             // счетчик потоков
						uint32 p_max = sound.players; // всего потоков играющих этот звук сейчас активно

						if (p_max)
						{ 
							for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
							if (GID == listRenderID[i]->GID)
							{
								auto & caller = *listRenderID[i]->caller;

								{
									float requested_time;
									_CHECK_TIME_ (requested_time) ;

									{
										glock stop_while_working_with_pause (sound.m_pause);

										if (pause_after)
											 sound.pause = true;  // открыли вход в паузу т.к. заказано, иначе...
										else sound.pause = false; // ...перемотка по умолчанию сбрасывает паузу

									//	caller.status.awakened = true; // выбьем из паузы
										caller.status.rewinded = true; // .
										caller.Awake(call_reason, requested_time);

										// зовём из паузы
										sound._CV_ALL(cv_pause);
									}
								}

								// сразу выйдем, если обзвонили всех
								if ((++p) == p_max) break;
							}
						}
					}
				}
				////////////////////////////////////////////////////////////////////////
				else // don't wait : everybody
				{
					glock stop_others_working_with_list (m_listRenderID);

					for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
					{
						SNDDATA & sound  = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);
						auto    & caller = *listRenderID[i]->caller;

						{
							float requested_time;
							_CHECK_TIME_ (requested_time) ;

							{
								glock stop_while_working_with_pause (sound.m_pause);

								if (pause_after)
									 sound.pause = true;  // открыли вход в паузу т.к. заказано, иначе...
								else sound.pause = false; // ...перемотка по умолчанию сбрасывает паузу

							//	caller.status.awakened = true; // выбьем из паузы
								caller.status.rewinded = true; // .
								caller.Awake(call_reason, requested_time);

								// зовём из паузы
								sound._CV_ALL(cv_pause);
							}
						}
					}
				}
			}
			else // wait
			{
				SNDWAIT   waiter;
				SNDWAIT * waiter_ptr = & waiter;

				if (!all) // wait : only one id
				{
					////////////////////////////////////////////////////////////////////////
					if (group) // wait : group
					{
						glock stop_others_working_with_list (m_listRenderID);

						uint32 target_group = (uint32)id;
						uint32 p = 0;

						for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
						if (target_group == listRenderID[i]->GID.group)
						{
							SNDDATA & sound  = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);
							auto    & caller = *listRenderID[i]->caller;

							float requested_time;
							_CHECK_TIME_ (requested_time) ;

							{
								glock stop_while_working_with_pause (sound.m_pause);

								if (pause_after) 
									 sound.pause = true;  // открыли вход в паузу т.к. заказано, иначе...
								else sound.pause = false; // ...перемотка по умолчанию сбрасывает паузу

							//	caller.status.awakened = true; // выбьем из паузы
								caller.status.rewinded = true; // .
								caller.wait.Store(waiter_ptr, call_reason);
								caller.Awake(call_reason, requested_time);
								p++;

								// зовём из паузы
								sound._CV_ALL(cv_pause);
							}
						}
					
						waiter.target = p;
					}
					////////////////////////////////////////////////////////////////////////
					else // wait : not group
					{
						glock stop_others_working_with_list (m_listRenderID);

						SNDDATA & sound = *snd[id];
						uint64    GID   = sound.GID;

						uint32 p     = 0;             // счетчик потоков
						uint32 p_max = sound.players; // всего потоков играющих этот звук сейчас активно

						if (p_max)
						{
							float requested_time;
							_CHECK_TIME_ (requested_time) ;

							{
								glock stop_while_working_with_pause (sound.m_pause);

								if (pause_after) 
									 sound.pause = true;  // открыли вход в паузу т.к. заказано, иначе...
								else sound.pause = false; // ...перемотка по умолчанию сбрасывает паузу

								for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
								if (GID == listRenderID[i]->GID)
								{
									auto & caller = *listRenderID[i]->caller;

								//	caller.status.awakened = true; // выбьем из паузы
									caller.status.rewinded = true; // .
									caller.wait.Store(waiter_ptr, call_reason);
									caller.Awake(call_reason, requested_time);

									// сразу выйдем, если обзвонили всех
									if ((++p) == p_max) break;
								}

								// зовём из паузы
								sound._CV_ALL(cv_pause);
							}
						}

						waiter.target = p;
					}
				}
				////////////////////////////////////////////////////////////////////////
				else // wait : everybody
				{
					glock stop_others_working_with_list (m_listRenderID);

					uint32 p = listRenderID_count;

					for (uint32 i = 0; i < listRenderID_count; i++) // пройдёмся по активным
					{
						SNDDATA & sound  = *reinterpret_cast<SNDDATA*>(listRenderID[i]->actual_sound);
						auto    & caller = *listRenderID[i]->caller;

						float requested_time;
						_CHECK_TIME_ (requested_time) ;

						{
							glock stop_while_working_with_pause (sound.m_pause);

							if (pause_after)
								 sound.pause = true;  // открыли вход в паузу т.к. заказано, иначе...
							else sound.pause = false; // ...перемотка по умолчанию сбрасывает паузу

						//	caller.status.awakened = true; // выбьем из паузы
							caller.status.rewinded = true; // .
							caller.wait.Store(waiter_ptr, call_reason);
							caller.Awake(call_reason, requested_time);

							// зовём из паузы
							sound._CV_ALL(cv_pause);
						}
					}
					
					waiter.target = p;
				}

				///// ВСЕ БЛОКИРОВКИ СНЯТЫ /////

				if (waiter.target)
					ControlWait(waiter_ptr, waiter.target);

				// ожидаем некоторое время до фактического вступления в силу изменений в потоке воспроизведеняи микшера
				if (hRes_mixer == eSoundError_TRUE)
				{
					int64 time_delta   = (int64) _TIMER(time_mixer);
					int64 time_to_wait = (int64) MIXER_PLAYTHREAD_WAIT_TIME ;

					if (time_delta >= 0)
						if (time_delta < time_to_wait)
							_SLEEP(time_to_wait - time_delta);
				}
			}

			return hRes;

			#undef _CHECK_TIME_
		}

	private:
		//>> Обеспечивает ожидание
		void ControlWait(SNDWAIT * waiter, uint32 num_threads)
		{
			SNDWAIT & w = *waiter;

			ulock lock (w.m_wait);
			while (w.counter < num_threads)
			{
				_CV_WAIT(w.cv_wait, lock);
			}
		}

#pragma region < DEFINES_FOR_CONTROL >

#define _GROUP_ID_CHECK_                                                                      \
		if (target_group >= eSoundGroup_USER_MAX) /* ERROR max group ID (LIMIT REACHED) */    \
			return eSoundError_MaxGroupID;                                                    \
		                                                                                      \
		if ((target_group + eSoundGroup_USER) >= snd_g.size()) /* ERROR bad ID of group */    \
			return eSoundError_BadID_G;

#define _SOUND_ID_CHECK_                         \
		uint64 id;                               \
		                                         \
		if (_ISMISS(GID._u64)) id = SOUND_ALL;   \
		else                                     \
		{                                        \
			error_t hRes = CheckSound(GID);      \
			if (hRes != eSoundError_TRUE)        \
				return hRes;                     \
				                                 \
			id = _SoundID(GID);                  \
		}

#pragma endregion

	public: 
		//>> Закрытие :: GID = SOUND_ALL для всех текущих
		error_t ControlInterrupt(uint64_gid GID, bool wait)
		{
			glock control_guard (m_control);

			_SOUND_ID_CHECK_
			                            //     ID             WAIT        GROUP
			return ControlInterruptPRIV ( std::move(id), std::move(wait), false );
		}

		//>> Закрытие :: GID = SOUND_ALL для всех текущих :: без ожидания
		error_t ControlInterrupt(uint64_gid GID)
		{
			glock control_guard (m_control);

			_SOUND_ID_CHECK_
				                        //     ID         WAIT  GROUP
			return ControlInterruptPRIV ( std::move(id), false, false );
		}

		//>> Закрытие :: id группы
		error_t ControlInterruptGroup(uint32 target_group, bool wait)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_
				                        //     ID             WAIT       GROUP
			return ControlInterruptPRIV ( target_group, std::move(wait), true );
		}

		//>> Закрытие :: id группы :: без ожидания
		error_t ControlInterruptGroup(uint32 target_group)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_
				                        //     ID        WAIT  GROUP
			return ControlInterruptPRIV ( target_group, false, true );
		}

	public:
		//>> Пауза :: GID = SOUND_ALL для всех текущих
		error_t ControlPause(uint64_gid GID, bool wait)
		{
			glock control_guard (m_control);

			_SOUND_ID_CHECK_
				                    //     ID             WAIT        GROUP
			return ControlPausePRIV ( std::move(id), std::move(wait), false );
		}

		//>> Пауза :: GID = SOUND_ALL для всех текущих :: без ожидания
		error_t ControlPause(uint64_gid GID)
		{
			glock control_guard (m_control);

			_SOUND_ID_CHECK_
				                    //     ID         WAIT   GROUP
			return ControlPausePRIV ( std::move(id), false, false );
		}

		//>> Пауза :: id группы
		error_t ControlPauseGroup(uint32 target_group, bool wait)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_
				                    //     ID             WAIT       GROUP
			return ControlPausePRIV ( target_group, std::move(wait), true );
		}

		//>> Пауза :: id группы :: без ожидания
		error_t ControlPauseGroup(uint32 target_group)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_
				                    //     ID        WAIT  GROUP
			return ControlPausePRIV ( target_group, false, true );
		}

	public:
		//>> Продолжить :: GID = SOUND_ALL для всех текущих
		error_t ControlResume(uint64_gid GID, bool wait)
		{
			glock control_guard (m_control);

			_SOUND_ID_CHECK_
				                     //     ID             WAIT        GROUP
			return ControlResumePRIV ( std::move(id), std::move(wait), false );
		}

		//>> Продолжить :: GID = SOUND_ALL для всех текущих :: без ожидания
		error_t ControlResume(uint64_gid GID)
		{
			glock control_guard (m_control);

			_SOUND_ID_CHECK_
				                     //     ID         WAIT  GROUP
			return ControlResumePRIV ( std::move(id), false, false );
		}

		//>> Продолжить :: id группы
		error_t ControlResumeGroup(uint32 target_group, bool wait)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_
				                     //     ID             WAIT       GROUP
			return ControlResumePRIV ( target_group, std::move(wait), true );
		}

		//>> Продолжить :: id группы :: без ожидания
		error_t ControlResumeGroup(uint32 target_group)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_
				                     //     ID        WAIT  GROUP
			return ControlResumePRIV ( target_group, false, true );
		}

	public:
		//>> Перемотка на отметку <time> :: GID = SOUND_ALL для всех текущих
		error_t ControlRewind(uint64_gid GID, float time_in_seconds, bool pause_after, bool wait)
		{
			glock control_guard (m_control);

			_SOUND_ID_CHECK_
				                     //     ID                   TIME                      PAUSE               WAIT        GROUP
			return ControlRewindPRIV ( std::move(id), std::move(time_in_seconds), std::move(pause_after), std::move(wait), false );
		}

		//>> Перемотка на отметку <time> :: GID = SOUND_ALL для всех текущих :: без ожидания, без паузы после
		error_t ControlRewind(uint64_gid GID, float time_in_seconds)
		{
			glock control_guard (m_control);

			_SOUND_ID_CHECK_
				                     //     ID                   TIME             PAUSE   WAIT  GROUP
			return ControlRewindPRIV ( std::move(id), std::move(time_in_seconds), false, false, false );
		}

		//>> Перемотка на отметку <time> :: id группы
		error_t ControlRewindGroup(uint32 target_group, float time_in_seconds, bool pause_after, bool wait)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_
				                     //     ID                   TIME                     PAUSE               WAIT        GROUP
			return ControlRewindPRIV ( target_group, std::move(time_in_seconds), std::move(pause_after), std::move(wait), true );
		}

		//>> Перемотка на отметку <time> :: id группы :: без ожидания, без паузы после
		error_t ControlRewindGroup(uint32 target_group, float time_in_seconds)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_
				                     //     ID                   TIME            PAUSE   WAIT  GROUP
			return ControlRewindPRIV ( target_group, std::move(time_in_seconds), false, false, true );
		}

	public:
		//>> Пауза и сброс в начало :: GID = SOUND_ALL для всех текущих
		error_t ControlStop(uint64_gid GID, bool wait)
		{	
			glock control_guard (m_control);

			_SOUND_ID_CHECK_
				                     //     ID       TIME  PAUSE       WAIT       GROUP
			return ControlRewindPRIV ( std::move(id), 0.f, true, std::move(wait), false );
		}

		//>> Пауза и сброс в начало :: GID = SOUND_ALL для всех текущих :: без ожидания
		error_t ControlStop(uint64_gid GID)
		{
			glock control_guard (m_control);

			_SOUND_ID_CHECK_
				                     //     ID       TIME  PAUSE  WAIT  GROUP
			return ControlRewindPRIV ( std::move(id), 0.f, true, false, false );
		}

		//>> Пауза и сброс в начало :: id группы
		error_t ControlStopGroup(uint32 target_group, bool wait)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_
				                     //     ID      TIME  PAUSE       WAIT       GROUP
			return ControlRewindPRIV ( target_group, 0.f, true, std::move(wait), true );
		}

		//>> Пауза и сброс в начало :: id группы :: без ожидания
		error_t ControlStopGroup(uint32 target_group)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_
				                     //     ID      TIME  PAUSE  WAIT  GROUP
			return ControlRewindPRIV ( target_group, 0.f, true, false, true );
		}

	public:
		//>> Запускает задачу управления в специальный поток исполнения :: передаёт task_id
		error_t ControlTaskInterrupt(bool with_result, uint64_gid GID, uint64 & task_id)
		{
			glock control_guard (m_control);

			_SOUND_ID_CHECK_
		
			if (with_result)                   //   FUNC                          CLASS      ID        WAIT  GROUP
				 task_id = controlpool.RunTaskRet (&CSound::ControlInterruptPRIV, this, std::move(id), true, false);
			else task_id = controlpool.RunTask    (&CSound::ControlInterruptPRIV, this, std::move(id), true, false);

			return eSoundError_TRUE;
		}

		//>> Запускает задачу управления в специальный поток исполнения :: передаёт task_id
		error_t ControlTaskPause(bool with_result, uint64_gid GID, uint64 & task_id)
		{
			glock control_guard (m_control);

			_SOUND_ID_CHECK_

			if (with_result)                   //   FUNC                      CLASS      ID        WAIT  GROUP
				 task_id = controlpool.RunTaskRet (&CSound::ControlPausePRIV, this, std::move(id), true, false);
			else task_id = controlpool.RunTask    (&CSound::ControlPausePRIV, this, std::move(id), true, false);

			return eSoundError_TRUE;
		}

		//>> Запускает задачу управления в специальный поток исполнения :: передаёт task_id
		error_t ControlTaskResume(bool with_result, uint64_gid GID, uint64 & task_id)
		{
			glock control_guard (m_control);

			_SOUND_ID_CHECK_

			if (with_result)                   //   FUNC                       CLASS      ID        WAIT  GROUP
				 task_id = controlpool.RunTaskRet (&CSound::ControlResumePRIV, this, std::move(id), true, false);
			else task_id = controlpool.RunTask    (&CSound::ControlResumePRIV, this, std::move(id), true, false);

			return eSoundError_TRUE;
		}

		//>> Запускает задачу управления в специальный поток исполнения :: передаёт task_id
		error_t ControlTaskRewind(bool with_result, uint64_gid GID, float time_in_seconds, bool pause_after, uint64 & task_id)
		{
			glock control_guard (m_control);

			_SOUND_ID_CHECK_

			if (with_result)                   //   FUNC                       CLASS      ID               TIME                         PAUSE           WAIT  GROUP
				 task_id = controlpool.RunTaskRet (&CSound::ControlRewindPRIV, this, std::move(id), std::move(time_in_seconds), std::move(pause_after), true, false);
			else task_id = controlpool.RunTask    (&CSound::ControlRewindPRIV, this, std::move(id), std::move(time_in_seconds), std::move(pause_after), true, false);

			return eSoundError_TRUE;
		}

		//>> Запускает задачу управления в специальный поток исполнения :: передаёт task_id
		error_t ControlTaskStop(bool with_result, uint64_gid GID, uint64 & task_id)
		{
			glock control_guard (m_control);

			_SOUND_ID_CHECK_

			if (with_result)                   //   FUNC                       CLASS      ID       TIME  PAUSE WAIT  GROUP
				 task_id = controlpool.RunTaskRet (&CSound::ControlRewindPRIV, this, std::move(id), 0.f, true, true, false);
			else task_id = controlpool.RunTask    (&CSound::ControlRewindPRIV, this, std::move(id), 0.f, true, true, false);

			return eSoundError_TRUE;
		}

	public:
		//>> Запускает задачу управления в специальный поток исполнения :: передаёт task_id
		error_t ControlTaskInterruptGroup(bool with_result, uint32 target_group, uint64 & task_id)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_

			if (with_result)                   //   FUNC                          CLASS      ID       WAIT  GROUP
				 task_id = controlpool.RunTaskRet (&CSound::ControlInterruptPRIV, this, target_group, true, true);
			else task_id = controlpool.RunTask    (&CSound::ControlInterruptPRIV, this, target_group, true, true);

			return eSoundError_TRUE;
		}

		//>> Запускает задачу управления в специальный поток исполнения :: передаёт task_id
		error_t ControlTaskPauseGroup(bool with_result, uint32 target_group, uint64 & task_id)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_

			if (with_result)                   //   FUNC                      CLASS      ID       WAIT  GROUP
				 task_id = controlpool.RunTaskRet (&CSound::ControlPausePRIV, this, target_group, true, true);
			else task_id = controlpool.RunTask    (&CSound::ControlPausePRIV, this, target_group, true, true);

			return eSoundError_TRUE;
		}

		//>> Запускает задачу управления в специальный поток исполнения :: передаёт task_id
		error_t ControlTaskResumeGroup(bool with_result, uint32 target_group, uint64 & task_id)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_

			if (with_result)                   //   FUNC                       CLASS      ID       WAIT  GROUP
				 task_id = controlpool.RunTaskRet (&CSound::ControlResumePRIV, this, target_group, true, true);
			else task_id = controlpool.RunTask    (&CSound::ControlResumePRIV, this, target_group, true, true);

			return eSoundError_TRUE;
		}

		//>> Запускает задачу управления в специальный поток исполнения :: передаёт task_id
		error_t ControlTaskRewindGroup(bool with_result, uint32 target_group, float time_in_seconds, bool pause_after, uint64 & task_id)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_

			if (with_result)                   //   FUNC                       CLASS      ID               TIME                         PAUSE          WAIT  GROUP
				 task_id = controlpool.RunTaskRet (&CSound::ControlRewindPRIV, this, target_group, std::move(time_in_seconds), std::move(pause_after), true, true);
			else task_id = controlpool.RunTask    (&CSound::ControlRewindPRIV, this, target_group, std::move(time_in_seconds), std::move(pause_after), true, true);

			return eSoundError_TRUE;
		}

		//>> Запускает задачу управления в специальный поток исполнения :: передаёт task_id
		error_t ControlTaskStopGroup(bool with_result, uint32 target_group, uint64 & task_id)
		{
			glock control_guard (m_control);

			_GROUP_ID_CHECK_

			if (with_result)                   //   FUNC                       CLASS      ID      TIME  PAUSE WAIT  GROUP
				 task_id = controlpool.RunTaskRet (&CSound::ControlRewindPRIV, this, target_group, 0.f, true, true, true);
			else task_id = controlpool.RunTask    (&CSound::ControlRewindPRIV, this, target_group, 0.f, true, true, true);

			return eSoundError_TRUE;
		}

	public:
		//>> Сообщает статус выполнения задачи и результат (если был заказан при запуске, иначе код результата MISSING)
		error_t ControlTaskStatus(uint64 task_id, error_t & task_result)
		{
			error_t hRes = eSoundError_TRUE;
			error_t result;

			eThreadCode state = controlpool.GrabResult(result, task_id, false, true);

			switch (state)
			{			
			case TC_SUCCESS:
				hRes = eSoundError_TRUE;	  // задача выполнена (с результатом)
				task_result = result;
				break;
			case TC_RESULT_WAIT:
			case TC_RESULT_NONE_IN:
				hRes = eSoundError_FALSE;	  // задача ещё не выполнена
				break;
			case TC_POOL_TASK_NOT_FOUND:
				hRes = eSoundError_NoneTask;  // задачи нет (возможно, уже выполнена и сброшена)
				break;
			case TC_POOL_UID_MISMATCH:
			case TC_POOL_ID_MISMATCH:
				hRes = eSoundError_BadTaskID; // ошибка ID
				break;
		//	case TC_POOL_TASK_REMOVED:
		//	case TC_RESULT_NONE_OUT:
		//	case TC_RESULT_LOCK:	
			default:
				hRes = eSoundError_TRUE;	  // задача выполнена (без результатов)
				task_result = MISSING;
				break;
			}

			return hRes;
		}

		//>> Ставит в ожидание до конца выполнения задачи
		error_t ControlTaskStatusWait(uint64 task_id)
		{
			error_t hRes = eSoundError_TRUE;

			auto state = controlpool.Wait(task_id);

			switch (state)
			{
			case TC_SUCCESS:
				hRes = eSoundError_TRUE;      // ожидание было выполнено
				break;
			case TC_POOL_UID_MISMATCH:
			case TC_POOL_ID_MISMATCH:
				hRes = eSoundError_BadTaskID; // ошибка ID
				break;
			case TC_POOL_TASK_NOT_FOUND:
				hRes = eSoundError_NoneTask;  // задачи нет (возможно, уже выполнена и сброшена)
				break;
			default:
				hRes = eSoundError_FALSE;     // ??? (ошибка кода)
				break;
			}

			return hRes;
		}

#pragma region < UNDEF_DEFINES_FOR_CONTROL >

#undef _GROUP_ID_CHECK_
#undef _SOUND_ID_CHECK_

#pragma endregion

	//////////////// DEBUG (testing purposes only) ////////////////

	private:
		//>> DEBUG : create *.wav file on HDD from current PCM buffer
		void SaveSoundPCM(SNDDATA & snd, const wchar_t* gamePath, const wchar_t* filename)
		{
			WAVERIFF wav;
			wchar_t filepath[256];

			wav._Generate (
				snd.file.count,
				snd.snddesc->nChannels,
				snd.snddesc->nSamplesPerSec,
				snd.snddesc->wBitsPerSample,
				snd.snddesc->wFormatTag,
				DATAMAKEBY
			);

			wsprintf(filepath, L"%s\\%s", gamePath, filename);
			wav._Write(filepath, snd.file.buf);
		}

		//>> DEBUG : create *.wav file on HDD from current MP3 buffer
		void SaveSoundMP3(SNDDATA & snd, const wchar_t* gamePath, const wchar_t* filename)
		{
			CMP3Converter converter;
			auto & _ = converter.settings.in;

			_.data        = snd.file.buf;
			_.size        = snd.file.count;
			_.start_frame = 0;
			_.start_time  = 0;
			_.time        = 0;
			_.use_frame   = 0;
			_.loop        = 0;

			auto time   = _TIME;
			auto result = converter.Run();

			if (result != eMP3ConvStatus_Normal)
				printf("\nMP3 converter ERROR (exit code = %i)", result);
			else
			{
				printf("\nMP3 converter done - FULL TIME : %i ms", _TIMER(time));

				SNDDATA out;

				out.file.buf                = converter.settings.out.data_out;
				out.file.count              = converter.settings.out.size_out;
				out.snddesc->nChannels      = converter.settings.out.nChannels;
				out.snddesc->nSamplesPerSec = converter.settings.out.nSamplesPerSec;
				out.snddesc->wBitsPerSample = converter.settings.out.wBitsPerSample;
				out.snddesc->wFormatTag     = converter.settings.out.wFormatTag;

				converter.settings.out.Printf();

				SaveSoundPCM(out, gamePath, filename);

				out.file.buf   = nullptr;
				out.file.count = 0;
			}
		}

	public:
		//>> DEBUG : create *.wav file on HDD from current SNDDATA
		bool SaveSound(uint64 id, const wchar_t* gamePath, const wchar_t* filename)
		{
			SNDDATA & snd = *(this->snd)[id];
			switch (snd.type)
			{
			case eSoundType_WAVE: SaveSoundPCM(snd, gamePath, filename); break;
			case eSoundType_MP3:  SaveSoundMP3(snd, gamePath, filename); break;
			case eSoundType_OGG:
			case eSoundType_AAC:
			case eSoundType_UNKNOWN:
			default:
				return false;
			}

			return true;
		}

		//>> DEBUG : Выводит в консоль информацию WAVEFORMATEX
		void PrintfSNDDESC(const SNDDESC & snddesc)
		{
			printf("\n"
				"\nwFormatTag          %4x-h"	// format type
				"\nnChannels           %4x-h"	// number of channels (i.e. mono, stereo...)
				"\nnSamplesPerSec  %8i"			// sample rate
				"\nnAvgBytesPerSec %8i"			// for buffer estimation
				"\nnBlockAlign         %4x-h"	// block size of data
				"\nwBitsPerSample      %4i"		// number of bits per sample of mono data
				"\ncbSize              %4x-h",	// the count in bytes of the size of extra information (after cbSize)
				snddesc.wFormatTag,
				snddesc.nChannels,
				snddesc.nSamplesPerSec,
				snddesc.nAvgBytesPerSec,
				snddesc.nBlockAlign,
				snddesc.wBitsPerSample,
				snddesc.cbSize);
		}

		//>> DEBUG : Вывод в консоль сведений из .wav заголовка
		void PrintWaveRiff(const WAVERIFF& wav)
		{
			printf(	"\n%8x-h chunkId"
					"\n%8x-h chunkSize"
					"\n%8x-h format"
					"\n%8x-h subchunk1Id"
					"\n%8x-h subchunk1Size"
					"\n%8x-h audioFormat"
					"\n%8x-h numChannels"
					"\n%8i   sampleRate"
					"\n%8i   byteRate"
					"\n%8x-h blockAlign"
					"\n%8i   bitsPerSample"
					"\n%8x-h subchunk2Id"
					"\n%8x-h subchunk2Size",
					wav.chunkId, wav.chunkSize, wav.format, wav.subchunk1Id, wav.subchunk1Size,
					wav.audioFormat, wav.numChannels, wav.sampleRate, wav.byteRate, wav.blockAlign,
					wav.bitsPerSample, wav.subchunk2Id, wav.subchunk2Size );

			FLOAT  sduration  = (FLOAT)wav.subchunk2Size;
			       sduration /= (FLOAT)wav.bitsPerSample / 8;
			       sduration /= wav.numChannels;
			       sduration /= wav.sampleRate;
			uint16 mduration = (uint16)(floor(sduration) / 60);
			sduration = sduration - (mduration * 60);
			printf("\n\nDuration [M:S.ms]: %02d:%06.3f\n", mduration, sduration);
		}

	/*
	public:
		//>> 
		void __ConvertWavTest()
		{
			Load(DIRECTORY_GAME, L"WakeDream.wav");

			SNDDESC snddesc_target;

			snddesc_target.nChannels      = 2;
			snddesc_target.nSamplesPerSec = 48000;
			snddesc_target.wBitsPerSample = 16;
			snddesc_target.wFormatTag     = WAVE_FORMAT_PCM;

			uint32 size_f2b    = snddesc_target.nChannels * (snddesc_target.wBitsPerSample / 8);
			uint32 size_f2b_1s = size_f2b * snddesc_target.nSamplesPerSec;

			CPCMConverter converter;

			converter.origin = (*snd[0]).snddesc;
			converter.target = & snddesc_target;

			converter.settings.loop            = false;
			converter.settings.allocate        = true;
			converter.settings.maxTarget       = false;

			converter.settings.origin.buffer      = (*snd[0]).file.buf;
			converter.settings.origin.size        = (*snd[0]).file.count;
			converter.settings.origin.samples     = 0;
			converter.settings.origin.time        = 0.f;
			converter.settings.origin.usetime     = false;
			converter.settings.origin.BE          = false;
			converter.settings.origin.user_signed = false;
			converter.settings.origin.auto_signed = true;

			converter.settings.target.buffer      = nullptr;
			converter.settings.target.size        = 0;
			converter.settings.target.samples     = 0;      //converter.origin->nSamplesPerSec * 3;
			converter.settings.target.time        = 1000.f; //3.f;
			converter.settings.target.usetime     = true;
			converter.settings.target.BE          = false;
			converter.settings.target.user_signed = false;
			converter.settings.target.auto_signed = true;

			uint32 result = converter.Run();

			if (result == ePCMConvStatus_Normal)
			{
				(*snd[0]).file.Close();

				(*snd[0]).file.buf   = (byte*) converter.settings.target.buffer;
				(*snd[0]).file.count = converter.settings.target.size;
			//	(*snd[0]).file.count = converter.settings.outInfo.target_samples * size_f2b;

				printf("\nOut frames : %i", converter.settings.outInfo.target_samples);

				uint32 chunk       = converter.target->nSamplesPerSec;
				uint32 chunk_count = converter.settings.outInfo.target_samples / chunk;
				uint32 chunk_add   = converter.settings.outInfo.target_samples - chunk * chunk_count;

				printf(" = %i x %i + %i", chunk, chunk_count, chunk_add);

				// Out frames : 164676 = 48000 x 3 + 20676
				// Out frames : 143998 = 48000 x 2 + 47998

				// Out frames : 164675 = 48000 x 3 + 20675

				(*snd[0]).snddesc->nChannels      = snddesc_target.nChannels;
				(*snd[0]).snddesc->nSamplesPerSec = snddesc_target.nSamplesPerSec;
				(*snd[0]).snddesc->wBitsPerSample = snddesc_target.wBitsPerSample;
				(*snd[0]).snddesc->wFormatTag     = snddesc_target.wFormatTag;

				SaveSound(0, DIRECTORY_GAME \
					L"\\data\\sounds", L"00000000000.wav");

				(*snd[0]).file.buf   = nullptr;
				(*snd[0]).file.count = 0;
			}
			else printf("\nConverter ERROR %i", result);
		}

		//>> 
		void __ConvertWavTest2()
		{
			TBUFFER <byte, uint32> storage (0xA00000h); // ~ 10MB
			uint32 storage_filled = 0;

			Load(DIRECTORY_GAME, L"WakeDream.wav"); 
			//Load(DIRECTORY_GAME, L"__32bit_48KHz.wav");

			SNDDESC snddesc_target;

			snddesc_target.nChannels      = 2;
			snddesc_target.nSamplesPerSec = 48000;
			snddesc_target.wBitsPerSample = 16;
			snddesc_target.wFormatTag     = WAVE_FORMAT_PCM;

			uint32 size_f2b    = snddesc_target.nChannels * (snddesc_target.wBitsPerSample / 8);
			uint32 size_f2b_1s = size_f2b * snddesc_target.nSamplesPerSec;

			TBUFFER <byte, uint32> storage_1s (size_f2b_1s);

			CPCMConverter converter;

			converter.origin = (*snd[0]).snddesc;
			converter.target = & snddesc_target;

			uint32 size_f2b_origin = converter.origin->nChannels * (converter.origin->wBitsPerSample / 8);

			converter.settings.loop            = false;
			converter.settings.allocate        = false;
			converter.settings.maxTarget       = false;

			converter.settings.origin.buffer      = (*snd[0]).file.buf;
			converter.settings.origin.size        = (*snd[0]).file.count;
			converter.settings.origin.samples     = 0;
			converter.settings.origin.time        = 0.f;
			converter.settings.origin.usetime     = false;
			converter.settings.origin.BE          = false;
			converter.settings.origin.user_signed = false;
			converter.settings.origin.auto_signed = true;

			converter.settings.target.buffer      = storage_1s.buf;
			converter.settings.target.size        = storage_1s.count;
			converter.settings.target.samples     = 0;
			converter.settings.target.time        = 1.f;
			converter.settings.target.usetime     = true;
			converter.settings.target.BE          = false;
			converter.settings.target.user_signed = false;
			converter.settings.target.auto_signed = true;

			uint32 outbytes;
			uint32 result = converter.Run();
			uint32 seconds = 0;

			if (result == ePCMConvStatus_Normal)
			{
				seconds++;

				outbytes = converter.settings.outInfo.target_samples * size_f2b;
				memcpy(storage.buf + storage_filled, storage_1s.buf, outbytes);
				storage_filled += outbytes;

				uint32 total_frames = converter.settings.outInfo.target_samples;
				printf("\nOut frames : %i", total_frames);

				while (result == ePCMConvStatus_Normal)
				{
					converter.settings.origin.samples += converter.settings.outInfo.origin_samples;

					result = converter.Run();

					if (result != ePCMConvStatus_Normal)
					{
						if (result != ePCMConvStatus_EndOfOrigin)
							printf("\nConverter ERROR %i", result);
						break;
					}

					outbytes = converter.settings.outInfo.target_samples * size_f2b;
					memcpy(storage.buf + storage_filled, storage_1s.buf, outbytes);
					storage_filled += outbytes;

					total_frames += converter.settings.outInfo.target_samples;
					printf("\nOut frames : %i ( + %i )", total_frames, converter.settings.outInfo.target_samples);

					seconds++;

					//if (seconds == 3) break;
				}

				(*snd[0]).file.Close();

				(*snd[0]).file.buf   = storage.buf;
				(*snd[0]).file.count = storage_filled;

				(*snd[0]).snddesc->nChannels      = snddesc_target.nChannels;
				(*snd[0]).snddesc->nSamplesPerSec = snddesc_target.nSamplesPerSec;
				(*snd[0]).snddesc->wBitsPerSample = snddesc_target.wBitsPerSample;
				(*snd[0]).snddesc->wFormatTag     = snddesc_target.wFormatTag;

				SaveSound(0, DIRECTORY_GAME \
					L"\\data\\sounds", L"11111111111.wav");

				(*snd[0]).file.buf   = nullptr;
				(*snd[0]).file.count = 0;
			}
			else printf("\n[INIT] Converter ERROR %i", result);
		}

		//>> 
		void __ConvertWavTest3()
		{
			SNDDESC snddesc_origin;
			SNDDESC snddesc_target;

			snddesc_origin.nChannels      = 1;
			snddesc_origin.nSamplesPerSec = 5;
			snddesc_origin.wBitsPerSample = 32;
			snddesc_origin.wFormatTag     = WAVE_FORMAT_PCM;

			snddesc_target.nChannels      = 1;
			snddesc_target.nSamplesPerSec = 10;
			snddesc_target.wBitsPerSample = 32;
			snddesc_target.wFormatTag     = WAVE_FORMAT_PCM;

			float seconds = 3.f;

			uint32 input_elements = snddesc_origin.nSamplesPerSec * seconds; 
			TBUFFER <uint32, uint32> input (input_elements);

			uint32 output_elements = snddesc_target.nSamplesPerSec * seconds;
			TBUFFER <uint32, uint32> output (output_elements);
			
			for (int i = 0; i < input_elements; i++)
				input[i] = i * 100;
			output.Clear();

			CPCMConverter converter;

			converter.origin = & snddesc_origin;
			converter.target = & snddesc_target;

			converter.settings.loop            = false;
			converter.settings.allocate        = false;
			converter.settings.maxTarget       = false;

			converter.settings.origin.buffer      = input.buf;
			converter.settings.origin.size        = input.count * (snddesc_origin.wBitsPerSample / 8);
			converter.settings.origin.samples     = 0;
			converter.settings.origin.time        = 0.f;
			converter.settings.origin.usetime     = false;
			converter.settings.origin.BE          = false;
			converter.settings.origin.user_signed = false;
			converter.settings.origin.auto_signed = true;

			converter.settings.target.buffer      = output.buf;
			converter.settings.target.size        = output.count * (snddesc_target.wBitsPerSample / 8);
			converter.settings.target.samples     = 0;
			converter.settings.target.time        = seconds; //0.33f;
			converter.settings.target.usetime     = true;
			converter.settings.target.BE          = false;
			converter.settings.target.user_signed = false;
			converter.settings.target.auto_signed = true;

			uint32 result = converter.Run();

			printf("\nresult = %i\n", result);

			for (int i = 0; i < input_elements; i++)
				printf("\ninput[%2i] = %i", i, input[i]);
			printf("\n");
			for (int i = 0; i < output_elements; i++)
				printf("\noutput[%2i] = %i", i, output[i]);
		}
		//*/
	
	/*
		//>> 
		void __ConvertWavTestVOLUME()
		{
			uint64_gid GID;
			Load(DIRECTORY_GAME, L"a-ha - Take On Me.wav", GID._u64);

			SNDDESC snddesc_target;

			snddesc_target.nChannels      = 2;
			snddesc_target.nSamplesPerSec = 44100;
			snddesc_target.wBitsPerSample = 32;
			snddesc_target.wFormatTag     = WAVE_FORMAT_IEEE_FLOAT; // WAVE_FORMAT_PCM ; WAVE_FORMAT_IEEE_FLOAT

			CPCMConverter converter;
			uint32 snd_id = _SoundID(GID);

			float volume[] = {	0.5f, // 1
								0.5f, // 2
								1.f, // 3
								1.f, // 4
								1.f, // 5
								1.f, // 6
								1.f, // 7
								1.f  // 8
			};

			converter.origin = (*snd[snd_id]).snddesc;
			converter.target = & snddesc_target;

			converter.settings.loop            = false;
			converter.settings.allocate        = true;
			converter.settings.maxTarget       = false;

			converter.settings.origin.buffer      = (*snd[snd_id]).file.buf;
			converter.settings.origin.size        = (*snd[snd_id]).file.count;
			converter.settings.origin.samples     = 0;
			converter.settings.origin.time        = 0;
			converter.settings.origin.usetime     = true;
			converter.settings.origin.BE          = false;
		//	converter.settings.origin.user_signed = false;
			converter.settings.origin.auto_signed = true;
			converter.settings.origin.volume      = volume;

			converter.settings.target.buffer      = nullptr;
			converter.settings.target.size        = 0;
			converter.settings.target.samples     = 0;
			converter.settings.target.time        = 60.f;
			converter.settings.target.usetime     = true;
			converter.settings.target.BE          = false;
		//	converter.settings.target.user_signed = false;
			converter.settings.target.auto_signed = true;

			uint32 result = converter.Run();

			if (result == ePCMConvStatus_Normal)
			{
				(*snd[snd_id]).file.Close();

				(*snd[snd_id]).file.buf   = (byte*) converter.settings.target.buffer;
				(*snd[snd_id]).file.count = converter.settings.target.size;
			//	(*snd[0]).file.count = converter.settings.outInfo.target_samples * size_f2b;

			//	printf("\nOut frames : %i", converter.settings.outInfo.target_samples);

			//	uint32 chunk       = converter.target->nSamplesPerSec;
			//	uint32 chunk_count = converter.settings.outInfo.target_samples / chunk;
			//	uint32 chunk_add   = converter.settings.outInfo.target_samples - chunk * chunk_count;

			//	printf(" = %i x %i + %i", chunk, chunk_count, chunk_add);


				(*snd[snd_id]).snddesc->nChannels      = snddesc_target.nChannels;
				(*snd[snd_id]).snddesc->nSamplesPerSec = snddesc_target.nSamplesPerSec;
				(*snd[snd_id]).snddesc->wBitsPerSample = snddesc_target.wBitsPerSample;
				(*snd[snd_id]).snddesc->wFormatTag     = snddesc_target.wFormatTag;

				SaveSound(snd_id, DIRECTORY_GAME \
					L"\\data\\sounds", L"___TEST_VOL.wav");

				(*snd[snd_id]).file.buf   = nullptr;
				(*snd[snd_id]).file.count = 0;
			}
			else printf("\nConverter ERROR %i", result);
		} //*/

	/*
		//>> 
		void __MixerWavTest()
		{
			uint64 GID[3] = { 0, 0, 0 };
			uint32 ID0 = 0;
			uint32 ID1 = 0;
			uint32 ID2 = 0;

			Load(DIRECTORY_GAME, L"sample_1.wav", GID[0]);
			Load(DIRECTORY_GAME, L"loop-01.wav", GID[1]);
			Load(DIRECTORY_GAME, L"sample_2.wav", GID[2]);

			ID0 = _SoundID(((uint64_gid)GID[0]));
			ID1 = _SoundID(((uint64_gid)GID[1]));
			ID2 = _SoundID(((uint64_gid)GID[2]));

			SNDDESC snddesc_target;

			snddesc_target.nChannels      = 2;
			snddesc_target.nSamplesPerSec = 48000;
			snddesc_target.wBitsPerSample = 24;
			snddesc_target.wFormatTag     = WAVE_FORMAT_PCM;

			TBUFFER <byte, uint32> outdata;

			float steptime[] = { 5.f, 3.f, 7.f, 0.f };
			float stepsum = steptime[0] + steptime[1] + steptime[2];

			outdata.Create ( (snddesc_target.wBitsPerSample / 8) * 
			                 snddesc_target.nChannels *
							 snddesc_target.nSamplesPerSec * 
							 stepsum
			);

			byte * outdata_pos = outdata.buf;

			CPCMMixer mixer;

			mixer.target = & snddesc_target;
		//	mixer.origin = snd[ID0]->snddesc;

			mixer.settings.allocate  = true;
			mixer.settings.maxTarget = false;

		//	mixer.settings.loop     = false;
		//	mixer.settings.origin.buffer  = snd[ID0]->file.buf;
		//	mixer.settings.origin.size    = snd[ID0]->file.count;
		//	mixer.settings.origin.samples = 0;
		//	mixer.settings.origin.usetime = false;

			mixer.settings.target.buffer  = nullptr;
			mixer.settings.target.size    = 0;
			mixer.settings.target.time    = steptime[0];
			mixer.settings.target.usetime = true;

			SoundMixSource source1, source2, source3;

			source1.origin                  = snd[ID0]->snddesc;
			source1.settings_loop           = false;
			source1.settings_origin.buffer  = snd[ID0]->file.buf;
			source1.settings_origin.size    = snd[ID0]->file.count;
			source1.settings_origin.samples = 0;
			source1.settings_origin.usetime = false;

			source2.origin                  = snd[ID1]->snddesc;
			source2.settings_loop           = true;
			source2.settings_origin.buffer  = snd[ID1]->file.buf;
			source2.settings_origin.size    = snd[ID1]->file.count;
			source2.settings_origin.samples = 0;
			source2.settings_origin.usetime = false;

			source3.origin                  = snd[ID2]->snddesc;
			source3.settings_loop           = false;
			source3.settings_origin.buffer  = snd[ID2]->file.buf;
			source3.settings_origin.size    = snd[ID2]->file.count;
			source3.settings_origin.samples = 0;
			source3.settings_origin.usetime = false;

			for (int steps = 0; steps < 3; steps++)
			{
			//	mixer.Run(&source2);
			//	mixer.Run(&source2, &source3);
				mixer.AutoRun( { &source1, &source3, &source2 } , steps + 1 );

				/////////////////////////////////////// сохр. результат

				memcpy ( outdata_pos,
					    mixer.settings.target.buffer, 
						mixer.settings.target.size );

				outdata_pos += mixer.settings.target.size;

				/////////////////////////////////////// след. цикл

			//	mixer.settings.origin.buffer = 0;    220500  132300  308700
			//	mixer.settings.origin.size   = 0;

			//	mixer.settings.origin.samples   += mixer.settings.outInfo.origin_samples;
				source1.settings_origin.samples += mixer.settings.outInfo.origin_samples;
				source2.settings_origin.samples += mixer.settings.outInfo.origin_samples;
				source3.settings_origin.samples += mixer.settings.outInfo.origin_samples;

				mixer.settings.target.time = steptime[steps + 1];
			}

			// сформировать *.wav

			snd[0]->Close();
			snd[0]->type = eSoundType_WAVE;			
			(*snd[0]->snddesc) = snddesc_target;
			snd[0]->file.buf   = outdata.buf;
			snd[0]->file.count = outdata.count;
			outdata.buf   = nullptr;
			outdata.count = 0;

			SaveSound(0, DIRECTORY_GAME \
				L"\\data\\sounds", L"11111111111.wav");

			printf("\nMixerWavTest() - done !");
		}
		//*/

		#undef MASTER_VOLUME_DEFAULT
		#undef _SoundID
		#undef ALLOWED_NUM_OF_COMMON_INTERFACES
	};
}

#endif // _SOUND_H