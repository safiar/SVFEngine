// ----------------------------------------------------------------------- //
//
// MODULE  : WAVE.h
//
// PURPOSE : Структуры и описания звука
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _WAVE_H
#define _WAVE_H

#include "link_defines.h"

// MMReg.h
#ifdef WINDOWS_IMM_WASAPI
	#define WAVE_FORMAT_UNKNOWN 0x0000 /* Microsoft Corporation */
	#define WAVE_FORMAT_PCM 0x0001 /* Microsoft Corporation */
	#define WAVE_FORMAT_ADPCM 0x0002 /* Microsoft Corporation */
	#define WAVE_FORMAT_IEEE_FLOAT 0x0003 /* Microsoft Corporation */
	#define WAVE_FORMAT_VSELP 0x0004 /* Compaq Computer Corp. */
	#define WAVE_FORMAT_IBM_CVSD 0x0005 /* IBM Corporation */
	#define WAVE_FORMAT_ALAW 0x0006 /* Microsoft Corporation */
	#define WAVE_FORMAT_MULAW 0x0007 /* Microsoft Corporation */
	#define WAVE_FORMAT_DTS 0x0008 /* Microsoft Corporation */
	#define WAVE_FORMAT_DRM 0x0009 /* Microsoft Corporation */
	#define WAVE_FORMAT_OKI_ADPCM 0x0010 /* OKI */
	#define WAVE_FORMAT_DVI_ADPCM 0x0011 /* Intel Corporation */
	#define WAVE_FORMAT_IMA_ADPCM (WAVE_FORMAT_DVI_ADPCM) /* Intel Corporation */
	#define WAVE_FORMAT_MEDIASPACE_ADPCM 0x0012 /* Videologic */
	#define WAVE_FORMAT_SIERRA_ADPCM 0x0013 /* Sierra Semiconductor Corp */
	#define WAVE_FORMAT_G723_ADPCM 0x0014 /* Antex Electronics Corporation */
	#define WAVE_FORMAT_DIGISTD 0x0015 /* DSP Solutions, Inc. */
	#define WAVE_FORMAT_DIGIFIX 0x0016 /* DSP Solutions, Inc. */
	#define WAVE_FORMAT_DIALOGIC_OKI_ADPCM 0x0017 /* Dialogic Corporation */
	#define WAVE_FORMAT_MEDIAVISION_ADPCM 0x0018 /* Media Vision, Inc. */
	#define WAVE_FORMAT_CU_CODEC 0x0019 /* Hewlett-Packard Company */
	#define WAVE_FORMAT_YAMAHA_ADPCM 0x0020 /* Yamaha Corporation of America */
	#define WAVE_FORMAT_SONARC 0x0021 /* Speech Compression */
	#define WAVE_FORMAT_DSPGROUP_TRUESPEECH 0x0022 /* DSP Group, Inc */
	#define WAVE_FORMAT_ECHOSC1 0x0023 /* Echo Speech Corporation */
	#define WAVE_FORMAT_AUDIOFILE_AF36 0x0024 /* Virtual Music, Inc. */
	#define WAVE_FORMAT_APTX 0x0025 /* Audio Processing Technology */
	#define WAVE_FORMAT_AUDIOFILE_AF10 0x0026 /* Virtual Music, Inc. */
	#define WAVE_FORMAT_PROSODY_1612 0x0027 /* Aculab plc */
	#define WAVE_FORMAT_LRC 0x0028 /* Merging Technologies S.A. */
	#define WAVE_FORMAT_DOLBY_AC2 0x0030 /* Dolby Laboratories */
	#define WAVE_FORMAT_GSM610 0x0031 /* Microsoft Corporation */
	#define WAVE_FORMAT_MSNAUDIO 0x0032 /* Microsoft Corporation */
	#define WAVE_FORMAT_ANTEX_ADPCME 0x0033 /* Antex Electronics Corporation */
	#define WAVE_FORMAT_CONTROL_RES_VQLPC 0x0034 /* Control Resources Limited */
	#define WAVE_FORMAT_DIGIREAL 0x0035 /* DSP Solutions, Inc. */
	#define WAVE_FORMAT_DIGIADPCM 0x0036 /* DSP Solutions, Inc. */
	#define WAVE_FORMAT_CONTROL_RES_CR10 0x0037 /* Control Resources Limited */
	#define WAVE_FORMAT_NMS_VBXADPCM 0x0038 /* Natural MicroSystems */
	#define WAVE_FORMAT_CS_IMAADPCM 0x0039 /* Crystal Semiconductor IMA ADPCM */
	#define WAVE_FORMAT_ECHOSC3 0x003A /* Echo Speech Corporation */
	#define WAVE_FORMAT_ROCKWELL_ADPCM 0x003B /* Rockwell International */
	#define WAVE_FORMAT_ROCKWELL_DIGITALK 0x003C /* Rockwell International */
	#define WAVE_FORMAT_XEBEC 0x003D /* Xebec Multimedia Solutions Limited */
	#define WAVE_FORMAT_G721_ADPCM 0x0040 /* Antex Electronics Corporation */
	#define WAVE_FORMAT_G728_CELP 0x0041 /* Antex Electronics Corporation */
	#define WAVE_FORMAT_MSG723 0x0042 /* Microsoft Corporation */
	#define WAVE_FORMAT_MPEG 0x0050 /* Microsoft Corporation */
	#define WAVE_FORMAT_RT24 0x0052 /* InSoft, Inc. */
	#define WAVE_FORMAT_PAC 0x0053 /* InSoft, Inc. */
	#define WAVE_FORMAT_MPEGLAYER3 0x0055 /* ISO/MPEG Layer3 Format Tag */
	#define WAVE_FORMAT_LUCENT_G723 0x0059 /* Lucent Technologies */
	#define WAVE_FORMAT_CIRRUS 0x0060 /* Cirrus Logic */
	#define WAVE_FORMAT_ESPCM 0x0061 /* ESS Technology */
	#define WAVE_FORMAT_VOXWARE 0x0062 /* Voxware Inc */
	#define WAVE_FORMAT_CANOPUS_ATRAC 0x0063 /* Canopus, co., Ltd. */
	#define WAVE_FORMAT_G726_ADPCM 0x0064 /* APICOM */
	#define WAVE_FORMAT_G722_ADPCM 0x0065 /* APICOM */
	#define WAVE_FORMAT_DSAT_DISPLAY 0x0067 /* Microsoft Corporation */
	#define WAVE_FORMAT_VOXWARE_BYTE_ALIGNED 0x0069 /* Voxware Inc */
	#define WAVE_FORMAT_VOXWARE_AC8 0x0070 /* Voxware Inc */
	#define WAVE_FORMAT_VOXWARE_AC10 0x0071 /* Voxware Inc */
	#define WAVE_FORMAT_VOXWARE_AC16 0x0072 /* Voxware Inc */
	#define WAVE_FORMAT_VOXWARE_AC20 0x0073 /* Voxware Inc */
	#define WAVE_FORMAT_VOXWARE_RT24 0x0074 /* Voxware Inc */
	#define WAVE_FORMAT_VOXWARE_RT29 0x0075 /* Voxware Inc */
	#define WAVE_FORMAT_VOXWARE_RT29HW 0x0076 /* Voxware Inc */
	#define WAVE_FORMAT_VOXWARE_VR12 0x0077 /* Voxware Inc */
	#define WAVE_FORMAT_VOXWARE_VR18 0x0078 /* Voxware Inc */
	#define WAVE_FORMAT_VOXWARE_TQ40 0x0079 /* Voxware Inc */
	#define WAVE_FORMAT_SOFTSOUND 0x0080 /* Softsound, Ltd. */
	#define WAVE_FORMAT_VOXWARE_TQ60 0x0081 /* Voxware Inc */
	#define WAVE_FORMAT_MSRT24 0x0082 /* Microsoft Corporation */
	#define WAVE_FORMAT_G729A 0x0083 /* AT&amp;T Labs, Inc. */
	#define WAVE_FORMAT_MVI_MVI2 0x0084 /* Motion Pixels */
	#define WAVE_FORMAT_DF_G726 0x0085 /* DataFusion Systems (Pty) (Ltd) */
	#define WAVE_FORMAT_DF_GSM610 0x0086 /* DataFusion Systems (Pty) (Ltd) */
	#define WAVE_FORMAT_ISIAUDIO 0x0088 /* Iterated Systems, Inc. */
	#define WAVE_FORMAT_ONLIVE 0x0089 /* OnLive! Technologies, Inc. */
	#define WAVE_FORMAT_SBC24 0x0091 /* Siemens Business Communications Sys */
	#define WAVE_FORMAT_DOLBY_AC3_SPDIF 0x0092 /* Sonic Foundry */
	#define WAVE_FORMAT_MEDIASONIC_G723 0x0093 /* MediaSonic */
	#define WAVE_FORMAT_PROSODY_8KBPS 0x0094 /* Aculab plc */
	#define WAVE_FORMAT_ZYXEL_ADPCM 0x0097 /* ZyXEL Communications, Inc. */
	#define WAVE_FORMAT_PHILIPS_LPCBB 0x0098 /* Philips Speech Processing */
	#define WAVE_FORMAT_PACKED 0x0099 /* Studer Professional Audio AG */
	#define WAVE_FORMAT_MALDEN_PHONYTALK 0x00A0 /* Malden Electronics Ltd. */
	#define WAVE_FORMAT_RHETOREX_ADPCM 0x0100 /* Rhetorex Inc. */
	#define WAVE_FORMAT_IRAT 0x0101 /* BeCubed Software Inc. */
	#define WAVE_FORMAT_VIVO_G723 0x0111 /* Vivo Software */
	#define WAVE_FORMAT_VIVO_SIREN 0x0112 /* Vivo Software */
	#define WAVE_FORMAT_DIGITAL_G723 0x0123 /* Digital Equipment Corporation */
	#define WAVE_FORMAT_SANYO_LD_ADPCM 0x0125 /* Sanyo Electric Co., Ltd. */
	#define WAVE_FORMAT_SIPROLAB_ACEPLNET 0x0130 /* Sipro Lab Telecom Inc. */
	#define WAVE_FORMAT_SIPROLAB_ACELP4800 0x0131 /* Sipro Lab Telecom Inc. */
	#define WAVE_FORMAT_SIPROLAB_ACELP8V3 0x0132 /* Sipro Lab Telecom Inc. */
	#define WAVE_FORMAT_SIPROLAB_G729 0x0133 /* Sipro Lab Telecom Inc. */
	#define WAVE_FORMAT_SIPROLAB_G729A 0x0134 /* Sipro Lab Telecom Inc. */
	#define WAVE_FORMAT_SIPROLAB_KELVIN 0x0135 /* Sipro Lab Telecom Inc. */
	#define WAVE_FORMAT_G726ADPCM 0x0140 /* Dictaphone Corporation */
	#define WAVE_FORMAT_QUALCOMM_PUREVOICE 0x0150 /* Qualcomm, Inc. */
	#define WAVE_FORMAT_QUALCOMM_HALFRATE 0x0151 /* Qualcomm, Inc. */
	#define WAVE_FORMAT_TUBGSM 0x0155 /* Ring Zero Systems, Inc. */
	#define WAVE_FORMAT_MSAUDIO1 0x0160 /* Microsoft Corporation */
	#define WAVE_FORMAT_UNISYS_NAP_ADPCM 0x0170 /* Unisys Corp. */
	#define WAVE_FORMAT_UNISYS_NAP_ULAW 0x0171 /* Unisys Corp. */
	#define WAVE_FORMAT_UNISYS_NAP_ALAW 0x0172 /* Unisys Corp. */
	#define WAVE_FORMAT_UNISYS_NAP_16K 0x0173 /* Unisys Corp. */
	#define WAVE_FORMAT_CREATIVE_ADPCM 0x0200 /* Creative Labs, Inc */
	#define WAVE_FORMAT_CREATIVE_FASTSPEECH8 0x0202 /* Creative Labs, Inc */
	#define WAVE_FORMAT_CREATIVE_FASTSPEECH10 0x0203 /* Creative Labs, Inc */
	#define WAVE_FORMAT_UHER_ADPCM 0x0210 /* UHER informatic GmbH */
	#define WAVE_FORMAT_QUARTERDECK 0x0220 /* Quarterdeck Corporation */
	#define WAVE_FORMAT_ILINK_VC 0x0230 /* I-link Worldwide */
	#define WAVE_FORMAT_RAW_SPORT 0x0240 /* Aureal Semiconductor */
	#define WAVE_FORMAT_ESST_AC3 0x0241 /* ESS Technology, Inc. */
	#define WAVE_FORMAT_IPI_HSX 0x0250 /* Interactive Products, Inc. */
	#define WAVE_FORMAT_IPI_RPELP 0x0251 /* Interactive Products, Inc. */
	#define WAVE_FORMAT_CS2 0x0260 /* Consistent Software */
	#define WAVE_FORMAT_SONY_SCX 0x0270 /* Sony Corp. */
	#define WAVE_FORMAT_FM_TOWNS_SND 0x0300 /* Fujitsu Corp. */
	#define WAVE_FORMAT_BTV_DIGITAL 0x0400 /* Brooktree Corporation */
	#define WAVE_FORMAT_QDESIGN_MUSIC 0x0450 /* QDesign Corporation */
	#define WAVE_FORMAT_VME_VMPCM 0x0680 /* AT&amp;T Labs, Inc. */
	#define WAVE_FORMAT_TPC 0x0681 /* AT&amp;T Labs, Inc. */
	#define WAVE_FORMAT_OLIGSM 0x1000 /* Ing C. Olivetti &amp; C., S.p.A. */
	#define WAVE_FORMAT_OLIADPCM 0x1001 /* Ing C. Olivetti &amp; C., S.p.A. */
	#define WAVE_FORMAT_OLICELP 0x1002 /* Ing C. Olivetti &amp; C., S.p.A. */
	#define WAVE_FORMAT_OLISBC 0x1003 /* Ing C. Olivetti &amp; C., S.p.A. */
	#define WAVE_FORMAT_OLIOPR 0x1004 /* Ing C. Olivetti &amp; C., S.p.A. */
	#define WAVE_FORMAT_LH_CODEC 0x1100 /* Lernout &amp; Hauspie */
	#define WAVE_FORMAT_NORRIS 0x1400 /* Norris Communications, Inc. */
	#define WAVE_FORMAT_SOUNDSPACE_MUSICOMPRESS 0x1500 /* AT&amp;T Labs, Inc. */
	#define WAVE_FORMAT_DVM 0x2000 /* FAST Multimedia AG */
	#define WAVE_FORMAT_EXTENSIBLE	0xFFFE	/* PCM/IEEE format in WAVEFORMATEXTENSIBLE structure */
