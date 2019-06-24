// ----------------------------------------------------------------------- //
//
// MODULE  : ModelRe.h
//
// PURPOSE : Отрисовка моделей
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _MODELRE_H
#define _MODELRE_H

#include "ModelPre.h"
#include "device3d.h"

using namespace SAVFGAME;

#define  CLEAR_MODEL_RAM  true	// удалять загруженное в VideoRAM из RAM ?

namespace SAVFGAME
{
	class CModelR final : public CModelPre
	{
	protected:
		CShader *						shader { nullptr };
		DEV3DBASE *						dev    { nullptr };
		shared_ptr <DataManagerModel>	mdata_shr;
		DataManagerModel *				mdata;
		VECDATAPP <DataManagerTexture>  tdata;
		bool							isInit;
	public:
		CModelR(const CModelR& src)				= delete;
		CModelR(CModelR&& src)					= delete;
		CModelR& operator=(CModelR&& src)		= delete;
		CModelR& operator=(const CModelR& src)	= delete;
	public:
		CModelR() :              CModelPre(),         isInit(false) {};
		CModelR(bool cull_CCW) : CModelPre(cull_CCW), isInit(false) {};
		~CModelR() { Close(); };

		void Close()
		{
			if (isInit)
			{
				tdata.Close(1,1);
				_CLOSESHARED(mdata_shr);
				mdata  = nullptr;
				shader = nullptr;
				dev    = nullptr;
				isInit = false;
			}
			CModelPre::Close();
		}

		//>> Установка шейдера
		void SetShader(const CShader * pShader)
		{
			if (pShader == nullptr) { _MBM(ERROR_PointerNone);  return; }

			shader = const_cast<CShader*>(pShader);
		};
		//>> Установка девайса
		void SetDevice(const DEV3DBASE * pDevice)
		{
			if (pDevice == nullptr) { _MBM(ERROR_PointerNone); return; }

			dev = const_cast<DEV3DBASE*>(pDevice);
		};

	public:
		//>> Загрузка данных в видеопамять
		bool PrepareModel(bool clear_mem)
		{
			if (isInit) return true;

			if (dev == nullptr)    { _MBM(ERROR_PointerNone);  return false; }
			if (shader == nullptr) { _MBM(ERROR_PointerNone);  return false; }
			if (!CModel::isInit)   { _MBM(ERROR_InitNone);     return false; }

			///////////// Загрузка текстур /////////////

			uint32 count = (uint32)texture.size();
			tdata.New(count);
			for(uint32 i=0; i<count; i++)
			{
				if (texture[i]->file.buf != nullptr && texture[i]->file.count > 0)
				{
					auto data = dev->CreateTextureData(texture[i]->file.buf, texture[i]->file.count);
					tdata.Reset(data, i);
				}
				//wprintf(L"\n%3i: buf %8X size %8X %s",
				//	i, texture[i].data->file.buf, texture[i].data->file.count, texture[i].data->name.c_str());
			}

			///////////// Загрузка модели /////////////

			mdata_shr = dev->CreateModelData(vertex.buf, indice.buf, vertex.count, indice.count, sizeof(MODELVERTEX));
			mdata = mdata_shr.get();

			/////////////

			if (clear_mem)
			{
				CModel::DeleteTEX(); // удаление данных текстур в RAM
				CModel::DeleteVI();  // удаление данных вершин и индексов в RAM
			}

			CModelPre::Init();

			return isInit = true;
		}
		//>> Загрузка модели в видеопамять
		bool PrepareModel()
		{
			return PrepareModel(CLEAR_MODEL_RAM);
		}

