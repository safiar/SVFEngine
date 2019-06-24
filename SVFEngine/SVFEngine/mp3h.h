// ----------------------------------------------------------------------- //
//
// MODULE  : mp3h.h
//
// PURPOSE : Чтение заголовков MPEG-1 Layer-3
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _MP3H_H
#define _MP3H_H

#include "helper.h"
#include "mp3hf.h"

#define ID3_SIGN      0x494433				// 'ID3' label
#define ID3_SIZE      10					// ID3 header size (in bytes)
#define APE_SIGN      0x4150455441474558	// 'APETAGEX' label
#define APE_SIZE      32                    // APE header size (in bytes)

#define MPG_SIZE      4          // MPEG header size (in bytes)
#define MPG_SYNC      0x7FF      // MPEG synchronization word
#define MPG_CRC16     2          // crc16 size
#define MPG_ERROR     -1		 // error ID
#define MPG_XING_VBR  0x58696E67 // xing VBR 'Xing' sign
#define MPG_XING_CBR  0x496E666F // xing CBR 'Info' sign
#define MPG_VBRI      0x56425249 // Fraunhofer 'VBRI'
#define MPG_VBRI_SIZE 26         // Fraunhofer 'VBRI' base size
#define MPG_LAME      0x4C414D45 // xing VBR 'LAME' sign

#define MP31152		1152	// samples per 1 channel in frame, MP3GRS * 2
#define MP3LINES    18      // 576 samples [32 subband blocks * 18 frequency lines] per 1 granule
#define MP3SUBBANDS 32      // 576 samples [32 subband blocks * 18 frequency lines] per 1 granule
#define MP3GRS      576     // 576 samples [32 subband blocks * 18 frequency lines] per 1 granule
#define MP3SFB_MS   3       // sfb min index of short in mixed block .s[3..12|rzero]
#define MP3SFB_ML   8       // sfb max index of long  in mixed block .l[0..8]
#define MP3SFB_SWIN 3       // count of windows in short blocks 
#define MP3MAXHUFF  8207    // max. sample value from huffman code = 1 + (0x1FFF + 0xF) = 1 + (8191 + 15) :: NOTE linbits max 13 -> 0x1FFF
#define MP3MAXSCAL  15      // max. scalefactor value (4 bits)
#define MP3MAXPRET  4		// max. scalefac_pretab value
#define MP3ANTIALI  8		// num of antialias steps (long blocks)
#define MP3STEREOR  7		// num of ratios for process stereo
#define MP3FREQINV  144     // freq. inversion num of steps

#define MP3JUMPS 16 // jump over 8 frames :: possible values for jumper is [4, 6, 8, 10...]

#define MP3FRAMESIZE      4608 // 2 * (2 * MP3GRS) * sizeof(uint16)  // 4608 = 2ch * (2gr * 576 samples) * 2 bytes of 16-bit out value
#define MP3FRAMESIZE_half 2304 // MP3FRAMESIZE / 2 == 4608 / 2 == one granule, two channels

#define MP3LAMEDECDELAY   528  // using to calc LAME-encoded paddings

namespace SAVFGAME
{

	//>> ID3 заголовок
	struct ID3TAG
	{
		ID3TAG(const ID3TAG & src)            = delete;
		ID3TAG(ID3TAG && src)                 = delete;
		ID3TAG& operator=(const ID3TAG & src) = delete;
		ID3TAG& operator=(ID3TAG && src)      = delete;

		ID3TAG() { _Reset(); };
		~ID3TAG() {};

		uint32 sign;	// 3 bytes, 'ID3' label
		uint16 version; // 2 bytes, major + minor version
		uint8  flag;	// 1 byte, special flag : 0/a/b/c/d
		uint32 size;	// 4 bytes, upcoming data size in synchsafe (mask 0x7F7F7F7F) integer

		//>> Сброс в ноль
		void _Reset()
		{
			sign    = 0;
			version = 0;
			flag    = 0;
			size    = 0;
		}

		//>> Чтение ID3 заголовка (возвращает FALSE при EOF)
		bool _Read(byte * data, uint32 size_in_bytes)
		{
			if (size_in_bytes < ID3_SIZE) // EOF
				return false;

			_Reset();

			sign    |= (data[0] << 16);
			sign    |= (data[1] <<  8);
			sign    |= (data[2]      );

			version |= (data[3] <<  8);
			version |= (data[4]      );

			flag    |= (data[5]      );

			size    |= ((data[6] & 0x7F) << 21);
			size    |= ((data[7] & 0x7F) << 14);
			size    |= ((data[8] & 0x7F) <<  7);
			size    |= ((data[9] & 0x7F)      );

			return true;
		}
		
		//>> Чтение ID3 заголовка (возвращает FALSE при EOF)
		//bool _Read(FILE * fp, bool fp_rewind)
		//{		
		//	uint64 fsize = _filelengthi64(_fileno(fp));
		//	if (!fp_rewind)
		//		fsize -= _ftelli64(fp);
		//	if (fsize < ID3_SIZE) // EOF
		//		return false;
		//
		//	byte data[ID3_SIZE];
		//	if (fp_rewind) rewind(fp);
		//	fread(data, 1, ID3_SIZE, fp);
		//	return _Read(data, ID3_SIZE);
		//}
	};

	//>> APE заголовок
	struct APETAG
	{
		APETAG(const APETAG & src)            = delete;
		APETAG(APETAG && src)                 = delete;
		APETAG& operator=(const APETAG & src) = delete;
		APETAG& operator=(APETAG && src)      = delete;

		APETAG() { _Reset(); };
		~APETAG() {};

		uint64 sign;	 // 8 bytes, 'APETAGEX' (41 50 45 54 41 47 45 58)
		uint32 version;  // 4 bytes, APE tag version
		uint32 size;	 // 4 bytes, APE tag size (without this header)
		uint32 count;	 // 4 bytes, APE num of items
		uint32 flags;	 // 4 bytes
		uint64 reserved; // 8 bytes

		//>> Сброс в ноль
		void _Reset()
		{
			sign     = 0;
			version  = 0;
			size     = 0;
			count    = 0;
			flags    = 0;
			reserved = 0;
		}

		//>> Чтение ID3 заголовка (возвращает FALSE при EOF)
		bool _Read(byte * data, uint32 size_in_bytes)
		{
			if (size_in_bytes < ID3_SIZE) // EOF
				return false;

			_Reset();

			int total = 0;
			for (int i = 56; i >= 0; i -= 8) sign     |= (data[total++] << i);
			for (int i = 24; i >= 0; i -= 8) version  |= (data[total++] << i);
			for (int i = 24; i >= 0; i -= 8) size     |= (data[total++] << i);
			for (int i = 24; i >= 0; i -= 8) count    |= (data[total++] << i);
			for (int i = 24; i >= 0; i -= 8) flags    |= (data[total++] << i);
			for (int i = 56; i >= 0; i -= 8) reserved |= (data[total++] << i);

			return true;
		}
	};

	///////////////////////////////////////////////////////////////////////////////////////////

	//>> Типы слоев
	enum MPEGLayer : unsigned int
	{
		eMPEGLayer0 = 0, // 00 : reserved
		eMPEGLayer1 = 3, // 11 : MPEG Layer-1
		eMPEGLayer2 = 2, // 10 : MPEG Layer-2
		eMPEGLayer3 = 1  // 01 : MPEG Layer-3
	};

	//>> Типы MPEG
	enum MPEGType : unsigned int
	{
		eMPEGType0  = 1, // 01 : reserved  
		eMPEGType25 = 0, // 00 : MPEG-2.5  
		eMPEGType2  = 2, // 10 : MPEG-2
		eMPEGType1  = 3, // 11 : MPEG-1
	};

	//>> Частота семплов
	enum MPEGFreq : unsigned int
	{
		eMPEGFreq44100 = 0, // 00 : 44100 KHz (MPEG-1), 22050 KHz (MPEG-2), 11025 KHz (MPEG-2.5)
		eMPEGFreq48000 = 1, // 01 : 48000 KHz (MPEG-1), 24000 KHz (MPEG-2), 12000 KHz (MPEG-2.5)
		eMPEGFreq32000 = 2, // 10 : 32000 KHz (MPEG-1), 18000 KHz (MPEG-2),  8000 KHz (MPEG-2.5)
		eMPEGFreqXXXXX = 3, // 11 : reserved  
	};

	//>> Битрейт kbps (kbps = 1000 bits per second)
	enum MPEGRate : unsigned int
	{ 
		eMPEGBitrateFREE = 0,  // 0000 : unknown
		eMPEGBitrate32   = 1,  // 0001 : MPEG-1 Layer 1 / 2 / 3 =  32 /  32 /  32 ;   MPEG-2(2.5) Layer 1 / 2, 3 =  32 /   8
		eMPEGBitrate40   = 2,  // 0010 : MPEG-1 Layer 1 / 2 / 3 =  64 /  48 /  40 ;   MPEG-2(2.5) Layer 1 / 2, 3 =  48 /  16
		eMPEGBitrate48   = 3,  // 0011 : MPEG-1 Layer 1 / 2 / 3 =  96 /  56 /  48 ;   MPEG-2(2.5) Layer 1 / 2, 3 =  56 /  24
		eMPEGBitrate56   = 4,  // 0100 : MPEG-1 Layer 1 / 2 / 3 = 128 /  64 /  56 ;   MPEG-2(2.5) Layer 1 / 2, 3 =  64 /  32
		eMPEGBitrate64   = 5,  // 0101 : MPEG-1 Layer 1 / 2 / 3 = 160 /  80 /  64 ;   MPEG-2(2.5) Layer 1 / 2, 3 =  80 /  40
		eMPEGBitrate80   = 6,  // 0110 : MPEG-1 Layer 1 / 2 / 3 = 192 /  96 /  80 ;   MPEG-2(2.5) Layer 1 / 2, 3 =  96 /  48
		eMPEGBitrate96   = 7,  // 0111 : MPEG-1 Layer 1 / 2 / 3 = 224 / 112 /  96 ;   MPEG-2(2.5) Layer 1 / 2, 3 = 112 /  56
		eMPEGBitrate112  = 8,  // 1000 : MPEG-1 Layer 1 / 2 / 3 = 256 / 128 / 112 ;   MPEG-2(2.5) Layer 1 / 2, 3 = 128 /  64
		eMPEGBitrate128  = 9,  // 1001 : MPEG-1 Layer 1 / 2 / 3 = 288 / 160 / 128 ;   MPEG-2(2.5) Layer 1 / 2, 3 = 144 /  80
		eMPEGBitrate160  = 10, // 1010 : MPEG-1 Layer 1 / 2 / 3 = 320 / 192 / 160 ;   MPEG-2(2.5) Layer 1 / 2, 3 = 160 /  96
		eMPEGBitrate192  = 11, // 1011 : MPEG-1 Layer 1 / 2 / 3 = 352 / 224 / 192 ;   MPEG-2(2.5) Layer 1 / 2, 3 = 176 / 112
		eMPEGBitrate224  = 12, // 1100 : MPEG-1 Layer 1 / 2 / 3 = 384 / 256 / 224 ;   MPEG-2(2.5) Layer 1 / 2, 3 = 192 / 128
		eMPEGBitrate256  = 13, // 1101 : MPEG-1 Layer 1 / 2 / 3 = 416 / 320 / 256 ;   MPEG-2(2.5) Layer 1 / 2, 3 = 224 / 144
		eMPEGBitrate320  = 14, // 1110 : MPEG-1 Layer 1 / 2 / 3 = 448 / 384 / 320 ;   MPEG-2(2.5) Layer 1 / 2, 3 = 256 / 160
		eMPEGBitrateXXXX = 15  // 1111 : reserved
	};

