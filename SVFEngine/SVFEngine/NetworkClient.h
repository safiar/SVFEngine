// ----------------------------------------------------------------------- //
//
// MODULE  : NetworkClient.h
//
// PURPOSE : Сетевой обработчик клиентской части
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _NETWORKCLIENT_H
#define _NETWORKCLIENT_H

#include "Network.h"
#include "NetworkGD.h"

#define  CONNECT_REQUEST_AGAIN_TIME  50 // ms, time between trying send connect message again

namespace SAVFGAME
{

	class CNetClient
	{
	private:
		struct
		{
			NETMSG					netmsg;
			NetPlayerClientData		pcd;
			NetConnectClientData	ccd;
			NetLoadedClientData		lcd;
		} _ ;
	protected:
		NETMSG * const					netmsg  { &_.netmsg }; // store temp message
		NetPlayerClientData * const		pcd     { &_.pcd    }; // store temp client player data
		NetConnectClientData * const	ccd     { &_.ccd    }; // store temp client connect data
		NetLoadedClientData * const		lcd		{ &_.lcd    }; // store temp client loaded data
	protected:
		CNetwork *				network { nullptr };	// ptr to net base
		CPlayerManager *		player  { nullptr };	// ptr to players
		CGameMapF *				map     { nullptr };	// ptr to current map
		CBaseCamera *			camera  { nullptr };	// ptr to own camera
		CPlayerF *				self_p  { nullptr };	// ptr to self player data
		CBaseInterface *		UI      { nullptr };	// ptr to UI
	protected:
		uint32				self_id   { SELF_PL_SLOT };	// player self id at server's list		
		uint32				clmid     { 0 };			// last gamedata ID that has been sended to server
		uint32				svmid     { 0 };            // last gamedata ID that recieved from server [any]
		uint32				svmid_s   { 0 };			// last gamedata ID that recieved from server [security]
		uint32				svmid_p   { 0 };			// last gamedata ID that recieved from server [players]
		uint32				svmid_o   { 0 };			// last gamedata ID that recieved from server [objects]
		uint32				svmid_t   { 0 };			// last gamedata ID that recieved from server [text/raw]		
	protected:
		timepoint			servertime;						// last seen
		timepoint			ticktime;						// time from last update (server <- client)
		uint32				tick         { CLIENT_TICK };	// ms, time between updates  (server <- client)
		EmpiricalTick		servertick_p;					// time between incoming players data (server -> client)
		EmpiricalTick		servertick_o;					// time between incoming objects data (server -> client)
	public:
		TickInterpolation * const	sv_tick_p { &servertick_p }; // provide interpolation value [players data]
		TickInterpolation * const	sv_tick_o { &servertick_o }; // provide interpolation value [objects data]
	protected:
		bool				connected	{ false };	// server confirmed connection [ NET_CONNECT ]
		bool				refused		{ false };	// server refused connection   [ NET_DISCONNECT ]
		bool				isInit		{ false };
	public:
		CNetClient(const CNetClient& src)				= delete;
		CNetClient(CNetClient&& src)					= delete;
		CNetClient& operator=(CNetClient&& src)			= delete;
		CNetClient& operator=(const CNetClient& src)	= delete;
	public:
		CNetClient() : servertick_p(SERVER_TICK), servertick_o(SERVER_TICK) {};
		~CNetClient() { Close(); };
		void Close()
		{
			if (!isInit) return;

			Disconnect();
			Reset();

			UI       = nullptr;
			map      = nullptr;
			camera   = nullptr;
			player   = nullptr;
			self_p   = nullptr;

			isInit = false;
		}

		//>> Возвращает <true>, если есть подтверждённое соединение с сервером
		bool Status()
		{
			return connected;
		}
		//>> Сброс статусов
		void Reset()
		{
			connected = false;
			refused   = false;
			self_id   = player->GetSelfID();
		}

