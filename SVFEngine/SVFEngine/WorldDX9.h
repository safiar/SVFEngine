// ----------------------------------------------------------------------- //
//
// MODULE  : WorldDX9.h
//
// PURPOSE : Загрузка и содержание объектов игры (DX9)
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _WORLDDX9_H
#define _WORLDDX9_H

#include "World.h"
#include "InputAnswers.h"

using namespace SAVFGAME;

namespace SAVFGAME
{
	class CWorldDX9 : public CWorld
	{
	protected:
	private:
	public:
		CWorldDX9() : CWorld() { };
		~CWorldDX9() { Close(); };

		void CWorldDX9::Close()
		{
			CWorld::Close();
		}

	/*	//>> TEST: Временный загрузчик
		void CWorldDX9::LoadObjects()
		{
			if (!d3ddev) { _MBM(ERROR_DeviceNone); return; }

			CWorld::LoadObjects();

			//InitTestDX9light();

			loading_thread.Finish(); // теперь поток вернётся в CWorld::DrawLoadingScreen()
		}//*/

/*		//>> Запуск первичной загрузки
		void CWorldDX9::RunInitLoading(int32 width, int32 height, uint32 client_width, uint32 client_height, uint32 bx, uint32 by)
		{
			if (!d3ddev) { _MBM(ERROR_DeviceNone); return; }

			LoadObjectsStart();
			UpdateWindowReferences(width, height, client_width, client_height, bx, by);
			LoadObjectsStart_UpdateScreenFromSettings();

			thread * TLS = new thread; // поток будет возвращён в CWorld::DrawLoadingScreen()
			loading_thread.Add(TLS, true);
			*TLS = thread(&CWorld::LoadObjectsLoadingThread, this);
			loading_screen = true;
		} //*/

