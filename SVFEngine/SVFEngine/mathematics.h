// ----------------------------------------------------------------------- //
//
// MODULE  : mathematics.h
//
// PURPOSE : Вспомогательные математические функции и определения
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _MATHS_H
#define _MATHS_H

#include <stdio.h>
#include <string.h>
#include <cmath>
#include "helper.h"

using namespace SAVFGAME;

struct D3DXMATRIX;  // DX9
struct D3DXVECTOR4; // DX9

#ifndef FLOAT
	#define FLOAT float
#endif

#ifndef BYTE
	#define BYTE unsigned char
#endif

#ifndef DWORD
	#define DWORD unsigned long
#endif

typedef double Int3D;

#define ANGLES_ISTEP		100    //   1 /  STEP
#define ANGLES_NUM			36000  // 360 * ISTEP
#define ANGLES_STEP			0.01f  //   1 / ISTEP

#define MathPI				3.141592654f // 3.14159265358979323846
#define MathPI2				6.283185307f // 6.28318530717958647692
#define MathSqrt2           1.414213538f // sqrt(2) == 1.4142135623730950488016887242097
#define MathSqrt2inv		0.707106769f // 1 / sqrt(2) == 0.70710678118654752440

#define TORADIANS(x)		(x*(MathPI/180.0f))
#define TODEGREES(x)		(x*(180.0f/MathPI))

#define COLORBYTE(x)        (min(0xFF,static_cast<int>(0xFF*x)))			// float color (0.0f..1.0f) to byte  (00..FF)
#define COLORFLOAT(x)       (static_cast<float>(min(0xFF,x)) * (1.f/255))	// byte  color (00..FF)     to float (0.0f..1.0f)

#define _VECSET(from,to)    { to.x = from.x;  to.y = from.y;  to.z = from.z;                 }
#define _VECSET2(from,to)   { to.x = from.x;  to.y = from.y;                                 }
#define _VECSET3(from,to)   { to.x = from.x;  to.y = from.y;  to.z = from.z;                 }
#define _VECSET4(from,to)   { to.x = from.x;  to.y = from.y;  to.z = from.z;  to.w = from.w; }
#define _MATSET(from,to)    { to._11 = from._11;  to._12 = from._12;  to._13 = from._13;  to._14 = from._14; } \
							{ to._21 = from._21;  to._22 = from._22;  to._23 = from._23;  to._24 = from._24; } \
							{ to._31 = from._31;  to._32 = from._32;  to._33 = from._33;  to._34 = from._34; } \
							{ to._41 = from._41;  to._42 = from._42;  to._43 = from._43;  to._44 = from._44; }

#define MATH3DVEC3  MATH3DVEC
#define COLORVEC3   MATH3DVEC
#define COLORVEC    MATH3DVEC4
#define COLORVEC4   MATH3DVEC4

#define PRECISION   0.001f				// default float compare precision
#define MATSIZE     16*sizeof(float)

#define X_AXIS 1,0,0
#define Y_AXIS 0,1,0
#define Z_AXIS 0,0,1

#define OX_VEC MATH3DVEC(X_AXIS)
#define OY_VEC MATH3DVEC(Y_AXIS)
#define OZ_VEC MATH3DVEC(Z_AXIS)

////////////////////// QUICK SHARED CODE //////////////////////

// FLOAT x,y,z position ; ret MT
#define MATH_TRANSLATE_MATRIX(x,y,z)	MATH3DMATRIX MT;	\
										MT._41 = x;			\
										MT._42 = y;			\
										MT._43 = z;
// FLOAT x,y,z,w QUATERNION ; ret MR
#define MATH_ROTATE_MATRIX(x,y,z,w)		MATH3DMATRIX MR;												\
										float QXX = x * x;   float QXW = x * w;   float QXY = x * y;	\
										float QYY = y * y;   float QYW = y * w;   float QXZ = x * z;	\
										float QZZ = z * z;   float QZW = z * w;   float QYZ = y * z;	\
																										\
										MR._11 = 1.0f - 2.0f * (   QYY    +    QZZ   );					\
										MR._12 =        2.0f * (   QXY    +    QZW   );					\
										MR._13 =        2.0f * (   QXZ    -    QYW   );					\
										MR._21 =        2.0f * (   QXY    -    QZW   );					\
										MR._22 = 1.0f - 2.0f * (   QXX    +    QZZ   );					\
										MR._23 =        2.0f * (   QYZ    +    QXW   );					\
										MR._31 =        2.0f * (   QXZ    +    QYW   );					\
										MR._32 =        2.0f * (   QYZ    -    QXW   );					\
										MR._33 = 1.0f - 2.0f * (   QXX    +    QYY   );
// FLOAT x,y,z scale ; ret MS
#define MATH_SCALE_MATRIX(x,y,z)		MATH3DMATRIX MS;	\
										MS._11 = x;			\
										MS._22 = y;			\
										MS._33 = z;

////////////////////// QUICK SHARED CODE //////////////////////

// FLOAT axis X angle ; ret MRx
#define MATH_ROTATE_MATRIX_X(ax)		MATH3DMATRIX MRx;								\
										MRx._22 = CTAB::cosA(ax);  MRx._33 =  MRx._22;	\
										MRx._23 = CTAB::sinA(ax);  MRx._32 = -MRx._23;
// FLOAT axis Y angle ; ret MRy
#define MATH_ROTATE_MATRIX_Y(ay)		MATH3DMATRIX MRy;								\
										MRy._11 = CTAB::cosA(ay);  MRy._33 =  MRy._11;	\
										MRy._31 = CTAB::sinA(ay);  MRy._13 = -MRy._31;
// FLOAT axis Z angle ; ret MRz
#define MATH_ROTATE_MATRIX_Z(az)		MATH3DMATRIX MRz;								\
										MRz._11 = CTAB::cosA(az);  MRz._22 =  MRz._11;	\
										MRz._12 = CTAB::sinA(az);  MRz._21 = -MRz._12;
// FLOAT x,y,z angles ; ret MRz
#define MATH_ROTATE_MATRIX_A(ax,ay,az)	MATH_ROTATE_MATRIX_X(ax) \
										MATH_ROTATE_MATRIX_Y(ay) \
										MATH_ROTATE_MATRIX_Z(az) \
										MRz *= MRx;				 \
										MRz *= MRy;			

//#define _INVSQRT(x)   1 / sqrt(x)
#define _INVSQRT(x)   InvSqrt(x)

///////////////////////////////////////////////////////////////

namespace SAVFGAME
{
	class CTAB // Таблицы констант для быстрого вызова
	{
	private:
		static float sinAngleTab [ANGLES_NUM];
		static float cosAngleTab [ANGLES_NUM];
		static float tanAngleTab [ANGLES_NUM];
		static bool isInit;
	public:	
		static void Init();
		static float sinA(float);
		static float cosA(float);
		static float tanA(float);
		static float sinR(float);
		static float cosR(float);
		static float tanR(float);
	private:
		virtual void __DO_NOT_CREATE_OBJECT_OF_THIS_CLASS() = 0;
	};
	
	// out_f = 1 / sqrt(in_f)
	inline float __fastcall InvSqrt(float in_f)
	{
		#define __ONE__ ((uint32)(0x3F800000))

		uint32 tmp = ((__ONE__ << 1) + __ONE__ - *(uint32*)& in_f) >> 1;
		float  f = *(float*) & tmp;
		return f * (1.47f - 0.47f * in_f * f * f);

		#undef __ONE__
	}

	struct MATH3DMATRIX {
		MATH3DMATRIX(): _11(1),  _12(0),  _13(0),  _14(0),
						_21(0),  _22(1),  _23(0),  _24(0),
						_31(0),  _32(0),  _33(1),  _34(0),
						_41(0),  _42(0),  _43(0),  _44(1)		{};	// Identity matrix (Единичная матрица)
		MATH3DMATRIX(const float x, const float y, const float z, const byte Nx, const byte Ny, const byte Nz)
					  : _11(1),  _12(0),  _13(0),  _14(0),
						_21(0),  _22(1),  _23(0),  _24(0),
						_31(0),  _32(0),  _33(1),  _34(0),
						_41(0),  _42(0),  _43(0),  _44(1)
		{
			switch(Nx)
			{
			case 11: _11=x; break;	case 12: _12=x; break;	case 13: _13=x; break;	case 14: _14=x; break;
			case 21: _21=x; break;	case 22: _22=x; break;	case 23: _23=x; break;	case 24: _24=x; break;
			case 31: _31=x; break;	case 32: _32=x; break;	case 33: _33=x; break;	case 34: _34=x; break;
			case 41: _41=x; break;	case 42: _42=x; break;	case 43: _43=x; break;	case 44: _44=x; break;
			}
			switch(Ny)
			{
			case 11: _11=y; break;	case 12: _12=y; break;	case 13: _13=y; break;	case 14: _14=y; break;
			case 21: _21=y; break;	case 22: _22=y; break;	case 23: _23=y; break;	case 24: _24=y; break;
			case 31: _31=y; break;	case 32: _32=y; break;	case 33: _33=y; break;	case 34: _34=y; break;
			case 41: _41=y; break;	case 42: _42=y; break;	case 43: _43=y; break;	case 44: _44=y; break;
			}
			switch(Nz)
			{
			case 11: _11=z; break;	case 12: _12=z; break;	case 13: _13=z; break;	case 14: _14=z; break;
			case 21: _21=z; break;	case 22: _22=z; break;	case 23: _23=z; break;	case 24: _24=z; break;
			case 31: _31=z; break;	case 32: _32=z; break;	case 33: _33=z; break;	case 34: _34=z; break;
			case 41: _41=z; break;	case 42: _42=z; break;	case 43: _43=z; break;	case 44: _44=z; break;
			}
		};
		MATH3DMATRIX  operator *  (const MATH3DMATRIX & M) const
		{
			MATH3DMATRIX out;

			// for (i=0; i<4; i++)  for (j=0; j<4; j++)  [i][j] = [i][0]*[0][j] + [i][1]*[1][j] + [i][2]*[2][j] + [i][3]*[3][j]

			out._11 = (_11 * M._11)  +  (_12 * M._21)  +  (_13 * M._31)  +  (_14 * M._41) ;
			out._12 = (_11 * M._12)  +  (_12 * M._22)  +  (_13 * M._32)  +  (_14 * M._42) ;
			out._13 = (_11 * M._13)  +  (_12 * M._23)  +  (_13 * M._33)  +  (_14 * M._43) ;
			out._14 = (_11 * M._14)  +  (_12 * M._24)  +  (_13 * M._34)  +  (_14 * M._44) ;

			out._21 = (_21 * M._11)  +  (_22 * M._21)  +  (_23 * M._31)  +  (_24 * M._41) ;
			out._22 = (_21 * M._12)  +  (_22 * M._22)  +  (_23 * M._32)  +  (_24 * M._42) ;
			out._23 = (_21 * M._13)  +  (_22 * M._23)  +  (_23 * M._33)  +  (_24 * M._43) ;
			out._24 = (_21 * M._14)  +  (_22 * M._24)  +  (_23 * M._34)  +  (_24 * M._44) ;

			out._31 = (_31 * M._11)  +  (_32 * M._21)  +  (_33 * M._31)  +  (_34 * M._41) ;
			out._32 = (_31 * M._12)  +  (_32 * M._22)  +  (_33 * M._32)  +  (_34 * M._42) ;
			out._33 = (_31 * M._13)  +  (_32 * M._23)  +  (_33 * M._33)  +  (_34 * M._43) ;
			out._34 = (_31 * M._14)  +  (_32 * M._24)  +  (_33 * M._34)  +  (_34 * M._44) ;

			out._41 = (_41 * M._11)  +  (_42 * M._21)  +  (_43 * M._31)  +  (_44 * M._41) ;
			out._42 = (_41 * M._12)  +  (_42 * M._22)  +  (_43 * M._32)  +  (_44 * M._42) ;
			out._43 = (_41 * M._13)  +  (_42 * M._23)  +  (_43 * M._33)  +  (_44 * M._43) ;
			out._44 = (_41 * M._14)  +  (_42 * M._24)  +  (_43 * M._34)  +  (_44 * M._44) ;
			
			return out;
		}; //*/
		MATH3DMATRIX& operator *= (const MATH3DMATRIX & M)
		{
			// for (i=0; i<4; i++)  for (j=0; j<4; j++)  [i][j] = [i][0]*[0][j] + [i][1]*[1][j] + [i][2]*[2][j] + [i][3]*[3][j]

			float f0, f1, f2, f3;

			       f0 = _11;      f1 = _12;      f2 = _13;      f3 = _14;

			_11 = (f0 * M._11) + (f1 * M._21) + (f2 * M._31) + (f3 * M._41);
			_12 = (f0 * M._12) + (f1 * M._22) + (f2 * M._32) + (f3 * M._42);
			_13 = (f0 * M._13) + (f1 * M._23) + (f2 * M._33) + (f3 * M._43);
			_14 = (f0 * M._14) + (f1 * M._24) + (f2 * M._34) + (f3 * M._44);

			       f0 = _21;      f1 = _22;      f2 = _23;      f3 = _24;

			_21 = (f0 * M._11) + (f1 * M._21) + (f2 * M._31) + (f3 * M._41);
			_22 = (f0 * M._12) + (f1 * M._22) + (f2 * M._32) + (f3 * M._42);
			_23 = (f0 * M._13) + (f1 * M._23) + (f2 * M._33) + (f3 * M._43);
			_24 = (f0 * M._14) + (f1 * M._24) + (f2 * M._34) + (f3 * M._44);

			       f0 = _31;      f1 = _32;      f2 = _33;      f3 = _34;

			_31 = (f0 * M._11) + (f1 * M._21) + (f2 * M._31) + (f3 * M._41);
			_32 = (f0 * M._12) + (f1 * M._22) + (f2 * M._32) + (f3 * M._42);
			_33 = (f0 * M._13) + (f1 * M._23) + (f2 * M._33) + (f3 * M._43);
			_34 = (f0 * M._14) + (f1 * M._24) + (f2 * M._34) + (f3 * M._44);

			       f0 = _41;      f1 = _42;      f2 = _43;      f3 = _44;

			_41 = (f0 * M._11) + (f1 * M._21) + (f2 * M._31) + (f3 * M._41);
			_42 = (f0 * M._12) + (f1 * M._22) + (f2 * M._32) + (f3 * M._42);
			_43 = (f0 * M._13) + (f1 * M._23) + (f2 * M._33) + (f3 * M._43);
			_44 = (f0 * M._14) + (f1 * M._24) + (f2 * M._34) + (f3 * M._44);
			
			return *this;
		};
		MATH3DMATRIX  operator +  (const MATH3DMATRIX & M) const
		{
			MATH3DMATRIX out;

			out._11 = (_11 + M._11);   out._12 = (_12 + M._12);   out._13 = (_13 + M._13);   out._14 = (_14 + M._14);
			out._21 = (_21 + M._21);   out._22 = (_22 + M._22);   out._23 = (_23 + M._23);   out._24 = (_24 + M._24);
			out._31 = (_31 + M._31);   out._32 = (_32 + M._32);   out._33 = (_33 + M._33);   out._34 = (_34 + M._34);
			out._41 = (_41 + M._41);   out._42 = (_42 + M._42);   out._43 = (_43 + M._43);   out._44 = (_44 + M._44);
			
			return out;
		}; //*/
		MATH3DMATRIX& operator += (const MATH3DMATRIX & M)
		{
			_11 = (_11 + M._11);   _12 = (_12 + M._12);   _13 = (_13 + M._13);   _14 = (_14 + M._14);
			_21 = (_21 + M._21);   _22 = (_22 + M._22);   _23 = (_23 + M._23);   _24 = (_24 + M._24);
			_31 = (_31 + M._31);   _32 = (_32 + M._32);   _33 = (_33 + M._33);   _34 = (_34 + M._34);
			_41 = (_41 + M._41);   _42 = (_42 + M._42);   _43 = (_43 + M._43);   _44 = (_44 + M._44);

			return *this;
		};
		MATH3DMATRIX  operator -  (const MATH3DMATRIX & M) const
		{
			MATH3DMATRIX out;

			out._11 = (_11 - M._11);   out._12 = (_12 - M._12);   out._13 = (_13 - M._13);   out._14 = (_14 - M._14);
			out._21 = (_21 - M._21);   out._22 = (_22 - M._22);   out._23 = (_23 - M._23);   out._24 = (_24 - M._24);
			out._31 = (_31 - M._31);   out._32 = (_32 - M._32);   out._33 = (_33 - M._33);   out._34 = (_34 - M._34);
			out._41 = (_41 - M._41);   out._42 = (_42 - M._42);   out._43 = (_43 - M._43);   out._44 = (_44 - M._44);
			
			return out;
		}; //*/
		MATH3DMATRIX& operator -= (const MATH3DMATRIX & M)
		{
			_11 = (_11 - M._11);   _12 = (_12 - M._12);   _13 = (_13 - M._13);   _14 = (_14 - M._14);
			_21 = (_21 - M._21);   _22 = (_22 - M._22);   _23 = (_23 - M._23);   _24 = (_24 - M._24);
			_31 = (_31 - M._31);   _32 = (_32 - M._32);   _33 = (_33 - M._33);   _34 = (_34 - M._34);
			_41 = (_41 - M._41);   _42 = (_42 - M._42);   _43 = (_43 - M._43);   _44 = (_44 - M._44);

			return *this;
		};
		D3DXMATRIX* D3DCAST()
		{
			return reinterpret_cast<D3DXMATRIX*>(this);
		}
		operator D3DXMATRIX*()
		{
			return reinterpret_cast <D3DXMATRIX*> (this);
		}
		operator float*()
		{
			return reinterpret_cast <float*> (this);
		}
		void _printf()
		{
			printf("\n%f %f %f %f"
				   "\n%f %f %f %f"
				   "\n%f %f %f %f"
				   "\n%f %f %f %f\n",
				   _11, _12, _13, _14,	
				   _21, _22, _23, _24,
				   _31, _32, _33, _34,
				   _41, _42, _43, _44);
		}
		void _transpose()
		{
		//	const auto matsz = MATSIZE;
		//	float temp[4][4];
		//	for(int i=0; i<4; i++)
		//	for(int j=0; j<4; j++)
		//		temp[i][j] = _[j][i];
		//	memcpy(_,temp,matsz);

			_SWAP(_13, _31);
			_SWAP(_21, _12);
			_SWAP(_32, _23);
			_SWAP(_41, _14);
			_SWAP(_42, _24);
			_SWAP(_43, _34);
		}
		void _default()
		{
			_11=1; _12=0; _13=0; _14=0;
			_21=0; _22=1; _23=0; _24=0;
			_31=0; _32=0; _33=1; _34=0;
			_41=0; _42=0; _43=0; _44=1;
		}
		union
		{
			struct { FLOAT	_11, _12, _13, _14,
							_21, _22, _23, _24,
							_31, _32, _33, _34,
							_41, _42, _43, _44; };
			FLOAT _[4][4];
		};
	};