	public:
		//>> Базовая работа с сетью
		void SetNetwork(const CNetwork * pNetwork)
		{
			if (pNetwork == nullptr) { _MBM(ERROR_PointerNone); return; }

			network = const_cast<CNetwork*>(pNetwork);
		}
		//>> Управление статусами игроков
		void SetPlayer(const CPlayerManager * pManager)
		{
			if (pManager == nullptr) { _MBM(ERROR_PointerNone); return; }

			player = const_cast<CPlayerManager*>(pManager);
			self_p = player->self();
			camera = self_p->camera;

			player->SetServerTick(sv_tick_p);
		}
		//>> Доступ к состоянию сцены
		void SetMap(const CGameMapF * pGamemap)
		{
			if (pGamemap == nullptr) { _MBM(ERROR_PointerNone); return; }

			map = const_cast<CGameMapF*>(pGamemap);
		}
		//>> Возможность вывода сообщений на экран
		void SetUI(const CBaseInterface * pUI)
		{
			if (pUI == nullptr) { _MBM(ERROR_PointerNone); return; }

			UI = const_cast<CBaseInterface*>(pUI);
		}

	protected:
		//>> Самопроверка на ошибки :: return init status <true> = fine, <false> = bad
		bool CheckErrors()
		{
			if (network == nullptr) { _MBM(ERROR_PointerNone); return isInit = false; }
			if (player  == nullptr) { _MBM(ERROR_PointerNone); return isInit = false; }
			if (map     == nullptr) { _MBM(ERROR_PointerNone); return isInit = false; }
			if (UI      == nullptr) { _MBM(ERROR_PointerNone); return isInit = false; }
			if (camera  == nullptr) { _MBM(ERROR_PointerNone); return isInit = false; }

			return isInit = true;
		}

		//////////////////////////////////////////////////////

	private:
		//>> Готовит исходные данные о себе для сервера
		void ConnectPrepare()
		{
			ccd->client_tick   = tick;
			ccd->model_id      = self_p->status.model.id;
			ccd->camera_aspect = camera->GetAspectRatio();
			ccd->camera_mode   = camera->GetCameraMode();

			if (ccd->camera_mode == CM_NOCONTROL) 
				ccd->camera_mode = camera->GetCameraPrevMode();

			ccd->name_id = self_p->status.name.ID;

			wstring & name = self_p->status.name.data->text;
			if (name.size() >= PLAYER_NAME_MAX_LEN)
				name.erase(name.begin() + PLAYER_NAME_MAX_LEN - 1, name.end());

			ZeroMemory(ccd->name, PLAYER_NAME_MAX_LEN * sizeof(wchar_t));
			memcpy(ccd->name, name.c_str(), name.size() * sizeof(wchar_t));

			netmsg->ResetInfo();
			netmsg->SetStatus(NET_CONNECT);
			netmsg->AddGameData(NET_GD_STRUCT_CCD, (void*)ccd, _SZTP(*ccd));
		}	
		
	public:
		//>> Пытается установить соединение в течение < milliseconds >
		bool Connect(const char* ip, const char* port, uint32 milliseconds)
		{
			if (!CheckErrors()) return false;

			Disconnect();		// разорвёт существующее соединение, если таковое есть
			Reset();			// .

			if (!network->ClientConnect(ip, port))									{
				printf("\n[NetClient] Can't resolve connection socket");
				return false;														}

			ConnectPrepare();

			if (!network->Send(netmsg, SERVER_ADDR))								{
				printf("\n[NetClient] Can't send connection message to server");
				return false;														}

			auto start = _TIME;
			auto again = _TIME;
			for (;;)
			{
				network->ClientListen(0);

				if (connected)													{
					printf("\n[NetClient] Connection to server confirmed");
					ticktime               = _TIME;
					network->status.client = true;
					network->status.server = false;
					Connect_Ex();
					return true;												}

				if (refused) break;
				if (milliseconds < _TIMER(start)) break;
				_SLEEP(1);
				if (CONNECT_REQUEST_AGAIN_TIME < _TIMER(again))
				{
					network->Send(netmsg, SERVER_ADDR);
					again = _TIME;
				}
			}

			network->ClientDisconnect();
			if (refused) printf("\n[NetClient] Connection to server failed");
			else         printf("\n[NetClient] Connection to server failed : timeout %i ms", milliseconds);
			return false;
		}
		//>> Разрывает существующее соединение
		void Disconnect()
		{
			if (!CheckErrors()) return;

			bool show_message = (connected == true);

			if (connected)
			{
				netmsg->ResetInfo();
				netmsg->SetStatus(NET_DISCONNECT);
				netmsg->SetPlayerID(self_id);

				network->Send(netmsg, SERVER_ADDR);
			}

			clmid   = 0;
			svmid   = 0;
			svmid_s = 0;
			svmid_p = 0;
			svmid_o = 0;
			svmid_t = 0;		

			connected = false;
			network->status.client = false;
			network->status.server = false;
			network->ClientDisconnect();

			if (show_message) printf("\n[NetClient] Connection closed");

			Disconnect_Ex();
		}
		
