// ----------------------------------------------------------------------- //
//
// MODULE  : Network.h
//
// PURPOSE : Интерфейс класса сетевого взаимодействия
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _NETWORK_H						// IPv4 mandates a path MTU of at least 576 bytes, IPv6 of at least 1280 bytes
#define _NETWORK_H						// MTU Ethernet = 1500 bytes, MTU PPPoE = 1492 bytes
										// IPv4 header ~20 bytes, IPv6 header ~40 bytes  +  UDP header ~8 bytes
#include "stdafx.h"						// In the case of Ethernet, the IP packet will additionally be wrapped in a 
#include "StatusIO.h"					// MAC packet (14 byte header + 4 byte CRC) which will be embedded in an 
										// Ethernet frame (8 byte preamble sequence) :: 14 + 4 + 8 == 26 extra bytes
										// BASE MTU 1500 bytes = ~100 IP/UDP/MAC/? + ~100 MSG header + 1300 RAW gamedata

/*
#include "version.h"
#define PROTOCOL_VERSION  ( ((uint64)((uint16)VERSION_MAJOR)    << 48) | \
						    ((uint64)((uint16)VERSION_MINOR)    << 32) | \
						    ((uint64)((uint16)VERSION_REVISION) << 16) | \
							((uint64)((uint16)VERSION_BUILD)) )
*/

#include "version_s.h"
#define PROTOCOL_VERSION  ( ((uint64)((uint16)VERSION_MAJOR)    << 48) | \
						    ((uint64)((uint16)VERSION_MINOR)    << 32) | \
						    ((uint64)((uint16)VERSION_REVISION) << 16) )

#define DEFAULT_IP		"127.0.0.1"
#define DEFAULT_PORT	"39213"
#define ANY_FREE_PORT   "0"
#define NETRAWSZ		1300 			// maximum size of raw-part of network message 
#define NETMSGSZ		sizeof(NETMSG)  // size of entire network message
#define NETMSGHDSZ		sizeof(NETMSGH) // size of network message header
#define NETGMDHDSZ		sizeof(NETGMDH) // size of network game data header

#define CL_SOCKET_SEND_BUFFER  0x20000  // client socket sending storage memory
#define CL_SOCKET_RECV_BUFFER  0x20000  // client socket receiving storage memory

#define SV_SOCKET_SEND_BUFFER  0x100000  // server socket sending storage memory
#define SV_SOCKET_RECV_BUFFER  0x100000  // server socket receiving storage memory

#define UNIQUE_GAME_ID	0x42B97A01 // own net protocol security ID

#define V4FAMILY 4		// 4  net values
#define V6FAMILY 16		// 16 net values

#ifndef INET6_ADDRSTRLEN
	#define INET6_ADDRSTRLEN 65
#endif

#define  PASSIVE_ADDR  nullptr	// to bind socket passively
#define  SERVER_ADDR   nullptr	// if no address presented, CNetwork sending data to server

#define  CLIENT_LISTEN_TIME  4	// ms, maximum time to working on handling incoming messages
#define  SERVER_LISTEN_TIME  4	// ms, maximum time to working on handling incoming messages

#define  CONNECTION_TIME        500     // ms, trying to connect (client -> server)
#define  TIMEOUT_TRY_RECONNECT	2000	// ms, from this timepoint - trying reestablish connection
#define  TIMEOUT_DISCONNECT		12000	// ms, from this timepoint - drop connection

#define  SEND_REPEAT_NUM  25  // count of repeating messages for ensure that data delivered to host

#define  SERVER_UPDATES  66                    // total updates per second   (server  -> client)
#define  SERVER_TICK	 1000 / SERVER_UPDATES // ms, update periodicity     (server  -> client)

#define  CLIENT_UPDATES  66                    // total updates per second   (server <-  client)
#define  CLIENT_TICK     1000 / CLIENT_UPDATES // ms, update periodicity     (server <-  client)

namespace SAVFGAME
{
	enum eNetGDType // [uint16] идентификаторы типов отсылаемых игровых данных
	{
		NET_GD_NONE,

		NET_GD_DISCONNECT_REASON, // char *	  (server  -> client) server has disconnected client by this reason

