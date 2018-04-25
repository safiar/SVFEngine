// ----------------------------------------------------------------------- //
//
// MODULE  : Sound.h
//
// PURPOSE : Базовый Audio класс
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _SOUND_H
#define _SOUND_H

#include "stdafx.h"
#include "link_defines.h"
#include "WAVE.h"

#define ERROR_SoundIncorrectBitsPerSample	L"Некорректный битрейт"

#define SOUND_THREADS_MAX 256	// предел одновременных звуковых потоков

#ifdef WINDOWS_IMM_WASAPI
	//#define SNDDESC WAVEFORMATEXTENSIBLE
	#define SNDDESC WAVEFORMATEX
#else
	#ifndef _WAVEFORMATEX_
	#define _WAVEFORMATEX_
		typedef struct tWAVEFORMATEX
		{
			WORD    wFormatTag;        /* format type */
			WORD    nChannels;         /* number of channels (i.e. mono, stereo...) */
			DWORD   nSamplesPerSec;    /* sample rate */
			DWORD   nAvgBytesPerSec;   /* for buffer estimation */
			WORD    nBlockAlign;       /* block size of data */
			WORD    wBitsPerSample;    /* Number of bits per sample of mono data */
			WORD    cbSize;            /* The count in bytes of the size of
											extra information (after cbSize) */

		} WAVEFORMATEX;
		typedef WAVEFORMATEX       *PWAVEFORMATEX;
		typedef WAVEFORMATEX NEAR *NPWAVEFORMATEX;
		typedef WAVEFORMATEX FAR  *LPWAVEFORMATEX;
	#endif /* _WAVEFORMATEX_ */
	//----------------------------------------------------------------
	#ifndef _WAVEFORMATEXTENSIBLE_
	#define _WAVEFORMATEXTENSIBLE_
		typedef struct {
		WAVEFORMATEX    Format;
		union {
				WORD wValidBitsPerSample;       /* bits of precision  */
				WORD wSamplesPerBlock;          /* valid if wBitsPerSample==0 */
				WORD wReserved;                 /* If neither applies, set to zero. */
			} Samples;
		DWORD           dwChannelMask;      /* which channels are */
											/* present in stream  */
		GUID            SubFormat;
		} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;
	#endif // !_WAVEFORMATEXTENSIBLE_
	//----------------------------------------------------------------
	//#define SNDDESC WAVEFORMATEXTENSIBLE
	#define SNDDESC WAVEFORMATEX
#endif

namespace SAVFGAME
{
	enum SNDTYPE
	{
		SND_UNKNOWN,
		SND_WAVE,

		SND_ENUM_MAX
	};

	struct SNDDATA
	{
	public:
		SNDDATA(const SNDDATA& src)				= delete;
		SNDDATA(SNDDATA&& src)					= delete;
		SNDDATA& operator=(SNDDATA&& src)       = delete;
		SNDDATA& operator=(const SNDDATA& src)	= delete;
	public:
		SNDDATA() : ticket(0)
		{
			snddesc = new SNDDESC;
			Close();
		};
		~SNDDATA()
		{
			Close();
			_DELETE(snddesc);
		}
		void Close()
		{
			file.Close();
			filename.clear();
			type = SND_UNKNOWN;
			duration = 0;
			volume_multiplier = 1.f;
			interrupt = false;
			ZeroMemory(snddesc, sizeof(SNDDESC));
		}
		bool CheckFile()
		{
			if (file.buf == nullptr ||
				file.count == 0) return false;
			return true;
		}
		////////////
		TBUFFER <byte, int32>	 file;				// собственно звук
		wstring					 filename;			// имя файла
		float					 duration;			// ms
		float					 volume_multiplier;	// out volume = master volume * multiplier
		SNDTYPE					 type;				// исходный формат файла (wav, mp3, ogg и т.п.)
		SNDDESC	*				 snddesc;			// свойства
		mutex					 m_Play;			// блокировщик воспроизведения в !asynch режиме
		atomic<bool>			 interrupt;			// прерыватель воспроизведения
		uint64					 ticket;			// номер последнего задания в playpool
	};

