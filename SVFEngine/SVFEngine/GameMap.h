// ----------------------------------------------------------------------- //
//
// MODULE  : GameMap.h
//
// PURPOSE : Загрузка и содержание состояния сцены
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _GAMEMAP_H
#define _GAMEMAP_H

#include "stdafx.h"
#include "Object.h"
#include "Light.h"
#include "Camera.h"
#include "ModelRe.h"
#include "SkyboxRe.h"
#include "Sprite.h"
#include "Sound.h"

#define  DEFAULT_MAP_ID        0
#define  DEFAULT_PLAYER_SPAWN  0

#define  CGameMapF  CGameMap  // class final

// TODO: ERROR-модель по умолчанию

namespace SAVFGAME
{
	enum eParentMethod
	{
		PARENT_UNSPECIFIC = 0, // game logic behaviour
		PARENT_POSITION   = 1, // moving with parent
		PARENT_CHASING    = 2, // chasing parent if it's move away

		PARENT_ENUM_MAX
	};

	enum eObjectType // Все возможные типы объектов
	{
		OBJECT_NONE,

		OBJECT_MODEL,		// MapModelSetting
		OBJECT_LIGHT,		// MapLightSetting
		OBJECT_FONT,		// 
		OBJECT_TEXT,		// 
		OBJECT_CAMERA,		// 
		OBJECT_SKYBOX,		// MapSkyboxSetting
		OBJECT_SHADER,		// 
		OBJECT_SOUND,		// MapSoundSetting
		OBJECT_PLAYER,		// MapPlayerSpawnSetting
		OBJECT_SPAWN,		// MapObjectSpawnSetting
		OBJECT_SPRITE,		// MapSpriteSetting

		OBJECT_ENUM_MAX
	};

	enum eObjectAction
	{
		OBJACTION_NONE   = 0,

		OBJACTION_RUN    = MASK_BIT_00,
		OBJACTION_PAUSE  = MASK_BIT_01,
		OBJACTION_RESUME = MASK_BIT_02,
		OBJACTION_STOP   = MASK_BIT_03,

		OBJACTION_ENABLE  = MASK_BIT_04,
		OBJACTION_DISABLE = MASK_BIT_05,
	};

	struct MapObjectLinker
	{
	public:
		MapObjectLinker(const MapObjectLinker& src)				= delete;
		MapObjectLinker(MapObjectLinker&& src)					= delete;
		MapObjectLinker& operator=(MapObjectLinker&& src)		= delete;
		MapObjectLinker& operator=(const MapObjectLinker& src)	= delete;
	public:
		MapObjectLinker()
		{
			Close();
		}
		void Close()
		{
			name   = L"";
			type   = OBJECT_NONE;
			method = PARENT_UNSPECIFIC;
			id     = MISSING;
			ptr    = nullptr;
		}
		wstring			name;		// имя связанного объекта
		eObjectType		type;		// тип связанного объекта
		eParentMethod	method;		// метод привязки к другому объекту
		uint32			id;			// номер связанного объекта в списке <MapData>
		void *			ptr;		// указатель на связанный объект
	};
	struct MapObjectSetting : public CObject
	{
		struct InterpolationMemory
		{
			InterpolationMemory()
			{
				P.Create(2);
				Q.Create(2);
				S.Create(2);

				Close();
			}
			~InterpolationMemory(){};
			void Close()
			{
				for (uint16 i = 0; i < S.count; i++) S[i]._default();
				for (uint16 i = 0; i < P.count; i++) P[i]._default();
				for (uint16 i = 0; i < Q.count; i++) Q[i]._default();
				P.prev = Q.prev = S.prev = 0;
				P.cur  = Q.cur  = S.cur  = 1;
			}
			TBUFFERNEXT <MATH3DVEC,        uint16>	P;	// [CLIENT]
			TBUFFERNEXT <MATH3DVEC,        uint16>	S;	// [CLIENT]
			TBUFFERNEXT <MATH3DQUATERNION, uint16>	Q;	// [CLIENT]
		};
	public:
		MapObjectSetting(const MapObjectSetting& src)            = delete;
		MapObjectSetting(MapObjectSetting&& src)                 = delete;
		MapObjectSetting& operator=(MapObjectSetting&& src)      = delete;
		MapObjectSetting& operator=(const MapObjectSetting& src) = delete;
	public:
		MapObjectSetting() : CObject(), pos(CObject::pos)
		{
			Close();
		}
		~MapObjectSetting() override {}
		void Close() override
		{
			name      = L"";
			filename  = L"";
			id_data   = MISSING;
			id_list   = MISSING;
			enable    = true;
			color     = COLORVEC(1,1,1,1);
			MEM.Close();
			parent.Close();
			child.Close(1,1);
			CObject::Close();
		}
	public:
		WPOS *						pos;			// позиция объекта
		wstring						name;			// формальное имя объекта
		wstring						filename;		// (опционально) связанный с объектом файл
		COLORVEC					color;			// (опционально) цвет объекта
		MapObjectLinker				parent;			// (опционально) связанный вышестоящий объект
		VECPDATA <MapObjectLinker>	child;			// связанные нижестоящие объекты		
		uint32						id_data;		// (опционально) номер в списке <MapData>
	protected:
		bool						enable;			// объект включен или выключен
		InterpolationMemory			MEM;            // memory for interpolation
	public:
		void MemoryS(const MATH3DVEC & scale)           { MEM.S.SetNext(scale);    }
		void MemoryP(const MATH3DVEC & position)        { MEM.P.SetNext(position); }
		void MemoryQ(const MATH3DQUATERNION & rotation) { MEM.Q.SetNext(rotation); }
		void InterpolateWPOS(float interpolation_t)
		{
			pos->P = MathPointLERP
				( MEM.P [ MEM.P.prev ],
				  MEM.P [ MEM.P.cur  ],
				  interpolation_t );

			pos->Q = MathQuaternionSLERP
				( MEM.Q [ MEM.Q.prev ],
				  MEM.Q [ MEM.Q.cur  ],
				  interpolation_t );

			pos->S = MathPointLERP
				( MEM.S [ MEM.S.prev ],
				  MEM.S [ MEM.S.cur  ],
				  interpolation_t );
		}
	protected:
		uint32						id_list;		// (опционально) номер в списке <MapLoadList>
	public:
		virtual void EnableObj()  { enable = true;  } // TIP: можно переопределить логику в конечном объекте
		virtual void DisableObj() { enable = false; } // TIP: можно переопределить логику в конечном объекте
		bool IsObjEnabled()       { return enable;  }
	};
	struct MapObjectSpawnSetting : public MapObjectSetting
	{
		MapObjectSpawnSetting() : MapObjectSetting(), spawn_type(OBJECT_NONE) { }
		~MapObjectSpawnSetting() override { }
		void Close() override
		{
			MapObjectSetting::Close();
		}
	public:
		eObjectType  spawn_type;
	};

