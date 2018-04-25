// ----------------------------------------------------------------------- //
//
// MODULE  : Camera.h
//
// PURPOSE : Состояние камеры, её общие характеристики
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _CAMERA_H
#define _CAMERA_H

#include "stdafx.h"
#include "device3d.h"
#include "StatusIO.h"

#define  FPS_CONSTANT  1.f / 17

#define  CPlayerCameraF  CBaseCamera // class final

#define  VIEW_OX(M)  M->_11, M->_21, M->_31
#define  VIEW_OY(M)  M->_12, M->_22, M->_32
#define  VIEW_OZ(M)  M->_13, M->_23, M->_33

#define  _VIEW_OX(M)  M._11, M._21, M._31
#define  _VIEW_OY(M)  M._12, M._22, M._32
#define  _VIEW_OZ(M)  M._13, M._23, M._33

#define  DF_VEC   OZ_VEC	// default view vector
#define  DF_AXIS  Z_AXIS	// default view axis

#define  DEFAULT_CAMERA_MODE  CM_1STPERSON

namespace SAVFGAME
{
	//>> Сигналы передвижения игрока
	enum eCameraSignal
	{
		PLUS  = +1,
		ZERO  =  0,
		MINUS = -1
	};
	//>> Режимы обзора игрока
	enum eCameraMode
	{
		CM_NOCONTROL = 0,		// статичная или управляемая скриптом камера

		CM_1STPERSON = 1,		// прогулочная камера (от 1 лица)
		CM_3RDPERSON = 3,		// прогулочная камера (от 3 лица)

		CM_FLY_SIMULATOR_1ST,	// TODO: симулятор воздушных полётов (от 1 лица)
		CM_FLY_SIMULATOR_3RD,	// TODO: симулятор воздушных полётов (от 3 лица)

		CM_TYPE0, // Заготовки для других типов
		CM_TYPE1,
		CM_TYPE2,
		CM_TYPE3,
		CM_TYPE4,
		CM_TYPE5,
		CM_TYPE6,
		CM_TYPE7,
		CM_TYPE8,
		CM_TYPE9,

		CM_ENUM_MAX
	};
	