	//>> Режим каналов
	enum MPEGCh : unsigned int
	{
		eMPEGChStereo       = 0, // 00 : STEREO (2 channels)
		eMPEGChJointStereo  = 1, // 01 : STEREO (2 channels)
		eMPEGChDual         = 2, // 10 : STEREO (2 channels) with same data (MONOx2)
		eMPEGChSingle       = 3  // 11 : MONO   (1 channel)
	};

	//>> Подрежим каналов для режима joint stereo
	enum MPEGChJ : unsigned int
	{
		eMPEGChJBands4  = 0, // 00 : Layer-1/2 = bands 4  to 31  ::  Layer-3 = 0,  M/S stereo OFF, Intensity stereo OFF
		eMPEGChJBands8  = 1, // 01 : Layer-1/2 = bands 8  to 31  ::  Layer-3 = 4,  M/S stereo OFF, Intensity stereo ON
		eMPEGChJBands12 = 2, // 10 : Layer-1/2 = bands 12 to 31  ::  Layer-3 = 8,  M/S stereo ON,  Intensity stereo OFF
		eMPEGChJBands16 = 3  // 11 : Layer-1/2 = bands 16 to 31  ::  Layer-3 = 16, M/S stereo ON,  Intensity stereo ON
	};

	//>> Данные требуют 're-equalize' звука после 'Dolby-like noise suppression'
	enum MPEGEmphasis : unsigned int
	{
		eMPEGEmphasisNONE = 0, // 00 : nothing
		eMPEGEmphasis5015 = 1, // 01 : 50/15 ms
		eMPEGEmphasisXXXX = 2, // 10 : reserved
		eMPEGEmphasisCCIT = 3, // 11 : CCIT J.17
	};

	//>> Семплов на 1 кадр mp3
	enum MPEGSamples : unsigned int
	{
		eMPEGSamplesLayer1   =  384, // MPEG-1/2/2.5 Layer-1 samples per frame
		eMPEGSamplesLayer2   = 1152, // MPEG-1/2/2.5 Layer-2 samples per frame
		eMPEGSamplesLayer3_1 = 1152, // MPEG-1       Layer-3 samples per frame
		eMPEGSamplesLayer3_2 = 576   // MPEG-2/2.5   Layer-3 samples per frame
	};

	//>> Главный заголовок кадра MPEG
	struct MPEGHeader
	{
		bool operator==(const MPEGHeader & src) { return (src.__RAW == __RAW); }
		bool operator==(MPEGHeader && src)      { return (src.__RAW == __RAW); }
		bool operator!=(const MPEGHeader & src) { return (src.__RAW != __RAW); }
		bool operator!=(MPEGHeader && src)      { return (src.__RAW != __RAW); }

		MPEGHeader() { _Reset(); };
		~MPEGHeader() { };

		union {
			struct {                     // POSITON          MASK      DESCRIPTION
				MPEGEmphasis  emph :  2; // bit 31 .. 32  :  00000003, emphasis type
				unsigned  original :  1; // bit 30        :  00000004, original = 1, copy = 0
				unsigned copyright :  1; // bit 29        :  00000008, copyright = 1, no copyright = 0
				MPEGChJ        chj :  2; // bit 27 .. 28  :  00000030, channels type extension, valid only for <joint stereo>
				MPEGCh          ch :  2; // bit 25 .. 26  :  000000C0, channels type
				unsigned      priv :  1; // bit 24        :  00000100, bit without pre-defined meaning, for private purposes only
				unsigned   padding :  1; // bit 23        :  00000200, data padding [ 0 == NO, 1 == padded with 1 slot ]
				MPEGFreq      freq :  2; // bit 21 .. 22  :  00000C00, sample rate index
				MPEGRate   bitrate :  4; // bit 17 .. 20  :  0000F000, bit rate index
				unsigned  no_crc16 :  1; // bit 16        :  00010000, error protection [ 1 == NO CRC16, 0 == CRC16 after header ] 
				MPEGLayer    layer :  2; // bit 14 .. 15  :  00060000, layer number
				MPEGType      mpeg :  2; // bit 12 .. 13  :  00180000, MPEG version
				unsigned      sync : 11; // bit  1 .. 11  :  FFE00000, sync word == 0x7FF == MPG_SYNC
			};  uint32      __RAW;       // bit  1 .. 32  :  FFFFFFFF, all data in one pack
		};

		//>> Сброс в ноль
		void _Reset()
		{
			__RAW = 0;
		}

		//>> Чтение MPEG заголовка
		bool _Read(byte * data, uint32 size_in_bytes)
		{
			if (size_in_bytes < MPG_SIZE) return false;

			_Reset();

			__RAW |= (data[0] << 24);
			__RAW |= (data[1] << 16);
			__RAW |= (data[2] <<  8);
			__RAW |= (data[3]      );

			return true;
		}

		//>> Чтение MPEG заголовка
		bool _Read(FILE * fp, bool fp_rewind)
		{
			uint64 fsize = _filelengthi64(_fileno(fp));
			if (!fp_rewind)
				fsize -= _ftelli64(fp);
			if (fsize < MPG_SIZE) // EOF
				return false; 

			byte data [MPG_SIZE];
			if (fp_rewind) rewind(fp);
			fread(data, 1, MPG_SIZE, fp);
			return _Read(data, MPG_SIZE);
		}

		static
		//>> Возвращает MPG_ERROR в случае некорректного режима для layer 2
		uint32 _CheckLayerII(MPEGLayer layer, MPEGRate bitrate, MPEGCh ch)
		{
			if (layer != eMPEGLayer2) return true;
			else
			{
				if (bitrate == eMPEGBitrate192 || // 224 kbps layer 2
					bitrate == eMPEGBitrate224 || // 256 kbps layer 2
					bitrate == eMPEGBitrate256 || // 320 kbps layer 2
					bitrate == eMPEGBitrate320)   // 384 kbps layer 2
				{
					if (ch == eMPEGChSingle) return MPG_ERROR;
					else                     return true;
				}
				else
				if (bitrate == eMPEGBitrate32 || // 32 kbps layer 2
					bitrate == eMPEGBitrate40 || // 48 kbps layer 2
					bitrate == eMPEGBitrate48 || // 56 kbps layer 2
					bitrate == eMPEGBitrate64)   // 80 kbps layer 2
				{
					if (ch == eMPEGChSingle) return true;
					else                     return MPG_ERROR;
				}
				else return true;
			}
		}

		//>> Возвращает MPG_ERROR в случае некорректного режима для layer 2
		uint32 _CheckLayerII()
		{
			return _CheckLayerII(layer, bitrate, ch);
		}

		static
		//>> Возвращает bitrate из его индекса (или MPG_ERROR в случае неудачи)
		uint32 _Bitrate(MPEGRate bitrate, MPEGType mpeg, MPEGLayer layer)
		{
			static const uint16 BITRATE [15] [5] =
			{
				{   0,   0,   0,   0,   0}, // MPEG-1 Layer 1 / 2 / 3 ;   MPEG-2(2.5) Layer 1 / 2, 3
				{  32,  32,  32,  32,   8}, // 
				{  64,  48,  40,  48,  16}, // 
				{  96,  56,  48,  56,  24}, // 
				{ 128,  64,  56,  64,  32}, // 
				{ 160,  80,  64,  80,  40}, // 
				{ 192,  96,  80,  96,  48}, // 
				{ 224, 112,  96, 112,  56}, // 
				{ 256, 128, 112, 128,  64}, // 
				{ 288, 160, 128, 144,  80}, // 
				{ 320, 192, 160, 160,  96}, // 
				{ 352, 224, 192, 176, 112}, // 
				{ 384, 256, 224, 192, 128}, // 
				{ 416, 320, 256, 224, 144}, // 
				{ 448, 384, 320, 256, 160}, // 
			};

			const uint32 error = MPG_ERROR;
				  uint32 index;

			switch (bitrate) // check ERROR
			{
			case eMPEGBitrate32:
			case eMPEGBitrate40:
			case eMPEGBitrate48:
			case eMPEGBitrate56:
			case eMPEGBitrate64:
			case eMPEGBitrate80:
			case eMPEGBitrate96:
			case eMPEGBitrate112:
			case eMPEGBitrate128:
			case eMPEGBitrate160:
			case eMPEGBitrate192:
			case eMPEGBitrate224:
			case eMPEGBitrate256: 
			case eMPEGBitrate320: break;
			case eMPEGBitrateFREE:
			case eMPEGBitrateXXXX:
			default:
				return error;
			};

			switch (mpeg) // check ERROR & get index
			{
			case eMPEGType1:
				switch (layer)
				{
				case eMPEGLayer1: index = 0; break;
				case eMPEGLayer2: index = 1; break;
				case eMPEGLayer3: index = 2; break;
				default:
					return error;
				};
				break;

			case eMPEGType2:
			case eMPEGType25:
				switch (layer)
				{
				case eMPEGLayer1: index = 3; break;
				case eMPEGLayer2:
				case eMPEGLayer3: index = 4; break;
				default:
					return error;
				};
				break;

			default:
				return error;
			};

			return 1000ul * BITRATE [bitrate] [index];
		}

