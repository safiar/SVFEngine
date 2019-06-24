// ----------------------------------------------------------------------- //
//
// MODULE  : SoundIMMN.h
//
// PURPOSE : Содержит реализацию интерфейса IMMNotificationClient
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _SOUNDIMMN_H
#define _SOUNDIMMN_H

#include "SoundDevice.h"

#include <initguid.h>          // Property keys init mechanism
#include <MMDeviceAPI.h>       // Multimedia Device (MMDevice) API    API to enumerate the audio endpoint devices in the system
#include <AudioClient.h>       // Windows Audio Session API (WASAPI)  API to create and manage audio streams to and from audio endpoint devices
#include <AudioPolicy.h>       // Windows Audio Session API (WASAPI)  .
//#include <DeviceTopology.h>  // DeviceTopology API      API to directly access the topological features inside hardware devices in audio adapters
//#include <EndpointVolume.h>  // EndpointVolume API      API to directly access the volume controls on audio endpoint devices by applications

//#pragma comment(lib, "uuid.lib")
//#include <setupapi.h>
//#include <devpkey.h>
//#include <Functiondiscoverykeys_devpkey.h> // Defines property keys for the Plug and Play Device Property API.
#include <Functiondiscoverykeys.h>
//#include <propkey.h>
//#include <mmdeviceapi.h>
#include <Audioenginebaseapo.h>

// некоторые доп. ключи

#pragma region <devpkey.h> 

DEFINE_PROPERTYKEY(PKEY_Device_ProblemStatus,           0x4340a6c5, 0x93fa, 0x4706, 0x97, 0x2c, 0x7b, 0x64, 0x80, 0x08, 0xa5, 0xa7, 12);     // DEVPROP_TYPE_NTSTATUS
DEFINE_PROPERTYKEY(PKEY_Device_InLocalMachineContainer, 0x8c7ed206, 0x3f8a, 0x4827, 0xb3, 0xab, 0xae, 0x9e, 0x1f, 0xae, 0xfc, 0x6c, 4);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_Device_ShowInUninstallUI,       0x80d81ea6, 0x7473, 0x4b0c, 0x82, 0x16, 0xef, 0xc1, 0x1a, 0x2c, 0x4c, 0x8b, 8);     // DEVPROP_TYPE_BOOLEAN

DEFINE_PROPERTYKEY(PKEY_Device_IsPresent,                0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 5);     // DEVPROP_TYPE_BOOL
DEFINE_PROPERTYKEY(PKEY_Device_HasProblem,               0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 6);     // DEVPROP_TYPE_BOOL
DEFINE_PROPERTYKEY(PKEY_Device_ConfigurationId,          0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 7);     // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_Device_ReportedDeviceIdsHash,    0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 8);     // DEVPROP_TYPE_UINT32
DEFINE_PROPERTYKEY(PKEY_Device_PhysicalDeviceLocation,   0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 9);     // DEVPROP_TYPE_BINARY
DEFINE_PROPERTYKEY(PKEY_Device_BiosDeviceName,           0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 10);    // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_Device_DriverProblemDesc,        0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 11);    // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_Device_DebuggerSafe,             0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 12);    // DEVPROP_TYPE_UINT32
DEFINE_PROPERTYKEY(PKEY_Device_PostInstallInProgress,    0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 13);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_Device_Stack,                    0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 14);    // DEVPROP_TYPE_STRING_LIST
DEFINE_PROPERTYKEY(PKEY_Device_ExtendedConfigurationIds, 0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 15);  // DEVPROP_TYPE_STRING_LIST
DEFINE_PROPERTYKEY(PKEY_Device_IsRebootRequired,         0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 16);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_Device_FirmwareDate,             0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 17);    // DEVPROP_TYPE_FILETIME
DEFINE_PROPERTYKEY(PKEY_Device_FirmwareVersion,          0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 18);    // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_Device_FirmwareRevision,         0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 19);    // DEVPROP_TYPE_STRING

