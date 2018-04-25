// ----------------------------------------------------------------------- //
//
// MODULE  : Shader.h
//
// PURPOSE : Общая входная информация о написанных шейдерах
//           1. Задать eShaderID своего шейдера
//           2. Задать eShaderVariableMemories своего шейдера (если надо)
//           3. Заполнить SHADERSINFO, используя eShaderID
//           4. Создать в верхнем .h модуле (DX9..12,OpenGL..) процедуру для своего шейдера
//           5. Задать направление на неё через точку входа ::State();
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _SHADER_H
#define _SHADER_H

#include "stdafx.h"
#include "link_defines.h"
#include "Camera.h"

#if   defined (DIRECTX_9)

#ifndef D3DXHANDLE
	#ifndef D3DXFX_LARGEADDRESS_HANDLE
		typedef LPCSTR D3DXHANDLE;
	#else
		typedef UINT_PTR D3DXHANDLE;
	#endif
	typedef D3DXHANDLE *LPD3DXHANDLE;
#endif

	#define SHADER_HANDLE	D3DXHANDLE
	#define SHADER_DESC		D3DXCONSTANT_DESC
	#define SHADERUIV		SHADER_VERTEX_UI		// Текущий используемый UI вершинный шейдер
	#define SHADERUIP		SHADER_PIXEL_UI			// Текущий используемый UI пиксельный шейдер
	#define SHADERSKYV		SHADER_VERTEX_SKYBOX	// Текущий используемый Skybox вершинный шейдер
	#define SHADERSKYP		SHADER_PIXEL_SKYBOX		// Текущий используемый Skybox пиксельный шейдер
	#define SHADERLIGHTV	SHADER_VERTEX_N_LIGHT	// Текущий используемый Light вершинный шейдер
	#define SHADERLIGHTP	SHADER_PIXEL_N_LIGHT	// Текущий используемый Light пиксельный шейдер
	#define VDECLMODEL		VDECL_MODELLIGHT		// Текущая декларация вершин для моделей
	#define VDECLUI			VDECL_UIFONT			// Текущая декларация вершин для текстов и UI
#elif defined (DIRECTX_10) ;
#elif defined (DIRECTX_11) ;
#elif defined (OPENGL)     ;
#endif

//#ifndef HRESULT
//	typedef long HRESULT;
//#endif

//#ifndef S_OK
//	#define S_OK ((HRESULT)0L)
//#endif

//#ifndef S_FALSE
//	#define S_FALSE ((HRESULT)1L)
//#endif

//#ifndef D3D_OK
//	#define D3D_OK S_OK
//#endif

#define RESNUM 32 + 4  // var_max + const

namespace SAVFGAME
{
	enum eVertexDecl
	{
		VDECL_MODELLIGHT,	// Структура вершин моделей (Light, Skybox шейдеры)
		VDECL_UIFONT,		// Структура вершин графики (UI шейдеры)

		VDECL_ENUM_MAX
	};

	struct SHADERINTERNAL
	{
		SHADERINTERNAL() : have_handles(false) {};
		/////////
		SHADER_HANDLE handle [RESNUM];
		bool          have_handles;
	};
	struct SHADEREXTERNAL
	{	
	//	UINT          count  [RESNUM];
	//	SHADER_DESC   desc   [RESNUM];	// description          { напр. можно узнать номер N текстуры для  device->SetTexture(N,texture); }
		SHADER_HANDLE handle [RESNUM];	// дубликат от INTERNAL { напр. для установки техники эффекта      render->SetEffectTechnique();  }
	};

	enum eShaderType // ID типа шейдера
	{
		TYPE_VERTEX,
		TYPE_PIXEL,
		TYPE_EFFECT30,	// DX9 : vs_3_0 , ps_3_0
		TYPE_GEOMETRY,	// DX10 (обрабатывает геометрические примитивы)
		TYPE_HULL,		// DX11 (тесселяция)
		TYPE_DOMAIN,	// DX11 (калькуляция внутри патча)

