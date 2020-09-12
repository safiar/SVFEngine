// ----------------------------------------------------------------------- //
//
// MODULE  : SoundDevice.h
//
// PURPOSE : Прототип класса управления звуковыми устройствами
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _SOUNDDEVICE_H
#define _SOUNDDEVICE_H

#include "WAVE.h"

namespace SAVFGAME
{
	enum eSoundDevType
	{
		eSDTypeUnknown = 0, // unknown

		eSDTypeRender  = 1 << 0, // identify render audio device
		eSDTypeCapture = 1 << 1, // identify capture audio device

		eSDTypeBOTH = eSDTypeRender | eSDTypeCapture // render & capture audio devices
	};
	enum eSoundDevState // Статус состояния звукового устройства
	{
		eSoundDevUnknown,			// информация не получена

		eSoundDevActive,			// DEVICE_STATE_ACTIVE
		eSoundDevDisabled,			// DEVICE_STATE_DISABLED
		eSoundDevNotPresent,		// DEVICE_STATE_NOTPRESENT
		eSoundDevUnlugged,			// DEVICE_STATE_UNPLUGGED

		eSoundDevRemoved,			// устройство удалено из системы

		eSoundDevEnumMax
	};
	enum eSoundDevForm // Физическая форма устройства (EndpointFormFactor)
	{
		eSDFormUnknown,						

		eSDFormRemoteNetwork,
		eSDFormSpeakers,
		eSDFormLineLevel,
		eSDFormHeadphones,
		eSDFormMicrophone,
		eSDFormHeadset,						// Headphones with mic
		eSDFormHandset,						// As mobile telephone
		eSDFormUnknownDigitalPassthrough,
		eSDFormSPDIF,						// Sony/Philips Digital Interface
		eSDFormDigitalAudioDisplayDevice,	// output display monitoring

		eSDFormEnumMax
	};
	enum eSoundDevRange // Физическое расположение воспроизводимых каналов в пространстве
	{
		eSDRangeDirectOut = 0, // default unknown state

		// MONO, STEREO, QUAD, SURROUND, 5.1, 7.1

		eSDRangeFrontLeft			= 1 << 0,
		eSDRangeFrontRight			= 1 << 1,
		eSDRangeFrontCenter			= 1 << 2,
		eSDRangeLowFrequency		= 1 << 3,
		eSDRangeBackLeft			= 1 << 4,
		eSDRangeBackRight			= 1 << 5,
		eSDRangeFrontLeftOfCenter	= 1 << 6,
		eSDRangeFrontRightOfCenter	= 1 << 7,
		eSDRangeBackCenter			= 1 << 8,

		// 5.1 surround, 7.1 surround

		eSDRangeSideLeft			= 1 << 9,
		eSDRangeSideRight			= 1 << 10,

		// TOP

		eSDRangeTopCenter			= 1 << 11,
		eSDRangeTopFrontLeft		= 1 << 12,
		eSDRangeTopFrontCenter		= 1 << 13,
		eSDRangeTopFrontRight		= 1 << 14,
		eSDRangeTopBackLeft			= 1 << 15,
		eSDRangeTopBackCenter		= 1 << 16,
		eSDRangeTopBackRight		= 1 << 17,

		// MONO			[     C     ] Front
		//
		// STEREO		[  L  o  R  ] Front
		//
		// QUAD			[  L     R  ] Front
		// QUAD			[     o     ] Front
		// QUAD			[  L     R  ] Back
		//
		// SURROUND		[     C     ] Front
		// SURROUND		[  L  o  R  ] Front
		// SURROUND		[     C     ] Back
		//
		// 5.1			[ L F C   R ] Front
		// 5.1			[     o     ] Front
		// 5.1			[ L       R ] Back
		//
		// 7.1			[ L F C   R ] Front
		// 7.1			[ L   o   R ] FrontOfCenter
		// 7.1			[   L   R   ] Back

		eSDRange_MONO				= eSDRangeFrontCenter,
		eSDRange_STEREO				= eSDRangeFrontLeft | eSDRangeFrontRight,
		eSDRange_QUAD				= eSDRangeFrontLeft | eSDRangeFrontRight | eSDRangeBackLeft  | eSDRangeBackRight,
		eSDRange_SURROUND			= eSDRangeFrontLeft | eSDRangeFrontRight | eSDRangeFrontCenter | eSDRangeBackCenter,
		eSDRange_5p1				= eSDRangeFrontLeft | eSDRangeFrontRight | eSDRangeFrontCenter |
										eSDRangeLowFrequency | eSDRangeBackLeft | eSDRangeBackRight,
		eSDRange_5p1_SURROUND		= eSDRangeFrontLeft | eSDRangeFrontRight | eSDRangeFrontCenter |
										eSDRangeLowFrequency | eSDRangeSideLeft | eSDRangeSideRight,
		eSDRange_7p1				= eSDRange_5p1 |
									  eSDRangeFrontLeftOfCenter | eSDRangeFrontRightOfCenter,
		eSDRange_7p1_SURROUND		= eSDRange_5p1_SURROUND |
									  eSDRangeBackLeft | eSDRangeBackRight,

