// ----------------------------------------------------------------------- //
//
// MODULE  : NetworkServer.h
//
// PURPOSE : Сетевой обработчик серверной части
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _NETWORKSERVER_H
#define _NETWORKSERVER_H

#include "Network.h"
#include "NetworkGD.h"

#define  DEFAULT_SERVER_NAME  L"Default server name"

#define  DISCONNECT_REASON_SHUTDOWN  "Server has been shuted down"
#define  DISCONNECT_REASON_MAX_PL    "Max. players"
#define  DISCONNECT_REASON_LOST_PL   "Lost connection to player"
#define  DISCONNECT_REASON_GAM_DIF	 "Your version of game is different from the server's"
#define  DISCONNECT_REASON_MAP_DIF	 "Your version of map is different from the server's"

#define  MAX_PLAYERS      64                // default limit
#define  FIRST_PLAYER     SELF_PL_SLOT + 1  // other players placed in slots after self slot  (self is '0' as default)

namespace SAVFGAME
{
	class CNetServer
	{
	private:		
		struct
		{
			NETMSG _;
			NETMSG s;
			NETMSG t;
			NetConnectServerData	csd;
			NetObjectServerData		osd;
			NetPlayersAtServer		pas;
			NetPlayerServerData		psd;
		} _ ;
	protected:
		NetConnectServerData * const	csd    { &_.csd };
		NetObjectServerData  * const	osd    { &_.osd };
		NetPlayersAtServer   * const	pas    { &_.pas };
		NetPlayerServerData  * const	psd    { &_.psd };
		NETMSG * const					netmsg   { &_._ };	// other
		NETMSG * const					netmsg_s { &_.s };	// security data
		VECPDATA <NETMSG>				netmsg_p;			// players data
		VECPDATA <NETMSG>				netmsg_o;			// objects data
		NETMSG * const					netmsg_t { &_.t };	// text/raw data
		wstring							name;				// server public name
	protected:	
		CNetwork *					network { nullptr };	// ptr to net base
		CPlayerManager *			player  { nullptr };	// ptr to players
		CGameMapF *					map     { nullptr };	// ptr to current map
		CBaseCamera *				camera  { nullptr };	// ptr to own camera
		CPlayerF *					self_p  { nullptr };	// ptr to self player data
		CBaseInterface *			UI      { nullptr };	// ptr to UI	
	protected:	
		timepoint					ticktime;					 // time from last update (server -> clients)
		uint32						tick        { SERVER_TICK }; // ms, time between updates  (server -> clients)
		uint32						max_players { MAX_PLAYERS }; // limit of players served
		uint32						svmid		{ 0 };			 // last gamedata ID that has been sended to all players
		bool						active      { false };		 // server is ON / OFF
		bool						isInit      { false };
	public:
		CNetServer(const CNetServer& src)				= delete;
		CNetServer(CNetServer&& src)					= delete;
		CNetServer& operator=(CNetServer&& src)			= delete;
		CNetServer& operator=(const CNetServer& src)	= delete;
	public:
		CNetServer() { name = DEFAULT_SERVER_NAME; netmsg_p(1); netmsg_o(1); };
		~CNetServer() { Close(); };
		void Close()
		{
			if (!isInit) return;

			Shutdown();

			UI     = nullptr;
			map    = nullptr;
			player = nullptr;

			isInit = false;
		}
		
		wstring GetServerName() { return name; }