	class CSound
	{
	public:
		CSound(const CSound& src)				= delete;
		CSound(CSound&& src)					= delete;
		CSound& operator=(const CSound& src)	= delete;
		CSound& operator=(CSound&& src)			= delete;
	protected:
		VECDATAP <SNDDATA>	snd;			// данные загруженных звуковых файлов
		THREADPOOL			playpool;		// содержит задачи и потоки воспроизведения
		float				master_volume;	// общая громкость
	private:
		bool  isInit;
	public:
		CSound() : master_volume(0.5f), isInit(false) {};
		virtual ~CSound() { } // Close();

		virtual uint32 Load(const wchar_t* gamePath, const wchar_t* filename) = 0;				// Прототип для конечного класса
		virtual void PlayThread(uint32 id, bool async, bool loop, float volume_multiplier) = 0;	// Прототип для конечного класса
		virtual void PlayThread(uint32 id, bool async, bool loop) = 0;							// Прототип для конечного класса

		virtual void Init()
		{
			if (isInit) return;

			//playpool.WorkersSet(SOUND_THREADS_MAX);

			isInit = true;
		};
		virtual void Close()
		{
			if (isInit)
			{
				playpool.Close();
				snd.Close(1,1);
				master_volume = 0.5f;
				isInit = false;
			}
		};

	//////////////// Basic block ////////////////

	protected:
		//>> Описание данных
		void ConvertToDesc(WAVERIFF& wav, SNDDESC * snddesc)
		{
			if (wav.audioFormat == WAVE_FORMAT_EXTENSIBLE)
				wav.audioFormat = WAVE_FORMAT_PCM;

			snddesc->wFormatTag      = wav.audioFormat;
			snddesc->nChannels       = wav.numChannels;
			snddesc->nSamplesPerSec  = wav.sampleRate;
			snddesc->nAvgBytesPerSec = wav.byteRate;		// WAVE_FORMAT_PCM : == nSamplesPerSec × nBlockAlign
			snddesc->nBlockAlign     = wav.blockAlign;		// WAVE_FORMAT_PCM : == (nChannels × wBitsPerSample) / 8
			snddesc->wBitsPerSample  = wav.bitsPerSample;	// WAVE_FORMAT_PCM : == 8 | 16
			snddesc->cbSize          = 0;

			if (snddesc->wBitsPerSample % 8)
				_MBM(ERROR_SoundIncorrectBitsPerSample);

		/*	snddesc.Format.nChannels       = wav.numChannels;
			snddesc.Format.nSamplesPerSec  = wav.sampleRate;
			snddesc.Format.nAvgBytesPerSec = wav.byteRate;			// WAVE_FORMAT_PCM : == nSamplesPerSec × nBlockAlign
			snddesc.Format.nBlockAlign     = wav.blockAlign;		// WAVE_FORMAT_PCM : == (nChannels × wBitsPerSample) / 8
			snddesc.Format.wBitsPerSample  = wav.bitsPerSample;		// WAVE_FORMAT_PCM : == 8 | 16
			snddesc.Format.cbSize          = 0x16;

			snddesc.Format.wFormatTag      = WAVE_FORMAT_EXTENSIBLE;
			INIT_WAVEFORMATEX_GUID(&snddesc.SubFormat, wav.audioFormat); //*/
		}

