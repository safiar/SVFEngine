#ifndef _LINKNETWORK_H
#define _LINKNETWORK_H

#include "link_defines.h"

#ifdef WINDOWS_WINSOCK2
	#include "NetworkWS2.h"
	#define CNetworkF CNetworkWS2  // Class Network Final
#endif

#define  CNetworkT  CNetworkF

#endif //_LINKNETWORK_H