// ----------------------------------------------------------------------- //
//
// MODULE  : FontRe.h
//
// PURPOSE : Отрисовка текстов
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _FONTRE_H
#define _FONTRE_H

#include "Font.h"
#include "ModelRe.h"

#define  CLEAR_FONT_RAM  true  // удалять загруженное в VideoRAM из RAM ?

using namespace SAVFGAME;

namespace SAVFGAME
{
	class CFontR : public CBaseFont
	{
	protected:
		DEV3DBASE *						dev       { nullptr };
		shared_ptr <DataManagerTexture>	tdata_shr { nullptr }; // текстура шрифта
		DataManagerTexture *			tdata     { nullptr }; // текстура шрифта
		shared_ptr <DataManagerModel>	mdata_shr { nullptr }; // текущий текст на показ
		DataManagerModel *				mdata     { nullptr }; // текущий текст на показ
	protected:
		VECDATAP <DataManagerModel>		cmdata;				// набор постоянных текстов
		int64							font_ct_iterator;	// итератор сброса cmdata :: значение MISSING (-1) зарезервировано
	private:
	public:
		CFontR() : CBaseFont(), font_ct_iterator(0) {};
		~CFontR() { Close(); };
		void CloseConstantTexts()
		{
			cmdata.Close(1,1);
			font_ct_iterator++;
			if (_ISMISS(font_ct_iterator))
				font_ct_iterator++;
		}
		void Close()
		{
			CloseConstantTexts();
			_CLOSESHARED(tdata_shr);
			_CLOSESHARED(mdata_shr);
			tdata = nullptr;
			mdata = nullptr;
			dev   = nullptr;
			font_ct_iterator = 0;
			CBaseFont::Close();
		}

		//>> Установка девайса
		void SetDevice(const DEV3DBASE * pDevice)
		{
			if (pDevice == nullptr) { _MBM(ERROR_PointerNone); return; }

			dev = const_cast<DEV3DBASE*>(pDevice);
		};

	protected:
		//>> Загрузка в видеопамять
		void PrepareTexture(bool clear_mem)
		{
			if (tdata_shr != nullptr) return;

			tdata_shr = dev->CreateTextureData(bmfont.block03.pageFile[0]->buf, bmfont.block03.pageFile[0]->count);
			tdata = tdata_shr.get();

			if (clear_mem) CBaseFont::DeleteTEX(); // удаление данных текстур в RAM
		}
		//>> Подготовка полигонов
		bool PrepareText(CText * info)
		{
			int64 ID = info->ConstantTextGetID(this->ID); // Номер к cmdata[ID] с данными полигонов (модели)

			bool _CT_Reload = false;					// Метка требования генерации текста заново
			bool _CT_ResetIteration = false;			// Метка несоответствия итерации у текста с текущей итерацией шрифта
			bool _CT_IsConst = info->IsTextConstant();	// 
			bool _CT_IsHaveId = (_NOMISS(ID));			// или info.ConstantTextIsHaveFontID(this->ID);  (достаточно проверить одно)
														
			if (_CT_IsHaveId) // ID != MISSING  или  font ID != 0  (достаточно проверить одно)
			{	
				bool _RL = info->ConstantTextCheckReload();
				int _FIT = info->ConstantTextCheckFontIteration(this->ID, font_ct_iterator);

				if (_CT_IsConst && _FIT == 0) // _FIT == false  (iteration mismatch) ; был вызов CloseConstantTexts(), cmdata очищено
				{
					_CT_ResetIteration = true;
				}
				else if (_CT_IsConst && !_RL) // Постоянный и уже подготовленный текст
				{
					mdata_shr = cmdata[ID];
					mdata = mdata_shr.get();

					if (mdata->num_vertices) return true;	// valid text
					else return false;						// empty text
				}
				else if (_CT_IsConst && _RL) // Постоянный, но требующий перезагрузку
				{
					//cmdata[ID].data->Close();
					_CT_Reload = _RL;
				}
				else // Текст был постоянным, но сейчас динамический
				{
					//cmdata[ID].data->Close();
				}
			}

			/////////////////////////////////////////////////////////////////////////////////////////////

			TBUFFER <FONTVERTEX, uint32>  vertices; // массив вершин
			TBUFFER <uint32,     uint32>  indices;  // массив индексов

			wstring * w_ptr = info->ExTextPointerGet();
			
			if (tdata_shr == nullptr) PrepareTexture(CLEAR_FONT_RAM);

			if (w_ptr != nullptr)
				 Generate(*w_ptr,     info->height, info->start_symbol_pos, info->end_symbol_pos, vertices, indices);
			else Generate(info->text, info->height, info->start_symbol_pos, info->end_symbol_pos, vertices, indices);

			if (info->gradient == false)
				 GenerateColor(vertices.buf, vertices.count, info->color1);
			else GenerateColor(vertices.buf, vertices.count, info->color1, info->color2);

			mdata_shr = dev->CreateModelData(vertices.buf, indices.buf, vertices.count, indices.count, sizeof(FONTVERTEX));
			mdata = mdata_shr.get();

			/////////////////////////////////////////////////////////////////////////////////////////////

			if (_CT_IsConst)
			{
				if (_CT_ResetIteration ||	// Вектор "constant_text" был очищен и буферы потеряны
					!_CT_IsHaveId)			// или новый постоянный текст
				{
					int64 ID = (int64)cmdata.size();
					if (_CT_ResetIteration && _CT_IsHaveId)
						 info->ConstantTextSetIDInAlreadyExistPattern (ID, this->ID, font_ct_iterator) ;
					else info->ConstantTextAddID                      (ID, this->ID, font_ct_iterator) ;
					cmdata.Add(1);
					cmdata.Reset(mdata_shr, ID);
				}
				else if (_CT_Reload) // Старый постоянный текст, требующий перезагрузку
				{
					cmdata.Reset(mdata_shr, ID);
				}
			}

			if (mdata->num_vertices) return true;	// valid text
			else return false;						// empty text
		}