	protected:
		//>> Рендер модели на сцене (в порядке используемого материала)
		void ShowModelProc(bool draw_translucent, uint32 idSMC)
		{
			if (draw_translucent && mtlsort_tr == -1) return;	// нет прозрачных

			PrepareMatrices(idSMC);

			dev->SendToRender(mdata, 0, sizeof(MODELVERTEX));

			eShaderID cur_vs = shader->GetCurVS(); // текущий вершинный шейдер
			eShaderID cur_ps = shader->GetCurPS(); // текущий пиксельный шейдер

			 int32 prevMatGroup = -1;
			uint32 start = 0;
			uint32 end = static_cast<uint32>(mtlsort.size());

			if (draw_translucent)									{	// прозрачные
				start = static_cast<uint32>(mtlsort_tr);
				if (mtlsort_none != -1)
					end = static_cast<uint32>(mtlsort_none);		}
			else													{	// непрозрачные
				if (mtlsort_none != -1)
					end = static_cast<uint32>(mtlsort_none);
				if (mtlsort_tr != -1)
					end = static_cast<uint32>(mtlsort_tr);			}

			SHADER_HANDLE mat_world = shader->exdata[cur_vs].handle[CONST_MAT_WORLD];

			if (cur_vs == SHADER_NONE && ISDIRECTX9) // default DX9 pipeline
			{
				if (matExternalWorld != nullptr)	dev->SetTransformWorld(matExternalWorld);
				else if (_NOMISS(idSMC))			dev->SetTransformWorld(matStaticWorld[idSMC]);
				else								dev->SetTransformWorld(matDynamicWorld);

				shader->SetVertexDecl(VDECLMODEL);
			}
			else
			{
				if (matExternalWorld != nullptr)	shader->SetMatrix(cur_vs, mat_world, matExternalWorld);
				else if (_NOMISS(idSMC))			shader->SetMatrix(cur_vs, mat_world, matStaticWorld[idSMC]);
				else								shader->SetMatrix(cur_vs, mat_world, matDynamicWorld);
			}
	
			if (draw_translucent)
			{
				dev->SetRenderState(RSTATE_CULLMODE,         _STATE_CULLMODE_NONE);
				dev->SetRenderState(RSTATE_ALPHABLENDENABLE, _STATE_ON);
			//	dev->SetRenderState(RSTATE_ZENABLE,          _STATE_ZENABLE_ON_Z_BUF);
				dev->SetRenderState(RSTATE_ZWRITEENABLE,     _STATE_OFF);
			//	dev->SetRenderState(RSTATE_STENCILENABLE,    _STATE_OFF);
			//	dev->SetRenderState(RSTATE_CLIPPLANEENABLE,  _STATE_OFF);
				dev->SetRenderState(RSTATE_BLENDOP,          _STATE_BLENDOP_ADD);
				dev->SetRenderState(RSTATE_SRCBLEND,         _STATE_BLEND_SRCALPHA);
				dev->SetRenderState(RSTATE_DESTBLEND,        _STATE_BLEND_INVSRCALPHA);
			}

			for(uint32 i=start; i<end; i++) // рисуем прозрачные или непрозрачные ; с незаданным материалом не рисуем
			{
				 int32      mID = mtlsort[i]->id_mtl;					// ID материала
				uint32   sCount = (uint32)mtlsort[i]->smodel.size();	// количество наборов вершин
				MODELMTL & mtrl = *material[mID];						// материал
				 int16     KdID = mtrl.id_map_Kd;						// номер diffuse текстуры
				 int16   bumpID = mtrl.id_map_bump;						// номер normal map текстуры
				
				if (cur_ps == SHADER_NONE && ISDIRECTX9) // настройка материала : default DX9 pipeline
				{
					if (mtrl.Tr)																{ // if (draw_translucent)
						uint32 ambient = 0x00FFFFFF & mtrl.Ka;
						       ambient = COLORBYTE(mtrl.d_Tr) << 24;
						uint32 diffuse = 0x00FFFFFF & mtrl.Kd;
						       diffuse = COLORBYTE(mtrl.d_Tr) << 24;
						uint32 speclar = 0x00FFFFFF & mtrl.Ks;
						       speclar = COLORBYTE(mtrl.d_Tr) << 24;
						 dev->SetMaterial(ambient, diffuse, speclar, 0xFF000000, mtrl.Ns);		}
					else dev->SetMaterial(mtrl.Ka, mtrl.Kd, mtrl.Ks, 0xFF000000, mtrl.Ns);
				}

				///////////////////

				if (_NOMISS(KdID)) // настройка diffuse texture
				{				
					dev->SendToRender(tdata[KdID], IDX_TEX_DIFFUSE);

					if (cur_ps == SHADER_NONE && ISDIRECTX9) 
						dev->SetTextureStageState(IDX_TEX_DIFFUSE, TSTATE_MODEL_DIFFUSE_YES); // default DX9 pipeline
				}
				else
				{
					dev->SendToRender((DataManagerTexture*)nullptr, IDX_TEX_DIFFUSE);

					if (cur_ps == SHADER_NONE && ISDIRECTX9) 
						dev->SetTextureStageState(IDX_TEX_DIFFUSE, TSTATE_MODEL_DIFFUSE_NO); // default DX9 pipeline
				};

				///////////////////

				if (_NOMISS(bumpID)) // настройка normal map
				{
					dev->SendToRender(tdata[bumpID], IDX_TEX_BUMP);

					if (cur_ps == SHADER_NONE && ISDIRECTX9) 
						dev->SetTextureStageState(IDX_TEX_BUMP, TSTATE_MODEL_NORMALMAP_YES); // default DX9 pipeline
				}
				else
				{
					dev->SendToRender((DataManagerTexture*)nullptr, IDX_TEX_BUMP);

					if (cur_ps == SHADER_NONE && ISDIRECTX9) 
						dev->SetTextureStageState(IDX_TEX_BUMP, TSTATE_MODEL_NORMALMAP_NO); // default DX9 pipeline
				};

				///////////////////

				if (cur_ps != SHADER_NONE) // настройка материала в свой шейдер
				{
					FLOAT MTLDATA[13] = { mtrl._Ka.r, mtrl._Ka.g, mtrl._Ka.b,
						                  mtrl._Kd.r, mtrl._Kd.g, mtrl._Kd.b,
										  mtrl._Ks.r, mtrl._Ks.g, mtrl._Ks.b,
										  mtrl.d_Tr,
										  mtrl.Ns,
					                      (_ISMISS(KdID)) ? 0.f : 1.f,
										  (_ISMISS(bumpID)) ? 0.f : 1.f };

					SHADER_HANDLE mtl_data = shader->exdata[cur_ps].handle[CONST_MTL_MODEL];

					//for(int __i=0; __i<100; __i++) { // тест нагрузки
					if (D3D_OK != shader->SetFloatArray(cur_ps, mtl_data, MTLDATA, 13)) _MBM(ERROR_ShaderMTLDATA); //};
				}

				bool isDynamic = (matExternalWorld == nullptr && _ISMISS(idSMC));

				for(uint32 n=0; n<sCount; n++)
				{
					uint32 sID = mtlsort[i]->smodel[n]; // номер поверхности SUB
					uint32 gID = smodel[sID]->g_ID;     // номер группы GROUP

					MODELSUB   * sModelCur = smodel[sID];
					MODELGROUP * gModelCur = gmodel[gID];

					//////////////////////////////////////////////////////////////////////////////////////

					if (isDynamic) // только для динамической модели
					{
						if (gModelCur->usePos) // текущая группа использует доп. отклонение
						{
							if ( prevMatGroup != gID )
							{
								if (cur_vs == SHADER_NONE && ISDIRECTX9)
									    dev->SetTransformWorld(matGroupWorld[gID]);
								else shader->SetMatrix(cur_vs, mat_world, matGroupWorld[gID]);
								prevMatGroup = gID;
							}
						}
						else // текущая группа НЕ использует доп. отклонение
						{
							if ( prevMatGroup != -1 )
							{
								if (cur_vs == SHADER_NONE && ISDIRECTX9)
									    dev->SetTransformWorld(matDynamicWorld);
								else shader->SetMatrix(cur_vs, mat_world, matDynamicWorld);
								prevMatGroup = -1;
							}
						}
					}

					else //////////////////////////////////////////////////////////////////////////////////////

					if (matExternalWorld != nullptr) // Подсчёт смещения группы для варианта с внешней матрицей
					{
						if (gModelCur->usePos)
						{
							if ( prevMatGroup != gID )
							{
								(*matResultWorld)  = (*matExternalWorld);
								(*matResultWorld) *= (*matGroupSelf[gID]);

								if (cur_vs == SHADER_NONE && ISDIRECTX9)
									    dev->SetTransformWorld(matResultWorld);
								else shader->SetMatrix(cur_vs, mat_world, matResultWorld);
								prevMatGroup = gID;
							}
						}
						else
						{
							if ( prevMatGroup != -1 )
							{
								if (cur_vs == SHADER_NONE && ISDIRECTX9)
									    dev->SetTransformWorld(matExternalWorld);
								else shader->SetMatrix(cur_vs, mat_world, matExternalWorld);
								prevMatGroup = -1;
							}
						}
					}

					dev->DrawPrimitive(TIP_PRIM_TRIANGLELIST, sModelCur->v_id, 0, sModelCur->nVert, sModelCur->i_id, sModelCur->nPrim);
				}
			}

			if (draw_translucent) // Возврат исходного статуса рендера
			{
				dev->PreviousRenderState(RSTATE_CULLMODE);
				dev->PreviousRenderState(RSTATE_ALPHABLENDENABLE);
			//	dev->PreviousRenderState(RSTATE_ZENABLE);
				dev->PreviousRenderState(RSTATE_ZWRITEENABLE);
			//	dev->PreviousRenderState(RSTATE_STENCILENABLE);
			//	dev->PreviousRenderState(RSTATE_CLIPPLANEENABLE);
				dev->PreviousRenderState(RSTATE_BLENDOP);
				dev->PreviousRenderState(RSTATE_SRCBLEND);
				dev->PreviousRenderState(RSTATE_DESTBLEND);
			}

			if (cur_vs == SHADER_NONE && ISDIRECTX9) shader->RestoreVertexDecl();
		}

