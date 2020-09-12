// ----------------------------------------------------------------------- //
//
// MODULE  : NetworkWS2.h
//
// PURPOSE : [UDP] Сервер / клиент : Windows Sockets 2 (Winsock2) API
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _NETWORKWS2_H
#define _NETWORKWS2_H

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "Ws2_32.lib")

#include "Network.h"
#include "NetworkServer.h"
#include "NetworkClient.h"

#define SZINADDR  sizeof(IN_ADDR)  // 4  FF.FF.FF.FF
#define SZINADDR6 sizeof(IN6_ADDR) // 16 FFFF:FFFF:FFFF:FFFF:FFFF:FF.FF.FF.FF:FFFF

namespace SAVFGAME
{
	class CNetworkWS2 final : public CNetwork
	{
		struct NETHELPER // содержит сгруппированные данные для работы c сетевыми адресами
		{
			NETREMOTEADDR	addr;			// remote net address common data
			sockaddr_in		fromV4;			// sockaddr_in   IPv4
			sockaddr_in6	fromV6;			// sockaddr_in6  IPv6
			sockaddr *		from_ptr;		// ptr to sockaddr_* IPv4 or IPv6
			int				from_len;		// size of sockaddr_* IPv4 or IPv6
			int *			from_len_ptr;	// ptr to size of sockaddr_* IPv4 or IPv6
			void *          sin_addr_ptr;	// ptr to IN_ADDR or IN6_ADDR (member of sockaddr_* IPv4 or IPv6)
			/////////////////////////////
			//>> Server update info about client's location after receiving new message
			void UpdateIPPORT()
			{
				switch (from_ptr->sa_family)
				{
				case AF_INET:  addr.family = V4FAMILY; break;
				case AF_INET6: addr.family = V6FAMILY; break;
				}

				if (addr.family == V4FAMILY)
				{
					addr.port = ntohs(fromV4.sin_port);
					inet_ntop(AF_INET, sin_addr_ptr, addr.ip, INET6_ADDRSTRLEN);

					addr.IPv6_scope_id = NULL;

					memcpy(addr.IPNET, sin_addr_ptr, SZINADDR);
				}
				else // V6FAMILY
				{
					addr.port = ntohs(fromV6.sin6_port);
					inet_ntop(AF_INET6, sin_addr_ptr, addr.ip, INET6_ADDRSTRLEN);

					addr.IPv6_scope_id = fromV6.sin6_scope_id;

					memcpy(addr.IPNET, sin_addr_ptr, SZINADDR6);
				}		
			}
			/////////////////////////////
			void copy(const NETHELPER& src)
			{
				addr     = src.addr;
				fromV4   = src.fromV4;
				fromV6   = src.fromV6;
				from_len = src.from_len;

				if (addr.family == V4FAMILY)
				{
					from_ptr = (sockaddr*) & fromV4;
					sin_addr_ptr = & fromV4.sin_addr;
				}
				else // V6FAMILY
				{
					from_ptr = (sockaddr*) & fromV6;
					sin_addr_ptr = & fromV6.sin6_addr;
				}

				from_len_ptr = & from_len;
			}
			NETHELPER(const NETHELPER& src)
			{
				copy(src);
			}
			NETHELPER& operator=(const NETHELPER& src)
			{
				if (this == &src) return *this;
				copy(src);
				return *this;
			}
			NETHELPER(NETHELPER&& src)            = delete;
			NETHELPER& operator=(NETHELPER&& src) = delete;
			NETHELPER(){};
		};
		struct SENDHELPER // содержит сгруппированные данные для помощи в отправке сообщения
		{
			sockaddr_in			  fromV4;
			sockaddr_in6		  fromV6;
			sockaddr_in  * const  fromV4_p;
			sockaddr_in6 * const  fromV6_p;
			int	const			  fromV4_len;
			int	const			  fromV6_len;
			void * const          sin4_addr_ptr;
			void * const          sin6_addr_ptr;
			SENDHELPER() : fromV4_p   (&fromV4),        fromV6_p   (&fromV6), 
				           fromV4_len (sizeof(fromV4)), fromV6_len (sizeof(fromV6)),
						   sin4_addr_ptr (&fromV4.sin_addr),
						   sin6_addr_ptr (&fromV6.sin6_addr)
			{}
			SENDHELPER(const SENDHELPER& src)				= delete;
			SENDHELPER(SENDHELPER&& src)					= delete;
			SENDHELPER& operator=(SENDHELPER&& src)			= delete;
			SENDHELPER& operator=(const SENDHELPER& src)	= delete;
		};
		struct LISTENHELPER // помогает в процедуре прослушивания входящих данных
		{
			LISTENHELPER(const LISTENHELPER& src)				= delete;
			LISTENHELPER(LISTENHELPER&& src)					= delete;
			LISTENHELPER& operator=(LISTENHELPER&& src)			= delete;
			LISTENHELPER& operator=(const LISTENHELPER& src)	= delete;
			LISTENHELPER()
			{
				message  = new NETMSG;
				h        = new NETHELPER;
				h_addr_p = & h->addr;
			}
			~LISTENHELPER()
			{
				_DELETE(message);
				_DELETE(h);
			}
			NETMSG *		message;  // received data
			NETHELPER *		h;		  // sender info
			NETREMOTEADDR *	h_addr_p; // ptr to sender's basic addr data
		};
	protected:
		SENDHELPER			sender;				// help with sending data
		WSADATA				wsaData;			// Winsock2 init data
		bool				isInit;				// WSAStartup() status
		bool				isServered;			// server socket status
		bool				isConnected;		// client socket status
	protected:
		SOCKET				sv_socket;			// server listening socket
		NETHELPER			sv_help;			// server info helper (content own address)
		LISTENHELPER		sv;					// server listener data (help manage incomings)
	protected:
		SOCKET				cl_socket;			// client listening socket
		NETHELPER			cl_help;			// client info helper (content remote server address)
		LISTENHELPER		cl;					// client listener data (help manage incomings)
	public:
		CNetServer			server;
		CNetClient			client;
	public:
		CNetworkWS2(const CNetworkWS2& src)					= delete;
		CNetworkWS2(CNetworkWS2&& src)						= delete;
		CNetworkWS2& operator=(CNetworkWS2&& src)			= delete;
		CNetworkWS2& operator=(const CNetworkWS2& src)		= delete;
	public:
		CNetworkWS2() : CNetwork(), isInit(false), isConnected(false), isServered(false),
			cl_socket(INVALID_SOCKET), sv_socket(INVALID_SOCKET)
		               { Init();  };
		~CNetworkWS2() override final { Close(); };

