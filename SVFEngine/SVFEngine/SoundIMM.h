// ----------------------------------------------------------------------- //
//
// MODULE  : SoundIMM.h
//
// PURPOSE : Воспроизведение звука средствами Windows Core Audio APIs
//			 - Multimedia Device (MMDevice) API
//           - Windows Audio Session API (WASAPI)
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _SOUNDIMM_H
#define _SOUNDIMM_H

#include "Sound.h"
#include "SoundDevice.h"
#include "SoundIMMN.h"

#include "version_conv.h"

#define UNK_PROPKEY_WSTR L"UNKNOWN"

#define ERROR_IMMStateChanged         L"[IMMStateChanged] Не удаётся найти звуковое устройство в списке"
#define ERROR_IMMDefaultDeviceChanged L"[IMMDefaultDeviceChanged] Не удаётся найти звуковое устройство в списке"
#define ERROR_IMMPropertyValueChanged L"[IMMPropertyValueChanged] Не удаётся найти звуковое устройство в списке"
#define ERROR_IMMDeviceRemoved        L"[IMMDeviceRemoved] Не удаётся найти звуковое устройство в списке"

#define   RENDER_DEVICE_FLOW  EDataFlow::eRender
#define  CAPTURE_DEVICE_FLOW  EDataFlow::eCapture
#define   RENDER_DEVICE_ROLE  ERole::eConsole
#define  CAPTURE_DEVICE_ROLE  ERole::eConsole //ERole::eCommunications

namespace SAVFGAME
{
	class CAudioSessionManager;

	class CSoundIMM : public CSound, public CSoundDevice
	{
		friend class CAudioSessionManager;
	private:
		IMMDeviceEnumerator *	pEnumerator;		// обеспечивает точку входа к перечислению звуковых устройств
		IMMDeviceCollection *	pRenderDevices;		// набор звуковых устройств вывода
		IMMDeviceCollection *	pCaptureDevices;	// набор звуковых устройств ввода
		CMMNotificationClient * pNotify;			// обеспечивает системные уведомления об изменении состояний устройств
		IMMDevice *             devRender;			// интерфейс текущего (default) устройства вывода
		IMMDevice *             devCapture;			// интерфейс текущего (default) устройства ввода
	private:
		THREADPOOL devChPool;   // исполняет DefaulDeviceChanged() 
		mutex  m_updateRender;	// останов для обновлений
		mutex  m_updateCapture;	// останов для обновлений
		uint32 counter_devRenderChanged  {0}; // счетчик смен устройства (не сбрасывается)
		uint32 counter_devCaptureChanged {0}; // счетчик смен устройства (не сбрасывается)
		bool isInit;
	public:
		CSoundIMM() : CSound(), CSoundDevice(),
					isInit(false),
					pEnumerator(nullptr),
					pRenderDevices(nullptr),
					pCaptureDevices(nullptr),
					pNotify(nullptr),
					devRender(nullptr),
					devCapture(nullptr)
		{};
		virtual ~CSoundIMM() override { Close(); };

		virtual void Close() override
		{
			if (!isInit) return;

			// Сначала уберём обработку входящих сообщений, т.к. она идёт в другом потоке
			if (pNotify) pEnumerator->UnregisterEndpointNotificationCallback(pNotify);
		
			_RELEASE(pNotify);
			_RELEASE(pEnumerator);
			_RELEASE(pRenderDevices);
			_RELEASE(pCaptureDevices);
			_RELEASE(devRender);
			_RELEASE(devCapture);

			propRenderDevice.erase(propRenderDevice.begin(), propRenderDevice.end());
			propCaptureDevice.erase(propCaptureDevice.begin(), propCaptureDevice.end());

			numRenderDevices  = 0;
			numCaptureDevices = 0;
			numRenderDefault  = MISSING;
			numCaptureDefault = MISSING;

			CoUninitialize(); // Закрываем COM библиотеку

			CSound::Close();

			isInit = false;
		};
		virtual bool Init() override
		{
			if (isInit) return true;

			if (!CSound::Init()) return false;

			
			InitCOMLibrary();
			InitDevEnumerator();
			InitDevCollections();
			InitDevicesStringIDs();
			InitDefaultDevicesInterfaces();

			GetDevicesStates();
			GetDevicesPropreties();	// После этого при выходе может выскакивать такое : The thread 0x11b4 has exited with code -1073741749 (0xc000004b).

			// https://www.gamedev.net/forums/topic/577421-c-interpreting-exit-codes/   STATUS_THREAD_IS_TERMINATING  0xc000004b
			// It's caused by funky goings on in MMDevApi.dll, independent of XAudio.
			// Basically, it calls into setupapi.dll to free a buffer after setupapi.dll has already had the DllMain(DLL_PROCESS_DETACH) notification.
			// This buffer freeing tries to enter a critical section that's already been deleted, however the memory of the critical section still
			// looks valid enough for Windows to think it's ok and locked by a different thread.Just before it tries to wait for it, there's a check
			// to see if the process is exiting (which it is, as all this happens deep in ExitProcess after main has returned) and so instead of an
			// incompletable wait, NtTerminateProcess is called with STATUS_THREAD_IS_TERMINATING which ends up as the thread and process return value.

			SetDefaultAsCurrentDevice(eSDTypeBOTH);

			// Базовая работа с данными в текущем потоке завершена
			// Теперь можно реагировать на уведомления в другом потоке
			InitNotificationInterface();
				
			//PrintfDevicesPropreties(MISSING, eSDTypeUnknown);
			//getchar();

			//PrintfDevicesPropreties(numRenderDefault, eSDTypeRender);
			//PrintfDevicesPropreties(numCaptureDefault, eSDTypeCapture);

			//*/

			return isInit = true;
		}

	protected:
		//>> Возвращает текущее устройство ввода или вывода и его настройки для shared mode
		void GetDevice(eSoundDevType type, IMMDevice* & pDevice, SNDDESC & modeShared)
		{
			switch (type) // На время блокируем возможность обновления из IncomingNotification()
			{
			case eSDTypeRender:
				{
					glock lock(m_updateRender);
					pDevice = devRender;
					modeShared = propRenderDevice[numRenderDefault].modeShared;
					break;
				}
			case eSDTypeCapture:
				{
					glock lock(m_updateCapture);
					pDevice = devCapture;
					modeShared = propCaptureDevice[numCaptureDefault].modeShared;
					break;
				}
			default:
				pDevice = nullptr;
				modeShared.Reset();
				_MBM(ERROR_FormatRestricted);
			}
		}

	protected:
		//>> Сообщает классу-потомку о смене устройства (производится в отдельном потоке)
		virtual void DefaulDeviceChanged(eSoundDevType type, uint32 iteration) override = 0;

	private:
		//>> Обработка событий устройств (выполняется в отдельном потоке)
		void IncomingNotification(eSoundDevNotification message, void * pData) override final
		{
			auto StateChanged_proc         = [this] (NotificationData * pData) -> void
			{
				//wprintf(L"\n[IMMNotify] Device state has been changed");

				uint32 N;
				eSoundDevType type;

				if (ReadDeviceNumber(pData->devID.c_str(), N, type))
				{
				/*	wprintf(L"\n");
					wprintf(L"\nID: %s", pData->devID.c_str());
					wprintf(L"\nN: %i", N);
					if ( (type == eSDTypeRender  && N == numRenderDefault) ||
						 (type == eSDTypeCapture && N == numCaptureDefault) )
						wprintf(L" (DEFAULT DEVICE)"); //*/

					SoundDeviceProp * prop = nullptr;
					     if (type == eSDTypeRender)  prop = &propRenderDevice[N];
					else if (type == eSDTypeCapture) prop = &propCaptureDevice[N];
					else { _MBM(ERROR_PointerNone); return; }

				/*	wprintf(L"\nSTATE: ");
					switch (prop->state)
					{
					case eSoundDevActive:     wprintf(L"Active [OLD] / ");      break;
					case eSoundDevDisabled:   wprintf(L"Disabled [OLD] / ");    break;
					case eSoundDevNotPresent: wprintf(L"NotPresent [OLD] / ");  break;
					case eSoundDevUnlugged:   wprintf(L"Unlugged [OLD] / ");    break;
					default:                  wprintf(L"UNKNOWN [OLD] / ");     break;
					} //*/

					ConvertState(pData->newState, prop->state);

				/*	switch (prop->state)
					{
					case eSoundDevActive:     wprintf(L"Active [NEW]");      break;
					case eSoundDevDisabled:   wprintf(L"Disabled [NEW]");    break;
					case eSoundDevNotPresent: wprintf(L"NotPresent [NEW]");  break;
					case eSoundDevUnlugged:   wprintf(L"Unlugged [NEW]");    break;
					default:                  wprintf(L"UNKNOWN [NEW]");     break;
					}
					wprintf(L"\n");
					PrintfDevicesPropreties(N, type); //*/
					
				}
				else _MBM(ERROR_IMMStateChanged);
			};
			auto DefaultDeviceChanged_proc = [this] (NotificationData * pData) -> void
			{
				//wprintf(L"\n[IMMNotify] User changes role of an default audio endpoint device");

				uint32 N;
				eSoundDevType type;

				// Игнорирую прочие роли
				if ((pData->flow == RENDER_DEVICE_FLOW  && pData->role != RENDER_DEVICE_ROLE) ||
					(pData->flow == CAPTURE_DEVICE_FLOW && pData->role != CAPTURE_DEVICE_ROLE))
					return;

				if (ReadDeviceNumber(pData->devID.c_str(), N, type))
				{	
					uint32 iteration;

					switch (type) // На время блокируем получение данных из GetDevice()
					{
					case eSDTypeRender:
						{
							//wprintf(L"\nDEFAULT AUDIO RENDER OLD/NEW : %i / %i", numRenderDefault, N);
							{
								glock lock(m_updateRender);
								numRenderDefault = N;
								SetDefaultAsCurrentDevice(type);
								counter_devRenderChanged++;
								iteration = counter_devRenderChanged;
							}
							break;
						}
					case eSDTypeCapture:
						{
							//wprintf(L"\nDEFAULT AUDIO CAPTURE OLD/NEW : %i / %i", numCaptureDefault, N);
							{
								glock lock(m_updateCapture);
								numCaptureDefault = N;
								SetDefaultAsCurrentDevice(type);
								counter_devCaptureChanged++;
								iteration = counter_devCaptureChanged;
							}
							break;
						}
					default:
						_MBM(L"DEVICE TYPE ERROR");
					}

					// Сообщаем выше, но не ждём, иначе можем застрять (в частности при ManageSession::ManageClient::Release() : _RELEASE(p))
					//std::thread(&CSoundIMM::DefaulDeviceChanged, this, type, iteration).detach();
					devChPool(&CSoundIMM::DefaulDeviceChanged, this, type, iteration);
				}
				else _MBM(ERROR_IMMDefaultDeviceChanged);
			};
			auto PropertyValueChanged_proc = [this] (NotificationData * pData) -> void
			{
				//wprintf(L"\n[IMMNotify] User changes value of a property of an audio endpoint device");

				uint32 N;
				eSoundDevType type;
				eSoundDevProp devProp;

				if (ReadDeviceNumber(pData->devID.c_str(), N, type))
				{
					if (ConvertProprety(pData->propKey, devProp))
					{
					//	PrintfDeviceProprety(N, type, 1, eSDP_NONE);
					//
					//	wprintf(L"\nOLD PROPERTY");
					//	if (devProp == eSDP_SKIPPED)
					//		PrintfDeviceProprety(N, type, 0, devProp); // have no MEM of skipped prop
					//	else
					//		PrintfDeviceProprety(N, type, 0, devProp);
						
						// Обновление свойства
						if ( (devProp == eSDPShareModeFormat) &&
							 ( (type == eSDTypeRender  && N == numRenderDefault) ||
							   (type == eSDTypeCapture && N == numCaptureDefault)
							 )
						   )
						{
							switch (type) // На время блокируем получение данных из GetDevice()
							{
							case eSDTypeRender:
								{
									glock lock(m_updateRender);
									ReadDeviceProprety(N, type, devProp);
									break;
								}
							case eSDTypeCapture:
								{
									glock lock(m_updateCapture);
									ReadDeviceProprety(N, type, devProp);
									break;
								}
							}
						}
						else
							ReadDeviceProprety(N, type, devProp);

					//	wprintf(L"\nNEW PROPERTY");
					//	if (devProp == eSDP_SKIPPED)
					//		PrintfDevicePropertyRAW(pData->devID.c_str(), pData->propKey, true);
					//	else
					//		PrintfDeviceProprety(N, type, 0, devProp);
					}
					//else wprintf(L"\n[.........] But property is unknown...");
				}
				else _MBM(ERROR_IMMPropertyValueChanged);
			};
			auto DeviceAdded_proc          = [this] (NotificationData * pData) -> void
			{
				//wprintf(L"\n[IMMNotify] Added new audio endpoint device");

				uint32 N;
				eSoundDevType type;
				SoundDeviceProp * prop = nullptr;
				bool returned = false;

				if (ReadDeviceNumber(pData->devID.c_str(), N, type)) // Устройство уже было ранее, его снова вернули
				{
						 if (type == eSDTypeRender)  prop = &propRenderDevice[N];
					else if (type == eSDTypeCapture) prop = &propCaptureDevice[N];
					else { _MBM(ERROR_PointerNone); return; }

					returned = true;
				}
				else // Новое устройство в системе
				{
					// Выясним из какой коллекции устройство (render/capture)

					//type = eSDTypeCapture;
					//UINT currentRenderDevices = 0;
					//pRenderDevices->GetCount(&currentRenderDevices);
					//for (UINT i = 0; i < currentRenderDevices; i++)
					//{
					//	IMMDevice*	pDevice;
					//	wchar_t*	devID;
					//
					//	if (S_OK != pRenderDevices->Item(i, &pDevice))
					//		_MBM(L"Не удалось Collection->Item()");
					//	pDevice->GetId(&devID);
					//	if (!pData->devID.compare(devID))
					//		type = eSDTypeRender;
					//	CoTaskMemFree(devID);
					//	_RELEASE(pDevice);
					//	if (type == eSDTypeRender) break;
					//}

					if (!ReadDeviceType(pData->devID.c_str(), type))
						return; // ERROR exit

					// Добавим новую ячейку свойств и получим указатель

					if (type == eSDTypeRender)
					{
						N = numRenderDevices;
						propRenderDevice.resize(++numRenderDevices);
						prop = &propRenderDevice[N];
					}
					else
					{
						N = numCaptureDevices;
						propCaptureDevice.resize(++numCaptureDevices);
						prop = &propCaptureDevice[N];
					}
				}

				// Обновим данные об устройстве

				prop->ID = pData->devID;
				ReadDeviceState(N, type, prop->state);

				if ( (returned) &&
					 ( (type == eSDTypeRender  && N == numRenderDefault) ||
					   (type == eSDTypeCapture && N == numCaptureDefault)
					 )
				   )
				{
					switch (type) // На время блокируем получение данных из GetDevice()
					{
					case eSDTypeRender:
						{
							glock lock(m_updateRender);
							ReadDeviceProprety(N, type, eSDP_ALL);
							break;
						}
					case eSDTypeCapture:
						{
							glock lock(m_updateCapture);
							ReadDeviceProprety(N, type, eSDP_ALL);
							break;
						}
					}
				}
				else
				{
					ReadDeviceProprety(N, type, eSDP_ALL);
				}

			};
			auto DeviceRemoved_proc        = [this] (NotificationData * pData) -> void
			{
				//wprintf(L"\n[IMMNotify] User removed audio endpoint device");

				uint32 N;
				eSoundDevType type;

				if (ReadDeviceNumber(pData->devID.c_str(), N, type))
				{
					SoundDeviceProp * prop = nullptr;
						 if (type == eSDTypeRender)  prop = &propRenderDevice[N];
					else if (type == eSDTypeCapture) prop = &propCaptureDevice[N];
					else { _MBM(ERROR_PointerNone); return; }

					// Просто отметим себе, что его больше нет
					prop->state = eSoundDevRemoved;
				}
				else _MBM(ERROR_IMMDeviceRemoved);
			};

			switch (message)
			{
			case eSDNotifyDeviceStateChanged:
				StateChanged_proc((NotificationData*)pData);
				break;
			case eSDNotifyDefaultDeviceChanged:
				DefaultDeviceChanged_proc((NotificationData*)pData);
				break;
			case eSDNotifyPropertyValueChanged:
				PropertyValueChanged_proc((NotificationData*)pData);
				break;
			case eSDNotifyDeviceAdded:
				DeviceAdded_proc((NotificationData*)pData);
				break;
			case eSDNotifyDeviceRemoved:
				DeviceRemoved_proc((NotificationData*)pData);
				break;
			}
		}

	private:
		//>> Инициализация COM-библиотеки
		void InitCOMLibrary()
		{
			//if (S_OK != CoInitialize(NULL)) 
			//	_MBM(L"Не удалось CoInitialize() [COM library]");

			// Инициализируем COM библиотеку в режиме доступа другим потокам
			if (S_OK != CoInitializeEx(NULL, COINIT_MULTITHREADED)) 
				_MBM(L"Не удалось CoInitializeEx() [COM library]");
		}

		//>> Создание интерфейса перечисления устройств
		void InitDevEnumerator()
		{
			const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
			const   IID  IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
			if (S_OK != CoCreateInstance(
					CLSID_MMDeviceEnumerator, // The CLSID associated with the data and code that will be used to create the object
					NULL,					  // If non-NULL, pointer to the aggregate object's IUnknown interface (the controlling IUnknown).
					CLSCTX_INPROC,			  // Context in which the code that manages the newly created object will run
					IID_IMMDeviceEnumerator,  // A reference to the identifier of the interface to be used to communicate with the object
					(void**)&pEnumerator      // Return *ppv contains the requested interface pointer. Upon failure, *ppv contains NULL.
					))
				_MBM(L"Не удалось CoCreateInstance() [MMDevice]");
		}

		//>> Получения набора устройств ввода (capture) и вывода (render)
		void InitDevCollections()
		{
			if (S_OK != pEnumerator->EnumAudioEndpoints(eRender, // Audio data flows from the application to the audio endpoint device, which renders the stream
				DEVICE_STATEMASK_ALL, // DEVICE_STATEMASK  _ALL _ACTIVE _DISABLED _NOTPRESENT _UNPLUGGED
				&pRenderDevices))
				_MBM(L"Не удалось EnumAudioEndpoints() [pRenderDevices]");

			if (S_OK != pEnumerator->EnumAudioEndpoints(eCapture, // Audio data flows from the audio endpoint device that captures the stream, to the application
				DEVICE_STATEMASK_ALL, // Audio data flows:  eRender / eCapture / eAll
				&pCaptureDevices))
				_MBM(L"Не удалось EnumAudioEndpoints() [pCaptureDevices]");

			// Запомним количество устройств
			pRenderDevices->GetCount(&numRenderDevices);	//printf("\n%i audio render devices", numRenderDevices);
			pCaptureDevices->GetCount(&numCaptureDevices);	//printf("\n%i audio capture devices", numCaptureDevices);
		}

		//>> Регистрация интерфейса получения системных уведомлений об изменении состояния устройств в системе
		void InitNotificationInterface()
		{
			// Создаём класс уведомлений о событии
			pNotify = new /*_NEW_NOTHROW*/ CMMNotificationClient(this);

			if (pNotify == nullptr)
				_MBM(L"Не удалось RegisterEndpointNotificationCallback()");

			// Регистрируем класс уведомлений о событии
			if (S_OK != pEnumerator->RegisterEndpointNotificationCallback(pNotify))
				_MBM(L"Не удалось RegisterEndpointNotificationCallback()");
		}