DEFINE_PROPERTYKEY(PKEY_Device_SessionId,        0x83da6326, 0x97a6, 0x4088, 0x94, 0x53, 0xa1, 0x92, 0x3f, 0x57, 0x3b, 0x29, 6);     // DEVPROP_TYPE_UINT32
DEFINE_PROPERTYKEY(PKEY_Device_InstallDate,      0x83da6326, 0x97a6, 0x4088, 0x94, 0x53, 0xa1, 0x92, 0x3f, 0x57, 0x3b, 0x29, 100);   // DEVPROP_TYPE_FILETIME
DEFINE_PROPERTYKEY(PKEY_Device_FirstInstallDate, 0x83da6326, 0x97a6, 0x4088, 0x94, 0x53, 0xa1, 0x92, 0x3f, 0x57, 0x3b, 0x29, 101);   // DEVPROP_TYPE_FILETIME
DEFINE_PROPERTYKEY(PKEY_Device_LastArrivalDate,  0x83da6326, 0x97a6, 0x4088, 0x94, 0x53, 0xa1, 0x92, 0x3f, 0x57, 0x3b, 0x29, 102);   // DEVPROP_TYPE_FILETIME
DEFINE_PROPERTYKEY(PKEY_Device_LastRemovalDate,  0x83da6326, 0x97a6, 0x4088, 0x94, 0x53, 0xa1, 0x92, 0x3f, 0x57, 0x3b, 0x29, 103);   // DEVPROP_TYPE_FILETIME

DEFINE_PROPERTYKEY(PKEY_DeviceClass_DHPRebalanceOptOut,  0xd14d3ef3, 0x66cf, 0x4ba2, 0x9d, 0x38, 0x0d, 0xdb, 0x37, 0xab, 0x47, 0x01, 2);    // DEVPROP_TYPE_BOOLEAN

DEFINE_PROPERTYKEY(PKEY_DeviceInterface_ReferenceString, 0x026e516e, 0xb814, 0x414b, 0x83, 0xcd, 0x85, 0x6d, 0x6f, 0xef, 0x48, 0x22, 5);   // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_DeviceInterface_Restricted,      0x026e516e, 0xb814, 0x414b, 0x83, 0xcd, 0x85, 0x6d, 0x6f, 0xef, 0x48, 0x22, 6);   // DEVPROP_TYPE_BOOLEAN

DEFINE_PROPERTYKEY(PKEY_DeviceInterfaceClass_Name,       0x14c83a99, 0x0b3f, 0x44b7, 0xbe, 0x4c, 0xa1, 0x78, 0xd3, 0x99, 0x05, 0x64, 3); // DEVPROP_TYPE_STRING