		//>> Возвращает bitrate из его индекса (или MPG_ERROR в случае неудачи)
		uint32 _Bitrate()
		{
			return _Bitrate (bitrate, mpeg, layer);
		}
	
		static
		//>> Возвращает количество семлов на кадр (или MPG_ERROR в случае неудачи)
		uint32 _Samples(MPEGType mpeg, MPEGLayer layer)
		{
			const uint32 error = MPG_ERROR;
			      uint32 ret   = error;

			switch (layer)
			{
			case eMPEGLayer1: ret = eMPEGSamplesLayer1; break;
			case eMPEGLayer2: ret = eMPEGSamplesLayer2; break;
			case eMPEGLayer3:
				switch (mpeg)
				{
				case eMPEGType1:  ret = eMPEGSamplesLayer3_1; break;
				case eMPEGType2:
				case eMPEGType25: ret = eMPEGSamplesLayer3_2; break;
				default:          ret = error;                break;
				};
				break;
			default:
				ret = error;
				break;
			};

			return ret;
		}

		//>> Возвращает количество семлов на кадр (или MPG_ERROR в случае неудачи)
		uint32 _Samples()
		{
			return _Samples(mpeg, layer);
		}
	
		static
		//>> Возвращает частоту семлов в секунду (или MPG_ERROR в случае неудачи)
		uint32 _Frequency(MPEGFreq frequency, MPEGType mpeg)
		{
			static const uint16 FREQUENCY [3] [4] =
			{
				{11025, 0, 22050, 44100}, // MPEG-2.5, reserved, MPEG-2, MPEG-1
				{12000, 0, 24000, 48000}, //
				{ 8000, 0, 18000, 32000}  //
			};

			const uint32 error = MPG_ERROR;

			switch (frequency) // check ERROR
			{
			case eMPEGFreq44100:
			case eMPEGFreq48000:
			case eMPEGFreq32000: break;
			case eMPEGFreqXXXXX:
			default:
				return error;
			};

			switch (mpeg) // check ERROR
			{
			case eMPEGType1:
			case eMPEGType2:
			case eMPEGType25: break;
			case eMPEGType0:
			default:
				return error;
			};

			return FREQUENCY [frequency] [mpeg];
		}

		//>> Возвращает частоту семлов в секунду (или MPG_ERROR в случае неудачи)
		uint32 _Frequency()
		{
			return _Frequency(freq, mpeg);
		}
	
		static
		//>> Возвращает размер слота кадра (или MPG_ERROR в случае неудачи)
		uint32 _Slot(MPEGLayer layer)
		{
			const uint32 error = MPG_ERROR;
			      uint32 ret   = error;

			switch (layer)
			{
			case eMPEGLayer1: ret =     4; break;
			case eMPEGLayer2:
			case eMPEGLayer3: ret =     1; break;
			default:          ret = error; break;
			}

			return ret;
		}

		//>> Возвращает размер слота кадра (или MPG_ERROR в случае неудачи)
		uint32 _Slot()
		{
			return _Slot(layer);
		}

		static
		//>> Возвращает размер для выравнивания кадра (или MPG_ERROR в случае неудачи)
		uint32 _Padding(MPEGLayer layer, bool pad)
		{
			const uint32 error = MPG_ERROR;
			      uint32 ret   = error;

			uint32 slot = _Slot(layer);

			if (slot == NULL) ret = error;
			else
			{
				if (pad) ret = slot;
				else     ret = 0;
			}

			return ret;
		}

		//>> Возвращает размер для выравнивания кадра (или MPG_ERROR в случае неудачи)
		uint32 _Padding()
		{
			return _Padding(layer, padding);
		}
	
		static
		//>> Возвращает размер сторонних данных в теле кадра (без crc) (или MPG_ERROR в случае неудачи)
		uint32 _Side(MPEGType mpeg, MPEGCh channel)
		{
			static const byte SIDE [2] [2] =
			{
				{17, 32}, // MPEG-1: {MONO, STEREO}
				{ 9, 17}  // MPEG-2: {MONO, STEREO}
			};

			const uint32 error = MPG_ERROR;
			      bool   mpeg2;
			      bool   stereo;

			switch (mpeg) // check ERROR & get mpeg type
			{
			case eMPEGType1:  mpeg2 = 0; break;
			case eMPEGType2:
			case eMPEGType25: mpeg2 = 1; break;
			default:
				return error;
			};

			switch (channel) // check ERROR & get channels type
			{
			case eMPEGChSingle:      stereo = 0; break;
			case eMPEGChDual:
			case eMPEGChStereo:
			case eMPEGChJointStereo: stereo = 1; break;
			default:
				return error;
			};

			return SIDE [mpeg2] [stereo];
		}

		//>> Возвращает размер сторонних данных в теле кадра (без crc) (или MPG_ERROR в случае неудачи)
		uint32 _Side()
		{
			return _Side(mpeg, ch);
		}

		static
		//>> Возвращает размер CRC, который следует за заголовком MPEG
		uint32 _CRC(bool no_crc16)
		{
			if (!no_crc16) return MPG_CRC16;
			else           return 0;
		}

		//>> Возвращает размер CRC, который следует за заголовком MPEG
		uint32 _CRC()
		{
			return _CRC(no_crc16);
		}

		static
		//>> Возвращает размер кадра, включая 4 байта заголовка MPEG (или MPG_ERROR в случае неудачи)
		uint32 _Frame(MPEGRate bitrate, MPEGType mpeg, MPEGLayer layer, MPEGFreq frequency, bool pad)
		{
			float ret;

			uint32 _bitrate   = _Bitrate(bitrate, mpeg, layer);
			uint32 _samples   = _Samples(mpeg, layer);
			uint32 _frequency = _Frequency(frequency, mpeg);
			uint32 _padding   = _Padding(layer, pad);
			uint32 _slot      = _Slot(layer);

				 if (_bitrate   == MPG_ERROR) return MPG_ERROR;
			else if (_samples   == MPG_ERROR) return MPG_ERROR;
			else if (_frequency == MPG_ERROR) return MPG_ERROR;
			else if (_padding   == MPG_ERROR) return MPG_ERROR;
			else
			{
				// Framesize = (((MPEGSamples / 8 * MPEGRate) / MPEGFreq) + padding) * slotsize

				ret  = (float)_samples / 8;
				ret *= (float)_bitrate / _frequency;
				ret *= _slot;
				ret += _padding; // <_padding> already have slotsize
			}

			return (uint32) ret;
		}

		//>> Возвращает размер кадра, включая 4 байта заголовка MPEG (или MPG_ERROR в случае неудачи)
		uint32 _Frame()
		{
			return _Frame(bitrate, mpeg, layer, freq, padding);
		}

		static
		//>> Возвращает чистый размер кадра (или MPG_ERROR в случае неудачи)
		uint32 _Pure(MPEGType mpeg, MPEGCh channel, MPEGRate bitrate, MPEGLayer layer, MPEGFreq frequency, bool no_crc16, bool pad)
		{
			// Для MPEG-1 Layer-3 минимально 66 (0x42) байт полезных данных

			uint32 _frame = _Frame(bitrate, mpeg, layer, frequency, pad);
			uint32 _side  = _Side(mpeg, channel);

			     if (_frame == MPG_ERROR) return MPG_ERROR;
			else if (_side  == MPG_ERROR) return MPG_ERROR;
			else
				return _frame - _side - _CRC(no_crc16) - MPG_SIZE;
		}

		//>> Возвращает чистый размер кадра (или MPG_ERROR в случае неудачи)
		uint32 _Pure()
		{
			return _Pure(mpeg, ch, bitrate, layer, freq, no_crc16, padding);
		}

		static
		//>> Возвращает продолжительность в секундах для ConstantBitRate (или MPG_ERROR в случае неудачи)
		double _TimeCBR(uint32 datasize, MPEGRate bitrate, MPEGType mpeg, MPEGLayer layer)
		{
			uint32 _bitrate = _Bitrate(bitrate, mpeg, layer);

			if (_bitrate == MPG_ERROR) return MPG_ERROR;

			return (double)datasize / (8 * _bitrate);
		}

		//>> Возвращает продолжительность в секундах для ConstantBitRate (или MPG_ERROR в случае неудачи)
		double _TimeCBR(uint32 datasize)
		{
			return _TimeCBR(datasize, bitrate, mpeg, layer);
		}
	
		static
		//>> Возвращает количество связок (или MPG_ERROR в случае неудачи)
		uint32 _Bands(MPEGCh ch, MPEGChJ chj, MPEGLayer layer)
		{
			const uint32 error = MPG_ERROR;
			      uint32 ret   = error;

			if (ch != eMPEGChJointStereo) // joint stereo only
			//	return 0;                 //
				return error;             //

			switch (layer)
			{
			case eMPEGLayer1:      // 
			case eMPEGLayer2:      // 
				ret = 4 + chj * 4; // 4, 8, 12, 16
				break;             //
			case eMPEGLayer3:			
				if (chj == eMPEGChJBands16) ret = 4 + chj * 4; // 16
				else                        ret = 0 + chj * 4; // 0, 4, 8
				break;                                        //
			default:         // ERROR
				ret = error; // 
				break;       // 
			}

			return ret;
		}

		//>> Возвращает количество <bands> (или MPG_ERROR в случае неудачи)
		uint32 _Bands()
		{
			return _Bands(ch, chj, layer);
		}

		static
		//>> Возвращает количество каналов
		byte _Channels(MPEGCh ch)
		{
			return (ch == eMPEGChSingle) ? 1 : 2;
		}

		//>> Возвращает количество каналов
		byte _Channels()
		{
			return _Channels(ch);
		}


		////////////////////////////////////////////