		void Close() override final
		{
			if (isInit)
			{
				server.Close(); //ServerShutdown();
				client.Close(); //ClientDisconnect();
				WSACleanup();
				isInit = false;
			}
		}

		void Init() override final
		{
			if (!isInit)
			{
				wchar_t error[128];
				int iResult;

				iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
				if (iResult != 0)
				{
					wsprintf(error, L"WSAStartup failed: %d", iResult);
					_MBM(error);
					return;
				}

				server.SetNetwork(this);
				client.SetNetwork(this);

				isInit = true;
			}
		}

		//>> Сообщает, готов ли клиентский сокет
		bool GetClientSocketStatus() override final
		{
			return isConnected;
		}
		//>> Сообщает, готов ли серверный сокет
		bool GetServerSocketStatus() override final
		{
			return isServered;
		}

	protected:
		//>> Отправка сообщения : готовый пакет (addr == nullptr если от клиента к серверу)
		bool Send(const NETMSG * message, const NETREMOTEADDR * addr) override final
		{
			// It is generally inadvisable for a broadcast datagram to exceed the size at which
			// fragmentation can occur, which implies that the data portion of the datagram
			// (excluding headers) should not exceed 512 bytes.

			bool client = (addr == nullptr); // отправитель : клиент (адресат : сервер)

			if (client && !isConnected) { printf("[WS-Client] Sending message failed (socket isn't ready)"); return false; }
			if (!client && !isServered) { printf("[WS-Server] Sending message failed (socket isn't ready)"); return false; }

			int iResult;
			int flags = 0; // MSG_PEEK ; MSG_OOB
			int msgsz = NETMSGHDSZ + message->header.length;

			if (client) iResult = sendto(cl_socket, (char*)message, msgsz, flags, cl_help.from_ptr, cl_help.from_len);
			else
			{
				sockaddr *		from_ptr;
				int             from_len;

				if (addr->family == V4FAMILY)
				{	
					from_ptr = (sockaddr*) sender.fromV4_p;
					from_len =             sender.fromV4_len;

					sender.fromV4.sin_family = AF_INET;
					sender.fromV4.sin_port   = htons(addr->port);
					//inet_pton(AF_INET, addr->ip, sender.sin4_addr_ptr);
					memcpy(sender.sin4_addr_ptr, addr->IPNET, SZINADDR);
				}
				else // V6FAMILY
				{
					from_ptr = (sockaddr*) sender.fromV6_p;
					from_len =             sender.fromV6_len;

					sender.fromV6.sin6_family = AF_INET6;
					sender.fromV6.sin6_port   = htons(addr->port);
					//inet_pton(AF_INET6, addr->ip, sender.sin6_addr_ptr);
					memcpy(sender.sin6_addr_ptr, addr->IPNET, SZINADDR6);

					sender.fromV6.sin6_scope_id = addr->IPv6_scope_id;
				}

				iResult = sendto(sv_socket, (char*)message, msgsz, flags, from_ptr, from_len);
			}

			if (iResult == SOCKET_ERROR)
			{ 
				auto error_code = WSAGetLastError();

				switch (error_code)
				{
				case WSAEWOULDBLOCK:
					{
						if (client) printf("[WS-Client] Sending message failed (error : %d)\n", error_code);
						else        printf("[WS-Server] Sending message failed (error : %d)\n", error_code);
									printf("            Resource temporarily unavailable\n");
						break;
					}
				case WSAEMSGSIZE:
					{
						if (client) printf("[WS-Client] Sending message failed (error : %d)\n", error_code);
						else        printf("[WS-Server] Sending message failed (error : %d)\n", error_code);
									printf("            Message too long\n");
						break;
					}
				default:
					{
						if (client) printf("\n[WS-Client] Sending message failed (error : %d)\n", error_code);
						else        printf("\n[WS-Server] Sending message failed (error : %d)\n", error_code);
						break;
					}
				}

				return false;
			}
			return true;
		}
		//>> Отправка сообщения : сырые данные  (addr == nullptr если от клиента к серверу)
		bool Send(const void * raw, int size, const NETREMOTEADDR * addr) override final
		{
			if (size > NETRAWSZ) { _MBM(L"[NETWORK] Exceeded size of RAWDATA"); return false; }

			netmsg->header.status = eNetStatus::NET_RAWDATA;
			netmsg->header.length = size;
			netmsg->header.count = 1;
			memcpy(netmsg->raw, raw, size);

			return Send(netmsg, addr);
		}
		//>> Отправка сообщения : только статус (addr == nullptr если от клиента к серверу)
		bool Send(eNetStatus status, const NETREMOTEADDR * addr) override final
		{
			netmsg->header.status = status;
			netmsg->header.length = 0;
			netmsg->header.count = 0;
			return Send(netmsg, addr);
		}

