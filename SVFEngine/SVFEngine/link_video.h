#ifndef _LINKVIDEO_H
#define _LINKVIDEO_H

#include "link_defines.h"

#if   defined (DIRECTX_9)
//	#error DIRECTX_9 support has been removed
	#include "RenderDX9.h"
	#define  CRenderF CGameRenderDX9  // Class Render Final
#elif defined (DIRECTX_11)
	#error DIRECTX_11 not presented yet
	#include "RenderDX11.h"
	#define CRenderF CGameRenderDX11  // Class Render Final
#elif defined (DIRECTX_12)
	#error DIRECTX_12 not presented yet
	#include "RenderDX10.h"
	#define CRenderF CGameRenderDX12  // Class Render Final
#elif defined (OPENGL_46)
	#error OPENGL_4.6 not presented yet
	#include "RenderOpenGL46.h"
	#define CRenderF CGameRenderO46   // Class Render Final
#endif

#define CRenderT CRenderF

//////////////////////////////////

#endif // _LINKVIDEO_H