	struct MapLightSetting : public MapObjectSetting
	{
		friend class MapObjects;
		friend class CGameMap;
		MapLightSetting() : MapObjectSetting(), setting(nullptr)
		{
			Close();
			setting_shr = make_shared<LightSetting>();
			setting     = setting_shr.get();

			MapObjectSetting::pos = setting->pos;
		}
		~MapLightSetting() override final
		{
			setting_shr = nullptr;
			setting     = nullptr;
		}
		void Close() override final
		{
			if (setting != nullptr)
				setting->Close();
			MapObjectSetting::Close();
		}
		shared_ptr<LightSetting>	setting_shr;	// состояние источника света
		LightSetting *				setting;		// состояние источника света
	};
	struct MapModelSetting : public MapObjectSetting
	{
		friend class MapObjects;
		friend class CGameMap;
		MapModelSetting() : MapObjectSetting() { Close(); }
		~MapModelSetting() override final { }
		void Close() override final
		{
			dynamic     = true;
			pick        = false;
			player		= false;
			id_matrix   = 0;
			data        = nullptr;
			MapObjectSetting::Close();
		}
		///////////
		bool         dynamic;	// статический или динамический
		bool         pick;		// возможность манипулировать (для динамических моделей)
		bool		 player;	// (tip) подсказка, что это модель игрока
	protected:
		uint32       id_matrix;	// номер статической матрицы (для статической модели)
	public:
		CModelR *    data;		// фактические данные модели
	public:
		//>> Отрисовка модели
		bool Show()
		{
			if (data == nullptr) return false;

			if (!dynamic) data->ShowModelStatic(pos, id_matrix);
			else          data->ShowModel(pos);

			return true;
		}
	};
	struct MapPlayerSpawnSetting : public MapObjectSpawnSetting
	{
		friend class MapObjects;
		friend class CGameMap;
		MapPlayerSpawnSetting() : MapObjectSpawnSetting()
		{
			spawn_type = OBJECT_PLAYER;
			camera_mode = CM_NOCONTROL;
		}
		~MapPlayerSpawnSetting() override final {}
		void Close() override final
		{
			spawn_type = OBJECT_PLAYER;
			camera_mode = CM_NOCONTROL;
			MapObjectSpawnSetting::Close();
		}
	public:
		eCameraMode  camera_mode;	// режим вида камеры
	};
	struct MapSoundSetting : public MapObjectSetting
	{
		friend class MapObjects;
		friend class CGameMap;
		MapSoundSetting() : MapObjectSetting() { Reset(); }
		~MapSoundSetting() override final {}
	private:
		void Reset()
		{
			GID.element = eSoundGroup_ELEMENT_MAX;
			GID.group   = eSoundGroup_USER_MAX;

			volume_max   = 1.f;
			volume_start = 1.f;
			volume       = 1.f;
			max_distance = 1000.f;
			group_id     = DEFAULT_SOUND_GROUP;

			b_activate             = false;
			b_eSoundRunAsync       = false;
			b_eSoundRunLoop        = false;
			b_eSoundRunStopAtStart = false;
			b_eSoundRunStopAtEnd   = false;
			b_eSoundRunMix         = false;
			b_UI                   = false;

			max_dist_reached       = false;
			engine_status_run      = false;
			engine_status_pause    = false;

			disable_forced         = false;
		}
	public:
		void Close() override final
		{
			Reset();
			MapObjectSetting::Close();
		}
	public:
		float volume_max;            // ограничитель (от 0.0 до 1.0) на изменения текущего <volume>
		float volume_start;          // запомненное стартовое значение
		float volume;                // текущее значение
		float max_distance;          // максимальное расстояние слышимости
		uint32 group_id;             // заказанный номер группы
		bool b_activate;             // запуск сразу после загрузки?
		bool b_eSoundRunAsync;       // метод запуска - асинхронный
		bool b_eSoundRunLoop;        // метод запуска - зацикленный звук
		bool b_eSoundRunStopAtStart; // метод запуска - не сразу, а по вызову Resume()
		bool b_eSoundRunStopAtEnd;   // метод запуска - не закрывать поток после окончания (недействительно при зацикленном)
		bool b_eSoundRunMix;         // метод запуска - через свой микшер (не требует доп. интерфейса у аудиокарты) 
		bool b_UI;                   // не является частью сцены (звук интерфейса)
	public:
	////// POST SET //////
		uint64_gid GID;              // выданный звуковым движком идентификатор
		bool max_dist_reached;       // признак достижения максимального расстояния, когда звук уже не слышно
		bool engine_status_run;      // статус фактического запуска в звуковом движке
		bool engine_status_pause;    // статус фактической паузы в звуковом движке
	private:
		bool disable_forced; // заказ на отключение объекта звука
	private:
		struct MapSoundSetting_Status
		{
			MapSoundSetting_Status() { Reset(); }

			void Reset()
			{
				init   = false;
				run    = false;
				pause  = false;
			}

			bool  init;   // статус первичной инициализации
			bool  run;    
			bool  pause;
		};
		MapSoundSetting_Status stat_prev;
		MapSoundSetting_Status stat_cur;
	public:
		void SoundInit()
		{
			bool hRes = !stat_cur.init;

			if (hRes)
			{
				stat_cur.init = true;
				stat_cur.run  = b_activate;

				volume = volume_start;
			}		

			//return hRes;
		};
		void SoundReInit()
		{
			stat_prev.Reset();
			stat_cur.Reset();

			//return
				SoundInit();
		};
		
	public:
		void SoundRuleRun()    { stat_cur.run   = true;  }
		void SoundRuleStop()   { stat_cur.run   = false; }
		void SoundRulePause()  { stat_cur.pause = true;  }
		void SoundRuleResume() { stat_cur.pause = false; }
	public:
		void SoundRuleCLEAR()
		{
			stat_prev.run   = false;
			stat_prev.pause = false;
			stat_cur.run    = false;
			stat_cur.pause  = false;
		}
		void SoundRuleCLEARPAUSE(bool val)
		{
			stat_prev.pause = val;
			stat_cur.pause  = val;
		}
	public:
		//>> Сообщает битовый набор <eObjectAction> действий к исполнению
		uint32 SoundCheckAction()
		{
			uint32 hRes = eObjectAction::OBJACTION_NONE;

			bool force_to_run  = !stat_prev.run &&  stat_cur.run ;
			bool force_to_stop =  stat_prev.run && !stat_cur.run ;

			bool force_to_pause   = !stat_prev.pause &&  stat_cur.pause ;
			bool force_to_resume  =  stat_prev.pause && !stat_cur.pause ;

				 if (force_to_run)  hRes |= OBJACTION_RUN;
			else if (force_to_stop) hRes |= OBJACTION_STOP;

				 if (force_to_pause)  hRes |= OBJACTION_PAUSE;
			else if (force_to_resume) hRes |= OBJACTION_RESUME;

			// запоминаем статус
			stat_prev = stat_cur;

			return hRes;
		}
	
	public:
		void EnableObj() override final
		{
			// если выключен - включить и сбросить память запроса на отключение

			if (!enable)
			{
				enable = true;
				DisableObj_ResetForce();
			}		
		}
		void DisableObj() override final
		{
			disable_forced = true;
		}
		void DisableObj_ResetForce()
		{
			disable_forced = false;
		}
		bool DisableObj_GetForce()
		{
			return disable_forced;
		}
		void DisableObj_MakeForce()
		{
			if (disable_forced)
				enable = false;
		}
		
	};
	struct MapSkyboxSetting : public MapObjectSetting
	{
		friend class MapObjects;
		friend class CGameMap;
		MapSkyboxSetting() : MapObjectSetting() {}
		~MapSkyboxSetting() override final {}
		void Close() override final
		{
			data = nullptr;
			MapObjectSetting::Close();
		}
	protected:
		CSkyboxR *	data; // фактические данные скайбокса
	public:
		//>> Отрисовка скайбокса
		void Show()
		{
			data->ShowSkybox();
		}
	};
	struct MapSpriteSetting : public MapObjectSetting
	{
		friend class MapObjects;
		friend class CGameMap;
		struct MapSpriteInit
		{
			friend struct MapSpriteSetting;
		private:
			CSprite			sprite;
		public:
			std::string		prefix;		// имя или префикс имени в профиле
			eBillboardType	eBT;
			float			width;		// размеры полигона (в юнитах)
			float			height;		// размеры полигона (в юнитах)
			uint16			center_x;	// положение центра полигона (в пикселях)
			uint16			center_y;	// положение центра полигона (в пикселях)
			bool			animated;
			uint16			time;		// milliseconds
			uint16			frame;		// (optional) beginning frame
			uint16			frameF;		// (optional) first frame in render circle
			uint16			frameL;		// (optional) last  frame in render circle
		};
		MapSpriteSetting() : MapObjectSetting() { pos = data->pos; Close(); }
		~MapSpriteSetting() override final { }
		void Close() override final
		{
			init.prefix   = "";
			init.eBT      = EBT_NONE;
			init.width    = 1;
			init.height   = 1;
			init.center_x = MISSING;
			init.center_y = MISSING;
			init.animated = false;
			init.time     = 0;
			init.frame    = 0;
			init.frameF   = 0;
			init.frameL   = 0;
			data->Close();
			CObject::Close();
		}
	public:
		MapSpriteInit		init;					// исходные настройки
		CSprite	* const		data { &init.sprite };	// фактические данные спрайта
	public:
		//>> Отрисовка спрайта
		void Show()
		{
			data->Show();
		}
	};

	class CGameMap
	{
	public:
		struct MapSetup // основные настройки карты
		{
			MapSetup() : crc32() { Close(); };
			~MapSetup() { Close(); }
			void Close()
			{
				skybox        = L"MISSING SKYBOX TEXTURE";
				defpmodel     = L"MISSING DEFAULT PLAYER MODEL";
				ambient       = COLORVEC3(0, 0, 0);
				skybox_id     = 0;
				defpmodel_id  = MISSING;
				shownames     = true;
				pickmode      = true;
				crc32.m_crc32 = 0;
				mapname       = L"";
			}
			wstring   skybox;		// initial sky object name
			wstring   defpmodel;	// default player model
			COLORVEC3 ambient;		// ambient light
			float     maxpickdist;	// maximum picking distance
			bool      shownames;	// showing players names
			bool      pickmode;		// picking objects enable
			//////////////////////
			wstring   mapname;		// mem map name
			HCRC32    crc32;
			////// POST SET //////
			uint32    skybox_id;	// (POST SET) id of initial sky object :: 0 as default
			uint32    defpmodel_id; // (POST SET) id of default player model :: MISSING (-1) as default
		};
		struct MapLoadList // имена файлов к загрузке
		{
			struct MapLoadListData
			{
				void Close()
				{
					filename.erase (filename.begin(), filename.end());
					obj_id.erase   (obj_id.begin(),   obj_id.end());
				}

				wstring filename;       // имя файла
				vector <uint32> obj_id; // ссылающиеся на это имя объекты
			};

			void Close()
			{
				model.Close(1,1);
				skybox.Close(1,1);
				sound.Close(1,1);
				tex_sprite.Close(1,1);
			}

