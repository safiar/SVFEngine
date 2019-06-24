// ----------------------------------------------------------------------- //
//
// MODULE  : SoundWAS.h
//
// PURPOSE : Воспроизведение звука средствами Windows Core Audio APIs
//			 - Multimedia Device (MMDevice) API
//           - Windows Audio Session API (WASAPI)
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _SOUNDWAS_H
#define _SOUNDWAS_H

// Заметки для себя
// * Данные snd[id] можно безопасно забирать. Удаления не произойдёт, пока (snd[id].a_players != 0)  ?  ИЗМ. не актуально
// * Если во время работы физически разъединить выход (наушники):
//    1) Сначала выпадет StateChanged_proc() девайса с Active на Unplugged
//       ИЗМЕНЕНИЯ - изменится переменная <state> устройства
//    2) Затем 3 раза DefaultDeviceChanged_proc() по каждой из 3 ролей, но я ловлю только 1 роль
//       ИЗМЕНЕНИЯ - изменится текущий запомненный номер N устройства по умолчанию
//       (!) здесь вызову DefaulDeviceChanged() и перезагружу менеджер сессии
// * CallerGet - malloc error ?  ИЗМ. м-м... это где? забыл
// * вывод явной ошибки при nChannels > 8  ?  ИЗМ. в смысле в профиле аудиокарты? вроде это имел ввиду
// * eSoundRunStopAtStart / eSoundRunStopAtEnd  ?  ИЗМ. доделал!
// * LoadAsCopyOf() сделать  ?  ИЗМ. доделал!
// * Собственное микширование и слив всего через одного менеджера                                        ?  ИЗМ. доделал!
//   Лучше не писать отдельный Play, а сделать доп. процедуру, производящую конкретные данные звука SND  ?  ИЗМ. доделал!

#include "SoundIMM.h"

#define IAudioClientF IAudioClient // Win8: IAudioClient2 ... Win10: IAudioClient3

// ЗАМЕТКА : можно обратиться к CSystem и узнать текущую платформу
// IAudioClient2::GetBufferSizeLimits()  - returns the buffer size limits of the hardware audio engine in 100-nanosecond units
// IAudioClient2::SetClientProperties() - can set AUDIO_STREAM_CATEGORY (game music, game effects, communication, etc.)
// IAudioClient2::IsOffloadCapable()
// IAudioClient3::GetCurrentSharedModeEnginePeriod()
// IAudioClient3::GetSharedModeEnginePeriod()
// IAudioClient3::InitializeSharedAudioStream()

//_del!_ резервирование мест и добавление в любую зарезервированную позицию 
//_DONE_ спрятать вектора pRenderClient в оболочку
//_DONE_ конвертация, микширование и др. исходников под конкретное аудиоустройство
//_DONE_ слежение за состоянием аудиоустройств (доделать класс CMMNotificationClient)
//_DONE_ пауза воспроизведения
//_DONE_ перемотка до определённой временной точки
//_DONE_ имитация 3d звука из определённой точки сцены относительно игрока (всё нужное теперь есть, дело за движком)
//_DONE_ ожидания на события потока
//_DONE_ работа по спискам / загрузка списков / управление списком / общая громкость группы
//_DONE_ своё микширование
//_DONE_ поддержка *.mp3 - MPEG-1 Layer-3 format
// TODO: поддержка *.ogg - Ogg Vorbis format
// TODO: поддержка *.aac - Advanced Audio Coding format
// TODO: всё по микрофону

#define REFTIMES_PER_SEC	        10000000	// 100-nanosecond units per sec
#define REFTIMES_PER_MILLISEC       10000		// 100-nanosecond units per millisec
#define REFTIMES_AUDIO_ORDER_MS     1000        // (time in ms) requested buffer for every sound session
#define REFTIMES_AUDIO_ORDER        (REFTIMES_AUDIO_ORDER_MS * REFTIMES_PER_MILLISEC) // (time in units) requested buffer for every sound session
#define REFTIMES_AUDIO_ORDER_MS_MIN  MIXER_PLAYBUFTIME  // (time in ms) min. buffer timesize

#define INIT_STUCK_TIME_MS    10000     // wait 10 seconds before aborting if IAudioClient::Initialize() is stuck [hardware problems]

namespace SAVFGAME
{
	class CAudioSessionEvents : public IAudioSessionEvents
	{
		LONG _cRef;

	public:
		CAudioSessionEvents() : _cRef(1) { };
		~CAudioSessionEvents() { };

		//>> IUnknown method
		ULONG STDMETHODCALLTYPE AddRef() override final
		{
			return InterlockedIncrement(&_cRef);
		}
		//>> IUnknown method
		ULONG STDMETHODCALLTYPE Release() override final
		{
			ULONG ulRef = InterlockedDecrement(&_cRef);
			if (0 == ulRef) { delete this; }
			return ulRef;
		}
		//>> IUnknown method
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface) override final
		{
			if (IID_IUnknown == riid)
			{
				AddRef();
				*ppvInterface = (IUnknown*)this;
			}
			else if (__uuidof(IAudioSessionEvents) == riid)
			{
				AddRef();
				*ppvInterface = (IAudioSessionEvents*)this;
			}
			else
			{
				*ppvInterface = NULL;
				return E_NOINTERFACE;
			}
			return S_OK;
		}

