// ----------------------------------------------------------------------- //
//
// MODULE  : Player.h
//
// PURPOSE : Всё по игроку / временно ! позже разделить /
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _PLAYER_H
#define _PLAYER_H

#include "stdafx.h"
#include "Object.h"
#include "Camera.h"
#include "FontRe.h"
#include "Network.h"
#include "StatusIO.h"
#include "KeyEvent.h"
#include "Interact.h"

#define  CPlayerF  CPlayer // class final

#define  SELF_PL_SLOT         0  // свой слот по умолчанию :: также определяет номер последнего зарезервированного
#define  PLAYER_NAME_MAX_LEN  32 // 31 wchar_t + 1 wchar_t null terminator

namespace SAVFGAME
{
	class CPlayerManager;
	class CBasePlayer;

	struct PlayerPerks
	{
		PlayerPerks()  { Close(); }
		~PlayerPerks() { }
		void Close()
		{

		}
	};
	struct PlayerAbilities
	{
		PlayerAbilities()  { Close(); }
		~PlayerAbilities() { }
		void Close()
		{

		}
	};
	struct PlayerParameters
	{
		PlayerParameters()  { Close(); }
		~PlayerParameters() { }
		void Close()
		{
			health = 100;
			armor  = 0;
			movement_speed = 1.f;
		}
		uint32		health;
		uint32		armor;
		float		movement_speed;
	};
	struct PlayerStatus
	{
		friend class CBasePlayer;
		friend class CPlayerManager;
		friend class CNetServer;
		friend class CNetClient;
	private:
		struct PlayerStatusNet
		{
			friend class CNetServer;
			friend class CNetClient;
			PlayerStatusNet() : tick(CLIENT_TICK) { Close(); }
			~PlayerStatusNet(){}
			void Close()
			{
				loaded    = false;
				connected = false;
				mid       = 0;
				tick.Reset(CLIENT_TICK);
			}
		private:
			NETREMOTEADDR _;
		protected:
			NETREMOTEADDR *	const	addr      { &_ };	 // [SERVER] player's remote address
			timepoint				time;				 // [SERVER] last seen player
			EmpiricalTick			tick;				 // [SERVER] time between player's incoming net messages
			uint32					mid       { 0 };	 // [SERVER] last recieved message ID
		public:
			TickInterpolation * const	cl_tick   { &tick };
			bool						connected { false }; // [SERVER] status of network connection
			bool						loaded    { false }; // [SERVER] user has loaded resources and ready for game
		};
		struct PlayerStatusModel : public CObject
		{
			friend struct PlayerStatus;
			friend class CBasePlayer;
			friend class CPlayerManager;
			friend class CNetServer;
			friend class CNetClient;
			////////////////////////
			struct InterpolationMemory
			{
				InterpolationMemory()
				{
					P.Create(2);
					Q.Create(2);

					Close();
				}
				~InterpolationMemory(){};
				void Close()
				{
					for (uint16 i = 0; i < P.count; i++) P[i]._default();
					for (uint16 i = 0; i < Q.count; i++) Q[i]._default();
					P.prev = Q.prev = 0;
					P.cur  = Q.cur  = 1;
				}
				TBUFFERNEXT <MATH3DVEC,        uint16>	P;	// [CLIENT]
				TBUFFERNEXT <MATH3DQUATERNION, uint16>	Q;	// [CLIENT]
			};
			PlayerStatusModel() : CObject(), pos(CObject::pos), pos_P(&pos->P), pos_Q(&pos->Q)
			{
				Close();
			}
			~PlayerStatusModel(){};
			void Close()
			{
				MEM.Close();
				pos->_default();
				maxCoord._default();
				id   = MISSING;
				show = true;		
			}
		public:	
			WPOS * const							pos;		// ptr to player model world position (full)
			MATH3DVEC * const						pos_P;		// ptr to player model world position (moving P status only)
			MATH3DQUATERNION * const				pos_Q;		// ptr to player model world position (rotation Q status only)
			bool									show;		// status of showing player model		
		protected:
			uint32				id;			// id in pack of models (default : MISSING)
			MATH3DVEC			maxCoord;	// max dx/dy/dz from center of model
		protected:
			InterpolationMemory	MEM; // memory for interpolation
		public:
			void MemoryP(const MATH3DVEC & mem_P)        { MEM.P.SetNext(mem_P); }
			void MemoryQ(const MATH3DQUATERNION & mem_Q) { MEM.Q.SetNext(mem_Q); }
		};
		struct PlayerStatusName
		{
			friend struct PlayerStatus;
			friend class CBasePlayer;
			friend class CPlayerManager;
			friend class CNetServer;
			friend class CNetClient;
			PlayerStatusName() : data(&_), pos(_.pos), text(&_.text), font(nullptr) { Close(); }
			~PlayerStatusName(){};
			void Close()
			{
				*data = L"";
				W     = 0;
				H     = 0;
				W_05  = 0;
				show  = true;
				//font  = nullptr;
				
				repeat  = 0;
				ID      = 0;
				NEXT_ID = false;
			}
		private:
			CText _;
		protected:
			CFontR *				font;	// player name font
			CText *	const			data;	// player name CText data		
			float					W;		// polygon width in units
			float					H;		// polygon height in units
			float					W_05;	// half of polygon width in units
		protected:
			uint16					repeat;		// [server/client] num of sending repeats
			uint16					ID;			// iteration ID
			bool					NEXT_ID;	// (tip) new name
		public:
			const wstring * const	text;	// ptr to text
			WPOS * const			pos;	// player name position
			bool					show;	// (default : true)  show player name (upon player model)	
		protected:
			void Show()
			{
				if (font == nullptr) { _MBM(ERROR_PointerNone); return; }

				if (!show) return;

				font->ShowText(data);
			};
			void NetSetNextID()					{ ID++; NEXT_ID = true; }
			void NetSetID(uint16 NEW_ID)		{ if (ID != NEW_ID) { ID = NEW_ID; NEXT_ID = true; } }
			void NetRepeatSet(uint32 repeats)	{ repeat = repeats; }
			void NetRepeatSubOne()				{ if (repeat) {repeat--; NEXT_ID = false;} }

		};
		struct PlayerStatusCam
		{
		friend class CBasePlayer;
			struct CameraMemory
			{
				CameraMemory(){}
				~CameraMemory(){};
				void Close()
				{
					view._default();
					rotate_XY._default();
					rotate._default();
				}
				MATH3DMATRIX		view;		// GetViewMatrix()
				MATH3DQUATERNION	rotate_XY;	// GetCameraRotateXY()
				MATH3DQUATERNION	rotate;		// GetCameraRotate()
			};
			struct InterpolationMemory
			{
				InterpolationMemory()
				{
					Az.Create(2);
					P.Create(2);
					Q.Create(2);

					Close();
				}
				~InterpolationMemory(){};
				void Close()
				{
					for (uint16 i=0; i < Az.count; i++) Az[i] = 0;
					for (uint16 i=0; i < P.count;  i++) P[i]._default();
					for (uint16 i=0; i < Q.count;  i++) Q[i]._default();
					Az.prev = P.prev = Q.prev = 0;
					Az.cur  = P.cur  = Q.cur  = 1;
				}
				TBUFFERNEXT <float,            uint16>	Az;	// [CLIENT]
				TBUFFERNEXT <MATH3DVEC,        uint16>	P;	// [CLIENT]
				TBUFFERNEXT <MATH3DQUATERNION, uint16>	Q;	// [SERVER]
			};
			PlayerStatusCam() { Close(); }
			void Close()
			{
				MEM.Close();
				first_person.Close();
				last_mode = CM_NOCONTROL;			
			}						
			CameraMemory				first_person;	// 1st person camera memory
			eCameraMode					last_mode;		// camera from previous frame
		protected:
			InterpolationMemory			MEM;			// memory for interpolation		
		public:
			void MemoryAz(float angle_z)				 { MEM.Az.SetNext(angle_z); }
			void MemoryP(const MATH3DVEC & mem_P)		 { MEM.P.SetNext(mem_P);    }
			void MemoryQ(const MATH3DQUATERNION & mem_Q) { MEM.Q.SetNext(mem_Q);    }
		public:
			void ResetMemoryAz(float cam_Az)                  { MEM.Az.Reset(cam_Az); }
			void ResetMemoryP(const MATH3DVEC & cam_P)        { MEM.P.Reset(cam_P);   }
			void ResetMemoryQ(const MATH3DQUATERNION & cam_Q) { MEM.Q.Reset(cam_Q);   }		
		};
		struct PlayerStatusChat
		{
			friend struct PlayerStatus;
			friend class CNetClient;
			friend class CNetServer;
			PlayerStatusChat() : name(nullptr)
			{
				message.reserve(512);
				Close();
			}
			void Close()
			{
				message     = L"";
				message_raw = L"";
				repeat      = 0;
				ID          = 0;
				NEXT_ID     = false;
			}
		private:
			const wstring *	name;			// ptr to player's name
			wstring			message;		// text with name
			wstring			message_raw;	// text as it is
			uint16			repeat;			// count of sending repeats
			uint16			ID;				// unique message ID
			bool			NEXT_ID;		// (tip) new ID / new message
		private:
			void NetSetMessageID(uint16 CLID) { if (ID != CLID) { ID = CLID; NEXT_ID = true; } }
			void NetSetMessage(const wchar_t * msg)
			{
				message.clear();
				message_raw.clear();

				message.append(L"\r[");
				message.append(*name);
				message.append(L"] ");
				message.append(msg);

				message_raw = msg;

				NEXT_ID = false;
			}
		private:
			void NetRepeatSet(uint16 repeats)       { repeat = repeats; }
			void NetRepeatSubOne()                  { if (repeat) { repeat--; NEXT_ID = false; } }
		};
	public:
		PlayerStatus()
		{
			chat.name = & name.data->text;
			Close();
		}
		~PlayerStatus() { }
		void Close()
		{
			model.Close();
			net.Close();
			name.Close();
			camera.Close();
			chat.Close();
			
			bot     = false;
			evlock  = 0;
		}	
	public:
		PlayerStatusModel	model;	// model status
		PlayerStatusName	name;	// name status
	protected:	
		PlayerStatusNet		net;	// network status (server side)
		PlayerStatusCam		camera;	// camera status
		PlayerStatusChat	chat;	// [SERVER] store incoming chat message
	public:
		bool	bot;		// status of PC control :: default = false	
	protected:
		uint32	evlock;		// if != 0 -> lock player actions
	public:
		PlayerStatus(const PlayerStatus& src)				= delete;
		PlayerStatus(PlayerStatus&& src)					= delete;
		PlayerStatus& operator=(const PlayerStatus& src)	= delete;
		PlayerStatus& operator=(PlayerStatus&& src)			= delete;
	protected:
		//>> Установка шрифта для имени игрока
		void SetFont(CFontR * pFont)
		{
			name.font = pFont;
		}
	public:
		//>> Установка имени игрока :: set <height = MISSING> to skip height
		void SetName(const wchar_t * player_name, float height)
		{
			if (name.font == nullptr) { _MBM(ERROR_PointerNone); return; }

			name.data->text = player_name;

			wstring & _ = name.data->text;
			if (_.size() >= PLAYER_NAME_MAX_LEN)
				_.erase(_.begin() + PLAYER_NAME_MAX_LEN - 1, _.end());

			name.NetSetNextID();

			if (_NOMISS(height)) SetNameHeight(height);
			else
			{
				name.font->GetGeneratedWH(name.data->text, name.data->height, 0, MISSING, name.W, name.H);
				name.W_05 = name.W * 0.5f;
				name.data->CallReloadConstantText();
			}
		}
		//>> Установка высоты имени игрока
		void SetNameHeight(float height)
		{
			name.data->height = abs(height);

			name.font->GetGeneratedWH(name.data->text, name.data->height, 0, MISSING, name.W, name.H);
			name.W_05 = name.W * 0.5f;

			name.data->CallReloadConstantText();
		}	
		//>> Установка модели игрока
		void SetModel(uint32 id, MATH3DVEC & box_max_coordinates)
		{
			model.id       = id;
			model.maxCoord = box_max_coordinates;
		}
		//>> Сообщает текущую модель игрока
		uint32 GetModelID()
		{
			return model.id;
		}
		//>> Is connected
		bool Connected() { return net.connected; }
		//>> Is game resources has been loaded 
		bool Loaded() { return net.loaded; }
		//>> Is connected & loaded
		bool InGame() { return (net.connected && net.loaded); }
	public:
		//>> Проверка, может ли игрок выполнять действия на сцене
		bool LockGameActions() { return _BOOL(evlock); }
		//>> Добавить +1 к блокировке действий на сцене
		void LockGameActionsAdd() { evlock++; }
		//>> Добавить -1 к блокировке действий на сцене
		void LockGameActionsSub() { if (evlock) evlock--; }
		//>> Сбросить блокировку действий на сцене
		void LockGameActionsReset() { evlock = 0; }
	protected:
		//>> Запоминает состояние камеры
		void StoreCameraCondition(CPlayerCameraF * pCamera, eCameraMode mode)
		{
			switch (mode)
			{
			case CM_1STPERSON:
				camera.first_person.view      = *pCamera->GetViewMatrix();
				camera.first_person.rotate_XY = *pCamera->GetCameraRotateXY();
				camera.first_person.rotate    = *pCamera->GetCameraRotate();
				break;
			}
		}
		//>> Восстановление вида 
		void CheckCameraCondition(CPlayerCameraF * pCamera)
		{
			eCameraMode cam_mode = pCamera->GetCameraMode();

			// от 3го лица к 1у лицу

			if (cam_mode == CM_1STPERSON && camera.last_mode == CM_3RDPERSON)
				pCamera->SetCameraRotation(camera.first_person.rotate);

			if (cam_mode == CM_3RDPERSON && camera.last_mode == CM_1STPERSON)
				StoreCameraCondition(pCamera, CM_1STPERSON);
		}
	};

