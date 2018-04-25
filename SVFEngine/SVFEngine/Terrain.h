// ----------------------------------------------------------------------- //
//
// MODULE  : Terrain.h
//
// PURPOSE : Генератор поверхностей
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _TERRAIN_H
#define _TERRAIN_H

#include "device3d.h"

namespace SAVFGAME
{
	class CTerrain // TODO
	{
	public:
		CTerrain(){};
		~CTerrain(){};
	public:
		CTerrain(const CTerrain& src)				= delete;
		CTerrain(CTerrain&& src)					= delete;
		CTerrain& operator=(CTerrain&& src)			= delete;
		CTerrain& operator=(const CTerrain& src)	= delete;
	};
};

#endif // _TERRAIN_H