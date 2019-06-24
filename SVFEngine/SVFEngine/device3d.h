// ----------------------------------------------------------------------- //
//
// MODULE  : device3d.h
//
// PURPOSE : Вспомогательная оболочка 3D девайса
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _DEVICE3D_H
#define _DEVICE3D_H

#include "stdafx.h"

#define ERROR_InitDEV3DDAT		L"Ошибка инициализации DEV3DDAT"
#define ERROR_InitDirect3D		L"Ошибка инициализации Direct3D"
#define ERROR_InitDevice		L"Ошибка инициализации 3D девайса"
#define ERROR_InitDeviceHVP		L"HARDWARE_VERTEXPROCESSING не поддерживается"

// Check D3DPMISCCAPS

#define ERROR_D3DPMISCCAPS_MASKZ						L"Device can enable and disable modification of the depth buffer on pixel operations."
#define ERROR_D3DPMISCCAPS_CULLNONE						L"The driver does not perform triangle culling."
#define ERROR_D3DPMISCCAPS_CULLCW						L"The driver [does not] supports clockwise triangle culling."
#define ERROR_D3DPMISCCAPS_CULLCCW						L"The driver [does not] supports counterclockwise culling."
#define ERROR_D3DPMISCCAPS_COLORWRITEENABLE				L"Device [does not] supports per-channel writes [...] through the D3DRS_COLORWRITEENABLE state."
#define ERROR_D3DPMISCCAPS_CLIPPLANESCALEDPOINTS		L"Device [does not] correctly clips scaled points of size > 1.0 to user-defined clipping planes."
#define ERROR_D3DPMISCCAPS_CLIPTLVERTS					L"Device [does not] clips post-transformed vertex primitives."
#define ERROR_D3DPMISCCAPS_TSSARGTEMP					L"Device [does not] supports D3DTA for temporary register."
#define ERROR_D3DPMISCCAPS_BLENDOP						L"Device [does not] supports alpha-blending operations other than D3DBLENDOP_ADD."
#define ERROR_D3DPMISCCAPS_NULLREFERENCE				L"A reference device that does not render."
#define ERROR_D3DPMISCCAPS_INDEPENDENTWRITEMASKS		L"Device [does not] supports independent write masks for multiple element textures / render targets."
#define ERROR_D3DPMISCCAPS_PERSTAGECONSTANT				L"Device [does not] supports per-stage constants. "
#define ERROR_D3DPMISCCAPS_POSTBLENDSRGBCONVERT			L"Device [does not] supports conversion to sRGB after blending."
#define ERROR_D3DPMISCCAPS_FOGANDSPECULARALPHA			L"Device [does not] supports separate fog and specular alpha."
#define ERROR_D3DPMISCCAPS_SEPARATEALPHABLEND			L"Device [does not] supports separate blend settings for the alpha channel."
#define ERROR_D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS		L"Device [does not] supports different bit depths for multiple render targets."
#define ERROR_D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING	L"Device [does not] supports post-pixel shader operations for multiple render targets."
#define ERROR_D3DPMISCCAPS_FOGVERTEXCLAMPED				L"Device [does not] clamps fog blend factor per vertex."

#if (CULL_DEFCCW)
	#define  CULL_DEF   D3DCULL_CCW		// default cullmode
	#define  CULL_REV   D3DCULL_CW		// reverse cullmode
	#define  CULL_NON   D3DCULL_NONE	// cullmode off
#else
	#define  CULL_DEF   D3DCULL_CW		// default cullmode
	#define  CULL_REV   D3DCULL_CCW		// reverse cullmode
	#define  CULL_NON   D3DCULL_NONE	// cullmode off
#endif

namespace SAVFGAME
{
	// Чтобы избежать путаницы в вопросе "что-где", объявим под какими индексами какие текстуры.
	// В шейдере номер проставляем соответственно:  sampler Tex0 : register(s0); // index 0 = IDX_TEX_DIFFUSE