	struct MATH3DVEC {
		MATH3DVEC() : x(0), y(0), z(0) { };
		MATH3DVEC(const float _x, const float _y, const float _z) : x(_x), y(_y), z(_z) { };
		MATH3DVEC  operator *  (const float f) const
		{
			MATH3DVEC vec(x*f, y*f, z*f);  return vec;
		};
		MATH3DVEC  operator -  (const float f) const
		{
			MATH3DVEC vec(x-f, y-f, z-f);  return vec;
		};
		MATH3DVEC  operator +  (const float f) const
		{
			MATH3DVEC vec(x+f, y+f, z+f);  return vec;
		};
		MATH3DVEC& operator *= (const float f)
		{
			x *= f;  y *= f;  z *= f;  return *this;
		};
		MATH3DVEC& operator -= (const float f)
		{
			x -= f;  y -= f;  z -= f;  return *this;
		};
		MATH3DVEC& operator += (const float f)
		{
			x += f;  y += f;  z += f;  return *this;
		};
		MATH3DVEC  operator -  (const MATH3DVEC & v) const
		{
			MATH3DVEC vec(x-v.x, y-v.y, z-v.z);  return vec;
		};
		MATH3DVEC  operator +  (const MATH3DVEC & v) const
		{
			MATH3DVEC vec(x+v.x, y+v.y, z+v.z);  return vec;
		};
		//>> простое перемножение компонент
		MATH3DVEC  operator *  (const MATH3DVEC & v) const 
		{
			MATH3DVEC vec(x*v.x, y*v.y, z*v.z);  return vec;
		}
		MATH3DVEC& operator -= (const MATH3DVEC & v)
		{
			x -= v.x;  y -= v.y;  z -= v.z;  return *this;
		};
		MATH3DVEC& operator += (const MATH3DVEC & v)
		{
			x += v.x;  y += v.y;  z += v.z;  return *this;
		};	
		//>> простое перемножение компонент
		MATH3DVEC& operator *= (const MATH3DVEC & v) 
		{
			x *= v.x;  y *= v.y;  z *= v.z;  return *this;
		}
		bool operator == (const MATH3DVEC & v) const
		{
			if (x >= v.x - PRECISION &&
				x <= v.x + PRECISION &&
				y >= v.y - PRECISION &&
				y <= v.y + PRECISION &&
				z >= v.z - PRECISION &&
				z <= v.z + PRECISION ) return true;
			return false;
		};
		bool operator != (const MATH3DVEC & v) const
		{
			if (x >= v.x - PRECISION &&
				x <= v.x + PRECISION &&
				y >= v.y - PRECISION &&
				y <= v.y + PRECISION &&
				z >= v.z - PRECISION &&
				z <= v.z + PRECISION ) return false;
			return true;
		};
		bool operator >= (const MATH3DVEC & v) const
		{
			if (x >= v.x - PRECISION &&
				y >= v.y - PRECISION &&
				z >= v.z - PRECISION ) return true;
			return false;
		};
		bool operator <= (const MATH3DVEC & v) const
		{
			if (x <= v.x + PRECISION &&
				y <= v.y + PRECISION &&
				z <= v.z + PRECISION ) return true;
			return false;
		};
		bool operator > (const MATH3DVEC & v) const
		{
			if (x > v.x - PRECISION &&
				y > v.y - PRECISION &&
				z > v.z - PRECISION ) return true;
			return false;
		};
		bool operator < (const MATH3DVEC & v) const
		{
			if (x < v.x + PRECISION &&
				y < v.y + PRECISION &&
				z < v.z + PRECISION ) return true;
			return false;
		};
		bool operator ! () const
		{
			if (*this != MATH3DVEC())
				return false;
			return true;
		};
		bool operator && (const MATH3DVEC & v) const
		{
			if (*this != MATH3DVEC() &&
				    v != MATH3DVEC() )
				return true;
			return false;
		};
		bool operator || (const MATH3DVEC & v) const
		{
			if (*this != MATH3DVEC() ||
				    v != MATH3DVEC() )
				return true;
			return false;
		};
		MATH3DVEC  operator *  (const MATH3DMATRIX & M) const
		{
			MATH3DVEC vec;
			float norm = (x * M._14) + (y * M._24) + (z * M._34) + M._44;

			if (norm)																{
				vec.x = ((x * M._11) + (y * M._21) + (z * M._31) + M._41) / norm;
				vec.y = ((x * M._12) + (y * M._22) + (z * M._32) + M._42) / norm;
				vec.z = ((x * M._13) + (y * M._23) + (z * M._33) + M._43) / norm;	}
			else																	{
				vec.x = 0;   vec.y = 0;   vec.z = 0;								}

			return vec;
		}//*/
		MATH3DVEC& operator *= (const MATH3DMATRIX & M)
		{
			float _x, _y, _z;
			float norm = (x * M._14) + (y * M._24) + (z * M._34) + M._44;

			if (norm)													{
				_x = ((x * M._11) + (y * M._21) + (z * M._31)) / norm;
				_y = ((x * M._12) + (y * M._22) + (z * M._32)) / norm;
				_z = ((x * M._13) + (y * M._23) + (z * M._33)) / norm;	}
			else														{
				_x = 0;   _y =0;   _z = 0;								}

			x = _x;   y = _y;   z = _z;   return *this;
		}
		template <class TYPE>
		MATH3DVEC& _set(const TYPE & src)
		{
			x = src.x;
			y = src.y;
			z = src.z;
			return *this;
		}
		operator float*()
		{
			return reinterpret_cast <float*> (this);
		}
		void _printf()
		{
			printf("\n%f %f %f", x, y, z);
		}	
		void _normalize()
		{
			float norm = x*x + y*y + z*z;	if (!norm) return;
			float mod = _INVSQRT(norm);
			x *= mod;
			y *= mod;
			z *= mod;
		}
		void _normalize_check()
		{
			float norm = x*x + y*y + z*z;	if (!norm) return;
			
			if ( norm > (1.000f + PRECISION) ||
				 norm < (1.000f - PRECISION) )
			{
				float mod = _INVSQRT(norm);
				x *= mod;
				y *= mod;
				z *= mod;
			}
		}	
		void _default()
		{
			x = y = z = 0;
		}
		union
		{
			struct { float  x,  y,  z; }; // coordinate-обращение
			struct { float ax, ay, az; }; // angle-обращение
			struct { float sx, sy, sz; }; // scale-обращение
			struct { float nx, ny, nz; }; // normal-обращение
			struct { float tx, ty, tz; }; // tangent-обращение
			struct { float bx, by, bz; }; // binormal-обращение
			struct { float  u,  v,  w; }; // texture1_UV-обращение
			struct { float u2, v2, w2; }; // texture2_UV-обращение
			struct { float u3, v3, w3; }; // texture3_UV-обращение
			struct { float  r,  g,  b; }; // color-обращение
			struct { float  A,  B,  C; }; // plane-обращение
			float _[3];
		};
	};

	struct MATH3DVEC4 {
		MATH3DVEC4() : x(0), y(0), z(0), w(0) {};
		MATH3DVEC4(const float _x, const float _y, const float _z) : x(_x), y(_y), z(_z), w(0) {};
		MATH3DVEC4(const float _x, const float _y, const float _z, const float _w) : x(_x), y(_y), z(_z), w(_w) {};
		MATH3DVEC4 operator *  (const float f) const
		{
			MATH3DVEC4 vec(x*f, y*f, z*f, w*f);  return vec;
		};
		MATH3DVEC4 operator -  (const MATH3DVEC4 & v) const
		{
			MATH3DVEC4 vec(x-v.x, y-v.y, z-v.z, w-v.w);  return vec;
		};
		MATH3DVEC4 operator +  (const MATH3DVEC4 & v) const
		{
			MATH3DVEC4 vec(x+v.x, y+v.y, z+v.z, w+v.w);  return vec;
		};
		bool operator == (const MATH3DVEC4 & v) const
		{
			if (x >= v.x - PRECISION &&
				x <= v.x + PRECISION &&
				y >= v.y - PRECISION &&
				y <= v.y + PRECISION &&
				z >= v.z - PRECISION &&
				z <= v.z + PRECISION &&
				w >= v.w - PRECISION &&
				w <= v.w + PRECISION ) return true;
			return false;
		};
		bool operator != (const MATH3DVEC4 & v) const
		{
			if (x >= v.x - PRECISION &&
				x <= v.x + PRECISION &&
				y >= v.y - PRECISION &&
				y <= v.y + PRECISION &&
				z >= v.z - PRECISION &&
				z <= v.z + PRECISION &&
				w >= v.w - PRECISION &&
				w <= v.w + PRECISION ) return false;
			return true;
		};
		template <class TYPE>
		MATH3DVEC4& _set(const TYPE & src)
		{
			x = src.x;
			y = src.y;
			z = src.z;
			w = src.w;
			return *this;
		}
		operator float*()
		{
			return reinterpret_cast <float*> (this);
		}
		// COLOR argb uint32 operator
		operator uint32() const
		{
			uint32 argb = 0;
			argb |= COLORBYTE(a) << 24;
			argb |= COLORBYTE(r) << 16;
			argb |= COLORBYTE(g) << 8;
			argb |= COLORBYTE(b);
			return argb;
		}
		void _printf()
		{
			printf("\n%f %f %f %f", x, y, z, w);
		}	
		void _normalize()
		{
			float norm = x*x + y*y + z*z + w*w;		if (!norm) return;
			float mod = _INVSQRT(norm);
			x *= mod;
			y *= mod;
			z *= mod;
			w *= mod;
		}	
		void _normalize_check()
		{
			float norm = x*x + y*y + z*z + w*w;		if (!norm) return;

			if (norm > (1.000f + PRECISION) ||
				norm < (1.000f - PRECISION))
			{
				float mod = _INVSQRT(norm);
				x *= mod;
				y *= mod;
				z *= mod;
				w *= mod;
			}
		}
		void _default()
		{
			x = y = z = w = 0;
		}
		union
		{
			struct { float x, y, z, w; };
			struct { float r, g, b, a; };
			struct { float A, B, C, D; };
			float _[4];
		};
	};

	struct MATH3DVEC2 {
		MATH3DVEC2() : x(0), y(0) {};
		MATH3DVEC2(const float _x, const float _y) : x(_x), y(_y) {};
		template <class TYPE>
		MATH3DVEC2& _set(const TYPE & src)
		{
			x = src.x;
			y = src.y;
			return *this;
		}
		operator float*()
		{
			return reinterpret_cast<FLOAT*>(this);
		}
		void _printf()
		{
			printf("\n%f %f", x, y);
		}	
		void _normalize()
		{
			float norm = x*x + y*y;				if (!norm) return;
			float mod = _INVSQRT(norm);
			x *= mod;
			y *= mod;
		}
		void _normalize_check()
		{
			float norm = x*x + y*y;				if (!norm) return;

			if (norm > (1.000f + PRECISION) ||
				norm < (1.000f - PRECISION))
			{
				float mod = _INVSQRT(norm);
				x *= mod;
				y *= mod;
			}
		}
		void _default()
		{
			x = y = 0;
		}
		union
		{
			struct { float x, y; };
			struct { float u, v; };
			float _[2];
		};
	};

	struct MATH3DPLANE {
		MATH3DPLANE() : a(0), b(0), c(0), d(0), P(MATH3DVEC(0,0,0)), N(MATH3DVEC(0,0,0)) {};
		MATH3DPLANE(const MATH3DVEC& point, const MATH3DVEC& normal)
		{
			a = normal.x;
			b = normal.y;
			c = normal.z;
			d = (-point.x*normal.x) + (-point.y*normal.y) + (-point.z*normal.z);
			P = point;
			N = normal;
		};
		MATH3DPLANE(const MATH3DVEC& point, const MATH3DVEC& point2, const MATH3DVEC& point3)
		{
			MATH3DVEC vec1(point2.x - point.x,  point2.y - point.y,  point2.z - point.z);
			MATH3DVEC vec2(point3.x - point.x,  point3.y - point.y,  point3.z - point.z);
			MATH3DVEC normal(vec1.y * vec2.z - vec1.z * vec2.y,
							 vec1.z * vec2.x - vec1.x * vec2.z,
							 vec1.x * vec2.y - vec1.y * vec2.x);

			normal._normalize_check();
			a = normal.x;
			b = normal.y;
			c = normal.z;
			d = (-point.x*normal.x) + (-point.y*normal.y) + (-point.z*normal.z);
			P = point;
			N = normal;
		};
		void _normalize()
		{
			float norm = a*a + b*b + c*c;		
			if (norm)
			{
				float mod = _INVSQRT(norm);
				a *= mod;
				b *= mod;
				c *= mod;
				d *= mod;
				N.x = a;
				N.y = b;
				N.z = c;
			}
		}
		void _default()
		{
			a = b = c = d = 0;
			P._default();
			N._default();
		}
		float a, b, c, d;
		MATH3DVEC  P;
		MATH3DVEC  N;
	};

	MATH3DVEC MathOrthogonalVec(const MATH3DVEC & v);