	class CBaseCamera
	{
	private:
		struct
		{
			MATH3DMATRIX	mView;
			MATH3DMATRIX	mViewXY;
			MATH3DMATRIX	mProjection;
			MATH3DMATRIX	mViewProj;
		} _ ;
	public:
		//>> Входящие данные от игрока
		struct CameraInputs
		{
			CameraInputs() { Reset(); };
			void Reset()
			{
				mouse_dx = 0;
				mouse_dy = 0;
				move_z = eCameraSignal::ZERO;
				move_x = eCameraSignal::ZERO;
				move_y = eCameraSignal::ZERO;
				roll   = eCameraSignal::ZERO;
			}
			int mouse_dx; // cursor movement
			int mouse_dy; // cursor movement
			eCameraSignal move_z; // signal +1 forward / -1 back / 0 none
			eCameraSignal move_x; // signal +1 right   / -1 left / 0 none
			eCameraSignal move_y; // signal +1 up      / -1 down / 0 none
			eCameraSignal roll;   // signal +1 CW      / -1 CCW  / 0 none
		};
		//>> Текущий статус камеры
		struct CameraSettings
		{
		private:
			struct
			{
				MATH3DVEC			pos;
				MATH3DVEC			lookat;
				MATH3DVEC 			angle;
				MATH3DQUATERNION	rotate;
				MATH3DQUATERNION	rotate_XY;
				MATH3DQUATERNION	rotate_Y;
				MATH3DQUATERNION	rotate_X;
				MATH3DQUATERNION	rotate_Z;
				MATH3DQUATERNION	rotate_ZZ;
				MATH3DQUATERNION	rotate_ZZXY;
			} _ ;
		public:
			friend class CBaseCamera;
			CameraSettings() : mode(CM_1STPERSON), radius(10), radius_speed(1.f), min_radius(1), max_radius(100), mouse_speed(0.5f),
								move_speed(0.2f), roll_speed(0.5f), fly_mode(false), fix_XZ_speed(true), fov(45), fovr(TORADIANS(45)), 
								aspect_ratio(4 / 3), near_plane(1.0f), far_plane(100.0f), update_proj_mat(true), timefix(1), mode_updated(0)
			{
				*lookat      = DF_VEC;
				time         = _TIME;
				prev_mode[0] = mode;
				prev_mode[1] = mode;
			};
			~CameraSettings()
			{
			}
			CameraInputs		input;		
			float				radius;			// CM_3RDPERSON: pos (камеры) = radius + lookat (игрок)
			float				min_radius;		// CM_3RDPERSON: pos (камеры) = radius + lookat (игрок)
			float				max_radius;		// CM_3RDPERSON: pos (камеры) = radius + lookat (игрок)
			float				mouse_speed;	// скорость вращения камеры
			float				move_speed;		// скорость движения камеры (игрока)
			float				roll_speed;		// скорость крена камеры
			float				radius_speed;	// скорость прибавления радиуса (для 3rd person)
			float				fov;			// field of view, в градусах
			float				fovr;			// field of view, в радианах
			float				aspect_ratio;	// W/H: 800/600 = 4/3
			float				near_plane;		// начало плана (конуса) отрисовки
			float				far_plane;		// конец  плана (конуса) отрисовки	
			float               angle_z;		// накапливаемый угол поворота Z			
			bool				fly_mode;		// выключает имитацию ходьбы и включает свободный полёт (для 1st/3rd person mode)
			bool				fix_XZ_speed;	// исправляет потерю скорости при взгляде вверх/вниз Oy в режиме прогулки (fly_mode == false)
		protected:
			timepoint			time;			// время с показа предыдущего кадра
			float				timefix;		// FPS-фикс камеры
		protected:
			MATH3DVEC * const			pos         { &_.pos         };	// позиция камеры
			MATH3DVEC *	const			lookat      { &_.lookat      };	// направление камеры
			MATH3DVEC *	const			angle       { &_.angle       };	// запрос поворота в градусах вокруг Ox (бок) / Oy (верх) / Oz (перед собой)
			MATH3DQUATERNION * const	rotate      { &_.rotate      };	// статус поворота камеры ZXY (комплексное состояние вгляда)
			MATH3DQUATERNION * const	rotate_XY   { &_.rotate_XY   };	// статус поворота камеры XY
			MATH3DQUATERNION * const	rotate_Y    { &_.rotate_Y    };	// статус поворота камеры Y
			MATH3DQUATERNION * const	rotate_X    { &_.rotate_X    };	// статус поворота камеры X
			MATH3DQUATERNION * const	rotate_Z    { &_.rotate_Z    };	// статус поворота камеры Z  (поворот OZ_VIEW локального вида)
			MATH3DQUATERNION * const	rotate_ZZ   { &_.rotate_ZZ   };	// статус поворота камеры ZZ (поворот OZ_VEC  исходного положения)
			MATH3DQUATERNION * const	rotate_ZZXY { &_.rotate_ZZXY };	// статус поворота камеры ZZXY
		protected:
			eCameraMode			mode;			 // текущий режим камеры
			eCameraMode			prev_mode[2];	 // [0] до предыдущего -> [1] предыдущий -> [mode] текущий режим
			bool				mode_updated;	 // метка, что режим поменялся с последнего раза
			bool				update_proj_mat; // метка для обновления projection matrix
		public:
			CameraSettings(const CameraSettings& src)				= delete;
			CameraSettings(CameraSettings&& src)					= delete;
			CameraSettings& operator=(CameraSettings&& src)			= delete;
			CameraSettings& operator=(const CameraSettings& src)	= delete;
		};
	protected:
		DEV3DBASE *				dev		{ nullptr };
		CStatusIO *				IO	 	{ nullptr };
	protected:
		CameraSettings			cs;                             // настройки камеры
		MATH3DMATRIX * const	mView       { &_.mView       }; // матрица вида XYZ
		MATH3DMATRIX * const	mViewXY     { &_.mViewXY     }; // матрица вида XY
		MATH3DMATRIX * const	mProjection { &_.mProjection }; // матрица проекции
		MATH3DMATRIX * const	mViewProj   { &_.mViewProj   }; // матрица вида-проекции XYZ
		bool					skip_update { 1 };
	protected:
		bool					server      { false }; // net status
		bool					client      { false }; // net status
		bool					server_self { false }; // self server camera
	private:
		const float VERTICAL_LOCK = 0.99f; // max (+/-)~89 degrees	
	public:
		CBaseCamera(const CBaseCamera& src)				= delete;
		CBaseCamera(CBaseCamera&& src)					= delete;
		CBaseCamera& operator=(CBaseCamera&& src)		= delete;
		CBaseCamera& operator=(const CBaseCamera& src)	= delete;
	public:
		CBaseCamera()  { };
		~CBaseCamera() { Close(); };
		void Close()
		{
			dev    = nullptr;
			IO     = nullptr;		
		}

		//>> Пометка сервером своей камеры
		void MarkServerSelf(bool server_self_camera) { server_self = server_self_camera; }
		//>> Камера в режиме сервера
		void SetNetServer(bool status_server) { server = status_server; }
		//>> Камера в режиме клиента, подключенного к серверу
		void SetNetClient(bool status_client) { client = status_client; }

