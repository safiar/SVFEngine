// ----------------------------------------------------------------------- //
//
// MODULE  : Model.h
//
// PURPOSE : Загрузочный класс моделей
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _MODEL_H
#define _MODEL_H

#include "Object.h"

using namespace SAVFGAME;

namespace SAVFGAME
{
	struct MODELTEX // Данные текстуры (RAM)
	{ 
		MODELTEX()  { }
		~MODELTEX() { }
		void Close()
		{
			file.Close();
			name.erase(name.begin(), name.end());
		}		
		TBUFFER <byte, int32> file; // file.count :: < 0 > не было попыток чтения, < MISSING (-1) > не удалось прочесть
		wstring name;
	public:
		MODELTEX(const MODELTEX& src)				= delete;
		MODELTEX(MODELTEX&& src)					= delete;
		MODELTEX& operator=(MODELTEX&& src)			= delete;
		MODELTEX& operator=(const MODELTEX& src)	= delete;
	};
	struct MODELMTL { // Описывает свойства материала поверхности :: paulbourke.net/dataformats/mtl/
		MODELMTL()  { Close(); };
		~MODELMTL() { };
		void Close()
		{
			Ka = 0; Ns = 1; _Ka._default();
			Kd = 0; Ni = 1; _Kd._default();
			Ks = 0; Tr = 0; _Ks._default();
			illum = 0;
			d_Tr  = 1.f;
			bump        = L"";
			refl        = L"";
			map_Ka      = L"";
			map_Kd      = L"";
			map_Ks      = L"";
			map_Ns      = L"";
			map_bump    = L"";
			map_d       = L"";
			map_opacity = L"";
			id_bump        = MISSING;
			id_refl        = MISSING;
			id_map_Ka      = MISSING;
			id_map_Kd      = MISSING;
			id_map_Ks      = MISSING;
			id_map_Ns      = MISSING;
			id_map_bump    = MISSING;
			id_map_d       = MISSING;
			id_map_opacity = MISSING;
		}
		uint32 Ka;		// Ambient  : Цвет окружающего освещения (рассеянный свет самосвечения)
		uint32 Kd;		// Diffuse  : Цвет от прямого источника света
		uint32 Ks;		// Specular : Цвет зеркального отражения
		MATH3DVEC4 _Ka; // extra: float _Ka[3];
		MATH3DVEC4 _Kd; // extra: float _Kd[3];
		MATH3DVEC4 _Ks; // extra: float _Ks[3];
		float  Ns;		// Коэффициент зеркального отражения (от 0 до 1000)
						// Ns exponent :: Specifies the specular exponent for the current material.  This defines the focus of the specular highlight.
						// "exponent" is the value for the specular exponent.  A high exponent results in a tight, concentrated highlight.
						// Ns values normally range from 0 to 1000.
						//
		float  Ni;		// Specifies the optical density for the surface.  This is also known as index of refraction. 
						// "optical_density" is the value for the optical density.  The values can range from 0.001 to 10.
						// A value of 1.0 means that light does not bend as it passes through an object.  Increasing the optical_density 
						// increases the amount of bending.  Glass has an index of refraction of  about 1.5.
						// Values of less than 1.0 produce bizarre results and are not recommended.
						//
		char   illum;	// 0		Color on and Ambient off
						// 1		Color on and Ambient on
						// 2		Highlight on
						// 3		Reflection on and Ray trace on
						// 4		Transparency: Glass on					Reflection: Ray trace on
						// 5												Reflection: Fresnel on and Ray trace on
						// 6		Transparency: Refraction on				Reflection: Fresnel off and Ray trace on
						// 7		Transparency: Refraction on				Reflection: Fresnel on and Ray trace on
						// 8		Reflection on and Ray trace off
						// 9		Transparency: Glass on					Reflection: Ray trace off
						// 10		Casts shadows onto invisible surfaces
						//
		wstring bump;	// bump -s 1 1 1 -o 0 0 0 -bm 1 sand.mpb
						// -s option scales size of texture pattern
						// -o option offsets position of texture map by shifting position ; default 0, 0, 0
						// -bm option specifies a bump multiplier ; bump information is stored in a separate .mpb file
						//
		wstring refl;	// A reflection map is an environment that simulates reflections in specified objects.
						// The environment is represented by a color texture file or procedural texture file
						// that is mapped on the inside of an infinitely large, space.  Reflection maps can be
						// spherical or cubic.  A spherical reflection map requires only one texture or image file,
						// while a cubic reflection map requires six.
						// refl -type sphere -mm 0 1 clouds.mpc
						// refl -type sphere -options -args filename		The reflection map statement in the .mtl file
						// refl -type cube_side -options -args filename
						// side: cube_top, cube_bottom, cube_front, cube_back, cube_left, cube_right
						//
		float d_Tr;		// "d " or "Tr " :: dissolve (translucent), amount this material dissolves into the background			d -halo factor
						// 1.0f is fully opaque / 0.0f is fully dissolved
						// Unlike a real transparent material, the dissolve does not depend upon material thickness nor
						// does it have any spectral character.  Dissolve works on all illumination models.
						// Specifies that a dissolve is dependent on the surface orientation relative to the viewer.
						// For example, a sphere with the following  dissolve, d -halo 0.0, will be fully dissolved at
						// its center and will appear gradually more opaque toward its edge.
						// "factor" is the minimum amount of dissolve applied to the material. The amount of dissolve will
						// vary between 1.0 (fully opaque) and the specified "factor".         The formula is:  dissolve = 1.0 - (N*v)(1.0-factor)
		wstring map_Ka;		 // Имя файла с картой-текстурой для Ka (не обязательно)  result_Ka = Ka * map_Ka;
		wstring map_Kd;		 // Имя файла с картой-текстурой для Kd (не обязательно)  result_Kd = Kd * map_Kd;
		wstring map_Ks;		 // Имя файла с картой-текстурой для Ks (не обязательно)  result_Ks = Ks * map_Ks;
		wstring map_Ns;		 // Имя файла с картой-текстурой для Ns (не обязательно)  result_Ns = Ns * map_Ns;
		wstring map_bump;	 // Bump textures modify surface normals ; Dark areas are depressions and light areas are high points
		wstring map_d;		 // (?) прозрачность с alpha-канала текстуры
		wstring map_opacity; // (?) ...
		wstring name;		 // название данной структуры MODELMTL
		int16 id_bump;			// id к структуре MODELTEX * , держащей копию текстуры в памяти :: id == -1  копия отсутствует
		int16 id_refl;			// id к структуре MODELTEX * , держащей копию текстуры в памяти :: id == -1  копия отсутствует
		int16 id_map_Ka;		// id к структуре MODELTEX * , держащей копию текстуры в памяти :: id == -1  копия отсутствует
		int16 id_map_Kd;		// id к структуре MODELTEX * , держащей копию текстуры в памяти :: id == -1  копия отсутствует
		int16 id_map_Ks;		// id к структуре MODELTEX * , держащей копию текстуры в памяти :: id == -1  копия отсутствует
		int16 id_map_Ns;		// id к структуре MODELTEX * , держащей копию текстуры в памяти :: id == -1  копия отсутствует
		int16 id_map_bump;		// id к структуре MODELTEX * , держащей копию текстуры в памяти :: id == -1  копия отсутствует
		int16 id_map_d;			// id к структуре MODELTEX * , держащей копию текстуры в памяти :: id == -1  копия отсутствует
		int16 id_map_opacity;	// id к структуре MODELTEX * , держащей копию текстуры в памяти :: id == -1  копия отсутствует
		bool Tr;
	public:
		MODELMTL(const MODELMTL& src)				= delete;
		MODELMTL(MODELMTL&& src)					= delete;
		MODELMTL& operator=(MODELMTL&& src)			= delete;
		MODELMTL& operator=(const MODELMTL& src)	= delete;
	};
	struct MODELVERTEX { // Структура вершины модели
	public:
		MODELVERTEX(const MODELVERTEX& src)				= delete;
		MODELVERTEX(MODELVERTEX&& src)					= delete;
		MODELVERTEX& operator=(MODELVERTEX&& src)		= delete;
		MODELVERTEX& operator=(const MODELVERTEX& src)	= delete;
	public:
		MODELVERTEX() : P(), N(), TEX(), TEX2(), TEX3(), T(), B() {};
		MODELVERTEX(float x, float y, float z)
			: P(x,y,z), N(), TEX(), TEX2(), TEX3(), T(), B()																			{};
		MODELVERTEX(float x, float y, float z, float nx, float ny, float nz)
			: P(x,y,z), N(nx,ny,nz), TEX(), TEX2(), TEX3(), T(), B()																	{};
		MODELVERTEX(float x, float y, float z, float u,  float v)
			: P(x,y,z), N(), TEX(u,v), TEX2(), TEX3(), T(), B()																			{};
		MODELVERTEX(float x, float y, float z, float nx, float ny, float nz, float u, float v)
			: P(x,y,z), N(nx,ny,nz), TEX(u,v), TEX2(), TEX3(), T(), B()																	{};
		MODELVERTEX(float x, float y, float z, float nx, float ny, float nz, float u, float v, float u2, float v2)
			: P(x,y,z), N(nx,ny,nz), TEX(u,v), TEX2(u2,v2), TEX3(), T(), B()															{};
		MODELVERTEX(float x, float y, float z, float nx, float ny, float nz, float u, float v, float u2, float v2, float u3, float v3)
			: P(x,y,z), N(nx,ny,nz), TEX(u,v), TEX2(u2,v2), TEX3(u3,v3), T(), B()														{};
		MODELVERTEX(MATH3DVEC3 _P)
			: P(_P), N(), TEX(), TEX2(), TEX3(), T(), B()												{};
		MODELVERTEX(MATH3DVEC3 _P, MATH3DVEC3 _N)
			: P(_P), N(_N), TEX(), TEX2(), TEX3(), T(), B()												{};
		MODELVERTEX(MATH3DVEC3 _P, MATH3DVEC2 _TEX)
			: P(_P), N(), TEX(_TEX), TEX2(), TEX3(), T(), B()											{};
		MODELVERTEX(MATH3DVEC3 _P, MATH3DVEC3 _N, MATH3DVEC2 _TEX)
			: P(_P), N(_N), TEX(_TEX), TEX2(), TEX3(), T(), B()											{};
		MODELVERTEX(MATH3DVEC3 _P, MATH3DVEC3 _N, MATH3DVEC2 _TEX, MATH3DVEC2 _TEX2)
			: P(_P), N(_N), TEX(_TEX), TEX2(_TEX2), TEX3(), T(), B()									{};
		MODELVERTEX(MATH3DVEC3 _P, MATH3DVEC3 _N, MATH3DVEC2 _TEX, MATH3DVEC2 _TEX2, MATH3DVEC2 _TEX3)
			: P(_P), N(_N), TEX(_TEX), TEX2(_TEX2), TEX3(_TEX3), T(), B()								{};
		MATH3DVEC3 P;			// x, y, z     Координаты 3D  
		MATH3DVEC3 N;			// nx, ny, nz  Вектор-нормаль  
		MATH3DVEC2 TEX;			// u, v        Координаты первой текстуры (diffuse 1 / normal 1 / light 1 map)
		MATH3DVEC2 TEX2;		// u2, v2      Координаты второй текстуры (diffuse 2)
		MATH3DVEC2 TEX3;		// u3, v3      Координаты третьей текстуры (diffuse 3)
		MATH3DVEC3 T;			// tx, ty, tz  Тангенсальная составляющая (для рельефного текстурирования)
		MATH3DVEC3 B;			// bx, by, bz  Бинормальная / Битангенсальная составляющая (для рельефного текстурирования)
	};
	struct MODELSUB { // Описывает поверхность
		MODELSUB()  { Close(); };
		~MODELSUB() { };
		void Close()
		{
			v_ptr = nullptr;
			i_ptr = nullptr;
			v_id  = 0;
			i_id  = 0;
			nVert = 0;
			nInd  = 0;
			nPrim = 0;
			mtl    = L"";
			mtl_ID = MISSING;
			g_ID   = 0;
		}
		///////////
		MODELVERTEX  * v_ptr;	// указатель на начало в общей куче вертексов
		uint32       * i_ptr;	// указатель на начало в общей куче индексов
		uint32		 v_id;		// id в общей куче вертексов
		uint32		 i_id;		// id в общей куче индексов
		uint32       nVert;		// num vertices
		uint32       nInd;		// num indices
		uint32       nPrim;		// num primitives (triangles == nInd / 3)
		wstring		 mtl;		// имя материала
		int32		 mtl_ID;	// порядковый ID материала MODELMTL в памяти, -1 = не задано
		uint32       g_ID;		// обратная ссылка на GROUP, которому принадлежит набор
	public:
		MODELSUB(const MODELSUB& src)				= delete;
		MODELSUB(MODELSUB&& src)					= delete;
		MODELSUB& operator=(MODELSUB&& src)			= delete;
		MODELSUB& operator=(const MODELSUB& src)	= delete;
	};
	struct MODELGROUP : public CObject // Описывает группу поверхностей MODELSUB
	{
		MODELGROUP() : CObject(), pos(CObject::pos) { Close(); };
		~MODELGROUP() { };
		void Close()
		{
			usePos = false;
			gname = "";
			id.erase(id.begin(),id.end());
			pos->_default();
		}
		///////////
		std::string		gname;	// имя группы
		vector<uint32>	id;		// номера наборов SUB
		WPOS * const	pos;	// относительное месторасположение
		bool			usePos;	// учитывать в подсчётах позиции собственное смещение группы? :: <false> по умолчанию
	public:
		MODELGROUP(const MODELGROUP& src)				= delete;
		MODELGROUP(MODELGROUP&& src)					= delete;
		MODELGROUP& operator=(MODELGROUP&& src)			= delete;
		MODELGROUP& operator=(const MODELGROUP& src)	= delete;
	};
	struct MTLSORT { // Предоставляет связку конкретного материала MODELMTL со всеми использующими его поверхностями MODELSUB
		MTLSORT()  { Close(); };
		~MTLSORT() { };
		void Close()
		{
			id_mtl = MISSING;
			smodel.erase(smodel.begin(),smodel.end());
		}
		int32			id_mtl;
		vector<uint32>	smodel;		// номера наборов, использующих данный материал id_mtl
	public:
		MTLSORT(const MTLSORT& src)				= delete;
		MTLSORT(MTLSORT&& src)					= delete;
		MTLSORT& operator=(MTLSORT&& src)		= delete;
		MTLSORT& operator=(const MTLSORT& src)	= delete;
	};
	struct MODELBOX // представление модели в виде коробки
	{
	public:
		MODELBOX(const MODELBOX& src)				= delete;
		MODELBOX(MODELBOX&& src)					= delete;
		MODELBOX& operator=(MODELBOX&& src)			= delete;
		MODELBOX& operator=(const MODELBOX& src)	= delete;
	public:
		MODELBOX()  { Close(); }
		~MODELBOX() { }
		void Close()
		{
			maxCoord._default();
			minCoord._default();
			midCoord._default();
			midRadius._default();
			midR_max = 0;
			midR_min = 0;
			midR_avr = 0;
			for (auto & cur : point)   cur._default();
			for (auto & cur : face_Xp) cur._default();
			for (auto & cur : face_Xn) cur._default();
			for (auto & cur : face_Yp) cur._default();
			for (auto & cur : face_Yn) cur._default();
			for (auto & cur : face_Zp) cur._default();
			for (auto & cur : face_Zn) cur._default();
			normal_Xp._default();
			normal_Xn._default();
			normal_Yp._default();
			normal_Yn._default();
			normal_Zp._default();
			normal_Zn._default();
			plane_Xp._default();
			plane_Xn._default();
			plane_Yp._default();
			plane_Yn._default();
			plane_Zp._default();
			plane_Zn._default();
		}
		/////////
		MATH3DVEC	maxCoord;		// максимальные координаты модели от локального центра
		MATH3DVEC	minCoord;		// минимальные  координаты модели от локального центра
		MATH3DVEC	midCoord;		// min + (max - min) / 2
		MATH3DVEC	midRadius;		//       (max - min) / 2
		float		midR_max;		// max midRadius ( Rx || Ry || Rz )
		float		midR_min;		// min midRadius ( Rx || Ry || Rz )
		float		midR_avr;		// (Rx+Ry+Rz)/3
		MATH3DVEC	point[8];		// ящик по 8 точкам
		MATH3DVEC	face_Xp[4];		// +X (4 точки)
		MATH3DVEC	face_Xn[4];		// -X (4 точки)
		MATH3DVEC	face_Yp[4];		// +Y (4 точки)
		MATH3DVEC	face_Yn[4];		// -Y (4 точки)
		MATH3DVEC	face_Zp[4];		// +Z (4 точки)
		MATH3DVEC	face_Zn[4];		// -Z (4 точки)
		MATH3DVEC   normal_Xp;
		MATH3DVEC   normal_Xn;
		MATH3DVEC   normal_Yp;
		MATH3DVEC   normal_Yn;
		MATH3DVEC   normal_Zp;
		MATH3DVEC   normal_Zn;
		MATH3DPLANE plane_Xp;
		MATH3DPLANE plane_Xn;
		MATH3DPLANE plane_Yp;
		MATH3DPLANE plane_Yn;
		MATH3DPLANE plane_Zp;
		MATH3DPLANE plane_Zn;	
	};

