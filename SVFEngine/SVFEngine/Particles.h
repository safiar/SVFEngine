// ----------------------------------------------------------------------- //
//
// MODULE  : Particles.h
//
// PURPOSE : Генератор частиц
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _PARTICLES_H
#define _PARTICLES_H

#include "device3d.h"

namespace SAVFGAME
{
	class CParticles // TODO
	{	
	public:
		CParticles(){};
		~CParticles(){};
	public:
		CParticles(const CParticles& src)				= delete;
		CParticles(CParticles&& src)					= delete;
		CParticles& operator=(CParticles&& src)			= delete;
		CParticles& operator=(const CParticles& src)	= delete;
	};
};

#endif // _PARTICLES_H