	protected:
		//>> Получает последнюю произошедшую ошибку
		eNetError HandleLastError(bool client)
		{
		//	https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms740668(v=vs.85).aspx#WSAEINTR

			eNetError ret = NET_ERR_OTHER;
			int error_code = WSAGetLastError();
			switch (error_code)
			{
			case WSAEWOULDBLOCK:
		//		printf("\nThe socket is marked as nonblocking and the recvfrom operation would block.");
				ret = NET_ERR_CONTINUE;
				break;
			case WSAECONNRESET:
				if (client) printf("\n[WS-Client] Server application is offline");
				ret = NET_ERR_DISCONNECT;
				break;
			case WSAEMSGSIZE:
				if (client) printf("\n[WS-Client] The message was too large and was truncated");
				else        printf("\n[WS-Server] The message was too large and was truncated");
				ret = NET_ERR_MSGLARGE;
				break;
			case WSAETIMEDOUT:
				if (client) printf("\n[WS-Client] The connection has been dropped, cause of a network down without notice");
				else        printf("\n[WS-Server] The connection has been dropped, cause of a network down without notice");
				break;
			case WSAENETRESET:
				if (client) printf("\n[WS-Client] For a datagram socket indicates that the time to live has expired");
				else        printf("\n[WS-Server] For a datagram socket indicates that the time to live has expired");
				break;		
			case WSAENOTSOCK:
				if (client) printf("\n[WS-Client] failed with error code = %d [WSAENOTSOCK]", error_code);
				else        printf("\n[WS-Server] failed with error code = %d [WSAENOTSOCK]", error_code);
				break;
			default:
				if (client) printf("\n[WS-Client] failed with error code = %d", error_code);
				else        printf("\n[WS-Server] failed with error code = %d", error_code);
				break;
			}
			return ret;
		}
		//>> Заполняет NETHELPER клиента/сервера (вызывать в конце инициализации)
		void HandleHelper(bool client, addrinfo * addr)
		{
			NETHELPER * h_ptr;
			uint16      family;

			switch (addr->ai_addr->sa_family)
			{
			case AF_INET:  family = V4FAMILY; break;
			case AF_INET6: family = V6FAMILY; break;
			}

			if (client)
			{
				h_ptr = &cl_help;
				if (family == V4FAMILY)
					 cl_help.fromV4 = *(sockaddr_in*) (addr->ai_addr);
				else cl_help.fromV6 = *(sockaddr_in6*)(addr->ai_addr);
			}
			else
			{
				h_ptr = &sv_help;
				if (family == V4FAMILY)
					 sv_help.fromV4 = *(sockaddr_in*) (addr->ai_addr);
				else sv_help.fromV6 = *(sockaddr_in6*)(addr->ai_addr);
			}

			NETHELPER & h  = * h_ptr      ;
			h.addr.family  =   family     ;
			h.from_len_ptr = & h.from_len ;

			if (family == V4FAMILY)
			{
				h.from_ptr = (sockaddr*) & h.fromV4  ;
				h.from_len = sizeof(h.fromV4)        ;
				h.sin_addr_ptr = & h.fromV4.sin_addr ;

				h.addr.port = ntohs(h.fromV4.sin_port);
				h.addr.IPv6_scope_id = NULL;
				memcpy(h.addr.IPNET, h.sin_addr_ptr, SZINADDR);

				inet_ntop(AF_INET, h.sin_addr_ptr, h.addr.ip, INET6_ADDRSTRLEN);
			}
			else // V6FAMILY
			{
				h.from_ptr = (sockaddr*) & h.fromV6   ;
				h.from_len = sizeof(h.fromV6)         ;
				h.sin_addr_ptr = & h.fromV6.sin6_addr ;

				h.addr.port = ntohs(h.fromV6.sin6_port);
				h.addr.IPv6_scope_id = h.fromV6.sin6_scope_id;
				memcpy(h.addr.IPNET, h.sin_addr_ptr, SZINADDR6);

				inet_ntop(AF_INET6, h.sin_addr_ptr, h.addr.ip, INET6_ADDRSTRLEN);
			}

			if (client) *cl.h = h;
			else        *sv.h = h;
		}