#endif

#define MAXWAVESIZE  0x8000000 // ignore *.wav > 128 MB
#define MAXWAVECHECK 0x800

#define SNDDESC SNDDESC_IEC61937

#define WAVERIFFSIZE				44 // 0x2c
#define WAVEFORMATEXSIZE			18
#define WAVEFORMATEXTENSIBLESIZE	40
#define WAVEFORMATIEC61937SIZE      52

namespace SAVFGAME
{
	enum eWaveErr
	{
		eWaveErr_NONE,    // no erorrs
		eWaveErr_NEXT,    // [internal procedure code]
		eWaveErr_MaxSize, // filesize > MAXWAVESIZE
		eWaveErr_BadRIFF, // chunkId != 'RIFF'  52 49 46 46
		eWaveErr_BadWAVE, // format  != 'WAVE'  57 41 56 45
		eWaveErr_FMTMiss, // missing FMT
		eWaveErr_FMTSize, // FMT size error
		eWaveErr_EOF,     // unexpected end of file [1]
		eWaveErr_EOF2,    // unexpected end of file [2]
		eWaveErr_EOF3,    // unexpected end of file [3]
		eWaveErr_EOF4,    // unexpected end of file [4]
		eWaveErr_EOF5,    // unexpected end of file [5]
	};