		//>> Получение набора строковых идентификаторов устройств
		void InitDevicesStringIDs()
		{
			auto InitDevicesStringIDs_proc = [this]
				(vector<SoundDeviceProp> & prop, uint32 numDevices, IMMDeviceCollection * pCollection) -> void
			{
				if (!numDevices) _MBM(ERROR_NUMZERO);
				if (pCollection == nullptr) _MBM(ERROR_PointerNone);

				prop.resize(numDevices);

				for (uint32 i = 0; i < numDevices; i++)
				{
					IMMDevice*	pDevice;	// temp
					wchar_t*	devID;		// temp

					auto & DEV = prop[i];

					if (S_OK != pCollection->Item(i, &pDevice))
						_MBM(L"Не удалось Collection->Item()");
					pDevice->GetId(&devID); // if false -> ??
					DEV.ID = devID;
					CoTaskMemFree(devID);

					_RELEASE(pDevice);
				}
			};

			// Выясним ID устройств, чтобы позже к ним обращаться

			for (int step = 0; step < 2; step++)
			{
				switch (step)
				{
				case 0:
					InitDevicesStringIDs_proc(propRenderDevice, numRenderDevices, pRenderDevices);
					break;
				case 1:
					InitDevicesStringIDs_proc(propCaptureDevice, numCaptureDevices, pCaptureDevices);
					break;
				}
			}
		}

		//>> Определение интерфейсов для устройств по умолчанию
		void InitDefaultDevicesInterfaces()
		{
			// If the method fails to find a rendering or capture device for the
			// specified role, this means that no rendering or capture device is available at all.If no device is
			// available, the method sets *ppEndpoint = NULL and returns ERROR_NOT_FOUND.
			//
			// ERole constant  Device role                    Rendering examples              Capture examples
			// eConsole        Interaction with the computer  Games and system notifications  Voice commands
			// eCommunications Voice communications           Chat and VoIP                   Chat and VoIP
			// eMultimedia     Playing or recording audio     Music and movies                Narration and live music recording

			auto InitDefaultDevicesInterfaces_proc = [this]
				(eSoundDevType type, uint32 numDevices) -> void
			{
				if (!numDevices) _MBM(ERROR_NUMZERO);

				IMMDevice* pDevice = nullptr;	// temp
				wchar_t* devID = nullptr;		// temp

				EDataFlow flow; // eRender, eCapture
				ERole     role; // eConsole, eMultimedia, eCommunications
				uint32 * numDevDefault         = nullptr;
				vector<SoundDeviceProp> * prop = nullptr;

				switch (type)
				{
				case eSDTypeRender:
					flow = RENDER_DEVICE_FLOW;
					role = RENDER_DEVICE_ROLE;
					numDevDefault = & numRenderDefault;
					prop          = & propRenderDevice;
					break;
				case eSDTypeCapture:
					flow = CAPTURE_DEVICE_FLOW;
					role = CAPTURE_DEVICE_ROLE;
					numDevDefault = & numCaptureDefault;
					prop          = & propCaptureDevice;
					break;
				default:
					_MBM(L"\nError eSoundDevType at InitDefaultDevicesInterfaces()!");
				}

				// Получим указатели на интерфейсы устройств по умолчанию и выясним их порядковые номера i в списках

				if (S_OK != pEnumerator->GetDefaultAudioEndpoint(
					flow,
					role, 
					& pDevice))
				_MBM(L"Не удалось GetDefaultAudioEndpoint()");

				pDevice->GetId(&devID);
				for (UINT i=0; i<numDevices; i++)
					if (!(*prop)[i].ID.compare(devID))
					{
						(*numDevDefault) = i;
						break;
					};
				CoTaskMemFree(devID);
				_RELEASE(pDevice);

				// Добавить проверку, что numDevDefault не MISSING ?
				
				//switch (type)
				//{
				//case eSDTypeRender:
				//	printf("\nDefault render device = %i", numDevDefault);
				//	break;
				//case eSDTypeCapture:
				//	printf("\nDefault capture device = %i", numDevDefault);
				//	break;
				//}
				
			};

			for (int step = 0; step < 2; step++)
			{
				switch (step)
				{
				case 0:
					InitDefaultDevicesInterfaces_proc(eSDTypeRender, numRenderDevices);
					break;
				case 1:
					InitDefaultDevicesInterfaces_proc(eSDTypeCapture, numCaptureDevices);
					break;
				}
			}
		}

		//>> Назначает помеченные в системе по умолчанию устройства как текущие (рабочие)
		bool SetDefaultAsCurrentDevice(eSoundDevType type)
		{
			if (!pEnumerator) { _MBM(ERROR_InitNone); return false; }

			if (type & eSDTypeRender)
			{
				_RELEASE(devRender);
				if (S_OK != pEnumerator->GetDevice(propRenderDevice[numRenderDefault].ID.c_str(), &devRender))
					{ _MBM(L"Не удалось pEnumerator->GetDevice()"); return false; }
			}

			if (type & eSDTypeCapture)
			{
				_RELEASE(devCapture);
				if (S_OK != pEnumerator->GetDevice(propCaptureDevice[numCaptureDefault].ID.c_str(), &devCapture))
					{ _MBM(L"Не удалось pEnumerator->GetDevice()"); return false; }
			}

			return true;
		};

		//>> Обновляет данные о свойствах устройств
		void GetDevicesPropreties()
		{
			auto GetDevicesPropreties_proc = [this]
				(eSoundDevType type, uint32 numDevices) -> void
			{
				if (!numDevices) _MBM(ERROR_NUMZERO);

				for (uint32 i=0; i<numDevices; i++)			{
					ReadDeviceProprety(i, type, eSDP_ALL);
					//PrintfDevicesPropretiesRAW(i, type);
				}			
			};

			for (int step = 0; step < 2; step++)
			{
				switch (step)
				{
				case 0:
					GetDevicesPropreties_proc(eSDTypeRender, numRenderDevices);
					break;
				case 1:
					GetDevicesPropreties_proc(eSDTypeCapture, numCaptureDevices);
					break;
				}
			}
		}

		//>> Обновляет данные о состоянии устройств
		void GetDevicesStates()
		{
			auto GetDevicesStates_proc = [this]
				(vector<SoundDeviceProp> & prop, uint32 numDevices) -> void
			{
				if (!numDevices) _MBM(ERROR_NUMZERO);

				for (uint32 i = 0; i < numDevices; i++)
				{
					auto & DEV = prop[i];
					ReadDeviceState(DEV.ID.c_str(), DEV.state);
				}
			};

			for (int step = 0; step < 2; step++)
			{
				switch (step)
				{
				case 0:
					GetDevicesStates_proc(propRenderDevice, numRenderDevices);
					break;
				case 1:
					GetDevicesStates_proc(propCaptureDevice, numCaptureDevices);
					break;
				}
			}
		}

	private:
		//>> [SUBFUNC] Возвращает <true> и тип устройства в случае успеха
		bool ReadDeviceType(IMMDevice * pDevice, eSoundDevType & type_out)
		{
			type_out = eSDTypeUnknown;

			if (pDevice == nullptr) { _MBM(ERROR_PointerNone); return false; }

			REFIID IID_IMMEndpoint = __uuidof(IMMEndpoint);
			
			IMMEndpoint * pDeviceEnd = nullptr;
			EDataFlow flow;
			
			if (S_OK != pDevice->QueryInterface(IID_IMMEndpoint, (void**)&pDeviceEnd))
				{ _MBM(L"Не удалось QueryInterface()"); return false; }
			if (S_OK != pDeviceEnd->GetDataFlow(&flow))
				{ _MBM(L"Не удалось GetDataFlow()"); return false; }

			switch (flow)
			{
			case EDataFlow::eRender:
				type_out = eSDTypeRender;
				break;
			case EDataFlow::eCapture:
				type_out = eSDTypeCapture;
				break;
			}

			_RELEASE(pDeviceEnd);
			return true;
		}

		//>> [SUBFUNC] Возвращает <true> и тип устройства в случае успеха
		bool ReadDeviceType(const wchar_t * ID, eSoundDevType & type_out)
		{
			if (!pEnumerator) { _MBM(ERROR_InitNone); return false; }

			IMMDevice * pDevice = nullptr;

			if (S_OK != pEnumerator->GetDevice(ID, &pDevice))
				{ _MBM(L"Не удалось pEnumerator->GetDevice()"); return false; }

			bool ret = ReadDeviceType(pDevice, type_out);

			_RELEASE(pDevice);
			return ret;
		}

		//>> [SUBFUNC] Возвращает <true> и текущее состояние устройства в случае успеха
		bool ReadDeviceState(uint32 N, eSoundDevType type, eSoundDevState & SDState)
		{
			if ((N > numRenderDevices  && type == eSDTypeRender) ||
				(N > numCaptureDevices && type == eSDTypeCapture))
			{ _MBM(ERROR_IDLIMIT); return false; }

			wstring ID;
			switch (type)
			{
			case eSDTypeRender:  ID = propRenderDevice[N].ID;  break;
			case eSDTypeCapture: ID = propCaptureDevice[N].ID; break;
			}

			return ReadDeviceState(ID.c_str(), SDState);
		}

		//>> [SUBFUNC] Возвращает <true> и текущее состояние устройства в случае успеха
		bool ReadDeviceState(const wchar_t * ID, eSoundDevState & SDState)
		{
			if (pEnumerator == nullptr)
				{ _MBM(ERROR_PointerNone); return false; }

			IMMDevice *	pDevice = nullptr;
			DWORD state;

			if (S_OK != pEnumerator->GetDevice(ID, &pDevice))
				{ _MBM(L"Не удалось pEnumerator->GetDevice()"); return false; }
			pDevice->GetState(&state);
			ConvertState(state, SDState);
			_RELEASE(pDevice);

			return true;
		}

