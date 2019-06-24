// ----------------------------------------------------------------------- //
//
// MODULE  : ModelPre.h
//
// PURPOSE : Подготовительный класс моделей
//           Данные статических моделей можно хранить в самом классе
//           Данные динамических моделей и группы подмодели подразумеваются
//           переменными, задаются перед отрисовкой и/или хранятся внешне
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _MODELPRE_H
#define _MODELPRE_H

#include "Model.h"
#include "Shader.h"

using namespace SAVFGAME;

namespace SAVFGAME
{
	class CModelPre : public CModel
	{
	protected:
		MATH3DMATRIX *				matResultWorld;      // для дополнительных расчётов
		MATH3DMATRIX *				matDynamicWorld;	 // для динамических моделей
		MATH3DMATRIX *				matExternalWorld;	 // для внешне заданной матрицы, по умолчанию не используется (nullptr)
		VECPDATA <MATH3DMATRIX>		matStaticWorld;		 // массив для хранения матриц статических моделей
		VECPDATA <MATH3DMATRIX>		matGroupWorld;		 // массив для хранения матриц групп: GroupWorld = DynamicWorld * GroupSelf
		VECPDATA <MATH3DMATRIX>		matGroupSelf;		 // массив для хранения матриц групп: GroupWorld = DynamicWorld * GroupSelf
		bool						isInit;
	public:
		CModelPre(const CModelPre& src)				= delete;
		CModelPre(CModelPre&& src)					= delete;
		CModelPre& operator=(CModelPre&& src)		= delete;
		CModelPre& operator=(const CModelPre& src)	= delete;
	public:
		CModelPre() :              CModel()
		{
			isInit = false;
			matExternalWorld = nullptr;
			matResultWorld   = new MATH3DMATRIX;
			matDynamicWorld  = new MATH3DMATRIX;
		}
		CModelPre(bool cull_CCW) : CModel(cull_CCW)
		{
			isInit = false;
			matExternalWorld = nullptr;
			matResultWorld   = new MATH3DMATRIX;
			matDynamicWorld  = new MATH3DMATRIX;
		}
		~CModelPre()
		{
			Close();
			_DELETE(matResultWorld);
			_DELETE(matDynamicWorld);
		};
		void Close()
		{
			if (isInit)
			{
				matStaticWorld.Delete(1);
				matGroupWorld.Delete(1);
				matGroupSelf.Delete(1);
				matExternalWorld = nullptr;
				isInit = false;
			}
			CModel::Close();
		}

		//>> Подготовка матриц
		void Init()
		{
			if (isInit) return;
			uint32 nGroups = (uint32)gmodel.size();
			matGroupWorld.Create(nGroups);
			matGroupSelf.Create(nGroups);
			isInit = true;
		}

		//>> Сообщает текущее количество статических копий
		uint32 StaticMatGetCount()
		{
			return (uint32)matStaticWorld.size();
		}

		//>> Добавляет (резервирует) <count> мест
		void StaticMatAdd(uint32 count)
		{
			matStaticWorld.Add(count);
		}

		//>> Увеличивает (резервирует) до <max> мест
		void StaticMatSet(uint32 max)
		{
			matStaticWorld.Set(max);
		}

		//>> Удаляет всё и заново создаёт (резервирует) <count> мест
		void StaticMatReset(uint32 count)
		{
			matStaticWorld.New(count);
		}

		//>> Удаляет всё
		void StaticMatErase()
		{
			matStaticWorld.Delete(1);
		}

		//>> Удаляет матрицу <id> из массива, позиции >id смещаются на -1
		bool StaticMatErase(uint32 id)
		{
			if (id >= (uint32)matStaticWorld.size()) return false;
			matStaticWorld.Delete(1, id, id + 1);    return true;
		}

		//>> Стирает матрицу <id>
		bool StaticMatDelete(uint32 id)
		{
			if (id >= (uint32)matStaticWorld.size()) return false;
			matStaticWorld.Delete(0, id, id + 1);    return true;
		}

		//>> Создаёт матрицу <id>, если <replace==true> стирает существующую и заменяет новой
		bool StaticMatCreate(uint32 id, WPOS * pos, bool replace)
		{
			if (id >= (uint32)matStaticWorld.size()) return false;

			if (replace) StaticMatDelete(id);
			else if (matStaticWorld[id] != nullptr) return false;

		//	if (!pos->A) matStaticWorld[id] = new MATH3DMATRIX (MathWorldMatrix(pos->P, pos->Q, pos->S));
		//	else         matStaticWorld[id] = new MATH3DMATRIX (MathWorldMatrix(pos->P, pos->Q * MATH3DQUATERNION(pos->A), pos->S));

			auto ptr = new MATH3DMATRIX;
			matStaticWorld.Reset(ptr, id);
			auto Q = pos->Q;
			if ((!pos->A) == false)
			{
				Q *= MATH3DQUATERNION(pos->A);
				Q._normalize_check();
				pos->A._default();
			}

			MathWorldMatrix(pos->P, Q, pos->S, *matStaticWorld[id]);

			return true;
		}

		//>> Добавляет новую матрицу в массив (возвращает её <id>)
		uint32 StaticMatAdd(WPOS * pos)
		{
			uint32 id = (uint32)matStaticWorld.size();
			matStaticWorld.emplace_back();

		//	if (!pos->A) matStaticWorld[id] = new MATH3DMATRIX (MathWorldMatrix(pos->P, pos->Q, pos->S));
		//	else         matStaticWorld[id] = new MATH3DMATRIX (MathWorldMatrix(pos->P, pos->Q * MATH3DQUATERNION(pos->A), pos->S));

			auto ptr = new MATH3DMATRIX;
			matStaticWorld.Reset(ptr, id);
			auto Q = pos->Q;
			if ((!pos->A) == false)
			{
				Q *= MATH3DQUATERNION(pos->A);
				Q._normalize_check();
				pos->A._default();
			}

			MathWorldMatrix(pos->P, Q, pos->S, *matStaticWorld[id]);

			return id;
		}