		eSDRange_5p1_BACK			= eSDRange_5p1,
		eSDRange_7p1_WIDE			= eSDRange_7p1,

		// DVD Speaker (музыкальная стерео система)

		eSDRange_DVDS_GROUND_FRONT_LEFT   = eSDRangeFrontLeft,
		eSDRange_DVDS_GROUND_FRONT_CENTER = eSDRangeFrontCenter,
		eSDRange_DVDS_GROUND_FRONT_RIGHT  = eSDRangeFrontRight,
		eSDRange_DVDS_GROUND_REAR_LEFT    = eSDRangeBackLeft,
		eSDRange_DVDS_GROUND_REAR_RIGHT   = eSDRangeBackRight,
		eSDRange_DVDS_TOP_MIDDLE          = eSDRangeTopCenter,
		eSDRange_DVDS_SUPER_WOOFER        = eSDRangeLowFrequency
	};
	enum eSoundDevProp
	{
		eSDP_NONE = 0,

		eSDPFriendlyName    = 1 << 0,  // wstring : PKEY_Device_FriendlyName
		eSDPFriendlyNameI   = 1 << 1,  // wstring : PKEY_DeviceInterface_FriendlyName
		eSDPInfoDescription = 1 << 2,  // wstring : PKEY_Device_DeviceDesc
		eSDPEndAssociation  = 1 << 3,  // wstring : PKEY_AudioEndpoint_Association
		eSDPPageProvider    = 1 << 4,  // wstring : PKEY_AudioEndpoint_ControlPanelPageProvider
		eSDPguid            = 1 << 5,  // wstring : PKEY_AudioEndpoint_GUID
		eSDPJackSubType     = 1 << 6,  // wstring : PKEY_AudioEndpoint_JackSubType

		eSDPShareModeFormat = 1 << 7,  // mode format : PKEY_AudioEngine_DeviceFormat  [чаще у active / disabled]
		eSDPDefaultFormat   = 1 << 8,  // mode format : PKEY_AudioEngine_OEMFormat     [чаще у not present / unplugged]

		eSDPSysFXState		= 1 << 9,  // value : PKEY_AudioEndpoint_Disable_SysFx
		eSDPFormFactor		= 1 << 10, // value : PKEY_AudioEndpoint_FormFactor
		eSDPRangeFull       = 1 << 11, // value : PKEY_AudioEndpoint_FullRangeSpeakers
		eSDPRangePhys       = 1 << 12, // value : PKEY_AudioEndpoint_PhysicalSpeakers
		eSDPEventDrivenMode = 1 << 13, // value : PKEY_AudioEndpoint_Supports_EventDriven_Mode

		// Additional properties

		eSDPEnumeratorName  = 1 << 20, // wstring : PKEY_Device_EnumeratorName
		eSDPIconPath        = 1 << 21, // wstring : PKEY_DeviceClass_IconPath

		// NOT PKEY props

		eSDPID				= 1 << 29, // ! не PKEY_ !  pDevice->GetId(&devID);
		eSDPState			= 1 << 30, // ! не PKEY_ !  pDevice->GetState(&state);

		// Skipped properties (we can mem them, but won't do it)

		eSDP_SKIPPED		= 1 << 31, // PKEY_Device_BaseContainerId
									   // PKEY_Device_ContainerId
									   // PKEY_Device_EXTRA_*****
									   // ...

		eSDP_ALL_Strings =
			eSDPFriendlyName | eSDPFriendlyNameI | eSDPInfoDescription | eSDPEndAssociation |	// descriptions
			eSDPPageProvider | eSDPguid | eSDPJackSubType |										// UIDs
			eSDPEnumeratorName, // Additional

		eSDP_ALL_Formats =
			eSDPShareModeFormat | eSDPDefaultFormat,

		eSDP_ALL_Values =
			eSDPSysFXState | eSDPFormFactor | eSDPRangeFull | eSDPRangePhys | eSDPEventDrivenMode,

		eSDP_ALL_Main =
			eSDPID | eSDPState,

		eSDP_ALL = 0xFFFFFFFF
	};
	enum eSoundDevNotification
	{
		eSDNotifyNone,

		eSDNotifyDeviceStateChanged,
		eSDNotifyDeviceAdded,
		eSDNotifyDeviceRemoved,
		eSDNotifyDefaultDeviceChanged,
		eSDNotifyPropertyValueChanged,

		eSDNotifyEnumMax
	};

	struct SoundDeviceProp
	{
		SoundDeviceProp() { Reset(); }