	class CModel : public CObject
	{
	protected:
		TBUFFER <MODELVERTEX, uint32> vertex; // общая куча вертексов
		TBUFFER <uint32,      uint32> indice; // общая куча индексов
		VECPDATA <MODELSUB>           smodel;       // отдельные поверхности со своим материалом
		VECPDATA <MODELGROUP>         gmodel;       // логические группы поверхностей (напр.: левое колесо, правая дверь и т.п.)
		VECPDATA <MODELMTL>           material;     // материалы модели
		VECPDATA <MODELTEX>           texture;      // текстуры модели
		VECPDATA <MTLSORT>            mtlsort;      // сортировка наборов вершин по используемому материалу [непрозрачные -> прозрачные -> нет материала]
		int64				          mtlsort_tr;   // номер i-ого элемента, с которого идёт использование прозрачных материалов :: MISSING (-1) == нет прозрачных
		int64				          mtlsort_none; // номер i-ого элемента, с которого идёт отсутствие материалов :: MISSING (-1) == все заданы
	protected:
		bool  cullmode_CCW;
		bool  isInit;
	public:
		MODELBOX   box;		// представление в виде коробки [bounding box]
		wstring    name;	// ..\\Путь\\..\\имя.xxx  данной модели для справок
		HCRC32     crc32;	// hash from *.obj model file + *.mtl materials library files 
	public:
		CModel(const CModel& src)				= delete;
		CModel(CModel&& src)					= delete;
		CModel& operator=(CModel&& src)			= delete;
		CModel& operator=(const CModel& src)	= delete;
	public:
		CModel()              : CObject(), cullmode_CCW(true),     isInit(false) { };
		CModel(bool cull_CCW) : CObject(), cullmode_CCW(cull_CCW), isInit(false) { };
		~CModel() { Close(); };
	protected:
		void DeleteVI()  { vertex.Close(); indice.Close(); }
		void DeleteTEX() { texture.Close(1,1); }
	public:
		void Close()
		{
			if (isInit)
			{ 
				DeleteVI();
				DeleteTEX();
				smodel.Close(1,1);
				gmodel.Close(1,1);
				material.Close(1,1);
				mtlsort.Close(1,1);
				crc32.m_crc32 = 0;
				isInit = false;
			}
		}
		
