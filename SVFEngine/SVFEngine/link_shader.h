#ifndef _LINKSHADER_H
#define _LINKSHADER_H

#include "link_defines.h"

#if   defined (DIRECTX_9)
//	#error DIRECTX_9 support has been removed
	#include "ShaderDX9.h"
    #define CShaderF CShaderDX9   // Class Shader Final	
#elif defined (DIRECTX_11)
	#error DIRECTX_11 not presented yet
	#include "ShaderDX11.h"
	#define CShaderF CShaderDX11  // Class Shader Final	
#elif defined (DIRECTX_12)
	#error DIRECTX_12 not presented yet
	#include "ShaderDX12.h"
	#define CShaderF CShaderDX12  // Class Shader Final	
#elif defined (OPENGL_46)
	#error OPENGL_4.6 not presented yet
	#include "ShaderOpenGL46.h"
	#define CShaderF CShaderO46   // Class Shader Final
#endif

#define CShaderT CShaderF

#endif //_LINKSHADER_H