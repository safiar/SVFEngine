// ----------------------------------------------------------------------- //
//
// MODULE  : Light.h
//
// PURPOSE : Описывает источники света на сцене
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _LIGHT_H
#define _LIGHT_H

#include "stdafx.h"
#include "device3d.h"
#include "Object.h"
#include "Shader.h"

namespace SAVFGAME
{
	enum eLightType
	{
		LIGHT_DIRECT,
		LIGHT_POINT,
		LIGHT_SPOT,

		LIGHT_ENUM_MAX
	};

	struct LightSetting : public CObject
	{
	public:
		LightSetting(const LightSetting& src)				= delete;
		LightSetting(LightSetting&& src)					= delete;
		LightSetting& operator=(LightSetting&& src)			= delete;
		LightSetting& operator=(const LightSetting& src)	= delete;	
	public:
		//>> Default constructor
		LightSetting() : CObject(0,0,0,1,0,0), type(LIGHT_DIRECT), diffuse(1,1,1), specular(1,1,1), pos(CObject::pos),
			range(100), att0(0), att1(0.125f), att2(0), phi(40.0f), theta(20.0f), falloff(1.0f), dynamic(false), enable(true) {};
		//>> Default constructor
		LightSetting(eLightType eLT) : CObject(0,0,0,1,0,0), type(eLT), diffuse(1,1,1), specular(1,1,1), pos(CObject::pos),
			range(100), att0(0), att1(0.125f), att2(0), phi(40.0f), theta(20.0f), falloff(1.0f), dynamic(false), enable(true) {};
		//>> Direct light constructor
		LightSetting(eLightType eLT, COLORVEC3 diff, COLORVEC3 spec, MATH3DVEC3 dir, bool isDynamic) :						
			CObject(0,0,0,dir.x,dir.y,dir.z), type(eLT), diffuse(diff), specular(spec), range(100), pos(CObject::pos),
			att0(0), att1(0.125f), att2(0), phi(40.0f), theta(20.0f), falloff(1.0f), dynamic(isDynamic), enable(true) {};
		//>> Point light constructor
		LightSetting(eLightType eLT, COLORVEC3 diff, COLORVEC3 spec, MATH3DVEC3 pos,
			float _range, float _att0, float _att1, float _att2, bool isDynamic) :												
			CObject(pos.x,pos.y,pos.z,1,0,0), type(eLT), diffuse(diff), specular(spec), range(_range), pos(CObject::pos),
			att0(_att0), att1(_att1), att2(_att2), phi(40.0f), theta(20.0f), falloff(1.0f), dynamic(isDynamic), enable(true) {};
		//>> Spot light constructor
		LightSetting(eLightType eLT, COLORVEC3 diff, COLORVEC3 spec, MATH3DVEC3 dir, MATH3DVEC3 pos,
			float _range, float _att0, float _att1, float _att2, float _phi, float _theta, float _falloff, bool isDynamic) :	
			CObject(pos,dir), type(eLT), diffuse(diff), specular(spec), range(_range), pos(CObject::pos),
			att0(_att0), att1(_att1), att2(_att2), phi(_phi), theta(_theta), falloff(_falloff), dynamic(isDynamic), enable(true) {};
		//////////
		eLightType type;
		COLORVEC3 diffuse;          // direct & spot & point
		COLORVEC3 specular;         // direct & spot & point
	//	MATH3DVEC3 direction;		// direct & spot
	//	MATH3DVEC3 position;		//          spot & point
		float range;				//          spot & point		attenuation = (dist >= range) ? 0 : 1 / (att0 + att1 * dist + att2 * dist^2)
		float att0;					//          spot & point
		float att1;					//          spot & point
		float att2;					//          spot & point
		float phi;					//          spot				outer cone angle
		float theta;				//          spot				inner cone angle
		float falloff;				//          spot
		bool  dynamic;
		bool  enable;
	public:
		WPOS * const	pos;		// direction & position
	};

