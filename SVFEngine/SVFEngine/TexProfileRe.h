// ----------------------------------------------------------------------- //
//
// MODULE  : TexProfileRe.h
//
// PURPOSE : Отрисовка текстурных элементов
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _TEXPROFILERE_H
#define _TEXPROFILERE_H

#include "TexProfile.h"
#include "FontRe.h"
#include "ModelRe.h"

#define  CLEAR_TEXPROFILE_RAM  true  // удалять загруженное в VideoRAM из RAM ?

namespace SAVFGAME
{
	class CTexProfileR final : public CTexProfile // Содержит текстурные элементы, готовые для рендера
	{
	protected:
		DEV3DBASE *						dev {nullptr};
		VECDATAPP <DataManagerTexture>	tdata;
		VECDATAPP <DataManagerModel>	mdata;
		bool							isInit;
	public:
		CTexProfileR(const CTexProfileR& src)				= delete;
		CTexProfileR(CTexProfileR&& src)					= delete;
		CTexProfileR& operator=(CTexProfileR&& src)			= delete;
		CTexProfileR& operator=(const CTexProfileR& src)	= delete;
	public:
		CTexProfileR() : CTexProfile(), dev(nullptr), isInit(false) {};
		~CTexProfileR() { Close(); }
		void Close()
		{
			if (isInit)
			{
				tdata.Close(1,1);
				mdata.Close(1,1);
				dev = nullptr;
				isInit = false;	
			}
			CTexProfile::Close();
		}

	protected:
		bool PrepareTexture()
		{
			if (dev == nullptr) return false;
			if (texture.count)
			{
				tdata.New(texture.count);
				for (int32 i=0; i<texture.count; i++)
					if (texture[i].file.buf != nullptr && texture[i].file.count > 0)
					{
						auto data = dev->CreateTextureData(texture[i].file.buf, texture[i].file.count);
						tdata.Reset(data, i);
					}
				return true;
			}
			return false;
		}
		bool PrepareModelProc(int32 ID, bool reload)
		{
			if (reload) { mdata[ID]->Close();			// delete
						  mdata.Reset(nullptr, ID); }
			if (mdata[ID] != nullptr) return false;		// done already

			if (!element[ID].centered)
			{
				//      1     2
				//    +-----+
				//    | 0   | 3
				//    +-----+

				uint32 CCW_ibuf[] = { 1, 3, 0,  1, 2, 3 };
				uint32  CW_ibuf[] = { 1, 0, 3,  1, 3, 2 };
				if (CULL_DEFCCW) { auto data = dev->CreateModelData(element[ID].vbuf, CCW_ibuf, 4, 6, sizeof(UIVERTEX));
								   mdata.Reset(data, ID); }
				else             { auto data = dev->CreateModelData(element[ID].vbuf,  CW_ibuf, 4, 6, sizeof(UIVERTEX));
								   mdata.Reset(data, ID); }
			}
			else
			{
				//      8     1     2
				//    +-----+-----+
				//    | 7   | 0   | 3
				//    +-----+-----+
				//    | 6   | 5   | 4
				//    +-----+-----+

				uint32 CCW_ibuf[] = { 1, 3, 0,   1, 2, 3,
									  0, 4, 5,   0, 3, 4,
									  7, 5, 6,   7, 0, 5,
									  8, 0, 7,   8, 1, 0 };

				uint32  CW_ibuf[] = { 1, 0, 3,   1, 3, 2,
									  0, 5, 4,   0, 4, 3,
									  7, 6, 5,   7, 5, 0,
									  8, 7, 0,   8, 0, 1 };

				if (CULL_DEFCCW) { auto data = dev->CreateModelData(element[ID].vbuf, CCW_ibuf, 9, 24, sizeof(UIVERTEX));
								   mdata.Reset(data, ID); }
				else             { auto data = dev->CreateModelData(element[ID].vbuf,  CW_ibuf, 9, 24, sizeof(UIVERTEX));
								   mdata.Reset(data, ID); }
			}
			
			return true;
		}
		void PrepareModel() // для первичного вызова
		{
			if (dev == nullptr) { _MBM(ERROR_PointerNone);  return; }
			if (element.count)
			{
				if (mdata.size()) { _MBM(ERROR_InitAlready);  return; }
				mdata.New(element.count);
				for (int32 i=0; i<element.count; i++)
					PrepareModelProc(i, false);
			}
		}
		void PrepareModel(int32 ID, bool reload)
		{
			if (dev == nullptr)                 { _MBM(ERROR_PointerNone); return; }
			if (!mdata.size())                  { _MBM(ERROR_InitNone);    return; }
			if (ID < 0 || ID >= element.count)  { _MBM(ERROR_IDLIMIT);     return; }
			bool result = PrepareModelProc(ID, reload);
			if (!result) _MBM(ERROR_BUFLOAD);
		}
		void PrepareModel(int32 from_ID, int32 to_ID, bool reload) // [from;to)
		{
			if (dev == nullptr)  { _MBM(ERROR_PointerNone); return; }
			if (!mdata.size())   { _MBM(ERROR_InitNone);    return; }
			if (from_ID > to_ID || from_ID < 0 || to_ID < 0 || from_ID >= element.count || to_ID > element.count)
				{ _MBM(ERROR_IDRANGE); return; }
			for (int32 i=from_ID; i<to_ID; i++)				{
				bool result = PrepareModelProc(i, reload);
				if (!result) _MBM(ERROR_BUFLOAD);			}		
		}