		//>> [SUBFUNC] Возвращает <true> и обновляет свойство <eSoundDevProp> в случае успеха
		bool ReadDeviceProprety(uint32 N, eSoundDevType type, uint32 devProp)
		{
			if (devProp == eSDP_SKIPPED) return true;

			IMMDevice *			pDevice = nullptr;			// temp
			IPropertyStore *	pPropertyStore = nullptr;	// temp
			PROPVARIANT			propVar;					// temp

			vector<SoundDeviceProp> * SDProp;

			switch (type)
			{
			case eSDTypeRender:  SDProp = &propRenderDevice;  break;
			case eSDTypeCapture: SDProp = &propCaptureDevice; break;
			default: 
				wprintf(L"\nIncorrect eSoundDevType [ReadDeviceProprety]");
				return false;
			}

			if ((N > numRenderDevices  && type == eSDTypeRender) ||
				(N > numCaptureDevices && type == eSDTypeCapture))
				{ _MBM(ERROR_IDLIMIT); return false; }

			auto & DEV = (*SDProp)[N];

			//////////////////////////////////////////

			if (S_OK != pEnumerator->GetDevice(DEV.ID.c_str(), &pDevice))
				wprintf(L"\nНе удалось pEnumerator->GetDevice()");
			if (S_OK != pDevice->OpenPropertyStore(STGM_READ, &pPropertyStore))
				wprintf(L"\nНе удалось OpenPropertyStore()");
					
			//////// String types : VT_LPWSTR ////////

			auto GetWideString_proc = [this, &propVar, &pPropertyStore, &DEV]
				(eSoundDevProp eSDP) -> void
			{
				wstring * wstr = nullptr;
				PropVariantInit(&propVar);
				switch (eSDP)
				{
				case eSDPFriendlyName:
					if (S_OK != pPropertyStore->GetValue(PKEY_Device_FriendlyName, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ FriendlyName");
					wstr = & DEV.name;
					break;
				case eSDPFriendlyNameI:
					if (S_OK != pPropertyStore->GetValue(PKEY_DeviceInterface_FriendlyName, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ InterfaceFriendlyName");
					wstr = & DEV.iname;
					break;
				case eSDPInfoDescription:
					if (S_OK != pPropertyStore->GetValue(PKEY_Device_DeviceDesc, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ DeviceDesc");
					wstr = & DEV.info;
					break;
				case eSDPEndAssociation:
					if (S_OK != pPropertyStore->GetValue(PKEY_AudioEndpoint_Association, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ Association");
					wstr = & DEV.assoc;
					break;
				case eSDPPageProvider:
					if (S_OK != pPropertyStore->GetValue(PKEY_AudioEndpoint_ControlPanelPageProvider, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ PageProvider");
					wstr = & DEV.pageProv;
					break;
				case eSDPguid:
					if (S_OK != pPropertyStore->GetValue(PKEY_AudioEndpoint_GUID, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ GUID");
					wstr = & DEV.GUID;
					break;
				case eSDPJackSubType:
					if (S_OK != pPropertyStore->GetValue(PKEY_AudioEndpoint_JackSubType, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ JackSubType");
					wstr = & DEV.jackSType;
					break;
				case eSDPEnumeratorName:
					if (S_OK != pPropertyStore->GetValue(PKEY_Device_EnumeratorName, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ EnumeratorName");
					wstr = & DEV.enumarName;
					break;
				case eSDPIconPath:
					if (S_OK != pPropertyStore->GetValue(PKEY_DeviceClass_IconPath, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ IconPath");
					wstr = & DEV.iconPath;
					break;
				}
				(*wstr) =
					//((propVar.vt & VT_TYPEMASK) != VT_LPWSTR) ? L"NO_INFO" : propVar.pwszVal;
					(propVar.vt != VT_LPWSTR) ? L"NO_INFO" : propVar.pwszVal;
				PropVariantClear(&propVar);
			};

			if (devProp & eSDPFriendlyName)    GetWideString_proc(eSDPFriendlyName);
			if (devProp & eSDPFriendlyNameI)   GetWideString_proc(eSDPFriendlyNameI);
			if (devProp & eSDPInfoDescription) GetWideString_proc(eSDPInfoDescription);
			if (devProp & eSDPEndAssociation)  GetWideString_proc(eSDPEndAssociation);
			if (devProp & eSDPPageProvider)    GetWideString_proc(eSDPPageProvider);
			if (devProp & eSDPguid)            GetWideString_proc(eSDPguid);
			if (devProp & eSDPJackSubType)     GetWideString_proc(eSDPJackSubType);
			if (devProp & eSDPEnumeratorName)  GetWideString_proc(eSDPEnumeratorName);
			if (devProp & eSDPIconPath)        GetWideString_proc(eSDPIconPath);

			//////// Audio formats : VT_BLOB ////////

			auto GetFormat_proc = [this, &propVar, &pPropertyStore, &DEV]
				(eSoundDevProp eSDP) -> void
			{
				SNDDESC * DevMode = nullptr;
				PropVariantInit(&propVar);
				switch (eSDP)
				{
				case eSDPShareModeFormat:
					if (S_OK != pPropertyStore->GetValue(PKEY_AudioEngine_DeviceFormat, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ DeviceFormat");
					DevMode = & DEV.modeShared;
					break;
				case eSDPDefaultFormat:
					if (S_OK != pPropertyStore->GetValue(PKEY_AudioEngine_OEMFormat, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ OEMFormat");
					DevMode = & DEV.modeDefault;
					break;
				}
				//((propVar.vt & VT_TYPEMASK) != VT_BLOB)
				if (propVar.vt != VT_BLOB) // may be no such info --> clear mem
					DevMode->Reset(); 
				else
				{
					// WAVEFORMATEX 18
					{
						WAVEFORMATEX & format = *(WAVEFORMATEX*)propVar.blob.pBlobData;
						DevMode->cbSize          = format.cbSize;
						DevMode->nAvgBytesPerSec = format.nAvgBytesPerSec;
						DevMode->nBlockAlign     = format.nBlockAlign;
						DevMode->nChannels       = format.nChannels;
						DevMode->nSamplesPerSec  = format.nSamplesPerSec;
						DevMode->wBitsPerSample  = format.wBitsPerSample;
						DevMode->wFormatTag      = format.wFormatTag;
					}

					// FULL 40 = WAVEFORMATEX 18 + extra of WAVEFORMATEXTENSIBLE 22
					if (propVar.blob.cbSize == WAVEFORMATEXTENSIBLESIZE &&					// 40 FULL
						DevMode->cbSize == (WAVEFORMATEXTENSIBLESIZE - WAVEFORMATEXSIZE))	// 22 EXTRA
					{
						WAVEFORMATEXTENSIBLE & format = *(WAVEFORMATEXTENSIBLE*)propVar.blob.pBlobData;
						DevMode->_ex_SamplesUnion.wReserved = format.Samples.wReserved;
						DevMode->_ex_dwChannelMask          = format.dwChannelMask;
						DevMode->_ex_SubFormat.Data1        = format.SubFormat.Data1;
						DevMode->_ex_SubFormat.Data2        = format.SubFormat.Data2;
						DevMode->_ex_SubFormat.Data3        = format.SubFormat.Data3;
						for (int x = 0; x < 8; x++)
							DevMode->_ex_SubFormat.Data4[x] = format.SubFormat.Data4[x];
					}
				}
				PropVariantClear(&propVar);
			};

			if (devProp & eSDPShareModeFormat) GetFormat_proc(eSDPShareModeFormat);
			if (devProp & eSDPDefaultFormat)   GetFormat_proc(eSDPDefaultFormat);

			//////// Additional values info : VT_UI4 ////////

			auto GetUInt_proc = [this, &propVar, &pPropertyStore, &DEV]
				(eSoundDevProp eSDP) -> void
			{
				auto DefineFormFactor_proc = [this, &DEV] (uint32 ulVal) -> void
				{
					switch (ulVal)
					{
					case EndpointFormFactor::RemoteNetworkDevice:
						DEV.formFactor = eSDFormRemoteNetwork;
						break;
					case EndpointFormFactor::Speakers:
						DEV.formFactor = eSDFormSpeakers;
						break;
					case EndpointFormFactor::LineLevel:
						DEV.formFactor = eSDFormLineLevel;
						break;
					case EndpointFormFactor::Headphones:
						DEV.formFactor = eSDFormHeadphones;
						break;
					case EndpointFormFactor::Microphone:
						DEV.formFactor = eSDFormMicrophone;
						break;
					case EndpointFormFactor::Headset:
						DEV.formFactor = eSDFormHeadset;
						break;
					case EndpointFormFactor::Handset:
						DEV.formFactor = eSDFormHandset;
						break;
					case EndpointFormFactor::UnknownDigitalPassthrough:
						DEV.formFactor = eSDFormUnknownDigitalPassthrough;
						break;
					case EndpointFormFactor::SPDIF:
						DEV.formFactor = eSDFormSPDIF;
						break;
					case EndpointFormFactor::DigitalAudioDisplayDevice:
						DEV.formFactor = eSDFormDigitalAudioDisplayDevice;
						break;
					default:
						DEV.formFactor = eSDFormUnknown;
					}
				};
				auto DefineRange_proc = [this, &DEV] (uint32 ulVal, bool physical) -> void
				{
					eSoundDevRange DEV_range;
					switch (ulVal)
					{
					case KSAUDIO_SPEAKER_MONO: // KSAUDIO_SPEAKER_GROUND_FRONT_CENTER
						DEV_range = eSDRange_MONO;
						break;
					case KSAUDIO_SPEAKER_STEREO:
						DEV_range = eSDRange_STEREO;
						break;
					case KSAUDIO_SPEAKER_QUAD:
						DEV_range = eSDRange_QUAD;
						break;
					case KSAUDIO_SPEAKER_SURROUND:
						DEV_range = eSDRange_SURROUND;
						break;
					case KSAUDIO_SPEAKER_5POINT1: // KSAUDIO_SPEAKER_5POINT1_BACK
						DEV_range = eSDRange_5p1;
						break;
					case KSAUDIO_SPEAKER_7POINT1: // KSAUDIO_SPEAKER_7POINT1_WIDE
						DEV_range = eSDRange_7p1;
						break;
					case KSAUDIO_SPEAKER_5POINT1_SURROUND:
						DEV_range = eSDRange_5p1_SURROUND;
						break;
					case KSAUDIO_SPEAKER_7POINT1_SURROUND:
						DEV_range = eSDRange_7p1_SURROUND;
						break;
					case KSAUDIO_SPEAKER_GROUND_FRONT_LEFT:				// DVD speaker
						DEV_range = eSDRange_DVDS_GROUND_FRONT_LEFT;
						break;
				//	case KSAUDIO_SPEAKER_GROUND_FRONT_CENTER:			// DVD speaker KSAUDIO_SPEAKER_MONO
				//		DEV.range = eSDRange_DVDS_GROUND_FRONT_CENTER;
				//		break;
					case KSAUDIO_SPEAKER_GROUND_FRONT_RIGHT:			// DVD speaker
						DEV_range = eSDRange_DVDS_GROUND_FRONT_RIGHT;
						break;
					case KSAUDIO_SPEAKER_GROUND_REAR_LEFT:				// DVD speaker
						DEV_range = eSDRange_DVDS_GROUND_REAR_LEFT;
						break;
					case KSAUDIO_SPEAKER_GROUND_REAR_RIGHT:				// DVD speaker
						DEV_range = eSDRange_DVDS_GROUND_REAR_RIGHT;
						break;
					case KSAUDIO_SPEAKER_TOP_MIDDLE:					// DVD speaker
						DEV_range = eSDRange_DVDS_TOP_MIDDLE;
						break;
					case KSAUDIO_SPEAKER_SUPER_WOOFER:					// DVD speaker
						DEV_range = eSDRange_DVDS_SUPER_WOOFER;
						break;
					case KSAUDIO_SPEAKER_DIRECTOUT:
					default:
						DEV_range = eSDRangeDirectOut;
					}
					if (physical) DEV.rangePhys = DEV_range;
					else          DEV.rangeFull = DEV_range;
				};
				PropVariantInit(&propVar);
				switch (eSDP)
				{
				case eSDPSysFXState:
					if (S_OK != pPropertyStore->GetValue(PKEY_AudioEndpoint_Disable_SysFx, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ SysFx");
					//((propVar.vt & VT_TYPEMASK) != VT_UI4)
					if (propVar.vt == VT_UI4)
						DEV.bSysFX = _BOOL(propVar.ulVal);
					break;
				case eSDPFormFactor:
					if (S_OK != pPropertyStore->GetValue(PKEY_AudioEndpoint_FormFactor, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ FormFactor");
					//((propVar.vt & VT_TYPEMASK) == VT_UI4)
					if (propVar.vt == VT_UI4)
						DefineFormFactor_proc(propVar.uintVal);
					break;
				case eSDPRangeFull:
					if (S_OK != pPropertyStore->GetValue(PKEY_AudioEndpoint_FullRangeSpeakers, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ FullRangeSpeakers");
					//((propVar.vt & VT_TYPEMASK) == VT_UI4)
					if (propVar.vt == VT_UI4)
						DefineRange_proc(propVar.uintVal, false);
					break;
				case eSDPRangePhys:
					if (S_OK != pPropertyStore->GetValue(PKEY_AudioEndpoint_PhysicalSpeakers, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ PhysicalSpeakers");
					//((propVar.vt & VT_TYPEMASK) == VT_UI4)
					if (propVar.vt == VT_UI4)
						DefineRange_proc(propVar.uintVal, true);
					break;
				case eSDPEventDrivenMode:
					if (S_OK != pPropertyStore->GetValue(PKEY_AudioEndpoint_Supports_EventDriven_Mode, &propVar))
						wprintf(L"\nНе удалось pPropertyStore->GetValue() _ EventDrivenMode");
					//((propVar.vt & VT_TYPEMASK) == VT_UI4)
					if (propVar.vt == VT_UI4)
						DEV.bEvDrvMode = _BOOL(propVar.uintVal);
					break;
				}
				PropVariantClear(&propVar);
			};

			if (devProp & eSDPSysFXState)      GetUInt_proc(eSDPSysFXState);
			if (devProp & eSDPFormFactor)      GetUInt_proc(eSDPFormFactor);
			if (devProp & eSDPRangeFull)       GetUInt_proc(eSDPRangeFull);
			if (devProp & eSDPRangePhys)       GetUInt_proc(eSDPRangePhys);
			if (devProp & eSDPEventDrivenMode) GetUInt_proc(eSDPEventDrivenMode);

			//////// ... ////////

			_RELEASE(pPropertyStore);
			_RELEASE(pDevice);

			return 0;
		}

		//>> [SUBFUNC] Возвращает <true> и обновляет свойство <eSoundDevProp> в случае успеха
		bool ReadDeviceProprety(const wchar_t * ID, uint32 devProp)
		{
			if (devProp == eSDP_SKIPPED) return true;

			uint32 N = MISSING;
			eSoundDevType type;

			if (ReadDeviceNumber(ID, N, type))
				return ReadDeviceProprety(N, type, devProp);

			wchar_t error[256];
			swprintf_s(error, L"Fail to read property for UNK AUDIO DEV:\n%s", ID);
			_MBM(error);
			return false;
		}

		//>> [SUBFUNC] Возвращает <true> и свойство <eSoundDevProp> из его сырого числового PID в случае успеха
		bool ConvertProprety(PROPERTYKEY & PID_in, eSoundDevProp & eSDP_out)
		{
			bool ret = false;

			#define _CHECKPROP(p, e) if (_BOOL(ret=_BOOL(PID_in==p))) eSDP_out=e; else

			_CHECKPROP(PKEY_Device_FriendlyName,                     eSDPFriendlyName)
			_CHECKPROP(PKEY_DeviceInterface_FriendlyName,            eSDPFriendlyNameI)
			_CHECKPROP(PKEY_Device_DeviceDesc,                       eSDPInfoDescription)
			_CHECKPROP(PKEY_AudioEndpoint_Association,               eSDPEndAssociation)
			_CHECKPROP(PKEY_AudioEndpoint_ControlPanelPageProvider,  eSDPPageProvider)
			_CHECKPROP(PKEY_AudioEndpoint_GUID,                      eSDPguid)
			_CHECKPROP(PKEY_AudioEndpoint_JackSubType,               eSDPJackSubType)
			_CHECKPROP(PKEY_AudioEngine_DeviceFormat,                eSDPShareModeFormat)
			_CHECKPROP(PKEY_AudioEngine_OEMFormat,                   eSDPDefaultFormat)
			_CHECKPROP(PKEY_AudioEndpoint_Disable_SysFx,             eSDPSysFXState)
			_CHECKPROP(PKEY_AudioEndpoint_FormFactor,                eSDPFormFactor)
			_CHECKPROP(PKEY_AudioEndpoint_FullRangeSpeakers,         eSDPRangeFull)
			_CHECKPROP(PKEY_AudioEndpoint_PhysicalSpeakers,          eSDPRangePhys)
			_CHECKPROP(PKEY_AudioEndpoint_Supports_EventDriven_Mode, eSDPEventDrivenMode)
			_CHECKPROP(PKEY_Device_EnumeratorName,                   eSDPEnumeratorName)
			_CHECKPROP(PKEY_DeviceClass_IconPath,                    eSDPIconPath)
			{
				#define _CHECKPROP_SKIP(p) _CHECKPROP(p, eSDP_SKIPPED)

				// skipped debug props

				_CHECKPROP_SKIP(PKEY_Device_BaseContainerId)
				_CHECKPROP_SKIP(PKEY_Device_ContainerId)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_BlobMain_1)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_DeviceDesc)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_FuncSubSys)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_FuncSubSys_2)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_IntA_1)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_IntA_2)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_IntA_3)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_BlobMain_2)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_BlobMain_3)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_Manufacturer)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_BlobSub)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_ExtraGUID)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_IntB_1)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_IntB_2)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_IntB_3)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_FuncCodec_1)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_FuncCodec_2)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_FuncCodec_3)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_IntC_1)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_IntC_2)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_ExtraGUIDx_1)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_ExtraGUIDx_2)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_ExtraGUIDx_3)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_IntD_1)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_IntD_2)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_Reserved_1)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_Reserved_2)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_BlobS_1)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_BlobS_2)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_IntE_1)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_Version)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_BlobEx)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_PropBool_1)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_PropBool_2)
				_CHECKPROP_SKIP(PKEY_Device_EXTRA_IntF_1)
				;
				#undef _CHECKPROP_SKIP
			};

			#undef _CHECKPROP

			return ret;
		}

		//>> [SUBFUNC] Возвращает <true> и тип состояния устройства в случае успеха
		bool ConvertState(uint32 DEVICE_STATE, eSoundDevState & state)
		{
			switch (DEVICE_STATE)
			{
			case DEVICE_STATE_ACTIVE:     state = eSoundDevActive;     return true;
			case DEVICE_STATE_DISABLED:   state = eSoundDevDisabled;   return true;
			case DEVICE_STATE_NOTPRESENT: state = eSoundDevNotPresent; return true;
			case DEVICE_STATE_UNPLUGGED:  state = eSoundDevUnlugged;   return true;
			//case DEVICE_STATEMASK_ALL:
			//default:
			//	state = eSoundDevUnknown; // Не ставить, чтобы не переписало <eSoundDevRemoved>
			}

			return false;
		}

	private:
		//>> [DEBUG] Сообщает в консоль <eSoundDevProp> свойства устройства (локальные копии)
		void PrintfDeviceProprety(int32 N, eSoundDevType type, bool show_title, uint32 devProp)
		{
			vector<SoundDeviceProp> * SDProp = nullptr;

			switch (type)
			{
			case eSDTypeRender:  SDProp = &propRenderDevice;  break;
			case eSDTypeCapture: SDProp = &propCaptureDevice; break;
			default:
				_MBM(L"Incorrect eSoundDevType [ReadDeviceProprety]");
				return;
			}

			if ((((uint32)N) > numRenderDevices  && type == eSDTypeRender) ||
				(((uint32)N) > numCaptureDevices && type == eSDTypeCapture))
				{ _MBM(ERROR_IDLIMIT); return; }

			auto & DEV = (*SDProp)[N];

			///////////////

			if (show_title)
			{
				wprintf(L"\n\n___Info for device # %2i :", N);
				switch (type)
				{
				case eSDTypeRender:  wprintf(L" (RENDER DEV)");  break;
				case eSDTypeCapture: wprintf(L" (CAPTURE DEV)"); break;
				}			
				if ((N == numRenderDefault) && (type == eSDTypeRender) ||
					(N == numCaptureDefault) && (type == eSDTypeCapture))
					wprintf(L" (DEFAULT)");
			}

			///////////////

			if (devProp == eSDP_SKIPPED) wprintf(L"\n<skipped_property>");

			if (devProp & eSDPID) wprintf(L"\nID: %s", DEV.ID.c_str());
			if (devProp & eSDPState)
			{
				wprintf(L"\nState: ");
				switch (DEV.state)
				{
				case eSoundDevActive:     wprintf(L"active");      break;
				case eSoundDevDisabled:   wprintf(L"disabled");    break;
				case eSoundDevNotPresent: wprintf(L"not present"); break;
				case eSoundDevUnlugged:   wprintf(L"unplugged");   break;
				case eSoundDevRemoved:    wprintf(L"REMOVED FROM SYSTEM"); break;
				default: wprintf(L"< unknown condition >");
				}
			}
			if (devProp & eSDPFriendlyName)    wprintf(L"\nName:  %s", DEV.name.c_str());
			if (devProp & eSDPFriendlyNameI)   wprintf(L"\nNameI: %s", DEV.iname.c_str());
			if (devProp & eSDPInfoDescription) wprintf(L"\nInfo:  %s", DEV.info.c_str());
			if (devProp & eSDPEndAssociation)  wprintf(L"\nAssoc: %s", DEV.assoc.c_str());
			if (devProp & eSDPPageProvider)    wprintf(L"\nPageProv: %s", DEV.pageProv.c_str());
			if (devProp & eSDPguid)            wprintf(L"\nGUID: %s", DEV.GUID.c_str());
			if (devProp & eSDPJackSubType)     wprintf(L"\nJackSubType: %s", DEV.jackSType.c_str());

			if (devProp & (eSDPShareModeFormat | eSDPDefaultFormat))
			{
				wprintf(L"\n       STREAM : SHARED / DEFAULT");
				wprintf(L"\nSamplesPerSec:  %6i / %6i", DEV.modeShared.nSamplesPerSec,
														DEV.modeDefault.nSamplesPerSec);
				wprintf(L"\nBitsPerSample:  %6i / %6i", DEV.modeShared.wBitsPerSample,
														DEV.modeDefault.wBitsPerSample);
				wprintf(L"\nChannels:       %6i / %6i", DEV.modeShared.nChannels,
													    DEV.modeDefault.nChannels);
				wprintf(L"\nAvgBytesPerSec: %6i / %6i", DEV.modeShared.nAvgBytesPerSec,
													    DEV.modeDefault.nAvgBytesPerSec);
				wprintf(L"\nBlockAlign:     %6i / %6i", DEV.modeShared.nBlockAlign,
													    DEV.modeDefault.nBlockAlign);
				if (DEV.modeShared.wFormatTag != WAVE_FORMAT_UNKNOWN)
				wprintf(L"\nFormatTag [SHARED]:  %s", WaveFormatToString(DEV.modeShared.wFormatTag).c_str());
				if (DEV.modeDefault.wFormatTag != WAVE_FORMAT_UNKNOWN)
				wprintf(L"\nFormatTag [DEFAULT]: %s", WaveFormatToString(DEV.modeDefault.wFormatTag).c_str());
			}

			if (devProp & eSDPFormFactor)
			{
				wprintf(L"\nForm: ");
				switch (DEV.formFactor)
				{
				case eSDFormRemoteNetwork: wprintf(L"Remote network"); break;
				case eSDFormSpeakers:      wprintf(L"Speakers");       break;
				case eSDFormLineLevel:     wprintf(L"Line level");     break;
				case eSDFormHeadphones:    wprintf(L"Headphones");     break;
				case eSDFormMicrophone:    wprintf(L"Microphone");     break;
				case eSDFormHeadset:       wprintf(L"Headset");        break;
				case eSDFormHandset:       wprintf(L"Handset");        break;
				case eSDFormSPDIF:         wprintf(L"Sony/Philips Digital Interface"); break;
				case eSDFormUnknownDigitalPassthrough: wprintf(L"Unknown digital passthrough"); break;
				case eSDFormDigitalAudioDisplayDevice: wprintf(L"Digital audio display device"); break;
				default: wprintf(L"< unknown condition >");
				}
			}

			if (devProp & (eSDPRangeFull | eSDPRangePhys))
			{			
				auto PrintfRange_proc = [] (eSoundDevRange range) -> void
				{
					switch (range)
					{
					case eSDRange_MONO:         wprintf(L"mono"); break;
					case eSDRange_STEREO:       wprintf(L"stereo"); break;
					case eSDRange_QUAD:         wprintf(L"quad"); break;
					case eSDRange_SURROUND:     wprintf(L"surround"); break;
					case eSDRange_5p1:          wprintf(L"5.1"); break;
					case eSDRange_7p1:          wprintf(L"7.1"); break;
					case eSDRange_5p1_SURROUND: wprintf(L"5.1 surround"); break;
					case eSDRange_7p1_SURROUND: wprintf(L"7.1 surround"); break;
					default: wprintf(L"< UNK >");
					}
				};
				
				wprintf(L"\nRange FULL/PHYS: ");
				PrintfRange_proc(DEV.rangeFull);
				wprintf(L" / ");
				PrintfRange_proc(DEV.rangePhys);
			}

			if (devProp & eSDPSysFXState)      printf("\nSysFX: %s", _BOOLYESNO(DEV.bSysFX));
			if (devProp & eSDPEventDrivenMode) printf("\nEvDrvMode: %s", _BOOLYESNO(DEV.bEvDrvMode));
		}

		//>> [DEBUG] Сообщает в консоль обзор по основным свойствам устройств (локальные копии)
		void PrintfDevicesPropreties(int32 N, eSoundDevType type)
		{
			auto PrintfDevicesPropreties_proc = [this, N]
				(eSoundDevType type, uint32 numDevices) -> void
			{
				uint32 start = 0;
				uint32 end = numDevices;

				if (_NOMISS(N))
				{
					if ((N >= 0) && ((uint32)N < numDevices)) {
						 start = N;
						 end = start + 1;					  }
					else end = 0;
				}

				for (uint32 i = start; i < end; i++)
				{
				//	PrintfDeviceProprety(i, type, 1, eSDP_ALL);

					PrintfDeviceProprety(i, type, 1, eSDP_NONE);
					PrintfDeviceProprety(i, type, 0, eSDPID);
					PrintfDeviceProprety(i, type, 0, eSDPFriendlyName);
					PrintfDeviceProprety(i, type, 0, eSDPState | eSDPFormFactor | eSDP_ALL_Formats);

					wprintf(L"\n");
				}
			};

			wprintf(L"\n___DEVICES PROPS AND CONDITIONS__");
			for (int step=0; step<2; step++)
			{
				switch (step)
				{
				case 0:
					if (type != eSDTypeCapture)												{
						wprintf(L"\n/////// Render devices: ///////");
						PrintfDevicesPropreties_proc(eSDTypeRender, numRenderDevices);		}
					break;
				case 1:
					if (type != eSDTypeRender)												{
						wprintf(L"\n/////// Capture devices: ///////");
						PrintfDevicesPropreties_proc(eSDTypeCapture, numCaptureDevices);	}
					break;
				}
			}
		}

		//>> [DEBUG] Переводит сырой числовой ID свойства в его текстовое название
		void ConvertPropertyToString(PROPERTYKEY & PID_in, wstring & WSTR_out)
		{
			WSTR_out = L"";

			#define _ENDGOOUT ; if (WSTR_out.compare(L"")) return ;

			#define _PKEYTOSTR(key)	if (_BOOL(PID_in == key)) WSTR_out = L"" \
																		TOWSTRING(key); else

			_PKEYTOSTR(PKEY_Device_FriendlyName)
			_PKEYTOSTR(PKEY_DeviceInterface_FriendlyName)
			_PKEYTOSTR(PKEY_Device_DeviceDesc)
			_PKEYTOSTR(PKEY_AudioEndpoint_Association)
			_PKEYTOSTR(PKEY_AudioEndpoint_ControlPanelPageProvider)
			_PKEYTOSTR(PKEY_AudioEndpoint_GUID)
			_PKEYTOSTR(PKEY_AudioEndpoint_JackSubType)
			_PKEYTOSTR(PKEY_AudioEngine_DeviceFormat)
			_PKEYTOSTR(PKEY_AudioEngine_OEMFormat)
			_PKEYTOSTR(PKEY_AudioEndpoint_Disable_SysFx)
			_PKEYTOSTR(PKEY_AudioEndpoint_FormFactor)
			_PKEYTOSTR(PKEY_AudioEndpoint_FullRangeSpeakers)
			_PKEYTOSTR(PKEY_AudioEndpoint_PhysicalSpeakers)
			_PKEYTOSTR(PKEY_AudioEndpoint_Supports_EventDriven_Mode)
			_PKEYTOSTR(PKEY_Device_EnumeratorName)
			_PKEYTOSTR(PKEY_DeviceClass_IconPath)

			_PKEYTOSTR(PKEY_Device_BaseContainerId)
			_PKEYTOSTR(PKEY_Device_ContainerId)
			_PKEYTOSTR(PKEY_Device_EXTRA_BlobMain_1)	
			_PKEYTOSTR(PKEY_Device_EXTRA_DeviceDesc)
			_PKEYTOSTR(PKEY_Device_EXTRA_FuncSubSys)
			_PKEYTOSTR(PKEY_Device_EXTRA_FuncSubSys_2)
			_PKEYTOSTR(PKEY_Device_EXTRA_IntA_1)
			_PKEYTOSTR(PKEY_Device_EXTRA_IntA_2)
			_PKEYTOSTR(PKEY_Device_EXTRA_IntA_3)
			_PKEYTOSTR(PKEY_Device_EXTRA_BlobMain_2)
			_PKEYTOSTR(PKEY_Device_EXTRA_BlobMain_3)
			_PKEYTOSTR(PKEY_Device_EXTRA_Manufacturer)
			_PKEYTOSTR(PKEY_Device_EXTRA_BlobSub)
			_PKEYTOSTR(PKEY_Device_EXTRA_ExtraGUID)
			_PKEYTOSTR(PKEY_Device_EXTRA_IntB_1)
			_PKEYTOSTR(PKEY_Device_EXTRA_IntB_2)
			_PKEYTOSTR(PKEY_Device_EXTRA_IntB_3)
			_PKEYTOSTR(PKEY_Device_EXTRA_FuncCodec_1)
			_PKEYTOSTR(PKEY_Device_EXTRA_FuncCodec_2)
			_PKEYTOSTR(PKEY_Device_EXTRA_FuncCodec_3)
			_PKEYTOSTR(PKEY_Device_EXTRA_IntC_1)
			_PKEYTOSTR(PKEY_Device_EXTRA_IntC_2)
			_PKEYTOSTR(PKEY_Device_EXTRA_ExtraGUIDx_1)
			_PKEYTOSTR(PKEY_Device_EXTRA_ExtraGUIDx_2)
			_PKEYTOSTR(PKEY_Device_EXTRA_ExtraGUIDx_3)
			_PKEYTOSTR(PKEY_Device_EXTRA_IntD_1)
			_PKEYTOSTR(PKEY_Device_EXTRA_IntD_2)
			_PKEYTOSTR(PKEY_Device_EXTRA_Reserved_1)
			_PKEYTOSTR(PKEY_Device_EXTRA_Reserved_2)
			_PKEYTOSTR(PKEY_Device_EXTRA_BlobS_1)
			_PKEYTOSTR(PKEY_Device_EXTRA_BlobS_2)
			_PKEYTOSTR(PKEY_Device_EXTRA_IntE_1)
			_PKEYTOSTR(PKEY_Device_EXTRA_Version)
			_PKEYTOSTR(PKEY_Device_EXTRA_BlobEx)
			_PKEYTOSTR(PKEY_Device_EXTRA_PropBool_1)
			_PKEYTOSTR(PKEY_Device_EXTRA_PropBool_2)
			_PKEYTOSTR(PKEY_Device_EXTRA_IntF_1)

			_ENDGOOUT

			#undef _PKEYTOSTR

			#define _PKEYTOSTR(key)	if (_BOOL(PID_in == key)) WSTR_out = L"$_NEW_$ : " \
																				TOWSTRING(key); else

#pragma region <Audioenginebaseapo.h>

			_PKEYTOSTR(PKEY_FX_Association)
			_PKEYTOSTR(PKEY_FX_PreMixEffectClsid)
			_PKEYTOSTR(PKEY_FX_PostMixEffectClsid)
			_PKEYTOSTR(PKEY_FX_UserInterfaceClsid)
			_PKEYTOSTR(PKEY_FX_FriendlyName)
			_PKEYTOSTR(PKEY_FX_StreamEffectClsid)
			_PKEYTOSTR(PKEY_FX_ModeEffectClsid)
			_PKEYTOSTR(PKEY_FX_EndpointEffectClsid)
			_PKEYTOSTR(PKEY_SFX_ProcessingModes_Supported_For_Streaming)
			_PKEYTOSTR(PKEY_MFX_ProcessingModes_Supported_For_Streaming)
			_PKEYTOSTR(PKEY_EFX_ProcessingModes_Supported_For_Streaming)

			_ENDGOOUT

			// <Audioenginebaseapo.h>
#pragma endregion

#pragma region <Functiondiscoverykeys_devpkey.h>

			_PKEYTOSTR(PKEY_NAME)
			_PKEYTOSTR(PKEY_Device_HardwareIds)
			_PKEYTOSTR(PKEY_Device_CompatibleIds)
			_PKEYTOSTR(PKEY_Device_Service)
			_PKEYTOSTR(PKEY_Device_Class)
			_PKEYTOSTR(PKEY_Device_ClassGuid)
			_PKEYTOSTR(PKEY_Device_Driver)
			_PKEYTOSTR(PKEY_Device_ConfigFlags)
			_PKEYTOSTR(PKEY_Device_Manufacturer)
			_PKEYTOSTR(PKEY_Device_LocationInfo)
			_PKEYTOSTR(PKEY_Device_PDOName)
			_PKEYTOSTR(PKEY_Device_Capabilities)
			_PKEYTOSTR(PKEY_Device_UINumber)
			_PKEYTOSTR(PKEY_Device_UpperFilters)
			_PKEYTOSTR(PKEY_Device_LowerFilters)
			_PKEYTOSTR(PKEY_Device_BusTypeGuid)
			_PKEYTOSTR(PKEY_Device_LegacyBusType)
			_PKEYTOSTR(PKEY_Device_BusNumber)
			_PKEYTOSTR(PKEY_Device_Security)
			_PKEYTOSTR(PKEY_Device_SecuritySDS)
			_PKEYTOSTR(PKEY_Device_DevType)
			_PKEYTOSTR(PKEY_Device_Exclusive)
			_PKEYTOSTR(PKEY_Device_Characteristics)
			_PKEYTOSTR(PKEY_Device_Address)
			_PKEYTOSTR(PKEY_Device_UINumberDescFormat)
			_PKEYTOSTR(PKEY_Device_PowerData)
			_PKEYTOSTR(PKEY_Device_RemovalPolicy)
			_PKEYTOSTR(PKEY_Device_RemovalPolicyDefault)
			_PKEYTOSTR(PKEY_Device_RemovalPolicyOverride)
			_PKEYTOSTR(PKEY_Device_InstallState)
			_PKEYTOSTR(PKEY_Device_LocationPaths)

			_PKEYTOSTR(PKEY_Device_DevNodeStatus)
			_PKEYTOSTR(PKEY_Device_ProblemCode)

			_PKEYTOSTR(PKEY_Device_EjectionRelations)
			_PKEYTOSTR(PKEY_Device_RemovalRelations)
			_PKEYTOSTR(PKEY_Device_PowerRelations)
			_PKEYTOSTR(PKEY_Device_BusRelations)
			_PKEYTOSTR(PKEY_Device_Parent)
			_PKEYTOSTR(PKEY_Device_Children)
			_PKEYTOSTR(PKEY_Device_Siblings)
			_PKEYTOSTR(PKEY_Device_TransportRelations)

			_PKEYTOSTR(PKEY_Device_Reported)
			_PKEYTOSTR(PKEY_Device_Legacy)
			_PKEYTOSTR(PKEY_Device_InstanceId)
			_PKEYTOSTR(PKEY_Device_ModelId)
			_PKEYTOSTR(PKEY_Device_FriendlyNameAttributes)
			_PKEYTOSTR(PKEY_Device_ManufacturerAttributes)
			_PKEYTOSTR(PKEY_Device_PresenceNotForDevice)
			_PKEYTOSTR(PKEY_Device_SignalStrength)
			_PKEYTOSTR(PKEY_Device_IsAssociateableByUserAction)
			_PKEYTOSTR(PKEY_Numa_Proximity_Domain)
			_PKEYTOSTR(PKEY_Device_DHP_Rebalance_Policy)
			_PKEYTOSTR(PKEY_Device_Numa_Node)
			_PKEYTOSTR(PKEY_Device_BusReportedDeviceDesc)
			_PKEYTOSTR(PKEY_Device_InstallInProgress)

			_PKEYTOSTR(PKEY_Device_DriverDate)
			_PKEYTOSTR(PKEY_Device_DriverVersion)
			_PKEYTOSTR(PKEY_Device_DriverDesc)
			_PKEYTOSTR(PKEY_Device_DriverInfPath)
			_PKEYTOSTR(PKEY_Device_DriverInfSection)
			_PKEYTOSTR(PKEY_Device_DriverInfSectionExt)
			_PKEYTOSTR(PKEY_Device_MatchingDeviceId)
			_PKEYTOSTR(PKEY_Device_DriverProvider)
			_PKEYTOSTR(PKEY_Device_DriverPropPageProvider)
			_PKEYTOSTR(PKEY_Device_DriverCoInstallers)
			_PKEYTOSTR(PKEY_Device_ResourcePickerTags)
			_PKEYTOSTR(PKEY_Device_ResourcePickerExceptions)
			_PKEYTOSTR(PKEY_Device_DriverRank)
			_PKEYTOSTR(PKEY_Device_DriverLogoLevel)
			_PKEYTOSTR(PKEY_Device_NoConnectSound)
			_PKEYTOSTR(PKEY_Device_GenericDriverInstalled)
			_PKEYTOSTR(PKEY_Device_AdditionalSoftwareRequested)

			_PKEYTOSTR(PKEY_Device_SafeRemovalRequired)
			_PKEYTOSTR(PKEY_Device_SafeRemovalRequiredOverride)

			_PKEYTOSTR(PKEY_DrvPkg_Model)
			_PKEYTOSTR(PKEY_DrvPkg_VendorWebSite)
			_PKEYTOSTR(PKEY_DrvPkg_DetailedDescription)
			_PKEYTOSTR(PKEY_DrvPkg_DocumentationLink)
			_PKEYTOSTR(PKEY_DrvPkg_Icon)
			_PKEYTOSTR(PKEY_DrvPkg_BrandingIcon)

			_PKEYTOSTR(PKEY_DeviceClass_UpperFilters)
			_PKEYTOSTR(PKEY_DeviceClass_LowerFilters)
			_PKEYTOSTR(PKEY_DeviceClass_Security)
			_PKEYTOSTR(PKEY_DeviceClass_SecuritySDS)
			_PKEYTOSTR(PKEY_DeviceClass_DevType)
			_PKEYTOSTR(PKEY_DeviceClass_Exclusive)
			_PKEYTOSTR(PKEY_DeviceClass_Characteristics)

			_PKEYTOSTR(PKEY_DeviceClass_Name)
			_PKEYTOSTR(PKEY_DeviceClass_ClassName)
			_PKEYTOSTR(PKEY_DeviceClass_Icon)
			_PKEYTOSTR(PKEY_DeviceClass_ClassInstaller)
			_PKEYTOSTR(PKEY_DeviceClass_PropPageProvider)
			_PKEYTOSTR(PKEY_DeviceClass_NoInstallClass)
			_PKEYTOSTR(PKEY_DeviceClass_NoDisplayClass)
			_PKEYTOSTR(PKEY_DeviceClass_SilentInstall)
			_PKEYTOSTR(PKEY_DeviceClass_NoUseClass)
			_PKEYTOSTR(PKEY_DeviceClass_DefaultService)

			_PKEYTOSTR(PKEY_DeviceClass_ClassCoInstallers)

			_PKEYTOSTR(PKEY_DeviceInterface_Enabled)
			_PKEYTOSTR(PKEY_DeviceInterface_ClassGuid)
			_PKEYTOSTR(PKEY_DeviceInterfaceClass_DefaultInterface)

			_ENDGOOUT

			// <Functiondiscoverykeys_devpkey.h>
#pragma endregion 

#pragma region <devpkey.h>

			_PKEYTOSTR(PKEY_Device_ProblemStatus)
			_PKEYTOSTR(PKEY_Device_InLocalMachineContainer)
			_PKEYTOSTR(PKEY_Device_ShowInUninstallUI)

			_PKEYTOSTR(PKEY_Device_IsPresent)
			_PKEYTOSTR(PKEY_Device_HasProblem)
			_PKEYTOSTR(PKEY_Device_ConfigurationId)
			_PKEYTOSTR(PKEY_Device_ReportedDeviceIdsHash)
			_PKEYTOSTR(PKEY_Device_PhysicalDeviceLocation)
			_PKEYTOSTR(PKEY_Device_BiosDeviceName)
			_PKEYTOSTR(PKEY_Device_DriverProblemDesc)
			_PKEYTOSTR(PKEY_Device_DebuggerSafe)
			_PKEYTOSTR(PKEY_Device_PostInstallInProgress)
			_PKEYTOSTR(PKEY_Device_Stack)
			_PKEYTOSTR(PKEY_Device_ExtendedConfigurationIds)
			_PKEYTOSTR(PKEY_Device_IsRebootRequired)
			_PKEYTOSTR(PKEY_Device_FirmwareDate)
			_PKEYTOSTR(PKEY_Device_FirmwareVersion)
			_PKEYTOSTR(PKEY_Device_FirmwareRevision)

			_PKEYTOSTR(PKEY_Device_SessionId)
			_PKEYTOSTR(PKEY_Device_InstallDate)
			_PKEYTOSTR(PKEY_Device_FirstInstallDate)
			_PKEYTOSTR(PKEY_Device_LastArrivalDate)
			_PKEYTOSTR(PKEY_Device_LastRemovalDate)

			_PKEYTOSTR(PKEY_DeviceClass_DHPRebalanceOptOut)

			_PKEYTOSTR(PKEY_DeviceInterface_ReferenceString)
			_PKEYTOSTR(PKEY_DeviceInterface_Restricted)

			_PKEYTOSTR(PKEY_DeviceInterfaceClass_Name)

			_PKEYTOSTR(PKEY_DeviceContainer_Address)
			_PKEYTOSTR(PKEY_DeviceContainer_DiscoveryMethod)
			_PKEYTOSTR(PKEY_DeviceContainer_IsEncrypted)
			_PKEYTOSTR(PKEY_DeviceContainer_IsAuthenticated)
			_PKEYTOSTR(PKEY_DeviceContainer_IsConnected)
			_PKEYTOSTR(PKEY_DeviceContainer_IsPaired)
			_PKEYTOSTR(PKEY_DeviceContainer_Icon)
			_PKEYTOSTR(PKEY_DeviceContainer_Version)
			_PKEYTOSTR(PKEY_DeviceContainer_Last_Seen)
			_PKEYTOSTR(PKEY_DeviceContainer_Last_Connected)
			_PKEYTOSTR(PKEY_DeviceContainer_IsShowInDisconnectedState)
			_PKEYTOSTR(PKEY_DeviceContainer_IsLocalMachine)
			_PKEYTOSTR(PKEY_DeviceContainer_MetadataPath)
			_PKEYTOSTR(PKEY_DeviceContainer_IsMetadataSearchInProgress)
			_PKEYTOSTR(PKEY_DeviceContainer_MetadataChecksum)
			_PKEYTOSTR(PKEY_DeviceContainer_IsNotInterestingForDisplay)
			_PKEYTOSTR(PKEY_DeviceContainer_LaunchDeviceStageOnDeviceConnect)
			_PKEYTOSTR(PKEY_DeviceContainer_LaunchDeviceStageFromExplorer)
			_PKEYTOSTR(PKEY_DeviceContainer_BaselineExperienceId)
			_PKEYTOSTR(PKEY_DeviceContainer_IsDeviceUniquelyIdentifiable)
			_PKEYTOSTR(PKEY_DeviceContainer_AssociationArray)
			_PKEYTOSTR(PKEY_DeviceContainer_DeviceDescription1)
			_PKEYTOSTR(PKEY_DeviceContainer_DeviceDescription2)
			_PKEYTOSTR(PKEY_DeviceContainer_HasProblem)
			_PKEYTOSTR(PKEY_DeviceContainer_IsSharedDevice)
			_PKEYTOSTR(PKEY_DeviceContainer_IsNetworkDevice)
			_PKEYTOSTR(PKEY_DeviceContainer_IsDefaultDevice)
			_PKEYTOSTR(PKEY_DeviceContainer_MetadataCabinet)
			_PKEYTOSTR(PKEY_DeviceContainer_RequiresPairingElevation)
			_PKEYTOSTR(PKEY_DeviceContainer_ExperienceId)
			_PKEYTOSTR(PKEY_DeviceContainer_Category)
			_PKEYTOSTR(PKEY_DeviceContainer_Category_Desc_Singular)
			_PKEYTOSTR(PKEY_DeviceContainer_Category_Desc_Plural)
			_PKEYTOSTR(PKEY_DeviceContainer_Category_Icon)
			_PKEYTOSTR(PKEY_DeviceContainer_CategoryGroup_Desc)
			_PKEYTOSTR(PKEY_DeviceContainer_CategoryGroup_Icon)
			_PKEYTOSTR(PKEY_DeviceContainer_PrimaryCategory)
			_PKEYTOSTR(PKEY_DeviceContainer_UnpairUninstall)
			_PKEYTOSTR(PKEY_DeviceContainer_RequiresUninstallElevation)
			_PKEYTOSTR(PKEY_DeviceContainer_DeviceFunctionSubRank)
			_PKEYTOSTR(PKEY_DeviceContainer_AlwaysShowDeviceAsConnected)
			_PKEYTOSTR(PKEY_DeviceContainer_ConfigFlags)
			_PKEYTOSTR(PKEY_DeviceContainer_PrivilegedPackageFamilyNames)
			_PKEYTOSTR(PKEY_DeviceContainer_CustomPrivilegedPackageFamilyNames)
			_PKEYTOSTR(PKEY_DeviceContainer_IsRebootRequired)

			_PKEYTOSTR(PKEY_DeviceContainer_FriendlyName)
			_PKEYTOSTR(PKEY_DeviceContainer_Manufacturer)
			_PKEYTOSTR(PKEY_DeviceContainer_ModelName)
			_PKEYTOSTR(PKEY_DeviceContainer_ModelNumber)

			_PKEYTOSTR(PKEY_DeviceContainer_InstallInProgress)

			_ENDGOOUT

			// <devpkey.h>
#pragma endregion 

#pragma region <Functiondiscoverykeys.h>

			_PKEYTOSTR(PKEY_FunctionInstance)
			//_PKEYTOSTR(FMTID_FD)
			_PKEYTOSTR(PKEY_FD_Visibility)
			//_PKEYTOSTR(FMTID_Device)

			_PKEYTOSTR(PKEY_Device_NotPresent)
			_PKEYTOSTR(PKEY_Device_QueueSize)
			_PKEYTOSTR(PKEY_Device_Status)
			_PKEYTOSTR(PKEY_Device_Comment)
			_PKEYTOSTR(PKEY_Device_Model)

			//_PKEYTOSTR(FMTID_DeviceInterface)

			_PKEYTOSTR(PKEY_DeviceInterface_DevicePath)

			_PKEYTOSTR(PKEY_DeviceDisplay_Address)
			_PKEYTOSTR(PKEY_DeviceDisplay_DiscoveryMethod)
			_PKEYTOSTR(PKEY_DeviceDisplay_IsEncrypted)
			_PKEYTOSTR(PKEY_DeviceDisplay_IsAuthenticated)
			_PKEYTOSTR(PKEY_DeviceDisplay_IsConnected)
			_PKEYTOSTR(PKEY_DeviceDisplay_IsPaired)
			_PKEYTOSTR(PKEY_DeviceDisplay_Icon)
			_PKEYTOSTR(PKEY_DeviceDisplay_Version)
			_PKEYTOSTR(PKEY_DeviceDisplay_Last_Seen)
			_PKEYTOSTR(PKEY_DeviceDisplay_Last_Connected)
			_PKEYTOSTR(PKEY_DeviceDisplay_IsShowInDisconnectedState)
			_PKEYTOSTR(PKEY_DeviceDisplay_IsLocalMachine)
			_PKEYTOSTR(PKEY_DeviceDisplay_MetadataPath)
			_PKEYTOSTR(PKEY_DeviceDisplay_IsMetadataSearchInProgress)
			_PKEYTOSTR(PKEY_DeviceDisplay_MetadataChecksum)
			_PKEYTOSTR(PKEY_DeviceDisplay_IsNotInterestingForDisplay)
			_PKEYTOSTR(PKEY_DeviceDisplay_LaunchDeviceStageOnDeviceConnect)
			_PKEYTOSTR(PKEY_DeviceDisplay_LaunchDeviceStageFromExplorer)
			_PKEYTOSTR(PKEY_DeviceDisplay_BaselineExperienceId)
			_PKEYTOSTR(PKEY_DeviceDisplay_IsDeviceUniquelyIdentifiable)
			_PKEYTOSTR(PKEY_DeviceDisplay_AssociationArray)
			_PKEYTOSTR(PKEY_DeviceDisplay_DeviceDescription1)
			_PKEYTOSTR(PKEY_DeviceDisplay_DeviceDescription2)
			_PKEYTOSTR(PKEY_DeviceDisplay_IsNotWorkingProperly)
			_PKEYTOSTR(PKEY_DeviceDisplay_IsSharedDevice)
			_PKEYTOSTR(PKEY_DeviceDisplay_IsNetworkDevice)
			_PKEYTOSTR(PKEY_DeviceDisplay_IsDefaultDevice)
			_PKEYTOSTR(PKEY_DeviceDisplay_MetadataCabinet)
			_PKEYTOSTR(PKEY_DeviceDisplay_RequiresPairingElevation)
			_PKEYTOSTR(PKEY_DeviceDisplay_ExperienceId)
			_PKEYTOSTR(PKEY_DeviceDisplay_Category)
			_PKEYTOSTR(PKEY_DeviceDisplay_Category_Desc_Singular)
			_PKEYTOSTR(PKEY_DeviceDisplay_Category_Desc_Plural)
			_PKEYTOSTR(PKEY_DeviceDisplay_Category_Icon)
			_PKEYTOSTR(PKEY_DeviceDisplay_CategoryGroup_Desc)
			_PKEYTOSTR(PKEY_DeviceDisplay_CategoryGroup_Icon)
			_PKEYTOSTR(PKEY_DeviceDisplay_PrimaryCategory)
			_PKEYTOSTR(PKEY_DeviceDisplay_UnpairUninstall)
			_PKEYTOSTR(PKEY_DeviceDisplay_RequiresUninstallElevation)
			_PKEYTOSTR(PKEY_DeviceDisplay_DeviceFunctionSubRank)
			_PKEYTOSTR(PKEY_DeviceDisplay_AlwaysShowDeviceAsConnected)

			_PKEYTOSTR(PKEY_DeviceDisplay_FriendlyName)
			_PKEYTOSTR(PKEY_DeviceDisplay_Manufacturer)
			_PKEYTOSTR(PKEY_DeviceDisplay_ModelName)
			_PKEYTOSTR(PKEY_DeviceDisplay_ModelNumber)

			_PKEYTOSTR(PKEY_DeviceDisplay_InstallInProgress)

			//_PKEYTOSTR(FMTID_Pairing)
			_PKEYTOSTR(PKEY_Pairing_ListItemText)
			_PKEYTOSTR(PKEY_Pairing_ListItemDescription)
			_PKEYTOSTR(PKEY_Pairing_ListItemIcon)
			_PKEYTOSTR(PKEY_Pairing_ListItemDefault)
			_PKEYTOSTR(PKEY_Pairing_IsWifiOnlyDevice)

			_PKEYTOSTR(PKEY_Device_BIOSVersion)

			_PKEYTOSTR(PKEY_Write_Time)
			_PKEYTOSTR(PKEY_Create_Time)

			_PKEYTOSTR(PKEY_Device_InstanceId)
			_PKEYTOSTR(PKEY_Device_Interface)

			_PKEYTOSTR(PKEY_ExposedIIDs)
			_PKEYTOSTR(PKEY_ExposedCLSIDs)
			_PKEYTOSTR(PKEY_InstanceValidatorClsid)

			//_PKEYTOSTR(FMTID_WSD)

			_PKEYTOSTR(PKEY_WSD_AddressURI)
			_PKEYTOSTR(PKEY_WSD_Types)
			_PKEYTOSTR(PKEY_WSD_Scopes)
			_PKEYTOSTR(PKEY_WSD_MetadataVersion)
			_PKEYTOSTR(PKEY_WSD_AppSeqInstanceID)
			_PKEYTOSTR(PKEY_WSD_AppSeqSessionID)
			_PKEYTOSTR(PKEY_WSD_AppSeqMessageNumber)
			_PKEYTOSTR(PKEY_WSD_XAddrs)

			_PKEYTOSTR(PKEY_WSD_MetadataClean)
			_PKEYTOSTR(PKEY_WSD_ServiceInfo)

			_PKEYTOSTR(PKEY_PUBSVCS_TYPE)
			_PKEYTOSTR(PKEY_PUBSVCS_SCOPE)
			_PKEYTOSTR(PKEY_PUBSVCS_METADATA)
			_PKEYTOSTR(PKEY_PUBSVCS_METADATA_VERSION)
			_PKEYTOSTR(PKEY_PUBSVCS_NETWORK_PROFILES_ALLOWED)
			_PKEYTOSTR(PKEY_PUBSVCS_NETWORK_PROFILES_DENIED)
			_PKEYTOSTR(PKEY_PUBSVCS_NETWORK_PROFILES_DEFAULT)

			_ENDGOOUT

			//_PKEYTOSTR(FMTID_PNPX)

			_PKEYTOSTR(PKEY_PNPX_GlobalIdentity)
			_PKEYTOSTR(PKEY_PNPX_Types)
			_PKEYTOSTR(PKEY_PNPX_Scopes)
			_PKEYTOSTR(PKEY_PNPX_XAddrs)
			_PKEYTOSTR(PKEY_PNPX_MetadataVersion)
			_PKEYTOSTR(PKEY_PNPX_ID)

			_PKEYTOSTR(PKEY_PNPX_RemoteAddress)
			_PKEYTOSTR(PKEY_PNPX_RootProxy)

			_PKEYTOSTR(PKEY_PNPX_ManufacturerUrl)
			_PKEYTOSTR(PKEY_PNPX_ModelUrl)
			_PKEYTOSTR(PKEY_PNPX_Upc)
			_PKEYTOSTR(PKEY_PNPX_PresentationUrl)

			_PKEYTOSTR(PKEY_PNPX_FirmwareVersion)
			_PKEYTOSTR(PKEY_PNPX_SerialNumber)
			_PKEYTOSTR(PKEY_PNPX_DeviceCategory)

			_PKEYTOSTR(PKEY_PNPX_SecureChannel)
			_PKEYTOSTR(PKEY_PNPX_CompactSignature)
			_PKEYTOSTR(PKEY_PNPX_DeviceCertHash)

			_PKEYTOSTR(PKEY_PNPX_DeviceCategory_Desc)
			_PKEYTOSTR(PKEY_PNPX_Category_Desc_NonPlural)

			_PKEYTOSTR(PKEY_PNPX_PhysicalAddress)
			_PKEYTOSTR(PKEY_PNPX_NetworkInterfaceLuid)
			_PKEYTOSTR(PKEY_PNPX_NetworkInterfaceGuid)
			_PKEYTOSTR(PKEY_PNPX_IpAddress)

			_PKEYTOSTR(PKEY_PNPX_ServiceAddress)
			_PKEYTOSTR(PKEY_PNPX_ServiceId)
			_PKEYTOSTR(PKEY_PNPX_ServiceTypes)
			_PKEYTOSTR(PKEY_PNPX_ServiceControlUrl)
			_PKEYTOSTR(PKEY_PNPX_ServiceDescUrl)
			_PKEYTOSTR(PKEY_PNPX_ServiceEventSubUrl)

			_PKEYTOSTR(PKEY_PNPX_Devnode)
			_PKEYTOSTR(PKEY_PNPX_AssociationState)
			_PKEYTOSTR(PKEY_PNPX_AssociatedInstanceId)
			_PKEYTOSTR(PKEY_PNPX_LastNotificationTime)

			_PKEYTOSTR(PKEY_PNPX_DomainName)
			_PKEYTOSTR(PKEY_PNPX_ShareName)

			_PKEYTOSTR(PKEY_SSDP_AltLocationInfo)
			_PKEYTOSTR(PKEY_SSDP_DevLifeTime)
			_PKEYTOSTR(PKEY_SSDP_NetworkInterface)

			//_PKEYTOSTR(FMTID_PNPXDynamicProperty)

			_PKEYTOSTR(PKEY_PNPX_Installable)
			_PKEYTOSTR(PKEY_PNPX_Associated)

			_PKEYTOSTR(PKEY_PNPX_CompatibleTypes)
			_PKEYTOSTR(PKEY_PNPX_InstallState)

			_PKEYTOSTR(PKEY_PNPX_Removable)
			_PKEYTOSTR(PKEY_PNPX_IPBusEnumerated)

			_PKEYTOSTR(PKEY_WNET_Scope)
			_PKEYTOSTR(PKEY_WNET_Type)
			_PKEYTOSTR(PKEY_WNET_DisplayType)
			_PKEYTOSTR(PKEY_WNET_Usage)
			_PKEYTOSTR(PKEY_WNET_LocalName)
			_PKEYTOSTR(PKEY_WNET_RemoteName)
			_PKEYTOSTR(PKEY_WNET_Comment)
			_PKEYTOSTR(PKEY_WNET_Provider)

			_PKEYTOSTR(PKEY_WCN_Version)
			_PKEYTOSTR(PKEY_WCN_RequestType)
			_PKEYTOSTR(PKEY_WCN_AuthType)
			_PKEYTOSTR(PKEY_WCN_EncryptType)
			_PKEYTOSTR(PKEY_WCN_ConnType)
			_PKEYTOSTR(PKEY_WCN_ConfigMethods)

			_PKEYTOSTR(PKEY_WCN_RfBand)
			_PKEYTOSTR(PKEY_WCN_AssocState)
			_PKEYTOSTR(PKEY_WCN_ConfigError)
			_PKEYTOSTR(PKEY_WCN_ConfigState)
			_PKEYTOSTR(PKEY_WCN_DevicePasswordId)
			_PKEYTOSTR(PKEY_WCN_OSVersion)
			_PKEYTOSTR(PKEY_WCN_VendorExtension)
			_PKEYTOSTR(PKEY_WCN_RegistrarType)

			_PKEYTOSTR(PKEY_Hardware_Devinst)
			_PKEYTOSTR(PKEY_Hardware_DisplayAttribute)
			_PKEYTOSTR(PKEY_Hardware_DriverDate)
			_PKEYTOSTR(PKEY_Hardware_DriverProvider)
			_PKEYTOSTR(PKEY_Hardware_DriverVersion)
			_PKEYTOSTR(PKEY_Hardware_Function)
			_PKEYTOSTR(PKEY_Hardware_Icon)
			_PKEYTOSTR(PKEY_Hardware_Image)
			_PKEYTOSTR(PKEY_Hardware_Manufacturer)
			_PKEYTOSTR(PKEY_Hardware_Model)
			_PKEYTOSTR(PKEY_Hardware_Name)
			_PKEYTOSTR(PKEY_Hardware_SerialNumber)
			_PKEYTOSTR(PKEY_Hardware_ShellAttributes)
			_PKEYTOSTR(PKEY_Hardware_Status)

			_ENDGOOUT

			// <Functiondiscoverykeys.h>
#pragma endregion 

#pragma region <propkey.h>

			_PKEYTOSTR(PKEY_Audio_ChannelCount)
			_PKEYTOSTR(PKEY_Audio_Compression)
			_PKEYTOSTR(PKEY_Audio_EncodingBitrate)
			_PKEYTOSTR(PKEY_Audio_Format)
			_PKEYTOSTR(PKEY_Audio_IsVariableBitRate)
			_PKEYTOSTR(PKEY_Audio_PeakValue)
			_PKEYTOSTR(PKEY_Audio_SampleRate)
			_PKEYTOSTR(PKEY_Audio_SampleSize)
			_PKEYTOSTR(PKEY_Audio_StreamName)
			_PKEYTOSTR(PKEY_Audio_StreamNumber)

			_PKEYTOSTR(PKEY_Calendar_Duration)
			_PKEYTOSTR(PKEY_Calendar_IsOnline)
			_PKEYTOSTR(PKEY_Calendar_IsRecurring)
			_PKEYTOSTR(PKEY_Calendar_Location)
			_PKEYTOSTR(PKEY_Calendar_OptionalAttendeeAddresses)
			_PKEYTOSTR(PKEY_Calendar_OptionalAttendeeNames)
			_PKEYTOSTR(PKEY_Calendar_OrganizerAddress)
			_PKEYTOSTR(PKEY_Calendar_OrganizerName)
			_PKEYTOSTR(PKEY_Calendar_ReminderTime)
			_PKEYTOSTR(PKEY_Calendar_RequiredAttendeeAddresses)
			_PKEYTOSTR(PKEY_Calendar_RequiredAttendeeNames)
			_PKEYTOSTR(PKEY_Calendar_Resources)
			_PKEYTOSTR(PKEY_Calendar_ResponseStatus)
			_PKEYTOSTR(PKEY_Calendar_ShowTimeAs)
			_PKEYTOSTR(PKEY_Calendar_ShowTimeAsText)

			_PKEYTOSTR(PKEY_Communication_AccountName)
			_PKEYTOSTR(PKEY_Communication_DateItemExpires)
			_PKEYTOSTR(PKEY_Communication_FollowupIconIndex)
			_PKEYTOSTR(PKEY_Communication_HeaderItem)
			_PKEYTOSTR(PKEY_Communication_PolicyTag)
			_PKEYTOSTR(PKEY_Communication_SecurityFlags)
			_PKEYTOSTR(PKEY_Communication_Suffix)
			_PKEYTOSTR(PKEY_Communication_TaskStatus)
			_PKEYTOSTR(PKEY_Communication_TaskStatusText)

			_PKEYTOSTR(PKEY_Computer_DecoratedFreeSpace)

			_ENDGOOUT

			_PKEYTOSTR(PKEY_Contact_AccountPictureDynamicVideo)
			_PKEYTOSTR(PKEY_Contact_AccountPictureLarge)
			_PKEYTOSTR(PKEY_Contact_AccountPictureSmall)
			_PKEYTOSTR(PKEY_Contact_Anniversary)
			_PKEYTOSTR(PKEY_Contact_AssistantName)
			_PKEYTOSTR(PKEY_Contact_AssistantTelephone)
			_PKEYTOSTR(PKEY_Contact_Birthday)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress1Country)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress1Locality)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress1PostalCode)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress1Region)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress1Street)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress2Country)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress2Locality)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress2PostalCode)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress2Region)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress2Street)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress3Country)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress3Locality)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress3PostalCode)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress3Region)
			_PKEYTOSTR(PKEY_Contact_BusinessAddress3Street)
			_PKEYTOSTR(PKEY_Contact_BusinessAddressCity)
			_PKEYTOSTR(PKEY_Contact_BusinessAddressCountry)
			_PKEYTOSTR(PKEY_Contact_BusinessAddressPostalCode)
			_PKEYTOSTR(PKEY_Contact_BusinessAddressPostOfficeBox)
			_PKEYTOSTR(PKEY_Contact_BusinessAddressState)
			_PKEYTOSTR(PKEY_Contact_BusinessAddressStreet)
			_PKEYTOSTR(PKEY_Contact_BusinessEmailAddresses)
			_PKEYTOSTR(PKEY_Contact_BusinessFaxNumber)
			_PKEYTOSTR(PKEY_Contact_BusinessHomePage)
			_PKEYTOSTR(PKEY_Contact_BusinessTelephone)
			_PKEYTOSTR(PKEY_Contact_CallbackTelephone)
			_PKEYTOSTR(PKEY_Contact_CarTelephone)
			_PKEYTOSTR(PKEY_Contact_Children)
			_PKEYTOSTR(PKEY_Contact_CompanyMainTelephone)
			_PKEYTOSTR(PKEY_Contact_ConnectedServiceDisplayName)
			_PKEYTOSTR(PKEY_Contact_ConnectedServiceIdentities)
			_PKEYTOSTR(PKEY_Contact_ConnectedServiceName)
			_PKEYTOSTR(PKEY_Contact_ConnectedServiceSupportedActions)
			_PKEYTOSTR(PKEY_Contact_DataSuppliers)
			_PKEYTOSTR(PKEY_Contact_Department)
			_PKEYTOSTR(PKEY_Contact_DisplayBusinessPhoneNumbers)
			_PKEYTOSTR(PKEY_Contact_DisplayHomePhoneNumbers)
			_PKEYTOSTR(PKEY_Contact_DisplayMobilePhoneNumbers)
			_PKEYTOSTR(PKEY_Contact_DisplayOtherPhoneNumbers)
			_PKEYTOSTR(PKEY_Contact_EmailAddress)
			_PKEYTOSTR(PKEY_Contact_EmailAddress2)
			_PKEYTOSTR(PKEY_Contact_EmailAddress3)
			_PKEYTOSTR(PKEY_Contact_EmailAddresses)
			_PKEYTOSTR(PKEY_Contact_EmailName)
			_PKEYTOSTR(PKEY_Contact_FileAsName)
			_PKEYTOSTR(PKEY_Contact_FirstName)
			_PKEYTOSTR(PKEY_Contact_FullName)
			_PKEYTOSTR(PKEY_Contact_Gender)
			_PKEYTOSTR(PKEY_Contact_GenderValue)
			_PKEYTOSTR(PKEY_Contact_Hobbies)
			_PKEYTOSTR(PKEY_Contact_HomeAddress)
			_PKEYTOSTR(PKEY_Contact_HomeAddress1Country)
			_PKEYTOSTR(PKEY_Contact_HomeAddress1Locality)
			_PKEYTOSTR(PKEY_Contact_HomeAddress1PostalCode)
			_PKEYTOSTR(PKEY_Contact_HomeAddress1Region)
			_PKEYTOSTR(PKEY_Contact_HomeAddress1Street)
			_PKEYTOSTR(PKEY_Contact_HomeAddress2Country)
			_PKEYTOSTR(PKEY_Contact_HomeAddress2Locality)
			_PKEYTOSTR(PKEY_Contact_HomeAddress2PostalCode)
			_PKEYTOSTR(PKEY_Contact_HomeAddress2Region)
			_PKEYTOSTR(PKEY_Contact_HomeAddress2Street)
			_PKEYTOSTR(PKEY_Contact_HomeAddress3Country)
			_PKEYTOSTR(PKEY_Contact_HomeAddress3Locality)
			_PKEYTOSTR(PKEY_Contact_HomeAddress3PostalCode)
			_PKEYTOSTR(PKEY_Contact_HomeAddress3Region)
			_PKEYTOSTR(PKEY_Contact_HomeAddress3Street)
			_PKEYTOSTR(PKEY_Contact_HomeAddressCity)
			_PKEYTOSTR(PKEY_Contact_HomeAddressCountry)
			_PKEYTOSTR(PKEY_Contact_HomeAddressPostalCode)
			_PKEYTOSTR(PKEY_Contact_HomeAddressPostOfficeBox)
			_PKEYTOSTR(PKEY_Contact_HomeAddressState)
			_PKEYTOSTR(PKEY_Contact_HomeAddressStreet)
			_PKEYTOSTR(PKEY_Contact_HomeEmailAddresses)
			_PKEYTOSTR(PKEY_Contact_HomeFaxNumber)
			_PKEYTOSTR(PKEY_Contact_HomeTelephone)
			_PKEYTOSTR(PKEY_Contact_IMAddress)
			_PKEYTOSTR(PKEY_Contact_Initials)

			_ENDGOOUT

			_PKEYTOSTR(PKEY_Contact_JA_CompanyNamePhonetic)
			_PKEYTOSTR(PKEY_Contact_JA_FirstNamePhonetic)
			_PKEYTOSTR(PKEY_Contact_JA_LastNamePhonetic)
			_PKEYTOSTR(PKEY_Contact_JobInfo1CompanyAddress)
			_PKEYTOSTR(PKEY_Contact_JobInfo1CompanyName)
			_PKEYTOSTR(PKEY_Contact_JobInfo1Department)
			_PKEYTOSTR(PKEY_Contact_JobInfo1Manager)
			_PKEYTOSTR(PKEY_Contact_JobInfo1OfficeLocation)
			_PKEYTOSTR(PKEY_Contact_JobInfo1Title)
			_PKEYTOSTR(PKEY_Contact_JobInfo1YomiCompanyName)
			_PKEYTOSTR(PKEY_Contact_JobInfo2CompanyAddress)
			_PKEYTOSTR(PKEY_Contact_JobInfo2CompanyName)
			_PKEYTOSTR(PKEY_Contact_JobInfo2Department)
			_PKEYTOSTR(PKEY_Contact_JobInfo2Manager)
			_PKEYTOSTR(PKEY_Contact_JobInfo2OfficeLocation)
			_PKEYTOSTR(PKEY_Contact_JobInfo2Title)
			_PKEYTOSTR(PKEY_Contact_JobInfo2YomiCompanyName)
			_PKEYTOSTR(PKEY_Contact_JobInfo3CompanyAddress)
			_PKEYTOSTR(PKEY_Contact_JobInfo3CompanyName)
			_PKEYTOSTR(PKEY_Contact_JobInfo3Department)
			_PKEYTOSTR(PKEY_Contact_JobInfo3Manager)
			_PKEYTOSTR(PKEY_Contact_JobInfo3OfficeLocation)
			_PKEYTOSTR(PKEY_Contact_JobInfo3Title)
			_PKEYTOSTR(PKEY_Contact_JobInfo3YomiCompanyName)
			_PKEYTOSTR(PKEY_Contact_JobTitle)
			_PKEYTOSTR(PKEY_Contact_Label)
			_PKEYTOSTR(PKEY_Contact_LastName)
			_PKEYTOSTR(PKEY_Contact_MailingAddress)
			_PKEYTOSTR(PKEY_Contact_MiddleName)
			_PKEYTOSTR(PKEY_Contact_MobileTelephone)
			_PKEYTOSTR(PKEY_Contact_NickName)
			_PKEYTOSTR(PKEY_Contact_OfficeLocation)
			_PKEYTOSTR(PKEY_Contact_OtherAddress)
			_PKEYTOSTR(PKEY_Contact_OtherAddress1Country)
			_PKEYTOSTR(PKEY_Contact_OtherAddress1Locality)
			_PKEYTOSTR(PKEY_Contact_OtherAddress1PostalCode)
			_PKEYTOSTR(PKEY_Contact_OtherAddress1Region)
			_PKEYTOSTR(PKEY_Contact_OtherAddress1Street)
			_PKEYTOSTR(PKEY_Contact_OtherAddress2Country)
			_PKEYTOSTR(PKEY_Contact_OtherAddress2Locality)
			_PKEYTOSTR(PKEY_Contact_OtherAddress2PostalCode)
			_PKEYTOSTR(PKEY_Contact_OtherAddress2Region)
			_PKEYTOSTR(PKEY_Contact_OtherAddress2Street)
			_PKEYTOSTR(PKEY_Contact_OtherAddress3Country)
			_PKEYTOSTR(PKEY_Contact_OtherAddress3Locality)
			_PKEYTOSTR(PKEY_Contact_OtherAddress3PostalCode)
			_PKEYTOSTR(PKEY_Contact_OtherAddress3Region)
			_PKEYTOSTR(PKEY_Contact_OtherAddress3Street)
			_PKEYTOSTR(PKEY_Contact_OtherAddressCity)
			_PKEYTOSTR(PKEY_Contact_OtherAddressCountry)
			_PKEYTOSTR(PKEY_Contact_OtherAddressPostalCode)
			_PKEYTOSTR(PKEY_Contact_OtherAddressPostOfficeBox)
			_PKEYTOSTR(PKEY_Contact_OtherAddressState)
			_PKEYTOSTR(PKEY_Contact_OtherAddressStreet)
			_PKEYTOSTR(PKEY_Contact_OtherEmailAddresses)
			_PKEYTOSTR(PKEY_Contact_PagerTelephone)
			_PKEYTOSTR(PKEY_Contact_PersonalTitle)
			_PKEYTOSTR(PKEY_Contact_PhoneNumbersCanonical)
			_PKEYTOSTR(PKEY_Contact_Prefix)
			_PKEYTOSTR(PKEY_Contact_PrimaryAddressCity)
			_PKEYTOSTR(PKEY_Contact_PrimaryAddressCountry)
			_PKEYTOSTR(PKEY_Contact_PrimaryAddressPostalCode)
			_PKEYTOSTR(PKEY_Contact_PrimaryAddressPostOfficeBox)
			_PKEYTOSTR(PKEY_Contact_PrimaryAddressState)
			_PKEYTOSTR(PKEY_Contact_PrimaryAddressStreet)
			_PKEYTOSTR(PKEY_Contact_PrimaryEmailAddress)
			_PKEYTOSTR(PKEY_Contact_PrimaryTelephone)
			_PKEYTOSTR(PKEY_Contact_Profession)
			_PKEYTOSTR(PKEY_Contact_SpouseName)
			_PKEYTOSTR(PKEY_Contact_Suffix)
			_PKEYTOSTR(PKEY_Contact_TelexNumber)
			_PKEYTOSTR(PKEY_Contact_TTYTDDTelephone)
			_PKEYTOSTR(PKEY_Contact_WebPage)
			_PKEYTOSTR(PKEY_Contact_Webpage2)
			_PKEYTOSTR(PKEY_Contact_Webpage3)

			_ENDGOOUT

			_PKEYTOSTR(PKEY_AcquisitionID)
			_PKEYTOSTR(PKEY_ApplicationDefinedProperties)
			_PKEYTOSTR(PKEY_ApplicationName)
			_PKEYTOSTR(PKEY_Author)
			_PKEYTOSTR(PKEY_CachedFileUpdaterContentIdForConflictResolution)
			_PKEYTOSTR(PKEY_CachedFileUpdaterContentIdForStream)
			_PKEYTOSTR(PKEY_Capacity)
			_PKEYTOSTR(PKEY_Category)
			_PKEYTOSTR(PKEY_Comment)
			_PKEYTOSTR(PKEY_Company)
			_PKEYTOSTR(PKEY_ComputerName)
			_PKEYTOSTR(PKEY_ContainedItems)
			_PKEYTOSTR(PKEY_ContentStatus)
			_PKEYTOSTR(PKEY_ContentType)
			_PKEYTOSTR(PKEY_Copyright)
			_PKEYTOSTR(PKEY_DataObjectFormat)
			_PKEYTOSTR(PKEY_DateAccessed)
			_PKEYTOSTR(PKEY_DateAcquired)
			_PKEYTOSTR(PKEY_DateArchived)
			_PKEYTOSTR(PKEY_DateCompleted)
			_PKEYTOSTR(PKEY_DateCreated)
			_PKEYTOSTR(PKEY_DateImported)
			_PKEYTOSTR(PKEY_DateModified)
			_PKEYTOSTR(PKEY_DefaultSaveLocationDisplay)
			_PKEYTOSTR(PKEY_DueDate)
			_PKEYTOSTR(PKEY_EndDate)
			_PKEYTOSTR(PKEY_ExpandoProperties)
			_PKEYTOSTR(PKEY_FileAllocationSize)
			_PKEYTOSTR(PKEY_FileAttributes)
			_PKEYTOSTR(PKEY_FileCount)
			_PKEYTOSTR(PKEY_FileDescription)
			_PKEYTOSTR(PKEY_FileExtension)
			_PKEYTOSTR(PKEY_FileFRN)
			_PKEYTOSTR(PKEY_FileName)
			_PKEYTOSTR(PKEY_FileOfflineAvailabilityStatus)
			_PKEYTOSTR(PKEY_FileOwner)
			_PKEYTOSTR(PKEY_FilePlaceholderStatus)
			_PKEYTOSTR(PKEY_FileVersion)
			_PKEYTOSTR(PKEY_FindData)
			_PKEYTOSTR(PKEY_FlagColor)
			_PKEYTOSTR(PKEY_FlagColorText)
			_PKEYTOSTR(PKEY_FlagStatus)
			_PKEYTOSTR(PKEY_FlagStatusText)
			_PKEYTOSTR(PKEY_FolderKind)
			_PKEYTOSTR(PKEY_FolderNameDisplay)
			_PKEYTOSTR(PKEY_FreeSpace)
			_PKEYTOSTR(PKEY_FullText)
			_PKEYTOSTR(PKEY_HighKeywords)
			_PKEYTOSTR(PKEY_Identity)
			_PKEYTOSTR(PKEY_Identity_Blob)
			_PKEYTOSTR(PKEY_Identity_DisplayName)
			_PKEYTOSTR(PKEY_Identity_InternetSid)
			_PKEYTOSTR(PKEY_Identity_IsMeIdentity)
			_PKEYTOSTR(PKEY_Identity_KeyProviderContext)
			_PKEYTOSTR(PKEY_Identity_KeyProviderName)
			_PKEYTOSTR(PKEY_Identity_LogonStatusString)
			_PKEYTOSTR(PKEY_Identity_PrimaryEmailAddress)
			_PKEYTOSTR(PKEY_Identity_PrimarySid)
			_PKEYTOSTR(PKEY_Identity_ProviderData)
			_PKEYTOSTR(PKEY_Identity_ProviderID)
			_PKEYTOSTR(PKEY_Identity_QualifiedUserName)
			_PKEYTOSTR(PKEY_Identity_UniqueID)
			_PKEYTOSTR(PKEY_Identity_UserName)
			_PKEYTOSTR(PKEY_IdentityProvider_Name)
			_PKEYTOSTR(PKEY_IdentityProvider_Picture)
			_PKEYTOSTR(PKEY_ImageParsingName)
			_PKEYTOSTR(PKEY_Importance)
			_PKEYTOSTR(PKEY_ImportanceText)
			_PKEYTOSTR(PKEY_IsAttachment)
			_PKEYTOSTR(PKEY_IsDefaultNonOwnerSaveLocation)
			_PKEYTOSTR(PKEY_IsDefaultSaveLocation)
			_PKEYTOSTR(PKEY_IsDeleted)
			_PKEYTOSTR(PKEY_IsEncrypted)
			_PKEYTOSTR(PKEY_IsFlagged)
			_PKEYTOSTR(PKEY_IsFlaggedComplete)
			_PKEYTOSTR(PKEY_IsIncomplete)
			_PKEYTOSTR(PKEY_IsLocationSupported)
			_PKEYTOSTR(PKEY_IsPinnedToNameSpaceTree)
			_PKEYTOSTR(PKEY_IsRead)

			_ENDGOOUT

			_PKEYTOSTR(PKEY_IsSearchOnlyItem)
			_PKEYTOSTR(PKEY_IsSendToTarget)
			_PKEYTOSTR(PKEY_IsShared)
			_PKEYTOSTR(PKEY_ItemAuthors)
			_PKEYTOSTR(PKEY_ItemClassType)
			_PKEYTOSTR(PKEY_ItemDate)
			_PKEYTOSTR(PKEY_ItemFolderNameDisplay)
			_PKEYTOSTR(PKEY_ItemFolderPathDisplay)
			_PKEYTOSTR(PKEY_ItemFolderPathDisplayNarrow)
			_PKEYTOSTR(PKEY_ItemName)
			_PKEYTOSTR(PKEY_ItemNameDisplay)
			_PKEYTOSTR(PKEY_ItemNameDisplayWithoutExtension)
			_PKEYTOSTR(PKEY_ItemNamePrefix)
			_PKEYTOSTR(PKEY_ItemNameSortOverride)
			_PKEYTOSTR(PKEY_ItemParticipants)
			_PKEYTOSTR(PKEY_ItemPathDisplay)
			_PKEYTOSTR(PKEY_ItemPathDisplayNarrow)
			_PKEYTOSTR(PKEY_ItemType)
			_PKEYTOSTR(PKEY_ItemTypeText)
			_PKEYTOSTR(PKEY_ItemUrl)
			_PKEYTOSTR(PKEY_Keywords)
			_PKEYTOSTR(PKEY_Kind)
			_PKEYTOSTR(PKEY_KindText)
			_PKEYTOSTR(PKEY_Language)
			_PKEYTOSTR(PKEY_LastSyncError)
			_PKEYTOSTR(PKEY_LowKeywords)
			_PKEYTOSTR(PKEY_MediumKeywords)
			_PKEYTOSTR(PKEY_MileageInformation)
			_PKEYTOSTR(PKEY_MIMEType)
			_PKEYTOSTR(PKEY_Null)
			_PKEYTOSTR(PKEY_OfflineAvailability)
			_PKEYTOSTR(PKEY_OfflineStatus)
			_PKEYTOSTR(PKEY_OriginalFileName)
			_PKEYTOSTR(PKEY_OwnerSID)
			_PKEYTOSTR(PKEY_ParentalRating)
			_PKEYTOSTR(PKEY_ParentalRatingReason)
			_PKEYTOSTR(PKEY_ParentalRatingsOrganization)
			_PKEYTOSTR(PKEY_ParsingBindContext)
			_PKEYTOSTR(PKEY_ParsingName)
			_PKEYTOSTR(PKEY_ParsingPath)
			_PKEYTOSTR(PKEY_PerceivedType)
			_PKEYTOSTR(PKEY_PercentFull)
			_PKEYTOSTR(PKEY_Priority)
			_PKEYTOSTR(PKEY_PriorityText)
			_PKEYTOSTR(PKEY_Project)
			_PKEYTOSTR(PKEY_ProviderItemID)
			_PKEYTOSTR(PKEY_Rating)
			_PKEYTOSTR(PKEY_RatingText)
			_PKEYTOSTR(PKEY_RemoteConflictingFile)
			_PKEYTOSTR(PKEY_Sensitivity)
			_PKEYTOSTR(PKEY_SensitivityText)
			_PKEYTOSTR(PKEY_SFGAOFlags)
			_PKEYTOSTR(PKEY_SharedWith)
			_PKEYTOSTR(PKEY_ShareUserRating)
			_PKEYTOSTR(PKEY_SharingStatus)
			_PKEYTOSTR(PKEY_Shell_OmitFromView)
			_PKEYTOSTR(PKEY_SimpleRating)
			_PKEYTOSTR(PKEY_Size)
			_PKEYTOSTR(PKEY_SoftwareUsed)
			_PKEYTOSTR(PKEY_SourceItem)
			_PKEYTOSTR(PKEY_StartDate)
			_PKEYTOSTR(PKEY_Status)
			_PKEYTOSTR(PKEY_StorageProviderError)
			_PKEYTOSTR(PKEY_StorageProviderFileIdentifier)
			_PKEYTOSTR(PKEY_StorageProviderFileRemoteUri)
			_PKEYTOSTR(PKEY_StorageProviderFileVersion)
			_PKEYTOSTR(PKEY_StorageProviderId)
			_PKEYTOSTR(PKEY_StorageProviderShareStatuses)
			_PKEYTOSTR(PKEY_StorageProviderSharingStatus)
			_PKEYTOSTR(PKEY_StorageProviderStatus)
			_PKEYTOSTR(PKEY_Subject)
			_PKEYTOSTR(PKEY_SyncTransferStatus)
			_PKEYTOSTR(PKEY_Thumbnail)
			_PKEYTOSTR(PKEY_ThumbnailCacheId)
			_PKEYTOSTR(PKEY_ThumbnailStream)
			_PKEYTOSTR(PKEY_Title)
			_PKEYTOSTR(PKEY_TitleSortOverride)
			_PKEYTOSTR(PKEY_TotalFileSize)
			_PKEYTOSTR(PKEY_Trademarks)
			_PKEYTOSTR(PKEY_TransferOrder)
			_PKEYTOSTR(PKEY_TransferPosition)
			_PKEYTOSTR(PKEY_TransferSize)
			_PKEYTOSTR(PKEY_VolumeId)
			_PKEYTOSTR(PKEY_ZoneIdentifier)

			_ENDGOOUT

			_PKEYTOSTR(PKEY_Device_PrinterURL)
			_PKEYTOSTR(PKEY_DeviceInterface_Bluetooth_Manufacturer)
			_PKEYTOSTR(PKEY_DeviceInterface_Bluetooth_ModelNumber)
			_PKEYTOSTR(PKEY_DeviceInterface_Bluetooth_ProductId)
			_PKEYTOSTR(PKEY_DeviceInterface_Bluetooth_ServiceGuid)
			_PKEYTOSTR(PKEY_DeviceInterface_Bluetooth_VendorId)
			_PKEYTOSTR(PKEY_DeviceInterface_Hid_IsReadOnly)
			_PKEYTOSTR(PKEY_DeviceInterface_Hid_ProductId)
			_PKEYTOSTR(PKEY_DeviceInterface_Hid_UsageId)
			_PKEYTOSTR(PKEY_DeviceInterface_Hid_UsagePage)
			_PKEYTOSTR(PKEY_DeviceInterface_Hid_VendorId)
			_PKEYTOSTR(PKEY_DeviceInterface_Hid_VersionNumber)
			_PKEYTOSTR(PKEY_DeviceInterface_PrinterDriverDirectory)
			_PKEYTOSTR(PKEY_DeviceInterface_PrinterDriverName)
			_PKEYTOSTR(PKEY_DeviceInterface_PrinterName)
			_PKEYTOSTR(PKEY_DeviceInterface_PrinterPortName)
			_PKEYTOSTR(PKEY_DeviceInterface_Proximity_SupportsNfc)
			_PKEYTOSTR(PKEY_DeviceInterface_WinUsb_DeviceInterfaceClasses)
			_PKEYTOSTR(PKEY_DeviceInterface_WinUsb_UsbClass)
			_PKEYTOSTR(PKEY_DeviceInterface_WinUsb_UsbProductId)
			_PKEYTOSTR(PKEY_DeviceInterface_WinUsb_UsbProtocol)
			_PKEYTOSTR(PKEY_DeviceInterface_WinUsb_UsbSubClass)
			_PKEYTOSTR(PKEY_DeviceInterface_WinUsb_UsbVendorId)
			_PKEYTOSTR(PKEY_Devices_AppPackageFamilyName)
			_PKEYTOSTR(PKEY_Devices_AudioDevice_RawProcessingSupported)
			_PKEYTOSTR(PKEY_Devices_BatteryLife)
			_PKEYTOSTR(PKEY_Devices_BatteryPlusCharging)
			_PKEYTOSTR(PKEY_Devices_BatteryPlusChargingText)
			_PKEYTOSTR(PKEY_Devices_Category)
			_PKEYTOSTR(PKEY_Devices_CategoryGroup)
			_PKEYTOSTR(PKEY_Devices_CategoryPlural)
			_PKEYTOSTR(PKEY_Devices_ChargingState)
			_PKEYTOSTR(PKEY_Devices_Children)
			_PKEYTOSTR(PKEY_Devices_CompatibleIds)
			_PKEYTOSTR(PKEY_Devices_Connected)
			_PKEYTOSTR(PKEY_Devices_ContainerId)
			_PKEYTOSTR(PKEY_Devices_DefaultTooltip)
			_PKEYTOSTR(PKEY_Devices_DeviceCapabilities)
			_PKEYTOSTR(PKEY_Devices_DeviceCharacteristics)
			_PKEYTOSTR(PKEY_Devices_DeviceDescription1)
			_PKEYTOSTR(PKEY_Devices_DeviceDescription2)
			_PKEYTOSTR(PKEY_Devices_DeviceHasProblem)
			_PKEYTOSTR(PKEY_Devices_DeviceInstanceId)
			_PKEYTOSTR(PKEY_Devices_DiscoveryMethod)
			_PKEYTOSTR(PKEY_Devices_FriendlyName)
			_PKEYTOSTR(PKEY_Devices_FunctionPaths)
			_PKEYTOSTR(PKEY_Devices_HardwareIds)
			_PKEYTOSTR(PKEY_Devices_Icon)
			_PKEYTOSTR(PKEY_Devices_InLocalMachineContainer)
			_PKEYTOSTR(PKEY_Devices_InterfaceClassGuid)
			_PKEYTOSTR(PKEY_Devices_InterfaceEnabled)
			_PKEYTOSTR(PKEY_Devices_InterfacePaths)
			_PKEYTOSTR(PKEY_Devices_IpAddress)
			_PKEYTOSTR(PKEY_Devices_IsDefault)
			_PKEYTOSTR(PKEY_Devices_IsNetworkConnected)
			_PKEYTOSTR(PKEY_Devices_IsShared)
			_PKEYTOSTR(PKEY_Devices_IsSoftwareInstalling)
			_PKEYTOSTR(PKEY_Devices_LaunchDeviceStageFromExplorer)
			_PKEYTOSTR(PKEY_Devices_LocalMachine)
			_PKEYTOSTR(PKEY_Devices_LocationPaths)
			_PKEYTOSTR(PKEY_Devices_Manufacturer)
			_PKEYTOSTR(PKEY_Devices_MetadataPath)
			_PKEYTOSTR(PKEY_Devices_MicrophoneArray_Geometry)
			_PKEYTOSTR(PKEY_Devices_MissedCalls)
			_PKEYTOSTR(PKEY_Devices_ModelId)
			_PKEYTOSTR(PKEY_Devices_ModelName)
			_PKEYTOSTR(PKEY_Devices_ModelNumber)
			_PKEYTOSTR(PKEY_Devices_NetworkedTooltip)
			_PKEYTOSTR(PKEY_Devices_NetworkName)
			_PKEYTOSTR(PKEY_Devices_NetworkType)
			_PKEYTOSTR(PKEY_Devices_NewPictures)
			_PKEYTOSTR(PKEY_Devices_Notification)
			_PKEYTOSTR(PKEY_Devices_Notifications_LowBattery)
			_PKEYTOSTR(PKEY_Devices_Notifications_MissedCall)
			_PKEYTOSTR(PKEY_Devices_Notifications_NewMessage)
			_PKEYTOSTR(PKEY_Devices_Notifications_NewVoicemail)
			_PKEYTOSTR(PKEY_Devices_Notifications_StorageFull)
			_PKEYTOSTR(PKEY_Devices_Notifications_StorageFullLinkText)
			_PKEYTOSTR(PKEY_Devices_NotificationStore)
			_PKEYTOSTR(PKEY_Devices_NotWorkingProperly)
			_PKEYTOSTR(PKEY_Devices_Paired)
			_PKEYTOSTR(PKEY_Devices_Parent)
			_PKEYTOSTR(PKEY_Devices_PhysicalDeviceLocation)
			_PKEYTOSTR(PKEY_Devices_PlaybackPositionPercent)
			_PKEYTOSTR(PKEY_Devices_PlaybackState)
			_PKEYTOSTR(PKEY_Devices_PlaybackTitle)
			_PKEYTOSTR(PKEY_Devices_Present)
			_PKEYTOSTR(PKEY_Devices_PresentationUrl)
			_PKEYTOSTR(PKEY_Devices_PrimaryCategory)
			_PKEYTOSTR(PKEY_Devices_RemainingDuration)
			_PKEYTOSTR(PKEY_Devices_RestrictedInterface)
			_PKEYTOSTR(PKEY_Devices_Roaming)
			_PKEYTOSTR(PKEY_Devices_SafeRemovalRequired)
			_PKEYTOSTR(PKEY_Devices_ServiceAddress)
			_PKEYTOSTR(PKEY_Devices_ServiceId)
			_PKEYTOSTR(PKEY_Devices_SharedTooltip)
			_PKEYTOSTR(PKEY_Devices_SignalStrength)
			_PKEYTOSTR(PKEY_Devices_SmartCards_ReaderKind)
			_PKEYTOSTR(PKEY_Devices_Status)
			_PKEYTOSTR(PKEY_Devices_Status1)
			_PKEYTOSTR(PKEY_Devices_Status2)
			_PKEYTOSTR(PKEY_Devices_StorageCapacity)
			_PKEYTOSTR(PKEY_Devices_StorageFreeSpace)
			_PKEYTOSTR(PKEY_Devices_StorageFreeSpacePercent)
			_PKEYTOSTR(PKEY_Devices_TextMessages)
			_PKEYTOSTR(PKEY_Devices_Voicemail)
			_PKEYTOSTR(PKEY_Devices_WiaDeviceType)

			_ENDGOOUT

			_PKEYTOSTR(PKEY_Storage_Portable)
			_PKEYTOSTR(PKEY_Storage_RemovableMedia)
			_PKEYTOSTR(PKEY_Storage_SystemCritical)

			_PKEYTOSTR(PKEY_Document_ByteCount)
			_PKEYTOSTR(PKEY_Document_CharacterCount)
			_PKEYTOSTR(PKEY_Document_ClientID)
			_PKEYTOSTR(PKEY_Document_Contributor)
			_PKEYTOSTR(PKEY_Document_DateCreated)
			_PKEYTOSTR(PKEY_Document_DatePrinted)
			_PKEYTOSTR(PKEY_Document_DateSaved)
			_PKEYTOSTR(PKEY_Document_Division)
			_PKEYTOSTR(PKEY_Document_DocumentID)
			_PKEYTOSTR(PKEY_Document_HiddenSlideCount)
			_PKEYTOSTR(PKEY_Document_LastAuthor)
			_PKEYTOSTR(PKEY_Document_LineCount)
			_PKEYTOSTR(PKEY_Document_Manager)
			_PKEYTOSTR(PKEY_Document_MultimediaClipCount)
			_PKEYTOSTR(PKEY_Document_NoteCount)
			_PKEYTOSTR(PKEY_Document_PageCount)
			_PKEYTOSTR(PKEY_Document_ParagraphCount)
			_PKEYTOSTR(PKEY_Document_PresentationFormat)
			_PKEYTOSTR(PKEY_Document_RevisionNumber)
			_PKEYTOSTR(PKEY_Document_Security)
			_PKEYTOSTR(PKEY_Document_SlideCount)
			_PKEYTOSTR(PKEY_Document_Template)
			_PKEYTOSTR(PKEY_Document_TotalEditingTime)
			_PKEYTOSTR(PKEY_Document_Version)
			_PKEYTOSTR(PKEY_Document_WordCount)

			_PKEYTOSTR(PKEY_DRM_DatePlayExpires)
			_PKEYTOSTR(PKEY_DRM_DatePlayStarts)
			_PKEYTOSTR(PKEY_DRM_Description)
			_PKEYTOSTR(PKEY_DRM_IsProtected)
			_PKEYTOSTR(PKEY_DRM_PlayCount)

			_ENDGOOUT

			_PKEYTOSTR(PKEY_GPS_Altitude)
			_PKEYTOSTR(PKEY_GPS_AltitudeDenominator)
			_PKEYTOSTR(PKEY_GPS_AltitudeNumerator)
			_PKEYTOSTR(PKEY_GPS_AltitudeRef)
			_PKEYTOSTR(PKEY_GPS_AreaInformation)
			_PKEYTOSTR(PKEY_GPS_Date)
			_PKEYTOSTR(PKEY_GPS_DestBearing)
			_PKEYTOSTR(PKEY_GPS_DestBearingDenominator)
			_PKEYTOSTR(PKEY_GPS_DestBearingNumerator)
			_PKEYTOSTR(PKEY_GPS_DestBearingRef)
			_PKEYTOSTR(PKEY_GPS_DestDistance)
			_PKEYTOSTR(PKEY_GPS_DestDistanceDenominator)
			_PKEYTOSTR(PKEY_GPS_DestDistanceNumerator)
			_PKEYTOSTR(PKEY_GPS_DestDistanceRef)
			_PKEYTOSTR(PKEY_GPS_DestLatitude)
			_PKEYTOSTR(PKEY_GPS_DestLatitudeDenominator)
			_PKEYTOSTR(PKEY_GPS_DestLatitudeNumerator)
			_PKEYTOSTR(PKEY_GPS_DestLatitudeRef)
			_PKEYTOSTR(PKEY_GPS_DestLongitude)
			_PKEYTOSTR(PKEY_GPS_DestLongitudeDenominator)
			_PKEYTOSTR(PKEY_GPS_DestLongitudeNumerator)
			_PKEYTOSTR(PKEY_GPS_DestLongitudeRef)
			_PKEYTOSTR(PKEY_GPS_Differential)
			_PKEYTOSTR(PKEY_GPS_DOP)
			_PKEYTOSTR(PKEY_GPS_DOPDenominator)
			_PKEYTOSTR(PKEY_GPS_DOPNumerator)
			_PKEYTOSTR(PKEY_GPS_ImgDirection)
			_PKEYTOSTR(PKEY_GPS_ImgDirectionDenominator)
			_PKEYTOSTR(PKEY_GPS_ImgDirectionNumerator)
			_PKEYTOSTR(PKEY_GPS_ImgDirectionRef)
			_PKEYTOSTR(PKEY_GPS_Latitude)
			_PKEYTOSTR(PKEY_GPS_LatitudeDecimal)
			_PKEYTOSTR(PKEY_GPS_LatitudeDenominator)
			_PKEYTOSTR(PKEY_GPS_LatitudeNumerator)
			_PKEYTOSTR(PKEY_GPS_LatitudeRef)
			_PKEYTOSTR(PKEY_GPS_Longitude)
			_PKEYTOSTR(PKEY_GPS_LongitudeDecimal)
			_PKEYTOSTR(PKEY_GPS_LongitudeDenominator)
			_PKEYTOSTR(PKEY_GPS_LongitudeNumerator)
			_PKEYTOSTR(PKEY_GPS_LongitudeRef)
			_PKEYTOSTR(PKEY_GPS_MapDatum)
			_PKEYTOSTR(PKEY_GPS_MeasureMode)
			_PKEYTOSTR(PKEY_GPS_ProcessingMethod)
			_PKEYTOSTR(PKEY_GPS_Satellites)
			_PKEYTOSTR(PKEY_GPS_Speed)
			_PKEYTOSTR(PKEY_GPS_SpeedDenominator)
			_PKEYTOSTR(PKEY_GPS_SpeedNumerator)
			_PKEYTOSTR(PKEY_GPS_SpeedRef)
			_PKEYTOSTR(PKEY_GPS_Status)
			_PKEYTOSTR(PKEY_GPS_Track)
			_PKEYTOSTR(PKEY_GPS_TrackDenominator)
			_PKEYTOSTR(PKEY_GPS_TrackNumerator)
			_PKEYTOSTR(PKEY_GPS_TrackRef)
			_PKEYTOSTR(PKEY_GPS_VersionID)

			_PKEYTOSTR(PKEY_History_VisitCount)

			_PKEYTOSTR(PKEY_Image_BitDepth)
			_PKEYTOSTR(PKEY_Image_ColorSpace)
			_PKEYTOSTR(PKEY_Image_CompressedBitsPerPixel)
			_PKEYTOSTR(PKEY_Image_CompressedBitsPerPixelDenominator)
			_PKEYTOSTR(PKEY_Image_CompressedBitsPerPixelNumerator)
			_PKEYTOSTR(PKEY_Image_Compression)
			_PKEYTOSTR(PKEY_Image_CompressionText)
			_PKEYTOSTR(PKEY_Image_Dimensions)
			_PKEYTOSTR(PKEY_Image_HorizontalResolution)
			_PKEYTOSTR(PKEY_Image_HorizontalSize)
			_PKEYTOSTR(PKEY_Image_ImageID)
			_PKEYTOSTR(PKEY_Image_ResolutionUnit)
			_PKEYTOSTR(PKEY_Image_VerticalResolution)
			_PKEYTOSTR(PKEY_Image_VerticalSize)

			_PKEYTOSTR(PKEY_Journal_Contacts)
			_PKEYTOSTR(PKEY_Journal_EntryType)

			_PKEYTOSTR(PKEY_LayoutPattern_ContentViewModeForBrowse)
			_PKEYTOSTR(PKEY_LayoutPattern_ContentViewModeForSearch)

			_PKEYTOSTR(PKEY_History_SelectionCount)
			_PKEYTOSTR(PKEY_History_TargetUrlHostName)

			_PKEYTOSTR(PKEY_Link_Arguments)
			_PKEYTOSTR(PKEY_Link_Comment)
			_PKEYTOSTR(PKEY_Link_DateVisited)
			_PKEYTOSTR(PKEY_Link_Description)
			_PKEYTOSTR(PKEY_Link_Status)
			_PKEYTOSTR(PKEY_Link_TargetExtension)
			_PKEYTOSTR(PKEY_Link_TargetParsingPath)
			_PKEYTOSTR(PKEY_Link_TargetSFGAOFlags)

			_ENDGOOUT

			_PKEYTOSTR(PKEY_Media_AuthorUrl)
			_PKEYTOSTR(PKEY_Media_AverageLevel)
			_PKEYTOSTR(PKEY_Media_ClassPrimaryID)
			_PKEYTOSTR(PKEY_Media_ClassSecondaryID)
			_PKEYTOSTR(PKEY_Media_CollectionGroupID)
			_PKEYTOSTR(PKEY_Media_CollectionID)
			_PKEYTOSTR(PKEY_Media_ContentDistributor)
			_PKEYTOSTR(PKEY_Media_ContentID)
			_PKEYTOSTR(PKEY_Media_CreatorApplication)
			_PKEYTOSTR(PKEY_Media_CreatorApplicationVersion)
			_PKEYTOSTR(PKEY_Media_DateEncoded)
			_PKEYTOSTR(PKEY_Media_DateReleased)
			_PKEYTOSTR(PKEY_Media_DlnaProfileID)
			_PKEYTOSTR(PKEY_Media_Duration)
			_PKEYTOSTR(PKEY_Media_DVDID)
			_PKEYTOSTR(PKEY_Media_EncodedBy)
			_PKEYTOSTR(PKEY_Media_EncodingSettings)
			_PKEYTOSTR(PKEY_Media_EpisodeNumber)
			_PKEYTOSTR(PKEY_Media_FrameCount)
			_PKEYTOSTR(PKEY_Media_MCDI)
			_PKEYTOSTR(PKEY_Media_MetadataContentProvider)
			_PKEYTOSTR(PKEY_Media_Producer)
			_PKEYTOSTR(PKEY_Media_PromotionUrl)
			_PKEYTOSTR(PKEY_Media_ProtectionType)
			_PKEYTOSTR(PKEY_Media_ProviderRating)
			_PKEYTOSTR(PKEY_Media_ProviderStyle)
			_PKEYTOSTR(PKEY_Media_Publisher)
			_PKEYTOSTR(PKEY_Media_SeasonNumber)
			_PKEYTOSTR(PKEY_Media_SubscriptionContentId)
			_PKEYTOSTR(PKEY_Media_SubTitle)
			_PKEYTOSTR(PKEY_Media_UniqueFileIdentifier)
			_PKEYTOSTR(PKEY_Media_UserNoAutoInfo)
			_PKEYTOSTR(PKEY_Media_UserWebUrl)
			_PKEYTOSTR(PKEY_Media_Writer)
			_PKEYTOSTR(PKEY_Media_Year)

			_PKEYTOSTR(PKEY_Message_AttachmentContents)
			_PKEYTOSTR(PKEY_Message_AttachmentNames)
			_PKEYTOSTR(PKEY_Message_BccAddress)
			_PKEYTOSTR(PKEY_Message_BccName)
			_PKEYTOSTR(PKEY_Message_CcAddress)
			_PKEYTOSTR(PKEY_Message_CcName)
			_PKEYTOSTR(PKEY_Message_ConversationID)
			_PKEYTOSTR(PKEY_Message_ConversationIndex)
			_PKEYTOSTR(PKEY_Message_DateReceived)
			_PKEYTOSTR(PKEY_Message_DateSent)
			_PKEYTOSTR(PKEY_Message_Flags)
			_PKEYTOSTR(PKEY_Message_FromAddress)
			_PKEYTOSTR(PKEY_Message_FromName)
			_PKEYTOSTR(PKEY_Message_HasAttachments)
			_PKEYTOSTR(PKEY_Message_IsFwdOrReply)
			_PKEYTOSTR(PKEY_Message_MessageClass)
			_PKEYTOSTR(PKEY_Message_Participants)
			_PKEYTOSTR(PKEY_Message_ProofInProgress)
			_PKEYTOSTR(PKEY_Message_SenderAddress)
			_PKEYTOSTR(PKEY_Message_SenderName)
			_PKEYTOSTR(PKEY_Message_Store)
			_PKEYTOSTR(PKEY_Message_ToAddress)
			_PKEYTOSTR(PKEY_Message_ToDoFlags)
			_PKEYTOSTR(PKEY_Message_ToDoTitle)
			_PKEYTOSTR(PKEY_Message_ToName)

			_ENDGOOUT

			_PKEYTOSTR(PKEY_Music_AlbumArtist)
			_PKEYTOSTR(PKEY_Music_AlbumArtistSortOverride)
			_PKEYTOSTR(PKEY_Music_AlbumID)
			_PKEYTOSTR(PKEY_Music_AlbumTitle)
			_PKEYTOSTR(PKEY_Music_AlbumTitleSortOverride)
			_PKEYTOSTR(PKEY_Music_Artist)
			_PKEYTOSTR(PKEY_Music_ArtistSortOverride)
			_PKEYTOSTR(PKEY_Music_BeatsPerMinute)
			_PKEYTOSTR(PKEY_Music_Composer)
			_PKEYTOSTR(PKEY_Music_ComposerSortOverride)
			_PKEYTOSTR(PKEY_Music_Conductor)
			_PKEYTOSTR(PKEY_Music_ContentGroupDescription)
			_PKEYTOSTR(PKEY_Music_DisplayArtist)
			_PKEYTOSTR(PKEY_Music_Genre)
			_PKEYTOSTR(PKEY_Music_InitialKey)
			_PKEYTOSTR(PKEY_Music_IsCompilation)
			_PKEYTOSTR(PKEY_Music_Lyrics)
			_PKEYTOSTR(PKEY_Music_Mood)
			_PKEYTOSTR(PKEY_Music_PartOfSet)
			_PKEYTOSTR(PKEY_Music_Period)
			_PKEYTOSTR(PKEY_Music_SynchronizedLyrics)
			_PKEYTOSTR(PKEY_Music_TrackNumber)

			_PKEYTOSTR(PKEY_Note_Color)
			_PKEYTOSTR(PKEY_Note_ColorText)

			_ENDGOOUT

			_PKEYTOSTR(PKEY_Photo_Aperture)
			_PKEYTOSTR(PKEY_Photo_ApertureDenominator)
			_PKEYTOSTR(PKEY_Photo_ApertureNumerator)
			_PKEYTOSTR(PKEY_Photo_Brightness)
			_PKEYTOSTR(PKEY_Photo_BrightnessDenominator)
			_PKEYTOSTR(PKEY_Photo_BrightnessNumerator)
			_PKEYTOSTR(PKEY_Photo_CameraManufacturer)
			_PKEYTOSTR(PKEY_Photo_CameraModel)
			_PKEYTOSTR(PKEY_Photo_CameraSerialNumber)
			_PKEYTOSTR(PKEY_Photo_Contrast)
			_PKEYTOSTR(PKEY_Photo_ContrastText)
			_PKEYTOSTR(PKEY_Photo_DateTaken)
			_PKEYTOSTR(PKEY_Photo_DigitalZoom)
			_PKEYTOSTR(PKEY_Photo_DigitalZoomDenominator)
			_PKEYTOSTR(PKEY_Photo_DigitalZoomNumerator)
			_PKEYTOSTR(PKEY_Photo_Event)
			_PKEYTOSTR(PKEY_Photo_EXIFVersion)
			_PKEYTOSTR(PKEY_Photo_ExposureBias)
			_PKEYTOSTR(PKEY_Photo_ExposureBiasDenominator)
			_PKEYTOSTR(PKEY_Photo_ExposureBiasNumerator)
			_PKEYTOSTR(PKEY_Photo_ExposureIndex)
			_PKEYTOSTR(PKEY_Photo_ExposureIndexDenominator)
			_PKEYTOSTR(PKEY_Photo_ExposureIndexNumerator)
			_PKEYTOSTR(PKEY_Photo_ExposureProgram)
			_PKEYTOSTR(PKEY_Photo_ExposureProgramText)
			_PKEYTOSTR(PKEY_Photo_ExposureTime)
			_PKEYTOSTR(PKEY_Photo_ExposureTimeDenominator)
			_PKEYTOSTR(PKEY_Photo_ExposureTimeNumerator)
			_PKEYTOSTR(PKEY_Photo_Flash)
			_PKEYTOSTR(PKEY_Photo_FlashEnergy)
			_PKEYTOSTR(PKEY_Photo_FlashEnergyDenominator)
			_PKEYTOSTR(PKEY_Photo_FlashEnergyNumerator)
			_PKEYTOSTR(PKEY_Photo_FlashManufacturer)
			_PKEYTOSTR(PKEY_Photo_FlashModel)
			_PKEYTOSTR(PKEY_Photo_FlashText)
			_PKEYTOSTR(PKEY_Photo_FNumber)
			_PKEYTOSTR(PKEY_Photo_FNumberDenominator)
			_PKEYTOSTR(PKEY_Photo_FNumberNumerator)
			_PKEYTOSTR(PKEY_Photo_FocalLength)
			_PKEYTOSTR(PKEY_Photo_FocalLengthDenominator)
			_PKEYTOSTR(PKEY_Photo_FocalLengthInFilm)
			_PKEYTOSTR(PKEY_Photo_FocalLengthNumerator)
			_PKEYTOSTR(PKEY_Photo_FocalPlaneXResolution)
			_PKEYTOSTR(PKEY_Photo_FocalPlaneXResolutionDenominator)
			_PKEYTOSTR(PKEY_Photo_FocalPlaneXResolutionNumerator)
			_PKEYTOSTR(PKEY_Photo_FocalPlaneYResolution)
			_PKEYTOSTR(PKEY_Photo_FocalPlaneYResolutionDenominator)
			_PKEYTOSTR(PKEY_Photo_FocalPlaneYResolutionNumerator)
			_PKEYTOSTR(PKEY_Photo_GainControl)
			_PKEYTOSTR(PKEY_Photo_GainControlDenominator)
			_PKEYTOSTR(PKEY_Photo_GainControlNumerator)
			_PKEYTOSTR(PKEY_Photo_GainControlText)
			_PKEYTOSTR(PKEY_Photo_ISOSpeed)
			_PKEYTOSTR(PKEY_Photo_LensManufacturer)
			_PKEYTOSTR(PKEY_Photo_LensModel)
			_PKEYTOSTR(PKEY_Photo_LightSource)
			_PKEYTOSTR(PKEY_Photo_MakerNote)
			_PKEYTOSTR(PKEY_Photo_MakerNoteOffset)
			_PKEYTOSTR(PKEY_Photo_MaxAperture)
			_PKEYTOSTR(PKEY_Photo_MaxApertureDenominator)
			_PKEYTOSTR(PKEY_Photo_MaxApertureNumerator)
			_PKEYTOSTR(PKEY_Photo_MeteringMode)
			_PKEYTOSTR(PKEY_Photo_MeteringModeText)
			_PKEYTOSTR(PKEY_Photo_Orientation)
			_PKEYTOSTR(PKEY_Photo_OrientationText)
			_PKEYTOSTR(PKEY_Photo_PeopleNames)
			_PKEYTOSTR(PKEY_Photo_PhotometricInterpretation)
			_PKEYTOSTR(PKEY_Photo_PhotometricInterpretationText)
			_PKEYTOSTR(PKEY_Photo_ProgramMode)
			_PKEYTOSTR(PKEY_Photo_ProgramModeText)
			_PKEYTOSTR(PKEY_Photo_RelatedSoundFile)
			_PKEYTOSTR(PKEY_Photo_Saturation)
			_PKEYTOSTR(PKEY_Photo_SaturationText)
			_PKEYTOSTR(PKEY_Photo_Sharpness)
			_PKEYTOSTR(PKEY_Photo_SharpnessText)
			_PKEYTOSTR(PKEY_Photo_ShutterSpeed)
			_PKEYTOSTR(PKEY_Photo_ShutterSpeedDenominator)
			_PKEYTOSTR(PKEY_Photo_ShutterSpeedNumerator)
			_PKEYTOSTR(PKEY_Photo_SubjectDistance)
			_PKEYTOSTR(PKEY_Photo_SubjectDistanceDenominator)
			_PKEYTOSTR(PKEY_Photo_SubjectDistanceNumerator)
			_PKEYTOSTR(PKEY_Photo_TagViewAggregate)
			_PKEYTOSTR(PKEY_Photo_TranscodedForSync)
			_PKEYTOSTR(PKEY_Photo_WhiteBalance)
			_PKEYTOSTR(PKEY_Photo_WhiteBalanceText)

			_ENDGOOUT

			_PKEYTOSTR(PKEY_PropGroup_Advanced)
			_PKEYTOSTR(PKEY_PropGroup_Audio)
			_PKEYTOSTR(PKEY_PropGroup_Calendar)
			_PKEYTOSTR(PKEY_PropGroup_Camera)
			_PKEYTOSTR(PKEY_PropGroup_Contact)
			_PKEYTOSTR(PKEY_PropGroup_Content)
			_PKEYTOSTR(PKEY_PropGroup_Description)
			_PKEYTOSTR(PKEY_PropGroup_FileSystem)
			_PKEYTOSTR(PKEY_PropGroup_General)
			_PKEYTOSTR(PKEY_PropGroup_GPS)
			_PKEYTOSTR(PKEY_PropGroup_Image)
			_PKEYTOSTR(PKEY_PropGroup_Media)
			_PKEYTOSTR(PKEY_PropGroup_MediaAdvanced)
			_PKEYTOSTR(PKEY_PropGroup_Message)
			_PKEYTOSTR(PKEY_PropGroup_Music)
			_PKEYTOSTR(PKEY_PropGroup_Origin)
			_PKEYTOSTR(PKEY_PropGroup_PhotoAdvanced)
			_PKEYTOSTR(PKEY_PropGroup_RecordedTV)
			_PKEYTOSTR(PKEY_PropGroup_Video)

			_PKEYTOSTR(PKEY_InfoTipText)

			_PKEYTOSTR(PKEY_PropList_ConflictPrompt)
			_PKEYTOSTR(PKEY_PropList_ContentViewModeForBrowse)
			_PKEYTOSTR(PKEY_PropList_ContentViewModeForSearch)
			_PKEYTOSTR(PKEY_PropList_ExtendedTileInfo)
			_PKEYTOSTR(PKEY_PropList_FileOperationPrompt)
			_PKEYTOSTR(PKEY_PropList_FullDetails)
			_PKEYTOSTR(PKEY_PropList_InfoTip)
			_PKEYTOSTR(PKEY_PropList_NonPersonal)
			_PKEYTOSTR(PKEY_PropList_PreviewDetails)
			_PKEYTOSTR(PKEY_PropList_PreviewTitle)
			_PKEYTOSTR(PKEY_PropList_QuickTip)
			_PKEYTOSTR(PKEY_PropList_TileInfo)
			_PKEYTOSTR(PKEY_PropList_XPDetailsPanel)

			_PKEYTOSTR(PKEY_RecordedTV_ChannelNumber)
			_PKEYTOSTR(PKEY_RecordedTV_Credits)
			_PKEYTOSTR(PKEY_RecordedTV_DateContentExpires)
			_PKEYTOSTR(PKEY_RecordedTV_EpisodeName)
			_PKEYTOSTR(PKEY_RecordedTV_IsATSCContent)
			_PKEYTOSTR(PKEY_RecordedTV_IsClosedCaptioningAvailable)
			_PKEYTOSTR(PKEY_RecordedTV_IsDTVContent)
			_PKEYTOSTR(PKEY_RecordedTV_IsHDContent)
			_PKEYTOSTR(PKEY_RecordedTV_IsRepeatBroadcast)
			_PKEYTOSTR(PKEY_RecordedTV_IsSAP)
			_PKEYTOSTR(PKEY_RecordedTV_NetworkAffiliation)
			_PKEYTOSTR(PKEY_RecordedTV_OriginalBroadcastDate)
			_PKEYTOSTR(PKEY_RecordedTV_ProgramDescription)
			_PKEYTOSTR(PKEY_RecordedTV_RecordingTime)
			_PKEYTOSTR(PKEY_RecordedTV_StationCallSign)
			_PKEYTOSTR(PKEY_RecordedTV_StationName)

			_PKEYTOSTR(PKEY_Search_AutoSummary)
			_PKEYTOSTR(PKEY_Search_ContainerHash)
			_PKEYTOSTR(PKEY_Search_Contents)
			_PKEYTOSTR(PKEY_Search_EntryID)
			_PKEYTOSTR(PKEY_Search_ExtendedProperties)
			_PKEYTOSTR(PKEY_Search_GatherTime)
			_PKEYTOSTR(PKEY_Search_HitCount)
			_PKEYTOSTR(PKEY_Search_IsClosedDirectory)
			_PKEYTOSTR(PKEY_Search_IsFullyContained)
			_PKEYTOSTR(PKEY_Search_QueryFocusedSummary)
			_PKEYTOSTR(PKEY_Search_QueryFocusedSummaryWithFallback)
			_PKEYTOSTR(PKEY_Search_QueryPropertyHits)
			_PKEYTOSTR(PKEY_Search_Rank)
			_PKEYTOSTR(PKEY_Search_Store)
			_PKEYTOSTR(PKEY_Search_UrlToIndex)
			_PKEYTOSTR(PKEY_Search_UrlToIndexWithModificationTime)

			_ENDGOOUT

			_PKEYTOSTR(PKEY_DescriptionID)
			_PKEYTOSTR(PKEY_InternalName)
			_PKEYTOSTR(PKEY_LibraryLocationsCount)
			_PKEYTOSTR(PKEY_Link_TargetSFGAOFlagsStrings)
			_PKEYTOSTR(PKEY_Link_TargetUrl)
			_PKEYTOSTR(PKEY_NamespaceCLSID)
			_PKEYTOSTR(PKEY_Shell_SFGAOFlagsStrings)
			_PKEYTOSTR(PKEY_StatusBarSelectedItemCount)
			_PKEYTOSTR(PKEY_StatusBarViewItemCount)

			_PKEYTOSTR(PKEY_AppUserModel_ExcludeFromShowInNewInstall)
			_PKEYTOSTR(PKEY_AppUserModel_ID)
			_PKEYTOSTR(PKEY_AppUserModel_IsDestListSeparator)
			_PKEYTOSTR(PKEY_AppUserModel_IsDualMode)
			_PKEYTOSTR(PKEY_AppUserModel_PreventPinning)
			_PKEYTOSTR(PKEY_AppUserModel_RelaunchCommand)
			_PKEYTOSTR(PKEY_AppUserModel_RelaunchDisplayNameResource)
			_PKEYTOSTR(PKEY_AppUserModel_RelaunchIconResource)
			_PKEYTOSTR(PKEY_AppUserModel_StartPinOption)

			_PKEYTOSTR(PKEY_EdgeGesture_DisableTouchWhenFullscreen)

			_PKEYTOSTR(PKEY_Software_DateLastUsed)
			_PKEYTOSTR(PKEY_Software_ProductName)

			_PKEYTOSTR(PKEY_Sync_Comments)
			_PKEYTOSTR(PKEY_Sync_ConflictDescription)
			_PKEYTOSTR(PKEY_Sync_ConflictFirstLocation)
			_PKEYTOSTR(PKEY_Sync_ConflictSecondLocation)
			_PKEYTOSTR(PKEY_Sync_HandlerCollectionID)
			_PKEYTOSTR(PKEY_Sync_HandlerID)
			_PKEYTOSTR(PKEY_Sync_HandlerName)
			_PKEYTOSTR(PKEY_Sync_HandlerType)
			_PKEYTOSTR(PKEY_Sync_HandlerTypeLabel)
			_PKEYTOSTR(PKEY_Sync_ItemID)
			_PKEYTOSTR(PKEY_Sync_ItemName)
			_PKEYTOSTR(PKEY_Sync_ProgressPercentage)
			_PKEYTOSTR(PKEY_Sync_State)
			_PKEYTOSTR(PKEY_Sync_Status)

			_PKEYTOSTR(PKEY_Task_BillingInformation)
			_PKEYTOSTR(PKEY_Task_CompletionStatus)
			_PKEYTOSTR(PKEY_Task_Owner)

			_PKEYTOSTR(PKEY_Video_Compression)
			_PKEYTOSTR(PKEY_Video_Director)
			_PKEYTOSTR(PKEY_Video_EncodingBitrate)
			_PKEYTOSTR(PKEY_Video_FourCC)
			_PKEYTOSTR(PKEY_Video_FrameHeight)
			_PKEYTOSTR(PKEY_Video_FrameRate)
			_PKEYTOSTR(PKEY_Video_FrameWidth)
			_PKEYTOSTR(PKEY_Video_HorizontalAspectRatio)
			_PKEYTOSTR(PKEY_Video_IsStereo)
			_PKEYTOSTR(PKEY_Video_Orientation)
			_PKEYTOSTR(PKEY_Video_SampleSize)
			_PKEYTOSTR(PKEY_Video_StreamName)
			_PKEYTOSTR(PKEY_Video_StreamNumber)
			_PKEYTOSTR(PKEY_Video_TotalBitrate)
			_PKEYTOSTR(PKEY_Video_TranscodedForSync)
			_PKEYTOSTR(PKEY_Video_VerticalAspectRatio)

			_PKEYTOSTR(PKEY_Volume_FileSystem)
			_PKEYTOSTR(PKEY_Volume_IsMappedDrive)
			_PKEYTOSTR(PKEY_Volume_IsRoot)

			_ENDGOOUT

			// <propkey.h>
