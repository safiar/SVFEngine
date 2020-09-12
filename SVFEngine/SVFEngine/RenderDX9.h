// ----------------------------------------------------------------------- //
//
// MODULE  : RenderDX9.h
//
// PURPOSE : Рендер средствами DirectX 9
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _RENDERDX9_H
#define _RENDERDX9_H

// TODO: мультирендер в разные участки окна /comics style/

#include "device3d9.h"
#include "Render.h"
#include "World.h"

// The maximum number of samplers is determined from two caps: MaxSimultaneousTextures and MaxTextureBlendStages of the D3DCAPS9 structure.

//ID3DXMesh *				mesh;
//ID3DXFont *				dxfont;
//IDirect3DSurface9 *       surf;

namespace SAVFGAME
{
	class CGameRenderDX9 final : public CRender
	{
	private:
		DEV3DDAT				_;
	protected:
		D3DPARAM				d3dpp;				// 
		IDirectT *				d3d    { nullptr };	// 
		IDeviceT *				d3ddev { nullptr };	// copy [dev->d3ddev]	
		DEV3DDAT * const		dev    { &_ };		// manage d3ddev & d3dcaps
	public:
		CWorld					world;
	public:
		CGameRenderDX9(const CGameRenderDX9& src)				= delete;
		CGameRenderDX9(CGameRenderDX9&& src)					= delete;
		CGameRenderDX9& operator=(CGameRenderDX9&& src)			= delete;
		CGameRenderDX9& operator=(const CGameRenderDX9& src)	= delete;
	public:
		CGameRenderDX9() : CRender() { };
		~CGameRenderDX9() override final { Close(); };