		//>> Возвращает <true>, если сервер включен
		bool Status()
		{
			return active;
		}
		//>> Сброс статусов
		void Reset()
		{
			active = false;
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
		//>> Имя сервера
		void SetServerName(const wstring & str)
		{
			name = str;
			if (name.size() > SV_NAME_MAX_LEN - 1)
				name.erase(name.begin() + (SV_NAME_MAX_LEN - 1), name.end());
		}

	protected:
		//>> Самопроверка на ошибки :: return init status <true> = fine, <false> = bad
		bool CheckErrors()
		{
			if (network == nullptr) { _MBM(ERROR_PointerNone); return isInit = false; }
			if (player  == nullptr) { _MBM(ERROR_PointerNone); return isInit = false; }
			if (map     == nullptr) { _MBM(ERROR_PointerNone); return isInit = false; }
			if (UI      == nullptr) { _MBM(ERROR_PointerNone); return isInit = false; }

			return isInit = true;
		}
		
	public:
		//>> Отключает игрока с указанием причины
		void DisconnectPlayer(uint32 id, char * reason)
		{
			if (id < FIRST_PLAYER || id >= max_players) return;

			auto p = player->get(id);
			if (!p->status.net.connected || p->status.bot) return;

			netmsg->ResetInfo();
			
			if (reason == nullptr) netmsg->header.length = 0;
			else
			{
				uint16 sz = (uint16)strlen(reason) + 1;

				netmsg->SetStatus(NET_DISCONNECT);
				netmsg->AddGameData(NET_GD_DISCONNECT_REASON, reason, sz);
			}

			network->Send(netmsg, p->status.net.addr);	
			//p->status.net.connected = false;
			//p->status.net.loaded    = false;
			//p->status.net.mid       = 0;
			p->status.Close();
		}
		//>> Устанавливает максимальное количество обслуживаемых игроков
		void SetPlayersLimit(uint32 count)
		{
			if (player == nullptr) { _MBM(ERROR_PointerNone); return; }

			if (count < FIRST_PLAYER) count = FIRST_PLAYER;
			if (count < max_players && active)
			{
				for (uint32 i = count; i < max_players; i++)
					DisconnectPlayer(i, "Server decreased max players");

				max_players = count;
				return;
			}
			else if (count == max_players) return;

			max_players = count;
			player->Set(max_players);
			return;
		}
		//>> Сообщает максимальное количество обслуживаемых игроков
		uint32 GetPlayersLimit()
		{
			return max_players;
		}
		
	public:
		//>> TODO
		void ReloadMap(){}

		//////////////////////////////////////////////////////

	public:
		//>> Запускает сервер
		bool Wakeup(const char * ip, const char * port, bool IPv6)
		{
			if (!CheckErrors()) return false;

			Shutdown(); // выключаем, если включен
			Reset();	// .

			if (network->ServerWakeup(ip, port, IPv6))
			{
				svmid                  = 0;
				active                 = true;
				ticktime               = _TIME;	
				network->status.client = false;
				network->status.server = true;
				Wakeup_Ex();
			}
			else active = false;

			return active;
		}
		//>> Выключает сервер
		void Shutdown()
		{
			if (!CheckErrors()) return;

			bool show_message = (active == true);

			if (active)
				for (uint32 i=FIRST_PLAYER; i<max_players; i++)
					DisconnectPlayer(i, DISCONNECT_REASON_SHUTDOWN);

			svmid  = 0;
			active = false;

			Shutdown_Ex();

			if (show_message) printf("\n[NetServer] Shutdown performed");
		}

	private:
		//>> Второстепенные действия при включении сервера
		void Wakeup_Ex()
		{
			player->Set(max_players);
			self_p->camera->MarkServerSelf(true);
			self_p->interact->MarkServerSelf(true);
			UI->chat_net.Close();
		}
		//>> Второстепенные действия при выключении сервера
		void Shutdown_Ex()
		{
			player->Remove();
			network->status.client = false;
			network->status.server = false;
			network->ServerShutdown();
			self_p->camera->MarkServerSelf(false);
			self_p->interact->MarkServerSelf(false);
			UI->chat_net.Close();
		}

		//////////////////////////////////////////////////////

	protected:
		//>> Клиент прислал NET_CONNECT
		void Handle_CL_CONNECT(NETMSG * message, NETREMOTEADDR * from)
		{
			CPlayerManager & p = *player;
			uint32 i, N = MISSING;

			if (message->header.version != PROTOCOL_VERSION) // different protocols -> disconnect
			{
				std::string reason = DISCONNECT_REASON_GAM_DIF;
				uint16 sz = (uint16)reason.size() + 1;

				netmsg->ResetInfo();
				netmsg->SetStatus(NET_DISCONNECT);
				netmsg->AddGameData(NET_GD_DISCONNECT_REASON, (void*)reason.c_str(), sz);

				network->Send(netmsg, from);
				return;
			}

			// Возможно игрок уже есть в списках -> проверим //

			for (i = FIRST_PLAYER; i < max_players; i++)
				if ( p[i]->status.net.connected &&
				    *p[i]->status.net.addr == *from )
					{ N = i; break; }

			// Если игрока нет в списках  -> найдём ему место //

			if (_ISMISS(N))
			for (i = FIRST_PLAYER; i < max_players; i++)
				if (p[i]->status.net.connected == false &&
					p[i]->status.bot           == false)
				{		
					auto & player = *p[i];
					player.Reset();

					GameDataReader(message, i);			 // Заполняем некоторые первоначальные данные

					player.status.net.connected = true;
					player.status.net.loaded    = false;
				//	player.status.bot           = false; // бот будет выключен   TODO когда бот будет реализован, всё перепроверить
					player.status.net.mid       = 0;
					*player.status.net.addr     = *from;

					auto & P = map->obj.player[DEFAULT_PLAYER_SPAWN]->pos->P;			// Установим начальную позицию игрока 
					auto & A = map->obj.player[DEFAULT_PLAYER_SPAWN]->pos->A;			// .
					auto & M = map->obj.player[DEFAULT_PLAYER_SPAWN]->camera_mode;		// .
					player.camera->ResetCamera(P, A, true);
					player.camera->SetCameraMode(M);
					player.status.camera.ResetMemoryQ(MATH3DQUATERNION(A));
					player.status.model.pos->P = P;
					player.status.model.pos->Q = MATH3DQUATERNION(A);

					N = i;
					wprintf(L"\n[NetServer] Player %i connected : %s", N, p[i]->status.name.data->text.c_str());
					 printf( "\n            %s:%i", from->ip, from->port);
					break;
				}

			netmsg->ResetInfo();

			// Если игроку нашлось место в списках //

			if (_NOMISS(N))
			{
				p[N]->status.net.time = _TIME;

				if (name.size() >= SV_NAME_MAX_LEN)
					name.erase(name.begin() + SV_NAME_MAX_LEN - 1, name.end());

				ZeroMemory(csd->server_name, SV_NAME_MAX_LEN * sizeof(wchar_t));
				memcpy(csd->server_name, name.c_str(), name.size() * sizeof(wchar_t));

				ZeroMemory(csd->map_name, MAP_NAME_MAX_LEN * sizeof(wchar_t));
				memcpy(csd->map_name, map->setup.mapname.c_str(), map->setup.mapname.size() * sizeof(wchar_t));

				csd->server_tick = tick;
				csd->max_players = max_players;
				csd->player_id   = N;

				csd->camera_fov  = camera->GetFOV();
				csd->camera_near = camera->GetNearPlane();
				csd->camera_far  = camera->GetFarPlane();

				auto & spawn = *map->obj.player[DEFAULT_PLAYER_SPAWN];
				csd->camera_reset.mode  = spawn.camera_mode;
				csd->camera_reset.pos   = spawn.pos->P;
				csd->camera_reset.angle = spawn.pos->A;

				netmsg->SetStatus(NET_CONNECT);
				netmsg->SetMessageID(++svmid);
				netmsg->AddGameData(NET_GD_STRUCT_CSD, (void*)csd, _SZTP(*csd));
			}
			else // мест нет 
			{
				char reason[sizeof(DISCONNECT_REASON_MAX_PL) + 128];
				sprintf_s(reason, "%s (%i)", DISCONNECT_REASON_MAX_PL, max_players);
				uint16 sz = (uint16)strlen(reason) + 1;

				netmsg->SetStatus(NET_DISCONNECT);
				netmsg->AddGameData(NET_GD_DISCONNECT_REASON, reason, sz);
			}

			network->Send(netmsg, from);
		}
		//>> Клиент прислал NET_DISCONNECT
		void Handle_CL_DISCONNECT(NETMSG * message, NETREMOTEADDR * from)
		{
			CPlayerManager & p = *player;
			uint32 i  = message->header.pid;

			if (i < max_players && *p[i]->status.net.addr == *from)
			{
				//p[i]->status.net.connected = false;
				//p[i]->status.net.mid = 0;
				p[i]->status.Close();
				printf("\n[NetServer] Player %i sent NET_DISCONNECT", i);
			}
		}
		//>> Клиент прислал NET_CHECK
		void Handle_CL_CHECK(NETMSG * message, NETREMOTEADDR * from)
		{
			CPlayerManager & p = *player;
			uint32 i = message->header.pid;

			if (i < max_players && *p[i]->status.net.addr == *from)
			{
				p[i]->status.net.time = _TIME;
				return;
			}
		}
		//>> Клиент прислал NET_LOADED
		void Handle_CL_LOADED(NETMSG * message, NETREMOTEADDR * from)
		{
			CPlayerManager & p = *player;
			uint32 i = message->header.pid;

			if (i < max_players && *p[i]->status.net.addr == *from)
			{
				if (p[i]->status.net.connected == true &&
				    p[i]->status.net.loaded == false)
				{
					void * data      = nullptr;
					uint16 data_len  = 0;
					auto   data_type = NET_GD_STRUCT_LCD;

					message->GetGameData(data_type, data, 0, data_len, NET_GET_PTR);

					NetLoadedClientData * cdata = (NetLoadedClientData*) data;

					if (cdata == nullptr)
					{
						printf("\n[NetServer] Player %i missing map crc32", i);
						DisconnectPlayer(i, DISCONNECT_REASON_MAP_DIF);
					}
					else if (cdata->map_crc32 != map->setup.crc32.m_crc32)
					{
						printf("\n[NetServer] Player %i has different map crc32", i);
						DisconnectPlayer(i, DISCONNECT_REASON_MAP_DIF);
					}
					else
					{
						p[i]->status.net.loaded = true;

						for (uint32 n = 0; n < max_players; n++) // Оставим запросы обновить между всеми личные данные
							if ( (p[n]->status.net.connected && p[n]->status.net.loaded) ||
								  p[n]->status.bot)
							{
								p[n]->status.name.NetRepeatSet(SEND_REPEAT_NUM); // Обновим все имена

								// ...
							}
					}

					p[i]->status.net.time = _TIME;
				}				
				return;
			}
		}
		//>> Клиент прислал NET_GAMEDATA
		void Handle_CL_GAMEDATA(NETMSG * message, NETREMOTEADDR * from)
		{
			CPlayerManager & p = *player;
			uint32 i = message->header.pid;
			uint32 ID = MISSING;

			if (i < max_players && *p[i]->status.net.addr == *from)
			{
				if (!p[i]->status.net.connected) return; // игрок найден, но числится отключенным
				ID = i;
			}

			if (_NOMISS(ID))
			{ 
				uint32 & old_id = p[ID]->status.net.mid;
				uint32 & new_id = message->header.mid;

				if (new_id < old_id) // слишком старое отбрасываем
				{
					if (new_id < 0x20000000 &&		// граничные условия нуля
						old_id > 0xE0000000) { }
					else return;
				}

				if (new_id != old_id) // новая итерация данных
				{
					old_id = new_id;
					p[ID]->status.net.time = _TIME;	// в принципе, практически тоже самое время в status.net.tick
				}

				p[ID]->status.net.tick.Update();

				GameDataReader(message, ID);
			}
		}

	public:
		//>> Обрабатывает входящие сообщения
		void HandleIncoming(NETMSG * message, NETREMOTEADDR * from)
		{
			auto status = message->header.status;
			switch (status)
			{
			case eNetStatus::NET_CONNECT:
				Handle_CL_CONNECT(message, from);
				break;
			case eNetStatus::NET_DISCONNECT:
				Handle_CL_DISCONNECT(message, from);
				break;
			case eNetStatus::NET_GAMEDATA:		
				Handle_CL_GAMEDATA(message, from);
				break;
			case eNetStatus::NET_CHECK:
			case eNetStatus::NET_LOADING:
				Handle_CL_CHECK(message, from);
				break;
			case eNetStatus::NET_LOADED:
				Handle_CL_LOADED(message, from);
				break;
			case eNetStatus::NET_RAWDATA:
			case eNetStatus::NET_ERROR:
			default:
				break;
			}
		};

		//////////////////////////////////////////////////////

	protected:
		//>> Обработка ошибки ERR_ ? ...
		void Handle_ERR_0(){}

	public:
		//>> Обрабатывает ошибки соединения :: return <true> for break listen loop or <false> to continue listen
		bool HandleError(eNetError error_code)
		{
			bool br = false;

			switch (error_code)
			{
			case NET_ERR_CONTINUE:   br = true;  break;
			case NET_ERR_DISCONNECT: br = true;  break;
			case NET_ERR_MSGLARGE:   br = false; break;
			default:                 br = false; break;
			}

			return br;
		}

		//////////////////////////////////////////////////////

	private:
		//>> Подготавливает защищённые данные
		void SendWorldStatusPrepareDataSecurity()
		{
			netmsg_s->ResetInfo();
			netmsg_s->SetStatus(NET_GAMEDATA);
			netmsg_s->SetMessageID(svmid);
			netmsg_s->SetFlags(NET_FLAG_SECURITY_DATA);
		}
		//>> Подготавливает данные об игроках
		void SendWorldStatusPrepareDataPlayers()
		{
			CPlayerManager & p = *player;

			for (auto message : netmsg_p) message->ResetInfo();

			uint16 jid = 0;
			uint16 jid_total = 1;

			netmsg_p[jid]->ResetInfo();
			netmsg_p[jid]->SetStatus(NET_GAMEDATA);
			netmsg_p[jid]->SetMessageID(svmid);
			netmsg_p[jid]->SetFlags(NET_FLAG_PLAYER_DATA);

			for (uint32 multiplier = 1 ; ; multiplier++)
			{
				if (_SZTP(*pas) + NETGMDHDSZ > NETRAWSZ - netmsg_p[jid]->header.length)
				{
					jid++;
					jid_total++;
					if (netmsg_p.size() < jid_total)
						netmsg_p.AddCreate(1);

					netmsg_p[jid]->SetStatus(NET_GAMEDATA);
					netmsg_p[jid]->SetFlags(NET_FLAG_OBJECT_DATA);
					netmsg_p[jid]->SetMessageID(svmid);
					netmsg_p[jid]->SetMessageJuniorID(jid);
				}

				pas->total      = max_players;
				pas->multiplier = multiplier;

				uint32 start  = (multiplier - 1) * PAS_MAX;
				uint32 target =  multiplier      * PAS_MAX;
				target = (target > max_players) ? max_players : target;

				uint32 i, n;

				for (n=0, i=start; i < target; i++, n++)
				{
					pas->info[n].connected = p[i]->status.net.connected;
					pas->info[n].loaded    = p[i]->status.net.loaded;
					pas->info[n].bot       = p[i]->status.bot;
				}

				netmsg_p[jid]->AddGameData(NET_GD_STRUCT_PAS, (void*)pas, _SZTP(*pas));

				if (target == max_players) break;
			}

			//////////////////////////////////////////////

			if (UI->chat_net.NEXT_ID) // собственное сообщение сервера
			{
				self_p->status.chat.NetSetMessage( UI->chat_net.message.c_str() );
				self_p->status.chat.ID      = UI->chat_net.ID;
				self_p->status.chat.NEXT_ID = true;
				UI->chat_net.NEXT_ID        = false;
			}

			//////////////////////////////////////////////

			for (uint32 i=0; i < max_players; i++)
			{
				auto & P = *p[i];
				if ( (P.status.net.connected && P.status.net.loaded) ||
					  P.status.bot )
				{
					if (_SZTP(*psd) + NETGMDHDSZ > NETRAWSZ - netmsg_p[jid]->header.length)
					{
						jid++;
						jid_total++;
						if (netmsg_p.size() < jid_total)
							netmsg_p.AddCreate(1);

						netmsg_p[jid]->SetStatus(NET_GAMEDATA);
						netmsg_p[jid]->SetFlags(NET_FLAG_OBJECT_DATA);
						netmsg_p[jid]->SetMessageID(svmid);
						netmsg_p[jid]->SetMessageJuniorID(jid);
					}

					psd->flag.Reset();

					psd->id              = i;
					psd->flag.update_pos = true;

					psd->camera_mode     = P.camera->GetCameraMode();
					psd->camera_position = * P.camera->GetCameraPos();
					psd->camera_lookat   = * P.camera->GetCameraLookAt();
					psd->camera_angle_z  = P.camera->GetAngleZ();

					psd->model_id        = P.status.model.id;
					psd->model_position  = P.status.model.pos->P;
					psd->model_rotation  = P.status.model.pos->Q;

					////////////////////////////////////////////////////////////////

					if (P.status.chat.NEXT_ID)
						P.status.chat.NetRepeatSet(SEND_REPEAT_NUM);

					if (P.status.chat.repeat) // запрос разослать данные
					{
						psd->flag.update_chat = true;
						psd->chat_message_id  = P.status.chat.ID;

						wstring & msg = P.status.chat.message_raw;
						if (msg.size() >= CHATMSGSZ)
							msg.erase(msg.begin() + CHATMSGSZ - 1, msg.end());

						ZeroMemory(psd->chat_message, CHATMSGSZ * sizeof(wchar_t));
						memcpy(psd->chat_message, msg.c_str(), msg.size() * sizeof(wchar_t));

						P.status.chat.NetRepeatSubOne(); // -1 запрос
					}

					////////////////////////////////////////////////////////////////

					if (P.status.name.NEXT_ID)
						P.status.name.NetRepeatSet(SEND_REPEAT_NUM);

					if (P.status.name.repeat) // запрос разослать данные
					{
						psd->flag.update_name = true;
						psd->name_id          = P.status.name.ID;

						auto & name = P.status.name.data->text;
						if (name.size() >= PLAYER_NAME_MAX_LEN)
							name.erase(name.begin() + PLAYER_NAME_MAX_LEN - 1, name.end());
						
						ZeroMemory(psd->name, PLAYER_NAME_MAX_LEN * sizeof(wchar_t));
						memcpy(psd->name, name.c_str(), name.size() * sizeof(wchar_t));

						P.status.name.NetRepeatSubOne(); // -1 запрос
					}

					////////////////////////////////////////////////////////////////

					netmsg_p[jid]->AddGameData(NET_GD_STRUCT_PSD, (void*)psd, _SZTP(*psd));
				}
			}

			for (jid = 0; jid < jid_total; jid++)
				netmsg_p[jid]->SetMessageJuniorTotal(jid_total);
		}
		//>> Подготавливает данные об объектах
		void SendWorldStatusPrepareDataObjects()
		{
			for (auto message : netmsg_o) message->ResetInfo();

			uint16 jid       = 0;
			uint16 jid_total = 1;

			netmsg_o[jid]->SetStatus(NET_GAMEDATA);
			netmsg_o[jid]->SetFlags(NET_FLAG_OBJECT_DATA);
			netmsg_o[jid]->SetMessageID(svmid);
			netmsg_o[jid]->SetMessageJuniorID(jid);

			osd->type    = OBJECT_MODEL;
			auto model   = map->obj.model;
			uint32 count = (uint32) model.size();

			for (uint32 i=0; i<count; i++)
			if (model[i]->enable && model[i]->dynamic)
			{
				if (_SZTP(*osd) + NETGMDHDSZ > NETRAWSZ - netmsg_o[jid]->header.length)
				{
					jid++;
					jid_total++;
					if (netmsg_o.size() < jid_total)
						netmsg_o.AddCreate(1);

					netmsg_o[jid]->SetStatus(NET_GAMEDATA);
					netmsg_o[jid]->SetFlags(NET_FLAG_OBJECT_DATA);
					netmsg_o[jid]->SetMessageID(svmid);
					netmsg_o[jid]->SetMessageJuniorID(jid);
				}

				auto wpos  = model[i]->pos;
				osd->id    = i;
				osd->pos   = wpos->P;
				osd->rot   = wpos->Q;
				osd->scale = wpos->S;

				netmsg_o[jid]->AddGameData(NET_GD_STRUCT_OSD, (void*)osd, _SZTP(*osd));
			}

			for (jid = 0; jid < jid_total; jid++)
				netmsg_o[jid]->SetMessageJuniorTotal(jid_total);
		}
		//>> Подготавливает текстовые данные
		void SendWorldStatusPrepareDataTexts()
		{
			netmsg_t->ResetInfo();
			netmsg_t->SetStatus(NET_GAMEDATA);
			netmsg_t->SetMessageID(svmid);
			netmsg_t->SetFlags(NET_FLAG_TEXT_DATA);
		}
		//>> Подготавливает рассылаемые данные
		void SendWorldStatusPrepareData()
		{
			++svmid;

			SendWorldStatusPrepareDataSecurity();
			SendWorldStatusPrepareDataPlayers();
			SendWorldStatusPrepareDataObjects();
			SendWorldStatusPrepareDataTexts();
				
		}

	public:
		//>> Рассылает игрокам статус игрового мира
		void SendWorldStatus()
		{
			if (!active) return; // server is offline
			
			if (_TIMER(ticktime) > tick)	// Высылаем обновление статуса мира только когда пришло время
				 ticktime = _TIME;			// .
			else return;					// .

			CPlayerManager & p = *player;

			SendWorldStatusPrepareData(); // подготовим, что мы собираемся разослать всем игрокам

			// ЗАМЕТКА
			// В будущем стоит для каждого игрока подготовить свои данные, в частности, данные по игрокам
			// Если один игрок не может видеть другого, не стоит передавать данные положения во избежание мошенничества

			for (uint32 i = FIRST_PLAYER; i < max_players; i++)
			{
				auto & status = p[i]->status;
				if (status.net.connected && !status.bot)
				{
					uint32 time = (uint32)_TIMER(status.net.time);

					if (time > TIMEOUT_DISCONNECT)
					{
						printf("\n[NetServer] Lost connection to player %i", i);
						DisconnectPlayer(i, DISCONNECT_REASON_LOST_PL);
						continue;
					}
					else if (time > TIMEOUT_TRY_RECONNECT)
					{
						network->Send(NET_CHECK, status.net.addr);
						continue;
					}
					else 
					{
						if (status.net.loaded) // игрок уже загрузился
						{
						//	network->Send(netmsg_s, status.net.addr);
						//	network->Send(netmsg_p, status.net.addr);
						//	network->Send(netmsg_o, status.net.addr);
						//	network->Send(netmsg_t, status.net.addr);

							uint32 i, count;

							count = netmsg_p[0]->header.mid_jr_total;
							for (i=0; i<count; i++)
								network->Send(netmsg_p[i], status.net.addr);

							count = netmsg_o[0]->header.mid_jr_total;
							for (i=0; i<count; i++)
								network->Send(netmsg_o[i], status.net.addr);
						}
					}
				}
			}
		}
		//>> (Во время загрузки) // TODO // Периодически повторяет отсылку загрузочных данных
		void SendAtLoading()
		{
			if (!active) return; // server is offline

			if (_TIMER(ticktime) > tick)
				 ticktime = _TIME;
			else return;

			// TODO ....
		}
		//>> (После загрузки) // TODO // Уведомляет о готовности
		void SendAfterLoading()
		{
			if (!active) return; // server is offline

			// TODO ....
		}

		//////////////////////////////////////////////////////

	protected:
		//>> Процедура обработки присланных игровых данных
		void GameDataReader(NETMSG * message, uint32 pid)
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
				case NET_GD_STRUCT_CCD: GameDataReader_ConnectClientData(data, size, pid); break;
				case NET_GD_STRUCT_PCD: GameDataReader_PlayerClientData(data, size, pid);  break;
				default: break;
				}
			}
			message->NextReset();
		}

	protected:
		void GameDataReader_ConnectClientData(void * data, uint16 size, uint32 pid)
		{
			CPlayerF *              P      = player->get(pid);
			CPlayerCameraF *		camera = P->camera;
			NetConnectClientData *	ccd    = reinterpret_cast <NetConnectClientData*> (data);

			camera->SetCameraMode  ( ccd->camera_mode   );
			camera->SetAspectRatio ( ccd->camera_aspect );

			P->status.net.tick.Reset(ccd->client_tick);

			uint32 mid = ccd->model_id;
			if (mid >= map->data.model.size())
				mid = map->GetDefaultModelID();

			auto model = map->data.model[mid].get();
			P->status.SetModel(mid, model->box.maxCoord);

			P->status.SetName(ccd->name, self_p->status.name.data->height);
		}
		void GameDataReader_PlayerClientData(void * data, uint16 size, uint32 pid)
		{
			CPlayerF *              P        = player->get(pid);
			CPlayerCameraF *		camera   = P->camera;
			CStatusIO *             IO       = P->IO;
			CBaseInteract *			interact = P->interact;
			NetPlayerClientData *	pcd      = reinterpret_cast <NetPlayerClientData*> (data);

			camera->SetCameraMode ( pcd->camera_mode     );
			camera->SetFlyMode    ( pcd->camera_fly_mode );
			camera->SetXZMoveFix  ( pcd->camera_xz_fix   );

			camera->SetMoveInputRight   ( pcd->camera_move_x );
			camera->SetMoveInputUp      ( pcd->camera_move_y );
			camera->SetMoveInputForward ( pcd->camera_move_z );
			camera->SetRollInput        ( pcd->camera_roll   );

		//	camera->SetCameraRotation ( pcd->camera_rotation );
			P->status.camera.MemoryQ(pcd->camera_rotation);

			if (pcd->camera_aspect != camera->GetAspectRatio())
				camera->SetAspectRatio ( pcd->camera_aspect );

			IO->mouse.client_x = pcd->IO_mouse_x;
			IO->mouse.client_y = pcd->IO_mouse_y;
			IO->window.client_width = pcd->IO_window_W;
			IO->window.client_height = pcd->IO_window_H;

			interact->MEM.SetAngles(pcd->interact_rotation_angles);
			interact->MEM.SetMove(pcd->interact_move_distance);

			P->status.evlock = pcd->event_locker;

			interact->SetRotate(pcd->event_model_rotate);
			interact->SetMove(pcd->event_model_move);

			//////////////////////////////////

			if (pcd->flag.update_chat)
			{
				P->status.chat.NetSetMessageID( pcd->chat_message_id ); // ID будет проверен, что он новый

				if (P->status.chat.NEXT_ID) // Если прошла проверка на новый ID
				{
					P->status.chat.NetSetMessage( pcd->chat_message );

					P->status.chat.NEXT_ID = true;					// Разошлём позже всем остальным
					P->status.chat.ID      = pcd->chat_message_id;

					UI->ChatHistoryAppend( P->status.chat.message, true, true );
				}
			}

			//////////////////////////////////

			if (pcd->flag.update_name)
			{
				P->status.name.NetSetID(pcd->name_id); // ID будет проверен, что он новый

				if (P->status.name.NEXT_ID) // Если прошла проверка на новый ID
				{
					P->status.SetName(pcd->name, self_p->status.name.data->height);
					P->status.name.NetRepeatSet(SEND_REPEAT_NUM);

					P->status.name.NEXT_ID = false;
					P->status.name.ID      = pcd->name_id;
				}
			}

			//////////////////////////////////
		}
	};
}

#endif // _NETWORKSERVER_H