			VECPDATA <MapLoadListData>	model;		// models to load
			VECPDATA <MapLoadListData>	skybox;		// skyboxes to load
			VECPDATA <MapLoadListData>	sound;		// sounds to load
			VECPDATA <MapLoadListData>	tex_sprite;	// textures profiles to load
		};
		struct MapObjects // объекты сцены
		{
			void Close()
			{
				light.Close(1,1);
				model.Close(1,1);
				player.Close(1,1);
				sound.Close(1,1);
				skybox.Close(1,1);
				sprite.Close(1,1);
			};
			
			VECDATAP <MapLightSetting>			light;
			VECDATAP <MapModelSetting>			model;
			VECDATAP <MapPlayerSpawnSetting>	player;
			VECDATAP <MapSoundSetting>			sound;
			VECDATAP <MapSkyboxSetting>			skybox;
			VECDATAP <MapSpriteSetting>			sprite;
		};
		struct MapData // фактические данные, используемые объектами карты
		{
			void Close()
			{
				model.Close(1,1);
				skybox.Close(1,1);		
				tex_sprite.Close(1,1);
				sound.Delete(1);
				////
				light.Close();			
			};
			VECDATAP <CModelR>					model;
			VECDATAP <CSkyboxR>					skybox;
			VECDATAP <CTexProfileR>				tex_sprite;
			VECPDATA <TBUFFERS <byte, int32>>	sound;
			CBaseLight							light;
		};
	public:
		MapSetup		setup;	// базовые настройки карты		
		MapObjects		obj;	// объекты карты (ячейки памяти, описывающие состояние элемента сцены)
		MapData			data;	// фактические данные, используемые объектами карты (буферы вершин моделей, текстуры и др.)
	protected:
		MapLoadList		list;	// имена файлов к загрузке
		uint32			sky_id;	// текущий активный skybox
	protected:
		bool			isInit;		// файл карты загружен ?
		bool			isLoaded;	// связанные с картой файлы загружены ?
	public:
		CGameMap(const CGameMap& src)				= delete;
		CGameMap(CGameMap&& src)					= delete;
		CGameMap& operator=(CGameMap&& src)			= delete;
		CGameMap& operator=(const CGameMap& src)	= delete;
	public:
		CGameMap() : isInit(false), isLoaded(false) {};
		~CGameMap() { Close(); };
		void Close()
		{
			if (isInit)
			{
				setup.Close();
				list.Close();
				obj.Close();
				data.Close();
				sky_id   = 0;
				isInit   = false;
				isLoaded = false;
			}
		}
		bool IsInit()   { return isInit; }
		bool IsLoaded() { return isLoaded; }

	public:
		//>> Загрузчик карты
		bool Load(const wchar_t* gamePath, const wchar_t* filename)
		{
			if (isInit) { _MBM(ERROR_InitAlready); return false; }

			wchar_t syspath[MAX_PATH], error[MAX_PATH * 2], p1[MAX_PATH], p2[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, syspath);

			wchar_t *p = nullptr;

			wsprintf(error, L"%s %s", ERROR_OpenFile, filename);
			wsprintf(p1, L"%s\\%s\\%s", gamePath, DIRECTORY_MAPS, filename);
			wsprintf(p2, L"%s\\%s\\%s", syspath, DIRECTORY_MAPS, filename);

			if (p = LoadFileCheck({ p1, p2 })) return isInit = LoadProc(p);
			else _MBM(error);

			return false;
		}