	protected:
		//>> Второстепенные действия при успешном соединении
		void Connect_Ex()
		{
			UI->NetPingResetTimer();
			UI->chat_net.Close();
		}
		//>> Второстепенные действия при разрыве соединения
		void Disconnect_Ex()
		{
			player->Remove();						// удаляем прочих игроков
			UI->EscapeMenuChangeConnectText(false);	// изменим текст подключения в UI
			UI->chat_net.Close();
		}

		//////////////////////////////////////////////////////

	protected:
		//>> Сервер прислал NET_CONNECT
		void Handle_SV_CONNECT(NETMSG * message, NETREMOTEADDR * from)
		{
			if (!connected) // не больше 1 раза
			{
				clmid   = 0;
				svmid   = message->header.mid - 1;
				svmid_s = message->header.mid - 1;
				svmid_p = message->header.mid - 1;
				svmid_o = message->header.mid - 1;
				svmid_t = message->header.mid - 1;
				GameDataReader(message);
			}

			connected  = true;
			servertime = _TIME;
		}
		//>> Сервер прислал NET_DISCONNECT
		void Handle_SV_DISCONNECT(NETMSG * message, NETREMOTEADDR * from)
		{
			void * reason      = nullptr;
			uint16 reason_len  = 0;
			auto   reason_type = NET_GD_DISCONNECT_REASON;

			message->GetGameData(reason_type, reason, 0, reason_len, NET_GET_PTR);

			printf("\n[NetClient] Server has closed connection with");

			if (reason != nullptr)
				 printf(" reason:\n            \"%s\"", reason);
			else printf(" no reason");
			
			Disconnect();
			refused = true;
		}
		//>> Сервер прислал NET_CHECK
		void Handle_SV_CHECK(NETMSG * message, NETREMOTEADDR * from)
		{
			servertime = _TIME;
		}
		//>> Сервер прислал NET_LOADED   // TODO //
		void Handle_SV_LOADED(NETMSG * message, NETREMOTEADDR * from)
		{
			// ? TODO ...

			servertime = _TIME;
		}
		//>> Сервер прислал NET_GAMEDATA
		void Handle_SV_GAMEDATA(NETMSG * message, NETREMOTEADDR * from)
		{
			uint32   old_id;
			uint32 & new_id = message->header.mid;

				 if (message->header.bSecurityData) old_id = svmid_s; // [security type]
			else if (message->header.bPlayerData)   old_id = svmid_p; // [players type]
			else if (message->header.bObjectData)   old_id = svmid_o; // [objects type]
			else if (message->header.bTextData)     old_id = svmid_t; // [text/raw type]
			else                                    old_id = svmid;   // [other]

			if (new_id < old_id) // слишком старое отбрасываем
			{
				if (new_id < 0x20000000 &&		// граничные условия нуля
					old_id > 0xE0000000) { }
				else return;
			}

			if (new_id != svmid) // новая итерация данных [any type]
			{
				UI->NetPingUpdate((uint32)_TIMER(servertime));
				servertime = _TIME;
				svmid      = new_id;
			}

			if (new_id != old_id) // новая итерация данных
			{
					 if (message->header.bPlayerData) servertick_p.Update();
				else if (message->header.bObjectData) servertick_o.Update();
			}

				 if (message->header.bSecurityData)	svmid_s = new_id;
			else if (message->header.bPlayerData)	svmid_p = new_id;
			else if (message->header.bObjectData)	svmid_o = new_id;
			else if (message->header.bTextData)		svmid_t = new_id;
			else									svmid   = new_id;

			GameDataReader(message);
		}

	public:
		//>> Обрабатывает входящие сообщения
		void HandleIncoming(NETMSG * message, NETREMOTEADDR * from)
		{
			auto status = message->header.status;
			switch (status)
			{
			case eNetStatus::NET_CONNECT:
				Handle_SV_CONNECT(message, from);
				break;
			case eNetStatus::NET_DISCONNECT:
				Handle_SV_DISCONNECT(message, from);
				break;
			case eNetStatus::NET_GAMEDATA:
				Handle_SV_GAMEDATA(message, from);
				break;
			case eNetStatus::NET_CHECK:
			case eNetStatus::NET_LOADING:
				Handle_SV_CHECK(message, from);
				break;
			case eNetStatus::NET_LOADED:
				Handle_SV_LOADED(message, from);
				break;
			case eNetStatus::NET_RAWDATA:
			case eNetStatus::NET_ERROR:
			default:
				break;
			}
		};

