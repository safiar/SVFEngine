// ----------------------------------------------------------------------- //
//
// MODULE  : ShaderDX9.cpp
//
// PURPOSE : Вспомогательный класс для написанных HLSL-шейдеров (DX9)
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#include "ShaderDX9.h"

using namespace SAVFGAME;

#define CShaderDX9Check		if (ID == SHADER_NONE) return D3D_OK;														\
							if (!indata[ID].have_handles) { _MBM(ERROR_ShaderEffNoHndl); return D3DERR_INVALIDCALL; }

namespace SAVFGAME
{
	//>> Вызов перед d3ddev->Reset()
	HRESULT CShaderDX9::OnLostDevice()
	{
		for(uint32 i=SHADER_DX9_START+1; i<SHADER_DX9_END; i++)
		{
			switch (info.type[i])
			{
			case TYPE_VERTEX: break;
			case TYPE_PIXEL: break;
			default:
				if (isInit[i])
				if (D3D_OK != eshader[i]->OnLostDevice())
				{
					_MBM(ERROR_OnLostDevice);
					return D3DERR_INVALIDCALL;
				}
			}
		}
		return D3D_OK;
	}

	//>> Вызов после d3ddev->Reset()
	HRESULT CShaderDX9::OnResetDevice()
	{
		for(uint32 i=SHADER_DX9_START+1; i<SHADER_DX9_END; i++)
		{
			switch (info.type[i])
			{
			case TYPE_VERTEX: break;
			case TYPE_PIXEL: break;
			default:
				if (isInit[i])
				if (D3D_OK != eshader[i]->OnResetDevice())
				{
					_MBM(ERROR_OnResetDevice);
					return D3DERR_INVALIDCALL;
				}
			}
		}
		return D3D_OK;
	}

	//>> Установка текстуры эффекта
	HRESULT CShaderDX9::SetEffectTexture(eShaderID ID, SHADER_HANDLE handle, void * p_texture)
	{
		CShaderDX9Check	
		return eshader[ID]->SetTexture(handle, (IDirect3DBaseTexture9*) p_texture);
	}

	//>> Установка техники эффекта
	HRESULT CShaderDX9::SetEffectTechnique(eShaderID ID, eShaderEffectTechID techID)
	{
		CShaderDX9Check
		if (__ISMISS(info.tech.cur[ID]))
		{
			_MBM(ERROR_ShaderEffTchMiss);
			return D3DERR_INVALIDCALL;
		}
		return eshader[ID]->SetTechnique( info.tech.p[ID][techID] );
	}

	//>> Начать эффект
	HRESULT CShaderDX9::EffectBegin(eShaderID ID, uint32 * pPasses, uint32 flags)
	{
		return eshader[ID]->Begin(pPasses,flags);
	}

	//>> Запуск прохода
	HRESULT CShaderDX9::EffectBeginPass(eShaderID ID, uint32 pass)
	{
		return eshader[ID]->BeginPass(pass);
	}

	//>> Окончания прохода
	HRESULT CShaderDX9::EffectEndPass(eShaderID ID)
	{
		return eshader[ID]->EndPass();
	}

	//>> Закончить эффект
	HRESULT CShaderDX9::EffectEnd(eShaderID ID)
	{
		return eshader[ID]->End();
	}

	//>> Установка матрицы
	HRESULT CShaderDX9::SetMatrix(eShaderID ID, SHADER_HANDLE handle, MATH3DMATRIX * pMatrix)
	{
		CShaderDX9Check
		switch(info.type[ID])
		{
		case TYPE_VERTEX:
		case TYPE_PIXEL:
			return table[ID]->SetMatrix(d3ddev, handle, D3DCAST(pMatrix)); break;
		default:
			return eshader[ID]->SetMatrix(handle, D3DCAST(pMatrix));
		}
	}

	//>> Установка транспонированной матрицы																			Mat * Vec == Vec * Transpose(Mat)	
	//>> Set transform states to initialize transformation matrices. Effects use transposed matrices for efficiency.	Vec * Mat == Transpose(Mat) * Vec
	//>> You can provide transposed matrices to an effect, or an effect will automatically transpose the matrices before using them.
	HRESULT CShaderDX9::SetMatrixTranspose(eShaderID ID, SHADER_HANDLE handle, MATH3DMATRIX * pMatrix)
	{
		CShaderDX9Check
		switch(info.type[ID])
		{
		case TYPE_VERTEX:
		case TYPE_PIXEL:
			return table[ID]->SetMatrixTranspose(d3ddev, handle, D3DCAST(pMatrix)); break;
		default:
			return eshader[ID]->SetMatrixTranspose(handle, D3DCAST(pMatrix));
		}
	}