DEFINE_PROPERTYKEY(PKEY_DeviceContainer_Address,                  0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 51);    // DEVPROP_TYPE_STRING | DEVPROP_TYPE_STRING_LIST
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_DiscoveryMethod,          0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 52);    // DEVPROP_TYPE_STRING_LIST
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_IsEncrypted,              0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 53);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_IsAuthenticated,          0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 54);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_IsConnected,              0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 55);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_IsPaired,                 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 56);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_Icon,                     0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 57);    // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_Version,                  0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 65);    // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_Last_Seen,                0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 66);    // DEVPROP_TYPE_FILETIME
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_Last_Connected,           0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 67);    // DEVPROP_TYPE_FILETIME
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_IsShowInDisconnectedState, 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 68);   // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_IsLocalMachine,           0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 70);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_MetadataPath,             0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 71);    // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_IsMetadataSearchInProgress, 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 72);          // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_MetadataChecksum,         0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 73);            // DEVPROP_TYPE_BINARY
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_IsNotInterestingForDisplay, 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 74);          // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_LaunchDeviceStageOnDeviceConnect, 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 76);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_LaunchDeviceStageFromExplorer, 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 77);       // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_BaselineExperienceId,     0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 78);    // DEVPROP_TYPE_GUID
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_IsDeviceUniquelyIdentifiable, 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 79);        // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_AssociationArray,         0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 80);    // DEVPROP_TYPE_STRING_LIST
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_DeviceDescription1,       0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 81);    // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_DeviceDescription2,       0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 82);    // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_HasProblem,               0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 83);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_IsSharedDevice,           0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 84);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_IsNetworkDevice,          0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 85);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_IsDefaultDevice,          0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 86);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_MetadataCabinet,          0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 87);    // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_RequiresPairingElevation, 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 88);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_ExperienceId,             0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 89);    // DEVPROP_TYPE_GUID
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_Category,                 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 90);    // DEVPROP_TYPE_STRING_LIST
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_Category_Desc_Singular,   0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 91);    // DEVPROP_TYPE_STRING_LIST
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_Category_Desc_Plural,     0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 92);    // DEVPROP_TYPE_STRING_LIST
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_Category_Icon,            0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 93);    // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_CategoryGroup_Desc,       0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 94);    // DEVPROP_TYPE_STRING_LIST
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_CategoryGroup_Icon,       0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 95);    // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_PrimaryCategory,          0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 97);    // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_UnpairUninstall,          0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 98);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_RequiresUninstallElevation, 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 99);  // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_DeviceFunctionSubRank,    0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 100);   // DEVPROP_TYPE_UINT32
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_AlwaysShowDeviceAsConnected, 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 101);    // DEVPROP_TYPE_BOOLEAN
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_ConfigFlags,              0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 105);   // DEVPROP_TYPE_UINT32
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_PrivilegedPackageFamilyNames, 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 106);   // DEVPROP_TYPE_STRING_LIST
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_CustomPrivilegedPackageFamilyNames, 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 107);   // DEVPROP_TYPE_STRING_LIST
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_IsRebootRequired,         0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 108);   // DEVPROP_TYPE_BOOLEAN

DEFINE_PROPERTYKEY(PKEY_DeviceContainer_FriendlyName,             0x656A3BB3, 0xECC0, 0x43FD, 0x84, 0x77, 0x4A, 0xE0, 0x40, 0x4A, 0x96, 0xCD, 12288); // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_Manufacturer,             0x656A3BB3, 0xECC0, 0x43FD, 0x84, 0x77, 0x4A, 0xE0, 0x40, 0x4A, 0x96, 0xCD, 8192);  // DEVPROP_TYPE_STRING
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_ModelName,                0x656A3BB3, 0xECC0, 0x43FD, 0x84, 0x77, 0x4A, 0xE0, 0x40, 0x4A, 0x96, 0xCD, 8194);  // DEVPROP_TYPE_STRING (localizable)
DEFINE_PROPERTYKEY(PKEY_DeviceContainer_ModelNumber,              0x656A3BB3, 0xECC0, 0x43FD, 0x84, 0x77, 0x4A, 0xE0, 0x40, 0x4A, 0x96, 0xCD, 8195);  // DEVPROP_TYPE_STRING

DEFINE_PROPERTYKEY(PKEY_DeviceContainer_InstallInProgress,        0x83da6326, 0x97a6, 0x4088, 0x94, 0x53, 0xa1, 0x92, 0x3f, 0x57, 0x3b, 0x29, 9);     // DEVPROP_TYPE_BOOLEAN

#pragma endregion

#pragma region EXTRA_AUDIO_KEYS_(debug)

// fmtid B3F8FA53 - 0004 - 438E-9003 - 51 A4 6E 13 9B FC pid  15 VT_BLOB
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_BlobMain_1,	0xB3F8FA53, 0x0004, 0x438E, 0x90, 0x03, 0x51, 0xA4, 0x6E, 0x13, 0x9B, 0xFC, 15); // VT_BLOB

// fmtid B3F8FA53 - 0004 - 438E-9003 - 51 A4 6E 13 9B FC pid   6 VT_LPWSTR
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_DeviceDesc,	0xB3F8FA53, 0x0004, 0x438E, 0x90, 0x03, 0x51, 0xA4, 0x6E, 0x13, 0x9B, 0xFC,  6); // VT_LPWSTR