		struct INFOFRAME
		{
			uint32 size;		// full size of frame
			uint32 bitrate;		// bits per second
			uint32 samples;		// samples in frame
			uint32 frequency;	// samples per second
			uint32 side;		// side data size
			uint32 crc;			// crc data size
			uint32 pure;		// main data size
			uint32 channels;	// num of channels
		};

		INFOFRAME FRAME; // calculated info about frame

		//>> Просчитывает FRAME информацию :: возвращает <false>, если есть ошибки при анализе данных
		bool RUNFRAMECALC()
		{
			FRAME.size      = _Frame();
			FRAME.bitrate   = _Bitrate();
			FRAME.frequency = _Frequency();
			FRAME.samples   = _Samples();
			FRAME.side      = _Side();
			FRAME.pure      = _Pure();
			FRAME.crc       = _CRC();
			FRAME.channels  = _Channels();

			if ( FRAME.size      == MPG_ERROR ||
				 FRAME.bitrate   == MPG_ERROR ||
				 FRAME.frequency == MPG_ERROR ||
				 FRAME.samples   == MPG_ERROR ||
				 FRAME.side      == MPG_ERROR ||
				 FRAME.pure      == MPG_ERROR )
				 return false;
			else return true;
		}
	};

	///////////////////////////////////////////////////////////////////////////////////////////

	//>> Количество гранул кадра
	enum MPEGGranules : unsigned int
	{
		eMPEG_1_Granules = 2, // MPEG-1
		eMPEG_2_Granules = 1, // MPEG-2/2.5
	};

	//>> Типы блоков
	enum MPEGBlockType : unsigned int
	{
		eMPEGBlockT_Reserved     = 0,
		eMPEGBlockT_StartBlock   = 1,
		eMPEGBlockT_3ShortWindow = 2,
		eMPEGBlockT_EndBlock     = 3,

		eMPEGBlockT_ENUM_MAX
	};

	//>> Подзаголовок кадра MPEG [только для MPEG-1/2/2.5 Layer-3]
	struct MPEGHeaderSide
	{
                                         // gr = granule (2 = MPEG-1 or 1 = MPEG-2/2.5), ch = channel	 
		uint16 main_data_begin;          // 9 or 8 bits (MPEG-1 or MPEG-2/2.5)
		byte   private_bits;             // MONO/STEREO : 3/5 bits (MPEG-1) or 2/1 bits (MPEG-2/2.5)
		bool   scfsi[2][4];              // [ch][band], ch=1-2, band=4 :: 1 bit per obj (MPEG-1 only)
		uint16 part23len[2][2];          // [gr][ch], gr=1-2, ch=1-2 ::  12 bits per obj (num bits of main data)
		uint16 big_values[2][2];         // [gr][ch], gr=1-2, ch=1-2 ::   9 bits per obj
		byte   global_gain[2][2];        // [gr][ch], gr=1-2, ch=1-2 ::   8 bits per obj
		uint16 scalefac_compress[2][2];  // [gr][ch], gr=1-2, ch=1-2 :: 4/9 bits per obj (MPEG-1 or MPEG-2/2.5)
		bool   switch_flag[2][2];        // [gr][ch], gr=1-2, ch=1-2 ::   1 bit  per obj
		byte   block_type[2][2];		 // [gr][ch], gr=1-2, ch=1-2 ::   2 bits per obj
										 // (switch_flag : true)  -> various (read from buffer)
										 // (switch_flag : false) -> block_type = 0;
		bool   mixed_block_flag[2][2];   // [gr][ch], gr=1-2, ch=1-2 ::   1 bit  per obj (switch_flag : true)
		byte   table_select[2][2][3];    // [gr][ch][region], gr=1-2, ch=1-2, region=2/3 (switch_flag : true/false) :: 5 bits per obj
		byte   subblock_gain[2][2][3];   // [gr][ch][window], gr=1-2, ch=1-2, window=3   (switch_flag : true) :: 3 bits per obj
		byte   region0_count[2][2];      // [gr][ch], gr=1-2, ch=1-2 ::   4 bits per obj
										 //	(switch_flag : true)  -> (block_type == 2 && mixed_block_flag == 0) 8 (ELSE) 7
										 //	(switch_flag : false) -> various (read from buffer)
		byte   region1_count[2][2];      // [gr][ch], gr=1-2, ch=1-2 ::   3 bits per obj
										 //	(switch_flag : true)  -> region1_count = 20 - region0_count;
										 //	(switch_flag : false) -> various (read from buffer)
		bool   preflag[2][2];            // [gr][ch], gr=1-2, ch=1-2 ::   1 bit  per obj (MPEG-1 only)
		bool   scalefac_scale[2][2];     // [gr][ch], gr=1-2, ch=1-2 ::   1 bit  per obj
		bool   count1table_select[2][2]; // [gr][ch], gr=1-2, ch=1-2 ::   1 bit  per obj
		uint16 count1[2][2];			 // [gr][ch], 1st sample ID of rzero-reg, calc while huff dec (MPEG-1 L3 : 576 max ID)

		// main_data_begin : num of bytes "main data" ends before next frame header
		// private_bits : this is garbage
		// scfsi : if (scfsi == true) scalefac[cur.granule] = scalefac[prev.granule]
		// part23len : len of scalefactors and maindata in bits
		// big_value : num of values in each big_region
		// global_gain : quantizer step size
		// scalefac_compress : used to determine the values of slen1 and slen2
		// switch_flag : window switch flag
		// block_type  : window type for granule
		//               0 = reserved, 1 = start block, 2 = 3 short windows, 3 = end block
		// mixed_block_flag : num of scalefactor bands before window switching (8/3 <true> or 0/0 <false> long/short) 
		// table_select : huffman table num for big_region
		// region0_count : num of scalefactor bands in 1st big value region  (scale factor bands 12 * 3 = 36)
		// region1_count : num of scalefactor bands in 3rd big value region 
		// preflag : if (true) add values from table to scalefactors
		// scalefac_scale : provide step's size
		// count1table_select : select count1 table

		MPEGHeaderSide(const MPEGHeaderSide & src)            = delete;
		MPEGHeaderSide(MPEGHeaderSide && src)                 = delete;
		MPEGHeaderSide& operator=(const MPEGHeaderSide & src) = delete;
		MPEGHeaderSide& operator=(MPEGHeaderSide && src)      = delete;

		MPEGHeaderSide() { _Reset(); };
		~MPEGHeaderSide() { };

		//>> Сброс в ноль
		void _Reset()
		{
			ZeroMemory(this, sizeof(MPEGHeaderSide));
		}

		//>> Чтение MPEG заголовка   / ошибки чтения не проверяются! заголовок внутреннего пользования /
		void _Read(byte *& data, byte & position_bit, MPEGHeader * mpeg)
		{
			_Reset();

			static const byte PRIVATE_BITS_LEN [2] [2] = {
				{1, 2}, // MPEG-2: stereo/mono
				{3, 5}  // MPEG-1: stereo/mono
			};

			bool mpeg_1   = (mpeg->mpeg == eMPEGType1);
			bool mono     = (mpeg->ch == eMPEGChSingle);
			byte channels = mono ? 1 : 2;
			byte granules = mpeg_1 ? eMPEG_1_Granules : eMPEG_2_Granules;

			main_data_begin = (uint16) ReadBits (data, position_bit, mpeg_1 ? 9:8);
			private_bits    = (byte)   ReadBits (data, position_bit, PRIVATE_BITS_LEN [mpeg_1] [mono] );

			if (mpeg_1) // MPEG-1 scalefactor select info
			for (byte   ch = 0;   ch < channels;   ch++)
			for (byte band = 0; band < 4;        band++)
			scfsi[ch][band] = _BOOL(ReadBits(data, position_bit, 1));

			for (byte gr = 0; gr < granules; gr++) {
				for (byte ch = 0; ch < channels; ch++) {
					part23len         [gr][ch] = (uint16) ReadBits (data, position_bit, 12);
					big_values        [gr][ch] = (uint16) ReadBits (data, position_bit,  9);
					global_gain       [gr][ch] = (byte)   ReadBits (data, position_bit,  8);
					scalefac_compress [gr][ch] = (uint16) ReadBits (data, position_bit,  mpeg_1 ? 4:9);
					switch_flag       [gr][ch] = _BOOL(   ReadBits (data, position_bit,  1) );

					if (switch_flag [gr][ch])
					{
						block_type        [gr][ch]         = (byte) ReadBits (data, position_bit, 2);
						mixed_block_flag  [gr][ch]         = _BOOL( ReadBits (data, position_bit, 1) );
						for (byte region = 0; region < 2; region++)
							table_select  [gr][ch][region] = (byte) ReadBits (data, position_bit, 5);
						for (byte window = 0; window < 3; window++)
							subblock_gain [gr][ch][window] = (byte) ReadBits (data, position_bit, 3);

						if (block_type [gr][ch] == 2 && mixed_block_flag [gr][ch] == 0) // ? mixed_block_flag нужен ?
							 region0_count [gr][ch] = 8;
						else region0_count [gr][ch] = 7;

						region1_count [gr][ch] = 20 - region0_count [gr][ch];
					}
					else
					{
						block_type        [gr][ch]         = 0;
						mixed_block_flag  [gr][ch]         = false;
						for (byte region = 0; region < 3; region++)
							table_select  [gr][ch][region] = (byte) ReadBits (data, position_bit, 5);
						region0_count     [gr][ch]         = (byte) ReadBits (data, position_bit, 4);
						region1_count     [gr][ch]         = (byte) ReadBits (data, position_bit, 3);
					}

					if (mpeg_1)
					preflag            [gr][ch] = _BOOL( ReadBits (data, position_bit, 1) );
					else // MPEG-2/2.5
					preflag            [gr][ch] = (scalefac_compress [gr][ch] >= 500);

					scalefac_scale     [gr][ch] = _BOOL( ReadBits (data, position_bit, 1) );
					count1table_select [gr][ch] = _BOOL( ReadBits (data, position_bit, 1) );
				}
			}
		}
	};

	///////////////////////////////////////////////////////////////////////////////////////////

	// MPEG-1 
	// Кадр состоит из 2 гранул
	// Гранула содержит 32 блока
	// Блок содержит 18 частотных линий