	std::wstring WaveFormatToString(uint16 format);

	struct WAVERIFF
	{

	#define WAVERIFF_BEGIN  0xC   // [chunkId, chunkSize, format] ... <JUNK> ...
	#define WAVERIFF_MIDDLE 0x18  // [subchunk1Id ... bitsPerSample] ... <LIST> ...
	#define WAVERIFF_END    0x8   // [subchunk2Id, subchunk2Size]
	#define WAVERIFF_FULL   ( WAVERIFF_BEGIN + WAVERIFF_MIDDLE + WAVERIFF_END )
	#define SUBHEADER_SIZE  8
	#define FMT_SIGN  0x20746d66
	#define FMT_SIZE  ( WAVERIFF_MIDDLE - SUBHEADER_SIZE )
	#define RIFF_SIGN 0x46464952
	#define WAVE_SIGN 0x45564157
	#define DATA_SIGN 0x61746164
	#define JUNK_SIGN 0x4b4e554a
	#define LIST_SIGN 0x5453494c

		WAVERIFF() { _Reset(); }
		~WAVERIFF() {}

	private:
		//>> Читает подзаголовоки
		eWaveErr _ReadNext(byte * data, const __int32 size_in_bytes, __int32 & position)
		{
			eWaveErr hRes = eWaveErr_NONE;

			struct HEADER {
				unsigned __int32 subchunk_Id;
				unsigned __int32 subchunk_Size;
			};

			HEADER * h;

			if (size_in_bytes - position < SUBHEADER_SIZE) // ERROR
				return hRes = eWaveErr_EOF3;               // .

			h = reinterpret_cast <HEADER*> (data + position);

			position += SUBHEADER_SIZE;

			switch (h->subchunk_Id)
			{
				case FMT_SIGN:
				{
					subchunk1Id   = h->subchunk_Id;   // == FMT_SIGN
					subchunk1Size = h->subchunk_Size; // == FMT_SIZE

					if (size_in_bytes - position < FMT_SIZE) // ERROR
						return hRes = eWaveErr_EOF4;         // .

					// read data
					memcpy((byte*)this + WAVERIFF_BEGIN + SUBHEADER_SIZE, data + position, FMT_SIZE);

				//	position += FMT_SIZE;      // standart FMT 0x10, 
					position += subchunk1Size; // non-standart FMT - 0x28 for example - have 0x18 extra data

					break;
				}
				case DATA_SIGN:
				{
					if (subchunk1Id != FMT_SIGN)        // ERROR
						return hRes = eWaveErr_FMTMiss; // .

				//	if (subchunk1Size != FMT_SIZE)      // ERROR
					if (subchunk1Size  < FMT_SIZE)      // ERROR  min. 0x10 or greater
						return hRes = eWaveErr_FMTSize; // .

					subchunk2Id   = h->subchunk_Id;
					subchunk2Size = h->subchunk_Size;

					return hRes; // eWaveErr_NONE
				}
				case JUNK_SIGN:
				case LIST_SIGN:
				default:
				{
					if (size_in_bytes - position < (__int32)(h->subchunk_Size)) // ERROR
						return hRes = eWaveErr_EOF5;                            // .

					// skip data

					position += h->subchunk_Size;

					break;
				}
			}

			return hRes = eWaveErr_NEXT;
		}