		std::wstring	ID;				// [_!!_IMPORTANT_!!_] unique string ID
		std::wstring	GUID;			// global device identifier
		std::wstring	name;			// friendly name
		std::wstring	iname;			// interface's friendly name
		std::wstring	info;			// text info description
		std::wstring	assoc;			// GUID associates a kernel-streaming (KS) pin category with an audio endpoint device
		std::wstring	pageProv;		// CLSID of the registered provider of the device-properties extension
		std::wstring	jackSType;		// contains an output category GUID for an audio endpoint device
		std::wstring	enumarName;		// enumerator name
		std::wstring	iconPath;		// ../mmres.dll
		SNDDESC			modeShared;		// shared mode format desc.
		SNDDESC			modeDefault;	// default format desc.
		eSoundDevState	state;			// ACTIVE / DISABLED / NOTPRESENT / UNPLUGGED or REMOVED FROM SYSTEM
		eSoundDevForm	formFactor;		// indicates physical attributes (RemoteNetworkDevice, Speakers, LineLevel, Headphones, ...)
		eSoundDevRange	rangeFull;		// [RENDER DEVICE] channel-configuration mask for the full-range speakers
		eSoundDevRange	rangePhys;		// [RENDER DEVICE] mask indicates the physical configuration of a set of speakers
		bool			bEvDrvMode;		// indicates whether the endpoint supports the event-driven mode
		bool			bSysFX;			// specifies whether system effects are enabled in the shared-mode stream

		void Reset()
		{
			ID.clear();
			GUID.clear();
			name.clear();
			iname.clear();
			info.clear();
			assoc.clear();
			pageProv.clear();
			jackSType.clear();

			modeShared.Reset();
			modeDefault.Reset();

			state      = eSoundDevUnknown;
			formFactor = eSDFormUnknown;
			rangeFull  = eSDRangeDirectOut;
			rangePhys  = eSDRangeDirectOut;
			bEvDrvMode = false;
			bSysFX     = false;
		}
	};

	class CSoundDeviceNotifyCatcher
	{
	public:
		CSoundDeviceNotifyCatcher(const CSoundDeviceNotifyCatcher& src)				= delete;
		CSoundDeviceNotifyCatcher(CSoundDeviceNotifyCatcher&& src)					= delete;
		CSoundDeviceNotifyCatcher& operator=(const CSoundDeviceNotifyCatcher& src)	= delete;
		CSoundDeviceNotifyCatcher& operator=(CSoundDeviceNotifyCatcher&& src)		= delete;

		CSoundDeviceNotifyCatcher(){};
		virtual ~CSoundDeviceNotifyCatcher(){};

#ifdef WINDOWS_IMM_WASAPI
		friend class CMMNotificationClient;
#endif

	private:
		//>> Обработка событий устройств
		virtual void IncomingNotification(eSoundDevNotification message, void * pData) = 0;
	};

	class CSoundDevice : public CSoundDeviceNotifyCatcher
	{
		// ЗАМЕТКА
		// Не путать numRenderDevices / numCaptureDevices с количеством в реальной коллекции у системы
		// При начальном запуске они соответствуют количеству в коллекции у системы, но будут меняться по-своему
		// Даже есть устройтво было удалено и в коллекции у системы его больше нет, у нас оно останется с пометкой eSoundDevRemoved
		// 

	protected:
		vector<SoundDeviceProp>	propRenderDevice;   // свойства устройств вывода
		vector<SoundDeviceProp>	propCaptureDevice;  // свойства устройств ввода
		unsigned int numRenderDevices  {0};         // количество устройств вывода в <propRenderDevice>
		unsigned int numCaptureDevices {0};         // количество устройств ввода в <propCaptureDevice>
		unsigned int numRenderDefault  {(unsigned int) MISSING};   // номер устройства вывода используемого по-умолчанию
		unsigned int numCaptureDefault {(unsigned int) MISSING};   // номер устройства ввода используемого по-умолчанию

	public:
		CSoundDevice(const CSoundDevice& src)				= delete;
		CSoundDevice(CSoundDevice&& src)					= delete;
		CSoundDevice& operator=(const CSoundDevice& src)	= delete;
		CSoundDevice& operator=(CSoundDevice&& src)			= delete;

		CSoundDevice(){};
		virtual ~CSoundDevice() override {};

	protected:
		//>> Возвращает <true> и порядковый номер с типом в случае успеха
		bool ReadDeviceNumber(const wchar_t * ID, uint32 & N, eSoundDevType & type)
		{
			bool ret = false;
			N    = MISSING;
			type = eSDTypeUnknown;

			for (uint32 i = 0; i < numRenderDevices; i++)
				if (!propRenderDevice[i].ID.compare(ID))
				{
					ret  = true;
					type = eSDTypeRender;
					N    = i;
					break;
				}

			if (ret) return ret;

			for (uint32 i = 0; i < numCaptureDevices; i++)
				if (!propCaptureDevice[i].ID.compare(ID))
				{
					ret  = true;
					type = eSDTypeCapture;
					N    = i;
					break;
				}

			return ret;
		}

	};
}

#endif // _SOUNDDEVICE_H