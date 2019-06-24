// ----------------------------------------------------------------------- //
//
// MODULE  : NetworkWS2.h
//
// PURPOSE : [Test chat] Windows Sockets 2 (Winsock2) API
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

#define  SERVER_PORT	"49213"
#define  MAX_CLIENTS	64
#define  NET_RAW_MAX	512
#define  NET_TXT_MAX	256
#define  NET_NAMELEN	32

#ifdef WINDOWS_WINSOCK2
	#define CNetworkF CNetwork  // Class Network Final
#endif

/*
CNetwork network;
char command[256];

std::cout << "Enter '-connect' for set server connection...\n\n";
for (;;)
{
	std::cin.getline(command, 256);

	if (Compare(command, "-exit", 5)) break;
	else if (Compare(command, "-wakeup", 7)) network.ServerWakeup();
	else if (Compare(command, "-shutdown", 9)) network.ServerShutdown();
	else if (Compare(command, "-connect", 8)) network.ClientConnect("92.243.162.64"); //network.ClientConnect("127.0.0.1");
	else if (Compare(command, "-disconnect", 11)) network.ClientDisconnect();
	else if (Compare(command, "-rename ", 8)) network.ClientChatRename(&command[8]);
	else network.ClientChatSentMessage(command);
}
*/

namespace SAVFGAME
{
	enum eNetStatus
	{
		NET_NONE,

		NET_CONNECT,		// установка соединения
		NET_DISCONNECT,		// разрыв соединения
		NET_NOFREESLOTS,	// разрыв соединения (нет мест)	
		NET_RECEIVED,		// сообщение получено
		NET_CLIENTNAME,		// имя клиента
		NET_SERVERNAME,		// имя сервера
		NET_TEXTDATA,		// текстовые данные
		NET_RAWDATA,		// сырые данные

		NET_ENUM_MAX
	};

	struct NETMSG
	{
		eNetStatus status;
		int        raw_len;
		char       raw[NET_RAW_MAX];	
	};

	class CNetwork
	{
	protected:
		WSADATA			wsaData;
		char			server_name[NET_NAMELEN];
		char			client_name[NET_NAMELEN];
		bool			isInit;
		bool			isConnected;
		bool			isServered;
	protected:
		addrinfo *		sv_addr_result;
		addrinfo		sv_addr_hints;
		SOCKET			sv_socket;					// server listening socket
		THREADDATA		sv_thread;					// server listening (waiting) thread		
		SOCKET			w_socket[1+MAX_CLIENTS];	// server working with %i client at w_socket[%i]
		THREADDATA		w_thread[1+MAX_CLIENTS];	// server working with %i client in w_thread[%i]
		char			w_name[1+MAX_CLIENTS][NET_NAMELEN];		
	protected:
		addrinfo *		cl_addr_result;
		addrinfo		cl_addr_hints;
		SOCKET			cl_socket;			// client connection socket
		THREADDATA		cl_thread;			// client receiving (waiting) thread
	protected:
		atomic<bool>	sv_exit; // server exit call
		atomic<bool>	cl_exit; // client exit call
	public:
		CNetwork(const CNetwork& src)				= delete;
		CNetwork(CNetwork&& src)					= delete;
		CNetwork& operator=(CNetwork&& src)			= delete;
		CNetwork& operator=(const CNetwork& src)	= delete;
	public:
		CNetwork() : isInit(false), isConnected(false), isServered(false), sv_exit(false), cl_exit(false),
			cl_addr_result(nullptr), cl_socket(INVALID_SOCKET),
			sv_addr_result(nullptr), sv_socket(INVALID_SOCKET)
		            { Init(); };
		~CNetwork() { Close(); };

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

		bool Init()
		{
			if (isInit) return true;

			wchar_t error[128];
			int iResult;

			iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult != 0)
			{			
				wsprintf(error, L"WSAStartup failed: %d", iResult);
				_MBM(error);
				return isInit=false;
			}

			return isInit = true;
		}