		TYPE_ENUM_MAX
	};
	enum eShaderID // ID каждого написанного шейдера
	{
		SHADER_NONE,

		SHADER_DX9_START,

		SHADER_VERTEX_SKYBOX,	// DX9 : Вершинный шейдер для CSkybox
		SHADER_PIXEL_SKYBOX,	// DX9 : Пиксельный шейдер для CSkybox

		SHADER_VERTEX_N_LIGHT,	// DX9 : Шейдер света с Normal mapping и проч.
		SHADER_PIXEL_N_LIGHT,	// DX9 : Шейдер света с Normal mapping и проч.

		SHADER_VERTEX_UI,		// DX9 : Шейдер для вывода текста и прочего UI
		SHADER_PIXEL_UI,		// DX9 : Шейдер для вывода текста и прочего UI

		// ...

		SHADER_DX9_END,

		SHADER_DX10_START,
		SHADER_DX10_END,

		SHADER_DX11_START,
		SHADER_DX11_END,

		SHADER_DX12_START,
		SHADER_DX12_END,

		SHADER_OPENGL_START,
		SHADER_OPENGL_END,	

		// ...

		SHADER_OLD_GARBAGE_START,

	//	SHADER_EFFECT30_SKYBOX,	// DX9 : (УСТАРЕЛО)
	//	SHADER_VERTEX_LIGHT,	// DX9 : Шейдер света (УСТАРЕЛО)
	//	SHADER_PIXEL_LIGHT,		// DX9 : Шейдер света (УСТАРЕЛО)

		SHADER_OLD_GARBAGE_END,

		// ...

		SHADER_ENUM_MAX
	};	
	enum eShaderVariableMemories // ID своих переменных для внешнего доступа через SHADEREXTERNAL[SHADER_ID].<dataname>[THIS_ID]
	{
		VAR_ANYSHADER_ANYVAR   = 0, // example

		VAR_LIGHT_COUNT        = 0, // DX9 : SHADER_PIXEL_N_LIGHT
		VAR_LIGHT_AMBIENT      = 1, // .
		VAR_LIGHT_FLHT         = 2, // . light floats  : type, range, att0-1-2, cos_phi, cos_theta, falloff	
		VAR_LIGHT_VLHT         = 3, // . light vectors : diffuse, specular, direction, position
		VAR_LIGHT_PRLLX_SCALE  = 4, // .

		VAR_MAX                = RESNUM - 1 - 4,

		CONST_MAT_VIEWPROJ     = RESNUM - 1 - 3, // для передачи матрицы view * projection (vs)
		CONST_MAT_WORLD        = RESNUM - 1 - 2, // для передачи матрицы world позиции (vs)
		CONST_VEC_CAMERA       = RESNUM - 1 - 1, // для передачи позиции камеры (ps)
		CONST_MTL_MODEL        = RESNUM - 1		 // для передачи свойств материала (ps)
	};
	
