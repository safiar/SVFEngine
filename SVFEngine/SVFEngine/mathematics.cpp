// ----------------------------------------------------------------------- //
//
// MODULE  : mathematics.cpp
//
// PURPOSE : Вспомогательные математические функции и определения
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#include "mathematics.h"

using namespace SAVFGAME;

namespace SAVFGAME
{
	float CTAB::sinAngleTab [ANGLES_NUM];
	float CTAB::cosAngleTab [ANGLES_NUM];
	float CTAB::tanAngleTab [ANGLES_NUM];
	bool  CTAB::isInit = false;

	void CTAB::Init()
	{
		if (isInit) return;

		int idx = 0;
		for (float i=0; idx < ANGLES_NUM; i+=ANGLES_STEP, idx++)
		{
			float angle = TORADIANS(i);
			CTAB::sinAngleTab[idx]  = sin(angle);
			CTAB::cosAngleTab[idx]  = cos(angle);
			CTAB::tanAngleTab[idx]  = tan(angle);
		}

		isInit = true;
	};

	float CTAB::sinA(float angle)
	{
		if (angle > 360 || angle < -360)
			angle -= 360.f * static_cast<int>(angle * (1.f/360));
		if (angle < 0)
			angle += 360.f;
		return CTAB::sinAngleTab[static_cast<int>(angle * ANGLES_ISTEP)];
	}
	float CTAB::cosA(float angle)
	{
		if (angle > 360 || angle < -360)
			angle -= 360.f * static_cast<int>(angle * (1.f/360));
		if (angle < 0)
			angle += 360.f;
		return CTAB::cosAngleTab[static_cast<int>(angle * ANGLES_ISTEP)];
	}
	float CTAB::tanA(float angle)
	{
		if (angle > 360 || angle < -360)
			angle -= 360.f * static_cast<int>(angle * (1.f/360));
		if (angle < 0)
			angle += 360.f;
		return CTAB::tanAngleTab[static_cast<int>(angle * ANGLES_ISTEP)];
	}
	float CTAB::sinR(float radian)
	{
		return CTAB::sinA(TODEGREES(radian));
	}
	float CTAB::cosR(float radian)
	{
		return CTAB::cosA(TODEGREES(radian));
	}
	float CTAB::tanR(float radian)
	{
		return CTAB::tanA(TODEGREES(radian));
	}

};