	public:
		//>> Установка девайса
		void SetDevice(const DEV3DBASE * pDevice)
		{
			if (pDevice == nullptr) { _MBM(ERROR_PointerNone); return; }

			dev = const_cast<DEV3DBASE*>(pDevice);
		};

		//>> Первичная инициализация 
		void Prepare(bool clear_mem)
		{
			if (isInit) return;

			if (dev    == nullptr) { _MBM(ERROR_PointerNone);  return; }
			if (shader == nullptr) { _MBM(ERROR_PointerNone);  return; }
			if (IO     == nullptr) { _MBM(ERROR_PointerNone);  return; }

			if (!element.Check() || !texture.Check()) { _MBM(ERROR_InitNone); return; }
			if (!(isInit = PrepareTexture()))         { _MBM(ERROR_InitNone); return; }

			CTexProfile::Update();

			GenerateVerticies(0,0,0,0);
			SetVerticiesColor(COLORVEC(1,1,1,1));
			PrepareModel();

			if (clear_mem) CTexProfile::DeleteTEX(); // удаление данных текстур в RAM
		}

		//>> Первичная инициализация 
		void Prepare()
		{
			Prepare(CLEAR_TEXPROFILE_RAM);
		}

		//>> Меняет центрированную модель полигона (по умолчанию полигоны не центрированы) :: cx, cy [0.f ... 1.f]
		void ResetProfileElement(bool is_centered, float cx, float cy, int32 ID)
		{
			if (!isInit) { _MBM(ERROR_InitNone); return; }

			if (!ReloadElementUV(ID, is_centered, cx, cy)) return;
			GenerateVerticies(0,0,0,0,ID);
			PrepareModel(ID, true);
		}

		//>> Переопределяет параметры элемента и перезагружает его ; указать 0 для default W/H/sx/sy
		void ResetProfileElement(float W, float H, float sx, float sy, const COLORVEC & color, int32 ID)
		{
			if (!isInit) { _MBM(ERROR_InitNone); return; }
			GenerateVerticies(W, H, sx, sy, ID);
			SetVerticiesColor(color, ID);
			PrepareModel(ID, true);
		}

		//>> Переопределяет параметры элементоа и перезагружает их ; указать 0 для default W/H/sx/sy ; в пределах [ from_ID; to_ID )
		void ResetProfileElement(float W, float H, float sx, float sy, const COLORVEC & color, int32 from_ID, int32 to_ID)
		{
			if (!isInit) { _MBM(ERROR_InitNone); return; }
			GenerateVerticies(W, H, sx, sy, from_ID, to_ID);
			SetVerticiesColor(color, from_ID, to_ID);
			PrepareModel(from_ID, to_ID, true);
		}

