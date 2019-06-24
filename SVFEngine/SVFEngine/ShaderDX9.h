// ----------------------------------------------------------------------- //
//
// MODULE  : ShaderDX9.h
//
// PURPOSE : Управление HLSL-шейдерами (DX9)
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _SHADERDX9_H
#define _SHADERDX9_H

#include "device3d9.h"
#include "Shader.h"

#ifdef _DEBUG
	#define SCMPLSET (D3DXSHADER_DEBUG)
#else
	#define SCMPLSET (NULL)
//	#define SCMPLSET (D3DXSHADER_SKIPVALIDATION)
#endif

using namespace SAVFGAME;

namespace SAVFGAME
{
	// if (d3dcaps.PixelShaderVersion  < D3DPS_VERSION(3, 0)) {}
	// if (d3dcaps.VertexShaderVersion < D3DPS_VERSION(3, 0)) {}

	struct ModelVerticesDeclDX9 // Описывает типовой вертекс модели
	{
		ModelVerticesDeclDX9()
		{
			memset(decl,0,20*sizeof(D3DVERTEXELEMENT9));
			D3DVERTEXELEMENT9 model_vertex_decl[] =
			{ 
				{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, // FLOAT x, y, z
						D3DDECLUSAGE_POSITION, 0},
				{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, // FLOAT nx, ny, nz  нормаль [Z]
						D3DDECLUSAGE_NORMAL,   0},
				{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, // FLOAT u, v		diffuse 1 / normal 1 / light 1 map
						D3DDECLUSAGE_TEXCOORD, 0},
				{0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, // FLOAT u2, v2		diffuse 2
						D3DDECLUSAGE_TEXCOORD, 1},
				{0, 40, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, // FLOAT u3, v3		diffuse 3
						D3DDECLUSAGE_TEXCOORD, 2},
				{0, 48, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, // FLOAT tx, ty, tz	тангенсальный вектор [X]
						D3DDECLUSAGE_TANGENT,  0},
				{0, 60, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, // FLOAT bx, by, bz	двойная нормаль [Y]
						D3DDECLUSAGE_BINORMAL, 0},
				D3DDECL_END()
			};
			int num = sizeof(model_vertex_decl) / sizeof(D3DVERTEXELEMENT9);
			for(int i=0; i<num; i++)
				decl[i] = model_vertex_decl[i];
		};
		D3DVERTEXELEMENT9 decl[20];
	};
	struct FontVerticesDeclDX9 // Описывает типовой вертекс шрифтов и UI
	{
		FontVerticesDeclDX9()
		{
			memset(decl,0,20*sizeof(D3DVERTEXELEMENT9));
			D3DVERTEXELEMENT9 model_vertex_decl[] =
			{ 
				{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, // FLOAT x, y, z
						D3DDECLUSAGE_POSITION, 0},
				{0, 12, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, // FLOAT r, g, b, a	color
						D3DDECLUSAGE_COLOR,    0},	
				{0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, // FLOAT u, v		font map
						D3DDECLUSAGE_TEXCOORD, 0},			
				D3DDECL_END()
			};
			int num = sizeof(model_vertex_decl) / sizeof(D3DVERTEXELEMENT9);
			for(int i=0; i<num; i++)
				decl[i] = model_vertex_decl[i];
		};
		D3DVERTEXELEMENT9 decl[20];
	};

	class CShaderDX9 final : public CShader
	{
	protected:
		DEV3DDAT *							  dev		{ nullptr };
		IDeviceT *							  d3ddev	{ nullptr }; // (copy) quick ptr == dev->d3ddev
		vector<ID3DXConstantTable*>			  table;
		vector<IDirect3DVertexShader9*>		  vshader;
		vector<IDirect3DPixelShader9*>		  pshader;
		vector<ID3DXEffect*>				  eshader;
		IDirect3DVertexDeclaration9 *		  vdecl_model;	// типовой вертекс модели
		IDirect3DVertexDeclaration9 *		  vdecl_font;	// типовой вертекс текстов и UI
		IDirect3DVertexDeclaration9 *		  vdecl_mem;
		IDirect3DVertexDeclaration9 ** const  vdecl_mem_ptr;
	public:
		CShaderDX9(const CShaderDX9& src)				= delete;
		CShaderDX9(CShaderDX9&& src)					= delete;
		CShaderDX9& operator=(CShaderDX9&& src)			= delete;
		CShaderDX9& operator=(const CShaderDX9& src)	= delete;
	public:
		CShaderDX9() : CShader(), table(SHADER_ENUM_MAX), vshader(SHADER_ENUM_MAX), pshader(SHADER_ENUM_MAX), eshader(SHADER_ENUM_MAX),
			vdecl_mem_ptr(&vdecl_mem)
		{
			vdecl_model = nullptr;
			vdecl_font = nullptr;
			vdecl_mem = nullptr;
			for (auto & vs : vshader) vs = nullptr;
			for (auto & ps : pshader) ps = nullptr;
			for (auto & es : eshader) es = nullptr;
		};
		~CShaderDX9() override final { Close(); };