		//>> Задаёт указатель на внешнюю матрицу (исп. вместо динамической и статической)
		void ExternalMoveMat(MATH3DMATRIX * matrix)
		{
			matExternalWorld = matrix;
		}

		//>> Удаляет указатель на внешнюю матрицу (исп. динамичесую и статическую)
		void ExternalRemoveMat()
		{
			matExternalWorld = nullptr;
		}

		//>> Подготовка мировых матриц
		void PrepareMatrices(uint32 idSMC)
		{
			bool matDynamicInit = false;
			bool isStatic = (_NOMISS(idSMC));
			uint32 nGroups = 1;

			if (!isStatic) nGroups = (uint32)gmodel.size();

			// Логические группы (дверь/колесо авто и др.) могут иметь своё смещение относительно всей модели (только для динамических)
	
			for(uint32 n=0; n<nGroups; n++) 
				if (matExternalWorld != nullptr) // Матрица задана извне
				{
					MODELGROUP * group = gmodel[n];
					if (group->usePos)
					{
						WPOS *                       _gpos = group->pos;
						if (group->expos != nullptr) _gpos = group->expos;

						if (!_gpos->A) {}
						else { _gpos->Q *= MATH3DQUATERNION(_gpos->A);
						       _gpos->Q._normalize_check();
							   _gpos->A._default();           }

						MathWorldMatrix(_gpos->P, _gpos->Q, _gpos->S, *matGroupSelf[n]);

						group->expos = nullptr; // сброс
					}
				}
				else if (!isStatic || matStaticWorld[idSMC] == nullptr) // Динамическая или ещё не рассчитанная статическая
				{
					MODELGROUP * group = gmodel[n];
					if (!isStatic && group->usePos) // Динамическая с использованием группы
					{
						WPOS *                _pos = CObject::pos;
						if (expos != nullptr) _pos = CObject::expos;

						WPOS *                       _gpos = group->pos;
						if (group->expos != nullptr) _gpos = group->expos;

						if (!_pos->A) {}
						else { _pos->Q *= MATH3DQUATERNION(_pos->A);
							   _pos->Q._normalize_check();
							   _pos->A._default();           }

						if (!_gpos->A) {}
						else { _gpos->Q *= MATH3DQUATERNION(_gpos->A);
							   _gpos->Q._normalize_check();
							   _gpos->A._default();          }

						auto P = _pos->P;	P += _gpos->P;
						auto Q = _pos->Q;	Q *= _gpos->Q;
						auto S = _pos->S;	S *= _gpos->S;

						MathWorldMatrix( P, Q, S, *matGroupWorld[n] );

						group->expos = nullptr; // сброс
					}
					else if ( (!isStatic && !matDynamicInit) ||                  // Не рассчитанная динамическая без группы 
						      ( isStatic &&  matStaticWorld[idSMC] == nullptr) ) // или не рассчитанная статическая
					{
						WPOS *                _pos = CObject::pos;
						if (expos != nullptr) _pos = CObject::expos;

						if (!_pos->A) { }
						else { _pos->Q *= MATH3DQUATERNION(_pos->A); // Q = Q * ((Qx * Qz) * Qy)
							   _pos->Q._normalize_check();					  
							   _pos->A._default();		   }

						if (!isStatic)																{
							MathWorldMatrix(_pos->P, _pos->Q, _pos->S, *matDynamicWorld);
							matDynamicInit  = true;													}
						else																		{
							auto ptr = new MATH3DMATRIX;
							matStaticWorld.Reset(ptr, idSMC);
							MathWorldMatrix(_pos->P, _pos->Q, _pos->S, *matStaticWorld[idSMC]);		}
					}	
				}

			expos = nullptr; // сброс
		}

	/*	//>> OLD: Подготовка мировых матриц (без кватернионов. без указателей)
		void PrepareMatrices_old(const uint32& idSMC)
		{
			bool matDynamicInit = FALSE;

			// Логические группы (дверь авто, колесо авто и т.д.) могут иметь своё смещение относительно всей модели

			for(uint32 n=0; n<nGroups; n++) 
				if (matExternalUse) // Матрица задаётся извне
				{
					if (gmodel[n].usePos)
						matGroupSelf[n]  = MathWorldMatrix( gmodel[n].pos.P,
															gmodel[n].pos.A,
															gmodel[n].pos.S );
				}
				else if ( !matStaticInit[idSMC] || !isStatic ) // Динамическая или ещё не рассчитанная статическая
				{
					if (!isStatic && gmodel[n].usePos) // Динамическая с использованием группы
					{
						matGroupWorld[n] = MathWorldMatrix( pos.P + gmodel[n].pos.P,
															pos.A + gmodel[n].pos.A,
															pos.S * gmodel[n].pos.S );
					}
					else if (!matDynamicInit || !matStaticInit[idSMC]) // Не рассчитанная динамическая или статическая
					{
						MATH3DMATRIX mtemp = MathWorldMatrix(pos.P, pos.A, pos.S);

						if (!matDynamicInit)		{	matDynamicWorld = mtemp;
														matDynamicInit	= TRUE;				}
						if (!matStaticInit[idSMC])	{	matStaticWorld[idSMC] = mtemp;
														matStaticInit[idSMC]  = TRUE;		}
					}	
				}
		}
		*/
	};
}

#endif // _MODELPRE_H
