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
		virtual ~DEV3DDAT()
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
			InitAdapterIdentifier(d3d, adapter);
			InitDeviceCaps(d3d, adapter, type);
			InitSamplesInfo(d3d, adapter, type);
			InitBackBufferFormat(d3d, adapter, type);
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
				   nSamples_windowed = 1;
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
				   nSamples_fullscreen = 1;
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

			d3ddev->SetStreamSource(0, model->vbuf, start_vertex_offset_in_bytes, sz_vrt_struct);
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
		virtual eDeviceError CheckDevice() override final
		{
			if (d3dcaps == nullptr) return DEVERROR_NONE;

			if (!(d3dcaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT))
			{ _MBM(ERROR_InitDeviceHVP); return DEVERROR_HVP; }

			// D3DFMT_INDEX32,	 // 16, 32   проверить возможности девайса !

			return DEVERROR_NONE;
		}	
		//////////
		IDeviceT *				d3ddev;		// videocard interface
		IDevcapT *				d3dcaps;	// videocard properties
		DWORD *					value;		// temp val
		D3DMATERIAL9 *			material;	// temp material
	protected:
		virtual void Default() override final
		{
			d3ddev = nullptr;
			d3dcaps = nullptr;
			PresentationInterval = EID_VSYNC_ON;
		}
	};
}

#endif // _DEVICE3D9_H