		//>> ...
		HRESULT STDMETHODCALLTYPE OnDisplayNameChanged(LPCWSTR NewDisplayName, LPCGUID EventContext) override final
		{
			//wprintf(L"\n[IAudioSessionEvents] Display Name Changed");

			return S_OK;
		};
		//>> ...
		HRESULT STDMETHODCALLTYPE OnIconPathChanged(LPCWSTR NewIconPath, LPCGUID EventContext) override final
		{
			//wprintf(L"\n[IAudioSessionEvents] Icon Path Changed");

			return S_OK;
		};
		//>> ...
		HRESULT STDMETHODCALLTYPE OnSimpleVolumeChanged(float NewVolume, BOOL NewMute, LPCGUID EventContext) override final
		{
			//wprintf(L"\n[IAudioSessionEvents] Simple Volume Changed");

			return S_OK;
		};
		//>> ...
		HRESULT STDMETHODCALLTYPE OnChannelVolumeChanged(DWORD ChannelCount, float NewChannelVolumeArray[], 
			DWORD ChangedChannel, LPCGUID EventContext) override final
		{
			//wprintf(L"\n[IAudioSessionEvents] Channel Volume Changed");

			return S_OK;
		};
		//>> ...
		HRESULT STDMETHODCALLTYPE OnGroupingParamChanged(LPCGUID NewGroupingParam, LPCGUID EventContext) override final
		{
			//wprintf(L"\n[IAudioSessionEvents] Grouping Param Changed");

			return S_OK;
		};
		//>> ...
		HRESULT STDMETHODCALLTYPE OnStateChanged(AudioSessionState NewState) override final
		{
			//wprintf(L"\n[IAudioSessionEvents] State Changed");

			return S_OK;
		};
		//>> ...
		HRESULT STDMETHODCALLTYPE OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason) override final
		{
			//wprintf(L"\n[IAudioSessionEvents] Session Disconnected");

			return S_OK;
		};

	};

	class CAudioSessionManager
	{
	public:
		struct ManageClient
		{
		friend class CAudioSessionManager;

			struct ManageClientData
			{
				struct ManageRunSettings
				{
					CSound::MIXERDATA::THREADSHARED * mixer_p { nullptr };
					CSound::SNDCALL * caller        { nullptr };
					void *            actual_sound  { nullptr };
					bool              loop          { false };
					bool              stop_at_start { false };
					bool              stop_at_end   { false };
					bool              mixer         { false };
				};
				struct ManageConverter
				{
				private:
					CPCMConverter     __pcm;
					CMP3Converter     __mp3;
				public:
					TBUFFER <byte, uint32>	window;          // окно для конвертирования
					CPCMConverter * const   pcm { & __pcm }; // прозводит преобразование .wav
					CMP3Converter * const   mp3 { & __mp3 }; // прозводит преобразование .mp3 в .wav
					void * const            ogg { nullptr }; // RESERVED
					void * const            aac { nullptr }; // RESERVED		
				};

				ManageConverter			conv;           // содержит конверторы
				ManageRunSettings		set;            // содержит запомненные настройки запуска
				IAudioClientF *			p  { nullptr }; // содержит запомненный базовый интерфейс
				IAudioRenderClient *	r  { nullptr }; // содержит запомненный контроль буфера (render)
				IAudioCaptureClient *	c  { nullptr }; // содержит запомненный контроль буфера (capture)
				IAudioStreamVolume *	v  { nullptr }; // содержит запомненный контроль уровня звучания
			};

		public:
			ManageClientData * const	out { &_ };		// данные к выдаче

		private:
			ManageClientData			_ ;			    // данные к выдаче
			IAudioClientF *				p  { nullptr }; // базовый интерфейс
			IAudioRenderClient *		r  { nullptr }; // контроль буфера (render)
			IAudioCaptureClient *		c  { nullptr }; // контроль буфера (capture)
			IAudioStreamVolume *		v  { nullptr }; // контроль уровня звучания
			atomic<bool>				m  { 0 };       // контроль доступа потокам         

		private:
			bool InitAudioRenderClient()
			{
				if (p == nullptr) return false; // ERROR потерялся базовый интерфейс
				if (r != nullptr) return true;  // ERROR уже создано

				if (S_OK != p->GetService(__uuidof(IAudioRenderClient), (void**)&r))
				{
					_MBM(L"<Sound WASAPI> Не удалось GetService() IAudioRenderClient");
					return false;
				}

				return true;
			}
			bool InitAudioCaptureClient()
			{
				if (p == nullptr) return false; // ERROR потерялся базовый интерфейс
				if (c != nullptr) return true;  // уже создано

				if (S_OK != p->GetService(__uuidof(IAudioCaptureClient), (void**)&c))
				{
					_MBM(L"<Sound WASAPI> Не удалось GetService() IAudioCaptureClient");
					return false;
				}

				return true;
			}
			bool InitAudioStreamVolume()
			{
				if (p == nullptr) return false; // ERROR потерялся базовый интерфейс
				if (v != nullptr) return true;  // ERROR уже создано

				if (S_OK != p->GetService(__uuidof(IAudioStreamVolume), (void**)&v))
				{
					_MBM(L"<Sound WASAPI> Не удалось GetService() IAudioStreamVolume");
					return false;
				}

				return true;
			}
			bool InitServices(eSoundDevType type)
			{
				switch (type)
				{
				case eSDTypeRender: 
					if (!InitAudioRenderClient()) return false;
					break;
				case eSDTypeCapture: 
					if (!InitAudioCaptureClient()) return false;
					break;
				}
					
				if (!InitAudioStreamVolume()) return false;

				return true;
			}
			void Release()
			{
				_RELEASE(v);
				_RELEASE(c);
				_RELEASE(r);
				_RELEASE(p);
			}
			void UpdateOut()
			{
				_.c = c;
				_.p = p;
				_.r = r;
				_.v = v;
			}
		public:
			//>> Определяет клиента как занятого
			void Lock() { _RLX_1(m); UpdateOut(); }
			//>> Определяет клиента как доступного
			void Unlock() { _RLX_0(m); }
			//>> Проверяет доступность клиента
			bool CheckLock() { return _RLX_LD(m); }
		};

	private:
		CSoundIMM *		MM;			// initial param (never changes)
		eSoundDevType	type;		// initial param (never changes)
		uint32			client_max;	// initial param (never changes)
		SNDDESC         __mode;

	private:
		THREADPOOL	terminator;		// защита от застревания
		mutex		m_terminate;	// защита от застревания
		cvariable	cv_terminate;	// защита от застревания
		bool		terminate;		// защита от застревания

	private:
		GUID							sessionID;
		IAudioSessionManager  *			manager  { nullptr }; // not used for now
		IAudioSessionManager2 *			manager2 { nullptr }; // not used for now
		IAudioSessionControl  *			control  { nullptr }; // not used for now
		IAudioSessionControl2 *			control2 { nullptr }; // not used for now
		CAudioSessionEvents   *			events   { nullptr }; // not used for now
		ISimpleAudioVolume    *			volume   { nullptr }; // not used for now
	public: 
		TBUFFER <ManageClient, uint32>	client;			  // набор клиентов не больше <client_max>
		SNDDESC	* const					mode { &__mode }; // параметры .wav в "shared mode" аудиокарты
	private:
		uint64			devicePeriod_default;	// REFTIMES in 100-nanosecond units
		uint64			devicePeriod_minimum;	// REFTIMES in 100-nanosecond units
		uint64			devicePeriod;			// requested hnsBufferDuration (default = REFTIMES_AUDIO_ORDER)
	private: 
		IMMDevice *		device  { nullptr }; // запомненный ptr на device
		byte			clcount { 0 };		 // текущее количество инициализированных клиентов
		bool			isInit  { 0 };

		// TODO : device & mode изменились по ходу работы - как узнать, что делать ?
		// p, r, c, v надо будет перезагружать -- надо тогда запретить напрямую брать их в PlayAsync() -> создать процедуру взятия !
		// mode доступен напрямую - тоже надо давать только копию -> создать процедуру взятия !

		// ЗАМЕТКА : client'ы даны напрямую как объекты, а не через VECDATAP,
		// в каждом клиенте есть atomic<bool>, а значит их двигать в памяти нельзя,
		// но нам и не нужно, т.к. мы сразу создаём их в кол-ве maxthreads

	private:
		void StuckInit(int N, IAudioClientF * p, AUDCLNT_SHAREMODE audclnt_sharemode, HRESULT * hRes)
		{
			// AUDCLNT_STREAMFLAGS_XXX
			// AUDCLNT_STREAMFLAGS_CROSSPROCESS - more than 1 process (2 or more apps with same sessionID)
			// AUDCLNT_STREAMFLAGS_LOOPBACK - copy render to capture (only on a rendering endpoint with a shared-mode stream)
			// AUDCLNT_STREAMFLAGS_EVENTCALLBACK - event-driven buffering (mostly exclusive mode ?)
			// AUDCLNT_STREAMFLAGS_NOPERSIST - volume and mute states are not save after end of session (flag for render)
			// AUDCLNT_STREAMFLAGS_RATEADJUST - enables to get ref to IAudioClockAdjustment interface to set the sample rate for the stream
			// AUDCLNT_SESSIONFLAGS_EXPIREWHENUNOWNED - session expires when there are no associated streams
			// AUDCLNT_SESSIONFLAGS_DISPLAY_HIDE - volume control is hidden in the volume mixer user interface 
			// AUDCLNT_SESSIONFLAGS_DISPLAY_HIDEWHENEXPIRED - volume control is hidden in the volume mixer user interface after the session expires

			// ERR = AUDCLNT_E_NOT_INITIALIZED					AUDCLNT_ERR(0x001)  (0x88890001)
			// ERR = AUDCLNT_E_ALREADY_INITIALIZED				AUDCLNT_ERR(0x002)  (0x88890002)
			// The IAudioClient object is already initialized.
			// ERR = AUDCLNT_E_WRONG_ENDPOINT_TYPE				AUDCLNT_ERR(0x003)
			// The AUDCLNT_STREAMFLAGS_LOOPBACK flag is set but the endpoint device is a capture device, not a rendering device.
			// ERR = AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED			AUDCLNT_ERR(0x019)
			// The requested buffer size is not aligned. This code can be returned for a render or a capture device if the caller
			// specified AUDCLNT_SHAREMODE_EXCLUSIVE and the AUDCLNT_STREAMFLAGS_EVENTCALLBACK flags. The caller must call
			// Initialize again with the aligned buffer size.
			// ERR = AUDCLNT_E_BUFFER_SIZE_ERROR				AUDCLNT_ERR(0x016)
			// Indicates that the buffer duration value requested by an exclusive-mode client is out of range.
			// The requested duration value for pull mode must not be greater than 500 milliseconds; for push mode
			// the duration value must not be greater than 2 seconds.
			// ERR = AUDCLNT_E_DEVICE_INVALIDATED				AUDCLNT_ERR(0x004)
			// The audio endpoint device has been unplugged, or the audio hardware or associated hardware resources
			// have been reconfigured, disabled, removed, or otherwise made unavailable for use.
			// ERR = AUDCLNT_E_CPUUSAGE_EXCEEDED				AUDCLNT_ERR(0x017)
			// Indicates that the process-pass duration exceeded the maximum CPU usage.
			// The audio engine keeps track of CPU usage by maintaining the number of times the process-pass
			// duration exceeds the maximum CPU usage. The maximum CPU usage is calculated as a percent of
			// the engine's periodicity. The percentage value is the system's CPU throttle value (within the
			// range of 10% and 90%). If this value is not found, then the default value of 40% is used to calculate the maximum CPU usage.
			// ERR = AUDCLNT_E_DEVICE_IN_USE					AUDCLNT_ERR(0x00a)
			// The endpoint device is already in use. Either the device is being used in exclusive mode, or the
			// device is being used in shared mode and the caller asked to use the device in exclusive mode.
			// ERR = AUDCLNT_E_ENDPOINT_CREATE_FAILED			AUDCLNT_ERR(0x00f)
			// The method failed to create the audio endpoint for the render or the capture device.
			// This can occur if the audio endpoint device has been unplugged, or the audio hardware or
			// associated hardware resources have been reconfigured, disabled, removed, or otherwise made unavailable for use.
			// ERR = AUDCLNT_E_INVALID_DEVICE_PERIOD			AUDCLNT_ERR(0x020)
			// Indicates that the device period requested by an exclusive-mode client is greater than 500 milliseconds.
			// ERR = AUDCLNT_E_UNSUPPORTED_FORMAT				AUDCLNT_ERR(0x008)
			// The audio engine (shared mode) or audio endpoint device (exclusive mode) does not support the specified format.
			// ERR = AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED		AUDCLNT_ERR(0x00e)
			// The caller is requesting exclusive-mode use of the endpoint device, but the user has disabled exclusive-mode use of the device.
			// ERR = AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL		AUDCLNT_ERR(0x013)
			// The AUDCLNT_STREAMFLAGS_EVENTCALLBACK flag is set but parameters hnsBufferDuration and hnsPeriodicity are not equal.
			// ERR = AUDCLNT_E_SERVICE_NOT_RUNNING				AUDCLNT_ERR(0x010)
			// The Windows audio service is not running.
			// ERR = E_POINTER									_HRESULT_TYPEDEF_(0x80004003L)
			// Parameter pFormat is NULL.
			// ERR = E_INVALIDARG								_HRESULT_TYPEDEF_(0x80070057L)
			// Parameter pFormat points to an invalid format description;
			// or the AUDCLNT_STREAMFLAGS_LOOPBACK flag is set but ShareMode is not equal to AUDCLNT_SHAREMODE_SHARED;
			// or the AUDCLNT_STREAMFLAGS_CROSSPROCESS flag is set but ShareMode is equal to AUDCLNT_SHAREMODE_EXCLUSIVE.
			// A prior call to IAudioClient2::SetClientProperties was made with an invalid category for audio / render streams.
			// ERR = E_OUTOFMEMORY								_HRESULT_TYPEDEF_(0x8007000EL)
			// Out of memory. (possibly num_clients * REFTIMES_AUDIO_ORDER overflow)

			(*hRes) = p->Initialize (
				audclnt_sharemode,		// AUDCLNT_SHAREMODE_XXX: _SHARED / _EXCLUSIVE
				0,						// AUDCLNT_STREAMFLAGS_XXX
				devicePeriod,			// заказать буфер
				0,						// (hnsPeriodicity) всегда 0 (shared mode)
				(WAVEFORMATEX*) mode,	// 
				& sessionID );			// NULL / DEFINE_GUIDNAMED(GUID_NULL)

			if (N == 0)
			if ((*hRes) == E_INVALIDARG || // 0x80070057
				(*hRes) == AUDCLNT_E_UNSUPPORTED_FORMAT)		
			{
				_RELEASE(p);

				REFIID IID_IAudioClientF = __uuidof(std::remove_reference<decltype(p)>);

				HRESULT hRes_;
				if (S_OK == (hRes_ = device->Activate(IID_IAudioClientF, CLSCTX_INPROC, NULL, (void**)& p)))
				{				
					WAVEFORMATEX * mixmode;
					if (S_OK == (hRes_ = p->GetMixFormat(&mixmode)))
					{
						mixmode->cbSize = 0;
						switch (mixmode->wFormatTag)
						{
						case WAVE_FORMAT_EXTENSIBLE:
						case WAVE_FORMAT_PCM:
						{
							mixmode->wFormatTag = WAVE_FORMAT_PCM;

							hRes_ = p->Initialize(
								audclnt_sharemode,		// AUDCLNT_SHAREMODE_XXX: _SHARED / _EXCLUSIVE
								0,						// AUDCLNT_STREAMFLAGS_XXX
								devicePeriod,			// заказать буфер
								0,						// всегда 0 (shared mode)
								(WAVEFORMATEX*)mixmode,	// 
								& sessionID  );			// NULL / DEFINE_GUIDNAMED(GUID_NULL)

							if (hRes_ == S_OK)
							{
								mode->nChannels       = mixmode->nChannels;
								mode->nSamplesPerSec  = mixmode->nSamplesPerSec;
								mode->wBitsPerSample  = mixmode->wBitsPerSample;
								mode->nAvgBytesPerSec = mixmode->nAvgBytesPerSec;
								mode->nBlockAlign     = mixmode->nBlockAlign;

								(*hRes) = S_OK;
							}

							break;
						}
						default:
							break;
						}

						CoTaskMemFree(mixmode);
					}
				}

				if (hRes_ != S_OK)
				{
					if (hRes_ == E_INVALIDARG) // antivirus block / local admin block ?
					{
						switch (type)
						{
						case eSDTypeRender:
							wprintf(L"\nSomething block initialize of IAudioClient [render]");
							break;
						case eSDTypeCapture:
							wprintf(L"\nSomething block initialize of IAudioClient [capture]");
							break;
						}
					}				
				}
			}

			{
				glock lock (m_terminate);
				terminate = false;
				_CV_ALL(cv_terminate);
			}
		}
		bool StuckManager()
		{
			auto  time_point   = _TIME;
			int32 sleep_time   = INIT_STUCK_TIME_MS;
			bool  do_terminate = false;

			{
				ulock sleep_lock (m_terminate);

				while (terminate) // sleep loop
				{
					cvstate timeout = _CV_FOR(cv_terminate, sleep_lock, timems(sleep_time));

					if (timeout == _CV_NO_TIMEOUT)
					{
						if (terminate == false)
						{
							break;
						}
						else // fake awakening, try continue sleep
						{
							sleep_time = (int32)INIT_STUCK_TIME_MS - (int32)_TIMER(time_point);
							if (sleep_time <= 0)
								break;
						}
					}
					else // _CV_TIMEOUT
						break;
				}

				do_terminate = terminate;
			}

			return do_terminate;
		}
		bool InitSessionID(eSoundDevType type)
		{
			// одинаковый ID сессии для render и capture == нормально ?

			switch (type)
			{
			case eSDTypeCapture:
			case eSDTypeRender:
				{
					sessionID.Data1 = 0xA892EF00;
					sessionID.Data2 = 0xA892;
					sessionID.Data3 = 0xEF00;
					for (auto & b : sessionID.Data4) b = 0x11;
					break;
				}
			default:
				return false;
			}

			return true;
		}
		bool InitSession()
		{
			///// manager & manager 2 /////

			if (S_OK != device->Activate(__uuidof(IAudioSessionManager), CLSCTX_INPROC, NULL, (void**) &manager))
			{
				_MBM(L"<Sound WASAPI> Не удалось Activate() session manager");
				return false;
			}
			if (S_OK != device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC, NULL, (void**) &manager2))
			{
				_MBM(L"<Sound WASAPI> Не удалось Activate() session manager 2");
				return false;
			}

			///// control & control 2 /////

			if (S_OK != manager->GetAudioSessionControl(&sessionID, 0, &control))
			{
				_MBM(L"<Sound WASAPI> Не удалось GetAudioSessionControl()");
				return false;
			}
			control->QueryInterface(__uuidof(IAudioSessionControl2), (void**) &control2);

			///// simple volume /////

			if (S_OK != manager->GetSimpleAudioVolume(&sessionID, 0, &volume))
			{
				_MBM(L"<Sound WASAPI> Не удалось GetSimpleAudioVolume()");
				return false;
			}

			///// events registration /////

			events = new /*_NEW_NOTHROW*/ CAudioSessionEvents;

			if (events == nullptr) // malloc ERROR
				return false;

			if (S_OK != control->RegisterAudioSessionNotification(events))
			{
				_MBM(L"<Sound WASAPI> Не удалось RegisterAudioSessionNotification()");
				return false;
			}

			return true;
		}
		bool InitClients()
		{                             // client_max
			for (clcount = 0; clcount < client.count; clcount++)
			{
				bool error    = false;
				bool exit     = false;
				int  try_more = 50;

				while (!exit)
				{
					HRESULT hRes = S_OK;
					if (!InitClient(clcount, hRes))
					{
						if (hRes == AUDCLNT_E_CPUUSAGE_EXCEEDED && try_more) // 0x88890017 
						{
							client[clcount].Release(); // RETRY
							try_more--;                // .
							continue;                  // .
						}
						else
							error = true;
					}
					exit = true;
				}
				//printf("\n%i", 50 - try_more);
				if (error) break;				
			}

			if (clcount != client.count) // num existing != client_max
			{
				if (type == eSDTypeCapture) return false;
				else // eSDTypeRender
				{
					if ((clcount - (1 + CAPTURE_THREADS)) < RENDER_THREADS_MIN) // + reserve some for capture client (CAPTURE_THREADS)
						return false;
					else // release ~10% of clients
					{
						int to_release = (clcount - (1 + CAPTURE_THREADS)) / 10;
						if ((clcount - (1 + CAPTURE_THREADS) - to_release) < RENDER_THREADS_MIN)
							to_release -= RENDER_THREADS_MIN - (clcount - (1 + CAPTURE_THREADS) - to_release);
						to_release += 1 + CAPTURE_THREADS;
						for (int i = 0; i < to_release; i++)
							client[clcount--].Release();
					}
				}
			}

			return true;
		}
		bool InitClient(int N, HRESULT & hRes)
		{
			auto & rc = client[N];

			AUDCLNT_SHAREMODE audclnt_sharemode = AUDCLNT_SHAREMODE_SHARED;

			REFIID IID_IAudioClientF = __uuidof(std::remove_reference<decltype(rc.p)>);

			if (S_OK != device->Activate(IID_IAudioClientF, CLSCTX_INPROC, NULL, (void**)&rc.p))
			{
				_MBM(L"<Sound WASAPI> Не удалось Activate() pAudioClient");
				return false;
			}

			if (N == 0)
			{
				WAVEFORMATEX * closest = nullptr;
				hRes = rc.p->IsFormatSupported(audclnt_sharemode, (WAVEFORMATEX*)mode, &closest);		
				if (hRes != S_OK)
				{
					if (closest != nullptr)
					{
						mode->nChannels       = closest->nChannels;
						mode->nSamplesPerSec  = closest->nSamplesPerSec;
						mode->wBitsPerSample  = closest->wBitsPerSample;
						mode->nAvgBytesPerSec = closest->nAvgBytesPerSec;
						mode->nBlockAlign     = closest->nBlockAlign;
					}
					else
					{
						WAVEFORMATEX * mixmode = nullptr;
						if (S_OK == rc.p->GetMixFormat(&mixmode))
						if (mixmode != nullptr)
						{
							switch (mixmode->wFormatTag)
							{
							case WAVE_FORMAT_EXTENSIBLE:
							case WAVE_FORMAT_PCM:
								mode->nChannels       = mixmode->nChannels;
								mode->nSamplesPerSec  = mixmode->nSamplesPerSec;
								mode->wBitsPerSample  = mixmode->wBitsPerSample;
								mode->nAvgBytesPerSec = mixmode->nAvgBytesPerSec;
								mode->nBlockAlign     = mixmode->nBlockAlign;
								break;
							default:
								break;
							}
							CoTaskMemFree(mixmode);
						}
					}
				}
				if (closest != nullptr) CoTaskMemFree(closest);

				REFERENCE_TIME phnsDefaultDevicePeriod;
				REFERENCE_TIME phnsMinimumDevicePeriod;
				hRes = rc.p->GetDevicePeriod(&phnsDefaultDevicePeriod, &phnsMinimumDevicePeriod);
				if (hRes == S_OK)
				{
					devicePeriod_default = (uint64) phnsDefaultDevicePeriod;
					devicePeriod_minimum = (uint64) phnsMinimumDevicePeriod;
					devicePeriod = (REFTIMES_AUDIO_ORDER >= devicePeriod_minimum) ? REFTIMES_AUDIO_ORDER : devicePeriod_minimum;
					//printf("\ndevicePeriod_default = %f ms", (float)devicePeriod_default / REFTIMES_PER_MILLISEC);
					//printf("\ndevicePeriod_minimum = %f ms", (float)devicePeriod_minimum / REFTIMES_PER_MILLISEC);
				}
				else
				{
					devicePeriod_default = 0;
					devicePeriod_minimum = 0;
					devicePeriod = REFTIMES_AUDIO_ORDER;
				}
			}
			
			uint64 T = terminator.GetTasks();
			uint64 W = terminator.GetWorkers();
			if (T >= W)
				terminator.WorkersAdd(1);

			terminate = true;

			terminator(&CAudioSessionManager::StuckInit, this, N, rc.p, audclnt_sharemode, &hRes);

			if (StuckManager())
			{
				_MBM(L"Audioclient initialize is stuck."); // Aborting..."); _ABORT;

				// попробуем продолжить с тем, что есть
				return false;
			}

			if (hRes != S_OK)
			{
				wchar_t error [MAX_PATH];
				switch (hRes)
				{
				case AUDCLNT_E_CPUUSAGE_EXCEEDED: // 0x88890017
					swprintf_s(error, L"<Sound WASAPI> Не удалось Initialize() pClient\nERR = %x\n"
						L"The audio engine fail creating new streams", (int)hRes);
					break;
				case E_OUTOFMEMORY: // 0x8007000E
					swprintf_s(error, L"<Sound WASAPI> Не удалось Initialize() pClient\nERR = %x\n"
						L"Out of memory", (int)hRes);
					break;
				case E_INVALIDARG: // 0x80070057
					swprintf_s(error, L"<Sound WASAPI> Не удалось Initialize() pClient\nERR = %x\n"
						L"Parameter pFormat points to an invalid format description", (int)hRes);
					break;
				case AUDCLNT_E_DEVICE_IN_USE:
					swprintf_s(error, L"<Sound WASAPI> Не удалось Initialize() pClient\nERR = %x\n"
						L"Can't get shared mode. Device is being used in exclusive mode.", (int)hRes);
					break;
				default:
					swprintf_s(error, L"<Sound WASAPI> Не удалось Initialize() pClient\nERR = %x", (int)hRes);
					break;
				}
				if (hRes != AUDCLNT_E_CPUUSAGE_EXCEEDED) // 0x88890017
				{
					_MBM(error);
				}		
				_RELEASE(rc.p);
				return false;
			}

			// надо дополнительно проверить размер выделенного аудиокартой буфера (в микросекундах)
			{
				uint32 audiocard_buffer_frames;  // количество кадров в буфере
				uint32 audiocard_buffer_time_ms; // время буфера в мс
			
				if (S_OK != (hRes = rc.p->GetBufferSize(&audiocard_buffer_frames)))
				{
					// hRes AUDCLNT_E_NOT_INITIALIZED     The audio stream has not been successfully initialized
					// hRes AUDCLNT_E_DEVICE_INVALIDATED  The audio endpoint device unavailable for use (eg. unplugged)
					_RELEASE(rc.p);
					return false;
				}

				audiocard_buffer_time_ms = (uint32)(1000 * ((float)audiocard_buffer_frames / mode->nSamplesPerSec));

				// пока что определил MS_MIN размер как размер необходимый работе микшера
				if (audiocard_buffer_time_ms < REFTIMES_AUDIO_ORDER_MS_MIN)
				{
					hRes = S_FALSE; // микшер не сможет работать с этим - отбросим
					return false;   // .
				}
			}
			
			return rc.InitServices(type);
		}
	public:
		bool Init()
		{
			if (isInit) return true;

			// Выделим память лишь однажды и удалим в деструкторе //

			if (client.buf == nullptr)
			{
				if (!client.Create(client_max)) // malloc ERROR
					return false;
			}

			MM->GetDevice(type, device, *mode);

			if (device == nullptr)
				return false;

			mode->cbSize = 0;
			switch (mode->wFormatTag)
			{
			case WAVE_FORMAT_EXTENSIBLE:
			case WAVE_FORMAT_PCM:
				mode->wFormatTag = WAVE_FORMAT_PCM;
				break;
			default:
				_MBM(L"<Sound WASAPI> Unexpected default render format");
			}

			if (!InitSessionID(type)) return false;
			if (!InitSession())       { Release(); return false; }
			if (!InitClients())       { Release(); return false; }					

			return isInit = true;
		}
	private:
		void ReleaseClients()
		{
			for (uint32 i = 0; i < client.count; i++)
		//	for (uint32 i = 0; i < clcount; i++)
				client[i].Release();
			clcount = 0;
		}
	public:
		void Release()
		{
			if (events)
				control->UnregisterAudioSessionNotification(events);
			ReleaseClients();
			_RELEASE(events);
			_RELEASE(volume);
			_RELEASE(control2);
			_RELEASE(control);
			_RELEASE(manager2);
			_RELEASE(manager);
			device = nullptr;

			isInit = false;
		}
	public:
		CAudioSessionManager(const CSoundIMM * pMMInterface, eSoundDevType devtype, uint32 maxthreads)
		{
			if (pMMInterface == nullptr)
				_MBM(ERROR_PointerNone);

			if (devtype != eSDTypeCapture && devtype != eSDTypeRender)
				_MBM(ERROR_FormatRestricted);

			MM         = const_cast<CSoundIMM*>(pMMInterface);
			type       = devtype;
			client_max = (!maxthreads) ? 1 : maxthreads;
		}
		~CAudioSessionManager()
		{
			Release();			// сброс интерфейсов
			client.Close();		// освобождение памяти
		}
	public:
		//>> Для поправки <actual_render_threads>
		uint32 GetNumOfExistedClients()
		{
			return clcount;
		}
		//>> Возвращает <true> и номер свободного клиента в случае успеха
		bool GetAvailableClient(int & out)
		{
			if (!isInit) { _MBM(ERROR_InitNone); return false; }

			for (int i=0; i<clcount; i++)
			{
				if (!client[i].CheckLock())
				{
					out = i;
					return true;
				}
			}

			return false;
		}
		/*//>> Возвращает <true> и номер нового клиента в случае успеха
		bool AddClient(int & out)
		{
			if (!isInit) { _MBM(ERROR_InitNone); return false; }

			if (clcount == client.count) return false; // достигнуто max threads

			if (InitClient(clcount))
			{
				out = clcount;
				clcount++;	
			}
			else return false;

			return true;
		}//*/
		//>> Перезагрузка интерфейсов (из-за смены устройства)
		bool ReInit()
		{
			Release(); // сброс интерфейсов
			return Init();
		}
	};

	class CSoundWASAPI final : public CSoundIMM
	{		
		typedef  CAudioSessionManager::ManageClient::ManageClientData  CAudioData;

	protected:
		CAudioSessionManager		render;				// управление воспроизведением
		CAudioSessionManager		capture;			// управление записью
		mutex						m_RunPlayback;		// контроль доступа RunPlayback()
		mutex						m_RunRecord;		// контроль доступа RunRecord()
		mutex						m_DeviceChanged;	// контроль доступа DefaulDeviceChanged()
		atomic<bool>				a_RenderChanged;	// уведомление от DefaulDeviceChanged()
		atomic<bool>				a_CaptureChanged;	// уведомление от DefaulDeviceChanged()
	private:
		uint32                      counter_devRenderChanged  {0}; // счетчик смен устройства (не сбрасывается)
		uint32                      counter_devCaptureChanged {0}; // счетчик смен устройства (не сбрасывается)
		bool						isInit;             // исп. блок <m_init_close>		
		bool						state_render  {0};  // исп. блок <m_init_close>
		bool						state_capture {0};  // исп. блок <m_init_close>
	public:
		CSoundWASAPI() : 
			CSoundIMM(),
			isInit(false),
			a_RenderChanged(false),
			a_CaptureChanged(false),
			capture(this, eSDTypeCapture, CAPTURE_THREADS),
			render(this, eSDTypeRender, RENDER_THREADS_MAX)
		{
			// размер виртуальной области внутри реального буфера всегда не больше размера этого реального буфера
			static_assert (
				REFTIMES_AUDIO_ORDER_MS >= MIXER_PLAYBUFTIME,
				"CSoundWASAPI() : Incorrect REAL buffer timesize < VIRTUAL mixer-related buffer timesize"
			);

			// для стабильности работы
			static_assert (
				REFTIMES_AUDIO_ORDER_MS >= 250,
				"CSoundWASAPI() : Incorrect REFTIMES_AUDIO_ORDER_MS < 250 ms"
			);

			// для стабильности работы
			static_assert (
				MIXER_PLAYBUFTIME <= 1000,
				"CSoundWASAPI() : Incorrect MIXER_PLAYBUFTIME > 1000 ms"
			);
		};
		~CSoundWASAPI() override final { Close(); };

		void Close() override final
		{
			rglock guard (m_init_close);

			if (!isInit) return;

			// Завершение работы микшера
			error_t hRes = DeActivateMixer();

			if (hRes != eSoundError_TRUE)
			{
				//printf("\nCan't de-activate mixer, ERR = %i", hRes);
				//getchar();
			}

			Delete();
			render.Release();
			capture.Release();
			CSoundIMM::Close();

			state_render  = false;
			state_capture = false;

			isInit = false;
		}
		bool Init() override final
		{
			rglock guard (m_init_close);

			if (isInit) return true;

			///////// запуск информационной части /////////

			if (!CSoundIMM::Init())				
			{
				// критическая ошибка - нет информации об устройствах
				printf("\nCSound::Init() : Can't activate CSoundIMM ! Aborting...");
				return false;
			}			

			///////// запуск воспроизводящей части /////////

			if (!render.Init())
			{
				// критическая ошибка - нет вывода звука
				printf("\nCSound::Init() : Can't activate render manager ! Aborting...");
				CSoundIMM::Close();
				return false;
			}
			else
			{
				actual_render_interfaces = render.GetNumOfExistedClients();
				state_render = true;
			}

			///////// запуск записывающей части /////////

			if (!capture.Init())		
			{
				// ошибка, но не критическая - продолжим без микрофона
				printf("\nCSound::Init() : Can't activate capture manager !");

				//render.Release();
				//CSoundIMM::Close();
				//return false;

				actual_capture_interfaces = 0;
				state_capture = false;
			}
			else
			{
				actual_capture_interfaces = capture.GetNumOfExistedClients();
				state_capture = true;
			}

			///////// запуск собственного микшера /////////

			error_t hRes = ActivateMixer();

			if (hRes != eSoundError_TRUE)
			{			
				// критическая ошибка !
				printf("\nCSound::Init() : Can't activate sound mixer! ERR = %i - Aborting...", hRes);

				render.Release();
				capture.Release();
				CSoundIMM::Close();

				state_render  = false;
				state_capture = false;

				return false;
			}

			return isInit = true;
		}

	private:
		error_t ActivateMixer() override final
		{
			return Mixer(render.mode, true);
		}
		error_t DeActivateMixer() override final
		{
			return Mixer(nullptr, false);
		}
	public:
		//>> Если при попытке запустить воспроизведение в микшер возвращается результат <eSoundError_NoMixer>, вероятно,
		//>> микшер был выключен в результате неудачного авто-перезапуска . Можно попробовать ещё раз отсюда .
		error_t ReActivateMixer() override final
		{
			error_t hRes = eSoundError_FALSE;

			{
				rglock guard (m_init_close);

				bool try_activate_mixer = false;

				if (state_render) try_activate_mixer = true;
				else // нет render-менеджера, попробовать перезапустить
				{
					error_t hRes_render = ReActivateRender();

					if (hRes_render == eSoundError_TRUE)
						 try_activate_mixer = true;
					else hRes = hRes_render;
				}

				if (try_activate_mixer)
				{
					DeActivateMixer();
					hRes = ActivateMixer();
				}
			}

			return hRes; // POSSIBLE : eSoundError _FALSE / _TRUE / _NoInit_R / _NoInit_F
		}
		//>> Если была получена ошибка <eSoundError_NoInit_R> или проверка GetStateSpeakers() вернула <false>, вероятно,
		//>> менеджер Render-сессий был выключен в результате неудачного авто-перезапуска . Можно попробовать ещё раз отсюда .
		error_t ReActivateRender() override final
		{
			error_t hRes = eSoundError_FALSE;

			{
				rglock guard (m_init_close);

				if (!isInit)
				{
					hRes = eSoundError_NoInit_F;
				}
				else // класс был инициализирован и с тех пор не закрывался
				{
					if (state_render)
					{
						hRes = eSoundError_TRUE;
					}
					else // класс работает, но render в какой-то момент упал
					{
						if (render.ReInit())
						{
							state_render = true;
							actual_render_interfaces = render.GetNumOfExistedClients();
							hRes = eSoundError_TRUE;
						}
					}
				}
			}

			return hRes; // POSSIBLE : eSoundError _FALSE / _TRUE / _NoInit_F
		}
		//>> Если была получена ошибка <eSoundError_NoInit_C> или проверка GetStateMicrophone() вернула <false>, вероятно,
		//>> менеджер Capture-сессий был выключен в результате неудачного авто-перезапуска . Можно попробовать ещё раз отсюда .
		error_t ReActivateCapture() override final
		{
			error_t hRes = eSoundError_FALSE;

			{
				rglock guard (m_init_close);

				if (!isInit)
				{
					hRes = eSoundError_NoInit_F;
				}
				else // класс был инициализирован и с тех пор не закрывался
				{
					if (state_capture)
					{
						hRes = eSoundError_TRUE;
					}
					else // класс работает, но capture в какой-то момент упал
					{
						if (capture.ReInit())
						{
							state_capture = true;
							actual_capture_interfaces = capture.GetNumOfExistedClients();
							hRes = eSoundError_TRUE;
						}
					}
				}
			}

			return hRes; // POSSIBLE : eSoundError _FALSE / _TRUE / _NoInit_F
		}

	private:
		//>> Событие смены профиля устройства (производится в отдельном потоке)
		void DefaulDeviceChanged(eSoundDevType type, uint32 iteration) override final
		{
			glock others_will_wait (m_DeviceChanged); // изолируем вызов

			//////// Блок защиты последовательности ////////

			switch (type)
			{
			case eSDTypeRender:
				if (iteration > counter_devRenderChanged)
				{
					if (counter_devRenderChanged < 0x10000000 && // Переполнение счетчика (...FE, FF, 0, 1, 2...)
						iteration                > 0xF0000000)   // .
						return;
					else
						counter_devRenderChanged = iteration;
				}
				else return;
				break;
			case eSDTypeCapture:
				if (iteration > counter_devCaptureChanged)
				{
					if (counter_devCaptureChanged < 0x10000000 && // Переполнение счетчика (...FE, FF, 0, 1, 2...)
						iteration                 > 0xF0000000)   // .
						return;
					else
						counter_devCaptureChanged = iteration;
				}
				else return;
				break;
			}

			////////////////////////////////////////////////

			rglock init_lock (m_init_close);  // останов инициализации/закрытия
			 glock safe_lock (m_load_delete); // не даём добавлять/удалять
			
			switch (type)
			{
			case eSDTypeRender:
				{			
					if (!state_render) return; // не инициализировано

					// Нужно остановить любое воспроизведение и заблокировать новые

				//	glock running_guard (m_run_play); // основной блокировщик (больше не нужен, теперь через <m_init_close>)

					_REL_1(a_RenderChanged); // <-- блокирую воспр. (доп. блокировщик для уже запущенных)

					DeActivateMixer();
					ControlInterrupt(SOUND_ALL, true);
					playpool.Close(); // actual_render_interfaces может измениться в меньшую сторону - сбросить!

					///////// RELOAD MANAGER /////////

					bool render_reinit_success = render.ReInit();

					if (!render_reinit_success)
					{					
						//_MBM(L"Failed to ReInit() sound render manager!");
						wprintf(L"Failed to ReInit() sound render manager!");

						// CRITICAL ERROR ! шлюпки на воду, всё сбросить

						render.Release();
						capture.Release();
						state_render  = false;
						state_capture = false;
						isInit        = false;
						actual_render_interfaces = 0;
					}
					else
					{
						wprintf(L"\nReInit() : success\n");
						actual_render_interfaces = render.GetNumOfExistedClients();
					}

					_REL_0(a_RenderChanged); // <-- снимаю блокировку
			
					///////// RELOAD MIXER /////////

					if (render_reinit_success)
					{
						// микшер перезапускаем после снятия блокировки play-запусков
						error_t hRes = ActivateMixer();

						if (hRes != eSoundError_TRUE)
						{
							//_MBM(L"Failed to Re-ActivateMixer() !");
							wprintf(L"Failed to Re-ActivateMixer() !");

							// ERROR !

							DeActivateMixer();

							// ... ?
						}
						else
						{
							wprintf(L"\nRe-ActivateMixer() : success\n");
						}
					}

					break;
				}
			case eSDTypeCapture:
				{
					if (!state_capture) return;

					_REL_1(a_CaptureChanged);

					// ...interrupt MIC RECORD THREAD code...

					if (!capture.ReInit())
					{
						//_MBM(L"Failed to ReInit() sound capture manager!");
						wprintf(L"Failed to ReInit() sound capture manager!");

						state_capture = false;
						actual_capture_interfaces = 0;
					}
					else
					{
						actual_capture_interfaces = capture.GetNumOfExistedClients();
					}

					_REL_0(a_CaptureChanged);

					break;
				}
			}
		}

	private:
		//>> Воспроизведение :: возвращает код выхода <eSoundPlayError> :: SOUND ENGINE CORE FUNCTION
		error_t Play(CAudioData * pAudioData)
		{
			#define _RETURN {             \
						return _.ERRCODE; }

			#define _AWAKE_FREQ 4  // частота пробуждений (для нормальной работы MIN=2х) [не микшер!]

			#if (REFTIMES_AUDIO_ORDER_MS <= 1000)
				#define _CONV_1x  1.f // кол-во секунд на 1 период
				#define _CONV_2x  2.f // кол-во секунд на 2 периода
				#define _WINDOW_S 4   // размер рабочего окна в секундах : периоды = текущий 1, следующий 1, запас 2
			#else
				#define _CONV_1x  (float) ( (float)  REFTIMES_AUDIO_ORDER_MS      / 1000) // кол-во секунд на 1 период
				#define _CONV_2x  (float) ( (float) (REFTIMES_AUDIO_ORDER_MS * 2) / 1000) // кол-во секунд на 2 периода
				#define _WINDOW_S (int)   (_CEIL32(_CONV_1x * 4)) // размер рабочего окна в секундах : периоды = текущий 1, следующий 1, запас 2
			#endif

			if (pAudioData == nullptr)                   return eSoundError_pAudioData;
			if (pAudioData->set.actual_sound == nullptr) return eSoundError_BadSndPTR;

			// все рабочие данные для удобства сведены в одну структуру
			struct _ALLWORKINGDATA_ {
				error_t                ERRCODE;      // код выхода
				SNDDATA *              sound;        // все данные по звуку
				CSound::MIXERDATA::THREADSHARED * pMixer; // общие данные с микшером (для потока воспр. микшера)
				SNDCALL *              pCaller;      // зовущий : пробуждает ото сна, содержит ожидающих, наши статусы
				IAudioClientF *        pClient;      // базовый интерфейс
				IAudioRenderClient *   pRender;      // контроль буфера (render)
				IAudioStreamVolume *   pVolume;      // контроль уровня звучания
				CPCMConverter *        ConverterPCM; // прозводит преобразование .wav
				CMP3Converter *        ConverterMP3; // прозводит преобразование .mp3 в .wav
				void *                 ConverterAAC; // RESERVED
				void *                 ConverterOGG; // RESERVED
				SNDDESC *              origin_mode;  // формат [origin] .wav
				SNDDESC *              target_mode;  // формат [target] .wav
				byte *                 window;       // окно, содержащее сконвертированный набор семплов
				byte *                 window_end;   // указатель на конец окна
				byte *                 window_ptr;   // текущая позиция в окне
				uint32                 window_size;  // насколько окно заполнено от текущей позиции (кол-во данных для проигрывания)
				uint32                 window_delta; // сколько в последний раз было отправлено на рендер, в байтах
				uint32                 window_shift; // если окно переполнится, здесь будет размер != 0 сконвертированного в начало окна
				byte *      audiocard_buffer;        // буфер аудиокарты
				uint32      audiocard_buffer_frames; // размер буфера аудиокарты (в кадрах)
				uint32      audiocard_buffer_bytes;  // размер буфера аудиокарты (в байтах)
				uint32      audiocard_buffer_f2b;	 // размер кадра в байтах
				uint32      audiocard_buffer_f2b_1s; // размер 1 секунды в байтах
				uint32      audiocard_buffer_f2b_m;  // размер виртуальной области в буфере, определенный микшером (в байтах)
				uint32      audiocard_buffer_m;      // размер виртуальной области в буфере, определенный микшером (в кадрах)
				uint32      audiocard_buffer_flags;  // флаги воспроизведения буфера
				uint32   playback_1x;                // размер 1х периода _CONV_1x в байтах
				int32    playback_duration;          // время проигрывания (milliseconds) - полное   (например 1000 ms)
				int32    playback_duration_half;     // время проигрывания (milliseconds) - половина (например 500 ms)
				float    playback_volume_multiplier; // (без изменений) запомненная громкость звука относительно общей громкости
				float    playback_master_volume;     // (без изменений) запомненная общая громкость
				float    playback_channels_volume[PCM_MAXCHANNELS]; // (без изменений) запомненные множители громкости по каналам
				float    playback_volume;            // (рассчитанный) громкость как = (master * multiplier)
				float    playback_rewind_time;       // 
				T_TIME   playback_timepoint;         // начальная точка отсчёта очередного цикла
				bool     playback_origin_end;        // признак конца исходных данных (при обычном незацикленном воспроизведении)
				bool     playback_interrupt;         // признак вызова на выход
				bool     playback_break_sleep;       // разрыв циклов сна
				bool     playback_loop;              // настройка зацикленности
				bool     playback_start_pause;       // флаг паузы в начале (при заказе <stop_at_start>)
				bool     playback_end_return;        // флаг возврата в начало (при заказе <stop_at_end>)
				bool     playback_running;           // статус воспроизведения (саморегуляция между Run() / Stop())
				bool     playback_mixer;             // метка, что воспроизводятся часто меняющиеся данные от микшера
			} _ ;

			//const int szdata = sizeof(_ALLWORKINGDATA_);

			// Н.У.
			_.ERRCODE                = eSoundError_Unknown;
			_.sound                  = (SNDDATA*) pAudioData->set.actual_sound;
			_.pMixer                 = pAudioData->set.mixer_p;
			_.pCaller                = pAudioData->set.caller;
			_.pClient                = pAudioData->p;
			_.pRender                = pAudioData->r;
			_.pVolume                = pAudioData->v;
			_.ConverterPCM           = nullptr;
			_.ConverterMP3           = nullptr;
			_.ConverterAAC           = nullptr;
			_.ConverterOGG           = nullptr;
			_.origin_mode            = _.sound->snddesc;
			_.target_mode            = render.mode;
			_.playback_loop          = pAudioData->set.loop;
			_.playback_start_pause   = pAudioData->set.stop_at_start;
			_.playback_end_return    = false;
			_.playback_running       = false;
			_.playback_mixer         = pAudioData->set.mixer;
			_.audiocard_buffer_flags = 0;

			// РАБОТА В РЕЖИМЕ МИКШЕРА
			// Аудиокарта выдаёт размер буфера не меньше MIXER_PLAYBUFTIME
			// В рабочем окне мы содержим не меньше MIXER_PLAYBUFTIME, добавляя ещё один MIXER_PLAYBUFTIME если наблюдается меньше
			// Спим [MIXER_PLAYTHREAD_PERIODICITY] времени от MIXER_PLAYBUFTIME соответственно
			// Микшер гарантирует, что при конвертировании MIXER_PLAYBUFTIME данных они всегда являются актуальным набором

			if (_.pCaller == nullptr) return eSoundError_pCaller;
			if (_.pClient == nullptr) return eSoundError_pClient;
			if (_.pRender == nullptr) return eSoundError_pRender;
			if (_.pVolume == nullptr) return eSoundError_pVolume;

			auto GetConvertors     = [this, &_, pAudioData] () -> bool
			{
				switch (_.target_mode->wFormatTag)
				{
				case WAVE_FORMAT_EXTENSIBLE:
					_.target_mode->wFormatTag = WAVE_FORMAT_PCM;
					break;
				case WAVE_FORMAT_PCM:
				case WAVE_FORMAT_IEEE_FLOAT:
					break;
				default:
					_.ERRCODE = eSoundError_wFormatTag;
					return false;
				}

				switch (_.sound->type)
				{
				case eSoundType_MP3:
					{
						_.ConverterMP3 = pAudioData->conv.mp3;
						_.ConverterPCM = pAudioData->conv.pcm;

						if (_.ConverterMP3 == nullptr ||
							_.ConverterPCM == nullptr)
						{
							_.ERRCODE = eSoundError_noConvPtrs;
							return false;
						}
						break;
					}
				case eSoundType_WAVE:
					{
						_.ConverterPCM = pAudioData->conv.pcm;

						if (_.ConverterPCM == nullptr)
						{
							_.ERRCODE = eSoundError_noConvPtrs;
							return false;
						}
						break;
					}
				case eSoundType_OGG:	// TODO
				case eSoundType_AAC:	// TODO
				case eSoundType_UNKNOWN:
				default:
					_.ERRCODE = eSoundError_soundType;
					return false;
				}

				return true;
			};
			auto GetBufferSize     = [this, &_]             () -> bool
			{
				long hRes;

				if (S_OK != (hRes = _.pClient->GetBufferSize(&_.audiocard_buffer_frames)))
				{
					// hRes AUDCLNT_E_NOT_INITIALIZED     The audio stream has not been successfully initialized
					// hRes AUDCLNT_E_DEVICE_INVALIDATED  The audio endpoint device unavailable for use (eg. unplugged)
					_.ERRCODE = eSoundError_getBufSz;
					return false;
				}

				_.audiocard_buffer_m      = (uint32)((0.001f * MIXER_PLAYBUFTIME) * _.target_mode->nSamplesPerSec);

				_.audiocard_buffer_f2b    = _.target_mode->nChannels  * (_.target_mode->wBitsPerSample / 8);
				_.audiocard_buffer_f2b_1s = _.audiocard_buffer_f2b    *  _.target_mode->nSamplesPerSec;
				_.audiocard_buffer_f2b_m  = _.audiocard_buffer_f2b    *  _.audiocard_buffer_m;
				_.audiocard_buffer_bytes  = _.audiocard_buffer_frames *  _.audiocard_buffer_f2b;

				_.playback_1x = ceil(_CONV_1x * _.audiocard_buffer_f2b_1s) ;

			// Сделал эту проверку в CAudioSessionManager, чтобы сразу отбросить непригодный к использованию интерфейс
			//
			//	// выделенный буфер в ms
			//	uint32 audiocard_buffer_time_ms = (uint32)(1000 * ((float)_.audiocard_buffer_frames / _.target_mode->nSamplesPerSec));
			//
			//	// пока что определил MS_MIN размер как размер необходимый работе микшера (т.е. две проверки ниже равносильны друг другу)
			//	if (audiocard_buffer_time_ms < REFTIMES_AUDIO_ORDER_MS_MIN)
			//	{
			//		_.ERRCODE = eSoundError_BadBufSz; // недостаточный размер буфера
			//		return false;
			//	}
			//
			//	// аудиокарта не смогла выделить фрагмент буфера требуемого размера для нормальной работы микшера
			//	if (_.playback_mixer)
			//	if (_.audiocard_buffer_m > _.audiocard_buffer_frames)
			//	{
			//		_.ERRCODE = eSoundError_BadBufSzMixer; // недостаточный размер буфера для работы микшера
			//		return false;
			//	}

				return true;
			};
			auto GetWindowConvAAC  = [this, &_] (float start_timepoint) -> bool
			{
				//long hRes;

				// ...

				// дополнительная информация
				_.pCaller->status.UpdateInfoTime(start_timepoint, start_timepoint / _.sound->duration);

				return false;
			};
			auto GetWindowConvOGG  = [this, &_] (float start_timepoint) -> bool
			{
				//long hRes;

				// ...

				// дополнительная информация
				_.pCaller->status.UpdateInfoTime(start_timepoint, start_timepoint / _.sound->duration);

				return false;
			};
			auto GetWindowConvMP3  = [this, &_] (float start_timepoint) -> bool
			{
				long hRes;

				auto ConverterMP3 = _.ConverterMP3;
				auto ConverterPCM = _.ConverterPCM;

				ConverterMP3->settings.in.data        = _.sound->file.buf + _.sound->file_offset;
				ConverterMP3->settings.in.size        = _.sound->file_size;
				ConverterMP3->settings.in.start_time  = start_timepoint;
				ConverterMP3->settings.in.start_frame = 0;	
				ConverterMP3->settings.in.use_frame   = false;	    // сейчас через время, но потом через фреймы
				ConverterMP3->settings.in.time        = _CONV_2x ;	// подготовим сразу 2x размер
				ConverterMP3->settings.in.loop        = _.playback_loop;
				ConverterMP3->settings.in.maximize    = false;
				
				if (eMP3ConvStatus_Normal != (hRes = ConverterMP3->Run()))
				{
					if (hRes != eMP3ConvStatus_BufferEnd)
					{
							//printf("\nERROR : ConverterMP3->Run() error %i (_debug_1_MP3)", hRes);
							_.ERRCODE  = eSoundError_convMP3;
							_.ERRCODE |= hRes << eSoundError__MASK_CONVERTER_SHIFT;
					}
					else _.ERRCODE  = eSoundError_Normal_End;
					return false;
				}

				//printf("\nmp3 converted time %f", ConverterMP3->settings.out.time);

				ConverterMP3->settings.in.use_frame = true;	     // в дальнейшем правим начало через фреймы
				ConverterMP3->settings.in.time      = _CONV_1x ; // в дальнейшем будем работать по 1x

				// теперь надо сконвертировать выходной PCM под аудиокарту

				_.origin_mode->nChannels      = ConverterMP3->settings.out.nChannels;
				_.origin_mode->nSamplesPerSec = ConverterMP3->settings.out.nSamplesPerSec;
				_.origin_mode->wBitsPerSample = ConverterMP3->settings.out.wBitsPerSample;
				_.origin_mode->wFormatTag     = ConverterMP3->settings.out.wFormatTag;

				ConverterPCM->origin = _.origin_mode;
				ConverterPCM->target = _.target_mode;

				ConverterPCM->settings.loop      = false;
				ConverterPCM->settings.allocate  = false;
				ConverterPCM->settings.maxTarget = false; // DO NOT USE <TRUE>

				ConverterPCM->settings.origin.buffer      = ConverterMP3->settings.out.data_out;
				ConverterPCM->settings.origin.size        = ConverterMP3->settings.out.size_out;
				ConverterPCM->settings.origin.samples     = 0;
				ConverterPCM->settings.origin.time        = 0.f;
				ConverterPCM->settings.origin.usetime     = false;
				ConverterPCM->settings.origin.BE          = false;
				ConverterPCM->settings.origin.user_signed = false;
				ConverterPCM->settings.origin.auto_signed = true;

				ConverterPCM->settings.target.buffer      = _.window_ptr;
				ConverterPCM->settings.target.size        = _.playback_1x * 2 ; // mp3 ранее выдал не более 2x
				ConverterPCM->settings.target.samples     = 0;
				ConverterPCM->settings.target.time        = _CONV_2x ; // ожидаем не более 2x
				ConverterPCM->settings.target.usetime     = true;
				ConverterPCM->settings.target.BE          = false;
				ConverterPCM->settings.target.user_signed = false;
				ConverterPCM->settings.target.auto_signed = true;

				if (ePCMConvStatus_Normal != (hRes = ConverterPCM->Run()))
				{
					if (hRes != ePCMConvStatus_EndOfOrigin)
					{
						//printf("\nERROR : ConverterPCM->Run() error %i (_debug_1_MP3-PCM)", hRes);
							_.ERRCODE  = eSoundError_convPCM;
							_.ERRCODE |= hRes << eSoundError__MASK_CONVERTER_SHIFT;
					}
					else _.ERRCODE  = eSoundError_Normal_End;
					return false;
				}

				ConverterPCM->settings.target.size = _.playback_1x ; // в дальнейшем не более 1x
				ConverterPCM->settings.target.time = _CONV_1x      ; // .

				// определим, сколько сделали по факту
				_.window_size = ConverterPCM->settings.outInfo.target_samples * _.audiocard_buffer_f2b;

				// дополнительная информация
				_.pCaller->status.UpdateInfoTime(start_timepoint, start_timepoint / _.sound->duration);

				return true;
			};
			auto GetWindowConvPCM  = [this, &_] (float start_timepoint) -> bool
			{
				long hRes;

				float target_time;

				if (_.playback_mixer) target_time = 0.001f * MIXER_PLAYBUFTIME; // к заказу = время рабочей области микшера %f сек.
				else                  target_time = _CONV_1x ;                  // к заказу = 1x

				auto & ConverterPCM = * _.ConverterPCM;

				ConverterPCM.origin = _.origin_mode;
				ConverterPCM.target = _.target_mode;

				ConverterPCM.settings.loop      = _.playback_loop;
				ConverterPCM.settings.allocate  = false;
				ConverterPCM.settings.maxTarget = false;

				ConverterPCM.settings.origin.buffer      = _.sound->file.buf + _.sound->file_offset;
				ConverterPCM.settings.origin.size        = _.sound->file_size;
				ConverterPCM.settings.origin.samples     = 0;
				ConverterPCM.settings.origin.time        = start_timepoint;
				ConverterPCM.settings.origin.usetime     = true;  // сейчас через время, но потом через семплы
				ConverterPCM.settings.origin.BE          = false;
				ConverterPCM.settings.origin.user_signed = false;
				ConverterPCM.settings.origin.auto_signed = true;

				ConverterPCM.settings.target.buffer      = _.window_ptr;
				ConverterPCM.settings.target.size        = ceil( target_time * _.audiocard_buffer_f2b_1s ) ; // размер всегда на <target_time>
				ConverterPCM.settings.target.samples     = 0;
				ConverterPCM.settings.target.time        = target_time; // закажем не больше <target_time> (может выйти меньше)
				ConverterPCM.settings.target.usetime     = true;
				ConverterPCM.settings.target.BE          = false;
				ConverterPCM.settings.target.user_signed = false;
				ConverterPCM.settings.target.auto_signed = true;

				// если данные от микшера
				if (_.playback_mixer) // || (_.sound->modified && _.sound->modified_clr_wav))
					 ConverterPCM.settings.clear_origin = true;
				else ConverterPCM.settings.clear_origin = false;

				if (ePCMConvStatus_Normal != (hRes = ConverterPCM.Run()))
				{
					if (hRes != ePCMConvStatus_EndOfOrigin)
					{
							//printf("\nERROR : ConverterPCM->Run() error %i (_debug_1_)", hRes);
							_.ERRCODE  = eSoundError_convPCM;
							_.ERRCODE |= hRes << eSoundError__MASK_CONVERTER_SHIFT;
					}
					else _.ERRCODE = eSoundError_Normal_End;
					return false;
				}

				ConverterPCM.settings.origin.usetime = false; // в дальнейшем правим начало через номер семпла

				// определим, сколько сделали по факту
				_.window_size = ConverterPCM.settings.outInfo.target_samples * _.audiocard_buffer_f2b;

				// дополнительная информация
				_.pCaller->status.UpdateInfoTime(start_timepoint, start_timepoint / _.sound->duration);

				if (_.playback_mixer) // при совместной работе с микшером
				{
					_.pMixer->rw_pos    = ConverterPCM.settings.outInfo.loop_end_sample;
					_.pMixer->rw_count += ConverterPCM.settings.outInfo.origin_samples;

					// ограничение от переполнения
					if (_.pMixer->rw_count > MIXER_MAX_BUFTIME_SAMPLES)
						_.pMixer->rw_count = MIXER_MAX_BUFTIME_SAMPLES;
				}

				return true;
			};
			auto GetWindowConv     = [this, &_, pAudioData, GetWindowConvAAC, GetWindowConvOGG, GetWindowConvMP3, GetWindowConvPCM]() -> bool
			{
				// Подготовим рабочее окно и сконвертируем в него 1х или 2х сек. данных
				{
					uint32 requested_size = _.audiocard_buffer_f2b_1s * _WINDOW_S ;
					auto & conv_window = pAudioData->conv.window;

					if (conv_window.buf == nullptr ||		// Выделим место, если нет или не хватает
						conv_window.count < requested_size)	// .
					{
						if (!conv_window.Create(requested_size))
						{
							_.ERRCODE = eSoundError_convBuf;
							return false;
						}
					}

					// ЗАМЕТКА : Для потока воспр. микшера размер окна будет такой же, хотя можно было бы и меньше

					_.window      = conv_window.buf;
					_.window_end  = conv_window.buf + requested_size;
				}

				// Сбросить окно конвертации
				_.window_ptr   = _.window;
				_.window_size  = 0;
				_.window_shift = 0;
				_.window_delta = 0;

				bool ret;

					 if (_.ConverterAAC != nullptr) ret = GetWindowConvAAC(0.f);
				else if (_.ConverterOGG != nullptr) ret = GetWindowConvOGG(0.f);		
				else if (_.ConverterMP3 != nullptr) ret = GetWindowConvMP3(0.f);		
				else
				{
					if (_.playback_mixer) // блок на разделяемые с микшером данные
					{
						glock lock_read_write (_.pMixer->m_ReadWrite);
						ret = GetWindowConvPCM(0.f);
					}
					else
						ret = GetWindowConvPCM(0.f);
				}

				return ret;
			};
			auto GetBufferPutData  = [this, &_]             () ->bool
			{
				long hRes;

				/////////// Заполним shared-буфер аудиокарты ///////////

				if (S_OK != (hRes = _.pRender->GetBuffer(_.audiocard_buffer_frames, &_.audiocard_buffer)))
				{
					// hRes AUDCLNT_E_BUFFER_TOO_LARGE  exceeds the available buffer space (buffer size minus padding size)
					// hRes AUDCLNT_E_DEVICE_INVALIDATED  The audio endpoint device unavailable for use (eg. unplugged)
					_.ERRCODE = eSoundError_getBuf;
					return false;
				}

				//printf("\naudiocard_buffer_frames %i", audiocard_buffer_frames);

				_.window_delta = (_.window_size > _.audiocard_buffer_bytes) ? _.audiocard_buffer_bytes : _.window_size;
				memcpy(_.audiocard_buffer, _.window_ptr, _.window_delta);
				_.window_ptr  += _.window_delta;
				_.window_size -= _.window_delta;

				// MSDN : [Remarks] The client must release the same number of frames that it requested in the preceding call
				// MSDN : [Parameters] NumFramesWritten
				//        The number of audio frames written by the client to the data packet.
				//        The value of this parameter must be less than or equal to the size of the data packet,
				//        as specified in the NumFramesRequested parameter passed to the IAudioRenderClient::GetBuffer method.
				//
				// Согласно [Remarks] надо отпустить столько же, сколько заказал - даже если заполнил не полностью
				// Согласно [Parameters] можно меньше, т.е. сколько записал
				// Если отпустить больше, чем записал, в конце проигрывания будут явные звуковые артефакты
				// Вариант 1. Отпустить столько, сколько записал т.е. window_delta / audiocard_buffer_f2b
				// Вариант 2. Отпустить заказанный audiocard_buffer_frames, но обнулить на конце незаписанный объём
				// Вероятно, ошибка в [Remarks] ? Делаю по 1 варианту.

				if (S_OK != (hRes = _.pRender->ReleaseBuffer(
					//audiocard_buffer_frames,				 // Вариант 2
					_.window_delta / _.audiocard_buffer_f2b, // Вариант 1
					_.audiocard_buffer_flags)))
				{
					// hRes AUDCLNT_E_DEVICE_INVALIDATED  The audio endpoint device unavailable for use (eg. unplugged)
					_.ERRCODE = eSoundError_releaseBuf;
					return false;
				}

				return true;
			};
			auto GetPlaybackTime   = [this, &_]             () -> bool
			{
				if (_.playback_mixer)
				{
					_.playback_duration      = MIXER_PLAYBUFTIME;
					_.playback_duration_half = MIXER_PLAYTHREAD_PERIODICITY;
				}
				else
				{
					_.playback_duration      = (int32) (((float)_.audiocard_buffer_frames / _.target_mode->nSamplesPerSec) * 1000); // напрмиер ~ 1000 ms
					_.playback_duration_half = _.playback_duration / _AWAKE_FREQ ; // например ~ 500 ms при _AWAKE_FREQ == 2x
				}

				if (_.playback_duration <= 0)
				{
					_.ERRCODE = eSoundError_duration;
					return false;
				}
				return true;
			};
			auto GetVolumeSettings = [this, &_]             () -> bool
			{
				_.playback_volume_multiplier = _ATM_LD(_.sound->a_volume_multiplier);
				_.playback_master_volume     = _ATM_LD(a_master_volume);

				_.playback_volume = _CLAMP(_.playback_master_volume * _.playback_volume_multiplier, 0.f, 1.f);

				_.sound->GetChannelsVolume(_.playback_channels_volume);

				return true;
			};
			auto GetErrorsCheck    = [this, &_]             () -> bool
			{
				long hRes;

				// проверка на всякий случай
				{
					uint32 channels_count;
					if (S_OK != (hRes = _.pVolume->GetChannelCount(&channels_count)))
					{
						// hRes AUDCLNT_E_DEVICE_INVALIDATED  The audio endpoint device unavailable for use (eg. unplugged)
						_.ERRCODE = eSoundError_getChNum;
						return false;
					}
					if (channels_count != _.target_mode->nChannels)
					{
						//printf("\nERROR : Unexpected difference in channels count");
						_.ERRCODE = eSoundError_diffChNum;
						return false;
					}
				}

				byte nCh = (byte) _.target_mode->nChannels;
				for (byte channel = 0; channel < nCh; channel++)
				{
					float ch_volume;

					if (channel < PCM_MAXCHANNELS) 
					{
						 ch_volume = _.playback_volume * _.playback_channels_volume[channel];
						 ch_volume = _CLAMP(ch_volume, 0.f, 1.f);
					}
					else ch_volume = _.playback_volume;

					// FIX MIXER VOLUME
					if (_.playback_mixer) ch_volume = 1.f;

					if (S_OK != (hRes = _.pVolume->SetChannelVolume(channel, ch_volume)))
					{
						// hRes AUDCLNT_E_DEVICE_INVALIDATED  The audio endpoint device unavailable for use (eg. unplugged)
						_.ERRCODE = eSoundError_setChVol;
						return false;
					}
				}

				return true;
			};

			if (!GetConvertors())      _RETURN;
			if (!GetBufferSize())      _RETURN;
			if (!GetWindowConv())      _RETURN;
			if (!GetBufferPutData())   _RETURN;
			if (!GetPlaybackTime())    _RETURN;
			if (!GetVolumeSettings())  _RETURN;
			if (!GetErrorsCheck())     _RETURN;

			////////////////////////////////////////////

			/////////// Вход в рабочий цикл  ///////////

			////////////////////////////////////////////

			auto Run   = [this, &_] () -> bool
			{
				long hRes;

				if (!_.playback_running)
				{
					if (S_OK != (hRes = _.pClient->Start()))
					{
						// hRes AUDCLNT_E_NOT_INITIALIZED     The audio stream has not been successfully initialized.
						// hRes AUDCLNT_E_NOT_STOPPED         The audio stream was not stopped at the time of the Start call.
						// hRes AUDCLNT_E_DEVICE_INVALIDATED  The audio endpoint device unavailable for use (eg. unplugged)
						_.ERRCODE = eSoundError_start;
						return false;
					}
					_.playback_running = true;
				}
				return true;
			};
			auto Stop  = [this, &_] () -> bool
			{
				long hRes;

				if (_.playback_running)
				{
					if (S_OK != (hRes = _.pClient->Stop()))
					{
						// hRes AUDCLNT_E_NOT_INITIALIZED     The client has not been successfully initialized
						// hRes AUDCLNT_E_SERVICE_NOT_RUNNING The Windows audio service is not running
						_.ERRCODE = eSoundError_stop;
						return false;
					}
					_.playback_running = false;
				}
				return true;
			};
			auto Reset = [this, &_, Stop]() -> bool
			{
				long hRes;

				if (_.playback_running)
					if (!Stop())
						return false;

				if (S_OK != (hRes = _.pClient->Reset()))
				{
					// hRes AUDCLNT_E_NOT_INITIALIZED           The audio stream has not been successfully initialized.
					// hRes AUDCLNT_E_NOT_STOPPED               The audio stream was not stopped at the time of the Start call.
					// hRes AUDCLNT_E_BUFFER_OPERATION_PENDING  The client is currently writing to or reading from the buffer.
					_.ERRCODE = eSoundError_start;
					return false;
				}
				return true;
			};

			// Н.У.
			_.playback_origin_end = false;
			_.playback_interrupt  = false;
			_.playback_timepoint  = _TIME;
			_.window_shift        = 0;

			//printf("\nPre-run test stop 5 sec");	_SLEEP(5000);

			if (!_.playback_start_pause)
				if (!Run()) _RETURN;
 
			// Поведение при особых случаях в критической секции
			// 1. Любое действие с ожиданием ---> X (нет особых случаев)
			// 2. Поток встаёт в самопаузу <stop_at_end> ---> X (нет особых случаев, кроме тех, что для обычной паузы)
			// 3. Перемотка + пауза (нет ожидания) ---> перемоталось, но не вошло в паузу ---> новая перемотка + пауза (с ожиданием) ---> саморегуляция Run()/Stop()
			// 4. Пауза/самопауза (нет ожидания) ---> перемотка + пауза (с ожиданием) ---> проверять status.rewinded
			// 5. Перемотка на стадии самопаузы <first_run> ---> проверять status.rewinded
			// 6. Поток в паузе ---> вызов Rewind() ---> не успел выйти, уже прислали новую паузу ---> проверять status.rewinded
			// 7. Поток в паузе ---> вызов Resume() ---> не успел выйти, уже прислали новую паузу ---> снять метку awakened
			// 8. Вклинивание новой перемотки во время текущей ---> регулировка <rewinded> через <rewind_new_order>
			// 9. ??? ещё что-нибудь ??? PROFIT

			while (_.audiocard_buffer_flags != AUDCLNT_BUFFERFLAGS_SILENT)
			{
				//printf("\nTIME : %f sec. [%7.3f %%]", _.pCaller->status.time_t, 100 * _.pCaller->status.time_p);

				auto MakeSleep   = [this, &_, GetWindowConvAAC, GetWindowConvOGG, GetWindowConvMP3, GetWindowConvPCM, GetBufferPutData, Run, Stop, Reset]() -> bool
				{
					auto CheckRewind       = [this, &_, GetWindowConvAAC, GetWindowConvOGG, GetWindowConvMP3, GetWindowConvPCM, GetBufferPutData, 
						Run, Reset] (uint32 eSCR) -> bool
					{
						long hRes;
						float rewind_time;

						// признак собственной перемотки на запуске нового цикла
						bool self_back = _BOOL(eSCR & eSoundCallReason_SelfBack);

						if (self_back)					
							 rewind_time = 0.f;
						else rewind_time = _.playback_rewind_time; //_.sound->_ACQ_LD(a_rewind_time);

						// Сбросить окно конвертации
						_.window_ptr   = _.window;
						_.window_delta = 0;
						_.window_shift = 0;
						_.window_size  = 0;

						// Сконвертировать новое
						     if (_.ConverterAAC != nullptr) hRes = GetWindowConvAAC(rewind_time);
						else if (_.ConverterOGG != nullptr) hRes = GetWindowConvOGG(rewind_time);
						else if (_.ConverterMP3 != nullptr) hRes = GetWindowConvMP3(rewind_time);
						else                                hRes = GetWindowConvPCM(rewind_time);

						if (!hRes) // ошибка конвертации
							return false; 

						// Сбросить воспроизведение
						if (!Reset())
							return false;

						// Записать в буфер аудиокарты
						if (!GetBufferPutData())
							return false;

						// Если идём далее в секцию паузы, то запуск будет оттуда
						if (!(eSCR & eSoundCallReason_Pause))
						{
							// Перезапуск воспроизведения
							if (!Run())
								return false;
						}

						// Обновить <rewinded> исходя из наличия нового заказа <rewind_new_order>
						// Если новый заказ не появился, <rewinded> будет сброшен (снимается блок входа в паузу)
						{
							// ПРИМЕЧАНИЕ: В ControlRewindPRIV() сначала берётся <m_pause> блок !

							glock lock_by_pause  (_.sound->m_pause);  // блок работы с <rewinded>
							glock lock_by_caller (_.pCaller->m_call); // блок работы с <rewind_new_order>

							// Сбросит статус, если нет нового заказа, и разблокирует вход в паузу
							_.pCaller->status.rewinded = _.pCaller->rewind_new_order;
						}

						// можно сделать rewind() сначала без ожидающего на время TIME_1, затем сразу с ожидающим, но на другое время TIME_2
						// тогда возможно ложное оповещение от TIME_1 на ожидающего TIME_2
						// чтобы этого не было, перемотка конкретизирует свой тип : Rewind_WAITER или Rewind_NO_WAITER

						// оповестить ожидающего
						if ((eSCR & eSoundCallReason_Rewind_WAITER) == eSoundCallReason_Rewind_WAITER)
							_.pCaller->wait.Close(eSoundCallReason_Rewind);

						//printf("\nRewind test stop 5 sec"); _SLEEP(5000);

						// новая точка отсчёта
						_.playback_timepoint = _TIME;

						return true;
					};
					auto CheckPause        = [this, &_, Run, Stop] (uint32 eSCR) -> bool
					{
						bool self_pause, user_pause, pause, first_run;
						auto & status = _.pCaller->status;

						{
							ulock pause_lock (_.sound->m_pause);

							self_pause = _BOOL(eSCR & eSoundCallReason_SelfPause);
							user_pause = _.sound->pause;
							pause      = user_pause | self_pause;
							first_run  = self_pause && !_BOOL(eSCR & eSoundCallReason_SelfBack);

							// уходим, если есть новый заказ перемотки
							if (status.rewinded)
								pause = false;

							// обычный <awakened> не надо проверять т.к. его наличие означает (!user_pause),
							// а (self_pause) он не должен касаться

							if (pause)
							{
							//	printf("\nENTER pause section");

								// Остановить (если ещё не остановлено)
								if (!Stop())
									return false;

								// Разорвать циклы сна после выхода из паузы
								_.playback_break_sleep = true;

								// Оповестить возможного ожидающего
								_.pCaller->wait.Close(eSoundCallReason_Pause);

								// при (first_run = true) сброс был перед запуском потока
								if (!first_run)
								{
									status.awakened = false;
									//status.rewinded = false;  ! СБРОС В CheckRewind()
								}							

								status.in_pause = true;

								// дополнительная информация
								status.UpdateInfoPause(self_pause, user_pause);
																								//_.sound->debug_num_in_pause++;
								for (;;)
								{
									if (status.awakened)                 // (RESUME) признак к пробуждению
										break;

									if (status.rewinded)                 // (REWIND) признак к пробуждению из-за перемотки
										break;

									if (_.sound->_ATM_LD(a_interrupt))   // (INTERRUPT) безусловное прерывание
										break;

								//	printf("\n(RE-)ACTIVATE PAUSE");

									// m_pause lock/unlock section
									_.sound->_CV_WAIT(cv_pause, pause_lock);

									// simulate too late <m_pause> lock
								//	pause_lock.unlock();
								//	printf("\nTest AWAKE and UNLOCK pause 5sec waiting..."); _SLEEP(5000);
								//	pause_lock.lock();

								}																//_.sound->debug_num_in_pause--;

							//	printf("\nEXIT pause section");

								// дополнительная информация
								status.UpdateInfoPause(false, false);
							}

							// сброс
							status.in_pause = false;
							status.awakened = false;
							//status.rewinded = false;  ! СБРОС В CheckRewind()

							if (first_run)
								status.runned = true;

						} // m_pause unlocked

						// Продолжить (если было остановлено)
						if (!Run()) 
							return false;
						
						// Оповестить возможного ожидающего
						_.pCaller->wait.Close(eSoundCallReason_Resume);

						return true;
					};
					auto CheckInterrupt    = [this, &_] () -> bool
					{
						if (_ATM_LD(_.sound->a_interrupt))
						{
							_.playback_interrupt     = true;
							_.audiocard_buffer_flags = AUDCLNT_BUFFERFLAGS_SILENT;
							return true;
						}
						return false;
					};
					auto CheckVolumeChange = [this, &_] (uint32 eSCR) -> bool
					{
						// ЗАМЕТКА : _CLAMP в принципе не нужен, но лучше не зависеть от кода в другом месте

						{
							bool update_playback_volume = false;

							if (eSCR & eSoundCallReason_MasterVolume)
							{
								_.playback_master_volume = _ATM_LD(a_master_volume);
								update_playback_volume = true;
							}
							if (eSCR & eSoundCallReason_VolumeMultiplier)
							{
								_.playback_volume_multiplier = _ATM_LD(_.sound->a_volume_multiplier);
								update_playback_volume = true;
							}
							if (eSCR & eSoundCallReason_ChannelVolume)
							{
								_.sound->GetChannelsVolume(_.playback_channels_volume);
							}

							if (update_playback_volume)
							{
								_.playback_volume = _.playback_master_volume * _.playback_volume_multiplier;
								_.playback_volume = _CLAMP(_.playback_volume, 0.f, 1.f);
							}
						}

						byte nCh = (byte)_.target_mode->nChannels;
						for (byte channel = 0; channel < nCh; channel++)
						{
							long hRes;
							float ch_volume;

							if (channel < PCM_MAXCHANNELS) {
							       ch_volume = _.playback_volume * _.playback_channels_volume[channel];
								   ch_volume = _CLAMP(ch_volume, 0.f, 1.f);
							} else ch_volume = _.playback_volume;

							// FIX MIXER VOLUME
							if (_.playback_mixer) ch_volume = 1.f;

							if (S_OK != (hRes = _.pVolume->SetChannelVolume(channel, ch_volume)))
							{
								// hRes AUDCLNT_E_DEVICE_INVALIDATED  The audio endpoint device unavailable for use (eg. unplugged)
								_.ERRCODE = eSoundError_setChVol;
								return false;
							}
						}

						return true;
					};

					bool ret = true; // true = NORMAL, false = ERROR / INTERRUPT
				
					_.playback_break_sleep = false; // выход, чтобы отправиться записать ещё данных аудиокарте

					while (!_.playback_break_sleep)
					{
						uint32 eSCR = eSoundCallReason_NONE; // eSoundCallReason

						// если запуск нового цикла - эту секцию опускаем
						if (!(_.playback_end_return && _.playback_start_pause))
						{
							ulock sleep_lock (_.pCaller->m_call);

							if (_.pCaller->call) // already called
							{
								eSCR = _.pCaller->reason; // Забираем причину
								_.pCaller->Close();       // .

								// при перемотке забираем время (под блоком <m_call>)
								if (eSCR & eSoundCallReason_Rewind)
								{
									_.playback_rewind_time = _.pCaller->GetRewindTime();
								}
							}
							else // try to sleep						
							{
								int32 sleep_time = _.playback_duration_half - (int32)_TIMER(_.playback_timepoint);
								if (sleep_time > _.playback_duration_half)
									sleep_time = _.playback_duration_half;
								if (sleep_time > 0)
								{
									//printf("\nsleep_time %i (duration %i)", sleep_time, _.playback_duration);

									for (;;) // sleep loop
									{
										cvstate timeout;
										timems sleep_time_ms (sleep_time);

										timeout = _.pCaller->_CV_FOR(cv_call, sleep_lock, sleep_time_ms);

										if (timeout == _CV_NO_TIMEOUT)
										{
											if (_.pCaller->call)
											{
												eSCR = _.pCaller->reason; // Забираем причину
												_.pCaller->Close();       // .

												// при перемотке забираем время (под блоком <m_call>)
												if (eSCR & eSoundCallReason_Rewind)
												{
													_.playback_rewind_time = _.pCaller->GetRewindTime();
												}

												break;
											}
											else // fake awakening, try continue sleep
											{
												sleep_time = _.playback_duration_half - (int32) _TIMER(_.playback_timepoint);
												if (sleep_time > _.playback_duration_half)
													sleep_time = _.playback_duration_half;
												if (sleep_time <= 0)
													break;
											}
										}
										else // _CV_TIMEOUT
											break;
									}
									// exit sleep loop
								}
								else // no time to sleep
									_.playback_break_sleep = true;
							}
							// <sleep_lock> will auto-unlocked here
						} 

						///////////////////////////////////////////////////////////////

					//	if (eSCR & eSoundCallReason_Rewind)
					//		{ printf("\nSimulate rewind catch & slow moving to rewind func... 5 sec"); _SLEEP(5000); }

						// (новый цикл) обработка начальной паузы
						if (_.playback_start_pause)
						{
							_.playback_start_pause = false;

							eSCR = eSoundCallReason_SelfPause |
							       eSoundCallReason_Pause;
						}
						
						// (новый цикл) обработка возврата и паузы
						if (_.playback_end_return)
						{
							_.playback_end_return = false;

							eSCR = eSoundCallReason_SelfBack         |
								   eSoundCallReason_SelfPause        |
							       eSoundCallReason_Rewind_NO_WAITER |
							       eSoundCallReason_Pause;
						}

						// make call checks

						if (eSCR != eSoundCallReason_NONE)
						{
							if (eSCR & eSoundCallReason_Interrupt)	// Прерывание - наивысший приоритет
							{ 
								if (ret)
								if (CheckInterrupt()) ret = false;
							}
							if (eSCR & eSoundCallReason_Rewind)		// Перемотку до паузы
							{
								if (ret)
								if (!CheckRewind(eSCR)) ret = false;
							}
							if (eSCR & eSoundCallReason_Pause)		// Пауза
							{
								if (ret)
								if (!CheckPause(eSCR)) ret = false;
							}
							if (eSCR & eSoundCallReason_VolumeAll)	// Звук (второстепенно)
							{
								if (ret)
								if (!CheckVolumeChange(eSCR)) ret = false;
							}
						}

						if (!ret) // error OR interrupt call
							_.playback_break_sleep = true;
					}

					// новая точка отсчёта
					_.playback_timepoint = _TIME;

					return ret;
				};
				auto ConvertMore = [this, &_] () -> bool
				{
					long hRes;

					auto ConvertMoreAAC = [this, &_, &hRes] (void * next_target) -> bool
					{
						return false;
					};
					auto ConvertMoreOGG = [this, &_, &hRes] (void * next_target) -> bool
					{
						return false;
					};
					auto ConvertMoreMP3 = [this, &_, &hRes] (void * next_target) -> bool
					{
						auto & ConverterMP3 = * _.ConverterMP3;
						auto & ConverterPCM = * _.ConverterPCM;

						// сдвинем начало mp3 на количество кадров с прошлого раза
						ConverterMP3.settings.in.start_frame = ConverterMP3.settings.out.loop_end_frame;

						// время на 1x уже заказано в настройке ранее, можно запускать конвертирование
						if (eMP3ConvStatus_Normal != (hRes = ConverterMP3.Continue()))
						{
							if (hRes == eMP3ConvStatus_BufferEnd)
							{
								//printf("\neMP3ConvStatus_BufferEnd");
								hRes = ePCMConvStatus_EndOfOrigin;
								_.playback_origin_end = true;
							}
							else
							{
								//printf("\nERROR : ConverterMP3->Run() error %i (_debug_2_MP3)", hRes);
								_.ERRCODE  = eSoundError_convMP3;
								_.ERRCODE |= hRes << eSoundError__MASK_CONVERTER_SHIFT;
								return false;
							}
						}

						//printf("\nmp3 converted samples - time %i - %f ", ConverterMP3->settings.out.samples, ConverterMP3->settings.out.time);

						if (!_.playback_origin_end)
						{
							// по идее должен оставаться старый, но, на всякий случай, будем обновлять
							ConverterPCM.settings.origin.buffer = ConverterMP3.settings.out.data_out;
							ConverterPCM.settings.origin.size   = ConverterMP3.settings.out.size_out;

							// переопределим [target]
							ConverterPCM.settings.target.buffer = next_target;

							if (ePCMConvStatus_Normal != (hRes = ConverterPCM.Run()))
							{
								//printf("\nERROR : ConverterPCM->Run() error %i (_debug_2_MP3-PCM)", hRes);
								_.ERRCODE  = eSoundError_convPCM;
								_.ERRCODE |= hRes << eSoundError__MASK_CONVERTER_SHIFT;
								return false;
							}
						}

						return true;
					};
					auto ConvertMorePCM = [this, &_, &hRes] (void * next_target) -> bool
					{
						auto & ConverterPCM = * _.ConverterPCM;

						// сдвинем начало [origin] на количество блоков семплов с прошлого раза

						//ConverterPCM->settings.origin.samples += ConverterPCM->settings.outInfo.origin_samples;
						//ConverterPCM->settings.origin.samples %= ConverterPCM->settings.outInfo.buffer_samples;
						ConverterPCM.settings.origin.samples = ConverterPCM.settings.outInfo.loop_end_sample;
						
						// переопределим [target]
						ConverterPCM.settings.target.buffer  = next_target; // заказанное время на 1x сек. либо время микшера X сек.

						//printf("\norigin.samples %i", ConverterPCM->settings.origin.samples);

						if (ePCMConvStatus_Normal != (hRes = ConverterPCM.Run()))
						{
							if (hRes == ePCMConvStatus_EndOfOrigin)
							{
								//printf("\nePCMConvStatus_EndOfOrigin");
								_.playback_origin_end = true;
							}
							else
							{
								//printf("\nERROR : ConverterPCM->Run() error %i (_debug_2_)", hRes);
								_.ERRCODE  = eSoundError_convPCM;
								_.ERRCODE |= hRes << eSoundError__MASK_CONVERTER_SHIFT;
								return false;
							}
						}

						if (_.playback_mixer) // при совместной работе с микшером
						{
							_.pMixer->rw_pos    = ConverterPCM.settings.outInfo.loop_end_sample;
							_.pMixer->rw_count += ConverterPCM.settings.outInfo.origin_samples;

							// ограничение от переполнения
							if (_.pMixer->rw_count > MIXER_MAX_BUFTIME_SAMPLES)
								_.pMixer->rw_count = MIXER_MAX_BUFTIME_SAMPLES;
						}

						return true;
					};

					/////////// Если в окне не хватает данных на 1х секунд, сконвертировать ещё на +1х секунд ///////////

					if (!_.playback_origin_end) // ещё не достигли конца, либо у нас зациклено loop
					{
						bool need_convertion;

						if (_.playback_mixer)
							 need_convertion = (_.window_size + _.window_shift) < _.audiocard_buffer_f2b_m  ; // сделаем ещё +X  сек. (время микшера)
						else need_convertion = (_.window_size + _.window_shift) < _.playback_1x             ; // сделаем ещё +1х сек.
					
						if (need_convertion)
						{
							byte * window_ptr_temp = _.window_ptr + _.window_size; // рабочий указатель, куда конвертировать
							bool   window_end_max  = false; // признак недостатка места в конце окна
							bool   bConv;

							//printf("\nAvailable window place : %i frames", (_.window_end - window_ptr_temp) / _.audiocard_buffer_f2b);
				
							if (  // Если места в конце окна не хватает для записи ещё 1x сек. (не микшер) ИЛИ
								  (( (uint32)(_.window_end - window_ptr_temp) < _.playback_1x )     )//       && !_.playback_mixer) ||
								  // Если места в конце окна не хватает для записи ещё +X сек. (время микшера)
							//	  (( (uint32)(_.window_end - window_ptr_temp) < _.audiocard_buffer_f2b_m ) &&  _.playback_mixer)
							   )
							{
								// Будем записывать в начало окна + поправка на (возможно) уже лежащие там shift-данные
								window_ptr_temp = _.window + _.window_shift;
								window_end_max  = true;
							}

								 if (_.ConverterAAC != nullptr) bConv = ConvertMoreAAC(window_ptr_temp);					
							else if (_.ConverterOGG != nullptr) bConv = ConvertMoreOGG(window_ptr_temp);						
							else if (_.ConverterMP3 != nullptr) bConv = ConvertMoreMP3(window_ptr_temp);						
							else
							{
								if (_.playback_mixer) // блок на разделяемые с микшером данные
								{
									glock lock_read_write (_.pMixer->m_ReadWrite);
									bConv = ConvertMorePCM(window_ptr_temp);
								}
								else
									bConv = ConvertMorePCM(window_ptr_temp);							
							}

							if (!bConv) // ошибка конвертации
								return false;

							// определим, сколько сделали по факту
							if (!_.playback_origin_end)
							{
								//printf("\ntarget_samples %i", ConverterPCM->settings.outInfo.target_samples);

								uint32 converted_size = (hRes == ePCMConvStatus_EndOfOrigin) ? 0 :
									_.ConverterPCM->settings.outInfo.target_samples * _.audiocard_buffer_f2b;

								//printf("\nconverted_size %i", converted_size);

								if (window_end_max) _.window_shift += converted_size;
								else                _.window_size  += converted_size;
							}
						}
					}

					return true;
				};
				auto WriteBuffer = [this, &_] () -> bool
				{
					uint32 audiocard_buffer_available_bytes; // доступно байт для записи

					auto ManageWindowGet     = [this, &_, &audiocard_buffer_available_bytes] () -> bool
					{
						long hRes;

						uint32 audiocard_buffer_padding;   // кадров имеется
						uint32 audiocard_buffer_available; // кадров доступно для записи

						if (S_OK != (hRes = _.pClient->GetCurrentPadding(&audiocard_buffer_padding)))
						{
							// hRes AUDCLNT_E_DEVICE_INVALIDATED  The audio endpoint device unavailable for use (eg. unplugged)
							_.ERRCODE = eSoundError_padding;
							return false;
						}

						audiocard_buffer_available       = _.audiocard_buffer_frames    -   audiocard_buffer_padding;
						audiocard_buffer_available_bytes =   audiocard_buffer_available * _.audiocard_buffer_f2b;

						if (_.playback_mixer) // пересчет с учетом неполного заполнения (заполненность виртуальной области, определённой микшером)
						{
							// кадров не меньше, чем размер виртуальной области ?
							if (audiocard_buffer_padding >= _.audiocard_buffer_m)
								 audiocard_buffer_available_bytes = 0;
							else audiocard_buffer_available_bytes = (_.audiocard_buffer_m - audiocard_buffer_padding) * _.audiocard_buffer_f2b;
						}

						// дополнительная информация в выходные данные
						// НУЖНО ИСПРАВИТЬ
						// 1. d_time неправильно напрямую от паддинга считать (нужна дельта D между проходами)
						// 2. забыта поправка времени на loop
						// D = abs(текущий паддинг - (прошлый паддинг + сколько добавили));
						{
							float d_time = (float) (audiocard_buffer_padding * _.audiocard_buffer_f2b) / _.audiocard_buffer_f2b_1s;
							auto & state = _.pCaller->status;
							{
								glock lock (state.m_STATSELF);

								float time = state.STATSELF.time_t;
								//float d_time2 = (float)audiocard_buffer_available_bytes / _.audiocard_buffer_f2b_1s;
								//printf("\nTIME: %f -> %f : %f", time, time + d_time, time + d_time + d_time2);
								time += d_time;
								state.STATSELF.time_t = time;
								state.STATSELF.time_p = time / _.sound->duration;
							}
						}

						if (S_OK != (hRes = _.pRender->GetBuffer(audiocard_buffer_available, &_.audiocard_buffer)))
						{
							// hRes AUDCLNT_E_BUFFER_TOO_LARGE  exceeds the available buffer space (buffer size minus padding size)
							// hRes AUDCLNT_E_DEVICE_INVALIDATED  The audio endpoint device unavailable for use (eg. unplugged)
							_.ERRCODE = eSoundError_getBuf;
							return false;
						}
						//printf("\naudiocard_buffer_padding %i", audiocard_buffer_padding);
						//printf("\naudiocard_buffer_frames  %i", audiocard_buffer_available);

						return true;
					};
					auto ManageWindowSize    = [this, &_, &audiocard_buffer_available_bytes] () -> void
					{
						if (_.window_size)
						{
							uint32 window_delta = (_.window_size > audiocard_buffer_available_bytes) ?
								audiocard_buffer_available_bytes : _.window_size;

							//printf("\nwindow_size - window_delta %i - %i", window_size, window_delta);

							if (window_delta)
							{
								memcpy(_.audiocard_buffer, _.window_ptr, window_delta);
								_.window_ptr  += window_delta;
								_.window_size -= window_delta;
							}

							_.window_delta = window_delta;
						}
						else _.window_delta = 0; // понадобится для переопределения времени проигрывания
					};
					auto ManageWindowShift   = [this, &_, &audiocard_buffer_available_bytes] () -> void
					{
						if (!_.window_size && _.window_shift)
						{
							uint32 audiocard_buffer_available_bytes_for_shift = audiocard_buffer_available_bytes - _.window_delta;
						
							// если ещё осталось место, куда запихнуть
							if (audiocard_buffer_available_bytes_for_shift)
							{
								byte * audiocard_buffer_shift = _.audiocard_buffer + _.window_delta; // место для запихивания
								uint32 window_delta_shift;                                           // сколько удаётся запихнуть

								window_delta_shift = (_.window_shift > audiocard_buffer_available_bytes_for_shift) ?
									(audiocard_buffer_available_bytes_for_shift) : (_.window_shift);

								_.window_delta += window_delta_shift;

								//printf("\nwindow_shift - window_delta_shift %i - %i", window_shift, window_delta_shift);

								memcpy(audiocard_buffer_shift, _.window, window_delta_shift);
								_.window_ptr   = _.window       + window_delta_shift;
								_.window_size  = _.window_shift - window_delta_shift;
								_.window_shift = 0;
							}
						}
					};
					auto ManageWindowRelease = [this, &_]                                    () -> bool
					{
						long hRes;

						//printf("\nwindow_delta %i", window_delta);

						if (S_OK != (hRes = _.pRender->ReleaseBuffer(
							//audiocard_buffer_available,
							_.window_delta / _.audiocard_buffer_f2b, // audiocard_buffer_writed_frames
							_.audiocard_buffer_flags)))
						{
							// hRes AUDCLNT_E_DEVICE_INVALIDATED  The audio endpoint device unavailable for use (eg. unplugged)
							_.ERRCODE = eSoundError_releaseBuf;
							return false;
						}

						return true;
					};

					if (!ManageWindowGet()) return false;

					ManageWindowSize();  // сначала основной window_size (конец окна)
					ManageWindowShift(); // теперь дополнительный window_shift (начало окна)

					if (!ManageWindowRelease()) return false;

					return true;
				};

				if (!MakeSleep())   break;
				if (!ConvertMore()) break;
				if (!WriteBuffer()) break;

				auto CheckExit = [this, &_, pAudioData, Stop] () -> void
				{
					// условие выхода при нормальной отработке
					if (_.playback_origin_end &&
						_.window_size  == 0   &&
						_.window_shift == 0 )
					{ 
						// дополнительная информация
						_.pCaller->status.UpdateInfoTime(_.sound->duration, 1.f);

						// заказано вернуться на начало и встать в паузу
						if (pAudioData->set.stop_at_end)
						{
							Stop();                        //printf("\nGoing to end pause");
							_.playback_origin_end = false; //printf("\nEnd pause test stop 5 sec.");  _SLEEP(5000);
							_.playback_end_return = true;
						}
						else
							_.audiocard_buffer_flags = AUDCLNT_BUFFERFLAGS_SILENT;
					}
				};

				CheckExit();
			}

			//////////////////////////////

			/////////// Выход  ///////////

			//////////////////////////////

			if (_.ERRCODE <= eSoundError_Unknown && 
				_.audiocard_buffer_flags == AUDCLNT_BUFFERFLAGS_SILENT)
			{
				if (_.playback_interrupt == false)
				{
					int32 sleep_time = _.playback_duration - (int32)_TIMER(_.playback_timepoint);
					if (sleep_time > _.playback_duration)
						sleep_time = _.playback_duration;
					if (sleep_time > 0)
						_SLEEP(sleep_time);

					   _.ERRCODE = eSoundError_Normal_1;
				} else _.ERRCODE = eSoundError_Normal_2;
			}
			//else // TODO - добавить анализ причины выхода и запомнить, если причина в кривых данных
				// _.sound->ERR = _.ERRCODE;

			Stop();
			Reset();

			_RETURN;

			#undef _RETURN
			#undef _WINDOW_S
			#undef _CONV_1x
			#undef _CONV_2x
			#undef _AWAKE_FREQ
		}

		//>> 
		error_t Record(CAudioData * pAudioData)
		{
			return eSoundError_Unknown;
		}

	protected:
		//>> Запуск воспроизведения :: возвращает код выхода <eSoundPlayError>
		error_t RunPlayback(SNDCHUNK * list) override final
		{
			CAudioData * pAudioData = nullptr;

			int N = MISSING; // client №
			error_t hRes = eSoundError_Unknown;

			if (_ACQ_LD(a_RenderChanged)) // заблокировано DefaulDeviceChanged()
			{
				hRes = eSoundError_RENDERREINIT;
			}
			else
			{
				{
					glock others_will_wait (m_RunPlayback); // изолируем вызов

					if (render.GetAvailableClient(N)) // get exist client
					{
						//printf("\nDEBUG : %i client grabbed", N);
					}
					//else if (render.AddClient(N)) // add new client
					//{
					//	//printf("\nDEBUG : Added %i client", N);
					//}
					else printf("\nDEBUG : NO MORE ROOM --> task will be dropped");

					if (_NOMISS(N))
					{
						render.client[N].Lock(); // заняли место

						pAudioData = render.client[N].out;

						auto & _out = pAudioData->set;
						auto & _in  = * list;

						_out.caller        = _in.caller;
						_out.actual_sound  = _in.actual_sound;
						_out.loop          = _in.loop;
						_out.stop_at_end   = _in.stop_at_end;
						_out.stop_at_start = _in.stop_at_start;
						_out.mixer         = _in.mixer;					
						_out.mixer_p       = reinterpret_cast<decltype(_out.mixer_p)>(_in.mixer_p);
					}
				}

				hRes = Play(pAudioData);

				if (_NOMISS(N))
				{
					render.client[N].Unlock(); // освободили место
				}
				else // DEBUG этого не должно быть
				{
					hRes = eSoundError_RMAXCLIENTS;
				}
			}

		//	printf("\nSOUND PLAY EXIT CODE = %i",  hRes & eSoundError__MASK_PLAYER_ERROR);
		//	printf("\nCONVERTER EXTRA CODE = %i", (hRes & eSoundError__MASK_CONVERTER_ERROR)
		//		>> eSoundError__MASK_CONVERTER_SHIFT);

			return hRes;
		}

		//>> Запуск захвата :: возвращает код выхода <eSoundPlayError>
		error_t RunRecord(uint64 id) override final
		{
			CAudioData * pAudioData = nullptr;

			int N = MISSING; // client №
			error_t hRes = eSoundError_Unknown;

			if (_ACQ_LD(a_CaptureChanged))
				hRes = eSoundError_CAPTUREREINIT;
			else
			{
				{
					glock others_will_wait (m_RunRecord); // изолируем вызов

					if (capture.GetAvailableClient(N)) // get exist client
					{
						//printf("\n%i client grabbed", N);
					}
					//else if (capture.AddClient(N)) // add new client
					//{
					//	//printf("\nAdded %i client", N);
					//}
					//else printf("\nNO MORE ROOM --> task will be dropped");

					if (_NOMISS(N))
					{
						capture.client[N].Lock(); // заняли место

						pAudioData = capture.client[N].out;

						//pAudioData->set.id = id;
					}
				}

				hRes = Record(pAudioData);

				if (_NOMISS(N))
				{
					capture.client[N].Unlock(); // освободили место
				}
				else // этого не должно быть
				{
					hRes = eSoundError_CMAXCLIENTS;
				}
			}

			printf("\nSOUND RECORD EXIT CODE = %i",  hRes & eSoundError__MASK_PLAYER_ERROR);

			return hRes;
		}

	public:
		//>> Возвращает статус общей инициализации
		bool GetStateInit() override final
		{
			bool hRes;

			{
				rglock guard (m_init_close);

				hRes = isInit;
			}

			return hRes;
		}
		//>> Сообщает актуальное число каналов вывода
		byte GetNumChannels() override final
		{
			byte hRes;

			{
				rglock guard (m_init_close);

				hRes = (byte) render.mode->nChannels;
			}

			return hRes;
		}
		//>> Возвращает статус по захвату звука (capture)
		bool GetStateMicrophone() override final
		{
			bool hRes;

			{
				rglock guard (m_init_close);

				hRes = state_capture;
			}

			return hRes;
		}
		//>> Возвращает статус по выводу звука (render)
		bool GetStateSpeakers() override final
		{
			bool hRes;

			{
				rglock guard (m_init_close);

				hRes = state_render;
			}

			return hRes;
		}

	};
}

#endif // #define _SOUNDWAS_H