		void SetMouseSpeed(float speed)            { cs.mouse_speed    = speed;   }
		void SetMoveSpeed(float speed)             { cs.move_speed     = speed;   }
		void SetRollSpeed(float speed)             { cs.roll_speed     = speed;   }
		void SetRadiusSpeed(float speed)           { cs.radius_speed   = speed;   }
		void SetMoveInputForward(eCameraSignal dz) { cs.input.move_z   = dz;      }
		void SetMoveInputRight(eCameraSignal dx)   { cs.input.move_x   = dx;      }
		void SetMoveInputUp(eCameraSignal dy)      { cs.input.move_y   = dy;      }
		void SetRollInput(eCameraSignal roll)      { cs.input.roll     = roll;    }
		void SetMouseDX(int32 dx)                  { cs.input.mouse_dx = dx;      }
		void SetMouseDY(int32 dy)                  { cs.input.mouse_dy = dy;      }
		void SetMinCameraRadius(float min_r)       { cs.min_radius     = abs(min_r); }
		void SetMaxCameraRadius(float max_r)       { cs.max_radius     = abs(max_r); }
		void SetAspectRatio(float ratio)           { cs.aspect_ratio   = abs(ratio);                                cs.update_proj_mat = true; }
		void SetFOV(float degrees)                 { cs.fov            = degrees;    cs.fovr = TORADIANS(degrees);  cs.update_proj_mat = true; }
		void SetFOVR(float radians)                { cs.fovr           = radians;    cs.fov  = TODEGREES(radians);  cs.update_proj_mat = true; }
		void SetNearPlane(float nplane)            { cs.near_plane     = abs(nplane);                               cs.update_proj_mat = true; }
		void SetFarPlane(float fplane)             { cs.far_plane      = abs(fplane);                               cs.update_proj_mat = true; }
		void SetFlyMode(bool ON)                   { cs.fly_mode       = ON;      }
		void SetXZMoveFix(bool ON)                 { cs.fix_XZ_speed   = ON;      }
		void SwitchFlyMode()                       { cs.fly_mode      ^= 1;       }
		void SwitchXZMoveFix()                     { cs.fix_XZ_speed  ^= 1;       }		
		void ResetCamera(const MATH3DVEC& pos, const MATH3DVEC& angle, bool update)
		{
			auto mem_sys_x = IO->mouse.sys_x;
			auto mem_sys_y = IO->mouse.sys_y;
			auto mem_cli_x = IO->mouse.client_x;
			auto mem_cli_y = IO->mouse.client_y;

			IO->mouse.sys_x = max(0, IO->window.center_x);					// избавляет от лишнего сдвига камеры
			IO->mouse.sys_y = max(0, IO->window.center_y);					// .
			IO->mouse.client_x = IO->mouse.sys_x - IO->window.border_left;	// .
			IO->mouse.client_y = IO->mouse.sys_y - IO->window.border_top;	// .

			SetCameraMode(DEFAULT_CAMERA_MODE);
			ResetCamera(true);

			*cs.pos   = pos;
			*cs.angle = angle;
			cs.angle_z = 0;
			cs.rotate_Z->_default();
			cs.rotate->_default();

			if (update) Update(0);

			IO->mouse.sys_x = mem_sys_x;
			IO->mouse.sys_y = mem_sys_y;
			IO->mouse.client_x = mem_cli_x;
			IO->mouse.client_y = mem_cli_y;
		}
		void ResetCamera(bool update)
		{
			auto mem_sys_x = IO->mouse.sys_x;
			auto mem_sys_y = IO->mouse.sys_y;
			auto mem_cli_x = IO->mouse.client_x;
			auto mem_cli_y = IO->mouse.client_y;

			IO->mouse.sys_x = max(0, IO->window.center_x);					// избавляет от лишнего сдвига камеры
			IO->mouse.sys_y = max(0, IO->window.center_y);					// .
			IO->mouse.client_x = IO->mouse.sys_x - IO->window.border_left;	// .
			IO->mouse.client_y = IO->mouse.sys_y - IO->window.border_top;	// .

			SetCameraMode(DEFAULT_CAMERA_MODE);
			Update(0);

			cs.pos->_default();
			cs.angle->_default();
			cs.angle_z = 0;
			cs.rotate_Z->_default();
			cs.rotate->_default();
			if (update) Update(0);

			IO->mouse.sys_x = mem_sys_x;
			IO->mouse.sys_y = mem_sys_y;
			IO->mouse.client_x = mem_cli_x;
			IO->mouse.client_y = mem_cli_y;
		}
		void ResetCameraMovements()
		{
			cs.input.Reset();
		}
		void UpdateCameraTime()
		{
			cs.timefix = FPS_CONSTANT * _TIMER(cs.time);
			cs.time    = _TIME;
		};
		void SetCameraRadius(float value)
		{
			cs.radius = _CLAMP(value, cs.min_radius, cs.max_radius);
		}
		void AddCameraRadius(int value)
		{ 
			cs.radius = _CLAMP(cs.radius + (cs.radius_speed * value), cs.min_radius, cs.max_radius);
		};
		void AddCameraAngles(float axis_x, float axis_y, float axis_z)
		{
			cs.angle->x += axis_x;	cs.angle->x = fmod(cs.angle->x, 360.0f);
			cs.angle->y += axis_y;	cs.angle->y = fmod(cs.angle->y, 360.0f);
			cs.angle->z += axis_z;	cs.angle->z = fmod(cs.angle->z, 360.0f);
		};
		void AddCameraAngleX(float axis_x)
		{
			cs.angle->x += axis_x;	cs.angle->x = fmod(cs.angle->x, 360.0f);
		};
		void AddCameraAngleY(float axis_y)
		{
			cs.angle->y += axis_y;	cs.angle->y = fmod(cs.angle->y, 360.0f);
		};
		void AddCameraAngleZ(float axis_z)
		{
			cs.angle->z += axis_z;	cs.angle->z = fmod(cs.angle->z, 360.0f);
		};
		void AddCameraAngleX90(float axis_x)
		{
			cs.angle->x = _CLAMP(cs.angle->x + axis_x, -89.9f, 89.9f);
		};
		void AddCameraAngleY90(float axis_y)
		{
			cs.angle->y = _CLAMP(cs.angle->y + axis_y, -89.9f, 89.9f);
		};
		void AddCameraAngleZ90(float axis_z)
		{
			cs.angle->z = _CLAMP(cs.angle->z + axis_z, -89.9f, 89.9f);
		};
		void SetCameraAngleZ(float angle_z)
		{
			cs.angle_z = angle_z;
		}