	protected:
		//>> Загрузчик карты
		bool LoadProc(const wchar_t* filepath)
		{
			TBUFFER <byte, int32> file;								//wprintf(L"\nReading %s", filepath);
			ReadFileInMemory(filepath, file, 0);					if (file.buf == nullptr) return false;
			byte* fpbuf = file.buf;
			byte* fpend = fpbuf + file.count;

			setup.mapname.assign(GetFilename(filepath));
			setup.crc32.ProcessCRC(file.buf, file.count);
			setup.crc32.ProcessCRC((void*)setup.mapname.c_str(), (int)setup.mapname.size() * 2);

			eObjectType cur_type = OBJECT_NONE;

			CountMapObjects(file.buf, file.count);

			for (; fpbuf != fpend; fpbuf++)
			{
				CODESKIPCOMMENTS
				else if (_EQUAL(*fpbuf, ':')) // параметры
				{
					switch (cur_type)
					{
					case OBJECT_NONE:   ReadProperties(cur_type, 0, ++fpbuf);                     break;
					case OBJECT_MODEL:  ReadProperties(cur_type, obj.model.last_id(),  ++fpbuf);  break;
					case OBJECT_PLAYER: ReadProperties(cur_type, obj.player.last_id(), ++fpbuf);  break;
					case OBJECT_LIGHT:  ReadProperties(cur_type, obj.light.last_id(),  ++fpbuf);  break;
					case OBJECT_SKYBOX: ReadProperties(cur_type, obj.skybox.last_id(), ++fpbuf);  break;
					case OBJECT_SPRITE: ReadProperties(cur_type, obj.sprite.last_id(), ++fpbuf);  break;
					case OBJECT_SOUND:  ReadProperties(cur_type, obj.sound.last_id(),  ++fpbuf);  break;
					}
					CODESKIPSTRING
				}
				else if (Compare(fpbuf, _SLEN("setup"))) // основные настройки
				{
					cur_type = OBJECT_NONE;
					CODESKIPSTRING
				}
				else if (Compare(fpbuf, _SLEN("model"))) // объекты моделей
				{
					cur_type = OBJECT_MODEL;
					obj.model.AddCreate(1);
					CODESKIPSTRING
				}
				else if (Compare(fpbuf, _SLEN("player"))) // точки появления игрока
				{
					cur_type = OBJECT_PLAYER;
					obj.player.AddCreate(1);
					CODESKIPSTRING
				}
				else if (Compare(fpbuf, _SLEN("light"))) // источники света
				{
					cur_type = OBJECT_LIGHT;
					obj.light.AddCreate(1);
					CODESKIPSTRING
				}
				else if (Compare(fpbuf, _SLEN("skybox"))) // окружение
				{
					cur_type = OBJECT_SKYBOX;
					obj.skybox.AddCreate(1);
					CODESKIPSTRING
				}
				else if (Compare(fpbuf, _SLEN("sprite"))) // спрайты
				{
					cur_type = OBJECT_SPRITE;
					obj.sprite.AddCreate(1);
					CODESKIPSTRING
				}
				else if (Compare(fpbuf, _SLEN("sound"))) // звуки
				{
					cur_type = OBJECT_SOUND;
					obj.sound.AddCreate(1);
					CODESKIPSTRING
				}
			}

			SortLoadedFileNames();
			MakeParentLinking();

			//PrintfDebugInfo(OBJECT_SPRITE); getchar();

			return true;
		}
		//>> Подсчёт количества объектов и резервирование памяти
		void CountMapObjects(byte* fpbuf, uint32 fpsize)
		{
			uint32 _player = 0;
			uint32 _model  = 0;
			uint32 _light  = 0;
			uint32 _skybox = 0;
			uint32 _sprite = 0;
			uint32 _sound  = 0;

			byte* fpend = fpbuf + fpsize;
			for (; fpbuf != fpend; fpbuf++)
			{
				CODESKIPCOMMENTS
				else if (_EQUAL(*fpbuf, ':')) CODESKIPSTRING // параметры
				else if (Compare(fpbuf, _SLEN("player")))  { _player++;  CODESKIPSTRING }
				else if (Compare(fpbuf, _SLEN("model")))   { _model++;   CODESKIPSTRING }
				else if (Compare(fpbuf, _SLEN("light")))   { _light++;   CODESKIPSTRING }
				else if (Compare(fpbuf, _SLEN("skybox")))  { _skybox++;  CODESKIPSTRING }
				else if (Compare(fpbuf, _SLEN("sprite")))  { _sprite++;  CODESKIPSTRING }
				else if (Compare(fpbuf, _SLEN("sound")))   { _sound++;   CODESKIPSTRING }
			}

			obj.light.reserve(_light);
			obj.model.reserve(_model);
			obj.player.reserve(_player);
			obj.skybox.reserve(_skybox);
			obj.sprite.reserve(_sprite);
			obj.sound.reserve(_sound);

			/*
			printf("\nTOTAL MAP PLAYER  : %i", _player);
			printf("\nTOTAL MAP MODEL   : %i", _model);
			printf("\nTOTAL MAP LIGHT   : %i", _light);
			printf("\nTOTAL MAP SKYBOX  : %i", _skybox);
			//*/
		}
		//>> Чтение свойства
		void ReadProperties(eObjectType type, uint64 id, byte * fpbuf)
		{
			byte str[256];
			uint32 n;

			CODESKIPSPACE
			ZeroMemory(str, 256);
			CODEREADSTRINGTEXT

			switch (type)
			{
			case OBJECT_NONE:   ReadPropertySetup(str,  fpbuf);      break;
			case OBJECT_MODEL:  ReadPropertyModel(str,  fpbuf, id);  break;
			case OBJECT_PLAYER: ReadPropertyPlayer(str, fpbuf, id);  break;
			case OBJECT_LIGHT:  ReadPropertyLight(str,  fpbuf, id);  break;
			case OBJECT_SKYBOX: ReadPropertySkybox(str, fpbuf, id);  break;
			case OBJECT_SPRITE: ReadPropertySprite(str, fpbuf, id);  break;
			case OBJECT_SOUND:  ReadPropertySound(str,  fpbuf, id);  break;
			default: ;
			}		
		}
		//>> Чтение настройки карты
		void ReadPropertySetup(byte * setting, byte * fpbuf)
		{
			uint32 readtype = 0;

				 if (Compare(setting, _SLEN("ambient")))      readtype = 1;
			else if (Compare(setting, _SLEN("pickmode")))     readtype = 2;
			else if (Compare(setting, _SLEN("shownames")))    readtype = 4;
			else if (Compare(setting, _SLEN("maxpickdist")))  readtype = 5;
			else if (Compare(setting, _SLEN("skybox")))       readtype = 101;
			else if (Compare(setting, _SLEN("player_model"))) readtype = 102;
			
			if (readtype)
			{
				char str[256];
				uint32 n;				//printf("\nReading setup type %i", readtype);
				float f;

				CODESKIPSPACE
				ZeroMemory(str, 256);
				if (readtype < 100) // not wstring
				{
					CODEREADSTRINGVALUE		//printf(" : %s", str);
				}
				else // wstring
				{
					CODEREADSTRINGWTEXT		//wprintf(L" : %s", (wchar_t*)str);
				}

				switch (readtype)
				{
				case 1: sscanf_s(str, "%f,%f,%f", &setup.ambient.r, &setup.ambient.g, &setup.ambient.b); break;
				case 2: sscanf_s(str, "%i", &n); setup.pickmode = _BOOL(n);		break;
				case 4: sscanf_s(str, "%i", &n); setup.shownames = _BOOL(n);	break;
				case 5: sscanf_s(str, "%f", &f); setup.maxpickdist = f;			break;
				case 101: setup.skybox.assign((wchar_t*)str); break;
				case 102: setup.defpmodel.assign((wchar_t*)str); break;
				}
			} //else printf("\nUnknown setup type : %s", setting);
		}
		//>> Чтение настройки игрока (spawn)
		void ReadPropertyPlayer(byte * setting, byte * fpbuf, uint64 id)
		{
			uint32 readtype = 0;

				 if (Compare(setting, _SLEN("view")))     readtype = 3;
			else if (Compare(setting, _SLEN("enable")))   readtype = 4;
			else if (Compare(setting, _SLEN("parent_m"))) readtype = 98;
			else if (Compare(setting, _SLEN("pos")))      readtype = 99;
			else if (Compare(setting, _SLEN("name")))     readtype = 101;
			else if (Compare(setting, _SLEN("parent")))   readtype = 103;

			if (readtype)
			{
				char str[256];
				uint32 n;				//printf("\nReading player type %i", readtype);

				CODESKIPSPACE
				ZeroMemory(str, 256);
				if (readtype < 100) // not wstring
				{
					CODEREADSTRINGVALUE		//printf(" : %s", str);
				}
				else // wstring
				{
					CODEREADSTRINGWTEXT		//wprintf(L" : %s", (wchar_t*)str);
				}

				WPOS * pos;
				switch (readtype)
				{
				case 3:  sscanf_s(str, "%i", &n); obj.player[id]->camera_mode = (eCameraMode)n;      break;
				case 4:  sscanf_s(str, "%i", &n); obj.player[id]->enable = _BOOL(n);                 break;
				case 98: sscanf_s(str, "%i", &n); obj.player[id]->parent.method = (eParentMethod)n;  break;
				case 99:
					pos = obj.player[id]->pos;
					sscanf_s(str, "%f,%f,%f;%f,%f,%f;%f,%f,%f",
						&pos->x,  &pos->y,  &pos->z,
						&pos->ax, &pos->ay, &pos->az,
						&pos->sx, &pos->sy, &pos->sz);
					break;
				case 101: obj.player[id]->name.assign((wchar_t*)str);        break;
				case 103: obj.player[id]->parent.name.assign((wchar_t*)str); break;
				}
			} //else printf("\nUnknown player type : %s", setting);
		}
		//>> Чтение настройки предметов
		void ReadPropertyModel(byte * setting, byte * fpbuf, uint64 id)
		{
			uint32 readtype = 0;
 
			     if (Compare(setting, _SLEN("dynamic")))  readtype = 3;
			else if (Compare(setting, _SLEN("pick")))     readtype = 4;
			else if (Compare(setting, _SLEN("enable")))   readtype = 5;
			else if (Compare(setting, _SLEN("player")))   readtype = 6;
			else if (Compare(setting, _SLEN("parent_m"))) readtype = 98;
			else if (Compare(setting, _SLEN("pos")))      readtype = 99;
			else if (Compare(setting, _SLEN("name")))     readtype = 101;
			else if (Compare(setting, _SLEN("file")))     readtype = 102;
			else if (Compare(setting, _SLEN("parent")))   readtype = 103;

			if (readtype)
			{
				char str[256];
				uint32 n;				//printf("\nReading model type %i", readtype);

				CODESKIPSPACE
				ZeroMemory(str, 256);
				if (readtype < 100) // not wstring
				{
					CODEREADSTRINGVALUE		//printf(" : %s", str);
				}
				else // wstring
				{
					CODEREADSTRINGWTEXT		//wprintf(L" : %s", (wchar_t*)str);
				}			

				WPOS * pos;
				switch (readtype)
				{
				case 3: sscanf_s(str, "%i", &n);  obj.model[id]->dynamic = _BOOL(n);                break;
				case 4: sscanf_s(str, "%i", &n);  obj.model[id]->pick = _BOOL(n);                   break;
				case 5: sscanf_s(str, "%i", &n);  obj.model[id]->enable = _BOOL(n);                 break;
				case 6: sscanf_s(str, "%i", &n);  obj.model[id]->player = _BOOL(n);                 break;
				case 98: sscanf_s(str, "%i", &n); obj.model[id]->parent.method = (eParentMethod)n;  break;
				case 99:
					pos = obj.model[id]->pos;
					sscanf_s(str, "%f,%f,%f;%f,%f,%f;%f,%f,%f",
						&pos->x,  &pos->y,  &pos->z,
						&pos->ax, &pos->ay, &pos->az,
						&pos->sx, &pos->sy, &pos->sz);
					break;
				case 101: obj.model[id]->name.assign((wchar_t*)str);        break;
				case 102: obj.model[id]->filename.assign((wchar_t*)str);    break;
				case 103: obj.model[id]->parent.name.assign((wchar_t*)str); break;
				}
			} //else printf("\nUnknown model type : %s", setting);
		}
		//>> Чтение настройки источника освещения
		void ReadPropertyLight(byte * setting, byte * fpbuf, uint64 id)
		{
			uint32 readtype = 0;

			     if (Compare(setting, _SLEN("type")))      readtype = 2;
			else if (Compare(setting, _SLEN("range")))     readtype = 3;
			else if (Compare(setting, _SLEN("att0")))      readtype = 4;
			else if (Compare(setting, _SLEN("att1")))      readtype = 5;
			else if (Compare(setting, _SLEN("att2")))      readtype = 6;
			else if (Compare(setting, _SLEN("phi")))       readtype = 7;
			else if (Compare(setting, _SLEN("theta")))     readtype = 8;
			else if (Compare(setting, _SLEN("falloff")))   readtype = 9;
			else if (Compare(setting, _SLEN("dynamic")))   readtype = 10;
			else if (Compare(setting, _SLEN("enable")))    readtype = 11;
			else if (Compare(setting, _SLEN("diffuse")))   readtype = 95;
			else if (Compare(setting, _SLEN("specular")))  readtype = 96;
			else if (Compare(setting, _SLEN("direction"))) readtype = 97;
			else if (Compare(setting, _SLEN("parent_m")))  readtype = 98;
			else if (Compare(setting, _SLEN("position")))  readtype = 99;
			else if (Compare(setting, _SLEN("name")))      readtype = 101;
			else if (Compare(setting, _SLEN("parent")))    readtype = 103;

			if (readtype)
			{
				char str[256];
				uint32 n;				//printf("\nReading light type %i", readtype);
				float  f;

				CODESKIPSPACE
				ZeroMemory(str, 256);
				if (readtype < 100) // not wstring
				{
					CODEREADSTRINGVALUE		//printf(" : %s", str);
				}
				else // wstring
				{
					CODEREADSTRINGWTEXT		//wprintf(L" : %s", (wchar_t*)str);
				}

				auto set = obj.light[id]->setting;
				auto pos = obj.light[id]->pos;
				switch (readtype)
				{
				case 2:  sscanf_s(str, "%i", &n);  set->type = (eLightType)n;  break;
				case 3:  sscanf_s(str, "%f", &f);  set->range = f;   break;
				case 4:  sscanf_s(str, "%f", &f);  set->att0 = f;    break;
				case 5:  sscanf_s(str, "%f", &f);  set->att1 = f;    break;
				case 6:  sscanf_s(str, "%f", &f);  set->att2 = f;    break;
				case 7:  sscanf_s(str, "%f", &f);  set->phi = f;     break;
				case 8:  sscanf_s(str, "%f", &f);  set->theta = f;   break;
				case 9:  sscanf_s(str, "%f", &f);  set->falloff = f; break;
				case 10: sscanf_s(str, "%i", &n);  set->dynamic = _BOOL(n);  break;
				case 11: sscanf_s(str, "%i", &n);  set->enable = _BOOL(n);  obj.light[id]->enable = set->enable;  break;
				case 95: sscanf_s(str, "%f,%f,%f", &set->diffuse.r,  &set->diffuse.g,  &set->diffuse.b);   break;
				case 96: sscanf_s(str, "%f,%f,%f", &set->specular.r, &set->specular.g, &set->specular.b);  break;
				case 97: sscanf_s(str, "%f,%f,%f", &pos->ax, &pos->ay, &pos->az);     break;
				case 99: sscanf_s(str, "%f,%f,%f", &pos->x,  &pos->y,  &pos->z);      break;
				///////
				case 98: sscanf_s(str, "%i", &n); obj.light[id]->parent.method = (eParentMethod)n;  break;
				case 101: obj.light[id]->name.assign((wchar_t*)str);        break;
				case 103: obj.light[id]->parent.name.assign((wchar_t*)str); break;
				}
			} //else printf("\nUnknown light type : %s", setting);
		}
		//>> Чтение настройки окружения
		void ReadPropertySkybox(byte * setting, byte * fpbuf, uint64 id)
		{
			uint32 readtype = 0;

				 if (Compare(setting, _SLEN("texture"))) readtype = 102;
			else if (Compare(setting, _SLEN("name")))    readtype = 101;

			if (readtype)
			{
				char str[256];
				uint32 n;				//printf("\nReading skybox type %i", readtype);

				CODESKIPSPACE
				ZeroMemory(str, 256);
				if (readtype < 100) // not wstring
				{
					CODEREADSTRINGVALUE		//printf(" : %s", str);
				}
				else // wstring
				{
					CODEREADSTRINGWTEXT		//wprintf(L" : %s", (wchar_t*)str);
				}

				switch (readtype)
				{
				case 101: obj.skybox[id]->name.assign((wchar_t*)str); break;
				case 102: obj.skybox[id]->filename.assign((wchar_t*)str); break;
				}
			}
		}
		//>> Чтение настройки спрайтов
		void ReadPropertySprite(byte * setting, byte * fpbuf, uint64 id)
		{
			uint32 readtype = 0;

				 if (Compare(setting, _SLEN("billboard")))   readtype = 1;
			else if (Compare(setting, _SLEN("animated")))    readtype = 2;
			else if (Compare(setting, _SLEN("frames")))      readtype = 3;
			else if (Compare(setting, _SLEN("frame")))       readtype = 4;
			else if (Compare(setting, _SLEN("time")))        readtype = 5;
			else if (Compare(setting, _SLEN("width")))       readtype = 6;
			else if (Compare(setting, _SLEN("height")))      readtype = 7;
			else if (Compare(setting, _SLEN("center_dx")))   readtype = 8;
			else if (Compare(setting, _SLEN("center_dy")))   readtype = 9;
			else if (Compare(setting, _SLEN("enable")))      readtype = 10;
			else if (Compare(setting, _SLEN("parent_m")))    readtype = 96;
			else if (Compare(setting, _SLEN("color_f")))     readtype = 97;
			else if (Compare(setting, _SLEN("color")))       readtype = 98;
			else if (Compare(setting, _SLEN("pos")))         readtype = 99;
			else if (Compare(setting, _SLEN("name")))        readtype = 101;
			else if (Compare(setting, _SLEN("file")))        readtype = 102;
			else if (Compare(setting, _SLEN("parent")))      readtype = 103;
			else if (Compare(setting, _SLEN("prefix")))      readtype = 201;

			if (readtype)
			{
				char str[256];
				uint32 n, n2, n3, n4;				//printf("\nReading sprite type %i", readtype);
				float f;

				CODESKIPSPACE
				ZeroMemory(str, 256);
				if (readtype < 100) // not wstring
				{
					CODEREADSTRINGVALUE		//printf(" : %s", str);
				}
				else if (readtype < 200) // wstring
				{
					CODEREADSTRINGWTEXT		//wprintf(L" : %s", (wchar_t*)str);
				}
				else // std::string
				{
					CODEREADSTRINGTEXT
				}

				WPOS * pos;
				switch (readtype)
				{
				case 1: sscanf_s(str, "%i", &n);          obj.sprite[id]->init.eBT = (eBillboardType)n;      break;
				case 2: sscanf_s(str, "%i", &n);          obj.sprite[id]->init.animated = _BOOL(n);          break;
				case 3: sscanf_s(str, "%i, %i", &n, &n2); obj.sprite[id]->init.frameF = n;
				                                          obj.sprite[id]->init.frameL = n2;                  break;
				case 4: sscanf_s(str, "%i", &n);          obj.sprite[id]->init.frame = n;                    break;
				case 5: sscanf_s(str, "%i", &n);          obj.sprite[id]->init.time  = n;                    break;
				case 6: sscanf_s(str, "%f", &f);          obj.sprite[id]->init.width = f;                    break;
				case 7: sscanf_s(str, "%f", &f);          obj.sprite[id]->init.height = f;                   break;
				case 8: sscanf_s(str, "%i", &n);          obj.sprite[id]->init.center_x = n;                 break;
				case 9: sscanf_s(str, "%i", &n);          obj.sprite[id]->init.center_y = n;                 break;
				case 10: sscanf_s(str, "%i", &n);         obj.sprite[id]->enable = _BOOL(n);                 break;
				case 96: sscanf_s(str, "%i", &n);         obj.sprite[id]->parent.method = (eParentMethod)n;  break;
				case 97: {
					COLORVEC & color = obj.sprite[id]->color;
					sscanf_s(str, "%f,%f,%f,%f",
						&color.r, &color.g, &color.b, &color.a);
					break; }
				case 98: {
					COLORVEC & color = obj.sprite[id]->color;
					sscanf_s(str, "%i,%i,%i,%i",
						&n, &n2, &n3, &n4);
					color.r = COLORFLOAT(n);
					color.g = COLORFLOAT(n2);
					color.b = COLORFLOAT(n3);
					color.a = COLORFLOAT(n4);
					break; }
				case 99:
					pos = obj.sprite[id]->pos;
					sscanf_s(str, "%f,%f,%f;%f,%f,%f;%f,%f,%f",
						&pos->x,  &pos->y,  &pos->z,
						&pos->ax, &pos->ay, &pos->az,
						&pos->sx, &pos->sy, &pos->sz);
					break;
				case 101: obj.sprite[id]->name.assign((wchar_t*)str);        break;
				case 102: obj.sprite[id]->filename.assign((wchar_t*)str);    break;
				case 103: obj.sprite[id]->parent.name.assign((wchar_t*)str); break;
				case 201: obj.sprite[id]->init.prefix.assign(str);           break;
				}
			} //else printf("\nUnknown sprite type : %s", setting);
			//*/
		}
		//>> Чтение настройки звуков
		void ReadPropertySound(byte * setting, byte * fpbuf, uint64 id)
		{
			uint32 readtype = 0;

				 if (Compare(setting, _SLEN("async")))       readtype = 1;
			else if (Compare(setting, _SLEN("loop")))        readtype = 2;
			else if (Compare(setting, _SLEN("stop_start")))  readtype = 3;
			else if (Compare(setting, _SLEN("stop_end")))    readtype = 4;
			else if (Compare(setting, _SLEN("in_mixer")))    readtype = 5;
			else if (Compare(setting, _SLEN("interface")))   readtype = 6;
			else if (Compare(setting, _SLEN("activate")))    readtype = 7;
			else if (Compare(setting, _SLEN("group_id")))    readtype = 8;
			else if (Compare(setting, _SLEN("volume_max")))  readtype = 9;
			else if (Compare(setting, _SLEN("volume")))      readtype = 10;
			else if (Compare(setting, _SLEN("distance")))    readtype = 11;
			else if (Compare(setting, _SLEN("enable")))      readtype = 12;
			else if (Compare(setting, _SLEN("parent_m")))    readtype = 98;
			else if (Compare(setting, _SLEN("pos")))         readtype = 99;
			else if (Compare(setting, _SLEN("name")))        readtype = 101;
			else if (Compare(setting, _SLEN("file")))        readtype = 102;
			else if (Compare(setting, _SLEN("parent")))      readtype = 103;

			if (readtype)
			{
				char str[256];
				uint32 n=0, n2=0, n3=0, n4=0;				//printf("\nReading sound type %i", readtype);
				float f;

				CODESKIPSPACE
				ZeroMemory(str, 256);
				if (readtype < 100) // not wstring
				{
					CODEREADSTRINGVALUE		//printf(" : %s", str);
				}
				else if (readtype < 200) // wstring
				{
					CODEREADSTRINGWTEXT		//wprintf(L" : %s", (wchar_t*)str);
				}
				else // std::string
				{
					CODEREADSTRINGTEXT
				}

				//WPOS * pos;
				auto pos = obj.sound[id]->pos;
				switch (readtype)
				{
				case 1: sscanf_s(str, "%i", &n); obj.sound[id]->b_eSoundRunAsync       = _BOOL(n); break;
				case 2: sscanf_s(str, "%i", &n); obj.sound[id]->b_eSoundRunLoop        = _BOOL(n); break;
				case 3: sscanf_s(str, "%i", &n); obj.sound[id]->b_eSoundRunStopAtStart = _BOOL(n); break;
				case 4: sscanf_s(str, "%i", &n); obj.sound[id]->b_eSoundRunStopAtEnd   = _BOOL(n); break;
				case 5: sscanf_s(str, "%i", &n); obj.sound[id]->b_eSoundRunMix         = _BOOL(n); break;
				case 6: sscanf_s(str, "%i", &n); obj.sound[id]->b_UI                   = _BOOL(n); break;
				case 7: sscanf_s(str, "%i", &n); obj.sound[id]->b_activate             = _BOOL(n); break;
				case 8: sscanf_s(str, "%i", &n); obj.sound[id]->group_id               = n; break;
				case 9: sscanf_s(str, "%f", &f); obj.sound[id]->volume_max             = f; break;
				case 10: sscanf_s(str, "%f", &f); obj.sound[id]->volume_start          = f;        // запоминаем стартовое значение
					                              obj.sound[id]->volume                = f; break; // устанавливаем стартовое текущим
				case 11: sscanf_s(str, "%f", &f); obj.sound[id]->max_distance          = f; break;
				case 12: sscanf_s(str, "%i", &n); obj.sound[id]->enable                = _BOOL(n); break;
				case 98: sscanf_s(str, "%i", &n); obj.sound[id]->parent.method = (eParentMethod)n;  break;
				case 99: sscanf_s(str, "%f,%f,%f", &pos->x,  &pos->y,  &pos->z);                    break;
				case 101: obj.sound[id]->name.assign((wchar_t*)str);        break;
				case 102: obj.sound[id]->filename.assign((wchar_t*)str);    break;
				case 103: obj.sound[id]->parent.name.assign((wchar_t*)str); break;
				}
			} //else printf("\nUnknown sound type : %s", setting);
			//*/
		}
		template<class T1, class T2>
		//>> Составление списка файлов на загрузку (тело процедуры)
		void SortLoadedFileNames_Proc(T1 & obj_vec, T2 & target_list)
		{
			uint32 i_obj = 0;
			for (auto & o : obj_vec)
			{
				bool name_already_in_list = false;	// отсортируем повторы
				uint32 listed_name_id = 0;			// текущий сравниваемый номер имени в списке

				for (auto _list : target_list)
				{
					if (!_list->filename.compare(o->filename)) // если такое имя уже есть в списке
					{			
						_list->obj_id.emplace_back(i_obj); // сохраним id объекта в списке
						o->id_list = listed_name_id;       // сохраним списковый id в объекте
						name_already_in_list = true;
						break;
					}
					listed_name_id++;
				}

				if (!name_already_in_list) // если мы так и не нашли такого имени в списке
				{				
					target_list.emplace_back_create(); // создаём новую страницу списка
					auto _list = target_list.last();   // .

					_list->filename = o->filename;     // дополним список новым именем
					_list->obj_id.emplace_back(i_obj); // сохраним id объекта в списке
					o->id_list = listed_name_id;	   // сохраним списковый id в объекте

					//wprintf(L"\nLIST NAME: %s", o->filename.c_str());
				}

				i_obj++; // next object
			}
		}
		//>> Составление списка файлов на загрузку
		void SortLoadedFileNames()
		{
			SortLoadedFileNames_Proc(obj.skybox, list.skybox);
			SortLoadedFileNames_Proc(obj.model,  list.model);
			SortLoadedFileNames_Proc(obj.sprite, list.tex_sprite);
			SortLoadedFileNames_Proc(obj.sound,  list.sound);

			bool default_sky_present = false;
			for (auto & o : obj.skybox)
				if (!o->name.compare(setup.skybox))
					{ default_sky_present = true; break; }

			if (!default_sky_present)
			{
				wchar_t error[256];
				wsprintf(error, L"Can't find initial skybox '%s' [map %s]", setup.skybox.c_str(), setup.mapname.c_str());
				_MBM(error);
			}
		}
		//>> TODO :: Связывает объекты друг с другом
		void MakeParentLinking()
		{
		/*	for (int tp = 0; tp < OBJECT_ENUM_MAX; tp++)
			{
				switch ((eObjectType)tp)
				{

				}
			} //*/
		}
		//>> debug
		void PrintfDebugInfo(eObjectType type)
		{
			wprintf(L"\n\nMap setup info");
			wprintf(L"\n- ambient      %f %f %f", setup.ambient.r, setup.ambient.g, setup.ambient.b);
			wprintf(L"\n- shownames    %i", (int)setup.shownames);
			wprintf(L"\n- pickmode     %i", (int)setup.pickmode);
			wprintf(L"\n- maxpickdist  %f", setup.maxpickdist);
			wprintf(L"\n- skybox       %s", setup.skybox.c_str());
			wprintf(L"\n- player_model %s", setup.defpmodel.c_str());

			if (type == OBJECT_NONE || type == OBJECT_SKYBOX)
			for (auto & cur : obj.skybox)
			{
				wprintf(L"\n\nSkybox info");
				wprintf(L"\n- name    %s", cur->name.c_str());
				wprintf(L"\n- texture %s", cur->filename.c_str());
			}

			if (type == OBJECT_NONE || type == OBJECT_PLAYER)
			for (auto & cur : obj.player)
			{
				auto pos = cur->pos;
				wprintf(L"\n\nPlayer info");
				wprintf(L"\n- name      %s", cur->name.c_str());
				wprintf(L"\n- parent    %s", cur->parent.name.c_str());
				wprintf(L"\n- parent_t  %i", cur->parent.type);
				wprintf(L"\n- parent_m  %i", cur->parent.method);
				wprintf(L"\n- view      %i", cur->camera_mode);
				wprintf(L"\n- enable    %i", cur->enable);
				wprintf(L"\n- pos %4.2f %4.2f %4.2f;  %4.2f %4.2f %4.2f;  %4.2f %4.2f %4.2f",
					pos->x,  pos->y,  pos->z,
					pos->ax, pos->ay, pos->az,
					pos->sx, pos->sy, pos->sz);
			}

			if (type == OBJECT_NONE || type == OBJECT_SPRITE)
			for (auto & cur : obj.sprite)
			{
				auto pos = cur->pos;
				wprintf(L"\n\nSprite info");
				wprintf(L"\n- name       %s",     cur->name.c_str());
				wprintf(L"\n- file       %s",     cur->filename.c_str());
				 printf( "\n- prefix     %s",     cur->init.prefix.c_str());
				wprintf(L"\n- parent     %s",     cur->parent.name.c_str());
				wprintf(L"\n- parent_t   %i",     cur->parent.type);
				wprintf(L"\n- parent_m   %i",     cur->parent.method);
				wprintf(L"\n- width      %f",     cur->init.width);
				wprintf(L"\n- height     %f",     cur->init.height);
				wprintf(L"\n- center_x   %i",     cur->init.center_x);
				wprintf(L"\n- center_y   %i",     cur->init.center_y);
				wprintf(L"\n- billboard  %i",     cur->init.eBT);
				wprintf(L"\n- animated   %i",     cur->init.animated);
				wprintf(L"\n- frame      %i",     cur->init.frame);
				wprintf(L"\n- frames     %i, %i", cur->init.frameF, cur->init.frameL);
				wprintf(L"\n- time       %i",     cur->init.time);
				wprintf(L"\n- enable     %i",     cur->enable);
				wprintf(L"\n- color %2.3f %2.3f %2.3f %2.3f",
					cur->color.r, cur->color.g, cur->color.b, cur->color.a);
				wprintf(L"\n- pos %4.2f %4.2f %4.2f;  %4.2f %4.2f %4.2f;  %4.2f %4.2f %4.2f",
					pos->x, pos->y, pos->z,
					pos->ax, pos->ay, pos->az,
					pos->sx, pos->sy, pos->sz);
			}

			if (type == OBJECT_NONE || type == OBJECT_MODEL)
			for (auto & cur : obj.model)
			{
				auto pos = cur->pos;
				wprintf(L"\n\nModel info");
				wprintf(L"\n- name     %s", cur->name.c_str());
				wprintf(L"\n- file     %s", cur->filename.c_str());
				wprintf(L"\n- parent   %s", cur->parent.name.c_str());
				wprintf(L"\n- parent_t %i", cur->parent.type);
				wprintf(L"\n- parent_m %i", cur->parent.method);
				wprintf(L"\n- dynamic  %i", cur->dynamic);
				wprintf(L"\n- pick     %i", cur->pick);
				wprintf(L"\n- enable   %i", cur->enable);
				wprintf(L"\n- player   %i", cur->player);
				wprintf(L"\n- pos %4.2f %4.2f %4.2f;  %4.2f %4.2f %4.2f;  %4.2f %4.2f %4.2f",
					pos->x,  pos->y,  pos->z,
					pos->ax, pos->ay, pos->az,
					pos->sx, pos->sy, pos->sz);
			}

			if (type == OBJECT_NONE || type == OBJECT_LIGHT)
			for (auto & cur : obj.light)
			{
				switch (cur->setting->type)
				{
				case LIGHT_DIRECT:
					wprintf(L"\n\nLight info : direct light");
					wprintf(L"\n- specular %4.2f %4.2f %4.2f",
						cur->setting->specular.r,
						cur->setting->specular.g,
						cur->setting->specular.b);
					wprintf(L"\n- directn  %4.2f %4.2f %4.2f",
						cur->pos->ax,
						cur->pos->ay,
						cur->pos->az);
					break;
				case LIGHT_POINT:
					wprintf(L"\n\nLight info : point light");
					wprintf(L"\n- specular %4.2f %4.2f %4.2f",
						cur->setting->specular.r,
						cur->setting->specular.g,
						cur->setting->specular.b);
					wprintf(L"\n- position %4.2f %4.2f %4.2f",
						cur->pos->x,
						cur->pos->y,
						cur->pos->z);
					wprintf(L"\n- range    %f", cur->setting->range);
					wprintf(L"\n- att0     %f", cur->setting->att0);
					wprintf(L"\n- att1     %f", cur->setting->att1);
					wprintf(L"\n- att2     %f", cur->setting->att2);
					break;
				case LIGHT_SPOT:
					wprintf(L"\n\nLight info : spot light");
					wprintf(L"\n- specular %4.2f %4.2f %4.2f",
						cur->setting->specular.r,
						cur->setting->specular.g,
						cur->setting->specular.b);
					wprintf(L"\n- directn  %4.2f %4.2f %4.2f",
						cur->pos->ax,
						cur->pos->ay,
						cur->pos->az);
					wprintf(L"\n- position %4.2f %4.2f %4.2f",
						cur->pos->x,
						cur->pos->y,
						cur->pos->z);
					wprintf(L"\n- range    %f", cur->setting->range);
					wprintf(L"\n- att0     %f", cur->setting->att0);
					wprintf(L"\n- att1     %f", cur->setting->att1);
					wprintf(L"\n- att2     %f", cur->setting->att2);
					wprintf(L"\n- phi      %f", cur->setting->phi);
					wprintf(L"\n- theta    %f", cur->setting->theta);
					wprintf(L"\n- falloff  %f", cur->setting->falloff);
					break;
				}
				    wprintf(L"\n- diffuse  %4.2f %4.2f %4.2f",
						cur->setting->diffuse.r,
						cur->setting->diffuse.g,
						cur->setting->diffuse.b);
				    wprintf(L"\n- name     %s", cur->name.c_str());
					wprintf(L"\n- parent   %s", cur->parent.name.c_str());
					wprintf(L"\n- parent_t %i", cur->parent.type);
					wprintf(L"\n- parent_m %i", cur->parent.method);
					wprintf(L"\n- dynamic  %i", cur->setting->dynamic);
				    wprintf(L"\n- enable   %i", cur->setting->enable);
			}
		};