	class CBasePlayer // базовый класс игрока
	{
		friend class CPlayerManager;
	private:
		struct CBasePlayerD
		{
			CPlayerCameraF	camera;
			CStatusIO		IO;
			CKeyEvent		keyev;
			CBaseInteract	interact;
		};
		CBasePlayerD _;
	public:
		CPlayerCameraF * const		camera   { & _.camera   };
		CStatusIO * const			IO       { & _.IO       };
		CKeyEvent * const			keyev    { & _.keyev    };
		CBaseInteract * const		interact { & _.interact };
	public:
		PlayerStatus  status;  // player basics
	public:
		CBasePlayer(const CBasePlayer& src)				= delete;
		CBasePlayer(CBasePlayer&& src)					= delete;
		CBasePlayer& operator=(const CBasePlayer& src)	= delete;
		CBasePlayer& operator=(CBasePlayer&& src)		= delete;
	public:
		CBasePlayer()
		{
			camera->SetIO(IO);
			interact->Init(camera, status.model.pos_P, IO);

			Close();
		};
		~CBasePlayer(){};
		void Close()
		{
			status.Close();
		}

	public:
		//>> Показ имени на сцене
		void ShowPlayerName()
		{
			status.name.Show();
		}	

	protected:	
		//>> Обновление памяти о состоянии камеры и позиции игрока
		void UpdatePlayerData(bool client)
		{		
			auto cam_mode = camera->GetCameraMode();
			status.camera.last_mode = cam_mode;

			if (cam_mode == CM_NOCONTROL) return;

			const MATH3DQUATERNION * camera_rotate_XY = camera->GetCameraRotateXY();
			const MATH3DQUATERNION * camera_rotate_Z  = camera->GetCameraRotateZ();
			const MATH3DVEC *		 camera_pos       = camera->GetCameraPos();
			const MATH3DVEC *		 camera_look_at   = camera->GetCameraLookAt();

			auto & player_position = *status.model.pos_P;
			auto & player_rotation = *status.model.pos_Q;

			auto move_x   = camera->GetMoveInputRight();
		//	auto move_y   = camera->GetMoveInputUp();
			auto move_z   = camera->GetMoveInputForward();
			auto roll_az  = camera->GetRollInput();
			
			if (camera->IsFirstPerson())
			{
				player_position = *camera_pos;
				player_rotation = (*camera_rotate_XY) * (*camera_rotate_Z);
				player_rotation._normalize_check();
			}
			else
			{
				player_position = *camera_look_at;
				if (move_x || move_z) // при движении модель игрока повернётся -> обновим статус поворота [1st person]
				{	
					player_rotation = (*camera_rotate_XY) * (*camera_rotate_Z);
					player_rotation._normalize_check();
					status.StoreCameraCondition(camera, CM_1STPERSON);
				}	
			//	else if (roll_az)
				else if ( (!client && roll_az) ||
				          ( client && !status.camera.MEM.Az.Equal())
						) // повернём модель игрока вокруг локальной Z вида 1-го лица
				{	
					MATH3DVEC z_axis(_VIEW_OZ(status.camera.first_person.view));
					float angle_z = camera->GetAngleZ();
					player_rotation = status.camera.first_person.rotate_XY * MATH3DQUATERNION(z_axis, angle_z);
					player_rotation._normalize_check();
				}
			}
		}
		//>> Обновление позиции отображаемого на 3D сцене имени
		void UpdatePlayerNamePos(CPlayerCameraF * self_camera)
		{
			auto camera_rotate_XY = self_camera->GetCameraRotateXY();
			auto name_wpos   = status.name.pos;
			auto player_wpos = status.model.pos;
		
			// для каждого игрока позиция имени других игроков видится по своему //
			// соответственно, этот расчёт каждый делает себе сам //
		
			//MATH3DVEC Forward(Z_AXIS);		
			//Forward *= MathRotateMatrix(*camera_rotate_XY);

			name_wpos->Q = *camera_rotate_XY;
			name_wpos->P = player_wpos->P;
			name_wpos->y += 1 + status.model.maxCoord.y; // fix на высоту модели

			// отцентрируем положение имени //
		
			//MATH3DVEC x_axis(X_AXIS);
			//x_axis *= MathRotateMatrix(*camera_rotate);
			MATH3DVEC x_axis(VIEW_OX(self_camera->GetViewMatrixXY()));

			float cosXX = MathDotVec(x_axis, OX_VEC);
			float cosXZ = MathDotVec(x_axis, OZ_VEC);

			name_wpos->x -= status.name.W_05 * cosXX;
			name_wpos->z -= status.name.W_05 * cosXZ;
		}
		//>> Интерполяция собственного положения (если мы клиент на сервере)
		void UpdateClientSelfInterpolate(float interpolation_t)
		{
			if (camera->GetCameraMode() == CM_NOCONTROL) return; // исправляет сдвиги, в частности, скайбокса, 
																 // если клиент переключится в CM_NOCONTROL
			camera->SetCameraAuto 
				( MathPointLERP
					( status.camera.MEM.P.Previous(),
					  status.camera.MEM.P.Current(),
					  interpolation_t )
				);

			float az_prev = status.camera.MEM.Az.Previous();
			float az_cur  = status.camera.MEM.Az.Current();

			if (az_cur != az_prev)
				camera->SetCameraAngleZ
					( MathSimpleLERP(az_prev, az_cur, interpolation_t) );
		}
		//>> Интерполяция положения прочих игроков (если мы клиент на сервере)
		void UpdateClientInterpolate(float interpolation_t)
		{
			status.model.pos->P = MathPointLERP
				( status.model.MEM.P.Previous(),
				  status.model.MEM.P.Current(),
				  interpolation_t );

			status.model.pos->Q = MathQuaternionSLERP
				( status.model.MEM.Q.Previous(),
				  status.model.MEM.Q.Current(),
				  interpolation_t );
		}
		//>> Интерполяция данных прочих игроков (если мы сервер)
		void UpdateServerInterpolate()
		{
			float interpolation_t = status.net.cl_tick->GetInterpolation();

			camera->SetCameraRotation
				( MathQuaternionSLERP
					( status.camera.MEM.Q.Previous(),
					  status.camera.MEM.Q.Current(),
					  interpolation_t )
				);

			interact->MEM.SetInterpolation(interpolation_t);
		}
		//>> Обновление камеры и других связанных данных
		void Update(CPlayerCameraF * self_camera, float interpolation_t, bool self, bool server, bool client)
		{
			camera->SetNetClient(client);
			camera->SetNetServer(server);

			if (self) // только для себя
			{
				if (client)
				{
					UpdateClientSelfInterpolate(interpolation_t);
				}

				status.CheckCameraCondition(camera);
				camera->Update(0);
				UpdatePlayerData(client);
			}
			else // другие игроки
			{
				if (client) // интерполируем положение других игроков
				{
					if ( (status.net.connected && status.net.loaded) ||
						  status.bot )
						UpdateClientInterpolate(interpolation_t);
				}
				else
				{
					if (status.net.connected && status.net.loaded) // сервер должен просчитать и других игроков
					{
						UpdateServerInterpolate();

						status.CheckCameraCondition(camera);
						camera->Update(0);
						UpdatePlayerData(client);
					}
					else if (status.bot)
					{
						// ... дополнительные операции для ботов ...

						status.CheckCameraCondition(camera);
						camera->Update(0);
						UpdatePlayerData(client);
					}
				}
			}

			if ((status.net.connected && status.net.loaded) || status.bot)
				UpdatePlayerNamePos(self_camera);
		}
	};
	class CPlayer : public CBasePlayer // класс игрока
	{
		friend class CPlayerManager;
	public:
		PlayerParameters	parameter;
		PlayerAbilities		ability;
		PlayerPerks			perk;			
	public:
		CPlayer(const CPlayer& src)					= delete;
		CPlayer(CPlayer&& src)						= delete;
		CPlayer& operator=(const CPlayer& src)		= delete;
		CPlayer& operator=(CPlayer&& src)			= delete;
	public:
		CPlayer() : CBasePlayer() { Close(); };
		~CPlayer() { };
		void Close()
		{
			Reset();
			CBasePlayer::Close();
		}