	public:
		//>> Рендер динамической модели на сцене (в 2 прохода, непрозрачные части, затем прозрачные)
		void ShowModel(const WPOS * expos)
		{
			if (!isInit)
				if (!PrepareModel(CLEAR_MODEL_RAM)) return;

			this->expos = const_cast<WPOS*>(expos);		ShowModelProc(false, MISSING);
			this->expos = const_cast<WPOS*>(expos);		ShowModelProc(true, MISSING);
		}
		//>> Рендер динамической модели на сцене (в 1 проход, на выбор)
		void ShowModel(const WPOS * expos, bool draw_translucent)
		{
			if (!isInit)
				if (!PrepareModel(CLEAR_MODEL_RAM)) return;

			this->expos = const_cast<WPOS*>(expos);		ShowModelProc(draw_translucent, MISSING);
		}
		//>> Рендер статической  модели на сцене (в 2 прохода)
		void ShowModelStatic(const WPOS * expos, uint32 static_model_id)
		{
			if (!isInit)
				if (!PrepareModel(CLEAR_MODEL_RAM)) return;

			if (static_model_id >= (uint32)matStaticWorld.size())
				{ _MBM(ERROR_IDLIMIT); return; }

			this->expos = const_cast<WPOS*>(expos);		ShowModelProc(false, static_model_id);
			this->expos = const_cast<WPOS*>(expos);		ShowModelProc(true, static_model_id);
		}
		//>> Рендер статической  модели на сцене (в 1 проход)
		void ShowModelStatic(const WPOS * expos, bool draw_translucent, uint32 static_model_id)
		{
			if (!isInit)
				if (!PrepareModel(CLEAR_MODEL_RAM)) return;

			if (static_model_id >= (uint32)matStaticWorld.size())
				{ _MBM(ERROR_IDLIMIT); return; }

			this->expos = const_cast<WPOS*>(expos);		ShowModelProc(draw_translucent, static_model_id);
		}
	};
}

#endif // _MODELRE_H