		void SetCameraRotation(const MATH3DQUATERNION & rotation)
		{
			*cs.rotate = rotation;
		}
		void SetCameraRotation(const MATH3DQUATERNION * rotation)
		{
			*cs.rotate = *rotation;
		}
		void SetCameraPos(const MATH3DVEC & pos)
		{
			*cs.pos = pos;
		}
		void SetCameraPos(const MATH3DVEC * pos)
		{
			*cs.pos = *pos;
		}
		void SetCameraLookAt(const MATH3DVEC & lookat)
		{
			*cs.lookat = lookat;
		}
		void SetCameraLookAt(const MATH3DVEC * lookat)
		{
			*cs.lookat = *lookat;
		}

		CameraSettings & GetCameraSettings()  { return cs; }
		eCameraSignal GetMoveInputForward() { return cs.input.move_z;   }
		eCameraSignal GetMoveInputRight()   { return cs.input.move_x;   }
		eCameraSignal GetMoveInputUp()      { return cs.input.move_y;   }
		eCameraSignal GetRollInput()        { return cs.input.roll;     }
		int32         GetMouseDX()          { return cs.input.mouse_dx; }
		int32         GetMouseDY()          { return cs.input.mouse_dy; }	
		eCameraMode GetCameraMode()      { return cs.mode;         }
		eCameraMode GetCameraPrevMode()  { return cs.prev_mode[1]; }
		eCameraMode GetCameraPrevMode0() { return cs.prev_mode[0]; }
		float GetAspectRatio()           { return cs.aspect_ratio; }
		float GetFOV()           { return cs.fov; }
		float GetFOVR()          { return cs.fovr; }
		float GetMouseSpeed()    { return cs.mouse_speed;  }
		float GetMoveSpeed()     { return cs.move_speed;   }
		float GetRollSpeed()     { return cs.roll_speed;   }
		float GetRadiusSpeed()   { return cs.radius_speed; }
		bool  GetFlyMode()       { return cs.fly_mode; }
		bool  GetXZMoveFix()     { return cs.fix_XZ_speed; }
		float GetTimeFix()       { return cs.timefix;    }
		float GetNearPlane()     { return cs.near_plane; }
		float GetFarPlane()      { return cs.far_plane;  }
		float GetAngleZ()        { return cs.angle_z; }
		const MATH3DVEC *  GetCameraPos()        { return cs.pos;         }
		const MATH3DVEC *  GetCameraLookAt()     { return cs.lookat;      }
		const MATH3DQUATERNION * GetCameraRotate()     { return cs.rotate;      }
		const MATH3DQUATERNION * GetCameraRotateXY()   { return cs.rotate_XY;   }
		const MATH3DQUATERNION * GetCameraRotateX()    { return cs.rotate_X;    }
		const MATH3DQUATERNION * GetCameraRotateY()    { return cs.rotate_Y;    }
		const MATH3DQUATERNION * GetCameraRotateZ()    { return cs.rotate_Z;    }
		const MATH3DQUATERNION * GetCameraRotateZZ()   { return cs.rotate_ZZ;   }
		const MATH3DQUATERNION * GetCameraRotateZZXY() { return cs.rotate_ZZXY; }
		const MATH3DMATRIX * GetViewMatrix()     { return mView;       }
		const MATH3DMATRIX * GetViewMatrixXY()   { return mViewXY;     }
		const MATH3DMATRIX * GetProjMatrix()     { return mProjection; }
		const MATH3DMATRIX * GetViewProjMatrix() { return mViewProj;   }
			
		//>> Доступ к видеоадаптеру
		void SetDevice(const DEV3DBASE * pDevice)
		{
			if (pDevice == nullptr) { _MBM(ERROR_PointerNone); return; }

			dev = const_cast<DEV3DBASE*>(pDevice);
		}
		//>> Доступ к состоянию устройств ввода-вывода
		void SetIO(const CStatusIO * io)
		{
			if (io == nullptr) { _MBM(ERROR_PointerNone); return; }

			IO = const_cast<CStatusIO*>(io);
		}