			//>> TEST: Сцена на которой можно вращать/перемещать объекты
		/*	void CWorldDX9::DrawTestPick()
			{
			static vector<OBJDATA> obj; // здесь объекты сцены
			static uint32 obj_num;
			static uint32 pickedID = MISSING;
			static bool lmouse = FALSE;
			static bool rmouse = FALSE;
			static const FLOAT maxPickDist = 50.0f;
			static eCameraMode camMode = camera.GetCameraMode();
			static int mode = 0;

			static bool once = TRUE; // задаём объекты 1 раз
			if (once)
			{
			obj.push_back(OBJDATA(1,WPOS(0,0,-14,45,45,45,2,2,2))); // box01
			obj.push_back(OBJDATA(1,WPOS(0,3,7,0,0,0,1,1,1)));      // box02
			obj.push_back(OBJDATA(1,WPOS(0,-21,0,45,0,0,1,1,1)));   // box03
			//	obj.push_back(OBJDATA(0,WPOS(0,10,0,-90,0,90,1,1,1)));  // car01
			//	obj.push_back(OBJDATA(0,WPOS(0,-10,0,-90,0,90,1,1,1))); // car02
			//	obj.push_back(OBJDATA(0,WPOS(10,0,0,-90,0,90,1,1,1)));  // car03
			//	obj.push_back(OBJDATA(0,WPOS(-10,0,0,-90,0,90,1,1,1))); // car04
			//	obj.push_back(OBJDATA(0,WPOS(0,0,10,-90,0,90,1,1,1)));  // car05
			//	obj.push_back(OBJDATA(0,WPOS(0,0,-10,-90,0,90,1,1,1))); // car06
			obj.push_back(OBJDATA(0,WPOS(0,0,0,-90,0,90,1,1,1)));   // car07
			obj_num = (uint32)obj.size();
			once = FALSE;
			}

			if (lock_game_actions) goto SKIP_ALL_AND_ONLY_DRAW_OBJECTS;

			bool lmouse_cur = UI.GetLeftMouse();
			bool rmouse_cur = UI.GetRightMouse();

			if ( lmouse_cur != lmouse || // состояние MOUSE изменилось ?
			rmouse_cur != rmouse )
			{
			//printf("\nLM %i RM %i", (int)lmouse_cur, (int)rmouse_cur);

			if ( (lmouse_cur || rmouse_cur) // кнопка MOUSE нажата
			&& mode==0 ) // и режим не выбран
			{
			uint32 i, num;
			MATH3DMATRIX & projMat = *camera.GetProjMatrixPtr();
			MATH3DMATRIX & viewMat = *camera.GetViewMatrixPtr();
			MATH3DVEC & player_pos = *camera.GetCameraPosPtr();

			// Cначала проверяем удалённость объектов от игрока (<= maxPickDist)
			// Затем входящие в радиус захвата сортируем от ближнего N[0] к дальнему N[num-1]
			auto *dist = new DATAID<FLOAT> [obj_num];
			for(i=0, num=0; i<obj_num; i++)										{
			FLOAT fdist = abs(MathLenVec(player_pos - obj[i].obj.pos->P));
			if (fdist <= maxPickDist)	{
			dist[num].data = fdist;
			dist[num++].ID = i;		}									}
			uint32* N = SortID <DATAID<FLOAT>> (dist,num,true);

			// Проверяем захват в порядке удалённости объекта от игрока
			for(i=0; i<num; i++)
			if (TestPick(model[obj[N[i]].ID], *obj[N[i]].obj.pos, projMat, viewMat))
			{
			pickedID = N[i];
			//	for(int x=0; x<num; x++)
			//		printf("\nN[%i]: %i", x, N[x]);
			//	printf("\nPICKED: %i\n", N[i]);
			break;
			}

			delete[] dist;
			delete[] N;
			}
			else if ( (mode==1 && !lmouse_cur) || // кнопка MOUSE отпущена и соотв. активному режиму
			(mode==2 && !rmouse_cur) )
			{
			camera.SetCameraMode(camMode); // set mem cam mode
			pickedID = MISSING;
			mode = 0;
			}

			if ( (lmouse_cur || rmouse_cur) && // кнопка MOUSE нажата && объект успешно выбран --> выбор режима : 2 RM, 1 LM
			(mode == 0) &&
			(pickedID != MISSING) )
			{
			camMode = camera.GetCameraMode(); // get mem cam mode
			camera.SetCameraMode(CM_NOCONTROL);

			if (lmouse_cur && rmouse_cur)	{ // Обе зажаты сейчас
			if (lmouse == lmouse_cur)
			mode = 2;
			else mode = 1;				}
			else							{ // Зажата только одна
			if (lmouse_cur) mode = 1;
			else            mode = 2;	}

			//if (mode == 1) sound.PlayThread(0, false, false);
			//if (mode == 2) sound.PlayThread(1, false, false);
			sound.PlayThread(3, true, false);
			}

			lmouse = lmouse_cur;
			rmouse = rmouse_cur;	//printf("\nLM %i RM %i", lmouse, rmouse);
			}

			if (pickedID != MISSING) // MOUSE нажата и объект выбран --> управление объектом
			{
			//int32 dx = UI.GetCursorX() - W/2; // Поскольку курсор центрируется
			//int32 dy = UI.GetCursorY() - H/2; //

			int32 dx = wwrm.cursor_x - wwrm.center_x; // Поскольку курсор центрируется
			int32 dy = wwrm.cursor_y - wwrm.center_y; //

			if (dx||dy) switch (mode)
			{
			case 1: TestPickRotate(dx, dy, *obj[pickedID].obj.pos); break;
			case 2: TestPickMove(dx, dy, *obj[pickedID].obj.pos); break;
			}
			}

			SKIP_ALL_AND_ONLY_DRAW_OBJECTS:

			for(uint32 i=0; i<obj_num; i++)					{
			model[obj[i].ID].SetExPos(obj[i].obj.pos);
			model[obj[i].ID].ShowModel();				}

			// test static box
			static bool test_once = true;
			if (test_once)
			{
			WPOS * sboxpos = new WPOS(10, 10, 10, 0, 0, 0, 0.5f, 0.5f, 0.5f);
			model[obj[1].ID].StaticAddMat(*sboxpos);
			delete sboxpos;
			test_once = false;
			}
			model[obj[1].ID].ShowModelStatic(0);
			} //*/