		//>> Считывание файла .wav
		bool OpenSoundWAV(FILE *fp, const wchar_t* filename)
		{		
			WAVERIFF wav;
			fread(&wav,WAVERIFFSIZE,1,fp);

			fseek(fp, 0x14 + wav.subchunk1Size, 0);	// идём к 'data'
			fread(&wav.subchunk2Id,4,1,fp);			// .
			fread(&wav.subchunk2Size,4,1,fp);		// .

			if (wav.subchunk2Id == 0x5453494c) // 'LIST' вместо 'data'
			{
				fseek(fp,wav.subchunk2Size,1);
				fread(&wav.subchunk2Id,4,1,fp);
				fread(&wav.subchunk2Size,4,1,fp);
			}
			if (wav.subchunk1Id == 0x4b4e554a) // 'JUNK' вместо 'fmt'
			{
				int x;
				fseek(fp,12+4,0);
				fread(&x,4,1,fp);
				fseek(fp,x,1);
				fread(&wav.subchunk1Id, WAVERIFFSIZE-12, 1, fp);
			}
			if (wav.subchunk2Id != 0x61746164) // != 'data'
				{ _MBM(ERROR_UnknownDataType); return false; }

			snd.emplace_back_create();
			auto & sound = *snd.last();

			sound.filename = filename;
			sound.file.Create(wav.subchunk2Size);
			fread(sound.file.buf, sizeof(byte), sound.file.count, fp);
			//PrintWaveRiff(wav);
			
			sound.duration  = (float)wav.subchunk2Size;		// size
			sound.duration /= (float)wav.bitsPerSample / 8;	// size / bytes per sample
			sound.duration /= wav.numChannels;				// size / num channels in sample
			sound.duration /= wav.sampleRate;				// size / samples per second

			ConvertToDesc(wav, sound.snddesc);

			return true;
		}

		//>> Определение формата и вызов соотв. функции
		bool OpenSoundFormat(const wchar_t* filepath, const wchar_t* filename)
		{
			bool ret = false;

			uint32 check[16];
			SNDTYPE format = SND_UNKNOWN;
			FILE *fp = _wfsopen(filepath, L"rb", _SH_DENYNO);
			fread(&check,16*sizeof(uint32),1,fp);
			rewind(fp);

			if (check[0] == 0x46464952 && // RIFF
				check[2] == 0x45564157)   // WAVE
					format = SND_WAVE;

			switch (format)
			{
			case SND_WAVE: ret=OpenSoundWAV(fp, filename); break;
			default: _MBM(ERROR_UnknownDataType);
			};

			fclose(fp);
			return ret;
		};

		//>> Открытие файла с проверкой доступности
		bool OpenSoundProc(const wchar_t* gamePath, const wchar_t* filename)
		{
			wchar_t syspath[256], error[256], p1[256], p2[256];
			GetCurrentDirectory(256,syspath);

			wchar_t *p = nullptr;

			wsprintf(error,L"%s %s",ERROR_OpenFile,filename);
			wsprintf(p1,L"%s\\%s\\%s",gamePath,DIRECTORY_SOUNDS,filename);
			wsprintf(p2,L"%s\\%s\\%s",syspath,DIRECTORY_SOUNDS,filename);
				
			if (p = LoadFileCheck(2, p1, p2)) return OpenSoundFormat(p, filename);
			else _MBM(error);

			return false;
		};

	//////////////// Support block ////////////////

	protected:
		//>> Вывод в консоль сведений из .wav заголовка
		void PrintWaveRiff(const WAVERIFF& wav)
		{
			printf(	"\n%8x-h chunkId"
					"\n%8x-h chunkSize"
					"\n%8x-h format"
					"\n%8x-h subchunk1Id"
					"\n%8x-h subchunk1Size"
					"\n%8x-h audioFormat"
					"\n%8x-h numChannels"
					"\n%8i   sampleRate"
					"\n%8i   byteRate"
					"\n%8x-h blockAlign"
					"\n%8i   bitsPerSample"
					"\n%8x-h subchunk2Id"
					"\n%8x-h subchunk2Size",
					wav.chunkId, wav.chunkSize, wav.format, wav.subchunk1Id, wav.subchunk1Size,
					wav.audioFormat, wav.numChannels, wav.sampleRate, wav.byteRate, wav.blockAlign,
					wav.bitsPerSample, wav.subchunk2Id, wav.subchunk2Size );

			FLOAT  sduration  = (FLOAT)wav.subchunk2Size;
			       sduration /= (FLOAT)wav.bitsPerSample / 8;
			       sduration /= wav.numChannels;
			       sduration /= wav.sampleRate;
			uint16 mduration = (uint16)(floor(sduration) / 60);
			sduration = sduration - (mduration * 60);
			printf("\n\nDuration [M:S.ms]: %02d:%06.3f\n", mduration, sduration);
		}

