// ----------------------------------------------------------------------- //
//
// MODULE  : device3d9.h
//
// PURPOSE : Вспомогательная оболочка 3D девайса (DX9)
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _DEVICE3D9_H
#define _DEVICE3D9_H

#include "device3d.h"

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment (lib, "d3d9.lib")
#ifdef _DEBUG
#	pragma comment (lib, "d3dx9d.lib")
#else
#	pragma comment (lib, "d3dx9.lib")
#endif
//#include <d2d1.h>
//#include <d2d1helper.h>

#define  IDirectT   IDirect3D9
#define	 IDeviceT   IDirect3DDevice9
#define  IDevcapT   D3DCAPS9
#define  D3DPARAM   D3DPRESENT_PARAMETERS

#define CONCURRENT_DATA_STREAM_default 0  // d3ddev->SetStreamSource(N, ...)  &  check by d3dcaps->MaxStreams

namespace SAVFGAME
{
	enum eDisplayFormat // surfaces' formats
	{
		EDF_R8G8B8       = D3DFMT_R8G8B8,
		EDF_A8R8G8B8     = D3DFMT_A8R8G8B8,
		EDF_X8R8G8B8     = D3DFMT_X8R8G8B8,	// display mode default format
		EDF_R5G6B5       = D3DFMT_R5G6B5,
		EDF_X1R5G5B5     = D3DFMT_X1R5G5B5,
		EDF_A1R5G5B5     = D3DFMT_A1R5G5B5,
		EDF_A4R4G4B4     = D3DFMT_A4R4G4B4,
		EDF_R3G3B2       = D3DFMT_R3G3B2,
		EDF_A8           = D3DFMT_A8,
		EDF_A8R3G3B2     = D3DFMT_A8R3G3B2,
		EDF_X4R4G4B4     = D3DFMT_X4R4G4B4,
		EDF_A2B10G10R10  = D3DFMT_A2B10G10R10,
		EDF_A8B8G8R8     = D3DFMT_A8B8G8R8,
		EDF_X8B8G8R8     = D3DFMT_X8B8G8R8,
		EDF_G16R16       = D3DFMT_G16R16,
		EDF_A2R10G10B10  = D3DFMT_A2R10G10B10,
		EDF_A16B16G16R16 = D3DFMT_A16B16G16R16,

		EDF_D16_LOCKABLE = D3DFMT_D16_LOCKABLE,
		EDF_D32          = D3DFMT_D32,			//  Specifies a 32-bit depth buffer
		EDF_D15S1        = D3DFMT_D15S1,
		EDF_D24S8        = D3DFMT_D24S8,	// (default) Specifies a 24-bit depth buffer with 8 bits reserved as the stencil buffer
		EDF_D24X8        = D3DFMT_D24X8,	// Specifies a 24-bit depth buffer only
		EDF_D24X4S4      = D3DFMT_D24X4S4,	// Specifies a 24-bit buffer with 4 bits reserved for the stencil buffer
		EDF_D16          = D3DFMT_D16,		// Specifies a 16-bit depth buffer only

		EDF_INDEX16      = D3DFMT_INDEX16,
		EDF_INDEX32      = D3DFMT_INDEX32,

		EDF_ENUM_MAX
	};
	enum eIntervalData // refresh interval type
	{
		EID_VSYNC_ON  = D3DPRESENT_INTERVAL_DEFAULT,	// _DEFAULT == _ONE
		EID_VSYNC_OFF = D3DPRESENT_INTERVAL_IMMEDIATE,

		EID_INTERVAL_DEFAULT	= D3DPRESENT_INTERVAL_DEFAULT,
		EID_INTERVAL_IMMEDIATE	= D3DPRESENT_INTERVAL_IMMEDIATE,
		EID_INTERVAL_ONE		= D3DPRESENT_INTERVAL_ONE,
		EID_INTERVAL_TWO		= D3DPRESENT_INTERVAL_TWO,
		EID_INTERVAL_THREE		= D3DPRESENT_INTERVAL_THREE,
		EID_INTERVAL_FOUR		= D3DPRESENT_INTERVAL_FOUR,

		EID_ENUM_MAX
	};
	
	class DEV3DDAT;

	struct DataManagerCubeTextureDX9 : public DataManagerCubeTexture // 2d texture manager : directx 9 implementation
	{
		friend class DEV3DDAT;
		//////////////////////
		DataManagerCubeTextureDX9() : tex(nullptr) {};
		virtual ~DataManagerCubeTextureDX9() { Close(); };
		void LoadData(IDirect3DDevice9 * d3ddev, const void* buf, uint32 size)
		{
			if (tex != nullptr) return; // already loaded

			if (buf == nullptr || size == 0) return; // error input data

			if (D3D_OK != D3DXCreateCubeTextureFromFileInMemory(d3ddev, buf, size, &tex))
				_MBM(ERROR_CrtTextureFFIM);
		}
		virtual void Close() override final
		{
			_RELEASE(tex);
		}
		virtual bool Loaded() override final
		{
			return (tex != nullptr);
		}
	protected:
		IDirect3DCubeTexture9 *		tex;
	};
	struct DataManagerTextureDX9 : public DataManagerTexture // 2d texture manager : directx 9 implementation
	{
		friend class DEV3DDAT;
		//////////////////////
		DataManagerTextureDX9() : tex(nullptr) {};
		virtual ~DataManagerTextureDX9() { Close(); };
		void LoadData(IDirect3DDevice9 * d3ddev, const void* buf, uint32 size)
		{
			if (tex != nullptr) return; // already loaded

			if (buf == nullptr || size == 0) return; // error input data

			if (D3D_OK != D3DXCreateTextureFromFileInMemory(d3ddev, buf, size, &tex))
				_MBM(ERROR_CrtTextureFFIM);
		}
		virtual void Close() override final
		{
			_RELEASE(tex);
		}
		virtual bool Loaded() override final
		{
			return (tex != nullptr);
		}
	protected:
		IDirect3DTexture9 *		tex;
	};
	struct DataManagerModelDX9 : public DataManagerModel // verticies manager : directx 9 implementation
	{
		friend class DEV3DDAT;
		//////////////////////
		DataManagerModelDX9() : vbuf(nullptr), ibuf(nullptr) {};
		virtual ~DataManagerModelDX9() { Close(); };
		void LoadData(IDirect3DDevice9 * d3ddev, const void* verticies, const uint32* indicies, uint32 vrtCount, uint32 idcCount,
			uint32 size_of_vertex_struct)
		{
			if (vbuf != nullptr) return; // already loaded

			if (verticies == nullptr || vrtCount == 0 ||
				indicies  == nullptr || idcCount == 0 || size_of_vertex_struct == 0) return; // error input data

			/*
			#if   defined (DIRECTX_9)
			//#define MODELFVF (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX3)
			#define MODELFVF (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX3)

			//#define FONTFVF (D3DFVF_XYZ | D3DFVF_TEX1)
			#define FONTFVF (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)
			#define UIFVF FONTFVF
			#endif
			//*/

			DWORD FVF = 0;

			num_vertices   = vrtCount;
			num_indices    = idcCount;
			num_primitives = idcCount / 3;	// triangle as default

			uint32 VBufLen = num_vertices * size_of_vertex_struct;
			uint32 IBufLen = num_indices  * sizeof(uint32);

			if ( D3D_OK != (d3ddev->CreateVertexBuffer(
								VBufLen,
								0,				// 0 = STATIC;  D3DUSAGE_DYNAMIC  _POINTS  _SOFTWAREPROCESSING  _WRITEONLY
								FVF,
								D3DPOOL_MANAGED,
								&vbuf,
								NULL)) )
				_MBM(ERROR_CrtVertexBuf);

			if ( D3D_OK != (d3ddev->CreateIndexBuffer(
								IBufLen,
								D3DUSAGE_WRITEONLY,
								D3DFMT_INDEX32,	 // 16, 32
								D3DPOOL_MANAGED,
								&ibuf,
								NULL)) )
				_MBM(ERROR_CrtIndexBuf);

			void* pVoid;

			vbuf->Lock(0,0,(void**)&pVoid,0);
			memcpy(pVoid, verticies, VBufLen);
			vbuf->Unlock();

			ibuf->Lock(0,0,(void**)&pVoid,0);
			memcpy(pVoid, indicies, IBufLen);
			ibuf->Unlock();
		};
		virtual void Close() override final
		{
			_RELEASE(vbuf);
			_RELEASE(ibuf);
			num_vertices = 0;
			num_indices = 0;
			num_primitives = 0;
		};
		virtual bool Loaded() override final
		{
			return (vbuf != nullptr && ibuf != nullptr);
		}
	protected:
		IDirect3DVertexBuffer9 *	vbuf;
		IDirect3DIndexBuffer9  *	ibuf;
	};