		//////////////////////////////////////////////////////

	protected:
		//>> Обработка ошибки ERR_DISCONNECT
		void Handle_ERR_DISCONNECT()
		{
			Disconnect();
			refused = true;
		}

	public:
		//>> Обрабатывает ошибки соединения :: return break-tip to listen loop
		bool HandleError(eNetError error_code)
		{
			bool br = false;

			switch (error_code)
			{
			case NET_ERR_CONTINUE:   br = true;  break;
			case NET_ERR_DISCONNECT: br = true;  Handle_ERR_DISCONNECT(); break;
			case NET_ERR_MSGLARGE:   br = false; break;
			default:                 br = false; break;
			}

			return br;
		}

		//////////////////////////////////////////////////////

	private:
		//>> Отсылает серверу статус игрока (тело процедуры)
		void SendPlayerStatusProc()
		{
			pcd->flag.Reset();

			pcd->camera_mode     = camera->GetCameraMode();
			pcd->camera_aspect   = camera->GetAspectRatio();
			pcd->camera_fly_mode = camera->GetFlyMode();
			pcd->camera_xz_fix   = camera->GetXZMoveFix();
			pcd->camera_move_x   = camera->GetMoveInputRight();
			pcd->camera_move_y   = camera->GetMoveInputUp();
			pcd->camera_move_z   = camera->GetMoveInputForward();
			pcd->camera_roll     = camera->GetRollInput();
			pcd->camera_rotation = *camera->GetCameraRotate();

			pcd->event_locker       = self_p->status.evlock;
			pcd->event_model_rotate = self_p->interact->GetRotate();
			pcd->event_model_move   = self_p->interact->GetMove();

			pcd->IO_mouse_x  = self_p->IO->mouse.client_x;
			pcd->IO_mouse_y  = self_p->IO->mouse.client_y;
			pcd->IO_window_W = self_p->IO->window.client_width;
			pcd->IO_window_H = self_p->IO->window.client_height;

			self_p->interact->MEM.GetMove   ( pcd->interact_move_distance   );
			self_p->interact->MEM.GetAngles ( pcd->interact_rotation_angles );

			////////////////////////////////////////////////////////////////

			if (UI->chat_net.NEXT_ID)
				UI->chat_net.NetRepeatSet(SEND_REPEAT_NUM);

			if (UI->chat_net.repeat) // запрос разослать данные
			{
				pcd->flag.update_chat = true;
				pcd->chat_message_id  = UI->chat_net.ID;

				wstring & msg = UI->chat_net.message;
				if (msg.size() >= CHATMSGSZ)
					msg.erase(msg.begin() + CHATMSGSZ - 1, msg.end());

				ZeroMemory(pcd->chat_message, CHATMSGSZ * sizeof(wchar_t));
				memcpy(pcd->chat_message, msg.c_str(), msg.size() * sizeof(wchar_t));

				UI->chat_net.NetRepeatSubOne(); // -1 запрос
			}

			////////////////////////////////////////////////////////////////

			if (self_p->status.name.NEXT_ID)
				self_p->status.name.NetRepeatSet(SEND_REPEAT_NUM);

			if (self_p->status.name.repeat) // запрос разослать данные
			{
				pcd->flag.update_name = true;
				pcd->name_id          = self_p->status.name.ID;

				auto & name = self_p->status.name.data->text;
				if (name.size() >= PLAYER_NAME_MAX_LEN)
					name.erase(name.begin() + PLAYER_NAME_MAX_LEN - 1, name.end());

				ZeroMemory(pcd->name, PLAYER_NAME_MAX_LEN * sizeof(wchar_t));
				memcpy(pcd->name, name.c_str(), name.size() * sizeof(wchar_t));

				self_p->status.name.NetRepeatSubOne(); // -1 запрос
			}

			////////////////////////////////////////////////////////////////

			netmsg->ResetInfo();
			netmsg->SetStatus(NET_GAMEDATA);
			netmsg->SetPlayerID(self_id);
			netmsg->SetMessageID(++clmid);
			netmsg->AddGameData(NET_GD_STRUCT_PCD, (void*)pcd, _SZTP(*pcd));

			network->Send(netmsg, SERVER_ADDR);
		}

