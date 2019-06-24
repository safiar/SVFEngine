// ----------------------------------------------------------------------- //
//
// MODULE  : pcm.h
//
// PURPOSE : Обработка RAW звука
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _PCM_H
#define _PCM_H

#include "helper.h"
#include "SoundDevice.h"

#define DECIBEL_TO_VOLUME_GAIN_2 (db) pow(2, (float)dB/6)  // multiplier =  2 ^ (DECIBEL/6)
#define DECIBEL_TO_VOLUME_GAIN_10(db) pow(10,(float)dB/20) // multiplier = 10 ^ (DECIBEL/20)

#define PCM_MAXSAMPLESPERSEC 192000 // ограничитель, макс. частота семплов (192 KHz)
#define PCM_MAXBITSPERSAMPLE 64		// максимальный размер семпла 64 бита
#define PCM_MAXCHANNELS      8      // ограничитель, макс. каналов 7+1 (7.1 аудио)
#define PCM_NULL8BIT         0x80	// ноль 8-битного семпла
#define PCM_DEFAULT_4_CH	 eSDRange_QUAD	// физ. расположение 4-канальной системы по умолчанию
#define PCM_DEFAULT_6_CH	 eSDRange_5p1	// физ. расположение 6-канальной системы по умолчанию
#define PCM_DEFAULT_8_CH	 eSDRange_7p1	// физ. расположение 8-канальной системы по умолчанию


//	#define PCM_COMMON_SYNCH_1S_DEBUG   // общие точки синхронизации в секундных порциях (иначе раздельные порции)
//
#ifdef PCM_COMMON_SYNCH_1S_DEBUG
	#define PCM_ONE_FIX	1 // 0 - раздельные секундные порции, 1 - общие точки синхронизации в секундных порциях
#else
	#define PCM_ONE_FIX	0 // 0 - раздельные секундные порции, 1 - общие точки синхронизации в секундных порциях
//	#define PCM_SEPARATE_SYNCH_1S_DEBUG // раздельный способ подсчёта для раздельных секундных порций (иначе слитный)

	#ifdef PCM_SEPARATE_SYNCH_1S_DEBUG
		#define PCM_DISABLE_INTERMEDIATE // отключить промежуточные точки синхронизации
	#endif
#endif


//////////////////////////////////////
//                                  //
// Speaker position order list      //
// from first (up) to last (bottom) //
//                                  //
// SPEAKER_FRONT_LEFT               //
// SPEAKER_FRONT_RIGHT              //
// SPEAKER_FRONT_CENTER             //
// SPEAKER_LOW_FREQUENCY            //
// SPEAKER_BACK_LEFT                //
// SPEAKER_BACK_RIGHT               //
// SPEAKER_FRONT_LEFT_OF_CENTER     //
// SPEAKER_FRONT_RIGHT_OF_CENTER    //
// SPEAKER_BACK_CENTER              //
// SPEAKER_SIDE_LEFT                //
// SPEAKER_SIDE_RIGHT               //
// SPEAKER_TOP_CENTER               //
// SPEAKER_TOP_FRONT_LEFT           //
// SPEAKER_TOP_FRONT_CENTER         //
// SPEAKER_TOP_FRONT_RIGHT          //
// SPEAKER_TOP_BACK_LEFT            //
// SPEAKER_TOP_BACK_CENTER          //
// SPEAKER_TOP_BACK_RIGHT           //
//                                  //
//////////////////////////////////////

//////////////////////////////////////////////
//                                          //
// MONO			[     C     ] Front         //
//                                          //
// STEREO		[  L  o  R  ] Front         //
//                                          //
// QUAD			[  L     R  ] Front         //
// QUAD			[     o     ] Front         //
// QUAD			[  L     R  ] Back          //
//                                          //
// SURROUND		[     C     ] Front         //
// SURROUND		[  L  o  R  ] Front         //
// SURROUND		[     C     ] Back          //
//                                          //
// 5.1			[ L F C   R ] Front         //
// 5.1			[     o     ] Front         //
// 5.1			[ L       R ] Back          //
//                                          //
// 7.1			[ L F C   R ] Front         //
// 7.1			[ L   o   R ] FrontOfCenter //
// 7.1			[   L   R   ] Back          //
//                                          //
//////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                      //
// CHANNEL   0             1            2             3              4          5           6                     7                     //
//                                                                                                                                      //
// MONO      FRONT_CENTER  .            .             .              .          .           .                     .                     //
// STEREO    FRONT_LEFT    FRONT_RIGHT  .             .              .          .           .                     .                     //
// QUAD      FRONT_LEFT    FRONT_RIGHT  BACK_LEFT     BACK_RIGHT     .          .           .                     .                     //
// SURROUND  FRONT_LEFT    FRONT_RIGHT  FRONT_CENTER  BACK_CENTER    .          .           .                     .                     //
// 5.1       FRONT_LEFT    FRONT_RIGHT  FRONT_CENTER  LOW_FREQUENCY  BACK_LEFT  BACK_RIGHT  .                     .                     //
// 5.1 SURR  FRONT_LEFT    FRONT_RIGHT  FRONT_CENTER  LOW_FREQUENCY  SIDE_LEFT  SIDE_RIGHT  .                     .                     //
// 7.1       FRONT_LEFT    FRONT_RIGHT  FRONT_CENTER  LOW_FREQUENCY  BACK_LEFT  BACK_RIGHT  FRONT_LEFT_OF_CENTER  FRONT_RIGHT_OF_CENTER //
// 7.1 SURR  FRONT_LEFT    FRONT_RIGHT  FRONT_CENTER  LOW_FREQUENCY  SIDE_LEFT  SIDE_RIGHT  BACK_LEFT             BACK_RIGHT            //
//                                                                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////
// 8-bit special format                Normally     //
// [0..127]   negative  [0x0..0x7F]    [0x80..0xFF] //
// [128]      zero      [0x80]         [0x0]        //
// [129..255] positive  [0x81..0xFF]   [0x1..0x7F ] //
//////////////////////////////////////////////////////

//           Порядок каналов в           Название канала	       Цветовая маркировка
//   MP3 / WAV / FLAC       DTS / AAC      
//        0                    1	     Фронтальный левый         Белый
//        1                    2	     Фронтальный правый        Красный
//        2                    0	     Центральный               Зелёный
//        3                    5	     Низкочастотный            Пурпурный
//        4                    3	     Окружной левый            Голубой
//        5                    4	     Окружной правый           Серый
//        6                    6	     Окружной тыловой левый    Коричневый
//        7                    7	     Окружной тыловой правый   Хаки (жёлтый)

namespace SAVFGAME
{

	// PCM converter returning codes (UNT8)
	enum ePCMConvStatus : unsigned char
	{
		ePCMConvStatus_Normal,      // no errors, success convertion
		ePCMConvStatus_EndOfOrigin,	// reach end of origin buffer && loop == false

		ePCMConvStatus_EndOfTarget, // target buffer is too small for conversion
		ePCMConvStatus_NullFormat,	// in/out SNDDESC * ptr is nullptr
		ePCMConvStatus_NullBuffer,	// in/out PCM data ptr is nullptr
		ePCMConvStatus_NullSize,	// in/out PCM data size is 0
		ePCMConvStatus_MaxTalloc,	// incorrect settings : <maxTarget> mismatch <allocate>	
		ePCMConvStatus_NoChannels,	// invalid nChannels : allowed [1..8]
		ePCMConvStatus_NoFreq,		// invalid nSamplesPerSec : allowed [2..MAXSAMPLESPERSEC]
		ePCMConvStatus_NoBitsPerS,	// invalid wBitsPerSample : allowed [8, 16, 24, 32, 40, 48, 56, 64]
		ePCMConvStatus_NoFormatTag, // invalid wFormatTag : allowed WAVE_FORMAT_PCM (1), WAVE_FORMAT_IEEE_FLOAT (3), WAVE_FORMAT_EXTENSIBLE (0xFFFE)
		ePCMConvStatus_Unsign64bit,	// 40/48/56/64 bit samples should be signed //TEMPORARY ARRANGEMENT//
		ePCMConvStatus_MissBuffer,	// target buffer ptr located inside of origin buffer

		ePCMConvStatus_AllocErr_START,

		ePCMConvStatus_AllocTmpErr,	// can't allocate temp buffers
		ePCMConvStatus_AllocOutErr,	// can't allocate target buffer
		ePCMConvStatus_AllocMixErr, // [MIXER ERROR] can't allocate temp buffer 

		ePCMConvStatus_AllocErr_END,

		ePCMConvStatus_MixerErr_START,

		ePCMConvStatus_NullMixSrc,  // [MIXER ERROR] input SoundMixSource is nullptr
		ePCMConvStatus_BadInputs,   // [MIXER ERROR] all sources are invalid, AutoRun() failed
		ePCMConvStatus_NoInputs,    // [MIXER ERROR] additional sources vector is EMPTY , Run() / AutoRun() failed

		ePCMConvStatus_MixerErr_END
	};

	//>> Прототип конвертора
	class CPCMConverter_hint
	{
	protected:
		virtual UDOUBLE MixChannels(UDOUBLE c1, UDOUBLE c2) = 0;
		virtual void WriteTarget(uint64 sample) = 0;

	protected:
		template <class ...T>
		//>> Возвращает микшированный канал
		UDOUBLE MixChannels(UDOUBLE c1, UDOUBLE c2, T... channels)
		{
			static_assert (
				CHECK_T_TYPES_IS_SAME(T, UDOUBLE),
				"CPCMConverter::MixChannels() : All values must be UDOUBLE type"
			);

			return MixChannels(MixChannels(c1, c2), channels...);
		}

		template <class ...T>
		//>> Записывает семплы и двигает указатель target-буфера
		void WriteTarget(uint64 sample, T... samples)
		{
			static_assert (
				CHECK_T_TYPES_IS_SAME_2(T, uint64, UDOUBLE),
				"CPCMConverter::WriteTarget() : All values must be uint64 or UDOUBLE type"
			);

			WriteTarget(sample);
			WriteTarget(samples...);
		}
	};

	//>> Управление конвертированием потока PCM/IEEE   : в CalculateBase() не забыть позже для TBUFFER тоже проверку выделения памяти сделать
	class CPCMConverter : protected CPCMConverter_hint
	{
		using CPCMConverter_hint::MixChannels; // overload functions
		using CPCMConverter_hint::WriteTarget; // .

	public:
		struct SoundConvertSettings
		{
			friend class CPCMConverter;

			SoundConvertSettings(const SoundConvertSettings & src)            = delete;
			SoundConvertSettings(SoundConvertSettings && src)                 = delete;
			SoundConvertSettings& operator=(const SoundConvertSettings & src) = delete;
			SoundConvertSettings& operator=(SoundConvertSettings && src)      = delete;

			// input data
			struct SoundSet
			{
				void Reset()
				{
					buffer      = nullptr;
					size        = 0;
					samples     = 0;
					time        = 0;
					usetime     = false;
					BE          = false;
					user_signed = false;
					auto_signed = true;
					range       = eSDRangeDirectOut;
					volume      = nullptr;
				}

				void *			buffer      {nullptr}; // sound buffer ptr
				uint32			size        {0};       // sound buffer size (in bytes)
				uint32			samples     {0};       // start position [origin] or how many blocks to convert from origin [target]
				double			time        {0};       // start timepos [origin] or how much time to convert from origin [target] [in seconds]
				bool			usetime     {false};   // (default: false) use <time> instead of <samples>
				bool			BE          {false};   // (default: false) byte order <true> BE or <false> LE
				bool			user_signed {false};   // (default: false) user define <true> for signed samples or <false> for unsigned
				bool			auto_signed {true};    // (default: true)  <true> auto, <false> user defined
				eSoundDevRange	range       {eSDRangeDirectOut}; // additional info in case of 4ch, 5.1ch, 7.1ch
				float *         volume      {nullptr}; // [origin] every channel volume will be in target, [target] NONE , [TIP] volume_gain = 2^(dB/6) = 10^(dB/20) ;
			};

			// output data
			struct SoundInfo
			{
				void Reset()
				{
					origin_samples  = 0;
					origin_time     = 0;
					target_samples  = 0;
					target_time     = 0;

					start_time      = 0;
					start_sample    = 0;
					end_time_o      = 0;
					end_time_t      = 0;
					end_sample      = 0;

					buffer_time     = 0;
					buffer_samples  = 0;

					loop_end_time_o = 0;
					loop_end_time_t = 0;
					loop_end_sample = 0;

					origin_time_weight = 0;
					target_time_weight = 0;
				}

				void Printf()
				{
					printf("\norigin_samples  = %i" \
						   "\ntarget_samples  = %i" \
						   "\norigin_time     = %f" \
						   "\ntarget_time     = %f" \
						   "\nstart_sample    = %i" \
						   "\nend_sample      = %i" \
						   "\nstart_time      = %f" \
						   "\nend_time_o      = %f" \
						   "\nend_time_t      = %f" \
						   "\nbuffer_samples  = %i" \
						   "\nbuffer_time     = %f" \
						   "\nloop_end_sample = %i" \
						   "\nloop_end_time_o = %f" \
						   "\nloop_end_time_t = %f" \
						   "\norigin_time_weight = %f" \
						   "\ntarget_time_weight = %f",
						origin_samples,
						target_samples,
						origin_time,
						target_time,
						start_sample,
						end_sample,
						start_time,
						end_time_o,
						end_time_t,
						buffer_samples,
						buffer_time,
						loop_end_sample,
						loop_end_time_o,
						loop_end_time_t,
						origin_time_weight,
						target_time_weight
					);
				}

				uint32 origin_samples  {0}; // count of [origin] blocks used in converting
				double origin_time     {0}; // count of [origin] time   used in converting
				uint32 target_samples  {0}; // count of [target] blocks out
				double target_time     {0}; // count of [target] time   out

				double start_time      {0}; // absolute start timepoint at [origin]
				uint32 start_sample    {0}; // absolute start block at [origin] : { 0, 1, 2... }
				double end_time_o      {0}; // absolute end timepoint at [origin] by [origin] time-weight
				double end_time_t      {0}; // absolute end timepoint at [origin] by [target] time-weight
				uint32 end_sample      {0}; // absolute end block at [origin] : { 0, 1, 2... }

				double buffer_time     {0}; // total available [origin] time   in fact
				uint32 buffer_samples  {0}; // total available [origin] blocks in fact

				double loop_end_time_o {0}; // loop absolute end timepoint at [origin] by [origin] time-weight
				double loop_end_time_t {0}; // loop absolute end timepoint at [origin] by [target] time-weight
				uint32 loop_end_sample {0}; // loop absolute end block at [origin] : { 0, 1, 2... }

				double origin_time_weight {0}; // block weight in seconds [origin]
				double target_time_weight {0}; // block weight in seconds [target]
			};

			SoundSet  origin;	// settings of input data
			SoundSet  target;	// settings of output data
			SoundInfo outInfo;  // additional output info returned by converter
			bool      allocate  {false}; // allocate target buffer memory : default == false
			bool      loop      {false}; // loop [origin] : default == false
			bool      maxTarget {false}; // fill target as much as can [ target MUST BE pre-allocated ] : default == false
			bool      clear_origin {false}; // replace readed [origin] samples with DUMMY after conversion : default == false