		void Close() override final
		{
			for(uint32 i=0; i<SHADER_ENUM_MAX; i++)
			if (isInit[i])
			{
				_RELEASE(table[i]);
				_RELEASE(vshader[i]);
				_RELEASE(pshader[i]);
				_RELEASE(eshader[i]);
			}
			dev = nullptr;
			vs  = nullptr;
			ps  = nullptr;
			_RELEASE(vdecl_model);
			_RELEASE(vdecl_font);
			vdecl_mem = nullptr;
			CShader::Close();
		}

		//>> Установка девайса
		bool SetDevice(DEV3DBASE * ptr)
		{
			if (dev != nullptr && ptr != nullptr)
				{ _MBM(ERROR_PointerAlready);  return false; }
			dev = reinterpret_cast<DEV3DDAT*>(ptr);
			d3ddev = dev->d3ddev; // set up quick ptr
			vs = (char*)D3DXGetVertexShaderProfile(d3ddev);
			ps = (char*)D3DXGetPixelShaderProfile(d3ddev);
			vs_u32 = dev->d3dcaps->VertexShaderVersion & 0xFFFF;
			ps_u32 = dev->d3dcaps->PixelShaderVersion  & 0xFFFF;
			return true;
		};

	protected:
		//>> Объявление структур вершин
		void InitVerticiesDeclarations()
		{
			if (d3ddev == nullptr) { _MBM(ERROR_InitDevice); return; }

			ModelVerticesDeclDX9 VDModel;
			FontVerticesDeclDX9  VDFont;

			if (D3D_OK != d3ddev->CreateVertexDeclaration(VDModel.decl, &vdecl_model))
				_MBM(ERROR_VertexDecl);

			if (D3D_OK != d3ddev->CreateVertexDeclaration(VDFont.decl, &vdecl_font))
				_MBM(ERROR_VertexDecl);

			vdecl_mem = vdecl_model;
		}

	public:
		//>> Установка структуры вершин
		void SetVertexDecl(eVertexDecl type) override final
		{
			if (vdecl_model == nullptr) InitVerticiesDeclarations();

			d3ddev->GetVertexDeclaration(vdecl_mem_ptr);

			switch (type)
			{
			case VDECL_MODELLIGHT: d3ddev->SetVertexDeclaration(vdecl_model);  break;
			case VDECL_UIFONT:     d3ddev->SetVertexDeclaration(vdecl_font);   break;
			}
		}

		//>> Возврат предыдущей структуры вершин
		void RestoreVertexDecl() override final
		{
			d3ddev->SetVertexDeclaration(vdecl_mem);
		}

	protected:
		//>> Загрузка и компиляция шейдеров
		void LoadProc(uint32 ID, wchar_t* path)
		{
			ID3DXBuffer * errors = nullptr;
			ID3DXBuffer * shader = nullptr;
			HRESULT result;
			const char * type;
			bool compile_effect = false;

			char compiler_msg[256];
			_bstr_t filename(info.name[ID].c_str());
			sprintf_s(compiler_msg,"%s %s",ERROR_ShaderCompiler,(char*)filename);

			switch(info.type[ID])
			{
			case TYPE_VERTEX: type = vs; break;
			case TYPE_PIXEL:  type = ps; break;
			default: type = "none"; compile_effect = true;
			}
			
			if (!compile_effect)
			{
				if (D3D_OK != D3DXCompileShaderFromFile(path,0,0,info.func[ID].c_str(),type,SCMPLSET,&shader,&errors,&table[ID])) 
				{	            _MB (NULL, ERROR_ShaderCompile, ERROR_Warning,  ERROR_MB); 
					if (errors) _MBS(NULL, errors,              compiler_msg,   ERROR_MB);  return;
				}   if (errors) _MBS(NULL, errors,              compiler_msg,   ERROR_MB);

				switch(info.type[ID])
				{
				case TYPE_VERTEX:
					result = d3ddev->CreateVertexShader((DWORD*)shader->GetBufferPointer(), &vshader[ID]);
					pshader[ID] = nullptr; break;
				case TYPE_PIXEL:
					result = d3ddev->CreatePixelShader((DWORD*)shader->GetBufferPointer(), &pshader[ID]);
					vshader[ID] = nullptr; break;
				}   eshader[ID] = nullptr;

				_RELEASE(shader);

				if (result != D3D_OK) { _MBM(ERROR_ShaderCompile_2); return; }
			}
			else
			{
				switch (info.type[ID])
				{
				case TYPE_EFFECT50:
					if (vs_u32 < 0x0500 && ps_u32 < 0x0500)
					{
						wprintf(L"\nEffect 5.0 [%s] not supported (vs=%04x,ps=%04x)",
							info.name[ID].c_str(), vs_u32, ps_u32);
						return;
					}
					break;
				}

				if (D3D_OK != D3DXCreateEffectFromFile(d3ddev,path,0,0,SCMPLSET,0,&eshader[ID],&errors))
				{	            _MB (NULL, ERROR_ShaderCompile_F, ERROR_Warning,  ERROR_MB); 
					if (errors) _MBS(NULL, errors,                compiler_msg,   ERROR_MB);  return;
				}   if (errors) _MBS(NULL, errors,                compiler_msg,   ERROR_MB);
				
				pshader[ID] = nullptr;
				vshader[ID] = nullptr;

				info.tech.cur[ID] = MISSING;

				byte EffectTechMAX = info.tech.max[ID];
				for (byte i = AnyEffectTechMIN; i < EffectTechMAX; i++)
				{
					info.tech.p[ID][i] = eshader[ID]->GetTechniqueByName( info.tech.n[ID][i].c_str() );

					if (D3D_OK == eshader[ID]->ValidateTechnique(info.tech.p[ID][i]))
						 info.tech.valid[ID][i] = true;
					else info.tech.valid[ID][i] = false;

					// Первая доступная техника будет установлена текущей

					if (info.tech.valid[ID][i])
						info.tech.cur[ID] = i;

					//eshader[ID]->FindNextValidTechnique
				}
			}

			isInit[ID] = TRUE;
		};