	public:
		//>> [DEBUG] Генерирует заголовок
		void _Generate(__int32 buffer_size, __int16 channels, __int32 samples_rate, __int16 bits_per_sample, __int16 waveFormat,
			const char* license_information)
		{
			chunkId = RIFF_SIGN;			// 'RIFF'
			chunkSize = buffer_size +
				WAVERIFF_BEGIN - SUBHEADER_SIZE +
				WAVERIFF_MIDDLE +
				WAVERIFF_END;
			format = WAVE_SIGN;		// 'WAVE'
			subchunk1Id = FMT_SIGN;		// 'fmt '
			subchunk1Size = FMT_SIZE;
			audioFormat = waveFormat;		// WAVE_FORMAT_PCM (1), WAVE_FORMAT_IEEE_FLOAT (3), WAVE_FORMAT_EXTENSIBLE (-1)
			numChannels = channels;
			sampleRate = samples_rate;
			blockAlign = (bits_per_sample / 8) * channels;
			byteRate = blockAlign * samples_rate;
			bitsPerSample = bits_per_sample;
			subchunk2Id = DATA_SIGN;		// 'data'
			subchunk2Size = buffer_size;
			//////////////////
			__info = license_information;
			unsigned __int32 info_size = (unsigned __int32)__info.size();
			if (info_size)
			{
				if (info_size % 4) // fix padding
				{
					__int32 extra_size = (__int32)(4 - (info_size % 4));
					for (__int32 i = 0; i < extra_size; i++)
						__info.append(" ");
					info_size = (unsigned __int32)__info.size();
				}
				// 'JUNK' + junk size + info size //twice
				chunkSize += 4 + 4 + info_size;// *2;
			}
		}

