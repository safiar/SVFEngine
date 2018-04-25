// ----------------------------------------------------------------------- //
//
// MODULE  : NetworkWS2.h
//
// PURPOSE : [TCP] Сервер / клиент : Windows Sockets 2 (Winsock2) API
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _NETWORKWS2_H
#define _NETWORKWS2_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "stdafx.h"

#define DEFAULT_IP   "127.0.0.1"
#define DEFAULT_PORT "49213"
#define NETRAWSZ	0x2000					// (8192) maximum size of raw-part of network message 
#define NETMSGSZ	sizeof(NETMSG)			// maximum size of entire network message
#define NETMSGHDSZ	sizeof(NETMSGHEADER)	// size of network message header

//#define CNetworkF CNetworkWS2  // class final

namespace SAVFGAME
{
	enum eNetStatus
	{
		NET_NONE,

		NET_DISCONNECT,		// разрыв соединения
		NET_ERROR,			// внутренняя ошибка
		NET_MAX_USERS,		// достигнут максимум соединений к серверу
		NET_RAWDATA,		// игровые данные
		
		NET_ENUM_MAX
	};

#pragma pack(push, 1)
	struct NETMSGHEADER
	{
		eNetStatus	status;	// базовое сообщение
		uint32		length; // длина дополнительных <raw> данных
	};
	struct NETMSG
	{	
		NETMSG() { reset(); };
		void reset()
		{
			header.status = NET_NONE;
			header.length = 0;
			ZeroMemory(raw, NETRAWSZ);
		}
		////////////
		NETMSGHEADER	header;
		byte			raw[NETRAWSZ];
	};
#pragma pack(pop)

	class CNetworkWS2
	{
	protected:
	//	wchar_t				server_name[128];
		WSADATA				wsaData;			// хранит сведения об инициализации Windows Sockets 2
		bool				isInit;	
		bool				isServered;
		bool				isConnected;
		atomic <uint32>		max_users;			// limit for connections :: default is 0x7FFFFFFF (almost unlimited)
	protected:
		addrinfo *			sv_addr_result;		// https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.2.0/com.ibm.zos.v2r2.hala001/rexx_getaddrinfo_r.htm
		addrinfo			sv_addr_hints;
		SOCKET				sv_socket;			// server listening socket
		THREADDATA			sv_thread;			// server listening (waiting) thread
		atomic <uint32>		sv_active_clients;	// count of currently active connections
	protected:
		vector <SOCKET>		w_socket;			// clients socket data (shared)
		vector <uint64>		w_ticket;			// tickets from pool tasks
		THREADPOOL			w_pool;				// clients run through thread pool
		mutex				w_socket_m;			// protect shared socket data while working with it
		mutex				w_exit_m;			// synchronizes ServerShutdown() and self-closing attempts
	protected:
		addrinfo *			cl_addr_result;
		addrinfo			cl_addr_hints;
		SOCKET				cl_socket;			// client listening socket
		THREADDATA			cl_thread;			// client receiving (waiting) thread
		recursive_mutex		cl_exit_m;			// synchronizes closing connection attempts (in self-cause & server-cause case)
	protected:
		atomic <bool>		sv_exit; // server exit tip
		atomic <bool>		cl_exit; // client exit tip
	public:
		CNetworkWS2(const CNetworkWS2& src)				= delete;
		CNetworkWS2(CNetworkWS2&& src)					= delete;
		CNetworkWS2& operator=(CNetworkWS2&& src)		= delete;
		CNetworkWS2& operator=(const CNetworkWS2& src)	= delete;
	public:
		CNetworkWS2() : isInit(false), isConnected(false), isServered(false), sv_exit(false), cl_exit(false), sv_active_clients(0),
			cl_addr_result(nullptr), cl_socket(INVALID_SOCKET),
			sv_addr_result(nullptr), sv_socket(INVALID_SOCKET), max_users(0x7FFFFFFF)
		{
			Init();
		};
		~CNetworkWS2() { Close(); };

		void Close()
		{
			if (isInit)
			{
				ServerShutdown();
				ClientDisconnect();
				WSACleanup();
				isInit = false;
			}
		}

		void Init()
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