	protected:
	/*	//>> Обновление матрицы взгляда для игры от 1-го лица
		void __UpdateFirstPersonView()
		{
			MATH3DVEC Position = *cs.pos;
			MATH3DVEC Target   = *cs.lookat;
			MATH3DMATRIX matRot, matRotY;

			if ( (cs.prev_mode[1] == CM_3RDPERSON) ||
				 (cs.prev_mode[0] == CM_3RDPERSON && cs.prev_mode[1] == CM_NOCONTROL) )
			{
				cs.prev_mode[1] = cs.mode; // = текущий вид
				Position = Target;
			}	

		//	Target = MathNormalizeVec( MATH3DVEC(0,0,1) * MathRotateMatrix(cs.angle->x, cs.angle->y, 0) );
			MathRotateMatrix(MATH3DQUATERNION(cs.angle->x, cs.angle->y, 0), matRot);
			Target._set(MATH3DVEC(0,0,1));
			Target *= matRot;
			Target._normalize_check();

		//	MATH3DMATRIX matRotY = MathRotateMatrix_Y(cs.angle->y);		
			MathRotateMatrix(MATH3DQUATERNION(0, cs.angle->y, 0), matRotY);

			MATH3DVEC Right(1,0,0);		MathVecTransformNormal(matRotY, Right);		//Right._normalize_check();
			MATH3DVEC Forward(0,0,1);	MathVecTransformNormal(matRotY, Forward);	//Forward._normalize_check();
			MATH3DVEC Up(0,1,0);

			float step_x = cs.input.move_x * cs.move_speed * cs.timefix;
			float step_z = cs.input.move_z * cs.move_speed * cs.timefix;
			float step_y = cs.input.move_y * cs.move_speed * cs.timefix;
			
			Position += Right   * step_x;	// (+ right, - left)
			Position += Forward * step_z;	// (+ forward, - back)
			Position += Up      * step_y;	// (+ up, - down)

			Target = Position + Target;

			//MathVecTransformNormal(matRotZ, Up); // Крен вокруг оси Z (Roll)

			MathLookAtLHMatrix(Position, Target, Up, *mView);
	
			*cs.lookat = Target;	// Запоминаем новую цель до следующего вызова
			*cs.pos = Position;		// Запоминаем новую позицию до следующего вызова
		} //*/
		//>> Обновление матрицы взгляда для игры от 1-го лица (на кватернионе)
		void UpdateFirstPersonView()
		{
			MATH3DVEC Position = *cs.pos;
			MATH3DVEC Target   = *cs.lookat;

			if (cs.mode_updated)
			{
				if (!IsFirstPersonPrev())
					Position = Target;
				cs.mode_updated = false;
			}

			cs.angle_z = fmod(cs.angle_z + cs.angle->z, 360.0f);	// запомним новый локальный угол крена Z
			if (cs.angle_z < 0.01f && cs.angle_z > -0.01f)			// фикс машинной ошибки типа (angle_z == -5.96046448e-008)
				cs.angle_z = 0;										// .
			cs.angle->z = cs.angle_z;								// .

			////////////////////// CALC BASICS //////////////////////

			MATH3DVEC x_axis(VIEW_OX(mView)); // Локальная СК вида :: бок
			MATH3DVEC y_axis(VIEW_OY(mView)); // Локальная СК вида :: вертикаль
			MATH3DVEC z_axis(VIEW_OZ(mView)); // Локальная СК вида :: прямо

			MATH3DQUATERNION Qx(x_axis, cs.angle->x); // Вращение вокруг локальных осей СК вида :: тангаж / altitude / pitch
			MATH3DQUATERNION Qy(y_axis, cs.angle->y); // Вращение вокруг локальных осей СК вида :: рысканье / heading / yaw
			MATH3DQUATERNION Qz(z_axis, cs.angle->z); // Вращение вокруг локальных осей СК вида :: крен / bank / roll

			cs.angle->x = cs.angle->y = cs.angle->z = 0;

			MATH3DQUATERNION QR = *cs.rotate * Qx * Qy; // Вращение мышью (pitch & yaw)
			QR._normalize_check();

			////////////////////// CALC TARGET //////////////////////

			Target._set(DF_VEC);
			Target *= MathRotateMatrix(QR);										//Target._printf();
			if (Target.y < -VERTICAL_LOCK || Target.y > VERTICAL_LOCK)
			{
				Target = *cs.lookat - *cs.pos; // old
				Target._normalize();
			}
			else { *cs.rotate = QR; } // new

			////////////////////// CALC POSITION //////////////////////

			MATH3DVEC Right   (X_AXIS); // Глобальная СК :: Ox
			MATH3DVEC Forward (Z_AXIS); // Глобальная СК :: Oz
			MATH3DVEC Up      (Y_AXIS); // Глобальная СК :: Oy

			if (!client || server) // single player game  OR  we are server at multiplayer game
			{
				float move_speed = cs.move_speed * cs.timefix;
				float step_x = cs.input.move_x * move_speed; // (+ right, - left)		перемещение в локальном виде
				float step_z = cs.input.move_z * move_speed; // (+ forward, - back)		перемещение в локальном виде
				float step_y = cs.input.move_y * move_speed; // (+ up, - down)			перемещение по глобальной Y_AXIS

				float cosXX = MathDotVec(x_axis, Right);
				float cosXY = MathDotVec(x_axis, Up);
				float cosXZ = MathDotVec(x_axis, Forward);
			
				float cosZX = MathDotVec(z_axis, Right);
				float cosZY = MathDotVec(z_axis, Up);
				float cosZZ = MathDotVec(z_axis, Forward);
			
				Position.y += step_y;
			
				if (cs.fly_mode)
				{
					Position.x += step_x * cosXX + step_z * cosZX;
					Position.y += step_x * cosXY + step_z * cosZY;
					Position.z += step_x * cosXZ + step_z * cosZZ;
				}
				else if (cs.fix_XZ_speed == false)
				{
					Position.x += step_x * cosXX + step_z * cosZX;
					Position.z += step_x * cosXZ + step_z * cosZZ;
				}
				else // нужно повернуть вектор в плоскость XZ, чтобы проекции на оси X и Z были полными (а проекция на ось Y нулевая)
				{
					float step_z_dx = step_z * cosZX;		float step_x_dx = step_x * cosXX;
					float step_z_dy = step_z * cosZY;		float step_x_dy = step_x * cosXY;
					float step_z_dz = step_z * cosZZ;		float step_x_dz = step_x * cosXZ;

					MATH3DVEC step_z_XYZ (step_z_dx, step_z_dy, step_z_dz);		MATH3DVEC step_x_XYZ (step_x_dx, step_x_dy, step_x_dz);
					MATH3DVEC step_z_XY  (step_z_dx,         0, step_z_dz);		MATH3DVEC step_x_XY  (step_x_dx,         0, step_x_dz);

					MATH3DQUATERNION STEP_Z_ROTATE(step_z_XYZ, step_z_XY);		MATH3DQUATERNION STEP_X_ROTATE(step_x_XYZ, step_x_XY);

					step_z_XYZ *= MathRotateMatrix(STEP_Z_ROTATE);				step_x_XYZ *= MathRotateMatrix(STEP_X_ROTATE);

					Position.x += step_x_XYZ.x  +  step_z_XYZ.x; // Извлекаем проекции из векторов движения
					Position.z += step_x_XYZ.z  +  step_z_XYZ.z; // .
				}
			}

			Target = Position + Target;

			////////////////////// CALC RESULTS //////////////////////

			MathLookAtLHMatrix(Position, Target, Up, *mViewXY); // Без крена камеры

			*cs.rotate_XY = MATH3DQUATERNION(Right, MATH3DVEC(VIEW_OX(mViewXY))) *
							MATH3DQUATERNION(Up,	MATH3DVEC(VIEW_OY(mViewXY)));		//cs.rotate_XY->_normalize_check();

			*cs.rotate_Z    = Qz;														//cs.rotate_Z->_normalize_check();
			*cs.rotate_ZZ   = MATH3DQUATERNION(0,0,cs.angle_z);							//cs.rotate_ZZ->_normalize_check();
			*cs.rotate_ZZXY = (*cs.rotate_ZZ) * (*cs.rotate_XY);						//cs.rotate_ZZXY->_normalize_check();

			MATH3DVEC DF(DF_AXIS);
			DF *= MathRotateMatrix(*cs.rotate_XY);
			*cs.rotate_Y = MATH3DQUATERNION(DF_VEC, MATH3DVEC(DF.x, 0, DF.z));
			*cs.rotate_X = MATH3DQUATERNION(DF_VEC, MATH3DVEC(0, DF.y, DF.z));

			Up *= MathRotateMatrix(Qz);							// С учётом крена камеры
			MathLookAtLHMatrix(Position, Target, Up, *mView);	// .

			*cs.lookat = Target;
			*cs.pos = Position;
		}
	/*	//>> Обновление матрицы взгляда для игры от 3-го лица
		void __UpdateThirdPersonView()
		{
			MATH3DVEC Position = *cs.pos;
			MATH3DVEC Target   = *cs.lookat;
			MATH3DMATRIX matRot;

			if ( (cs.prev_mode[1] == CM_1STPERSON) ||
				 (cs.prev_mode[0] == CM_1STPERSON && cs.prev_mode[1] == CM_NOCONTROL) )
			{
				cs.prev_mode[1] = cs.mode; // = текущий вид
				Target = Position;
			}			

		//	MATH3DMATRIX matRot = MathRotateMatrix_Y(cs.angle->y);
			MathRotateMatrix(MATH3DQUATERNION(0, cs.angle->y, 0), matRot);

			MATH3DVEC Right(1,0,0);		MathVecTransformNormal(matRot, Right);
			MATH3DVEC Forward(0,0,1);	MathVecTransformNormal(matRot, Forward);
			MATH3DVEC Up(0,1,0);

			float step_x = cs.input.move_x * cs.move_speed * cs.timefix;
			float step_z = cs.input.move_z * cs.move_speed * cs.timefix;
			float step_y = cs.input.move_y * cs.move_speed * cs.timefix;

			Target += Right   * step_x;	// (+ right, - left)
			Target += Forward * step_z; // (+ forward, - back)
			Target += Up      * step_y;

			// +/- cs.radius : инверсия для опций
			//Position = Target + MATH3DVEC(0,0,static_cast<float>(-cs.radius)) * MathRotateMatrix(cs.angle->x, cs.angle->y, 0);
			MathRotateMatrix(MATH3DQUATERNION(cs.angle->x, cs.angle->y, 0), matRot);
			MATH3DVEC ROTVEC(0, 0, static_cast<float>(-cs.radius));
			ROTVEC *= matRot;
			Position = Target + ROTVEC;

			MathLookAtLHMatrix(Position, Target, Up, *mView);
		
			*cs.lookat = Target;	// Запоминаем новую цель до следующего вызова
			*cs.pos    = Position;	// Запоминаем новую позицию до следующего вызова
		}//*/
		//>> Обновление матрицы взгляда для игры от 3-го лица (на кватернионе)
		void UpdateThirdPersonView()
		{
			MATH3DVEC Position = *cs.pos;
			MATH3DVEC Target   = *cs.lookat;

			if (cs.mode_updated)
			{
				if (IsFirstPersonPrev())
					Target = Position;
				cs.mode_updated = false;
			}

			cs.angle_z = fmod(cs.angle_z + cs.angle->z, 360.0f);	// запомним новый локальный угол крена Z
			if (cs.angle_z < 0.01f && cs.angle_z > -0.01f)			// фикс машинной ошибки типа (angle_z == -5.96046448e-008)
				cs.angle_z = 0;										// .
			cs.angle->z = cs.angle_z;								// .

			////////////////////// CALC BASICS //////////////////////

			MATH3DVEC x_axis(VIEW_OX(mView)); // Локальная СК вида :: бок
			MATH3DVEC y_axis(VIEW_OY(mView)); // Локальная СК вида :: вертикаль
			MATH3DVEC z_axis(VIEW_OZ(mView)); // Локальная СК вида :: прямо

			MATH3DQUATERNION Qx(x_axis, cs.angle->x); // Вращение вокруг локальных осей СК вида :: тангаж / altitude / pitch
			MATH3DQUATERNION Qy(y_axis, cs.angle->y); // Вращение вокруг локальных осей СК вида :: рысканье / heading / yaw
			MATH3DQUATERNION Qz(z_axis, cs.angle->z); // Вращение вокруг локальных осей СК вида :: крен / bank / roll

			cs.angle->x = cs.angle->y = cs.angle->z = 0; // Запрос поворотов обработан -> сброс

			MATH3DQUATERNION QR = *cs.rotate * Qx * Qy; // Вращение мышью (pitch & yaw)
			QR._normalize_check();

			////////////////////// CALC TARGET //////////////////////

			MATH3DVEC Right   (X_AXIS); // Глобальная СК :: Ox
			MATH3DVEC Forward (Z_AXIS); // Глобальная СК :: Oz
			MATH3DVEC Up      (Y_AXIS); // Глобальная СК :: Oy

			if (!client || server) // single player game  OR  we are server at multiplayer game
			{
				float move_speed = cs.move_speed * cs.timefix;
				float step_x = cs.input.move_x * move_speed; // (+ right, - left)		перемещение в локальном виде
				float step_z = cs.input.move_z * move_speed; // (+ forward, - back)		перемещение в локальном виде
				float step_y = cs.input.move_y * move_speed; // (+ up, - down)			перемещение по глобальной Y_AXIS

				float cosXX = MathDotVec(x_axis, Right);
				float cosXY = MathDotVec(x_axis, Up);
				float cosXZ = MathDotVec(x_axis, Forward);

				float cosZX = MathDotVec(z_axis, Right);
				float cosZY = MathDotVec(z_axis, Up);
				float cosZZ = MathDotVec(z_axis, Forward);

				Target.y += step_y;

				if (cs.fly_mode)
				{
					Target.x += step_x * cosXX + step_z * cosZX;
					Target.y += step_x * cosXY + step_z * cosZY;
					Target.z += step_x * cosXZ + step_z * cosZZ;
				}
				else if (cs.fix_XZ_speed == false)
				{
					Target.x += step_x * cosXX + step_z * cosZX;
					Target.z += step_x * cosXZ + step_z * cosZZ;
				}
				else // нужно повернуть вектор в плоскость XZ, чтобы проекции на оси X и Z были полными (а проекция на ось Y нулевая)
				{
					float step_z_dx = step_z * cosZX;		float step_x_dx = step_x * cosXX;
					float step_z_dy = step_z * cosZY;		float step_x_dy = step_x * cosXY;
					float step_z_dz = step_z * cosZZ;		float step_x_dz = step_x * cosXZ;

					MATH3DVEC step_z_XYZ (step_z_dx, step_z_dy, step_z_dz);		MATH3DVEC step_x_XYZ (step_x_dx, step_x_dy, step_x_dz);
					MATH3DVEC step_z_XY  (step_z_dx,         0, step_z_dz);		MATH3DVEC step_x_XY  (step_x_dx,         0, step_x_dz);

					MATH3DQUATERNION STEP_Z_ROTATE(step_z_XYZ, step_z_XY);		MATH3DQUATERNION STEP_X_ROTATE(step_x_XYZ, step_x_XY);

					step_z_XYZ *= MathRotateMatrix(STEP_Z_ROTATE);				step_x_XYZ *= MathRotateMatrix(STEP_X_ROTATE);

					Target.x += step_x_XYZ.x + step_z_XYZ.x; // Извлекаем проекции из векторов движения
					Target.z += step_x_XYZ.z + step_z_XYZ.z; // .
				}
			}

			////////////////////// CALC POSITION //////////////////////

			MATH3DVEC ROTVEC(DF_AXIS);
			ROTVEC *= MathRotateMatrix(QR);									// ROTVEC._printf();
			if (ROTVEC.y < -VERTICAL_LOCK || ROTVEC.y > VERTICAL_LOCK)
			{
				ROTVEC = *cs.lookat - *cs.pos; // old
				ROTVEC._normalize();
			}
			else { *cs.rotate = QR; } // new
	
			Position = Target + ROTVEC * static_cast<float>(-cs.radius);

			////////////////////// CALC RESULTS //////////////////////

			MathLookAtLHMatrix(Position, Target, Up, *mViewXY); // Без крена камеры

			*cs.rotate_XY = MATH3DQUATERNION(Right, MATH3DVEC(VIEW_OX(mViewXY))) *
				            MATH3DQUATERNION(Up,    MATH3DVEC(VIEW_OY(mViewXY))) ;			//cs.rotate_XY->_normalize_check();

			*cs.rotate_Z    = Qz;															//cs.rotate_Z->_normalize_check();
			*cs.rotate_ZZ   = MATH3DQUATERNION(0, 0, cs.angle_z);							//cs.rotate_ZZ->_normalize_check();
			*cs.rotate_ZZXY = (*cs.rotate_ZZ) * (*cs.rotate_XY);							//cs.rotate_ZZXY->_normalize_check();

			MATH3DVEC DF(DF_AXIS);
			DF *= MathRotateMatrix(*cs.rotate_XY);
			*cs.rotate_Y = MATH3DQUATERNION(DF_VEC, MATH3DVEC(DF.x, 0, DF.z));
			*cs.rotate_X = MATH3DQUATERNION(DF_VEC, MATH3DVEC(0, DF.y, DF.z));

		//	Up *= MathRotateMatrix(Qz);							// С учётом крена камеры
		//	MathLookAtLHMatrix(Position, Target, Up, *mView);	// .
			*mView = *mViewXY;									// Отображение крена для 3го лица убрано

			*cs.lookat = Target;
			*cs.pos = Position;
		}
		//>> Обновление матрицы взгляда (общая процедура)
		void UpdateViewMatix()
		{
			UpdateCameraTime();

			int32 mouse_dx = 0;
			int32 mouse_dy = 0;

			if (!server || server_self) // только свою камеру
			{
				mouse_dx = cs.input.mouse_dx = IO->mouse.sys_x - IO->window.center_x; // delta around center
				mouse_dy = cs.input.mouse_dy = IO->mouse.sys_y - IO->window.center_y; // .
			}

		//	if (cs.input.mouse_dy || cs.input.mouse_dx)
		//		printf("\n%9f  %9f", cs.angle.x, cs.angle.y);

			switch (cs.mode)
			{
			case CM_NOCONTROL: break;
			case CM_1STPERSON:
			case CM_3RDPERSON:
				if (cs.input.mouse_dx) AddCameraAngleY   (cs.mouse_speed * mouse_dx);
				if (cs.input.mouse_dy) AddCameraAngleX90 (cs.mouse_speed * mouse_dy);
				if (!client || server)
					if (cs.input.roll)
						AddCameraAngleZ   (-cs.input.roll * cs.roll_speed * cs.timefix);
				break;
			default:
				if (cs.input.mouse_dx) AddCameraAngleY (cs.mouse_speed * mouse_dx);
				if (cs.input.mouse_dy) AddCameraAngleX (cs.mouse_speed * mouse_dy);
			};

			// TODO: следует оптимизировать расчёты для случая с сервером

			switch (cs.mode)
			{
			case CM_1STPERSON: UpdateFirstPersonView(); break;
			case CM_3RDPERSON: UpdateThirdPersonView(); break;
			}
		}
		//>> Обновление матрицы проекции (линзы)
		void UpdateProjectionMatrix()
		{
			if (cs.update_proj_mat)
			{
				cs.update_proj_mat = false;
				MathPerspectiveFovLHMatrix(cs.fov, cs.aspect_ratio, cs.near_plane, cs.far_plane, *mProjection);
			}
		}
		