	//Encoder delays :
	//ISO models & MPECKER : 528 samples
	//FhG mp3enc31 : 1160 samples

	enum eMPEGDStatus : unsigned int
	{
		eMPEGDStatus_Normal     = 0,           // no errors
		eMPEGDStatus_PhysFrame  = MASK_BIT_00, // phys-frame shift-back error (недостаточно данных для работы -- пропускаем)
		eMPEGDStatus_Huffman    = MASK_BIT_01, // huffman table error (ошибка движения по таблице -- некорректные входные данные)
		eMPEGDStatus_Part23end  = MASK_BIT_02, // out of main data due huffman reading (вне рассчитанных границ чтения -- некорректные входные данные)
		eMPEGDStatus_nullData00 = MASK_BIT_03, // gr[0]ch[0] no main data (part23len == 0)
		eMPEGDStatus_nullData01 = MASK_BIT_04, // gr[0]ch[1] no main data (part23len == 0)
		eMPEGDStatus_nullData10 = MASK_BIT_05, // gr[1]ch[0] no main data (part23len == 0)
		eMPEGDStatus_nullData11 = MASK_BIT_06, // gr[1]ch[1] no main data (part23len == 0)
		eMPEGDStatus_XingLame   = MASK_BIT_07, // XING/LAME header
		eMPEGDStatus_Fraunhofer = MASK_BIT_08, // VBRI header

		eMPEGDStatus_nullData0g = eMPEGDStatus_nullData00 | eMPEGDStatus_nullData01,  // no main data in gr 0 ch 0,1
		eMPEGDStatus_nullData1g = eMPEGDStatus_nullData10 | eMPEGDStatus_nullData11,  // no main data in gr 1 ch 0,1
		eMPEGDStatus_nullData0c = eMPEGDStatus_nullData00 | eMPEGDStatus_nullData10,  // no main data in ch 0 gr 0,1
		eMPEGDStatus_nullData1c = eMPEGDStatus_nullData01 | eMPEGDStatus_nullData11,  // no main data in ch 1 gr 0,1  (обычно в JointStereo M/S stereo ON)
		eMPEGDStatus_nullData   = eMPEGDStatus_nullData00 | eMPEGDStatus_nullData01 | // no main data in all frame
		                          eMPEGDStatus_nullData10 | eMPEGDStatus_nullData11,

		eMPEGDStatus_EncoderDAT = eMPEGDStatus_XingLame | eMPEGDStatus_Fraunhofer // XING/LAME or VBRI header

	};

	//>> MPEG header XING
	struct XINGHeader
	{
		XINGHeader(const XINGHeader & src)            = delete;
		XINGHeader(XINGHeader && src)                 = delete;
		XINGHeader& operator=(const XINGHeader & src) = delete;
		XINGHeader& operator=(XINGHeader && src)      = delete;

		XINGHeader() { _Reset(); };
		~XINGHeader() {};

	protected:
		union XINGFL
		{
			uint32 _u32;
			struct
			{
				unsigned frames  : 1;
				unsigned bytes   : 1;
				unsigned TOC     : 1;
				unsigned quality : 1;
			};		
		};

	protected:
		uint32 total; // mem position

	public:
		struct
		{
			uint32 sign;	 // VBR 'Xing' (58 69 6E 67) / CBR 'Info' (49 6E 66 6F) 
			XINGFL flags;	 // 1|2|4|8 == Frames/Bytes/TOC/Quality indicator field is present
			uint32 frames;	 // num of frames, as BE big endian
			uint32 bytes;	 // len of file, as BE big endian
			byte   TOC[100]; // seeking table
			uint32 quality;  // 0 - best, 100 - worst, as BE big endian
		} XING;

	public:
		//>> Сброс в ноль
		virtual void _Reset()
		{
			XING.sign       = 0;
			XING.flags._u32 = 0;
			XING.frames     = 0;
			XING.bytes      = 0;
			XING.quality    = 0;
			ZeroMemory(XING.TOC, sizeof(XING.TOC));

			total = 0;
		}

		//>> Чтение XING заголовка (возвращает FALSE при EOF или если не XING)
		virtual bool _Read(byte * data, uint32 size_in_bytes)
		{
			if (size_in_bytes < 8) return false; // EOF

			_Reset();

			for (int i = 24; i >= 0; i -= 8) XING.sign       |= (data[total++] << i);

			if (XING.sign != 0x496e666f && // CBR 'Info' tag
				XING.sign != 0x58696e67)   // VBR/ABR 'Xing' tag
				return false;

			for (int i = 24; i >= 0; i -= 8) XING.flags._u32 |= (data[total++] << i);

			if (XING.flags.frames)  total += 4;
			if (XING.flags.bytes)   total += 4;
			if (XING.flags.TOC)     total += 100;
			if (XING.flags.quality) total += 4;

			if (size_in_bytes < total) return false; // EOF

			total = 8;

			if (XING.flags.frames)  for (int i = 24; i >= 0; i -= 8) XING.frames |= (data[total++] << i);
			if (XING.flags.bytes)   for (int i = 24; i >= 0; i -= 8) XING.bytes  |= (data[total++] << i);
			if (XING.flags.TOC)
			{
				memcpy(XING.TOC, data + total, sizeof(XING.TOC));
				total += 100;
			}
			if (XING.flags.quality) for (int i = 24; i >= 0; i -= 8) XING.quality |= (data[total++] << i);

			return true;
		}

	};

	//>> MPEG header XING-LAME
	struct LAMEHeader : public XINGHeader
	{
		LAMEHeader(const LAMEHeader & src)            = delete;
		LAMEHeader(LAMEHeader && src)                 = delete;
		LAMEHeader& operator=(const LAMEHeader & src) = delete;
		LAMEHeader& operator=(LAMEHeader && src)      = delete;

		LAMEHeader() : XINGHeader() {};
		~LAMEHeader() {};

	protected:
		union LAMEFL
		{
			uint8 _u8;
			struct
			{
				unsigned vbr_method : 4;
				unsigned revision   : 4; // info tag revision
			};
		};
		union LAMEGN
		{
			uint64 _u64;
			struct
			{
				uint16 audiophile_replay_gain;
				uint16 radio_replay_gain;
				float  peak_signal_amplitude;
			};
		};
		union LAMEFL2
		{
			uint8 _u8;
			struct
			{
				unsigned ATH_type        : 4;
				unsigned nspsytune       : 1;
				unsigned nssafejoint     : 1;
				unsigned nogap_c_next    : 1;
				unsigned nogap_c_earlier : 1;
			};
		};
		union LAMEMISC
		{
			uint32 _u32;
			struct
			{
				unsigned noise_shaping : 2;
				unsigned stereo_mode   : 3;
				unsigned unk           : 1;
				unsigned source_freq   : 2;
				unsigned delay_end     : 12; // 12 bit : samples padding at  end  of PCM   - MP3LAMEDECDELAY + 1
				unsigned delay_start   : 12; // 12 bit : samples padding at start of PCM   + MP3LAMEDECDELAY + 1
			};
		};

	public:
		struct {
			uint32   sign;				// 'LAME' (4C 41 4D 45)
			byte     ver[5];			// LAME version, for example, "3.12r"
			LAMEFL   flags;
			byte     lowpass_filter;
			LAMEGN   gain;
			LAMEFL2  flags2;
			byte     bitrate;			// minimal (CBR/VBR) or specified (ABR)
			LAMEMISC misc;
			byte     mp3_gain;
			uint16   surrinfo;			// preset and surround info
			uint32   time;				// music length
			uint16   crc16_data;		// CRC-16 of the complete mp3 music data
			uint16   crc16_header;		// CRC-16 of the first 190 (0xBD) bytes 
		} LAME; // sizeof(LAME) with padding = 56, with out = 36 = 4+5+1+1+8+1+1+4+1+2+4+2+2

		//>> Сброс в ноль
		void _Reset() override final
		{
			XINGHeader::_Reset();
			ZeroMemory(&LAME, sizeof(LAME));
		}

		//>> Чтение XING-LAME заголовка (возвращает FALSE при EOF или если не XING/LAME)
		bool _Read(byte * data, uint32 size_in_bytes) override final
		{
			_Reset();

			if (!XINGHeader::_Read(data, size_in_bytes)) return false; // EOF or not XING
			if (size_in_bytes < total + 36)              return false; // EOF

			for (int i = 24; i >= 0; i -= 8) LAME.sign         |= (data[total++] << i);

			if (LAME.sign != 0x4c414d45) // 'LAME' tag
				return false;

			memcpy(LAME.ver, data + total, sizeof(LAME.ver));
			total += sizeof(LAME.ver);

			LAME.flags._u8      = data[total++];
			LAME.lowpass_filter = data[total++];

			for (int i = 56; i >= 0; i -= 8) LAME.gain._u64    |= (data[total++] << i);

			LAME.flags2._u8     = data[total++];
			LAME.bitrate        = data[total++];

			for (int i = 24; i >= 0; i -= 8) LAME.misc._u32    |= (data[total++] << i);

			LAME.mp3_gain       = data[total++];

			for (int i =  8; i >= 0; i -= 8) LAME.surrinfo     |= (data[total++] << i);
			for (int i = 24; i >= 0; i -= 8) LAME.time         |= (data[total++] << i);
			for (int i =  8; i >= 0; i -= 8) LAME.crc16_data   |= (data[total++] << i);
			for (int i =  8; i >= 0; i -= 8) LAME.crc16_header |= (data[total++] << i);

			return true;
		}
	};

	//>> MPEG VBR header VBRI (Fraunhofer Encoder)
	struct VBRIHeader
	{
		VBRIHeader(const VBRIHeader & src)            = delete;
		VBRIHeader(VBRIHeader && src)                 = delete;
		VBRIHeader& operator=(const VBRIHeader & src) = delete;
		VBRIHeader& operator=(VBRIHeader && src)      = delete;

		VBRIHeader() { _Reset(); }
		~VBRIHeader() { }