		private:
			uint32 old_buffer_size { 0       }; // mem <target.size>
			void * old_buffer      { nullptr }; // mem <target.buffer>
			bool   old_allocate    { false   }; // mem <allocate>

		private:
			//>> allocate [target] memory
			byte * AllocateTarget(uint32 size)
			{
				//target.buffer = new byte [size];
				//target.size   = size;

				if (size)
				{
					target.buffer = malloc(size);
					if (target.buffer != nullptr)
						 target.size = size;
					else target.size = 0; // ERROR : CAN'T ALLOC MEM
				}
				else
				{
					target.buffer = nullptr;
					target.size   = 0;
				}

				// save [target] memory for further freeing
				old_buffer      = target.buffer;
				old_buffer_size = target.size;
				old_allocate    = true; //allocate;

				return reinterpret_cast<byte*>(target.buffer);
			}
			//>> try to free & forget [target] memory
			void FreeTarget()
			{
				if (old_allocate)
					//_DELETEN(old_buffer);
					_FREE(old_buffer);

				DropTarget();
			}
			//>> forget [target] memory
			void DropTarget()
			{
				old_allocate    = false;
				old_buffer      = nullptr;
				old_buffer_size = 0;
			}

		public:
			//>> set to default ( + trying to free target )
			void Reset()
			{
				FreeTarget();
				origin.Reset();
				target.Reset();
				outInfo.Reset();
				allocate  = false;
				loop      = false;
				maxTarget = false;
			}

			SoundConvertSettings() { }
			~SoundConvertSettings() { FreeTarget(); }
		};

	public:
		SoundConvertSettings settings;	// conversion order settings
		SNDDESC *            origin {nullptr};	// input  PCM/IEEE profile
		SNDDESC	*            target {nullptr};	// output PCM/IEEE profile
	private:
		bool bReCalcBase {true}; // recalc base values if PCM profile changes
		bool blank_run;          // indicate that [origin] is same as [target]
	public:
		ePCMConvStatus STATUS;   // this is memorized returning value of Run() proc.

	private: // base values	
		double timeWeightOrigin;     // sample weight in seconds [origin]
		double timeWeightTarget;     // sample weight in seconds [target]
		double timeWeightOrigin_moves; // time weight of step in 1 sec. portion [origin]
		double timeWeightTarget_moves; // time weight of step in 1 sec. portion [target]
		byte byteWeightOrigin_;      // sample weight in bytes, per channel [origin]
		byte byteWeightTarget_;      // sample weight in bytes, per channel [target]
		byte byteWeightOrigin;       // sample weight in bytes, all channels [origin]
		byte byteWeightTarget;       // sample weight in bytes, all channels [target]
		uint32 synchQ_origin;        // synch quantum for [origin] frequency (in samples)
		uint32 synchQ_target;	     // synch quantum for [target] frequency (in samples)
		bool   synchQ_minus;         // test
		uint32 _FreqOrigin;          // frequency minus PCM_ONE_FIX [origin]
		uint32 _FreqTarget;		     // frequency minus PCM_ONE_FIX [target]
		uint32 _FreqOrigin_moves;    // num of steps in 1 sec. portion (freq.-1) [origin]
		uint32 _FreqTarget_moves;    // num of steps in 1 sec. portion (freq.-1) [target]
		double ks;                   // used for convert O-bit value of [origin] to T-bit value of [target]
		double kch;				     // example: 1 / FF == 1 / maxSampleTarget
		double kch_half_max;		 // example: 1 / 7F == 1 / maxSampleTarget_half
		double kch_half_min;		 // example: 1 / 80 == 1 / (maxSampleTarget - maxSampleTarget_half)
		uint64 maxSampleOrigin;		 // 0xFF or 0xFFFF or etc.
		uint64 maxSampleTarget;		 // 0xFF or 0xFFFF or etc.
		uint64 maxSampleOrigin_half; // maxSampleOrigin / 2 == 0x7F or 0x7FFF or etc.
		uint64 maxSampleTarget_half; // maxSampleTarget / 2 == 0x7F or 0x7FFF or etc.
		TBUFFER <UDOUBLE, WORD> sampleOrigin_prev;      // mem [origin] samples from prev step
		TBUFFER <UDOUBLE, WORD> sampleOrigin_prev_temp; // mem [origin] samples from prev step (temp)
		TBUFFER <UDOUBLE, WORD> sampleTarget_prev;      // mem [target] samples from prev step

	private: // cycle inputs
		uint32 startSample;        // start block at [origin]
		uint32 samplesToConvert;   // count of blocks need to convert from [origin]
		uint32 samplesConverted;   // count of blocks will be at [target]
		bool   origin_signed;      // indicate signed [origin] samples
		bool   target_signed;	   // indicate signed [target] samples
		bool   origin_8bit_format; // indicate special 8-bit format [origin]
		bool   target_8bit_format; // indicate special 8-bit format [target]
		bool   origin_32bit_float; // indicate special 32-bit float format [origin]
		bool   target_32bit_float; // indicate special 32-bit float format [origin]
		UNION32 volume_change;     // indicate that out channel volume is not 1.f
	
	private: // cycle temps
		byte * _from;              // buffer [origin]
		byte * _from_end;          // buffer [origin] end
		byte * _to;                // buffer [target]
		double timeOrigin;         // time position of [origin]
		double timeOrigin_prev;    // time position of [origin] (prev step)
		double timeTarget;         // time position of [target]
		uint32 counterTarget;      // count how many samples converted [target]
		uint32 counterFreqOrigin;  // count samples up to full second [origin]
		uint32 counterFreqTarget;  // count samples up to full second [target]
		uint32 counterSynchOrigin; // count samples up to synchQ_origin
		uint32 counterSynchTarget; // count samples up to synchQ_target

	private: // special
		UDOUBLE       _silence_ { 0ui64 };                   // muted sample
	//	UDOUBLE       f_sample  { 0 };                   // float sample
	//	UFLOAT        f_sample;                          // float sample
	//	int32         sample_i;                          // sample_i as int32
	//	float * const sample_f  { (float*) & sample_i }; // sample_i as float

	public:
		CPCMConverter(const CPCMConverter & src)            = delete;
		CPCMConverter(CPCMConverter && src)                 = delete;
		CPCMConverter& operator=(const CPCMConverter & src) = delete;
		CPCMConverter& operator=(CPCMConverter && src)      = delete;

	public:
		CPCMConverter() : CPCMConverter_hint() {};
		~CPCMConverter(){};

	private:
		//>> Вычисляет коэффициенты веса семпла
		void GetSampleMultiplier(uint16 in_bits, uint16 out_bits)
		{
			maxSampleOrigin = 0;
			maxSampleTarget = 0;

			for (int x = in_bits  - 1; x >= 0; x--)
				maxSampleOrigin |= (uint64) 1 << x;

			for (int x = out_bits - 1; x >= 0; x--)
				maxSampleTarget |= (uint64) 1 << x;

			maxSampleTarget_half = maxSampleTarget / 2;	 // FF / 2 = 7F
			maxSampleOrigin_half = maxSampleOrigin / 2;	 // FF / 2 = 7F

			///////////////////////////////////////////////////////////////////

			kch = (double)1 /  maxSampleTarget;										// 1 / FF

			ks = (double)maxSampleTarget / maxSampleOrigin;							// FFFF / FF ; FF / FFFF

			kch_half_max = (double)1 /  maxSampleTarget_half;						// 1 / 7F

			kch_half_min = (double)1 / (maxSampleTarget - maxSampleTarget_half);	// 1 / 80 = 1 / (FF - 7F)
		}

	/*	//>> Вычисляет два кванта времени до общей точки синхронизации [DEBUG]
		void GetSynchQuantum_test()
		{
			//  0     1     2     3  [4]    synchQ_minus = true
			//  | . . | . . | . . |         Q1 = 1
			//  0 1 2 3 4 5 6 7 8 9  [10]   Q2 = 3
			//
			//     0     1     2     [3]    synchQ_minus = false
			//  |  .  |  .  |  .  x         Q1 = 1
			//   0  1  2  3  4  5    [6]    Q2 = 2

			uint32 synchQ_origin_temp[2];
			uint32 synchQ_target_temp[2];

			for (uint32 step = 0; step < 2; step++)
			{
				uint32 freq1 = origin->nSamplesPerSec;
				uint32 freq2 = target->nSamplesPerSec;

				if (step)
				{
					freq1--;
					freq2--;
				}

				for(;;)
				{
					uint32 freq1_mem = freq1;
					uint32 freq2_mem = freq2;

					//while (freq1 != freq2)
					//{
					//	if (freq1 > freq2)
					//		 freq1 = freq1 - freq2;
					//	else freq2 = freq2 - freq1;
					//}
					//uint32 NOD = freq1;

					while (freq1 && freq2)
					{
						if (freq1 > freq2)
							 freq1 = freq1 % freq2;
						else freq2 = freq2 % freq1;
					}
					uint32 NOD = freq1 + freq2; // greatest common divisor (gcd)

					if (NOD == 1)
					{
						synchQ_origin_temp[step] = freq1_mem;
						synchQ_target_temp[step] = freq2_mem;
						break;
					}
					else
					{
						freq1 = freq1_mem / NOD;
						freq2 = freq2_mem / NOD;
					}
				}
			}

			if (synchQ_origin_temp[1] <= synchQ_origin_temp[0] &&
				synchQ_target_temp[1] <= synchQ_target_temp[0])
			{
				synchQ_origin = synchQ_origin_temp[1];
				synchQ_target = synchQ_target_temp[1];
				synchQ_minus  = true;
			}
			else
			{
				synchQ_origin = synchQ_origin_temp[0];
				synchQ_target = synchQ_target_temp[0];
				synchQ_minus  = false;
			}
		} //*/

		//>> Вычисляет два кванта времени до общей точки синхронизации
		void GetSynchQuantum()
		{
			uint32 freq1 = origin->nSamplesPerSec - PCM_ONE_FIX;
			uint32 freq2 = target->nSamplesPerSec - PCM_ONE_FIX;

#ifdef PCM_DISABLE_INTERMEDIATE
			synchQ_origin = freq1;
			synchQ_target = freq2;
			return;
#endif

			for (;;)
			{
				uint32 freq1_mem = freq1;
				uint32 freq2_mem = freq2;

				while (freq1 && freq2)
				{
					if (freq1 > freq2)
						 freq1 = freq1 % freq2;
					else freq2 = freq2 % freq1;
				}
				uint32 NOD = freq1 + freq2; // greatest common divisor (gcd)

				if (NOD == 1)
				{
					synchQ_origin = freq1_mem;
					synchQ_target = freq2_mem;
					return;
				}
				else
				{
					freq1 = freq1_mem / NOD;
					freq2 = freq2_mem / NOD;
				}
			}
		}

	protected:
		//>> Читает семпл и двигает указатель буфера
		virtual void ReadOrigin(byte *& _from, uint64 & sample, byte weight, bool BE)
		{
			// ЗАМЕТКА
			// В миксере Mix() после чтения конвертирует 8-битные семплы
			// Если вставить сюда авто-конвертирование, в миксере надо будет убрать
			// Либо создать bool признак состояния и отслеживать его

			sample = 0;
		
			// take every 8 bits (1 byte) of sample

			if (BE)
				for (int x = weight - 1; x >= 0; x--)
				{
					sample |= ((uint64)(*_from) << (x * 8));
					_from++;
				}
			else
				for (int x = 0; x < weight; x++)
				{
					sample |= ((uint64)(*_from) << (x * 8));
					_from++;
				}
		}

		//>> Записывает семпл и двигает указатель буфера
		virtual void WriteTarget(byte *& _to, uint64 sample, byte weight, bool BE)
		{
			// ЗАМЕТКА
			// В миксере Mix() не конвертирует 8-битные семплы при записи
			// Если отсюда убрать конвертирование, надо будет править миксер

			// special <-> standart
			if (target_8bit_format)
				ConvertSpecial ( //maxSampleTarget_half, 
				sample );

			// write every 8 bits (1 byte) of sample

			if (BE)
				for (int x = weight - 1; x >= 0; x--)
				{
					*_to = (byte)(sample >> (x * 8));
					_to++;
				}
			else
				for (int x = 0; x < weight; x++)
				{
					*_to = (byte)(sample >> (x * 8));
					_to++;
				}
		}

	private:
		//>> Читает семпл и двигает указатель буфера
		void ReadOrigin(byte *& _from, uint64 & sample)
		{
			ReadOrigin (
				std::forward<byte *&>(_from),
				std::forward<uint64 &>(sample),
				std::forward<byte>(byteWeightOrigin_),
				std::forward<bool>(settings.origin.BE)
			);
		}

		//>> Записывает семпл и двигает указатель target-буфера
		void WriteTarget(uint64 sample) override final
		{
			WriteTarget (
				std::forward<byte *&>(_to),
				std::forward<uint64 &>(sample),
				std::forward<byte>(byteWeightTarget_),
				std::forward<bool>(settings.target.BE)
			);
		}

	/*	//>> Пересчитывает значение семпла, приводя от [origin] к [target] битности [DEBUG]
		void ChangeWeight_test(uint64 & sample)
		{
			uint32 i, steps, steps_8;
			uint64 min, max, temp, temp_prev;
			double f_ks[8];
			bool min_to_max;

			if (maxSampleOrigin > maxSampleTarget)
			{
				max = maxSampleOrigin;
				min = maxSampleTarget;
				min_to_max = false;
			}
			else
			{
				max = maxSampleTarget;
				min = maxSampleOrigin;
				min_to_max = true;
			}

			steps = 0;
			temp = max;
			while (temp != min)
			{
				temp = (temp >> 1);
				steps++;
			}

			steps_8 = 0;
			temp = max;
			i = 0;
			while (temp != min)
			{
				temp_prev = temp;

				for (bool br = 0; i < steps; i++)
				{
					if (br && !(i % 8)) break;
					temp = temp >> 1;
					br = 1;
				}

				if (min_to_max)
					 f_ks[steps_8] = (std::decay_t<decltype(f_ks[0])>) temp_prev / temp;
				else f_ks[steps_8] = (std::decay_t<decltype(f_ks[0])>) temp / temp_prev;

				steps_8++;
			}

			for (i = 0; i < steps_8; i++)
				sample = (uint64)(f_ks[i] * sample);
		} //*/

		//>> Пересчитывает значение семпла, приводя от [origin] к [target] битности
		void ChangeWeight(uint64 & sample)
		{
			sample = (uint64)(ks * sample);
		}