	protected:
		//>> Установка сокета для подключения к серверу
		bool ClientConnect(const char* ip, const char* port) override final
		{
			if (!isInit) { _MBM(ERROR_InitNone); return false; }
			if (isConnected) { ClientDisconnect(); }

			addrinfo *	cl_addr_result = nullptr;
			addrinfo	cl_addr_hints;

			int iResult;

			if (ip == NULL || ip == nullptr)
				ip = DEFAULT_IP;

			if (port == NULL || port == nullptr)
				port = DEFAULT_PORT;

			bool IPv6 = IsIPv6(ip);

			ZeroMemory(&cl_addr_hints, sizeof(cl_addr_hints));
			if (IPv6) cl_addr_hints.ai_family = AF_INET6;
			else      cl_addr_hints.ai_family = AF_INET;
			//cl_addr_hints.ai_family = AF_UNSPEC;
			cl_addr_hints.ai_socktype = SOCK_DGRAM;  // SOCK_STREAM, SOCK_DGRAM
			cl_addr_hints.ai_protocol = IPPROTO_UDP; // IPPROTO_TCP, IPPROTO_UDP
			cl_addr_hints.ai_flags = AI_ALL | AI_V4MAPPED;

			iResult = getaddrinfo(ip, port, &cl_addr_hints, &cl_addr_result);
			if (iResult != 0)
			{
				printf("\n[WS-Client] getaddrinfo() failed: %i", iResult);
				return false;
			}

			cl_socket = socket ( cl_addr_result->ai_family,
								 cl_addr_result->ai_socktype,
								 cl_addr_result->ai_protocol  );

			if (cl_socket == INVALID_SOCKET)
			{
				printf("\n[WS-Client] Init socket failed: %i", WSAGetLastError());
				freeaddrinfo(cl_addr_result);	cl_addr_result = nullptr;
				return false;
			}

			DWORD nonBlocking = 1;
			if (ioctlsocket(cl_socket, FIONBIO, &nonBlocking) != 0)
			{
				printf("\n[WS-Client] Failed to set non-blocking socket");
				return false;
			}

			ClientSocketSetOptions();

			// Необязательная часть, т.к. bind() всё равно прошёл бы автоматически при вызове sendto()
			// sendto() : If the socket is unbound, unique values are assigned to the local association by the system,
			// and the socket is then marked as bound. If the socket is connected, the getsockname function can
			// be used to determine the local IP address and port associated with the socket.
			// sendto() : If the socket is not connected, the getsockname function can be used to determine the local
			// port number associated with the socket but the IP address returned is set to the wildcard address for
			// the given protocol(for example, INADDR_ANY or "0.0.0.0" for IPv4 and IN6ADDR_ANY_INIT or "::" for IPv6).

			{
				addrinfo * self       = nullptr;
				addrinfo   self_hints = cl_addr_hints;
				self_hints.ai_flags |= AI_PASSIVE;
				if (!client_ip.size()) getaddrinfo(PASSIVE_ADDR, ANY_FREE_PORT, &self_hints, &self);
				else
				{
					if (IsIPv6(client_ip.c_str())) self_hints.ai_family = AF_INET6;
					else                           self_hints.ai_family = AF_INET;
					getaddrinfo(client_ip.c_str(), ANY_FREE_PORT, &self_hints, &self);
				}
				iResult = bind(cl_socket, self->ai_addr, (int)self->ai_addrlen);
				if (iResult == SOCKET_ERROR)
				{
					auto errcode = WSAGetLastError();
					switch (errcode)
					{
					case WSAENETUNREACH:   printf("\n[WS-Client] Bind socket failed: Network is unreachable"); break;
					case WSAENETDOWN:      printf("\n[WS-Client] Bind socket failed: Network is down");        break;	
					case WSAEADDRNOTAVAIL: printf("\n[WS-Client] Bind socket failed: Address not available");  break;
					case WSAEINVAL:        printf("\n[WS-Client] Bind socket failed: Invalid argument");       break;
					default:               printf("\n[WS-Client] Bind socket failed: ERR = %i", errcode);
					}
					closesocket(cl_socket);			cl_socket = INVALID_SOCKET;
					freeaddrinfo(cl_addr_result);	cl_addr_result = nullptr;		freeaddrinfo(self);
					return false;
				}
				//else ClientPrintf();
				freeaddrinfo(self);
			}

			// Необязательная часть для UDP, но проведём для дополнительной проверки

			iResult = connect(cl_socket, cl_addr_result->ai_addr, (int)cl_addr_result->ai_addrlen); 
			if (iResult == SOCKET_ERROR)
			{
				auto errcode = WSAGetLastError();
				switch (errcode)
				{
				case WSAENETUNREACH:   printf("\n[WS-Client] Connection to server failed: Network is unreachable"); break;
				case WSAENETDOWN:      printf("\n[WS-Client] Connection to server failed: Network is down");        break;
				case WSAEADDRNOTAVAIL: printf("\n[WS-Client] Connection to server failed: Address not available");  break;
				case WSAEINVAL:        printf("\n[WS-Client] Connection to server failed: Invalid argument");       break;
				default:               printf("\n[WS-Client] Connection to server failed: ERR = %i", errcode);
				}
				closesocket(cl_socket);			cl_socket = INVALID_SOCKET;
				freeaddrinfo(cl_addr_result);	cl_addr_result = nullptr;
				return false;
			}

			HandleHelper(true, cl_addr_result);
			freeaddrinfo(cl_addr_result);
			isConnected = true;

			printf("\n[WS-Client] Ready for connect to server %s:%d", cl_help.addr.ip, cl_help.addr.port);

		//	ClientSocketPrintf();

			return true;
		}
		//>> Проверка, что данные пришли от сервера, а не от неизвестного источника
		bool ClientCheckSender(NETHELPER * h)
		{
			NETHELPER & _ = * h;
			bool _ip, _port;

			if (cl_help.addr.family == AF_INET)
			{
				_ip = Compare((char*)cl_help.sin_addr_ptr, (char*)_.sin_addr_ptr, SZINADDR);
				_port = (cl_help.fromV4.sin_port == h->fromV4.sin_port);
			}
			else // AF_INET6
			{
				_ip = Compare((char*)cl_help.sin_addr_ptr, (char*)_.sin_addr_ptr, SZINADDR6);
				_port = (cl_help.fromV6.sin6_port == h->fromV6.sin6_port);
			}

			return (_ip && _port);
		}
		