		//>> [DEBUG] Записывает заголовок и данные
		bool _Write(const wchar_t* filepath, void * data)
		{
			if (data == nullptr)
				return false; // ERROR

			FILE *fp = _wfsopen(filepath, L"wb", _SH_DENYNO);
			if (fp)
			{
				auto info_size = __info.size();
				fwrite(this, WAVERIFF_BEGIN, 1, fp);
				if (info_size)
				{
					__int32 _JUNK_ = JUNK_SIGN;
					fwrite(&_JUNK_, 4, 1, fp);
					fwrite(&info_size, 4, 1, fp);
					fwrite(__info.c_str(), info_size, 1, fp);
				}
				fwrite((byte*)this + WAVERIFF_BEGIN, WAVERIFF_MIDDLE + WAVERIFF_END, 1, fp);
				fwrite(data, subchunk2Size, 1, fp);
				//if (info_size) // info twice
				//	fwrite(info.c_str(), info_size, 1, fp);
				fclose(fp);
				return true;
			}
			else // ERROR
				return false; 
		}

	public:
		//>> Сброс в ноль
		void _Reset()
		{
			chunkId       = 0;
			chunkSize     = 0;
			format        = 0;
			subchunk1Id   = 0;
			subchunk1Size = 0;
			audioFormat   = 0;
			numChannels   = 0;
			sampleRate    = 0;
			byteRate      = 0;
			blockAlign    = 0;
			bitsPerSample = 0;
			subchunk2Id   = 0;
			subchunk2Size = 0;

			__info.clear();
		}