	public:		
		//>> (Во время игры) Отсылает серверу данные игрока, проверяет активность подключения
		void SendPlayerStatus()
		{
			if (!connected) return; // player is offline

			if (_TIMER(ticktime) > tick)	// Высылаем серверу свой статус только когда пришло время
				ticktime = _TIME;			// .
			else
			{
				UI->NetPingShow();
				return;
			}

			uint32 time = (uint32)_TIMER(servertime);

			if (time > TIMEOUT_DISCONNECT)
			{
				printf("\n[NetClient] Lost connection to server");
				Disconnect();
				return;
			}
			else if (time > TIMEOUT_TRY_RECONNECT)
			{
				netmsg->ResetInfo();
				netmsg->SetStatus(NET_CHECK);
				netmsg->SetPlayerID( self_id ); // ( player->GetSelfID() );

				network->Send(netmsg, SERVER_ADDR);

				UI->NetPingLostConnection(TIMEOUT_DISCONNECT - time);
				return;
			}
			else
			{
				UI->NetPingShow();
				SendPlayerStatusProc();
			}
		}
		//>> (Во время загрузки) Поддерживает активность подключения
		void SendAtLoading()
		{
			if (!connected) return; // player is offline

			if (_TIMER(ticktime) > tick)
				 ticktime = _TIME;
			else return;

			netmsg->ResetInfo();
			netmsg->SetStatus(NET_LOADING);
			netmsg->SetPlayerID(self_id);

			network->Send(netmsg, SERVER_ADDR);

			servertime = _TIME; // считаем, что сервер работает
		}
		//>> (После загрузки) Уведомляет о готовности
		void SendAfterLoading()
		{
			if (!connected) return; // player is offline

			lcd->map_crc32 = map->setup.crc32.m_crc32;

			netmsg->ResetInfo();
			netmsg->SetStatus(NET_LOADED);
			netmsg->SetPlayerID(self_id);
			netmsg->AddGameData(NET_GD_STRUCT_LCD, (void*)lcd, _SZTP(*lcd));

			for (uint32 i=0; i<SEND_REPEAT_NUM; i++)
				network->Send(netmsg, SERVER_ADDR);

			servertick_p.Reset();
			servertick_o.Reset();
		}

		//////////////////////////////////////////////////////

	protected:
		//>> Процедура обработки присланных игровых данных
		void GameDataReader(NETMSG * message)
		{
			void * data;
			uint16 size;
			eNetGDType type;

			message->NextReset();
			for (;;)
			{
				data = nullptr;
				size = 0;
				type = NET_GD_ANY;

				if (!message->NextRead(type, data, size, NET_GET_PTR)) break;

				if (data == nullptr) continue;

				switch (type)
				{
				case NET_GD_STRUCT_CR:  GameDataReader_CameraReset(data, size);       break;
				case NET_GD_STRUCT_CSD: GameDataReader_ConnectServerData(data, size); break;
				case NET_GD_STRUCT_OSD: GameDataReader_ObjectServerData(data, size);  break;
				case NET_GD_STRUCT_PAS: GameDataReader_PlayersAtServer(data, size);   break;
				case NET_GD_STRUCT_PSD: GameDataReader_PlayerServerData(data, size);  break;
				default: break;
				}
			}
			message->NextReset();
		}
		