	public:
		//>> Прослушивание сервера не дольше < milliseconds >
		bool ClientListen(uint32 milliseconds) override final
		{
			if (!isConnected) { return false; }

			int iResult;
			const int flags = 0; // MSG_PEEK ; MSG_OOB

			auto start = _TIME;		
			for (;;)
			{
				if (milliseconds < _TIMER(start)) break;

				iResult = recvfrom(cl_socket, (char*)cl.message, NETMSGSZ, flags, cl.h->from_ptr, cl.h->from_len_ptr);

				if (iResult == SOCKET_ERROR) // manage errors
				{
					auto errcode = HandleLastError(1);
					bool br = client.HandleError(errcode);
					if (br) break;
					else continue;
				}

				if (ClientCheckSender(cl.h) == false) continue;				// unknown sender
				if (cl.message->header.gameid != UNIQUE_GAME_ID) continue;	// unknown data

				if (iResult > 0)
				{
					//printf("\nCLIENT: Bytes received %d\n", iResult);
					client.HandleIncoming(cl.message, cl.h_addr_p);
				}
				else // iResult == 0 
				{
					printf("\n[WS-Client] Bytes received NONE\n");
				}			
			}

			return true;
		}
		
	protected:
		//>> debug
		void ClientPrintf()
		{
			// getpeername(cl_socket, ... );
			// For datagram sockets, only the address of a peer specified in a previous connect call will be returned.
			// Any address specified by a previous sendto() call will not be returned by getpeername.

			sockaddr s;
			int s_len;
		//	char addr[INET6_ADDRSTRLEN];
		//	getpeername(cl_socket, &s, &s_len);
			getsockname(cl_socket, &s, &s_len);

			// You specify INADDR_ANY rather than a specific IP address, so it binds to the wildcard(all interfaces)
			//    0.0.0.0.So, when you call getsockname() that's what you get back.
			// If you specified 0.0.0.0 as the IP address rather than INADDR_ANY you would get the same behavior;
			//    you will bind to all network interfaces on the machine.
			// For example, lets say you only have one network interface with the IP 192.168.1.12 assigned to it.
			//    You also have the loopback by default - 127.0.0.1
			// Using 0.0.0.0 or INADDR_ANY means you'll be bound to both those addresses, rather than a specific one.
			//    You will be able to connect to to your process via either IP.
			// If you were to bind to a specific IP rather than INADDR_ANY, your process would only listen on that IP
			//    and you'd get back that specific IP with getsockname().

			if (s.sa_family == AF_INET6)
			{
				//sockaddr_in6 * v6 = (sockaddr_in6*)&s
				sockaddr_in6 v6;
				getsockname(cl_socket, (sockaddr*)&v6, &s_len);
				//inet_ntop(AF_INET6, & v6->sin6_addr, addr, INET6_ADDRSTRLEN);
				//printf("\n[WS-Client] Listen at %s:%i", addr, ntohs(v6->sin6_port));
				printf("\n[WS-Client] Listen at port %i", ntohs(v6.sin6_port));			
			}
			else
			{
				//sockaddr_in * v4 = (sockaddr_in*)&s;
				sockaddr_in v4;
				getsockname(cl_socket, (sockaddr*)&v4, &s_len);
				//inet_ntop(AF_INET, & v4->sin_addr, addr, INET6_ADDRSTRLEN);
				//printf("\n[WS-Client] Listen at %s:%i", addr, ntohs(v4->sin_port));
				printf("\n[WS-Client] Listen at port %i", ntohs(v4.sin_port));
			}

		/*	WORD wVersionRequested;
			char name[255];
			PHOSTENT hostinfo;
			wVersionRequested = MAKEWORD(1, 1);
			char *ip;

			if (gethostname(name, sizeof(name)) == 0)
			{
				printf("Host name: %s\n", name);

				if ((hostinfo = gethostbyname(name)) != NULL)
				{
					int nCount = 0;
					while (hostinfo->h_addr_list[nCount])
					{
						ip = inet_ntoa(*(in_addr*)hostinfo->h_addr_list[nCount]);

						printf("IP #%d: %s\n", ++nCount, ip);
					}
				}
			} //*/
		}
		//>> debug
		void ClientSocketSetOptionsManageError(const char * info, int value)
		{
			int error = WSAGetLastError();
			switch (error)
			{
			case WSAEINVAL:
				printf("\n[WS-Client] %s: VAL %i, ERR = %i (WSAEINVAL)", info, value, error);
				break;
			default:
				printf("\n[WS-Client] %s: VAL %i, ERR = %i", info, value, error);
			}
		}
		//>> Настраивает сокет
		void ClientSocketSetOptions()
		{
			int default_send_buffer = 0;					int current_send_buffer = 0;
			int default_send_buffer_sz = sizeof(int);
			int default_recv_buffer = 0;					int current_recv_buffer = 0;
			int default_recv_buffer_sz = sizeof(int);

			getsockopt(cl_socket, SOL_SOCKET, SO_SNDBUF, (char*) & default_send_buffer, & default_send_buffer_sz);
			getsockopt(cl_socket, SOL_SOCKET, SO_RCVBUF, (char*) & default_recv_buffer, & default_recv_buffer_sz);

		//	printf("\n[WS-Client] /DEBUG/ Default SO_SNDBUF = %i (%X)", default_send_buffer, default_send_buffer);
		//	printf("\n[WS-Client] /DEBUG/ Default SO_RCVBUF = %i (%X)", default_recv_buffer, default_recv_buffer);

			int send_buffer = CL_SOCKET_SEND_BUFFER;
			int recv_buffer = CL_SOCKET_RECV_BUFFER;

			send_buffer = (send_buffer > default_send_buffer) ? send_buffer : default_send_buffer;
			recv_buffer = (recv_buffer > default_recv_buffer) ? recv_buffer : default_recv_buffer;

			if (setsockopt(cl_socket, SOL_SOCKET, SO_SNDBUF, (char*) & send_buffer, sizeof(send_buffer)) == SOCKET_ERROR)
				ClientSocketSetOptionsManageError("Set SO_SNDBUF failed", send_buffer);

			if (setsockopt(cl_socket, SOL_SOCKET, SO_RCVBUF, (char*) & recv_buffer, sizeof(recv_buffer)) == SOCKET_ERROR)
				ClientSocketSetOptionsManageError("Set SO_RCVBUF failed", recv_buffer);

			getsockopt(cl_socket, SOL_SOCKET, SO_SNDBUF, (char*) & current_send_buffer, & default_send_buffer_sz);
			getsockopt(cl_socket, SOL_SOCKET, SO_RCVBUF, (char*) & current_recv_buffer, & default_recv_buffer_sz);

			if (current_send_buffer != send_buffer) // Попробуем меньше
			for (;;) 
			{
				if (current_send_buffer != send_buffer) send_buffer -= 0x100; else break;
				if (send_buffer < default_send_buffer) break;

				if (setsockopt(cl_socket, SOL_SOCKET, SO_SNDBUF, (char*)& send_buffer, sizeof(send_buffer)) == SOCKET_ERROR)
					{}; // ClientSocketSetOptionsManageError("Set SO_SNDBUF failed", send_buffer);

				getsockopt(cl_socket, SOL_SOCKET, SO_SNDBUF, (char*) & current_send_buffer, & default_send_buffer_sz);
			}

			if (current_recv_buffer != recv_buffer) // Попробуем меньше
			for (;;) 
			{
				if (current_recv_buffer != recv_buffer) recv_buffer -= 0x100; else break;
				if (recv_buffer < default_recv_buffer) break;

				if (setsockopt(cl_socket, SOL_SOCKET, SO_RCVBUF, (char*) & recv_buffer, sizeof(recv_buffer)) == SOCKET_ERROR)
					{}; // ClientSocketSetOptionsManageError("Set SO_RCVBUF failed", recv_buffer);

				getsockopt(cl_socket, SOL_SOCKET, SO_RCVBUF, (char*) & current_recv_buffer, & default_recv_buffer_sz);
			}
		}
		//>> debug
		void ClientSocketPrintf()
		{
			// Difference between message-oriented protocols and stream-oriented protocols
			// Message Oriented protocols send data in distinct chunks or groups.
			// The receiver of data can determine where one message ends and another begins.
			// Stream protocols send a continuous flow of data.
			// Here is an example with mobile phones. Text messages would be a message oriented
			// protocol as each text message is distinct from the other messages.
			// A phone call is stream oriented as there is a continuous flow of audio throughout the call.
			// Common protocols used on the internet are UDP (message oriented) and TCP (stream oriented).

			// SOL_SOCKET Socket Options
			// https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms740532(v=vs.85).aspx

			TBUFFER <char, int> data;
			data.Create(256);

			// default : 65507   NOT SETTABLE

			getsockopt(cl_socket, SOL_SOCKET, SO_MAX_MSG_SIZE, data.buf, &data.count);							
			printf("\n[WS-Client] /DEBUG/ SO_MAX_MSG_SIZE = %i (%X)", *(DWORD*)data.buf, *(DWORD*)data.buf);

			// default : 8192

			getsockopt(cl_socket, SOL_SOCKET, SO_SNDBUF, data.buf, &data.count);								
			printf("\n[WS-Client] /DEBUG/ SO_SNDBUF = %i (%X)", *(DWORD*)data.buf, *(DWORD*)data.buf);

			// default : 8192

			getsockopt(cl_socket, SOL_SOCKET, SO_RCVBUF, data.buf, &data.count);								
			printf("\n[WS-Client] /DEBUG/ SO_RCVBUF = %i (%X)", *(DWORD*)data.buf, *(DWORD*)data.buf);

			
		}

