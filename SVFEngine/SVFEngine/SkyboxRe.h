// ----------------------------------------------------------------------- //
//
// MODULE  : SkyboxRe.h
//
// PURPOSE : Отрисовка skybox
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _SKYBOXRE_H
#define _SKYBOXRE_H

#include "device3d.h"
#include "Shader.h"
#include "Skybox.h"

#define  CLEAR_SKYBOX_RAM  true  // удалять загруженное в VideoRAM из RAM ?

using namespace SAVFGAME;

namespace SAVFGAME
{
	class CSkyboxR final : public CSkybox
	{
	protected:
		DEV3DBASE *							dev       { nullptr };
		CShader *							shader    { nullptr };
		shared_ptr<DataManagerModel>		mdata_shr { nullptr };
		shared_ptr<DataManagerCubeTexture>	tdata_shr { nullptr };
		DataManagerModel *					mdata     { nullptr };
		DataManagerCubeTexture *			tdata     { nullptr };
		bool								isInit;
	private:
	public:
		CSkyboxR() : CSkybox(), isInit(false) {};
		~CSkyboxR() { Close(); };

		void Close()
		{
			if (isInit)
			{
				_CLOSESHARED(tdata_shr);
				_CLOSESHARED(mdata_shr);
				mdata  = nullptr;
				tdata  = nullptr;
				dev    = nullptr;
				shader = nullptr;
				isInit = false;
			}
			CSkybox::Close();
		}

		void Init(const DEV3DBASE * pDevice, const CShader * pShader)
		{
			if (pShader == nullptr) { _MBM(ERROR_PointerNone);  return; }
			if (pDevice == nullptr) { _MBM(ERROR_PointerNone);  return; }

			shader = const_cast<CShader*>(pShader);
			dev    = const_cast<DEV3DBASE*>(pDevice);
		}

		//>> Загрузка в видеопамять
		void PrepareSkybox(bool clear_mem)
		{
			if (isInit) return;

			if (dev == nullptr)    { _MBM(ERROR_PointerNone);   return; }
			if (shader == nullptr) { _MBM(ERROR_PointerNone);   return; }
			if (!CSkybox::isInit)  { _MBM(ERROR_InitNone);      return; }

			tdata_shr = dev->CreateCubeTextureData(texture.buf, texture.count);
			mdata_shr = dev->CreateModelData(vertices.buf, indices.buf, vertices.count, indices.count, sizeof(MODELVERTEX));
			tdata = tdata_shr.get();
			mdata = mdata_shr.get();

			if (clear_mem)
			{
				CSkybox::DeleteTEX(); // удаление данных текстур в RAM
				CSkybox::DeleteVI();  // удаление данных вершин и индексов в RAM
			}

			isInit = true;
		}

		//>> Загрузка в видеопамять
		void PrepareSkybox()
		{
			PrepareSkybox(CLEAR_SKYBOX_RAM);
		}

		//>> Рендер скайбокса через вершинный и пискельный шейдеры
		void ShowSkybox()
		{
			if (!isInit) PrepareSkybox(CLEAR_SKYBOX_RAM);

			shader->State(SHADERSKYV, true);
			shader->State(SHADERSKYP, true);

			//d3ddev->SetTexture(shader->exdata[SHADER_PIXEL_SKYBOX].desc[VAR_SKYBOX_TEX_ID].RegisterIndex, tdata.texture); // shader var "SkyBoxTexture"

			dev->SendToRender(tdata, IDX_TEX_ENVCUBE);
			dev->SendToRender(mdata, 0, sizeof(MODELVERTEX));

			dev->SetRenderState(RSTATE_CULLMODE,     _STATE_CULLMODE_NONE);
			dev->SetRenderState(RSTATE_ZENABLE,      _STATE_ZENABLE_OFF);
			dev->SetRenderState(RSTATE_ZWRITEENABLE, _STATE_OFF);

			dev->DrawPrimitive(TIP_PRIM_TRIANGLELIST, 0, 0, mdata->num_vertices, 0, mdata->num_primitives);

			dev->PreviousRenderState(RSTATE_CULLMODE);
			dev->PreviousRenderState(RSTATE_ZENABLE);
			dev->PreviousRenderState(RSTATE_ZWRITEENABLE);

			shader->State(SHADERSKYP, false);
			shader->State(SHADERSKYV, false);

			//IDirect3DSurface9 * face_p_x;
			//IDirect3DSurface9 * back_buffer;
			//tdata.texture->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_X, 0, &face_p_x);
			//d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
			//d3ddev->UpdateSurface(face_p_x, 0, back_buffer, 0);
			//back_buffer->Release();
			//face_p_x->Release();
		}

	/*	//>> TEST: Рендер скайбокса через FX эффект
		void ShowSkyboxEffect()
		{
			if (!isPrepared) PrepareSkybox(CLEAR_SKYBOX_RAM);

			eShaderID ID = SHADER_EFFECT30_SKYBOX;

			shader->State(ID, true);

			d3ddev->SetTexture(IDX_TEX_ENVCUBE, tdata.texture); // для прочих целей вне skybox шейдера

			shader->SetEffectTexture(ID, shader->exdata[ID].handle[VAR_SKYBOX_EFF_TEX_ID], tdata.texture);

			d3ddev->SetStreamSource(0, mdata.vbuf, 0, sizeof(MODELVERTEX));
			d3ddev->SetIndices(mdata.ibuf);

			UINT numPasses = 0;
			shader->EffectBegin(ID, &numPasses, NULL);
			shader->EffectBeginPass(ID, 0);
			d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mdata.numVert, 0, mdata.primCount);
			shader->EffectEndPass(ID);
			shader->EffectEnd(ID);

			shader->State(ID, false);
		} //*/
	};
}

#endif // _SKYBOXRE_H