	public:
		//>> Загрузчик
		bool Load(const wchar_t* gamePath, const wchar_t* objName)
		{
			if (isInit) { _MBM(ERROR_InitAlready);  return false; }

			wchar_t syspath[256], error[256], p1[256], p2[256];
			GetCurrentDirectory(256,syspath);
			wsprintf(error, L"%s %s", ERROR_OpenFile, objName);
			wsprintf(p1,L"%s\\%s",gamePath,DIRECTORY_MODELS);
			wsprintf(p2,L"%s\\%s",syspath,DIRECTORY_MODELS);

			if (!ObjInit(p1,objName,gamePath))
				if (!ObjInit(p2,objName,syspath)) { _MBM(error); return false; }

			return true;
		}

	protected:
		//>> Инициализация модели из obj-файла
		bool ObjInit(const wchar_t* objPath, const wchar_t* objName, const wchar_t* gamePath)
		{
			if (isInit) { _MBM(ERROR_InitAlready); return false; }

			wchar_t objFile[256];
			wsprintf(objFile,L"%s\\%s",objPath,objName);

			TBUFFER <byte, int32> file;								//wprintf(L"\nReading %s", objFile);
			ReadFileInMemory(objFile, file, 0);						if (file.buf == nullptr) return false;
			byte* fpbuf = file.buf;
			byte* fpend = fpbuf + file.count;

			Close(); // на всякий случай

			name.assign(objFile);
			crc32.ProcessCRC(file.buf, file.count);
			                                                                                      //макс. вертексов в описании 1-ой поверхности
			uint32 pointCount, normalCount, texCount, faceCount, groupCount, mtlCount, mtllibCount, maxVertexIndex;
			uint32 pointCur=0, normalCur=0, texCur=0, faceCur=0, groupCur=0, mtlCur=0, mtllibCur=0;
			ObjCount(fpbuf, file.count, pointCount, normalCount, texCount, faceCount, groupCount, mtlCount, mtllibCount, maxVertexIndex);

			uint32 nSModels, nGModels;

			if (!groupCount) { nGModels = 1;	       gmodel.Create(nGModels);	 gmodel[0]->gname = "default"; }
			else             { nGModels = groupCount;  gmodel.Create(nGModels);  groupCur = -1;                }

			mtlCur  = -1;
			nSModels = mtlCount;
			smodel.Create(nSModels);

			vector< MATH3DVEC4 >    point;		point.reserve(pointCount);	
			vector< MATH3DVEC3 >    normal;		normal.reserve(normalCount);
			vector< MATH3DVEC3 >    texpoint;	texpoint.reserve(texCount);
			vector< wstring >       mtllib;		mtllib.reserve(mtllibCount);

			uint32 numVertices = faceCount * maxVertexIndex;				// 2+1 треуг | 2+2 четырёхуг | 2+3 пятиуг    | ...
			uint32 numIndices  = faceCount * (maxVertexIndex - 2) * 3;		// 3*1 треуг | 3*2 два треуг | 3*3 три треуг | ...

			vertex.Create(numVertices);		vertex.count = 0; // Готовим к подсчёту
			indice.Create(numIndices);		indice.count = 0;

			MATH3DVEC4    data;		// point
			MATH3DVEC3    ndata;	// normal
			MATH3DVEC3    tdata;	// texpoint
			char  str[256];
			int32 have_w;
			uint32 iv, n;
			uint32 lastVertex = 0;	// для составления наборов SUBMODEL
			uint32 lastIndice = 0;	// для составления наборов SUBMODEL

			clock_t time  = clock();
			for( ; fpbuf!=fpend; fpbuf++)
			{
				CODESKIPCOMMENTS
				else if (_EQUAL(*fpbuf,'s')) CODESKIPSTRING
				else if (Compare(fpbuf, _SLEN("v "))) // Координаты 3d точки
				{			
					fpbuf += strlen("v ");
					have_w = 0;
					CODESKIPSPACE
					for(n=0;;n++,fpbuf++) // custom CODEREADSTRINGVALUE
					{
							 if (_EQUAL(*fpbuf,0x0A)) {         break;} //0A     \n
						else if (_EQUAL(*fpbuf,0x0D)) {fpbuf++; break;} //0D0A \r\n
						else if (_EQUAL(*fpbuf,0x20)) {have_w++;      } //' '
						str[n] = *fpbuf;
					}	str[n] = NULL;

					if (have_w==3) { sscanf_s(str,"%f %f %f %f", &data.x, &data.y, &data.z, &data.w);       }
					else           { sscanf_s(str,"%f %f %f",    &data.x, &data.y, &data.z); data.w = 1.0f; }
					point.emplace_back(data);
					//printf("\n%s\n%f %f %f %f", str, point[pointCur].x, point[pointCur].y, point[pointCur].z, point[pointCur].w);  getchar();
					pointCur++;
				}
				else if (Compare(fpbuf, _SLEN("vn "))) // Вектор нормали
				{
					fpbuf += strlen("vn ");
					CODESKIPSPACE
					CODEREADSTRINGVALUE

					sscanf_s(str,"%f %f %f", &ndata.nx, &ndata.ny, &ndata.nz);
					normal.emplace_back(ndata);
					//printf("\n%s\n%f %f %f", str, normal[normalCur].nx, normal[normalCur].ny, normal[normalCur].nz);  getchar();
					normalCur++;
				}
				else if (Compare(fpbuf, _SLEN("vt "))) // Координаты текстуры
				{
					fpbuf += strlen("vt ");
					have_w = 0;
					CODESKIPSPACE
					for(n=0;;n++,fpbuf++) // custom CODEREADSTRINGVALUE
					{
							 if (_EQUAL(*fpbuf,0x0A)) {         break;} //0A     \n
						else if (_EQUAL(*fpbuf,0x0D)) {fpbuf++; break;} //0D0A \r\n
						else if (_EQUAL(*fpbuf,0x20)) {have_w++;      } //' '
						str[n] = *fpbuf;
					}	str[n] = NULL;

					if (have_w==2) { sscanf_s(str,"%f %f %f", &tdata.u, &tdata.v, &tdata.w);       }
					else           { sscanf_s(str,"%f %f",    &tdata.u, &tdata.v); tdata.w = 0.0f; }
					texpoint.emplace_back(tdata);
					//printf("\n%s\n%f %f %f", str, texpoint[texCur].u, texpoint[texCur].v, texpoint[texCur].w);  getchar();				
					texCur++;
				}
				else if (_EQUAL(*fpbuf,'f')) // Описание поверхности
				{
					fpbuf++;
					CODESKIPSPACE
					for(iv=0; iv<maxVertexIndex; iv++)
					{
						int32 vp = -1, vn = -1, vt = -1;
						int32 format = 1;

						for(n=0;;n++,fpbuf++)
						{
							if (_EQUAL3(*fpbuf,' ',0x0A,0x0D)) break;
							if (format && _EQUAL(*fpbuf,'/') && _EQUAL(fpbuf[1],'/'))
								format=0;
							if (format && _EQUAL(*fpbuf,'/'))
								format++;
							str[n] = *fpbuf;		
						}   str[n] = NULL;

						//printf("\nformat %i str %s iv %i global %i",format,str,iv,nVertices);

						MODELVERTEX & mv = vertex[vertex.count + iv];
						switch(format)
						{
						case 3:
							sscanf_s(str,"%d/%d/%d",&vp,&vt,&vn);
							vp = (vp<0) ? (pointCur+vp)  : (vp-1);  mv.P._set(point[vp]);
							vn = (vn<0) ? (normalCur+vn) : (vn-1);  mv.N._set(normal[vn]);
							vt = (vt<0) ? (texCur+vt)    : (vt-1);  mv.TEX._set(texpoint[vt]);
							                                        mv.TEX2._set(texpoint[vt]);
																	mv.TEX3._set(texpoint[vt]);		break;
						case 0:
							sscanf_s(str,"%d//%d",&vp,&vn);
							vp = (vp<0) ? (pointCur+vp)  : (vp-1);  mv.P._set(point[vp]);
							vn = (vn<0) ? (normalCur+vn) : (vn-1);  mv.N._set(normal[vn]);			break;
						case 2:
							sscanf_s(str,"%d/%d",&vp,&vt);
							vp = (vp<0) ? (pointCur+vp)  : (vp-1);  mv.P._set(point[vp]);
							vt = (vt<0) ? (texCur+vt)    : (vt-1);  mv.TEX._set(texpoint[vt]);
							                                        mv.TEX2._set(texpoint[vt]);
																	mv.TEX3._set(texpoint[vt]);		break;
						case 1:
							sscanf_s(str,"%d",&vp);
							vp = (vp<0) ? (pointCur+vp)  : (vp-1);  mv.P._set(point[vp]);			break;
						default:
							_MBM(ERROR_ObjVertexFormat);
						}

					/*	printf("\nvp %i vt %i vn %i",vp,vt,vn);
						printf("\n%i + %i: %f %f %f (xyz)",nVertices,iv,vertex[nVertices + iv].x,vertex[nVertices + iv].y,vertex[nVertices + iv].z);
						printf("\n%i + %i: %f %f %f (normal)",nVertices,iv,vertex[nVertices + iv].nx,vertex[nVertices + iv].ny,vertex[nVertices + iv].nz);
						printf("\n%i + %i: %f %f (uv)",nVertices,iv,vertex[nVertices + iv].u,vertex[nVertices + iv].v);
						getchar();//*/

						CODESKIPSPACE
						if (_EQUAL2(*fpbuf, 0x0A, 0x0D)) break;
					}

					if (_EQUAL(iv,maxVertexIndex)) _MBM(ERROR_ObjVertexMax);

					// Генерация индексов исходя из количества вертексов поверхности
					
					uint32 numidx = (++iv-2)*3;		//printf("\niv %i numidx %i",iv,numidx); getchar();			
					if(cullmode_CCW)		
						for(n=0;n<numidx;n+=3)
						{							//fix на начало набора SUB через lastVertix
							indice[indice.count + n]     = vertex.count - lastVertex;
							indice[indice.count + n + 1] = vertex.count - lastVertex + (n+3)/3;
							indice[indice.count + n + 2] = vertex.count - lastVertex + 1+((n+3)/3);
						}
					else
						for(n=0;n<numidx;n+=3)
						{
							indice[indice.count + n]     = vertex.count - lastVertex;
							indice[indice.count + n + 2] = vertex.count - lastVertex + (n+3)/3;
							indice[indice.count + n + 1] = vertex.count - lastVertex + 1+((n+3)/3);
						}
					
				/*	if (1)//(nVertices - lastVertix < 12)
					{
						printf("\nGROUP %i SUB %i",groupCur,mtlCur);
						for(n=0;n<numidx;n+=3) printf("\n%i %i %i",indice[nIndices + n], indice[nIndices + n+1], indice[nIndices + n+2]);
						//getchar();
					} //*/

					vertex.count += iv;		// Итоговый счётчик вертексов
					indice.count += numidx;	// Итоговый счётчик индексов
				}
				else if (_EQUAL2(*fpbuf,'g','o')) // группа или объект
				{
					fpbuf++;
					groupCur++;
					MODELGROUP & gModelCur = *gmodel[groupCur];

					CODESKIPSPACE
					CODEREADSTRINGTEXT
					gModelCur.gname.assign(str);

					//printf("\ngroup %i: %s [%X]", groupCur, gModelCur.gname.c_str(), fpbuf - _fpbuf);  //getchar();
				}
				else if (Compare(fpbuf, _SLEN("usemtl"))) // название материала
				{
					fpbuf += strlen("usemtl");
					mtlCur++;
					MODELSUB & sModelCur = *smodel[mtlCur];

					CODESKIPSPACE
					CODEREADSTRINGWTEXT
					sModelCur.mtl.assign((wchar_t*)str);

					if (mtlCur) // запись данных предыдущего набора SUB
					{
						MODELSUB & sModelPrev = *smodel[mtlCur-1];
						sModelPrev.v_ptr = &vertex[lastVertex];
						sModelPrev.i_ptr = &indice[lastIndice];
						sModelPrev.v_id  = lastVertex;
						sModelPrev.i_id  = lastIndice;
						sModelPrev.nVert = vertex.count - lastVertex;
						sModelPrev.nInd  = indice.count - lastIndice;
						sModelPrev.nPrim = sModelPrev.nInd / 3;
					}				
					lastVertex = vertex.count;
					lastIndice = indice.count;

					gmodel[groupCur]->id.emplace_back(mtlCur);	// приобщение текущего набора SUB к текущей группе GROUP
					sModelCur.g_ID = groupCur;					// обратная ссылка на группу

					//wprintf(L"\nmodel %i: mtl %s [%X]", mtlCur, smodel[mtlCur].mtl.c_str(), fpbuf-_fpbuf);  //getchar();
				}
				else if (Compare(fpbuf, _SLEN("mtllib"))) // имя файла библиотеки материалов
				{
					fpbuf += strlen("mtllib");
					CODESKIPSPACE
					CODEREADSTRINGWTEXT
					mtllib.emplace_back((wchar_t*)str);
					mtllibCur++;

					/////////////////// Чтение mtl-файла материалов //////////////////////
					if (!ObjReadMtlLib(objPath, mtllib[mtllibCur-1].c_str()))
					{
						wchar_t error[256];
						wsprintf(error, L"%s %s", ERROR_ReadFile, mtllib[mtllibCur - 1].c_str());
						_MBM(error);
					}
					///////////////////////////////////////////////////////////////////////
				}
			}

			// запись данных последнего набора SUB
			MODELSUB & sModelCur = *smodel[mtlCur];
			sModelCur.v_ptr = &vertex[lastVertex];
			sModelCur.i_ptr = &indice[lastIndice];
			sModelCur.v_id  = lastVertex;
			sModelCur.i_id  = lastIndice;
			sModelCur.nVert = vertex.count - lastVertex;
			sModelCur.nInd  = indice.count - lastIndice;
			sModelCur.nPrim = sModelCur.nInd / 3;		

			// простановка порядковых ID материала у наборов поверхностей MODELSUB
			uint32 mtlnum = (uint32)material.size();
			for(uint32 i=0; i<nSModels; i++)
				for(n=0; n<mtlnum; n++)
					if (!smodel[i]->mtl.compare(material[n]->name))
						{smodel[i]->mtl_ID = n; break;}

			ObjReadTextures(gamePath,objName);
			ObjReadSortMTL();
			PrepareNormal();
			PrepareTangentAndBinormal();
			CalculateBox();

		/*	for(n=0; n<mtlnum; n++)
			{
				if (material[n].id_map_Kd != -1)
				wprintf(L"\n%s -> ID %3i = %s", material[n].name.c_str(), material[n].id_map_Kd, 
					texture[material[n].id_map_Kd].name.c_str());
				else
				wprintf(L"\n%s -> ID %3i = %s", material[n].name.c_str(), material[n].id_map_Kd, 
					material[n].map_Kd.c_str());
			} //*/

			wprintf(L"\nEND read model %s: %i ms (%i verts, %i inds)", objName, clock()-time, vertex.count, indice.count);
			return isInit = true;
		}