	public:
		//>> Проверка корректности ID
		bool CheckSoundID(uint64 id)
		{
			if (id < snd.size()) return true;
			else ; //_MBM(ERROR_IDLIMIT);
			return false;
		}

		//>> Проверка файла
		bool CheckSoundFile(uint64 id)
		{
			if ( snd[id] == nullptr ||
				!snd[id]->CheckFile()) return false;
			return true;
		}

		//>> CheckSoundID() + CheckSoundFile()
		bool CheckSound(uint64 id)
		{
			if (!CheckSoundID(id) || !CheckSoundFile(id))
				return false;
			return true;
		}

		//>> Устанавливает общую громкость [+0.f..+1.f]
		void SetMasterVolume(float vol)
		{
			vol = max(0.f, vol);
			vol = min(1.f, vol);
			master_volume = vol;
		}

		//>> Устанавливает множитель громкости [+0.f..+1.f..+N.f]
		void SetVolumeMultiplier(uint32 id, float volume_multiplier, bool all)
		{
			if (!all)												{
				if (id >= snd.size())   return; /* wrong id */
				if (snd[id] == nullptr) return; /* missing sound */	}

			if (volume_multiplier >= 0)
			{
				if (!all) snd[id]->volume_multiplier = volume_multiplier;
				else for (auto & cur : snd)
				{
					if (cur == nullptr) continue; /* missing sound */
					cur->volume_multiplier = volume_multiplier;
				}
			}
		}

	protected:
		//>> Выводит в консоль информацию WAVEFORMATEX
		void PrintfSNDDESC(const SNDDESC& snddesc)
		{
			printf("\n"
				"\nwFormatTag          %4x-h"	// format type
				"\nnChannels           %4x-h"	// number of channels (i.e. mono, stereo...)
				"\nnSamplesPerSec  %8i"			// sample rate
				"\nnAvgBytesPerSec %8i"			// for buffer estimation
				"\nnBlockAlign         %4x-h"	// block size of data
				"\nwBitsPerSample      %4i"		// number of bits per sample of mono data
				"\ncbSize              %4x-h",	// the count in bytes of the size of extra information (after cbSize)
				snddesc.wFormatTag,
				snddesc.nChannels,
				snddesc.nSamplesPerSec,
				snddesc.nAvgBytesPerSec,
				snddesc.nBlockAlign,
				snddesc.wBitsPerSample,
				snddesc.cbSize);
		}

	//////////////// Threads control block ////////////////

	public:
		//>> Вызов принудительного завершения :: id == MISSING (-1) завершает все
		bool InterruptSound(uint64 id)
		{
			bool all = (_ISMISS(id));
			uint64 i;

			if (!all) if (!CheckSound(id)) return false;

			i = 0;
			if (!all) snd[id]->_RLX_1(interrupt);
			else for (auto & cur : snd)
			{
				if (!CheckSoundFile(i++)) continue;
				cur->_RLX_1(interrupt);
			}

			i = 0;
			if (!all) playpool.Wait(snd[id]->ticket);
			else for (auto & cur : snd)
			{
				if (!CheckSoundFile(i++)) continue;
				playpool.Wait(cur->ticket);
			}

			i = 0;
			if (!all) snd[id]->_RLX_0(interrupt);
			else for (auto & cur : snd)
			{
				if (!CheckSoundFile(i++)) continue;
				cur->_RLX_0(interrupt);
			}

			return true;
		}

	};
}

#endif // _SOUND_H