		//>> 
		bool Init(const HWND hWnd, const CStatusWindow * pState) override final
		{
			if (pState == nullptr) { _MBM(ERROR_PointerNone); return false; }
			if (hWnd   == nullptr) { _MBM(ERROR_PointerNone); return false; }

			hwnd  = const_cast<HWND>(hWnd);
			state = const_cast<CStatusWindow*>(pState);

			int width  = state->client_width;
			int height = state->client_height;
			auto mode  = state->mode;

			if (!(d3d = Direct3DCreate9(D3D_SDK_VERSION)))	// get interface
				{ _MBM(ERROR_InitDirect3D); return false; }

			UINT		adapter		= D3DADAPTER_DEFAULT;	// using default (using Windows now)
			D3DDEVTYPE	deviceType	= D3DDEVTYPE_HAL;		// using hardware (not CPU software)

			dev->Init(d3d, adapter, deviceType);
			dev->SetPresentationInterval(EID_VSYNC_OFF);
			dev->SetBackBuffersNum(ONE);
			dev->SetDesktopResolution(state->desktop_horizontal, state->desktop_vertical);

			if (dev->CheckDevice() != DEVERROR_NONE)		// check device
				{ _RELEASE(d3d); return false; }

			ZeroMemory(&d3dpp, sizeof(D3DPARAM));

			// Swap effect must be D3DSWAPEFFECT_DISCARD for multi-sampling support.
			d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// antialiasing : D3DSWAPEFFECT_COPY -> D3DSWAPEFFECT_DISCARD
			d3dpp.hDeviceWindow = hWnd;

			if (mode == WM_FULLSCREEN)
			{
				d3dpp.Windowed = false;
				//d3dpp.MultiSampleQuality = dev->GetSamplesFullscreen(true);					// antialiasing
				d3dpp.MultiSampleType = (D3DMULTISAMPLE_TYPE) dev->GetSamplesFullscreen(true);	// antialiasing
			}
			else
			{
				d3dpp.Windowed = true;
				//d3dpp.MultiSampleQuality = dev->GetSamplesWindowed(true);						// antialiasing
				d3dpp.MultiSampleType = (D3DMULTISAMPLE_TYPE) dev->GetSamplesWindowed(true);	// antialiasing
			}

			// CHECK THIS INFO ( ? )
			// If you've done this and you're using the old GDI stuff, it's not your vsync setting that's wrong, but the window settings.
			// You must enable double buffering or you'll still get tearing.
			// You cannot vsync while in windowed, only in fullscreen.

			d3dpp.PresentationInterval = dev->GetPresentationInterval(); // VSync
			d3dpp.Flags = 0;
			
			d3dpp.BackBufferFormat = (D3DFORMAT) dev->GetDisplay().format;

			if (mode == WM_FULLSCREEN) d3dpp.FullScreen_RefreshRateInHz = dev->GetDisplay().refreshRate;
			else					   d3dpp.FullScreen_RefreshRateInHz = 0;

			d3dpp.BackBufferWidth  = width;
			d3dpp.BackBufferHeight = height;
			d3dpp.BackBufferCount  = dev->GetBackBuffersNum();

			d3dpp.EnableAutoDepthStencil =             dev->GetDisplay().dstencil.enableAuto;
			d3dpp.AutoDepthStencilFormat = (D3DFORMAT) dev->GetDisplay().dstencil.format;

			// create a device class using this information and information from the d3dpp struct

			int hRes = d3d->CreateDevice (
							adapter,
							deviceType,
							hWnd,
							D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE, //precision for Direct3D floating-point calculations
							&d3dpp,
							&dev->d3ddev );

			if (hRes != D3D_OK) // D3DERR_INVALIDCALL = 8876086c
			{
				wchar_t error[256];
				swprintf_s(error, L"%s\nERR = %x", ERROR_InitDevice, (int)hRes);
				_MBM(error);
				return false;
			}

			d3ddev = dev->d3ddev; // set up quick ptr

			dev->SetViewPort(0, 0, width, height, 0, 1);

			Default();

			world.RunInitLoading(dev, pState);

			return true;
		};
		//>> 
		void Default()
		{
			// Настройки default DX9 pipeline (на случай, если свои шейдеры выключены)
			if (1)
			{
				//d3ddev->SetFVF(MODELFVF);
				d3ddev->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);

				// Смешивание при участии цветов вершин в просчёте
				d3ddev->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1); // _MATERIAL  _COLOR1  _COLOR2
				d3ddev->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
				d3ddev->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR1);
				d3ddev->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR1);
				d3ddev->SetRenderState(D3DRS_COLORVERTEX, FALSE);
			} //*/

			// Состояние рендера по умолчанию

			d3ddev->SetRenderState(D3DRS_CULLMODE, CULL_DEF);
			d3ddev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
			d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			d3ddev->SetRenderState(D3DRS_STENCILENABLE, FALSE);
			d3ddev->SetRenderState(D3DRS_CLIPPLANEENABLE, FALSE);
			d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);

			//d3ddev->SetRenderState(D3DRS_FOGENABLE, FALSE); // туман : default DX9 pipeline

			//d3ddev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);			// альфа-тест включён  : default DX9 pipeline
			//d3ddev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL); // метод проверки      : default DX9 pipeline
			//d3ddev->SetRenderState(D3DRS_ALPHAREF, 0x08);					// значение к методу   : default DX9 pipeline
		 
			d3ddev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);  // D3DFILL_SOLID норм, D3DFILL_WIREFRAME сетка, D3DFILL_POINT вершины

			d3ddev->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);  // СГЛАЖИВАНИЕ 
			d3ddev->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE); // СГЛАЖИВАНИЕ
			//d3ddev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);		// включение равномерной заливки треугольника
			//d3ddev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);	// включение заливки по Гуро

			d3ddev->SetSamplerState(IDX_TEX_DIFFUSE, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);			// _POINT, _LINEAR
			d3ddev->SetSamplerState(IDX_TEX_DIFFUSE, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
			d3ddev->SetSamplerState(IDX_TEX_DIFFUSE, D3DSAMP_MAXANISOTROPY, dev->d3dcaps->MaxAnisotropy);
			d3ddev->SetSamplerState(IDX_TEX_DIFFUSE, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);			// _POINT, _LINEAR, _PYRAMIDALQUAD, _GAUSSIANQUAD
			d3ddev->SetSamplerState(IDX_TEX_DIFFUSE, D3DSAMP_MAXMIPLEVEL, 0);

			//D3DSAMP_DMAPOFFSET
			//Vertex offset in the presampled displacement map. This is a constant used by the tessellator, its default value is 0.

			d3ddev->SetSamplerState(IDX_TEX_DIFFUSE, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP); // _WRAP повтор, _MIRROR повтор с отражением, _CLAMP цвет uv(0,0)..(1,1), _BORDER
			d3ddev->SetSamplerState(IDX_TEX_DIFFUSE, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP); //
			d3ddev->SetSamplerState(IDX_TEX_DIFFUSE, D3DSAMP_BORDERCOLOR, D3DCOLOR_ARGB(255,255,255,255));  // D3DTADDRESS_BORDER

			//d3ddev->SetRenderState(D3DRS_CULLMODE, CULL_NON);
		}
		//>> 
		bool Reset(const HWND hWnd, const CStatusWindow * pState) override final
		{
			// All video memory must be released before a device can be reset from a lost state to an operational state.
			// This means that the application should release any swap chains created with IDirect3DDevice9::CreateAdditionalSwapChain
			// and any resources placed in the D3DPOOL_DEFAULT memory class. The application need not release resources in the
			// D3DPOOL_MANAGED or D3DPOOL_SYSTEMMEM memory classes. Other state data is automatically destroyed by the transition
			// to an operational state.

			// MultiSampleType
			// Member of the D3DMULTISAMPLE_TYPE enumerated type.The value must be D3DMULTISAMPLE_NONE unless SwapEffect has been set to
			// D3DSWAPEFFECT_DISCARD.Multisampling is supported only if the swap effect is D3DSWAPEFFECT_DISCARD.

			// MultiSampleQuality
			// Quality level.The valid range is between zero and one less than the level returned by pQualityLevels used by
			// CheckDeviceMultiSampleType.Passing a larger value returns the error D3DERR_INVALIDCALL.Paired values of render targets or of
			// depth stencil surfaces and D3DMULTISAMPLE_TYPE must match.

			if (hWnd   != nullptr) hwnd  = const_cast<HWND>(hWnd);
			if (pState != nullptr) state = const_cast<CStatusWindow*>(pState);

			int width  = state->client_width;	if (!width)  width  = state->sys_width;
			int height = state->client_height;  if (!height) height = state->sys_height;
			auto mode  = state->mode;

			world.to_render.GetBackbuffer(width, height);

			d3dpp.BackBufferWidth		= width;
			d3dpp.BackBufferHeight		= height;
			d3dpp.BackBufferCount		= dev->GetBackBuffersNum();
			d3dpp.PresentationInterval	= dev->GetPresentationInterval();	// VSync

			printf("\nX %i Y %i (render back buffer size)", d3dpp.BackBufferWidth, d3dpp.BackBufferHeight);

			if (mode == WM_FULLSCREEN)
			{
				d3dpp.Windowed = false;
			//	d3dpp.MultiSampleQuality = dev->GetSamplesFullscreen(true);						// antialiasing
				d3dpp.MultiSampleType = (D3DMULTISAMPLE_TYPE)dev->GetSamplesFullscreen(true);	// antialiasing
				d3dpp.FullScreen_RefreshRateInHz = dev->GetDisplay().refreshRate;
			}
			else
			{
				d3dpp.Windowed = true;
			//	d3dpp.MultiSampleQuality = dev->GetSamplesWindowed(true);						// antialiasing
				d3dpp.MultiSampleType = (D3DMULTISAMPLE_TYPE)dev->GetSamplesWindowed(true);		// antialiasing
				d3dpp.FullScreen_RefreshRateInHz = 0;
			}

			world.OnLostDevice();

			switch (d3ddev->Reset(&d3dpp))
			{
			case D3D_OK: break;
			case D3DERR_INVALIDCALL: _MBM(ERROR_ResetDeviceINV); return false;
			default:                // _MBM(ERROR_ResetDevice);    return false;
				{
					for (uint64 i=1;;i++)
					{
						_SLEEP(1000);
						if (d3ddev->Reset(&d3dpp) == D3D_OK)
							break;
						else
						{
							if (!(i % 15)) printf("\nRENDER : Can't reset D3D device! (%i sec.)", (int)i);
						}
					}
				}
			};

			//dev->SetViewPort(0, 0, rwm.GetClientWidth(), rwm.GetClientHeight(), 0, 1);

			world.UpdateWindow(pState);

			world.OnResetDevice();
			//world.CallSkipFrame();

			return true;
		}
		//>> 
		bool Update() override final
		{	
			world.UpdateWindow();

			return true;
		}
		//>> 
		bool Run() override final
		{
			d3ddev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
			d3ddev->BeginScene();

			//d3ddev->SetCursorPosition(width/2, height/2, D3DCURSOR_IMMEDIATE_UPDATE);
			//d3ddev->ShowCursor(false);

			world.Run();

			d3ddev->EndScene();
			if (d3ddev->Present(NULL, NULL, NULL, NULL) != D3D_OK)
			{
				// LOST DEVICE (например, при ctrl+alt+del)
				to_framework.ResetRenderCall();
			}

			////////////////////////////////////////////// Update : world -> render -> framework

			if (world.to_render.ExitGameCheck()) return false;

			to_framework.show_cursor     = world.to_render.show_cursor;
			to_framework.centered_cursor = world.to_render.centered_cursor;

			if (world.to_render.CursorMoveCheck())
			{
				to_framework.x = world.to_render.x;
				to_framework.y = world.to_render.y;
				to_framework.CursorMoveCall();
			}	

			if (world.to_render.WindowResetCheck())
				to_framework.WindowResetCall();

			if (world.to_render.WindowMinimizeCheck())
				to_framework.WindowMinimizeCall();

			return true;
		};
		//>> 
		void Close() override final
		{
			world.Close();		// erase CWorld	
			d3ddev = nullptr;	// clear copy from DEV3DDAT
			_RELEASE(d3d);		// release interface
		};

	};
}

#endif // _RENDERDX9_H