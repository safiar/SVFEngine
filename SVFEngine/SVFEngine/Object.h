// ----------------------------------------------------------------------- //
//
// MODULE  : Object.h
//
// PURPOSE : Прототип объектов сцены
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _OBJECT_H
#define _OBJECT_H

#include "stdafx.h"
#include "mathematics.h"

namespace SAVFGAME
{
	// Фактическое состояние поворота модели содержится в кватернионе
	// Через углы передаётся необходимый поворот в углах Эйлера (сбрасываются в 0 после применения изменений к кватерниону, см. ModelPre.h)
	// Либо можно непосредственно менять кватернион

	struct WPOS
	{
		WPOS() : P(), A(), S(1,1,1), Q()																	{};
		WPOS(float x, float y, float z) 
			: P(x,y,z), A(), S(1,1,1), Q()																	{};
		WPOS(float x, float y, float z, float ax, float ay, float az) 
			: P(x,y,z), A(), S(1,1,1), Q(ax,ay,az)															{};
		WPOS(float x, float y, float z, float ax, float ay, float az, float sx, float sy, float sz) 
			: P(x,y,z), A(), S(sx,sy,sz), Q(ax,ay,az)														{};
		WPOS(MATH3DVEC3 position) 
			: P(position), A(), S(1,1,1), Q()																{};
		WPOS(MATH3DVEC3 position, MATH3DVEC3 angle) 
			: P(position), A(), S(1,1,1), Q(angle)															{};
		WPOS(MATH3DVEC3 position, MATH3DVEC3 angle, MATH3DVEC3 scale)  
			: P(position), A(), S(scale), Q(angle)															{};
		void _default()
		{
			 x =  y =  z = 0;
			ax = ay = az = 0;
			sz = sy = sz = 1;
			Q._default();
		}
		union
		{
			struct
			{
				float x, y, z;		// coordinate
				float angle_x;		// angle to rotate
				float angle_y;		// angle to rotate
				float angle_z;		// angle to rotate
				float scale_x;		// scale
				float scale_y;		// scale
				float scale_z;		// scale
				MATH3DQUATERNION Q; // rotation status
			};
			struct
			{
				float dx, dy, dz;	// coordinate
				float ax, ay, az;	// angle to rotate
				float sx, sy, sz;	// scale
				MATH3DQUATERNION Q; // rotation status
			};
			struct
			{
				MATH3DVEC3 P;		// coordinate
				MATH3DVEC3 A;		// angle to rotate
				MATH3DVEC3 S;		// scale
				MATH3DQUATERNION Q; // rotation status
			};
			float _[3+3+3+4]; // RAW
		};
	};

	class CObject
	{
	public:
		CObject(const CObject& src)				= delete;
		CObject(CObject&& src)					= delete;
		CObject& operator=(CObject&& src)		= delete;
		CObject& operator=(const CObject& src)	= delete;
	private:
		WPOS _;
	public:
		WPOS *			expos {nullptr}; // memory slot : additional pointer to other (external) WPOS data
	protected:
		WPOS * const	pos   {&_};		 // own data
	public:
		CObject()
		{
		};
		CObject(float x, float y, float z)
		{
			*pos = WPOS(x, y, z);
		};
		CObject(float x, float y, float z, float ax, float ay, float az)
		{
			*pos = WPOS(x, y, z, ax, ay, az);
		};
		CObject(float x, float y, float z, float ax, float ay, float az, float sx, float sy, float sz)
		{
			*pos = WPOS(x, y, z, ax, ay, az, sx, sy, sz);
		};
		CObject(MATH3DVEC3 position)
		{
			*pos = WPOS(position);
		};
		CObject(MATH3DVEC3 position, MATH3DVEC3 angle)
		{
			*pos = WPOS(position, angle);
		};
		CObject(MATH3DVEC3 position, MATH3DVEC3 angle, MATH3DVEC3 scale)
		{
			*pos = WPOS(position, angle, scale);
		};
		CObject(WPOS position)
		{
			*pos = WPOS(position);
		};
		virtual ~CObject() { };
		virtual void Close()
		{
			expos = nullptr;
			pos->_default();
		}

		//>> Устанавливает данные о позиции
		void  WPosSet(const WPOS * position) { *pos = *position; }

		//>> Устанавливает данные о позиции
		void  WPosSet(const WPOS & position) { *pos = position;  }

		//>> Возвращает данные о позиции
		const WPOS * WPosGet()               { return pos; }

		///////////////////////////////////////////////////////////////////

		float & WPosGetX()  { return pos->x; }
		float & WPosGetY()  { return pos->y; }
		float & WPosGetZ()  { return pos->z; }
		float & WPosGetAX() { return pos->ax; }
		float & WPosGetAY() { return pos->ay; }
		float & WPosGetAZ() { return pos->az; }
		float & WPosGetSX() { return pos->sx; }
		float & WPosGetSY() { return pos->sy; }
		float & WPosGetSZ() { return pos->sz; }
		MATH3DVEC3 & WPosGetP() { return pos->P; }
		MATH3DVEC3 & WPosGetA() { return pos->A; }
		MATH3DVEC3 & WPosGetS() { return pos->S; }
		MATH3DQUATERNION & WPosGetQ() { return pos->Q; }

		///////////////////////////////////////////////////////////////////

		void WPosSetX(float x)  { pos->x = x; }
		void WPosSetY(float y)  { pos->y = y; }
		void WPosSetZ(float z)  { pos->z = z; }
		void WPosSetAX(float ax) { pos->ax = ax; }
		void WPosSetAY(float ay) { pos->ay = ay; }
		void WPosSetAZ(float az) { pos->az = az; }
		void WPosSetSX(float sx) { pos->sx = sx; }
		void WPosSetSY(float sy) { pos->sy = sy; }
		void WPosSetSZ(float sz) { pos->sz = sz; }
		void WPosSetP(MATH3DVEC3 & P) { pos->P = P; }
		void WPosSetA(MATH3DVEC3 & A) { pos->A = A; }
		void WPosSetS(MATH3DVEC3 & S) { pos->S = S; }
		void WPosSetQ(MATH3DQUATERNION & Q) { pos->Q = Q; }

		///////////////////////////////////////////////////////////////////

		float GetDistTo(MATH3DVEC3 & other_point)
		{
			return MathLenVec(other_point - pos->P);
		}
		float GetDistTo(const MATH3DVEC3 * other_point)
		{
			return MathLenVec(*other_point - pos->P);
		}
	};
};

#endif // _OBJECT_H