		//>> Изменяет громкость в [target] : APPLYING POST-CONVERSION EFFECTS :
		void ChangeVolume()
		{
			//        (-MAX)   (0)     (+MAX)
			// uint8  = 0..0x______80..0xFF______
			//          (0)    ...     (+MAX)
			// uint16 = 0..0x____8000..0xFFFF____
			// uint24 = 0..0x__800000..0xFFFFFF__
			// uint32 = 0..0x80000000..0xFFFFFFFF
			//              (-MAX)  (-1)       (0) (+1) (+MAX)
			//  int16 = 0x____8000..0xFFFF____, 0, 0x1..0x7FFF____
			//  int24 = 0x__800000..0xFFFFFF__, 0, 0x1..0x7FFFFF__
			//  int32 = 0x80000000..0xFFFFFFFF, 0, 0x1..0x7FFFFFFF
			//
			//  float = -1.f..0..+1.f
			//
			//  OUT_SAMPLE = _CLAMP((abs(IN_SAMPLE) * abs(MULTIPLIER)), 0, +MAX) * SIGN_OF_SAMPLE

			byte * ptr      = (byte*) settings.target.buffer;
			byte * ptr_read = ptr;
			byte numCh      = (byte) target->nChannels;
			bool BE         = settings.target.BE;
			byte weight     = byteWeightTarget_; // 1..4

			float multiplier[8];
			UDOUBLE sample;

			for (byte ch = 0; ch < numCh; ch++)
				multiplier[ch] = abs( settings.origin.volume[ch] );

			for (uint32 step = 0; step < samplesConverted; step++)
			{			
				ptr = ptr_read;

				for (byte ch = 0; ch < numCh; ch++)
				{
					// skip 1.f volume
					{
						uint32 BITMASK = 1ul << ch;

						if (!(volume_change.u32 & BITMASK))
						{
							ptr_read += weight;
							continue;
						}
					}

					byte * ptr_ch = ptr_read;

					////////// READ //////////

					sample.u64 = 0;

					if (BE)
						for (int x = weight - 1; x >= 0; x--)
						{
							sample.u64 |= ((uint64)(*ptr_read) << (x * 8));
							ptr_read++;
						}
					else
						for (int x = 0; x < weight; x++)
						{
							sample.u64 |= ((uint64)(*ptr_read) << (x * 8));
							ptr_read++;
						}

					////////// APPLY VOLUME //////////

					if (target_32bit_float)
					{
						int sign = (sample.f32_1 < 0) ? -1 : +1 ;

						sample.f32_1 = abs( sample.f32_1 ) * multiplier[ch]  ;
						sample.f32_1 = _CLAMP(sample.f32_1, 0.f, 1.f) * sign ;
					}
					else if (target_8bit_format)
					{
						bool positive = (sample.u64 >= PCM_NULL8BIT) ;
						uint64 abs;
						float  abs_f;

						const float aa = 1.f / 0x7F; // kch_half_max

						if (positive)
							 abs = sample.u64 - PCM_NULL8BIT; // [0x80..0xFF] - 0x80
						else abs = PCM_NULL8BIT - sample.u64; // 0x80 - [0x00..0x7F]

						abs_f = aa * abs;
						abs_f = abs_f * multiplier[ch];
						abs_f = _CLAMP(abs_f, 0.f, 1.f);

						abs = (uint64)(abs_f * 0x7F);

						if (positive) 
							 sample.u64 = abs + PCM_NULL8BIT; // [0..7F] + 0x80 = 0x80..0xFF
						else sample.u64 = PCM_NULL8BIT - abs; // 0x80 - [1..7F] = 0x7F..0x01
					}
					else
					{
						if (target_signed)
						{
							uint64 abs;   // 0..0x7FFFFFFF
							double abs_f;
							bool positive = (sample.u64 <= maxSampleTarget_half) ;

							if (positive)
								 abs = sample.u64;
							else abs = (maxSampleTarget - sample.u64) + 1; // MulMinus()

							abs_f = kch_half_max * abs; // kch * abs;
							abs_f = abs_f * multiplier[ch];
							abs_f = _CLAMP(abs_f, 0., 1.);

							abs = (uint64)(abs_f * maxSampleTarget_half); // maxSampleTarget);

							if (positive)
								 sample.u64 = abs;
							else sample.u64 = (maxSampleTarget - abs) + 1; // MulMinus()
						}
						else // unsigned
						{
							double abs_f;

							abs_f = kch * sample.u64;
							abs_f = abs_f * multiplier[ch];
							abs_f = _CLAMP(abs_f, 0., 1.);

							sample.u64 = (uint64) (abs_f * maxSampleTarget) ;
						}
					}

					////////// WRITE //////////

					if (BE)
						for (int x = weight - 1; x >= 0; x--)
						{
							*ptr_ch = (byte)(sample.u64 >> (x * 8));
							ptr_ch++;
						}
					else
						for (int x = 0; x < weight; x++)
						{
							*ptr_ch = (byte)(sample.u64 >> (x * 8));
							ptr_ch++;
						}
				}
			}
		}

	private:
		//>> Аналог умножения на -1
		uint64 MulMinus(uint64 limit, uint64 sample)
		{
			// [80..FF] -128..-1  ::  +128..+1 [80..1]

			return (limit - sample) + 1;
		}

	protected:
		//>> Конвертирует особую запись 8-битовых семплов в привычный вид
		void ConvertSpecial(uint64 limit_half, uint64 & sample)
		{
			//      SPECIAL                STANDART
			// [   -1, 0, 1    ]      [   -1,  0, 1    ]
			// [0..7F,80,81..FF] <--> [80..FF, 0, 1..7F]

			if (sample >= limit_half + 1)
				 sample -= limit_half + 1;
			else sample += limit_half + 1;
		}

		//>> Конвертирует особую запись 8-битовых семплов в привычный вид
		virtual void ConvertSpecial(uint64 & sample)
		{
			//      SPECIAL                STANDART
			// [   -1, 0, 1    ]      [   -1,  0, 1    ]
			// [0..7F,80,81..FF] <--> [80..FF, 0, 1..7F]

			if  (sample >= PCM_NULL8BIT)
				 sample -= PCM_NULL8BIT;
			else sample += PCM_NULL8BIT;
		}

	private:
	//	//>> Конвертирует int32 во float
	//	void ConvertToFloat(uint64 & sample)
	//	{
	//		const float LIMIT = 1.f / 0x7FFFFFFF;
	//		const uint64 MASK = 0xFFFFFFFF;
	//
	//	//	(*sample_f) = LIMIT * (int32)sample;
	//	//	(*sample_f) = _CLAMP((*sample_f), -1.f, +1.f);
	//	//	  sample    = ((uint64)sample_i) & MASK;
	//
	//	//	f_sample.f  = LIMIT * (int32)sample;
	//	//	f_sample.f  = _CLAMP(f_sample.f, -1.f, +1.f);
	//	//	  sample    = ((uint64)f_sample.i) & MASK;
	//
	//		f_sample.f32_1 = LIMIT * (int32)sample;
	//		f_sample.f32_1 = _CLAMP(f_sample.f32_1, -1.f, +1.f);
	//		  sample       = f_sample.i64;
	//	}

	//	//>> Конвертирует float в int32
	//	void ConvertFromFloat(uint64 & sample)
	//	{
	//		const  int32 LIMIT = 0x7FFFFFFF;
	//		const uint64 MASK  = 0xFFFFFFFF;
	//
	//	//	  sample_i  = (int32) sample;
	//	//	(*sample_f) = _CLAMP((*sample_f), -1.f, +1.f);
	//	//	  sample    = ((uint64)((*sample_f) * LIMIT)) & MASK;
	//
	//	//	f_sample.i  = (int32) sample;
	//	//	f_sample.f  = _CLAMP(f_sample.f, -1.f, +1.f);
	//	//	  sample    = ((uint64)(f_sample.f * LIMIT)) & MASK;
	//
	//		f_sample.i64   = sample;
	//		f_sample.f32_1 = _CLAMP(f_sample.f32_1, -1.f, +1.f);
	//		  sample       = (uint64)(f_sample.f32_1 * LIMIT);
	//	}

	//	//>> Извлекает float-значение из записи
	//	float ExtractFloat(uint64 & sample)
	//	{
	//		const uint64 MASK = 0xFFFFFFFF;
	//
	//	//	sample_i = (int32)(sample & MASK);
	//	//	return (*sample_f);
	//
	//	//	f_sample.i = (int32)(sample & MASK);
	//	//	return f_sample.f;
	//
	//		f_sample.i64 = sample;
	//		return f_sample.f32_1;
	//	}

	//	//>> Вставляет float-значение в запись
	//	void PackFloat(float value, uint64 & sample)
	//	{
	//		const uint64 MASK = 0xFFFFFFFF;
	//
	//	//	(*sample_f) = value;
	//	//	sample = ((uint64)sample_i) & MASK;
	//
	//	//	f_sample.f = value;
	//	//	sample = ((uint64)f_sample.i) & MASK;
	//
	//		f_sample.f32_1 = value;
	//		sample = f_sample.i64;
	//	}

	//	//>> Возвращает float-значение в виде записи
	//	uint64 PackFloat(float value)
	//	{
	//		const uint64 MASK = 0xFFFFFFFF;
	//
	//	//	(*sample_f) = value;
	//	//	return ((uint64)sample_i) & MASK;
	//
	//	//	f_sample.f = value;
	//	//	return ((uint64)f_sample.i) & MASK;
	//
	//		f_sample.f32_1 = value;
	//		return f_sample.i64;
	//	}

	private:
		//>> Вычисляет базовые данные на основе PCM профиля
		bool CalculateBase()
		{
			if (!bReCalcBase) return true;

			_FreqOrigin = origin->nSamplesPerSec - PCM_ONE_FIX;
			_FreqTarget = target->nSamplesPerSec - PCM_ONE_FIX;

			_FreqOrigin_moves = _FreqOrigin;
			_FreqTarget_moves = _FreqTarget;

#ifdef PCM_SEPARATE_SYNCH_1S_DEBUG
			_FreqOrigin_moves--; // nSamplesPerSec - 1
			_FreqTarget_moves--; // nSamplesPerSec - 1
#endif
			timeWeightOrigin = (decltype(timeWeightOrigin)) 1 / _FreqOrigin;
			timeWeightTarget = (decltype(timeWeightTarget)) 1 / _FreqTarget;

			timeWeightOrigin_moves = (decltype(timeWeightOrigin_moves)) 1 / _FreqOrigin_moves;
			timeWeightTarget_moves = (decltype(timeWeightTarget_moves)) 1 / _FreqTarget_moves;

			byteWeightOrigin_ = origin->wBitsPerSample / 8;
			byteWeightTarget_ = target->wBitsPerSample / 8;

			byteWeightOrigin = byteWeightOrigin_ * origin->nChannels; 
			byteWeightTarget = byteWeightTarget_ * target->nChannels;

			// alloc temp buffers
			{
				bool malloc_success = true;
				WORD num;

				num = origin->nChannels;

				if (sampleOrigin_prev.count      < num)
					malloc_success &= sampleOrigin_prev.      Create (num);

				if (sampleOrigin_prev_temp.count < num)
					malloc_success &= sampleOrigin_prev_temp. Create (num);

				num = (target->nChannels > num) ? target->nChannels : num;

				if (sampleTarget_prev.count      < num)
					malloc_success &= sampleTarget_prev.      Create (num);
			
				if (!malloc_success)
				{
					STATUS = ePCMConvStatus_AllocTmpErr;
					return false;
				}
			}

			GetSampleMultiplier(byteWeightOrigin_ * 8, byteWeightTarget_ * 8);

			GetSynchQuantum();

			//target->wFormatTag      = WAVE_FORMAT_PCM ? WAVE_FORMAT_EXTENSIBLE ? WAVE_FORMAT_IEEE_FLOAT
			target->nBlockAlign     = target->nChannels * target->wBitsPerSample / 8;
			target->nAvgBytesPerSec = target->nSamplesPerSec * target->nBlockAlign;
			//target->cbSize          = 0 ? N

			bReCalcBase = false;

			return true;
		}

		//>> Вычисляет входные данные для текущего прохода
		void CalculateInput()
		{
			if (settings.origin.usetime) // start at nearest early sample 
				 startSample = (uint32) (settings.origin.time * _FreqOrigin);
			else startSample = (uint32)  settings.origin.samples;

			if (settings.target.usetime) // round up
				 samplesToConvert = (uint32)  ceil(settings.target.time * _FreqOrigin) + PCM_ONE_FIX;
			else samplesToConvert = (uint32)  settings.target.samples;

			if (settings.target.usetime)
				 samplesConverted = (uint32) ((settings.target.time)                                * _FreqTarget) + PCM_ONE_FIX;
			else samplesConverted = (uint32) ((timeWeightOrigin * (samplesToConvert - PCM_ONE_FIX)) * _FreqTarget) + PCM_ONE_FIX;

			// Возможная ошибка выхода за пределы буфера из-за float-вычислений при заказе <maxTarget>
			if (settings.maxTarget && settings.target.size < (samplesConverted * byteWeightTarget))
			{
				double timeWeightTarget_001 = timeWeightTarget * 0.01f;
				do {			
					settings.target.time -= timeWeightTarget_001; // small decrease of time
					samplesConverted = (uint32) (settings.target.time * _FreqTarget) + PCM_ONE_FIX;
				} while
					(settings.target.size < (samplesConverted * byteWeightTarget));
				samplesToConvert = (uint32) ceil(settings.target.time * _FreqOrigin) + PCM_ONE_FIX;
			}

			origin_8bit_format = (origin->wBitsPerSample <= 8);
			target_8bit_format = (target->wBitsPerSample <= 8);

			origin_signed = true;
			if (!settings.origin.auto_signed) // if user defined
				if (!origin_8bit_format)
					origin_signed = settings.origin.user_signed;

			target_signed = true;
			if (!settings.target.auto_signed) // if user defined
				if (!target_8bit_format)
					target_signed = settings.target.user_signed;

			origin_32bit_float = (origin->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) &&
								 (origin->wBitsPerSample == 32) &&
								 origin_signed;

			target_32bit_float = (target->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) &&
								 (target->wBitsPerSample == 32) &&
								 target_signed;
		}