	struct MATH3DQUATERNION {										// http://www.gamedev.ru/code/articles/?id=4215&page=2
		MATH3DQUATERNION() : x(0), y(0), z(0), w(1) {};
		//>> Конструктор из углов Эйлера
		MATH3DQUATERNION(const float ax, const float ay, const float az) 
		{
			float s_ax = CTAB::sinA(ax * 0.5f);
			float c_ax = CTAB::cosA(ax * 0.5f);
			float s_ay = CTAB::sinA(ay * 0.5f);
			float c_ay = CTAB::cosA(ay * 0.5f);
			float s_az = CTAB::sinA(az * 0.5f);
			float c_az = CTAB::cosA(az * 0.5f);

			x = s_ay * c_ax * s_az  +  c_ay * s_ax * c_az;
			y = s_ay * c_ax * c_az  -  c_ay * s_ax * s_az;
			z = c_ay * c_ax * s_az  -  s_ay * s_ax * c_az;
			w = c_ay * c_ax * c_az  +  s_ay * s_ax * s_az;

			_normalize_check();
		};
		//>> Конструктор из углов Эйлера
		MATH3DQUATERNION(const MATH3DVEC & angles) 
		{
			float s_ax = CTAB::sinA(angles.ax * 0.5f);
			float c_ax = CTAB::cosA(angles.ax * 0.5f);
			float s_ay = CTAB::sinA(angles.ay * 0.5f);
			float c_ay = CTAB::cosA(angles.ay * 0.5f);
			float s_az = CTAB::sinA(angles.az * 0.5f);
			float c_az = CTAB::cosA(angles.az * 0.5f);

			x = s_ay * c_ax * s_az  +  c_ay * s_ax * c_az;
			y = s_ay * c_ax * c_az  -  c_ay * s_ax * s_az;
			z = c_ay * c_ax * s_az  -  s_ay * s_ax * c_az;
			w = c_ay * c_ax * c_az  +  s_ay * s_ax * s_az;

			_normalize_check();
		}	
		//>> Конструктор из сферических координат
		MATH3DQUATERNION(const float latitude, const float longitude, const float angle, bool _reserved)
		{
			float sin_a = CTAB::sinA(angle * 0.5f);
			float cos_a = CTAB::cosA(angle * 0.5f);

			float sin_lat = CTAB::sinA(latitude);
			float cos_lat = CTAB::cosA(latitude);

			float sin_long = CTAB::sinA(longitude);
			float cos_long = CTAB::cosA(longitude);

			x = sin_a * cos_lat * sin_long;
			y = sin_a * sin_lat;
			z =               y * cos_long;
			w = cos_a;

			_normalize_check();

			// Обратно
		/*	cos_angle = q->qw;
			sin_angle = sqrt(1.0 - cos_angle * cos_angle);
			angle = acos(cos_angle) * 2 * RADIANS;

			if (fabs(sin_angle) < 0.0005)
				sa = 1;

			tx = q->qx / sa;
			ty = q->qy / sa;
			tz = q->qz / sa;

			latitude = -asin(ty);

			if (tx * tx + tz * tz < 0.0005)
				longitude = 0;
			else
				longitude = atan2(tx, tz) * RADIANS;

			if (longitude < 0)
				longitude += 360.0; //*/
		}
		//>> Конструктор из произвольной оси
		MATH3DQUATERNION(const float X, const float Y, const float Z, const float angle) 
		{
			float sin = CTAB::sinA(angle * 0.5f);
			float cos = CTAB::cosA(angle * 0.5f);

			x = sin * X;
			y = sin * Y;
			z = sin * Z;
			w = cos;

			_normalize_check();
		}
		//>> Конструктор из произвольной оси
		MATH3DQUATERNION(const MATH3DVEC & axis, const float angle)
		{
			float sin = CTAB::sinA(angle * 0.5f);
			float cos = CTAB::cosA(angle * 0.5f);

			x = sin * axis.x;
			y = sin * axis.y;
			z = sin * axis.z;
			w = cos;

			_normalize_check();
		}		
	/*	//>> Конструктор кратчайшей дуги (shortest arc)
		MATH3DQUATERNION(const MATH3DVEC & from, const MATH3DVEC & to)
		{
			// half vector method // 

			MATH3DVEC _from = from;
			MATH3DVEC _to   = to;

			_from._normalize_check();
			_to._normalize_check();

			if (_from == (_to * -1)) // разворот на 180 вокруг любого ортогонального
			{
				MATH3DVEC ortho = MathOrthogonalVec(_from);			
				ortho._normalize_check();

				x = ortho.x;
				y = ortho.y;
				z = ortho.z;
				w = 0;
			}
			else
			{
				MATH3DVEC half = _from + _to;
				half._normalize_check();

				float dot = _from.x * half.x + _from.y * half.y + _from.z * half.z;

				MATH3DVEC cross(_from.y * half.z - _from.z * half.y,
								_from.z * half.x - _from.x * half.z,
								_from.x * half.y - _from.y * half.x);

				x = cross.x;	
				y = cross.y;
				z = cross.z;
				w = dot;
			}
		}//*/
		//>> Конструктор кратчайшей дуги (shortest arc)
		MATH3DQUATERNION(const MATH3DVEC & from, const MATH3DVEC & to)
		{
			// half quat method //  в 3 раза быстрее чем half vector method

			float dot      = from.x * to.x   + from.y * to.y   + from.z * to.z   ;
			float len_from = from.x * from.x + from.y * from.y + from.z * from.z ;
			float len_to   =   to.x * to.x   +   to.y * to.y   +   to.z * to.z   ;

			float k = sqrt(len_from * len_to);

			if (dot / k == -1)
			{
				MATH3DVEC ortho = MathOrthogonalVec(from);
				ortho._normalize_check();

				x = ortho.x;
				y = ortho.y;
				z = ortho.z;
				w = 0;
			}
			else
			{
				MATH3DVEC cross(from.y * to.z - from.z * to.y,
								from.z * to.x - from.x * to.z,
								from.x * to.y - from.y * to.x);

				x = cross.x;
				y = cross.y;
				z = cross.z;
				w = dot + k;

				_normalize();
			}
		}
	/*	//>> Конструктор кратчайшей дуги (shortest arc) :: test
		MATH3DQUATERNION(const MATH3DVEC & _from, const MATH3DVEC & _to, bool gems_method)
		{
			MATH3DVEC from = _from;
			MATH3DVEC to   = _to;

			from._normalize_check();
			to._normalize_check();

			float dot = from.x * to.x + from.y * to.y + from.z * to.z;

			if (gems_method)
			{
				float s = sqrt((1 + dot) * 2);

				if (s < std::numeric_limits<float>::epsilon())
				{
				//	// Generate an axis
				//	Vector3f a = Vector3f::UnitX().cross(v0);
				//	// pick another if collinear
				//	if (a.squaredNorm() < std::numeric_limits<float>::epsilon())
				//	{
				//		a = Vector3f::UnitY().cross(v0);
				//	}
				//	a.normalize();
				//	return Quaternionf(AngleAxisf(boost::math::constants::pi<float>(), a));

					MATH3DVEC ortho;
					MathOrthogonalVec(from, ortho);
					ortho._normalize_check();

					x = ortho.x;
					y = ortho.y;
					z = ortho.z;
					w = 0;
				}
				else
				{
					MATH3DVEC cross(from.y * to.z - from.z * to.y,
									from.z * to.x - from.x * to.z,
									from.x * to.y - from.y * to.x);

					float s_ = 1 / s ;

					x = cross.x * s_ ;
					y = cross.y * s_ ;
					z = cross.z * s_ ;
					w =    0.5f * s  ;
				}
			}
			else
			{
				MATH3DVEC cross(from.y * to.z - from.z * to.y,
								from.z * to.x - from.x * to.z,
								from.x * to.y - from.y * to.x);

				float len_from = from.x * from.x + from.y * from.y + from.z * from.z ;
				float len_to   =   to.x * to.x   +   to.y * to.y   +   to.z * to.z   ;

				x = cross.x;
				y = cross.y;
				x = cross.z;
			//	w = dot;
				w = dot + sqrt(len_from * len_to);

				_normalize_check();

			//	w += 1.0f;      // reducing angle to halfangle
			//	if (w <= TINY) // angle close to PI
			//	{
			//		if ((from.z*from.z) > (from.x*from.x))
			//			set(0, from.z, -from.y, w); //from*vector3(1,0,0) 
			//		else
			//			set(from.y, -from.x, 0, w); //from*vector3(0,0,1) 
			//	}
			//	normalize();
			}

			_normalize_check();
		}//*/
		//>> Конструктор из матрицы вращения
		MATH3DQUATERNION(const MATH3DMATRIX & M)
		{
			float s, s_, trace;

			trace = M._11 + M._22 + M._33 + 1.0f;
			if (trace > 1.0f)
			{
				s  = 2.0f * sqrt(trace);  
				s_ = 1 / s;
				x  = (M._23 - M._32) * s_;
				y  = (M._31 - M._13) * s_;
				z  = (M._12 - M._21) * s_;
				w  = 0.25f * s;
			}
			else
			{
				int maxi = 0;

				for (int i=1; i<3; i++)
					if (M._[i][i] > M._[maxi][maxi])
						maxi = i;

				switch (maxi)
				{
				case 0:
					s  = 2.0f * sqrt(1.0f + M._11 - M._22 - M._33);
					s_ = 1 / s;
					x  = 0.25f * s;
					y  = (M._12 + M._21) * s_;
					z  = (M._13 + M._31) * s_;
					w  = (M._23 - M._32) * s_; break;
				case 1:
					s  = 2.0f * sqrt(1.0f + M._22 - M._11 - M._33);
					s_ = 1 / s;
					x  = (M._12 + M._21) * s_;
					y  = 0.25f * s;
					z  = (M._23 + M._32) * s_;
					w  = (M._31 - M._13) * s_; break;
				case 2:
					s  = 2.0f * sqrt(1.0f + M._33 - M._11 - M._22);
					s_ = 1 / s;
					x  = (M._13 + M._31) * s_;
					y  = (M._23 + M._32) * s_;
					z  = 0.25f * s;
					w  = (M._12 - M._21) * s_; break;
				}
			}

			_normalize_check();
		}	
		//>> Конструктор RAW
		MATH3DQUATERNION(const MATH3DVEC4 & raw)
		{
			x = raw.x;
			y = raw.y;
			z = raw.z;
			w = raw.w;
		}
		MATH3DQUATERNION  operator +  (const MATH3DQUATERNION & Q) const
		{
			MATH3DQUATERNION out;

			out.x = Q.x + x;
			out.y = Q.y + y;
			out.z = Q.z + z;
			out.w = Q.w + w;

			return out;
		}
		MATH3DQUATERNION& operator += (const MATH3DQUATERNION & Q)
		{
			x += Q.x;
			y += Q.y;
			z += Q.z;
			w += Q.w;

			return *this;
		}
		MATH3DQUATERNION  operator -  (const MATH3DQUATERNION & Q) const
		{
			MATH3DQUATERNION out;

			out.x = Q.x - x;
			out.y = Q.y - y;
			out.z = Q.z - z;
			out.w = Q.w - w;

			return out;
		}
		MATH3DQUATERNION& operator -= (const MATH3DQUATERNION & Q)
		{
			x -= Q.x;
			y -= Q.y;
			z -= Q.z;
			w -= Q.w;

			return *this;
		}
		MATH3DQUATERNION  operator *  (const MATH3DQUATERNION & Q) const
		{
			MATH3DQUATERNION out;	// out = this * Q

			out.x = Q.w * x  +  Q.x * w  +  Q.y * z  -  Q.z * y;
			out.y = Q.w * y  -  Q.x * z  +  Q.y * w  +  Q.z * x;
			out.z = Q.w * z  +  Q.x * y  -  Q.y * x  +  Q.z * w;
			out.w = Q.w * w  -  Q.x * x  -  Q.y * y  -  Q.z * z;

			return out;
		}
		MATH3DQUATERNION& operator *= (const MATH3DQUATERNION & Q)
		{
			MATH3DQUATERNION out;	// out = this * Q

			out.x = Q.w * x  +  Q.x * w  +  Q.y * z  -  Q.z * y;
			out.y = Q.w * y  -  Q.x * z  +  Q.y * w  +  Q.z * x;
			out.z = Q.w * z  +  Q.x * y  -  Q.y * x  +  Q.z * w;
			out.w = Q.w * w  -  Q.x * x  -  Q.y * y  -  Q.z * z;

			x = out.x;
			y = out.y;
			z = out.z;
			w = out.w;

			return *this;
		}
		bool operator == (const MATH3DQUATERNION & Q) const
		{
			if (x >= Q.x - PRECISION &&
				x <= Q.x + PRECISION &&
				y >= Q.y - PRECISION &&
				y <= Q.y + PRECISION &&
				z >= Q.z - PRECISION &&
				z <= Q.z + PRECISION &&
				w >= Q.w - PRECISION &&
				w <= Q.w + PRECISION) return true;
			return false;
		};
		bool operator != (const MATH3DQUATERNION & Q) const
		{
			if (x >= Q.x - PRECISION &&
				x <= Q.x + PRECISION &&
				y >= Q.y - PRECISION &&
				y <= Q.y + PRECISION &&
				z >= Q.z - PRECISION &&
				z <= Q.z + PRECISION &&
				w >= Q.w - PRECISION &&
				w <= Q.w + PRECISION) return false;
			return true;
		};
		operator float*()
		{
			return reinterpret_cast<float*>(this);
		}
		void _default()
		{
			x = y = z = 0;
			w = 1;
		}
		void _printf()
		{
			printf("\n%f %f %f %f", x, y, z, w);
		}
		void _normalize()
		{
			float norm = x*x + y*y + z*z + w*w;		if (!norm) return;
			float mod = _INVSQRT(norm);
			x *= mod;
			y *= mod;
			z *= mod;
			w *= mod;
		}
		void _normalize_check()
		{
			float norm = x*x + y*y + z*z + w*w;		if (!norm) return;
			
			if ( norm > (1.000f + PRECISION) ||
				 norm < (1.000f - PRECISION) )
			{
				float mod = _INVSQRT(norm);
				x *= mod;
				y *= mod;
				z *= mod;
				w *= mod;
			}
		}	
		void _conjugate()
		{
			x *= -1;
			y *= -1;
			z *= -1;
		}
		void _inverse()
		{
			float norm = x*x + y*y + z*z + w*w;		

			if (norm > (1.000f + PRECISION) ||
				norm < (1.000f - PRECISION))
			{
				if (!norm) return;
				float mod = _INVSQRT(norm);
				x *= -mod;
				y *= -mod;
				z *= -mod;
				w *=  mod;
			}
			else
			{
				x *= -1;
				y *= -1;
				z *= -1;
			}
		}
		void _angles(MATH3DVEC & v)
		{
			// ...надо будет ещё посмотреть...
			// qx2 = q.x * q.x
			// qy2 = q.y * q.y
			// qz2 = q.z * q.z
			// bank = atan2(2 * (q.x * q.w + q.y * q.z), 1 - 2 * (qx2 + qy2))
			// altitude = Asin(2 * (q.y * q.w - q.z * q.x))
			// heading = atan2(2 * (q.z * q.w + q.x * q.y), 1 - 2 * (qy2 + qz2))

			// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
			// https://www.mathworks.com/help/robotics/ref/quat2eul.html
			// https://github.com/tminnigaliev/euler_angles/blob/master/eul_from_rotm.m
			// https://www.mathworks.com/help/robotics/ref/quat2eul.html


			float sinr = 2.0f * (x*w - y*z);				// -M._32 = 2.0f * -(QYZ - QXW);	
			sinr = (sinr >=  1.0f) ?  1.0f : sinr;
			sinr = (sinr <= -1.0f) ? -1.0f : sinr;
			v.x = TODEGREES(asin(sinr));

		/*	if (sinr > 0.99f || sinr < -0.99f)
			{
				FLOAT siny =        2.0f * (x*z - y*w);		// M._13 =        2.0f * (   QXZ    -    QYW   );
				FLOAT cosy = 1.0f - 2.0f * (y*y + z*z);		// M._11 = 1.0f - 2.0f * (   QYY    +    QZZ   );
				v.z = TODEGREES(atan2(siny,cosy));

				FLOAT sinp =        2.0f * (x*y - z*w);		// 
				FLOAT cosp = 1.0f - 2.0f * (z*z + y*y);		// 
				v.y = 90 + TODEGREES(atan2(sinp,cosp));
			}
			else
			{ //*/
				float siny =        2.0f * (x*z + y*w);		// M._31 =        2.0f * (   QXZ    +    QYW   );
				float cosy = 1.0f - 2.0f * (x*x + y*y);		// M._33 = 1.0f - 2.0f * (   QXX    +    QYY   );
				v.y = TODEGREES(atan2(siny,cosy));

				float sinp =        2.0f * (x*y + z*w);		// M._12 =        2.0f * (   QXY    +    QZW   );
				float cosp = 1.0f - 2.0f * (x*x + z*z);		// M._22 = 1.0f - 2.0f * (   QXX    +    QZZ   );
				v.z = TODEGREES(atan2(sinp,cosp));
		//	} //*/

		}		
		union
		{
			struct { float x, y, z, w; };
			float _[4];
		};
	};

	/////////////////////////////////////////

	inline const D3DXMATRIX* D3DCAST(const MATH3DMATRIX* ptr)
	{
		return reinterpret_cast<const D3DXMATRIX*>(ptr);
	}
	inline const D3DXVECTOR4* D3DCAST(const MATH3DVEC4* ptr)
	{
		return reinterpret_cast<const D3DXVECTOR4*>(ptr);
	}
	inline const MATH3DMATRIX* D3DCAST(const D3DXMATRIX* ptr)
	{
		return reinterpret_cast<const MATH3DMATRIX*>(ptr);
	}
	inline const MATH3DVEC4* D3DCAST(const D3DXVECTOR4* ptr)
	{
		return reinterpret_cast<const MATH3DVEC4*>(ptr);
	}
	inline D3DXMATRIX* D3DCAST (MATH3DMATRIX* ptr)
	{
		return reinterpret_cast<D3DXMATRIX*>(ptr);
	}
	inline D3DXVECTOR4* D3DCAST (MATH3DVEC4* ptr)
	{
		return reinterpret_cast<D3DXVECTOR4*>(ptr);
	}
	inline MATH3DMATRIX* D3DCAST (D3DXMATRIX* ptr)
	{
		return reinterpret_cast<MATH3DMATRIX*>(ptr);
	}
	inline MATH3DVEC4* D3DCAST (D3DXVECTOR4* ptr)
	{
		return reinterpret_cast<MATH3DVEC4*>(ptr);
	}
	inline const float* FCAST (const MATH3DMATRIX* ptr)
	{
		return reinterpret_cast<const float*>(ptr);
	}
	inline const float* FCAST(const MATH3DVEC4* ptr)
	{
		return reinterpret_cast<const float*>(ptr);
	}
	inline const float* FCAST(const MATH3DVEC3* ptr)
	{
		return reinterpret_cast<const float*>(ptr);
	}
	inline float* FCAST(MATH3DMATRIX* ptr)
	{
		return reinterpret_cast<float*>(ptr);
	}
	inline float* FCAST(MATH3DVEC4* ptr)
	{
		return reinterpret_cast<float*>(ptr);
	}
	inline float* FCAST(MATH3DVEC3* ptr)
	{
		return reinterpret_cast<float*>(ptr);
	}
	
	//>> Измеряет длину (модуль) вектора
	inline float MathLenVec(const MATH3DVEC & vec)					{
		return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);		};

	//>> Измеряет длину (модуль) вектора
	inline void MathLenVec(const MATH3DVEC & vec, float& length)		{
		length = sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);		};

	//>> Измеряет расстояние между двумя точками
	inline float MathDistance(const MATH3DVEC & P1, const MATH3DVEC & P2)
	{
		return MathLenVec(P1 - P2);
	}

	//>> Измеряет расстояние между двумя точками
	inline void MathDistance(const MATH3DVEC & P1, const MATH3DVEC & P2, float & out)
	{
		out = MathLenVec(P1 - P2);
	}

	//>> Вычитает vec1 - vec2
/*	inline MATH3DVEC MathSubtractVec(const MATH3DVEC & vec1, const MATH3DVEC & vec2)
	{
		return MATH3DVEC(vec1.x - vec2.x,
						 vec1.y - vec2.y,
						 vec1.z - vec2.z);
	} //*/
	;
	;
	//>> Вычитает vec1 - vec2