// fmtid B3F8FA53 - 0004 - 438E-9003 - 51 A4 6E 13 9B FC pid   2 VT_LPWSTR
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_FuncSubSys,	0xB3F8FA53, 0x0004, 0x438E, 0x90, 0x03, 0x51, 0xA4, 0x6E, 0x13, 0x9B, 0xFC,  2); // VT_LPWSTR

// fmtid B3F8FA53 - 0004 - 438E-9003 - 51 A4 6E 13 9B FC pid  11 VT_LPWSTR
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_FuncSubSys_2,  0xB3F8FA53, 0x0004, 0x438E, 0x90, 0x03, 0x51, 0xA4, 0x6E, 0x13, 0x9B, 0xFC, 11); // VT_LPWSTR

// fmtid B3F8FA53 - 0004 - 438E-9003 - 51 A4 6E 13 9B FC pid   0 VT_UI4
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_IntA_1,		0xB3F8FA53, 0x0004, 0x438E, 0x90, 0x03, 0x51, 0xA4, 0x6E, 0x13, 0x9B, 0xFC,  0); // VT_UI4

// fmtid B3F8FA53 - 0004 - 438E-9003 - 51 A4 6E 13 9B FC pid  12 VT_UI4
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_IntA_2,		0xB3F8FA53, 0x0004, 0x438E, 0x90, 0x03, 0x51, 0xA4, 0x6E, 0x13, 0x9B, 0xFC, 12); // VT_UI4

// fmtid B3F8FA53 - 0004 - 438E-9003 - 51 A4 6E 13 9B FC pid  10 VT_UI4
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_IntA_3,		0xB3F8FA53, 0x0004, 0x438E, 0x90, 0x03, 0x51, 0xA4, 0x6E, 0x13, 0x9B, 0xFC, 10); // VT_UI4

// fmtid B3F8FA53 - 0004 - 438E-9003 - 51 A4 6E 13 9B FC pid   5 VT_BLOB
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_BlobMain_2,	0xB3F8FA53, 0x0004, 0x438E, 0x90, 0x03, 0x51, 0xA4, 0x6E, 0x13, 0x9B, 0xFC,  5); // VT_BLOB

// fmtid B3F8FA53 - 0004 - 438E-9003 - 51 A4 6E 13 9B FC pid   1 VT_BLOB
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_BlobMain_3,	0xB3F8FA53, 0x0004, 0x438E, 0x90, 0x03, 0x51, 0xA4, 0x6E, 0x13, 0x9B, 0xFC,  1); // VT_BLOB

// fmtid 83DA6326 - 97A6 - 4088-9453 - A1 92 3F 57 3B 29 pid   3 VT_LPWSTR
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_Manufacturer,	0x83DA6326, 0x97A6, 0x4088, 0x94, 0x53, 0xA1, 0x92, 0x3F, 0x57, 0x3B, 0x29,  3); // VT_LPWSTR

// fmtid 840B8171 - B0AD - 410F-8581 - CC CC 03 82 CF EF pid   0 VT_BLOB
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_BlobSub,		0x840B8171, 0xB0AD, 0x410F, 0x85, 0x81, 0xCC, 0xCC, 0x03, 0x82, 0xCF, 0xEF,  0); // VT_BLOB

// fmtid F1056047 - B091 - 4D85-A5C0 - B1 3D 4D 8B AC 57 pid   0 VT_LPWSTR
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_ExtraGUID,		0xF1056047, 0xB091, 0x4D85, 0xA5, 0xC0, 0xB1, 0x3D, 0x4D, 0x8B, 0xAC, 0x57,  0); // VT_LPWSTR

// fmtid 9A82A7DB - 3EBB - 41B4-83BA - 18 B7 31 17 18 FC pid   1 VT_UI4
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_IntB_1,		0x9A82A7DB, 0x3EBB, 0x41B4, 0x83, 0xBA, 0x18, 0xB7, 0x31, 0x17, 0x18, 0xFC,  1); // VT_UI4