	class CBaseLight
	{
	protected:
		CShader *	shader { nullptr };
		float		parallax_scale;
		bool		isInit;
	public:
		COLORVEC3				 ambient;	// минимальное освещение
		VECDATAPP <LightSetting> source;	// источники света
	protected:
		SHADER_HANDLE	var_light_count;
		SHADER_HANDLE	var_light_ambient;
		SHADER_HANDLE	var_light_vlht;
		SHADER_HANDLE	var_light_flht;
		SHADER_HANDLE	var_prllx_scale;
	public:
		CBaseLight(const CBaseLight& src)				= delete;
		CBaseLight(CBaseLight&& src)					= delete;
		CBaseLight& operator=(CBaseLight&& src)			= delete;
		CBaseLight& operator=(const CBaseLight& src)	= delete;
	public:
		CBaseLight() { Close(); };
		~CBaseLight() { Close(); };
		void Close()
		{
			shader = nullptr;
			ambient = COLORVEC3(0.3f, 0.3f, 0.3f);
			parallax_scale = -0.020f;
			source.Delete(1);
			isInit = false;
		}
		
		void  IncreaseParallaxScale()      { parallax_scale += 0.001f; };
		void  DecreaseParallaxScale()      { parallax_scale -= 0.001f; };
		void  SetParallaxScale(float prlx) { parallax_scale = prlx;    };
		float GetParallaxScale()           { return parallax_scale;    };

	public:
		//>> Установка шейдера
		void SetShader(const CShader * pShader)
		{
			if (pShader == nullptr) { _MBM(ERROR_PointerNone); return; }

			shader = const_cast<CShader*>(pShader);
		};
		//>> Установка минимальной освещённости сцены
		void SetAmbientColor(const COLORVEC3 & color)
		{
			ambient = color;
		};

