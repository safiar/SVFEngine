// ----------------------------------------------------------------------- //
//
// MODULE  : device3d11.h
//
// PURPOSE : Вспомогательная оболочка 3D девайса (DX11)
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _DEVICE3D11_H
#define _DEVICE3D11_H

#include "device3d.h"

#include <d3d11.h>
#include <d3dx11.h>
#pragma comment (lib, "d3d11.lib")
#ifdef _DEBUG
#	pragma comment (lib, "d3dx11d.lib")
#else
#	pragma comment (lib, "d3dx11.lib")
#endif

//#define  IDirectT   IDirect3D11
//#define  IDeviceT   IDirect3DDevice11
//#define  IDevcapT   D3DCAPS11
//#define  D3DPARAM   D3DPRESENT_PARAMETERS

namespace SAVFGAME
{
	class DEV3DDAT : public DEV3DBASE // videodevice DEV3DBASE : directx 11 implementation
	{
		// TODO
	};
}

#endif // _DEVICE3D11_H