// fmtid 9A82A7DB - 3EBB - 41B4-83BA - 18 B7 31 17 18 FC pid 146 VT_UI4
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_IntB_2,		0x9A82A7DB, 0x3EBB, 0x41B4, 0x83, 0xBA, 0x18, 0xB7, 0x31, 0x17, 0x18, 0xFC, 146); // VT_UI4

// fmtid 9A82A7DB - 3EBB - 41B4 83BA - 18 B7 31 17 18 FC pid 356 VT_UI4
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_IntB_3,		0x9A82A7DB, 0x3EBB, 0x41B4, 0x83, 0xBA, 0x18, 0xB7, 0x31, 0x17, 0x18, 0xFC, 356); // VT_UI4

// fmtid 233164C8 - 1B2C - 4C7D-BC68 - B6 71 68 7A 25 67 pid   1 VT_LPWSTR
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_FuncCodec_1,	0x233164C8, 0x1B2C, 0x4C7D, 0xBC, 0x68, 0xB6, 0x71, 0x68, 0x7A, 0x25, 0x67,  1); // VT_LPWSTR

// fmtid 233164C8 - 1B2C - 4C7D-BC68 - B6 71 68 7A 25 67 pid 146 VT_LPWSTR
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_FuncCodec_2,	0x233164C8, 0x1B2C, 0x4C7D, 0xBC, 0x68, 0xB6, 0x71, 0x68, 0x7A, 0x25, 0x67, 146); // VT_LPWSTR

// fmtid 233164C8 - 1B2C - 4C7D-BC68 - B6 71 68 7A 25 67 pid 356 VT_LPWSTR
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_FuncCodec_3,	0x233164C8, 0x1B2C, 0x4C7D, 0xBC, 0x68, 0xB6, 0x71, 0x68, 0x7A, 0x25, 0x67, 356); // VT_LPWSTR

// fmtid 5A9125B7 - F367 - 4924-ACE2 - 08 03 A4 A3 A4 71 pid   0 VT_UI4
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_IntC_1,		0x5A9125B7, 0xF367, 0x4924, 0xAC, 0xE2, 0x08, 0x03, 0xA4, 0xA3, 0xA4, 0x71,  0); // VT_UI4

// fmtid 5A9125B7 - F367 - 4924-ACE2 - 08 03 A4 A3 A4 71 pid   2 VT_UI4
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_IntC_2,		0x5A9125B7, 0xF367, 0x4924, 0xAC, 0xE2, 0x08, 0x03, 0xA4, 0xA3, 0xA4, 0x71,  2); // VT_UI4

// fmtid 9D631510 - 92A8 - 4A79-A79E - A8 38 12 C9 C1 19 pid   0 VT_LPWSTR
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_ExtraGUIDx_1,	0x9D631510, 0x92A8, 0x4A79, 0xA7, 0x9E, 0xA8, 0x38, 0x12, 0xC9, 0xC1, 0x19,  0); // VT_LPWSTR

// fmtid 9D631510 - 92A8 - 4A79-A79E - A8 38 12 C9 C1 19 pid   1 VT_LPWSTR
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_ExtraGUIDx_2,	0x9D631510, 0x92A8, 0x4A79, 0xA7, 0x9E, 0xA8, 0x38, 0x12, 0xC9, 0xC1, 0x19,  1); // VT_LPWSTR

// fmtid 9D631510 - 92A8 - 4A79-A79E - A8 38 12 C9 C1 19 pid   2 VT_LPWSTR
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_ExtraGUIDx_3,	0x9D631510, 0x92A8, 0x4A79, 0xA7, 0x9E, 0xA8, 0x38, 0x12, 0xC9, 0xC1, 0x19,  2); // VT_LPWSTR

// fmtid 9D631510 - 92A8 - 4A79-A79E - A8 38 12 C9 C1 19 pid   3 VT_UI4
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_IntD_1,		0x9D631510, 0x92A8, 0x4A79, 0xA7, 0x9E, 0xA8, 0x38, 0x12, 0xC9, 0xC1, 0x19,  3); // VT_UI4

