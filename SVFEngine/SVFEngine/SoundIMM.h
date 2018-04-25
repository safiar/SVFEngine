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

// TODO: резервирование мест и добавление в любую зарезервированную позицию 
// TODO: спрятать вектора pRenderClient в оболочку
// TODO: конвертация, муксинг и др. исходников под конкретное аудиоустройство
// TODO: слежение за состоянием аудиоустройств (доделать класс CMMNotificationClient)
// TODO: всё по микрофону
// TODO: поддержка других форматов, помимо .wav
// TODO: имитация 3d звука из определённой точки сцены относительно игрока

#include <MMDeviceAPI.h>       // Multimedia Device (MMDevice) API    API to enumerate the audio endpoint devices in the system
#include <AudioClient.h>       // Windows Audio Session API (WASAPI)  API to create and manage audio streams to and from audio endpoint devices
#include <AudioPolicy.h>       // Windows Audio Session API (WASAPI)  .
//#include <DeviceTopology.h>  // DeviceTopology API      API to directly access the topological features inside hardware devices in audio adapters
//#include <EndpointVolume.h>  // EndpointVolume API      API to directly access the volume controls on audio endpoint devices by applications

#include "Sound.h"

#define REFTIMES_PER_SEC	  10000000	// CSoundWASAPI
#define REFTIMES_PER_MILLISEC 10000		// CSoundWASAPI

#define USEDUMMYIFNONE true

namespace SAVFGAME
{
	class CSoundWASAPI;
	typedef void (CSoundWASAPI::*pSoundFunc)(uint32 id, bool loop);

	enum SOUNDERRORCODES
	{
		SNDERR_StreamNotSupported = 0xBAD001, // ERROR_StreamANotSup (формат не поддерживается устройством)
	};

	class CMMNotificationClient : public IMMNotificationClient	// TODO : дореализовать
	{															// https://msdn.microsoft.com/ru-ru/library/windows/desktop/dd370810(v=vs.85).aspx
		LONG _cRef;												// stream routing implementation
		IMMDeviceEnumerator * _pEnumerator;						// https://msdn.microsoft.com/ru-ru/library/windows/desktop/dd756610(v=vs.85).aspx
	public:
		CMMNotificationClient() : _cRef(1), _pEnumerator(nullptr) { };
		~CMMNotificationClient() { _RELEASE(_pEnumerator); };

		// IUnknown method
		ULONG STDMETHODCALLTYPE AddRef()
		{
			return InterlockedIncrement(&_cRef);
		}
		// IUnknown method
		ULONG STDMETHODCALLTYPE Release()
		{
			ULONG ulRef = InterlockedDecrement(&_cRef);
			if (0 == ulRef) { delete this; }
			return ulRef;
		}
		// IUnknown method
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface)
		{
			if (IID_IUnknown == riid)
			{
				AddRef();
				*ppvInterface = (IUnknown*)this;
			}
			else if (__uuidof(IMMNotificationClient) == riid)
			{
				AddRef();
				*ppvInterface = (IMMNotificationClient*)this;
			}
			else
			{
				*ppvInterface = NULL;
				return E_NOINTERFACE;
			}
			return S_OK;
		}