		//>> Сброс параметров
		void Reset()
		{
			perk.Close();
			ability.Close();
			parameter.Close();
			status.Close();
		}
	};

	class CPlayerManager
	{
		typedef float(*pGetInterpolation)();
	public:
		pGetInterpolation		GetInt;
	protected:
		CStatusNetwork *		network { nullptr }; // ptr to net status
		TickInterpolation *		sv_tick { nullptr }; // [CLIENT] ptr to server's tick interpolation		
	protected:
		VECDATAPP <CPlayerF>	player;  // список игроков
		uint32					self_id; // собственный номер в списке
		shared_ptr <CFontR>		font;	 // для отрисовки имени на сцене
	public:
		CPlayerManager(const CPlayerManager& src)			 = delete;
		CPlayerManager(CPlayerManager&& src)				 = delete;
		CPlayerManager& operator=(const CPlayerManager& src) = delete;
		CPlayerManager& operator=(CPlayerManager&& src)		 = delete;
	public:
		CPlayerManager() : self_id(SELF_PL_SLOT) { Reset(); }
		~CPlayerManager() { }
		void Close()
		{
			Reset();
		}

		CPlayerF * operator[](size_t idx) { return player[idx]; };

		auto begin() -> decltype(player.begin())
		{
			return player.begin();
		}
		auto end() -> decltype(player.end())
		{
			return player.end();
		}
		auto self() -> CPlayerF *
		{
			return player[self_id];
		}
		auto get(uint32 id) -> CPlayerF *
		{
			return player[id];
		}