	struct SHADERSINFO // Данные для заполнения
	{
		SHADERSINFO()
		{
			name.resize(SHADER_ENUM_MAX);
			func.resize(SHADER_ENUM_MAX);
			type.resize(SHADER_ENUM_MAX);

			name[SHADER_VERTEX_SKYBOX] = L"directx9/skybox_dx9_vshader.fx";
			func[SHADER_VERTEX_SKYBOX] = "MainFunc";
			type[SHADER_VERTEX_SKYBOX] = TYPE_VERTEX;

			name[SHADER_PIXEL_SKYBOX]  = L"directx9/skybox_dx9_pshader.fx";
			func[SHADER_PIXEL_SKYBOX]  = "MainFunc";
			type[SHADER_PIXEL_SKYBOX]  = TYPE_PIXEL;

	/*		name[SHADER_EFFECT30_SKYBOX] = L"directx9/skybox_dx9_3.0.fx"; // не актуален, не использовать
	//		func[SHADER_EFFECT30_SKYBOX] = "MainFunc";
	//		type[SHADER_EFFECT30_SKYBOX] = TYPE_EFFECT30;

	//		name[SHADER_VERTEX_LIGHT] = L"directx9/light_dx9_vshader.fx"; // не актуален, не использовать
	//		func[SHADER_VERTEX_LIGHT] = "MainFunc";
	//		type[SHADER_VERTEX_LIGHT] = TYPE_VERTEX;

	//		name[SHADER_PIXEL_LIGHT] = L"directx9/light_dx9_pshader.fx"; // не актуален, не использовать
	//		func[SHADER_PIXEL_LIGHT] = "MainFunc";
	//		type[SHADER_PIXEL_LIGHT] = TYPE_PIXEL; //*/

			name[SHADER_VERTEX_N_LIGHT] = L"directx9/light_N_dx9_vshader.fx";
			func[SHADER_VERTEX_N_LIGHT] = "MainFunc";
			type[SHADER_VERTEX_N_LIGHT] = TYPE_VERTEX;

			name[SHADER_PIXEL_N_LIGHT] = L"directx9/light_N_dx9_pshader.fx";
			func[SHADER_PIXEL_N_LIGHT] = "MainFunc";
			type[SHADER_PIXEL_N_LIGHT] = TYPE_PIXEL;

			name[SHADER_VERTEX_UI] = L"directx9/UI_dx9_vshader.fx";
			func[SHADER_VERTEX_UI] = "MainFunc";
			type[SHADER_VERTEX_UI] = TYPE_VERTEX;

			name[SHADER_PIXEL_UI] = L"directx9/UI_dx9_pshader.fx";
			func[SHADER_PIXEL_UI] = "MainFunc";
			type[SHADER_PIXEL_UI] = TYPE_PIXEL;
		}
		vector<std::wstring> name;	// имя файла
		vector<std::string>  func;	// имя main-функции
		vector<eShaderType>  type;	// тип шейдера
	};

	class CShader // shader manager : virtual base class
	{
		friend class CBaseCamera;
	protected:		
		char*					vs;			// Версия вершинного шейдера   напр.: "vs_3_0"
		char*					ps;			// Версия пиксельного шейдера  напр.: "ps_3_0"
		SHADERSINFO				info;		// 
		eShaderID				cur_vs;		// текущий
		eShaderID				cur_ps;		// текущий
		eShaderID				cur_eff;	// текущий
		vector<bool>			isInit;		// Метка, что шейдер скомпилирован
		vector<bool>			isEnabled;  // Метка, что шейдер включен
		vector<SHADERINTERNAL>	indata;		// Информация внутреннего доступа
	public:
		vector<SHADEREXTERNAL>	exdata;		// Информация внешнего доступа
	protected:
		const MATH3DVEC	 *		posCamera		{ nullptr };	// Данные от камеры
		const MATH3DMATRIX *	matView			{ nullptr };	// Данные от камеры
		const MATH3DMATRIX *	matProjection	{ nullptr };	// Данные от камеры
		const MATH3DMATRIX *	matViewProj		{ nullptr };	// Данные от камеры
		bool					refs_prepared   { false };
	public:
		CShader(const CShader& src)				= delete;
		CShader(CShader&& src)					= delete;
		CShader& operator=(CShader&& src)		= delete;
		CShader& operator=(const CShader& src)	= delete;
	public:
		CShader() : isInit(SHADER_ENUM_MAX), isEnabled(SHADER_ENUM_MAX), indata(SHADER_ENUM_MAX), exdata(SHADER_ENUM_MAX),
			cur_vs(SHADER_NONE), cur_ps(SHADER_NONE), cur_eff(SHADER_NONE), vs(nullptr), ps(nullptr)
		{};
		~CShader() { Close(); };