		//>> Подсчёт количества точек и проч. в obj-файле
		void ObjCount(byte* fpbuf, uint32 size, uint32& pointCount, uint32& normalCount, uint32& texCount,
			uint32& faceCount, uint32& groupCount, uint32& mtlCount, uint32& mtllibCount, uint32& maxVertexIndex)
		{
			pointCount=0;											// www.martinreddy.net/gfx/3d/OBJ.spec
			normalCount=0;
			texCount=0;
			faceCount=0;
			groupCount=0;
			mtlCount=0;
			mtllibCount=0;
			maxVertexIndex=0;

			byte* fpstart = fpbuf;
			byte* fpend   = fpbuf + size;
			for( ; fpbuf!=fpend; fpbuf++)
			{
				CODESKIPCOMMENTS
				else if (_EQUAL(*fpbuf,'f'))						// определение поверхности  f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 v4/vt4/vn4 ...
				{													// f 1 2 3  ||  f 3/1 4/2 5/3  ||  f 6/4/1 3/5/3 7/6/5  ||  f 6//1 3//3 7//5
					fpbuf++; faceCount++; uint32 maxV=0;		
					CODESKIPSPACE
					for(;;)
					{
						for(;;fpbuf++) if (_EQUAL3(*fpbuf,' ',0x0A,0x0D)) break;
						CODESKIPSPACE
						maxV++;
						if (_EQUAL2(*fpbuf, 0x0A, 0x0D)) break;
					}

					if (maxVertexIndex < maxV)
						maxVertexIndex = maxV;
				}
				else if (_EQUAL(*fpbuf,'g'))						// группа
				{													// g Group1
					groupCount++;
					CODESKIPSTRING
				}
				else if (_EQUAL(*fpbuf,'o'))						// объект
				{													// o Object1
					groupCount++;
					CODESKIPSTRING
				}
				else if (_EQUAL(*fpbuf,'s'))						// smoothing group
				{													// s off  ||  s 2
					CODESKIPSTRING
				}
				else if (Compare(fpbuf,"v ",2))						// список вершин (x,y,z[,w]), w является не обязательным и по умолчанию 1.0f
				{													// v 0.123 0.234 0.345 1.0
					pointCount++;
					CODESKIPSTRING
				}
				else if (Compare(fpbuf,"vn ",3))					// нормали (x,y,z); нормали могут быть не нормированными.
				{													// vn 0.707 0.000 0.707
					normalCount++;
					CODESKIPSTRING
				}													// текстурные координаты (u,v[,w]), w является не обязательным и по умолчанию 0			
				else if (Compare(fpbuf,"vt ",3))					// текстурная координата по y может быть указана как 1 - v, и при этом по x = u							
				{													// vt 0.500 -1.352 0.234
					texCount++;
					CODESKIPSTRING
				}
			//	else if (Compare(fpbuf,"vp ",3))					// параметры вершин в пространстве (u [,v] [,w]); свободная форма геометрического состояния
			//	{													// vp 0.310000 3.210000 2.100000
			//		for(;;i++,fpbuf++) if (!(*fpbuf^0x0A)) break;
			//	}
				else if (Compare(fpbuf,"usemtl ",7))				// имя материала в библиотеке для последующих f
				{													// usemtl body_paint
					mtlCount++;
					CODESKIPSTRING
				}
				else if (Compare(fpbuf,"mtllib ",7))				// сопроводительный файл, библиотека материалов
				{
				/*	newmtl название_материала1   # Объявление очередного материала
					# Цвета
					Ka 1,000 1,000 0,000         # Цвет окружающего освещения (желтый)
					Kd 1,000 1,000 1,000         # Диффузный цвет (белый)
					# Параметры отражения
					Ks 0,000 0,000 0,000         # Цвет зеркального отражения (0;0;0 - выключен)
					Ns 10,000                    # Коэффициент зеркального отражения (от 0 до 1000)
					# Параметры прозрачности
					d 0,9                        # Прозрачность указывается с помощью директивы d
					Tr 0,9                       #   или в других реализациях формата с помощью Tr
					#Следующий материал
					newmtl название_материала2 */

					mtllibCount++;
					CODESKIPSTRING
				}
				else
				{
					wchar_t error[256];
					wsprintf(error, L"%s [OFFSET %X]", ERROR_ReadFile, fpbuf - fpstart);
					_MBM(error);
				}
			}
			if (maxVertexIndex < 3) _MBM(ERROR_ObjVertexNum);
			//printf("\nmaxVertexIndex %i",maxVertexIndex);	getchar();	
		};