		//>> Вычисляет выходные данные для текущего прохода
		void CalculateOutput()
		{
			settings.outInfo.origin_time_weight = timeWeightOrigin;
			settings.outInfo.target_time_weight = timeWeightTarget;

			settings.outInfo.origin_samples = samplesToConvert;
			settings.outInfo.target_samples = samplesConverted;

			// N = (nSamplesPerSec - 1) * seconds + 1
			// seconds = (N - 1) / (nSamplesPerSec - 1)

			uint32 origin_full_seconds = (samplesToConvert - PCM_ONE_FIX) / _FreqOrigin;
			uint32 target_full_seconds = (samplesConverted - PCM_ONE_FIX) / _FreqTarget;
			uint32 origin_partial_samples = (samplesToConvert - PCM_ONE_FIX) - (origin_full_seconds * _FreqOrigin);
			uint32 target_partial_samples = (samplesConverted - PCM_ONE_FIX) - (target_full_seconds * _FreqTarget);

			settings.outInfo.origin_time  = timeWeightOrigin * origin_partial_samples;
			settings.outInfo.origin_time += origin_full_seconds;
			settings.outInfo.target_time  = timeWeightTarget * target_partial_samples;
			settings.outInfo.target_time += target_full_seconds;

			///////////

			uint32 start_full_seconds    = startSample / _FreqOrigin; // startSample = 0, 1, 2...
			uint32 start_partial_samples = startSample - (start_full_seconds * _FreqOrigin);

			settings.outInfo.start_time  = timeWeightOrigin * start_partial_samples;
			settings.outInfo.start_time += start_full_seconds;
			settings.outInfo.end_time_o  = settings.outInfo.start_time + settings.outInfo.origin_time;
			settings.outInfo.end_time_t  = settings.outInfo.start_time + settings.outInfo.target_time;

			settings.outInfo.start_sample = startSample;
			settings.outInfo.end_sample   = startSample + samplesToConvert - PCM_ONE_FIX;

			///////////

			settings.outInfo.buffer_samples = settings.origin.size / byteWeightOrigin;

			uint32 buffer_full_seconds    = (settings.outInfo.buffer_samples - PCM_ONE_FIX) / _FreqOrigin;
			uint32 buffer_partial_samples = (settings.outInfo.buffer_samples - PCM_ONE_FIX) - (buffer_full_seconds * _FreqOrigin);

			settings.outInfo.buffer_time  = timeWeightOrigin * buffer_partial_samples;
			settings.outInfo.buffer_time += buffer_full_seconds;

			///////////

			if (settings.loop)
			{
				settings.outInfo.loop_end_sample = settings.outInfo.end_sample % (settings.outInfo.buffer_samples - PCM_ONE_FIX);
				settings.outInfo.loop_end_time_o = settings.outInfo.end_time_o -
					((uint32)(settings.outInfo.end_time_o / settings.outInfo.buffer_time)) * settings.outInfo.buffer_time;
				settings.outInfo.loop_end_time_t = settings.outInfo.end_time_t -
					((uint32)(settings.outInfo.end_time_t / settings.outInfo.buffer_time)) * settings.outInfo.buffer_time;
			}
			else
			{
				settings.outInfo.loop_end_sample = settings.outInfo.end_sample;
				settings.outInfo.loop_end_time_o = settings.outInfo.end_time_o;
				settings.outInfo.loop_end_time_t = settings.outInfo.end_time_t;
			}
		}

		//>> Сбрасывает временные переменные для начала прохода
		bool ResetTemps()
		{
			_from = (byte*) settings.origin.buffer;
			_to   = (byte*) settings.target.buffer;

			_from_end = _from + settings.origin.size;

			timeOrigin         = 0;
			timeOrigin_prev    = 0;
			timeTarget         = 0;

			counterTarget      = 0;
			counterFreqOrigin  = 0;
			counterFreqTarget  = 0;
			counterSynchOrigin = 0;
			counterSynchTarget = 0;

			if (settings.allocate)
			{
				uint32 alloc_size = samplesConverted * byteWeightTarget;

				if (alloc_size)
				{
					if (settings.old_allocate) // есть старый, когда-то созданный буфер
					{				
						if (settings.old_buffer_size >= alloc_size) // если хватает размера старого выделенного
						{
							settings.target.buffer = settings.old_buffer; // старый ptr
							settings.target.size   = alloc_size;          // размер текущего прохода
						}
						else // иначе - нужно выделить больше
						{						
							_FREE(settings.old_buffer);    // удаление старого
							settings.old_buffer_size = 0;  // .
							settings.old_allocate = false; // .

							void * new_buffer = malloc(alloc_size);

							if (new_buffer)
							{
								settings.old_allocate    = true;
								settings.old_buffer      = new_buffer;
								settings.old_buffer_size = alloc_size;
								settings.target.buffer   = new_buffer;
								settings.target.size     = alloc_size;
							}
							else // ERROR : ALLOC MEM FAIL
							{
								settings.target.buffer = nullptr;
								settings.target.size   = 0;
							}
						}

						_to = reinterpret_cast<byte*>(settings.target.buffer);
					}
					else // никакого буфера нет - нужно создать и запомнить его
					{
						_to = settings.AllocateTarget(alloc_size);
					}

					if (_to == nullptr)
						{ STATUS = ePCMConvStatus_AllocOutErr; return false; }
				}
				else // alloc_size == 0   (trying to convert nothing)
				{
					settings.target.buffer = nullptr;
					settings.target.size   = 0;
					_to = nullptr;
				}
			}

			return true;
		}

	protected:
		//>> Возвращает микшированный канал
		virtual UDOUBLE MixChannels(UDOUBLE c1, UDOUBLE c2) override
		{
			if (!c1) return c2;
			if (!c2) return c1;

			//uint64 ret;
			UDOUBLE ret;

			// DEBUG TEST : Какой вариант лучше?
			#define __mix_float_as_int32__ false

			// MIN..0..MAX
			// x = (a+b) - (a*b/MIN), all negative   a<0 && b<0
			// x = (a+b) - (a*b/MAX), all positive   a>0 && b>0
			// x = (a+b),             negative and positive

			if (target_32bit_float) // float samples
			{
				if (!__mix_float_as_int32__) // возврат прямого float-микса
				{
				//	float c1_f = ExtractFloat(c1);
				//	float c2_f = ExtractFloat(c2);
				//	float c_f;
				//
				//	bool c1_negative = (c1_f < 0);
				//	bool c2_negative = (c2_f < 0);
				//
				//	if ((!c1_negative && !c2_negative) || // same sign
				//		( c1_negative &&  c2_negative))
				//	{
				//		c_f = c1_f + c2_f - c1_f * c2_f;
				//	}
				//	else // positive and negative
				//	{
				//		c_f = c1_f + c2_f;
				//	}
				//
				//	c_f = _CLAMP(c_f, -1.f, +1.f);
				//
				//	return PackFloat(c_f); // __EXIT__

					bool c1_negative = (c1.f32_1 < 0);
					bool c2_negative = (c2.f32_1 < 0);

					if ((!c1_negative && !c2_negative) || // same sign
						( c1_negative &&  c2_negative))
					{
						c1.f32_1 = c1.f32_1 + c2.f32_1 - c1.f32_1 * c2.f32_1;
					}
					else // positive and negative
					{
						c1.f32_1 = c1.f32_1 + c2.f32_1;
					}

					c1.f32_1 = _CLAMP(c1.f32_1, -1.f, +1.f);

					return c1; // __EXIT__

				}
				else // идём дальше через целые числа
				{
					F2I(c1);
					F2I(c2);
				}
			}		

			if (!target_signed) // unsigned samples
			{
				ret = c1 + c2 - (uint64)(kch * c1 * c2);
			}
			else // signed samples
			{
				bool c1_negative = (c1 > maxSampleTarget_half) ;
				bool c2_negative = (c2 > maxSampleTarget_half) ;

				if (!c1_negative && !c2_negative) // all positive
				{
					ret = c1 + c2 - (uint64)(kch_half_max * c1 * c2);
				}
				else if (c1_negative && c2_negative) // all negative
				{
					uint64 abs_c1_negative = MulMinus(maxSampleTarget, c1);
					uint64 abs_c2_negative = MulMinus(maxSampleTarget, c2);

					ret = abs_c1_negative + abs_c2_negative -
						(uint64)(kch_half_min * abs_c1_negative * abs_c2_negative);

					ret = MulMinus(maxSampleTarget, ret); // back from abs to negative
				}
				else // positive and negative
				{
					uint64 c_positive;
					uint64 c_negative;

					if (c1_negative)
					{
						c_positive = c2;
						c_negative = c1;
					}
					else
					{
						c_positive = c1;
						c_negative = c2;
					}

					uint64 abs_c_negative = MulMinus(maxSampleTarget, c_negative);
					byte abs_c_negative_fix = 0; // fix -7F to -80 later

					if (abs_c_negative > maxSampleTarget_half) // 80 > 7F (positive max is 7F)
					{
						abs_c_negative = maxSampleTarget_half;
						abs_c_negative_fix = 1; 
					}

					if (c_positive >= abs_c_negative) // return positive value [0..7F]
					{
						ret = (c_positive - abs_c_negative) - abs_c_negative_fix;
					}
					else // return negative value [80..FF]
					{
						ret = MulMinus(maxSampleTarget, abs_c_negative - c_positive) - abs_c_negative_fix;
					}
				}
			}

			if (target_32bit_float && __mix_float_as_int32__)
			{
				I2F(ret);
			}

			return ret;

			#undef __mix_float_as_int32__
		}

	private:
		//>> Процедура конвертирования и записи каналов из 1 : MONO
		void MixChannels_1(uint32 target_channels)
		{
			auto & front_center = sampleTarget_prev[0];

			switch (target_channels)
			{
			case 1: // MONO to MONO
				{
					WriteTarget(front_center); // FRONT_CENTER
					break;
				}
			case 2: // MONO to STEREO
			case 3:
				{
					WriteTarget(front_center,  // FRONT_LEFT
								front_center); // FRONT_RIGHT

					if (target_channels == 3)
						WriteTarget(_silence_);

					break;
				}
			case 4: // MONO to QUAD / SURROUND
			case 5:
				{
					WriteTarget(front_center,  // FRONT_LEFT
								front_center,  // FRONT_RIGHT
								front_center,  // BACK_LEFT  (QUAD) or FRONT_CENTER (SURROUND)
								front_center); // BACK_RIGHT (QUAD) or BACK_CENTER  (SURROUND)

					if (target_channels == 5)
						WriteTarget(_silence_);

					break;
				}
			case 6: // MONO to 5.1 / 5.1 SURROUND
			case 7:
				{
					WriteTarget(front_center,  // FRONT_LEFT
								front_center,  // FRONT_RIGHT
								front_center,  // FRONT_CENTER
								_silence_,     // LOW_FREQUENCY
								front_center,  // BACK_LEFT  or SIDE_LEFT  (SURROUND)
								front_center); // BACK_RIGHT or SIDE_RIGHT (SURROUND)

					if (target_channels == 7)
						WriteTarget(_silence_);

					break;
				}
			case 8: // MONO to 7.1 / 7.1 SURROUND
				{
					WriteTarget(front_center,  // FRONT_LEFT
								front_center,  // FRONT_RIGHT
								front_center,  // FRONT_CENTER
								_silence_,     // LOW_FREQUENCY
								front_center,  // BACK_LEFT  or SIDE_LEFT  (SURROUND)
								front_center,  // BACK_RIGHT or SIDE_RIGHT (SURROUND)
								front_center,  // FRONT_LEFT_OF_CENTER  or BACK_LEFT  (SURROUND)
								front_center); // FRONT_RIGHT_OF_CENTER or BACK_RIGHT (SURROUND)
					break;
				}
			default:
				; // ERROR
			}
		}

		//>> Процедура конвертирования и записи каналов из 2 : STEREO
		void MixChannels_2(uint32 target_channels)
		{
			auto & front_left  = sampleTarget_prev[0];
			auto & front_right = sampleTarget_prev[1];

			switch (target_channels)
			{
			case 1: // STEREO to MONO
				{
					WriteTarget ( // FRONT_CENTER
						MixChannels(front_left, front_right) 
					); 
					break;
				}
			case 2: // STEREO to STEREO
			case 3:
				{
					WriteTarget(front_left,   // FRONT_LEFT
								front_right); // FRONT_RIGHT

					if (target_channels == 3)
						WriteTarget(_silence_);

					break;
				}
			case 4: // STEREO to QUAD / SURROUND
			case 5:
				{
					if (settings.target.range != eSDRange_SURROUND)
					{
						WriteTarget(front_left,   // FRONT_LEFT
									front_right,  // FRONT_RIGHT
									front_left,   // BACK_LEFT  (QUAD)
									front_right); // BACK_RIGHT (QUAD)
					}
					else
					{
						auto center = MixChannels ( front_left, front_right );

						WriteTarget(front_left,   // FRONT_LEFT
									front_right,  // FRONT_RIGHT
									center,       // FRONT_CENTER (SURROUND)
									center);      // BACK_CENTER  (SURROUND)
					}

					if (target_channels == 5)
						WriteTarget(_silence_);

					break;
				}
			case 6: // STEREO to 5.1 / 5.1 SURROUND
			case 7:
				{
					auto center = MixChannels ( front_left, front_right );

					WriteTarget(front_left,    // FRONT_LEFT
								front_right,   // FRONT_RIGHT
								center,        // FRONT_CENTER
								_silence_,     // LOW_FREQUENCY
								front_left,    // BACK_LEFT  or SIDE_LEFT  (SURROUND)
								front_right);  // BACK_RIGHT or SIDE_RIGHT (SURROUND)

					if (target_channels == 7)
						WriteTarget(_silence_);

					break;
				}
			case 8: // STEREO to 7.1 / 7.1 SURROUND
				{
					auto center = MixChannels ( front_left, front_right );

					WriteTarget(front_left,    // FRONT_LEFT
								front_right,   // FRONT_RIGHT
								center,        // FRONT_CENTER
								_silence_,     // LOW_FREQUENCY
								front_left,    // BACK_LEFT  or SIDE_LEFT  (SURROUND)
								front_right,   // BACK_RIGHT or SIDE_RIGHT (SURROUND)
								front_left,    // FRONT_LEFT_OF_CENTER  or BACK_LEFT  (SURROUND)
								front_right);  // FRONT_RIGHT_OF_CENTER or BACK_RIGHT (SURROUND)
					break;
				}
			default:
				; // ERROR
			}
		}