	//>> Установка raw value
	HRESULT CShaderDX9::SetRaw(eShaderID ID, SHADER_HANDLE handle, void * pData, uint32 bytes)
	{
		CShaderDX9Check
		switch(info.type[ID])
		{
		case TYPE_VERTEX:
		case TYPE_PIXEL:
			return table[ID]->SetValue(d3ddev,handle,pData,bytes); break;
		default:
			return eshader[ID]->SetValue(handle,pData,bytes);
		}
	}

	//>> Установка bool
	HRESULT CShaderDX9::SetBool(eShaderID ID, SHADER_HANDLE handle, int32 b)
	{
		CShaderDX9Check
		switch(info.type[ID])
		{
		case TYPE_VERTEX:
		case TYPE_PIXEL:
			return table[ID]->SetBool(d3ddev, handle, b); break;
		default:
			return eshader[ID]->SetBool(handle, b);
		}
	}

	//>> Установка int
	HRESULT CShaderDX9::SetInt(eShaderID ID, SHADER_HANDLE handle, int32 n)
	{
		CShaderDX9Check
		switch(info.type[ID])
		{
		case TYPE_VERTEX:
		case TYPE_PIXEL:
			return table[ID]->SetInt(d3ddev,handle,n); break;
		default:
			return eshader[ID]->SetInt(handle,n);
		}
	}

	//>> Установка float
	HRESULT CShaderDX9::SetFloat(eShaderID ID, SHADER_HANDLE handle, float f)
	{
		CShaderDX9Check
		switch(info.type[ID])
		{
		case TYPE_VERTEX:
		case TYPE_PIXEL:
			return table[ID]->SetFloat(d3ddev,handle,f); break;
		default:
			return eshader[ID]->SetFloat(handle,f);
		}
	}

	//>> Установка vector
	HRESULT CShaderDX9::SetVector(eShaderID ID, SHADER_HANDLE handle, MATH3DVEC4 * pVector)
	{
		CShaderDX9Check
		switch(info.type[ID])
		{
		case TYPE_VERTEX:
		case TYPE_PIXEL:
			return table[ID]->SetVector(d3ddev, handle, D3DCAST(pVector)); break;
		default:
			return eshader[ID]->SetVector(handle, D3DCAST(pVector));
		}
	}

	//>> Установка массива bool
	HRESULT CShaderDX9::SetBoolArray(eShaderID ID, SHADER_HANDLE handle, int32 *pb, uint32 Count)
	{
		CShaderDX9Check
		switch(info.type[ID])
		{
		case TYPE_VERTEX:
		case TYPE_PIXEL:
			return table[ID]->SetBoolArray(d3ddev, handle, pb, Count); break;
		default:
			return eshader[ID]->SetBoolArray(handle, pb, Count);
		}
	}

	//>> Установка массива int
	HRESULT CShaderDX9::SetIntArray(eShaderID ID, SHADER_HANDLE handle, int32 *pn, uint32 Count)
	{
		CShaderDX9Check
		switch(info.type[ID])
		{
		case TYPE_VERTEX:
		case TYPE_PIXEL:
			return table[ID]->SetIntArray(d3ddev,handle,pn,Count); break;
		default:
			return eshader[ID]->SetIntArray(handle,pn,Count);
		}
	}

	//>> Установка массива float
	HRESULT CShaderDX9::SetFloatArray(eShaderID ID, SHADER_HANDLE handle, float *pf, uint32 Count)
	{
		CShaderDX9Check
		switch(info.type[ID])
		{
		case TYPE_VERTEX:
		case TYPE_PIXEL:
			return table[ID]->SetFloatArray(d3ddev,handle,pf,Count); break;
		default:
			return eshader[ID]->SetFloatArray(handle,pf,Count);
		}
	}

	//>> Установка массива vector
	HRESULT CShaderDX9::SetVectorArray(eShaderID ID, SHADER_HANDLE handle, MATH3DVEC4 * pVector, uint32 Count)
	{
		CShaderDX9Check
		switch(info.type[ID])
		{
		case TYPE_VERTEX:
		case TYPE_PIXEL:
			return table[ID]->SetVectorArray(d3ddev, handle, D3DCAST(pVector), Count); break;
		default:
			return eshader[ID]->SetVectorArray(handle, D3DCAST(pVector), Count);
		}
	}
}