		//>> Подключение к серверу
		bool ClientConnect(char* ip)
		{
			if (!isInit) { _MBM(L"Network didn't inited"); return false; }
			if (isConnected) { ClientDisconnect(); }

			wchar_t error[128];
			int iResult;

			ZeroMemory(&cl_addr_hints, sizeof(cl_addr_hints));
			cl_addr_hints.ai_family   = AF_UNSPEC;
			cl_addr_hints.ai_socktype = SOCK_STREAM;
			cl_addr_hints.ai_protocol = IPPROTO_TCP;

			iResult = getaddrinfo(ip, SERVER_PORT, &cl_addr_hints, &cl_addr_result);
			if (iResult != 0)
			{	
				wsprintf(error, L"Client getaddrinfo failed: %d", iResult);
				_MBM(error);
				return isConnected=false;
			}

			cl_socket = socket(	cl_addr_result->ai_family,
								cl_addr_result->ai_socktype,
								cl_addr_result->ai_protocol	);

			if (cl_socket == INVALID_SOCKET)
			{
				wsprintf(error, L"Init cl_socket failed: %ld", WSAGetLastError());
				_MBM(error);
				freeaddrinfo(cl_addr_result);	cl_addr_result = nullptr;
				return isConnected=false;
			}

			iResult = connect(cl_socket, cl_addr_result->ai_addr, (int)cl_addr_result->ai_addrlen);
			if (iResult == SOCKET_ERROR)
			{
				printf("\nConnection to server failed\n"); // (%s)\n", ip);
				closesocket(cl_socket);			cl_socket = INVALID_SOCKET;
				freeaddrinfo(cl_addr_result);	cl_addr_result = nullptr;
				return isConnected=false;
			}

			////// Запуск потока ожидания ответа сервера //////

		/*	thread * thr = new thread;
			if (!cl_thread.Add(thr, true))
			{
				_MBM(L"Client listen thread failed [NETWORK]");
				closesocket(cl_socket);			cl_socket = INVALID_SOCKET;
				freeaddrinfo(cl_addr_result);	cl_addr_result = nullptr;
				return isConnected = false;
			}
			*thr = thread(&CNetwork::ClientListen, this); //*/

			if (cl_thread(&CNetwork::ClientListen, this) != TC_SUCCESS)
			{
				_MBM(L"Client listen thread failed [NETWORK]");
				closesocket(cl_socket);			cl_socket = INVALID_SOCKET;
				freeaddrinfo(cl_addr_result);	cl_addr_result = nullptr;
				return isConnected = false;
			}

			sprintf_s(client_name, "YOU");
			printf("\nSuccessfully connected to server\n"); //%s\n", ip);

			return isConnected=true;
		}

	protected:
		//>> Поток приёма сообщений от сервера
		void ClientListen()
		{
			int iResult;
			NETMSG * msg = new NETMSG;

			do // receiving messages loop
			{
				iResult = recv(cl_socket, (char*)msg, sizeof(NETMSG), 0);

				if (_ATM_LD(cl_exit)) //cl_thread.Interrupt_check()) // interrupt call from ::Disconnect()
				{
					printf("\nCLIENT: Interrupt listening thread...\n");
					break;
				}

				if (iResult > 0)
				{
					//printf("\nCLIENT: Bytes received %d\n", iResult);

					if (msg->status == NET_DISCONNECT)
					{
						printf("\nCLIENT: Server sent 'disconnect'\n");
						break;
					}

					if (msg->status == NET_NOFREESLOTS)
					{
						printf("\nCLIENT: Server sent 'no free slots'\n");
						break;
					}

					if (msg->status == NET_TEXTDATA) // chat from server
					{
						printf("\n%s\n", msg->raw);
					}

					if (msg->status == NET_CLIENTNAME) // some client renamed
					{
						printf("\n%s\n", msg->raw);
					}

				}
				else if (iResult == 0) // Server has been losted
				{
					printf("\nCLIENT: Server lost\n");
					break;
				}
				else // loop error
				{
					auto error_code = WSAGetLastError();
					switch (error_code)
					{
					case WSAECONNRESET: printf("\nCLIENT: Connection reset by server\n"); break;
					default:			printf("\nCLIENT: Recv() failed %d\n", error_code); break;
					}
					break;
				}
			} while (iResult > 0);


			printf("\nCLIENT: Closing listening thread...\n");

			delete msg;
			//cl_thread.Finish();
		}