	public:
		//>> Рендерит текст
		bool ShowText(CText * info)
		{
			if (dev == nullptr)     { _MBM(ERROR_PointerNone);  return false; }
			if (!CBaseFont::isInit) { _MBM(ERROR_InitNone);     return false; }

			if (window_W != IO->window.client_width ||
				window_H != IO->window.client_height) CBaseFont::Update();

			if (!PrepareText(info)) return false;

			eShaderID cur_vs = shader->GetCurVS(); // текущий вершинный шейдер
			eShaderID cur_ps = shader->GetCurPS(); // текущий пиксельный шейдер

			dev->SendToRender(tdata, IDX_TEX_DIFFUSE);

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

			dev->SendToRender(mdata, 0, sizeof(FONTVERTEX));
		
			SHADER_HANDLE mat_world    = shader->exdata[cur_vs].handle[CONST_MAT_WORLD];
			SHADER_HANDLE mat_viewproj = shader->exdata[cur_vs].handle[CONST_MAT_VIEWPROJ];

			if (!info->UI) // Текст на сцене
			{
				const WPOS * pos = info->pos;
				MathWorldMatrix(pos->P, pos->Q, pos->S, *matWorld);			
				
				//ЗАМЕТКА: Ещё вариант расчёта матрицы преобразования для щита billboard
				//Device->GetTransform(D3DTS_VIEW, &matView);
				//D3DXMatrixTranspose(&matTransposed, &matView);
				//Device->SetTransform(D3DTS_WORLD, &matTransposed);

				if (cur_vs == SHADER_NONE && ISDIRECTX9)
					    dev->SetTransformWorld(matWorld);
				else shader->SetMatrix(cur_vs, mat_world, matWorld);
			}
			else // Текст на экране (UI)
			{
				const WPOS * pos = info->pos;		
				MathWorldMatrix(MATH3DVEC(pos->x, window_H - pos->y, 0), pos->Q, pos->S, *matWorld);

				if (cur_vs == SHADER_NONE && ISDIRECTX9) // default DX9 pipeline
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

			dev->DrawPrimitive(TIP_PRIM_TRIANGLELIST, 0, 0, mdata->num_vertices, 0, mdata->num_primitives);

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

#endif // _FONTRE_H