		//>> Чтение сопроводительного файла *.mtl с описанием материалов (должен лежать рядом с *.obj)
		bool ObjReadMtlLib(const wchar_t* mtlPath, const wchar_t* mtlName)
		{
			wchar_t mtlFile[256];
			wsprintf(mtlFile,L"%s\\%s",mtlPath,mtlName);

			TBUFFER <byte, int32> file;								//wprintf(L"\nReading file %s", mtlFile);
			ReadFileInMemory(mtlFile, file, 1);						if (file.buf == nullptr) return false;
			byte* fpbuf = file.buf;
			byte* fpend = fpbuf + file.count;

			crc32.ProcessCRC(file.buf, file.count);

			uint32 n;
			wstring wstr;
			char str[256];
			float flt[3];

			MODELMTL * mtl = nullptr;
			
			for( ; fpbuf!=fpend ; fpbuf++)
			{
				CODESKIPCOMMENTS
				else if (_EQUAL(*fpbuf,'d'))	// по умолчанию  d = 1.0f, Tr = false
				{
					fpbuf += 2;
					CODEREADSTRINGVALUE

					sscanf_s(str,"%f", &mtl->d_Tr);
					if (mtl->d_Tr < 1)
						mtl->Tr = true; // метка, что материал прозрачен
				}
				else if (Compare(fpbuf, _SLEN("Ka spectral ")))	
				{
					_MBM(L"OBJ material: 'Ka spectral' currently not supported");
					CODESKIPSTRING
				}
				else if (Compare(fpbuf, _SLEN("Ka xyz ")))	
				{
					_MBM(L"OBJ material: 'Ka xyz' currently not supported");
					CODESKIPSTRING
				}
				else if (Compare(fpbuf, _SLEN("Ka ")))										// Ka r g b
				{																			// Ka spectral file.rfl factor
					fpbuf += strlen("Ka ");													// Ka xyz x y z
					CODEREADSTRINGVALUE

					sscanf_s(str,"%f %f %f", &flt[0], &flt[1], &flt[2]);
					mtl->_Ka[0] = flt[0];
					mtl->_Ka[1] = flt[1];
					mtl->_Ka[2] = flt[2];
					mtl->_Ka[3] = 0xFF;
					mtl->Ka = 0xFF000000;				// A  0xAARRGGBB
					mtl->Ka |= COLORBYTE(flt[0]) << 16;	// R
					mtl->Ka |= COLORBYTE(flt[1]) << 8;	// G
					mtl->Ka |= COLORBYTE(flt[2]);		// B
					//printf("\n%s\n%f %f %f\n%X",str,flt[0],flt[1],flt[2],mtl.Ka);	//getchar();
				}
				else if (Compare(fpbuf, _SLEN("Kd spectral ")))	
				{
					_MBM(L"OBJ material: 'Kd spectral' currently not supported");
					CODESKIPSTRING
				}
				else if (Compare(fpbuf, _SLEN("Kd xyz ")))	
				{
					_MBM(L"OBJ material: 'Kd xyz' currently not supported");
					CODESKIPSTRING
				}
				else if (Compare(fpbuf, _SLEN("Kd ")))
				{
					fpbuf += strlen("Kd ");
					CODEREADSTRINGVALUE

					sscanf_s(str,"%f %f %f", &flt[0], &flt[1], &flt[2]);
					mtl->_Kd[0] = flt[0];
					mtl->_Kd[1] = flt[1];
					mtl->_Kd[2] = flt[2];
					mtl->_Kd[3] = 0xFF;
					mtl->Kd = 0xFF000000;				// A  0xAARRGGBB
					mtl->Kd |= COLORBYTE(flt[0]) << 16;	// R
					mtl->Kd |= COLORBYTE(flt[1]) << 8;	// G
					mtl->Kd |= COLORBYTE(flt[2]);		// B
					//printf("\n%s\n%f %f %f\n%X",str,flt[0],flt[1],flt[2],mtl.Kd);	//getchar();
				}
				else if (Compare(fpbuf, _SLEN("Ks spectral ")))	
				{
					_MBM(L"OBJ material: 'Ks spectral' currently not supported");
					CODESKIPSTRING
				}
				else if (Compare(fpbuf, _SLEN("Ks xyz ")))	
				{
					_MBM(L"OBJ material: 'Ks xyz' currently not supported");
					CODESKIPSTRING
				}
				else if (Compare(fpbuf, _SLEN("Ks ")))
				{
					fpbuf += strlen("Ks ");
					CODEREADSTRINGVALUE

					sscanf_s(str,"%f %f %f", &flt[0], &flt[1], &flt[2]);
					mtl->_Ks[0] = flt[0];
					mtl->_Ks[1] = flt[1];
					mtl->_Ks[2] = flt[2];
					mtl->_Ks[3] = 0xFF;
					mtl->Ks = 0xFF000000;				// A  0xAARRGGBB
					mtl->Ks |= COLORBYTE(flt[0]) << 16;	// R
					mtl->Ks |= COLORBYTE(flt[1]) << 8;	// G
					mtl->Ks |= COLORBYTE(flt[2]);		// B
					//printf("\n%s\n%f %f %f\n%X",str,flt[0],flt[1],flt[2],mtl.Ks);	//getchar();
				}
				else if (Compare(fpbuf, _SLEN("Ns ")))
				{
					fpbuf += strlen("Ns ");
					CODEREADSTRINGVALUE
					sscanf_s(str,"%f", &mtl->Ns);			//printf("\n%s\n%f",str,mtl.Ns);	//getchar();
				}
				else if (Compare(fpbuf, _SLEN("Ni ")))
				{
					fpbuf += strlen("Ni ");
					CODEREADSTRINGVALUE
					sscanf_s(str,"%f", &mtl->Ni);			//printf("\n%s\n%f",str,mtl.Ni);	//getchar();
				}
				else if (Compare(fpbuf, _SLEN("illum ")))
				{
					fpbuf += strlen("illum ");
					CODEREADSTRINGVALUE
					sscanf_s(str,"%d", &mtl->illum);		//printf("\n%s\n%d",str,mtl.illum);	//getchar();
				}
				else if (Compare(fpbuf, _SLEN("bump")))
				{
					fpbuf += strlen("bump");
					CODESKIPSPACE
					CODEREADSTRINGWTEXT
					mtl->bump.assign((wchar_t*)str);		//if (mtl.bump.size() > 1) wprintf(L"\nbump: %s",mtl.bump.c_str());
				}
				else if (Compare(fpbuf, _SLEN("refl ")))
				{
					fpbuf += strlen("refl ");
					CODEREADSTRINGVALUE
					if (n) _MBM(L"OBJ material: 'refl' currently not supported");
				}
				else if (Compare(fpbuf, _SLEN("map_kA ")))
				{
					fpbuf += strlen("map_kA ");
					CODEREADSTRINGVALUE
					if (n) _MBM(L"OBJ material: 'map_kA' currently not supported");
				}
				else if (Compare(fpbuf, _SLEN("map_Kd")))
				{
					fpbuf += strlen("map_Kd");
					CODESKIPSPACE
					CODEREADSTRINGWTEXT
					mtl->map_Kd.assign((wchar_t*)str);		//if (mtl.map_Kd.size() > 1) wprintf(L"\nmap_Kd: %s",mtl.map_Kd.c_str());
				}
				else if (Compare(fpbuf, _SLEN("map_kS ")))
				{
					fpbuf += strlen("map_kS ");
					CODEREADSTRINGVALUE
					if (n) _MBM(L"OBJ material: 'map_kS' currently not supported");
				}
				else if (Compare(fpbuf, _SLEN("map_Ns ")))
				{
					fpbuf += strlen("map_Ns ");
					CODEREADSTRINGVALUE
					if (n) _MBM(L"OBJ material: 'map_Ns' currently not supported");
				}
				else if (Compare(fpbuf, _SLEN("map_d")))
				{
					fpbuf += strlen("map_d");
					CODESKIPSPACE
					CODEREADSTRINGWTEXT
					mtl->map_d.assign((wchar_t*)str);		//if (mtl.map_d.size() > 1) wprintf(L"\nmap_d : %s",mtl.map_d.c_str());
				}
				else if (Compare(fpbuf, _SLEN("map_bump")))
				{
					fpbuf += strlen("map_bump");
					CODESKIPSPACE
					CODEREADSTRINGWTEXT
					mtl->map_bump.assign((wchar_t*)str);	//if (mtl.map_bump.size() > 1) wprintf(L"\nmap_bump: %s",mtl.map_bump.c_str());
				}
				else if (Compare(fpbuf, _SLEN("map_opacity")))
				{
					fpbuf += strlen("map_opacity");
					CODESKIPSPACE
					CODEREADSTRINGWTEXT
					mtl->map_opacity.assign((wchar_t*)str);	//if (mtl.map_opacity.size() > 1) wprintf(L"\nmap_op: %s",mtl.map_opacity.c_str());
				}
				else if (Compare(fpbuf, _SLEN("newmtl")))
				{
					material.emplace_back_create();		// Добавляем новый материал
					mtl = material.last();				//

					fpbuf += strlen("newmtl");
					CODESKIPSPACE
					CODEREADSTRINGWTEXT
					mtl->name.assign((wchar_t*)str);	//wprintf(L"\n%s", mtl->name.c_str());
				}
				else
				{
					wchar_t error[256];
					wsprintf(error, L"%s [%s OFFSET %X]", ERROR_ReadFile, mtlName, fpbuf - file.buf);
					_MBM(error);
				}
			}

			return true;
		}