	public:
		//>> Загрузчик данных
		void RAMDataLoader(const CShader * shader,
			               const DEV3DBASE * dev,
						   const CBaseCamera * camera,
						   const CStatusIO * IO,
						   const CSound * sound)
		{
			if (!isInit) { _MBM(ERROR_InitNone); return; }

			auto cam = const_cast<CBaseCamera*>(camera);
			RAMDataLoadSkybox(shader, dev, cam->GetNearPlane());
			RAMDataLoadLights(shader);
			RAMDataLoadSounds(const_cast<CSound*>(sound));
			RAMDataLoadSprites(shader, dev, camera, IO);
			RAMDataLoadModels(shader, dev);
			
			isLoaded = true;
		}
	
	protected:
		//>> Подготовка неба
		void RAMDataLoadSkybox(const CShader * shader, const DEV3DBASE * dev, float camera_near_plane)
		{
			uint32 i; // , n;

			i = 0;
			for (auto & _list : list.skybox) // список загружаемых skybox'ов
			{
				data.skybox.AddCreate(1);					// создаём новый фактический skybox
				uint32 N = (uint32)data.skybox.last_id();	// текущий skybox data ID
				auto skybox = data.skybox[N].get();
				skybox->Init(dev, shader);
				skybox->SetSphereTexture(DIRECTORY_GAME, _list->filename.c_str(), camera_near_plane + 1, CULL_DEFCCW);

			//	n = 0;
			//	for (auto & o : obj.skybox) // свяжем объект 'obj' с фактическими данными 'data'
			//	{
			//		if (o->id_list == i)
			//		{
			//			o->id_data = N;
			//			o->data = skybox;
			//		}
			//
			//		// установим текущий используемый skybox карты согласно настройкам //
			//
			//		if (setup.skybox_id == n) 
			//			this->sky_id = o->id_data;
			//
			//		n++; // next obj
			//	}

				for (auto obj_id : _list->obj_id) // свяжем объекты с фактическими данными
				{
					auto & o = obj.skybox[obj_id];

					o->id_data = N;
					o->data = skybox; // передаём в объект указатель на данные

					if (setup.skybox_id == obj_id) // установим текущий используемый skybox карты согласно настройкам
						this->sky_id = N;
				}

				i++; // next list
			}
		}
		//>> Подготовка света
		void RAMDataLoadLights(const CShader * shader)
		{
			data.light.SetShader(shader);
			data.light.ambient = setup.ambient;

			for (auto & o : obj.light)
			{
				uint32 N = (uint32)data.light.source.size();
				data.light.source.Add(1);
				data.light.source.Reset(o->setting_shr, N);
				o->id_data = N;
			}
		}
		//>> Подготовка моделей
		void RAMDataLoadModels(const CShader * shader, const DEV3DBASE * dev)
		{
			uint32 i = 0;
			for (auto & _list : list.model) // список загружаемых моделей
			{
				data.model.AddCreate(1);					// создаём новую модель
				uint32 N = (uint32)data.model.last_id();	// номер созданной модели
				auto model = data.model[N].get();

				if (!model->Load(DIRECTORY_GAME, _list->filename.c_str())) // загружаем...   TODO: ERROR-модель по умолчанию
				{
					wchar_t error[256];
					wsprintf(error, L"Can't find model '%s' [map %s]", _list->filename.c_str(), setup.mapname.c_str());
					_MBM(error);
				}
				model->SetDevice(dev);
				model->SetShader(shader);

			//	for (auto & o : obj.model) // свяжем объект с фактическими данными
			//	{
			//		if (o->id_list == i)
			//		{
			//			o->id_data = N;
			//			o->data = model; // передаём в объект указатель на данные модели
			//		}
			//
			//		// для статической модели сразу зададим мировую матрицу //
			//
			//		if (!o->dynamic) 
			//			o->id_matrix = model->StaticMatAdd(o->pos);
			//	}

				for (auto obj_id : _list->obj_id) // свяжем объекты с фактическими данными
				{
					auto & o = obj.model[obj_id];

					o->id_data = N;
					o->data = model; // передаём в объект указатель на данные

					if (!o->dynamic) // для статической модели сразу зададим мировую матрицу
						o->id_matrix = model->StaticMatAdd(o->pos);
				}

				if (!setup.defpmodel.compare(_list->filename)) // установим id для модели игрока по умолчанию
					setup.defpmodel_id = N;

				i++; // next list
			}
		}
		//>> Подготовка спрайтов
		void RAMDataLoadSprites(const CShader * shader, const DEV3DBASE * dev, const CBaseCamera * camera, const CStatusIO * IO)
		{
			uint32 i = 0;
			for (auto & _list : list.tex_sprite) // список загружаемых текстурных профилей
			{
				data.tex_sprite.AddCreate(1);					// создаём новый профиль
				uint32 N = (uint32)data.tex_sprite.last_id();	// номер созданного профиля
				auto tex = data.tex_sprite[N].get();

				tex->Load(DIRECTORY_GAME, DIRECTORY_SPRITES, _list->filename.c_str()); // загружаем...
				tex->SetDevice(dev);
				tex->SetShader(shader);
				tex->SetIO(IO);

			//	for (auto & o : obj.sprite) // свяжем объект с фактическими данными
			//		if (o->id_list == i)
			//			o->id_data = N;

				for (auto obj_id : _list->obj_id) // свяжем объекты с фактическими данными
				{
					auto & o = obj.sprite[obj_id];

					o->id_data = N;
				}

				i++; // next list
			}
		}
		//>> Подготовка звуков
		void RAMDataLoadSounds(CSound * sound)
		{
			wchar_t syspath[MAX_PATH], error[MAX_PATH * 2], p1[MAX_PATH], p2[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, syspath);

			uint32 i = 0;
			for (auto & _list : list.sound) // список загружаемых звуков
			{
				data.sound.AddCreate(1);					// создаём новый профиль
				uint32 N = (uint32)data.sound.last_id();	// номер созданного профиля
				auto file = data.sound[N];
				bool file_loaded = false;

				// загрузим файлы с ЖД
				{
					wchar_t *p = nullptr;

					wsprintf(p1, L"%s\\%s\\%s", DIRECTORY_GAME, DIRECTORY_SOUNDS, _list->filename.c_str());
					wsprintf(p2, L"%s\\%s\\%s",        syspath, DIRECTORY_SOUNDS, _list->filename.c_str());

				//	if (p = LoadFileCheck(2, p1, p2))
					if (p = LoadFileCheck( { p1, p2 } ))
					{
						file_loaded = ReadFileInMemory(p, *file, true);
					}
					else
					{
						wsprintf(error, L"%s %s", ERROR_OpenFile, _list->filename.c_str());
						_MBM(error);
						//wprintf(L"\n%s", error);
					}
				}

				// вгрузим в звуковой движок

				bool first = true;
				uint64_gid first_gid;

				for (auto obj_id : _list->obj_id) 
				{
					auto & o = obj.sound[obj_id];

					error_t hRes;
					if (file_loaded)
					{
						if (first) // проверку валидности файла - только 1 раз
						{
							hRes = sound->Load(file, _list->filename.c_str(), o->group_id, o->GID._u64);

							first     = false;
							first_gid = o->GID;

							if (hRes == eSoundError_TRUE)
							{
								sound->FormalNameSet(o->GID, o->name);
							}
						}
						else // остальные быстро связываем с первым
						{
							if (hRes == eSoundError_TRUE)
							{
								sound->Load(first_gid, o->group_id, o->GID._u64);
								sound->FormalNameSet(o->GID, o->name);
							}
						}

						o->id_data = N;
					}
				}

				i++; // next list
			}

			// можно очистить
			data.sound.Close(1,1);
		}