				isInit = true;
			}
		}

		bool IsIPv6(char* IP)
		{
			uint32 i, len = strlen(IP);
			for (i=0; i<len; i++)
				if (IP[i] == ':') return true;
			return false;
		}

		//>> Подключение к серверу
		bool ClientConnect(char* ip, char* port)
		{
			if (!isInit) { _MBM(ERROR_ClassNotInited); return false; }
			if (isConnected) { ClientDisconnect(); }

			wchar_t error[128];
			int iResult;

			if (ip == NULL || ip == nullptr)
				ip = DEFAULT_IP;

			if (port == NULL || port == nullptr)
				port = DEFAULT_PORT;

			bool IPv6 = IsIPv6(ip);

			ZeroMemory(&cl_addr_hints, sizeof(cl_addr_hints));
			cl_addr_hints.ai_family = AF_UNSPEC; // AF_UNSPEC , AF_INET (ipv4) , AF_INET6 (ipv6)
			cl_addr_hints.ai_socktype = SOCK_DGRAM;  // SOCK_STREAM, SOCK_DGRAM
			cl_addr_hints.ai_protocol = IPPROTO_UDP; // IPPROTO_TCP, IPPROTO_UDP
			cl_addr_hints.ai_flags = AI_ALL | AI_V4MAPPED;

			iResult = getaddrinfo(ip, port, &cl_addr_hints, &cl_addr_result);
			if (iResult != 0)
			{
				wsprintf(error, L"Client getaddrinfo failed: %d", iResult);
				_MBM(error);
				return false;
			}

			cl_socket = socket ( cl_addr_result->ai_family,
								 cl_addr_result->ai_socktype,
								 cl_addr_result->ai_protocol );

			/*	DWORD nonBlocking = 1;
			if (ioctlsocket(cl_socket, FIONBIO, &nonBlocking) != 0)
			{
				wprintf(error, "Failed to set non-blocking socket");
				_MBM(error);
				return false;
			} //*/

			if (cl_socket == INVALID_SOCKET)
			{
				wsprintf(error, L"Init cl_socket failed: %ld", WSAGetLastError());
				_MBM(error);
				freeaddrinfo(cl_addr_result);	cl_addr_result = nullptr;
				return false;
			}

			iResult = connect(cl_socket, cl_addr_result->ai_addr, (int)cl_addr_result->ai_addrlen);
			if (iResult == SOCKET_ERROR)
			{
				printf("\nConnection to server failed: %ld\n", WSAGetLastError());
				closesocket(cl_socket);			cl_socket = INVALID_SOCKET;
				freeaddrinfo(cl_addr_result);	cl_addr_result = nullptr;
				return false;
			}

			////// Запуск потока ожидания ответа сервера //////

			isConnected = true;
			if (cl_thread(&CNetworkWS2::ClientListen, this) != TC_SUCCESS)
			{
				closesocket(cl_socket);			cl_socket = INVALID_SOCKET;
				freeaddrinfo(cl_addr_result);	cl_addr_result = nullptr;
				isConnected = false;

				_MBM(L"[NETWORK] Client listen thread unexpectedly failed at start");

				return false;
			}

			printf("\nSuccessfully connected to server %s:%s\n", ip, port);

			return true;
		}

	protected:
		//>> Поток прослушивания сервера
		void ClientListen()
		{
			NETMSG message;
			char * message_ptr = (char*)&message;

			int iResult;

			do
			{
				iResult = recv(cl_socket, message_ptr, NETMSGSZ, 0);

				if (iResult > 0)
				{
					//printf("\nCLIENT: Bytes received %d\n", iResult);

					if (message.header.status == NET_DISCONNECT)
					{
						printf("\nCLIENT: Server disconnected\n");
						break;
					}
					else if (message.header.status == NET_RAWDATA)
					{
						// обработка игровых данных
					}
				}
				else if (iResult == 0)
				{
					printf("\nCLIENT: Server losted\n");
					break;
				}
				else // loop error
				{
					if (_ATM_LD(cl_exit)) // user disconnect himself
					{
						printf("\nCLIENT: Interrupt listening thread...\n");
						break;
					}

					auto error_code = WSAGetLastError();
					switch (error_code)
					{
					case WSAECONNRESET: printf("\nCLIENT: Connection reseted by server\n"); break;
					default:			printf("\nCLIENT: Recv() failed %d\n", error_code); break;
					}
					break;
				}
			} while (iResult > 0);

			printf("\nCLIENT: Listen thread shuted down...\n");

			ClientDisconnectProc();
		}

		//>> Отключение от сервера
		void ClientDisconnectProc()
		{
			rulock exit_lock (cl_exit_m);	

			if (!isConnected) return;

			wchar_t error[128];
			int iResult;

			iResult = shutdown(cl_socket, SD_BOTH);
			if (iResult == SOCKET_ERROR)
			{
				wsprintf(error, L"Client disconnect failed: %d\n", WSAGetLastError());
				_MBM(error);
			}

			freeaddrinfo(cl_addr_result); cl_addr_result = nullptr;
			closesocket(cl_socket);       cl_socket = INVALID_SOCKET;

			isConnected = false;
		}

	public:
		//>> Отключение от сервера
		void ClientDisconnect()
		{
			rulock exit_lock (cl_exit_m);

			if (!isConnected) return;

			NETMSG message;
			char * message_ptr = (char*)&message;

			message.header.status = NET_DISCONNECT;
			message.header.length = 0;
			send(cl_socket, message_ptr, NETMSGHDSZ, 0); // Сообщаем серверу о разрыве соединения (не обязательно)

			_ATM_1(cl_exit);
			ClientDisconnectProc();		// Закрываемся
			exit_lock.unlock();			// Даём зелёный свет ClientListen()
			cl_thread.Join(true);		// Ждём окончания ClientListen()
			_ATM_0(cl_exit);
		}

		//>> Включение сервера
		bool ServerWakeup(char * ip, char * port, bool IPv6)
		{
			wchar_t error[128];
			int iResult;

			if (ip != NULL && ip != nullptr)
				IPv6 = IsIPv6(ip);

			if (port == NULL || port == nullptr)
				port = DEFAULT_PORT;

			ZeroMemory(&sv_addr_hints, sizeof(sv_addr_hints));
			if (IPv6) sv_addr_hints.ai_family = AF_INET6;
			else      sv_addr_hints.ai_family = AF_INET;
			sv_addr_hints.ai_socktype = SOCK_STREAM; // SOCK_STREAM, SOCK_DGRAM
			sv_addr_hints.ai_protocol = IPPROTO_TCP; // IPPROTO_TCP, IPPROTO_UDP
			sv_addr_hints.ai_flags = AI_PASSIVE;

			iResult = getaddrinfo(ip, port, &sv_addr_hints, &sv_addr_result);
			if (iResult != 0)
			{
				wsprintf(error, L"Server getaddrinfo failed: %d\n", iResult);
				_MBM(error);
				return false;
			}

			sv_socket = socket ( sv_addr_result->ai_family,
								 sv_addr_result->ai_socktype,
								 sv_addr_result->ai_protocol  );

		/*	DWORD nonBlocking = 1;
			if (ioctlsocket(sv_socket, FIONBIO, &nonBlocking) != 0)
			{
				wprintf(error, "Failed to set non-blocking socket");
				_MBM(error);
				return false;
			} //*/

			if (sv_socket == INVALID_SOCKET)
			{
				wsprintf(error, L"Init sv_socket failed: %ld\n", WSAGetLastError());
				_MBM(error);
				freeaddrinfo(sv_addr_result);	sv_addr_result = nullptr;
				return false;
			}

			iResult = bind(sv_socket, sv_addr_result->ai_addr, (int)sv_addr_result->ai_addrlen);
			if (iResult == SOCKET_ERROR)
			{
				wsprintf(error, L"Bind sv_socket failed: %d\n", WSAGetLastError());
				_MBM(error);
				closesocket(sv_socket);			sv_socket = INVALID_SOCKET;
				freeaddrinfo(sv_addr_result);	sv_addr_result = nullptr;
				return false;
			}

			iResult = listen(sv_socket, SOMAXCONN);
			if (iResult == SOCKET_ERROR) {
				wsprintf(error, L"Listen sv_socket failed: %ld\n", WSAGetLastError());
				_MBM(error);
				closesocket(sv_socket);			sv_socket = INVALID_SOCKET;
				freeaddrinfo(sv_addr_result);	sv_addr_result = nullptr;
				return false;
			}

			////// Запуск потока прослушивания //////

			isServered = true;
			if (sv_thread(&CNetworkWS2::ServerListen, this) != TC_SUCCESS)
			{
				closesocket(sv_socket);			sv_socket = INVALID_SOCKET;
				freeaddrinfo(sv_addr_result);	sv_addr_result = nullptr;
				isServered = false;

				_MBM(L"[NETWORK] Server listen thread unexpectedly failed at start");

				return false;
			}

			return true;
		}

	protected:
		//>> Поток прослушивания
		void ServerListen()
		{
			wchar_t error[128];
			int iResult;

			NETMSG message;
			char * message_ptr = (char*)&message;
			SOCKET socket = INVALID_SOCKET; // temp socket

			for (;;)
			{
				socket = accept(sv_socket, NULL, NULL); // wait for new connection

				if (_ATM_LD(sv_exit)) break; // server is shutting down...

				if (socket != INVALID_SOCKET)
				{
					printf("\nSERVER: Incoming connection!\n");

					if (_ATM_LD(sv_active_clients) == _ATM_LD(max_users))
					{
						message.header.length = 0;
						message.header.status = NET_MAX_USERS;
						send(socket, message_ptr, NETMSGHDSZ, 0);
						message.header.status = NET_DISCONNECT;
						send(socket, message_ptr, NETMSGHDSZ, 0);

						iResult = closesocket(socket);
						if (iResult)
						{
							wsprintf(error, L"[SERVER] Incoming MAX USER disconnected with error: %d\n", WSAGetLastError());
							_MBM(error);
						}
					}

					_ATM_ADD(sv_active_clients, 1); // sv_active_clients++

					if (_ATM_LD(sv_active_clients) > w_pool.GetWorkers()) // нужно больше негров
						w_pool.WorkersAdd(1);

					{
						glock protect_socket_data (w_socket_m);

						if (_ATM_LD(sv_active_clients) > w_socket.size()) // нужно больше золота
						{ 
							w_socket.emplace_back(INVALID_SOCKET);
							w_ticket.emplace_back(NULL);
						}

						size_t i, size = w_socket.size();		// let's find any available socket slot
						for (i = size-1; i != -1; i--)			// . (это не самый быстрый способ)
							if (w_socket[i] == INVALID_SOCKET)	// . (но пока нет причин усложнять)
							{
								printf("\nSERVER: Client connected to %i slot\n", i);
								std::swap(w_socket[i], socket);
								w_ticket[i] = w_pool(&CNetworkWS2::ServerWorkOnClient, this, i);
								break;
							}
					}

					if (socket != INVALID_SOCKET) // что-то пошло не так, проверь код !
					{
						_ATM_SUB(sv_active_clients, 1); // sv_active_clients--
						
						message.header.length = 0;
						message.header.status = NET_ERROR;				
						send(socket, message_ptr, NETMSGHDSZ, 0);
						message.header.status = NET_DISCONNECT;
						send(socket, message_ptr, NETMSGHDSZ, 0);

						iResult = closesocket(socket);
						if (iResult)
						{
							wsprintf(error, L"[SERVER] Incoming connection closed with unexpected error: %d\n", WSAGetLastError());
							_MBM(error);
						}
						else
						{
							wsprintf(error, L"[SERVER] Incoming connection closed with unexpected error");
							_MBM(error);
						}

					//	printf("\nSERVER: Incoming connection closed with unexpected error !\n");

					}
				}
			}
			
			printf("\nSERVER: Listen thread shuted down...\n");
		
		}

		//>> Поток активного соединения
		void ServerWorkOnClient(size_t id)
		{
			wchar_t error[128];

			NETMSG message;
			char * message_ptr = (char*)&message;
			SOCKET socket = INVALID_SOCKET; // socket == w_socket[id]

			{
				glock protect_socket_data (w_socket_m);
				socket = w_socket[id];
			}

			int iResult;
			
			do // receiving messages loop
			{
				iResult = recv(socket, message_ptr, NETMSGSZ, 0);

				if (iResult > 0)
				{
					//printf("\nSERVER: Bytes received %d\n", iResult);

					if (message.header.status == NET_DISCONNECT)
					{
						printf("\nSERVER: Client %i disconnected\n", id);
						break;
					}
					else if (message.header.status == NET_RAWDATA)
					{
						// обработка игровых данных
					}
				}
				else if (iResult == 0) // lost client
				{
					printf("\nSERVER: Client has been losted...\n");
					break;
				}
				else // loop error
				{
					if (_ATM_LD(sv_exit)) // server is shutting down...
					{
						printf("\nSERVER: Interrupt working thread %i...", id);
						break;
					}

					auto error_code = WSAGetLastError();
					switch (error_code)
					{
					case WSAECONNRESET: printf("\nSERVER: Connection %i reseted by client\n", id); break;
					default:			printf("\nSERVER: Recv() %i failed %d\n", id, error_code); break;
					}
					break;
				}

			} while (iResult > 0);

			printf("\nSERVER: Closing %i thread...\n", id);

			{
				glock exit_lock (w_exit_m); // Возможно, в это время происходит ServerShutdown()

				if (_ATM_LD(sv_exit)) return; // Если вызван ServerShutdown(), сами ничего уже не закрываем

				iResult = closesocket(socket); // socket == w_socket[id]
				if (iResult)
				{
					wsprintf(error, L"[SERVER] w_socket %i has closed with error: %d\n", id, WSAGetLastError());
					_MBM(error);
				}

				{
					glock protect_socket_data (w_socket_m);
					w_socket[id] = INVALID_SOCKET;
				}

				_ATM_SUB(sv_active_clients, 1); // sv_active_clients--
			}
		}

		//>> Закрывает поток прослушивания
		void ServerShutdownProc()
		{
			wchar_t error[128];
			int iResult;

			iResult = closesocket(sv_socket);
			if (iResult)
			{
				wsprintf(error, L"[SERVER] Closed listen socket with error: %d\n", WSAGetLastError());
				_MBM(error);
			}
			sv_socket = INVALID_SOCKET;
			freeaddrinfo(sv_addr_result);
			sv_addr_result = nullptr;

			sv_thread.Join(true);
		}

	public:
		//>> Выключение сервера
		void ServerShutdown()
		{
			if (!isServered) return;

			NETMSG message;
			char * message_ptr = (char*)&message;

			wchar_t error[128];
			int iResult;

			_ATM_1(sv_exit);

			// Закрытие потока прослушивания ServerListen()

			ServerShutdownProc();	

			// Закрытие активных соединений ServerWorkOnClient()

			{
				glock exit_lock (w_exit_m);					// Возможно, в это время кто-то из ServerWorkOnClient() пытается сам закрыться
				//glock protect_socket_data (w_socket_m);

				message.header.status = NET_DISCONNECT;
				message.header.length = 0;

				uint32 i = 0;
				for (auto & socket : w_socket)
				{
					if (socket != INVALID_SOCKET)
					{
						send(socket, message_ptr, NETMSGHDSZ, 0); // Сообщаем клиентам о закрытии (не обязательно)
						iResult = closesocket(socket);
						if (iResult)
						{
							wsprintf(error, L"[SERVER] Closed w_socket %i with error: %d\n", i, WSAGetLastError());
							_MBM(error);
						}
						socket = INVALID_SOCKET;
					}
					i++;
				}
			}

			for (auto & ticket : w_ticket)
				w_pool.Wait(ticket);
			
			_ATM_0(sv_exit);
			_ATM_ST(sv_active_clients, 0);

			printf("\nServer shuted down!\n");

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
				"\nlpVendorInfo   %x"
				"\n",
				wsaData.iMaxSockets,
				wsaData.iMaxUdpDg,
				wsaData.wHighVersion,
				wsaData.wVersion,
				wsaData.szDescription,
				wsaData.szSystemStatus,
				(int)wsaData.lpVendorInfo);
		}

	};
}

#endif // _NETWORKWS2_H