		NET_GD_STRUCT_CR,	// NetCameraReset       (server  -> client) reset player's camera
		NET_GD_STRUCT_CCD,	// NetConnectClientData (server <-  client) client data via connection request 
		NET_GD_STRUCT_CSD,	// NetConnectServerData (server  -> client) server data via connection request 
		NET_GD_STRUCT_LCD,	// NetLoadedClientData  (server <-  client) client data after loading map
		NET_GD_STRUCT_PAS,	// NetPlayersAtServer	(server  -> client) base info about players at server
		NET_GD_STRUCT_PSD,	// NetPlayerServerData  (server  -> client) player's data (server side)
		NET_GD_STRUCT_PCD,  // NetPlayerClientData  (server <-  client) player's data (client side)
		NET_GD_STRUCT_OSD,	// NetObjectServerData  (server  -> client) object's data (server side)
		
		NET_GD_ANY,			// any type of game data

		NET_GD_ENUM_MAX
	};

	enum eNetStatus // идентификаторы типа сетевого сообщения
	{
		NET_NONE,

		NET_CONNECT,		// новое подключение
		NET_DISCONNECT,		// разрыв соединения
		NET_ERROR,			// внутренняя ошибка
		NET_RAWDATA,		// сырые данные :: непонятный хлам
		NET_GAMEDATA,		// игровые данные
		NET_CHECK,			// обновление соединения :: сообщает, что мы живы и на связи
		NET_LOADING,		// идёт загрузка ресурсов
		NET_LOADED,			// загрузка ресурсов завершена
	//	NET_INGAME,			// в процессе игры
		NET_STATUS,			// запрос общей информации (для сервера : имя сервера, количество игроков и др.)

		NET_ENUM_MAX
	};
	enum eNetError // идентификаторы сетевых ошибок
	{
		NET_ERR_NONE,

		NET_ERR_CONTINUE,	// нет входящих сообщений
		NET_ERR_DISCONNECT,	// нет связи с сервером
		NET_ERR_MSGLARGE,	// превышен размер сообщения
		NET_ERR_OTHER,		// необработанная ошибка

		NET_ERR_ENUM_MAX
	};
	enum eNetMsgGetType
	{
		NET_GET_COPY,				// copy data to memory at presented pointer
		NET_GET_COPY_WITH_ALLOC,	// allocate memory, return it's pointer, copy data to allocated memory
		NET_GET_PTR					// don't copy anything, just return pointer to data inside net message 
	};
	enum eNetMessageFlag
	{
		NET_FLAG_COMPRESSED    = 1 << 0,
		NET_FLAG_ENCRYPTED     = 1 << 1,
		NET_FLAG_PLAYER_DATA   = 1 << 2,
		NET_FLAG_OBJECT_DATA   = 1 << 3,
		NET_FLAG_TEXT_DATA     = 1 << 4,
		NET_FLAG_SECURITY_DATA = 1 << 5,
	};

#pragma pack(push, 1)
	struct NETMSGH // заголовок сетевого сообщения :: TIP контролировать размер не больше 100 байт
	{
		NETMSGH() : gameid(UNIQUE_GAME_ID), version(PROTOCOL_VERSION)
		{}
		uint32 const	gameid;		  // уникальный сетевой ID всех сообщений / ID игры
		uint64 const	version;	  // версия программы / протоколов общения
		eNetStatus		status;		  // базовая характеристика сообщения
		uint32			mid;		  // порядковый номер сообщения (для игровых данных)
		uint16			mid_jr;       // младший порядковый номер сообщения
		uint16			mid_jr_total; // всего младших сообщений с общим номером < mid >
		uint32			pid;		  // заявленный ID игрока (для сервера)
		uint16			length;		  // полная длина вложенных RAW данных
		uint16			count;		  // количество вложенных в RAW логических единиц данных
		union
		{	
			uint32		_reserved; // 32 флага
			struct
			{
				bool	bCompressed   : 1;	// TODO   в данный момент не реализовано
				bool	bEcrypted     : 1;	// TODO   в данный момент не реализовано
				bool	bPlayerData   : 1;
				bool	bObjectData   : 1;
				bool	bTextData     : 1;
				bool	bSecurityData : 1;
				// ...
			};		
		};
	};
	struct NETGMDH // заголовок пакета игровых данных
	{
		uint16	type;	// (eNetGDType) type of game data
		uint16	size;	// size of game data
	};
	struct NETMSG // сетевое сообщение
	{
	public:
		NETMSGH	header;				// данные к отправке
		char	raw [NETRAWSZ];		// данные к отправке
	private:
		NETGMDH _;					// helper
		NETGMDH * const gh { &_ };	// helper
		uint64  next_pos;			// текущая позиция при последовательном чтении
		bool	next_mode;			// метка последовательного чтения
		bool	next_have;			// метка продолжения последовательного чтения
	public:
		NETMSG() { Reset(); };
		~NETMSG() { };