		// Called when the device state of an audio endpoint device changes
		HRESULT STDMETHODCALLTYPE OnDeviceStateChanged	( LPCWSTR pwstrDeviceId, DWORD dwNewState ) { return S_OK; };
		// Called when the user adds an audio endpoint device to the system
		HRESULT STDMETHODCALLTYPE OnDeviceAdded			( LPCWSTR pwstrDeviceId ) { return S_OK; };
		// Called when the user removes an audio endpoint device from the system
		HRESULT STDMETHODCALLTYPE OnDeviceRemoved		( LPCWSTR pwstrDeviceId ) { return S_OK; };
		// Called when the user changes the device role of an audio endpoint device
		HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged( EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) { return S_OK; };
		// Called when the value of a property of an audio endpoint device changes
		HRESULT STDMETHODCALLTYPE OnPropertyValueChanged( LPCWSTR pwstrDeviceId, const PROPERTYKEY key) { return S_OK; };
		
	};

	class CSoundIMM : public CSound
	{
	protected:
		IMMDeviceEnumerator *	pEnumerator;		// interface provides methods for enumerating multimedia device resources
		IMMDeviceCollection *	pRenderDevices;		// interface represents a collection of multimedia device resources   [audio endpoint devices]
		IMMDeviceCollection *	pCaptureDevices;	// .
		CMMNotificationClient * pNotify;			// interface provides notifications when an audio endpoint device is added or removed
		IMMDevice *             devRender;			// текущий render device interface
		IMMDevice *             devCapture;			// текущий capture device interface
		IMMEndpoint *			devRenderEnd;		// конечный интерфейс от текущего render device interface
		IMMEndpoint *			devCaptureEnd;		// конечный интерфейс от текущего capture device interface
	//	IPropertyStore *		pPropertyStore;		// Exposes methods for enumerating, getting, and setting property values
		vector<wstring>			idRenderDevice;		// идентификаторы устройств
		vector<wstring>			idCaptureDevice;	// .	
		UINT numRenderDevices;						// количество render-устройств в коллекции
		UINT numCaptureDevices;						// количество capture-устройств в коллекции
		UINT numRenderDefault;						// номер default render-устройства согласно номеру в коллекции
		UINT numCaptureDefault;						// номер default capture-устройства согласно номеру в коллекции
	private:
		bool isInit;
	public:
		CSoundIMM() : CSound(), isInit(false), pEnumerator(nullptr), pRenderDevices(nullptr), pCaptureDevices(nullptr), pNotify(nullptr),
					devRender(nullptr), devCapture(nullptr), devRenderEnd(nullptr), devCaptureEnd(nullptr) {};
		virtual ~CSoundIMM() { Close(); };

		virtual void Close() override
		{
			if (!isInit) return;

			if (pNotify) pEnumerator->UnregisterEndpointNotificationCallback(pNotify);
			_RELEASE(pNotify);
			_RELEASE(pEnumerator);
			_RELEASE(pRenderDevices);
			_RELEASE(pCaptureDevices);
			_RELEASE(devRenderEnd);
			_RELEASE(devCaptureEnd);
			_RELEASE(devRender);
			_RELEASE(devCapture);		
			idRenderDevice.erase(idRenderDevice.begin(),idRenderDevice.end());		//idRenderDevice.clear();
			idCaptureDevice.erase(idCaptureDevice.begin(),idCaptureDevice.end());	//idCaptureDevice.clear();	
			CoUninitialize();		 // Закрываем COM библиотеку
			CSound::Close();

			isInit = false;
		};

		virtual void Init() override
		{
			if (isInit) return;

			CSound::Init();

			IMMDevice* pDevice; // temp
			wchar_t* devID;		// temp

			pNotify = new CMMNotificationClient; // Создаём класс регистрации событий

		//	if (S_OK != CoInitialize(NULL)) // Инициализируем COM библиотеку
		//		_MBM(L"Не удалось CoInitialize() [COM library]");

			if (S_OK != CoInitializeEx(NULL, COINIT_MULTITHREADED)) // Инициализируем COM библиотеку (в режиме доступа другим потокам)
				_MBM(L"Не удалось CoInitializeEx() [COM library]");
			
			const CLSID CLSID_MMDeviceEnumerator = __uuidof( MMDeviceEnumerator);
			const   IID  IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
			if (S_OK != CoCreateInstance(
							CLSID_MMDeviceEnumerator, // The CLSID associated with the data and code that will be used to create the object
							NULL,					  // If non-NULL, pointer to the aggregate object's IUnknown interface (the controlling IUnknown).
							CLSCTX_INPROC,			  // Context in which the code that manages the newly created object will run
							IID_IMMDeviceEnumerator,  // A reference to the identifier of the interface to be used to communicate with the object
							(void**)&pEnumerator      // Return *ppv contains the requested interface pointer. Upon failure, *ppv contains NULL.
						))
				_MBM(L"Не удалось CoCreateInstance() [MMDevice]");

			if (S_OK != pEnumerator->EnumAudioEndpoints(eRender, // Audio data flows from the application to the audio endpoint device, which renders the stream
														DEVICE_STATEMASK_ALL, // DEVICE_STATEMASK  _ALL _ACTIVE _DISABLED _NOTPRESENT _UNPLUGGED
														&pRenderDevices))
				_MBM(L"Не удалось EnumAudioEndpoints() [pRenderDevices]");

			if (S_OK != pEnumerator->EnumAudioEndpoints(eCapture, // Audio data flows from the audio endpoint device that captures the stream, to the application
														DEVICE_STATEMASK_ALL, // Audio data flows:  eRender / eCapture / eAll
														&pCaptureDevices))
				_MBM(L"Не удалось EnumAudioEndpoints() [pCaptureDevices]");

			pRenderDevices->GetCount(&numRenderDevices);	//printf("\n%i audio render devices", numRenderDevices);
			pCaptureDevices->GetCount(&numCaptureDevices);	//printf("\n%i audio capture devices", numCaptureDevices);

			if (S_OK != pEnumerator->RegisterEndpointNotificationCallback(pNotify))
				_MBM(L"Не удалось RegisterEndpointNotificationCallback()");

			// Выясним ID устройств, чтобы позже к ним обращаться

			for(UINT i=0; i<numRenderDevices; i++)
			{
				if (S_OK != pRenderDevices->Item(i,&pDevice))
					_MBM(L"Не удалось Collection->Item()");
				pDevice->GetId(&devID);
				idRenderDevice.push_back(wstring(devID));	//wprintf(L"\nRender device %2i: %s",i,idRenderDevice[i].c_str());			
				CoTaskMemFree(devID);
				_RELEASE(pDevice);
			}

			for(UINT i=0; i<numCaptureDevices; i++)
			{
				if (S_OK != pCaptureDevices->Item(i,&pDevice))
					_MBM(L"Не удалось Collection->Item()");
				pDevice->GetId(&devID);
				idCaptureDevice.push_back(wstring(devID));	//wprintf(L"\nCapture device %2i: %s",i,idCaptureDevice[i].c_str());
				CoTaskMemFree(devID);
				_RELEASE(pDevice);
			}

			// Выясним номер ID текущих устройств по умолчанию

			if (S_OK != pEnumerator->GetDefaultAudioEndpoint(eRender,
															 eConsole,			// eConsole / eMultimedia / eCommunications    см. "Device Roles"
															 &pDevice))
				_MBM(L"Не удалось GetDefaultAudioEndpoint()");

			pDevice->GetId(&devID);
			for(UINT i=0; i<numRenderDevices; i++)
				if(!idRenderDevice[i].compare(devID))
					{numRenderDefault = i; break;};
			CoTaskMemFree(devID);
			_RELEASE(pDevice);		//printf("\nDefault Render Device = %i", numRenderDefault);

			if (S_OK != pEnumerator->GetDefaultAudioEndpoint(eCapture,
															 eCommunications,
															 &pDevice))
				_MBM(L"Не удалось GetDefaultAudioEndpoint()");

			pDevice->GetId(&devID);
			for(UINT i=0; i<numCaptureDevices; i++)
				if(!idCaptureDevice[i].compare(devID))
					{numCaptureDefault = i; break;};
			CoTaskMemFree(devID);
			_RELEASE(pDevice);		//printf("\nDefault Capture Device = %i", numCaptureDefault);
	
			SetCurrentAsDefaultDevices();
			//PrintfDevicesConditions();

			isInit = true;
		}

		//>> Сообщает в консоль состояние устройств
		void PrintfDevicesConditions()
		{
			IMMDevice* pDevice;
			DWORD devState;

			for(UINT i=0; i<numRenderDevices; i++)
			{
				pEnumerator->GetDevice(idRenderDevice[i].c_str(), &pDevice);
				pDevice->GetState(&devState);
				switch(devState)
				{
				case DEVICE_STATE_ACTIVE: printf("\nRenderDev %2i: active",i); break;
				case DEVICE_STATE_DISABLED: printf("\nRenderDev %2i: disabled",i); break;
				case DEVICE_STATE_NOTPRESENT: printf("\nRenderDev %2i: not present",i); break;
				case DEVICE_STATE_UNPLUGGED: printf("\nRenderDev %2i: unplugged",i); break;
				case DEVICE_STATEMASK_ALL:
				default: printf("\nRenderDev %2i: [ERROR] unknown condition",i); break;
				}
				if (i == numRenderDefault) printf(" (DEFAULT DEV)");
				_RELEASE(pDevice);
			}

			for(UINT i=0; i<numCaptureDevices; i++)
			{
				pEnumerator->GetDevice(idCaptureDevice[i].c_str(), &pDevice);
				pDevice->GetState(&devState);
				switch(devState)
				{
				case DEVICE_STATE_ACTIVE: printf("\nCaptureDev %2i: active",i); break;
				case DEVICE_STATE_DISABLED: printf("\nCaptureDev %2i: disabled",i); break;
				case DEVICE_STATE_NOTPRESENT: printf("\nCaptureDev %2i: not present",i); break;
				case DEVICE_STATE_UNPLUGGED: printf("\nCaptureDev %2i: unplugged",i); break;
				case DEVICE_STATEMASK_ALL:
				default: printf("\nCaptureDev %2i: [ERROR] unknown condition",i); break;
				}
				if (i == numCaptureDefault) printf(" (DEFAULT DEV)");
				_RELEASE(pDevice);
			}
		}

		//>> Переназначает на текущие устройства помеченные в системе по умолчанию
		bool SetCurrentAsDefaultDevices()
		{
			if (!pEnumerator) { _MBM(ERROR_InitNone); return false; }

			_RELEASE(devRenderEnd);
			_RELEASE(devCaptureEnd);
			_RELEASE(devRender);
			_RELEASE(devCapture);

			pEnumerator->GetDevice(idRenderDevice[numRenderDefault].c_str(), &devRender);
			pEnumerator->GetDevice(idCaptureDevice[numCaptureDefault].c_str(), &devCapture);

			REFIID IID_IMMEndpoint = __uuidof(IMMEndpoint);

			devRender->QueryInterface(IID_IMMEndpoint, (void**)&devRenderEnd);
			devCapture->QueryInterface(IID_IMMEndpoint, (void**)&devCaptureEnd);

			return true;
		};
	};

	class CSoundWASAPI final : public CSoundIMM
	{												// https://msdn.microsoft.com/ru-ru/library/windows/desktop/dd370800(v=vs.85).aspx Capturing a Stream
	protected:										// https://msdn.microsoft.com/ru-ru/library/windows/desktop/dd316756(v=vs.85).aspx Rendering a Stream
		vector<IAudioClient*>		pRenderClient;	// Enables a client to create and initialize an audio stream
		vector<IAudioRenderClient*> pRenderClientA;	// Enables a client to write output data to a rendering endpoint buffer
		vector<IAudioStreamVolume*> pRenderClientB;	// Enables a client to control and monitor the volume levels for all of the channels
		IAudioClient*				pCaptureClient;
		pSoundFunc const			pPlayFunc;
		pSoundFunc const			pPlayAsyncFunc;
		mutex						m_PlayThread;	// приостанавливает при попытке одновременного вызова PlayThread()
		mutex						m_PlayAsync;	// приостанавливает при попытке одновременного вызова PlayAsync()
	private:
		bool isInit;
	public:
		CSoundWASAPI() : CSoundIMM(), isInit(false), pCaptureClient(nullptr),
			pPlayFunc(&CSoundWASAPI::Play), pPlayAsyncFunc(&CSoundWASAPI::PlayAsync) {};
		~CSoundWASAPI() { Close(); };

		void Close() override final
		{
			if (isInit)
			{
				EraseSounds(0);
				_RELEASE(pCaptureClient);
				CSoundIMM::Close();
				isInit = false;
			}
		}

		void Init() override final
		{
			if (isInit) return;
			CSoundIMM::Init();
			isInit = true;
		}

	protected:
		//>> Создание IAudioClient
		IAudioClient * Init_IAudioClient(uint32 id, uint32 & error_code)
		{
			REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
			IAudioClient * pClient = nullptr;
			error_code = NULL;
			HRESULT hRes;
			wchar_t error[256];

			//	IID_IAudioEndpointVolume		IID_IAudioClient		IID_IDirectSound			IID_IMFTrustedOutput
			//	IID_IAudioMeterInformation		IID_IBaseFilter			IID_IDirectSound8			IID_ISpatialAudioClient
			//	IID_IAudioSessionManager		IID_IDeviceTopology		IID_IDirectSoundCapture		IID_ISpatialAudioMetadataClient
			//	IID_IAudioSessionManager2								IID_IDirectSoundCapture8

			//	->Activate : pActivationParams [in]
			//	Set to NULL to activate an IAudioClient, IAudioEndpointVolume, IAudioMeterInformation, IAudioSessionManager, or IDeviceTopology
			//	interface on an audio endpoint device. When activating an IBaseFilter, IDirectSound, IDirectSound8, IDirectSoundCapture, or
			//	IDirectSoundCapture8 interface on the device, the caller can specify a pointer to a PROPVARIANT structure that contains
			//	stream-initialization information.

			REFIID IID_IAudioClient = __uuidof(IAudioClient);

			if (S_OK != devRender->Activate(IID_IAudioClient, CLSCTX_INPROC, NULL, (void**)&pClient))
			{
				_MBM(L"<Sound WASAPI> Не удалось Activate() pClient");
				return nullptr;
			}

			WAVEFORMATEX *pwfxRender = nullptr;
			WAVEFORMATEX *pwfxRenderClosestMatch = nullptr;
			//pClient->GetMixFormat(&pwfxRender);
			//PrintfWaveFormatEx(*pwfxRender);
			//CoTaskMemFree(pwfxRender);	
			pwfxRender = (WAVEFORMATEX*)snd[id]->snddesc;
			//PrintfWaveFormatEx(*pwfxRender);

			hRes = pClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, pwfxRender, &pwfxRenderClosestMatch);
			if (pwfxRenderClosestMatch) CoTaskMemFree(pwfxRenderClosestMatch);
			if (hRes != S_OK)
			{
				wsprintf(error, L"%s [id=%i] %s", ERROR_StreamANotSup, id, snd[id]->filename.c_str());
				//_MBM(error);
				wprintf_s(L"\n%s", error);
				//printf("\nhRes %x [IsFormatSupported]",(int)hRes);
				_RELEASE(pClient);
				error_code = SNDERR_StreamNotSupported;
				return nullptr;
			}

			hRes = pClient->Initialize(AUDCLNT_SHAREMODE_SHARED, // AUDCLNT_SHAREMODE_SHARED ; AUDCLNT_SHAREMODE_EXCLUSIVE
				NULL, // ? AUDCLNT_STREAMFLAGS_EVENTCALLBACK ? AUDCLNT_STREAMFLAGS_RATEADJUST
				hnsRequestedDuration,
				0,	// Always 0 in shared mode  ;  IAudioClient::GetDevicePeriod()
				pwfxRender,
				NULL); // DEFINE_GUIDNAMED(GUID_NULL)
			if (hRes != S_OK)
			{
				wchar_t error[256];
				//The audio engine will fail creating new streams
				switch (hRes)
				{
				case 0x88890017: 
					swprintf_s(error, L"<Sound WASAPI> Не удалось Initialize() pClient\nERR = %x\n"
						              L"The audio engine fail creating new streams", (int)hRes);
					break;
				default:
					swprintf_s(error, L"<Sound WASAPI> Не удалось Initialize() pClient\nERR = %x", (int)hRes);
					break;
				}
				_MBM(error);
				_RELEASE(pClient);
				return nullptr;
			}
			//AUDCLNT_E_NOT_INITIALIZED
			//AUDCLNT_E_ALREADY_INITIALIZED
			//CO_E_FIRST !!! CoInitializeEx(NULL, COINIT_MULTITHREADED)

			return pClient;
		}
		//>> Создание IAudioRenderClient
		IAudioRenderClient * Init_IAudioRenderClient(IAudioClient * pClient)
		{
			IAudioRenderClient * pRClient = nullptr;
			HRESULT hRes;

			REFIID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

			//	IID_IAudioCaptureClient		IID_IAudioStreamVolume
			//	IID_IAudioClock				IID_IChannelAudioVolume
			//	IID_IAudioRenderClient		IID_IMFTrustedOutput
			//	IID_IAudioSessionControl	IID_ISimpleAudioVolume

			hRes = pClient->GetService(IID_IAudioRenderClient, (void**)&pRClient);
			if (hRes != S_OK)
			{
				_MBM(L"<Sound WASAPI> Не удалось GetService() [pRenderClient]");
				return nullptr;
			}
			return pRClient;
		}
		//>> Создание IAudioStreamVolume
		IAudioStreamVolume * Init_IAudioStreamVolume(IAudioClient * pClient)
		{
			IAudioStreamVolume * pVClient = nullptr;
			HRESULT hRes;

			REFIID IID_IAudioStreamVolume = __uuidof(IAudioStreamVolume);

			//	IID_IAudioCaptureClient		IID_IAudioStreamVolume
			//	IID_IAudioClock				IID_IChannelAudioVolume
			//	IID_IAudioRenderClient		IID_IMFTrustedOutput
			//	IID_IAudioSessionControl	IID_ISimpleAudioVolume

			hRes = pClient->GetService(IID_IAudioStreamVolume, (void**)&pVClient);
			if (hRes != S_OK)
			{
				_MBM(L"<Sound WASAPI> Не удалось GetService() [pVolumeClient]");
				return nullptr;
			}
			return pVClient;
		}
		//>> Ставит пустышку (на случай недоступности файла)
		void OpenSoundDummy()
		{
			snd.emplace_back();
			pRenderClient.emplace_back(nullptr);
			pRenderClientA.emplace_back(nullptr);
			pRenderClientB.emplace_back(nullptr);
		}
		//>> Подготовка звука после загрузки
		uint32 OpenSoundPrepare(uint32 id)
		{
			uint32 error_code;
			uint32 pClientID = (uint32)pRenderClient.size();
			pRenderClient.emplace_back(nullptr);
			pRenderClientA.emplace_back(nullptr);
			pRenderClientB.emplace_back(nullptr);
			IAudioClient       * pClient = nullptr;
			IAudioRenderClient * pRClient = nullptr;
			IAudioStreamVolume * pVClient = nullptr;
			if (pClient = Init_IAudioClient(id, error_code))
			{
				pRClient = Init_IAudioRenderClient(pClient);
				pVClient = Init_IAudioStreamVolume(pClient);
			}
			pRenderClient[pClientID] = pClient;
			pRenderClientA[pClientID] = pRClient;
			pRenderClientB[pClientID] = pVClient;
			return error_code;
		}

	public:
		//>> Загрузчик
		uint32 Load(const wchar_t* gamePath, const wchar_t* filename) override final
		{
			if (!isInit) { _MBM(ERROR_InitNone); return false; }

			uint32 ret = CSound::OpenSoundProc(gamePath, filename);
			if (ret)
			{
				uint32 id = (uint32)(snd.size() - 1);
				uint32 error_code = OpenSoundPrepare(id);
				switch (error_code)
				{
				case SNDERR_StreamNotSupported: // пока что просто удаляю _ TODO: конвертация
					snd[id]->Close();
					snd.Reset(nullptr, id);
					ret=error_code;
					break;
				default: ;
				}
			}
			else if (USEDUMMYIFNONE) OpenSoundDummy();

			return ret; // 0: failed to open, 1: opened, ERROR_CODE: other
		}
		//>> Освобождает память от (ID >= from_id) до конца        ; порядок загруженных треков (ID) сохраняется
		void DeleteSounds(uint32 from_id)
		{
			uint32 i, size = (uint32)snd.size();		if (!size) return;

			if (from_id >= size) { _MBM(L"CSoundF::CloseSounds() -> from_id >= size");  return; }

			if (from_id == 0)
				InterruptSound(MISSING);
			else for (i=from_id; i<size; i++)
				InterruptSound(i);

			for (i=from_id; i<size; i++)		{
				_RELEASE(pRenderClient[i]);
				_RELEASE(pRenderClientA[i]);
				_RELEASE(pRenderClientB[i]);	}

			snd.Close(1,0,from_id);
		}
		//>> Освобождает память от (ID == from_id) до (ID < to_id) ; порядок загруженных треков (ID) сохраняется
		void DeleteSounds(uint32 from_id, uint32 to_id)
		{
			uint32 i, size = (uint32)snd.size();		if (!size) return;

			if (from_id >= size || to_id > size || to_id <= from_id) { _MBM(L"CSoundF::CloseSounds() -> from_id, to_id mismatch");  return; }

			for (i=from_id; i<to_id; i++)
				InterruptSound(i);

			for (i = from_id; i<to_id; i++)		{
				_RELEASE(pRenderClient[i]);
				_RELEASE(pRenderClientA[i]);
				_RELEASE(pRenderClientB[i]);	}

			snd.Close(1,0,from_id,to_id);
		}
		//>> Удаляет начиная от (ID >= from_id) до конца           ; порядок загруженных треков (ID) реконструируется
		void EraseSounds(uint32 from_id)
		{
			uint32 i, size = (uint32)snd.size();		if (!size) return;

			if (from_id >= size) { _MBM(L"CSoundF::DeleteSounds() -> from_id >= size");  return; }

			if (from_id == 0)
				InterruptSound(MISSING);
			else for (i=from_id; i<size; i++)
				InterruptSound(i);

			for (i=from_id; i<size; i++)		{
				_RELEASE(pRenderClient[i]);
				_RELEASE(pRenderClientA[i]);
				_RELEASE(pRenderClientB[i]);	}

			snd.Close(1,1,from_id);
			pRenderClient.erase(pRenderClient.begin() + from_id, pRenderClient.end());
			pRenderClientA.erase(pRenderClientA.begin() + from_id, pRenderClientA.end());
			pRenderClientB.erase(pRenderClientB.begin() + from_id, pRenderClientB.end());
		}
		//>> Удаляет начиная от (ID == from_id) до (ID < to_id)    ; порядок загруженных треков (ID) реконструируется
		void EraseSounds(uint32 from_id, uint32 to_id)
		{
			uint32 i, size = (uint32)snd.size();		if (!size) return;

			if (from_id >= size || to_id > size || to_id <= from_id) { _MBM(L"CSoundF::DeleteSounds() -> from_id, to_id mismatch");  return; }

			for (i=from_id; i<to_id; i++)
				InterruptSound(i);

			for (i=from_id; i<to_id; i++)		{
				_RELEASE(pRenderClient[i]);
				_RELEASE(pRenderClientA[i]);
				_RELEASE(pRenderClientB[i]);	}

			snd.Close(1,1,from_id,to_id);
			pRenderClient.erase(pRenderClient.begin() + from_id, pRenderClient.begin() + to_id);
			pRenderClientA.erase(pRenderClientA.begin() + from_id, pRenderClientA.begin() + to_id);
			pRenderClientB.erase(pRenderClientB.begin() + from_id, pRenderClientB.begin() + to_id);
		}

	protected:
		//>> Воспроизведение
		void Play_proc(uint32 id, bool loop, IAudioClient * pClient, IAudioRenderClient * pRClient, IAudioStreamVolume * pVClient)
		{		
			if (pClient  == nullptr) return; // Неподдерживаемый формат
			if (pRClient == nullptr) return; // Неподдерживаемый формат
			if (pVClient == nullptr) return; // Неподдерживаемый формат

			HRESULT hRes;

			auto sound_ptr = snd[id];	// забиваем за собой shared_ptr, для реализации мягкого прерывания на будущее
			auto & sound = *sound_ptr;
			float vol;
			float mem_volume_multiplier = sound.volume_multiplier;
			float mem_master_volume     = master_volume;

			const uint32 frames2bytes    = sound.snddesc->nChannels * (sound.snddesc->wBitsPerSample / 8);
			const uint32 snd_buffer_size = sound.file.count;
			const byte*  snd_buffer      = sound.file.buf;

			uint32 numBufferFrames = 0;
			uint32 numBufferBytes  = 0;

			pClient->GetBufferSize(&numBufferFrames);
			numBufferBytes = numBufferFrames * frames2bytes;

			//printf("\nbuffer: %8i frames %8i [%8x] bytes",numBufferFrames,numBufferBytes,numBufferBytes);

			if (!numBufferBytes) { _MBM(ERROR_BUFLIMIT); return; }

			BYTE *pData;
			uint32 snd_initsize, snd_curpos, snd_available;
			DWORD flags = 0;

			if (hRes = pRClient->GetBuffer(numBufferFrames, &pData))
			{
				_MBM(L"Не удалось GetBuffer() [IAudioRenderClient]");
				//printf("\nhRes %x [GetBuffer]",(int)hRes); //AUDCLNT_E_BUFFER_TOO_LARGE
				return;
			}
			snd_initsize = (snd_buffer_size > numBufferBytes) ? numBufferBytes : snd_buffer_size;
			memcpy(pData, snd_buffer, snd_initsize);
			pRClient->ReleaseBuffer(snd_initsize / frames2bytes, flags); // enum _AUDCLNT_BUFFERFLAGS

			snd_curpos    = snd_initsize;
			snd_available = snd_buffer_size - snd_curpos;

			REFERENCE_TIME hnsActualDuration = static_cast<REFERENCE_TIME>                 (
				(double)REFTIMES_PER_SEC * numBufferFrames / sound.snddesc->nSamplesPerSec );

			DWORD halfsleep = (DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2);

			uint32 numFramesPadding;
			uint32 numFramesAvailable;
			char* pBufPos = (char*)snd_buffer + snd_initsize;

			vol = max(0.f, mem_master_volume * mem_volume_multiplier);
			vol = min(1.f, vol);

			pVClient->SetChannelVolume(0, vol);	// TODO: не только стерео
			pVClient->SetChannelVolume(1, vol);	//

			bool interrupt = false;

			if (S_OK != pClient->Start())
				_MBM(L"Не удалось Start() [IAudioClient]");

			while (flags != AUDCLNT_BUFFERFLAGS_SILENT)
			{
				if (_RLX_LD(sound.interrupt))
					{ interrupt = true; break; }

				if (mem_volume_multiplier != sound.volume_multiplier ||
					mem_master_volume != master_volume)
				{
					mem_master_volume = master_volume;
					mem_volume_multiplier = sound.volume_multiplier;
					vol = max(0.f, mem_master_volume * mem_volume_multiplier);
					vol = min(1.f, vol);
					pVClient->SetChannelVolume(0, vol);
					pVClient->SetChannelVolume(1, vol);
				}

				if (!snd_available && loop) // новый loop-виток
				{		
					pBufPos = (char*)snd_buffer;
					snd_curpos = 0;
					snd_available = snd_buffer_size;
				}
				else
					Sleep(halfsleep); // жду, пока половина не воспроизведётся

				pClient->GetCurrentPadding(&numFramesPadding);
				numFramesAvailable = numBufferFrames - numFramesPadding;

				if (S_OK != pRClient->GetBuffer(numFramesAvailable, &pData))
				{
					_MBM(L"Ошибка воспроизведения Play() [Sound]");
					pClient->Stop();
					pClient->Reset();
					return;
				}
				numBufferBytes = numFramesAvailable * frames2bytes;
				snd_initsize = (snd_available > numBufferBytes) ? numBufferBytes : snd_available;
				memcpy(pData, pBufPos, snd_initsize);
				pRClient->ReleaseBuffer(snd_initsize / frames2bytes, flags);

				pBufPos       += snd_initsize;
				snd_curpos    += snd_initsize;
				snd_available  = snd_buffer_size - snd_curpos;

				if (!snd_available && !loop) flags = AUDCLNT_BUFFERFLAGS_SILENT;
			}
			if (!interrupt) _SLEEP(halfsleep);
			pClient->Stop();
			pClient->Reset();
		}
		//>> Воспроизведение (one at a time)
		void Play(uint32 id, bool loop)
		{
			ulock guard (snd[id]->m_Play, std::try_to_lock);
			if (guard.owns_lock())
			{
				Play_proc(id, loop,
					pRenderClient[id],
					pRenderClientA[id],
					pRenderClientB[id]);
			}
		}
		//>> Воспроизведение (many at a time)
		void PlayAsync(uint32 id, bool loop)
		{
			uint32 error_code;

			IAudioClient       * pClient  = nullptr;
			IAudioRenderClient * pRClient = nullptr;
			IAudioStreamVolume * pVClient = nullptr;

			{
				glock others_will_wait (m_PlayAsync);

				pClient  = Init_IAudioClient(id, error_code);
				pRClient = (pClient) ? Init_IAudioRenderClient(pClient) : nullptr;
				pVClient = (pClient) ? Init_IAudioStreamVolume(pClient) : nullptr;
			}

			Play_proc(id, loop, pClient, pRClient, pVClient);

			_RELEASE(pClient);
			_RELEASE(pRClient);
			_RELEASE(pVClient);
		}

	public:
		//>> Воспроизведение
		void PlayThread(uint32 id, bool async, bool loop, float volume_multiplier) override final
		{
			glock others_will_wait (m_PlayThread);

			if (!isInit) { _MBM(ERROR_InitNone); return; }

			if (!CheckSoundID(id)) return;     // wrong id
			if ( snd[id] == nullptr ||
				!snd[id]->CheckFile()) return; // missing sound

			if (volume_multiplier >= 0)
				snd[id]->volume_multiplier = volume_multiplier;

			size_t T = playpool.GetTasks();
			size_t W = playpool.GetWorkers();

			if (T >= W && W < SOUND_THREADS_MAX) playpool.WorkersAdd(1);

			if (async) snd[id]->ticket = playpool ( pPlayAsyncFunc, this, id, loop );
			else       snd[id]->ticket = playpool ( pPlayFunc,      this, id, loop );

		/*	bool thread_added = false;
			for (auto & cur : playthread.v) // try to register thread
			{
				if (async) thread_added = ((*cur.data)( (int32)id, pPlayAsyncFunc, this, id, loop ) == TC_SUCCESS);
				else       thread_added = ((*cur.data)( (int32)id, pPlayFunc,      this, id, loop ) == TC_SUCCESS);
				if (thread_added) break;
			}

			if (!thread_added) // no more room in the hell
				_MBM(ERROR_THREADNUMMAX); //*/
		}
		//>> Воспроизведение
		void PlayThread(uint32 id, bool async, bool loop) override final
		{
			PlayThread(id, async, loop, -1.f);
		}
	};
}

#endif // _SOUNDIMM_H