/*	inline void MathSubtractVec(const MATH3DVEC & vec1, const MATH3DVEC & vec2, MATH3DVEC& vec)
	{
		vec = MATH3DVEC(vec1.x - vec2.x,
						vec1.y - vec2.y,
						vec1.z - vec2.z);
	} //*/
	;
	//>> Складывает vec1 + vec2
/*	inline MATH3DVEC MathAddVec(const MATH3DVEC & vec1, const MATH3DVEC & vec2)
	{
		return MATH3DVEC(vec1.x + vec2.x,
						 vec1.y + vec2.y,
						 vec1.z + vec2.z);
	} //*/
	;
	//>> Складывает vec1 + vec2
/*	inline void MathAddVec(const MATH3DVEC & vec1, const MATH3DVEC & vec2, MATH3DVEC& vec)
	{
		vec = MATH3DVEC(vec1.x + vec2.x,
						vec1.y + vec2.y,
						vec1.z + vec2.z);
	} //*/
	;
	//>> Возвращает скалярное произведение 2 векторов (DOT product)
	inline float MathDotVec(const MATH3DVEC & vec1, const MATH3DVEC & vec2)				{
		return vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z;						};

	//>> Возвращает скалярное произведение 2 векторов (DOT product)
	inline void MathDotVec(const MATH3DVEC & vec1, const MATH3DVEC & vec2, float& dot)		{
		dot = vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z;							};

	//>> Возвращает косинус угла между 2 векторами (скалярное произведение + нормализация)
	inline float MathCosVec(const MATH3DVEC & vec1, const MATH3DVEC & vec2)		{
		return MathDotVec(vec1,vec2) / MathLenVec(vec1) * MathLenVec(vec2);		};

	//>> Возвращает векторное произведение 2 векторов (CROSS product)
	inline MATH3DVEC MathCrossVec(const MATH3DVEC & vec1, const MATH3DVEC & vec2)		{
		return MATH3DVEC(vec1.y * vec2.z - vec1.z * vec2.y,
						 vec1.z * vec2.x - vec1.x * vec2.z,
						 vec1.x * vec2.y - vec1.y * vec2.x);						};

	//>> Возвращает векторное произведение 2 векторов (CROSS product)
	inline void MathCrossVec(const MATH3DVEC & vec1, const MATH3DVEC & vec2, MATH3DVEC& out)	{
		out = MATH3DVEC(vec1.y * vec2.z - vec1.z * vec2.y,
						vec1.z * vec2.x - vec1.x * vec2.z,
						vec1.x * vec2.y - vec1.y * vec2.x);										};

	//>> Приведение вектора к единичной длине
	inline MATH3DVEC MathNormalizeVec(const MATH3DVEC & vec)
	{
		float m, l = vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;

		if (!l) m = 1;
		else    m = _INVSQRT(l);

		return MATH3DVEC(vec.x * m, vec.y * m, vec.z * m);
	};
	
	//>> Вектор из абсолютных значений
/*	inline MATH3DVEC MathAbsVec(const MATH3DVEC & vec)
	{
		return MATH3DVEC(abs(vec.x), abs(vec.y), abs(vec.z));
	}; //*/
	;
	//>> Составляет ортогональный вектор
	inline MATH3DVEC MathOrthogonalVec(const MATH3DVEC & v)
	{
		float x = abs(v.x);
		float y = abs(v.y);
		float z = abs(v.z);

		MATH3DVEC other = x < y ? (x < z ? OX_VEC : OZ_VEC) : (y < z ? OY_VEC : OZ_VEC);
		return MathCrossVec(v, other);
	}
	
	//>> Составляет ортогональный вектор
	inline void MathOrthogonalVec(const MATH3DVEC & v, MATH3DVEC & out)
	{
		float x = abs(v.x);
		float y = abs(v.y);
		float z = abs(v.z);

		MATH3DVEC other = x<y ? (x<z ? OX_VEC : OZ_VEC) : (y<z ? OY_VEC : OZ_VEC);
		MathCrossVec(v, other, out);
	}

	//>> Применение world-матрицы к точке пространства
	inline MATH3DVEC MathVecTransformCoord(const MATH3DVEC & v, const MATH3DMATRIX & M)
	{
		MATH3DVEC out;
		float norm = (v.x * M._14) + (v.y * M._24) + (v.z * M._34) + M._44;

		if (norm)																		{
			out.x = ((v.x * M._11) + (v.y * M._21) + (v.z * M._31) + M._41) / norm;
			out.y = ((v.x * M._12) + (v.y * M._22) + (v.z * M._32) + M._42) / norm;
			out.z = ((v.x * M._13) + (v.y * M._23) + (v.z * M._33) + M._43) / norm;		}
		else																			{
			out.x = 0;   out.y = 0;   out.z = 0;										}

		return out;
	} //*/
	;
	//>> Применение world-матрицы к точке пространства
	inline void MathVecTransformCoord(const MATH3DVEC & v, const MATH3DMATRIX & M, MATH3DVEC& out)
	{
		MATH3DVEC _out;
		float norm = (v.x * M._14) + (v.y * M._24) + (v.z * M._34) + M._44;

		if (norm)																		{
			_out.x = ((v.x * M._11) + (v.y * M._21) + (v.z * M._31) + M._41) / norm;
			_out.y = ((v.x * M._12) + (v.y * M._22) + (v.z * M._32) + M._42) / norm;
			_out.z = ((v.x * M._13) + (v.y * M._23) + (v.z * M._33) + M._43) / norm;	}
		else																			{
			_out.x = 0;   out.y = 0;   out.z = 0;										}

		out = _out;
	}

	//>> Применение world-матрицы к точке пространства
	inline void MathVecTransformCoord(const MATH3DMATRIX & M, MATH3DVEC& in_out)
	{
		float norm = (in_out.x * M._14) + (in_out.y * M._24) + (in_out.z * M._34) + M._44;

		if (norm)																		{
			MATH3DVEC v = in_out;
			in_out.x = ((v.x * M._11) + (v.y * M._21) + (v.z * M._31) + M._41) / norm;
			in_out.y = ((v.x * M._12) + (v.y * M._22) + (v.z * M._32) + M._42) / norm;
			in_out.z = ((v.x * M._13) + (v.y * M._23) + (v.z * M._33) + M._43) / norm;
		}
		else																			{
			in_out.x = 0;   in_out.y = 0;   in_out.z = 0;								}
	}

	//>> Применение world-матрицы к вектору
	inline MATH3DVEC MathVecTransformNormal(const MATH3DVEC & v, const MATH3DMATRIX & M)
	{
		MATH3DVEC out;

		out.x = (v.x * M._11) + (v.y * M._21) + (v.z * M._31);
		out.y = (v.x * M._12) + (v.y * M._22) + (v.z * M._32);
		out.z = (v.x * M._13) + (v.y * M._23) + (v.z * M._33);

		return out;
	} //*/
	;
	//>> Применение world-матрицы к вектору
	inline void MathVecTransformNormal(const MATH3DVEC & v, const MATH3DMATRIX & M, MATH3DVEC& out)
	{
		MATH3DVEC _out;
		_out.x = (v.x * M._11) + (v.y * M._21) + (v.z * M._31);
		_out.y = (v.x * M._12) + (v.y * M._22) + (v.z * M._32);
		_out.z = (v.x * M._13) + (v.y * M._23) + (v.z * M._33);
		out = _out;
	} 

	//>> Применение world-матрицы к вектору
	inline void MathVecTransformNormal(const MATH3DMATRIX & M, MATH3DVEC& in_out)
	{
		MATH3DVEC v = in_out;
		in_out.x = (v.x * M._11) + (v.y * M._21) + (v.z * M._31);
		in_out.y = (v.x * M._12) + (v.y * M._22) + (v.z * M._32);
		in_out.z = (v.x * M._13) + (v.y * M._23) + (v.z * M._33);
	}

	//>> Составление кватерниона из углов Эйлера
/*	inline MATH3DQUATERNION MathQuaternionRotationXYZ(const float ax, const float ay, const float az)
	{
		MATH3DQUATERNION Q;

		float s_ax = CTAB::sinA(ax * 0.5f);
		float c_ax = CTAB::cosA(ax * 0.5f);
		float s_ay = CTAB::sinA(ay * 0.5f);
		float c_ay = CTAB::cosA(ay * 0.5f);
		float s_az = CTAB::sinA(az * 0.5f);
		float c_az = CTAB::cosA(az * 0.5f);

		Q.x = s_ay * c_ax * s_az  +  c_ay * s_ax * c_az;
		Q.y = s_ay * c_ax * c_az  -  c_ay * s_ax * s_az;
		Q.z = c_ay * c_ax * s_az  -  s_ay * s_ax * c_az;
		Q.w = c_ay * c_ax * c_az  +  s_ay * s_ax * s_az;

		return Q;
	} //*/
	;
	//>> Составление кватерниона из углов Эйлера
/*	inline void MathQuaternionRotationXYZ(const float ax, const float ay, const float az, MATH3DQUATERNION& Q)
	{
		float s_ax = CTAB::sinA(ax * 0.5f);
		float c_ax = CTAB::cosA(ax * 0.5f);
		float s_ay = CTAB::sinA(ay * 0.5f);
		float c_ay = CTAB::cosA(ay * 0.5f);
		float s_az = CTAB::sinA(az * 0.5f);
		float c_az = CTAB::cosA(az * 0.5f);

		Q.x = s_ay * c_ax * s_az  +  c_ay * s_ax * c_az;
		Q.y = s_ay * c_ax * c_az  -  c_ay * s_ax * s_az;
		Q.z = c_ay * c_ax * s_az  -  s_ay * s_ax * c_az;
		Q.w = c_ay * c_ax * c_az  +  s_ay * s_ax * s_az;
	} //*/
	;
	//>> Составление кватерниона из углов Эйлера
/*	inline MATH3DQUATERNION MathQuaternionRotationXYZ(const MATH3DVEC& angles)
	{
		return MathQuaternionRotationXYZ(angles.ax, angles.ay, angles.az);
	} //*/
	;
	//>> Составление кватерниона из углов Эйлера
/*	inline void MathQuaternionRotationXYZ(const MATH3DVEC& angles, MATH3DQUATERNION& Q)
	{
		MathQuaternionRotationXYZ(angles.ax, angles.ay, angles.az, Q);
	} //*/
	;
	//>> Составление кватерниона из произвольной оси
/*	inline MATH3DQUATERNION MathQuaternionRotationAxis(const float X, const float Y, const float Z, const float angle)
	{
		MATH3DQUATERNION Q;
		float sin = CTAB::sinA(angle * 0.5f);
		float cos = CTAB::cosA(angle * 0.5f);

		Q.x = sin * X;
		Q.y = sin * Y;
		Q.z = sin * Z;
		Q.w = cos;

		return Q;
	} //*/
	;
	//>> Составление кватерниона из произвольной оси
/*	inline void MathQuaternionRotationAxis(const float X, const float Y, const float Z, const float angle, MATH3DQUATERNION& Q)
	{
		float sin = CTAB::sinA(angle * 0.5f);
		float cos = CTAB::cosA(angle * 0.5f);

		Q.x = sin * X;
		Q.y = sin * Y;
		Q.z = sin * Z;
		Q.w = cos;
	} //*/
	;
	//>> Составление кватерниона из произвольной оси
/*	inline MATH3DQUATERNION MathQuaternionRotationAxis(const MATH3DVEC & axis, const float angle)
	{
		return MathQuaternionRotationAxis(axis.x, axis.y, axis.z, angle);
	} //*/
	;
	//>> Составление кватерниона из произвольной оси
/*	inline void MathQuaternionRotationAxis(const MATH3DVEC& axis, const float angle, MATH3DQUATERNION& Q)
	{
		MathQuaternionRotationAxis(axis.x, axis.y, axis.z, angle, Q);
	} //*/
	;
	//>> Конвертация кватерниона в ось и угол
	inline void MathQuaternionToRotationAxis(const MATH3DQUATERNION& Q, MATH3DVEC& axis, float& angle)
	{
		float val = sqrt(Q.x*Q.x + Q.y*Q.y + Q.z*Q.z);
		if (val >= PRECISION)
		{
			float val_ = 1.0f / val;		// TODO: оптимизировать atan2()
			axis.x = Q.x * val_;
			axis.y = Q.y * val_;
			axis.z = Q.z * val_;
			if (Q.w < 0)
				angle = TODEGREES(2.0f * atan2(-val, -Q.w)); // [-PI, 0]
			else
				angle = TODEGREES(2.0f * atan2(val, Q.w)); // [0, PI]
		}
		else
		{
			axis.x = axis.y = axis.z = 0;
			angle = 0;
		}
	}
	
	//>> Составление кватерниона из матрицы вращения
/*	inline MATH3DQUATERNION MathQuaternionRotationMatrix(const MATH3DMATRIX & M)
	{
		MATH3DQUATERNION out;
		float s, trace;

		trace = M._11 + M._22 + M._33 + 1.0f;
		if (trace > 1.0f)
		{
			s = 2.0f * sqrt(trace);
			out.x = (M._23 - M._32) / s;
			out.y = (M._31 - M._13) / s;
			out.z = (M._12 - M._21) / s;
			out.w = 0.25f * s;
		}
		else
		{
			int i, maxi = 0;
			
			for (i=1; i<3; i++)
				if (M._[i][i] > M._[maxi][maxi])
					maxi = i;

			switch (maxi)
			{
			case 0:
				s = 2.0f * sqrt(1.0f + M._11 - M._22 - M._33);
				out.x = 0.25f * s;
				out.y = (M._12 + M._21) / s;
				out.z = (M._13 + M._31) / s;
				out.w = (M._23 - M._32) / s; break;
			case 1:
				s = 2.0f * sqrt(1.0f + M._22 - M._11 - M._33);
				out.x = (M._12 + M._21) / s;
				out.y = 0.25f * s;
				out.z = (M._23 + M._32) / s;
				out.w = (M._31 - M._13) / s; break;
			case 2:
				s = 2.0f * sqrt(1.0f + M._33 - M._11 - M._22);
				out.x = (M._13 + M._31) / s;
				out.y = (M._23 + M._32) / s;
				out.z = 0.25f * s;
				out.w = (M._12 - M._21) / s; break;
			}
		}

		return out;
	} //*/
	;
	//>> Составление кватерниона из матрицы вращения
/*	inline void MathQuaternionRotationMatrix(const MATH3DMATRIX & M, MATH3DQUATERNION & out)
	{
		out = MathQuaternionRotationMatrix(M);
	} //*/
	;
	//>> Магнитуда кватерниона (длина)
	inline float MathQuaternionLength(const MATH3DQUATERNION & Q)
	{
		return sqrt(Q.x*Q.x + Q.y*Q.y + Q.z*Q.z + Q.w*Q.w);
	}
	
	//>> Магнитуда кватерниона (длина)
	inline void MathQuaternionLength(const MATH3DQUATERNION & Q, float & out)
	{
		out = sqrt(Q.x*Q.x + Q.y*Q.y + Q.z*Q.z + Q.w*Q.w);
	}

	//>> Квадрат длины кватерниона (норма)
	inline float MathQuaternionLengthSq(const MATH3DQUATERNION & Q)
	{
		return Q.x*Q.x + Q.y*Q.y + Q.z*Q.z + Q.w*Q.w;
	}

	//>> Квадрат длины кватерниона (норма)
	inline void MathQuaternionLengthSq(const MATH3DQUATERNION & Q, float & out)
	{
		out = Q.x*Q.x + Q.y*Q.y + Q.z*Q.z + Q.w*Q.w;
	}

	//>> Обратный / инверсный кватернион / сопряжение (conjugate)
/*	inline void MathQuaternionInvert(MATH3DQUATERNION & Q)
	{
		Q.x *= -1;
		Q.y *= -1;
		Q.z *= -1;
	} //*/
	;
	//>> Нормализация кватерниона
/*	inline MATH3DQUATERNION MathQuaternionNormalize(const MATH3DQUATERNION & Q)
	{
		MATH3DQUATERNION out;

		float len = sqrt(Q.x*Q.x + Q.y*Q.y + Q.z*Q.z + Q.w*Q.w);
		if (!len) return Q;
		out.x = Q.x / len;
		out.y = Q.y / len;
		out.z = Q.z / len;
		out.w = Q.w / len;
	
		return out;
	} //*/
	;
	//>> Умножение кватернионов (скалярное)
	inline float MathQuaternionDot(const MATH3DQUATERNION & Q1, const MATH3DQUATERNION & Q2)
	{
		return (Q1.x * Q2.x) + (Q1.y * Q2.y) + (Q1.z * Q2.z) + (Q1.w * Q2.w);
	}

	//>> Умножение кватернионов (скалярное)
	inline void MathQuaternionDot(const MATH3DQUATERNION & Q1, const MATH3DQUATERNION & Q2, float & out)
	{
		out = (Q1.x * Q2.x) + (Q1.y * Q2.y) + (Q1.z * Q2.z) + (Q1.w * Q2.w);
	}

	//>> Умножение кватернионов
/*	inline MATH3DQUATERNION MathQuaternionMultiply(const MATH3DQUATERNION & Q1, const MATH3DQUATERNION & Q2)
	{
		MATH3DQUATERNION Q;

		Q.x = Q2.w * Q1.x  +  Q2.x * Q1.w  +  Q2.y * Q1.z  -  Q2.z * Q1.y;
		Q.y = Q2.w * Q1.y  -  Q2.x * Q1.z  +  Q2.y * Q1.w  +  Q2.z * Q1.x;
		Q.z = Q2.w * Q1.z  +  Q2.x * Q1.y  -  Q2.y * Q1.x  +  Q2.z * Q1.w;
		Q.w = Q2.w * Q1.w  -  Q2.x * Q1.x  -  Q2.y * Q1.y  -  Q2.z * Q1.z;

		return Q;
	} //*/
	;
	//>> Умножение кватернионов