		//>> Полная очистка памяти
		void Reset()
		{
			ResetInfo();
			ZeroMemory(raw, NETRAWSZ);
		}
		//>> Сброс информации о пакете
		void ResetInfo()
		{
			header.status       = NET_NONE;
			header.length       = 0;
			header.count        = 0;
			header.mid          = 0;
			header.mid_jr       = 0;
			header.mid_jr_total = 1;
			header.pid          = 0;
			header._reserved    = 0;
			next_pos            = 0;
			next_mode           = false;
			next_have           = false;
		}
		//>> Установка типа сетевого сообщения
		void SetStatus(eNetStatus status)
		{
			header.status = status;
		}
		//>> Установка заявленного ID игрока
		void SetPlayerID(uint32 player_id)
		{
			header.pid = player_id;
		}
		//>> Установка порядкового номера сообщения
		void SetMessageID(uint32 message_id)
		{
			header.mid = message_id;
		}
		//>> Установка младшего порядкового номера сообщения
		void SetMessageJuniorID(uint32 message_jr_id)
		{
			header.mid_jr= message_jr_id;
		}
		//>> Установка общего количества младших номеров
		void SetMessageJuniorTotal(uint32 message_jr_total)
		{
			header.mid_jr_total = message_jr_total;
		}
		//>> Установка булевых флагов
		void SetFlags(uint32 eNetMessageFlag)
		{
			if (eNetMessageFlag & NET_FLAG_COMPRESSED)    header.bCompressed   = true;
			if (eNetMessageFlag & NET_FLAG_ENCRYPTED)     header.bEcrypted     = true;
			if (eNetMessageFlag & NET_FLAG_PLAYER_DATA)   header.bPlayerData   = true;
			if (eNetMessageFlag & NET_FLAG_OBJECT_DATA)   header.bObjectData   = true;
			if (eNetMessageFlag & NET_FLAG_TEXT_DATA)     header.bTextData     = true;
			if (eNetMessageFlag & NET_FLAG_SECURITY_DATA) header.bSecurityData = true;
		}
		//>> Добавление игровых данных
		void AddGameData(eNetGDType type, void * data, uint16 size)
		{
			if (header.length + size + NETGMDHDSZ > NETRAWSZ) { _MBM(ERROR_NETBUFLIMIT); return; }

			gh->type = type;
			gh->size = size;

			memcpy(raw + header.length, gh, NETGMDHDSZ);
			header.length += NETGMDHDSZ;

			memcpy(raw + header.length, data, size);
			header.length += size;

			header.count++;
		}
		//>> Изъятие определённых игровых данных
		bool GetGameData(eNetGDType & type_in_out, void * & data_out, uint32 id, uint16 & data_size_out, eNetMsgGetType get_type)
		{
			if (!header.length || !header.count) return false;

			NETGMDH *  h;		
			char    *  p     = raw;
			char    *  start = p;
			char    *  end   = p + header.length;
			uint32     i     = 0;

			if (next_mode) // режим последовательного чтения
			{
				p += next_pos;		// go to last position
				start = p;			// mem where we are starting
				next_have = false;  // reset continue reading tip
			}

			for (; p < end ;)
			{
				h = reinterpret_cast <NETGMDH*> (p);
				if (type_in_out == NET_GD_ANY || h->type == type_in_out)
				{
					if (i != id) i++; // another requested, skip this
					else
					{
						if (next_mode) next_have = true; // NextRead() will return tip to continue reading

						if (h->size == 0) // exit from loop to FALSE CASE (there is nothing to return)
						{
							if (next_mode) next_pos += sizeof(NETGMDH); // update delta pos
							break;
						}

						////// TRUE CASE //////

						data_size_out = h->size; // return data size

						if (get_type == NET_GET_COPY_WITH_ALLOC) // allocate memory if requested
							data_out = malloc(h->size);

						if (get_type == NET_GET_PTR) 
							 data_out = p + sizeof(NETGMDH);					// return only pointer to data
						else memcpy(data_out, p + sizeof(NETGMDH), h->size);	// or copy data itself

						type_in_out = static_cast <eNetGDType> (h->type); // return type (in case of NET_GD_ANY)

						if (next_mode)
							next_pos += (p - start) + sizeof(NETGMDH) + h->size; // update delta pos

						return true;
					}
				}
				p += sizeof(NETGMDH) + h->size; // go next data
			}

			////// FALSE CASE //////

			if (next_mode) next_pos += (p - start); // update delta pos

			// <type_in_out> not updated

			data_size_out = 0; // nothing to return

			if (get_type == NET_GET_PTR ||				// carefully with <dest_out> if not requested to modify it's pointer
				get_type == NET_GET_COPY_WITH_ALLOC)
				data_out = nullptr;

			return false;
		}
		//>> Последовательное чтение :: return <true> = we can continue reading; <false> = we have reached end of message
		bool NextRead(eNetGDType & type_in_out, void * & data_out, uint16 & data_size_out, eNetMsgGetType get_type)
		{
			next_mode = true;
			GetGameData(type_in_out, data_out, 0, data_size_out, get_type);
			next_mode = false;

			return next_have;
		}
		//>> Сброс позиции на начало при последовательном чтении
		void NextReset()
		{
			next_pos = 0;
		}
		//>>
		void Printf()
		{
			int sz = header.length + NETMSGHDSZ;
			printf("\nMSG SZ = %i (%X)", sz, sz);
		}