		//>> Процедура конвертирования и записи каналов из 4 : QUAD / SURROUND
		void MixChannels_4(uint32 target_channels)
		{
			auto & front_left   = sampleTarget_prev[0];
			auto & front_right  = sampleTarget_prev[1];

			auto & back_left    = sampleTarget_prev[2];
			auto & back_right   = sampleTarget_prev[3];

			auto & front_center = back_left;
			auto & back_center  = back_right;

			switch (target_channels)
			{
			case 1: // QUAD_SURROUND to MONO
				{
					WriteTarget ( // FRONT_CENTER
						MixChannels ( front_left, 
									  front_right,
									  back_left,
									  back_right )
					); 
					break;
				}
			case 2: // QUAD_SURROUND to STEREO
			case 3:
				{
					if (settings.origin.range != eSDRange_SURROUND)
					{
						WriteTarget ( MixChannels ( front_left,     // FRONT_LEFT
													back_left ),    //
									  MixChannels ( front_right,    // FRONT_RIGHT
													back_right ) ); //
					}
					else
					{
						auto center = MixChannels ( front_center,
													back_center );

						WriteTarget ( MixChannels ( front_left,  center ),   // FRONT_LEFT
									  MixChannels ( front_right, center ) ); // FRONT_RIGHT
					}

					if (target_channels == 3)
						WriteTarget(_silence_);

					break;
				}
			case 4: // QUAD_SURROUND to QUAD / SURROUND
			case 5:
				{
					if (settings.origin.range != eSDRange_SURROUND)
					{
						// QUAD to QUAD
						if (settings.target.range != eSDRange_SURROUND) 
						{
							WriteTarget ( front_left,   // FRONT_LEFT
										  front_right,  // FRONT_RIGHT
										  back_left,    // BACK_LEFT  (QUAD)
										  back_right ); // BACK_RIGHT (QUAD)
						}
						else 
						// QUAD to SURROUND
						{
							WriteTarget ( front_left,   // FRONT_LEFT
										  front_right,  // FRONT_RIGHT
										  MixChannels ( // FRONT_CENTER (SURROUND)
											 front_left, front_right ),
										  MixChannels ( // BACK_CENTER  (SURROUND)
											 back_left,  back_right  ) );
						}
					}
					else 
					{
						// SURROUND to QUAD
						if (settings.target.range != eSDRange_SURROUND) 
						{
							auto center = MixChannels ( front_center, back_center );
							auto left   = MixChannels ( front_left,   center      );
							auto right  = MixChannels ( front_right,  center      );

							WriteTarget ( left,    // FRONT_LEFT
										  right,   // FRONT_RIGHT
										  left,    // BACK_LEFT  (QUAD)
										  right ); // BACK_RIGHT (QUAD)
						}
						else
						// SURROUND to SURROUND
						{
							WriteTarget ( front_left,    // FRONT_LEFT
										  front_right,   // FRONT_RIGHT
										  front_center,  // FRONT_CENTER (SURROUND)
										  back_center ); // BACK_CENTER  (SURROUND)
						}
					}

					if (target_channels == 5)
						WriteTarget(_silence_);

					break;
				}
			case 6: // QUAD_SURROUND to 5.1 / 5.1 SURROUND
			case 7:
				{
					if (settings.origin.range != eSDRange_SURROUND)
					{
						auto center = MixChannels ( front_left, front_right );

						WriteTarget ( front_left,    // FRONT_LEFT
									  front_right,   // FRONT_RIGHT
									  center,        // FRONT_CENTER
									  _silence_,     // LOW_FREQUENCY
									  back_left,     // BACK_LEFT  or SIDE_LEFT  (SURROUND)
									  back_right );  // BACK_RIGHT or SIDE_RIGHT (SURROUND)
					}
					else
					{
						auto left  = MixChannels ( back_left,  back_center );
						auto right = MixChannels ( back_right, back_center );

						WriteTarget ( front_left,    // FRONT_LEFT
									  front_right,   // FRONT_RIGHT
									  front_center,  // FRONT_CENTER
									  _silence_,     // LOW_FREQUENCY
									  left,          // BACK_LEFT  or SIDE_LEFT  (SURROUND)
									  right );       // BACK_RIGHT or SIDE_RIGHT (SURROUND)
					}

					if (target_channels == 7)
						WriteTarget(_silence_);

					break;
				}
			case 8: // QUAD_SURROUND to 7.1 / 7.1 SURROUND
				{
					if (settings.origin.range != eSDRange_SURROUND)
					{
						auto center = MixChannels ( front_left,  front_right );
						auto left   = MixChannels ( front_left,  back_left   );
						auto right  = MixChannels ( front_right, back_right  );

						WriteTarget ( front_left,    // FRONT_LEFT
									  front_right,   // FRONT_RIGHT
									  center,        // FRONT_CENTER
									  _silence_,     // LOW_FREQUENCY
									  back_left,     // BACK_LEFT  or SIDE_LEFT  (SURROUND)
									  back_right,    // BACK_RIGHT or SIDE_RIGHT (SURROUND)
									  left,          // FRONT_LEFT_OF_CENTER  or BACK_LEFT  (SURROUND)
									  right );       // FRONT_RIGHT_OF_CENTER or BACK_RIGHT (SURROUND)
					}
					else
					{
						auto left  = MixChannels ( back_left,  back_center );
						auto right = MixChannels ( back_right, back_center );

						WriteTarget ( front_left,    // FRONT_LEFT
									  front_right,   // FRONT_RIGHT
									  front_center,  // FRONT_CENTER
									  _silence_,     // LOW_FREQUENCY
									  left,          // BACK_LEFT  or SIDE_LEFT  (SURROUND)
									  right,         // BACK_RIGHT or SIDE_RIGHT (SURROUND)
									  front_left,    // FRONT_LEFT_OF_CENTER  or BACK_LEFT  (SURROUND)
									  front_right ); // FRONT_RIGHT_OF_CENTER or BACK_RIGHT (SURROUND)
					}

					break;
				}
			default:
				; // ERROR
			}
		}

		//>> Процедура конвертирования и записи каналов из 6 : 5.1 / 5.1 SURROUND
		void MixChannels_6(uint32 target_channels)
		{
			auto & front_left    = sampleTarget_prev[0];
			auto & front_right   = sampleTarget_prev[1];

			auto & front_center  = sampleTarget_prev[2];
			auto & low_frequency = sampleTarget_prev[3];

			auto & back_left     = sampleTarget_prev[4];
			auto & back_right    = sampleTarget_prev[5];

			auto & side_left     = back_left;
			auto & side_right    = back_right;

			switch (target_channels)
			{
			case 1: // 5.1 to MONO
				{
					WriteTarget ( // FRONT_CENTER
						MixChannels ( front_left, 
									  front_right,
									  front_center,
									  low_frequency,
									  back_left,
									  back_right )
					);
					break;
				}
			case 2: // 5.1 to STEREO
			case 3:
				{
					WriteTarget ( // FRONT_LEFT
								  MixChannels ( front_left,
												front_center,
												low_frequency,
												back_left ),
								  // FRONT_RIGHT
								  MixChannels ( front_right,
												front_center,
												low_frequency,
												back_right )
					);

					if (target_channels == 3)
						WriteTarget(_silence_);

					break;
				}
			case 4: // 5.1 to QUAD / SURROUND
			case 5:
				{
					if (settings.target.range != eSDRange_SURROUND)
					{
						WriteTarget ( // FRONT_LEFT
									  MixChannels ( front_left,
													front_center,
													low_frequency ),
									  // FRONT_RIGHT
									  MixChannels ( front_right,
													front_center,
													low_frequency ),
									  // BACK_LEFT  (QUAD)
									  MixChannels ( back_left,
													low_frequency ),
									  // BACK_RIGHT (QUAD)
									  MixChannels ( back_right,
													low_frequency ) );
					}
					else
					{
						WriteTarget ( // FRONT_LEFT
									  MixChannels ( front_left,
													back_left,
													low_frequency ),
									  // FRONT_RIGHT
									  MixChannels ( front_right,
													back_right,
													low_frequency ),
									  // FRONT_CENTER (SURROUND)
									  MixChannels ( front_center,
													front_left,
													front_right,
													low_frequency ),
									  // BACK_CENTER  (SURROUND)
									  MixChannels ( back_right,
													back_left,
													low_frequency ) );
					}

					if (target_channels == 5)
						WriteTarget(_silence_);

					break;
				}
			case 6: // 5.1 to 5.1 / 5.1 SURROUND
			case 7:
				{
					WriteTarget ( front_left,    // FRONT_LEFT
								  front_right,   // FRONT_RIGHT
								  front_center,  // FRONT_CENTER
								  low_frequency, // LOW_FREQUENCY
								  back_left,     // BACK_LEFT  or SIDE_LEFT  (SURROUND)
								  back_right );  // BACK_RIGHT or SIDE_RIGHT (SURROUND)

					if (target_channels == 7)
						WriteTarget(_silence_);

					break;
				}
			case 8: // 5.1 to 7.1 / 7.1 SURROUND
				{
					WriteTarget ( front_left,    // FRONT_LEFT
								  front_right,   // FRONT_RIGHT
								  front_center,  // FRONT_CENTER
								  low_frequency, // LOW_FREQUENCY
								  back_left,     // BACK_LEFT  or SIDE_LEFT  (SURROUND)
								  back_right,    // BACK_RIGHT or SIDE_RIGHT (SURROUND)
								  back_left,     // FRONT_LEFT_OF_CENTER  or BACK_LEFT  (SURROUND)
								  back_right );  // FRONT_RIGHT_OF_CENTER or BACK_RIGHT (SURROUND)
					break;
				}
			default:
				; // ERROR
			}
		}

		//>> Процедура конвертирования и записи каналов из 8 : 7.1 / 7.1 SURROUND
		void MixChannels_8(uint32 target_channels)
		{
			auto & front_left     = sampleTarget_prev[0];
			auto & front_right    = sampleTarget_prev[1];

			auto & front_center   = sampleTarget_prev[2];
			auto & low_frequency  = sampleTarget_prev[3];

			auto & back_left      = sampleTarget_prev[4];
			auto & back_right     = sampleTarget_prev[5];

			auto & side_left      = back_left;
			auto & side_right     = back_right;

			auto & front_left_of  = sampleTarget_prev[6];
			auto & front_right_of = sampleTarget_prev[7];

			auto & back_left_of   = front_left_of;
			auto & back_right_of  = front_right_of;

			switch (target_channels)
			{
			case 1: // 7.1 to MONO
				{
					WriteTarget ( // FRONT_CENTER
						MixChannels ( front_left, 
									  front_right,
									  front_center,
									  low_frequency,
									  back_left,
									  back_right,
									  front_left_of,
									  front_right_of )
					);
					break;
				}
			case 2: // 7.1 to STEREO
			case 3:
				{
					WriteTarget ( // FRONT_LEFT
								  MixChannels ( front_left,
												front_center,
												low_frequency,
												back_left,
												front_left_of  ),
								  // FRONT_RIGHT
								  MixChannels ( front_right,
												front_center,
												low_frequency,
												back_right,
												front_right_of )
					);

					if (target_channels == 3)
						WriteTarget(_silence_);

					break;
				}
			case 4: // 7.1 to QUAD / SURROUND
			case 5:
				{
					if (settings.target.range != eSDRange_SURROUND)
					{
						WriteTarget ( // FRONT_LEFT
									  MixChannels ( front_left,
													front_center,
													low_frequency,
													front_left_of ),
									  // FRONT_RIGHT
									  MixChannels ( front_right,
													front_center,
													low_frequency,
													front_right_of ),
									  // BACK_LEFT  (QUAD)
									  MixChannels ( back_left,
													low_frequency,
													front_left_of ),
									  // BACK_RIGHT (QUAD)
									  MixChannels ( back_right,
													low_frequency,
													front_right_of ) );
					}
					else
					{
						WriteTarget ( // FRONT_LEFT
									  MixChannels ( front_left,
													back_left,
													low_frequency,
													front_left_of ),
									  // FRONT_RIGHT
									  MixChannels ( front_right,
													back_right,
													low_frequency,
													front_right_of ),
									  // FRONT_CENTER (SURROUND)
									  MixChannels ( front_center,
													front_left,
													front_right,
													low_frequency ),
									  // BACK_CENTER  (SURROUND)
									  MixChannels ( back_right,
													back_left,
													low_frequency ) );
					}

					if (target_channels == 5)
						WriteTarget(_silence_);

					break;
				}
			case 6: // 7.1 to 5.1 / 5.1 SURROUND
			case 7:
				{
					auto left  = MixChannels ( back_left,  front_left_of  );
					auto right = MixChannels ( back_right, front_right_of );

					WriteTarget ( front_left,    // FRONT_LEFT
								  front_right,   // FRONT_RIGHT
								  front_center,  // FRONT_CENTER
								  low_frequency, // LOW_FREQUENCY
								  left,          // BACK_LEFT  or SIDE_LEFT  (SURROUND)
								  right );       // BACK_RIGHT or SIDE_RIGHT (SURROUND)

					if (target_channels == 7)
						WriteTarget(_silence_);

					break;
				}
			case 8: // 7.1 to 7.1 / 7.1 SURROUND
				{
					WriteTarget ( front_left,       // FRONT_LEFT
								  front_right,      // FRONT_RIGHT
								  front_center,     // FRONT_CENTER
								  low_frequency,    // LOW_FREQUENCY
								  back_left,        // BACK_LEFT  or SIDE_LEFT  (SURROUND)
								  back_right,       // BACK_RIGHT or SIDE_RIGHT (SURROUND)
								  front_left_of,    // FRONT_LEFT_OF_CENTER  or BACK_LEFT  (SURROUND)
								  front_right_of ); // FRONT_RIGHT_OF_CENTER or BACK_RIGHT (SURROUND)
					break;
				}
			default:
				; // ERROR
			}
		}

		//>> Процедура конвертирования и записи каналов
		void MixChannelsRun(uint32 origin_channels, uint32 target_channels)
		{
			// количество 2+1 / 4+1 / 6+1 не должны вызывать ошибки,
			// поэтому лишний канал просто глушится

			switch (origin_channels)
			{
			case 1:
				MixChannels_1(target_channels);
				break;
			case 2:
			case 3: // skip 3 ch
				MixChannels_2(target_channels);
				break;
			case 4:
			case 5: // skip 5 ch
				MixChannels_4(target_channels);
				break;
			case 6:
			case 7: // skip 7 ch
				MixChannels_6(target_channels);
				break;
			case 8:
				MixChannels_8(target_channels);
				break;
			default:
				; // ERROR
			}
		}

	private:
		//>> Очистка исходника по заказу после успешной конвертации
		void ClearOrigin()
		{
			uint32 size_of_origin_in_bytes = settings.origin.size;
			byte * start_pos_abs           = (byte*) settings.origin.buffer;
			byte * start_pos_loc           = start_pos_abs + startSample * byteWeightOrigin;
			byte * end_pos_abs             = start_pos_abs + size_of_origin_in_bytes;
			uint32 converted_bytes         = settings.outInfo.origin_samples * byteWeightOrigin;

			int null_value = (origin_8bit_format) ? 0x80 : 0;
				
			// if full loop circle
			if (converted_bytes >= size_of_origin_in_bytes)
			{
				memset(start_pos_abs, null_value, size_of_origin_in_bytes);
			}
			else // else - partial
			{
				//   part_before   part_after  
				// + ->------->- + ->------>- + (end of buffer)

				uint32 part_before_in_bytes = (uint32) (start_pos_loc - start_pos_abs);
				uint32 part_after_in_bytes  = (uint32) (end_pos_abs - start_pos_loc);

				uint32 part_after_to_clear  = (converted_bytes > part_after_in_bytes) ? part_after_in_bytes                     : converted_bytes;
				uint32 part_before_to_clear = (converted_bytes > part_after_in_bytes) ? (converted_bytes - part_after_in_bytes) : 0;

				if (part_after_to_clear)
					memset(start_pos_loc, null_value, part_after_to_clear);

				if (part_before_to_clear)
					memset(start_pos_abs, null_value, part_before_to_clear);
			}
		}