		//>> Отключение от сервера
		void ClientDisconnectProc()
		{
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
			if (!isConnected) return;

			// Сообщаем серверу о разрыве соединения

			NETMSG * msg = new NETMSG;
			msg->status = NET_DISCONNECT;
			send(cl_socket, (char*)msg, sizeof(NETMSG), 0);
			delete msg;

			// Закрываем

			_ATM_1(cl_exit);
			//cl_thread.Join(true);
			//cl_thread.Interrupt_call(); // ClientListen() interrupt
			ClientDisconnectProc();
			cl_thread.Join(true);
			_ATM_0(cl_exit);
		}

		//>> test
		void ClientChatSentMessage(char* text)
		{
			if (!isConnected) return;

			int len = (int)strlen(text);
			if (len < 1) return;

			NETMSG * msg = new NETMSG;
			msg->status = NET_TEXTDATA;
			msg->raw_len = min(len, NET_TXT_MAX);
			ZeroMemory(msg->raw, NET_RAW_MAX);
			memcpy(msg->raw, text, msg->raw_len);

			send(cl_socket, (char*)msg, sizeof(NETMSG), 0);
			printf("\n[%s]: %s\n", client_name, msg->raw);

			delete msg;
		}

		//>> test
		void ClientChatRename(char* text)
		{
			if (!isConnected) return;

			int len = (int)strlen(text);
			if (len<1 || len>NET_NAMELEN-1) return;

			NETMSG * msg = new NETMSG;
			msg->status = NET_CLIENTNAME;
			msg->raw_len = min(len, NET_NAMELEN-1); // -1 for null terminator
			ZeroMemory(msg->raw, NET_RAW_MAX);
			memcpy(msg->raw, text, msg->raw_len);
			
			send(cl_socket, (char*)msg, sizeof(NETMSG), 0);
			sprintf_s(client_name, "%s", msg->raw);
			printf("\nYOU re-named to '%s'\n", msg->raw);

			delete msg;
		}

		//>> Включение сервера
		bool ServerWakeup()
		{
			wchar_t error[128];
			int iResult;

			ZeroMemory(&sv_addr_hints, sizeof(sv_addr_hints));
			sv_addr_hints.ai_family   = AF_INET;
			sv_addr_hints.ai_socktype = SOCK_STREAM;
			sv_addr_hints.ai_protocol = IPPROTO_TCP;
			sv_addr_hints.ai_flags    = AI_PASSIVE;

			iResult = getaddrinfo(NULL, SERVER_PORT, &sv_addr_hints, &sv_addr_result);
			if (iResult != 0)
			{
				wsprintf(error, L"Server getaddrinfo failed: %d\n", iResult);
				_MBM(error);
				return isServered=false;
			}

			sv_socket = socket(	sv_addr_result->ai_family,
								sv_addr_result->ai_socktype,
								sv_addr_result->ai_protocol );

			if (sv_socket == INVALID_SOCKET)
			{
				wsprintf(error, L"Init sv_socket failed: %ld\n", WSAGetLastError());
				_MBM(error);
				freeaddrinfo(sv_addr_result);	sv_addr_result = nullptr;
				return isServered=false;
			}

			iResult = bind(sv_socket, sv_addr_result->ai_addr, (int)sv_addr_result->ai_addrlen);
			if (iResult == SOCKET_ERROR)
			{
				wsprintf(error, L"Bind sv_socket failed: %d\n", WSAGetLastError());
				_MBM(error);
				closesocket(sv_socket);			sv_socket = INVALID_SOCKET;
				freeaddrinfo(sv_addr_result);	sv_addr_result = nullptr;
				return isServered=false;
			}

			iResult = listen(sv_socket, SOMAXCONN);
			if (iResult == SOCKET_ERROR) {
				wsprintf(error, L"Listen sv_socket failed: %ld\n", WSAGetLastError());
				_MBM(error);
				closesocket(sv_socket);			sv_socket = INVALID_SOCKET;
				freeaddrinfo(sv_addr_result);	sv_addr_result = nullptr;
				return isServered=false;
			}

			////// Запуск потока прослушивания //////

		/*	thread * thr = new thread;
			if (!listen_thread.Add(thr, true))
			{
				_MBM(L"Server listen thread failed [NETWORK]");
				closesocket(sv_socket);			sv_socket = INVALID_SOCKET;
				freeaddrinfo(sv_addr_result);	sv_addr_result = nullptr;
				return isServered=false;

			}
			*thr = thread(&CNetwork::ServerListen, this); //*/

			if (sv_thread(&CNetwork::ServerListen, this) != TC_SUCCESS)
			{
				_MBM(L"Server listen thread failed [NETWORK]");
				closesocket(sv_socket);			sv_socket = INVALID_SOCKET;
				freeaddrinfo(sv_addr_result);	sv_addr_result = nullptr;
				return isServered = false;
			}

			return isServered=true;
		}

