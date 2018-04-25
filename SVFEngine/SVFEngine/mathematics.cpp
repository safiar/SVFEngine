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

#define ANGLES_ISTEP    100  //   1 /  STEP
#define ANGLES_NUM    36000  // 360 * ISTEP
#define ANGLES_STEP   0.01f  //   1 / ISTEP

namespace SAVFGAME
{
	float* CTAB::sinAngleTab  = new float [ANGLES_NUM];
	float* CTAB::cosAngleTab  = new float [ANGLES_NUM];
	float* CTAB::tanAngleTab  = new float [ANGLES_NUM];

	void CTAB::Close()
	{
		if (CTAB::sinAngleTab  != nullptr)  delete[] CTAB::sinAngleTab;
		if (CTAB::cosAngleTab  != nullptr)  delete[] CTAB::cosAngleTab;
		if (CTAB::tanAngleTab  != nullptr)  delete[] CTAB::tanAngleTab;
		CTAB::sinAngleTab  = nullptr;
		CTAB::cosAngleTab  = nullptr;
		CTAB::tanAngleTab  = nullptr;
	};
	void CTAB::Init()
	{
		if (CTAB::sinAngleTab  == nullptr)  CTAB::sinAngleTab  = new float [ANGLES_NUM];
		if (CTAB::cosAngleTab  == nullptr)  CTAB::cosAngleTab  = new float [ANGLES_NUM];
		if (CTAB::tanAngleTab  == nullptr)  CTAB::tanAngleTab  = new float [ANGLES_NUM];

		int idx = 0;
		for (float i=0; idx < ANGLES_NUM; i+=ANGLES_STEP, idx++)
		{
			auto angle = TORADIANS(i);
			CTAB::sinAngleTab[idx]  = sin(angle);
			CTAB::cosAngleTab[idx]  = cos(angle);
			CTAB::tanAngleTab[idx]  = tan(angle);
		}
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