#pragma endregion 

			#undef _PKEYTOSTR

			WSTR_out = UNK_PROPKEY_WSTR;

			#undef _ENDGOOUT
		}

		//>> [DEBUG] Сообщает в консоль сырые свойства устройства (исходники)
		void PrintfDevicesPropretiesRAW(uint32 N, eSoundDevType type)
		{
			IMMDevice *			pDevice = nullptr;			// temp
			IPropertyStore *	pPropertyStore = nullptr;	// temp

			vector<SoundDeviceProp> * SDProp;

			switch (type)
			{
			case eSDTypeRender:  SDProp = &propRenderDevice;  break;
			case eSDTypeCapture: SDProp = &propCaptureDevice; break;
			default: 
				_MBM(L"Incorrect eSoundDevType [ReadDeviceProprety]");
				return;
			}

			if ((N > numRenderDevices  && type == eSDTypeRender) ||
				(N > numCaptureDevices && type == eSDTypeCapture))
				{ _MBM(ERROR_IDLIMIT); return; }

			auto & DEV = (*SDProp)[N];

			//////////////////////////////////////////

			if (S_OK != pEnumerator->GetDevice(DEV.ID.c_str(), &pDevice))
				{ _MBM(L"Не удалось pEnumerator->GetDevice()"); return; }
			if (S_OK != pDevice->OpenPropertyStore(STGM_READ, &pPropertyStore))
				{ _MBM(L"Не удалось OpenPropertyStore()"); return; }

			//////////////////////////////////////////

			DWORD cProps = 0;
			if (S_OK != pPropertyStore->GetCount(&cProps))
				_MBM(L"Не удалось pPropertyStore->GetCount()");

			PrintfDeviceProprety(N, type, 1, eSDP_NONE);
			wprintf(L"\n- Num of properties : %i", cProps);
			for (uint32 i = 0; i < cProps; i++)
			{
				PROPERTYKEY pKey;			
				pPropertyStore->GetAt(i, &pKey);

				PrintfDevicePropertyRAW(pPropertyStore, pKey, true);
			}

			_RELEASE(pPropertyStore);
			_RELEASE(pDevice);
		}

		//>> [DEBUG] Сообщает в консоль сырое свойство устройства (исходник)
		void PrintfDevicePropertyRAW(IPropertyStore * pPropertyStore, PROPERTYKEY pKey, bool value)
		{
			PROPVARIANT propVar;
			pPropertyStore->GetValue(pKey, &propVar);

			wstring pKeyInfo;
			ConvertPropertyToString(pKey, pKeyInfo);

			if (pKeyInfo.compare(UNK_PROPKEY_WSTR))
				wprintf(L"\n[__] %s", pKeyInfo.c_str());
			else
			{
				wprintf(L"\n[__] fmtid %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X pid %3i",
					pKey.fmtid.Data1, pKey.fmtid.Data2, pKey.fmtid.Data3,
					pKey.fmtid.Data4[0], pKey.fmtid.Data4[1], pKey.fmtid.Data4[2], pKey.fmtid.Data4[3],
					pKey.fmtid.Data4[4], pKey.fmtid.Data4[5], pKey.fmtid.Data4[6], pKey.fmtid.Data4[7],
					pKey.pid);

				value = true; 
			}
			//wprintf(L"\n");

				 if (propVar.vt == VT_ILLEGAL)       wprintf(L" VT_ILLEGAL");
			else if (propVar.vt == VT_ILLEGALMASKED) wprintf(L" VT_ILLEGALMASKED or VT_BSTR_BLOB");
			else
			{
				if (propVar.vt & VT_VECTOR)   wprintf(L" VT_VECTOR |");
				if (propVar.vt & VT_ARRAY)    wprintf(L" VT_ARRAY |");
				if (propVar.vt & VT_BYREF)    wprintf(L" VT_BYREF |");
				if (propVar.vt & VT_RESERVED) wprintf(L" VT_RESERVED |");

				uint16 masked = propVar.vt & VT_TYPEMASK;

				#define _PRINTVT(x)	if (masked == x) wprintf(L" " \
															TOWSTRING(x) ); else
				_PRINTVT(VT_EMPTY)
				_PRINTVT(VT_NULL)
				_PRINTVT(VT_I2)
				_PRINTVT(VT_I4)
				_PRINTVT(VT_R4)
				_PRINTVT(VT_R8)
				_PRINTVT(VT_CY)
				_PRINTVT(VT_DATE)
				_PRINTVT(VT_BSTR)
				_PRINTVT(VT_DISPATCH)
				_PRINTVT(VT_ERROR)
				_PRINTVT(VT_BOOL)
				_PRINTVT(VT_VARIANT)
				_PRINTVT(VT_UNKNOWN)
				_PRINTVT(VT_DECIMAL)
				_PRINTVT(VT_I1)
				_PRINTVT(VT_UI1)
				_PRINTVT(VT_UI2)
				_PRINTVT(VT_UI4)
				_PRINTVT(VT_I8)
				_PRINTVT(VT_UI8)
				_PRINTVT(VT_INT)
				_PRINTVT(VT_UINT)
				_PRINTVT(VT_VOID)
				_PRINTVT(VT_HRESULT)
				_PRINTVT(VT_PTR)
				_PRINTVT(VT_SAFEARRAY)
				_PRINTVT(VT_CARRAY)
				_PRINTVT(VT_USERDEFINED)
				_PRINTVT(VT_LPSTR)
				_PRINTVT(VT_LPWSTR)
				_PRINTVT(VT_RECORD)
				_PRINTVT(VT_INT_PTR)
				_PRINTVT(VT_UINT_PTR)
				_PRINTVT(VT_FILETIME)
				_PRINTVT(VT_BLOB)
				_PRINTVT(VT_STREAM)
				_PRINTVT(VT_STORAGE)
				_PRINTVT(VT_STREAMED_OBJECT)
				_PRINTVT(VT_STORED_OBJECT)
				_PRINTVT(VT_BLOB_OBJECT)
				_PRINTVT(VT_CF)
				_PRINTVT(VT_CLSID)
				_PRINTVT(VT_VERSIONED_STREAM)
				;
				#undef _PRINTVT
			}

			if (value)
			{
				if ((propVar.vt & VT_VECTOR)   ||
					(propVar.vt & VT_ARRAY)    ||
					(propVar.vt & VT_BYREF)    ||
				    (propVar.vt & VT_RESERVED))
				{
					// ...
				}
				else
				{
					auto & guid = *propVar.puuid;

					switch (propVar.vt & VT_TYPEMASK)
					{
					case VT_LPSTR:
						printf("\n     %s", propVar.pszVal);
						break;
					case VT_LPWSTR:
						wprintf(L"\n     %s", propVar.pwszVal);
						break;
					case VT_UI4:
						wprintf(L"\n     %08X (%i)", propVar.ulVal, propVar.ulVal);
						break;
					case VT_BOOL:
						printf("\n     b = \"%s\"", _BOOLYESNO(propVar.boolVal));
						break;
					case VT_CLSID:			
						wprintf(L"\n     %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
							guid.Data1, guid.Data2, guid.Data3,
							guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
							guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
						break;
					case VT_BLOB:
						wstring wstr = L"";
						for (uint32 i=0; i<propVar.blob.cbSize; i++)
						{
							wchar_t wstr_val[32];
							swprintf_s(wstr_val, L"%02X", propVar.blob.pBlobData[i]);
							wstr.append(wstr_val);

							if (!((i+1)%4))
								swprintf_s(wstr_val, L";  ", propVar.blob.pBlobData[i]);
							else
								swprintf_s(wstr_val, L",", propVar.blob.pBlobData[i]);
							wstr.append(wstr_val);

							if (i != (propVar.blob.cbSize - 1))
							if (!((i+1)%16))
							{
								swprintf_s(wstr_val, L"\n     ");
								wstr.append(wstr_val);
							}
						}
						wprintf(L"\n     %s", wstr.c_str());
						break;
					}
				}
			}
		}

		//>> [DEBUG] Сообщает в консоль сырое свойство устройства (исходник)
		void PrintfDevicePropertyRAW(const wchar_t * ID, PROPERTYKEY pKey, bool value)
		{
			if (pEnumerator == nullptr)
				{ _MBM(ERROR_PointerNone); return; }

			IMMDevice *			pDevice = nullptr;			// temp
			IPropertyStore *	pPropertyStore = nullptr;	// temp

			if (S_OK != pEnumerator->GetDevice(ID, &pDevice))
				{ _MBM(L"Не удалось pEnumerator->GetDevice()"); return; }
			if (S_OK != pDevice->OpenPropertyStore(STGM_READ, &pPropertyStore))
				{ _MBM(L"Не удалось OpenPropertyStore()"); return; }

			PrintfDevicePropertyRAW(pPropertyStore, pKey, value);

			_RELEASE(pDevice);
			_RELEASE(pPropertyStore);
		}

		//>> [DEBUG] Сообщает в консоль сырое свойство устройства (исходник)
		void PrintfDevicePropertyRAW(uint32 N, eSoundDevType type, PROPERTYKEY pKey, bool value)
		{
			if ((N > numRenderDevices  && type == eSDTypeRender) ||
				(N > numCaptureDevices && type == eSDTypeCapture))
			{ _MBM(ERROR_IDLIMIT); return; }

			switch (type)
			{
			case eSDTypeRender:
				PrintfDevicePropertyRAW(propRenderDevice[N].ID.c_str(), pKey, value);
				break;
			case eSDTypeCapture:
				PrintfDevicePropertyRAW(propCaptureDevice[N].ID.c_str(), pKey, value);
				break;
			default:
				_MBM(L"Incorrect eSoundDevType [ReadDeviceProprety]");
				return;
			}
		}
	};
}

#endif // _SOUNDIMM_H