	protected:
		void GameDataReader_CameraReset(void * data, uint16 size)
		{
			NetCameraReset * cr = (NetCameraReset*) data;

			eCameraMode memory = camera->GetCameraMode();
			camera->ResetCamera(cr->pos, cr->angle, true);
			camera->SetCameraMode(memory);

			self_p->status.camera.ResetMemoryP(cr->pos);
			self_p->status.camera.ResetMemoryAz(cr->angle.z);

			if (cr->mode != CM_ENUM_MAX) camera->SetCameraMode(cr->mode);
		}
		void GameDataReader_ConnectServerData(void * data, uint16 size)
		{
			NetConnectServerData * csd = (NetConnectServerData*) data;

			wprintf(L"\nServer name = %s",    csd->server_name );
			wprintf(L"\nServer tick = %i ms", csd->server_tick );
			wprintf(L"\nMax players = %i",    csd->max_players );
			wprintf(L"\nSelf plr ID = %i",    csd->player_id   );
			wprintf(L"\nMap name    = %s",    csd->map_name    );

			self_id = csd->player_id;

			servertick_p.Reset(csd->server_tick);
			servertick_o.Reset(csd->server_tick);
			player->Set(csd->max_players);
			player->SwapSelfID(self_id);
			
			UI->LoadingCall(csd->map_name, DEFAULT_MAP_ID);

			if (self_p->camera->GetFOV() != csd->camera_fov)
				self_p->camera->SetFOV(csd->camera_fov);

			if (self_p->camera->GetNearPlane() != csd->camera_near)
				self_p->camera->SetNearPlane(csd->camera_near);

			if (self_p->camera->GetFarPlane() != csd->camera_far)
				self_p->camera->SetFarPlane(csd->camera_far);

			GameDataReader_CameraReset(&csd->camera_reset, _SZTP(csd->camera_reset));
		}
		void GameDataReader_ObjectServerData(void * data, uint16 size)
		{
			NetObjectServerData * osd = (NetObjectServerData*) data;

			switch (osd->type)
			{
			case OBJECT_MODEL:
				{
					auto model = map->obj.model[osd->id].get();
					model->MemoryP(osd->pos);
					model->MemoryQ(osd->rot);
					model->MemoryS(osd->scale);
					break;
				}
			}
		}
		void GameDataReader_PlayersAtServer(void * data, uint16 size)
		{
			NetPlayersAtServer * pas = (NetPlayersAtServer*) data;

			if (pas->total != player->Count())
				player->Set(pas->total);

			uint32 start  = (pas->multiplier - 1) * PAS_MAX;
			uint32 target =  pas->multiplier      * PAS_MAX;
			target = (target > pas->total) ? pas->total : target;

			auto & P = *player;
			uint32 i, n;
			for (n=0, i=start; i < target; i++, n++)
			{
				if (P[i]->status.net.connected && !pas->info[n].connected)
					P[i]->status.Close();				

				P[i]->status.net.connected = pas->info[n].connected;
				P[i]->status.net.loaded    = pas->info[n].loaded;
				P[i]->status.bot           = pas->info[n].bot;
			}
		}
		void GameDataReader_PlayerServerData(void * data, uint16 size)
		{
			NetPlayerServerData * psd = (NetPlayerServerData*) data;
			CPlayerF * P = player->get(psd->id);

			if (psd->flag.update_pos)
			{
				if (P == self_p)
				{
					if (camera->GetCameraMode() == psd->camera_mode)
					{
						if (camera->IsFirstPerson())
							 self_p->status.camera.MemoryP( psd->camera_position );
						else self_p->status.camera.MemoryP( psd->camera_lookat   );
					}

					self_p->status.camera.MemoryAz( psd->camera_angle_z );
				}
				else
				{
					// ...
				}

				//////////////////////////////////

				uint32 MID = P->status.model.id;
				if (__ISMISS(MID) || MID != psd->model_id)
				{
					MID = psd->model_id;
					auto model = map->data.model[MID].get();
					P->status.SetModel(MID, model->box.maxCoord);
				}

				if (P != self_p)
				{
					P->status.model.MemoryP( psd->model_position );
					P->status.model.MemoryQ( psd->model_rotation );
				}
			}

			//////////////////////////////////

			if (psd->flag.update_chat && P != self_p)
			{
				P->status.chat.NetSetMessageID( psd->chat_message_id ); // ID будет проверен, что он новый

				if (P->status.chat.NEXT_ID) // Если прошла проверка на новый ID
				{
					P->status.chat.NetSetMessage( psd->chat_message );

					P->status.chat.NEXT_ID = false;
					P->status.chat.ID      = psd->chat_message_id;

					UI->ChatHistoryAppend( P->status.chat.message, true, true );
				}
			}

			//////////////////////////////////

			if (psd->flag.update_name)
			{
				P->status.name.NetSetID(psd->name_id); // ID будет проверен, что он новый

				if (P->status.name.NEXT_ID) // Если прошла проверка на новый ID
				{
					P->status.SetName(psd->name, self_p->status.name.data->height);
					P->status.name.NetRepeatSet(0);

					P->status.name.NEXT_ID = false;
					P->status.name.ID      = psd->name_id;
				}
			}

			//////////////////////////////////
		}
	};
}

#endif // _NETWORKCLIENT_H