/*	inline void MathQuaternionMultiply(const MATH3DQUATERNION & Q1, const MATH3DQUATERNION & Q2, MATH3DQUATERNION& out)
	{
		out.x = Q2.w * Q1.x  +  Q2.x * Q1.w  +  Q2.y * Q1.z  -  Q2.z * Q1.y;
		out.y = Q2.w * Q1.y  -  Q2.x * Q1.z  +  Q2.y * Q1.w  +  Q2.z * Q1.x;
		out.z = Q2.w * Q1.z  +  Q2.x * Q1.y  -  Q2.y * Q1.x  +  Q2.z * Q1.w;
		out.w = Q2.w * Q1.w  -  Q2.x * Q1.x  -  Q2.y * Q1.y  -  Q2.z * Q1.z;
	} //*/
	;
	//>> Умножение кватернионов (более долгий вариант)
/*	inline void MathQuaternionMultiplyAlt(const MATH3DQUATERNION & Q1, const MATH3DQUATERNION & Q2, MATH3DQUATERNION& out)
	{
		float A, B, C, D, E, F, G, H;

		A = (Q1.w + Q1.x) * (Q2.w + Q2.x);
		B = (Q1.z - Q1.y) * (Q2.y - Q2.z);
		C = (Q1.x - Q1.w) * (Q2.y + Q2.z);
		D = (Q1.y + Q1.z) * (Q2.x - Q2.w);
		E = (Q1.x + Q1.z) * (Q2.x + Q2.y);
		F = (Q1.x - Q1.z) * (Q2.x - Q2.y);
		G = (Q1.w + Q1.y) * (Q2.w - Q2.z);
		H = (Q1.w - Q1.y) * (Q2.w + Q2.z);

		out.w =  B + (-E - F + G + H) * 0.5f;
		out.x =  A - (E + F + G + H)  * 0.5f;
		out.y = -C + (E - F + G - H)  * 0.5f;
		out.z = -D + (E - F - G + H)  * 0.5f;
	} //*/

	//>> Умножение кватерниона и вектора
	inline void MathQuaternionMultiply(const MATH3DQUATERNION & Q, const MATH3DVEC & V, MATH3DQUATERNION & out)
	{
		MATH3DQUATERNION _Q = Q;

		out.x = V.x * _Q.w + V.y * _Q.z - V.z * _Q.y;
		out.y = V.x * _Q.z + V.y * _Q.w + V.z * _Q.x;
		out.z = V.x * _Q.y - V.y * _Q.x + V.z * _Q.w;
		out.w = V.x * _Q.x - V.y * _Q.y - V.z * _Q.z;
	}
	
	//>> Умножение кватерниона и вектора
	inline void MathQuaternionMultiply(const MATH3DVEC & V, const MATH3DQUATERNION & Q, MATH3DQUATERNION & out)
	{
		MATH3DQUATERNION _Q = Q;

		out.x = _Q.w * V.x + _Q.y * V.z - _Q.z * V.y;
		out.y = _Q.w * V.y - _Q.x * V.z + _Q.z * V.x;
		out.z = _Q.w * V.z + _Q.x * V.y - _Q.y * V.x;
		out.w = _Q.x * V.x - _Q.y * V.y - _Q.z * V.z;
	}
	
	//>> Применение кватерниона к вектору V'= q * V * q–1 
	inline void MathQuaternionApply(const MATH3DQUATERNION & Q, MATH3DVEC & in_out)
	{
		MATH3DQUATERNION _Q, _Q_inv(-Q.x, -Q.y, -Q.z, Q.w);

		_Q.x = Q.w * in_out.x + Q.y * in_out.z - Q.z * in_out.y;
		_Q.y = Q.w * in_out.y - Q.x * in_out.z + Q.z * in_out.x;
		_Q.z = Q.w * in_out.z + Q.x * in_out.y - Q.y * in_out.x;
		_Q.w = Q.x * in_out.x - Q.y * in_out.y - Q.z * in_out.z;

		_Q = _Q * _Q_inv;

		in_out.x = _Q.x;
		in_out.y = _Q.y;
		in_out.z = _Q.z;
	}

	//>> Сферическая линейная интерполяция между 2 кватернионами
	inline MATH3DQUATERNION MathQuaternionSLERP(const MATH3DQUATERNION & Q1, const MATH3DQUATERNION & Q2,
		const float t)
	{	
		// (p sin((1–t)a) – q sin(ta)) / sin(a);  t = [0..1];  a->cos(a) = DOT(q, p)

		// http://wat.gamedev.ru/articles/quaternions?page=2

		char epsilon = 1;	
		float cos_a = MathQuaternionDot(Q1, Q2);
		if (cos_a < 0) epsilon = -1;

		float scale_1, scale_2;
		float delta_a = 0.9848f; // до 10 градусов - линейно

		if (cos_a * epsilon < delta_a) // большой угол
		{
			float angle = TODEGREES(acos(cos_a));
			float sin_a = 1.f / CTAB::sinA(angle);
			scale_1 = CTAB::sinA((1.f - t) * angle) * sin_a;
			scale_2 = CTAB::sinA(       t  * angle) * sin_a;
		}
		else // маленький угол -> линейно
		{
			scale_1 = 1.f - t;
			scale_2 = t;
		}

		scale_2 *= epsilon;

		MATH3DQUATERNION out;

		out.x = scale_1 * Q1.x + scale_2 * Q2.x;
		out.y = scale_1 * Q1.y + scale_2 * Q2.y;
		out.z = scale_1 * Q1.z + scale_2 * Q2.z;
		out.w = scale_1 * Q1.w + scale_2 * Q2.w;

		out._normalize_check();

		return out;
	}

	//>> Сферическая линейная интерполяция между 2 кватернионами
	inline void MathQuaternionSLERP(const MATH3DQUATERNION & Q1, const MATH3DQUATERNION & Q2,
		const float t, MATH3DQUATERNION & out)
	{	
		char epsilon = 1;	
		float cos_a = MathQuaternionDot(Q1, Q2);
		if (cos_a < 0) epsilon = -1;

		float scale_1, scale_2;
		float delta_a = 0.9848f; // до 10 градусов - линейно

		if (cos_a * epsilon < delta_a) // большой угол
		{
			float angle = TODEGREES(acos(cos_a));
			float sin_a = 1.f / CTAB::sinA(angle);
			scale_1 = CTAB::sinA((1.f - t) * angle) * sin_a;
			scale_2 = CTAB::sinA(       t  * angle) * sin_a;
		}
		else // маленький угол -> линейно
		{
			scale_1 = 1.f - t;
			scale_2 = t;
		}

		scale_2 *= epsilon;

		out.x = scale_1 * Q1.x + scale_2 * Q2.x;
		out.y = scale_1 * Q1.y + scale_2 * Q2.y;
		out.z = scale_1 * Q1.z + scale_2 * Q2.z;
		out.w = scale_1 * Q1.w + scale_2 * Q2.w;

		out._normalize_check();
	}

	//>> Сферическая линейная интерполяция между 2 кватернионами
	inline MATH3DQUATERNION MathQuaternionSLERP(const MATH3DQUATERNION & Q1, const MATH3DQUATERNION & Q2,
		const float t, const float cos_linear)
	{	
		char epsilon = 1;	
		float cos_a = MathQuaternionDot(Q1, Q2);
		if (cos_a < 0) epsilon = -1;

		float scale_1, scale_2;
		float cos_a__ = cos_a * epsilon;

		// избегаем деления на ноль
		// sin(0.5f) < 0.009
		// cos(0.5f) > 0.99995

		if (cos_a__ < 0.99995f && cos_a__ < cos_linear) // большой угол
		{
			float angle = TODEGREES(acos(cos_a));
			float sin_a = 1.f / CTAB::sinA(angle);
			scale_1 = CTAB::sinA((1.f - t) * angle) * sin_a;
			scale_2 = CTAB::sinA(       t  * angle) * sin_a;
		}
		else // маленький угол -> линейно
		{
			scale_1 = 1.f - t;
			scale_2 = t;
		}

		scale_2 *= epsilon;

		MATH3DQUATERNION out;

		out.x = scale_1 * Q1.x + scale_2 * Q2.x;
		out.y = scale_1 * Q1.y + scale_2 * Q2.y;
		out.z = scale_1 * Q1.z + scale_2 * Q2.z;
		out.w = scale_1 * Q1.w + scale_2 * Q2.w;

		out._normalize_check();

		return out;
	}

	//>> Сферическая линейная интерполяция между 2 кватернионами
	inline void MathQuaternionSLERP(const MATH3DQUATERNION & Q1, const MATH3DQUATERNION & Q2,
		const float t, const float cos_linear, MATH3DQUATERNION & out)
	{	
		char epsilon = 1;	
		float cos_a = MathQuaternionDot(Q1, Q2);
		if (cos_a < 0) epsilon = -1;

		float scale_1, scale_2;
		float cos_a__ = cos_a * epsilon;

		// избегаем деления на ноль
		// sin(0.5f) < 0.009
		// cos(0.5f) > 0.99995

		if (cos_a__ < 0.99995f && cos_a__ < cos_linear) // большой угол
		{
			float angle = TODEGREES(acos(cos_a));
			float sin_a = 1.f / CTAB::sinA(angle);
			scale_1 = CTAB::sinA((1.f - t) * angle) * sin_a;
			scale_2 = CTAB::sinA(       t  * angle) * sin_a;
		}
		else // маленький угол -> линейно
		{
			scale_1 = 1.f - t;
			scale_2 = t;
		}

		scale_2 *= epsilon;

		out.x = scale_1 * Q1.x + scale_2 * Q2.x;
		out.y = scale_1 * Q1.y + scale_2 * Q2.y;
		out.z = scale_1 * Q1.z + scale_2 * Q2.z;
		out.w = scale_1 * Q1.w + scale_2 * Q2.w;

		out._normalize_check();
	}

	//>> Кубическая интерполяция между 4 кватернионами
	inline void MathQuaternionSQUAD(const MATH3DQUATERNION & Q1, const MATH3DQUATERNION & Q2, const MATH3DQUATERNION & Q3,
		const MATH3DQUATERNION & Q4, const float t, MATH3DQUATERNION & out)
	{
		MATH3DQUATERNION Q14, Q23;

		MathQuaternionSLERP(Q1, Q4, t, Q14);
		MathQuaternionSLERP(Q2, Q3, t, Q23);
		MathQuaternionSLERP(Q14, Q23, 2.0f * t * (1.0f - t), out);
	}

	//>> Кубическая интерполяция между 4 кватернионами
	inline void MathQuaternionSQUAD(const MATH3DQUATERNION & Q1, const MATH3DQUATERNION & Q2, const MATH3DQUATERNION & Q3,
		const MATH3DQUATERNION & Q4, const float t, const float cos_linear, MATH3DQUATERNION & out)
	{
		MATH3DQUATERNION Q14, Q23;

		MathQuaternionSLERP(Q1, Q4, t, cos_linear, Q14);
		MathQuaternionSLERP(Q2, Q3, t, cos_linear, Q23);
		MathQuaternionSLERP(Q14, Q23, 2.0f * t * (1.0f - t), cos_linear, out);
	}

	//>> Задаёт кватернион в барицентрических координатах
	inline void MathQuaternionBaryCentric(const MATH3DQUATERNION & Q1, const MATH3DQUATERNION & Q2, const MATH3DQUATERNION & Q3,
		const float f, const float g, MATH3DQUATERNION & out)
	{
		MATH3DQUATERNION Q12, Q13;
		float fg = f + g;

		MathQuaternionSLERP(Q1, Q2, fg, Q12);
		MathQuaternionSLERP(Q1, Q3, fg, Q13);
		MathQuaternionSLERP(Q12, Q13, g / fg, out);
	}

	//>> Задаёт кватернион в барицентрических координатах
	inline void MathQuaternionBaryCentric(const MATH3DQUATERNION & Q1, const MATH3DQUATERNION & Q2, const MATH3DQUATERNION & Q3,
		const float f, const float g, const float cos_linear, MATH3DQUATERNION & out)
	{
		MATH3DQUATERNION Q12, Q13;
		float fg = f + g;

		MathQuaternionSLERP(Q1, Q2, fg, cos_linear, Q12);
		MathQuaternionSLERP(Q1, Q3, fg, cos_linear, Q13);
		MathQuaternionSLERP(Q12, Q13, g / fg, cos_linear, out);
	}

	//>> Поиск точки пересечения луча и плоскости (плоскость по 3 точкам)
	inline MATH3DVEC MathRayToPlaneIntersect(const MATH3DVEC & p, const MATH3DVEC & p2, const MATH3DVEC & p3,
		const MATH3DVEC & rp, const MATH3DVEC & rp2, bool& NON)
	{
		MATH3DVEC vec1(p2.x-p.x, p2.y-p.y, p2.z-p.z);
		MATH3DVEC vec2(p3.x-p.x, p3.y-p.y, p3.z-p.z);
		MATH3DVEC normal(vec1.y*vec2.z - vec1.z*vec2.y,
						 vec1.z*vec2.x - vec1.x*vec2.z,
						 vec1.x*vec2.y - vec1.y*vec2.x);

		normal._normalize_check();
		NON = 0;
		MATH3DVEC sub(rp - p);
		MATH3DVEC dir(rp2 - rp);
		float d = MathDotVec(normal, sub);		// длина по нормали от точки ray_point до плоскости
		float e = MathDotVec(normal, dir);		// длина по нормали от точки ray_point до ray_point2

		//printf("\n%f %f %f",   normal.x, normal.y, normal.z);
		//printf("\n%f %f %f\n", subd.x, subd.y, subd.z);

		//printf("\n%f %f", e, d);

		if (e) return (rp + (dir * (-d / e)));	// точка пересечения
		else if (!d) return (rp);				// прямая лежит в плоскости
		else NON = 1;							// прямая параллельна плоскости

		return MATH3DVEC(0,0,0);
	} //*/
	;
	//>> Поиск точки пересечения луча и плоскости (плоскость по 3 точкам)
	inline void MathRayToPlaneIntersect(const MATH3DVEC & p, const MATH3DVEC & p2, const MATH3DVEC & p3,
		const MATH3DVEC & rp, const MATH3DVEC & rp2, MATH3DVEC& out, bool& NON)
	{
		MATH3DVEC vec1(p2.x-p.x, p2.y-p.y, p2.z-p.z);
		MATH3DVEC vec2(p3.x-p.x, p3.y-p.y, p3.z-p.z);
		MATH3DVEC normal(vec1.y*vec2.z - vec1.z*vec2.y,
						 vec1.z*vec2.x - vec1.x*vec2.z,
						 vec1.x*vec2.y - vec1.y*vec2.x);

		normal._normalize_check();
		NON = 0;
		MATH3DVEC sub(rp - p);
		MATH3DVEC dir(rp2 - rp);
		float d = MathDotVec(normal, sub);		// длина по нормали от точки ray_point до плоскости
		float e = MathDotVec(normal, dir);		// длина по нормали от точки ray_point до ray_point2

		//printf("\n%f %f %f",   normal.x, normal.y, normal.z);
		//printf("\n%f %f %f\n", subd.x, subd.y, subd.z);
		//printf("\n%f %f", e, d);

		if (e)       { out = rp + (dir * (-d / e)); return; }	// точка пересечения
		else if (!d) { out = rp;                    return; }	// прямая лежит в плоскости
		else NON = 1;											// прямая параллельна плоскости

		out.x = out.y = out.z = 0;
	}

	//>> Поиск точки пересечения луча и плоскости (плоскость по точке и нормали)
	inline MATH3DVEC MathRayToPlaneIntersect(const MATH3DVEC & p, const MATH3DVEC & normal,
		const MATH3DVEC & rp, const MATH3DVEC & rp2, bool& NON)
	{
		NON = 0;
		MATH3DVEC sub(rp - p);					// 
		MATH3DVEC dir(rp2 - rp);				// направление взгляда
		float d = MathDotVec(normal, sub);		// длина по нормали от точки ray_point до плоскости
		float e = MathDotVec(normal, dir);		// длина по нормали от точки ray_point до ray_point2

		if (e) return (rp + (dir * (-d / e)));	// точка пересечения
		else if (!d) return (rp);				// прямая лежит в плоскости
		else NON = 1;							// прямая параллельна плоскости

		return MATH3DVEC(0,0,0);
	}//*/
	;
	//>> Поиск точки пересечения луча и плоскости (плоскость по точке и нормали)
	inline void MathRayToPlaneIntersect(const MATH3DVEC & p, const MATH3DVEC & normal,
		const MATH3DVEC & rp, const MATH3DVEC & rp2, MATH3DVEC& out, bool& NON)
	{
		NON = 0;
		MATH3DVEC sub(rp - p);					// 
		MATH3DVEC dir(rp2 - rp);				// направление взгляда
		float d = MathDotVec(normal, sub);		// длина по нормали от точки ray_point до плоскости
		float e = MathDotVec(normal, dir);		// длина по нормали от точки ray_point до ray_point2

		if (e)       { out = rp + (dir * (-d / e)); return; }	// точка пересечения
		else if (!d) { out = rp;                    return; }	// прямая лежит в плоскости
		else NON = 1;											// прямая параллельна плоскости

		out.x = out.y = out.z = 0;
	}

	//>> Поиск точки пересечения луча и плоскости
	inline MATH3DVEC MathRayToPlaneIntersect(const MATH3DPLANE plane, const MATH3DVEC rp, const MATH3DVEC rp2, bool& NON)
	{
		NON = 0;
		MATH3DVEC sub(rp - plane.P);
		MATH3DVEC dir(rp2 - rp);
		float d = MathDotVec(plane.N, sub);		// длина по нормали от точки ray_point до плоскости
		float e = MathDotVec(plane.N, dir);		// длина по нормали от точки ray_point до ray_point2

		if (e) return (rp + (dir * (-d / e)));	// точка пересечения
		else if (!d) return (rp);				// прямая лежит в плоскости
		else NON = 1;							// прямая параллельна плоскости

		return MATH3DVEC(0,0,0);

	//	NON = 0;
	//	MATH3DVEC sub(rp - plane.P);
	//	MATH3DVEC dir(rp2 - rp);
	//	FLOAT d = MathDotVec(plane.N, sub);
	//	FLOAT e = MathDotVec(plane.N, dir);
	//
	//	if (e) return (rp - (dir * ((plane.d + MathDotVec(plane.N, rp)) / e)));
	//	else if (!d) return (rp);
	//	else NON = 1;
	//	
	//	return MATH3DVEC(0,0,0);
	} //*/
	;
	//>> Поиск точки пересечения луча и плоскости
	inline void MathRayToPlaneIntersect(const MATH3DPLANE & plane,
		const MATH3DVEC & rp, const MATH3DVEC & rp2, MATH3DVEC& out, bool& NON)
	{
		NON = 0;
		MATH3DVEC sub(rp - plane.P);
		MATH3DVEC dir(rp2 - rp);
		float d = MathDotVec(plane.N, sub);		// длина по нормали от точки ray_point до плоскости
		float e = MathDotVec(plane.N, dir);		// длина по нормали от точки ray_point до ray_point2

		if (e)       { out = rp + (dir * (-d / e)); return; } // точка пересечения
		else if (!d) { out = rp;                    return; } // прямая лежит в плоскости
		else NON = 1;										  // прямая параллельна плоскости

		out.x = out.y = out.z = 0;
	}

	//>> Составление матрицы перемещения
	inline MATH3DMATRIX MathTranslateMatrix(const float dx, const float dy, const float dz)
	{
		MATH_TRANSLATE_MATRIX(dx, dy, dz)
		return MT;
	} //*/
	;
	//>> Составление матрицы перемещения
	inline MATH3DMATRIX MathTranslateMatrix(const MATH3DVEC & pos)
	{
		MATH_TRANSLATE_MATRIX(pos.x, pos.y, pos.z)
		return MT;
	}//*/
	;
	//>> Составление матрицы перемещения
	inline void MathTranslateMatrix(const float dx, const float dy, const float dz, MATH3DMATRIX& M)
	{
		MATH_TRANSLATE_MATRIX(dx, dy, dz)
		M = MT;
	}

	//>> Составление матрицы перемещения
	inline void MathTranslateMatrix(const MATH3DVEC & pos, MATH3DMATRIX& M)
	{	
		MATH_TRANSLATE_MATRIX(pos.x, pos.y, pos.z)
		M = MT;
	}

	//>> Составление матрицы масштабирования
	inline MATH3DMATRIX MathScaleMatrix(const float sx, const float sy, const float sz)
	{
		MATH_SCALE_MATRIX(sx, sy, sz)
		return MS;
	} //*/
	;
	//>> Составление матрицы масштабирования
	inline MATH3DMATRIX MathScaleMatrix(const MATH3DVEC & scale)
	{
		MATH_SCALE_MATRIX(scale.x, scale.y, scale.z)
		return MS;
	} //*/
	;
	//>> Составление матрицы масштабирования
	inline void MathScaleMatrix(const float sx, const float sy, const float sz, MATH3DMATRIX& M)
	{
		MATH_SCALE_MATRIX(sx, sy, sz)
		M = MS;
	}

	//>> Составление матрицы масштабирования
	inline void MathScaleMatrix(const MATH3DVEC & scale, MATH3DMATRIX& M)
	{
		MATH_SCALE_MATRIX(scale.x, scale.y, scale.z)
		M = MS;
	}

	//>> Составление матрицы вращения (вокруг оси X)