	public:
		//>> Загрузчик ../<gamefolder>/data/shaders/
		void Load(wchar_t* gamePath) override final
		{
			//>> TODO: проверка версии у эффектов

			if (dev == nullptr)         { _MBM(ERROR_PointerNone); return; }
			if (refs_prepared == false) { _MBM(ERROR_InitNone);    return; }

			wchar_t syspath[MAX_PATH], error[MAX_PATH * 2], p1[MAX_PATH], p2[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, syspath);

			for(uint32 i=SHADER_DX9_START+1; i<SHADER_DX9_END; i++) if (!isInit[i])
			{
				wchar_t *p = nullptr;

				wsprintf(error,L"%s %s",ERROR_OpenFile,info.name[i].c_str());
				wsprintf(p1,L"%s\\%s\\%s",gamePath,DIRECTORY_SHADERS,info.name[i].c_str());
				wsprintf(p2,L"%s\\%s\\%s",syspath,DIRECTORY_SHADERS,info.name[i].c_str());
				
				//if (p = LoadFileCheck(2, p1, p2)) LoadShadersProc(info, i, p);
				if (p = LoadFileCheck64((int64)p1, (int64)p2)) LoadProc(i, p);
				else _MBM(error);
			}
		}

	protected:
		//>> Процедура включения/выключения
		bool StateProc(eShaderID ID, bool enable)
		{
			if (isEnabled[ID] == enable) return false;

			if (enable) /////// ON ///////
			{
				switch (info.type[ID])
				{
				case TYPE_VERTEX: d3ddev->SetVertexShader(vshader[ID]); cur_vs = ID; break;
				case TYPE_PIXEL:  d3ddev->SetPixelShader(pshader[ID]);  cur_ps = ID; break;
				default: cur_eff = ID;
				}

				isEnabled[ID] = enable;
			}
			else       /////// OFF ///////
			{
				isEnabled[ID] = enable;

				switch (info.type[ID])
				{
				case TYPE_VERTEX: d3ddev->SetVertexShader(0); cur_vs = SHADER_NONE; break;
				case TYPE_PIXEL:  d3ddev->SetPixelShader(0);  cur_ps = SHADER_NONE; break;
				default: cur_eff = SHADER_NONE;
				}
			}

			return true;
		}

		//>> Проверка, что один из шейдеров того же типа уже не был включен
		bool StateCheckCollision(eShaderID nameID)
		{
			switch (info.type[nameID])
			{
			case TYPE_VERTEX:
			case TYPE_PIXEL:
				for(uint32 i=SHADER_DX9_START+1; i<SHADER_DX9_END; i++)
				if (i!=nameID && isEnabled[i])
				if (info.type[i] == info.type[nameID])
				{
					if (info.type[i] == TYPE_VERTEX)
						 _MBM(ERROR_ShaderCollision)
					else _MBM(ERROR_ShaderCollision2)
					return false;
				}
				break;
			default:
				for(uint32 i=SHADER_DX9_START+1; i<SHADER_DX9_END; i++)
				if (i!=nameID && isEnabled[i])
				if (info.type[i] != TYPE_VERTEX &&
					info.type[i] != TYPE_PIXEL)
				{
					_MBM(ERROR_ShaderCollision3);
					return false;
				}
			}
			return true;
		}

	public:
		//>> Общая точка входа для выбора совместной с шейдером процедуры
		void State(eShaderID nameID, bool enable) override final
		{
			if (!isInit[nameID]) { _MBM(ERROR_ShaderNotExists); return; }

			if (!StateCheckCollision(nameID)) return;	// Выход - шейдер такого типа уже включен
			if (!StateProc(nameID, enable)) return;		// Выход - уже включено/выключено
			
			switch(nameID)
			{
			case SHADER_EFFECT30_SKYBOX: SkyboxProc(nameID); break;
			case SHADER_VERTEX_SKYBOX:   SkyboxProc(nameID); break;
			case SHADER_PIXEL_SKYBOX:    SkyboxProc(nameID); break;
		//	case SHADER_VERTEX_LIGHT:    LightProc(nameID); break;
		//	case SHADER_PIXEL_LIGHT:     LightProc(nameID); break;
			case SHADER_VERTEX_N_LIGHT:  LightProc(nameID); break;
			case SHADER_PIXEL_N_LIGHT:   LightProc(nameID); break;
			case SHADER_VERTEX_UI:		 UIProc(nameID); break;
			case SHADER_PIXEL_UI:		 UIProc(nameID); break;			
			default: ;
			}
		}