	public:
		NETMSG(const NETMSG& src)				= delete;
		NETMSG(NETMSG&& src)					= delete;
		NETMSG& operator=(NETMSG&& src)			= delete;
		NETMSG& operator=(const NETMSG& src)	= delete;
	};	
#pragma pack(pop)

	///////// Структурное описание сетевого пакета /////////
	//
	// + NETMSGH          <offset = 0>
	//   - gameid
	//   - version
	//   - status
	//   - mid
	//   - pid
	//   - length
	//   - count
	//   - reserved
	// + NETGMDH [0]      <offset = sizeof(NETMSGH)>
	//   - ID
	//   - size
	// + RAW [0]          <offset = sizeof(NETMSGH) + sizeof(NETGMDH)>
	//   - <data>
	// + NETGMDH [1]      <offset = sizeof(NETMSGH) + sizeof(NETGMDH) + sizeof RAW[0]>
	//   - ID
	//   - size
	// + RAW [1]          <offset = ... >
	//   - <data>
	// + ...
	// + ...
	// + ...
	// + NETGMDH [count - 1]
	//   - ID
	//   - size
	// + RAW [count - 1]
	//   - <data>
	// + END              <offset = sizeof(NETMSGH) + NETMSGH::length>
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////

	struct TickInterpolation // предоставляет возможность узнать текущую степень интерполяции 
	{
	protected:
		TickInterpolation(){};
		~TickInterpolation(){};
	public:
		//>> Сообщает текущую степень интерполяции [0.f ... 1.f]
		virtual float GetInterpolation() = 0;
	};
	struct EmpiricalTick : public TickInterpolation // эмпирически вычисляет среднее время между приходом сетевых пакетов данных
	{
		EmpiricalTick(uint32 start_tick) : TickInterpolation() { Reset(start_tick); }
		~EmpiricalTick() { }
		void Reset(uint32 start_tick)
		{
			time             = start_tick;
			last_update_time = _TIME;
			counter          = 0;
			counter_time     = 0;
		}
		void Reset()
		{
			Reset(time);
		}
		void Update()
		{
			counter_time += (uint32)_TIMER(last_update_time);
			counter++;
			if (counter >= average)
			{
				time         = counter_time / average;
				counter      = 0;
				counter_time = 0;
			}
			last_update_time = _TIME;
		}
		void SetAverage(uint16 messages_count)
		{
			average = (messages_count > 0) ? messages_count : 1;
		}
		float GetInterpolation() override final
		{
			return _CLAMP ( (float) _TIMER(last_update_time) / time,
							0.f,
							1.f );
		}
	protected:		
		uint32		counter;          // счётчик обновления
		uint32		counter_time;     // накопитель времени обновления
		uint16		average { 10 };   // количество входящих сообщений для вычисления среднего времени
		timepoint	last_update_time; // время последнего обновления / время последнего принятого пакета
		uint32		time;             // эмпирически рассчитанное среднее время между приходом пакетов данных
	};
	struct NETREMOTEADDR // описывает удалённый адрес
	{
		char    ip [INET6_ADDRSTRLEN];	// IP as text
		uint16  port;					// port as value 
		uint16  family;					// V4FAMILY == 4  /  V6FAMILY == 16
		uint32  IPv6_scope_id;			// IPv6 extra value
		union
		{
			uint32 IPNETu32 [4];		// IP as net values [ (1) IPv4 in_addr / (4)  IPv6 in6_addr ]
			uint16 IPNETu16 [8];		// IP as net values [ (2) IPv4 in_addr / (8)  IPv6 in6_addr ]
			uint8  IPNET    [16];		// IP as net values [ (4) IPv4 in_addr / (16) IPv6 in6_addr ]
		};
		/////////////////////
		bool operator==(const NETREMOTEADDR & other)
		{
			bool _ip   = Compare(IPNET, other.IPNET,  family);
			bool _port = (port == other.port);
			return  (_ip && _port);
		}
		bool operator!=(const NETREMOTEADDR & other)
		{
			bool _ip   = Compare(IPNET, other.IPNET,  family);
			bool _port = (port == other.port);
			return  (!_ip || !_port);
		}
	};
	struct CStatusNetwork
	{
		friend class CNetServer;
		friend class CNetClient;
	private:
		bool server { false }; // server status ON / OFF
		bool client { false }; // client status of connection to any server
	public:
		//>> Check if we are server
		bool Server() { return server; }
		//>> Check if we are connected to other server
		bool Client() { return client; }
	};