		//>> Читает заголовок
		eWaveErr _Read(byte * data, __int32 size_in_bytes, __int32 size_of_file, __int32 & out_position)
		{
			eWaveErr hRes = eWaveErr_NONE;

			out_position = 0;

			if (size_of_file > MAXWAVESIZE)     // ERROR
				return hRes = eWaveErr_MaxSize; // .

			if (size_in_bytes < WAVERIFF_FULL)  // ERROR
				return hRes = eWaveErr_EOF;     // .

			if (size_in_bytes > MAXWAVECHECK) // limited
				size_in_bytes = MAXWAVECHECK; // .

			//if (size_of_file < SUBHEADER_SIZE) // EOF
			//	return hRes = eWaveErr_EOF;

			memcpy(this, data, WAVERIFF_BEGIN);

			out_position += WAVERIFF_BEGIN;

			if (chunkId != RIFF_SIGN)           // ERROR
				return hRes = eWaveErr_BadRIFF; // .

			if (format != WAVE_SIGN)            // ERROR
				return hRes = eWaveErr_BadWAVE; // .


			if ((__int32)chunkSize > size_of_file - SUBHEADER_SIZE)            // truncated file
				chunkSize = (unsigned __int32)(size_of_file - SUBHEADER_SIZE); // .

			subchunk1Id   = 0;
			subchunk1Size = 0;

			for (bool done = false; done != true; )
			{
				eWaveErr ret = _ReadNext(data, size_in_bytes, out_position);
				switch (ret)
				{			
				case eWaveErr_NONE:
					done = true;
					break;
				case eWaveErr_NEXT:
					break;
				default:
					return hRes = ret;
				}
			}

			__int32 sz = size_of_file - out_position; // size until EOF

			if ((__int32)subchunk2Size > sz)          // truncated file
				subchunk2Size = (unsigned __int32)sz; // .

			if ((__int32)subchunk2Size <= NULL) // ERROR
				return hRes = eWaveErr_EOF2;    // .

			return hRes;
		}

		//>> Читает заголовок
		eWaveErr _Read(FILE *& fp)
		{
			eWaveErr hRes = eWaveErr_NONE;

			__int32 size, position;
			__int64 filesize = _filelengthi64(_fileno(fp));

			if (filesize > MAXWAVESIZE)         // ERROR
				return hRes = eWaveErr_MaxSize; // .

			// analyzing first MAXWAVECHECK bytes
			byte data [MAXWAVECHECK];

			size = (filesize > MAXWAVECHECK) ? MAXWAVECHECK : (__int32)filesize;

			rewind(fp);
			fread(data, 1, size, fp);

			// trying to read
			hRes = _Read(data, size, (__int32)filesize, position);

			if (hRes != eWaveErr_NONE)
			{
				rewind(fp);
			}
			else
			{
				_fseeki64(fp, position, 0);
			}

			return hRes;
		}

	public:
		static
		//>> Проверка WAVE RIFF
		bool _Check(__int32 * data, __int32 size_in_bytes)
		{
			if (size_in_bytes < WAVERIFF_BEGIN) // EOF
				return false;

			if (data[0] != RIFF_SIGN ||
				data[2] != WAVE_SIGN)
				return false;

			return true;
		}