// fmtid 9D631510 - 92A8 - 4A79-A79E - A8 38 12 C9 C1 19 pid   4 VT_UI4
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_IntD_2,		0x9D631510, 0x92A8, 0x4A79, 0xA7, 0x9E, 0xA8, 0x38, 0x12, 0xC9, 0xC1, 0x19,  4); // VT_UI4

// fmtid 9D631510 - 92A8 - 4A79-A79E - A8 38 12 C9 C1 19 pid   5 VT_BLOB or VT_VECTOR | VT_ARRAY | VT_BYREF | VT_RESERVED
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_Reserved_1,	0x9D631510, 0x92A8, 0x4A79, 0xA7, 0x9E, 0xA8, 0x38, 0x12, 0xC9, 0xC1, 0x19,  5); // VT_BLOB  or  VT_VECTOR | VT_ARRAY | VT_BYREF | VT_RESERVED

// fmtid 9D631510 - 92A8 - 4A79-A79E - A8 38 12 C9 C1 19 pid   6 VT_BLOB or VT_VECTOR | VT_ARRAY | VT_BYREF | VT_RESERVED
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_Reserved_2,	0x9D631510, 0x92A8, 0x4A79, 0xA7, 0x9E, 0xA8, 0x38, 0x12, 0xC9, 0xC1, 0x19,  6); // VT_BLOB  or  VT_VECTOR | VT_ARRAY | VT_BYREF | VT_RESERVED

// fmtid E4870E26 - 3CC5 - 4CD2-BA46 - CA 0A 9A 70 ED 04 pid   0 VT_BLOB
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_BlobS_1,		0xE4870E26, 0x3CC5, 0x4CD2, 0xBA, 0x46, 0xCA, 0x0A, 0x9A, 0x70, 0xED, 0x04,  0); // VT_BLOB

// fmtid E4870E26 - 3CC5 - 4CD2-BA46 - CA 0A 9A 70 ED 04 pid   1 VT_BLOB
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_BlobS_2,		0xE4870E26, 0x3CC5, 0x4CD2, 0xBA, 0x46, 0xCA, 0x0A, 0x9A, 0x70, 0xED, 0x04,  1); // VT_BLOB

// fmtid 7A8B74C8 - FA66 - 4460-8542 - D6 11 9C 59 E9 D4 pid   4 VT_UI4
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_IntE_1,		0x7A8B74C8, 0xFA66, 0x4460, 0x85, 0x42, 0xD6, 0x11, 0x9C, 0x59, 0xE9, 0xD4,  4); // VT_UI4

// fmtid 6CA6A085 - 3041 - 482B-9113 - C6 1E 7F 25 03 56 pid   0 VT_LPWSTR
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_Version,		0x6CA6A085, 0x3041, 0x482B, 0x91, 0x13, 0xC6, 0x1E, 0x7F, 0x25, 0x03, 0x56,  0); // VT_LPWSTR

// fmtid 9855C4CD - DF8C - 449C-A181 - 81 91 B6 8B D0 6C pid   0 VT_BLOB
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_BlobEx,		0x9855C4CD, 0xDF8C, 0x449C, 0xA1, 0x81, 0x81, 0x91, 0xB6, 0x8B, 0xD0, 0x6C,  0); // VT_BLOB

// fmtid 24DBB0FC - 9311 - 4B3D-9CF0 - 18 FF 15 56 39 D4 pid   3 VT_BOOL
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_PropBool_1,	0x24DBB0FC, 0x9311, 0x4B3D, 0x9C, 0xF0, 0x18, 0xFF, 0x15, 0x56, 0x39, 0xD4,  3); // VT_BOOL

// fmtid 24DBB0FC - 9311 - 4B3D-9CF0 - 18 FF 15 56 39 D4 pid   4 VT_BOOL
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_PropBool_2,	0x24DBB0FC, 0x9311, 0x4B3D, 0x9C, 0xF0, 0x18, 0xFF, 0x15, 0x56, 0x39, 0xD4,  4); // VT_BOOL