		//>> Чтение текстур в оперативную память и связывание с материалом по id
		void ObjReadTextures(const wchar_t* gamePath, const wchar_t* objName)
		{
			uint32 i, n, x, mtlnum = (uint32)material.size();
			 int16 curTexNum = 0;
			wchar_t objNameD[256]; // имя папки с текстурами в DIRECTORY_TEXTURESMDL соотв. имени *.obj без расширения
	
			memset(objNameD,0,256*sizeof(wchar_t));
			n = (uint32)wcslen(objName);
			for(i=0,x=0;i<n;i++,x++)
				if(objName[i] == 0x002e)
					x=0;
			if (x==n) wcsncat_s(objNameD,objName,n);
			else      wcsncat_s(objNameD,objName,n-x);
			//wprintf(L"\nobjNameD %s",objNameD);
			
			for(i=0; i<mtlnum; i++)
			{
				MODELMTL & mtrl = *material[i];

				if (mtrl.refl.size()     > 1) {}; // пока не поддерживаю
				if (mtrl.map_Ka.size()   > 1) {}; // пока не поддерживаю
				if (mtrl.map_Ks.size()   > 1) {}; // пока не поддерживаю		specular environment map ?
				if (mtrl.map_Ns.size()   > 1) {}; // пока не поддерживаю		specular environment map ?

				if (mtrl.map_Kd.size() > 1)		// Основная текстура модели
				{
					ObjReadTexturesSeg( gamePath, objNameD, mtrl.map_Kd, mtrl.id_map_Kd, curTexNum );
					//wprintf(L"\nmap_Kd %3i: (%8X) %i=%s", i, texture[material[i].id_map_Kd].bufsz, material[i].id_map_Kd,
					//	texture[material[i].id_map_Kd].name.c_str() ); //getchar();
				};
				if (mtrl.map_d.size() > 1)		// Тоже, что и выше /// текстура c прозрачностью, обычно совпадает с основной map_Kd (альфа-канал)
				{
					ObjReadTexturesSeg( gamePath, objNameD, mtrl.map_d, mtrl.id_map_d, curTexNum );
					//wprintf(L"\nmap_d  %3i: (%8X) %i=%s", i, texture[material[i].id_map_d].bufsz, material[i].id_map_d,
					//	texture[material[i].id_map_d].name.c_str() ); //getchar();
				}
				if (mtrl.map_opacity.size() > 1)	// Тоже, что и выше
				{
					ObjReadTexturesSeg( gamePath, objNameD, mtrl.map_opacity, mtrl.id_map_opacity, curTexNum );
					//wprintf(L"\nmap_op %3i: (%8X) %i=%s", i, texture[material[i].id_map_opacity].bufsz, material[i].id_map_opacity,
					//	texture[material[i].id_map_opacity].name.c_str() ); //getchar();
				}
				if (mtrl.map_bump.size() > 1)	// Bump Mapping
				{
					ObjReadTexturesSeg( gamePath, objNameD, mtrl.map_bump, mtrl.id_map_bump, curTexNum );
				};
				if (mtrl.bump.size() > 1)		// Тоже, что и выше
				{
					ObjReadTexturesSeg( gamePath, objNameD, mtrl.bump, mtrl.id_bump, curTexNum );
				}; 
			}
		}

		//>> Чтение текстур в оперативную память (сегмент функции выше)
		void ObjReadTexturesSeg(const wchar_t* gamePath, const wchar_t* objNameD, const wstring& s_mtl, int16& id_map, int16& curTexNum)
		{
			wchar_t texFile[256];
			bool skip_segment = 0;
			int16 n;

			for(n=0; n<curTexNum; n++)
			if(!s_mtl.compare(texture[n]->name))	// если уже загружали такую текстуру
			{
				id_map = n;
				skip_segment = 1;
				break;
			}

			if (!skip_segment)
			{
				wsprintf(texFile,L"%s\\%s\\%s\\%s", gamePath, DIRECTORY_TEXTURESMDL, objNameD, s_mtl.c_str());
				//wprintf(L"\ntexFile %s",texFile);
				id_map = curTexNum++;
				texture.emplace_back_create();
				MODELTEX * tex = texture.last();
				tex->name = s_mtl;
				ReadFileInMemory(texFile, tex->file, 1);	//wprintf(L"\n :::: %i=%s",id_map,texture[id_map].name.c_str())
				if (tex->file.buf == nullptr)
					tex->file.count = MISSING;
			}
		}

		//>> Сортировка наборов вершин по материалу
		void ObjReadSortMTL()
		{
			wchar_t error[256];
			uint32 i, n, cur = -1;
			uint32 numMtl = (uint32)material.size();
			uint32 nSModels = (uint32)smodel.size();

			mtlsort.reserve(numMtl + 1); // '+1' для наборов без материала

			mtlsort_tr   = MISSING;
			mtlsort_none = MISSING;

			for(i=0; i<numMtl; i++)   if (material[i]->Tr)         {++mtlsort_tr;   break;} // проверка наличия наборов с прозрачным материалом
			for(n=0; n<nSModels; n++) if (smodel[n]->mtl_ID == -1) {++mtlsort_none; break;} // проверка наличия наборов без материала

			for(i=0; i<numMtl; i++) if (!material[i]->Tr)		{ // ставим непрозрачные
				mtlsort.emplace_back_create();
				mtlsort.last()->id_mtl = i;						}

			if (_NOMISS(mtlsort_tr))
			for (i=0; i<numMtl; i++) if (material[i]->Tr)		{ // ставим прозрачные
				mtlsort.emplace_back_create();
				mtlsort.last()->id_mtl = i;						}

			if (_NOMISS(mtlsort_none))							{ // ставим без материала
				mtlsort.emplace_back_create();
				mtlsort.last()->id_mtl = MISSING;				}

			// сначала заполним непрозрачные
			for(i=0; i<numMtl; i++) if (!material[i]->Tr)				{	cur++;
			for(n=0; n<nSModels; n++) if (smodel[n]->mtl_ID == i)
				mtlsort[cur]->smodel.emplace_back(n);					}

			// теперь заполним прозрачные
			if (_NOMISS(mtlsort_tr))									{	mtlsort_tr = cur + 1;
			for(i=0; i<numMtl; i++) if (material[i]->Tr)			{		cur++;
			for(n=0; n<nSModels; n++) if (smodel[n]->mtl_ID == i)
				mtlsort[cur]->smodel.emplace_back(n);				}	}

			// теперь заполним без материалов
			if (_NOMISS(mtlsort_none))										{	mtlsort_none = cur + 1; // == mtlsort.size()-1
			for(n=0; n<nSModels; n++) if (_ISMISS(smodel[n]->mtl_ID))	{		cur++;
			{
				mtlsort[cur]->smodel.emplace_back(n);
				wsprintf(error,L"%s\nsubmodel %i\n%s", ERROR_MdlNoMaterial, n, name.c_str());
				_MBM(error);
			}															}	}
		}