	enum eTextureIndex
	{
		IDX_TEX_DIFFUSE = 0, // color 2D map
		IDX_TEX_BUMP    = 1, // normal 2D map
		IDX_TEX_LIGHT   = 2, // shadow map ; light map
		IDX_TEX_VAR0    = 3, // ?
		IDX_TEX_VAR1    = 4, // ?
		IDX_TEX_VAR2    = 5, // ?
		IDX_TEX_VAR3    = 6, // ?
		IDX_TEX_ENVCUBE = 7  // skybox ; environment 3D cubemap
	};
	enum eDisplayFormat;
	enum eIntervalData;
	enum eDeviceError // device errors
	{
		DEVERROR_NONE,
		DEVERROR_HVP,			// HARDWARE_VERTEXPROCESSING not supported
		DEVERROR_IDXBF32,		// 32bit INDEX BUFFER not supported
		DEVERROR_PRESENTINT0,	// D3DPRESENT_INTERVAL_IMMEDIATE not supported
		DEVERROR_PRESENTINT1,	// D3DPRESENT_INTERVAL_ONE not supported
		DEVERROR_PRESENTINT2,	// D3DPRESENT_INTERVAL_TWO not supported
		DEVERROR_PRESENTINT3,	// D3DPRESENT_INTERVAL_THREE not supported
		DEVERROR_PRESENTINT4,	// D3DPRESENT_INTERVAL_FOUR not supported

		DEVERROR_ENUM_MAX
	};
	enum ePrimTip // подсказка примитива
	{
		TIP_PRIM_POINTLIST,
		TIP_PRIM_LINELIST,
		TIP_PRIM_LINESTRIP,
		TIP_PRIM_TRIANGLELIST,
		TIP_PRIM_TRIANGLESTRIP,
		TIP_PRIM_TRIANGLEFAN,

		TIP_PRIM_ENUM_MAX
	};
	enum eRenderState
	{
		RSTATE_NONE = 0,

		RSTATE_CULLMODE,				// see _STATE_CULLMODE_*      ?
		RSTATE_LIGHTING,				// see _STATE_ON / OFF        DX9 pipeline
		RSTATE_FOGENABLE,				// see _STATE_ON / OFF        DX9 pipeline
		RSTATE_COLORVERTEX,				// see _STATE_ON / OFF        DX9 pipeline
		RSTATE_ALPHABLENDENABLE,		// see _STATE_ON / OFF        ?
		RSTATE_ZENABLE,					// see _STATE_ZENABLE_*       ?
		RSTATE_ZWRITEENABLE,			// see _STATE_ON / OFF        ?
		RSTATE_STENCILENABLE,			// see _STATE_ON / OFF        ?
		RSTATE_CLIPPLANEENABLE,			// see _STATE_ON / OFF        ?
		RSTATE_BLENDOP,					// see _STATE_BLENDOP_*       ?
		RSTATE_SRCBLEND,				// see _STATE_BLEND_*         ?
		RSTATE_DESTBLEND,				// see _STATE_BLEND_*         ?
		///////////////////////
		_STATE_ON,
		_STATE_OFF,
		///////////////////////
		_STATE_CULLMODE_NONE,
		_STATE_CULLMODE_CW,
		_STATE_CULLMODE_CCW,
		//////////////////////
		_STATE_ZENABLE_OFF,
		_STATE_ZENABLE_ON_Z_BUF,
		_STATE_ZENABLE_ON_W_BUF,
		//////////////////////
		_STATE_BLENDOP_ADD,				// Result = Source + Destination
		_STATE_BLENDOP_SUBTRACT,		// Result = Source - Destination
		_STATE_BLENDOP_REVSUBTRACT,		// Result = Destination - Source
		_STATE_BLENDOP_MIN,				// Result = MIN(Source, Destination)
		_STATE_BLENDOP_MAX,				// Result = MAX(Source, Destination)
		//////////////////////
		_STATE_BLEND_ZERO,				// Blend factor is (0, 0, 0, 0)
		_STATE_BLEND_ONE,				// Blend factor is (1, 1, 1, 1)
		_STATE_BLEND_SRCCOLOR,			// Blend factor is (R, G, B, A)
		_STATE_BLEND_INVSRCCOLOR,		// Blend factor is (1-R, 1-G, 1-B, 1-A)
		_STATE_BLEND_SRCALPHA,			// Blend factor is (A, A, A, A)
		_STATE_BLEND_INVSRCALPHA,		// Blend factor is (1-A, 1-A, 1-A, 1-A)
		_STATE_BLEND_DESTALPHA,			// Blend factor is (Ad, Ad, Ad, Ad)
		_STATE_BLEND_INVDESTALPHA,		// Blend factor is (1-Ad, 1-Ad, 1-Ad, 1-Ad)
		_STATE_BLEND_DESTCOLOR,			// Blend factor is (Rd, Gd, Bd, Ad)
		_STATE_BLEND_INVDESTCOLOR,		// Blend factor is (1-Rd, 1-Gd, 1-Bd, 1-Ad)
		_STATE_BLEND_SRCALPHASAT,		// Blend factor is (f, f, f, 1); where f = min(A, 1-Ad)
		_STATE_BLEND_BOTHSRCALPHA,		// ...
		_STATE_BLEND_BOTHINVSRCALPHA,	// Source blend factor is (1-A, 1-A, 1-A, 1-A), and destination blend factor is (A, A, A, A)
										// The destination blend selection is overridden
										// This blend mode is supported only for the RSTATE_SRCBLEND render state
		RSTATE_ENUM_MAX
	};
	enum eTextureState
	{
		TSTATE_FONT,				// (dx9) font/UI state settings
		TSTATE_MODEL_DIFFUSE_YES,	// (dx9) diffuse texture presented
		TSTATE_MODEL_DIFFUSE_NO,	// (dx9) diffuse texture missed
		TSTATE_MODEL_NORMALMAP_YES,	// (dx9) bump texture presented
		TSTATE_MODEL_NORMALMAP_NO,	// (dx9) bump texture missed

