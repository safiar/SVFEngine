// ----------------------------------------------------------------------- //
//
// MODULE  : Interact.h
//
// PURPOSE : Взаимодествие игрока со сценой
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _INTERACT_H
#define _INTERACT_H

#include "stdafx.h"
#include "StatusIO.h"
#include "ModelRe.h"
#include "GameMap.h"

using namespace SAVFGAME;

namespace SAVFGAME
{
	enum eInteractMode
	{
		INTMODE_NONE,			// passive mode
		INTMODE_NO_CHANGES,		// no changes from last call
		INTMODE_IN_VIEW,		// view object, but don't interact with it
		INTMODE_ROTATE,			// rotating objects mode
		INTMODE_TRANSLATE,		// moving objects mode

		INTMODE_ENUM_MAX
	};

	class CBaseInteract
	{
		struct InterpolationMemory
		{
			friend class CBaseInteract;
		protected:
			InterpolationMemory()
			{
				Close();
			}
			~InterpolationMemory(){};
			void Close()
			{
				move_maked._default();
				move._default();
				angle_maked._default();
				angle._default();
			}			
			MATH3DVEC2  move_maked;			// [SERVER]        already moved dx/dy
			MATH3DVEC2	move;				// [SERVER/CLIENT] target move dx/dy
			MATH3DVEC2	angle_maked;		// [SERVER]        already rotated ax/ay
			MATH3DVEC2	angle;				// [SERVER/CLIENT] target rotation ax/ay
			float		interpolation_t;	// [SERVER]        interpolation status
		public:
			//>> Server set actual interpolation at every frame
			void SetInterpolation(float t)
			{
				interpolation_t = t;
			}
			//>> Client pop out angle data and send it to server
			void GetAngles(MATH3DVEC2 & out)
			{
				out = angle;
				angle._default();
			}
			//>> Server put in client's angle data
			void SetAngles(const MATH3DVEC2 & in)
			{
				angle = in;
				angle_maked._default();
			}
			//>> Client pop out movement data and send it to server
			void GetMove(MATH3DVEC2 & out)
			{
				out = move;
				move._default();
			}
			//>> Server put in client's movement data
			void SetMove(const MATH3DVEC2 & in)
			{
				move = in;
				move_maked._default();
			}
		};
	protected:
		const CStatusIO *		IO     { nullptr };
		CPlayerCameraF *		camera { nullptr };
		bool					isInit { false };
	public:
		InterpolationMemory MEM;		// memory for interpolation
		eInteractMode		status;		// (additional info) returning status from last call : NO_CHANGES / IN_VIEW / ROTATE / TRANSLATE
	protected:
		eInteractMode		mode;		// current interact mode
		uint64				pickedID;	// current picked object's ID
		uint32				window_W;	// mem window width
		uint32				window_H;	// mem window height
		bool				rotate;		// rotate event call
		bool				move;		// move event call
		bool				rotate_mem;	// rotate event condition memory
		bool				move_mem;	// move event condition memory
	protected:		
		const MATH3DVEC *	  cameraPos;	// данные от камеры
		const MATH3DVEC *	  playerPos;	// данные от камеры (для вида от 3 лица, <posself> == точка модели)
		const MATH3DMATRIX *  projMatPtr;	// данные от камеры
		const MATH3DMATRIX *  viewMatPtr;	// данные от камеры
	protected:
		MATH3DVEC			vecpick;	// вектор захвата
		MATH3DVEC			posself;	// 1ая точка линии взгляда (позиция игрока)
		MATH3DVEC			pospick;	// 2ая точка линии взгляда (позиция захватываемой модели)
		bool				recalcvec;	// нужно ли рассчитывать снова
		bool				nextframe;  // сообщение о новом кадре
	protected:
		bool				server      { false }; // net status
		bool				client      { false }; // net status
		bool				server_self { false }; // self server interact
	public:
		CBaseInteract(const CBaseInteract& src)				= delete;
		CBaseInteract(CBaseInteract&& src)					= delete;
		CBaseInteract& operator=(CBaseInteract&& src)		= delete;
		CBaseInteract& operator=(const CBaseInteract& src)	= delete;
	public:
		CBaseInteract() { Reset(); };
		~CBaseInteract(){};
		void Close()
		{
			Reset();

			IO         = nullptr;
			camera     = nullptr;
			projMatPtr = nullptr;
			viewMatPtr = nullptr;
			cameraPos  = nullptr;
			playerPos  = nullptr;
			isInit     = false;
		};
		void Reset()
		{
			status     = INTMODE_NONE;
			mode       = INTMODE_NONE;
			pickedID   = MISSING;
			recalcvec  = true;
			window_W   = 0;
			window_H   = 0;
			nextframe  = true;
			MEM.Close();
		}
		void TipNextFrame() { nextframe = true; }
		void SetRotate(bool condition) { rotate = condition; }
		void SetMove(bool condition)   { move   = condition; }
		bool GetRotate()               { return rotate; }
		bool GetMove()                 { return move;   }
		eInteractMode GetMode()        { return mode; }