/*	inline MATH3DMATRIX MathRotateMatrix_X(const float angle)
	{
		MATH_ROTATE_MATRIX_X(angle)
		return MRx;
	} //*/
	;
	//>> Составление матрицы вращения (вокруг оси Y)
/*	inline MATH3DMATRIX MathRotateMatrix_Y(const float angle)
	{
		MATH_ROTATE_MATRIX_Y(angle)
		return MRy;
	} //*/
	;
	//>> Составление матрицы вращения (вокруг оси Z)
/*	inline MATH3DMATRIX MathRotateMatrix_Z(const float angle)
	{
		MATH_ROTATE_MATRIX_Z(angle)
		return MRz;
	} //*/
	;
	//>> Составление матрицы вращения (вокруг оси X)
/*	inline MATH3DMATRIX MathRotateMatrix_X(const float sin, const float cos)
	{
		MATH3DMATRIX M;

		M._22 = cos;  M._33 =  M._22;
		M._23 = sin;  M._32 = -M._23;  return M;
	} //*/
	;
	//>> Составление матрицы вращения (вокруг оси Y)
/*	inline MATH3DMATRIX MathRotateMatrix_Y(const float sin, const float cos)
	{
		MATH3DMATRIX M;

		M._11 = cos;  M._33 =  M._11;
		M._31 = sin;  M._13 = -M._31;  return M;
	} //*/
	;
	//>> Составление матрицы вращения (вокруг оси Z)
/*	inline MATH3DMATRIX MathRotateMatrix_Z(const float sin, const float cos)
	{
		MATH3DMATRIX M;

		M._11 = cos;  M._22 =  M._11;
		M._12 = sin;  M._21 = -M._12;  return M;
	} //*/
	;
	//>> Составление матрицы вращения
/*	inline MATH3DMATRIX MathRotateMatrix(const float ax, const float ay, const float az)
	{
		MATH_ROTATE_MATRIX_A(ax,ay,az)
		return MRz;
	} //*/
	;
	//>> Составление матрицы вращения
/*	inline MATH3DMATRIX MathRotateMatrix(const MATH3DVEC & angle)
	{
		MATH_ROTATE_MATRIX_A(angle.x, angle.y, angle.z)
		return MRz;
	} //*/
	;
	//>> Составление матрицы вращения (обратное преобразование)
/*	inline MATH3DMATRIX MathRotateMatrixBack(const float ax, const float ay, const float az)
	{
		MATH_ROTATE_MATRIX_X(-ax)
		MATH_ROTATE_MATRIX_Y(-ay)
		MATH_ROTATE_MATRIX_Z(-az)
		MRy *= MRx;
		MRy *= MRz;
		return MRy;
	} //*/
	;
	//>> Составление матрицы вращения (обратное преобразование)
/*	inline MATH3DMATRIX MathRotateMatrixBack(const MATH3DVEC & angle)
	{
		MATH_ROTATE_MATRIX_X(-angle.x)
		MATH_ROTATE_MATRIX_Y(-angle.y)
		MATH_ROTATE_MATRIX_Z(-angle.z)
		MRy *= MRx;
		MRy *= MRz;
		return MRy;
	} //*/
	;
	//>> Составление матрицы вращения (вокруг оси X)
/*	inline void MathRotateMatrix_X(const float angle, MATH3DMATRIX& M)
	{
		MATH_ROTATE_MATRIX_X(angle)
		M = MRx;	
	}//*/
	;
	//>> Составление матрицы вращения (вокруг оси Y)
/*	inline void MathRotateMatrix_Y(const float angle, MATH3DMATRIX& M)
	{
		MATH_ROTATE_MATRIX_Y(angle)
		M = MRy;
	}//*/
	;
	//>> Составление матрицы вращения (вокруг оси Z)
/*	inline void MathRotateMatrix_Z(const float angle, MATH3DMATRIX& M)
	{
		MATH_ROTATE_MATRIX_Z(angle)
		M = MRz;
	}//*/
	;
	//>> Составление матрицы вращения
/*	inline void MathRotateMatrix(const float ax, const float ay, const float az, MATH3DMATRIX& M)
	{
		MATH_ROTATE_MATRIX_A(ax,ay,az)
		M = MRz;
	}//*/
	;
	//>> Составление матрицы вращения
/*	inline void MathRotateMatrix(const MATH3DVEC & angle, MATH3DMATRIX& M)
	{
		MATH_ROTATE_MATRIX_A(angle.x, angle.y, angle.z)
		M = MRz;
	}//*/
	;
	//>> Составление матрицы вращения (обратное преобразование)
/*	inline void MathRotateMatrixBack(const float ax, const float ay, const float az, MATH3DMATRIX& M)
	{
		MATH_ROTATE_MATRIX_X(-ax)
		MATH_ROTATE_MATRIX_Y(-ay)
		MATH_ROTATE_MATRIX_Z(-az)
		MRy *= MRx;
		MRy *= MRz;
		M = MRy;
	} //*/
	;
	//>> Составление матрицы вращения (обратное преобразование)
/*	inline void MathRotateMatrixBack(const MATH3DVEC & angle, MATH3DMATRIX& M)
	{
		MATH_ROTATE_MATRIX_X(-angle.x)
		MATH_ROTATE_MATRIX_Y(-angle.y)
		MATH_ROTATE_MATRIX_Z(-angle.z)
		MRy *= MRx;
		MRy *= MRz;
		M = MRy;
	} //*/
	;
	//>> Составление матрицы вращения вокруг произвольной оси
	inline MATH3DMATRIX MathRotationAxisMatrix(const MATH3DVEC & axis, const float angle)
	{
		MATH3DMATRIX M;

		float  sin  = CTAB::sinA(angle);
		float  cos  = CTAB::cosA(angle);
		float _cos  = 1.0f - cos;
		float _cosX = _cos * axis.x;
		float _cosY = _cos * axis.y;
		float _cosZ = _cos * axis.z;
		float _sinX =  sin * axis.x;
		float _sinY =  sin * axis.y;
		float _sinZ =  sin * axis.z;

		M._11 = _cosX * axis.x + cos;
		M._21 = _cosX * axis.y - _sinZ;
		M._31 = _cosX * axis.z + _sinY;
		M._12 = _cosY * axis.x + _sinZ;
		M._22 = _cosY * axis.y + cos;
		M._32 = _cosY * axis.z - _sinX;
		M._13 = _cosZ * axis.x - _sinY;
		M._23 = _cosZ * axis.y + _sinX;
		M._33 = _cosZ * axis.z + cos;

		return M;
	} //*/
	;
	//>> Составление матрицы вращения вокруг произвольной оси
	inline void MathRotationAxisMatrix(const MATH3DVEC & axis, const float angle, MATH3DMATRIX& M)
	{
		M = MATH3DMATRIX();

		float  sin  = CTAB::sinA(angle);
		float  cos  = CTAB::cosA(angle);
		float _cos  = 1.0f - cos;
		float _cosX = _cos * axis.x;
		float _cosY = _cos * axis.y;
		float _cosZ = _cos * axis.z;
		float _sinX =  sin * axis.x;
		float _sinY =  sin * axis.y;
		float _sinZ =  sin * axis.z;

		M._11 = _cosX * axis.x + cos;
		M._21 = _cosX * axis.y - _sinZ;
		M._31 = _cosX * axis.z + _sinY;
		M._12 = _cosY * axis.x + _sinZ;
		M._22 = _cosY * axis.y + cos;
		M._32 = _cosY * axis.z - _sinX;
		M._13 = _cosZ * axis.x - _sinY;
		M._23 = _cosZ * axis.y + _sinX;
		M._33 = _cosZ * axis.z + cos;
	}

	//>> Составление матрицы вращения из кватерниона
	inline MATH3DMATRIX MathRotateMatrix(const MATH3DQUATERNION & Q)
	{
		MATH_ROTATE_MATRIX(Q.x, Q.y, Q.z, Q.w)
		return MR;
	} //*/
	;
	//>> Составление матрицы вращения из кватерниона
	inline void MathRotateMatrix(const MATH3DQUATERNION & Q, MATH3DMATRIX& M)
	{
		MATH_ROTATE_MATRIX(Q.x, Q.y, Q.z, Q.w)
		M = MR;
	}

	//>> Составление комплексной матрицы позиции в мире
/*	inline MATH3DMATRIX MathWorldMatrix(const float dx, const float dy, const float dz,
										const float ax, const float ay, const float az,
										const float sx, const float sy, const float sz)
	{
		return MathScaleMatrix(sx, sy, sz) * MathRotateMatrix(ax, ay, az) * MathTranslateMatrix(dx, dy, dz);
	}//*/
	;
	//>> Составление комплексной матрицы позиции в мире
/*	inline MATH3DMATRIX MathWorldMatrix(const MATH3DVEC & P, const MATH3DVEC & A, const MATH3DVEC & S)
	{
		MATH_TRANSLATE_MATRIX(P.x, P.y, P.z)
		MATH_ROTATE_MATRIX_A(A.x, A.y, A.z)
		MATH_SCALE_MATRIX(S.x, S.y, S.z)
		MS *= MRz;
		MS *= MT;
		return MS;
	} //*/
	;
	//>> Составление комплексной матрицы позиции в мире
/*	inline void MathWorldMatrix(const float dx, const float dy, const float dz,
								const float ax, const float ay, const float az,
								const float sx, const float sy, const float sz, MATH3DMATRIX& M)
	{
		MATH_TRANSLATE_MATRIX(dx, dy, dz)
		MATH_ROTATE_MATRIX_A(ax, ay, az)
		MATH_SCALE_MATRIX(sx, sy, sz)
		MS *= MRz;
		MS *= MT;
		M   = MS;
	} //*/
	;
	//>> Составление комплексной матрицы позиции в мире
/*	inline void MathWorldMatrix(const MATH3DVEC & P, const MATH3DVEC & A, const MATH3DVEC & S, MATH3DMATRIX& M)
	{
		MATH_TRANSLATE_MATRIX(P.x, P.y, P.z)
		MATH_ROTATE_MATRIX_A(A.x, A.y, A.z)
		MATH_SCALE_MATRIX(S.x, S.y, S.z)

		MS *= MRz;
		MS *= MT;
		M   = MS;
	} //*/
	;
	//>> Составление комплексной матрицы позиции в мире
	inline MATH3DMATRIX MathWorldMatrix(const MATH3DVEC & P, const MATH3DQUATERNION & Q, const MATH3DVEC & S)
	{
		MATH_TRANSLATE_MATRIX(P.x, P.y, P.z)
		MATH_ROTATE_MATRIX(Q.x, Q.y, Q.z, Q.w)
		MATH_SCALE_MATRIX(S.x, S.y, S.z)

		MS *= MR;
		MS *= MT;
		return MS;
	} //*/
	;
	//>> Составление комплексной матрицы позиции в мире
	inline void MathWorldMatrix(const MATH3DVEC & P, const MATH3DQUATERNION & Q, const MATH3DVEC & S, MATH3DMATRIX& M)
	{
		MATH_TRANSLATE_MATRIX(P.x, P.y, P.z)
		MATH_ROTATE_MATRIX(Q.x, Q.y, Q.z, Q.w)
		MATH_SCALE_MATRIX(S.x, S.y, S.z)

		MS *= MR;
		MS *= MT;
		M = MS;
	}

	//>> Составление комплексной матрицы позиции в мире
/*	inline MATH3DMATRIX MathWorldMatrix(const MATH3DMATRIX & mPos, const MATH3DMATRIX & mAngle, const MATH3DMATRIX & mScale)
	{
		MATH3DMATRIX M = mScale;
		M *= mAngle;
		M *= mPos;
		return M;
	}//*/
	;
	//>> Составление комплексной матрицы позиции в мире
/*	inline void MathWorldMatrix(const MATH3DMATRIX & mPos, const MATH3DMATRIX & mAngle, const MATH3DMATRIX & mScale, MATH3DMATRIX& out)
	{
		out  = mScale;
		out *= mAngle;
		out *= mPos;
	} //*/
	;
	//>> Составление комплексной матрицы позиции в мире (обратное преобразование)
/*	inline MATH3DMATRIX MathWorldMatrixBack(const float dx, const float dy, const float dz,
											const float ax, const float ay, const float az,
											const float sx, const float sy, const float sz)
	{
		MATH_TRANSLATE_MATRIX(-dx, -dy, -dz)
		MATH_SCALE_MATRIX(1.f/sx, 1.f/sy, 1.f/sz)
		
		MATH_ROTATE_MATRIX_X(-ax)
		MATH_ROTATE_MATRIX_Y(-ay)
		MATH_ROTATE_MATRIX_Z(-az)
		MRy *= MRx;
		MRy *= MRz;
		
		MT *= MRy;
		MT *= MS;

		return MT;
	} //*/
	;
	//>> Составление комплексной матрицы позиции в мире (обратное преобразование)
/*	inline MATH3DMATRIX MathWorldMatrixBack(const MATH3DVEC & P, const MATH3DVEC & A, const MATH3DVEC & S)
	{
		MATH_TRANSLATE_MATRIX(-P.x, -P.y, -P.z)
		MATH_SCALE_MATRIX(1.f/S.x, 1.f/S.y, 1.f/S.z)

		MATH_ROTATE_MATRIX_X(-A.x)
		MATH_ROTATE_MATRIX_Y(-A.y)
		MATH_ROTATE_MATRIX_Z(-A.z)
		MRy *= MRx;
		MRy *= MRz;
		
		MT *= MRy;
		MT *= MS;

		return MT;
	} //*/
	;
	//>> Составление комплексной матрицы позиции в мире (обратное преобразование)