	public:
		//>> Установка шрифта имени
		void SetFont(shared_ptr<CFontR> player_name_font)
		{
			font = player_name_font;
			for (auto cur : player)
				cur->status.SetFont(font.get());
		}
		//>> Установка информации о статусе сети (клиент/сервер)
		void SetStatusNetwork(const CStatusNetwork * pStatusNetwork)
		{
			network = const_cast<CStatusNetwork*>(pStatusNetwork);
		}
		//>> Данные о частоте обновления информации игроков от сервера
		void SetServerTick(const TickInterpolation * server_tick)
		{
			sv_tick = const_cast<TickInterpolation*>(server_tick);
		}

	public:
		//>> Меняет местами в списке игроков между собой
		void SwapID(uint32 player_id_0, uint32 player_id_1)
		{
			if (player_id_0 >= Count()) { _MBM(ERROR_IDLIMIT); return; }
			if (player_id_1 >= Count()) { _MBM(ERROR_IDLIMIT); return; }

			if (player_id_0 == player_id_1) return;
			
			player.Swap(player_id_0, player_id_1);
		}
		//>> Меняет местами в списке себя с указанным игроком
		void SwapSelfID(uint32 player_id)
		{
			SwapID(self_id, player_id);
			self_id = player_id;
		}
		//>> Сообщает собственный ID игрока
		uint32 GetSelfID()
		{
			return self_id;
		}

