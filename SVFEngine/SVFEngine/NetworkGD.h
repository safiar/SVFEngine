// ----------------------------------------------------------------------- //
//
// MODULE  : NetworkGD.h
//
// PURPOSE : Описания и структуры игровых данных сетевой части
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _NETWORKGD_H
#define _NETWORKGD_H

#include "stdafx.h"
#include "Player.h"
#include "GameMap.h"
#include "UI.h"

#define  MAX_STRUCT_SIZE   1296	// Струтуры не должны превышать размер единичного сообщения (+ 4 заголовок = 1300 NETRAWSZ)

#define  SV_NAME_MAX_LEN   32	// wchar_t, 31 + 1 null
#define  MAP_NAME_MAX_LEN  256	// wchar_t

#define  PAS_MAX  256 // максимальное число описываемых игроков в структуре NetPlayersAtServer

// TODO  "smallest three" compression for transmitting Quaternion  [16 bytes to 7 bytes]
// TODO  оптимизация прочих компонент при передаче, уменьшение количества передаваемых данных
// TODO  сжатие составленных пакетов

namespace SAVFGAME
{
#pragma pack(push, 1)

	//>> Сервер определяет новое положение камеры игрока
	struct NetCameraReset
	{
		MATH3DVEC		pos;
		MATH3DVEC		angle;
		eCameraMode		mode;
	};

	//>> Данные, присылаемые клиентом в запросе на соединение
	struct NetConnectClientData
	{
		union
		{
			char _ [MAX_STRUCT_SIZE];
			struct
			{
				uint16			name_id;
				wchar_t			name[PLAYER_NAME_MAX_LEN];
				eCameraMode		camera_mode;
				float			camera_aspect;
				uint32			model_id;
				uint32			client_tick;
			};
		};
	};

	//>> Данные, присылаемые сервером в ответ на запрос соединения
	struct NetConnectServerData
	{
		union
		{
			char _ [MAX_STRUCT_SIZE];
			struct
			{
				wchar_t			server_name[SV_NAME_MAX_LEN];
				wchar_t			map_name[MAP_NAME_MAX_LEN];
				uint32			player_id;
				uint32			max_players;
				uint32			server_tick;
				float			camera_fov;
				float			camera_near;
				float			camera_far;
				NetCameraReset	camera_reset;
			};
		};
	};

	//>> Данные, присылаемые клиентом после загрузки карты
	struct NetLoadedClientData
	{
		uint32	map_crc32;
	};

	//>> Сведения об игроках на сервере
	struct NetPlayersAtServer 
	{
		struct PASDATA
		{
			// ... 
			union
			{
				byte _reserved;	// total 8 flags
				struct
				{
					bool connected : 1; // flag 0 : player is connected to server
					bool loaded    : 1; // flag 1 : player has loaded all resources
					bool bot       : 1; // flag 2 : player isn't real human
					// ...
				};
			};
		};

		uint32	 total;			 // max players at server
		uint32   multiplier {1}; // ID multiplier
		PASDATA  info [PAS_MAX]; // player_ID = info_ID * ID_multiplier
	};

	//>> Сведения о конкретном игроке на сервере
	struct NetPlayerServerData
	{
		struct PSDFLAG
		{
			union
			{
				uint32 _reserved;	// total 32 flags
				struct
				{
					bool update_name       : 1; // flag 0 : client will update player's name at own list
					bool update_pos        : 1; // flag 1 : client will know where is another player
					bool update_chat       : 1; // flag 2 : client will update chat history with player's chat message
					// ...
				};
			};
			void Reset() { _reserved = 0; }
		};

		uint32				id;							// id at server's list
		PSDFLAG				flag;						// player's flags
		uint16				name_id;					// presented only if < flag.update_name = true >
		wchar_t				name[PLAYER_NAME_MAX_LEN];	// presented only if < flag.update_name = true >
		eCameraMode			camera_mode;				// presented only if < flag.update_pos  = true >
		float				camera_angle_z;				// presented only if < flag.update_pos  = true >
		MATH3DVEC			camera_position;			// presented only if < flag.update_pos  = true >
		MATH3DVEC			camera_lookat;				// presented only if < flag.update_pos  = true >
		uint32				model_id;					// presented only if < flag.update_pos  = true >
		MATH3DVEC			model_position;				// presented only if < flag.update_pos  = true >
		MATH3DQUATERNION	model_rotation;				// presented only if < flag.update_pos  = true >
		uint16				chat_message_id;			// presented only if < flag.update_chat = true >
		wchar_t				chat_message[CHATMSGSZ];	// presented only if < flag.update_chat = true >
	};

	//>> Сведения игрока о себе для сервера
	struct NetPlayerClientData
	{
		struct PCDFLAG
		{
			union
			{
				uint32 _reserved; // total 32 flags
				struct
				{
					bool update_name : 1; // flag 0 : server will update player's name at own list
					bool update_chat : 1; // flag 1 : server will update chat history with player's chat message
					// ...
				};
			};
			void Reset() { _reserved = 0; }
		};

		PCDFLAG				flag;
		uint16				name_id;						// presented only if < flag.update_name = true >
		wchar_t				name[PLAYER_NAME_MAX_LEN];		// presented only if < flag.update_name = true >
		eCameraMode			camera_mode;
		float				camera_aspect;
		bool				camera_fly_mode;
		bool				camera_xz_fix;
		MATH3DQUATERNION	camera_rotation;
		eCameraSignal		camera_move_x;
		eCameraSignal		camera_move_y;
		eCameraSignal		camera_move_z;
		eCameraSignal		camera_roll;
		uint32				event_locker;
		bool				event_model_move;
		bool				event_model_rotate;
		int32				IO_mouse_x;
		int32				IO_mouse_y;
		int32				IO_window_W;
		int32				IO_window_H;
		MATH3DVEC2			interact_rotation_angles;
		MATH3DVEC2			interact_move_distance;
		uint16				chat_message_id;				// presented only if < flag.update_chat = true >
		wchar_t				chat_message[CHATMSGSZ];		// presented only if < flag.update_chat = true >
	};

	//>> Сведения об объекте сцены
	struct NetObjectServerData
	{
		eObjectType			type;
		uint32				id;
		MATH3DVEC			pos;
		MATH3DQUATERNION	rot;
		MATH3DVEC			scale;
	};

#pragma pack(pop)
}

#endif // _NETWORKGD_H