		//>> Холостой ход (копирование)
		void BlankRun()
		{
			// Пропуск бессмысленной операции
			if (!settings.allocate && (settings.origin.buffer == settings.target.buffer))
			{
				settings.outInfo.start_time   = 0;
				settings.outInfo.start_sample = 0;

				settings.outInfo.end_sample = settings.outInfo.buffer_samples - PCM_ONE_FIX;
				settings.outInfo.end_time_o = settings.outInfo.buffer_time;
				settings.outInfo.end_time_t = settings.outInfo.buffer_time;

				settings.outInfo.loop_end_sample = settings.outInfo.end_sample;
				settings.outInfo.loop_end_time_o = settings.outInfo.end_time_o;
				settings.outInfo.loop_end_time_t = settings.outInfo.end_time_t;

				settings.outInfo.origin_samples = settings.outInfo.buffer_samples;
				settings.outInfo.origin_time    = settings.outInfo.buffer_time;

				settings.outInfo.target_samples = settings.outInfo.origin_samples;
				settings.outInfo.target_time    = settings.outInfo.origin_time;

			//	if (settings.clear_origin)
			//		ClearOrigin();

			//	if (volume_change.u32)
			//		ChangeVolume();

				return;
			}
		
			uint32 task = samplesConverted;					// blocks to copy (total)
			uint32 pos  = startSample * byteWeightOrigin;	// cur byte position

			while (task)
			{
				uint32 toCopy = (settings.origin.size - pos) / byteWeightOrigin; // blocks to copy (cur)
					   toCopy = (toCopy > task) ? task : toCopy;                 // 
				uint32 szCopy = toCopy * byteWeightOrigin;

				memcpy(_to, _from, szCopy);

				_from += szCopy;
				_to   += szCopy;
				pos   += szCopy;
				task  -= toCopy;
			
				if (_from == _from_end)
				{
					if (settings.loop)
					{
						_from = byteWeightOrigin + (byte*)settings.origin.buffer;
						pos   = byteWeightOrigin;
					}
					else break;
				}
			}

			// заказано сбросить прочитанное
			if (settings.clear_origin)
				ClearOrigin();

			// изменение выходной громкости
			if (volume_change.u32)
				ChangeVolume();
		}

		//>> Процедура конвертирования ИКМ
		void ConvertPCM()
		{
			if (!ResetTemps()) return; // ERROR : failed to allocate [target] memory

			// shift to start position
			_from += startSample * byteWeightOrigin;

			if (blank_run) { BlankRun(); return; } // just do copy and exit

			//printf("\nNOT BLANK RUN -- converter working...");

			byte origin_channels = (byte)origin->nChannels;
			byte target_channels = (byte)target->nChannels;

#ifdef PCM_SEPARATE_SYNCH_1S_DEBUG
			bool next_second        = false; // DEBUG : sign for jump over 1 sec. timepoint
			bool next_second_origin = false; // DEBUG : [origin] reached 1 sec. timepoint
			bool next_second_target = false; // DEBUG : [target] reached 1 sec. timepoint
#endif
			bool freeze_at_origin = false; // don't move in [origin]

			bool unsigned_to_signed = (!origin_signed &&  target_signed); // U origin to S target
			bool signed_to_unsigned = ( origin_signed && !target_signed); // S origin to U target

			bool float_to_float   = ( origin_32bit_float &&  target_32bit_float); // FLT origin to FLT target
			bool float_to_integer = ( origin_32bit_float && !target_32bit_float); // FLT origin to INT target
			bool integer_to_float = (!origin_32bit_float &&  target_32bit_float); // INT origin to FLT target

			bool different_weight = (byteWeightOrigin_ != byteWeightTarget_); // byte weight is not identical

			for (uint32 i=0; i<samplesToConvert; )
			{
				//if (i > samplesToConvert - 10)
				//{
				//	printf("\n");
				//	printf("\n                i = %i", i);
				//	printf("\ncounterFreqOrigin = %i / %i", counterFreqOrigin, samplesToConvert);
				//	printf("\ncounterFreqTarget = %i / %i", counterFreqTarget, samplesConverted);
				//	printf("\ntimeOrigin        = %f", timeOrigin);
				//	printf("\ntimeTarget        = %f", timeTarget);
				//	printf("\ntime T > O        = %s", _BOOLYESNO(timeTarget > timeOrigin));
				//}

				bool all_done = (counterTarget == samplesConverted); // all [target] samples are done
				bool target_out_of_range = false; // [target] time > [origin] time

				if (timeTarget > timeOrigin)
					target_out_of_range = true; // don't calc [target]

				float kt = 1.f; // time coefficient = (t - t1) / (t2 - t1)   (t = between, t1/t2 = start/end)
				if (i) kt = (float)((timeTarget - timeOrigin_prev) / (timeOrigin - timeOrigin_prev));
				//if (kt>1) kt=1;	

				byte * _from_temp = _from; // local temporary copy of _from

				for (byte channel = 0; channel < origin_channels; channel++)
				{
					UDOUBLE   sampleOrigin = 0; // take from [origin]
					UDOUBLE   sampleTarget = 0; // generate to [target]
					UDOUBLE & sampleOriginPrev = sampleOrigin_prev[channel]; // prev [origin]

					ReadOrigin(_from_temp, sampleOrigin);

					if (float_to_integer) // float [origin] to int32
						F2I(sampleOrigin);
			
					if (!target_out_of_range && !all_done) // calc target sample
					{
						// 1. Change rate : find target sample
						// 2. Change sample sign
						// 3. Change sample weight
						// 4. Remember result and mix channels later

						// (t - t1) / (t2 - t1) = (x - x1) / (x2 - x1)
						// x = (t - t1) / (t2 - t1) * (x2 - x1) + x1
						// x = kt                   * (x2 - x1) + x1

						// if 1-sec synch timepoint OR intermediate synch timepoint
						if ( //(counterFreqTarget  == 0 && counterFreqOrigin  == 0) ||
							 (counterSynchTarget == 0 && counterSynchOrigin == 0) )						
						{
							sampleTarget = sampleOrigin;
						}
						else
						{
							if (float_to_float) // float [origin] to float [target]
							{
								//float sampleOrigin_f     = ExtractFloat(sampleOrigin);
								//float sampleOriginPrev_f = ExtractFloat(sampleOriginPrev);
								//float sampleTarget_f = sampleOriginPrev_f + kt * (sampleOrigin_f - sampleOriginPrev_f);
								//PackFloat(sampleTarget_f, sampleTarget);

								sampleTarget.f32_1 = _CLAMP (
										(sampleOriginPrev.f32_1 + kt * (sampleOrigin.f32_1 - sampleOriginPrev.f32_1)),
										-1.f, +1.f );
							}
							else if (!origin_signed || origin_8bit_format) // unsigned samples OR special format
							{
								if (sampleOrigin >= sampleOriginPrev)
									 sampleTarget = sampleOriginPrev + (uint64)(kt * (sampleOrigin - sampleOriginPrev));
								else sampleTarget = sampleOriginPrev - (uint64)(kt * (sampleOriginPrev - sampleOrigin));
							}
							else // signed samples
							{
								bool prev_negative = (sampleOriginPrev > maxSampleOrigin_half);
								bool next_negative = (sampleOrigin     > maxSampleOrigin_half);

								if (!prev_negative && !next_negative) // all positive
								{
									if (sampleOrigin >= sampleOriginPrev)
										 sampleTarget = sampleOriginPrev + (uint64)(kt * (sampleOrigin - sampleOriginPrev));
									else sampleTarget = sampleOriginPrev - (uint64)(kt * (sampleOriginPrev - sampleOrigin));
								}
								else if (prev_negative && next_negative) // all negative
								{
									uint64 abs_prev_negative = MulMinus(maxSampleOrigin, sampleOriginPrev);
									uint64 abs_next_negative = MulMinus(maxSampleOrigin, sampleOrigin);

									if (abs_next_negative >= abs_prev_negative)
										 sampleTarget = abs_prev_negative + (uint64)(kt * (abs_next_negative - abs_prev_negative));
									else sampleTarget = abs_prev_negative - (uint64)(kt * (abs_prev_negative - abs_next_negative));

									sampleTarget = MulMinus(maxSampleOrigin, sampleTarget);
								}
								else // positive and negative
								{
									uint64 positive;
									uint64 negative;

									if (prev_negative)
									{
										positive = sampleOrigin;
										negative = sampleOriginPrev;
									}
									else
									{
										positive = sampleOriginPrev;
										negative = sampleOrigin;
									}

									uint64 abs_negative = MulMinus(maxSampleOrigin, negative);

									// calc chunk
									sampleTarget = (uint64)(kt * (positive + abs_negative));

									if (prev_negative) // from - to +
									{
										if (sampleTarget >= abs_negative) // +0..+N  else  -N..-1
											 sampleTarget =                           sampleTarget - abs_negative  ;
										else sampleTarget = MulMinus(maxSampleOrigin, abs_negative - sampleTarget) ;
									}
									else // from + to -
									{
										if (positive >= sampleTarget) // +N..+0  else  -1..-N
											 sampleTarget =                           positive - sampleTarget  ;
										else sampleTarget = MulMinus(maxSampleOrigin, sampleTarget - positive) ;
									}					
								}
							}
						}

						// special to standart (обратно конвертируем потом при записи, после микширования)
						if (origin_8bit_format)
							ConvertSpecial ( //maxSampleOrigin_half, 
							sampleTarget );
					
						// resolve signs
						if (unsigned_to_signed) // 0..65535 [0xFFFF]   ::   (0x8000..0xFFFF) -32768..-1  0  1..+32767 (0x1..0x7FFF)
						{
							// do scaling, for example: 0..0xFFFF as 0..0x7FFF
							sampleTarget = (uint64)((float)0.5 * sampleTarget);
						}
						else if (signed_to_unsigned)
						{
							// if negative value -> make it positive
							if (sampleTarget > maxSampleOrigin_half)
							{
								// x = abs(x);
								sampleTarget = MulMinus(maxSampleOrigin, sampleTarget);
								if (sampleTarget > maxSampleOrigin_half) // 80 > 7F
									sampleTarget = maxSampleOrigin_half;
							}

							// do scaling, for example: 0..0x7FFF as 0..0xFFFF
							sampleTarget *= 2;
						}

						// byte-weight is not identical
						if (different_weight)
							ChangeWeight(sampleTarget);

						// [origin] to float [target]
						if (integer_to_float) 
							I2F(sampleTarget);

						// mem sample
						sampleTarget_prev[channel] = sampleTarget;
					}

					// mem sample
					sampleOrigin_prev_temp[channel] = sampleOrigin;
				}

				// calc channels & write and move [target] buffer
				if (!target_out_of_range && !all_done)
					MixChannelsRun(origin_channels, target_channels);

				////////////////////////////////////////////////////////////////////////////

				// if we have normal calculations before
				if (!target_out_of_range && !all_done)
				{
					// count samples [target]
					counterTarget++;
					counterFreqTarget++;
					counterSynchTarget++;

					// move forward at timeline [target]
					timeTarget += timeWeightTarget_moves;
				}

				// float-fix of timeline [target] at 1-sec synch timepoint
				if (counterFreqTarget == _FreqTarget_moves)
				{
					timeTarget = round(timeTarget);
					counterFreqTarget  = 0; // reset for a new round
					counterSynchTarget = 0; // reset intermediate
#ifdef PCM_SEPARATE_SYNCH_1S_DEBUG
					next_second_target = true;
#endif
				}
#ifndef PCM_DISABLE_INTERMEDIATE
				// at intermediate synch timepoint
				else if (counterSynchTarget == synchQ_target &&
						 counterSynchOrigin == synchQ_origin)
				{
					timeTarget = timeOrigin;
					counterSynchTarget = 0; // reset intermediate
					counterSynchOrigin = 0; //
				}
#endif

				// check freeze [origin] condition
				if ( target_out_of_range || all_done || 
#ifdef PCM_SEPARATE_SYNCH_1S_DEBUG
					 next_second ||
#endif
					 timeTarget > timeOrigin ) freeze_at_origin = false;
				else                           freeze_at_origin = true;

				// next i
				if (!freeze_at_origin)
				{
					//if (i) // mem timepoint
						timeOrigin_prev = timeOrigin;

					i++;

					// move at [origin]
					_from += byteWeightOrigin;

					// move forward at timeline [origin]
					timeOrigin += timeWeightOrigin_moves;

					// count samples [origin]
					counterFreqOrigin++;
					counterSynchOrigin++;

					// swap temp with actually used
					_SWAP(sampleOrigin_prev.buf, sampleOrigin_prev_temp.buf);

					// loop condition
					if (_from == _from_end && settings.loop)
					{
						_from = byteWeightOrigin + (byte*) settings.origin.buffer;
					}
				}

				// float-fix of timeline [origin] at 1-sec synch timepoint
				if (counterFreqOrigin == _FreqOrigin_moves)
				{
					timeOrigin = round(timeOrigin);
					counterFreqOrigin  = 0; // reset for a new round
					counterSynchOrigin = 0; // reset intermediate
#ifdef PCM_SEPARATE_SYNCH_1S_DEBUG
					next_second_origin = true;
#endif
				}
#ifndef PCM_DISABLE_INTERMEDIATE
				// at intermediate synch timepoint
				else if (counterSynchTarget == synchQ_target &&
						 counterSynchOrigin == synchQ_origin)
				{
					timeOrigin = timeTarget;
					counterSynchTarget = 0; // reset intermediate
					counterSynchOrigin = 0; //
				}
#endif
			
#ifdef PCM_SEPARATE_SYNCH_1S_DEBUG
				// jump over 1 sec. timepoint    (каждая секунда конвертируется раздельно)
				{
					if (next_second)
					{
						timeTarget         = round(timeTarget);
						counterFreqTarget  = 0; // reset for a new round
						counterSynchTarget = 0; // reset intermediate

						timeOrigin         = round(timeOrigin);
						counterFreqOrigin  = 0; // reset for a new round
						counterSynchOrigin = 0; // reset intermediate

						next_second_origin = false;
						next_second_target = false;
						next_second        = false;
					}

					if (next_second_origin && next_second_target)
						next_second = true;
				}
#endif

			}

			// DEBUG / сюда мы никогда не должны попадать (исключение - слитный подсчёт раздельных сек. порций)
			// if last [target] sample(s) is out of range --> copypaste from prev
			if (counterTarget < samplesConverted) 
			{
				//printf("\nERROR : ConverterPCM last [target] sample(s) is out of range");
				int samples = samplesConverted - counterTarget;
				for (int i = 0; i < samples; i++)
					//for (uint32 channel = 0; channel < target_channels; channel++)
						//WriteTarget(sampleTarget_prev[channel]);
					MixChannelsRun(origin_channels, target_channels);
			}

			// В случае слитного подсчета возможно возникновение [target] за пределом временной линии [origin], например
			// [origin]  5 per sec : 1.....3.....5.....7.....9......x[NODATA]
			// [target] 10 per sec : 1..2..3..4..5..6..7..8..9..10..x[NODATA]    10-target за пределами последнего 9-origin

			// заказано сбросить прочитанное
			if (settings.clear_origin)
				ClearOrigin();

			// изменение выходной громкости
			if (volume_change.u32)
				ChangeVolume();
		}