		uint32 sign;      //  0 ..  4 : 'VBRI' (56 42 52 49)
		uint16 version;   //  4 ..  6 : int
		uint16 delay;	  //  6 ..  8 : float
		uint16 quality;   //  8 .. 10 : int
		uint32 bytes;     // 10 .. 14 : int : total size
		uint32 frames;    // 14 .. 18 : int : total frames
		uint16 entnum;    // 18 .. 20 : int : NUM of TOC entries
		uint16 entscale;  // 20 .. 22 : int : scale factor for entries NUM 
		uint16 entsize;	  // 22 .. 24 : int : size of TOC entry
		uint16 entframes; // 24 .. 26 : int : frames per one entry
		TBUFFER <byte, uint32> TOC; // toc size = (entnum * entscale) * entsize

		//>> Сброс в ноль
		void _Reset()
		{
			sign      = 0;
			version   = 0;
			delay     = 0;
			quality   = 0;
			bytes     = 0;
			frames    = 0;
			entnum    = 0;
			entscale  = 0;
			entsize   = 0;
			entframes = 0;
			TOC.Close();
		}

		//>> Чтение VBRI заголовка (возвращает FALSE при EOF или если не VBRI)
		bool _Read(byte * data, uint32 size_in_bytes)
		{
			if (size_in_bytes < MPG_VBRI_SIZE) // EOF
				return false;

			_Reset();

			int total = 0;
			for (int i = 24; i >= 0; i -= 8) sign      |= (data[total++] << i);

			if (sign != 0x56425249) // 'VBRI'
				return false;

			for (int i =  8; i >= 0; i -= 8) version   |= (data[total++] << i);
			for (int i =  8; i >= 0; i -= 8) delay     |= (data[total++] << i);
			for (int i =  8; i >= 0; i -= 8) quality   |= (data[total++] << i);
			for (int i = 24; i >= 0; i -= 8) bytes     |= (data[total++] << i);
			for (int i = 24; i >= 0; i -= 8) frames    |= (data[total++] << i);
			for (int i =  8; i >= 0; i -= 8) entnum    |= (data[total++] << i);
			for (int i =  8; i >= 0; i -= 8) entscale  |= (data[total++] << i);
			for (int i =  8; i >= 0; i -= 8) entsize   |= (data[total++] << i);
			for (int i =  8; i >= 0; i -= 8) entframes |= (data[total++] << i);

			uint32 toc_size = (entnum * entscale) * entsize;

			if (size_in_bytes < MPG_VBRI_SIZE + toc_size) // EOF
				return false;

			TOC.Create(toc_size);

			memcpy(TOC.buf, data + MPG_VBRI_SIZE, toc_size);

			return true;
		}
	};

	//>> Основные данные кадра MPEG [только для MPEG-1/2/2.5 Layer-3]
	struct MPEGMainData
	{
	private:
		// main_data_begin = 9 bit = 0x1FF max offset
		// MPEG-1 Layer-3 min. pure frame size is 66 (0x42) --> mem up to 8 phys.frames [MP3JUMPS = 16] (0x42 * 8 = 0x210 > 0x1FF)
		// MPEG-1 Layer-3 max. pure frame size is 1402 (0x57A)

	//	byte * jump[4];  // 2-frames jumper : 0 pre-prev end, 1 prev start, 2 prev end, 3 cur start
		byte * jump[MP3JUMPS];

	public:
		LAMEHeader lame;                                 // valid if (STATUS & eMPEGDStatus_XingLame)
		VBRIHeader vbri;                                 // valid if (STATUS & eMPEGDStatus_Fraunhofer)
		byte   scalefac_l [2][2][MP3SFB_L];              // [gr][ch][sfb]         :: 0-4 bits per obj
		byte   scalefac_s [2][2][MP3SFB_S][MP3SFB_SWIN]; // [gr][ch][sfb][window] :: 0-4 bits per obj
		float  sample     [2][2][MP3GRS];                // [gr][ch][line] 32 subband blocks * 18 frequency line	
		// + ancillary_bits;

		//uint32 offset; // debug info

		MPEGMainData(const MPEGMainData & src)            = delete;
		MPEGMainData(MPEGMainData && src)                 = delete;
		MPEGMainData& operator=(const MPEGMainData & src) = delete;
		MPEGMainData& operator=(MPEGMainData && src)      = delete;

		MPEGMainData() { _Reset(); };
		~MPEGMainData() { };

		//>> Сброс в ноль
		void _Reset()
		{
			ZeroMemory(jump,       sizeof(jump));
			ZeroMemory(scalefac_l, sizeof(scalefac_l));
			ZeroMemory(scalefac_s, sizeof(scalefac_s));
			ZeroMemory(sample,     sizeof(sample));
			lame._Reset();
		}

		//>> Сброс в ноль (только самое необходимое)
		void _Reset_fast()
		{
			ZeroMemory(jump, sizeof(jump));
		}