/*	inline void MathWorldMatrixBack(const float dx, const float dy, const float dz,
									const float ax, const float ay, const float az,
									const float sx, const float sy, const float sz, MATH3DMATRIX& M)
	{
		MATH_TRANSLATE_MATRIX(-dx, -dy, -dz)
		MATH_SCALE_MATRIX(1.f/sx, 1.f/sy, 1.f/sz)
		
		MATH_ROTATE_MATRIX_X(-ax)
		MATH_ROTATE_MATRIX_Y(-ay)
		MATH_ROTATE_MATRIX_Z(-az)
		MRy *= MRx;
		MRy *= MRz;
		
		MT *= MRy;
		MT *= MS;

		M = MT;
	} //*/
	;
	//>> Составление комплексной матрицы позиции в мире (обратное преобразование)
/*	inline void MathWorldMatrixBack(const MATH3DVEC & P, const MATH3DVEC & A, const MATH3DVEC & S, MATH3DMATRIX& M)
	{
		MATH_TRANSLATE_MATRIX(-P.x, -P.y, -P.z)
		MATH_SCALE_MATRIX(1.f/S.x, 1.f/S.y, 1.f/S.z)

		MATH_ROTATE_MATRIX_X(-A.x)
		MATH_ROTATE_MATRIX_Y(-A.y)
		MATH_ROTATE_MATRIX_Z(-A.z)
		MRy *= MRx;
		MRy *= MRz;
		
		MT *= MRy;
		MT *= MS;

		M = MT;
	} //*/
	;
	//>> Выполняет векторное произведение 3 векторов VEC4 (CROSS product)
	inline MATH3DVEC4 MathCrossVec(const MATH3DVEC4 & vec1, const MATH3DVEC4 & vec2, const MATH3DVEC4 & vec3)
	{
		return MATH3DVEC4(
			vec1.y * (vec2.z * vec3.w - vec3.z * vec2.w) - vec1.z * (vec2.y * vec3.w - vec3.y * vec2.w) + vec1.w * (vec2.y * vec3.z - vec2.z * vec3.y)  ,
		  -(vec1.x * (vec2.z * vec3.w - vec3.z * vec2.w) - vec1.z * (vec2.x * vec3.w - vec3.x * vec2.w) + vec1.w * (vec2.x * vec3.z - vec3.x * vec2.z)) ,
			vec1.x * (vec2.y * vec3.w - vec3.y * vec2.w) - vec1.y * (vec2.x * vec3.w - vec3.x * vec2.w) + vec1.w * (vec2.x * vec3.y - vec3.x * vec2.y)  ,
		  -(vec1.x * (vec2.y * vec3.z - vec3.y * vec2.z) - vec1.y * (vec2.x * vec3.z - vec3.x * vec2.z) + vec1.z * (vec2.x * vec3.y - vec3.x * vec2.y)) );
	}; //*/
	;
	//>> Выполняет векторное произведение 3 векторов VEC4 (CROSS product)
	inline void MathCrossVec(const MATH3DVEC4 & vec1, const MATH3DVEC4 & vec2, const MATH3DVEC4 & vec3, MATH3DVEC4& out)
	{
		out = MATH3DVEC4(
			vec1.y * (vec2.z * vec3.w - vec3.z * vec2.w) - vec1.z * (vec2.y * vec3.w - vec3.y * vec2.w) + vec1.w * (vec2.y * vec3.z - vec2.z * vec3.y)  ,
		  -(vec1.x * (vec2.z * vec3.w - vec3.z * vec2.w) - vec1.z * (vec2.x * vec3.w - vec3.x * vec2.w) + vec1.w * (vec2.x * vec3.z - vec3.x * vec2.z)) ,
			vec1.x * (vec2.y * vec3.w - vec3.y * vec2.w) - vec1.y * (vec2.x * vec3.w - vec3.x * vec2.w) + vec1.w * (vec2.x * vec3.y - vec3.x * vec2.y)  ,
		  -(vec1.x * (vec2.y * vec3.z - vec3.y * vec2.z) - vec1.y * (vec2.x * vec3.z - vec3.x * vec2.z) + vec1.z * (vec2.x * vec3.y - vec3.x * vec2.y)) );
	};

	//>> Определитель матрицы
	inline float MathMatrixDeterminant(const MATH3DMATRIX & M)
	{
		MATH3DVEC4 v1, v2, v3;

		v1.x = M._11;   v1.y = M._21;   v1.z = M._31;   v1.w = M._41;
		v2.x = M._12;   v2.y = M._22;   v2.z = M._32;   v2.w = M._42;
		v3.x = M._13;   v3.y = M._23;   v3.z = M._33;   v3.w = M._43;

		MATH3DVEC4 minor; MathCrossVec(v1, v2, v3, minor);

		return -( (M._14 * minor.x) + (M._24 * minor.y) + (M._34 * minor.z) + (M._44 * minor.w) );
	}

	//>> Определитель матрицы
	inline void MathMatrixDeterminant(const MATH3DMATRIX & M, float& det)
	{
		MATH3DVEC4 v1, v2, v3;

		v1.x = M._11;   v1.y = M._21;   v1.z = M._31;   v1.w = M._41;
		v2.x = M._12;   v2.y = M._22;   v2.z = M._32;   v2.w = M._42;
		v3.x = M._13;   v3.y = M._23;   v3.z = M._33;   v3.w = M._43;

		MATH3DVEC4 minor;	MathCrossVec(v1, v2, v3, minor);

		det = -( (M._14 * minor.x) + (M._24 * minor.y) + (M._34 * minor.z) + (M._44 * minor.w) );
	}

	//>> Составления матрицы обратного преобразования
	inline MATH3DMATRIX MathInverseMatrix(const MATH3DMATRIX & M, float& det)
	{
		MATH3DMATRIX out;
		MATH3DVEC4 v, _v[3];

		MathMatrixDeterminant(M, det);		if (!det) return MATH3DMATRIX();

		for (int i=0; i<4; i++) {			// i 0      1      2      3
			for (int n, j=0; j<4; j++)	{	// j 1,2,3  0,2,3  0,1,3  0,1,2 -> 234 134 124 123
				if (j != i )
			{
				n = j;
				if ( j > i ) n--;			// n 0,1,2  0,1,2  0,1,2  0,1,2 -> [012].xyzw = (234 134 124 123) [0123]
				_v[n].x = M._[j][0];
				_v[n].y = M._[j][1];
				_v[n].z = M._[j][2];
				_v[n].w = M._[j][3];
			} }

			MathCrossVec(_v[0],_v[1],_v[2],v);

			float f1, f2;
			switch(i)
			{
				case 0: f1 =  1; break;
				case 1: f1 = -1; break;
				case 2: f1 =  1; break;
				case 3: f1 = -1; break;
			}

			for (int j=0; j<4; j++)
			{
				switch(j)
				{
					case 0: f2 = v.x; break;
					case 1: f2 = v.y; break;
					case 2: f2 = v.z; break;
					case 3: f2 = v.w; break;
				}
				out._[j][i] = f1 * f2 / det;
			}}

	//	MATH3DMATRIX out;
	//	MATH3DVEC4 v, v1, v2, v3;
	//
	//	det = MathMatrixDeterminant(M);		if (!det) return MATH3DMATRIX();
	//
	//	v1.x = M._21;	v2.x = M._31;	v3.x = M._41;
	//	v1.y = M._22;	v2.y = M._32;	v3.y = M._42;
	//	v1.z = M._23;	v2.z = M._33;	v3.z = M._43;
	//	v1.w = M._24;	v2.w = M._34;	v3.w = M._44;		v = MathCrossVec(v1,v2,v3);
	//
	//	out._11 =  v.x / det;
	//	out._21 =  v.y / det;
	//	out._31 =  v.z / det;
	//	out._41 =  v.w / det;
	//
	//	v1.x = M._11;	v2.x = M._31;	v3.x = M._41;
	//	v1.y = M._12;	v2.y = M._32;	v3.y = M._42;
	//	v1.z = M._13;	v2.z = M._33;	v3.z = M._43;
	//	v1.w = M._14;	v2.w = M._34;	v3.w = M._44;		v = MathCrossVec(v1,v2,v3);
	//
	//	out._12 = -v.x / det;
	//	out._22 = -v.y / det;
	//	out._32 = -v.z / det;
	//	out._42 = -v.w / det;
	//
	//	v1.x = M._11;	v2.x = M._21;	v3.x = M._41;
	//	v1.y = M._12;	v2.y = M._22;	v3.y = M._42;
	//	v1.z = M._13;	v2.z = M._23;	v3.z = M._43;
	//	v1.w = M._14;	v2.w = M._24;	v3.w = M._44;		v = MathCrossVec(v1,v2,v3);
	//
	//	out._13 =  v.x / det;
	//	out._23 =  v.y / det;
	//	out._33 =  v.z / det;
	//	out._43 =  v.w / det;
	//
	//	v1.x = M._11;	v2.x = M._21;	v3.x = M._31;
	//	v1.y = M._12;	v2.y = M._22;	v3.y = M._32;
	//	v1.z = M._13;	v2.z = M._23;	v3.z = M._33;
	//	v1.w = M._14;	v2.w = M._24;	v3.w = M._34;		v = MathCrossVec(v1,v2,v3);
	//
	//	out._14 = -v.x / det;
	//	out._24 = -v.y / det;
	//	out._34 = -v.z / det;
	//	out._44 = -v.w / det;
		
		return out;
	} //*/
	;
	//>> Составления матрицы обратного преобразования
	inline void MathInverseMatrix(const MATH3DMATRIX & M, float& det, MATH3DMATRIX& out)
	{
		MATH3DVEC4 v, _v[3];

		MathMatrixDeterminant(M, det);		if (!det) { out._default(); return; }

		for (int i=0; i<4; i++) {			// i 0      1      2      3
			for (int n, j=0; j<4; j++)	{	// j 1,2,3  0,2,3  0,1,3  0,1,2 -> 234 134 124 123
				if (j != i )
			{
				n = j;
				if ( j > i ) n--;			// n 0,1,2  0,1,2  0,1,2  0,1,2 -> [012].xyzw = (234 134 124 123) [0123]
				_v[n].x = M._[j][0];
				_v[n].y = M._[j][1];
				_v[n].z = M._[j][2];
				_v[n].w = M._[j][3];
			} }

			MathCrossVec(_v[0],_v[1],_v[2],v);

			float f1, f2;
			switch(i)
			{
				case 0: f1 =  1; break;
				case 1: f1 = -1; break;
				case 2: f1 =  1; break;
				case 3: f1 = -1; break;
			}

			for (int j=0; j<4; j++)
			{		
				switch(j)
				{
					case 0: f2 = v.x; break;
					case 1: f2 = v.y; break;
					case 2: f2 = v.z; break;
					case 3: f2 = v.w; break;
				}
				out._[j][i] = f1 * f2 / det; 
			} }
	}

	//>> Составление транспонированной матрицы
	inline MATH3DMATRIX MathTransposeMatrix(const MATH3DMATRIX & M)
	{
		MATH3DMATRIX out;

		for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			out._[i][j] = M._[j][i];

		return out;
	}
	;
	//>> Составление транспонированной матрицы
	inline void MathTransposeMatrix(const MATH3DMATRIX & M, MATH3DMATRIX& out)
	{
	/*	MATH3DMATRIX _out;

		for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			_out._[i][j] = M._[j][i];

		out = _out; //*/
		
		out = M;

		_SWAP(out._13, out._31);
		_SWAP(out._21, out._12);
		_SWAP(out._32, out._23);
		_SWAP(out._41, out._14);
		_SWAP(out._42, out._24);
		_SWAP(out._43, out._34);
	}
	;
	//>> Составление транспонированной матрицы