	protected:
		//>> Закрытие сокета подключения
		void ClientDisconnectProc()
		{
			int iResult;

			iResult = shutdown(cl_socket, SD_BOTH); // SD_RECEIVE, SD_SEND, SD_BOTH
			if (iResult == SOCKET_ERROR)
				printf("[WS-Client] Shutdown listen socket with error = %i", WSAGetLastError());

			iResult = closesocket(cl_socket);
			if (iResult)
				printf("[WS-Client] Closed listen socket with error = %i", WSAGetLastError());

			cl_socket = INVALID_SOCKET;
		}
		//>> Закрытие сокета подключения
		void ClientDisconnect() override final
		{
			if (!isConnected) return;
			ClientDisconnectProc();
			isConnected = false;
		}

		//>> Установка сокета сервера
		bool ServerWakeup(const char * ip, const char * port, bool IPv6) override final
		{
			if (!isInit) { _MBM(ERROR_InitNone); return false; }
			if (isServered) { ServerShutdown(); }

			addrinfo *	sv_addr_result = nullptr;
			addrinfo	sv_addr_hints;

			int iResult;

			if (ip != NULL && ip != nullptr)
				IPv6 = IsIPv6(ip);

			if (port == NULL || port == nullptr)
				port = DEFAULT_PORT;

			ZeroMemory(&sv_addr_hints, sizeof(sv_addr_hints));
			if (IPv6) sv_addr_hints.ai_family = AF_INET6;
			else      sv_addr_hints.ai_family = AF_INET;
			sv_addr_hints.ai_socktype = SOCK_DGRAM;
			sv_addr_hints.ai_protocol = IPPROTO_UDP;
			if (ip == NULL || ip == nullptr)
				sv_addr_hints.ai_flags = AI_PASSIVE;

			iResult = getaddrinfo(ip, port, &sv_addr_hints, &sv_addr_result);
			if (iResult != 0)
			{
				printf("\n[WS-Server] getaddrinfo() failed: %i", iResult);
				return false;
			}

			sv_socket = socket ( sv_addr_result->ai_family,
								 sv_addr_result->ai_socktype,
								 sv_addr_result->ai_protocol  );

			if (sv_socket == INVALID_SOCKET)
			{
				printf("\n[WS-Server] Init socket failed: %i", WSAGetLastError());
				freeaddrinfo(sv_addr_result);	sv_addr_result = nullptr;
				return false;
			}

			DWORD nonBlocking = 1;
			if (ioctlsocket(sv_socket, FIONBIO, &nonBlocking) != 0)
			{
				printf("\n[WS-Server] Failed to set non-blocking socket");
				return false;
			}

			ServerSocketSetOptions();

			iResult = bind(sv_socket, sv_addr_result->ai_addr, (int)sv_addr_result->ai_addrlen);
			if (iResult == SOCKET_ERROR)
			{
				auto errcode = WSAGetLastError();
				switch (errcode)
				{
				case WSAEADDRINUSE:    printf("\n[WS-Server] Bind socket failed: Address already in use");  break;
				case WSAENETDOWN:      printf("\n[WS-Server] Bind socket failed: Network is down");         break;
				case WSAEADDRNOTAVAIL: printf("\n[WS-Server] Bind socket failed: Address not available");   break;
				case WSAEINVAL:        printf("\n[WS-Server] Bind socket failed: Invalid argument");        break;
				default:               printf("\n[WS-Server] Bind socket failed: ERR = %i", errcode);
				}
				closesocket(sv_socket);			sv_socket = INVALID_SOCKET;
				freeaddrinfo(sv_addr_result);	sv_addr_result = nullptr;
				return false;
			}

			HandleHelper(false, sv_addr_result);
			freeaddrinfo(sv_addr_result);
			isServered = true;

			printf("\n[WS-Server] Ready at %s:%d", sv_help.addr.ip, sv_help.addr.port);

		//	ServerSocketPrintf();

			return true;
		}