	protected:
		//>> Проверка холостого хода
		void CheckBlankRun()
		{
			if (origin == nullptr || target == nullptr) return;

			// ОЦЕНКА ЗАКАЗАННЫХ ИЗМЕНЕНИЙ ВЫХОДНОЙ ГРОМКОСТИ //

			volume_change.u32 = 0;

			if (settings.origin.volume != nullptr)
			{
				uint32 BITMASK = 1;
				auto & volume = settings.origin.volume;

				for (uint32 i = 0; i < PCM_MAXCHANNELS; i++)
				{
					if (volume[i] != 1.f)             // бит <true> при изменённой громкости канала
						volume_change.u32 |= BITMASK; // .

					BITMASK <<= 1 ; // next bit
				}
			}

			// ОЦЕНКА ХОЛОСТОГО ХОДА //

			blank_run =
				( origin->nChannels      == target->nChannels      ) && // равное количество каналов
				( origin->nSamplesPerSec == target->nSamplesPerSec ) && // равная частота семплов
				( origin->wBitsPerSample == target->wBitsPerSample ) && // равная битность
				( origin->wFormatTag     == target->wFormatTag     ) && // тот же формат 
			//	( volume_change.u32 == 0 ) &&

			  ( ( settings.origin.range  == settings.target.range  ) || // одинаковое физ. расположение

				( ( settings.origin.range  == eSDRangeDirectOut )    &&
				  ( ( settings.target.range  == PCM_DEFAULT_4_CH )       ||
					( settings.target.range  == PCM_DEFAULT_6_CH )       ||
					( settings.target.range  == PCM_DEFAULT_8_CH ) ) )   ||

				( ( settings.target.range  == eSDRangeDirectOut )    &&
				  ( ( settings.origin.range  == PCM_DEFAULT_4_CH )       ||
					( settings.origin.range  == PCM_DEFAULT_6_CH )       ||
					( settings.origin.range  == PCM_DEFAULT_8_CH ) ) )  ) ;

			//blank_run = false; //test
		}

		//>> Базовые проверки
		bool CheckFormat(const SNDDESC * format, bool origin)
		{
			if (format == nullptr) 
				{ STATUS = ePCMConvStatus_NullFormat; return false; }

			// некорректная настройка работы
			if (!origin && settings.maxTarget && settings.allocate)
				{ STATUS = ePCMConvStatus_MaxTalloc; return false; }

			if ( (origin && settings.origin.buffer == nullptr) ||
				(!origin && settings.target.buffer == nullptr && !settings.allocate) )
				{ STATUS = ePCMConvStatus_NullBuffer; return false; }
		
			if (format->nChannels == 0 ||
				format->nChannels > PCM_MAXCHANNELS)
				{ STATUS = ePCMConvStatus_NoChannels; return false; }

			if (format->nSamplesPerSec < 2 || 
				format->nSamplesPerSec > PCM_MAXSAMPLESPERSEC)
				{ STATUS = ePCMConvStatus_NoFreq; return false; }

			if (format->wBitsPerSample == 0 ||
				format->wBitsPerSample > PCM_MAXBITSPERSAMPLE ||
				format->wBitsPerSample % 8 )
				{ STATUS = ePCMConvStatus_NoBitsPerS; return false; }

			// 64 битовый должен быть знаковый, иначе в MixChannels() начнутся 
			// проблемы float-вычислений (?? вопрос требует изучения ??)
			if (!origin && target->wBitsPerSample > 32 &&
				!settings.target.auto_signed &&
				!settings.target.user_signed)
				{ STATUS = ePCMConvStatus_Unsign64bit; return false; }

			if (!(format->wFormatTag == WAVE_FORMAT_PCM ||
				  format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
				  format->wFormatTag == WAVE_FORMAT_EXTENSIBLE))
				{ STATUS = ePCMConvStatus_NoFormatTag; return false; }

			if (( origin &&  !settings.origin.size) || 
				(!origin && (!settings.target.size && !settings.allocate)))
				{ STATUS = ePCMConvStatus_NullSize; return false; }

			if (!settings.allocate)
			{
				// целевой буфер в пределах исходного
				if ((((byte*)settings.origin.buffer                       ) < ((byte*)settings.target.buffer)) &&
					(((byte*)settings.origin.buffer + settings.origin.size) > ((byte*)settings.target.buffer)))
					{ STATUS = ePCMConvStatus_MissBuffer; return false; }

				// целевой буфер равен исходному и у нас не холостой ход
				if ((settings.origin.buffer == settings.target.buffer) && !blank_run)
					{ STATUS = ePCMConvStatus_MissBuffer; return false; }
			}

			if ( origin && settings.origin.time < 0) settings.origin.time = abs(settings.origin.time);
			if (!origin && settings.target.time < 0) settings.target.time = abs(settings.target.time);

			return true;
		}

		//>> Обмен внутренними буферами (между микшером и конвертором)
		void Exchange(void *& old_buffer, uint32 & old_buffer_size,
			          void *  new_buffer, uint32   new_buffer_size)
		{
			old_buffer      = settings.old_buffer;
			old_buffer_size = settings.old_buffer_size;

			if (new_buffer && new_buffer_size)
			{
				settings.old_buffer      = new_buffer;
				settings.old_buffer_size = new_buffer_size;
				settings.old_allocate    = true;
			}
			else
			{
				settings.old_buffer      = nullptr;
				settings.old_buffer_size = 0;
				settings.old_allocate    = false;
			}
		}

	public:
		//>> Уведомление конвертора об изменении PCM профилей SNDDESC
		void Change()
		{
			bReCalcBase = true;
		}

		//>> Сброс конвертора в состояние по умолчанию (все данные будут потеряны)
		void Reset()
		{
			settings.Reset();
			origin = nullptr;
			target = nullptr;
			bReCalcBase = true;
		}

		//>> Удаление старого <allocate>-[target] буфера
		void Free()
		{
			settings.FreeTarget();
		}

		//>> Предотвращает удаление/перезапись старого <allocate>-[target] - пользователь должен сам позже сделать _FREE(old_buffer)
		void Drop(void *& old_buffer, uint32 & old_buffer_size)
		{
			old_buffer      = settings.old_buffer;
			old_buffer_size = settings.old_buffer_size;
			settings.DropTarget();
		}

		//>> Запускает процесс конвертирования
		ePCMConvStatus Run()
		{
			STATUS = ePCMConvStatus_Normal;

			double mem_time;     // settings.target.time
			bool   mem_usetime;  // settings.target.usetime

			bool   mem_loop_fix = false;
			bool   mem_loop_usetime; // settings.origin.usetime
			uint32 mem_loop_samples; // settings.origin.samples

			CheckBlankRun();

			if ( !CheckFormat(origin, 1) ||
				 !CheckFormat(target, 0) ) return STATUS; // __EXIT__

			// Предварительные вычисления

			if (!CalculateBase())
				return STATUS;

			// Закажем сколько влезет в [target]
			if (settings.maxTarget) 
			{
				mem_time    = settings.target.time;    // Вернём позже
				mem_usetime = settings.target.usetime; // .

				//settings.target.time = timeWeightTarget * (settings.target.size / target->nBlockAlign);

				uint32 blocks  = settings.target.size / target->nBlockAlign; // blocks == frames
				uint32 seconds = (blocks - PCM_ONE_FIX) /            (target->nSamplesPerSec - PCM_ONE_FIX);
						blocks = (blocks - PCM_ONE_FIX) - (seconds * (target->nSamplesPerSec - PCM_ONE_FIX));

				settings.target.time     = (double) seconds;			  //if (blocks)
				settings.target.time    += timeWeightTarget * blocks;
				settings.target.usetime  = true;		
			}

			CalculateInput();
			CalculateOutput();

			// Проверка начальной позиции [origin]
			if (settings.outInfo.buffer_samples <= (settings.outInfo.start_sample + PCM_ONE_FIX))
			{
			/*	// конец как начало для <loop>
				if ( (settings.outInfo.buffer_samples == (settings.outInfo.start_sample + PCM_ONE_FIX)) &&
						settings.loop )
				{
					settings.origin.samples = 0;
					settings.origin.time    = 0;

					CalculateInput();
					CalculateOutput();
				}
				else
					STATUS = ePCMConvStatus_EndOfOrigin; //*/

				// пересчитать замкнутую позицию
				if (settings.loop)
				{
					mem_loop_fix     = true;
					mem_loop_samples = settings.origin.samples;
					mem_loop_usetime = settings.origin.usetime;

					settings.origin.samples = (settings.outInfo.start_sample + PCM_ONE_FIX) % settings.outInfo.buffer_samples;
					settings.origin.usetime = false;

					CalculateInput();
					CalculateOutput();
				}
				else
					STATUS = ePCMConvStatus_EndOfOrigin;
			}

			if (STATUS == ePCMConvStatus_Normal)
			{
				if (!settings.loop) // один проход [input] буфера до конца
				{
					// Доступно с учётом начальной позиции
					uint32 available_samples = settings.outInfo.buffer_samples - settings.outInfo.start_sample;

					// Пересчитаем всё с учётом сколько возможно сделать
					if (available_samples < settings.outInfo.origin_samples)
					{
						auto mem1 = settings.target.samples;
						auto mem2 = settings.target.usetime;

						settings.target.samples = available_samples;
						settings.target.usetime = false;

						CalculateInput();
						CalculateOutput();

						settings.target.samples = mem1; // Можно вернуть, используются только в CalculateInput()
						settings.target.usetime = mem2; // .
					}

					// Проверка выхода за пределы [target] буфера
					if (!settings.allocate && settings.target.size < (samplesConverted * byteWeightTarget))
						STATUS = ePCMConvStatus_EndOfTarget;
					else
						ConvertPCM();
				}
				else // зацикленный [input] буфер
				{
					// Проверка выхода за пределы [target] буфера
					if (!settings.allocate && settings.target.size < (samplesConverted * byteWeightTarget))
						STATUS = ePCMConvStatus_EndOfTarget;
					else
						ConvertPCM();
				}
			}

			// секция возврата настроек

			if (settings.maxTarget)
			{
				settings.target.time    = mem_time;
				settings.target.usetime = mem_usetime;
			}

			if (mem_loop_fix)
			{
				settings.origin.samples = mem_loop_samples;
				settings.origin.usetime = mem_loop_usetime;
			}

			return STATUS;
		}

	};

	//>> Переопределение конвертора для микшера
	class CPCMMixerConverter : public CPCMConverter
	{
	public:
		CPCMMixerConverter() : CPCMConverter() {};
		~CPCMMixerConverter() {};

	public:
		//>> Возвращает микшированный канал
		UDOUBLE MixChannels(UDOUBLE c1, UDOUBLE c2) override final
		{
			return CPCMConverter::MixChannels (
				std::forward<UDOUBLE>(c1),
				std::forward<UDOUBLE>(c2)
			);
		}

		//>> Читает семпл и двигает указатель буфера
		void ReadOrigin(byte *& _from, uint64 & sample, byte weight, bool BE) override final
		{
			CPCMConverter::ReadOrigin (
				std::forward<byte *&>(_from),
				std::forward<uint64 &>(sample),
				std::forward<byte>(weight),
				std::forward<bool>(BE)
			);
		}

		//>> Записывает семпл и двигает указатель буфера
		void WriteTarget(byte *& _to, uint64 sample, byte weight, bool BE) override final
		{
			CPCMConverter::WriteTarget(
				std::forward<byte *&>(_to),
				std::forward<uint64 &>(sample),
				std::forward<byte>(weight),
				std::forward<bool>(BE)
			);
		}

		//>> Конвертирует особую запись 8-битовых семплов в привычный вид
		void ConvertSpecial(uint64 & sample) override final
		{
			CPCMConverter::ConvertSpecial (
				std::forward<uint64 &>(sample)
			);
		}

		//>> Проверка холостого хода
		void CheckBlankRun()
		{
			CPCMConverter::CheckBlankRun();
		}

		//>> Базовые проверки
		bool CheckFormat(const SNDDESC * format, bool origin)
		{
			return
			CPCMConverter::CheckFormat (
				std::forward<const SNDDESC *>(format),
				std::forward<bool>(origin)
			);
		}

		//>> Обмен внутренними буферами между микшером и конвертором
		void Exchange(void *& old_buffer, uint32 & old_buffer_size,
			          void *  new_buffer, uint32   new_buffer_size)
		{
			CPCMConverter::Exchange (
				std::forward <void *&>  (old_buffer),
				std::forward <uint32 &> (old_buffer_size),
				std::forward <void *>   (new_buffer),
				std::forward <uint32>   (new_buffer_size)
			);
		}
	};

	///////////////////////////////////////////

	class CPCMMixer_hint;
	class CPCMMixer;

	struct SoundMixSource
	{
		friend class CPCMMixer_hint;
		friend class CPCMMixer;
	public:
		SNDDESC * origin { nullptr };                                    // source profile
		CPCMConverter::SoundConvertSettings::SoundSet  settings_origin;  // source mix settings
		CPCMConverter::SoundConvertSettings::SoundInfo settings_outInfo; // returned conversion info
		bool settings_loop { false };                                    // mix this source as a loop : default == false
		ePCMConvStatus ERR { ePCMConvStatus_Normal };                    // output error status
		uint64 ID { MISSING };                                           // user-specified id
	private:
		bool skip { false }; // drop this source out of task (AutoRun() fix)
	};

	//>> Прототип микшера
	class CPCMMixer_hint
	{
	public:
		virtual ePCMConvStatus Run(SoundMixSource * additional) = 0;

	protected:
		bool first_run    { 0 }; // indicate init step: origin to target 
		bool group_scheme { 0 }; // indicate more then 1 additional sources
		bool one_source   { 0 }; // 1 main and 0 additionals

	public:
		//>> Запуск микшера
		ePCMConvStatus Run(const vector < SoundMixSource * > & additional, uint64 count)
		{
			ePCMConvStatus hRes = ePCMConvStatus_Normal;

			if (count > additional.size())
				count = additional.size();

			if (!count) hRes = ePCMConvStatus_NoInputs;
			else
			{
				bool first_run_done = false;

				group_scheme = true;  // tip about existing others...

				for (uint64 i = 0; i < count; i++)
				{
					auto & cur = additional[i];
					
					if (cur) // в случае запуска в AutoRun() только 1 источника : one_source = TRUE (и skip = TRUE т.к. он же и есть основной)
					{
						// сброс
						cur->ERR = ePCMConvStatus_Normal;

						// мы всегда входим в секцию далее, если это не AutoRun() запуск
						if (!cur->skip || one_source)
						{
							if (first_run_done)
								Run(cur);
							else
							{
								first_run = true;     // run first time
								hRes      = Run(cur); // .
								first_run = false;    // .

								if (hRes != ePCMConvStatus_Normal &&
									hRes != ePCMConvStatus_EndOfOrigin)
								{
									break; // first run fail
								}

								first_run_done = true;
							}
						}
					}
				}

				group_scheme = false; // reset tip about existing others...
			}

			return hRes;
		}

		template <class ...T>
		//>> Запуск микшера
		ePCMConvStatus Run(SoundMixSource * additional, T... others)
		{
			static_assert (
				CHECK_T_TYPES_IS_SAME(T, SoundMixSource*),
				"CPCMMixer::Run() : All values must be SoundMixSource* type"
			);

			const vector <SoundMixSource *> & argvec = { additional, others... };

			return Run(argvec, argvec.size());
		}
	};

	//>> Смешивание разных PCM/IEEE источников
	class CPCMMixer : public CPCMMixer_hint
	{
	public:
		using CPCMMixer_hint::Run; // overload function

