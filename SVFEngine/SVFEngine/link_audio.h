#ifndef _LINKAUDIO_H
#define _LINKAUDIO_H

#include "link_defines.h"

#ifdef WINDOWS_IMM_WASAPI
	#include "SoundIMM.h"
    #define	CSoundF	CSoundWASAPI // Class Sound Final
#endif

#define  CSoundT  CSoundF

#endif //_LINKAUDIO_H