	public:
		//>> Прослушивание клиентов не дольше < milliseconds >
		bool ServerListen(uint32 milliseconds) override final
		{
			if (!isServered) { return false; }

			int iResult;
			const int flags = 0; // MSG_PEEK ; MSG_OOB

			auto start = _TIME;
			for (;;)
			{
				if (milliseconds < _TIMER(start)) break;

				iResult = recvfrom(sv_socket, (char*)sv.message, NETMSGSZ, flags, sv.h->from_ptr, sv.h->from_len_ptr);

				if (iResult == SOCKET_ERROR) // manage errors
				{
					auto errcode = HandleLastError(0);
					bool br = server.HandleError(errcode);
					if (br) break;
					else continue;
				}

				if (sv.message->header.gameid  != UNIQUE_GAME_ID)  continue; // unknown data

				sv.h->UpdateIPPORT(); // update sender info

				if (iResult > 0)
				{
					//printf("\nSERVER: Bytes received %d\n", iResult);
					server.HandleIncoming(sv.message, sv.h_addr_p);
				}
				else // iResult == 0
				{
					printf("\n[WS-Server] Bytes received NONE\n");
				}
			}

			return true;
		}

	protected:
		//>> debug
		void ServerSocketSetOptionsManageError(const char * info, int value)
		{
			int error = WSAGetLastError();
			switch (error)
			{
			case WSAEINVAL:
				printf("\n[WS-Server] %s: VAL %i, ERR = %i (WSAEINVAL)", info, value, error);
				break;
			default:
				printf("\n[WS-Server] %s: VAL %i, ERR = %i", info, value, error);
			}
		}
		//>> Настраивает сокет
		void ServerSocketSetOptions()
		{
			int default_send_buffer = 0;					int current_send_buffer = 0;
			int default_send_buffer_sz = sizeof(int);
			int default_recv_buffer = 0;					int current_recv_buffer = 0;
			int default_recv_buffer_sz = sizeof(int);

			getsockopt(sv_socket, SOL_SOCKET, SO_SNDBUF, (char*) & default_send_buffer, & default_send_buffer_sz);
			getsockopt(sv_socket, SOL_SOCKET, SO_RCVBUF, (char*) & default_recv_buffer, & default_recv_buffer_sz);

		//	printf("\n[WS-Server] /DEBUG/ Default SO_SNDBUF = %i (%X)", default_send_buffer, default_send_buffer);
		//	printf("\n[WS-Server] /DEBUG/ Default SO_RCVBUF = %i (%X)", default_recv_buffer, default_recv_buffer);

			int send_buffer = SV_SOCKET_SEND_BUFFER;
			int recv_buffer = SV_SOCKET_RECV_BUFFER;

			send_buffer = (send_buffer > default_send_buffer) ? send_buffer : default_send_buffer;
			recv_buffer = (recv_buffer > default_recv_buffer) ? recv_buffer : default_recv_buffer;

			if (setsockopt(sv_socket, SOL_SOCKET, SO_SNDBUF, (char*) & send_buffer, sizeof(send_buffer)) == SOCKET_ERROR)
				ServerSocketSetOptionsManageError("Set SO_SNDBUF failed", send_buffer);

			if (setsockopt(sv_socket, SOL_SOCKET, SO_RCVBUF, (char*) & recv_buffer, sizeof(recv_buffer)) == SOCKET_ERROR)
				ServerSocketSetOptionsManageError("Set SO_RCVBUF failed", recv_buffer);

			getsockopt(sv_socket, SOL_SOCKET, SO_SNDBUF, (char*) & current_send_buffer, & default_send_buffer_sz);
			getsockopt(sv_socket, SOL_SOCKET, SO_RCVBUF, (char*) & current_recv_buffer, & default_recv_buffer_sz);

			if (current_send_buffer != send_buffer) // Попробуем меньше
			for (;;) 
			{
				if (current_send_buffer != send_buffer) send_buffer -= 0x100; else break;
				if (send_buffer < default_send_buffer) break;

				if (setsockopt(sv_socket, SOL_SOCKET, SO_SNDBUF, (char*)& send_buffer, sizeof(send_buffer)) == SOCKET_ERROR)
					{}; // ServerSocketSetOptionsManageError("Set SO_SNDBUF failed", send_buffer);

				getsockopt(sv_socket, SOL_SOCKET, SO_SNDBUF, (char*) & current_send_buffer, & default_send_buffer_sz);
			}

			if (current_recv_buffer != recv_buffer) // Попробуем меньше
			for (;;) 
			{
				if (current_recv_buffer != recv_buffer) recv_buffer -= 0x100; else break;
				if (recv_buffer < default_recv_buffer) break;

				if (setsockopt(sv_socket, SOL_SOCKET, SO_RCVBUF, (char*) & recv_buffer, sizeof(recv_buffer)) == SOCKET_ERROR)
					{}; // ServerSocketSetOptionsManageError("Set SO_RCVBUF failed", recv_buffer);

				getsockopt(sv_socket, SOL_SOCKET, SO_RCVBUF, (char*) & current_recv_buffer, & default_recv_buffer_sz);
			}
		}
		//>> debug
		void ServerSocketPrintf()
		{
			// SOL_SOCKET Socket Options
			// https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms740532(v=vs.85).aspx

			TBUFFER <char, int> data;
			data.Create(256);

			// default : 65507   NOT SETTABLE

			getsockopt(sv_socket, SOL_SOCKET, SO_MAX_MSG_SIZE, data.buf, &data.count);
			printf("\n[WS-Server] /DEBUG/ SO_MAX_MSG_SIZE = %i (%X)", *(DWORD*)data.buf, *(DWORD*)data.buf);

			// default : 8192

			getsockopt(sv_socket, SOL_SOCKET, SO_SNDBUF, data.buf, &data.count);
			printf("\n[WS-Server] /DEBUG/ SO_SNDBUF = %i (%X)", *(DWORD*)data.buf, *(DWORD*)data.buf);

			// default : 8192

			getsockopt(sv_socket, SOL_SOCKET, SO_RCVBUF, data.buf, &data.count);
			printf("\n[WS-Server] /DEBUG/ SO_RCVBUF = %i (%X)", *(DWORD*)data.buf, *(DWORD*)data.buf);


		}

	protected:
		//>> Закрытие сокета сервера
		void ServerShutdownProc()
		{
			int iResult;

			iResult = closesocket(sv_socket);
			if (iResult)
				printf("\n[WS-Server] Closed listen socket with error = %i", WSAGetLastError());

			sv_socket = INVALID_SOCKET;
		}
		//>> Закрытие сокета сервера
		void ServerShutdown() override final
		{
			if (!isServered) return;
			ServerShutdownProc();
			isServered = false;
		}

		//>>
		void PrintWSAData()
		{
			printf("\niMaxSockets  %i"
				"\niMaxUdpDg    %i"
				"\nwHighVersion %04x"
				"\nwVersion     %04x"
				"\nszDescription  %s"
				"\nszSystemStatus %s"
				"\nlpVendorInfo   %llx"
				"\n",
				wsaData.iMaxSockets,
				wsaData.iMaxUdpDg,
				wsaData.wHighVersion,
				wsaData.wVersion,
				wsaData.szDescription,
				wsaData.szSystemStatus,
				(size_t)wsaData.lpVendorInfo);
		}

	};
}

#endif // _NETWORKWS2_H