/*	inline void MathTransposeMatrix(MATH3DMATRIX& in_out)
	{
		_SWAP(in_out._13, in_out._31);
		_SWAP(in_out._21, in_out._12);
		_SWAP(in_out._32, in_out._23);
		_SWAP(in_out._41, in_out._14);
		_SWAP(in_out._42, in_out._24);
		_SWAP(in_out._43, in_out._34);
	} //*/
	;
	//>> Составляет левостороннюю матрицу перспективной проекции (FOV based)
	inline MATH3DMATRIX MathPerspectiveFovLHMatrix(const float fovy, const float aspect, const float zn, const float zf)
	{
		MATH3DMATRIX M;
		float t = 1.0f / CTAB::tanA(fovy*0.5f); //  / 2.0f);
		float z = zn - zf;

		M._11 = t / aspect;
		M._22 = t;
		M._33 = zf / -z;
		M._34 = 1.0f;
		M._43 = (zf * zn) / z;
		M._44 = 0.0f;

		return M;
	} //*/
	;
	//>> Составляет левостороннюю матрицу перспективной проекции (FOV based)
	inline void MathPerspectiveFovLHMatrix(const float fovy, const float aspect, const float zn, const float zf, MATH3DMATRIX& M)
	{
		M._default();
		float t = 1.0f / CTAB::tanA(fovy*0.5f); // / 2.0f);
		float z = zn - zf;

		M._11 = t / aspect;
		M._22 = t;
		M._33 = zf / -z;
		M._34 = 1.0f;
		M._43 = (zf * zn) / z;
		M._44 = 0.0f;
	}

	//>> Составляет правостороннюю матрицу перспективной проекции (FOV based)
	inline MATH3DMATRIX MathPerspectiveFovRHMatrix(const float fovy, const float aspect, const float zn, const float zf)
	{
		MATH3DMATRIX M;
		float t = CTAB::tanA(fovy*0.5f); // / 2.0f);
		float z = zn - zf;

		M._11 = 1.0f / (aspect * t);
		M._22 = 1.0f / t;
		M._33 = zf / z;
		M._34 = -1.0f;
		M._43 = (zf * zn) / z;
		M._44 = 0.0f;

		return M;
	} //*/
	;
	//>> Составляет правостороннюю матрицу перспективной проекции (FOV based)
	inline void MathPerspectiveFovRHMatrix(const float fovy, const float aspect, const float zn, const float zf, MATH3DMATRIX& M)
	{
		M._default();
		float t = CTAB::tanA(fovy*0.5f); // / 2.0f);
		float z = zn - zf;

		M._11 = 1.0f / (aspect * t);
		M._22 = 1.0f / t;
		M._33 = zf / z;
		M._34 = -1.0f;
		M._43 = (zf * zn) / z;
		M._44 = 0.0f;
	}

	//>> Составляет левостороннюю матрицу перспективной проекции
	inline MATH3DMATRIX MathPerspectiveLHMatrix(const float w, const float h, const float zn, const float zf)
	{
		MATH3DMATRIX M;
		float z  = zn - zf;
		float z2 = 2.0f * zn;

		M._11 = z2 / w;
		M._22 = z2 / h;
		M._33 = zf / -z;
		M._43 = zn * zf / z;
		M._34 = 1.0f;
		M._44 = 0.0f;

		return M;
	} //*/
	;
	//>> Составляет левостороннюю матрицу перспективной проекции
	inline void MathPerspectiveLHMatrix(const float w, const float h, const float zn, const float zf, MATH3DMATRIX& M)
	{
		M._default();
		float z  = zn - zf;
		float z2 = 2.0f * zn;

		M._11 = z2 / w;
		M._22 = z2 / h;
		M._33 = zf / -z;
		M._43 = zn * zf / z;
		M._34 = 1.0f;
		M._44 = 0.0f;
	}

	//>> Составляет правостороннюю матрицу перспективной проекции
	inline MATH3DMATRIX MathPerspectiveRHMatrix(const float w, const float h, const float zn, const float zf)
	{
		MATH3DMATRIX M;
		float z  = zn - zf;
		float z2 = 2.0f * zn;

		M._11 = z2 / w;
		M._22 = z2 / h;
		M._33 = zf / z;
		M._43 = zn * zf / z;
		M._34 = -1.0f;
		M._44 = 0.0f;

		return M;
	} //*/
	;
	//>> Составляет правостороннюю матрицу перспективной проекции
	inline void MathPerspectiveRHMatrix(const float & w, const float & h, const float & zn, const float & zf, MATH3DMATRIX& M)
	{
		M._default();
		float z  = zn - zf;
		float z2 = 2.0f * zn;

		M._11 = z2 / w;
		M._22 = z2 / h;
		M._33 = zf / z;
		M._43 = zn * zf / z;
		M._34 = -1.0f;
		M._44 = 0.0f;
	}

	//>> Составляет настроенную левостороннюю матрицу перспективной проекции (left, right, bottom, top, z-near, z-far)
	inline MATH3DMATRIX MathPerspectiveOffCenterLHMatrix(const float l, const float r, const float b, const float t, const float zn, const float zf)
	{
		MATH3DMATRIX M;
		float  z = zn - zf;
		float z2 = 2.0f * zn;
		float rl = r - l;
		float bt = b - t;

		M._11 = z2 / rl;
		M._22 = -z2 / bt;
		M._31 = -1.0f - 2.0f * l / rl;
		M._32 = 1.0f + 2.0f * t / bt;
		M._33 = -zf / z;
		M._43 = (zn * zf) / z;
		M._34 = 1.0f;
		M._44 = 0.0f;

		return M;
	} //*/
	;
	//>> Составляет настроенную левостороннюю матрицу перспективной проекции (left, right, bottom, top, z-near, z-far)
	inline void MathPerspectiveOffCenterLHMatrix(const float l, const float r, const float b, const float t, const float zn, const float zf,
		MATH3DMATRIX& M)
	{
		M._default();
		float  z = zn - zf;
		float z2 = 2.0f * zn;
		float rl = r - l;
		float bt = b - t;

		M._11 = z2 / rl;
		M._22 = -z2 / bt;
		M._31 = -1.0f - 2.0f * l / rl;
		M._32 = 1.0f + 2.0f * t / bt;
		M._33 = -zf / z;
		M._43 = (zn * zf) / z;
		M._34 = 1.0f;
		M._44 = 0.0f;
	}

	//>> Составляет настроенную правостороннюю матрицу перспективной проекции (left, right, bottom, top, z-near, z-far)
	inline MATH3DMATRIX MathPerspectiveOffCenterRHMatrix(const float l, const float r, const float b, const float t, const float zn, const float zf)
	{
		MATH3DMATRIX M;
		float  z = zn - zf;
		float z2 = 2.0f * zn;
		float rl = r - l;
		float bt = b - t;

		M._11 = z2 / rl;
		M._22 = -z2 / bt;
		M._31 = 1.0f + 2.0f * l / rl;
		M._32 = -1.0f - 2.0f * t / bt;
		M._33 = zf / z;
		M._43 = (zn * zf) / z;
		M._34 = -1.0f;
		M._44 = 0.0f;

		return M;
	} //*/
	;
	//>> Составляет настроенную правостороннюю матрицу перспективной проекции (left, right, bottom, top, z-near, z-far)
	inline void MathPerspectiveOffCenterRHMatrix(const float l, const float r, const float b, const float t, const float zn, const float zf,
		MATH3DMATRIX& M)
	{
		M._default();
		float  z = zn - zf;
		float z2 = 2.0f * zn;
		float rl = r - l;
		float bt = b - t;

		M._11 = z2 / rl;
		M._22 = -z2 / bt;
		M._31 = 1.0f + 2.0f * l / rl;
		M._32 = -1.0f - 2.0f * t / bt;
		M._33 = zf / z;
		M._43 = (zn * zf) / z;
		M._34 = -1.0f;
		M._44 = 0.0f;
	}

	//>> Составляет левостороннюю матрицу вида (взгляда)
	inline MATH3DMATRIX MathLookAtLHMatrix(const MATH3DVEC & eye, const MATH3DVEC & at, const MATH3DVEC & up)
	{
		MATH3DMATRIX M;

		MATH3DVEC OZ = at - eye;				OZ._normalize_check();
		MATH3DVEC OX = MathCrossVec(up, OZ);	OX._normalize_check();
		MATH3DVEC OY = MathCrossVec(OZ, OX);	OY._normalize_check();

		M._11 = OX.x;   M._12 = OY.x;   M._13 = OZ.x;   M._14 = 0.0f;
		M._21 = OX.y;   M._22 = OY.y;   M._23 = OZ.y;   M._24 = 0.0f;
		M._31 = OX.z;   M._32 = OY.z;   M._33 = OZ.z;   M._34 = 0.0f;

		M._41 = -MathDotVec(OX, eye);
		M._42 = -MathDotVec(OY, eye);
		M._43 = -MathDotVec(OZ, eye);
		M._44 = 1.0f;

		return M;
	} //*/
	;
	//>> Составляет левостороннюю матрицу вида (взгляда)
	inline void MathLookAtLHMatrix(const MATH3DVEC & eye, const MATH3DVEC & at, const MATH3DVEC & up, MATH3DMATRIX& M)
	{
		MATH3DVEC OZ = at - eye;				OZ._normalize_check();
		MATH3DVEC OX = MathCrossVec(up, OZ);	OX._normalize_check();
		MATH3DVEC OY = MathCrossVec(OZ, OX);	OY._normalize_check();

		M._11 = OX.x;   M._12 = OY.x;   M._13 = OZ.x;   M._14 = 0.0f;
		M._21 = OX.y;   M._22 = OY.y;   M._23 = OZ.y;   M._24 = 0.0f;
		M._31 = OX.z;   M._32 = OY.z;   M._33 = OZ.z;   M._34 = 0.0f;

		M._41 = -MathDotVec(OX, eye);
		M._42 = -MathDotVec(OY, eye);
		M._43 = -MathDotVec(OZ, eye);
		M._44 = 1.0f;
	}

	//>> Составляет правостороннюю матрицу вида (взгляда)
	inline MATH3DMATRIX MathLookAtRHMatrix(const MATH3DVEC & eye, const MATH3DVEC & at, const MATH3DVEC & up)
	{
		MATH3DMATRIX M;

		MATH3DVEC OZ = at - eye;				OZ._normalize_check();
		MATH3DVEC OX = MathCrossVec(up, OZ);	OX._normalize_check();
		MATH3DVEC OY = MathCrossVec(OZ, OX);	OY._normalize_check();

		M._11 = -OX.x;   M._12 = OY.x;   M._13 = -OZ.x;   M._14 = 0.0f;
		M._21 = -OX.y;   M._22 = OY.y;   M._23 = -OZ.y;   M._24 = 0.0f;
		M._31 = -OX.z;   M._32 = OY.z;   M._33 = -OZ.z;   M._34 = 0.0f;

		M._41 = MathDotVec(OX, eye);
		M._42 = -MathDotVec(OY, eye);
		M._43 = MathDotVec(OZ, eye);
		M._44 = 1.0f;

		return M;
	} //*/

	//>> Составляет правостороннюю матрицу вида (взгляда)
	inline void MathLookAtRHMatrix(const MATH3DVEC & eye, const MATH3DVEC & at, const MATH3DVEC & up, MATH3DMATRIX& M)
	{
		MATH3DVEC OZ = at - eye;				OZ._normalize_check();
		MATH3DVEC OX = MathCrossVec(up, OZ);	OX._normalize_check();
		MATH3DVEC OY = MathCrossVec(OZ, OX);	OY._normalize_check();

		M._11 = -OX.x;   M._12 = OY.x;   M._13 = -OZ.x;   M._14 = 0.0f;
		M._21 = -OX.y;   M._22 = OY.y;   M._23 = -OZ.y;   M._24 = 0.0f;
		M._31 = -OX.z;   M._32 = OY.z;   M._33 = -OZ.z;   M._34 = 0.0f;

		M._41 = MathDotVec(OX, eye);
		M._42 = -MathDotVec(OY, eye);
		M._43 = MathDotVec(OZ, eye);
		M._44 = 1.0f;
	}

	//>> Составляет настроенную левостороннюю матрицу ортогональной проекции (left, right, bottom, top, z-near, z-far)
	inline MATH3DMATRIX MathOrthoOffCenterLHMatrix(const float l, const float r, const float b, const float t, const float zn, const float zf)
	{
		MATH3DMATRIX M;
		float rl = 2.0f / (r - l);
		float bt = 2.0f / (b - t);
		float  z = zn - zf;

		M._11 = rl;
		M._22 = -bt;
		M._33 = 1.0f / -z;
		M._41 = -1.0f - l * rl;
		M._42 = 1.0f + t * bt;
		M._43 = zn / z;

		return M;
	} //*/
	;
	//>> Составляет настроенную левостороннюю матрицу ортогональной проекции (left, right, bottom, top, z-near, z-far)
	inline void MathOrthoOffCenterLHMatrix(const float l, const float r, const float b, const float t, const float zn, const float zf,
		MATH3DMATRIX& M)
	{
		M._default();
		float rl = 2.0f / (r - l);
		float bt = 2.0f / (b - t);
		float  z = zn - zf;

		M._11 = rl;
		M._22 = -bt;
		M._33 = 1.0f / -z;
		M._41 = -1.0f - l * rl;
		M._42 = 1.0f + t * bt;
		M._43 = zn / z;
	}

	//>> Составляет настроенную правостороннюю матрицу ортогональной проекции (left, right, bottom, top, z-near, z-far)
	inline MATH3DMATRIX MathOrthoOffCenterRHMatrix(const float l, const float r, const float b, const float t, const float zn, const float zf)
	{
		MATH3DMATRIX M;
		float rl = 2.0f / (r - l);
		float bt = 2.0f / (b - t);
		float  z = zn - zf;

		M._11 = rl;
		M._22 = -bt;
		M._33 = 1.0f / z;
		M._41 = -1.0f - l * rl;
		M._42 = 1.0f + t * bt;
		M._43 = zn / z;

		return M;
	} //*/
	;
	//>> Составляет настроенную правостороннюю матрицу ортогональной проекции (left, right, bottom, top, z-near, z-far)
	inline void MathOrthoOffCenterRHMatrix(const float l, const float r, const float b, const float t, const float zn, const float zf,
		MATH3DMATRIX& M)
	{
		M._default();
		float rl = 2.0f / (r - l);
		float bt = 2.0f / (b - t);
		float  z = zn - zf;

		M._11 = rl;
		M._22 = -bt;
		M._33 = 1.0f / z;
		M._41 = -1.0f - l * rl;
		M._42 = 1.0f + t * bt;
		M._43 = zn / z;
	}

	//>> Составляет левостороннюю матрицу ортогональной проекции
	inline MATH3DMATRIX MathOrthoLHMatrix(const float w, const float h, const float zn, const float zf)
	{
		MATH3DMATRIX M;
		float z = zn - zf;

		M._11 = 2.0f / w;
		M._22 = 2.0f / h;
		M._33 = 1.0f / -z;
		M._43 = zn / z;

		return M;
	} //*/
	;
	//>> Составляет левостороннюю матрицу ортогональной проекции
	inline void MathOrthoLHMatrix(const float w, const float h, const float zn, const float zf, MATH3DMATRIX& M)
	{
		M._default();
		float z = zn - zf;

		M._11 = 2.0f / w;
		M._22 = 2.0f / h;
		M._33 = 1.0f / -z;
		M._43 = zn / z;
	}

	//>> Составляет правостороннюю матрицу ортогональной проекции
	inline MATH3DMATRIX MathOrthoRHMatrix(const float w, const float h, const float zn, const float zf)
	{
		MATH3DMATRIX M;
		float z = zn - zf;

		M._11 = 2.0f / w;
		M._22 = 2.0f / h;
		M._33 = 1.0f / z;
		M._43 = zn / z;

		return M;
	} //*/
	;
	//>> Составляет правостороннюю матрицу ортогональной проекции
	inline void MathOrthoRHMatrix(const float w, const float h, const float zn, const float zf, MATH3DMATRIX& M)
	{
		M._default();
		float z = zn - zf;

		M._11 = 2.0f / w;
		M._22 = 2.0f / h;
		M._33 = 1.0f / z;
		M._43 = zn / z;
	}

	//>> Составляет матрицу отражения относительно плоскости
	inline MATH3DMATRIX MathReflectMatrix(const MATH3DPLANE & plane)
	{
		MATH3DMATRIX M;

		M._11 = 1.0f - 2.0f * plane.a * plane.a;
		M._12 =      - 2.0f * plane.a * plane.b;
		M._13 =      - 2.0f * plane.a * plane.c;
		M._21 =      - 2.0f * plane.a * plane.b;
		M._22 = 1.0f - 2.0f * plane.b * plane.b;
		M._23 =      - 2.0f * plane.b * plane.c;
		M._31 =      - 2.0f * plane.c * plane.a;
		M._32 =      - 2.0f * plane.c * plane.b;
		M._33 = 1.0f - 2.0f * plane.c * plane.c;
		M._41 =      - 2.0f * plane.d * plane.a;
		M._42 =      - 2.0f * plane.d * plane.b;
		M._43 =      - 2.0f * plane.d * plane.c;

		return M;
	} //*/
	;
	//>> Составляет матрицу отражения относительно плоскости
	inline void MathReflectMatrix(const MATH3DPLANE & plane, MATH3DMATRIX& M)
	{
		M._11 = 1.0f - 2.0f * plane.a * plane.a;
		M._12 =      - 2.0f * plane.a * plane.b;
		M._13 =      - 2.0f * plane.a * plane.c;   M._14 = 0;
		M._21 =      - 2.0f * plane.a * plane.b;
		M._22 = 1.0f - 2.0f * plane.b * plane.b;
		M._23 =      - 2.0f * plane.b * plane.c;   M._24 = 0;
		M._31 =      - 2.0f * plane.c * plane.a;
		M._32 =      - 2.0f * plane.c * plane.b;
		M._33 = 1.0f - 2.0f * plane.c * plane.c;   M._34 = 0;
		M._41 =      - 2.0f * plane.d * plane.a;
		M._42 =      - 2.0f * plane.d * plane.b;
		M._43 =      - 2.0f * plane.d * plane.c;   M._44 = 1;
	}

	//>> Нормализация компонент плоскости
/*	inline MATH3DPLANE MathPlaneNormalize(const MATH3DPLANE & plane)
	{
		MATH3DPLANE out;
		float norm = sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);

		if (norm)
		{
			out.a = plane.a / norm;
			out.b = plane.b / norm;
			out.c = plane.c / norm;
			out.d = plane.d / norm;
			out.N.x = out.a;
			out.N.y = out.b;
			out.N.z = out.c;
			out.P.x = plane.P.x;
			out.P.y = plane.P.y;
			out.P.z = plane.P.z;
		}
		else
		{
			out.a = 0.0f;
			out.b = 0.0f;
			out.c = 0.0f;
			out.d = 0.0f;
			out.N.x = 0.0f;
			out.N.y = 0.0f;
			out.N.z = 0.0f;
			out.P.x = plane.P.x;
			out.P.y = plane.P.y;
			out.P.z = plane.P.z;
		}

		return out;
	} //*/
	;
	//>> Проверка местоположения точки относительно плоскости (возращает 0, если лежит в ней)
	inline float MathPlaneDotCoord(const MATH3DPLANE & plane, const MATH3DVEC & coord)
	{
		float ret = (plane.a * coord.x + plane.b * coord.y + plane.c * coord.z + plane.d);

		if (ret < + PRECISION &&
			ret > - PRECISION) ret = 0;

		return ret;
	}

	//>> Проверка местоположения точки относительно плоскости (возращает 0, если лежит в ней)
	inline void MathPlaneDotCoord(const MATH3DPLANE & plane, const MATH3DVEC & coord, float& out)
	{
		out = (plane.a * coord.x + plane.b * coord.y + plane.c * coord.z + plane.d);

		if (out < + PRECISION &&
			out > - PRECISION) out = 0;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	
	//>> Сравнение (V1 == V2) с точностью < precision >
	inline bool MathCompareV3_Equal(const MATH3DVEC & v1, const MATH3DVEC & v2, float precision)
	{
		if (v1.x >= v2.x - precision &&
			v1.x <= v2.x + precision &&
			v1.y >= v2.y - precision &&
			v1.y <= v2.y + precision &&
			v1.z >= v2.z - precision &&
			v1.z <= v2.z + precision) return true;
		return false;
	}

	//>> Сравнение (V1 != V2) с точностью < precision >
	inline bool MathCompareV3_NotEqual(const MATH3DVEC & v1, const MATH3DVEC & v2, float precision)
	{
		if (v1.x >= v2.x - precision &&
			v1.x <= v2.x + precision &&
			v1.y >= v2.y - precision &&
			v1.y <= v2.y + precision &&
			v1.z >= v2.z - precision &&
			v1.z <= v2.z + precision) return false;
		return true;
	}

	//>> Сравнение (V1 > V2) с точностью < precision >
	inline bool MathCompareV3_Greater(const MATH3DVEC & v1, const MATH3DVEC & v2, float precision)
	{
		if (v1.x > v2.x - precision &&
			v1.y > v2.y - precision &&
			v1.z > v2.z - precision) return true;
		return false;
	}

	//>> Сравнение (V1 >= V2) с точностью < precision >
	inline bool MathCompareV3_GreaterEqual(const MATH3DVEC & v1, const MATH3DVEC & v2, float precision)
	{
		if (v1.x >= v2.x - precision &&
			v1.y >= v2.y - precision &&
			v1.z >= v2.z - precision) return true;
		return false;
	}

	//>> Сравнение (V1 < V2) с точностью < precision >
	inline bool MathCompareV3_Less(const MATH3DVEC & v1, const MATH3DVEC & v2, float precision)
	{
		if (v1.x < v2.x + precision &&
			v1.y < v2.y + precision &&
			v1.z < v2.z + precision) return true;
		return false;
	}

	//>> Сравнение (V1 <= V2) с точностью < precision >
	inline bool MathCompareV3_LessEqual(const MATH3DVEC & v1, const MATH3DVEC & v2, float precision)
	{
		if (v1.x <= v2.x + precision &&
			v1.y <= v2.y + precision &&
			v1.z <= v2.z + precision) return true;
		return false;
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	template <typename T>
	//>> Линейная интерполяция между 2 числами
	inline T MathSimpleLERP(const T & previous, const T & current, float t)
	{
		return previous + static_cast<T>(t * (current - previous));
	}

	template <typename T>
	//>> Линейная интерполяция между 2 числами
	inline void MathSimpleLERP(const T & previous, const T & current, float t, T & out)
	{
		T = previous + static_cast<T>(t * (current - previous));
	}

	//>> Линейная интерполяция между 2 точками
	inline MATH3DVEC MathPointLERP(const MATH3DVEC & p1, const MATH3DVEC & p2, float t)
	{
		return p1 + (p2 - p1) * t;
	}

	//>> Линейная интерполяция между 2 точками
	inline void MathPointLERP(const MATH3DVEC & p1, const MATH3DVEC & p2, float t, MATH3DVEC & out)
	{
		out  = p1;
		out += (p2 - p1) * t;
	}



	//>> TODO  Cubic Spline interpolation
	inline void MathPointSQUAD(){}
};

#endif // _MATHS_H