	public:
		//>> Загрузчик данных в видеопамять
		void RAMVideoDataLoader(const CBaseCamera * camera)
		{
			// Для подготовки спрайтов сначала требуется подготовить текстурные профили

			for (auto & texprofile : data.tex_sprite)
				texprofile->Prepare();

			// Теперь можно подготовить спрайты

			for (auto & o : obj.sprite)
			{
				auto sprite = o->data;
				if (o->init.animated)
				{
					sprite->Init(data.tex_sprite[o->id_data], camera, o->init.prefix.c_str(), o->init.time);
					if (o->init.frame)
						sprite->ResetAnimationFrame(o->init.frame);
					if (o->init.frameF && o->init.frameL)
						sprite->ResetAnimationFrame(o->init.frameF, o->init.frameL);
				}
				else
				{
					sprite->Init(data.tex_sprite[o->id_data], camera, o->init.prefix.c_str());
				}
				sprite->Reset(o->init.eBT);
				sprite->Reset(o->init.width, o->init.height, 1.f, 1.f, o->color);
				if (_NOMISS(o->init.center_x) && _NOMISS(o->init.center_y))
					sprite->Reset(true, o->init.center_x, o->init.center_y, true);
			}

			// Предзагрузка моделей
			for (auto & o : obj.model)
			{
				auto model = o->data;

				model->PrepareModel();
			}

			// Предзагрузка неба
			for (auto & o : obj.skybox)
			{
				auto skybox = o->data;

				skybox->PrepareSkybox();
			}
		}