// fmtid 3BA0CD54 - 830F - 4551-A6EB - F3 EA B6 8E 37 00 pid  17 VT_UI4
DEFINE_PROPERTYKEY(PKEY_Device_EXTRA_IntF_1,		0x3BA0CD54, 0x830F, 0x4551, 0xA6, 0xEB, 0xF3, 0xEA, 0xB6, 0x8E, 0x37, 0x00, 17); // VT_UI4

#pragma endregion

namespace SAVFGAME
{
	struct NotificationData
	{
		wstring			devID;
		uint32			newState;
		PROPERTYKEY		propKey;
		EDataFlow		flow;
		ERole			role;
	};

	class CMMNotificationClient : public IMMNotificationClient
	{
		LONG _cRef;
		CSoundDevice * pManager;

	public:
		CMMNotificationClient(CSoundDevice * pDeviceManager) : _cRef(1) { pManager = pDeviceManager; };
		~CMMNotificationClient() { }; 

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

		//>> Уведомляет, когда изменилось состояние устройства <eSoundDevState>
		HRESULT STDMETHODCALLTYPE OnDeviceStateChanged	( LPCWSTR pwstrDeviceId, DWORD dwNewState ) override final
		{
			//wprintf(L"\n[IMMNotify] Device state has been changed");

			if (pManager == nullptr)
				{ _MBM(ERROR_PointerNone); return S_FALSE; }

			NotificationData pData;
			pData.devID    = pwstrDeviceId;
			pData.newState = dwNewState;

			pManager->IncomingNotification(eSDNotifyDeviceStateChanged, &pData);

			return S_OK;
		};
		//>> Уведомляет о появлении нового устройства в системе
		HRESULT STDMETHODCALLTYPE OnDeviceAdded			( LPCWSTR pwstrDeviceId ) override final
		{
			//wprintf(L"\n[IMMNotify] Added new audio endpoint device");

			if (pManager == nullptr)
				{ _MBM(ERROR_PointerNone); return S_FALSE; }

			NotificationData pData;
			pData.devID    = pwstrDeviceId;

			pManager->IncomingNotification(eSDNotifyDeviceAdded, &pData);

			return S_OK;
		};
		//>> Уведомляет об удалении существующего устройства из системы
		HRESULT STDMETHODCALLTYPE OnDeviceRemoved		( LPCWSTR pwstrDeviceId ) override final
		{
			//wprintf(L"\n[IMMNotify] User removed audio endpoint device");

			if (pManager == nullptr)
				{ _MBM(ERROR_PointerNone); return S_FALSE; }

			NotificationData pData;
			pData.devID    = pwstrDeviceId;

			pManager->IncomingNotification(eSDNotifyDeviceRemoved, &pData);

			return S_OK;
		};
		//>> Уведомляет, что сменилось устройство по умолчанию для определённой роли (eConsole / eMultimedia / eCommunications)
		HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged( EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override final
		{
			//wprintf(L"\n[IMMNotify] User changes role of an default audio endpoint device");

			if (pManager == nullptr)
				{ _MBM(ERROR_PointerNone); return S_FALSE; }

			NotificationData pData;
			pData.devID = pwstrDefaultDeviceId;
			pData.flow  = flow;
			pData.role  = role;

			pManager->IncomingNotification(eSDNotifyDefaultDeviceChanged, &pData);

			return S_OK;
		};
		//>> Уведомляет, что изменилось некоторое свойство устройства
		HRESULT STDMETHODCALLTYPE OnPropertyValueChanged( LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override final
		{
			//wprintf(L"\n[IMMNotify] User changes value of a property of an audio endpoint device");

			if (pManager == nullptr)
				{ _MBM(ERROR_PointerNone); return S_FALSE; }

			NotificationData pData;
			pData.devID   = pwstrDeviceId;
			pData.propKey = key;

			pManager->IncomingNotification(eSDNotifyPropertyValueChanged, &pData);

			return S_OK;
		};
	};
}

#endif // _SOUNDIMMN_H