	public:
		//>> Устанавливает число слотов игроков до <count>
		void Set(uint32 count)
		{
			auto size = player.size();
			auto target = count;
			player.SetCreate(count);
			for (auto i = size; i < target; i++)
			{
				player[i]->status.SetFont(font.get());
			}
		}
		//>> Добавляет число слотов игроков на <count>
		void Add(uint32 count)
		{
			auto size = player.size();
			auto target = size + count;
			player.AddCreate(count);
			for (auto i = size; i < target; i++)
			{
				player[i]->status.SetFont(font.get());
			}
		}
		//>> Сообщает текущее число слотов игроков
		uint32 Count()
		{
			return (uint32)player.size();
		}
		//>> Возврат себя в слот по умолчанию и удаление слотов других игроков (за исключением зарезервированных мест)
		void Remove()
		{
			if (Count() < SELF_PL_SLOT + 1)
				Set(SELF_PL_SLOT + 1);

			SwapSelfID(SELF_PL_SLOT);
			player.Close(true, true, SELF_PL_SLOT + 1, Count());
		}
		//>> Полный сброс параметров (своих и зарезервированных) + удаление <Remove()> прочих слотов игроков 
		void Reset()
		{
			Remove();

			for (uint32 i=0; i < SELF_PL_SLOT + 1; i++)
			{
				player[i]->Reset();
				player[i]->status.SetFont(font.get());
			}

			self()->status.net.connected = true;
			self()->status.net.loaded    = true;
		}
		
	public:
		//>> TODO боты    Запускает бота на место игрока
		void RunBot(uint32 player_id)
		{
			if (player_id >= Count()) { _MBM(ERROR_IDLIMIT); return; }
			if (player_id == 0)       { _MBM(ERROR_IDRANGE); return; } // нельзя запустить вместо себя

			player[player_id]->status.net.connected = false;
			player[player_id]->status.net.loaded = false;
			player[player_id]->status.bot = true;
		}
		//>> Обновление в новый кадр
		void Update()
		{
			if (network == nullptr) { _MBM(ERROR_PointerNone); return; }
			if (sv_tick == nullptr) { _MBM(ERROR_PointerNone); return; }

			auto self_p      = self();
			auto self_camera = self_p->camera;
			auto server		 = network->Server();
			auto client		 = network->Client();

			float t = sv_tick->GetInterpolation();

			size_t count = player.size();
			for (size_t i=0; i<count; i++)
				if (i != self_id)
					 player[i]->Update(self_camera, t, false, server, client);
				else self_p->Update(self_camera, t, true, server, client);
		}
	};
}

#endif // _PLAYER_H