	public:
		//>> Возвращает ID модели по умолчанию [0 as default]
		uint32 GetDefaultModelID()
		{
			return (_ISMISS(setup.defpmodel_id)) ? 0 : setup.defpmodel_id;
		}
		
	public:
		//>> Показ текущего скайбокса
		void ShowSkybox()
		{
			obj.skybox[sky_id]->Show();
		}
		//>> Устанавливает текущий скайбокс
		void SetSkybox(uint32 ID)
		{
			if (ID >= obj.skybox.size()) { _MBM(ERROR_IDLIMIT); return; }
			sky_id = ID;
		}
		
	public:
		//>> Включение шейдера источников света
		void EnableLightShader()
		{
			data.light.EnableLightShader();
		}
		//>> Выключение шейдера источников света
		void DisableLightShader()
		{
			data.light.DisableLightShader();
		}
	};

	class CGameMapManager
	{
	protected:
		const CShader *			shader	{ nullptr };
		const DEV3DBASE *		dev		{ nullptr };
		const CBaseCamera *		camera	{ nullptr };
		const CStatusIO *		IO		{ nullptr };
		const CSound *          sound   { nullptr };
	protected:
		VECDATAPP <CGameMapF>	map;		// набор карт
		uint32					cid;		// id текущей карты
		bool					isInit;
	public:
		CGameMapManager(const CGameMapManager& src)				= delete;
		CGameMapManager(CGameMapManager&& src)					= delete;
		CGameMapManager& operator=(CGameMapManager&& src)		= delete;
		CGameMapManager& operator=(const CGameMapManager& src)	= delete;
	public:
		CGameMapManager() : cid(0), isInit(false) { Set(1); }
		~CGameMapManager() { }
		void Close()
		{	
			if (!isInit) return;

			Reset(1);
			cid    = 0;
			shader = nullptr;
			dev    = nullptr;
			camera = nullptr;
			IO     = nullptr;
			sound  = nullptr;
			isInit = false;
		}