		//>> TEST: Сцена с зеркалом
		/*	void CWorldDX9::DrawTestMirror()
			{
			CModelT & model_box = model[1];
			CModelT & model_mirror = model[2];

			MATH3DVEC p(0,0,14);   // позиция ящиков
			MATH3DVEC m(0,0,7);    // позиция зеркала
			MATH3DVEC a(30,30,0);  // поворот зеркала

			//	model_box.SetModelStatic(false);
			model_box.SetPosInstant(WPOS(p.x, p.y, p.z, 0,0,0,1,1,1));
			model_box.ShowModel();
			model_box.SetPosInstant(WPOS(p.x, p.y + 2, -7, 0, 0, 0, 1, 1, 1));
			model_box.ShowModel();

			d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			model_mirror.SetPosInstant(WPOS(m.x, m.y, m.z, a.x, a.y, a.z, 1, 1, 1));
			model_mirror.ShowModel();
			d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

			model_box.SetPosInstant(WPOS(p.x + 6, p.y - 1, -7, 0, 0, 0, 1, 1, 1));	// Ещё ящики
			model_box.ShowModel();
			model_box.SetPosInstant(WPOS(p.x + 6, p.y - 6, p.z, 0, 0, 0, 1, 1, 1));	// у этого отражение тоже
			model_box.ShowModel();

			////////////////// Рисуем зеркало только в трафарет, буфер без изменений //////////////////////

			d3ddev->SetRenderState(D3DRS_STENCILENABLE,		TRUE);
			d3ddev->SetRenderState(D3DRS_STENCILFUNC,		D3DCMP_ALWAYS);			// always pass
			d3ddev->SetRenderState(D3DRS_STENCILREF,		0x1);					// эталонное значение
			d3ddev->SetRenderState(D3DRS_STENCILMASK,		0xffffffff);
			d3ddev->SetRenderState(D3DRS_STENCILWRITEMASK,	0xffffffff);
			d3ddev->SetRenderState(D3DRS_STENCILZFAIL,		D3DSTENCILOP_KEEP);
			d3ddev->SetRenderState(D3DRS_STENCILFAIL,		D3DSTENCILOP_KEEP);
			d3ddev->SetRenderState(D3DRS_STENCILPASS,		D3DSTENCILOP_REPLACE);	// записывается эталонное значение

			d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);		// просчёт глубины убран, т.к. сейчас мы заняты трафаретом
			d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);	// кадр не меняю, только трафарет  Result = Src*(0,0,0,0) + Dest*(1,1,1,1) = Dest

			d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);			// рисую в трафарет отражающую сторону зеркала
			model_mirror.SetPosInstant(WPOS(m.x, m.y, m.z, a.x, a.y, a.z, 1, 1, 1));
			model_mirror.ShowModel();
			d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

			d3ddev->SetRenderState(D3DRS_ZWRITEENABLE,	TRUE);		// просчёт глубины включен

			///////////////////////////////////////////////////////////////////////////////////////////////

			///////////////////////////////// Рисуем отражение коробки ////////////////////////////////////

			d3ddev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);			// (ref & mask) == (value & mask)
			d3ddev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);		// (0x1 & 0xffffffff) == (value & 0xffffffff)
			// (0x1) == (value & 0xffffffff)
			MATH3DMATRIX  mirror_plane_rotation_matrix = MathRotateMatrix(a);
			MATH3DVEC     mirror_plane_normal = MathVecTransformNormal(MATH3DVEC(0,0,1), mirror_plane_rotation_matrix);
			MATH3DPLANE   mirror_plane = MATH3DPLANE(m, MathNormalizeVec(mirror_plane_normal));

			MATH3DMATRIX  matReflect = MathReflectMatrix(mirror_plane);

			// Очистим z-буфер, чтобы зеркало не загораживало отражение
			// Если очищать здесь Z-буфер, все последующие объекты, нарисованные на сцене,
			// будут видны сквозь ранее нарисованные объекты, поэтому следует рисовать
			// отражения последними

			d3ddev->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,0,0,0), 1.0f, 0);

			// Смешиваем зеркало и отражение

			//d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			//d3ddev->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_DESTCOLOR);	// Result = Src * Dest + Dest * (0,0,0,0) = Src * Dest
			//d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
			d3ddev->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);		// Result = Src * 1 + Dest * 1 = Src + Dest
			d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			//d3ddev->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);		// Result = Src
			//d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

			//d3ddev->SetClipPlane(0, FLOATCAST(&mirror_plane));
			//d3ddev->SetRenderState(D3DRS_CLIPPLANEENABLE, TRUE);

			eShaderID cur_vs = shader.GetCurVS();
			eShaderID cur_ps = shader.GetCurPS();

			if (cur_vs == SHADER_NONE)
			{
			d3ddev->SetTransform(D3DTS_VIEW, matReflect * (*camera.GetViewMatrixPtr()));
			}
			else
			{
			SHADER_HANDLE mat_viewproj = shader.exdata[cur_vs].handle[CONST_MAT_VIEWPROJ];

			MATH3DMATRIX matViewProj = matReflect * (*camera.GetViewProjMatrixPtr());
			//MATH3DMATRIX matViewProj = matReflect * camera.GetViewMatrix() *  camera.GetProjMatrix();
			shader.SetMatrix(cur_vs, mat_viewproj, matViewProj);
			}

			if (cur_ps != SHADER_NONE)
			{
			SHADER_HANDLE vec_camera = shader.exdata[cur_ps].handle[CONST_VEC_CAMERA];
			MATH3DVEC vecCamera = (*camera.GetCameraPosPtr()) * matReflect;
			shader.SetFloatArray(cur_ps, vec_camera, vecCamera, 3);
			}

			d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			model_box.SetPosInstant(WPOS(p.x, p.y, p.z, 0, 0, 0, 1, 1, 1));
			model_box.ShowModel();
			model_box.SetPosInstant(WPOS(p.x + 6, p.y - 6, p.z, 0, 0, 0, 1, 1, 1));
			model_box.ShowModel();
			d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

			// Возвращаем нормальные настройки
			d3ddev->SetRenderState(D3DRS_STENCILENABLE,	FALSE);
			d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			d3ddev->SetRenderState(D3DRS_CLIPPLANEENABLE, FALSE);
			} //*/

	};
}

#endif // _WORLDDX9_H