	protected:
		//>> Процедура для DX9 Skybox-шейдеров
		void SkyboxProc(eShaderID ID)
		{
			const wchar_t SKYERR0[] = L"Matrix <World> error [Skybox]";
			const wchar_t SKYERR1[] = L"Matrix <ViewProj> error [Skybox]";
			const wchar_t SKYERR2[] = L"FloatArray <CameraPosition> error [Skybox]";
		//	const wchar_t SKYERR3[] = L"Technique <SkyTech> isn't valid [Skybox]";
		//	const wchar_t SKYERR4[] = L"Technique <SkyTech> error [Skybox]";

			switch(ID)
			{	////////////////////////////////////////////////////////////////////////////////////////////////
				case SHADER_EFFECT30_SKYBOX:
					if (isEnabled[ID])			// ON skybox vertex shader
					{
						const UINT V0 = CONST_MAT_WORLD;
						const UINT V1 = CONST_MAT_VIEWPROJ;
						const UINT V2 = CONST_VEC_CAMERA;
						const UINT VL = V2;					// VAR last

						SetVertexDecl(VDECL_MODELLIGHT);

						if (!indata[ID].have_handles) // только для первого вызова
						{
							indata[ID].handle[V0] = eshader[ID]->GetParameterByName(NULL, "World");
							indata[ID].handle[V1] = eshader[ID]->GetParameterByName(NULL, "ViewProj");
							indata[ID].handle[V2] = eshader[ID]->GetParameterByName(NULL, "CameraPosition");
							//indata[ID].handle[V0] = table[ID]->GetConstantByName(NULL, "World");
							//indata[ID].handle[V1] = table[ID]->GetConstantByName(NULL, "ViewProj");
							//indata[ID].handle[V2] = table[ID]->GetConstantByName(0, "CameraPosition");
							indata[ID].have_handles = true;
						}

						//MATH3DMATRIX matWorld;
						//MathTranslateMatrix(*posCamera, matWorld);
						MATH_TRANSLATE_MATRIX(posCamera->x, posCamera->y, posCamera->z)

						if (D3D_OK != eshader[ID]->SetFloatArray(indata[ID].handle[V0], MT,                 16))  _MBM(SKYERR0);
						if (D3D_OK != eshader[ID]->SetFloatArray(indata[ID].handle[V1], FCAST(matViewProj), 16))  _MBM(SKYERR1);
						if (D3D_OK != eshader[ID]->SetFloatArray(indata[ID].handle[V2], FCAST(posCamera),    3))  _MBM(SKYERR2);
					}
					else
					{
						RestoreVertexDecl();
					}
					break;
				////////////////////////////////////////////////////////////////////////////////////////////////
				case SHADER_VERTEX_SKYBOX:
					if (isEnabled[ID])			// ON skybox vertex shader
					{
						const UINT V0 = CONST_MAT_WORLD;
						const UINT V1 = CONST_MAT_VIEWPROJ;
						const UINT V2 = CONST_VEC_CAMERA;
						const UINT VL = V2;					// VAR last

						SetVertexDecl(VDECL_MODELLIGHT);

						if (!indata[ID].have_handles) // только для первого вызова
						{
							indata[ID].handle[V0] = table[ID]->GetConstantByName(NULL, "World");
							indata[ID].handle[V1] = table[ID]->GetConstantByName(NULL, "ViewProj");
							indata[ID].handle[V2] = table[ID]->GetConstantByName(0, "CameraPosition");
							indata[ID].have_handles = true;
						}

						//MATH3DMATRIX matWorld;
						//MathTranslateMatrix(*posCamera, matWorld);
						MATH_TRANSLATE_MATRIX(posCamera->x, posCamera->y, posCamera->z)

						if (D3D_OK != table[ID]->SetFloatArray(d3ddev, indata[ID].handle[V0], MT,                 16))  _MBM(SKYERR0);
						if (D3D_OK != table[ID]->SetFloatArray(d3ddev, indata[ID].handle[V1], FCAST(matViewProj), 16))  _MBM(SKYERR1);
						if (D3D_OK != table[ID]->SetFloatArray(d3ddev, indata[ID].handle[V2], FCAST(posCamera),    3))  _MBM(SKYERR2);
					}
					else						// OFF skybox vertex shader
					{
						RestoreVertexDecl();
					}
					break;
				////////////////////////////////////////////////////////////////////////////////////////////////
				case SHADER_PIXEL_SKYBOX:
					if (isEnabled[ID])			// ON skybox pixel shader
					{
						if (!indata[ID].have_handles) // только для первого вызова
						{
						//	indata[ID].handle[VAR_SKYBOX_TEX_ID] = table[ID]->GetConstantByName(0,"SkyBoxTexture");						
						//	table[ID]->GetConstantDesc                (	// для установки текстуры извне
						//		indata[ID].handle[VAR_SKYBOX_TEX_ID],
						//		&exdata[ID].desc[VAR_SKYBOX_TEX_ID],
						//		&exdata[ID].count[VAR_SKYBOX_TEX_ID]  );
							indata[ID].have_handles = true;
						}
					// device->SetTexture (
					//			shader->exdata[SHADER_PIXEL_SKYBOX].desc[VAR_SKYBOX_TEX_ID].RegisterIndex,
					//			<texture> );
					}
					else						// OFF skybox pixel shader
					{
						//memset(&exdata[ID].desc[0],  0, sizeof(D3DXCONSTANT_DESC));
						//memset(&exdata[ID].count[0], 0, sizeof(UINT));
					}
					break;
				////////////////////////////////////////////////////////////////////////////////////////////////
			/*	case SHADER_EFFECT30_SKYBOX:
					if (isEnabled[ID])			// ON skybox effect
					{
						const UINT V0 = VAR_SKYBOX_EFF_TECH_ID; // enum VAR 0
						const UINT V1 = VAR_SKYBOX_EFF_TEX_ID;  // enum VAR 1
						const UINT VL = V1;					    // enum VAR last

						SetVertexDecl(VDECL_MODELLIGHT);

						if (!indata[ID].have_handles) // только для первого вызова
						{
							indata[ID].handle[V0]   = eshader[ID]->GetTechniqueByName("SkyTech");
							indata[ID].handle[V1]   = eshader[ID]->GetParameterByName(0,"SkyBoxTexture");
							indata[ID].handle[VL+1] = eshader[ID]->GetParameterByName(0,"World");
							indata[ID].handle[VL+2] = eshader[ID]->GetParameterByName(0,"ViewProj");
							indata[ID].handle[VL+3] = eshader[ID]->GetParameterByName(0,"CameraPosition");
							
							indata[ID].have_handles = TRUE;

							exdata[ID].handle[VAR_SKYBOX_EFF_TEX_ID] = indata[ID].handle[VAR_SKYBOX_EFF_TEX_ID]; // для установки текстуры извне

							if (D3D_OK != eshader[ID]->ValidateTechnique(indata[ID].handle[V0])) _MBM(SKYERR3);
							if (D3D_OK != eshader[ID]->SetTechnique(indata[ID].handle[V0]))      _MBM(SKYERR4);
						}

					//  Текстура задаётся извне
					//	shader->SetEffectTexture(SHADER_EFFECT30_SKYBOX, shader->exdata[SHADER_EFFECT30_SKYBOX].handle[VAR_SKYBOX_EFF_TEX_ID], <texture>);

						//MATH3DMATRIX matWorld = MathTranslateMatrix(vecCamera.x, vecCamera.y, vecCamera.z);
						//MATH3DMATRIX matViewProj = matView * matProjection;
						MATH3DMATRIX matWorld(move(MathTranslateMatrix(*posCamera)));
						

						if (D3D_OK != eshader[ID]->SetFloatArray(indata[ID].handle[VL+1], matWorld, 16))	 _MBM(SKYERR0);
						if (D3D_OK != eshader[ID]->SetFloatArray(indata[ID].handle[VL+2], *matViewProj, 16)) _MBM(SKYERR1);
						if (D3D_OK != eshader[ID]->SetFloatArray(indata[ID].handle[VL+3], *posCamera, 3))	 _MBM(SKYERR2);

						d3ddev->GetRenderState(D3DRS_CULLMODE, &estate.CULLMODE);
						d3ddev->GetRenderState(D3DRS_LIGHTING, &estate.LIGHTING);
						d3ddev->GetRenderState(D3DRS_ZENABLE,  &estate.ZENABLE);
						d3ddev->GetRenderState(D3DRS_ZWRITEENABLE, &estate.ZWRITEENABLE);

						d3ddev->SetRenderState(D3DRS_CULLMODE, CULL_NON); //CULL_REV);
						d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);
						d3ddev->SetRenderState(D3DRS_ZENABLE,  D3DZB_FALSE);
						d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
					}
					else						// OFF skybox effect
					{
						RestoreVertexDecl();

						d3ddev->SetRenderState(D3DRS_CULLMODE, estate.CULLMODE);
						d3ddev->SetRenderState(D3DRS_LIGHTING, estate.LIGHTING);
						d3ddev->SetRenderState(D3DRS_ZENABLE,  estate.ZENABLE);
						d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, estate.ZWRITEENABLE);
					}
					break; //*/
			}
		}
	