		CGameMapF * operator[](size_t idx) { return map[idx]; }

		bool Init(const DEV3DBASE * pDevice,
			      const CShader * pShader,
				  const CBaseCamera * pCamera, 
				  const CStatusIO * pIO,
				  const CSound * pSound)
		{
			if (isInit) return false;

			if (pShader == nullptr) { _MBM(ERROR_PointerNone); return false; }
			if (pDevice == nullptr) { _MBM(ERROR_PointerNone); return false; }
			if (pCamera == nullptr) { _MBM(ERROR_PointerNone); return false; }
			if (pIO     == nullptr) { _MBM(ERROR_PointerNone); return false; }
			if (pSound  == nullptr) { _MBM(ERROR_PointerNone); return false; }

			shader = pShader;
			dev    = pDevice;
			camera = pCamera;
			IO     = pIO;
			sound  = pSound;

			return isInit = true;
		}

	public:
		//>> Сообщает <id> карту
		auto GetMap(uint32 id) -> CGameMapF *
		{
			return map[id];
		}
		//>> Сообщает текущую карту
		auto GetMap() -> CGameMapF *
		{
			return map[cid];
		}
		//>> Сообщает ID текущей карты
		auto GetMapID() -> uint32
		{
			return cid;
		}
		//>> Сообщает имя текущей карты
		auto GetMapName() -> wstring
		{
			return map[cid]->setup.mapname;
		}

	public:
		//>> Устанавливает число слотов для карт до <count>
		void Set(uint32 count)
		{
			map.SetCreate(count);
		}
		//>> Добавляет число слотов для карт на <count>
		void Add(uint32 count)
		{
			map.AddCreate(count);
		}
		//>> Сообщает число слотов карт
		uint32 Count()
		{
			return (uint32)map.size();
		}
		//>> Сбрасывает и устанавливает число слотов карт до <count>
		void Reset(uint32 count)
		{
			if (!count) count = 1;
			map.Close(1,1);
			map.Create(count);
		}

	public:
		//>> Очищает занятую память от текущей карты (слот остаётся доступен)
		void DeleteMap()
		{
			if (map[cid] != nullptr)
				map[cid]->Close();
		}
		//>> Очищает занятую память от указанной <id> карты (слот остаётся доступен)
		void DeleteMap(uint32 id)
		{
			if (id >= Count()) return;

			if (map[id] != nullptr)
				map[id]->Close();
		}

	public:
		//>> Переключает текущую карту на <id> :: return <true> if succeeded
		bool Switch(uint32 id)
		{
			if (id >= Count())      return false;
			if (map[id] == nullptr) return false;
			if (!map[id]->IsInit()) return false;

			cid = id;

			return true;
		}
		//>> Загружает карту <filename> в слот <id> :: return <true> if succeeded
		bool Load(uint32 id, wstring & filename)
		{
			if (!isInit) { _MBM(ERROR_InitNone); return false; }

			DeleteMap(id);
			Set(id + 1);

			if (!map[id]->Load(DIRECTORY_GAME, filename.c_str())) return false;

			map[id]->RAMDataLoader(shader, dev, camera, IO, sound);

			return true;
		}
		//>> Заранее подгружает некоторые данные текущей карты в видеопамять
		bool LoadVRAM()
		{
			if (!isInit)               { _MBM(ERROR_InitNone); return false; }
			if (!map[cid]->IsLoaded()) { _MBM(ERROR_InitNone); return false; }

			map[cid]->RAMVideoDataLoader(camera);

			return true;
		}
	};
}

#endif // _GAMEMAP_H