	public:
		//>> Обновление позиции камеры и матриц в новый кадр
		void Update(bool restore)
		{
			if (!server || server_self)
			if (dev    == nullptr) { _MBM(ERROR_PointerNone); return; }
			if (IO     == nullptr) { _MBM(ERROR_PointerNone); return; }

			if (!restore)
			{
				if (skip_update) skip_update = false;
				else 
				{
					UpdateViewMatix();
					UpdateProjectionMatrix();
					(*mViewProj)  = (*mView);
					(*mViewProj) *= (*mProjection);
				}
			}

			if (!server || server_self)
			{
				switch (cs.mode)
				{
				case CM_NOCONTROL:
				case CM_1STPERSON:
				case CM_3RDPERSON:
				default:
					dev->SetTransformView(mView); break;
				}
				dev->SetTransformProjection(mProjection);
			}

			return;
		}
		//>> Пропуск обновления кадра
		void SkipUpdate()
		{
			skip_update = true;
		}
		//>> Устанавливает новый (не совпадающий с текущим) режим камеры, запоминая предыдущий : RestoreCameraMode()
		void SetCameraMode(const eCameraMode mode)
		{
			if (cs.mode == mode) return;

			cs.prev_mode[0] = cs.prev_mode[1];
			cs.prev_mode[1] = cs.mode;
			cs.mode = mode;
			cs.mode_updated = true;
		};
		//>> Возвращает предыдущий режим камеры
		void RestoreCameraMode()
		{
			//cs.mode = cs.prev_mode[1];
			SetCameraMode(cs.prev_mode[1]);
		}
		