		//>> Сообщает, блокирует ли текущий интерактивный режим управление камерой
		bool IsCameraBlockingMode()
		{
			switch (mode)
			{
			case INTMODE_ROTATE:
			case INTMODE_TRANSLATE:
				return true;
			default:
				return false;
			}
			return false;
		}

		//>> Пометка сервером своей копии класса
		void MarkServerSelf(bool server_self_interact) { server_self = server_self_interact; }
		//>> В режиме сервера
		void SetNetServer(bool status_server) { server = status_server; }
		//>> В режиме клиента, подключенного к серверу
		void SetNetClient(bool status_client) { client = status_client; }

		void Init(const CPlayerCameraF * player_camera, const MATH3DVEC * player_position, const CStatusIO * io_status)
		{
			if (player_camera   == nullptr) { _MBM(ERROR_PointerNone); return; }
			if (player_position == nullptr) { _MBM(ERROR_PointerNone); return; }
			if (io_status       == nullptr) { _MBM(ERROR_PointerNone); return; }

			IO         = io_status;
			camera     = const_cast<CPlayerCameraF*>(player_camera);
			projMatPtr = camera->GetProjMatrix();
			viewMatPtr = camera->GetViewMatrix();
			cameraPos  = camera->GetCameraPos();
			playerPos  = player_position;

			isInit = true;
		}
		