		TSTATE_ENUM_MAX
	};
	struct eRenderStateMem
	{
		eRenderStateMem()
		{
			MEM_RSTATE_CULLMODE         = RSTATE_NONE;
			MEM_RSTATE_LIGHTING         = RSTATE_NONE;
			MEM_RSTATE_FOGENABLE        = RSTATE_NONE;
			MEM_RSTATE_COLORVERTEX      = RSTATE_NONE;
			MEM_RSTATE_ALPHABLENDENABLE = RSTATE_NONE;
			MEM_RSTATE_ZENABLE          = RSTATE_NONE;
			MEM_RSTATE_ZWRITEENABLE     = RSTATE_NONE;
			MEM_RSTATE_STENCILENABLE    = RSTATE_NONE;
			MEM_RSTATE_CLIPPLANEENABLE  = RSTATE_NONE;
			MEM_RSTATE_BLENDOP          = RSTATE_NONE;
			MEM_RSTATE_SRCBLEND         = RSTATE_NONE;
			MEM_RSTATE_DESTBLEND        = RSTATE_NONE;
			restore_memory_call = false;
		}
		eRenderState	MEM_RSTATE_CULLMODE;
		eRenderState	MEM_RSTATE_LIGHTING;
		eRenderState    MEM_RSTATE_FOGENABLE;
		eRenderState    MEM_RSTATE_COLORVERTEX;
		eRenderState	MEM_RSTATE_ALPHABLENDENABLE;
		eRenderState	MEM_RSTATE_ZENABLE;
		eRenderState	MEM_RSTATE_ZWRITEENABLE;
		eRenderState	MEM_RSTATE_STENCILENABLE;
		eRenderState	MEM_RSTATE_CLIPPLANEENABLE;
		eRenderState	MEM_RSTATE_BLENDOP;
		eRenderState	MEM_RSTATE_SRCBLEND;
		eRenderState	MEM_RSTATE_DESTBLEND;
		bool			restore_memory_call;
	};
	struct DEPTHSTENCIL
	{
		DEPTHSTENCIL()  { Reset(); }
		~DEPTHSTENCIL() { }
		void Reset()
		{
			format = 0;
			enableAuto = true;
		}
		UINT	format;		// eDisplayFormat
		bool	enableAuto;	// EnableAutoDepthStencil
	};
	struct DISPLAYMODE // monitor settings
	{	
		DISPLAYMODE()  { Reset(); }
		~DISPLAYMODE() { }
		void Reset()
		{
			width       = 0;
			height      = 0;
			refreshRate = 0;
			format      = 0;
			////////////////
			numModes32  = 0;
			numModes16  = 0;
			for (auto & _ : mode) _ = false;
			dstencil.Reset();
		}
		UINT	width;			// monitor width (pixels)
		UINT	height;			// monitor height (pixels)
		UINT	refreshRate;	// Hz
		UINT	format;			// eDisplayFormat (surface format)	
		////////////////////
		UINT	numModes32;		// debug
		UINT	numModes16;		// debug
		bool	mode[0x100];	// debug :: true == is OK
		////////////////////
		DEPTHSTENCIL dstencil;
	};
	struct ADAPTER3DDESC
	{
		struct ADAPTER_GUID { // < a42790e0-7810-11cf-8f52-0040333594a3 >
			uint32 Data1;
			uint16 Data2;
			uint16 Data3;
			uint8  Data4[8];
		};
		union DRIVERVER
		{
			uint64 _U64;
			struct
			{
				uint32 _U32HI;
				uint32 _U32LO;
			};
			struct
			{
				uint16 product;
				uint16 version;
				uint16 subVersion;
				uint16 build;
			};		
		};
		union WHQLCERT
		{
			uint32 _U32;
			struct
			{
				uint16 year;
				uint8  month;
				uint8  day;
			};
			struct
			{
				uint32 certified; // 0 = Not certified, 1 = WHQL validated, but no date information is available
			};
		};
		ADAPTER3DDESC()
		{
			driver        = "Not presented";
			description   = "Not presented";
			deviceName    = "Not presented";
			driverVersion._U64 = 0;
			vendorId  = 0;
			deviceId  = 0;
			subSysId  = 0;
			revision  = 0;
			WHQLLevel._U32 = 0;
			ZeroMemory(&deviceIdentifier, sizeof(GUID));
		}
		std::string		driver;
		std::string		description;
		std::string		deviceName;
		DRIVERVER		driverVersion;
		uint32			vendorId;				// identify a chip set ::   manufacturer   :: can be zero if unknown
		uint32			deviceId;				// identify a chip set ::       type       :: can be zero if unknown
		uint32			subSysId;				// identify a chip set :: particular board :: can be zero if unknown
		uint32			revision;				// identify a chip set ::       level      :: can be zero if unknown
		ADAPTER_GUID	deviceIdentifier;		// unique identifier for the driver and chip set pair
		WHQLCERT		WHQLLevel;
		////////////////////////////////
		void Printf()
		{
			printf("\n3D ADAPTER DESCRIPTION");
			printf("\n- driver      = %s", driver.c_str());
			printf("\n- description = %s", description.c_str());
			printf("\n- deviceName  = %s", deviceName.c_str());
			printf("\n- driverVer   = %llX", driverVersion._U64);
			printf("\n  - product   = %4X", driverVersion.product);
			printf("\n  - vers      = %4X", driverVersion.version);
			printf("\n  - subVers   = %4X", driverVersion.subVersion);
			printf("\n  - build     = %4X", driverVersion.build);
			printf("\n- vendorId    = %X", vendorId);
			printf("\n- deviceId    = %X", deviceId);
			printf("\n- subSysId    = %X", subSysId);
			printf("\n- revision    = %X", revision);
			printf("\n- GUID D1     = %X", deviceIdentifier.Data1);
			printf("\n- GUID D2     = %X", deviceIdentifier.Data2);
			printf("\n- GUID D3     = %X", deviceIdentifier.Data3);
			printf("\n- GUID D4     = %X%X%X%X%X%X%X%X",
				deviceIdentifier.Data4[0], deviceIdentifier.Data4[1],
				deviceIdentifier.Data4[2], deviceIdentifier.Data4[3],
				deviceIdentifier.Data4[4], deviceIdentifier.Data4[5],
				deviceIdentifier.Data4[6], deviceIdentifier.Data4[7]);
			printf("\n- WHQL cert   = %X", WHQLLevel.certified);
			printf("\n- WHQL year   = %i", WHQLLevel.year);
			printf("\n- WHQL month  = %i", WHQLLevel.month);
			printf("\n- WHQL day    = %i", WHQLLevel.day);
		}
	};
	struct ADAPTERCAPS
	{
		ADAPTERCAPS()
		{
			maxVertexIndex         = 0;
			presentationInterval._ = 0;		
		}
		uint32 maxVertexIndex;
		union
		{
			byte _;
			struct
			{
				bool immediate : 1;
				bool one       : 1;
				bool two       : 1;
				bool three     : 1;
				bool four      : 1;
			};
		} presentationInterval;
	};