		//>> Процедура для DX9 Light-шейдеров
		void LightProc(eShaderID ID)
		{
			const wchar_t LIGHTERR1[] = L"Matrix <ViewProj> error [Light]";
			const wchar_t LIGHTERR2[] = L"FloatArray <CameraPosition> error [Light]";

			switch(ID)
			{	////////////////////////////////////////////////////////////////////////////////////////////////
		/*		case SHADER_VERTEX_LIGHT:
					if (isEnabled[ID])			// ON direct light vertex shader
					{
				//		const UINT V0  = VAR_LIGHT_MTL_AMBIENT;	// enum VAR 0
				//		const UINT V1  = VAR_LIGHT_MTL_DIFFUSE;	// enum VAR 1
				//		const UINT V2  = VAR_LIGHT_MTL_SPECULAR;	// enum VAR 2
				//		const UINT V3  = VAR_LIGHT_MTL_POWER;	// enum VAR 3
						const UINT V4  = CONST_MAT_WORLD; //VAR_LIGHT_MAT_WORLD;
						const UINT V5  = VAR_LIGHT_COUNT;
				//		const UINT V6  = VAR_LIGHT_TYPE;
						const UINT V7  = VAR_LIGHT_AMBIENT;
				//		const UINT V8  = VAR_LIGHT_DIFFUSE;
				//		const UINT V9  = VAR_LIGHT_SPECULAR;
				//		const UINT V10 = VAR_LIGHT_DIRECTION;
				//		const UINT V11 = VAR_LIGHT_POSITION;
				//		const UINT V12 = VAR_LIGHT_RANGE;
				//		const UINT V13 = VAR_LIGHT_ATT0;
				//		const UINT V14 = VAR_LIGHT_ATT1;
				//		const UINT V15 = VAR_LIGHT_ATT2;
				//		const UINT V16 = VAR_LIGHT_COS_PHI;
				//		const UINT V17 = VAR_LIGHT_COS_THETA;
				//		const UINT V18 = VAR_LIGHT_FALLOFF;
				//		const UINT V19 = VAR_LIGHT_ON;
				//		const UINT V20 = VAR_LIGHT_AFFECTED;
						const UINT VL  = V7;					// enum VAR last

						SetVertexDecl(VDECL_MODELLIGHT);

						if (!indata[ID].have_handles) // только для первого вызова
						{
					//		indata[ID].handle[V0] = table[ID]->GetConstantByName(NULL,"MTL_ambient");
					//		indata[ID].handle[V1] = table[ID]->GetConstantByName(NULL,"MTL_diffuse");
					//		indata[ID].handle[V2] = table[ID]->GetConstantByName(NULL,"MTL_specular");
					//		indata[ID].handle[V3] = table[ID]->GetConstantByName(NULL,"MTL_power");
							indata[ID].handle[V4] = table[ID]->GetConstantByName(NULL,"World");
							indata[ID].handle[V5] = table[ID]->GetConstantByName(NULL,"Light_count");
					//		indata[ID].handle[V6] = table[ID]->GetConstantByName(NULL,"Light_type");
							indata[ID].handle[V7] = table[ID]->GetConstantByName(NULL,"Light_ambient");
					//		indata[ID].handle[V8] = table[ID]->GetConstantByName(NULL,"Light_diffuse");
					//		indata[ID].handle[V9] = table[ID]->GetConstantByName(NULL,"Light_specular");
					//		indata[ID].handle[V10] = table[ID]->GetConstantByName(NULL,"Light_direction");
					//		indata[ID].handle[V11] = table[ID]->GetConstantByName(NULL,"Light_position");
					//		indata[ID].handle[V12] = table[ID]->GetConstantByName(NULL,"Light_range");
					//		indata[ID].handle[V13] = table[ID]->GetConstantByName(NULL,"Light_att0");
					//		indata[ID].handle[V14] = table[ID]->GetConstantByName(NULL,"Light_att1");
					//		indata[ID].handle[V15] = table[ID]->GetConstantByName(NULL,"Light_att2");
					//		indata[ID].handle[V16] = table[ID]->GetConstantByName(NULL,"Light_cos_phi");
					//		indata[ID].handle[V17] = table[ID]->GetConstantByName(NULL,"Light_cos_theta");
					//		indata[ID].handle[V18] = table[ID]->GetConstantByName(NULL,"Light_falloff");
					//		indata[ID].handle[V19] = table[ID]->GetConstantByName(NULL,"Light_on");
					//		indata[ID].handle[V20] = table[ID]->GetConstantByName(NULL,"Light_affected");						
							indata[ID].handle[VL+1] = table[ID]->GetConstantByName(NULL,"ViewProj");
							indata[ID].handle[VL+2] = table[ID]->GetConstantByName(NULL,"CameraPosition");

							indata[ID].have_handles = true;

					//		exdata[ID].handle[V0] = indata[ID].handle[V0]; // для установки извне
					//		exdata[ID].handle[V1] = indata[ID].handle[V1]; // для установки извне
					//		exdata[ID].handle[V2] = indata[ID].handle[V2]; // для установки извне
					//		exdata[ID].handle[V3] = indata[ID].handle[V3]; // для установки извне
							exdata[ID].handle[V4] = indata[ID].handle[V4];
							exdata[ID].handle[V5] = indata[ID].handle[V5];
					//		exdata[ID].handle[V6] = indata[ID].handle[V6];
							exdata[ID].handle[V7] = indata[ID].handle[V7];
					//		exdata[ID].handle[V8] = indata[ID].handle[V8];
					//		exdata[ID].handle[V9] = indata[ID].handle[V9];
					//		exdata[ID].handle[V10] = indata[ID].handle[V10];
					//		exdata[ID].handle[V11] = indata[ID].handle[V11];
					//		exdata[ID].handle[V12] = indata[ID].handle[V12];
					//		exdata[ID].handle[V13] = indata[ID].handle[V13];
					//		exdata[ID].handle[V14] = indata[ID].handle[V14];
					//		exdata[ID].handle[V15] = indata[ID].handle[V15];
					//		exdata[ID].handle[V16] = indata[ID].handle[V16];
					//		exdata[ID].handle[V17] = indata[ID].handle[V17];
					//		exdata[ID].handle[V18] = indata[ID].handle[V18];
					//		exdata[ID].handle[V19] = indata[ID].handle[V19];
					//		exdata[ID].handle[V20] = indata[ID].handle[V20];
						}

						//MATH3DMATRIX matViewProj = matView * matProjection;

						if (D3D_OK != table[ID]->SetFloatArray(d3ddev, indata[ID].handle[VL+1], *matViewProj, 16)) _MBM(LIGHTERR1);
						if (D3D_OK != table[ID]->SetFloatArray(d3ddev, indata[ID].handle[VL+2], *posCamera,   3))  _MBM(LIGHTERR2);

						//d3ddev->GetRenderState(D3DRS_LIGHTING, &state.LIGHTING);
						//d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);
					}
					else						// OFF direct light vertex shader
					{
						RestoreVertexDecl();

						//d3ddev->SetRenderState(D3DRS_LIGHTING, state.LIGHTING);
					}
					break; //*/
				////////////////////////////////////////////////////////////////////////////////////////////////
		/*		case SHADER_PIXEL_LIGHT:
					if (isEnabled[ID])			// ON light pixel shader
					{
						const UINT V1 = VAR_LIGHT_DIFF_TEX;
						if (!indata[ID].have_handles) // только для первого вызова
						{
							indata[ID].handle[V1] = table[ID]->GetConstantByName(NULL,"Material_diffuse_texture");
							indata[ID].have_handles = TRUE;
							exdata[ID].handle[V1] = indata[ID].handle[V1];
						}
					}
					else						// OFF light pixel shader
					{
						// ...
					}
					break; //*/
				////////////////////////////////////////////////////////////////////////////////////////////////
				case SHADER_VERTEX_N_LIGHT:
					if (isEnabled[ID])			// ON light vertex shader with normal mapping
					{
						const UINT V0  = CONST_MAT_WORLD;
						const UINT V1  = CONST_MAT_VIEWPROJ;
						const UINT VL  = V1;

						SetVertexDecl(VDECL_MODELLIGHT);

						if (!indata[ID].have_handles) // только для первого вызова
						{
							indata[ID].handle[V0] = table[ID]->GetConstantByName(NULL,"World");
							indata[ID].handle[V1] = table[ID]->GetConstantByName(NULL,"ViewProj");

							indata[ID].have_handles = true;

							exdata[ID].handle[V0]  = indata[ID].handle[V0]; // для установки извне
							exdata[ID].handle[V1]  = indata[ID].handle[V1];
						}

						if (D3D_OK != table[ID]->SetFloatArray(d3ddev, indata[ID].handle[V1], FCAST(matViewProj), 16)) _MBM(LIGHTERR1);
					}
					else						// OFF light vertex shader with normal mapping
					{
						RestoreVertexDecl();
					}
					break;
				////////////////////////////////////////////////////////////////////////////////////////////////
				case SHADER_PIXEL_N_LIGHT:
					if (isEnabled[ID])			// ON light pixel shader with normal mapping
					{
						const UINT V0  = VAR_LIGHT_COUNT;		// num of lights
						const UINT V1  = VAR_LIGHT_AMBIENT;		// ambient light color
						const UINT V2  = VAR_LIGHT_FLHT;		// lights' float vars
						const UINT V3  = VAR_LIGHT_VLHT;		// lights' vector vars
						const UINT V4  = VAR_LIGHT_PRLLX_SCALE;	// parallax scale
						const UINT V34 = CONST_VEC_CAMERA;		// camera position
						const UINT V35 = CONST_MTL_MODEL;		// material desc
						const UINT VL  = V35;					// VAR last

						if (!indata[ID].have_handles) // только для первого вызова
						{
							indata[ID].handle[V0]  = table[ID]->GetConstantByName(NULL,"Light_count");
							indata[ID].handle[V1]  = table[ID]->GetConstantByName(NULL,"Light_ambient");
							indata[ID].handle[V2]  = table[ID]->GetConstantByName(NULL,"FLHT");
							indata[ID].handle[V3]  = table[ID]->GetConstantByName(NULL,"VLHT");
							indata[ID].handle[V4]  = table[ID]->GetConstantByName(NULL,"Parallax_scale");
							indata[ID].handle[V34] = table[ID]->GetConstantByName(NULL,"CameraPosition");
							indata[ID].handle[V35] = table[ID]->GetConstantByName(NULL,"MTL");
							
							indata[ID].have_handles = true;

							exdata[ID].handle[V0]  = indata[ID].handle[V0];
							exdata[ID].handle[V1]  = indata[ID].handle[V1];
							exdata[ID].handle[V2]  = indata[ID].handle[V2];
							exdata[ID].handle[V3]  = indata[ID].handle[V3];
							exdata[ID].handle[V4]  = indata[ID].handle[V4];
							exdata[ID].handle[V34] = indata[ID].handle[V34];
							exdata[ID].handle[V35] = indata[ID].handle[V35];
							
						}

						if (D3D_OK != table[ID]->SetFloatArray(d3ddev, indata[ID].handle[V34], FCAST(posCamera), 3)) {}; //_MBM(LIGHTERR2);
					}
					else						// OFF light pixel shader with normal mapping
					{
						// ...
					}
					break;
			}
		}