	protected:
		//>> ..test..
		void Init(eShaderID ID)
		{
			if (isInit) return;

			if (shader == nullptr) { _MBM(ERROR_PointerNone); return; }

			var_light_count   = shader->exdata[ID].handle[VAR_LIGHT_COUNT];
			var_light_ambient = shader->exdata[ID].handle[VAR_LIGHT_AMBIENT];
			var_light_flht    = shader->exdata[ID].handle[VAR_LIGHT_FLHT];
			var_light_vlht    = shader->exdata[ID].handle[VAR_LIGHT_VLHT];
			var_prllx_scale   = shader->exdata[ID].handle[VAR_LIGHT_PRLLX_SCALE];

			isInit = true;
		}
		//>> ..test..
		void UpdateParallaxScale(eShaderID ID, float scale)
		{
			if (S_OK != shader->SetFloat(ID, var_prllx_scale, scale)) _MBM(L"Check UpdateParallaxScale() func");
		}
		//>> ..test..
		void UpdateLightShaderData(eShaderID ID)
		{
			if (!isInit) Init(ID);

			uint32 count = 0;
			uint32 src_count = (uint32)source.size();

			for (uint32 src_i = 0; src_i<src_count; src_i++)
				if (source[src_i] != nullptr)
					if (source[src_i]->enable)
						count++;

			struct VLHTDATA
			{
				MATH3DVEC4 diffuse;
				MATH3DVEC4 specular;
				MATH3DVEC4 dir;
				MATH3DVEC4 pos;
			};
			struct FLHTDATA
			{
				float type;
				float range;
				float att0;
				float att1;
				float att2;
				float cos_phi;
				float cos_theta;
				float falloff;
			};
			struct LDATA
			{
				LDATA(){};
				LDATA(size_t num) { Create(num); }
				void Create(size_t num)
				{
					flht.Create(num);
					vlht.Create(num);
				}
				TBUFFER <VLHTDATA, size_t>  vlht;
				TBUFFER <FLHTDATA, size_t>  flht;
				///////////////
				MATH3DVEC4  ambient;
			};

			LDATA l(count);		
			l.ambient = { ambient.r, ambient.g, ambient.b, 1.0f };

			for (uint32 i=0, src_i=0; src_i<src_count; src_i++)
				if (source[src_i] != nullptr)
					if (source[src_i]->enable)
			{
				auto & light = *source[src_i];
				const WPOS * wpos = light.pos;

				l.flht[i].type = static_cast<float>(light.type);
				l.vlht[i].diffuse._set(MATH3DVEC4(light.diffuse.r, light.diffuse.g, light.diffuse.b, 1.0f));
				l.vlht[i].specular._set(MATH3DVEC4(light.specular.r, light.specular.g, light.specular.b, 1.0f));

				switch (light.type)
				{
				case LIGHT_DIRECT:
					l.vlht[i].dir._set(MATH3DVEC4(wpos->ax, wpos->ay, wpos->az, 0.0f));
					break;
				case LIGHT_POINT:
					l.vlht[i].pos._set(MATH3DVEC4(wpos->x, wpos->y, wpos->z, 0.0f));
					l.flht[i].range = light.range;
					l.flht[i].att0  = light.att0;
					l.flht[i].att1  = light.att1;
					l.flht[i].att2  = light.att2;
					break;
				case LIGHT_SPOT:
					l.vlht[i].dir._set(MATH3DVEC4(wpos->ax, wpos->ay, wpos->az, 0.0f));
					l.vlht[i].pos._set(MATH3DVEC4(wpos->x, wpos->y, wpos->z, 0.0f));
					l.flht[i].range = light.range;
					l.flht[i].att0  = light.att0;
					l.flht[i].att1  = light.att1;
					l.flht[i].att2  = light.att2;
					l.flht[i].cos_phi   = CTAB::cosA(light.phi / 2);
					l.flht[i].cos_theta = CTAB::cosA(light.theta / 2);
					l.flht[i].falloff   = light.falloff;
					break;
				}
				i++;
			}

			const uint32 vlhtsz = sizeof(VLHTDATA) / sizeof(float);
			const uint32 flhtsz = sizeof(FLHTDATA) / sizeof(float);

			if (S_OK != shader->SetInt       (ID, var_light_count,                     count         )) _MBM(L"Check var_light_count");
			if (S_OK != shader->SetFloatArray(ID, var_light_ambient, l.ambient,		   4             )) _MBM(L"Check var_light_ambient");
			if (S_OK != shader->SetFloatArray(ID, var_light_vlht,    (float*)l.vlht(), vlhtsz * count)) _MBM(L"Check var_light_vlht");
			if (S_OK != shader->SetFloatArray(ID, var_light_flht,    (float*)l.flht(), flhtsz * count)) _MBM(L"Check var_light_flht");
		}

	public:
		//>> Включение light/model шейдера
		void EnableLightShader()
		{
			if (shader == nullptr) { _MBM(ERROR_PointerNone); return; }

		/*	//shader->State(SHADER_VERTEX_LIGHT,true);
			//shader->State(SHADER_PIXEL_LIGHT,true);

			static bool once = 1;
			if (once) {
			UpdateLightShaderData(SHADER_VERTEX_LIGHT); //once = 0; }
			for(int i=0; i<100; i++)
				UpdateLightShaderData(SHADER_VERTEX_LIGHT);

			//shader->State(SHADER_VERTEX_LIGHT,false);
			//shader->State(SHADER_PIXEL_LIGHT,false);

			//------------ //*/

			shader->State(SHADERLIGHTV, true);
			shader->State(SHADERLIGHTP, true);

			UpdateLightShaderData(SHADERLIGHTP);
			UpdateParallaxScale(SHADERLIGHTP, parallax_scale);
		};
		//>> Выключение light/model шейдера
		void DisableLightShader()
		{
			if (shader == nullptr) { _MBM(ERROR_PointerNone); return; }

			shader->State(SHADERLIGHTV, false);
			shader->State(SHADERLIGHTP, false);
		};
	};
}

#endif // _LIGHT_H