	protected:
		//>> Расчёт вектора захвата
		void PickVecCalc()
		{
			MATH3DVEC cursor;
		//	float det;

			cursor.x = static_cast<float>(IO->mouse.client_x);
			cursor.y = static_cast<float>(IO->mouse.client_y);

		//	MATH3DMATRIX viewMatInv = MathInverseMatrix(*viewMatPtr, det);			if (!det) _MBM(ERROR_MatrixDetNull);
		//	MATH3DMATRIX viewMatInv = MathTransposeMatrix(*viewMatPtr);

			// [-1..+1]
			cursor.x =  (2 * cursor.x / static_cast<float>(window_W)) - 1;
			cursor.y = ((2 * cursor.y / static_cast<float>(window_H)) - 1) * -1;

			//printf("\n%f %f : %i %i", cursor.x, cursor.y, window_W, window_H);

			// учёт пропорций окна просмотра
			cursor.x = cursor.x / projMatPtr->_11; // _11 = w = 1.0/tan(fov_horiz*0.5);
			cursor.y = cursor.y / projMatPtr->_22; // _22 = h = 1.0/tan(fov_vert*0.5);

			// вектор захвата

			//vecpick.x = (cursor.x * viewMatInv._11) + (cursor.y * viewMatInv._21) + viewMatInv._31;
			//vecpick.y = (cursor.x * viewMatInv._12) + (cursor.y * viewMatInv._22) + viewMatInv._32;
			//vecpick.z = (cursor.x * viewMatInv._13) + (cursor.y * viewMatInv._23) + viewMatInv._33;

			vecpick.x = (cursor.x * viewMatPtr->_11) + (cursor.y * viewMatPtr->_12) + viewMatPtr->_13;
			vecpick.y = (cursor.x * viewMatPtr->_21) + (cursor.y * viewMatPtr->_22) + viewMatPtr->_23;
			vecpick.z = (cursor.x * viewMatPtr->_31) + (cursor.y * viewMatPtr->_32) + viewMatPtr->_33;

			posself = *playerPos;
			pospick = vecpick + posself;

			recalcvec = false;
		}
		//>> Проверка взгляда на объект модели
		bool Pick(const CModel * mdl, const WPOS * wPos)
		{
			// Краткое объяснение
			// Каждая модель имеет виртуальный ящик (Bounding Box), в который она заключена
			// Задача - найти пересечения "луча" взгляда с одной из плоскостей этого ящика
			// Я произвожу изменение ящика в соотв. с текущим состоянием модели и пересчитываю новые max/min координаты ящика (в изменённом положении)
			// Составляю луч и ищу, где он пересекается с изменёнными плоскостями ящика
			// Нахожу точку и сравниваю с max/min координатами в изменённом положении
			// Если точка в пределах max/min (изменённых) координат ящика, делаю шаг 2
			// Шаг 2 - вернуть всё обратно и сравнить в исходном положении, это избавит от ошибки выделения "пустых карманов" у ящика (из-за поворотов)
			// Координаты исходного ящика уже известны изначально, надо лишь точку пересечения преобразовать обратной мировой матрицей (worldMatBack)
			// Преобразую точку, проверяю её координаты с default max/min координатами ящика - и готово

			MATH3DMATRIX  worldMat, worldMatBack;
			float det;

			if (recalcvec) PickVecCalc();

			MathWorldMatrix(wPos->P, wPos->Q, wPos->S, worldMat);
		//	MathWorldMatrixBack( wPos->P, wPos->A, wPos->S, worldMatBack );

			// Эта матрица понадобится при обратной конвертации точки пересечения
			MathInverseMatrix(worldMat, det, worldMatBack);								if (!det) return false; //_MBM(ERROR_MatrixDetNull);

			MATH3DVEC pMin, pMax, pBox[8];

			for (int i = 0; i < 8; i++)
				MathVecTransformCoord((*mdl).box.point[i], worldMat, pBox[i]);

			pMin = pMax = pBox[0];

			for (int i=0; i<8; i++) // Новые min/max координаты в повёрнутом положении
			{
					 if (pBox[i].x > pMax.x) pMax.x = pBox[i].x;
				else if (pBox[i].x < pMin.x) pMin.x = pBox[i].x;

					 if (pBox[i].y > pMax.y) pMax.y = pBox[i].y;
				else if (pBox[i].y < pMin.y) pMin.y = pBox[i].y;

					 if (pBox[i].z > pMax.z) pMax.z = pBox[i].z;
				else if (pBox[i].z < pMin.z) pMin.z = pBox[i].z;
			}

			bool result = true;
			bool bresult = false;

			// Проверка, что смотрим вперёд

			MATH3DVEC mpos = wPos->P;	//_VECSET(wPos, mpos);
			MATH3DVEC dist_vec = posself - mpos;
			float fcos = MathDotVec(vecpick, dist_vec);
			if (fcos > 0) result = false;

			MATH3DVEC pOutMem, pOutCur;

			const float precision = 0.1f; // поправка на точность float сравнений

			if (result)
				for (int i=0; i<6; i++)
				{
					bool NON;
					MATH3DVEC pA, pN;

					switch (i) // Проверяем все 6 сторон
					{
					case 0: pA = (*mdl).box.plane_Xp.P;  pN = (*mdl).box.plane_Xp.N; break;
					case 1: pA = (*mdl).box.plane_Xn.P;  pN = (*mdl).box.plane_Xn.N; break;
					case 2: pA = (*mdl).box.plane_Yp.P;  pN = (*mdl).box.plane_Yp.N; break;
					case 3: pA = (*mdl).box.plane_Yn.P;  pN = (*mdl).box.plane_Yn.N; break;
					case 4: pA = (*mdl).box.plane_Zp.P;  pN = (*mdl).box.plane_Zp.N; break;
					case 5: pA = (*mdl).box.plane_Zn.P;  pN = (*mdl).box.plane_Zn.N; break;
					}

					MathVecTransformCoord(worldMat, pA);
					MathVecTransformNormal(worldMat, pN);
					MathRayToPlaneIntersect(pA, pN, posself, pospick, pOutCur, NON);   if (NON) continue;

					if (MathCompareV3_GreaterEqual (pOutCur, pMin, precision) && 
						MathCompareV3_LessEqual    (pOutCur, pMax, precision))   
					//	pOutCur >= pMin && // условие, что точка лежит в пределах 
					//	pOutCur <= pMax)   // World-координат ящика (т.е. повёрнутого)
					{
						result = false;
						MathVecTransformCoord(pOutCur, worldMatBack, mpos);

						if (MathCompareV3_GreaterEqual (mpos, (*mdl).box.minCoord, precision) &&
							MathCompareV3_LessEqual    (mpos, (*mdl).box.maxCoord, precision))
						//	mpos >= (*mdl).box.minCoord && // условие, что точка лежит в пределах Local-координат ящика 
						//	mpos <= (*mdl).box.maxCoord)   // (не повёрнутого, fix ошибки вращения)
							result = true;

						if (result)
						{
							if (!bresult) { pOutMem = pOutCur; bresult = true; }
							else
							{
								float lenMem = MathLenVec(posself - pOutMem);
								float lenCur = MathLenVec(posself - pOutCur);

								if (lenMem > lenCur) pOutMem = pOutCur;		// выбираем ближайшую точку
							}
						}
					}
				};

		/*	//vecpick -= posself; // campos

			static bool bl = 0;
			if (bresult == FALSE)
			{
				if (bl)  {
					printf("\n\nNO");
					printf("\nCAM %f %f %f\nVEC %f %f %f",
						posself.x, posself.y, posself.z,   vecpick.x, vecpick.y, vecpick.z);
					printf("\n--- %f %f %f", pOutCur.x, pOutCur.y, pOutCur.z);
					bl=0;
				};
			}
			else
			{
				if (!bl) {
					printf("\n\nYES %f %f %f\nMIN %f %f %f\nMAX %f %f %f",
						pOutMem.x, pOutMem.y, pOutMem.z,  pMin.x, pMin.y, pMin.z,   pMax.x, pMax.y, pMax.z);
					printf("\nCAM %f %f %f\nVEC %f %f %f",
						posself.x, posself.y, posself.z,   vecpick.x, vecpick.y, vecpick.z);
					bl=1;
				};
			}; //*/

			return bresult;
		}
		//>> Выбирает тип управления
		void PickChooser(CGameMap * map)
		{
			if (_NOMISS(pickedID)) // кнопка нажата и объект был выбран --> управление объектом
			{
				int32 dx = camera->GetMouseDX();
				int32 dy = camera->GetMouseDY();	//printf("\ndx %3i dy %3i", dx, dy);

				WPOS * wpos = map->obj.model[pickedID]->pos;

				if ( (dx || dy) ||				// own
					 (server && !server_self) )	// others
				switch (mode)
				{
				case INTMODE_ROTATE:    PickRotate(dx, dy, wpos); break;
				case INTMODE_TRANSLATE: PickMove(dx, dy, wpos);   break;
				}
			}
		}
		//>> Вращение объекта мышкой
		void PickRotate(int32 dx, int32 dy, WPOS * wPos)
		{
			float speed = 1.0f;// *camera->GetCameraTimeFix();
			float angleY = speed * (-dx);
			float angleX = speed * (-dy);

			if (client)
			{
				MEM.angle.x += angleX;
				MEM.angle.y += angleY;
				return;
			}
			else if (server && !server_self)
			{
				angleX = MEM.angle.x * MEM.interpolation_t;	
				angleY = MEM.angle.y * MEM.interpolation_t; 

				angleX -= MEM.angle_maked.x;
				angleY -= MEM.angle_maked.y;

				MEM.angle_maked.x += angleX;
				MEM.angle_maked.y += angleY;

				if (!angleX && !angleY) return;
			}

			MATH3DVEC x_axis(VIEW_OX(viewMatPtr));
			MATH3DVEC y_axis(VIEW_OY(viewMatPtr));

			MATH3DQUATERNION Qx(x_axis, angleX);
			MATH3DQUATERNION Qy(y_axis, angleY);

			wPos->Q *= Qx * Qy;
			wPos->Q._normalize_check();

			//MATH3DVEC angles = wPos.Q._angles();
			//printf("\nangles %f %f %f", angles.ax, angles.ay, angles.az);
		}
		//>> Перемещение объекта мышкой
		void PickMove(int32 dx, int32 dy, WPOS * wPos)
		{
			float speed = 0.1f;
			float dX = speed * (dx);
			float dY = speed * (-dy);

			if (client)
			{
				MEM.move.x += dX;
				MEM.move.y += dY;
				return;
			}
			else if (server && !server_self)
			{
				dX = MEM.move.x * MEM.interpolation_t;
				dY = MEM.move.y * MEM.interpolation_t;

				dX -= MEM.move_maked.x;
				dY -= MEM.move_maked.y;

				MEM.move_maked.x += dX;
				MEM.move_maked.y += dY;

				if (!dX && !dY) return;
			}

			MATH3DVEC Ox(X_AXIS), Oy(Y_AXIS), Oz(Z_AXIS);
			MATH3DVEC Ox_view(VIEW_OX(viewMatPtr));
			MATH3DVEC Oy_view(VIEW_OY(viewMatPtr));

			float cosXX = MathDotVec(Ox_view, Ox);
			float cosXY = MathDotVec(Ox_view, Oy);
			float cosXZ = MathDotVec(Ox_view, Oz);

			float cosYX = MathDotVec(Oy_view, Ox);
			float cosYY = MathDotVec(Oy_view, Oy);
			float cosYZ = MathDotVec(Oy_view, Oz);

			wPos->x += dX * cosXX + dY * cosYX;
			wPos->y += dX * cosXY + dY * cosYY;
			wPos->z += dX * cosXZ + dY * cosYZ;
		}
		//>> Обновление связанных данных
		void CheckRecalcVec()
		{
			if (IO == nullptr) { _MBM(ERROR_PointerNone);  return; }

			if (nextframe)
			{
				recalcvec = true;
				nextframe = false;

				window_W = IO->window.client_width;
				window_H = IO->window.client_height;
			}

		/*	if ( camera->GetMouseDX() ||
				 camera->GetMouseDY() )
			{
				recalcvec = true; return;
			}

			if ( posself != *playerPos )
			{
				recalcvec = true; return;
			}

			if ( window_W != IO->window.client_width  ||
				 window_H != IO->window.client_height )
			{
				window_W = IO->window.client_width;
				window_H = IO->window.client_height;

				recalcvec = true; return;
			} //*/
		}
		//>> Управление объектами сцены :: поиск pickedID
		uint64 PickProc(CGameMap * map)
		{
			uint64 ret = MISSING;
			uint64 i, pickable_count = 0;

			// Подсчитаем сколько в данный момент объектов доступны и интерактивны //

			for (auto & o : map->obj.model)
				if (o->dynamic && o->enable && o->pick)
					pickable_count++;

			// Проверим удалённость объектов от игрока //

			vector<pair<float, uint64>> distance;	// std::map <float, uint64>
			distance.reserve(pickable_count);

			i = 0;
			for (auto & o : map->obj.model)											{
				if (o->dynamic && o->enable && o->pick)							{
					float dist = o->GetDistTo(playerPos);
					if (dist <= map->setup.maxpickdist)
						distance.emplace_back(pair<float, uint64>(dist, i));	}
				i++;																}

			// Отсортируем входящие в радиус захвата от ближнего к дальнему //

			std::sort(distance.begin(), distance.end(), std::less_equal<>());

			// Проверяем захват в порядке удалённости объекта от игрока //

			for (auto & cur : distance) 
			{
				const WPOS   * wpos  = map->obj.model[cur.second]->pos;
				const CModel * model = map->obj.model[cur.second]->data;
				
				if (Pick(model, wpos))	{
					ret = cur.second;
					break;				}
			}

			return ret;
		}
		