		//>> Рассчёт тангенсальной и бинормальной (битангенсальной) составляющих
		void PrepareTangentAndBinormal()
		{
			//  normal [Z]
			//  ^    
			//  |   * binormal / bitangent [Y]      ____________  x,y
			//  |  /                               /           /
			//  | /                               /   face    /
			//  |-------> tangent [X]            /___________/

			for(uint32 i=0; i<indice.count; i+=3)
			{
				MODELVERTEX & v0 = vertex[indice[i]];
				MODELVERTEX & v1 = vertex[indice[i+1]];
				MODELVERTEX & v2 = vertex[indice[i+2]];

				MATH3DVEC edge1(v1.P - v0.P);
				MATH3DVEC edge2(v2.P - v0.P);

				FLOAT deltaU1 = v1.TEX.u - v0.TEX.u; // edge1 = (u1-u0)T + (v1-v0)B		| E1x E1y E1z |   | deltaU1 deltaV1 | * | Tx Ty Tz |
				FLOAT deltaV1 = v1.TEX.v - v0.TEX.v; // edge2 = (u2-u0)T + (v2-v0)B		| E2x E2y E2z | = | deltaU2 deltaV2 |   | Bx By Bz |
				FLOAT deltaU2 = v2.TEX.u - v0.TEX.u; // https://gamedev.stackexchange.com/questions/68612/how-to-compute-tangent-and-bitangent-vectors
				FLOAT deltaV2 = v2.TEX.v - v0.TEX.v; //

				FLOAT f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

				MATH3DVEC tangent, bitangent;

				tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
				tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
				tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

				bitangent.x = f * (-deltaU2 * edge1.x + deltaU1 * edge2.x);
				bitangent.y = f * (-deltaU2 * edge1.y + deltaU1 * edge2.y);
				bitangent.z = f * (-deltaU2 * edge1.z + deltaU1 * edge2.z);

			//	tangent   = MathNormalizeVec(tangent - normal * MathDotVec(tangent, normal));
			//	bitangent = MathCrossVec(normal, tangent);
			//	if (f < 0) bitangent = bitangent * -1;
																					// A common way to orthogonalize any matrix is the Gram–Schmidt process:
			//	tangent   = tangent   - normal  * MathDotVec( tangent,   normal );  // orthonormalization ot the tangent vectors
			//	bitangent = bitangent - normal  * MathDotVec( bitangent, normal );  // orthonormalization of the binormal vectors to the normal vector 
			//	bitangent = bitangent - tangent * MathDotVec( bitangent, tangent ); // orthonormalization of the binormal vectors to the tangent vector		

				tangent._normalize();
				bitangent._normalize();

			/*	// Calculate the handedness of the local tangent space.
				// The bitangent vector is the cross product between the triangle face
				// normal vector and the calculated tangent vector. The resulting bitangent
    			// vector should be the same as the bitangent vector calculated from the
    			// set of linear equations above. If they point in different directions
    			// then we need to invert the cross product calculated bitangent vector. We
    			// store this scalar multiplier in the tangent vector's 'w' component so
    			// that the correct bitangent vector can be generated in the normal mapping
    			// shader's vertex shader.

    			D3DXVECTOR3 n(normal[0], normal[1], normal[2]);
    			D3DXVECTOR3 t(tangent.x, tangent.y, tangent.z);
    			D3DXVECTOR3 b;
        
    			D3DXVec3Cross(&b, &n, &t);
    			tangent.w = (D3DXVec3Dot(&b, &bitangent) < 0.0f) ? -1.0f : 1.0f; //*/

				v0.T += tangent;   v0.B += bitangent;
				v1.T += tangent;   v1.B += bitangent;
				v2.T += tangent;   v2.B += bitangent;
			}

			for(uint32 i=0; i<vertex.count; i++)
			{
				vertex[i].T._normalize();
				vertex[i].B._normalize();
			}
		};

		//>> Рассчёт нормалей (если не заданы)
		void PrepareNormal()
		{
			for(uint32 i=0; i<indice.count; i+=3)
			{
				MODELVERTEX & v0 = vertex[indice[i]];
				MODELVERTEX & v1 = vertex[indice[i+1]];
				MODELVERTEX & v2 = vertex[indice[i+2]];

			//	if ( (!v0.N.nx && !v0.N.ny && !v0.N.nz) ||
			//		 (!v1.N.nx && !v1.N.ny && !v1.N.nz) ||
			//		 (!v2.N.nx && !v2.N.ny && !v2.N.nz) )

				if ( !v0.N || !v1.N || !v2.N )  // Если у хотя бы одной из вершин ещё не известно		
				{
					MATH3DVEC edge1(v1.P - v0.P);
					MATH3DVEC edge2(v2.P - v0.P);

				//	if ( (!edge1.x && !edge1.y && !edge1.z) ||
				//		 (!edge2.x && !edge2.y && !edge2.z) )

					if ( !edge1 || !edge2 ) // v1 == v0 или v2 == v0
					{
						continue; // TODO: рассчитывать усреднённые нормали для таких рёбер
					}

					MATH3DVEC normal = MathCrossVec(edge1, edge2);

					normal._normalize_check();

					v0.N += normal;
					v1.N += normal;
					v2.N += normal;
				}
			}

			for(uint32 i=0; i<vertex.count; i++)
			{
				vertex[i].N._normalize();
			}
		};