		static
		//>> Проверка WAVE RIFF
		bool _Check(FILE * fp)
		{
			__int32 data[WAVERIFF_BEGIN / sizeof(__int32)];

			if (_filelengthi64(_fileno(fp)) < WAVERIFF_BEGIN) // EOF
				return false;

			rewind(fp);
			fread(data, 1, WAVERIFF_BEGIN, fp);

			return _Check(data, WAVERIFF_BEGIN);
		}

		////////////////////////////

		unsigned __int32 chunkId;			//>> 'RIFF'  52 49 46 46
		unsigned __int32 chunkSize;			//>> = fsize - 8;
		unsigned __int32 format;			//>> 'WAVE'  57 41 56 45    'AVI ', 'RMID', ...
		// ...
		// <JUNK>
		// ...
		unsigned __int32 subchunk1Id;		//>> 'fmt '  66 6d 74 20    'JUNK'  4a 55 4e 4b  'PAD '  50 41 44 20
		unsigned __int32 subchunk1Size;		//>> байт до subchunk2Id
		unsigned __int16 audioFormat;		//>> PCM = 1 (линейная импульсно-кодовая модуляция), ...
		unsigned __int16 numChannels;		//>> Mono = 1, Stereo = 2, ...
		unsigned __int32 sampleRate;		//>> частота дискретизации : 8000 Гц, 44100 Гц ...
		unsigned __int32 byteRate;			//>> байт в секунду
		unsigned __int16 blockAlign;		//>> байт одного семпла (все каналы)
		unsigned __int16 bitsPerSample;		//>> бит в семпле (глубина звучания) : 8, 16, 24, 32
		// ...
		// <LIST>
		// ...
		unsigned __int32 subchunk2Id;		//>> 'data'  64 61 74 61  'LIST'  4c 49 53 54
		unsigned __int32 subchunk2Size;		//>> data size
		//
		std::string __info; // <license_information> при генерации

	#undef WAVERIFF_BEGIN
	#undef WAVERIFF_MIDDLE
	#undef WAVERIFF_END
	#undef WAVERIFF_FULL
	#undef SUBHEADER_SIZE
	#undef FMT_SIGN
	#undef FMT_SIZE
	#undef RIFF_SIGN
	#undef WAVE_SIGN
	#undef DATA_SIGN
	#undef JUNK_SIGN
	#undef LIST_SIGN

	};

	/*typedef struct _GUID {
		unsigned long  Data1;
		unsigned short Data2;
		unsigned short Data3;
		unsigned char  Data4[8];
	} GUID; */

	struct SNDDESC_DEF // WAVEFORMATEX
	{
		SNDDESC_DEF() { Reset(); }


		WORD    wFormatTag;        // format type : (1) WAVE_FORMAT_PCM (3) WAVE_FORMAT_IEEE_FLOAT (0xFFFE) WAVE_FORMAT_EXTENSIBLE
		WORD    nChannels;         // number of channels (i.e. mono, stereo...)
		DWORD   nSamplesPerSec;    // sample rate
		DWORD   nAvgBytesPerSec;   // for buffer estimation
		WORD    nBlockAlign;       // size of frame (block) in bytes
		WORD    wBitsPerSample;    // Number of bits per sample of mono data
		WORD    cbSize;            // extra info size : 0 or 22 == (40 - 18) == SNDDESC_EX (WAVEFORMATEXTENSIBLE)

		void Reset()
		{
			wFormatTag      = 0;
			nChannels       = 0;
			nSamplesPerSec  = 0;
			nAvgBytesPerSec = 0;
			nBlockAlign     = 0;
			wBitsPerSample  = 0;
			cbSize          = 0;
		}
		void Printf()
		{
			wprintf(L"\nwFormatTag      = %04X = %s", 
				wFormatTag, 
				WaveFormatToString(wFormatTag).c_str());
			wprintf(L"\nnChannels       = %i", nChannels);
			wprintf(L"\nnSamplesPerSec  = %i", nSamplesPerSec);
			wprintf(L"\nnAvgBytesPerSec = %i", nAvgBytesPerSec);
			wprintf(L"\nnBlockAlign     = %i", nBlockAlign);
			wprintf(L"\nwBitsPerSample  = %i", wBitsPerSample);
			wprintf(L"\ncbSize          = %i", cbSize);
		}
	};
	struct SNDDESC_EX : public SNDDESC_DEF // WAVEFORMATEXTENSIBLE
	{
		SNDDESC_EX() : SNDDESC_DEF() { Reset(); }
		