	public:		
		//>> Управление объектами сцены
		eInteractMode Pick(const CGameMap * gamemap, bool lock_game_actions)
		{
			if (!isInit)            { _MBM(ERROR_InitNone);    return status = INTMODE_NONE; }
			if (gamemap == nullptr) { _MBM(ERROR_PointerNone); return status = INTMODE_NONE; }

			if (lock_game_actions)  // player at escape menu / chat / ...
			{
				pickedID = MISSING;
				mode = INTMODE_NONE;
				return status = INTMODE_NONE;
			}

			auto map = const_cast<CGameMap*>(gamemap);
			status = mode;  // eInteractMode ret = mode;

			if (!map->setup.pickmode) return status = INTMODE_NONE; // if disallowed by map settings

			CheckRecalcVec();

			bool status_in_view = true;

			if ( (rotate == rotate_mem && mode == INTMODE_ROTATE)   ||
				 (move   == move_mem   && mode == INTMODE_TRANSLATE) ) { }
			else
			{
				uint64 pickedID_cur = PickProc(map);
				status_in_view = (_ISMISS(pickedID_cur)) ? false : true;
			
				if (rotate != rotate_mem || move != move_mem) // состояние изменилось ?
				{
					if ( (rotate || move) && (mode == INTMODE_NONE) ) // режим не выбран
					{
						pickedID = pickedID_cur;
					}
					else if ((mode == INTMODE_ROTATE    && !rotate) || // кнопка отпущена и режим активен
							 (mode == INTMODE_TRANSLATE && !move))
					{
						camera->RestoreCameraMode();
						pickedID = MISSING;
						mode = INTMODE_NONE;
					}

					if ((rotate || move) &&			// кнопка нажата 
						(mode == INTMODE_NONE) &&	// при пассивном режиме
						(_NOMISS(pickedID)))		// и объект успешно был выбран --> выбор активного режима
					{
						camera->SetCameraMode(CM_NOCONTROL);

						if (rotate && move)				{ // Обе кнопки зажаты
							if (rotate_mem == rotate)
								 mode = INTMODE_TRANSLATE;
							else mode = INTMODE_ROTATE;
						}
						else							{ // Зажата только одна
							if (rotate)
								 mode = INTMODE_ROTATE;
							else mode = INTMODE_TRANSLATE;
						}
					}

					rotate_mem = rotate;
					move_mem   = move;
				}
			}

			PickChooser(map);

			if (mode == INTMODE_ROTATE || mode == INTMODE_TRANSLATE)
			{ 
				if (status == mode) status = INTMODE_NO_CHANGES;
				else                status = mode;
			}
			else // mode == INTMODE_NONE
			{
				if (status_in_view == true) status = INTMODE_IN_VIEW;
			}

			return status;
		}
		//>> Проверка, что объект виден
		bool CheckVisibility(const WPOS * model)
		{
			if (recalcvec) PickVecCalc();

			float fcos = MathDotVec(vecpick, *cameraPos - model->P);
			if (fcos > 0) return false;
			else return true;
		}
	};

}

#endif // _INTERACT_H