		//>> Процедура для DX9 UI-шейдеров
		void UIProc(eShaderID ID)
		{
			const wchar_t UIERR1[] = L"Matrix <ViewProj> error [UI]";

			switch(ID)
			{	////////////////////////////////////////////////////////////////////////////////////////////////
				case SHADER_VERTEX_UI:
					if (isEnabled[ID])			// ON UI vertex shader
					{
						const UINT V0  = CONST_MAT_WORLD;
						const UINT V1  = CONST_MAT_VIEWPROJ;
						const UINT VL  = V1;					// VAR last

						SetVertexDecl(VDECL_UIFONT);

						if (!indata[ID].have_handles) // только для первого вызова
						{
							indata[ID].handle[V0] = table[ID]->GetConstantByName(NULL,"World");
							indata[ID].handle[V1] = table[ID]->GetConstantByName(NULL,"ViewProj");

							indata[ID].have_handles = TRUE;

							exdata[ID].handle[V0] = indata[ID].handle[V0]; // для установки извне
							exdata[ID].handle[V1] = indata[ID].handle[V1]; // для установки извне
						}

						if (D3D_OK != table[ID]->SetFloatArray(d3ddev, indata[ID].handle[V1], FCAST(matViewProj), 16)) _MBM(UIERR1);
					}
					else						// OFF UI vertex shader
					{
						RestoreVertexDecl();
					}
					break;
				////////////////////////////////////////////////////////////////////////////////////////////////
				case SHADER_PIXEL_UI:
					if (isEnabled[ID])			// ON UI pixel shader
					{
						// ...
					}
					else						// OFF UI pixel shader
					{
						// ...
					}
					break;
				////////////////////////////////////////////////////////////////////////////////////////////////
				case 0:
					if (isEnabled[ID])			// ON 
					{
						// ...
					}
					else						// OFF
					{
						// ...
					}
					break;
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////

	public: // в .cpp части
		HRESULT OnLostDevice()  override final;	// вызов перед d3ddev->Reset()
		HRESULT OnResetDevice() override final; // вызов после d3ddev->Reset()
		HRESULT SetEffectTexture(eShaderID ID, SHADER_HANDLE handle, void * p_texture)                 override final;
		HRESULT SetEffectTechnique(eShaderID ID, eShaderEffectTechID techID)                           override final;
		HRESULT EffectBegin(eShaderID ID, uint32 * pPasses, uint32 flags)                              override final;
		HRESULT EffectBeginPass(eShaderID ID, uint32 pass)                                             override final;
		HRESULT EffectEndPass(eShaderID ID)                                                            override final;
		HRESULT EffectEnd(eShaderID ID)                                                                override final;
		HRESULT SetMatrix(eShaderID ID, SHADER_HANDLE handle, MATH3DMATRIX * pMatrix)                  override final;
		HRESULT SetMatrixTranspose(eShaderID ID, SHADER_HANDLE handle, MATH3DMATRIX * pMatrix)         override final;
		HRESULT SetRaw(eShaderID ID, SHADER_HANDLE handle, void * pData, uint32 bytes)                 override final;
		HRESULT SetBool(eShaderID ID, SHADER_HANDLE handle, int32 b)                                   override final;
		HRESULT SetInt(eShaderID ID, SHADER_HANDLE handle, int32 n)                                    override final;
		HRESULT SetFloat(eShaderID ID, SHADER_HANDLE handle, float f)                                  override final;
		HRESULT SetVector(eShaderID ID, SHADER_HANDLE handle, MATH3DVEC4 * pVector)                    override final;
		HRESULT SetBoolArray(eShaderID ID, SHADER_HANDLE handle, int32 *pb, uint32 Count)              override final;
		HRESULT SetIntArray(eShaderID ID, SHADER_HANDLE handle, int32 *pn, uint32 Count)               override final;
		HRESULT SetFloatArray(eShaderID ID, SHADER_HANDLE handle, float *pf, uint32 Count)             override final;
		HRESULT SetVectorArray(eShaderID ID, SHADER_HANDLE handle, MATH3DVEC4 * pVector, uint32 Count) override final;
	};
}

#endif // _SHADERDX9_H