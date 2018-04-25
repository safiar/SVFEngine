// ----------------------------------------------------------------------- //
//
// MODULE  : ModelDX9.cpp
//
// PURPOSE : Методы отрисовки моделей с использованием средств Direct3D_9
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#include "ModelDX9.h"

using namespace SAVFGAME;

namespace SAVFGAME
{
		/*void CModelDX9::ShowModelProc_old1test(const bool& draw_translucent, const uint32& idSMC)
		{
			D3DMATERIAL9  d3d_material;

			D3DXMATRIX	matDynamicWorld;
			bool		matDynamicInit = FALSE;

			//d3ddev->SetFVF(MODELFVF);
			d3ddev->SetStreamSource(0, mdata.vbuf, 0, sizeof(MODELVERTEX));
			d3ddev->SetIndices(mdata.ibuf);

			/////////////////////////// Настройка матриц преобразования //////////////////////////////////

			for(uint32 n=0; n<nGroups; n++) // Логические группы (дверь авто, колесо авто и т.д.), могут иметь своё смещение относительно всей модели
			{
				D3DXMATRIX matTranslate, matRotation, matScale, matGroupWorld;

				if ( (!matStaticInit[idSMC] || !isStatic) && (!matExternalUse) ) // (не внешняя)  и  (статич. ещё не рассчитана  или  динамич.)
				{
					if (!isStatic && gmodel[n].usePos) // (динамич.) вместе с (принимаем в рассчёт относительное смещение группы)
					{
						D3DXMatrixTranslation ( &matTranslate,
							pos.x + gmodel[n].pos.x,
							pos.y + gmodel[n].pos.y,
							pos.z + gmodel[n].pos.z );
						D3DXMatrixRotationYawPitchRoll ( &matRotation,
							D3DXToRadian(fmod(pos.angle_y + gmodel[n].pos.angle_y, 360)),
							D3DXToRadian(fmod(pos.angle_x + gmodel[n].pos.angle_x, 360)),
							D3DXToRadian(fmod(pos.angle_z + gmodel[n].pos.angle_z, 360)) );
						D3DXMatrixScaling ( &matScale,
							pos.scale_x * gmodel[n].pos.scale_x,
							pos.scale_y * gmodel[n].pos.scale_y,
							pos.scale_z * gmodel[n].pos.scale_z );

						matGroupWorld = matScale * matRotation * matTranslate;
					}
					else if (!matDynamicInit || !matStaticInit[idSMC]) // статич. или динамич. не рассчитана
					{
						D3DXMatrixTranslation ( &matTranslate, pos.x, pos.y, pos.z );
						D3DXMatrixRotationYawPitchRoll ( &matRotation,
							D3DXToRadian(pos.angle_y),
							D3DXToRadian(pos.angle_x),
							D3DXToRadian(pos.angle_z) );
						D3DXMatrixScaling( &matScale, pos.scale_x, pos.scale_y, pos.scale_z );

						if (!matDynamicInit)												{
							matDynamicWorld = matScale * matRotation * matTranslate;
							matDynamicInit  = TRUE;											}
						if (!matStaticInit[idSMC])											{
							matStaticWorld[idSMC] = matScale * matRotation * matTranslate;
							matStaticInit[idSMC]  = 1;										}
					}	
				}

				D3DXMATRIX matShader;

					 if (matExternalUse)	{d3ddev->SetTransform(D3DTS_WORLD, &matExternalWorld);		matShader = matExternalWorld;}
				else if (isStatic)			{d3ddev->SetTransform(D3DTS_WORLD, &matStaticWorld[idSMC]); matShader = matStaticWorld[idSMC];}
				else if (!gmodel[n].usePos)	{d3ddev->SetTransform(D3DTS_WORLD, &matDynamicWorld);		matShader = matDynamicWorld;}
				else						{d3ddev->SetTransform(D3DTS_WORLD, &matGroupWorld);			matShader = matGroupWorld;}
			
			//	static const D3DXHANDLE mat_world    = shader->exdata[SHADER_VERTEX_LIGHT].handle[VAR_LIGHT_MAT_WORLD];

			//	shader->SetMatrix(SHADER_VERTEX_LIGHT, mat_world,    &matShader);

				/////////////////////////// Настройка цвета и текстур //////////////////////////////////

				uint32 mNum = gmodel[n].id.size();
				for(uint32 i=0; i<mNum; i++) // Наборы вершин, у каждого набора своя текстура, материал, настройка прозрачности, цвет...
				{
					int curID  = gmodel[n].id[i];		  // номер набора smodel и smdata
					int32 mID  = smodel[curID].mtl_ID;	  // номер материала 
					int16 KdID = material[mID].id_map_Kd; // номер diffuse текстуры
																						  // Условие пропуска
					if (!draw_translucent &&   material[mID].Tr               ) continue; // Рисовать непрозрачную часть, но материал прозрачен
					if ( draw_translucent && (!material[mID].Tr || KdID != -1)) continue; // Рисовать   прозрачную часть, но материал непрозрачен или текстура

					DWORD CULLMODE_state;   d3ddev->GetRenderState(D3DRS_CULLMODE,			&CULLMODE_state);
					DWORD ALPHA_state;		d3ddev->GetRenderState(D3DRS_ALPHABLENDENABLE,	&ALPHA_state);
					DWORD ZBUF_state;		d3ddev->GetRenderState(D3DRS_ZENABLE,			&ZBUF_state);
					DWORD ZBUFWR_state;		d3ddev->GetRenderState(D3DRS_ZWRITEENABLE,		&ZBUFWR_state);
					DWORD STENCIL_state;	d3ddev->GetRenderState(D3DRS_STENCILENABLE,		&STENCIL_state);
					DWORD BLENDOP_state;	d3ddev->GetRenderState(D3DRS_BLENDOP,			&BLENDOP_state);
					DWORD SRCBLEND_state;	d3ddev->GetRenderState(D3DRS_SRCBLEND,			&SRCBLEND_state);
					DWORD DESTBLEND_state;	d3ddev->GetRenderState(D3DRS_DESTBLEND,			&DESTBLEND_state);

					//d3ddev->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL); // D3DMCS_MATERIAL   D3DMCS_COLOR1
					//d3ddev->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
					//d3ddev->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
					//d3ddev->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
					//d3ddev->SetRenderState(D3DRS_COLORVERTEX, FALSE);

					//d3ddev->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);

					if (mID != -1)
					{				
					//	d3d_material.Ambient  = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);  // фоновый
					//	d3d_material.Diffuse  = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);  // рассеиваемый
					//	d3d_material.Specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);  // отражаемый
					//	d3d_material.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);	// свечение
					//	d3d_material.Power    = 1.0f;								// резкость зеркальных отражений

						d3d_material.Ambient  = D3DXCOLOR(material[mID].Ka);	// 
						d3d_material.Diffuse  = D3DXCOLOR(material[mID].Kd);	// D3DTA_DIFFUSE
						d3d_material.Specular = D3DXCOLOR(material[mID].Ks);	// 
						d3d_material.Emissive = D3DXCOLOR(0xFF000000);			// D3DCOLOR_ARGB(255,0,0,0)
						d3d_material.Power    = material[mID].Ns;

					//	switch(material[mID].illum) {
							// Ft	Fresnel reflectance
							// Ft	Fresnel transmittance
							// Ia	ambient light
							// I	light intensity
							// Ir	intensity from reflected direction
							//		(reflection map and/or ray tracing)
							// It	intensity from transmitted direction
							// Ka	ambient reflectance
							// Kd	diffuse reflectance
							// Ks	specular reflectance
							// Tf	transmission filter

							// H	unit vector bisector between L and V
							// L	unit light vector
							// N	unit surface normal
							// V	unit view vector
					//	case 0:														// 0: color = Kd
					//		d3d_material.Diffuse  = D3DXCOLOR(material[mID].Kd);
					//		break;
					//	case 1:														// 1: color = KaIa
					//		d3d_material.Ambient  = D3DXCOLOR(material[mID].Ka);	//          + Kd  { SUM j=1..ls, (N * Lj)Ij }
					//		d3d_material.Diffuse  = D3DXCOLOR(material[mID].Kd);
					//		break;
					//	case 2:														// 2: color = KaIa 
					//		d3d_material.Ambient  = D3DXCOLOR(material[mID].Ka);	//          + Kd  { SUM j=1..ls, (N*Lj)Ij }
					//		d3d_material.Diffuse  = D3DXCOLOR(material[mID].Kd);	//          + Ks  { SUM j=1..ls, ((H*Hj)^Ns)Ij }
					//		d3d_material.Specular = D3DXCOLOR(material[mID].Ks);
					//		d3d_material.Power    = material[mID].Ns;
					//		break;
					//	case 3:
					//		d3d_material.Ambient  = D3DXCOLOR(material[mID].Ka);	// 3: color = KaIa
					//		d3d_material.Diffuse  = D3DXCOLOR(material[mID].Kd);	//          + Kd  { SUM j=1..ls, (N*Lj)Ij }
					//		d3d_material.Specular = D3DXCOLOR(material[mID].Ks);	//          + Ks ({ SUM j=1..ls, ((H*Hj)^Ns)Ij } + Ir)
					//		d3d_material.Power    = material[mID].Ns;
					//		break;
					//	case 4:  // Transparency: Glass on			Reflection: Ray trace on
					//	case 5:  //									Reflection: Fresnel on and Ray trace on
					//	case 6:  // Transparency: Refraction on		Reflection: Fresnel off and Ray trace on
					//	case 7:  // Transparency: Refraction on		Reflection: Fresnel on and Ray trace on
					//	case 8:  //									Reflection on and Ray trace off
					//	case 9:  // Transparency: Glass on			Reflection: Ray trace off
					//	case 10: //			Casts shadows onto invisible surfaces
					//		d3d_material.Ambient  = D3DXCOLOR(material[mID].Ka);
					//		d3d_material.Diffuse  = D3DXCOLOR(material[mID].Kd);
					//		d3d_material.Specular = D3DXCOLOR(material[mID].Ks);
					//		d3d_material.Power    = material[mID].Ns;
					//		break;
					//	default: ;
					//	}
						d3ddev->SetMaterial(&d3d_material);

						//d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
						//d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
						//d3ddev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
						//d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
						//d3ddev->SetRenderState(D3DRS_STENCILENABLE, FALSE);
						//d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
						//d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
						//d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);  // D3DBLEND_DESTCOLOR  D3DBLEND_INVSRCALPHA
						
						if (KdID != -1)
						{
							d3ddev->SetTexture(0, tdata[KdID].texture);
							d3ddev->SetTexture(1, NULL);
							d3ddev->SetTexture(2, NULL);

							d3ddev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
							d3ddev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
							d3ddev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
							d3ddev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
							d3ddev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

							d3ddev->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
							d3ddev->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
							d3ddev->SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_DISABLE);
							d3ddev->SetTextureStageState(2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

						//	d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
						//	d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
						//	d3ddev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
						//	d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
						//	d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);	//  _SUBTRACT s-d  _REVSUBTRACT d-s  _MIN min(s,d)  _MAX max(s,d)
						//	d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); // D3DBLEND_SRCALPHA
						//	d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);  // D3DBLEND_DESTCOLOR  D3DBLEND_INVSRCALPHA
						}
						else
						{
							d3ddev->SetTexture(0, NULL);

							//d3ddev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);	// D3DTOP_SELECTARG1
							//d3ddev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
							d3ddev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);	
							d3ddev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
							d3ddev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
							d3ddev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
							//d3ddev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CONSTANT);
							//d3ddev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CONSTANT);
							//d3ddev->SetTextureStageState(0, D3DTSS_CONSTANT,  D3DXCOLOR(0xFFFFFFFF));
							
							if (material[mID].Tr)
							{
								uint32 clr  = 0x00FFFFFF & material[mID].Kd;
								       clr |= COLORBYTE(material[mID].d_Tr) << 24;

								d3d_material.Diffuse = D3DXCOLOR(clr);

								d3ddev->SetRenderState(D3DRS_CULLMODE, CULL_NON);
								d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
								d3ddev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
								d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
								d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
								d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
								d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);  // D3DBLEND_DESTCOLOR  D3DBLEND_INVSRCALPHA
							}
						}
					}
					else
					{
						d3ddev->SetTexture(0, NULL);

						wchar_t error[256];
						wsprintf(error,L"%s\nsubmodel %i\n%s", ERROR_MdlNoMaterial, curID, name.c_str());
						_MB(NULL, error, ERROR_Warning, ERROR_MB);
					}

				//	static const D3DXHANDLE mtl_ambient  = shader->exdata[SHADER_VERTEX_LIGHT].handle[VAR_LIGHT_MTL_AMBIENT];
				//	static const D3DXHANDLE mtl_diffuse  = shader->exdata[SHADER_VERTEX_LIGHT].handle[VAR_LIGHT_MTL_DIFFUSE];
				//	static const D3DXHANDLE mtl_specular = shader->exdata[SHADER_VERTEX_LIGHT].handle[VAR_LIGHT_MTL_SPECULAR];

				//	shader->SetVector(SHADER_VERTEX_LIGHT, mtl_ambient,  &D3DXVECTOR4(material[mID]._Ka[0], material[mID]._Ka[1], material[mID]._Ka[2], 1.0f) );
				//	shader->SetVector(SHADER_VERTEX_LIGHT, mtl_diffuse,  &D3DXVECTOR4(material[mID]._Kd[0], material[mID]._Kd[1], material[mID]._Kd[2], d3d_material.Diffuse.a) );
				//	shader->SetVector(SHADER_VERTEX_LIGHT, mtl_specular, &D3DXVECTOR4(material[mID]._Ks[0], material[mID]._Ks[1], material[mID]._Ks[2], 1.0f) );

					d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, smodel[curID].v_id, 0, smodel[curID].nVert, smodel[curID].i_id, smodel[curID].nPrim);

					// Возврат исходного статуса рендера
					d3ddev->SetRenderState(D3DRS_CULLMODE,			CULLMODE_state); 
					d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE,	ALPHA_state);
					d3ddev->SetRenderState(D3DRS_ZENABLE,			ZBUF_state);
					d3ddev->SetRenderState(D3DRS_ZWRITEENABLE,		ZBUFWR_state);
					d3ddev->SetRenderState(D3DRS_STENCILENABLE,		STENCIL_state);
					d3ddev->SetRenderState(D3DRS_BLENDOP,			BLENDOP_state);
					d3ddev->SetRenderState(D3DRS_SRCBLEND,			SRCBLEND_state);
					d3ddev->SetRenderState(D3DRS_DESTBLEND,			DESTBLEND_state);
				}
			}
		} */ ;
	
		
}