	struct DataManagerCubeTexture // 3d texture manager : virtual base class
	{
	public:
		DataManagerCubeTexture() {};
		virtual ~DataManagerCubeTexture() {};
		virtual void Close() = 0;
		virtual bool Loaded() = 0;
	private:
		DataManagerCubeTexture(DataManagerCubeTexture& src);
		DataManagerCubeTexture(DataManagerCubeTexture&& src);
		DataManagerCubeTexture& operator=(DataManagerCubeTexture& src);
		DataManagerCubeTexture& operator=(DataManagerCubeTexture&& src);
	};
	struct DataManagerTexture // 2d texture manager : virtual base class
	{
	public:
		DataManagerTexture() {};
		virtual ~DataManagerTexture() {};
		virtual void Close() = 0;
		virtual bool Loaded() = 0;
	private:
		DataManagerTexture(DataManagerTexture& src);
		DataManagerTexture(DataManagerTexture&& src);
		DataManagerTexture& operator=(DataManagerTexture& src);
		DataManagerTexture& operator=(DataManagerTexture&& src);
	};
	struct DataManagerModel // verticies manager : virtual base class
	{
	public:
		DataManagerModel() : num_vertices(0), num_indices(0), num_primitives(0) {};
		virtual ~DataManagerModel() {};
		virtual void Close() = 0;
		virtual bool Loaded() = 0;
		uint32	num_vertices;
		uint32	num_indices;
		uint32	num_primitives;
	private:
		DataManagerModel(DataManagerModel& src);
		DataManagerModel(DataManagerModel&& src);
		DataManagerModel& operator=(DataManagerModel& src);
		DataManagerModel& operator=(DataManagerModel&& src);
	};