		//>> Рендер элемента
		bool Show(int32 ID, const WPOS * pos, bool UI)
		{
			if (!isInit) Prepare(CLEAR_TEXPROFILE_RAM); // { _MBM(ERROR_TexProfile);  return false; }

			if (window_W != IO->window.client_width ||
				window_H != IO->window.client_height) CTexProfile::Update();

			eShaderID cur_vs = shader->GetCurVS(); // текущий вершинный шейдер
			eShaderID cur_ps = shader->GetCurPS(); // текущий пиксельный шейдер

			auto TID = element[ID].TexID;

		//	if (!tdata[TID].data->Loaded()) return false;
		//	if (!mdata[ID].data->Loaded()) return false;

			dev->SendToRender(tdata[TID], IDX_TEX_DIFFUSE);

			if (cur_ps == SHADER_NONE && ISDIRECTX9) // default DX9 pipeline
			{
				dev->SetTextureStageState(IDX_TEX_DIFFUSE, TSTATE_FONT);

				dev->SetRenderState(RSTATE_LIGHTING,    _STATE_OFF);
			//	dev->SetRenderState(RSTATE_FOGENABLE,   _STATE_OFF);
				dev->SetRenderState(RSTATE_COLORVERTEX, _STATE_ON);

				shader->SetVertexDecl(VDECLUI);
			}

			dev->SetRenderState(RSTATE_CULLMODE,         _STATE_CULLMODE_NONE);
			dev->SetRenderState(RSTATE_ALPHABLENDENABLE, _STATE_ON);
			dev->SetRenderState(RSTATE_BLENDOP,          _STATE_BLENDOP_ADD);
			dev->SetRenderState(RSTATE_SRCBLEND,         _STATE_BLEND_SRCALPHA);
			dev->SetRenderState(RSTATE_DESTBLEND,        _STATE_BLEND_INVSRCALPHA);

			dev->SendToRender(mdata[ID], 0, sizeof(UIVERTEX));

			SHADER_HANDLE mat_world    = shader->exdata[cur_vs].handle[CONST_MAT_WORLD];
			SHADER_HANDLE mat_viewproj = shader->exdata[cur_vs].handle[CONST_MAT_VIEWPROJ];

			if (!UI) // на 3D сцене
			{
				MathWorldMatrix(pos->P, pos->Q, pos->S, *matWorld);

				if (cur_vs == SHADER_NONE && ISDIRECTX9)
					    dev->SetTransformWorld(matWorld);
				else shader->SetMatrix(cur_vs, mat_world, matWorld);
			}
			else // на экране
			{
				MathWorldMatrix(MATH3DVEC(pos->x, window_H - pos->y, 0), pos->Q, pos->S, *matWorld);

				if (cur_vs == SHADER_NONE && ISDIRECTX9)	// default DX9 pipeline
				{
					dev->SetTransformView(matViewUI);
					dev->SetTransformProjection(matProjUI);
					dev->SetTransformWorld(matWorld);
				}
				else
				{
					shader->SetMatrix(cur_vs, mat_viewproj, matVPUI);
					shader->SetMatrix(cur_vs, mat_world,    matWorld);
				}
			}

			dev->DrawPrimitive(TIP_PRIM_TRIANGLELIST, 0, 0, mdata[ID]->num_vertices, 0, mdata[ID]->num_primitives);

			dev->PreviousRenderState(RSTATE_CULLMODE);
			dev->PreviousRenderState(RSTATE_ALPHABLENDENABLE);
			dev->PreviousRenderState(RSTATE_BLENDOP);
			dev->PreviousRenderState(RSTATE_SRCBLEND);
			dev->PreviousRenderState(RSTATE_DESTBLEND);

			if (cur_ps == SHADER_NONE && ISDIRECTX9)
			{
				dev->PreviousRenderState(RSTATE_LIGHTING);
			//	dev->PreviousRenderState(RSTATE_FOGENABLE);
				dev->PreviousRenderState(RSTATE_COLORVERTEX);

				shader->RestoreVertexDecl();
			}

			return true;
		}
	};
}

#endif // _TEXPROFILERE_H