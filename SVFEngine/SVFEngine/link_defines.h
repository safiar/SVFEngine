#ifndef _LINKDEFINES_H
#define _LINKDEFINES_H

////////////////////////////////// VIDEO             OPENGL 2.0..2.1, 3.0..3.3, 4.0..4.6  ;  Vulkan

/**/
#define DIRECTX_9		/*
#define DIRECTX_11		/*
#define DIRECTX_12		/*
#define OPENGL_46		/*  
#define UNKNOWN_API		*/

#if   defined (DIRECTX_9)
	#define  ISDIRECTX9  true
	#define  ISDIRECTX11 false
	#define  ISDIRECTX12 false
	#define  ISOPENGL46  false
#elif defined (DIRECTX_11)
	#define  ISDIRECTX9  false
	#define  ISDIRECTX11 true
	#define  ISDIRECTX12 false
	#define  ISOPENGL46  false
#elif defined (DIRECTX_12)
	#define  ISDIRECTX9  false
	#define  ISDIRECTX11 false
	#define  ISDIRECTX12 true
	#define  ISOPENGL46  false
#elif defined (OPENGL_46)
	#define  ISDIRECTX9  false
	#define  ISDIRECTX11 false
	#define  ISDIRECTX12 false
	#define  ISOPENGL46  true
#endif

////////////////////////////////// AUDIO

							/**/
#define WINDOWS_IMM_WASAPI	/*
#define UNKNOWN_API			*/

////////////////////////////////// NETWORK

//#define _WINSOCK_DEPRECATED_NO_WARNINGS

							/**/
#define WINDOWS_WINSOCK2	/*
#define UNKNOWN_API			*/

//////////////////////////////////

#endif // _LINKDEFINES_H