	protected:
		//>> Поток порта прослушивания
		void ServerListen()
		{
			for (;;) // listen loop
			{ 
				if (_ATM_LD(sv_exit)) break; //(listen_thread.Interrupt_check()) break; // exit if server shutdown

				SOCKET tmpsock = accept(sv_socket, NULL, NULL);	// wait for connection
				if (tmpsock != INVALID_SOCKET)
				{
					printf("\nSERVER: Incoming connection!\n");

					//thread * thr = new thread;
					for (int i=0; i<MAX_CLIENTS; i++) // try to store into free slot
					{
					/*	if (w_thread[i].Add(thr, true))
						{
							printf("\nSERVER: Incoming connection in %i slot\n", i);
							w_socket[i] = tmpsock;
							tmpsock = INVALID_SOCKET;
							*thr = thread(&CNetwork::ServerWorkOnClient, this, i);
							break;
						}//*/
						if (w_thread[i].IsFinished())
						{
							w_socket[i] = std::move(tmpsock);
							tmpsock = INVALID_SOCKET;
							if (w_thread[i](&CNetwork::ServerWorkOnClient, this, i) != TC_SUCCESS)
							{
								tmpsock = std::move(w_socket[i]);
								w_socket[i] = INVALID_SOCKET;
							}
							else
							{
								printf("\nSERVER: Incoming connection in %i slot\n", i);
								break;
							}
						}
					}

					if (tmpsock != INVALID_SOCKET) // if no free slots
					{
						// Сообщаем клиенту об отсутствии места

						NETMSG * msg = new NETMSG;
						msg->status = NET_NOFREESLOTS;
						send(tmpsock, (char*)msg, sizeof(NETMSG), 0);
						delete msg;

						printf("\nSERVER: Incoming connection closed with no free slots\n");
						closesocket(tmpsock);
						//delete thr;
					}
				}
			}	

			//listen_thread.Finish();
		}