		union {
			WORD wValidBitsPerSample;       // bits of precision
			WORD wSamplesPerBlock;          // valid if wBitsPerSample==0
			WORD wReserved;                 // If neither applies, set to zero.
		} _ex_SamplesUnion;
		DWORD _ex_dwChannelMask;			// which channels are
		GUID  _ex_SubFormat;				// GUID

		void Reset()
		{
			SNDDESC_DEF::Reset();
			_ex_SamplesUnion.wReserved = 0;
			_ex_dwChannelMask          = 0;
			_ex_SubFormat.Data1 = 0;
			_ex_SubFormat.Data2 = 0;
			_ex_SubFormat.Data3 = 0;
			for (auto & Data4 : _ex_SubFormat.Data4)
				Data4 = 0;
		}
		void Printf()
		{
			SNDDESC_DEF::Printf();

			wprintf(L"\nex_Reserved     = %i (wValidBitsPerSample / wSamplesPerBlock)", _ex_SamplesUnion.wReserved);
			wprintf(L"\nex_ChannelMask  = %i", _ex_dwChannelMask);
			wprintf(L"\nex_SubFormat    = %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
				_ex_SubFormat.Data1, _ex_SubFormat.Data2, _ex_SubFormat.Data3,
				_ex_SubFormat.Data4[0], _ex_SubFormat.Data4[1], _ex_SubFormat.Data4[2], _ex_SubFormat.Data4[3], 
				_ex_SubFormat.Data4[4], _ex_SubFormat.Data4[5], _ex_SubFormat.Data4[6], _ex_SubFormat.Data4[7] );
		}
	};
	struct SNDDESC_IEC61937 : public SNDDESC_EX
	{
		SNDDESC_IEC61937() : SNDDESC_EX() { Reset(); }

		DWORD   iecEncodedSamplesPerSec;	// after decoding
		DWORD   iecEncodedChannelCount;		// after decoding
		DWORD   iecAverageBytesPerSec;		// after decoding, can be 0

		void Reset()
		{
			SNDDESC_EX::Reset();

			iecEncodedSamplesPerSec = 0;
			iecEncodedChannelCount  = 0;
			iecAverageBytesPerSec   = 0;
		}
		void Printf()
		{
			SNDDESC_EX::Printf();

			wprintf(L"\niec_SamplesPSec = %i", iecEncodedSamplesPerSec);
			wprintf(L"\niec_Channels    = %i", iecEncodedChannelCount);
			wprintf(L"\niec_AverageBPS  = %i", iecAverageBytesPerSec);
		}
	};

	/* https://msdn.microsoft.com/ru-ru/library/windows/desktop/dd390970(v=vs.85).aspx
	nBlockAlign
	Block alignment, in bytes.The block alignment is the minimum atomic unit of data for the wFormatTag
	  format type.If wFormatTag is WAVE_FORMAT_PCM, nBlockAlign must equal(nChannels × wBitsPerSample) / 8.
	  For non - PCM formats, this member must be computed according to the manufacturer's specification of
	  the format tag.
	Software must process a multiple of nBlockAlign bytes of data at a time.Data written to and read from
	  a device must always start at the beginning of a block.For example, it is illegal to start playback
	  of PCM data in the middle of a sample(that is, on a non - block - aligned boundary).
	*/

	//>> Возвращает текстовое описание формата
	std::wstring WaveFormatToString(uint16 format)
	{
		std::wstring ret;
		switch (format)
		{
		case WAVE_FORMAT_UNKNOWN:
			ret = L"Unknown";
			break;
		case WAVE_FORMAT_PCM:
			ret = L"PCM, pulse code modulation (Microsoft)";
			break;
		case WAVE_FORMAT_ADPCM:
			ret = L"ADPCM, adaptive differential pulse-code modulation (Microsoft)";
			break;
		case WAVE_FORMAT_EXTENSIBLE: /* PCM/IEEE format in WAVEFORMATEXTENSIBLE structure */
			ret = L"PCM/IEEE, pulse code modulation (Microsoft)";
			break;
		default:
			ret = L"Other";
		}
		return ret;
	}
}

#endif // _WAVE_H