	class CNetwork
	{
		friend class CNetClient;
		friend class CNetServer;
	public:
		CStatusNetwork	status;		// network status
		std::string		client_ip;	// additional, not necessary
	protected:
		NETMSG *		netmsg;		// store temp message
	public:
		CNetwork(const CNetwork& src)				= delete;
		CNetwork(CNetwork&& src)					= delete;
		CNetwork& operator=(CNetwork&& src)			= delete;
		CNetwork& operator=(const CNetwork& src)	= delete;
	public:
		CNetwork()
		{
			netmsg = new NETMSG;
			client_ip = "";
		};
		virtual ~CNetwork()
		{
			_DELETE(netmsg);
		};

		virtual void Close() = 0;
		virtual void Init()  = 0;

		virtual bool GetClientSocketStatus() = 0;
		virtual bool GetServerSocketStatus() = 0;

	protected:
		virtual bool ClientConnect(const char* ip, const char* port) = 0;
		virtual void ClientDisconnect()                              = 0;

		virtual bool ServerWakeup(const char * ip, const char * port, bool IPv6) = 0;
		virtual void ServerShutdown()                                            = 0;

	public:
		virtual bool ServerListen(uint32 milliseconds) = 0;
		virtual bool ClientListen(uint32 milliseconds) = 0;

	protected:
		virtual bool Send(const NETMSG * message, const NETREMOTEADDR * addr)     = 0;
		virtual bool Send(const void * raw, int size, const NETREMOTEADDR * addr) = 0;
		virtual bool Send(eNetStatus status, const NETREMOTEADDR * addr)          = 0;

	protected:
		//>> Проверка версии текстового представления адреса
		bool IsIPv6(const char* IP)
		{
			uint32 i, len = (uint32)strlen(IP);
			for (i = 0; i<len; i++)
				if (IP[i] == ':') return true;
			return false;
		}	
	};
}

#endif // _NETWORK_H