		//>> Поток порта активного соединения
		void ServerWorkOnClient(uint32 id)
		{
			THREADDATA & wthread = w_thread[id];
			SOCKET & wsocket = w_socket[id];

			char ctemp[NET_RAW_MAX];
			int iResult;// , iSendResult;
			NETMSG * msg = new NETMSG;

			sprintf_s(w_name[id], "CLIENT %02i", id);

			//// Уведомим остальных о новом участнике чата ////

			msg->status  = NET_TEXTDATA;
			msg->raw_len = NET_TXT_MAX;
			sprintf_s(msg->raw, "%s enter in chat", w_name[id]);

			for (uint32 i=0; i<MAX_CLIENTS; i++)
				if (id!=i && !w_thread[i].IsFinished())
					send(w_socket[i], (char*)msg, sizeof(NETMSG), 0);

			//// Уведомим нового участника об остальных ////

			std::string sstr;
			sstr.assign("Currently in chat: ");
			for (uint32 i=0; i<MAX_CLIENTS; i++)
				if (id != i && !w_thread[i].IsFinished())
				{
					sprintf_s(ctemp, "'%s', ", w_name[i]);
					sstr.append(ctemp);
				};
			msg->status = NET_TEXTDATA;
			msg->raw_len = NET_TXT_MAX;
			sprintf_s(msg->raw, "%s", sstr.c_str());
			send(wsocket, (char*)msg, sizeof(NETMSG), 0);

			do // receiving messages loop
			{
				iResult = recv(wsocket, (char*)msg, sizeof(NETMSG), 0);

				if (_RLX_LD(sv_exit)) //(wthread.Interrupt_check()) // exit if server shutdowning
				{
					printf("\nSERVER: Interrupt %i thread\n", id);
					break; 
				}

				if (iResult > 0) // msg has been received
				{
					//printf("\nSERVER: Bytes received %d\n", iResult);

					if (msg->status == NET_DISCONNECT)
					{
						printf("\nSERVER: Client %i disconnected\n", id);
						break;
					}

					if (msg->status == NET_TEXTDATA) // chat re-selling to clients  ; from ClientChatSentMessage()
					{
						sprintf_s(ctemp, "[%s]: %s", w_name[id], msg->raw);
						memcpy(msg->raw, ctemp, NET_RAW_MAX);
						printf("\n%s\n", msg->raw);

						for (uint32 i=0; i<MAX_CLIENTS; i++)
							if (id!=i && !w_thread[i].IsFinished())
								send(w_socket[i], (char*)msg, sizeof(NETMSG), 0);
					}

					if (msg->status == NET_CLIENTNAME) // client renamed  ; from ClientChatRename()
					{
						sprintf_s(ctemp, "client '%s' re-named to '%s'", w_name[id], msg->raw);
						memcpy(w_name[id], msg->raw, NET_NAMELEN);
						memcpy(msg->raw, ctemp, NET_RAW_MAX);
						printf("\n%s\n", msg->raw);

						for (uint32 i = 0; i<MAX_CLIENTS; i++)
							if (id != i && !w_thread[i].IsFinished())
								send(w_socket[i], (char*)msg, sizeof(NETMSG), 0);
					}

					//msg->status = NET_RECEIVED;
					//iSendResult = send(wsocket, (char*)msg, sizeof(NETMSG), 0);
					//if (iSendResult == SOCKET_ERROR)
					//{
					//	printf("\nSend failed: %d\n", WSAGetLastError());
					//	break;
					//}
					//printf("\nBytes sent: %d\n", iSendResult);
				}
				else if (iResult == 0) // lost client
				{
					printf("\nSERVER: Client has been losted...\n");
					break;
				}
				else // loop error
				{
					auto error_code = WSAGetLastError();
					switch (error_code)
					{
					case WSAECONNRESET: printf("\nSERVER: Connection %i reset by client\n", id); break;
					default:			printf("\nSERVER: Recv() %i failed %d\n", id, error_code); break;
					}
					break;
				}

			} while (iResult > 0);

			printf("\nSERVER: Closing working thread (id = %i) [%s]\n", id, w_name[id]);

			//// Уведомим остальных о выходе участнике чата ////

			msg->status = NET_TEXTDATA;
			msg->raw_len = NET_TXT_MAX;
			sprintf_s(msg->raw, "%s leave from chat", w_name[id]);

			for (uint32 i = 0; i<MAX_CLIENTS; i++)
				if (id != i && !w_thread[i].IsFree())
					send(w_socket[i], (char*)msg, sizeof(NETMSG), 0);

			delete msg;
		//	wthread.Finish();
		}

	public:
		//>> Выключение сервера
		void ServerShutdown()
		{
			if (!isServered) return;

			wchar_t error[128];
			int iResult;

			// Закрытие прослушивания ServerListen()

			_ATM_1(sv_exit);
		//	listen_thread.Interrupt_call();	
			freeaddrinfo(sv_addr_result);		sv_addr_result = nullptr;
			closesocket(sv_socket);				sv_socket = INVALID_SOCKET;
		//	listen_thread.Join(true);
			sv_thread.Join(true);
			
			// Закрытие активных соединений ServerWorkOnClient()

			for (int i=0; i<MAX_CLIENTS; i++)
				if (w_thread[i].IsFree() == false)
			{
				// Сообщаем клиенту о закрытии

				NETMSG * msg = new NETMSG;
				msg->status = NET_DISCONNECT;
				send(w_socket[i], (char*)msg, sizeof(NETMSG), 0);
				delete msg;

				// Закрываем

			//	w_thread[i].Interrupt_call();
				iResult = shutdown(w_socket[i], SD_SEND);
				if (iResult == SOCKET_ERROR)
				{
					wsprintf(error, L"Server w_socket[%i] shutdown failed: %d\n", i, WSAGetLastError());
					_MBM(error);
				}
				closesocket(w_socket[i]);
				w_socket[i] = INVALID_SOCKET;
				w_thread[i].Join(true);
			}
			_ATM_0(sv_exit);

			printf("\nServer shutdown...\n");

			isServered = false;
		}

		//>> Показ WSADATA
		void PrintWSAData()
		{
			printf(	"\niMaxSockets  %i"
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