	private:
		CPCMMixerConverter converter;                      // worker
	public:
		CPCMMixerConverter::SoundConvertSettings settings;       // conversion order settings
		SNDDESC *      origin        { nullptr };                // base input PCM/IEEE profile
		SNDDESC	*      target        { nullptr };                // output PCM/IEEE profile
		ePCMConvStatus origin_status { ePCMConvStatus_Normal };  // returning code of converting base input
	private:
		uint32 _old_size     {0};       // mem old output [target] mixed data size
		void * _old_buffer   {nullptr}; // mem old output [target] mixed data
		bool   _old_allocate {false};   // mem old output [target] allocate setting
	private:
		TBUFFER <byte, uint32> addit; // temporary buffer for mixing

	public:
		CPCMMixer(const CPCMMixer & src)            = delete;
		CPCMMixer(CPCMMixer && src)                 = delete;
		CPCMMixer& operator=(const CPCMMixer & src) = delete;
		CPCMMixer& operator=(CPCMMixer && src)      = delete;

	public:
		CPCMMixer() : CPCMMixer_hint() { }
		~CPCMMixer() { Free(); };

	private:
		//>> Производит микширование основного и дополнительного источника
		void Mix()
		{
			uint32 sizeToMix   = settings.target.size;			// how much work to do
			uint32 sizeOfBlock = target->nBlockAlign;			// sample size in bytes [all channels]
			byte   weight      = target->wBitsPerSample / 8;	// sample size in bytes [per channel]
			bool   BE          = settings.target.BE;			// LE or BE
			bool   sample_8bit = (weight == 1);					// 8 bit sample
			byte   numChannels = (byte)target->nChannels;		// count of channels

			byte * base  = (byte*) settings.target.buffer;           // целевой буфер микширование
			byte * addit = (byte*) converter.settings.target.buffer; // сконвертированный источник для микширования в <base>

			while (sizeToMix)
			{
				for (byte channel = 0; channel < numChannels; channel++)
				{
					uint64 sample_base;
					uint64 sample_addit;

					converter.ReadOrigin(base,  sample_base,  weight, BE);
					converter.ReadOrigin(addit, sample_addit, weight, BE);

					if (sample_8bit) // ReadOrigin() don't init auto-convertion, so we need to do it manually
					{
						converter.ConvertSpecial(sample_base);
						converter.ConvertSpecial(sample_addit);
					}

					sample_base = converter.MixChannels(sample_base, sample_addit);

					// return back for saving result
					base -= weight;

					// if <sample_8bit == true> it will auto-converted while writing
					converter.WriteTarget(base, sample_base, weight, BE);
				}

				sizeToMix -= sizeOfBlock;
			}
		}

	public:
		//>> Удаление старых <allocate>-[settings.target.buffer] результатов
		void Free()
		{
			if (_old_allocate)
				//_DELETEN(old_buffer);
				_FREE(_old_buffer);

			_old_allocate = false; 
			_old_buffer   = nullptr;
			_old_size     = 0;

			// + сброс доп. буфера
			addit.Close();
		}

		//>> Предотвращает удаление старых <allocate>-[settings.target.buffer] результатов - пользователь должен сам позже сделать _FREE(old_buffer)
		void Drop(void *& old_buffer, uint32 & old_buffer_size)
		{
			old_buffer      = _old_buffer;
			old_buffer_size = _old_size;

			_old_allocate = false; 
			_old_buffer   = nullptr;
			_old_size     = 0;
		}

		//>> Запуск микшера
		ePCMConvStatus Run(SoundMixSource * additional) override final
		{
			// Если у нас всего 2 потока смешиваются, RunGroup() не будет выполнено,
			//   <group_scheme> будет false и надо самим выставить <first_run> = true
			// При смешиваниия первых 2 потоков : <first_run> = true ---> поправка <steps> = 2
			// При смешиваниия остальных, если они есть : RunGroup() установит <first_run> = false, а мы оставим <steps> = 1

			ePCMConvStatus hRes   = ePCMConvStatus_Normal;
			ePCMConvStatus hRes_a = ePCMConvStatus_Normal;
			bool b_ret_a, b_ret = false;

			byte steps = 1;
			
			if (!group_scheme)
				first_run = true;

			if (first_run && !one_source)
				steps = 2;

			for (byte i = 0; i < steps; i++)
			{
				if (first_run) // convert first stream
				{
					// обмен внутреннего буфера микширования в качестве целевого
					if (settings.allocate)
					{
						uint32 _conv_size;
						void * _conv_buffer;

						converter.Exchange (
							_conv_buffer, _conv_size, // то, что лежит у конвертора (вернём ему позже)
							_old_buffer,  _old_size   // передаём буфер микширования конвертору
						);

						_old_buffer = _conv_buffer; // запомним, чтобы вернуть позже
						_old_size   = _conv_size;   // .
					}

					converter.origin             = origin; // профили SNDDESC
					converter.target             = target; // .
					converter.settings.origin    = settings.origin; // настройки к профилям
					converter.settings.target    = settings.target; // .
					converter.settings.loop      = settings.loop;
					converter.settings.allocate  = settings.allocate;
					converter.settings.maxTarget = settings.maxTarget;

					// safe tip
					converter.Change();

					// run initial
					hRes = converter.Run();

					// обратный обмен буфера микширования
					if (settings.allocate)
					{
						uint32 _conv_size;
						void * _conv_buffer;

						converter.Exchange (
							_conv_buffer, _conv_size, // буфер микширования, что лежит у конвертора
							_old_buffer,  _old_size   // то, что лежало у конвертора ранее
						);

						if (_conv_buffer && _conv_size) // буфер микширования 
						{
							_old_buffer   = _conv_buffer;
							_old_size     = _conv_size;
							_old_allocate = true;
						}
						else 
						{
							_old_buffer   = nullptr;
							_old_size     = 0;
							_old_allocate = false;
						}
					}

					if (hRes == ePCMConvStatus_Normal ||
						hRes == ePCMConvStatus_EndOfOrigin)
					{ 
						b_ret = true;

						// save info
						settings.outInfo = converter.settings.outInfo;

						if (settings.allocate)
						{
							// save results
							settings.target.buffer = converter.settings.target.buffer; // буфер микширования (или его часть)
							settings.target.size   = converter.settings.target.size;   // .
						}

						// check size of pre-allocated additional buffer
						if (settings.target.size > addit.count)
						{
							if (!addit.Create(settings.target.size)) // MALLOC ERROR
							{
								hRes  = ePCMConvStatus_AllocMixErr;
								b_ret = false;
							}
						}

						// next we will convert all additionals - from now we will use pre-allocated buffer
						converter.settings.target.buffer  = addit.buf;
						converter.settings.target.size    = settings.target.size; // size <= addit.count
						converter.settings.maxTarget      = true;					
						converter.settings.allocate       = false;
					}
					else // ERROR
					{
						if (settings.allocate) // clearing outinfo
						{
							settings.target.buffer = nullptr;
							settings.target.size   = 0;
						}

						b_ret = false;
					}

					// mem status of origin
					origin_status = hRes;

					//if (one_source)
						hRes_a = hRes;
				}
				else // convert additional stream(s)
				{
					if (additional)
					{
						if (additional->origin)
						{
							// конвертер наполнит addit насколько сможет, остальное будет смешиваться как 0

							// clear target
							if (target->wBitsPerSample == 8) addit.Clear(PCM_NULL8BIT);
							else                             addit.Clear();

							converter.origin            = additional->origin;
							converter.settings.origin   = additional->settings_origin;
							converter.settings.loop     = additional->settings_loop;

							// tip about new origin
							converter.Change();

							// run additional
							hRes_a = converter.Run();

							if (hRes_a == ePCMConvStatus_Normal ||
								hRes_a == ePCMConvStatus_EndOfOrigin)
							{
								additional->settings_outInfo = converter.settings.outInfo;
								b_ret_a = true;
							}
							else
							{
								// был нулевой 0 заказ времени/семплов на выход
								if ( converter.settings.target.size == 0 && 
									 (hRes_a == ePCMConvStatus_NullSize ||
									  hRes_a == ePCMConvStatus_NullBuffer) )
								{
									hRes_a = ePCMConvStatus_Normal;
								}

								b_ret_a = false;
							}
						}
						else // additional->origin == nullptr
						{
							hRes_a  = ePCMConvStatus_NullBuffer;
							b_ret_a = false;
						}

						// mem status of additional
						additional->ERR = hRes_a;
					}
					else // additional == nullptr
					{
						hRes_a  = ePCMConvStatus_NullMixSrc;
						b_ret_a = false;
					}

					b_ret |= b_ret_a;
				}

				if (!b_ret) { // conversion ERROR
					first_run = false;
					break; }			

				// RUN mixing
				if (!first_run && b_ret_a)
					Mix();

				// off init step
				first_run = false;
			}

			if (steps == 1)
				hRes = hRes_a;

			// return value have critical effect (stop Mixer work) only on first (main) stream (first_run == true)
			// additional streams with errors just skipped (status saving into additional->ERR)

			return hRes;
		}

		//>> Запуск микшера с автовыбором основного источника (выбирается наиболее длительный) :: достаточно 1 источника
		ePCMConvStatus AutoRun(const vector < SoundMixSource * > & others, int64 count)
		{
			#define NONE_ORIGIN -1
			#define FIRST_ONE    0

			ePCMConvStatus hRes = ePCMConvStatus_Normal;

			if (count > (int64) others.size())
				count = (int64) others.size();

			if (count == 1)
				one_source = true;
			else if (count < 1)
				return hRes = ePCMConvStatus_NoInputs;

			bool   origin_choosed = false;  // force out
			int64  origin_i       = NONE_ORIGIN;
			int64  samples_i      = 0;
			auto  _p_args = others.begin(); // begin point
			auto   p_args = _p_args;        // iterator

			converter.target             = target;              // prepare [target] info for CheckFormat()
			converter.settings.target    = settings.target;     // .
			converter.settings.allocate  = settings.allocate;   // .
			converter.settings.maxTarget = settings.maxTarget;  // .

			settings.target.time = abs(settings.target.time);

			for (int64 i = FIRST_ONE; i < count; i++)
			{
				SoundMixSource * p_source;

			//	if (i == FIRST_ONE)   p_source = first;               // 1
			//	else                { p_source = *p_args; p_args++; } // 2, 3, 4...

				{ p_source = *p_args; p_args++; }

				// source ptr is valid ?
				if (p_source == nullptr) continue;
				else
				{   // source desc. is valid ?
					if (p_source->origin == nullptr)
					{
						p_source->ERR = ePCMConvStatus_NullFormat;
						continue;
					}
					else
					{
						p_source->ERR = ePCMConvStatus_Normal;
						auto & source = *p_source;

						// if source buffer & size are valid
						if (source.settings_origin.size && source.settings_origin.buffer)
						{
							p_source->settings_origin.time = abs(p_source->settings_origin.time);

							converter.origin          = p_source->origin;          // prepare [origin] info for CheckFormat()
							converter.settings.origin = p_source->settings_origin; // .
							converter.settings.loop   = p_source->settings_loop;   // .

							converter.CheckBlankRun();

							if (!converter.CheckFormat(p_source->origin, true))
							{
								p_source->ERR = converter.STATUS;
								continue;
							}
							else // source format is valid
							{
								if (source.settings_loop)
								{
									origin_choosed = true; // valid endless source
									origin_i       = i;    // .
								}							
								else // not looped origin
								{
									uint32 start_sample, samples_to_convert, buffer_samples, available_samples;

									uint32  freq_origin = source.origin->nSamplesPerSec - PCM_ONE_FIX;
									uint32  frame_size  = (source.origin->wBitsPerSample / 8) * source.origin->nChannels;

									if (source.settings_origin.usetime)
										 start_sample = (uint32) (source.settings_origin.time * freq_origin);
									else start_sample = (uint32) source.settings_origin.samples;

									buffer_samples = source.settings_origin.size / frame_size;

									// have any samples to convert
									if (buffer_samples > (start_sample + PCM_ONE_FIX))
									{
										if (!settings.allocate) // && settings.target.buffer && settings.target.size)
										{
											origin_choosed = true; // valid source & pre-allocated target buffer 
											origin_i       = i;    // (и неважно, валидный целевой или нет - проверять его не наша текущая задача)
										}
										else
										{
											available_samples = buffer_samples - start_sample;

											if (settings.target.usetime)
												 samples_to_convert = (uint32) ceil(settings.target.time * freq_origin) + PCM_ONE_FIX;
											else samples_to_convert = (uint32) settings.target.samples;

											uint32 work_samples = (available_samples > samples_to_convert) ? samples_to_convert : available_samples;

											if (work_samples > samples_i)
											{
												samples_i = work_samples; // (re)choose this one as origin
												 origin_i = i;            // .
											}
										}
									}
								}
							}
						}
					}
				}

				// досрочный выход при наличии зацикленного источника или уже выделенном конечном размере
				if (origin_choosed) break;
			}

			if (origin_i != NONE_ORIGIN)
			{
				SoundMixSource * p_source;

			//	if (origin_i == FIRST_ONE) p_source = first;
			//	else                       p_source = *(_p_args + origin_i);

				p_source = *(_p_args + origin_i);

				auto & source = *p_source;

				origin          = source.origin;
				settings.origin = source.settings_origin;
				settings.loop   = source.settings_loop;

				source.skip = true;
				hRes = Run(others, count);
				source.skip = false;

				// запись статуса основного
				source.ERR              = origin_status;    // [origin] error status
				source.settings_outInfo = settings.outInfo; // [origin] spec. info
			}
			else 
				hRes = ePCMConvStatus_BadInputs;

			// reset
			one_source = false;

			return hRes;

			// возможные возвращаемые результаты
			// A. ePCMConvStatus_NoInputs  - если меньше 1 задачи запустили
			// B. ePCMConvStatus_BadInputs - все входящие задачи "плохие" (не удалось выбрать [origin])
			// C. ---Если удалось выбрать [origin]---
			// C.a1. ePCMConvStatus_Normal      - удачное выполнение
			// C.a2. ePCMConvStatus_EndOfOrigin - удачное выполнение
			// C.b.  ---ошибка первого прохода при конвертировании выбранного [origin]---
			// C.b1. ePCMConvStatus_AllocErr_*  - ошибки выделения памяти при работе
			// C.b2. <???> - прочие ошибки из CheckFormat() функции (некорректный [origin] или неверная настройка запуска)	

			#undef NONE_ORIGIN
			#undef FIRST_ONE
		}

		template < class ...T >
		//>> Запуск микшера с автовыбором основного источника (выбирается наиболее длительный) :: достаточно 1 источника
		ePCMConvStatus AutoRun(SoundMixSource * first, T... others)
		{
			static_assert (
				CHECK_T_TYPES_IS_SAME(T, SoundMixSource*),
				"CPCMMixer::AutoRun() : All values must be SoundMixSource* type"
			);

			const vector <SoundMixSource *> & argvec = { first, others... };

			return AutoRun(argvec, argvec.size());
		}
	};

}

#undef MAXSAMPLESPERSEC
#undef MAXBITSPERSAMPLE
#undef MAXCHANNELS
#undef NULL8BIT
#undef DEFAULT_4_CH
#undef DEFAULT_6_CH
#undef DEFAULT_8_CH

#endif // _PCM_H