		//>> Чтение MPEG заголовка (пока только MPEG-1) :: возвращает eMPEGDStatus
		error_t _Read(byte *& data, byte & position_bit, const MPEGHeader * mpeg, MPEGHeaderSide * side)
		{
			static const byte scalefac_len [16][2] = { // {scalefac_compress} -> { slen1, slen2 }
					{ 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, 
					{ 3, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 },
					{ 2, 1 }, { 2, 2 }, { 2, 3 }, { 3, 1 },
					{ 3, 2 }, { 3, 3 }, { 4, 2 }, { 4, 3 }
			};

			// Block types : [0] = reserved, [1] = start block, [2] = 3 short windows, [3] = end block
			// ...
			// Normal blocks (block_type != 2)                            : slen1 [0..10], slen2 [11..20]
			// Short  blocks (block_type == 2) && (mixed_block_flag == 1) : slen1 [0..5],  slen2 [6..11]
			// Short  blocks (block_type == 2) && (mixed_block_flag == 0) : 
			//   slen1 [0..7] (long window scalefactor band) or [3..5] (short window scalefactor band)
			//   slen2 [6..11]

			bool mpeg_1   = (mpeg->mpeg == eMPEGType1);
			bool mono     = (mpeg->ch == eMPEGChSingle);
			byte channels = mono ? 1 : 2;
			byte granules = mpeg_1 ? eMPEG_1_Granules : eMPEG_2_Granules;
			byte sfb_max  = mpeg_1 ? 8 : 6; // 8 MPEG-1, 6 MPEG-2/2.5

			error_t STATUS = eMPEGDStatus_Normal;

			MPEGHeaderSide & _ = *side;

			// position check
			if (position_bit > 7)
			{
				data += position_bit >> 3;
				position_bit %= 8;
			}

			// nullData check
			for (byte gr = 0; gr < granules; gr++)
			for (byte ch = 0; ch < channels; ch++)
			if (_.part23len[gr][ch] == 0)
			{
				if (gr == 0)											{
					if (ch == 0) STATUS |= eMPEGDStatus_nullData00;
					else         STATUS |= eMPEGDStatus_nullData01;		}
				else													{
					if (ch == 0) STATUS |= eMPEGDStatus_nullData10;
					else         STATUS |= eMPEGDStatus_nullData11;		}
			}

			// eMPEGDStatus_nullData - may be it's XING-LAME or VBRI header - try to read it
			if ((STATUS & eMPEGDStatus_nullData) == eMPEGDStatus_nullData)
			{
				if (lame._Read(data, mpeg->FRAME.pure))
					STATUS |= eMPEGDStatus_XingLame;      else
				if (vbri._Read(data, mpeg->FRAME.pure))
					STATUS |= eMPEGDStatus_Fraunhofer;
			}

			////////////////////////////////
			// MEM PHYSICAL FRAME BORDERS //
			////////////////////////////////

			if ((STATUS & eMPEGDStatus_EncoderDAT) == 0) // not an encoder infodata
			{
				for (uint32 i=1; i<MP3JUMPS; i++)
					jump[i-1] = jump[i];

				jump[MP3JUMPS-1] = data;
			}

			//auto data_mem = data; // debug info

			/////////////////////////////
			// SHIFT TO START POSITION //
			/////////////////////////////

			if (_.main_data_begin) // max 0x1FF (9 bits)
			{
				uint16 pure_sum = 0;
				for (uint16 i=MP3JUMPS-2; ;i-=2)
				{
					if (i != 0)
					{
						uint16 pure = (uint16) (jump[i] - jump[i-1]); // phys. frame pure size : end (i) - start (i-1)
						if (_.main_data_begin > pure_sum + pure)
						{
							pure_sum += pure;
							continue;
						}
					}

					if (!jump[i]) STATUS |= eMPEGDStatus_PhysFrame;
					else data = jump[i] - (_.main_data_begin - pure_sum);

					break;
				}
			} // uint32 offset_shift = offset - (data_mem - data); // debug info

			if (!(STATUS & eMPEGDStatus_PhysFrame)) // skip if can't read
			for (byte gr = 0; gr < granules; gr++)
			for (byte ch = 0; ch < channels; ch++)
			{
				//////////////////////
				// CALC END BORDERS //
				//////////////////////

				byte * part23end;
				byte   part23endbit;

				{
					uint32 part23end_fix_bits = _.part23len[gr][ch];

					for (uint32 i=0; i<MP3JUMPS; i+=2)
					{
						if ( data < jump[i] &&
							(data + ((part23end_fix_bits + position_bit) >> 3)) >= jump[i])
						part23end_fix_bits += ((uint32)(jump[i+1] - jump[i])) << 3;
					}

					part23end    = data         + (part23end_fix_bits >> 3); // ptr to part23 end, in file's buffer
					part23endbit = position_bit + (part23end_fix_bits %  8); // part23 ending bit

					if (part23endbit > 7)
					{
						part23end++;
						part23endbit %= 8;
					}

					//uint32 offset_shift_start = offset - (data_mem - data);             // debug info
					//uint32 offset_shift_end  = offset_shift_start + (part23end - data); // debug info
				}

				/////////////////////////////
				// READ SCALE FACTORS BITS //
				/////////////////////////////

				uint16 nslen = _.scalefac_compress[gr][ch];
				byte slen1 = scalefac_len [nslen] [0];
				byte slen2 = scalefac_len [nslen] [1];

				//bool PRETAB = !mpeg_1 && _.preflag[gr][ch]; // MPEG-2/2.5 only + preflag

				if (_.switch_flag[gr][ch] && _.block_type[gr][ch] == eMPEGBlockT_3ShortWindow)
				{
					byte sfb_min;
					auto sl = scalefac_l [gr][ch];
					auto ss = scalefac_s [gr][ch];

					if (_.mixed_block_flag[gr][ch]) // MIXED BLOCK L+S : long window scalefactor bands
					{
						for (byte sfb = 0; sfb < sfb_max; sfb++)
							sl [sfb] = (byte) ReadBits <MP3JUMPS> (data, position_bit, slen1, jump);
							           //+ ((PRETAB) ? scalefac_pretab[sfb] : 0);

						   sfb_min = 3;	
					} else sfb_min = 0;

					for (byte sfb = sfb_min; sfb < MP3SFB_S; sfb++) // short window scalefactor bands
					{
						// sfb_slen_switch == (6) == (12 / 2) == (MP3SFB_S / 2)

						byte slen = (sfb < 6) ? slen1 : slen2;
						auto sssfb = ss [sfb];

						for (byte window = 0; window < MP3SFB_SWIN; window++)
							sssfb [window] = (byte) ReadBits <MP3JUMPS> (data, position_bit, slen, jump);
							                   //+ ((PRETAB) ? scalefac_pretab[sfb] : 0);
					}
				}
				else // switch_flag == false, block_type == 0
				{
					auto sl  = scalefac_l [gr][ch];
					
					if (mpeg_1)
					{
						auto sl0 = scalefac_l [0][ch];
						auto sl1 = scalefac_l [1][ch];

						// Scale factor bands [0..5]

						if ((_.scfsi[ch][0] == 0) || (gr == 0)) // own
							for (byte sfb = 0; sfb < 6; sfb++)
								sl  [sfb] = (byte) ReadBits <MP3JUMPS> (data, position_bit, slen1, jump);
						else
						if ((_.scfsi[ch][0] == 1) && (gr == 1)) // copy
							for (byte sfb = 0; sfb < 6; sfb++)
								sl1 [sfb] = sl0 [sfb];

						// Scale factor bands [6..10]

						if ((_.scfsi[ch][1] == 0) || (gr == 0)) // own
							for (byte sfb = 6; sfb < 11; sfb++)
								sl  [sfb] = (byte) ReadBits <MP3JUMPS> (data, position_bit, slen1, jump);
						else
						if ((_.scfsi[ch][1] == 1) && (gr == 1)) // copy
							for (byte sfb = 6; sfb < 11; sfb++)
								sl1 [sfb] = sl0 [sfb];

						// Scale factor bands [11..15]

						if ((_.scfsi[ch][2] == 0) || (gr == 0)) // own
							for (byte sfb = 11; sfb < 16; sfb++)
								sl  [sfb] = (byte) ReadBits <MP3JUMPS> (data, position_bit, slen2, jump);
						else
						if ((_.scfsi[ch][2] == 1) && (gr == 1)) // copy
							for (byte sfb = 11; sfb < 16; sfb++)
								sl1 [sfb] = sl0 [sfb];

						// Scale factor bands [16..20]

						if ((_.scfsi[ch][3] == 0) || (gr == 0)) // own
							for (byte sfb = 16; sfb < MP3SFB_L; sfb++)
								sl  [sfb] = (byte) ReadBits <MP3JUMPS> (data, position_bit, slen2, jump);
						else
						if ((_.scfsi[ch][3] == 1) && (gr == 1)) // copy
							for (byte sfb = 16; sfb < MP3SFB_L; sfb++)
								sl1 [sfb] = sl0 [sfb];

					}
					else // MPEG-2/2.5
					{
						for (byte sfb = 0; sfb < MP3SFB_L; sfb++)
						{
							// sfb_slen_switch == (11) ~ (21 / 2) == (MP3SFB_L / 2)

							byte slen = (sfb < 11) ? slen1 : slen2; // TODO ? 11 ?  проверить!

							sl [sfb] = (byte) ReadBits <MP3JUMPS> (data, position_bit, slen, jump);
								       //+ ((PRETAB) ? scalefac_pretab[sfb] : 0);
						}						
					}
				}

				///////////////////////
				// READ HUFFMAN BITS //
				///////////////////////
				
			//	const uint16 region_0_start = 0;
				      uint16 region_1_start, region_2_start;

				float * _sample = sample[gr][ch];

				if (_.part23len[gr][ch] == 0) // no data
				{
					ZeroMemory(_sample, 4 * MP3GRS);
				}
				else // part23len is not NULL
				{
					if (_.switch_flag[gr][ch] && _.block_type[gr][ch] == eMPEGBlockT_3ShortWindow)
					{
						region_1_start = sfb_indice[mpeg->freq].l[MP3SFB_ML]; // 36
						region_2_start = MP3GRS;                              // no region 2
					}
					else
					{
						auto & indice = sfb_indice[mpeg->freq].l;

						region_1_start = indice[_.region0_count[gr][ch] + 1];
						region_2_start = indice[_.region0_count[gr][ch] +
							                    _.region1_count[gr][ch] + 2];
					}

					uint32 i;
					 int32 x, y, v, w;
					  byte table;

					// Reading big values
					
					uint32 ibv = _.big_values[gr][ch] * 2;

					for (i = 0; i < ibv; )
					{
						     if (i < region_1_start) table = _.table_select[gr][ch][0];
						else if (i < region_2_start) table = _.table_select[gr][ch][1];
						else                         table = _.table_select[gr][ch][2];
						
						if (!DecodeHuffman(data, position_bit, table, x, y, v, w))
							return STATUS |= eMPEGDStatus_Huffman;

						_sample [i++] = (float) x;
						_sample [i++] = (float) y;
					}

					// Reading small values

					table = _.count1table_select[gr][ch] + 32; // ??? 32 subband blocks ???

					const uint32 isv = MP3GRS - 4;

					for (i = ibv; (i <= isv) && (data < part23end || (data == part23end && position_bit < part23endbit)) ;)
					{
						if (!DecodeHuffman(data, position_bit, table, x, y, v, w))
							return STATUS |= eMPEGDStatus_Huffman;

						_sample [i++] = (float) v;
						_sample [i++] = (float) w;
						_sample [i++] = (float) x;
						_sample [i++] = (float) y;
					}

					// error
					if ((data > part23end) || (data == part23end && position_bit > part23endbit))
					{
						//uint32 offset_shift_end   = offset + (part23end - data_mem); // debug info
						//uint32 offset_shift_error = offset + (data      - data_mem); // debug info
						STATUS |= eMPEGDStatus_Part23end;
					}				

					_.count1[gr][ch] = i;

					for ( ; i < MP3GRS; i++)
						_sample [i] = 0;

					/////////////////////////
					// SKIP ANCILLARY BITS //
					/////////////////////////

					// ...

					// move next
					data         = part23end;
					position_bit = part23endbit;
				}
			}

			////////////////////////////////
			// MEM PHYSICAL FRAME BORDERS //
			////////////////////////////////

			if ((STATUS & eMPEGDStatus_EncoderDAT) == 0) // not an encoder infodata
			{
				for (byte i=1; i<MP3JUMPS; i++)
					jump[i-1] = jump[i];

				jump[MP3JUMPS-1] = jump[MP3JUMPS-2] + mpeg->FRAME.pure;
			}

			return STATUS;
		}
	
	private:
		//>> Декодирует код Хаффмана по таблицам MP3 (var.1)
		bool DecodeHuffman1(byte *& data, byte & position_bit, byte table_id, int32 & x, int32 & y, int32 & v, int32 & w)
		{
			UNION32 * table   = (UNION32*) mp3hf[table_id][MP3HF_TABPTR];
			uint16    treelen = (uint16)   mp3hf[table_id][MP3HF_TREELEN]; // max 512
			uint16    linbits = (uint16)   mp3hf[table_id][MP3HF_LINBITS]; // max 13
			uint16    tree_p  = 0;
			 int8     bits_p  = 31;
			 bool     ret     = false;

			if (!treelen) { x = y = v = w = 0; return true; }

			uint32 bitpool = (uint32) ReadBits <MP3JUMPS> (data, position_bit, 32, jump);

			while (tree_p < treelen && bits_p >= 0)
			{
				if (!table[tree_p].u16_2) // destination : u32 & 0xFFFF0000
				{
					x = table[tree_p].u4_2; // (u32 >> 4) & 0xF
					y = table[tree_p].u4_1; //  u32       & 0xF
					
					// возврат каретки на количество незадействованных бит
					for (int8 bits_back = bits_p + 1;;)
					{
						if (bits_back > position_bit)
						{
							bits_back   -= position_bit + 1;
							position_bit = 7;
							--data;
						}
						else
						{
							position_bit -= bits_back;
							break;
						}

						for (int32 jp = MP3JUMPS - 1; jp > 0; jp -= 2) // ..., 3-2, 1-0  обратный прыжок
						if (data == jump[jp]-1)
						{
							data =  jump[jp-1]-1;
							break;
						}
					}

					ret = true;
					break;
				}

				//if (ReadBits<bool, MP3JUMPS>(data, position_bit, 1, jump)) // go right in tree
				if (bitpool & (1ul << bits_p)) // go right in tree
				{
					while (table[tree_p].u8_1 >= 0xFA) // (u32 & 0xFF) >= 250
					tree_p += table[tree_p].u8_1; // += u32 & 0xFF
					tree_p += table[tree_p].u8_1; // += u32 & 0xFF
				}
				else // go left in tree
				{
					while (table[tree_p].u8_3 >= 0xFA) // ((u32 >> 16) & 0xFF) >= 250
					tree_p += table[tree_p].u8_3; // += (u32 >> 16) & 0xFF
					tree_p += table[tree_p].u8_3; // += (u32 >> 16) & 0xFF
				}

				bits_p--;
			}

			if (!ret) 
			{
				printf("\nDEBUG: check huffman table error!"); getchar();
				return ret; // ERROR in mp3 file
			}

			if (table_id > 31) // small values
			{
				v = (y >> 3) & 1; // table[tree_p].u1_1_4
				w = (y >> 2) & 1; // table[tree_p].u1_1_3
				x = (y >> 1) & 1; // table[tree_p].u1_1_2
				y =  y       & 1; // table[tree_p].u1_1_1

				//if (v) if (ReadBits<bool>(data, position_bit, 1, jump)) v = -v;
				//if (w) if (ReadBits<bool>(data, position_bit, 1, jump)) w = -w;
				//if (x) if (ReadBits<bool>(data, position_bit, 1, jump)) x = -x;
				//if (y) if (ReadBits<bool>(data, position_bit, 1, jump)) y = -y;

				byte readbits = x + y + v + w;
				byte bitpool  = (byte) ReadBits <MP3JUMPS> (data, position_bit, readbits, jump);

				if (v) if (bitpool & (1ul << (readbits-- - 1))) v = -v;
				if (w) if (bitpool & (1ul << (readbits-- - 1))) w = -w;
				if (x) if (bitpool & (1ul << (readbits-- - 1))) x = -x;
				if (y) if (bitpool & (1ul << (readbits   - 1))) y = -y;
			}
			else // big values
			{
				//if (linbits && x == 0xF)
				//	x += ReadBits<uint32>(data, position_bit, linbits, jump);
				//
				//if (x) if (ReadBits<bool>(data, position_bit, 1, jump)) x = -x;
				//
				//if (linbits && y == 0xF)
				//	y += ReadBits<uint32>(data, position_bit, linbits, jump);
				//
				//if (y) if (ReadBits<bool>(data, position_bit, 1, jump)) y = -y;

				  byte readbits = 0;
				uint32 bitpool  = (uint32) ReadBits <MP3JUMPS> (data, position_bit, 32, jump);

				if (linbits && x == 0xF)
				{
					x += (bitpool << readbits) >> (32 - linbits);
					readbits += linbits;
				}
				if (x)
				{
					if ((bitpool << readbits) & MASK_BIT_31) x = -x;
					readbits += 1;
				}
				if (linbits && y == 0xF)
				{
					y += (bitpool << readbits) >> (32 - linbits);
					readbits += linbits;
				}
				if (y)
				{
					if ((bitpool << readbits) & MASK_BIT_31) y = -y;
					readbits += 1;
				}

				// возврат каретки на количество незадействованных бит
				for (int8 bits_back = 32 - readbits;;)
				{
					if (bits_back > position_bit)
					{
						bits_back -= position_bit + 1;
						position_bit = 7;
						--data;
					}
					else
					{
						position_bit -= bits_back;
						break;
					}

					for (int32 jp = MP3JUMPS - 1; jp > 0; jp -= 2) // ..., 3-2, 1-0  обратный прыжок
					if (data == jump[jp]-1)
					{
						data =  jump[jp-1]-1;
						break;
					}
				}
			}

			return true;
		}

		//>> Декодирует код Хаффмана по таблицам MP3 (var.2)
		bool DecodeHuffman(byte *& data, byte & position_bit, byte table_id, int32 & x, int32 & y, int32 & v, int32 & w)
		{
			UNION32 * table   = (UNION32*) mp3hf[table_id][MP3HF_TABPTR];
			uint16    treelen = (uint16)   mp3hf[table_id][MP3HF_TREELEN]; // max 512
			uint8     linbits = (uint8)    mp3hf[table_id][MP3HF_LINBITS]; // max 13
			uint16    tree_p  = 0;
			 int8     bits_p  = 31;
			 bool     ret     = false;

			byte   bits_to_read = 1;
			uint32 bits_out;

			if (!treelen) { x = y = v = w = 0; return true; }		

#pragma region <ReadBits_J>

			#define _ReadBits_J {                                             \
					uint32 RET_VALUE = 0;                                     \
					byte   BITS_DONE = 0;                                     \
					byte   BITS_FORWARD = 8 - position_bit;                   \
					                                                          \
					while (BITS_DONE < bits_to_read)                          \
					{                                                         \
						byte BITS_NOT_DONE = bits_to_read - BITS_DONE;        \
						byte BITS_ROUND;                                      \
						                                                      \
						if (BITS_FORWARD > BITS_NOT_DONE)                     \
							BITS_ROUND = BITS_NOT_DONE;                       \
						else                                                  \
							BITS_ROUND = BITS_FORWARD;                        \
							                                                  \
						RET_VALUE <<= BITS_ROUND;                             \
						RET_VALUE |= ((*data) >> (BITS_FORWARD - BITS_ROUND)) & ((1 << BITS_ROUND) - 1); \
						                                                      \
						BITS_FORWARD -= BITS_ROUND;                           \
						BITS_DONE    += BITS_ROUND;                           \
						position_bit += BITS_ROUND;                           \
						                                                      \
						if (BITS_FORWARD == 0)		{                         \
							BITS_FORWARD = 8;                                 \
							position_bit = 0;                                 \
							data++;                                           \
							                                                  \
							for (uint32 jp=0; jp<MP3JUMPS; jp+=2)             \
							if (data == jump[jp])                             \
							{                                                 \
								data =  jump[jp+1];                           \
								break;                                        \
							}                                                 \
                                                    }                         \
					}                                                         \
					                                                          \
					bits_out = RET_VALUE;                                     \
			}

#pragma endregion

			while (tree_p < treelen && bits_p >= 0)
			{
				if (!table[tree_p].u16_2) // destination : u32 & 0xFFFF0000
				{
					x = table[tree_p].u4_2; // (u32 >> 4) & 0xF
					y = table[tree_p].u4_1; //  u32       & 0xF
					ret = true;
					break;
				}

				_ReadBits_J;

				if (bits_out)
				//if (ReadBits<bool>(data, position_bit, 1, jump)) // go right in tree
				//if (bitpool & (1ul << bits_p)) // go right in tree
				{
					while (table[tree_p].u8_1 >= 0xFA) // (u32 & 0xFF) >= 250
					tree_p += table[tree_p].u8_1; // += u32 & 0xFF
					tree_p += table[tree_p].u8_1; // += u32 & 0xFF
				}
				else // go left in tree
				{
					while (table[tree_p].u8_3 >= 0xFA) // ((u32 >> 16) & 0xFF) >= 250
					tree_p += table[tree_p].u8_3; // += (u32 >> 16) & 0xFF
					tree_p += table[tree_p].u8_3; // += (u32 >> 16) & 0xFF
				}

				bits_p--;
			}

			if (!ret) return ret; // ERROR in mp3 file

			if (table_id > 31) // small values
			{
				v = (y >> 3) & 1; // table[tree_p].u1_1_4
				w = (y >> 2) & 1; // table[tree_p].u1_1_3
				x = (y >> 1) & 1; // table[tree_p].u1_1_2
				y =  y       & 1; // table[tree_p].u1_1_1

				if (v) { _ReadBits_J; if (bits_out) v = -v; }
				if (w) { _ReadBits_J; if (bits_out) w = -w; }
				if (x) { _ReadBits_J; if (bits_out) x = -x; }
				if (y) { _ReadBits_J; if (bits_out) y = -y; }

				//if (v) if (ReadBits<bool>(data, position_bit, 1, jump)) v = -v;
				//if (w) if (ReadBits<bool>(data, position_bit, 1, jump)) w = -w;
				//if (x) if (ReadBits<bool>(data, position_bit, 1, jump)) x = -x;
				//if (y) if (ReadBits<bool>(data, position_bit, 1, jump)) y = -y;

				//byte readbits = x + y + v + w;
				//byte bitpool = ReadBits<byte>(data, position_bit, readbits, jump);
				//
				//if (v) if (bitpool & (1ul << (readbits-- - 1))) v = -v;
				//if (w) if (bitpool & (1ul << (readbits-- - 1))) w = -w;
				//if (x) if (bitpool & (1ul << (readbits-- - 1))) x = -x;
				//if (y) if (bitpool & (1ul << (readbits   - 1))) y = -y;
			}
			else // big values
			{
				if (linbits && x == 0xF) { bits_to_read = linbits; _ReadBits_J;               x += bits_out; }
				if (x)                   { bits_to_read = 1;       _ReadBits_J; if (bits_out) x  = -x;       }
				if (linbits && y == 0xF) { bits_to_read = linbits; _ReadBits_J;               y += bits_out; }
				if (y)                   { bits_to_read = 1;       _ReadBits_J; if (bits_out) y  = -y;       }

				//if (linbits && x == 0xF)
				//	x += ReadBits<uint32>(data, position_bit, linbits, jump);
				//
				//if (x) if (ReadBits<bool>(data, position_bit, 1, jump)) x = -x;
				//
				//if (linbits && y == 0xF)
				//	y += ReadBits<uint32>(data, position_bit, linbits, jump);
				//
				//if (y) if (ReadBits<bool>(data, position_bit, 1, jump)) y = -y;

				//byte readbits = 0;
				//uint32 bitpool = ReadBits<uint32>(data, position_bit, 32, jump);
				//
				//if (linbits && x == 0xF)
				//{
				//	x += (bitpool << readbits) >> (32 - linbits);
				//	readbits += linbits;
				//}
				//if (x)
				//{
				//	if ((bitpool << readbits) & MASK_BIT_31) x = -x;
				//	readbits += 1;
				//}
				//if (linbits && y == 0xF)
				//{
				//	y += (bitpool << readbits) >> (32 - linbits);
				//	readbits += linbits;
				//}
				//if (y)
				//{
				//	if ((bitpool << readbits) & MASK_BIT_31) y = -y;
				//	readbits += 1;
				//}
				//
				//// возврат каретки на количество незадействованных бит
				//for (int8 bits_back = 32 - readbits;;)
				//{
				//	if (bits_back > position_bit)
				//	{
				//		bits_back -= position_bit + 1;
				//		position_bit = 7;
				//		--data;
				//	}
				//	else
				//	{
				//		position_bit -= bits_back;
				//		break;
				//	}
				//
				//	if (data == jump[3] - 1) // обратный прыжок с физ.кадра 2 (тек) в физ.кадр 1 (пред)
				//		data = jump[2] - 1;
				//	else
				//		if (data == jump[1] - 1) // обратный прыжок с физ.кадра 1 (пред) в физ.кадр 0 (пре-пред)
				//			data = jump[0] - 1;
				//}
			}

			return true;

			#undef _ReadBits_J
		}

	};

}

#endif // _MP3H_H