	class DEV3DBASE // videodevice : virtual base class
	{
	public:
		DEV3DBASE() { Default(); };
		virtual ~DEV3DBASE() { Default(); };
		virtual void Close() = 0;
	public:
		virtual void SetTransformView(MATH3DMATRIX * mView) = 0;
		virtual void SetTransformProjection(MATH3DMATRIX * mProjection) = 0;
		virtual void SetTransformWorld(MATH3DMATRIX * mWorld) = 0;
		virtual void SetViewPort(uint32 x, uint32 y, uint32 width, uint32 height, float minZ, float maxZ) = 0;
		virtual void SetMaterial(uint32 ambient, uint32 diffuse, uint32 specular, uint32 emissive, float power) = 0;
		virtual void SetTextureStageState(byte index, eTextureState state) = 0;
		virtual void DrawPrimitive(ePrimTip tip, int32 base_vert, uint32 min_vert, uint32 verts, uint32 start_index, uint32 prims) = 0;
		virtual eDeviceError CheckDevice() = 0;
		virtual void PrintfDeviceCaps() = 0;
	public:
		virtual UINT GetRenderState(eRenderState RSTATE) = 0;
		virtual void SetRenderState(eRenderState RSTATE, UINT _STATE_) = 0;
		virtual void PreviousRenderState(eRenderState RSTATE) = 0;
	public:
		virtual void SendToRender(const DataManagerModel * modeldata, uint32 start_vertex_offset_in_bytes, uint32 szvrt) = 0;
		virtual void SendToRender(const DataManagerTexture * texturedata, uint32 stage) = 0;
		virtual void SendToRender(const DataManagerCubeTexture * texturedata, uint32 stage) = 0;
		virtual shared_ptr<DataManagerModel> CreateModelData(const void* vrt, const uint32* idc, uint32 vrtNum, uint32 idcNum, uint32 szvrt)=0;
		virtual shared_ptr<DataManagerTexture> CreateTextureData(const void* texbuf, uint32 texsize) = 0;
		virtual shared_ptr<DataManagerCubeTexture> CreateCubeTextureData(const void* texbuf, uint32 texsize) = 0;
	public:
		void SetDesktopResolution(int horizontal, int vertical)
		{
			SetDisplay(horizontal, vertical, MISSING, MISSING);
		}
		virtual bool CheckPresentationInterval(eIntervalData present) = 0;
		void SetPresentationInterval(eIntervalData present)
		{
			if (CheckPresentationInterval(present))
				PresentationInterval = present;
		}
		void SetSamplesWindowed(DWORD nSamples)
		{
			     if (nSamples == 0)                 nSamples_windowed_now = 1;
			else if (nSamples >= nSamples_windowed) nSamples_windowed_now = nSamples_windowed;
			else                                    nSamples_windowed_now = nSamples;		
		}
		void SetSamplesFullscreen(DWORD nSamples)
		{
			     if (nSamples == 0)                   nSamples_fullscreen_now = 1;
			else if (nSamples >= nSamples_fullscreen) nSamples_fullscreen_now = nSamples_fullscreen;
			else                                      nSamples_fullscreen_now = nSamples;
		}
		void SetBackBuffersNum(DWORD nBuffers)
		{
			if (!nBuffers)			nBackBuffer = 1;
			else if (nBuffers > 8)	nBackBuffer = 8;
			else					nBackBuffer = nBuffers;
		}
		eIntervalData GetPresentationInterval()
		{
			return PresentationInterval;
		}
		DWORD GetSamplesWindowed(bool true_get_now_or_false_get_max)
		{
			if (true_get_now_or_false_get_max == true)
				 return nSamples_windowed_now;
			else return nSamples_windowed;
		}
		DWORD GetSamplesFullscreen(bool true_get_now_or_false_get_max)
		{
			if (true_get_now_or_false_get_max == true)
				 return nSamples_fullscreen_now;
			else return nSamples_fullscreen;
		}
		DWORD GetBackBuffersNum()
		{
			return nBackBuffer;
		}
		DISPLAYMODE GetDisplay()
		{
			return display;
		}
		DISPLAYMODE GetDisplayDefault()
		{
			return display_default;
		}
	protected:
		void SetDisplay(UINT width, UINT height, UINT refreshRate, UINT format)
		{
			if (_NOMISS(width))			display.width		= width;
			if (_NOMISS(height))		display.height		= height;
			if (_NOMISS(refreshRate))	display.refreshRate = refreshRate;
			if (_NOMISS(format))		display.format		= format;

		/*	printf("\ndisplayMode.Width       %i", display.width);
			printf("\ndisplayMode.Height      %i", display.height);
			printf("\ndisplayMode.RefreshRate %i", display.refreshRate);
			printf("\ndisplayMode.Format      %i", display.format);			//*/
		}
		void SetDisplay(DEPTHSTENCIL dstencil)
		{
			display.dstencil.format     = dstencil.format;
			display.dstencil.enableAuto = dstencil.enableAuto;
		}
		void SetDisplayDefault(UINT width, UINT height, UINT refreshRate, UINT format)
		{
			if (_NOMISS(width))			display_default.width       = width;
			if (_NOMISS(height))		display_default.height      = height;
			if (_NOMISS(refreshRate))	display_default.refreshRate = refreshRate;
			if (_NOMISS(format))		display_default.format      = format;
		}
		void SetDisplayDefault(UINT numModes32, UINT numModes16)
		{
			display_default.numModes32 = numModes32;
			display_default.numModes16 = numModes16;
		}
		void SetDisplayDefault(UINT mode_id, bool working_condition)
		{
			display_default.mode[mode_id] = working_condition;
		}
		void SetDisplayDefault(DEPTHSTENCIL dstencil)
		{
			display_default.dstencil.format     = dstencil.format;
			display_default.dstencil.enableAuto = dstencil.enableAuto;
		}
	private:
		DEV3DBASE(const DEV3DBASE& src);
		DEV3DBASE(DEV3DBASE&& src);
		DEV3DBASE& operator=(DEV3DBASE&& src);
		DEV3DBASE& operator=(const DEV3DBASE& src);
	protected:
		virtual void Default()
		{
			nSamples_windowed = 1;
			nSamples_fullscreen = 1;
			nSamples_windowed_now = 1;
			nSamples_fullscreen_now = 1;
			nBackBuffer = 1;
			display.Reset();
		}
		////////////
		DWORD			nBackBuffer;				// count back buffers
		DWORD			nSamples_windowed;			// antialiasing
		DWORD			nSamples_fullscreen;		// antialiasing
		DWORD			nSamples_windowed_now;		// antialiasing
		DWORD			nSamples_fullscreen_now;	// antialiasing
		eIntervalData	PresentationInterval;		// VSync
		DISPLAYMODE		display;					// presented monitor settings
		DISPLAYMODE		display_default;			// [DEBUG] default settings
		ADAPTER3DDESC	adapter_description;		// [DEBUG]
		ADAPTERCAPS		adapter_caps;
		eRenderStateMem RS;							// render state memory
	};
}

#endif // _DEVICE3D_H