		virtual void Load(wchar_t* gamePath) = 0;			// Прототип загрузки шейдеров
		virtual void State(eShaderID ID, bool enable) = 0;	// Прототип включения/выключения шейдеров	
		virtual void SetVertexDecl(eVertexDecl type) = 0;	// Прототип установки структуры вершин
		virtual void RestoreVertexDecl() = 0;				// Прототип возврата предыдущей структуры вершин

		bool isShaderInit(eShaderID ID)   { return isInit[ID];    }
		bool isShaderEnable(eShaderID ID) { return isEnabled[ID]; }
		eShaderID GetCurVS()  { return cur_vs;  }
		eShaderID GetCurPS()  { return cur_ps;  }
		eShaderID GetCurEFF() { return cur_eff; }

		void SetReferences(const CBaseCamera * pCamera)
		{
			if (pCamera == nullptr) { _MBM(ERROR_PointerNone); return; }

			CBaseCamera * cam = const_cast<CBaseCamera*>(pCamera);

			posCamera     = cam->GetCameraPos();
			matView       = cam->GetViewMatrix();
			matProjection = cam->GetProjMatrix();
			matViewProj   = cam->GetViewProjMatrix();

			refs_prepared = true;
		}

		virtual void Close()
		{
			for(uint32 i=0; i<SHADER_ENUM_MAX; i++)
			if (isInit[i])
			{
				isEnabled[i] = false;
				isInit[i]    = false;
			}
			indata.clear();
			exdata.clear();
			
			posCamera     = nullptr;
			matView       = nullptr;
			matProjection = nullptr;
			matViewProj   = nullptr;
			refs_prepared = false;
		}

		///////////////////////////////////////////////////////////////

	public:
		virtual HRESULT OnLostDevice() = 0;	 // вызов перед d3ddev->Reset()
		virtual HRESULT OnResetDevice() = 0; // вызов после d3ddev->Reset()
	//	virtual HRESULT SetEffectTexture(eShaderID ID, SHADER_HANDLE handle, IDirect3DBaseTexture9 * texture) = 0;
	//	virtual HRESULT SetEffectTechnique(eShaderID ID, SHADER_HANDLE handle) = 0;
	//	virtual HRESULT EffectBegin(eShaderID ID, uint32 * pPasses, DWORD flags) = 0;
	//	virtual HRESULT EffectBeginPass(eShaderID ID, uint32 pass) = 0;
	//	virtual HRESULT EffectEndPass(eShaderID ID) = 0;
	//	virtual HRESULT EffectEnd(eShaderID ID) = 0;
		virtual HRESULT SetMatrix(eShaderID ID, SHADER_HANDLE handle, MATH3DMATRIX * pMatrix) = 0;
		virtual HRESULT SetMatrixTranspose(eShaderID ID, SHADER_HANDLE handle, MATH3DMATRIX * pMatrix) = 0;
		virtual HRESULT SetRaw(eShaderID ID, SHADER_HANDLE handle, void * pData, uint32 bytes) = 0;
		virtual HRESULT SetBool(eShaderID ID, SHADER_HANDLE handle, int32 b) = 0;
		virtual HRESULT SetInt(eShaderID ID, SHADER_HANDLE handle, int32 n) = 0;
		virtual HRESULT SetFloat(eShaderID ID, SHADER_HANDLE handle, float f) = 0;
		virtual HRESULT SetVector(eShaderID ID, SHADER_HANDLE handle, MATH3DVEC4 * pVector) = 0;
		virtual HRESULT SetBoolArray(eShaderID ID, SHADER_HANDLE handle, int32 *pb, uint32 Count) = 0;
		virtual HRESULT SetIntArray(eShaderID ID, SHADER_HANDLE handle, int32 *pn, uint32 Count) = 0;
		virtual HRESULT SetFloatArray(eShaderID ID, SHADER_HANDLE handle, float *pf, uint32 Count) = 0;
		virtual HRESULT SetVectorArray(eShaderID ID, SHADER_HANDLE handle, MATH3DVEC4 * pVector, uint32 Count) = 0;
	};
}

#endif // _SHADER_H