	protected:
		//>> Проверяет режим на принадлежность к набору видов от первого лица
		bool IsFirstPerson(eCameraMode camera_mode)
		{
			switch (camera_mode)
			{
			case CM_1STPERSON:
			case CM_FLY_SIMULATOR_1ST:
				return true;
			case CM_3RDPERSON:
			case CM_FLY_SIMULATOR_3RD:
				return false;
			}

			return true;
		}

	public:
		//>> Позволяет определить являлась ли позиция игрока позицией камеры в предыдущем режиме
		bool IsFirstPersonPrev()
		{
			eCameraMode previous_mode = cs.prev_mode[1];

			if (previous_mode == CM_NOCONTROL)
				previous_mode = cs.prev_mode[0];

			return IsFirstPerson(previous_mode);
		}
		//>> Позволяет определить является ли позиция игрока позицией камеры в данный момент
		bool IsFirstPerson()
		{
			eCameraMode active_mode = cs.mode;

			if (active_mode == CM_NOCONTROL)
				active_mode = cs.prev_mode[1];

			return IsFirstPerson(active_mode);
		}
		//>> Устанавливает position или lookat камеры в зависимости от текущего режима вида
		void SetCameraAuto(const MATH3DVEC & player_position)
		{
			if (IsFirstPerson()) SetCameraPos(player_position);
			else                 SetCameraLookAt(player_position);
		}
	};
}

#endif // _CAMERA_H