	class DEV3DDAT : public DEV3DBASE // videodevice DEV3DBASE : directx 9 implementation
	{
	public:
		DEV3DDAT() : DEV3DBASE()
		{
			value    = new DWORD;
			material = new D3DMATERIAL9;
			Default();
		};
		virtual ~DEV3DDAT() override final
		{
			Close();
			_DELETE(value);
			_DELETE(material);
		}
		virtual void Close() override final
		{
			_RELEASE(d3ddev);
			_DELETE(d3dcaps);
			Default();
			DEV3DBASE::Default();
		}
		void Init(IDirectT * d3d, UINT adapter, D3DDEVTYPE type)
		{
			// MEM //
			d3d_p      = d3d;
			d3dadapter = adapter;
			d3dtype    = type;

			InitAdapterIdentifier(d3d, adapter);
			InitDeviceCaps(d3d, adapter, type);
			InitSamplesInfo(d3d, adapter, type);
			InitBackBufferFormat(d3d, adapter, type);
			InitDepthStencil(d3d, adapter, type);

			//PrintfDeviceCaps();
		}
	protected:
		void InitAdapterIdentifier(IDirectT * d3d, UINT adapter)
		{
			D3DADAPTER_IDENTIFIER9 identifier;
			DWORD flags = D3DENUM_WHQL_LEVEL; // 0;

			// Flags sets the WHQLLevel member of D3DADAPTER_IDENTIFIER9.
			// Flags can be set to either 0 or D3DENUM_WHQL_LEVEL.
			// If D3DENUM_WHQL_LEVEL is specified, this call can connect to the Internet
			// to download new Microsoft Windows Hardware Quality Labs(WHQL) certificates.

			d3d->GetAdapterIdentifier(adapter, flags, &identifier);

			adapter_description.driver             = identifier.Driver;
			adapter_description.description        = identifier.Description;
			adapter_description.deviceName         = identifier.DeviceName;
			adapter_description.driverVersion._U64 = identifier.DriverVersion.QuadPart;
			adapter_description.vendorId           = identifier.VendorId;
			adapter_description.deviceId           = identifier.DeviceId;
			adapter_description.subSysId           = identifier.SubSysId;
			adapter_description.revision           = identifier.Revision;
			adapter_description.WHQLLevel._U32     = identifier.WHQLLevel;

			memcpy(	& adapter_description.deviceIdentifier,
					& identifier.DeviceIdentifier,
					_SZTP(identifier.DeviceIdentifier));

			//adapter_description.Printf();
		}
		void InitDeviceCaps(IDirectT * d3d, UINT adapter, D3DDEVTYPE type)
		{
			if (d3d == nullptr) { _MBM(ERROR_InitDEV3DDAT);  return; }

			if (d3dcaps == nullptr)
				d3dcaps = new IDevcapT;

			d3d->GetDeviceCaps(adapter, type, d3dcaps);
		}
		void InitSamplesInfo(IDirectT * d3d, UINT adapter, D3DDEVTYPE type)
		{
			if (d3d == nullptr) { _MBM(ERROR_InitDEV3DDAT);  return; }

			DWORD dQualityLevels;
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_16_SAMPLES, &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_15_SAMPLES, &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_14_SAMPLES, &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_13_SAMPLES, &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_12_SAMPLES, &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_11_SAMPLES, &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_10_SAMPLES, &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_9_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_8_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_7_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_6_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_5_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_4_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_3_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_2_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, true, D3DMULTISAMPLE_NONMASKABLE, &dQualityLevels))	{
				   nSamples_windowed = D3DMULTISAMPLE_NONE;
			} else nSamples_windowed = D3DMULTISAMPLE_NONMASKABLE;
			} else nSamples_windowed = D3DMULTISAMPLE_2_SAMPLES;
			} else nSamples_windowed = D3DMULTISAMPLE_3_SAMPLES;
			} else nSamples_windowed = D3DMULTISAMPLE_4_SAMPLES;
			} else nSamples_windowed = D3DMULTISAMPLE_5_SAMPLES;
			} else nSamples_windowed = D3DMULTISAMPLE_6_SAMPLES;
			} else nSamples_windowed = D3DMULTISAMPLE_7_SAMPLES;
			} else nSamples_windowed = D3DMULTISAMPLE_8_SAMPLES;
			} else nSamples_windowed = D3DMULTISAMPLE_9_SAMPLES;
			} else nSamples_windowed = D3DMULTISAMPLE_10_SAMPLES;
			} else nSamples_windowed = D3DMULTISAMPLE_11_SAMPLES;
			} else nSamples_windowed = D3DMULTISAMPLE_12_SAMPLES;
			} else nSamples_windowed = D3DMULTISAMPLE_13_SAMPLES;
			} else nSamples_windowed = D3DMULTISAMPLE_14_SAMPLES;
			} else nSamples_windowed = D3DMULTISAMPLE_15_SAMPLES;
			} else nSamples_windowed = D3DMULTISAMPLE_16_SAMPLES;

			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_16_SAMPLES, &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_15_SAMPLES, &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_14_SAMPLES, &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_13_SAMPLES, &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_12_SAMPLES, &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_11_SAMPLES, &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_10_SAMPLES, &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_9_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_8_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_7_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_6_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_5_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_4_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_3_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_2_SAMPLES,  &dQualityLevels))	{
			if (D3D_OK != d3d->CheckDeviceMultiSampleType(adapter, type, D3DFMT_X8R8G8B8, false, D3DMULTISAMPLE_NONMASKABLE, &dQualityLevels))	{
				   nSamples_fullscreen = D3DMULTISAMPLE_NONE;
			} else nSamples_fullscreen = D3DMULTISAMPLE_NONMASKABLE;
			} else nSamples_fullscreen = D3DMULTISAMPLE_2_SAMPLES;
			} else nSamples_fullscreen = D3DMULTISAMPLE_3_SAMPLES;
			} else nSamples_fullscreen = D3DMULTISAMPLE_4_SAMPLES;
			} else nSamples_fullscreen = D3DMULTISAMPLE_5_SAMPLES;
			} else nSamples_fullscreen = D3DMULTISAMPLE_6_SAMPLES;
			} else nSamples_fullscreen = D3DMULTISAMPLE_7_SAMPLES;
			} else nSamples_fullscreen = D3DMULTISAMPLE_8_SAMPLES;
			} else nSamples_fullscreen = D3DMULTISAMPLE_9_SAMPLES;
			} else nSamples_fullscreen = D3DMULTISAMPLE_10_SAMPLES;
			} else nSamples_fullscreen = D3DMULTISAMPLE_11_SAMPLES;
			} else nSamples_fullscreen = D3DMULTISAMPLE_12_SAMPLES;
			} else nSamples_fullscreen = D3DMULTISAMPLE_13_SAMPLES;
			} else nSamples_fullscreen = D3DMULTISAMPLE_14_SAMPLES;
			} else nSamples_fullscreen = D3DMULTISAMPLE_15_SAMPLES;
			} else nSamples_fullscreen = D3DMULTISAMPLE_16_SAMPLES;

			nSamples_windowed_now = nSamples_windowed;
			nSamples_fullscreen_now = nSamples_fullscreen;

			//d3d->

			//printf("\nsamples_windowed   %i", nSamples_windowed);
			//printf("\nsamples_fullscreen %i", nSamples_fullscreen);
		}
		void InitBackBufferFormat(IDirectT * d3d, UINT adapter, D3DDEVTYPE type)
		{
			if (d3d == nullptr) { _MBM(ERROR_InitDEV3DDAT);  return; }

			D3DDISPLAYMODE displayMode, dM_def;
			ZeroMemory(&displayMode, sizeof(D3DDISPLAYMODE));
			ZeroMemory(&dM_def,      sizeof(D3DDISPLAYMODE));

			d3d->GetAdapterDisplayMode(adapter, &dM_def);
		//	printf("\nDISPLAY DEFAULTS\nFormat %i\nRefreshRate %i\nWidth %i\nHeight %i",
		//		dM_def.Format, dM_def.RefreshRate, dM_def.Width, dM_def.Height);

			UINT numModes32 = d3d->GetAdapterModeCount(adapter, D3DFMT_X8R8G8B8) - 1;
			UINT numModes16 = d3d->GetAdapterModeCount(adapter, D3DFMT_R5G6B5)   - 1;

			// MEM FOR DEBUG //
			SetDisplayDefault(dM_def.Width, dM_def.Height, dM_def.RefreshRate, dM_def.Format);
			SetDisplayDefault(numModes32, numModes16);

			if (numModes32 < (UINT)dM_def.Format ||
				numModes32 > 255)
				numModes32 = (UINT)dM_def.Format;

			//printf("\nADAPTER MODES COUNT = %i", numModes32);

			// ЗАМЕТКА
			// Не стоит надеяться на корректные возвращаемые данные в displayMode от EnumAdapterModes()

			if (D3D_OK == d3d->EnumAdapterModes(adapter, D3DFMT_A8R8G8B8, (UINT)D3DFMT_A8B8G8R8, &displayMode))
			{
				//printf("\nD3DFMT_A8R8G8B8 is OK"); 
				displayMode.Format = D3DFMT_A8R8G8B8;
			}			
			else if (D3D_OK == d3d->EnumAdapterModes(adapter, dM_def.Format, (UINT)dM_def.Format, &displayMode))
			{
				//printf("\nD3DFMT_DEFAULT is OK (%i)", dM_def.Format);		
				displayMode.Format = dM_def.Format;
			}			
			else //*/
			{
				bool everything_is_fine = false;
				for(int i=numModes32; i>=0; i--)
				{
					HRESULT ires = d3d->EnumAdapterModes(adapter, (D3DFORMAT)i, (UINT)i, &displayMode);
					ires = (ires==D3D_OK) ? true : false;
					//printf("\nMODE %3i result = %i", i, ires);
					if (ires)
					{
						//printf("\nD3DFMT_CHOOSER is OK (%i)", i);
						everything_is_fine = true;
						displayMode.Format = (D3DFORMAT)i;
						break;
					}
				}
				if (!everything_is_fine)
					displayMode.Format = dM_def.Format;
			}

		//	if (displayMode.RefreshRate != 60 &&
		//		displayMode.RefreshRate != 75)			
			displayMode.RefreshRate = dM_def.RefreshRate;

			printf("\nRefresh rate   %i", displayMode.RefreshRate);
			printf("\nSurface format %i", displayMode.Format);

			SetDisplay(MISSING, MISSING, displayMode.RefreshRate, displayMode.Format);

			////////////////////////////

			D3DDISPLAYMODE dM_debug;
			for (int i = numModes32; i >= 0; i--)
			{
				HRESULT ires = d3d->EnumAdapterModes(adapter, (D3DFORMAT)i, (UINT)i, &dM_debug);
				ires = (ires == D3D_OK) ? true : false;

				// MEM FOR DEBUG //
				SetDisplayDefault((UINT)i, _BOOL(ires));
			}
		}
		void InitDepthStencil(IDirectT * d3d, UINT adapter, D3DDEVTYPE type)
		{
			DEPTHSTENCIL dstencil;
			const UINT max_format = 7;

			for (int i = 0; i < max_format; i++)
			{
				eDisplayFormat frmt;
				switch (i)
				{
				case 0: frmt = EDF_D24S8;        break;
				case 1: frmt = EDF_D24X4S4;      break; 
				case 2: frmt = EDF_D15S1;        break; 
				case 3: frmt = EDF_D24X8;        break;
				case 4: frmt = EDF_D32;          break;
				case 5: frmt = EDF_D16;          break;			
				case 6: frmt = EDF_D16_LOCKABLE; break;
				}

				if (D3D_OK != d3d->CheckDeviceFormat(adapter, type,
					(D3DFORMAT)display_default.format,	// AdapterFormat
					D3DUSAGE_DEPTHSTENCIL,				// UsageOption for the surface = combination of D3DUSAGE and D3DUSAGE_QUERY
					D3DRTYPE_SURFACE,					// ResourceType
					(D3DFORMAT)((UINT)frmt)				// DepthFormat
					))
				{
					if (i == max_format - 1) _MBM(ERROR_DepthStencilFrmt);
				}
				else
				{
					if (D3D_OK != d3d->CheckDepthStencilMatch(adapter, type,
						(D3DFORMAT)display_default.format,	// AdapterFormat
						(D3DFORMAT)display_default.format,	// BackBufferFormat
						(D3DFORMAT)((UINT)frmt)))			// DepthFormat
					{
						if (i == max_format - 1) _MBM(ERROR_DepthStencilFrmt);
					}
					else
					{
						dstencil.format = frmt;
						break;
					}
				}
			}
			dstencil.enableAuto = true;

			SetDisplay(dstencil);
			// MEM FOR DEBUG //
			SetDisplayDefault(dstencil);
		}
	public:
		virtual void SetTransformView(MATH3DMATRIX * mView)                                                                        override final
		{
			if (d3ddev == nullptr) { _MBM(ERROR_InitDevice); return; }

			d3ddev->SetTransform(D3DTS_VIEW, D3DCAST(mView));
		}
		virtual void SetTransformProjection(MATH3DMATRIX * mProjection)                                                            override final
		{
			if (d3ddev == nullptr) { _MBM(ERROR_InitDevice); return; }

			d3ddev->SetTransform(D3DTS_PROJECTION, D3DCAST(mProjection));
		}
		virtual void SetTransformWorld(MATH3DMATRIX * mWorld)                                                                      override final
		{
			if (d3ddev == nullptr) { _MBM(ERROR_InitDevice); return; }

			d3ddev->SetTransform(D3DTS_WORLD, D3DCAST(mWorld));
		}
		virtual void SetViewPort(uint32 x, uint32 y, uint32 width, uint32 height, float minZ, float maxZ)                          override final
		{
			if (d3ddev == nullptr) { _MBM(ERROR_InitDevice); return; }

			D3DVIEWPORT9 viewPort = { x, y, width, height, minZ, maxZ };
			d3ddev->SetViewport(&viewPort);
		}
		virtual void SetMaterial(uint32 ambient, uint32 diffuse, uint32 specular, uint32 emissive, float power)                    override final
		{
			material->Ambient  = D3DXCOLOR(ambient);	// 
			material->Diffuse  = D3DXCOLOR(diffuse);	// D3DTA_DIFFUSE
			material->Specular = D3DXCOLOR(specular);	// 
			material->Emissive = D3DXCOLOR(emissive);	// D3DCOLOR_ARGB(255,0,0,0)
			material->Power    = power;					// specular power

			d3ddev->SetMaterial(material);
		}
		virtual void SetTextureStageState(byte index, eTextureState state)                                                         override final
		{
			switch (state)
			{
			//////////////////////////////
			case TSTATE_FONT:
			{
				d3ddev->SetTextureStageState(index, D3DTSS_TEXCOORDINDEX, index);
				d3ddev->SetTextureStageState(index, D3DTSS_COLOROP,       D3DTOP_MODULATE);
				d3ddev->SetTextureStageState(index, D3DTSS_COLORARG1,     D3DTA_TEXTURE);
				d3ddev->SetTextureStageState(index, D3DTSS_COLORARG2,     D3DTA_DIFFUSE);
				d3ddev->SetTextureStageState(index, D3DTSS_ALPHAOP,       D3DTOP_MODULATE);
				d3ddev->SetTextureStageState(index, D3DTSS_ALPHAARG1,     D3DTA_TEXTURE);
				d3ddev->SetTextureStageState(index, D3DTSS_ALPHAARG2,     D3DTA_DIFFUSE);
				break;
			}
			//////////////////////////////
			case TSTATE_MODEL_DIFFUSE_YES:
			{
				d3ddev->SetTextureStageState(index, D3DTSS_TEXCOORDINDEX, index);
				d3ddev->SetTextureStageState(index, D3DTSS_COLOROP,       D3DTOP_MODULATE);
				d3ddev->SetTextureStageState(index, D3DTSS_COLORARG1,     D3DTA_TEXTURE);
				d3ddev->SetTextureStageState(index, D3DTSS_COLORARG2,     D3DTA_DIFFUSE);
				d3ddev->SetTextureStageState(index, D3DTSS_ALPHAOP,       D3DTOP_SELECTARG1);
				d3ddev->SetTextureStageState(index, D3DTSS_ALPHAARG1,     D3DTA_TEXTURE);

				break;
			}
			//////////////////////////////
			case TSTATE_MODEL_DIFFUSE_NO:
			{
				d3ddev->SetTextureStageState(index, D3DTSS_TEXCOORDINDEX, index);
				d3ddev->SetTextureStageState(index, D3DTSS_COLOROP,       D3DTOP_SELECTARG1);
				d3ddev->SetTextureStageState(index, D3DTSS_COLORARG1,     D3DTA_DIFFUSE);
				d3ddev->SetTextureStageState(index, D3DTSS_ALPHAOP,       D3DTOP_SELECTARG1);
				d3ddev->SetTextureStageState(index, D3DTSS_ALPHAARG1,     D3DTA_DIFFUSE);
				break;
			}
			//////////////////////////////
			case TSTATE_MODEL_NORMALMAP_YES:
			{
				d3ddev->SetTextureStageState(index, D3DTSS_TEXCOORDINDEX, index);
				d3ddev->SetTextureStageState(index, D3DTSS_COLOROP,       D3DTOP_DISABLE);
				d3ddev->SetTextureStageState(index, D3DTSS_ALPHAOP,       D3DTOP_DISABLE);
				break;
			}
			//////////////////////////////
			case TSTATE_MODEL_NORMALMAP_NO:
			{
				d3ddev->SetTextureStageState(index, D3DTSS_TEXCOORDINDEX, index);
				d3ddev->SetTextureStageState(index, D3DTSS_COLOROP,       D3DTOP_DISABLE);
				d3ddev->SetTextureStageState(index, D3DTSS_ALPHAOP,       D3DTOP_DISABLE);
				break;
			}
			//////////////////////////////
			}
		}
		virtual void DrawPrimitive(ePrimTip tip, int32 base_vert, uint32 min_vert, uint32 verts, uint32 start_index, uint32 prims) override final
		{
			switch (tip)
			{
			case TIP_PRIM_TRIANGLELIST:
				d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,  base_vert, min_vert, verts, start_index, prims); break;
			case TIP_PRIM_POINTLIST:
				d3ddev->DrawIndexedPrimitive(D3DPT_POINTLIST,     base_vert, min_vert, verts, start_index, prims); break;
			case TIP_PRIM_LINELIST:
				d3ddev->DrawIndexedPrimitive(D3DPT_LINELIST,      base_vert, min_vert, verts, start_index, prims); break;
			case TIP_PRIM_LINESTRIP:
				d3ddev->DrawIndexedPrimitive(D3DPT_LINESTRIP,     base_vert, min_vert, verts, start_index, prims); break;
			case TIP_PRIM_TRIANGLESTRIP:
				d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, base_vert, min_vert, verts, start_index, prims); break;
			case TIP_PRIM_TRIANGLEFAN:
				d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN,   base_vert, min_vert, verts, start_index, prims); break;
			default:
				d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,  base_vert, min_vert, verts, start_index, prims);
			}
		}
	public:
		virtual UINT GetRenderState(eRenderState RSTATE)               override final
		{
			switch (RSTATE)
			{
			/////////////////////////////
			case RSTATE_CULLMODE:
			{
				d3ddev->GetRenderState(D3DRS_CULLMODE, value);
				switch (*value)
				{
				case D3DCULL_NONE: return _STATE_CULLMODE_NONE;
				case D3DCULL_CW:   return _STATE_CULLMODE_CW;
				case D3DCULL_CCW:  return _STATE_CULLMODE_CCW;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_LIGHTING:
			{
				d3ddev->GetRenderState(D3DRS_LIGHTING, value);
				switch (*value)
				{
				case 0:  return _STATE_OFF;
				default: return _STATE_ON;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_FOGENABLE:
			{
				d3ddev->GetRenderState(D3DRS_FOGENABLE, value);
				switch (*value)
				{
				case 0:  return _STATE_OFF;
				default: return _STATE_ON;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_COLORVERTEX:
			{
				d3ddev->GetRenderState(D3DRS_COLORVERTEX, value);
				switch (*value)
				{
				case 0:  return _STATE_OFF;
				default: return _STATE_ON;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_ALPHABLENDENABLE:
			{
				d3ddev->GetRenderState(D3DRS_ALPHABLENDENABLE, value);
				switch (*value)
				{
				case 0:  return _STATE_OFF;
				default: return _STATE_ON;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_ZENABLE:
			{
				d3ddev->GetRenderState(D3DRS_ZENABLE, value);
				switch (*value)
				{
				case D3DZB_FALSE: return _STATE_ZENABLE_OFF;
				case D3DZB_TRUE:  return _STATE_ZENABLE_ON_Z_BUF;
				case D3DZB_USEW:  return _STATE_ZENABLE_ON_W_BUF;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_ZWRITEENABLE:
			{
				d3ddev->GetRenderState(D3DRS_ZWRITEENABLE, value);
				switch (*value)
				{
				case 0:  return _STATE_OFF;
				default: return _STATE_ON;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_STENCILENABLE:
			{
				d3ddev->GetRenderState(D3DRS_STENCILENABLE, value);
				switch (*value)
				{
				case 0:  return _STATE_OFF;
				default: return _STATE_ON;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_CLIPPLANEENABLE:
			{
				d3ddev->GetRenderState(D3DRS_CLIPPLANEENABLE, value);
				switch (*value)
				{
				case 0:  return _STATE_OFF;
				default: return _STATE_ON;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_BLENDOP:
			{
				d3ddev->GetRenderState(D3DRS_BLENDOP, value);
				switch (*value)
				{
				case D3DBLENDOP_ADD:         return _STATE_BLENDOP_ADD;
				case D3DBLENDOP_SUBTRACT:    return _STATE_BLENDOP_SUBTRACT;
				case D3DBLENDOP_REVSUBTRACT: return _STATE_BLENDOP_REVSUBTRACT;
				case D3DBLENDOP_MIN:         return _STATE_BLENDOP_MIN;
				case D3DBLENDOP_MAX:         return _STATE_BLENDOP_MAX;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_SRCBLEND:
			{
				d3ddev->GetRenderState(D3DRS_SRCBLEND, value);
				switch (*value)
				{
				case D3DBLEND_ZERO:            return _STATE_BLEND_ZERO;
				case D3DBLEND_ONE:             return _STATE_BLEND_ONE;
				case D3DBLEND_SRCCOLOR:        return _STATE_BLEND_SRCCOLOR;
				case D3DBLEND_INVSRCCOLOR:     return _STATE_BLEND_INVSRCCOLOR;
				case D3DBLEND_SRCALPHA:        return _STATE_BLEND_SRCALPHA;
				case D3DBLEND_INVSRCALPHA:     return _STATE_BLEND_INVSRCALPHA;
				case D3DBLEND_DESTALPHA:       return _STATE_BLEND_DESTALPHA;
				case D3DBLEND_INVDESTALPHA:    return _STATE_BLEND_INVDESTALPHA;
				case D3DBLEND_DESTCOLOR:       return _STATE_BLEND_DESTCOLOR;
				case D3DBLEND_INVDESTCOLOR:    return _STATE_BLEND_INVDESTCOLOR;
				case D3DBLEND_SRCALPHASAT:     return _STATE_BLEND_SRCALPHASAT;
				case D3DBLEND_BOTHSRCALPHA:    return _STATE_BLEND_BOTHSRCALPHA;
				case D3DBLEND_BOTHINVSRCALPHA: return _STATE_BLEND_BOTHINVSRCALPHA;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_DESTBLEND:
			{
				d3ddev->GetRenderState(D3DRS_DESTBLEND, value);
				switch (*value)
				{
				case D3DBLEND_ZERO:            return _STATE_BLEND_ZERO;
				case D3DBLEND_ONE:             return _STATE_BLEND_ONE;
				case D3DBLEND_SRCCOLOR:        return _STATE_BLEND_SRCCOLOR;
				case D3DBLEND_INVSRCCOLOR:     return _STATE_BLEND_INVSRCCOLOR;
				case D3DBLEND_SRCALPHA:        return _STATE_BLEND_SRCALPHA;
				case D3DBLEND_INVSRCALPHA:     return _STATE_BLEND_INVSRCALPHA;
				case D3DBLEND_DESTALPHA:       return _STATE_BLEND_DESTALPHA;
				case D3DBLEND_INVDESTALPHA:    return _STATE_BLEND_INVDESTALPHA;
				case D3DBLEND_DESTCOLOR:       return _STATE_BLEND_DESTCOLOR;
				case D3DBLEND_INVDESTCOLOR:    return _STATE_BLEND_INVDESTCOLOR;
				case D3DBLEND_SRCALPHASAT:     return _STATE_BLEND_SRCALPHASAT;
				case D3DBLEND_BOTHSRCALPHA:    return _STATE_BLEND_BOTHSRCALPHA;
				case D3DBLEND_BOTHINVSRCALPHA: return _STATE_BLEND_BOTHINVSRCALPHA;
				}
				break;
			}
			/////////////////////////////
			}

			return RSTATE_NONE;
		}
		virtual void SetRenderState(eRenderState RSTATE, UINT _STATE_) override final
		{
			eRenderState MEM;
			if (RS.restore_memory_call) MEM = static_cast <eRenderState> (_STATE_);					// do not change memory
			else                        MEM = static_cast <eRenderState> (GetRenderState(RSTATE));	// new memory value

			switch (RSTATE)
			{
			/////////////////////////////
			case RSTATE_CULLMODE:
			{
				switch (_STATE_)
				{
				case _STATE_CULLMODE_NONE: d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); RS.MEM_RSTATE_CULLMODE = MEM; return;
				case _STATE_CULLMODE_CW:   d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);   RS.MEM_RSTATE_CULLMODE = MEM; return;
				case _STATE_CULLMODE_CCW:  d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);  RS.MEM_RSTATE_CULLMODE = MEM; return;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_LIGHTING:
			{
				switch (_STATE_)
				{
				case _STATE_OFF: d3ddev->SetRenderState(D3DRS_LIGHTING, 0); RS.MEM_RSTATE_LIGHTING = MEM; return;
				case _STATE_ON:  d3ddev->SetRenderState(D3DRS_LIGHTING, 1); RS.MEM_RSTATE_LIGHTING = MEM; return;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_FOGENABLE:
			{
				switch (_STATE_)
				{
				case _STATE_OFF: d3ddev->SetRenderState(D3DRS_FOGENABLE, 0); RS.MEM_RSTATE_FOGENABLE = MEM; return;
				case _STATE_ON:  d3ddev->SetRenderState(D3DRS_FOGENABLE, 1); RS.MEM_RSTATE_FOGENABLE = MEM; return;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_COLORVERTEX:
			{
				switch (_STATE_)
				{
				case _STATE_OFF: d3ddev->SetRenderState(D3DRS_COLORVERTEX, 0); RS.MEM_RSTATE_COLORVERTEX = MEM; return;
				case _STATE_ON:  d3ddev->SetRenderState(D3DRS_COLORVERTEX, 1); RS.MEM_RSTATE_COLORVERTEX = MEM; return;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_ALPHABLENDENABLE:
			{
				switch (_STATE_)
				{
				case _STATE_OFF: d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, 0); RS.MEM_RSTATE_ALPHABLENDENABLE = MEM; return;
				case _STATE_ON:  d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, 1); RS.MEM_RSTATE_ALPHABLENDENABLE = MEM; return;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_ZENABLE:
			{
				switch (_STATE_)
				{
				case _STATE_ZENABLE_OFF:      d3ddev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE); RS.MEM_RSTATE_ZENABLE = MEM; return;
				case _STATE_ZENABLE_ON_Z_BUF: d3ddev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);  RS.MEM_RSTATE_ZENABLE = MEM; return;
				case _STATE_ZENABLE_ON_W_BUF: d3ddev->SetRenderState(D3DRS_ZENABLE, D3DZB_USEW);  RS.MEM_RSTATE_ZENABLE = MEM; return;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_ZWRITEENABLE:
			{
				switch (_STATE_)
				{
				case _STATE_OFF: d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, 0); RS.MEM_RSTATE_ZWRITEENABLE = MEM; return;
				case _STATE_ON:  d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, 1); RS.MEM_RSTATE_ZWRITEENABLE = MEM; return;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_STENCILENABLE:
			{
				switch (_STATE_)
				{
				case _STATE_OFF: d3ddev->SetRenderState(D3DRS_STENCILENABLE, 0); RS.MEM_RSTATE_STENCILENABLE = MEM; return;
				case _STATE_ON:  d3ddev->SetRenderState(D3DRS_STENCILENABLE, 1); RS.MEM_RSTATE_STENCILENABLE = MEM; return;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_CLIPPLANEENABLE:
			{
				switch (_STATE_)
				{
				case _STATE_OFF: d3ddev->SetRenderState(D3DRS_CLIPPLANEENABLE, 0); RS.MEM_RSTATE_CLIPPLANEENABLE = MEM; return;
				case _STATE_ON:  d3ddev->SetRenderState(D3DRS_CLIPPLANEENABLE, 1); RS.MEM_RSTATE_CLIPPLANEENABLE = MEM; return;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_BLENDOP:
			{
				switch (_STATE_)
				{
				case _STATE_BLENDOP_ADD:         d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);         RS.MEM_RSTATE_BLENDOP = MEM; return;
				case _STATE_BLENDOP_SUBTRACT:    d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_SUBTRACT);    RS.MEM_RSTATE_BLENDOP = MEM; return;
				case _STATE_BLENDOP_REVSUBTRACT: d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT); RS.MEM_RSTATE_BLENDOP = MEM; return;
				case _STATE_BLENDOP_MIN:         d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MIN);         RS.MEM_RSTATE_BLENDOP = MEM; return;
				case _STATE_BLENDOP_MAX:         d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MAX);         RS.MEM_RSTATE_BLENDOP = MEM; return;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_SRCBLEND:
			{
				switch (_STATE_)
				{
				case _STATE_BLEND_ZERO:            d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);            RS.MEM_RSTATE_SRCBLEND = MEM; return;
				case _STATE_BLEND_ONE:             d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);             RS.MEM_RSTATE_SRCBLEND = MEM; return;
				case _STATE_BLEND_SRCCOLOR:        d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);        RS.MEM_RSTATE_SRCBLEND = MEM; return;
				case _STATE_BLEND_INVSRCCOLOR:     d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVSRCCOLOR);     RS.MEM_RSTATE_SRCBLEND = MEM; return;
				case _STATE_BLEND_SRCALPHA:        d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);        RS.MEM_RSTATE_SRCBLEND = MEM; return;
				case _STATE_BLEND_INVSRCALPHA:     d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA);     RS.MEM_RSTATE_SRCBLEND = MEM; return;
				case _STATE_BLEND_DESTALPHA:       d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);       RS.MEM_RSTATE_SRCBLEND = MEM; return;
				case _STATE_BLEND_INVDESTALPHA:    d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTALPHA);    RS.MEM_RSTATE_SRCBLEND = MEM; return;
				case _STATE_BLEND_DESTCOLOR:       d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);       RS.MEM_RSTATE_SRCBLEND = MEM; return;
				case _STATE_BLEND_INVDESTCOLOR:    d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);    RS.MEM_RSTATE_SRCBLEND = MEM; return;
				case _STATE_BLEND_SRCALPHASAT:     d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHASAT);     RS.MEM_RSTATE_SRCBLEND = MEM; return;
				case _STATE_BLEND_BOTHSRCALPHA:    d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BOTHSRCALPHA);    RS.MEM_RSTATE_SRCBLEND = MEM; return;
				case _STATE_BLEND_BOTHINVSRCALPHA: d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BOTHINVSRCALPHA); RS.MEM_RSTATE_SRCBLEND = MEM; return;
				}
				break;
			}
			/////////////////////////////
			case RSTATE_DESTBLEND:
			{
				switch (_STATE_)
				{
				case _STATE_BLEND_ZERO:            d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);            RS.MEM_RSTATE_DESTBLEND = MEM; return;
				case _STATE_BLEND_ONE:             d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);             RS.MEM_RSTATE_DESTBLEND = MEM; return;
				case _STATE_BLEND_SRCCOLOR:        d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);        RS.MEM_RSTATE_DESTBLEND = MEM; return;
				case _STATE_BLEND_INVSRCCOLOR:     d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);     RS.MEM_RSTATE_DESTBLEND = MEM; return;
				case _STATE_BLEND_SRCALPHA:        d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);        RS.MEM_RSTATE_DESTBLEND = MEM; return;
				case _STATE_BLEND_INVSRCALPHA:     d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);     RS.MEM_RSTATE_DESTBLEND = MEM; return;
				case _STATE_BLEND_DESTALPHA:       d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA);       RS.MEM_RSTATE_DESTBLEND = MEM; return;
				case _STATE_BLEND_INVDESTALPHA:    d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVDESTALPHA);    RS.MEM_RSTATE_DESTBLEND = MEM; return;
				case _STATE_BLEND_DESTCOLOR:       d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);       RS.MEM_RSTATE_DESTBLEND = MEM; return;
				case _STATE_BLEND_INVDESTCOLOR:    d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVDESTCOLOR);    RS.MEM_RSTATE_DESTBLEND = MEM; return;
				case _STATE_BLEND_SRCALPHASAT:     d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHASAT);     RS.MEM_RSTATE_DESTBLEND = MEM; return;
				case _STATE_BLEND_BOTHSRCALPHA:    d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_BOTHSRCALPHA);    RS.MEM_RSTATE_DESTBLEND = MEM; return;
				case _STATE_BLEND_BOTHINVSRCALPHA: d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_BOTHINVSRCALPHA); RS.MEM_RSTATE_DESTBLEND = MEM; return;
				}
				break;
			}
			/////////////////////////////
			}

			_MBM(L"Missing SetRenderState() type of state");
		}
		virtual void PreviousRenderState(eRenderState RSTATE)          override final
		{
			eRenderState MEM = RSTATE_NONE;
			switch (RSTATE)
			{
			case RSTATE_CULLMODE:         MEM = RS.MEM_RSTATE_CULLMODE;         break;
			case RSTATE_LIGHTING:         MEM = RS.MEM_RSTATE_LIGHTING;         break;
			case RSTATE_FOGENABLE:        MEM = RS.MEM_RSTATE_FOGENABLE;        break;
			case RSTATE_COLORVERTEX:      MEM = RS.MEM_RSTATE_COLORVERTEX;      break;
			case RSTATE_ALPHABLENDENABLE: MEM = RS.MEM_RSTATE_ALPHABLENDENABLE; break;
			case RSTATE_ZENABLE:          MEM = RS.MEM_RSTATE_ZENABLE;          break;
			case RSTATE_ZWRITEENABLE:     MEM = RS.MEM_RSTATE_ZWRITEENABLE;     break;
			case RSTATE_STENCILENABLE:    MEM = RS.MEM_RSTATE_STENCILENABLE;    break;
			case RSTATE_CLIPPLANEENABLE:  MEM = RS.MEM_RSTATE_CLIPPLANEENABLE;  break;
			case RSTATE_BLENDOP:          MEM = RS.MEM_RSTATE_BLENDOP;          break;
			case RSTATE_SRCBLEND:         MEM = RS.MEM_RSTATE_SRCBLEND;         break;
			case RSTATE_DESTBLEND:        MEM = RS.MEM_RSTATE_DESTBLEND;        break;
			}
			if (MEM == RSTATE_NONE) return;
												RS.restore_memory_call = true;
			SetRenderState(RSTATE, MEM);		RS.restore_memory_call = false;
		}
	public:
		virtual void SendToRender(const DataManagerModel       * modeldata, uint32 start_vertex_offset_in_bytes, uint32 sz_vrt_struct)       override final
		{
			const DataManagerModelDX9 * model = reinterpret_cast<const DataManagerModelDX9*>(modeldata);

			d3ddev->SetStreamSource(CONCURRENT_DATA_STREAM_default, model->vbuf, start_vertex_offset_in_bytes, sz_vrt_struct);
			d3ddev->SetIndices(model->ibuf);
		}
		virtual void SendToRender(const DataManagerTexture     * texturedata, uint32 stage)                                                  override final
		{
			if (texturedata == nullptr)
				 d3ddev->SetTexture(stage, nullptr);
			else d3ddev->SetTexture(stage, reinterpret_cast<const DataManagerTextureDX9*>(texturedata)->tex);
		}
		virtual void SendToRender(const DataManagerCubeTexture * texturedata, uint32 stage)                                                  override final
		{
			if (texturedata == nullptr)
				 d3ddev->SetTexture(stage, nullptr);
			else d3ddev->SetTexture(stage, reinterpret_cast<const DataManagerCubeTextureDX9*>(texturedata)->tex);
		}
		virtual shared_ptr<DataManagerModel> CreateModelData(const void* vrt, const uint32* idc, uint32 vrtNum, uint32 idcNum, uint32 szvrt) override final
		{
			shared_ptr<DataManagerModelDX9> model = make_shared<DataManagerModelDX9>();
			model->LoadData(d3ddev, vrt, idc, vrtNum, idcNum, szvrt);		
			return static_pointer_cast<DataManagerModel>(model);
		}
		virtual shared_ptr<DataManagerTexture> CreateTextureData(const void* texbuf, uint32 texsize)                                         override final
		{
			shared_ptr<DataManagerTextureDX9> texture = make_shared<DataManagerTextureDX9>();
			texture->LoadData(d3ddev, texbuf, texsize);
			return static_pointer_cast<DataManagerTexture>(texture);
		}
		virtual shared_ptr<DataManagerCubeTexture> CreateCubeTextureData(const void* texbuf, uint32 texsize)                                 override final
		{
			shared_ptr<DataManagerCubeTextureDX9> texture = make_shared<DataManagerCubeTextureDX9>();
			texture->LoadData(d3ddev, texbuf, texsize);
			return static_pointer_cast<DataManagerCubeTexture>(texture);
		}
	public:
		virtual bool CheckPresentationInterval(eIntervalData present) override final
		{
			switch (present)
			{
			case EID_INTERVAL_IMMEDIATE: return adapter_caps.presentationInterval.immediate;
			case EID_INTERVAL_ONE:       return adapter_caps.presentationInterval.one;
			case EID_INTERVAL_TWO:       return adapter_caps.presentationInterval.two;
			case EID_INTERVAL_THREE:     return adapter_caps.presentationInterval.three;
			case EID_INTERVAL_FOUR:      return adapter_caps.presentationInterval.four;
			default:                     return false;
			}
		}
	public:
		// TODO: Проверка индексов на MaxVertexIndex при загрузке модели
		virtual eDeviceError CheckDevice() override final
		{
			if (d3dcaps == nullptr)
			{
				_MBM(L"CheckDevice() failed cause of d3dcaps is nullptr");
				return DEVERROR_NONE;
			}

			adapter_caps.maxVertexIndex = d3dcaps->MaxVertexIndex;
			adapter_caps.presentationInterval.immediate = _BOOL(d3dcaps->PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE);
			adapter_caps.presentationInterval.one       = _BOOL(d3dcaps->PresentationIntervals & D3DPRESENT_INTERVAL_ONE);
			adapter_caps.presentationInterval.two       = _BOOL(d3dcaps->PresentationIntervals & D3DPRESENT_INTERVAL_TWO);
			adapter_caps.presentationInterval.three     = _BOOL(d3dcaps->PresentationIntervals & D3DPRESENT_INTERVAL_THREE);
			adapter_caps.presentationInterval.four      = _BOOL(d3dcaps->PresentationIntervals & D3DPRESENT_INTERVAL_FOUR);

			if (!(d3dcaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT))
			{ _MBM(ERROR_InitDeviceHVP); return DEVERROR_HVP; }
		
			if (adapter_caps.maxVertexIndex <= 0xFFFF)
			{
				_MBM(ERROR_IndexBuf32);
				return DEVERROR_IDXBF32;
			}

			if (!adapter_caps.presentationInterval.four)						{
				if (!adapter_caps.presentationInterval.three)					{
					if (!adapter_caps.presentationInterval.two)					{
						if (!adapter_caps.presentationInterval.one)				{
							if (!adapter_caps.presentationInterval.immediate)
							{
								_MBM(ERROR_PresentInterv0);
								return DEVERROR_PRESENTINT0;
							}
							else {	/*_MBM(ERROR_PresentInterv1);*/
									/*return DEVERROR_PRESENTINT1;*/ }										}
						else {	/*_MBM(ERROR_PresentInterv2);*/
								/*return DEVERROR_PRESENTINT2;*/ }											}
					else {	/*_MBM(ERROR_PresentInterv3);*/
							/*return DEVERROR_PRESENTINT3;*/ }												}
				else {	/*_MBM(ERROR_PresentInterv4); */
						/*return DEVERROR_PRESENTINT4;*/ }													}

			// Подумать над прочими проверками

			// PrimitiveMiscCaps ?
			// ZCmpCaps, AlphaCmpCaps ?
			// SrcBlendCaps, DestBlendCaps ?
			// TextureFilterCaps, TextureAddressCaps ?
			// StencilCaps ?
			// MaxTextureWidth, MaxTextureHeight, MaxAnisotropy ?
			// MaxStreams, MaxStreamStride ?

			return DEVERROR_NONE;
		}	
		virtual void PrintfDeviceCaps() override final
		{
			if (d3dcaps != nullptr)
			{
				printf("\n__Device capabilities table__");

				switch (d3dcaps->DeviceType)
				{
				case D3DDEVTYPE_HAL:     printf("\nDeviceType = D3DDEVTYPE_HAL");     break;
				case D3DDEVTYPE_NULLREF: printf("\nDeviceType = D3DDEVTYPE_NULLREF"); break;
				case D3DDEVTYPE_REF:     printf("\nDeviceType = D3DDEVTYPE_REF");     break;
				case D3DDEVTYPE_SW:      printf("\nDeviceType = D3DDEVTYPE_SW");      break;
				default: printf("\nDeviceType = %i (unknown)", d3dcaps->DeviceType);  break;
				}

				printf("\nAdapterOrdinal          = %i", d3dcaps->AdapterOrdinal);
				printf("\nMasterAdapterOrdinal    = %i", d3dcaps->MasterAdapterOrdinal);
				printf("\nAdapterOrdinalInGroup   = %i", d3dcaps->AdapterOrdinalInGroup);
				printf("\nNumberOfAdaptersInGroup = %i", d3dcaps->NumberOfAdaptersInGroup);

				printf("\nCAPS (%08X):", d3dcaps->Caps);
				printf("\n- D3DCAPS_READ_SCANLINE = %s", _BOOLYESNO(d3dcaps->Caps & D3DCAPS_READ_SCANLINE));
				printf("\n- D3DCAPS_OVERLAY       = %s", _BOOLYESNO(d3dcaps->Caps & D3DCAPS_OVERLAY));

				printf("\nCAPS2 (%08X):", d3dcaps->Caps2);
				printf("\n- D3DCAPS2_CANAUTOGENMIPMAP  = %s", _BOOLYESNO(d3dcaps->Caps2 & D3DCAPS2_CANAUTOGENMIPMAP));
				printf("\n- D3DCAPS2_CANCALIBRATEGAMMA = %s", _BOOLYESNO(d3dcaps->Caps2 & D3DCAPS2_CANCALIBRATEGAMMA));
				printf("\n- D3DCAPS2_CANSHARERESOURCE  = %s", _BOOLYESNO(d3dcaps->Caps2 & D3DCAPS2_CANSHARERESOURCE));
				printf("\n- D3DCAPS2_CANMANAGERESOURCE = %s", _BOOLYESNO(d3dcaps->Caps2 & D3DCAPS2_CANMANAGERESOURCE));
				printf("\n- D3DCAPS2_DYNAMICTEXTURES   = %s", _BOOLYESNO(d3dcaps->Caps2 & D3DCAPS2_DYNAMICTEXTURES));
				printf("\n- D3DCAPS2_FULLSCREENGAMMA   = %s", _BOOLYESNO(d3dcaps->Caps2 & D3DCAPS2_FULLSCREENGAMMA));

				printf("\nCAPS3 (%08X):", d3dcaps->Caps3);
				printf("\n- D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD = %s", _BOOLYESNO(d3dcaps->Caps3 & D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD));
				printf("\n- D3DCAPS3_COPY_TO_VIDMEM    = %s", _BOOLYESNO(d3dcaps->Caps3 & D3DCAPS3_COPY_TO_VIDMEM));
				printf("\n- D3DCAPS3_COPY_TO_SYSTEMMEM = %s", _BOOLYESNO(d3dcaps->Caps3 & D3DCAPS3_COPY_TO_SYSTEMMEM));
				printf("\n- D3DCAPS3_DXVAHD            = %s", _BOOLYESNO(d3dcaps->Caps3 & D3DCAPS3_DXVAHD));
				printf("\n- D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION = %s", _BOOLYESNO(d3dcaps->Caps3 & D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION));
				
				printf("\nPresentation intervals supporting (%08X):", d3dcaps->PresentationIntervals);
				printf("\n- IMMEDIATE      = %s", _BOOLYESNO(d3dcaps->PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE));
				printf("\n- INTERVAL_ONE   = %s", _BOOLYESNO(d3dcaps->PresentationIntervals & D3DPRESENT_INTERVAL_ONE));
				printf("\n- INTERVAL_TWO   = %s", _BOOLYESNO(d3dcaps->PresentationIntervals & D3DPRESENT_INTERVAL_TWO));
				printf("\n- INTERVAL_THREE = %s", _BOOLYESNO(d3dcaps->PresentationIntervals & D3DPRESENT_INTERVAL_THREE));
				printf("\n- INTERVAL_FOUR  = %s", _BOOLYESNO(d3dcaps->PresentationIntervals & D3DPRESENT_INTERVAL_FOUR));
				
				printf("\nCursor caps (%08X):", d3dcaps->CursorCaps);
				printf("\n- D3DCURSORCAPS_COLOR  = %s", _BOOLYESNO(d3dcaps->CursorCaps & D3DCURSORCAPS_COLOR));
				printf("\n- D3DCURSORCAPS_LOWRES = %s", _BOOLYESNO(d3dcaps->CursorCaps & D3DCURSORCAPS_LOWRES));

				printf("\nDevice caps (%08X):", d3dcaps->DevCaps);
				printf("\n- D3DDEVCAPS_CANBLTSYSTONONLOCAL = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_CANBLTSYSTONONLOCAL));
				printf("\n- D3DDEVCAPS_CANRENDERAFTERFLIP  = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_CANRENDERAFTERFLIP));
				printf("\n- D3DDEVCAPS_DRAWPRIMITIVES2     = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_DRAWPRIMITIVES2));
				printf("\n- D3DDEVCAPS_DRAWPRIMITIVES2EX   = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_DRAWPRIMITIVES2EX));
				printf("\n- D3DDEVCAPS_DRAWPRIMTLVERTEX    = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_DRAWPRIMTLVERTEX));
				printf("\n- D3DDEVCAPS_EXECUTESYSTEMMEMORY = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_EXECUTESYSTEMMEMORY));
				printf("\n- D3DDEVCAPS_EXECUTEVIDEOMEMORY  = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_EXECUTEVIDEOMEMORY));
				printf("\n- D3DDEVCAPS_HWRASTERIZATION     = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_HWRASTERIZATION));
				printf("\n- D3DDEVCAPS_HWTRANSFORMANDLIGHT = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT));
				printf("\n- D3DDEVCAPS_NPATCHES            = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_NPATCHES));
				printf("\n- D3DDEVCAPS_PUREDEVICE          = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_PUREDEVICE));
				printf("\n- D3DDEVCAPS_QUINTICRTPATCHES    = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_QUINTICRTPATCHES));
				printf("\n- D3DDEVCAPS_RTPATCHES           = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_RTPATCHES));
				printf("\n- D3DDEVCAPS_RTPATCHHANDLEZERO   = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_RTPATCHHANDLEZERO));
				printf("\n- D3DDEVCAPS_SEPARATETEXTUREMEMORIES = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES));
				printf("\n- D3DDEVCAPS_TEXTURENONLOCALVIDMEM   = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_TEXTURENONLOCALVIDMEM));
				printf("\n- D3DDEVCAPS_TEXTURESYSTEMMEMORY     = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_TEXTURESYSTEMMEMORY));
				printf("\n- D3DDEVCAPS_TEXTUREVIDEOMEMORY      = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_TEXTUREVIDEOMEMORY));
				printf("\n- D3DDEVCAPS_TLVERTEXSYSTEMMEMORY    = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_TLVERTEXSYSTEMMEMORY));
				printf("\n- D3DDEVCAPS_TLVERTEXVIDEOMEMORY     = %s", _BOOLYESNO(d3dcaps->DevCaps & D3DDEVCAPS_TLVERTEXVIDEOMEMORY));

				printf("\nDevice caps 2 (%08X):", d3dcaps->DevCaps2);
				printf("\n- D3DDEVCAPS2_ADAPTIVETESSRTPATCH  = %s", _BOOLYESNO(d3dcaps->DevCaps2 & D3DDEVCAPS2_ADAPTIVETESSRTPATCH));
				printf("\n- D3DDEVCAPS2_ADAPTIVETESSNPATCH   = %s", _BOOLYESNO(d3dcaps->DevCaps2 & D3DDEVCAPS2_ADAPTIVETESSNPATCH));
				printf("\n- D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES = %s", _BOOLYESNO(d3dcaps->DevCaps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES));
				printf("\n- D3DDEVCAPS2_DMAPNPATCH           = %s", _BOOLYESNO(d3dcaps->DevCaps2 & D3DDEVCAPS2_DMAPNPATCH));
				printf("\n- D3DDEVCAPS2_PRESAMPLEDDMAPNPATCH = %s", _BOOLYESNO(d3dcaps->DevCaps2 & D3DDEVCAPS2_PRESAMPLEDDMAPNPATCH));
				printf("\n- D3DDEVCAPS2_STREAMOFFSET         = %s", _BOOLYESNO(d3dcaps->DevCaps2 & D3DDEVCAPS2_STREAMOFFSET));
				printf("\n- D3DDEVCAPS2_VERTEXELEMENTSCANSHARESTREAMOFFSET = %s", _BOOLYESNO(d3dcaps->DevCaps2 & D3DDEVCAPS2_VERTEXELEMENTSCANSHARESTREAMOFFSET));

				printf("\nPrimitive misc caps (%08X):", d3dcaps->PrimitiveMiscCaps);
				printf("\n- D3DPMISCCAPS_MASKZ       = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_MASKZ));
				printf("\n- D3DPMISCCAPS_CULLNONE    = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_CULLNONE));
				printf("\n- D3DPMISCCAPS_CULLCW      = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_CULLCW));
				printf("\n- D3DPMISCCAPS_CULLCCW     = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_CULLCCW));
				printf("\n- D3DPMISCCAPS_COLORWRITEENABLE      = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_COLORWRITEENABLE));
				printf("\n- D3DPMISCCAPS_CLIPPLANESCALEDPOINTS = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_CLIPPLANESCALEDPOINTS));
				printf("\n- D3DPMISCCAPS_CLIPTLVERTS = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_CLIPTLVERTS));
				printf("\n- D3DPMISCCAPS_TSSARGTEMP  = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_TSSARGTEMP));
				printf("\n- D3DPMISCCAPS_BLENDOP     = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_BLENDOP));
				printf("\n- D3DPMISCCAPS_NULLREFERENCE         = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_NULLREFERENCE));
				printf("\n- D3DPMISCCAPS_INDEPENDENTWRITEMASKS = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_INDEPENDENTWRITEMASKS));
				printf("\n- D3DPMISCCAPS_PERSTAGECONSTANT      = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_PERSTAGECONSTANT));
				printf("\n- D3DPMISCCAPS_POSTBLENDSRGBCONVERT  = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_POSTBLENDSRGBCONVERT));
				printf("\n- D3DPMISCCAPS_FOGANDSPECULARALPHA   = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_FOGANDSPECULARALPHA));
				printf("\n- D3DPMISCCAPS_SEPARATEALPHABLEND    = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_SEPARATEALPHABLEND));
				printf("\n- D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS    = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS));
				printf("\n- D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING));
				printf("\n- D3DPMISCCAPS_FOGVERTEXCLAMPED = %s", _BOOLYESNO(d3dcaps->PrimitiveMiscCaps & D3DPMISCCAPS_FOGVERTEXCLAMPED));
				
				printf("\nRaster-drawing caps (%08X):", d3dcaps->RasterCaps);
				printf("\n- D3DPRASTERCAPS_ANISOTROPY       = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_ANISOTROPY));
				printf("\n- D3DPRASTERCAPS_COLORPERSPECTIVE = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_COLORPERSPECTIVE));
				printf("\n- D3DPRASTERCAPS_DITHER        = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_DITHER));
				printf("\n- D3DPRASTERCAPS_DEPTHBIAS     = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_DEPTHBIAS));
				printf("\n- D3DPRASTERCAPS_FOGRANGE      = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_FOGRANGE));
				printf("\n- D3DPRASTERCAPS_FOGTABLE      = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_FOGTABLE));
				printf("\n- D3DPRASTERCAPS_FOGVERTEX     = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_FOGVERTEX));
				printf("\n- D3DPRASTERCAPS_MIPMAPLODBIAS = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS));
				printf("\n- D3DPRASTERCAPS_MULTISAMPLE_TOGGLE  = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_MULTISAMPLE_TOGGLE));
				printf("\n- D3DPRASTERCAPS_SCISSORTEST         = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_SCISSORTEST));
				printf("\n- D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS));
				printf("\n- D3DPRASTERCAPS_WBUFFER        = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_ANISOTROPY));
				printf("\n- D3DPRASTERCAPS_WFOG           = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_WFOG));
				printf("\n- D3DPRASTERCAPS_ZBUFFERLESSHSR = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR));
				printf("\n- D3DPRASTERCAPS_ZFOG           = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_ZFOG));
				printf("\n- D3DPRASTERCAPS_ZTEST          = %s", _BOOLYESNO(d3dcaps->RasterCaps & D3DPRASTERCAPS_ZTEST));

				printf("\nZ-buffer / Alpha-test comparison caps (%08X / %08X):", d3dcaps->ZCmpCaps, d3dcaps->AlphaCmpCaps);
				printf("\n- D3DPCMPCAPS_ALWAYS       = %s / %s", _BOOLYESNO(d3dcaps->ZCmpCaps     & D3DPCMPCAPS_ALWAYS),
					                                             _BOOLYESNO(d3dcaps->AlphaCmpCaps & D3DPCMPCAPS_ALWAYS));
				printf("\n- D3DPCMPCAPS_EQUAL        = %s / %s", _BOOLYESNO(d3dcaps->ZCmpCaps     & D3DPCMPCAPS_EQUAL),
					                                             _BOOLYESNO(d3dcaps->AlphaCmpCaps & D3DPCMPCAPS_EQUAL));
				printf("\n- D3DPCMPCAPS_GREATER      = %s / %s", _BOOLYESNO(d3dcaps->ZCmpCaps     & D3DPCMPCAPS_GREATER),
					                                             _BOOLYESNO(d3dcaps->AlphaCmpCaps & D3DPCMPCAPS_GREATER));
				printf("\n- D3DPCMPCAPS_GREATEREQUAL = %s / %s", _BOOLYESNO(d3dcaps->ZCmpCaps     & D3DPCMPCAPS_GREATEREQUAL),
					                                             _BOOLYESNO(d3dcaps->AlphaCmpCaps & D3DPCMPCAPS_GREATEREQUAL));
				printf("\n- D3DPCMPCAPS_LESS         = %s / %s", _BOOLYESNO(d3dcaps->ZCmpCaps     & D3DPCMPCAPS_LESS),
					                                             _BOOLYESNO(d3dcaps->AlphaCmpCaps & D3DPCMPCAPS_LESS));
				printf("\n- D3DPCMPCAPS_LESSEQUAL    = %s / %s", _BOOLYESNO(d3dcaps->ZCmpCaps     & D3DPCMPCAPS_LESSEQUAL),
					                                             _BOOLYESNO(d3dcaps->AlphaCmpCaps & D3DPCMPCAPS_LESSEQUAL));
				printf("\n- D3DPCMPCAPS_NEVER        = %s / %s", _BOOLYESNO(d3dcaps->ZCmpCaps     & D3DPCMPCAPS_NEVER),
					                                             _BOOLYESNO(d3dcaps->AlphaCmpCaps & D3DPCMPCAPS_NEVER));
				printf("\n- D3DPCMPCAPS_NOTEQUAL     = %s / %s", _BOOLYESNO(d3dcaps->ZCmpCaps     & D3DPCMPCAPS_NOTEQUAL),
					                                             _BOOLYESNO(d3dcaps->AlphaCmpCaps & D3DPCMPCAPS_NOTEQUAL));

				printf("\nBlending source / destination caps (%08X / %08X):", d3dcaps->SrcBlendCaps, d3dcaps->DestBlendCaps);
				printf("\n- D3DPBLENDCAPS_BLENDFACTOR     = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_BLENDFACTOR),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_BLENDFACTOR));
				printf("\n- D3DPBLENDCAPS_BOTHINVSRCALPHA = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_BOTHINVSRCALPHA),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_BOTHINVSRCALPHA));
				printf("\n- D3DPBLENDCAPS_BOTHSRCALPHA    = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_BOTHSRCALPHA),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_BOTHSRCALPHA));
				printf("\n- D3DPBLENDCAPS_DESTALPHA       = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_DESTALPHA),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_DESTALPHA));
				printf("\n- D3DPBLENDCAPS_DESTCOLOR       = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_DESTCOLOR),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_DESTCOLOR));
				printf("\n- D3DPBLENDCAPS_INVDESTALPHA    = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_INVDESTALPHA),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_INVDESTALPHA));
				printf("\n- D3DPBLENDCAPS_INVDESTCOLOR    = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_INVDESTCOLOR),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_INVDESTCOLOR));
				printf("\n- D3DPBLENDCAPS_INVSRCALPHA     = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_INVSRCALPHA),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA));
				printf("\n- D3DPBLENDCAPS_INVSRCCOLOR     = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_INVSRCCOLOR),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR));
				printf("\n- D3DPBLENDCAPS_INVSRCCOLOR2    = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_INVSRCCOLOR2),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR2));
				printf("\n- D3DPBLENDCAPS_ONE             = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_ONE),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_ONE));
				printf("\n- D3DPBLENDCAPS_SRCALPHA        = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_SRCALPHA),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_SRCALPHA));
				printf("\n- D3DPBLENDCAPS_SRCALPHASAT     = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_SRCALPHASAT),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_SRCALPHASAT));
				printf("\n- D3DPBLENDCAPS_SRCCOLOR        = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_SRCCOLOR),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_SRCCOLOR));
				printf("\n- D3DPBLENDCAPS_SRCCOLOR2       = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_SRCCOLOR2),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_SRCCOLOR2));
				printf("\n- D3DPBLENDCAPS_ZERO            = %s / %s", _BOOLYESNO(d3dcaps->SrcBlendCaps  & D3DPBLENDCAPS_ZERO),
					                                                  _BOOLYESNO(d3dcaps->DestBlendCaps & D3DPBLENDCAPS_ZERO));
				
				printf("\nMiscellaneous texture-mapping caps (%08X):", d3dcaps->TextureCaps);
				printf("\n- D3DPTEXTURECAPS_ALPHA        = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_ALPHA));
				printf("\n- D3DPTEXTURECAPS_ALPHAPALETTE = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE));
				printf("\n- D3DPTEXTURECAPS_CUBEMAP      = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_CUBEMAP));
				printf("\n- D3DPTEXTURECAPS_CUBEMAP_POW2 = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_CUBEMAP_POW2));
				printf("\n- D3DPTEXTURECAPS_MIPCUBEMAP   = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_MIPCUBEMAP));
				printf("\n- D3DPTEXTURECAPS_MIPMAP       = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_MIPMAP));
				printf("\n- D3DPTEXTURECAPS_MIPVOLUMEMAP = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_MIPVOLUMEMAP));
				printf("\n- D3DPTEXTURECAPS_NONPOW2CONDITIONAL = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL));
				printf("\n- D3DPTEXTURECAPS_NOPROJECTEDBUMPENV = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_NOPROJECTEDBUMPENV));
				printf("\n- D3DPTEXTURECAPS_PERSPECTIVE  = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_PERSPECTIVE));
				printf("\n- D3DPTEXTURECAPS_POW2         = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_POW2));
				printf("\n- D3DPTEXTURECAPS_PROJECTED    = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_PROJECTED));
				printf("\n- D3DPTEXTURECAPS_SQUAREONLY   = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_SQUAREONLY));
				printf("\n- D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE));
				printf("\n- D3DPTEXTURECAPS_VOLUMEMAP      = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP));
				printf("\n- D3DPTEXTURECAPS_VOLUMEMAP_POW2 = %s", _BOOLYESNO(d3dcaps->TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP_POW2));

				printf("\nTexture-filtering caps VSHADER / 2D / CUBE / VOLUME:", d3dcaps->VertexTextureFilterCaps,
					                                                             d3dcaps->TextureFilterCaps,
					                                                             d3dcaps->CubeTextureFilterCaps,
					                                                             d3dcaps->VolumeTextureFilterCaps);
				printf("\n- D3DPTFILTERCAPS_CONVOLUTIONMONO   = %s / %s / %s / %s", _BOOLYESNO(d3dcaps->VertexTextureFilterCaps & D3DPTFILTERCAPS_CONVOLUTIONMONO),
					                                                                _BOOLYESNO(d3dcaps->TextureFilterCaps       & D3DPTFILTERCAPS_CONVOLUTIONMONO),
																					_BOOLYESNO(d3dcaps->CubeTextureFilterCaps   & D3DPTFILTERCAPS_CONVOLUTIONMONO),
																					_BOOLYESNO(d3dcaps->VolumeTextureFilterCaps & D3DPTFILTERCAPS_CONVOLUTIONMONO));
				printf("\n- D3DPTFILTERCAPS_MAGFPOINT         = %s / %s / %s / %s", _BOOLYESNO(d3dcaps->VertexTextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT),
					                                                                _BOOLYESNO(d3dcaps->TextureFilterCaps       & D3DPTFILTERCAPS_MAGFPOINT),
																					_BOOLYESNO(d3dcaps->CubeTextureFilterCaps   & D3DPTFILTERCAPS_MAGFPOINT),
																					_BOOLYESNO(d3dcaps->VolumeTextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT));
				printf("\n- D3DPTFILTERCAPS_MAGFLINEAR        = %s / %s / %s / %s", _BOOLYESNO(d3dcaps->VertexTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR),
					                                                                _BOOLYESNO(d3dcaps->TextureFilterCaps       & D3DPTFILTERCAPS_MAGFLINEAR),
																					_BOOLYESNO(d3dcaps->CubeTextureFilterCaps   & D3DPTFILTERCAPS_MAGFLINEAR),
																					_BOOLYESNO(d3dcaps->VolumeTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR));
				printf("\n- D3DPTFILTERCAPS_MAGFANISOTROPIC   = %s / %s / %s / %s", _BOOLYESNO(d3dcaps->VertexTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC),
					                                                                _BOOLYESNO(d3dcaps->TextureFilterCaps       & D3DPTFILTERCAPS_MAGFANISOTROPIC),
																					_BOOLYESNO(d3dcaps->CubeTextureFilterCaps   & D3DPTFILTERCAPS_MAGFANISOTROPIC),
																					_BOOLYESNO(d3dcaps->VolumeTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC));
				printf("\n- D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD = %s / %s / %s / %s", _BOOLYESNO(d3dcaps->VertexTextureFilterCaps & D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD),
					                                                                _BOOLYESNO(d3dcaps->TextureFilterCaps       & D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD),
																					_BOOLYESNO(d3dcaps->CubeTextureFilterCaps   & D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD),
																					_BOOLYESNO(d3dcaps->VolumeTextureFilterCaps & D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD));
				printf("\n- D3DPTFILTERCAPS_MAGFGAUSSIANQUAD  = %s / %s / %s / %s", _BOOLYESNO(d3dcaps->VertexTextureFilterCaps & D3DPTFILTERCAPS_MAGFGAUSSIANQUAD),
					                                                                _BOOLYESNO(d3dcaps->TextureFilterCaps       & D3DPTFILTERCAPS_MAGFGAUSSIANQUAD),
																					_BOOLYESNO(d3dcaps->CubeTextureFilterCaps   & D3DPTFILTERCAPS_MAGFGAUSSIANQUAD),
																					_BOOLYESNO(d3dcaps->VolumeTextureFilterCaps & D3DPTFILTERCAPS_MAGFGAUSSIANQUAD));
				printf("\n- D3DPTFILTERCAPS_MINFPOINT         = %s / %s / %s / %s", _BOOLYESNO(d3dcaps->VertexTextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT),
					                                                                _BOOLYESNO(d3dcaps->TextureFilterCaps       & D3DPTFILTERCAPS_MINFPOINT),
																					_BOOLYESNO(d3dcaps->CubeTextureFilterCaps   & D3DPTFILTERCAPS_MINFPOINT),
																					_BOOLYESNO(d3dcaps->VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT));
				printf("\n- D3DPTFILTERCAPS_MINFLINEAR        = %s / %s / %s / %s", _BOOLYESNO(d3dcaps->VertexTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR),
					                                                                _BOOLYESNO(d3dcaps->TextureFilterCaps       & D3DPTFILTERCAPS_MINFLINEAR),
																					_BOOLYESNO(d3dcaps->CubeTextureFilterCaps   & D3DPTFILTERCAPS_MINFLINEAR),
																					_BOOLYESNO(d3dcaps->VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR));
				printf("\n- D3DPTFILTERCAPS_MINFANISOTROPIC   = %s / %s / %s / %s", _BOOLYESNO(d3dcaps->VertexTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC),
					                                                                _BOOLYESNO(d3dcaps->TextureFilterCaps       & D3DPTFILTERCAPS_MINFANISOTROPIC),
																					_BOOLYESNO(d3dcaps->CubeTextureFilterCaps   & D3DPTFILTERCAPS_MINFANISOTROPIC),
																					_BOOLYESNO(d3dcaps->VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC));
				printf("\n- D3DPTFILTERCAPS_MINFPYRAMIDALQUAD = %s / %s / %s / %s", _BOOLYESNO(d3dcaps->VertexTextureFilterCaps & D3DPTFILTERCAPS_MINFPYRAMIDALQUAD),
					                                                                _BOOLYESNO(d3dcaps->TextureFilterCaps       & D3DPTFILTERCAPS_MINFPYRAMIDALQUAD),
																					_BOOLYESNO(d3dcaps->CubeTextureFilterCaps   & D3DPTFILTERCAPS_MINFPYRAMIDALQUAD),
																					_BOOLYESNO(d3dcaps->VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFPYRAMIDALQUAD));
				printf("\n- D3DPTFILTERCAPS_MINFGAUSSIANQUAD  = %s / %s / %s / %s", _BOOLYESNO(d3dcaps->VertexTextureFilterCaps & D3DPTFILTERCAPS_MINFGAUSSIANQUAD),
					                                                                _BOOLYESNO(d3dcaps->TextureFilterCaps       & D3DPTFILTERCAPS_MINFGAUSSIANQUAD),
					                                                                _BOOLYESNO(d3dcaps->CubeTextureFilterCaps   & D3DPTFILTERCAPS_MINFGAUSSIANQUAD),
																			        _BOOLYESNO(d3dcaps->VolumeTextureFilterCaps & D3DPTFILTERCAPS_MINFGAUSSIANQUAD));
				printf("\n- D3DPTFILTERCAPS_MIPFPOINT         = %s / %s / %s / %s", _BOOLYESNO(d3dcaps->VertexTextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT),
					                                                                _BOOLYESNO(d3dcaps->TextureFilterCaps       & D3DPTFILTERCAPS_MIPFPOINT),
					                                                                _BOOLYESNO(d3dcaps->CubeTextureFilterCaps   & D3DPTFILTERCAPS_MIPFPOINT),
																			        _BOOLYESNO(d3dcaps->VolumeTextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT));
				printf("\n- D3DPTFILTERCAPS_MIPFLINEAR        = %s / %s / %s / %s", _BOOLYESNO(d3dcaps->VertexTextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR),
					                                                                _BOOLYESNO(d3dcaps->TextureFilterCaps       & D3DPTFILTERCAPS_MIPFLINEAR),
					                                                                _BOOLYESNO(d3dcaps->CubeTextureFilterCaps   & D3DPTFILTERCAPS_MIPFLINEAR),
																			        _BOOLYESNO(d3dcaps->VolumeTextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR));

				printf("\nTexture-addressing caps 2D / VOLUME (%08X / %08X):", d3dcaps->TextureAddressCaps,
					                                                           d3dcaps->VolumeTextureAddressCaps);
				printf("\n- D3DPTADDRESSCAPS_BORDER        = %s / %s", _BOOLYESNO(d3dcaps->TextureAddressCaps       & D3DPTADDRESSCAPS_BORDER),
					                                                   _BOOLYESNO(d3dcaps->VolumeTextureAddressCaps & D3DPTADDRESSCAPS_BORDER));
				printf("\n- D3DPTADDRESSCAPS_CLAMP         = %s / %s", _BOOLYESNO(d3dcaps->TextureAddressCaps       & D3DPTADDRESSCAPS_CLAMP),
					                                                   _BOOLYESNO(d3dcaps->VolumeTextureAddressCaps & D3DPTADDRESSCAPS_CLAMP));
				printf("\n- D3DPTADDRESSCAPS_INDEPENDENTUV = %s / %s", _BOOLYESNO(d3dcaps->TextureAddressCaps       & D3DPTADDRESSCAPS_INDEPENDENTUV),
					                                                   _BOOLYESNO(d3dcaps->VolumeTextureAddressCaps & D3DPTADDRESSCAPS_INDEPENDENTUV));
				printf("\n- D3DPTADDRESSCAPS_MIRROR        = %s / %s", _BOOLYESNO(d3dcaps->TextureAddressCaps       & D3DPTADDRESSCAPS_MIRROR),
					                                                   _BOOLYESNO(d3dcaps->VolumeTextureAddressCaps & D3DPTADDRESSCAPS_MIRROR));
				printf("\n- D3DPTADDRESSCAPS_MIRRORONCE    = %s / %s", _BOOLYESNO(d3dcaps->TextureAddressCaps       & D3DPTADDRESSCAPS_MIRRORONCE),
					                                                   _BOOLYESNO(d3dcaps->VolumeTextureAddressCaps & D3DPTADDRESSCAPS_MIRRORONCE));
				printf("\n- D3DPTADDRESSCAPS_WRAP          = %s / %s", _BOOLYESNO(d3dcaps->TextureAddressCaps       & D3DPTADDRESSCAPS_WRAP),
					                                                   _BOOLYESNO(d3dcaps->VolumeTextureAddressCaps & D3DPTADDRESSCAPS_WRAP));

				printf("\nTexture operations caps (%08X):", d3dcaps->TextureOpCaps);
				printf("\n- D3DTEXOPCAPS_ADD         = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_ADD));
				printf("\n- D3DTEXOPCAPS_ADDSIGNED   = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_ADDSIGNED));
				printf("\n- D3DTEXOPCAPS_ADDSIGNED2X = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_ADDSIGNED2X));
				printf("\n- D3DTEXOPCAPS_ADDSMOOTH   = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_ADDSMOOTH));
				printf("\n- D3DTEXOPCAPS_BLENDCURRENTALPHA   = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_BLENDCURRENTALPHA));
				printf("\n- D3DTEXOPCAPS_BLENDDIFFUSEALPHA   = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_BLENDDIFFUSEALPHA));
				printf("\n- D3DTEXOPCAPS_BLENDFACTORALPHA    = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_BLENDFACTORALPHA));
				printf("\n- D3DTEXOPCAPS_BLENDTEXTUREALPHA   = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_BLENDTEXTUREALPHA));
				printf("\n- D3DTEXOPCAPS_BLENDTEXTUREALPHAPM = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_BLENDTEXTUREALPHAPM));
				printf("\n- D3DTEXOPCAPS_BUMPENVMAP          = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP));
				printf("\n- D3DTEXOPCAPS_BUMPENVMAPLUMINANCE = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAPLUMINANCE));
				printf("\n- D3DTEXOPCAPS_DISABLE     = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_DISABLE));
				printf("\n- D3DTEXOPCAPS_DOTPRODUCT3 = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3));
				printf("\n- D3DTEXOPCAPS_LERP        = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_LERP));
				printf("\n- D3DTEXOPCAPS_MODULATE    = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_MODULATE));
				printf("\n- D3DTEXOPCAPS_MODULATE2X  = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_MODULATE2X));
				printf("\n- D3DTEXOPCAPS_MODULATE4X  = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_MODULATE4X));
				printf("\n- D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR    = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR));
				printf("\n- D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA    = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA));
				printf("\n- D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR));
				printf("\n- D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA));
				printf("\n- D3DTEXOPCAPS_MULTIPLYADD = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_MULTIPLYADD));
				printf("\n- D3DTEXOPCAPS_PREMODULATE = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_PREMODULATE));
				printf("\n- D3DTEXOPCAPS_SELECTARG1  = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_SELECTARG1));
				printf("\n- D3DTEXOPCAPS_SELECTARG2  = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_SELECTARG2));
				printf("\n- D3DTEXOPCAPS_SUBTRACT    = %s", _BOOLYESNO(d3dcaps->TextureOpCaps & D3DTEXOPCAPS_SUBTRACT));

				printf("\nLine-drawing primitives caps (%08X):", d3dcaps->LineCaps);
				printf("\n- D3DLINECAPS_ALPHACMP  = %s", _BOOLYESNO(d3dcaps->LineCaps & D3DLINECAPS_ALPHACMP));
				printf("\n- D3DLINECAPS_ANTIALIAS = %s", _BOOLYESNO(d3dcaps->LineCaps & D3DLINECAPS_ANTIALIAS));
				printf("\n- D3DLINECAPS_BLEND     = %s", _BOOLYESNO(d3dcaps->LineCaps & D3DLINECAPS_BLEND));
				printf("\n- D3DLINECAPS_FOG       = %s", _BOOLYESNO(d3dcaps->LineCaps & D3DLINECAPS_FOG));
				printf("\n- D3DLINECAPS_TEXTURE   = %s", _BOOLYESNO(d3dcaps->LineCaps & D3DLINECAPS_TEXTURE));
				printf("\n- D3DLINECAPS_ZTEST     = %s", _BOOLYESNO(d3dcaps->LineCaps & D3DLINECAPS_ZTEST));

				printf("\nMaxTextureWidth       = %i", d3dcaps->MaxTextureWidth);
				printf("\nMaxTextureHeight      = %i", d3dcaps->MaxTextureHeight);
				printf("\nMaxVolumeExtent       = %i", d3dcaps->MaxVolumeExtent);
				printf("\nMaxTextureRepeat      = %i", d3dcaps->MaxTextureRepeat);
				printf("\nMaxTextureAspectRatio = %i", d3dcaps->MaxTextureAspectRatio);
				printf("\nMaxAnisotropy         = %i", d3dcaps->MaxAnisotropy);

				printf("\nMaxVertexW      = %f", d3dcaps->MaxVertexW);
				printf("\nGuardBandLeft   = %f", d3dcaps->GuardBandLeft);
				printf("\nGuardBandTop    = %f", d3dcaps->GuardBandTop);
				printf("\nGuardBandRight  = %f", d3dcaps->GuardBandRight);
				printf("\nGuardBandBottom = %f", d3dcaps->GuardBandBottom);
				printf("\nExtentsAdjust   = %f", d3dcaps->ExtentsAdjust);

				printf("\nStencil caps (%08X):", d3dcaps->StencilCaps);
				printf("\n- D3DSTENCILCAPS_KEEP     = %s", _BOOLYESNO(d3dcaps->StencilCaps & D3DSTENCILCAPS_KEEP));
				printf("\n- D3DSTENCILCAPS_ZERO     = %s", _BOOLYESNO(d3dcaps->StencilCaps & D3DSTENCILCAPS_ZERO));
				printf("\n- D3DSTENCILCAPS_REPLACE  = %s", _BOOLYESNO(d3dcaps->StencilCaps & D3DSTENCILCAPS_REPLACE));
				printf("\n- D3DSTENCILCAPS_INCRSAT  = %s", _BOOLYESNO(d3dcaps->StencilCaps & D3DSTENCILCAPS_INCRSAT));
				printf("\n- D3DSTENCILCAPS_DECRSAT  = %s", _BOOLYESNO(d3dcaps->StencilCaps & D3DSTENCILCAPS_DECRSAT));
				printf("\n- D3DSTENCILCAPS_INVERT   = %s", _BOOLYESNO(d3dcaps->StencilCaps & D3DSTENCILCAPS_INVERT));
				printf("\n- D3DSTENCILCAPS_INCR     = %s", _BOOLYESNO(d3dcaps->StencilCaps & D3DSTENCILCAPS_INCR));
				printf("\n- D3DSTENCILCAPS_DECR     = %s", _BOOLYESNO(d3dcaps->StencilCaps & D3DSTENCILCAPS_DECR));
				printf("\n- D3DSTENCILCAPS_TWOSIDED = %s", _BOOLYESNO(d3dcaps->StencilCaps & D3DSTENCILCAPS_TWOSIDED));

				printf("\nFlexible vertex format FVF caps (%08X):", d3dcaps->FVFCaps);
				printf("\n- D3DFVFCAPS_DONOTSTRIPELEMENTS = %s", _BOOLYESNO(d3dcaps->FVFCaps & D3DFVFCAPS_DONOTSTRIPELEMENTS));
				printf("\n- D3DFVFCAPS_PSIZE              = %s", _BOOLYESNO(d3dcaps->FVFCaps & D3DFVFCAPS_PSIZE));
				printf("\n- D3DFVFCAPS_TEXCOORDCOUNTMASK  = %s", _BOOLYESNO(d3dcaps->FVFCaps & D3DFVFCAPS_TEXCOORDCOUNTMASK));

				printf("\nVertex processing caps (%08X):", d3dcaps->VertexProcessingCaps);
				printf("\n- D3DVTXPCAPS_DIRECTIONALLIGHTS = %s", _BOOLYESNO(d3dcaps->VertexProcessingCaps & D3DVTXPCAPS_DIRECTIONALLIGHTS));
				printf("\n- D3DVTXPCAPS_LOCALVIEWER       = %s", _BOOLYESNO(d3dcaps->VertexProcessingCaps & D3DVTXPCAPS_LOCALVIEWER));
				printf("\n- D3DVTXPCAPS_MATERIALSOURCE7   = %s", _BOOLYESNO(d3dcaps->VertexProcessingCaps & D3DVTXPCAPS_MATERIALSOURCE7));
				printf("\n- D3DVTXPCAPS_NO_TEXGEN_NONLOCALVIEWER = %s", _BOOLYESNO(d3dcaps->VertexProcessingCaps & D3DVTXPCAPS_NO_TEXGEN_NONLOCALVIEWER));
				printf("\n- D3DVTXPCAPS_POSITIONALLIGHTS  = %s", _BOOLYESNO(d3dcaps->VertexProcessingCaps & D3DVTXPCAPS_POSITIONALLIGHTS));
				printf("\n- D3DVTXPCAPS_TEXGEN            = %s", _BOOLYESNO(d3dcaps->VertexProcessingCaps & D3DVTXPCAPS_TEXGEN));
				printf("\n- D3DVTXPCAPS_TEXGEN_SPHEREMAP  = %s", _BOOLYESNO(d3dcaps->VertexProcessingCaps & D3DVTXPCAPS_TEXGEN_SPHEREMAP));
				printf("\n- D3DVTXPCAPS_TWEENING          = %s", _BOOLYESNO(d3dcaps->VertexProcessingCaps & D3DVTXPCAPS_TWEENING));

				printf("\nMaxTextureBlendStages   = %i : Max.text.registers.(pshader)", d3dcaps->MaxTextureBlendStages);
				printf("\nMaxSimultaneousTextures = %i : no meaning", d3dcaps->MaxSimultaneousTextures);
				printf("\nMaxActiveLights         = %i : no meaning", d3dcaps->MaxActiveLights);
				printf("\nMaxUserClipPlanes       = %i", d3dcaps->MaxUserClipPlanes);
				printf("\nMaxVertexBlendMatrices    = %i", d3dcaps->MaxVertexBlendMatrices);
				printf("\nMaxVertexBlendMatrixIndex = %i", d3dcaps->MaxVertexBlendMatrixIndex);
				printf("\nMaxPointSize      = %f", d3dcaps->MaxPointSize);
				printf("\nMaxPrimitiveCount = %X : no meaning if 0xFFFF", d3dcaps->MaxPrimitiveCount);
				printf("\nMaxVertexIndex    = %X", d3dcaps->MaxVertexIndex);
				printf("\nMaxStreams        = %i : SetStreamSource(N, ...)", d3dcaps->MaxStreams);
				printf("\nMaxStreamStride   = %i : SetStreamSource(..., sz_vrt_struct_size) ", d3dcaps->MaxStreamStride);

				// pixel shader version token
				//#define D3DPS_VERSION(_Major,_Minor) (0xFFFF0000|((_Major)<<8)|(_Minor))
				// vertex shader version token
				//#define D3DVS_VERSION(_Major,_Minor) (0xFFFE0000|((_Major)<<8)|(_Minor))

				printf("\nVertexShaderVersion = %8X = %02i.%02i",  d3dcaps->VertexShaderVersion,
					                                              (d3dcaps->VertexShaderVersion & 0x0000FF00) >> 8,
					                                               d3dcaps->VertexShaderVersion & 0x000000FF);
				printf("\nPixelShaderVersion  = %8X = %02i.%02i",  d3dcaps->PixelShaderVersion,
					                                              (d3dcaps->PixelShaderVersion  & 0x0000FF00) >> 8,
					                                               d3dcaps->PixelShaderVersion  & 0x000000FF);

				printf("\nMaxVertexShaderConst       = %i", d3dcaps->MaxVertexShaderConst);
				printf("\nPixelShader1xMaxValue      = %f", d3dcaps->PixelShader1xMaxValue);
				printf("\nMaxNpatchTessellationLevel = %i", d3dcaps->MaxNpatchTessellationLevel);
				printf("\nNum simultaneous render targets = %i", d3dcaps->NumSimultaneousRTs);
				printf("\nMaxVShaderInstructionsExecuted    = %i", d3dcaps->MaxVShaderInstructionsExecuted);
				printf("\nMaxPShaderInstructionsExecuted    = %i", d3dcaps->MaxPShaderInstructionsExecuted);
				printf("\nMaxVertexShader30InstructionSlots = %i", d3dcaps->MaxVertexShader30InstructionSlots);
				printf("\nMaxPixelShader30InstructionSlots  = %i", d3dcaps->MaxPixelShader30InstructionSlots);
					
				printf("\nSupported vertex data types (%08X):", d3dcaps->DeclTypes);
				printf("\n- D3DDTCAPS_UBYTE4    = %s", _BOOLYESNO(d3dcaps->DeclTypes & D3DDTCAPS_UBYTE4));
				printf("\n- D3DDTCAPS_UBYTE4N   = %s", _BOOLYESNO(d3dcaps->DeclTypes & D3DDTCAPS_UBYTE4N));
				printf("\n- D3DDTCAPS_SHORT2N   = %s", _BOOLYESNO(d3dcaps->DeclTypes & D3DDTCAPS_SHORT2N));
				printf("\n- D3DDTCAPS_SHORT4N   = %s", _BOOLYESNO(d3dcaps->DeclTypes & D3DDTCAPS_SHORT4N));
				printf("\n- D3DDTCAPS_USHORT2N  = %s", _BOOLYESNO(d3dcaps->DeclTypes & D3DDTCAPS_USHORT2N));
				printf("\n- D3DDTCAPS_USHORT4N  = %s", _BOOLYESNO(d3dcaps->DeclTypes & D3DDTCAPS_USHORT4N));
				printf("\n- D3DDTCAPS_UDEC3     = %s", _BOOLYESNO(d3dcaps->DeclTypes & D3DDTCAPS_UDEC3));
				printf("\n- D3DDTCAPS_DEC3N     = %s", _BOOLYESNO(d3dcaps->DeclTypes & D3DDTCAPS_DEC3N));
				printf("\n- D3DDTCAPS_FLOAT16_2 = %s", _BOOLYESNO(d3dcaps->DeclTypes & D3DDTCAPS_FLOAT16_2));
				printf("\n- D3DDTCAPS_FLOAT16_4 = %s", _BOOLYESNO(d3dcaps->DeclTypes & D3DDTCAPS_FLOAT16_4));

				printf("\nCaps of StretchRect() method (%08X):", d3dcaps->StretchRectFilterCaps);
				printf("\n- D3DPTFILTERCAPS_MINFPOINT  = %s", _BOOLYESNO(d3dcaps->StretchRectFilterCaps & D3DPTFILTERCAPS_MINFPOINT));
				printf("\n- D3DPTFILTERCAPS_MAGFPOINT  = %s", _BOOLYESNO(d3dcaps->StretchRectFilterCaps & D3DPTFILTERCAPS_MAGFPOINT));
				printf("\n- D3DPTFILTERCAPS_MINFLINEAR = %s", _BOOLYESNO(d3dcaps->StretchRectFilterCaps & D3DPTFILTERCAPS_MINFLINEAR));
				printf("\n- D3DPTFILTERCAPS_MAGFLINEAR = %s", _BOOLYESNO(d3dcaps->StretchRectFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR));

				printf("\nExtended capabilities of vertex shader version 2_0:");
				printf("\n- Caps     = %i", d3dcaps->VS20Caps.Caps);		
				printf("\n- NumTemps = %i", d3dcaps->VS20Caps.NumTemps);
				printf("\n- DynamicFlowControlDepth = %i", d3dcaps->VS20Caps.DynamicFlowControlDepth);
				printf("\n- StaticFlowControlDepth  = %i", d3dcaps->VS20Caps.StaticFlowControlDepth);

				printf("\nExtended capabilities of pixel  shader version 2_0:");
				printf("\n- Caps     = %i", d3dcaps->PS20Caps.Caps);
				printf("\n- NumTemps = %i", d3dcaps->PS20Caps.NumTemps);
				printf("\n- DynamicFlowControlDepth = %i", d3dcaps->PS20Caps.DynamicFlowControlDepth);
				printf("\n- StaticFlowControlDepth  = %i", d3dcaps->PS20Caps.StaticFlowControlDepth);
				printf("\n- NumInstructionSlots     = %i", d3dcaps->PS20Caps.NumInstructionSlots);
				
				/*
				The MaxTextureBlendStages and MaxSimultaneousTextures members might seem similar,
				but they contain different information. The MaxTextureBlendStages member contains
				the total number of texture-blending stages supported by the current device, and
				the MaxSimultaneousTextures member describes how many of those stages can have
				textures bound to them by using the SetTexture method.

				When the driver fills this structure, it can set values for execute-buffer
				capabilities, even when the interface being used to retrieve the capabilities
				(such as IDirect3DDevice9) does not support execute buffers.

				In general, performance problems may occur if you use a texture and then modify
				it during a scene. Ensure that no texture used in the current BeginScene and
				EndScene block is evicted unless absolutely necessary. In the case of extremely
				high texture usage within a scene, the results are undefined. This occurs when
				you modify a texture that you have used in the scene and there is no spare
				texture memory available. For such systems, the contents of the z-buffer become
				invalid at EndScene. Applications should not call UpdateSurface to or from the
				back buffer on this type of hardware inside a BeginScene/EndScene pair.
				In addition, applications should not try to access the z-buffer if the
				D3DPRASTERCAPS_ZBUFFERLESSHSR capability flag is set. Finally, applications
				should not lock the back buffer or the z-buffer inside a BeginScene/EndScene pair.

				The following flags concerning mipmapped textures are not supported in Direct3D 9.

				D3DPTFILTERCAPS_LINEAR
				D3DPTFILTERCAPS_LINEARMIPLINEAR
				D3DPTFILTERCAPS_LINEARMIPNEAREST
				D3DPTFILTERCAPS_MIPNEAREST
				D3DPTFILTERCAPS_NEAREST
				*/
			}
		}
		//////////
		IDirectT *				d3d_p;		// direct3d interface
		IDeviceT *				d3ddev;		// videocard interface
		IDevcapT *				d3dcaps;	// videocard properties
		UINT					d3dadapter; // D3DADAPTER_DEFAULT = using now
		D3DDEVTYPE				d3dtype;	// D3DDEVTYPE_HAL = using hardware (not CPU software)
		//////////
		DWORD *					value;		// temp val
		D3DMATERIAL9 *			material;	// temp material
	protected:
		virtual void Default() override final
		{
			d3d_p                = nullptr;
			d3ddev               = nullptr;
			d3dcaps              = nullptr;
			d3dadapter           = D3DADAPTER_DEFAULT;
			d3dtype              = D3DDEVTYPE_HAL;
			PresentationInterval = EID_VSYNC_OFF;
		}
	};
}

#endif // _DEVICE3D9_H