		//>> Представление модели условным ящиком (Bounding Box)
		void CalculateBox()
		{
			float X_max=vertex[0].P.x, Y_max=vertex[0].P.y, Z_max=vertex[0].P.z;
			float X_min=vertex[0].P.x, Y_min=vertex[0].P.y, Z_min=vertex[0].P.z;

			for(uint32 i=0; i<vertex.count; i++)
			{
				MODELVERTEX & v = vertex[i];

					 if (v.P.x > X_max) X_max = v.P.x;
				else if (v.P.x < X_min) X_min = v.P.x;

					 if (v.P.y > Y_max) Y_max = v.P.y;
				else if (v.P.y < Y_min) Y_min = v.P.y;

					 if (v.P.z > Z_max) Z_max = v.P.z;
				else if (v.P.z < Z_min) Z_min = v.P.z;
			}

			box.maxCoord  = MATH3DVEC(X_max, Y_max, Z_max);			//printf("\n%f %f %f", box.maxCoord.x, box.maxCoord.y, box.maxCoord.z);
			box.minCoord  = MATH3DVEC(X_min, Y_min, Z_min);			//printf("\n%f %f %f", box.minCoord.x, box.minCoord.y, box.minCoord.z);
			box.midRadius = (box.maxCoord - box.minCoord) * 0.5f;	//printf("\n%f %f %f", box.midRadius.x, box.midRadius.y, box.midRadius.z);
			box.midCoord  = box.minCoord + box.midRadius;			//printf("\n%f %f %f", box.midCoord.x, box.midCoord.y, box.midCoord.z);
			box.midR_avr  = (box.midRadius.x + box.midRadius.y + box.midRadius.z) / 3; //printf("\nbox.midR_avr %f", box.midR_avr);
			box.midR_max  = max(box.midRadius.x, box.midRadius.y);
			box.midR_max  = max(box.midR_max,    box.midRadius.z);	//printf("\nbox.midR_max %f", box.midR_max);
			box.midR_min  = min(box.midRadius.x, box.midRadius.y);
			box.midR_min  = min(box.midR_min,    box.midRadius.z);  //printf("\nbox.midR_min %f", box.midR_min);

			box.point[0] = MATH3DVEC(box.midCoord.x + box.midRadius.x, box.midCoord.y + box.midRadius.y, box.midCoord.z + box.midRadius.z); // + + +
			box.point[1] = MATH3DVEC(box.midCoord.x - box.midRadius.x, box.midCoord.y - box.midRadius.y, box.midCoord.z - box.midRadius.z); // - - -

			box.point[2] = MATH3DVEC(box.midCoord.x + box.midRadius.x, box.midCoord.y + box.midRadius.y, box.midCoord.z - box.midRadius.z); // + + -
			box.point[3] = MATH3DVEC(box.midCoord.x + box.midRadius.x, box.midCoord.y - box.midRadius.y, box.midCoord.z + box.midRadius.z); // + - +
			box.point[4] = MATH3DVEC(box.midCoord.x - box.midRadius.x, box.midCoord.y + box.midRadius.y, box.midCoord.z + box.midRadius.z); // - + +

			box.point[5] = MATH3DVEC(box.midCoord.x + box.midRadius.x, box.midCoord.y - box.midRadius.y, box.midCoord.z - box.midRadius.z); // + - -
			box.point[6] = MATH3DVEC(box.midCoord.x - box.midRadius.x, box.midCoord.y + box.midRadius.y, box.midCoord.z - box.midRadius.z); // - + -
			box.point[7] = MATH3DVEC(box.midCoord.x - box.midRadius.x, box.midCoord.y - box.midRadius.y, box.midCoord.z + box.midRadius.z); // - - +

			// CCW style

			box.face_Xp[0] = box.point[2];	//printf("\n%f %f %f", box.face_Xp[0].x, box.face_Xp[0].y, box.face_Xp[0].z);
			box.face_Xp[1] = box.point[0];	//printf("\n%f %f %f", box.face_Xp[1].x, box.face_Xp[1].y, box.face_Xp[1].z);
			box.face_Xp[2] = box.point[3];	//printf("\n%f %f %f", box.face_Xp[2].x, box.face_Xp[2].y, box.face_Xp[2].z);
			box.face_Xp[3] = box.point[5];	//printf("\n%f %f %f", box.face_Xp[3].x, box.face_Xp[3].y, box.face_Xp[3].z);

			box.face_Xn[0] = box.point[1];	//printf("\n%f %f %f", box.face_Xn[0].x, box.face_Xn[0].y, box.face_Xn[0].z);
			box.face_Xn[1] = box.point[7];	//printf("\n%f %f %f", box.face_Xn[1].x, box.face_Xn[1].y, box.face_Xn[1].z);
			box.face_Xn[2] = box.point[4];	//printf("\n%f %f %f", box.face_Xn[2].x, box.face_Xn[2].y, box.face_Xn[2].z);
			box.face_Xn[3] = box.point[6];	//printf("\n%f %f %f", box.face_Xn[3].x, box.face_Xn[3].y, box.face_Xn[3].z);

			box.face_Yp[0] = box.point[2];	//printf("\n%f %f %f", box.face_Yp[0].x, box.face_Yp[0].y, box.face_Yp[0].z);
			box.face_Yp[1] = box.point[6];	//printf("\n%f %f %f", box.face_Yp[1].x, box.face_Yp[1].y, box.face_Yp[1].z);
			box.face_Yp[2] = box.point[4];	//printf("\n%f %f %f", box.face_Yp[2].x, box.face_Yp[2].y, box.face_Yp[2].z);
			box.face_Yp[3] = box.point[0];	//printf("\n%f %f %f", box.face_Yp[3].x, box.face_Yp[3].y, box.face_Yp[3].z);

			box.face_Yn[0] = box.point[3];	//printf("\n%f %f %f", box.face_Yn[0].x, box.face_Yn[0].y, box.face_Yn[0].z);
			box.face_Yn[1] = box.point[7];	//printf("\n%f %f %f", box.face_Yn[1].x, box.face_Yn[1].y, box.face_Yn[1].z);
			box.face_Yn[2] = box.point[1];	//printf("\n%f %f %f", box.face_Yn[2].x, box.face_Yn[2].y, box.face_Yn[2].z);
			box.face_Yn[3] = box.point[5];	//printf("\n%f %f %f", box.face_Yn[3].x, box.face_Yn[3].y, box.face_Yn[3].z);

			box.face_Zp[0] = box.point[0];	//printf("\n%f %f %f", box.face_Zp[0].x, box.face_Zp[0].y, box.face_Zp[0].z);
			box.face_Zp[1] = box.point[4];	//printf("\n%f %f %f", box.face_Zp[1].x, box.face_Zp[1].y, box.face_Zp[1].z);
			box.face_Zp[2] = box.point[7];	//printf("\n%f %f %f", box.face_Zp[2].x, box.face_Zp[2].y, box.face_Zp[2].z);
			box.face_Zp[3] = box.point[3];	//printf("\n%f %f %f", box.face_Zp[3].x, box.face_Zp[3].y, box.face_Zp[3].z);

			box.face_Zn[0] = box.point[5];	//printf("\n%f %f %f", box.face_Zn[0].x, box.face_Zn[0].y, box.face_Zn[0].z);
			box.face_Zn[1] = box.point[1];	//printf("\n%f %f %f", box.face_Zn[1].x, box.face_Zn[1].y, box.face_Zn[1].z);
			box.face_Zn[2] = box.point[6];	//printf("\n%f %f %f", box.face_Zn[2].x, box.face_Zn[2].y, box.face_Zn[2].z);
			box.face_Zn[3] = box.point[2];	//printf("\n%f %f %f", box.face_Zn[3].x, box.face_Zn[3].y, box.face_Zn[3].z);

			//box.normal_Xp = MathCrossVec((box.face_Xp[1]-box.face_Xp[0]), (box.face_Xp[2]-box.face_Xp[0]));  MathNormalizeVecVoid(box.normal_Xp);
			//box.normal_Xn = MathCrossVec((box.face_Xn[1]-box.face_Xn[0]), (box.face_Xn[2]-box.face_Xn[0]));  MathNormalizeVecVoid(box.normal_Xn);
			//box.normal_Yp = MathCrossVec((box.face_Yp[1]-box.face_Yp[0]), (box.face_Yp[2]-box.face_Yp[0]));  MathNormalizeVecVoid(box.normal_Yp);
			//box.normal_Yn = MathCrossVec((box.face_Yn[1]-box.face_Yn[0]), (box.face_Yn[2]-box.face_Yn[0]));  MathNormalizeVecVoid(box.normal_Yn);
			//box.normal_Zp = MathCrossVec((box.face_Zp[1]-box.face_Zp[0]), (box.face_Zp[2]-box.face_Zp[0]));  MathNormalizeVecVoid(box.normal_Zp);
			//box.normal_Zn = MathCrossVec((box.face_Zn[1]-box.face_Zn[0]), (box.face_Zn[2]-box.face_Zn[0]));  MathNormalizeVecVoid(box.normal_Zn);

			/*printf("\nbox.normal_Xp %f %f %f", box.normal_Xp.x, box.normal_Xp.y, box.normal_Xp.z); //*/	box.normal_Xp = MATH3DVEC(1,0,0);
			/*printf("\nbox.normal_Xn %f %f %f", box.normal_Xn.x, box.normal_Xn.y, box.normal_Xn.z); //*/	box.normal_Xn = MATH3DVEC(-1,0,0);
			/*printf("\nbox.normal_Yp %f %f %f", box.normal_Yp.x, box.normal_Yp.y, box.normal_Yp.z); //*/	box.normal_Yp = MATH3DVEC(0,1,0);
			/*printf("\nbox.normal_Yn %f %f %f", box.normal_Yn.x, box.normal_Yn.y, box.normal_Yn.z); //*/	box.normal_Yn = MATH3DVEC(0,-1,0);
			/*printf("\nbox.normal_Zp %f %f %f", box.normal_Zp.x, box.normal_Zp.y, box.normal_Zp.z); //*/	box.normal_Zp = MATH3DVEC(0,0,1);
			/*printf("\nbox.normal_Zn %f %f %f", box.normal_Zn.x, box.normal_Zn.y, box.normal_Zn.z); //*/	box.normal_Zn = MATH3DVEC(0,0,-1);

			box.plane_Xp = MATH3DPLANE(box.face_Xp[0], box.normal_Xp);
			box.plane_Xn = MATH3DPLANE(box.face_Xn[0], box.normal_Xn);
			box.plane_Yp = MATH3DPLANE(box.face_Yp[0], box.normal_Yp);
			box.plane_Yn = MATH3DPLANE(box.face_Yn[0], box.normal_Yn);
			box.plane_Zp = MATH3DPLANE(box.face_Zp[0], box.normal_Zp);
			box.plane_Zn = MATH3DPLANE(box.face_Zn[0], box.normal_Zn);

			//printf("\nplane_Xp %f %f %f %f", box.plane_Xp.a, box.plane_Xp.b, box.plane_Xp.c, box.plane_Xp.d);
			//printf("\nplane_Xn %f %f %f %f", box.plane_Xn.a, box.plane_Xn.b, box.plane_Xn.c, box.plane_Xn.d);
			//printf("\nplane_Yp %f %f %f %f", box.plane_Yp.a, box.plane_Yp.b, box.plane_Yp.c, box.plane_Yp.d);
			//printf("\nplane_Yn %f %f %f %f", box.plane_Yn.a, box.plane_Yn.b, box.plane_Yn.c, box.plane_Yn.d);
			//printf("\nplane_Zp %f %f %f %f", box.plane_Zp.a, box.plane_Zp.b, box.plane_Zp.c, box.plane_Zp.d);
			//printf("\nplane_Zn %f %f %f %f", box.plane_Zn.a, box.plane_Zn.b, box.plane_Zn.c, box.plane_Zn.d);
		};

	public:
		//>> Установка метода отсечения невидимых граней: TRUE==CCW (default), FALSE==CW. Нельзя выполнить на уже загруженной модели!
		void SetModelCullmode(const bool& cull_CCW)
		{
			if (isInit) { _MBM(ERROR_MdlSetCullmode); return; }
			cullmode_CCW = cull_CCW;
		}
	
/*		//>> Взятие смещения группы относительно остальной модели
		WPOS CModel::GetGroupPos(const uint32& gNum)
		{
			return gmodel[gNum].data->pos;
		}

		//>> Установка смещения группы относительно остальной модели
		void CModel::SetGroupPos(const uint32& gNum, const WPOS& position)
		{
			gmodel[gNum].data->pos = position;
		} //*/

		//>> Включает или выключает (default) использование относительной позиции в рассчётах для конкретной группы НЕ статической модели
		void SetGroupUsePos(const uint32& gNum, const bool& use_position)
		{
			gmodel[gNum]->usePos = use_position;
		}

		//>> DEBUG: Сообщает ID и имена групп, а также состояние смещения
		void TellGroupsDebugInfo(const int& mode)
		{
			uint32 nGModels = (uint32)gmodel.size();
			switch(mode)
			{
			case 1:
				for(uint32 i=0; i<nGModels; i++)
				{
					MODELGROUP & grp = *gmodel[i];
					const WPOS * pos = grp.pos;
					printf("\n%3i: %s",i,grp.gname.c_str());
					printf("\npos   X Y Z: %f %f %f\nangle X Y Z: %f %f %f\nscale X Y Z: %f %f %f\n",
						pos->x,       pos->y,       pos->z,
						pos->angle_x, pos->angle_y, pos->angle_z,
						pos->scale_x, pos->scale_y, pos->scale_z);
				};
				break;
			default:
				for(uint32 i=0; i<nGModels; i++)
					printf("\n%3i: %s",i,gmodel[i]->gname.c_str());
			}		
		}

		//>> Сообщает количество групп в модели
		uint32 GetGroupsNum()
		{
			return (uint32)gmodel.size();
		}
	};
}

#endif // _MODEL_H