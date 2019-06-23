// ----------------------------------------------------------------------- //
//
// MODULE  : mp3.h
//
// PURPOSE : Декодер MP3 ~ MPEG-1 Layer-3 audio
//           с поддержкой SSE/AVX/AVX512 оптимизации
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _MP3_H
#define _MP3_H

#include "mathematics.h"
#include "mp3h.h"

#define MAXMP3SIZE  0x4000000 // ~64 MB
#define MAXMP3CHECK 0xFFFFF   // ~1 MB

namespace SAVFGAME
{

	// MP3 converter returning codes (UNT8)
	enum eMP3ConvStatus : unsigned char
	{
		eMP3ConvStatus_Normal,      // no errors, success convertion
		eMP3ConvStatus_NoInputPtr,  // Input MP3 buffer is nullptr
		eMP3ConvStatus_NoInputSz,   // Input MP3 buffer is null sized
		eMP3ConvStatus_MaxInput,    // Input MP3 buffer is too big, check MAXMP3SIZE macro to know max. value
		eMP3ConvStatus_BufferEnd,   // EXIT OUT : loop == false && no available data in input buffer
		eMP3ConvStatus_InputMod,    // unsafe modification of input data at Continue() mode
		eMP3ConvStatus_NoRun,       // run Continue() mode without Run() previously
		eMP3ConvStatus_DiffFrames,  // unexpected different frames types (supported only MPEG-1 Layer-3)
		eMP3ConvStatus_HeaderFail,  // incorrect frame header data
		eMP3ConvStatus_DiffFreq,    // some of frames have different nSamplesPerSec, convertion aborted
		eMP3ConvStatus_StartPosErr, // invalid start position frame or time
		eMP3ConvStatus_AllocOutErr,	// can't allocate out PCM buffer
		eMP3ConvStatus_HuffmanRead, // error while reading huffman data code : huffman table error
		eMP3ConvStatus_HuffmanEnd,  // error while reading huffman data code : part23end error
		eMP3ConvStatus_EOF_ID3,     // unexpected end of file while reading ID3 header
		eMP3ConvStatus_OutSamples,	// unexpected <out.samples> < 0   (internal decoder error ?)
	};

	// MP3 converter warning bit codes (UNT32)
	enum eMP3ConvWarning : unsigned int
	{
		eMP3ConvWarning_Normal      = 0,           // no warnings
		eMP3ConvWarning_OutMod      = MASK_BIT_00, // data_out ptr has been modified by user
		eMP3ConvWarning_StartTime   = MASK_BIT_01, // input <start_time> value is -negative (below 0) and has been fixed to +positive
		eMP3ConvWarning_Time        = MASK_BIT_02, // input <time> value is -negative (below 0) and has been fixed to +positive
		eMP3ConvWarning_LamePadding = MASK_BIT_03, // LAME padding seems to be damaged
	};

	//>> Управление конвертированием потока MPEG-1 Layer-3 (таблицы)
	class CMP3Converter_hint
	{
	protected:
		union UNION_C_SIN // sin table union
		{
			float f [MP3LINES * 2]; // f[36] = 4 + 32 / 4x8 SSE / 8x4 AVX / 16x2 AVX-512F /
#ifdef ENGINE_SSE
			__m128  m128[9]; // 4 float x8  + last 4 float
#endif
#ifdef ENGINE_AVX
			__m256  m256[4]; // 8 float x4  (no last 4 float)
#endif
#ifdef ENGINE_AVX512F
			__m512  m512[2]; // 16 float x2 (no last 4 float)
#endif
		};

	protected:															// Requantize
		static float pow43 [MP3MAXHUFF];
		static float pow2A [(1 + 2 * (MP3MAXSCAL + MP3MAXPRET))];
		static float pow2B [(1 + 256 * 8)];
																		// Process stereo										
		static float ratio_l [7];
		static float ratio_r [7];
		static UNIONMAX   uSqrt2inv;
		static UNION512 & uSqrt2inv512;
		static UNION256 & uSqrt2inv256;
		static UNION128 & uSqrt2inv128;
																		// Antialias
		static UNION512 ucs2, uca2;
		static UNION256 & ucs, & uca;
		//static float cs [MP3ANTIALI];
		//static float ca [MP3ANTIALI];
																		// Hybrid synthesis
		static UNION_C_SIN u_IMDCT_sin [eMPEGBlockT_ENUM_MAX];
		static float *       IMDCT_sin [eMPEGBlockT_ENUM_MAX];
	//	static float IMDCT_sin [eMPEGBlockT_ENUM_MAX][MP3LINES * 2];
																		// Hybrid synthesis (simple)
		static float IMDCT_cos1tab [(1 + 12 * 6)];
		static float IMDCT_cos2tab [(1 + 36 * 18)];
																		// Hybrid synthesis (fast)
		static const float c_sqrt32;
		static const float c_cos9;
		static const float c_cos94;
		static const float c_cos92;
		static const float c_sin0;
		static const float c_sin1;
		static const float c_sin2;
		static const float c_sin3;
		static UNION128    c_sin;

		static UNION1024 u_form_18_36;
		static UNION512  u_form_9_18;
		static UNION256  u_form_6_12;
		static UNION128  u_form_6_12_3;
		static UNION128  u_form_4_9;
		static UNION128  u_form_3_6;
		static float * form_9_18;   // form_9_18  [9];
		static float * form_18_36;  // form_18_36 [18];
		static float * form_6_12;   // form_6_12  [7];
		static float * form_4_9;	// form_4_9   [4];
		static float * form_3_6;    // form_3_6   [4];		
																	// FreqInversion SSE/AVX/AVX-256F opt.
		static uint16     freqInvIdx[MP3FREQINV];
		static UNIONMAX   freqInvMinus;
		static UNION512 & freqInvMinus512;
		static UNION256 & freqInvMinus256;
		static UNION128 & freqInvMinus128;

																	// Subband synthesis (simple)
		static float synthesis_tab [MP3SUBBANDS * 2][MP3SUBBANDS];
																	// Subband synthesis (fast)
		static UNION512 u_form_16_32;
		static UNION256 u_form_8_16;
		static UNION128 u_form_4_8;
		static float * form_16_32;   // form_16_32 [16];
		static float * form_8_16;    // form_8_16  [8];
		static float * form_4_8;     // form_4_8   [4];
		static float form_2_4 [2];

		static float cosij [2][2];

		static bool opt_SSE;
		static bool opt_SSE_only;
		static bool opt_AVX;
		static bool opt_AVX_only;
		static bool opt_AVX512F;
		static bool opt_AVX512F_only;
		static bool opt_AVX512F_combo;

	public:
		CMP3Converter_hint() { Init(); };
		~CMP3Converter_hint() {};

	public:
		CMP3Converter_hint(const CMP3Converter_hint & src)            = delete;
		CMP3Converter_hint(CMP3Converter_hint && src)                 = delete;
		CMP3Converter_hint& operator=(const CMP3Converter_hint & src) = delete;
		CMP3Converter_hint& operator=(CMP3Converter_hint && src)      = delete;

	protected:
		static
		//>> Инициализация таблиц констант
		void Init();
	};

	//>> Управление конвертированием потока MPEG-1 Layer-3
	class CMP3Converter : public CMP3Converter_hint
	{
		struct MP3Settings
		{
			// input data
			struct SoundSet
			{
				friend class CMP3Converter;

				void Reset()
				{
					data        = nullptr;
					size        = 0;
					start_time  = 0;
					start_frame = 0;
					time        = 0;
					use_frame   = false;
					loop        = false;
					maximize    = false;

				//	data_mem    = nullptr;
				//	size_mem    = 0;
				}

				byte * data         { nullptr }; // origin MP3 data to convert
				uint32 size         { 0 };       // origin MP3 data size
				float  start_time   { 0 };	     // start timepoit in seconds
				uint32 start_frame  { 0 };		 // start MP3 frame number (alt. start)
				float  time         { 0 };       // requested time in seconds (0 == all available time, loop setting will be ignored)
				bool   use_frame    { false };   // use <start_frame> instead of <start_time> : default = false
				bool   loop         { false };   // loop MP3 : default = false		
				bool   maximize     { false };   // TRUE: ceil up out.time to next frame (FALSE: floor down) : default = false

				//
				// start_time                     out.time : maximize = 0      in.time    out.time : maximize = 1
				// !                                       !                      !                !
				// +---------------------------------------+---------------------------------------+
				// frame0                                  frame1                 -> ceil up ->    frame2

			private:
				byte * data_mem { nullptr };	// private hidden memorized <data>
				uint32 size_mem { 0 };			// private hidden memorized <size>
			};

			// output data
			struct SoundInfo
			{
				friend class CMP3Converter;

				void Reset()
				{
					_data           = nullptr;
					_size           = 0;			
					start_time      = 0;
					start_frame     = 0;
					time            = 0;
					samples         = 0;
					frames          = 0;
					total_frames    = 0;
					end_frame       = 0;
					loop_end_frame  = 0;
					_skipped        = 0;
					_padding        = 0;
					_padding_before = 0;
					_padding_after  = 0;
					wFormatTag      = 0;
					nSamplesPerSec  = 0;
					nChannels       = 0;
					wBitsPerSample  = 0;
					fsz_time        = 0;
					fsz_samples     = 0;
					fsz_bytes       = 0;

				//	data_out       = nullptr;
					size_out       = 0;
				//	size_out_mem   = 0;
				//  data_out_mem   = nullptr;


				}

				void Printf()
				{
					printf("\nstart_time     = %f" \
						   "\nstart_frame    = %i" \
						   "\ntime           = %f" \
						   "\nsamples        = %i" \
						   "\nframes         = %i" \
						   "\ntotal_frames   = %i" \
						   "\nend_frame      = %i" \
						   "\nloop_end_frame = %i" \
						   "\nskipped        = %i" \
						   "\npadding        = %i" \
						   "\npadding_before = %i" \
						   "\npadding_after  = %i" \
						   "\nwFormatTag     = %i" \
						   "\nnSamplesPerSec = %i" \
						   "\nnChannels      = %i" \
						   "\nwBitsPerSample = %i" \
						   "\nfsz_time       = %f" \
						   "\nfsz_samples    = %i" \
						   "\nfsz_bytes      = %i" \
						   ,
						start_time,
						start_frame,
						time,
						samples,
						frames,
						total_frames,
						end_frame,
						loop_end_frame,
						_skipped,
						_padding,
						_padding_before,
						_padding_after,
						wFormatTag,
						nSamplesPerSec,
						nChannels,
						wBitsPerSample,
						fsz_time,
						fsz_samples,
						fsz_bytes
					);
				}

				byte * _data           { nullptr };  // [DEBUG] stop point ptr
				uint32 _size           { 0 };        // [DEBUG] stop point available data
				byte * data_out        { nullptr };  // target PCM buffer (RECOMMENDED TO NOT TOUCH, IT'S AUTO MANAGED BY DEFAULT)
				uint32 size_out        { 0 };        // target PCM buffer size
				float  start_time      { 0 };        // start timepoit, in fact
				uint32 start_frame     { 0 };        // start MP3 frame number, in fact
				float  time            { 0 };        // converted time (in seconds)
				uint32 samples         { 0 };        // converted blocks of samples
				uint32 frames          { 0 };        // converted frames
				uint32 total_frames    { 0 };		 // count of frames in input mp3 buffer
				uint32 end_frame       { 0 };		 // stop frame # number
				uint32 loop_end_frame  { 0 };		 // stop frame # number (loop-fixed)
				uint32 _skipped        { 0 };		 // [DEBUG] num of skipped frames 
				uint32 _padding        { 0 };		 // [DEBUG] LAME num of dummy samples skipped
				uint32 _padding_before { 0 };		 // [DEBUG] LAME num of dummy samples added by LAME encoder at start 
				uint32 _padding_after  { 0 };		 // [DEBUG] LAME num of dummy samples added by LAME encoder at end
				uint32 wFormatTag      { 0 };        // 1 = WAVE_FORMAT_PCM (int samples), 3 = WAVE_FORMAT_IEEE_FLOAT (float samples)
				uint32 nSamplesPerSec  { 0 };        // samples per second
				uint16 nChannels       { 0 };        // count of channels
				uint16 wBitsPerSample  { 0 };        // bits per sample of mono data
				float  fsz_time        { 0 };        // size of 1 frame in seconds
				uint32 fsz_samples     { 0 };        // size of 1 frame in samples (blocks)
				uint32 fsz_bytes       { 0 };        // size of 1 frame in bytes
			
			private:
				uint32 size_out_mem   { 0 };		// private hidden memorized <size_out>
				byte * data_out_mem   { nullptr };	// private hidden memorized <data_out> :: for checking if user modified ptr

				// use in Continue() mode
				void Reset_continue()
				{
					start_time     = 0;
					start_frame    = 0;
					time           = 0;
					samples        = 0;
					frames         = 0;
					_skipped       = 0;
				}
			};

			SoundSet  in;	// user -> converter
			SoundInfo out;	// converter -> user
		};

	public:
		MP3Settings settings; // conversion settings
		uint32      warning;  // content bits of eMP3ConvWarning, you can check it after convertion

	private:
		MPEGHeader		 MPEG;  // content MPEG main header
		MPEGHeaderSide	 MPEGS; // content MPEG side header
		MPEGMainData     MPEGD; // content MPEG data
		MPEGHeader *     MPEG_p  { & MPEG };  // (const) ptr to MPEG main header
		MPEGHeaderSide * MPEGS_p { & MPEGS }; // (const) ptr to MPEG side header

	private:
		uint32 padding_b_frame { 0 }; // LAME : <padding_before> mem frame #
		uint32 padding_a_frame { 0 }; // LAME : <padding_after>  mem frame #
		uint32 padding_b       { 0 }; // LAME : <padding_before> current value
		uint32 padding_a       { 0 }; // LAME : <padding_after>  current value
		uint32 padding_before  { 0 }; // LAME : samples padding before original .wav
		uint32 padding_after   { 0 }; // LAME : samples padding after  original .wav
	private:
		uint32 frame0_position { MPG_ERROR }; // initial 0-frame MP3-buffer offset
		uint32 start_position  { MPG_ERROR }; // MP3-buffer starting offset
		uint32 start_frame     { MPG_ERROR }; // starting frame #
		uint32 total_frames    { MPG_ERROR }; // total frames in buffer
		uint32 stop_frame      { MPG_ERROR }; // counter STOP frame #
		uint32 continue_frame  { MPG_ERROR }; // pause position for Continue() mode 
		float  frame_time      { MPG_ERROR }; // how much time presented by 1 frame

		eMP3ConvStatus STATUS; // returning exit code

	private:
		//>> Бегунок в памяти
		struct Runner
		{
			byte *	p;		// [buffer] current ptr
			byte *	end;	// [buffer] end ptr
			uint32	pos;	// [buffer] position from start
			uint32	last;	// [buffer] count of bytes available up to end
		} runner;

		float  storehs[2][MP3SUBBANDS][MP3LINES]; // HybridSynth IMDCT
		float  storess[2][MP3SYNTHESIS * 2];      // SubbandSynth  2ch * 512x2 = 1024 = [16][64]
		byte   storess_pos[2];                    // 0..15

	public:
		CMP3Converter(const CMP3Converter & src)            = delete;
		CMP3Converter(CMP3Converter && src)                 = delete;
		CMP3Converter& operator=(const CMP3Converter & src) = delete;
		CMP3Converter& operator=(CMP3Converter && src)      = delete;

	public:
		CMP3Converter() : CMP3Converter_hint() {};
		~CMP3Converter()
		{
			if (settings.out.data_out != nullptr)
				free(settings.out.data_out);
		};

	private:
		//>> Производит сдвиг бегунка в указанное положение :: возвращает <false>, если достигли конца
		bool RunnerShift(uint32 position)
		{
			runner.p  += (int32)(position - runner.pos);
			runner.pos = position;
		
			if (runner.p > runner.end)
			{
				runner.last = 0;
				return false;
			}
			else // runner.p <= runner.end
			{
				runner.last = (uint32) (runner.end - runner.p);
				return true;
			}
		}

		//>> Производит сдвиг бегунка относительно текущей точки :: возвращает <false>, если достигли конца
		bool RunnerOffset(int32 offset)
		{
			runner.p   += offset;
			runner.pos += offset;

			if (runner.p > runner.end)
			{
				runner.last = 0;
				return false;
			}
			else // runner.p <= runner.end
			{
				runner.last = (uint32)(runner.end - runner.p);
				return true;
			}
		}

	private:
		//>> 1.1. Приведение семпла к исходному виду (возврат съэкономленных бит)
		void ConvertMP3_Requantize(byte gr, byte ch)
		{
			float * mpeg_samples = MPEGD.sample[gr][ch];

			uint32 pow2A_ss   = MPEGS.scalefac_scale[gr][ch] ? 2 : 1;
			uint32 pow2B_gg   = MPEGS.global_gain[gr][ch];
			auto   pow2A_sfl  = MPEGD.scalefac_l[gr][ch];
			auto   pow2A_sfs  = MPEGD.scalefac_s[gr][ch];
			auto   pow2B_sbg  = MPEGS.subblock_gain[gr][ch];
			bool   preflag    = MPEGS.preflag[gr][ch];

			auto RequantizeLong  = [this, mpeg_samples](byte gr, byte ch, uint16 sample_id, uint16 sfb) -> void
			{
				// x1 - get scale mul component == 2^A, A = -scale * (scalefac_l + pretab)
				// x2 - get scale from gain     == 2^B, B = 1/4 * (global_gain - 210)
				// restore sample from pow ^(3/4)
				//
				//float scale = MPEGS.scalefac_scale[gr][ch] ? 1.f : 0.5f;
				//byte  pref  = MPEGS.preflag[gr][ch] ? scalefac_pretab[sfb] : 0;
				//float x1    = pow(2.f, -(scale * ((int32)MPEGD.scalefac_l[gr][ch][sfb] + pref)));
				//float x2    = pow(2.f, 0.25f * ((int32)MPEGS.global_gain[gr][ch] - 210));

				float x1 = pow2A[ (MPEGS.scalefac_scale[gr][ch] ? 2ul : 1ul) *
								  (  MPEGD.scalefac_l[gr][ch][sfb] +
									(MPEGS.preflag[gr][ch] ? scalefac_pretab[sfb] : 0) 
								  )];
				float x2 = pow2B[ (uint32)MPEGS.global_gain[gr][ch]
								];

				float & sample = mpeg_samples[sample_id];
				if (sample < 0)
					 sample = -pow43[(uint16)-sample]; //-pow(-sample, sample_power);
				else sample =  pow43[(uint16) sample]; // pow( sample, sample_power);

				sample *= x1 * x2;
			};
			auto RequantizeShort = [this, mpeg_samples](byte gr, byte ch, uint16 sample_id, uint16 sfb, byte window) -> void
			{
				// x1 - get scale mul component == 2^A, A = -scale * scalefac_s
				// x2 - get scale from gain     == 2^B, B = 1/4 * (global_gain - 210 - 8 * sub_gain)
				// restore sample from pow ^(3/4)
				//
				//float scale = MPEGS.scalefac_scale[gr][ch] ? 1.f : 0.5f;
				//float x1 = pow(2.f, -(scale * (uint32)MPEGD.scalefac_s[gr][ch][sfb][window]));
				//float x2 = pow(2.f, 0.25f * ((int32)MPEGS.global_gain[gr][ch] - 210 - 8 * MPEGS.subblock_gain[gr][ch][window]));

				float x1 = pow2A[ (MPEGS.scalefac_scale[gr][ch] ? 2ul : 1ul) *
								   MPEGD.scalefac_s[gr][ch][sfb][window]
								];
				float x2 = pow2B[ (uint32)MPEGS.global_gain[gr][ch] + 256ul * MPEGS.subblock_gain[gr][ch][window]
								];

				float & sample = mpeg_samples[sample_id];
				if (sample < 0)
					 sample = -pow43[(uint16)-sample]; //-pow(-sample, sample_power);
				else sample =  pow43[(uint16) sample]; // pow( sample, sample_power);

				sample *= x1 * x2;
			};

	#pragma region <RequantizeOptCombo>

	#define _RequantizeOptCombo(sample_id, x1_x2) {             \
				float & sample = mpeg_samples[sample_id];       \
				if (sample < 0)                                 \
					 sample = -pow43[(uint16)-sample] * x1_x2;  \
				else sample =  pow43[(uint16) sample] * x1_x2;  \
			}

	#pragma endregion

	#pragma region <RequantizeOpt>

	#define _RequantizeOpt(sample_id, x1, x2) {                   \
				float & sample = mpeg_samples[sample_id];         \
				if (sample < 0)                                   \
					 sample = -pow43[(uint16)-sample] * x1 * x2;  \
				else sample =  pow43[(uint16) sample] * x1 * x2;  \
			}

	#pragma endregion

	#pragma region <RequantizeLong>

	#define _RequantizeLong(sample_id, sfb) {                                 \
				float x1 = pow2A [ pow2A_ss * (pow2A_sfl[sfb] +               \
									(preflag ? scalefac_pretab[sfb] : 0)) ];  \
				float x2 = pow2B [ pow2B_gg ];                                \
																			  \
				_RequantizeOpt(sample_id, x1, x2)                             \
			}

	#pragma endregion

	#pragma region <RequantizeShort>

	#define _RequantizeShort(sample_id, sfb, window) {                      \
				float x1 = pow2A [ pow2A_ss * pow2A_sfs[sfb][window] ];     \
				float x2 = pow2B [ pow2B_gg + 256ul * pow2B_sbg[window] ];  \
																			\
				_RequantizeOpt(sample_id, x1, x2)                           \
			}

	#pragma endregion

			uint16 win_len;
			uint16 next;    // id of frame of next sfb
			uint16 sfb = 0; // scalefactor band index
			auto   sfb_ind_l = sfb_indice[MPEG.freq].l;
			auto   sfb_ind_s = sfb_indice[MPEG.freq].s;
			auto   rzero = MPEGS.count1[gr][ch];

#ifdef _SSE_opt_is_too_bad_here_so_use_simple_code_

	#pragma region <RequantizeOpt_SSE>

	#define _RequantizeOpt_SSE(sample_id, u1_u2) {                          \
				UNION128 a, b;                                              \
				float f1 = mpeg_samples[sample_id];                         \
				float f2 = mpeg_samples[sample_id + 1];                     \
				float f3 = mpeg_samples[sample_id + 2];                     \
				float f4 = mpeg_samples[sample_id + 3];                     \
				                                                            \
				if (f1 < 0) f1 = -pow43[(uint16)-f1];                       \
				else        f1 =  pow43[(uint16) f1];                       \
				if (f2 < 0) f2 = -pow43[(uint16)-f2];                       \
				else        f2 =  pow43[(uint16) f2];                       \
				if (f3 < 0) f3 = -pow43[(uint16)-f3];                       \
				else        f3 =  pow43[(uint16) f3];                       \
				if (f4 < 0) f4 = -pow43[(uint16)-f4];                       \
				else        f4 =  pow43[(uint16) f4];                       \
				                                                            \
				a.m128 = _mm_set_ps(f4, f3, f2, f1);                        \
				b.m128 = _mm_mul_ps(a.m128, u1_u2.m128);                    \
				_mm_storeu_ps(mpeg_samples + sample_id, b.m128);            \
			}

	#pragma endregion

			if (opt_SSE_only) // SSE CODE = ??%..??% time
			{
				uint16 win_len_SSE;
				uint16 next_SSE, next_SSE_chunk;

				if (MPEGS.switch_flag[gr][ch] && MPEGS.block_type[gr][ch] == eMPEGBlockT_3ShortWindow)
				{
					uint16 i = 0;

					// Check for long blocks //

					if (MPEGS.mixed_block_flag[gr][ch]) // MIXED BLOCK L+S
					{
						UNION128 x1, x2, x1_x2; // mul constants

						x2.m128 = _mm_load_ps1(pow2B + pow2B_gg);
						x1.m128 = _mm_load_ps1(pow2A + pow2A_ss * 
											   (pow2A_sfl[sfb] + (preflag ? scalefac_pretab[sfb] : 0))
											  );
						x1_x2.m128 = _mm_mul_ps(x1.m128, x2.m128);

						uint16 i_max = sfb_ind_l[MP3SFB_ML]; // i_max = 36 = sfb_ind_l[MP3SFB_ML], MP3SFB_ML = 8

						next       = sfb_ind_l[sfb + 1];
						next_SSE   = next;
						next_SSE >>= 2; // next_SSE / _128bit_
						next_SSE <<= 2; // next_SSE * _128bit_
						for (; i < i_max ;)
						{
							if (i == next)
							{
								next     = sfb_ind_l[++sfb + 1];
								next_SSE = next - i;
								next_SSE = next_SSE >> 2;
								next_SSE = (next_SSE << 2) + i;
								x1.m128 = _mm_load_ps1(pow2A + pow2A_ss * 
													   (pow2A_sfl[sfb] + (preflag ? scalefac_pretab[sfb] : 0))
													  );
								x1_x2.m128 = _mm_mul_ps(x1.m128, x2.m128);
							}

							for (; i < next_SSE; i += 4)
								_RequantizeOpt_SSE(i, x1_x2);

							for (; i < next; i++)
								_RequantizeOptCombo(i, x1_x2.f[0]);
						}
						sfb = MP3SFB_MS;
					}
			
					// Short blocks //

					// sfb = 0 ? 3
					next    = sfb_ind_s[sfb + 1] * MP3SFB_SWIN;
					win_len = sfb_ind_s[sfb + 1] -
							  sfb_ind_s[sfb];

					win_len_SSE   = win_len;
					win_len_SSE >>= 2;
					win_len_SSE <<= 2;
		
					UNION128 x1;                 // x1 for short blocks
					UNION128 x2[MP3SFB_SWIN];    // x2 for short blocks
					UNION128 x1_x2[MP3SFB_SWIN]; // x1_x2 for short blocks

					for (int win = 0; win < MP3SFB_SWIN; win++)
					{
						x2[win].m128    = _mm_load_ps1( pow2B + pow2B_gg + 256ul * pow2B_sbg[win] );
						x1.m128         = _mm_load_ps1( pow2A + pow2A_ss * pow2A_sfs[sfb][win]    );
						x1_x2[win].m128 = _mm_mul_ps(x1.m128, x2[win].m128);
					}

					//i_max = rzero;
					// i = 0 ? 36
					for (; i < rzero ;) // i < i_max
					{
						if (i == next)
						{
							next    = sfb_ind_s[++sfb + 1] * MP3SFB_SWIN;
							win_len = sfb_ind_s[sfb + 1] -
									  sfb_ind_s[sfb];

							win_len_SSE   = win_len;
							win_len_SSE >>= 2;
							win_len_SSE <<= 2;

							if (sfb < MP3SFB_S)
							for (int win = 0; win < MP3SFB_SWIN; win++)
							{
								x1.m128         = _mm_load_ps1(pow2A + pow2A_ss * pow2A_sfs[sfb][win]);
								x1_x2[win].m128 = _mm_mul_ps(x1.m128, x2[win].m128);
							}
						}

					//	if (sfb >= MP3SFB_S)
					//	{
					//		for (byte win = 0; win < MP3SFB_SWIN; win++)
					//		for (uint16 n = 0; n < win_len; n++)
					//			mpeg_samples[i++] = 0;
					//	}
					//	else
					//	{
							for (byte win = 0; win < MP3SFB_SWIN; win++)
							{
								UNION128 & _x1_x2 = x1_x2[win];

								next_SSE       = i + win_len_SSE; // i value for full SSE steps
								next_SSE_chunk = i + win_len;     // i value for normal ending steps

								for (; i < next_SSE; i += 4)
									_RequantizeOpt_SSE(i, _x1_x2);

								for (; i < next_SSE_chunk; i++)
									_RequantizeOptCombo(i, _x1_x2.f[0]);
							}
					//	}
					} 
				}
				else // Long blocks only //
				{
					UNION128 x1, x2, x1_x2; // mul constants

					x2.m128 = _mm_load_ps1(pow2B + pow2B_gg);
					x1.m128 = _mm_load_ps1(pow2A + pow2A_ss * 
						                   (pow2A_sfl[sfb] + (preflag ? scalefac_pretab[sfb] : 0))
										  );
					x1_x2.m128 = _mm_mul_ps(x1.m128, x2.m128);

					next       = sfb_ind_l[sfb + 1];
					next_SSE   = next;
					next_SSE >>= 2; // next_SSE / _128bit_
					next_SSE <<= 2; // next_SSE * _128bit_
					for (uint16 i = 0; i < rzero;)
					{
						if (i == next)
						{
							next     = sfb_ind_l[++sfb + 1];
							next_SSE = next - i;
							next_SSE = next_SSE >> 2;
							next_SSE = (next_SSE << 2) + i;

							if (sfb < MP3SFB_L)
							{
								x1.m128 = _mm_load_ps1(pow2A + pow2A_ss * 
													   (pow2A_sfl[sfb] + (preflag ? scalefac_pretab[sfb] : 0))
													  );
								x1_x2.m128 = _mm_mul_ps(x1.m128, x2.m128);
							}
						}

					//	if (sfb >= MP3SFB_L)
					//		mpeg_samples[i++] = 0;
					//	else
					//	{
							for (; i < next_SSE; i += 4)
								_RequantizeOpt_SSE(i, x1_x2);

							for (; i < next; i++)
								_RequantizeOptCombo(i, x1_x2.f[0]);
					//	}
					}
				}
				#undef _RequantizeOpt_SSE
			}
			else
#endif		// SIMPLE CODE = 100% time
			{
				if (MPEGS.switch_flag[gr][ch] && MPEGS.block_type[gr][ch] == eMPEGBlockT_3ShortWindow)
				{
					uint16 i = 0;

					// Check for long blocks //

					if (MPEGS.mixed_block_flag[gr][ch]) // MIXED BLOCK L+S
					{
						float x2 = pow2B[pow2B_gg];
						float x1 = x2 * pow2A[pow2A_ss * (pow2A_sfl[sfb] + (preflag ? scalefac_pretab[sfb] : 0))];
						uint16 i_max = sfb_ind_l[MP3SFB_ML]; // i_max = 36 = sfb_ind_l[MP3SFB_ML], MP3SFB_ML = 8

						next = sfb_ind_l[sfb + 1];
						
						for (; i < i_max ;)
						{
							if (i == next)
							{
								next = sfb_ind_l[++sfb + 1];
								x1 = x2 * pow2A[pow2A_ss * (pow2A_sfl[sfb] + (preflag ? scalefac_pretab[sfb] : 0))];
							}

							//_RequantizeLong(i++, sfb);
							_RequantizeOptCombo(i++, x1);
						}
						sfb = MP3SFB_MS;
					}
			
					// Short blocks //

					// sfb = 0 ? 3
					next    = sfb_ind_s[sfb + 1] * MP3SFB_SWIN;
					win_len = sfb_ind_s[sfb + 1] -
							  sfb_ind_s[sfb];
		
					float x1[MP3SFB_SWIN]; // x1 for short blocks
					float x2[MP3SFB_SWIN]; // x2 for short blocks

					for (int win = 0; win < MP3SFB_SWIN; win++)
					{
						x2[win] = pow2B [ pow2B_gg + 256ul * pow2B_sbg[win] ];
						x1[win] = pow2A [ pow2A_ss * pow2A_sfs[sfb][win] ] * x2[win];
					}

					//i_max = rzero;
					// i = 0 ? 36
					for (; i < rzero ;) // i < i_max
					{
						if (i == next)
						{
							next    = sfb_ind_s[++sfb + 1] * MP3SFB_SWIN;
							win_len = sfb_ind_s[sfb + 1] -
									  sfb_ind_s[sfb];

							if (sfb < MP3SFB_S)
							for (int win = 0; win < MP3SFB_SWIN; win++)
								x1[win] = pow2A[ pow2A_ss * pow2A_sfs[sfb][win] ] * x2[win];
						}

					//	if (sfb >= MP3SFB_S)
					//	{
					//		for (byte win = 0; win < MP3SFB_SWIN; win++)
					//		for (uint16 n = 0; n < win_len; n++)
					//			mpeg_samples[i++] = 0;
					//	}
					//	else
					//	{
							for (byte win = 0; win < MP3SFB_SWIN; win++)
							{
								float _x1 = x1[win];
								for (uint16 n = 0; n < win_len; n++)
									//_RequantizeShort(i++, sfb, win);
									_RequantizeOptCombo(i++, _x1);
							}
					//	}
					}
				}
				else // Long blocks only //
				{
					float x2 = pow2B[pow2B_gg];
					float x1 = x2 * pow2A[pow2A_ss * (pow2A_sfl[sfb] + (preflag ? scalefac_pretab[sfb] : 0))];

					next = sfb_ind_l[sfb + 1];
					for (uint16 i = 0; i < rzero; )
					{
						if (i == next)
						{
							next = sfb_ind_l[++sfb + 1];

							if (sfb < MP3SFB_L)
							x1 = x2 * pow2A[pow2A_ss * (pow2A_sfl[sfb] + (preflag ? scalefac_pretab[sfb] : 0))];
						}

					//	if (sfb >= MP3SFB_L) 
					//		mpeg_samples[i++] = 0;
					//	else
							//_RequantizeLong(i++, sfb);
							_RequantizeOptCombo(i++, x1);
					}
				}
			}

		#undef _RequantizeLong	
		#undef _RequantizeShort
		#undef _RequantizeOpt
		#undef _RequantizeOptCombo

		}

		//>> 1.2. Восстановление порядка следования коротких блоков
		void ConvertMP3_Reorder(byte gr, byte ch)
		{
			if (MPEGS.switch_flag[gr][ch] && MPEGS.block_type[gr][ch] == eMPEGBlockT_3ShortWindow)
			{
				// Short blocks //

				uint16 win_len;
				uint16 next;    // id of frame of next sfb
				uint16 sfb = 0; // scalefactor band index
				auto   sfb_ind_l = sfb_indice[MPEG.freq].l;
				auto   sfb_ind_s = sfb_indice[MPEG.freq].s;
				auto   rzero = MPEGS.count1[gr][ch];

				float   sample[MP3GRS];					 // temp
				float * sample_o = MPEGD.sample[gr][ch]; // original location

				// Check for long blocks //

				if (MPEGS.mixed_block_flag[gr][ch]) // MIXED BLOCK L+S : do nothing with long blocks
					sfb = MP3SFB_MS;

				next    = sfb_ind_s[sfb + 1] * MP3SFB_SWIN;
				win_len = sfb_ind_s[sfb + 1] -
						  sfb_ind_s[sfb];

				// i = 0 ? 36
				for (uint16 i = (sfb) ? sfb_ind_l[MP3SFB_ML] : 0; i < MP3GRS;)
				{
					if (i == next)
					{
						// save results
						uint16 line = MP3SFB_SWIN * sfb_ind_s[sfb];
						uint16 n_max = win_len * MP3SFB_SWIN;
						//for (uint16 n = 0; n < n_max; n++)
						//	sample_o[line + n] = sample[n];
						memcpy(sample_o + line, sample, (n_max << 2)); // n_max * sizeof(float)

						if (i >= rzero)
							return;

						sfb++;
						next    = sfb_ind_s[sfb + 1] * MP3SFB_SWIN;
						win_len = sfb_ind_s[sfb + 1] -
								  sfb_ind_s[sfb];
					}

					for (byte win = 0; win < MP3SFB_SWIN; win++)
					for (uint16 n = 0; n < win_len; n++)
						sample[n * MP3SFB_SWIN + win] = sample_o[i++];
				}

				// save results - copy last sfb
				uint16 line = MP3SFB_SWIN * sfb_ind_s[MP3SFB_S];
				uint16 n_max = win_len * MP3SFB_SWIN;
				//for (uint16 n = 0; n < n_max; n++)
				//	sample_o[line + n] = sample[n];
				memcpy(sample_o + line, sample, (n_max << 2)); // n_max * sizeof(float)
			}
		}

		//>> 2. Этап декодирования для Joint Stereo
		void ConvertMP3_ProcessStereo(byte gr)
		{
			auto sfb_ind_l = sfb_indice[MPEG.freq].l;
			auto sfb_ind_s = sfb_indice[MPEG.freq].s;
			auto sample = MPEGD.sample[gr]; // [ch][576]

			auto ProcessStereoLong  = [this, &sfb_ind_l](byte gr, byte sfb) -> void
			{
				uint32 i_sample = MPEGD.scalefac_l[gr][0][sfb] % 8; // 4 bits : 0..15

				// no intensity stereo
				if (i_sample != MP3STEREOR)
				{
					for (uint32 i     = sfb_ind_l[sfb],
								i_max = sfb_ind_l[sfb + 1];
						i < i_max; i++)
					{
						float left  = ratio_l[i_sample] * MPEGD.sample[gr][0][i];
						float right = ratio_r[i_sample] * MPEGD.sample[gr][0][i];
						MPEGD.sample[gr][0][i] = left;
						MPEGD.sample[gr][1][i] = right;
					}
				}
			};
			auto ProcessStereoShort = [this, &sfb_ind_s](byte gr, byte sfb) -> void
			{
				uint16 win_len = sfb_ind_s[sfb + 1] -
								 sfb_ind_s[sfb];

				for (byte win = 0; win < MP3SFB_SWIN; win++)
				{
					uint16 i_sample = MPEGD.scalefac_s[gr][0][sfb][win] % 8; // 4 bits : 0..15

					// no intensity stereo
					if (i_sample != MP3STEREOR)
					{
						for (uint32 i = sfb_ind_s[sfb] * MP3SFB_SWIN + win_len * win,
									i_end = i + win_len;
							i < i_end; i++)
						{
							float left  = ratio_l[i_sample] * MPEGD.sample[gr][0][i];
							float right = ratio_r[i_sample] * MPEGD.sample[gr][0][i];
							MPEGD.sample[gr][0][i] = left;
							MPEGD.sample[gr][1][i] = right;
						}
					}
				}
			};

	#pragma region <ProcessStereoLong>

	#define _ProcessStereoLong(gr, sfb) {                                 \
				uint32 i_sample = MPEGD.scalefac_l[gr][0][sfb] % 8;       \
																		  \
				if (i_sample != MP3STEREOR)                               \
				{                                                         \
					for (uint32 i     = sfb_ind_l[sfb],                   \
								i_max = sfb_ind_l[sfb + 1];               \
						i < i_max; i++)                                   \
					{                                                     \
						sample[1][i]  = ratio_r[i_sample] * sample[0][i]; \
						sample[0][i] *= ratio_l[i_sample];                \
					}                                                     \
				}                                                         \
			}

	#pragma endregion

	#pragma region <ProcessStereoShort>

	#define _ProcessStereoShort(gr, sfb) {                                              \
				uint16 win_len = sfb_ind_s[sfb + 1] -                                   \
								 sfb_ind_s[sfb];                                        \
																						\
				for (byte win = 0; win < MP3SFB_SWIN; win++)                            \
				{                                                                       \
					uint16 i_sample = MPEGD.scalefac_s[gr][0][sfb][win] % 8;            \
																						\
					if (i_sample != MP3STEREOR)                                         \
					{                                                                   \
						for (uint32 i = sfb_ind_s[sfb] * MP3SFB_SWIN + win_len * win,   \
									i_end = i + win_len;                                \
							i < i_end; i++)                                             \
						{                                                               \
							sample[1][i]  = ratio_r[i_sample] * sample[0][i];           \
							sample[0][i] *= ratio_l[i_sample];                          \
						}                                                               \
					}                                                                   \
				}                                                                       \
			}

	#pragma endregion

			if ((MPEG.ch != eMPEGChJointStereo) || // not joint stereo
				(MPEG.layer != eMPEGLayer3))       // not layer 3
				return;

			// M/S stereo OFF, Intensity stereo OFF
			if (MPEG.chj == eMPEGChJBands4)
				return;

			uint16 rzero_1 = MPEGS.count1[gr][1];

			// M/S stereo ON
			if (MPEG.chj == eMPEGChJBands12 || MPEG.chj == eMPEGChJBands16)
			{ 
				uint16 rzero_0 = MPEGS.count1[gr][0];
				uint16 rzero   = (rzero_0 > rzero_1) ? rzero_0 : rzero_1;

				float * sample_0 = sample[0]; // ch 0
				float * sample_1 = sample[1]; // ch 1

#ifdef ENGINE_AVX512F
				if (opt_AVX512F_only) // AVX512F CODE = ??%..??% time _ NOT TESTED
				{
					#pragma message ("ConvertMP3_ProcessStereo : 512-bit AVX512F code not tested")

					uint16 i = 0;
					uint16 steps;

					steps = rzero >> 4; // (rzero / 16) * 16   _512bit AVX-512F
					steps = steps << 4; // .

					for (; i < steps; i += 16)
					{
						UNION512 a, b, c, r;

						a.m512 = _mm512_loadu_ps(sample_0 + i);
						b.m512 = _mm512_loadu_ps(sample_1 + i);

						c.m512 = _mm512_add_ps(a.m512, b.m512);
						r.m512 = _mm512_mul_ps(c.m512, uSqrt2inv512.m512);
						_mm512_storeu_ps(sample_0 + i, r.m512);

						c.m512 = _mm512_sub_ps(a.m512, b.m512);
						r.m512 = _mm512_mul_ps(c.m512, uSqrt2inv512.m512);
						_mm512_storeu_ps(sample_1 + i, r.m512);
					}

					steps = rzero >> 3; // (rzero /  8) * 8    _256bit AVX
					steps = steps << 3; // .

					for (; i < steps; i += 8)
					{
						UNION256 a, b, c, r;

						a.m256 = _mm256_loadu_ps(sample_0 + i);
						b.m256 = _mm256_loadu_ps(sample_1 + i);

						c.m256 = _mm256_add_ps(a.m256, b.m256);
						r.m256 = _mm256_mul_ps(c.m256, uSqrt2inv256.m256);
						_mm256_storeu_ps(sample_0 + i, r.m256);

						c.m256 = _mm256_sub_ps(a.m256, b.m256);
						r.m256 = _mm256_mul_ps(c.m256, uSqrt2inv256.m256);
						_mm256_storeu_ps(sample_1 + i, r.m256);
					}

					steps = rzero >> 2; // (rzero /  4) * 4    _128bit SSE
					steps = steps << 2; // .

					for (; i < steps; i += 4)
					{
						UNION128 a, b, c, r;

						a.m128 = _mm_loadu_ps(sample_0 + i);
						b.m128 = _mm_loadu_ps(sample_1 + i);

						c.m128 = _mm_add_ps(a.m128, b.m128);
						r.m128 = _mm_mul_ps(c.m128, uSqrt2inv128.m128);
						_mm_storeu_ps(sample_0 + i, r.m128);

						c.m128 = _mm_sub_ps(a.m128, b.m128);
						r.m128 = _mm_mul_ps(c.m128, uSqrt2inv128.m128);
						_mm_storeu_ps(sample_1 + i, r.m128);
					}

					for (; i < rzero; i++)
					{
						float left  = (sample_0[i] + sample_1[i]) * MathSqrt2inv;
						float right = (sample_0[i] - sample_1[i]) * MathSqrt2inv;
						sample_0[i] = left;
						sample_1[i] = right;
					}
				}
				else
#endif
#ifdef ENGINE_AVX
				if (opt_AVX_only) // AVX CODE = ??%..??% time _ NOT TESTED
				{
					#pragma message ("ConvertMP3_ProcessStereo : 256-bit AVX code not tested")

					uint16 i = 0;
					uint16 steps;
					
					steps = rzero >> 3; // (rzero /  8) * 8    _256bit AVX
					steps = steps << 3; // .

					for (; i < steps; i += 8)
					{
						UNION256 a, b, c, r;

						a.m256 = _mm256_loadu_ps(sample_0 + i);
						b.m256 = _mm256_loadu_ps(sample_1 + i);

						c.m256 = _mm256_add_ps(a.m256, b.m256);
						r.m256 = _mm256_mul_ps(c.m256, uSqrt2inv256.m256);
						_mm256_storeu_ps(sample_0 + i, r.m256);

						c.m256 = _mm256_sub_ps(a.m256, b.m256);
						r.m256 = _mm256_mul_ps(c.m256, uSqrt2inv256.m256);
						_mm256_storeu_ps(sample_1 + i, r.m256);
					}

					steps = rzero >> 2; // (rzero /  4) * 4    _128bit SSE
					steps = steps << 2; // .

					for (; i < steps; i += 4)
					{
						UNION128 a, b, c, r;

						a.m128 = _mm_loadu_ps(sample_0 + i);
						b.m128 = _mm_loadu_ps(sample_1 + i);

						c.m128 = _mm_add_ps(a.m128, b.m128);
						r.m128 = _mm_mul_ps(c.m128, uSqrt2inv128.m128);
						_mm_storeu_ps(sample_0 + i, r.m128);

						c.m128 = _mm_sub_ps(a.m128, b.m128);
						r.m128 = _mm_mul_ps(c.m128, uSqrt2inv128.m128);
						_mm_storeu_ps(sample_1 + i, r.m128);
					}

					for (; i < rzero; i++)
					{
						float left  = (sample_0[i] + sample_1[i]) * MathSqrt2inv;
						float right = (sample_0[i] - sample_1[i]) * MathSqrt2inv;
						sample_0[i] = left;
						sample_1[i] = right;
					}
				}
				else
#endif
#ifdef ENGINE_SSE			
				if (opt_SSE) // SSE CODE = 75%..80% time
				{
					uint16 i = 0;
					uint16 steps;

					steps = rzero >> 2; // (rzero /  4) * 4    _128bit SSE
					steps = steps << 2; // .

					for (; i < steps; i += 4)
					{
						UNION128 a, b, c, r;

						a.m128 = _mm_loadu_ps(sample_0 + i);
						b.m128 = _mm_loadu_ps(sample_1 + i);

						c.m128 = _mm_add_ps(a.m128, b.m128);
						r.m128 = _mm_mul_ps(c.m128, uSqrt2inv128.m128);
						_mm_storeu_ps(sample_0 + i, r.m128);

						c.m128 = _mm_sub_ps(a.m128, b.m128);
						r.m128 = _mm_mul_ps(c.m128, uSqrt2inv128.m128);
						_mm_storeu_ps(sample_1 + i, r.m128);
					}

					for (; i < rzero; i++)
					{
						float left  = (sample_0[i] + sample_1[i]) * MathSqrt2inv;
						float right = (sample_0[i] - sample_1[i]) * MathSqrt2inv;
						sample_0[i] = left;
						sample_1[i] = right;
					}
				}
				else
#endif			// SIMPLE CODE = 100% time
				{
					for (uint16 i = 0; i < rzero; i++)
					{
						float left  = (sample_0[i] + sample_1[i]) * MathSqrt2inv;
						float right = (sample_0[i] - sample_1[i]) * MathSqrt2inv;
						sample_0[i] = left;
						sample_1[i] = right;
					}
				}
			}

			// ! NOT TESTED ! (нужен рабочий семпл с intensity stereo)

			// Intensity stereo ON
			else //if (MPEG.chj == eMPEGChJBands8)
			{
				if (MPEGS.switch_flag[gr][0] && MPEGS.block_type[gr][0] == eMPEGBlockT_3ShortWindow)
				{
					if (MPEGS.mixed_block_flag[gr][0]) // MIXED BLOCK L+S
					{

						for (byte sfb = 0; sfb < MP3SFB_ML; sfb++)
							if (sfb_ind_l[sfb] >= rzero_1)
								_ProcessStereoLong(gr, sfb);

						for (byte sfb = MP3SFB_MS; sfb < MP3SFB_S; sfb++)
							if (sfb_ind_s[sfb] * MP3SFB_SWIN >= rzero_1)
								_ProcessStereoShort(gr, sfb);
					}
					else // Short blocks only //
					{
						for (byte sfb = 0; sfb < MP3SFB_S; sfb++)
							if (sfb_ind_s[sfb] * MP3SFB_SWIN >= rzero_1)
								_ProcessStereoShort(gr, sfb);
					}
				}
				else
				{
					// Long blocks //

					for (byte sfb = 0; sfb < MP3SFB_L; sfb++)
						if (sfb_ind_l[sfb] >= rzero_1)
							_ProcessStereoLong(gr, sfb);
				}
			}

			#undef _ProcessStereoLong
			#undef _ProcessStereoShort
		}

		//>> 3.1. Сглаживание длинных блоков
		void ConvertMP3_Antialias(byte gr, byte ch)
		{
			uint16 subband_max = MP3SUBBANDS; // 32 if L long blocks
	
			if (MPEGS.switch_flag[gr][ch] && MPEGS.block_type[gr][ch] == eMPEGBlockT_3ShortWindow)
			{
				if (!MPEGS.mixed_block_flag[gr][ch])
					 return;          // skip only S short blocks
				else subband_max = 2; // L+S (mixed block)
			}

			float * sample = MPEGD.sample[gr][ch];

#ifdef ENGINE_AVX512F
			if (opt_AVX512F_combo) // AVX512F CODE = ??%..??% time _ NOT TESTED
			{
				#pragma message ("ConvertMP3_Antialias : 512-bit AVX512F code not tested")

				// subband_max == 2  ---> i_max = (2  - 1) * 8 = 8 --------------------------> 1 step AVX
				// subband_max == 32 ---> i_max = (32 - 1) * 8 = 248 = 8 + 240 = 8 + 16x15 --> 1 step AVX + 15 steps AVX-256F

				uint16 i = 0;

				if (subband_max == MP3SUBBANDS) // make 15 steps AVX-256F
				{
					UNION512 sample_li, left;
					UNION512 sample_ui, right;
					UNION512 result;

					const uint16 i_max = (subband_max - 2) << 3; // "- 2" cause of 1 subband is 1 step AVX at the end
					for (; i < i_max; i += 16)
					{
						//                                           (sample + s2 - 8)    (sample + s2)    (sample + s2 + 8)
						// (sample + s1 - 8)      (sample + s1)      (sample + s1 + 8) 
						//        + ........<--...... + ........-->...... + ........<--...... + .......-->....... +
						//        |       -8..-1   region 1    0..7       |       -8..-1    region 2    0..7      |

						uint16 s1 = MP3LINES * (1 + (i >> 3)); // MP3LINES * subband:(1..subband_max)
						uint16 s2 = s1 + 16;

						float * _sample1 = sample + s1 - 8;
						float * _sample2 = sample + s2 - 8;

					//	sample_li.m256[0] = _mm256_loadu_ps(sample + s1 - 8); // -8..-1 (wrong) region 1
					//	sample_li.m256[1] = _mm256_loadu_ps(sample + s2 - 8); // -8..-1 (wrong) region 2

						sample_ui.m256[0] = _mm256_loadu_ps(sample + s1); //  0..7  region 1
						sample_ui.m256[1] = _mm256_loadu_ps(sample + s2); //  0..7  region 2

						sample_li.m256[0] = _mm256_set_ps ( // -1..-8 region 1
							_sample1[0],
							_sample1[1],
							_sample1[2],
							_sample1[3],
							_sample1[4],
							_sample1[5],
							_sample1[6],
							_sample1[7]                   );

						sample_li.m256[1] = _mm256_set_ps ( // -1..-8 region 2
							_sample2[0],
							_sample2[1],
							_sample2[2],
							_sample2[3],
							_sample2[4],
							_sample2[5],
							_sample2[6],
							_sample2[7]                   );

						left.m512   = _mm512_mul_ps(sample_li.m512, ucs2.m512);
						right.m512  = _mm512_mul_ps(sample_ui.m512, uca2.m512);
						result.m512 = _mm512_sub_ps(left.m512, right.m512);

						_sample1[0] = result.m256[0].f[7];   _sample2[0] = result.m256[1].f[7];
						_sample1[1] = result.m256[0].f[6];   _sample2[1] = result.m256[1].f[6];
						_sample1[2] = result.m256[0].f[5];   _sample2[2] = result.m256[1].f[5];
						_sample1[3] = result.m256[0].f[4];   _sample2[3] = result.m256[1].f[4];
						_sample1[4] = result.m256[0].f[3];   _sample2[4] = result.m256[1].f[3];
						_sample1[5] = result.m256[0].f[2];   _sample2[5] = result.m256[1].f[2];
						_sample1[6] = result.m256[0].f[1];   _sample2[6] = result.m256[1].f[1];
						_sample1[7] = result.m256[0].f[0];   _sample2[7] = result.m256[1].f[0];

						left.m512   = _mm512_mul_ps(sample_ui.m512, ucs2.m512);
						right.m512  = _mm512_mul_ps(sample_li.m512, uca2.m512); 
						result.m512 = _mm512_add_ps(left.m512, right.m512);

						_mm256_storeu_ps(sample + s1, result.m256[0]);
						_mm256_storeu_ps(sample + s2, result.m256[1]);
					}
				}

				// make 1 step AVX

				UNION256 sample_li, left;
				UNION256 sample_ui, right;
				UNION256 result;

				const uint16 i_max = (subband_max - 1) << 3;
				for (; i < i_max; i += 8)
				{
					// (sample + s - 8)      (sample + s)       (sample + s + 8) == (sample + next_s - 8)
					//        + .......<--....... + .......-->....... +

					uint16 s = MP3LINES * (1 + (i >> 3)); // MP3LINES * subband:(1..subband_max)
					float * _sample = sample + s - 8;     // 

				//	sample_li.m256 = _mm256_loadu_ps(sample + s - 8); // -8..-1 (wrong)
					sample_ui.m256 = _mm256_loadu_ps(sample + s);     //  0..7

					sample_li.m256 = _mm256_set_ps ( // -1..-8
						_sample[0],
						_sample[1],
						_sample[2],
						_sample[3],
						_sample[4],
						_sample[5],
						_sample[6],
						_sample[7]                 );

					left.m256   = _mm256_mul_ps(sample_li.m256, ucs.m256);
					right.m256  = _mm256_mul_ps(sample_ui.m256, uca.m256);
					result.m256 = _mm256_sub_ps(left.m256, right.m256);

					_sample[0] = result.f[7];
					_sample[1] = result.f[6];
					_sample[2] = result.f[5];
					_sample[3] = result.f[4];
					_sample[4] = result.f[3];
					_sample[5] = result.f[2];
					_sample[6] = result.f[1];
					_sample[7] = result.f[0];
			
					left.m256   = _mm256_mul_ps(sample_ui.m256, ucs.m256);
					right.m256  = _mm256_mul_ps(sample_li.m256, uca.m256); 
					result.m256 = _mm256_add_ps(left.m256, right.m256);

					_mm256_storeu_ps(sample + s, result.m256);
				}
			}
			else
#endif
#ifdef ENGINE_AVX
			if (opt_AVX) // AVX CODE = ??%..??% time _ NOT TESTED
			{
				#pragma message ("ConvertMP3_Antialias : 256-bit AVX code not tested")

				UNION256 sample_li, left;
				UNION256 sample_ui, right;
				UNION256 result;

				const uint16 i_max = (subband_max - 1) << 3; // * MP3ANTIALI; // кратно 8 (длина шага 1-ого subband)
				for (uint16 i = 0; i < i_max; i += 8)
				{
					// (sample + s - 8)      (sample + s)       (sample + s + 8) == (sample + next_s - 8)
					//        + ................. + ................. +

					uint16 s = MP3LINES * (1 + (i >> 3)); // MP3LINES * subband:(1..subband_max)
					float * _sample = sample + s - 8;     // 

				//	sample_li.m256 = _mm256_loadu_ps(sample + s - 8); // -8..-1 (wrong)
					sample_ui.m256 = _mm256_loadu_ps(sample + s);     //  0..7

					sample_li.m256 = _mm256_set_ps ( // -1..-8
						_sample[0],
						_sample[1],
						_sample[2],
						_sample[3],
						_sample[4],
						_sample[5],
						_sample[6],
						_sample[7]                 );

					left.m256   = _mm256_mul_ps(sample_li.m256, ucs.m256);
					right.m256  = _mm256_mul_ps(sample_ui.m256, uca.m256);
					result.m256 = _mm256_sub_ps(left.m256, right.m256);

					_sample[0] = result.f[7];
					_sample[1] = result.f[6];
					_sample[2] = result.f[5];
					_sample[3] = result.f[4];
					_sample[4] = result.f[3];
					_sample[5] = result.f[2];
					_sample[6] = result.f[1];
					_sample[7] = result.f[0];
			
					left.m256   = _mm256_mul_ps(sample_ui.m256, ucs.m256);
					right.m256  = _mm256_mul_ps(sample_li.m256, uca.m256); 
					result.m256 = _mm256_add_ps(left.m256, right.m256);

					_mm256_storeu_ps(sample + s, result.m256);
				}
			}
			else
#endif
#ifdef ENGINE_SSE
			if (opt_SSE) // SSE CODE = 70%..80% time
			{
				UNION128 sample_li, left;
				UNION128 sample_ui, right;
				UNION128 result;

				const uint16 i_max = (subband_max - 1) << 3; // * MP3ANTIALI; // кратно 8 (длина шага 1-ого subband)
				for (uint16 i = 0; i < i_max; i += 4)
				{
					uint16 n   = i & 7;  // n   = 0 ? 4
					uint16 n01 = n >> 2; // n01 = 0 ? 1
					uint16 s = MP3LINES * (1 + (i >> 3)); // MP3LINES * subband:(1..subband_max)
					float * _sample = sample + s - 4 - n;

				//	sample_li.m128 = _mm_loadu_ps(sample + s - 4 - n); // -4..-1, -8..-5 (wrong)
					sample_ui.m128 = _mm_loadu_ps(sample + s + n);     //  0.. 3,  4.. 7

					sample_li.m128 = _mm_set_ps ( // -1..-4, -5..-8
						_sample[0],
						_sample[1],
						_sample[2],
						_sample[3]              );

					left.m128   = _mm_mul_ps(sample_li.m128, ucs.m128[n01]); // m128[ 0 ? 1 ]
					right.m128  = _mm_mul_ps(sample_ui.m128, uca.m128[n01]); // .
					result.m128 = _mm_sub_ps(left.m128, right.m128);

					_sample[0] = result.f[3];
					_sample[1] = result.f[2];
					_sample[2] = result.f[1];
					_sample[3] = result.f[0];
					
					left.m128   = _mm_mul_ps(sample_ui.m128, ucs.m128[n01]); // m128[ 0 ? 1 ]
					right.m128  = _mm_mul_ps(sample_li.m128, uca.m128[n01]); // .
					result.m128 = _mm_add_ps(left.m128, right.m128);

					_mm_storeu_ps(sample + s + n, result.m128);
				}
			}
			else
#endif		// SIMPLE CODE = 100% time
			{
				for (uint16 subband = 1; subband < subband_max; subband++) // 31  (31 * 8 = 248)
				for (uint16 i = 0; i < MP3ANTIALI; i++)                    // 8
				{
					// -8, -7, ..., -1 (li)
					//  7,  6, ...,  0 (ul)
					
					uint16 li = MP3LINES * subband - 1 - i;
					uint16 ui = MP3LINES * subband + i;
					float lb = sample[li] * ucs.f[i] - sample[ui] * uca.f[i];
					float ub = sample[ui] * ucs.f[i] + sample[li] * uca.f[i];
					sample[li] = lb;
					sample[ui] = ub;
				}
			}
		}

		//>> 3.2. IMDCT Обратное модифицированное дискретное косинусное преобразование
		void ConvertMP3_HybridSynth_simple(byte gr, byte ch)
		{
			float   rawout[MP3LINES * 2];          // raw out [36]
			float * sample = MPEGD.sample[gr][ch]; // sample [576]

			auto IMDCT = [this, sample] (float * rawout, uint16 level, byte block_type) -> void
			{		
				float * in = sample + level;

				ZeroMemory(rawout, sizeof(rawout));

				if (block_type == eMPEGBlockT_3ShortWindow)
				{
					const uint16 N   = 12;
					const uint16 N2  = N / 2;
					//const uint16 Nx2 = N * 2;
					//const uint16 N2p = N2 + 1;

					float out_temp[N];

					for (uint16 i=0; i < 3; i++)
					{
						for (uint16 p=0; p < N; p++)
						{
							float sum = 0;

							for (uint16 m = 0; m < N2; m++)
								//sum += in[i + 3 * m] * cos(MathPI / Nx2 * (2 * p + N2p) * (2 * m + 1));
								sum += in[i + 3 * m] * IMDCT_cos1tab[p + N * m];

							out_temp[p] = sum * IMDCT_sin[block_type][p];
						}

						for (uint16 p = 0; p < N; p++)
						{
							rawout[6 * i + p + 6] += out_temp[p];
						}
					}
				}
				else
				{
					const uint16 N   = 36;
					const uint16 N2  = N / 2;
					//const uint16 Nx2 = N * 2;
					//const uint16 N2p = N2 + 1;

					for (uint16 p = 0; p < N; p++)
					{
						float sum = 0;

						for (uint16 m=0; m < N2; m++)
							//sum += in[m] * cos(MathPI / Nx2 * (2 * p + N2p) * (2 * m + 1));
							sum += in[m] * IMDCT_cos2tab[p + N * m];

						rawout[p] = sum * IMDCT_sin[block_type][p];
					}
				}
			};

			for (uint16 subband = 0; subband < MP3SUBBANDS; subband++)
			{
				byte block_type = MPEGS.block_type[gr][ch];
				uint16 level = subband * MP3LINES;			

				if (subband < 2 && MPEGS.switch_flag[gr][ch] && MPEGS.mixed_block_flag[gr][ch])
					 block_type = 0;		

				IMDCT(rawout, level, block_type);

				for (uint16 line = 0; line < MP3LINES; line++)
				{
					sample[level + line] = rawout[line] + storehs[ch][subband][line];
					storehs[ch][subband][line] = rawout[line + MP3LINES];
				}
			}
		}

		//>> 3.2. IMDCT Обратное модифицированное дискретное косинусное преобразование - fast ver.
		void ConvertMP3_HybridSynth(byte gr, byte ch)
		{
			float   rawout[MP3LINES * 2];	       // raw out [36]
			float * sample = MPEGD.sample[gr][ch]; // sample [576]
			 auto   storehs = this->storehs[ch];   // [32][18]
	
			auto IMDCT_3 = [this] (float in[3], float out[3]) -> void
			{
				float t0, t1;

				t0 = in[2] * 0.5f + in[0];
				t1 = in[1] * c_sqrt32;

				out[0] = t0 + t1;
				out[1] = in[0] - in[2];
				out[2] = t0 - t1;
			};
			auto IMDCT_Short = [this, IMDCT_3] (float in[6], float out[12]) -> void
			{
				int i;
				float H[6], h[6], even[3], odd[3], even_idct[3], odd_idct[3];

				H[0] = in[0];
				for (i = 1; i < 6; i++)
					H[i] = in[i - 1] + in[i];

				even[0] = H[0];
				even[1] = H[2];
				even[2] = H[4];

				IMDCT_3 (even, even_idct);

				odd[0] = H[1];
				odd[1] = H[1] + H[3];
				odd[2] = H[3] + H[5];

				IMDCT_3 (odd, odd_idct);

				odd_idct[0] *= form_3_6[0];
				odd_idct[1] *= form_3_6[1];
				odd_idct[2] *= form_3_6[2];

				h[0] = form_6_12[0] * (even_idct[0] + odd_idct[0]);
				h[1] = form_6_12[1] * (even_idct[1] + odd_idct[1]);
				h[2] = form_6_12[2] * (even_idct[2] + odd_idct[2]);

				h[3] = form_6_12[3] * (even_idct[2] - odd_idct[2]);
				h[4] = form_6_12[4] * (even_idct[1] - odd_idct[1]);
				h[5] = form_6_12[5] * (even_idct[0] - odd_idct[0]);

				out[0] =  h[3];   out[4] = -h[4];   out[8]  = -h[0];
				out[1] =  h[4];   out[5] = -h[3];   out[9]  = -h[0];
				out[2] =  h[5];   out[6] = -h[2];   out[10] = -h[1];
				out[3] = -h[5];   out[7] = -h[1];   out[11] = -h[2];
			};
			auto IMDCT_4 = [this] (float in[4], float out[4]) -> void
			{
				float t0, t1;

				t0 = in[3] * 0.5f + in[0];
				t1 = in[1] - in[2];

				out[0] = t0 + in[1] * c_cos9  + in[2] * c_cos92;
				out[1] = t1 * 0.5f  + in[0] - in[3];
				out[2] = t0 - in[1] * c_cos94 - in[2] * c_cos9;
				out[3] = t0 - in[1] * c_cos92 + in[2] * c_cos94;
			};
			auto IMDCT_5 = [this] (float in[5], float out[5]) -> void
			{
				float t0, t1, t2;

				t0 = in[3] * 0.5f + in[0];
				t1 = in[0] - in[3];
				t2 = in[1] - in[2] - in[4];

				out[0] = t0 + in[1] * c_cos9  + in[2] * c_cos92 + in[4] * c_cos94;
				out[1] = t2 * 0.5f + t1;
				out[2] = t0 - in[1] * c_cos94 - in[2] * c_cos9  + in[4] * c_cos92;
				out[3] = t0 - in[1] * c_cos92 + in[2] * c_cos94 - in[4] * c_cos9;
				out[4] = t1 - t2;
			};
			auto IMDCT_9 = [this, IMDCT_5, IMDCT_4](float in[9], float out[9]) -> void
			{
				int i;
				float H[9], h[9], even[5], odd[4], even_idct[5], odd_idct[4];

				for (i = 0; i < 9; i++)
					H[i] = in[i];

				for (i = 0; i < 5; i++)
					even[i] = H[2 * i];

				IMDCT_5(even, even_idct);

				odd[0] = H[1];
				for (i = 1; i < 4; i++)
					odd[i] = H[2 * i - 1] + H[2 * i + 1];

				IMDCT_4(odd, odd_idct);

				odd_idct[0] +=  in[7] * c_sin0;
				odd_idct[1] += -in[7] * c_sin1;
				odd_idct[2] +=  in[7] * c_sin2;
				odd_idct[3] += -in[7] * c_sin3;

				for (i = 0; i < 4; i++)
					odd_idct[i] *= form_4_9[i];

				for (i = 0; i < 4; i++)
					h[i] = even_idct[i] + odd_idct[i];

				h[4] = even_idct[4];

				for (i = 5; i < 9; i++)
					h[i] = even_idct[8 - i] - odd_idct[8 - i];

				for (i = 0; i < 9; i++)
					out[i] = h[i];
			};
			auto IMDCT_Long = [this, IMDCT_9](float in[18], float out[36]) -> void
			{
				int i;
				float H[18], h[18], even[9], odd[9], even_idct[9], odd_idct[9];

				H[0] = in[0];
				for (i = 1; i < 18; i++)
					H[i] = in[i - 1] + in[i];

				for (i = 0; i < 9; i++)
					even[i] = H[i * 2];

				IMDCT_9(even, even_idct);

				odd[0] = H[1];
				for (i = 1; i < 9; i++)
					odd[i] = H[i * 2 - 1] + H[i * 2 + 1];

				IMDCT_9(odd, odd_idct);

				for (i = 0; i < 9; i++)
					odd_idct[i] *= form_9_18[i];

				for (i = 0; i < 9; i++)
					h[i] = even_idct[i] + odd_idct[i];

				for (i = 9; i < 18; i++)
					h[i] = even_idct[17 - i] - odd_idct[17 - i];

				for (i = 0; i < 18; i++)
					h[i] *= form_18_36[i];

				out[0] = h[9];    out[9]  = -h[17];   out[18] = -h[8];   out[27] = -h[0];
				out[1] = h[10];   out[10] = -h[16];   out[19] = -h[7];   out[28] = -h[1];
				out[2] = h[11];   out[11] = -h[15];   out[20] = -h[6];   out[29] = -h[2];
				out[3] = h[12];   out[12] = -h[14];   out[21] = -h[5];   out[30] = -h[3];
				out[4] = h[13];   out[13] = -h[13];   out[22] = -h[4];   out[31] = -h[4];
				out[5] = h[14];   out[14] = -h[12];   out[23] = -h[3];   out[32] = -h[5];
				out[6] = h[15];   out[15] = -h[11];   out[24] = -h[2];   out[33] = -h[6];
				out[7] = h[16];   out[16] = -h[10];   out[25] = -h[1];   out[34] = -h[7];
				out[8] = h[17];   out[17] = -h[9];    out[26] = -h[0];   out[35] = -h[8];
			};
			auto IMDCT = [this, IMDCT_Short, IMDCT_Long, sample, &rawout](uint16 level, byte block_type) -> void
			{
				float * in = sample + level;

				ZeroMemory(rawout, sizeof(rawout));

				if (block_type == eMPEGBlockT_3ShortWindow)
				{
					float out[12];
					float _in[18];

					for (uint16 i = 0; i < 3; i++)
					for (uint16 m = 0; m < 6; m++)
						_in[i * 6 + m] = in[i + 3 * m];
				
					for (uint16 i = 0; i < 3; i++)
					{			
						IMDCT_Short(_in + 6 * i, out);
				
						for (uint16 p = 0; p < 12; p++)
							rawout[6 * i + p + 6] += out[p] * IMDCT_sin[block_type][p];
					}
				}
				else
				{
					IMDCT_Long(in, rawout);
				
					for (uint16 i = 0; i < 36; i++)
						rawout[i] *= IMDCT_sin[block_type][i];
				}
			};

	#pragma region <IMDCT_3>

			#define _IMDCT_3(in, out) {      \
				float t0, t1;                \
											 \
				t0 = in[2] * 0.5f + in[0];   \
				t1 = in[1] * c_sqrt32;       \
											 \
				out[0] = t0 + t1;            \
				out[1] = in[0] - in[2];      \
				out[2] = t0 - t1;            \
			}

	#pragma endregion

	#pragma region <IMDCT_Short>

			#define _IMDCT_Short(in, out) {                                    \
				int i;                                                         \
				float H[6], h[6], even[3], odd[3], even_idct[3], odd_idct[3];  \
																			   \
				H[0] = in[0];                                                  \
				for (i = 1; i < 6; i++)                                        \
					H[i] = in[i - 1] + in[i];                                  \
																			   \
				even[0] = H[0];                                                \
				even[1] = H[2];                                                \
				even[2] = H[4];                                                \
																			   \
				_IMDCT_3(even, even_idct);                                     \
																			   \
				odd[0] = H[1];                                                 \
				odd[1] = H[1] + H[3];                                          \
				odd[2] = H[3] + H[5];                                          \
																			   \
				_IMDCT_3(odd, odd_idct);                                       \
																			   \
				odd_idct[0] *= form_3_6[0];                                    \
				odd_idct[1] *= form_3_6[1];                                    \
				odd_idct[2] *= form_3_6[2];                                    \
																			   \
				h[0] = form_6_12[0] * (even_idct[0] + odd_idct[0]);            \
				h[1] = form_6_12[1] * (even_idct[1] + odd_idct[1]);            \
				h[2] = form_6_12[2] * (even_idct[2] + odd_idct[2]);            \
																			   \
				h[3] = form_6_12[3] * (even_idct[2] - odd_idct[2]);            \
				h[4] = form_6_12[4] * (even_idct[1] - odd_idct[1]);            \
				h[5] = form_6_12[5] * (even_idct[0] - odd_idct[0]);            \
																			   \
				out[0] =  h[3];   out[4] = -h[4];   out[8]  = -h[0];           \
				out[1] =  h[4];   out[5] = -h[3];   out[9]  = -h[0];           \
				out[2] =  h[5];   out[6] = -h[2];   out[10] = -h[1];           \
				out[3] = -h[5];   out[7] = -h[1];   out[11] = -h[2];           \
			}

	#pragma endregion

	#pragma region <IMDCT_4>

			#define _IMDCT_4(in, out) {                          \
				float t0, t1;                                    \
																 \
				t0 = in[3] * 0.5f + in[0];                       \
				t1 = in[1] - in[2];                              \
																 \
				out[0] = t0 + in[1] * c_cos9  + in[2] * c_cos92; \
				out[1] = t1 * 0.5f  + in[0] - in[3];             \
				out[2] = t0 - in[1] * c_cos94 - in[2] * c_cos9;  \
				out[3] = t0 - in[1] * c_cos92 + in[2] * c_cos94; \
			}

	#pragma endregion

	#pragma region <IMDCT_5>

			#define _IMDCT_5(in, out) {                                             \
				float t0, t1, t2;                                                   \
																					\
				t0 = in[3] * 0.5f + in[0];                                          \
				t1 = in[0] - in[3];                                                 \
				t2 = in[1] - in[2] - in[4];                                         \
																					\
				out[0] = t0 + in[1] * c_cos9  + in[2] * c_cos92 + in[4] * c_cos94;  \
				out[1] = t2 * 0.5f + t1;                                            \
				out[2] = t0 - in[1] * c_cos94 - in[2] * c_cos9  + in[4] * c_cos92;  \
				out[3] = t0 - in[1] * c_cos92 + in[2] * c_cos94 - in[4] * c_cos9;   \
				out[4] = t1 - t2;                                                   \
			}

	#pragma endregion

	#pragma region <IMDCT_9>
		
			#define _IMDCT_9(in, out) {                                                \
				int i;                                                                 \
				float H9[9], h9[9], even9[5], odd9[4], even_idct9[5], odd_idct9[4];    \
																					   \
				for (i = 0; i < 9; i++)                                                \
					H9[i] = in[i];                                                     \
																					   \
				for (i = 0; i < 5; i++)                                                \
					even9[i] = H9[2 * i];                                              \
																					   \
				_IMDCT_5(even9, even_idct9);                                           \
																					   \
				odd9[0] = H9[1];                                                       \
				for (i = 1; i < 4; i++)                                                \
					odd9[i] = H9[2 * i - 1] + H9[2 * i + 1];                           \
																					   \
				_IMDCT_4(odd9, odd_idct9);                                             \
																					   \
				odd_idct9[0] +=  in[7] * c_sin0;                                       \
				odd_idct9[1] += -in[7] * c_sin1;                                       \
				odd_idct9[2] +=  in[7] * c_sin2;                                       \
				odd_idct9[3] += -in[7] * c_sin3;                                       \
																					   \
				for (i = 0; i < 4; i++)                                                \
					odd_idct9[i] *= form_4_9[i];                                       \
																					   \
				for (i = 0; i < 4; i++)                                                \
					h9[i] = even_idct9[i] + odd_idct9[i];                              \
																					   \
				h9[4] = even_idct9[4];                                                 \
																					   \
				for (i = 5; i < 9; i++)                                                \
					h9[i] = even_idct9[8 - i] - odd_idct9[8 - i];                      \
																					   \
				for (i = 0; i < 9; i++)                                                \
					out[i] = h9[i];                                                    \
			}

	#pragma endregion

	#pragma region <IMDCT_Long>

			#define _IMDCT_Long(in, out) {                                                              \
				int i;                                                                                  \
				float H[18], h[18], even[9], odd[9], even_idct[9], odd_idct[9];                         \
																										\
				H[0] = in[0];                                                                           \
				for (i = 1; i < 18; i++)                                                                \
					H[i] = in[i - 1] + in[i];                                                           \
																										\
				for (i = 0; i < 9; i++)                                                                 \
					even[i] = H[i * 2];                                                                 \
																										\
				_IMDCT_9(even, even_idct);                                                              \
																										\
				odd[0] = H[1];                                                                          \
				for (i = 1; i < 9; i++)                                                                 \
					odd[i] = H[i * 2 - 1] + H[i * 2 + 1];                                               \
																										\
				_IMDCT_9(odd, odd_idct);                                                                \
																										\
				for (i = 0; i < 9; i++)                                                                 \
					odd_idct[i] *= form_9_18[i];                                                        \
																										\
				for (i = 0; i < 9; i++)                                                                 \
					h[i] = even_idct[i] + odd_idct[i];                                                  \
																										\
				for (i = 9; i < 18; i++)                                                                \
					h[i] = even_idct[17 - i] - odd_idct[17 - i];                                        \
																										\
				for (i = 0; i < 18; i++)                                                                \
					h[i] *= form_18_36[i];                                                              \
				                                                                                        \
				memcpy(out, h + 9, 36);                                                                 \
																										\
				out[9]  = -h[17];   out[18] = -h[8];   out[27] = -h[0];                                 \
				out[10] = -h[16];   out[19] = -h[7];   out[28] = -h[1];                                 \
				out[11] = -h[15];   out[20] = -h[6];   out[29] = -h[2];                                 \
				out[12] = -h[14];   out[21] = -h[5];   out[30] = -h[3];                                 \
				out[13] = -h[13];   out[22] = -h[4];   out[31] = -h[4];                                 \
				out[14] = -h[12];   out[23] = -h[3];   out[32] = -h[5];                                 \
				out[15] = -h[11];   out[24] = -h[2];   out[33] = -h[6];                                 \
				out[16] = -h[10];   out[25] = -h[1];   out[34] = -h[7];                                 \
				out[17] = -h[9];    out[26] = -h[0];   out[35] = -h[8];                                 \
			}

	#pragma endregion

	#pragma region <IMDCT>

			#define _IMDCT(level, block_type) {                        \
				float * in = sample + level;                           \
																	   \
				ZeroMemory(rawout, sizeof(rawout));                    \
																	   \
				if (block_type == eMPEGBlockT_3ShortWindow)            \
				{                                                      \
					float out[12];                                     \
					float _in[18];                                     \
																	   \
					for (uint16 i = 0; i < 3; i++)                     \
					for (uint16 m = 0; m < 6; m++)                     \
						_in[i * 6 + m] = in[i + 3 * m];                \
																	   \
					for (uint16 i = 0; i < 3; i++)                     \
					{                                                  \
						in = _in + i * 6;                              \
						_IMDCT_Short(in, out);                         \
																	   \
						for (uint16 p = 0; p < 12; p++)                \
							rawout[6 * i + p + 6] +=                   \
								out[p] * IMDCT_sin[block_type][p];     \
					}                                                  \
				}                                                      \
				else                                                   \
				{                                                      \
					_IMDCT_Long(in, rawout);                           \
																	   \
					for (uint16 i = 0; i < 36; i++)                    \
						rawout[i] *= IMDCT_sin[block_type][i];         \
				}                                                      \
			}

	#pragma endregion


#ifdef ENGINE_SSE
			if (opt_SSE_only) // SSE CODE = 80%..85% time
			{

	#pragma region <IMDCT_Short_SSE>

			#define _IMDCT_Short_SSE(in, out) {                                \
				float H[6], h[7], even[3], odd[3], even_idct[4], odd_idct[4];  \
				UNION128 a, b, r;                                              \
																			   \
				H[0] = in[0];                                                  \
				H[5] = in[4] + in[5];                                          \
				{                                                              \
					a.m128 = _mm_loadu_ps(in);                                 \
					b.m128 = _mm_loadu_ps(in + 1);                             \
					r.m128 = _mm_add_ps(a.m128, b.m128);                       \
					_mm_storeu_ps(H + 1, r.m128);                              \
				}                                                              \
																			   \
				even[0] = H[0];                                                \
				even[1] = H[2];                                                \
				even[2] = H[4];                                                \
																			   \
				_IMDCT_3(even, even_idct);                                     \
																			   \
				odd[0] = H[1];                                                 \
				odd[1] = H[1] + H[3];                                          \
				odd[2] = H[3] + H[5];                                          \
																			   \
				_IMDCT_3(odd, odd_idct);                                       \
																			   \
				{                                                              \
					a.m128 = _mm_loadu_ps(odd_idct);                           \
					r.m128 = _mm_mul_ps(a.m128, u_form_3_6.m128);              \
					_mm_storeu_ps(odd_idct, r.m128);                           \
				}                                                              \
																			   \
				{                                                              \
					a.m128 = _mm_loadu_ps(even_idct);                          \
					b.m128 = _mm_loadu_ps(odd_idct);                           \
					r.m128 = _mm_add_ps(a.m128, b.m128);                       \
					a.m128 = _mm_mul_ps(r.m128, u_form_6_12.m128[0]);          \
					_mm_storeu_ps(h, a.m128);                                  \
				}                                                              \
																			   \
				{                                                              \
					a.m128 = _mm_loadu_ps(even_idct);                          \
					b.m128 = _mm_loadu_ps(odd_idct);                           \
					r.m128 = _mm_sub_ps(a.m128, b.m128);                       \
					a.f[0] = r.f[2];                                           \
					a.f[1] = r.f[1];                                           \
					a.f[2] = r.f[0];                                           \
					r.m128 = _mm_mul_ps(a.m128, u_form_6_12_3.m128);           \
					_mm_storeu_ps(h + 3, r.m128);                              \
				}                                                              \
																			   \
				out[0] =  h[3];   out[4] = -h[4];   out[8]  = -h[0];           \
				out[1] =  h[4];   out[5] = -h[3];   out[9]  = -h[0];           \
				out[2] =  h[5];   out[6] = -h[2];   out[10] = -h[1];           \
				out[3] = -h[5];   out[7] = -h[1];   out[11] = -h[2];           \
			}

	#pragma endregion

	#pragma region <IMDCT_9_SSE>
		
			#define _IMDCT_9_SSE(in, out) {                                            \
				int i;                                                                 \
				float H9[9], h9[9], even9[5], odd9[4], even_idct9[5], odd_idct9[4];    \
				UNION128 a, b, r;                                                      \
				                                                                       \
				memcpy(H9, in, 36);                                                    \
																					   \
				for (i = 0; i < 5; i++)                                                \
					even9[i] = H9[2 * i];                                              \
																					   \
				_IMDCT_5(even9, even_idct9);                                           \
																					   \
				odd9[0] = H9[1];                                                       \
				for (i = 1; i < 4; i++)                                                \
					odd9[i] = H9[2 * i - 1] + H9[2 * i + 1];                           \
																					   \
				_IMDCT_4(odd9, odd_idct9);                                             \
																					   \
				{                                                                      \
					b.m128 = _mm_load_ps1(in + 7);                                     \
					b.f[1] = -b.f[1];                                                  \
					b.f[3] = -b.f[3];                                                  \
					r.m128 = _mm_mul_ps(c_sin.m128, b.m128);                           \
					a.m128 = _mm_loadu_ps(odd_idct9);                                  \
					b.m128 = _mm_add_ps(a.m128, r.m128);                               \
					_mm_storeu_ps(odd_idct9, b.m128);                                  \
			    }                                                                      \
																					   \
				{                                                                      \
					a.m128 = _mm_loadu_ps(odd_idct9);                                  \
					r.m128 = _mm_mul_ps(a.m128, u_form_4_9.m128);                      \
					_mm_storeu_ps(odd_idct9, r.m128);                                  \
				}                                                                      \
																					   \
				{                                                                      \
					a.m128 = _mm_loadu_ps(even_idct9);                                 \
					b.m128 = _mm_loadu_ps(odd_idct9);                                  \
					r.m128 = _mm_add_ps(a.m128, b.m128);                               \
					_mm_storeu_ps(h9, r.m128);                                         \
					r.m128 = _mm_sub_ps(a.m128, b.m128);                               \
					h9[4] = even_idct9[4];                                             \
					h9[5] = r.f[3];                                                    \
					h9[6] = r.f[2];                                                    \
					h9[7] = r.f[1];                                                    \
					h9[8] = r.f[0];                                                    \
				}                                                                      \
																					   \
				memcpy(out, h9, 36);                                                   \
			}

	#pragma endregion

	#pragma region <IMDCT_Long_SSE>

			#define _IMDCT_Long_SSE(in, out) {                                                          \
				int i;                                                                                  \
				float H[18], h[18], even[9], odd[9], even_idct[9], odd_idct[9];                         \
				UNION128 a, b, r;                                                                       \
																										\
				H[0]  = in[0];                                                                          \
				H[17] = in[16] + in[17];                                                                \
				for (i = 1; i < 17; i += 4)                                                             \
				{                                                                                       \
					a.m128 = _mm_loadu_ps(in + i);                                                      \
					b.m128 = _mm_loadu_ps(in + i - 1);                                                  \
					r.m128 = _mm_add_ps(a.m128, b.m128);                                                \
					_mm_storeu_ps(H + i, r.m128);                                                       \
				}                                                                                       \
																										\
				for (i = 0; i < 9; i++)                                                                 \
					even[i] = H[i * 2];                                                                 \
																										\
				_IMDCT_9_SSE(even, even_idct);                                                          \
																										\
				odd[0] = H[1];                                                                          \
				for (i = 1; i < 9; i++)                                                                 \
					odd[i] = H[i * 2 - 1] + H[i * 2 + 1];                                               \
																										\
				_IMDCT_9_SSE(odd, odd_idct);                                                            \
																										\
				for (i = 0; i < 8; i += 4)                                                              \
				{                                                                                       \
					a.m128 = _mm_loadu_ps(odd_idct + i);                                                \
					r.m128 = _mm_mul_ps(a.m128, u_form_9_18.m128[i>>2]);                                \
					_mm_storeu_ps(odd_idct + i, r.m128);                                                \
				}                                                                                       \
				odd_idct[8] *= form_9_18[8];                                                            \
																										\
				for (i = 0; i < 8; i += 4)                                                              \
				{                                                                                       \
					a.m128 = _mm_loadu_ps(even_idct + i);                                               \
					b.m128 = _mm_loadu_ps(odd_idct + i);                                                \
					r.m128 = _mm_add_ps(a.m128, b.m128);                                                \
					_mm_storeu_ps(h + i, r.m128);                                                       \
				}                                                                                       \
																										\
				h[8] = even_idct[8] + odd_idct[8];                                                      \
				h[9] = even_idct[8] - odd_idct[8];                                                      \
																										\
				for (i = 10; i < 18; i += 4)                                                            \
				{                                                                                       \
					a.m128 = _mm_loadu_ps(even_idct + (17 - i) - 3);                                    \
					b.m128 = _mm_loadu_ps(odd_idct  + (17 - i) - 3);                                    \
					r.m128 = _mm_sub_ps(a.m128, b.m128);                                                \
					h[i]   = r.f[3];                                                                    \
					h[i+1] = r.f[2];                                                                    \
					h[i+2] = r.f[1];                                                                    \
					h[i+3] = r.f[0];                                                                    \
				}                                                                                       \
				                                                                                        \
				h[16] *= form_18_36[16];                                                                \
				h[17] *= form_18_36[17];                                                                \
				for (i = 0; i < 16; i += 4)                                                             \
				{                                                                                       \
					a.m128 = _mm_loadu_ps(h + i);                                                       \
					r.m128 = _mm_mul_ps(a.m128, u_form_18_36.m128[i>>2]);                               \
					_mm_storeu_ps(h + i, r.m128);                                                       \
				}                                                                                       \
				                                                                                        \
				memcpy(out, h + 9, 36);                                                                 \
				                                                                                        \
				out[9]  = -h[17];   out[18] = -h[8];   out[27] = -h[0];                                 \
				out[10] = -h[16];   out[19] = -h[7];   out[28] = -h[1];                                 \
				out[11] = -h[15];   out[20] = -h[6];   out[29] = -h[2];                                 \
				out[12] = -h[14];   out[21] = -h[5];   out[30] = -h[3];                                 \
				out[13] = -h[13];   out[22] = -h[4];   out[31] = -h[4];                                 \
				out[14] = -h[12];   out[23] = -h[3];   out[32] = -h[5];                                 \
				out[15] = -h[11];   out[24] = -h[2];   out[33] = -h[6];                                 \
				out[16] = -h[10];   out[25] = -h[1];   out[34] = -h[7];                                 \
				out[17] = -h[9];    out[26] = -h[0];   out[35] = -h[8];                                 \
			}

	#pragma endregion

	#pragma region <IMDCT_SSE>

			#define _IMDCT_SSE(level, block_type) {                    \
				float * in = sample + level;                           \
				auto & IMDCT_sin_bt = u_IMDCT_sin[block_type];         \
				UNION128 a, b, r;                                      \
																	   \
				ZeroMemory(rawout, sizeof(rawout));                    \
																	   \
				if (block_type == eMPEGBlockT_3ShortWindow)            \
				{                                                      \
					float out[12];                                     \
					float _in[18];                                     \
																	   \
					for (uint16 i = 0; i < 3; i++)                     \
					for (uint16 m = 0; m < 6; m++)                     \
						_in[i * 6 + m] = in[i + 3 * m];                \
																	   \
					for (uint16 i = 0; i < 3; i++)                     \
					{                                                  \
						in = _in + i * 6;                              \
						_IMDCT_Short_SSE(in, out);                     \
						                                               \
						uint16 _6i6 = 6 * i + 6;                       \
						for (uint16 p = 0; p < 12; p += 4)             \
						{                                              \
							a.m128 = _mm_loadu_ps(out + p);            \
							r.m128 = _mm_mul_ps(a.m128, IMDCT_sin_bt.m128[p>>2]); \
							                                           \
							a.m128 = _mm_loadu_ps(rawout + _6i6 + p);  \
							b.m128 = _mm_add_ps(a.m128, r.m128);       \
							_mm_storeu_ps(rawout + _6i6 + p, b.m128);  \
						}                                              \
					}                                                  \
				}                                                      \
				else                                                   \
				{                                                      \
					_IMDCT_Long_SSE(in, rawout);                       \
																	   \
					for (uint16 i=0; i<36; i+=4)                       \
					{                                                  \
						a.m128 = _mm_loadu_ps(rawout + i);             \
						r.m128 = _mm_mul_ps(a.m128, IMDCT_sin_bt.m128[i>>2]); \
						_mm_storeu_ps(rawout + i, r.m128);             \
					}                                                  \
				}                                                      \
			}

	#pragma endregion

				for (uint16 subband = 0; subband < MP3SUBBANDS; subband++)
				{
					byte block_type = MPEGS.block_type[gr][ch];
					uint16 level = subband * MP3LINES;
					float * store = storehs[subband]; // [18]

					if (subband < 2 && MPEGS.switch_flag[gr][ch] && MPEGS.mixed_block_flag[gr][ch])
						 block_type = 0;

					_IMDCT_SSE(level, block_type);

					for (uint16 line = 0; line < 16; line += 4) // 16 = 4x4 (SSE) 8x2 (AVX) 16x1 (AVX256)
					{
						UNION128 a, b, r;

						a.m128 = _mm_loadu_ps(rawout + line);
						b.m128 = _mm_loadu_ps(store  + line);
						r.m128 = _mm_add_ps(a.m128, b.m128);
						_mm_storeu_ps(sample + level + line, r.m128);
					}

					sample[level + 16] = rawout[16] + store[16];
					sample[level + 17] = rawout[17] + store[17];

					memcpy(store, rawout + MP3LINES, MP3LINES * sizeof(float));
				}
			}
			else
#endif
#ifdef ENGINE_AVX
			if (opt_AVX_only) // AVX CODE = ??%..??% time _ NOT TESTED
			{

	#pragma region <IMDCT_Long_AVX>

			#define _IMDCT_Long_AVX(in, out) {                                    \
				int i;                                                            \
				float H[18], h[18], even[9], odd[9], even_idct[9], odd_idct[9];   \
				UNION256 a, b, r;                                                 \
				                                                                  \
				H[0]  = in[0];                                                    \
				H[17] = in[16] + in[17];                                          \
				for (i = 1; i < 17; i += 8)                                       \
				{                                                                 \
					a.m256 = _mm256_loadu_ps(in + i);                             \
					b.m256 = _mm256_loadu_ps(in + i - 1);                         \
					r.m256 = _mm256_add_ps(a.m256, b.m256);                       \
					_mm256_storeu_ps(H + i, r.m256);                              \
				}                                                                 \
				                                                                  \
				for (i = 0; i < 9; i++)                                           \
					even[i] = H[i * 2];                                           \
				                                                                  \
				_IMDCT_9_SSE(even, even_idct);                                    \
				                                                                  \
				odd[0] = H[1];                                                    \
				for (i = 1; i < 9; i++)                                           \
					odd[i] = H[i * 2 - 1] + H[i * 2 + 1];                         \
				                                                                  \
				_IMDCT_9_SSE(odd, odd_idct);                                      \
				                                                                  \
				a.m256 = _mm256_loadu_ps(odd_idct);                               \
				r.m256 = _mm256_mul_ps(a.m256, u_form_9_18.m256[0]);              \
				_mm256_storeu_ps(odd_idct, r.m256);                               \
				                                                                  \
				odd_idct[8] *= form_9_18[8];                                      \
				                                                                  \
				a.m256 = _mm256_loadu_ps(even_idct);                              \
				b.m256 = _mm256_loadu_ps(odd_idct);                               \
				r.m256 = _mm256_add_ps(a.m256, b.m256);                           \
				_mm256_storeu_ps(h, r.m256);                                      \
				                                                                  \
				h[8] = even_idct[8] + odd_idct[8];                                \
				h[9] = even_idct[8] - odd_idct[8];                                \
				                                                                  \
				a.m256 = _mm256_loadu_ps(even_idct);                              \
				b.m256 = _mm256_loadu_ps(odd_idct);                               \
				r.m256 = _mm256_sub_ps(a.m256, b.m256);                           \
				h[10] = r.f[7];                                                   \
				h[11] = r.f[6];                                                   \
				h[12] = r.f[5];                                                   \
				h[13] = r.f[4];                                                   \
				h[14] = r.f[3];                                                   \
				h[15] = r.f[2];                                                   \
				h[16] = r.f[1];                                                   \
				h[17] = r.f[0];                                                   \
				                                                                  \
				h[16] *= form_18_36[16];                                          \
				h[17] *= form_18_36[17];                                          \
				for (i = 0; i < 16; i += 8)                                       \
				{                                                                 \
					a.m256 = _mm256_loadu_ps(h + i);                              \
					r.m256 = _mm256_mul_ps(a.m256, u_form_18_36.m256[i>>3]);      \
					_mm256_storeu_ps(h + i, r.m256);                              \
				}                                                                 \
				                                                                  \
				memcpy(out, h + 9, 36);                                           \
				                                                                  \
				out[9]  = -h[17];   out[18] = -h[8];   out[27] = -h[0];           \
				out[10] = -h[16];   out[19] = -h[7];   out[28] = -h[1];           \
				out[11] = -h[15];   out[20] = -h[6];   out[29] = -h[2];           \
				out[12] = -h[14];   out[21] = -h[5];   out[30] = -h[3];           \
				out[13] = -h[13];   out[22] = -h[4];   out[31] = -h[4];           \
				out[14] = -h[12];   out[23] = -h[3];   out[32] = -h[5];           \
				out[15] = -h[11];   out[24] = -h[2];   out[33] = -h[6];           \
				out[16] = -h[10];   out[25] = -h[1];   out[34] = -h[7];           \
				out[17] = -h[9];    out[26] = -h[0];   out[35] = -h[8];           \
			}

	#pragma endregion

	#pragma region <IMDCT_AVX>

			#define _IMDCT_AVX(level, block_type) {                    \
				float * in = sample + level;                           \
				auto & IMDCT_sin_bt = u_IMDCT_sin[block_type];         \
				UNION256 a, b, r;                                      \
																	   \
				ZeroMemory(rawout, sizeof(rawout));                    \
																	   \
				if (block_type == eMPEGBlockT_3ShortWindow)            \
				{                                                      \
					float out[12];                                     \
					float _in[18];                                     \
																	   \
					for (uint16 i = 0; i < 3; i++)                     \
					for (uint16 m = 0; m < 6; m++)                     \
						_in[i * 6 + m] = in[i + 3 * m];                \
																	   \
					for (uint16 i = 0; i < 3; i++)                     \
					{                                                  \
						in = _in + i * 6;                              \
						_IMDCT_Short_SSE(in, out);                     \
						                                               \
						uint16 _6i6 = 6 * i + 6;                       \
						                                               \
						a.m256 = _mm256_loadu_ps(out);                 \
						r.m256 = _mm256_mul_ps(a.m256, IMDCT_sin_bt.m256[0]); \
							                                           \
						a.m256 = _mm256_loadu_ps(rawout + _6i6);       \
						b.m256 = _mm256_add_ps(a.m256, r.m256);        \
						_mm256_storeu_ps(rawout + _6i6, b.m256);       \
						                                               \
				                                                       \
						a.m128[0] = _mm_loadu_ps(out + 8);             \
						r.m128[0] = _mm_mul_ps(a.m128[0], IMDCT_sin_bt.m128[2]);  \
							                                           \
						a.m128[0] = _mm_loadu_ps(rawout + _6i6 + 8);   \
						b.m128[0] = _mm_add_ps(a.m128[0], r.m128[0]);  \
						_mm_storeu_ps(rawout + _6i6 + 8, b.m128[0]);   \
					}                                                  \
				}                                                      \
				else                                                   \
				{                                                      \
					_IMDCT_Long_AVX(in, rawout);                       \
																	   \
					for (uint16 i=0; i<32; i+=8)                       \
					{                                                  \
						a.m256 = _mm256_loadu_ps(rawout + i);          \
						r.m256 = _mm256_mul_ps(a.m256, IMDCT_sin_bt.m256[i>>3]); \
						_mm256_storeu_ps(rawout + i, r.m256);          \
					}                                                  \
					                                                   \
					a.m128[0] = _mm_loadu_ps(rawout + 32);             \
					r.m128[0] = _mm_mul_ps(a.m128[0], IMDCT_sin_bt.m128[8]);     \
					_mm_storeu_ps(rawout + 32, r.m128[0]);             \
				}                                                      \
			}

	#pragma endregion

				#pragma message ("ConvertMP3_HybridSynth : 256-bit AVX code not tested")

				for (uint16 subband = 0; subband < MP3SUBBANDS; subband++)
				{
					byte block_type = MPEGS.block_type[gr][ch];
					uint16 level = subband * MP3LINES;
					float * store = storehs[subband]; // [18]

					if (subband < 2 && MPEGS.switch_flag[gr][ch] && MPEGS.mixed_block_flag[gr][ch])
						 block_type = 0;

					_IMDCT_AVX(level, block_type);

					for (uint16 line = 0; line < 16; line += 8) // 16 = 4x4 (SSE) 8x2 (AVX) 16x1 (AVX256)
					{
						UNION256 a, b, r;

						a.m256 = _mm256_loadu_ps(rawout + line);
						b.m256 = _mm256_loadu_ps(store  + line);
						r.m256 = _mm256_add_ps(a.m256, b.m256);
						_mm256_storeu_ps(sample + level + line, r.m256);
					}

					sample[level + 16] = rawout[16] + store[16];
					sample[level + 17] = rawout[17] + store[17];

					memcpy(store, rawout + MP3LINES, MP3LINES * sizeof(float));
				}
			}
			else
#endif
#ifdef ENGINE_AVX512F

	#pragma region <IMDCT_Long_AVX512F>

			#define _IMDCT_Long_AVX512F(in, out) {                                \
				int i;                                                            \
				float H[18], h[18], even[9], odd[9], even_idct[9], odd_idct[9];   \
				UNION512 a, b, r;                                                 \
				                                                                  \
				H[0]  = in[0];                                                    \
				H[17] = in[16] + in[17];                                          \
				                                                                  \
				a.m512 = _mm512_loadu_ps(in + 1);                                 \
				b.m512 = _mm512_loadu_ps(in);                                     \
				r.m512 = _mm512_add_ps(a.m512, b.m512);                           \
				_mm512_storeu_ps(H + 1, r.m512);                                  \
				                                                                  \
				for (i = 0; i < 9; i++)                                           \
					even[i] = H[i * 2];                                           \
				                                                                  \
				_IMDCT_9_SSE(even, even_idct);                                    \
				                                                                  \
				odd[0] = H[1];                                                    \
				for (i = 1; i < 9; i++)                                           \
					odd[i] = H[i * 2 - 1] + H[i * 2 + 1];                         \
				                                                                  \
				_IMDCT_9_SSE(odd, odd_idct);                                      \
				                                                                  \
				a.m256[0] = _mm256_loadu_ps(odd_idct);                            \
				r.m256[0] = _mm256_mul_ps(a.m256[0], u_form_9_18.m256[0]);        \
				_mm256_storeu_ps(odd_idct, r.m256[0]);                            \
				                                                                  \
				odd_idct[8] *= form_9_18[8];                                      \
				                                                                  \
				a.m256[0] = _mm256_loadu_ps(even_idct);                           \
				b.m256[0] = _mm256_loadu_ps(odd_idct);                            \
				r.m256[0] = _mm256_add_ps(a.m256[0], b.m256[0]);                  \
				_mm256_storeu_ps(h, r.m256[0]);                                   \
				                                                                  \
				h[8] = even_idct[8] + odd_idct[8];                                \
				h[9] = even_idct[8] - odd_idct[8];                                \
				                                                                  \
				a.m256[0] = _mm256_loadu_ps(even_idct);                           \
				b.m256[0] = _mm256_loadu_ps(odd_idct);                            \
				r.m256[0] = _mm256_sub_ps(a.m256[0], b.m256[0]);                  \
				h[10] = r.f[7];                                                   \
				h[11] = r.f[6];                                                   \
				h[12] = r.f[5];                                                   \
				h[13] = r.f[4];                                                   \
				h[14] = r.f[3];                                                   \
				h[15] = r.f[2];                                                   \
				h[16] = r.f[1];                                                   \
				h[17] = r.f[0];                                                   \
				                                                                  \
				h[16] *= form_18_36[16];                                          \
				h[17] *= form_18_36[17];                                          \
				                                                                  \
				a.m512 = _mm512_loadu_ps(h);                                      \
				r.m512 = _mm512_mul_ps(a.m512, u_form_18_36.m512[0]);             \
				_mm512_storeu_ps(h, r.m512);                                      \
				                                                                  \
				memcpy(out, h + 9, 36);                                           \
				                                                                  \
				out[9]  = -h[17];   out[18] = -h[8];   out[27] = -h[0];           \
				out[10] = -h[16];   out[19] = -h[7];   out[28] = -h[1];           \
				out[11] = -h[15];   out[20] = -h[6];   out[29] = -h[2];           \
				out[12] = -h[14];   out[21] = -h[5];   out[30] = -h[3];           \
				out[13] = -h[13];   out[22] = -h[4];   out[31] = -h[4];           \
				out[14] = -h[12];   out[23] = -h[3];   out[32] = -h[5];           \
				out[15] = -h[11];   out[24] = -h[2];   out[33] = -h[6];           \
				out[16] = -h[10];   out[25] = -h[1];   out[34] = -h[7];           \
				out[17] = -h[9];    out[26] = -h[0];   out[35] = -h[8];           \
			}

	#pragma endregion

	#pragma region <IMDCT_AVX512F>

			#define _IMDCT_AVX512F(level, block_type) {                \
				float * in = sample + level;                           \
				auto & IMDCT_sin_bt = u_IMDCT_sin[block_type];         \
				UNION512 a, b, r;                                      \
																	   \
				ZeroMemory(rawout, sizeof(rawout));                    \
																	   \
				if (block_type == eMPEGBlockT_3ShortWindow)            \
				{                                                      \
					float out[12];                                     \
					float _in[18];                                     \
																	   \
					for (uint16 i = 0; i < 3; i++)                     \
					for (uint16 m = 0; m < 6; m++)                     \
						_in[i * 6 + m] = in[i + 3 * m];                \
																	   \
					for (uint16 i = 0; i < 3; i++)                     \
					{                                                  \
						in = _in + i * 6;                              \
						_IMDCT_Short_SSE(in, out);                     \
						                                               \
						uint16 _6i6 = 6 * i + 6;                       \
						                                               \
						a.m256[0] = _mm256_loadu_ps(out);              \
						r.m256[0] = _mm256_mul_ps(a.m256[0], IMDCT_sin_bt.m256[0]); \
							                                           \
						a.m256[0] = _mm256_loadu_ps(rawout + _6i6);    \
						b.m256[0] = _mm256_add_ps(a.m256[0], r.m256[0]); \
						_mm256_storeu_ps(rawout + _6i6, b.m256[0]);    \
						                                               \
				                                                       \
						a.m128[0] = _mm_loadu_ps(out + 8);             \
						r.m128[0] = _mm_mul_ps(a.m128[0], IMDCT_sin_bt.m128[2]);  \
							                                           \
						a.m128[0] = _mm_loadu_ps(rawout + _6i6 + 8);   \
						b.m128[0] = _mm_add_ps(a.m128[0], r.m128[0]);  \
						_mm_storeu_ps(rawout + _6i6 + 8, b.m128[0]);   \
					}                                                  \
				}                                                      \
				else                                                   \
				{                                                      \
					_IMDCT_Long_AVX512F(in, rawout);                   \
																	   \
					for (uint16 i=0; i<32; i+=16)                      \
					{                                                  \
						a.m512 = _mm512_loadu_ps(rawout + i);          \
						r.m512 = _mm512_mul_ps(a.m512, IMDCT_sin_bt.m512[i>>4]); \
						_mm512_storeu_ps(rawout + i, r.m512);          \
					}                                                  \
					                                                   \
					a.m128[0] = _mm_loadu_ps(rawout + 32);             \
					r.m128[0] = _mm_mul_ps(a.m128[0], IMDCT_sin_bt.m128[8]);     \
					_mm_storeu_ps(rawout + 32, r.m128[0]);             \
				}                                                      \
			}

	#pragma endregion

			if (opt_AVX512F_only) // AVX512F CODE = ??%..??% time _ NOT TESTED
			{
				#pragma message ("ConvertMP3_HybridSynth : 512-bit AVX512F code not tested")

				for (uint16 subband = 0; subband < MP3SUBBANDS; subband++)
				{
					byte block_type = MPEGS.block_type[gr][ch];
					uint16 level = subband * MP3LINES;
					float * store = storehs[subband]; // [18]

					if (subband < 2 && MPEGS.switch_flag[gr][ch] && MPEGS.mixed_block_flag[gr][ch])
						 block_type = 0;

					_IMDCT_AVX512F(level, block_type);

					//for (uint16 line = 0; line < 16; line += 16) // 16 = 4x4 (SSE) 8x2 (AVX) 16x1 (AVX256)
					{
						UNION512 a, b, r;

						a.m512 = _mm512_loadu_ps(rawout); // + line );
						b.m512 = _mm512_loadu_ps(store);  // + line );
						r.m512 = _mm512_add_ps(a.m512, b.m512);
						_mm512_storeu_ps(sample + level, // + line,
							r.m512);
					}

					sample[level + 16] = rawout[16] + store[16];
					sample[level + 17] = rawout[17] + store[17];

					memcpy(store, rawout + MP3LINES, MP3LINES * sizeof(float));
				}
			}
			else
#endif		// SIMPLE CODE = 100% time
			{
				for (uint16 subband = 0; subband < MP3SUBBANDS; subband++)
				{
					byte block_type = MPEGS.block_type[gr][ch];
					uint16 level = subband * MP3LINES;
					float * store = storehs[subband]; // [18]

					if (subband < 2 && MPEGS.switch_flag[gr][ch] && MPEGS.mixed_block_flag[gr][ch])
						 block_type = 0;

					_IMDCT(level, block_type);

					for (uint16 line = 0; line < MP3LINES; line++)
					{
						sample[level + line] = rawout[line] + store[line];
						//store[line] = rawout[line + MP3LINES]; // using memcpy instead
					}

					memcpy(store, rawout + MP3LINES, MP3LINES * sizeof(float));
				}
			}

			#ifdef ENGINE_AVX512F
				#undef _IMDCT_Long_AVX512F
				#undef _IMDCT_AVX512F
			#endif

			#ifdef ENGINE_AVX
				#undef _IMDCT_Long_AVX
				#undef _IMDCT_AVX
			#endif

			#ifdef ENGINE_SSE
				#undef _IMDCT_Short_SSE
				#undef _IMDCT_9_SSE
				#undef _IMDCT_Long_SSE
				#undef _IMDCT_SSE
			#endif

			#undef _IMDCT_3
			#undef _IMDCT_Short
			#undef _IMDCT_4
			#undef _IMDCT_5
			#undef _IMDCT_9
			#undef _IMDCT_Long
			#undef _IMDCT
		}

		//>> 3.3. Инверсия семплов
		void ConvertMP3_FreqInversion(byte gr, byte ch)
		{
			float * sample = MPEGD.sample[gr][ch];

#ifdef ENGINE_AVX512F
			if (opt_AVX512F) // AVX512F CODE = ??%..??% time _ NOT TESTED
			{
				#pragma message ("ConvertMP3_FreqInversion : 512-bit AVX512F code not tested")

				UNION512 data, out;
				uint16 * idx = freqInvIdx;

				for (int i=0; i<9; i++) // 144 = 4 x36 (SSE) or 8 x18 (AVX) or 16 x9 (AVX-512F)
				{
					data.m512 = _mm512_set_ps (
						sample[idx[15]], sample[idx[14]], sample[idx[13]], sample[idx[12]],
						sample[idx[11]], sample[idx[10]], sample[idx[9]],  sample[idx[8]],
						sample[idx[7]],  sample[idx[6]],  sample[idx[5]],  sample[idx[4]],
						sample[idx[3]],  sample[idx[2]],  sample[idx[1]],  sample[idx[0]]  );

					out.m512 = _mm512_mul_ps(data.m512, freqInvMinus512.m512);

					sample[idx[15]] = out.f[15];
					sample[idx[14]] = out.f[14];
					sample[idx[13]] = out.f[13];
					sample[idx[12]] = out.f[12];
					sample[idx[11]] = out.f[11];
					sample[idx[10]] = out.f[10];
					sample[idx[9]]  = out.f[9];
					sample[idx[8]]  = out.f[8];
					sample[idx[7]]  = out.f[7];
					sample[idx[6]]  = out.f[6];
					sample[idx[5]]  = out.f[5];
					sample[idx[4]]  = out.f[4];
					sample[idx[3]]  = out.f[3];
					sample[idx[2]]  = out.f[2];
					sample[idx[1]]  = out.f[1];
					sample[idx[0]]  = out.f[0];

					idx += 16;
				}
			}
			else
#endif
#ifdef ENGINE_AVX
			if (opt_AVX) // AVX CODE = ??%..??% time _ NOT TESTED
			{
				#pragma message ("ConvertMP3_FreqInversion : 256-bit AVX code not tested")

				UNION256 data, out;
				uint16 * idx = freqInvIdx;

				for (int i=0; i<18; i++) // 144 = 4 x36 (SSE) or 8 x18 (AVX) or 16 x9 (AVX-512F)
				{
					data.m256 = _mm256_set_ps (
						sample[idx[7]], sample[idx[6]], sample[idx[5]], sample[idx[4]],
						sample[idx[3]], sample[idx[2]], sample[idx[1]], sample[idx[0]]  );

					out.m256 = _mm256_mul_ps(data.m256, freqInvMinus256.m256);

					sample[idx[7]] = out.f[7];
					sample[idx[6]] = out.f[6];
					sample[idx[5]] = out.f[5];
					sample[idx[4]] = out.f[4];
					sample[idx[3]] = out.f[3];
					sample[idx[2]] = out.f[2];
					sample[idx[1]] = out.f[1];
					sample[idx[0]] = out.f[0];

					idx += 8;
				}
			}
			else
#endif
#ifdef ENGINE_SSE
			if (opt_SSE) // SSE CODE = 80%..90% time
			{
				// r.m128 = _mm_set_ps(a_store[3], a_store[2], a_store[1], a_store[0]);
				// r.m128 = _mm_loadu_ps(a_store);
				// r.m128 = _mm_mul_ps(a.m128, b.m128);
				// _mm_storeu_ps(a_store, r.m128);

				UNION128 data, out;
				uint16 * idx = freqInvIdx;

				for (int i=0; i<36; i++) // 144 = 4 x36 (SSE) or 8 x18 (AVX) or 16 x9 (AVX-512F)
				{
					data.m128 = _mm_set_ps (
						sample[idx[3]],
						sample[idx[2]],
						sample[idx[1]],
						sample[idx[0]]     );

					out.m128 = _mm_mul_ps(data.m128, freqInvMinus128.m128);

					sample[idx[3]] = out.f[3];
					sample[idx[2]] = out.f[2];
					sample[idx[1]] = out.f[1];
					sample[idx[0]] = out.f[0];

					idx += 4;
				}
			}
			else
#endif		// SIMPLE CODE = 100% time
			{
				//for (uint16 subband = 1; subband < MP3SUBBANDS; subband += 2)
				//{
				//	uint16 level = subband * MP3LINES;
				//	for (uint16 line = 1; line < MP3LINES; line += 2)
				//	{
				//		uint16 level_line = level + line;
				//		sample[level_line] = -sample[level_line];
				//	}
				//}

				for (int i = 0; i < MP3FREQINV; i++)
				{
					auto & s = sample[ freqInvIdx[i] ];
					s = -s;
				}
			}
		}

		//>> 3.4. Синтезирование полифазного набора фильтров
		void ConvertMP3_SubbandSynth_simple(byte gr, byte ch, bool mono, uint32 result_granule[MP3GRS])
		{	
			float synthesis [MP3SYNTHESIS];

			float * storess = this->storess[ch];    // store ss [1024] = [16][64]
			float * sample  = MPEGD.sample[gr][ch]; // sample [576]

			for (uint16 line = 0; line < MP3LINES; line++)
			{
				for (uint16 i = MP3SYNTHESIS * 2 - 1; i > MP3SUBBANDS * 2 - 1; i--)
					storess[i] = storess[i - MP3SUBBANDS * 2];

				for (uint16 subband = 0; subband < MP3SUBBANDS; subband++)
					synthesis[subband] = sample[subband * MP3LINES + line];

				for (uint16 i = 0; i < MP3SUBBANDS * 2; i++)
				{
					float sum = 0;
					for (uint16 j = 0; j < MP3SUBBANDS; j++)
						sum += synthesis_tab[i][j] * synthesis[j];
					storess[i] = sum;
				}

				for (uint16 i = 0; i < 8;  i++)
				for (uint16 j = 0; j < 32; j++)
				{
					//         i * 64                       i * 128
					synthesis[(i << 6) + j]      = storess[(i << 7) + j];
					synthesis[(i << 6) + j + 32] = storess[(i << 7) + j + 96];
				}

				for (uint16 i = 0; i < MP3SYNTHESIS; i++)
					synthesis[i] *= subband_synthesis_table[i];

				/* Calculate 32 samples and store them in the outdata vector */
				for (uint16 subband = 0; subband < MP3SUBBANDS; subband++)
				{
					float sum = 0;
					for (uint16 j = 0; j < 16; j++)
					{
						//                j * 32
						sum += synthesis[(j << 5) + subband];
					}

					int32 samp = (int32)(sum * MASK_INT16_MAX);

						 if (samp >  MASK_INT16_MAX) samp =  MASK_INT16_MAX;
					else if (samp < -MASK_INT16_MAX) samp = -MASK_INT16_MAX;

					samp &= MASK_INT16;

					if (!ch)
					{
						if (mono) result_granule [32 * line + subband]  = (samp << 16) | samp;
						else      result_granule [32 * line + subband]  =  samp;
					} else        result_granule [32 * line + subband] |=  samp << 16;
				}

			}
		}

		//>> 3.4. Синтезирование полифазного набора фильтров - fast ver.
		void ConvertMP3_SubbandSynth(byte gr, byte ch, bool mono, uint32 result_granule[MP3GRS], uint32 skip_samples)
		{
			float synthesis [MP3SYNTHESIS];

			auto  & storess_pos = this->storess_pos[ch];  // 0..15
			float * storess     = this->storess[ch];      // store sub synth [1024] = [16][64]
			float * sample      = MPEGD.sample[gr][ch];   // sample [576]

			auto DCT2  = []      (float* in, float* out) -> void
			{
				int i, j;

				for (i = 0; i < 2; i++)
				{
					out[i] = 0;
					for (j = 0; j < 2; j++)
					{
						out[i] += in[j] * cosij[i][j];
					}
				}
			};
			auto DCT4  = [DCT2]  (float* in, float* out) -> void
			{
				const int N  = 4;
				const int N2 = N >> 1;

				int i;
				float even_in[N2], even_out[N2], odd_in[N2], odd_out[N2];

				for (i = 0; i < N2; i++)
					even_in[i] = in[i] + in[N - 1 - i];

				DCT2(even_in, even_out);

				for (i = 0; i < N2; i++)
					odd_in[i] = (in[i] - in[N - 1 - i]) * form_2_4[i];

				DCT2(odd_in, odd_out);

				for (i = 0; i < N2; i++)
					out[2 * i] = even_out[i];

				for (i = 0; i < N2 - 1; i++)
					out[2 * i + 1] = odd_out[i] + odd_out[i + 1];

				out[N - 1] = odd_out[N2 - 1];
			};
			auto DCT8  = [DCT4]  (float* in, float* out) -> void
			{
				const int N = 8;
				const int N2 = N >> 1;

				int i;
				float even_in[N2], even_out[N2], odd_in[N2], odd_out[N2];

				for (i = 0; i < N2; i++)
					even_in[i] = in[i] + in[N - 1 - i];

				DCT4(even_in, even_out);

				for (i = 0; i < N2; i++)
					odd_in[i] = (in[i] - in[N - 1 - i]) * form_4_8[i];

				DCT4(odd_in, odd_out);

				for (i = 0; i < N2; i++)
					out[2 * i] = even_out[i];

				for (i = 0; i < N2 - 1; i++)
					out[2 * i + 1] = odd_out[i] + odd_out[i + 1];

				out[N - 1] = odd_out[N2 - 1];
			};
			auto DCT16 = [DCT8]  (float* in, float* out) -> void
			{
				const int N = 16;
				const int N2 = N >> 1;

				int i;
				float even_in[N2], even_out[N2], odd_in[N2], odd_out[N2];

				for (i = 0; i < N2; i++)
					even_in[i] = in[i] + in[N - 1 - i];

				DCT8(even_in, even_out);

				for (i = 0; i < N2; i++)
					odd_in[i] = (in[i] - in[N - 1 - i]) * form_8_16[i];

				DCT8(odd_in, odd_out);

				for (i = 0; i < N2; i++)
					out[2 * i] = even_out[i];

				for (i = 0; i < N2 - 1; i++)
					out[2 * i + 1] = odd_out[i] + odd_out[i + 1];

				out[N - 1] = odd_out[N2 - 1];
			};
			auto DCT32 = [DCT16] (float* in, float* out) -> void
			{
				const int N  = 32;
				const int N2 = N >> 1;	

				int i;
				float even_in[N2], even_out[N2], odd_in[N2], odd_out[N2];

				for (i = 0; i < N2; i++)
					even_in[i] = in[i] + in[N - 1 - i];

				DCT16(even_in, even_out);

				for (i = 0; i < N2; i++)
					odd_in[i] = (in[i] - in[N - 1 - i]) * form_16_32[i];

				DCT16(odd_in, odd_out);

				for (i = 0; i < N2; i++)
					out[2 * i] = even_out[i];

				for (i = 0; i < N2 - 1; i++)
					out[2 * i + 1] = odd_out[i] + odd_out[i + 1];

				out[N - 1] = odd_out[N2 - 1];
			};

			auto DCT_2pt = [] (float in[2], float out[2]) -> void
			{
				int i, j;
			
				for (i = 0; i < 2; i++)
				{
					out[i] = 0;
					for (j = 0; j < 2; j++)
					{
						out[i] += in[j] * cosij[i][j];
					}
				}
			};
			std::function<void(float*,float*,int)> DCT = [&, DCT_2pt] (float* in, float* out, int N)
			{
				// N = [32, 16, 8, 4, 2]
				//      4   2   1  0

				int i, N8 = N >> 3, N2 = N >> 1;
				float even_in[16], even_out[16], odd_in[16], odd_out[16];

				static const float * form[5] = { form_2_4, form_4_8, form_8_16, 0, form_16_32 };

				if (N == 2)
				{
					DCT_2pt(in, out);
					return;
				}

				for (i = 0; i < N2; i++)
					even_in[i] = in[i] + in[N - 1 - i]; 

				DCT(even_in, even_out, N2);

			//	for (i = 0; i < N_half; i++)
			//		odd_in[i] = (in[i] - in[N - 1 - i]) * POST_TWIDDLE(i, N);

				for (i = 0; i < N2; i++)
					odd_in[i] = (in[i] - in[N - 1 - i]) * form[N8][i];

				DCT(odd_in, odd_out, N2);

				for (i = 0; i < N2; i++)
					out[2 * i] = even_out[i];

				for (i = 0; i < N2 - 1; i++)
					out[2 * i + 1] = odd_out[i] + odd_out[i + 1];

				out[N - 1] = odd_out[N2 - 1];
			};
			auto PolyphaseMatrixing = [DCT, DCT32](float in[32], float out[64]) -> void
			{
				int i;
				float tmp[32];

				//DCT(in, tmp, 32);
				DCT32(in, tmp);

				out[16] = 0;

				for (i =  0; i < 16; i++) out[i] =  tmp[i + 16];
				for (i = 17; i < 32; i++) out[i] = -tmp[48 - i];
				for (i = 32; i < 48; i++) out[i] = -tmp[48 - i];
				for (i = 48; i < 64; i++) out[i] = -tmp[i - 48];
			};

	#pragma region <DCT2>

			#define _DCT2(in, out) {                                    \
				int i, j;												\
																		\
				for (i = 0; i < 2; i++)									\
				{														\
					out[i] = 0;											\
					for (j = 0; j < 2; j++)								\
					{													\
						out[i] += in[j] * cosij[i][j];					\
					}													\
				}														\
			}

	#pragma endregion

	#pragma region <DCT4>

			#define _DCT4(in, out) {                                    \
				int i;                                                  \
				float even_in2[2], even_out2[2];                        \
				float  odd_in2[2],  odd_out2[2];                        \
																		\
				for (i = 0; i < 2; i++)                                 \
					even_in2[i] = in[i] + in[3 - i];                    \
																		\
				_DCT2(even_in2, even_out2);                             \
																		\
				for (i = 0; i < 2; i++)                                 \
					odd_in2[i] = (in[i] - in[3 - i]) * form_2_4[i];     \
																		\
				_DCT2(odd_in2, odd_out2);                               \
																		\
				for (i = 0; i < 2; i++)                                 \
					out[2 * i] = even_out2[i];                          \
																		\
				for (i = 0; i < 1; i++)                                 \
					out[2 * i + 1] = odd_out2[i] + odd_out2[i + 1];     \
																		\
				out[3] = odd_out2[1];                                   \
			}

	#pragma endregion

	#pragma region <DCT8>

			#define _DCT8(in, out) {                                    \
				int i;                                                  \
				float even_in4[4], even_out4[4];                        \
				float  odd_in4[4],  odd_out4[4];                        \
																		\
				for (i = 0; i < 4; i++)                                 \
					even_in4[i] = in[i] + in[7 - i];                    \
																		\
				_DCT4(even_in4, even_out4);                             \
																		\
				for (i = 0; i < 4; i++)                                 \
					odd_in4[i] = (in[i] - in[7 - i]) * form_4_8[i];     \
																		\
				_DCT4(odd_in4, odd_out4);                               \
																		\
				for (i = 0; i < 4; i++)                                 \
					out[2 * i] = even_out4[i];                          \
																		\
				for (i = 0; i < 3; i++)                                 \
					out[2 * i + 1] = odd_out4[i] + odd_out4[i + 1];     \
																		\
				out[7] = odd_out4[3];                                   \
			}

	#pragma endregion

	#pragma region <DCT16>

			#define _DCT16(in, out) {                                   \
				int i;                                                  \
				float even_in8[8], even_out8[8];                        \
				float  odd_in8[8],  odd_out8[8];                        \
																		\
				for (i = 0; i < 8; i++)                                 \
					even_in8[i] = in[i] + in[15 - i];                   \
																		\
				DCT8(even_in8, even_out8);                              \
																		\
				for (i = 0; i < 8; i++)                                 \
					odd_in8[i] = (in[i] - in[15 - i]) * form_8_16[i];   \
																		\
				DCT8(odd_in8, odd_out8);                                \
																		\
				for (i = 0; i < 8; i++)                                 \
					out[2 * i] = even_out8[i];                          \
																		\
				for (i = 0; i < 7; i++)                                 \
					out[2 * i + 1] = odd_out8[i] + odd_out8[i + 1];     \
																		\
				out[15] = odd_out8[7];                                  \
			}

	#pragma endregion

	#pragma region <DCT32>

			#define _DCT32(in, out) {                                   \
				int i;                                                  \
				float even_in[16], even_out[16];                        \
				float  odd_in[16],  odd_out[16];                        \
																		\
				for (i = 0; i < 16; i++)                                \
					even_in[i] = in[i] + in[32 - 1 - i];                \
																		\
				_DCT16(even_in, even_out);                              \
																		\
				for (i = 0; i < 16; i++)                                \
					odd_in[i] = (in[i] - in[31 - i]) * form_16_32[i];   \
																		\
				_DCT16(odd_in, odd_out);                                \
																		\
				for (i = 0; i < 16; i++)                                \
					out[2 * i] = even_out[i];                           \
																		\
				for (i = 0; i < 15; i++)                                \
					out[2 * i + 1] = odd_out[i] + odd_out[i + 1];       \
																		\
				out[31] = odd_out[15];                                  \
			}

	#pragma endregion

	#pragma region <PolyphaseMatrixing>

			#define _PolyphaseMatrixing(in, out) {					\
				int i;												\
				float tmp[32];										\
																	\
				_DCT32(in, tmp);									\
																	\
				out[16] = 0;										\
																	\
				memcpy(out, tmp + 16, 16 * sizeof(float));			\
				for (i = 17; i < 32; i++) out[i] = -tmp[48 - i];	\
				for (i = 32; i < 48; i++) out[i] = -tmp[48 - i];	\
				for (i = 48; i < 64; i++) out[i] = -tmp[i - 48];	\
			}

	#pragma endregion

	#pragma region <CalcSamples>

	#define _CalcSamples {                                                        \
		                                                                          \
		uint32   shift_line = MP3SUBBANDS * line;                                 \
		uint32 * result_gr  = result_granule + shift_line - skip_samples;         \
		                                                                          \
		for (uint16 subband = 0; subband < MP3SUBBANDS; subband++)                \
		{                                                                         \
			UFLOAT out = 0;                                                       \
			                                                                      \
			for (uint16 j = 0; j < 16; j++)                                       \
				out.f32 += synthesis[(j << 5) + subband];                         \
				                                                                  \
			out.i32 = (int32)(out.f32 * MASK_INT16_MAX);                          \
			                                                                      \
				 if (out.i32 >  MASK_INT16_MAX) out.i32  =  MASK_INT16_MAX;       \
			else if (out.i32 < -MASK_INT16_MAX) out.i32  = -MASK_INT16_MAX;       \
			                                                                      \
			out.i32 &= MASK_INT16;                                                \
			                                                                      \
			if (shift_line + subband >= skip_samples)                             \
			if (!ch)                                                              \
			{                                                                     \
				if (mono) result_gr [subband]  = (out.i32 << 16) | out.i32;       \
				else      result_gr [subband]  =  out.i32;                        \
			} else        result_gr [subband] |=  out.i32 << 16;                  \
		}                                                                         \
	}

	#pragma endregion

#ifdef ENGINE_SSE

	#pragma region <DCT8_SSE>

			#define _DCT8_SSE(in, out) {                                \
				int i;                                                  \
				float even_in4[4], even_out4[4];                        \
				float  odd_in4[4],  odd_out4[4];                        \
				UNION128 a, b, r;                                       \
																		\
				a.m128 = _mm_loadu_ps(in);                              \
				b.m128 = _mm_set_ps ( in[4], in[5], in[6], in[7] );     \
				                                                        \
				r.m128 = _mm_add_ps(a.m128, b.m128);                    \
				_mm_storeu_ps(even_in4, r.m128);                        \
				                                                        \
				r.m128 = _mm_sub_ps(a.m128, b.m128);                    \
				a.m128 = _mm_mul_ps(r.m128, u_form_4_8.m128);           \
				_mm_storeu_ps(odd_in4, a.m128);                         \
																		\
				_DCT4(even_in4, even_out4);                             \
				_DCT4(odd_in4, odd_out4);                               \
																		\
				for (i = 0; i < 4; i++)                                 \
					out[2 * i] = even_out4[i];                          \
																		\
				for (i = 0; i < 3; i++)                                 \
					out[2 * i + 1] = odd_out4[i] + odd_out4[i + 1];     \
																		\
				out[7] = odd_out4[3];                                   \
			}

	#pragma endregion

	#pragma region <DCT16_SSE>

			#define _DCT16_SSE(in, out) {                               \
				int i;                                                  \
				float even_in8[8], even_out8[8];                        \
				float  odd_in8[8],  odd_out8[8];                        \
				UNION128 a, b, r;                                       \
																		\
				for (i = 0; i < 8; i += 4)                              \
				{                                                       \
					a.m128 = _mm_loadu_ps(in + i);                      \
					b.m128 = _mm_set_ps (                               \
						in[12-i], in[13-i], in[14-i], in[15-i] );       \
						                                                \
					r.m128 = _mm_add_ps(a.m128, b.m128);                \
					_mm_storeu_ps(even_in8 + i, r.m128);                \
					                                                    \
					r.m128 = _mm_sub_ps(a.m128, b.m128);                \
					a.m128 = _mm_mul_ps(r.m128, u_form_8_16.m128[i>>2]); \
					_mm_storeu_ps(odd_in8 + i, a.m128);                 \
				}                                                       \
																		\
				_DCT8_SSE(even_in8, even_out8);                         \
				_DCT8_SSE(odd_in8, odd_out8);                           \
				                                                        \
				for (i = 0; i < 8; i++)                                 \
					out[2 * i] = even_out8[i];                          \
				                                                        \
				a.m128 = _mm_loadu_ps(odd_out8);                        \
				b.m128 = _mm_loadu_ps(odd_out8 + 1);                    \
				r.m128 = _mm_add_ps(a.m128, b.m128);                    \
				out[1] = r.f[0];                                        \
				out[3] = r.f[1];                                        \
				out[5] = r.f[2];                                        \
				out[7] = r.f[3];                                        \
				                                                        \
				for (i = 4; i < 7; i++)                                 \
					out[2 * i + 1] = odd_out8[i] + odd_out8[i + 1];     \
																		\
				out[15] = odd_out8[7];                                  \
			}

	#pragma endregion

	#pragma region <DCT32_SSE>

			#define _DCT32_SSE(in, out) {                               \
				int i;                                                  \
				float even_in[16], even_out[16];                        \
				float  odd_in[16],  odd_out[16];                        \
				UNION128 a, b, r;                                       \
																		\
				for (i = 0; i < 16; i += 4)                             \
				{                                                       \
					a.m128 = _mm_loadu_ps(in + i);                      \
					b.m128 = _mm_set_ps (                               \
						in[28-i], in[29-i], in[30-i], in[31-i] );       \
						                                                \
					r.m128 = _mm_add_ps(a.m128, b.m128);                \
					_mm_storeu_ps(even_in + i, r.m128);                 \
					                                                    \
					r.m128 = _mm_sub_ps(a.m128, b.m128);                \
					a.m128 = _mm_mul_ps(r.m128, u_form_16_32.m128[i>>2]); \
					_mm_storeu_ps(odd_in + i, a.m128);                  \
				}                                                       \
																		\
				_DCT16_SSE(odd_in, odd_out);                            \
				_DCT16_SSE(even_in, even_out);                          \
																		\
				for (i = 0; i < 16; i++)                                \
					out[2 * i] = even_out[i];                           \
																		\
				for (i = 0; i < 12; i += 4)                             \
				{                                                       \
					a.m128 = _mm_loadu_ps(odd_out + i);                 \
					b.m128 = _mm_loadu_ps(odd_out + i + 1);             \
					r.m128 = _mm_add_ps(a.m128, b.m128);                \
					out[2 * i       + 1] = r.f[0];                      \
					out[2 * (i + 1) + 1] = r.f[1];                      \
					out[2 * (i + 2) + 1] = r.f[2];                      \
					out[2 * (i + 3) + 1] = r.f[3];                      \
				}                                                       \
			                                                            \
				for (i = 12; i < 15; i++)                               \
					out[2 * i + 1] = odd_out[i] + odd_out[i + 1];       \
																		\
				out[31] = odd_out[15];                                  \
			}

	#pragma endregion

	#pragma region <PolyphaseMatrixing_SSE>

			#define _PolyphaseMatrixing_SSE(in, out) {				\
				int i;												\
				float tmp[32];										\
																	\
				_DCT32_SSE(in, tmp);								\
																	\
				out[16] = 0;										\
																	\
				memcpy(out, tmp + 16, 16 * sizeof(float));			\
				for (i = 17; i < 32; i++) out[i] = -tmp[48 - i];	\
				for (i = 32; i < 48; i++) out[i] = -tmp[48 - i];	\
				for (i = 48; i < 64; i++) out[i] = -tmp[i - 48];	\
			}

	#pragma endregion

			if (opt_SSE_only) // SSE CODE = 79%..83% time
			{
				for (uint16 line = 0; line < MP3LINES; line++)
				{
					--storess_pos &= 0xF; // [0..15]

					for (uint16 subband = 0; subband < MP3SUBBANDS; subband++)
						synthesis[subband] = sample[subband * MP3LINES + line];

					{
						float * store64 = storess + (storess_pos << 6); // + (storess_pos * 64)
						_PolyphaseMatrixing_SSE(synthesis, store64);
					}

					for (uint16 i = 0; i < 8; i++) // [16][64] = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 } (1024)
					{
						float * synthesis_1 = synthesis + (i << 6);       // i * 64
						float * synthesis_2 = synthesis + (i << 6) + 32;  // i * 64 + 32
					//	float * storess_1 = storess + (i << 7);           // i * 128         0, 128, 256, 384, 512, 640, 768, 896, (1024) 
					//	float * storess_2 = storess + (i << 7) + 96;      // i * 128 + 96   96, 224, 352, 480, 608, 736, 864, 992
						float * storess_1 = storess + (((storess_pos << 6) + (i << 7)     ) % 1024); // storess[1024]
						float * storess_2 = storess + (((storess_pos << 6) + (i << 7) + 96) % 1024); // storess[1024]

						auto table_1 = subband_synthesis_table + (i << 6);
						auto table_2 = subband_synthesis_table + (i << 6) + 32;

						for (uint16 j = 0; j < MP3SUBBANDS; j += 4) // 32 = 4 x8 (SSE), 8 x4 (AVX), 16 x2 (AVX512F)
						{
							UNION128 a, b, r;

							a.m128 = _mm_loadu_ps(storess_1 + j);	// synthesis_1[j] = storess_1[j] * table_1[j];
							b.m128 = _mm_loadu_ps(table_1 + j);
							r.m128 = _mm_mul_ps(a.m128, b.m128);
							_mm_storeu_ps(synthesis_1 + j, r.m128);

							a.m128 = _mm_loadu_ps(storess_2 + j);	// synthesis_2[j] = storess_2[j] * table_2[j];
							b.m128 = _mm_loadu_ps(table_2 + j);
							r.m128 = _mm_mul_ps(a.m128, b.m128);
							_mm_storeu_ps(synthesis_2 + j, r.m128);
						}
					}

					_CalcSamples;
				}
			}
			else
#endif
#ifdef ENGINE_AVX

	#pragma region <DCT16_AVX>

			#define _DCT16_AVX(in, out) {                               \
				int i;                                                  \
				float even_in8[8], even_out8[8];                        \
				float  odd_in8[8],  odd_out8[8];                        \
				UNION256 a, b, r;                                       \
																		\
				a.m256 = _mm256_loadu_ps(in);                           \
				b.m256 = _mm256_set_ps (                                \
					in[8],  in[9],  in[10], in[11],                     \
					in[12], in[13], in[14], in[15]  );                  \
					                                                    \
				r.m256 = _mm256_add_ps(a.m256, b.m256);                 \
				_mm256_storeu_ps(even_in8, r.m256);                     \
				                                                        \
				r.m256 = _mm256_sub_ps(a.m256, b.m256);                 \
				a.m256 = _mm256_mul_ps(r.m256, u_form_8_16.m256);       \
				_mm256_storeu_ps(odd_in8, a.m256);                      \
																		\
				_DCT8_SSE(even_in8, even_out8);                         \
				_DCT8_SSE(odd_in8, odd_out8);                           \
				                                                        \
				for (i = 0; i < 8; i++)                                 \
					out[2 * i] = even_out8[i];                          \
				                                                        \
				a.m128[0] = _mm_loadu_ps(odd_out8);                     \
				b.m128[0] = _mm_loadu_ps(odd_out8 + 1);                 \
				r.m128[0] = _mm_add_ps(a.m128[0], b.m128[0]);           \
				out[1] = r.f[0];                                        \
				out[3] = r.f[1];                                        \
				out[5] = r.f[2];                                        \
				out[7] = r.f[3];                                        \
				                                                        \
				for (i = 4; i < 7; i++)                                 \
					out[2 * i + 1] = odd_out8[i] + odd_out8[i + 1];     \
																		\
				out[15] = odd_out8[7];                                  \
			}

	#pragma endregion

	#pragma region <DCT32_AVX>

			#define _DCT32_AVX(in, out) {                               \
				int i;                                                  \
				float even_in[16], even_out[16];                        \
				float  odd_in[16],  odd_out[16];                        \
				UNION256 a, b, r;                                       \
																		\
				for (i = 0; i < 16; i += 8)                             \
				{                                                       \
					a.m256 = _mm256_loadu_ps(in + i);                   \
					b.m256 = _mm256_set_ps (                            \
					    in[24-i], in[25-i], in[26-i], in[27-i],         \
						in[28-i], in[29-i], in[30-i], in[31-i] );       \
						                                                \
					r.m256 = _mm256_add_ps(a.m256, b.m256);             \
					_mm256_storeu_ps(even_in + i, r.m256);              \
					                                                    \
					r.m256 = _mm256_sub_ps(a.m256, b.m256);             \
					a.m256 = _mm256_mul_ps(r.m256, u_form_16_32.m256[i>>3]); \
					_mm256_storeu_ps(odd_in + i, a.m256);               \
				}                                                       \
																		\
				_DCT16_AVX(even_in, even_out);                          \
				_DCT16_AVX(odd_in, odd_out);                            \
																		\
				for (i = 0; i < 16; i++)                                \
					out[2 * i] = even_out[i];                           \
					                                                    \
				a.m256 = _mm256_loadu_ps(odd_out);                      \
				b.m256 = _mm256_loadu_ps(odd_out + 1);                  \
				r.m256 = _mm256_add_ps(a.m256, b.m256);                 \
				for (i = 0; i < 8; i++)                                 \
					out[2 * i + 1] = r.f[i];                            \
					                                                    \
				a.m128[0] = _mm_loadu_ps(odd_out + 8);                  \
				b.m128[0] = _mm_loadu_ps(odd_out + 9);                  \
				r.m128[0] = _mm_add_ps(a.m128[0], b.m128[0]);           \
				out[2 * 8  + 1] = r.f[0];                               \
				out[2 * 9  + 1] = r.f[1];                               \
				out[2 * 10 + 1] = r.f[2];                               \
				out[2 * 11 + 1] = r.f[3];                               \
			                                                            \
				for (i = 12; i < 15; i++)                               \
					out[2 * i + 1] = odd_out[i] + odd_out[i + 1];       \
																		\
				out[31] = odd_out[15];                                  \
			}

	#pragma endregion

	#pragma region <PolyphaseMatrixing_AVX>

			#define _PolyphaseMatrixing_AVX(in, out) {				\
				int i;												\
				float tmp[32];										\
																	\
				_DCT32_AVX(in, tmp);								\
																	\
				out[16] = 0;										\
																	\
				memcpy(out, tmp + 16, 16 * sizeof(float));			\
				for (i = 17; i < 32; i++) out[i] = -tmp[48 - i];	\
				for (i = 32; i < 48; i++) out[i] = -tmp[48 - i];	\
				for (i = 48; i < 64; i++) out[i] = -tmp[i - 48];	\
			}

	#pragma endregion

			if (opt_AVX_only) // AVX CODE = ??%..??% time _ NOT TESTED
			{
				#pragma message ("ConvertMP3_SubbandSynth : 256-bit AVX code not tested")

				for (uint16 line = 0; line < MP3LINES; line++)
				{
					--storess_pos &= 0xF; // [0..15]

					for (uint16 subband = 0; subband < MP3SUBBANDS; subband++)
						synthesis[subband] = sample[subband * MP3LINES + line];

					{
						float * store64 = storess + (storess_pos << 6); // + (storess_pos * 64)
						_PolyphaseMatrixing_AVX(synthesis, store64);
					}

					for (uint16 i = 0; i < 8; i++) // [16][64] = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 } (1024)
					{
						float * synthesis_1 = synthesis + (i << 6);       // i * 64
						float * synthesis_2 = synthesis + (i << 6) + 32;  // i * 64 + 32
					//	float * storess_1 = storess + (i << 7);           // i * 128         0, 128, 256, 384, 512, 640, 768, 896, (1024) 
					//	float * storess_2 = storess + (i << 7) + 96;      // i * 128 + 96   96, 224, 352, 480, 608, 736, 864, 992
						float * storess_1 = storess + (((storess_pos << 6) + (i << 7)     ) % 1024); // storess[1024]
						float * storess_2 = storess + (((storess_pos << 6) + (i << 7) + 96) % 1024); // storess[1024]

						auto table_1 = subband_synthesis_table + (i << 6);
						auto table_2 = subband_synthesis_table + (i << 6) + 32;

						for (uint16 j = 0; j < MP3SUBBANDS; j += 8) // 32 = 4 x8 (SSE), 8 x4 (AVX), 16 x2 (AVX512F)
						{
							UNION256 a, b, r;

							a.m256 = _mm256_loadu_ps(storess_1 + j);	// synthesis_1[j] = storess_1[j] * table_1[j];
							b.m256 = _mm256_loadu_ps(table_1 + j);
							r.m256 = _mm256_mul_ps(a.m256, b.m256);
							_mm256_storeu_ps(synthesis_1 + j, r.m256);

							a.m256 = _mm256_loadu_ps(storess_2 + j);	// synthesis_2[j] = storess_2[j] * table_2[j];
							b.m256 = _mm256_loadu_ps(table_2 + j);
							r.m256 = _mm256_mul_ps(a.m256, b.m256);
							_mm256_storeu_ps(synthesis_2 + j, r.m256);
						}
					}

					_CalcSamples;
				}
			}
			else
#endif
#ifdef ENGINE_AVX512F

	#pragma region <DCT32_AVX512F>

			#define _DCT32_AVX512F(in, out) {                           \
				int i;                                                  \
				float even_in[16], even_out[16];                        \
				float  odd_in[16],  odd_out[16];                        \
				UNION512 a, b, r;                                       \
																		\
				a.m512 = _mm512_loadu_ps(in);                           \
				b.m512 = _mm512_set_ps (                                \
					in[16], in[17], in[18], in[19],                     \
					in[20], in[21], in[22], in[23],                     \
					in[24], in[25], in[26], in[27],                     \
					in[28], in[29], in[30], in[31] );                   \
					                                                    \
				r.m512 = _mm512_add_ps(a.m512, b.m512);                 \
				_mm512_storeu_ps(even_in, r.m512);                      \
				                                                        \
				r.m512 = _mm512_sub_ps(a.m512, b.m512);                 \
				a.m512 = _mm512_mul_ps(r.m512, u_form_16_32.m512);      \
				_mm512_storeu_ps(odd_in, a.m256);                       \
																		\
				_DCT16_AVX(even_in, even_out);                          \
				_DCT16_AVX(odd_in, odd_out);                            \
																		\
				for (i = 0; i < 16; i++)                                \
					out[2 * i] = even_out[i];                           \
					                                                    \
				a.m256[0] = _mm256_loadu_ps(odd_out);                   \
				b.m256[0] = _mm256_loadu_ps(odd_out + 1);               \
				r.m256[0] = _mm256_add_ps(a.m256[0], b.m256[0]);        \
				for (i = 0; i < 8; i++)                                 \
					out[2 * i + 1] = r.f[i];                            \
					                                                    \
				a.m128[0] = _mm_loadu_ps(odd_out + 8);                  \
				b.m128[0] = _mm_loadu_ps(odd_out + 9);                  \
				r.m128[0] = _mm_add_ps(a.m128[0], b.m128[0]);           \
				out[2 * 8  + 1] = r.f[0];                               \
				out[2 * 9  + 1] = r.f[1];                               \
				out[2 * 10 + 1] = r.f[2];                               \
				out[2 * 11 + 1] = r.f[3];                               \
			                                                            \
				for (i = 12; i < 15; i++)                               \
					out[2 * i + 1] = odd_out[i] + odd_out[i + 1];       \
																		\
				out[31] = odd_out[15];                                  \
			}

	#pragma endregion

	#pragma region <PolyphaseMatrixing_AVX512F>

			#define _PolyphaseMatrixing_AVX512F(in, out) {			\
				int i;												\
				float tmp[32];										\
																	\
				_DCT32_AVX512F(in, tmp);							\
																	\
				out[16] = 0;										\
																	\
				memcpy(out, tmp + 16, 16 * sizeof(float));			\
				for (i = 17; i < 32; i++) out[i] = -tmp[48 - i];	\
				for (i = 32; i < 48; i++) out[i] = -tmp[48 - i];	\
				for (i = 48; i < 64; i++) out[i] = -tmp[i - 48];	\
			}

	#pragma endregion

			if (opt_AVX512F_only) // AVX512F CODE = ??%..??% time _ NOT TESTED
			{
				#pragma message ("ConvertMP3_SubbandSynth : 512-bit AVX512F code not tested")

				for (uint16 line = 0; line < MP3LINES; line++)
				{
					--storess_pos &= 0xF; // [0..15]

					for (uint16 subband = 0; subband < MP3SUBBANDS; subband++)
						synthesis[subband] = sample[subband * MP3LINES + line];

					{
						float * store64 = storess + (storess_pos << 6); // + (storess_pos * 64)
						_PolyphaseMatrixing_AVX512F(synthesis, store64);
					}

					for (uint16 i = 0; i < 8; i++) // [16][64] = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 } (1024)
					{
						float * synthesis_1 = synthesis + (i << 6);       // i * 64
						float * synthesis_2 = synthesis + (i << 6) + 32;  // i * 64 + 32
					//	float * storess_1 = storess + (i << 7);           // i * 128         0, 128, 256, 384, 512, 640, 768, 896, (1024) 
					//	float * storess_2 = storess + (i << 7) + 96;      // i * 128 + 96   96, 224, 352, 480, 608, 736, 864, 992
						float * storess_1 = storess + (((storess_pos << 6) + (i << 7)     ) % 1024); // storess[1024]
						float * storess_2 = storess + (((storess_pos << 6) + (i << 7) + 96) % 1024); // storess[1024]

						auto table_1 = subband_synthesis_table + (i << 6);
						auto table_2 = subband_synthesis_table + (i << 6) + 32;

						for (uint16 j = 0; j < MP3SUBBANDS; j += 16) // 32 = 4 x8 (SSE), 8 x4 (AVX), 16 x2 (AVX512F)
						{
							UNION512 a, b, r;

							a.m512 = _mm512_loadu_ps(storess_1 + j);	// synthesis_1[j] = storess_1[j] * table_1[j];
							b.m512 = _mm512_loadu_ps(table_1 + j);
							r.m512 = _mm512_mul_ps(a.m512, b.m512);
							_mm512_storeu_ps(synthesis_1 + j, r.m512);

							a.m512 = _mm512_loadu_ps(storess_2 + j);	// synthesis_2[j] = storess_2[j] * table_2[j];
							b.m512 = _mm512_loadu_ps(table_2 + j);
							r.m512 = _mm512_mul_ps(a.m256, b.m256);
							_mm512_storeu_ps(synthesis_2 + j, r.m512);
						}
					}

					_CalcSamples;
				}
			}
			else
#endif		// SIMPLE CODE = 100% time
			{
				for (uint16 line = 0; line < MP3LINES; line++)
				{
					--storess_pos &= 0xF; // [0..15]

					for (uint16 subband = 0; subband < MP3SUBBANDS; subband++)
						synthesis[subband] = sample[subband * MP3LINES + line];

					{
						float * store64 = storess + (storess_pos << 6); // + (storess_pos * 64)
						_PolyphaseMatrixing(synthesis, store64);
					}

					for (uint16 i = 0; i < 8; i++) // [16][64] = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 } (1024)
					{
						float * synthesis_1 = synthesis + (i << 6);       // i * 64
						float * synthesis_2 = synthesis + (i << 6) + 32;  // i * 64 + 32
					//	float * storess_1 = storess + (i << 7);           // i * 128         0, 128, 256, 384, 512, 640, 768, 896, (1024) 
					//	float * storess_2 = storess + (i << 7) + 96;      // i * 128 + 96   96, 224, 352, 480, 608, 736, 864, 992
						float * storess_1 = storess + (((storess_pos << 6) + (i << 7)     ) % 1024); // storess[1024]
						float * storess_2 = storess + (((storess_pos << 6) + (i << 7) + 96) % 1024); // storess[1024]

						auto table_1 = subband_synthesis_table + (i << 6);
						auto table_2 = subband_synthesis_table + (i << 6) + 32;

						for (uint16 j = 0; j < MP3SUBBANDS; j++)
						{
							synthesis_1[j] = storess_1[j] * table_1[j];
							synthesis_2[j] = storess_2[j] * table_2[j];
						}
					}

					_CalcSamples;
				}
			}

			#ifdef ENGINE_AVX512F
				#undef _PolyphaseMatrixing_AVX512F
				#undef _DCT32_AVX512F
			#endif

			#ifdef ENGINE_AVX
				#undef _PolyphaseMatrixing_AVX
				#undef _DCT32_AVX
				#undef _DCT16_AVX
			#endif

			#ifdef ENGINE_SSE
				#undef _PolyphaseMatrixing_SSE
				#undef _DCT32_SSE
				#undef _DCT16_SSE
				#undef _DCT8_SSE
			#endif

			#undef _CalcSamples
			#undef _PolyphaseMatrixing
			#undef _DCT32
			#undef _DCT16
			#undef _DCT8
			#undef _DCT4
			#undef _DCT2
		}

		//>> [MAIN]
		bool ConvertMP3()
		{
			uint32 frames         = 0;						// total frames counter
			uint32 frames_skipped = 0;						// skipped frames counter
			uint32 frames_loop    = start_frame;			// frame # in loop
			byte * result_granule = settings.out.data_out;	// out data ptr for current frame
			bool   padding_enable = false;					// do 'before' padding or not
			uint32 padding_skip_counter = 0;				// total skipped samples (by LAME padding)

			for (; frames < stop_frame; frames++, frames_loop++)
			{
				////////////// HANDLE PADDING ///////////////

				if (frames_loop == 1)
				{
					padding_b = padding_before; // update for a new loop round

					if (padding_after) 
					{
						// ЗАМЕТКА надеюсь в конце не бывает мусорных фреймов, иначе стоит
						// взять (MPEGD.lame.XING.frames + 1) вместо (total_frames)

						// calc 'after' frame #  (start of end padding)
						padding_a_frame = total_frames - (padding_after / MP31152) - 1;

						// calc how many samples to save at 'after' frame
						padding_a = (total_frames - padding_a_frame) * MP31152 - padding_after;
					}
				}

				if ( padding_b && 
					 (frames_loop == 1 ||				  // first start frame OR
					  frames_loop == padding_b_frame + 1) // next frame in chain (continue)
				   )
				{
					padding_enable  = true;
					padding_b_frame = frames_loop; // mem this frame
				}
				else // break chain of frames & disable 'before' padding
				{
					padding_enable  = false;
					padding_b_frame = 0;
					padding_b       = 0;
				}

				////////////// LOOP CONDITION ///////////////

				if (frames_loop == total_frames)
				{
					frames_loop = 0;

					// reset runner
					runner.p    = settings.in.data;
					runner.end  = runner.p + settings.in.size;
					runner.pos  = 0;
					runner.last = settings.in.size;

					// reset static data
					ZeroMemory(storehs,     sizeof(storehs));
					ZeroMemory(storess,     sizeof(storess));
					ZeroMemory(storess_pos, sizeof(storess_pos));
					MPEGD._Reset_fast();
				}

				///////////////////// READING /////////////////////

				uint32 mem_start_pos = runner.pos;

				if (!MPEG._Read(runner.p, runner.last)) // EOF
					break;

				if (!MPEG.RUNFRAMECALC()) // incorrect frame data (MP3 DONE)
					break;

				// move to side info
				RunnerOffset(MPG_SIZE + MPEG.FRAME.crc);

				// local runner
				byte * local_ptr = runner.p;
				byte   local_bit = 0;

				MPEGS._Read(local_ptr, local_bit, MPEG_p);

				// ERROR : this can't be
				//if (MPEG.FRAME.side != (local_ptr - runner.p)) 
				//	break;

				// ERROR : this can't be
				//if (local_bit != 0)
				//	{ printf("\nlocal.bit != 0"); getchar(); }
					//break;

				// move to mpeg main data
				RunnerOffset(MPEG.FRAME.side);

				//MPEGD.offset = runner.from.pos; // debug

				uint32 MPEGDStatus = MPEGD._Read(local_ptr, local_bit, MPEG_p, MPEGS_p);

				// move to next frame
				RunnerShift(mem_start_pos + MPEG.FRAME.size);

			//	printf("\n%3i: bitrate %i", frames, MPEG.FRAME.bitrate);
			//	printf("\n%3i: size %i", frames, MPEG.FRAME.size);
			//	printf("\n%3i: pure %i", frames, MPEG.FRAME.pure);

				// handle errors
				if (MPEGDStatus != eMPEGDStatus_Normal)
				{
					if (MPEGDStatus & eMPEGDStatus_PhysFrame)
					{
						//printf("\nSkip frame # %i : eMPEGDStatus_PhysFrame", frames);
						frames_skipped++;
						//if (frames > ((MP3JUMPS/2)-1))
						//{
						//	printf("PhysFrame border error & frame # %i", frames); getchar();
						//	return false;
						//}
						//else
						continue;
					}
					if (MPEGDStatus & eMPEGDStatus_Huffman) // critical error - EXIT
					{
						//printf("\nDEBUG: check huffman table error!"); getchar();
						STATUS = eMP3ConvStatus_HuffmanRead;
						return false;
					}
					if (MPEGDStatus & eMPEGDStatus_Part23end) // critical error - EXIT
					{
						//printf("\nERROR due stopping read huffman bits... check it!"); getchar();
						STATUS = eMP3ConvStatus_HuffmanEnd;
						return false;
					}
					if ((MPEGDStatus & eMPEGDStatus_nullData) == eMPEGDStatus_nullData)
						if (frames_loop == 0)
						{
							if (MPEGDStatus & eMPEGDStatus_XingLame)
							{
								padding_before = MPEGD.lame.LAME.misc.delay_start;
								padding_after  = MPEGD.lame.LAME.misc.delay_end;

								if (padding_before) padding_before += MP3LAMEDECDELAY + 1;
								if (padding_after)  padding_after  -= MP3LAMEDECDELAY + 1;

								if ((int32)padding_after < 0) {
									warning |= eMP3ConvWarning_LamePadding;
									padding_before = 0;
									padding_after  = 0;
								}

								//printf("\nSkip frame # %i : eMPEGDStatus_XingLame", frames);
								frames_skipped++;
								continue;
							}
							else if (MPEGDStatus & eMPEGDStatus_Fraunhofer)
							{
								//printf("\nSkip frame # %i : eMPEGDStatus_Fraunhofer", frames);
								frames_skipped++;
								continue;
							}
						}
					//if ((MPEGDStatus & eMPEGDStatus_nullData) != eMPEGDStatus_nullData)
					//{
					//	if (MPEGDStatus & eMPEGDStatus_nullData00)
					//		printf("\n\ng0c0 frame # %i : eMPEGDStatus_nullData00", frames);
					//	if (MPEGDStatus & eMPEGDStatus_nullData01)
					//		printf("\ng0c1 frame # %i : eMPEGDStatus_nullData01", frames);
					//	if (MPEGDStatus & eMPEGDStatus_nullData10)
					//		printf("\ng1c0 frame # %i : eMPEGDStatus_nullData10", frames);
					//	if (MPEGDStatus & eMPEGDStatus_nullData11)
					//		printf("\ng1c1 frame # %i : eMPEGDStatus_nullData11\n", frames);
					//	continue;
					//}
				}

				///////////////////// CONVERTING /////////////////////

				bool mpeg_1   = (MPEG.mpeg == eMPEGType1);
				bool mono     = (MPEG.ch == eMPEGChSingle);
				byte channels = mono ? 1 : 2;
				byte granules = mpeg_1 ? eMPEG_1_Granules : eMPEG_2_Granules;

				for (byte gr = 0; gr < granules; gr++)
				{
					uint32 skip_samples = 0; // LAME encoder padding

					if (padding_enable) // check how many samples to skip
					{
						if (padding_b > MP3GRS) skip_samples = MP3GRS;
						else                    skip_samples = padding_b;

						padding_skip_counter += skip_samples;
					}

					///////////// begin /////////////

					for (byte ch = 0; ch < channels; ch++)
					{
						ConvertMP3_Requantize(gr, ch);
						ConvertMP3_Reorder(gr, ch);
					}

					ConvertMP3_ProcessStereo(gr);

					for (byte ch = 0; ch < channels; ch++)
					{
						ConvertMP3_Antialias(gr, ch);
						ConvertMP3_HybridSynth(gr, ch);
						ConvertMP3_FreqInversion(gr, ch);
						ConvertMP3_SubbandSynth(gr, ch, mono, (uint32*)result_granule, skip_samples);
					}

					///////////// end /////////////

					// manage 'before' padding
					if (padding_enable)
					{
						if (padding_b > MP3GRS) // skip_samples == MP3GRS
						{
							padding_b -= MP3GRS;
						}
						else // padding <= MP3GRS
						{
							result_granule += ((uint32)MP3GRS - skip_samples) << 2; // * 2ch * 16bit
							padding_b       = 0;
							padding_enable  = false;
						}
					}
					else // no padding
					{
						result_granule += MP3FRAMESIZE_half;
					}

					// manage 'after' padding
					if (padding_after)
					{
						if (frames_loop == padding_a_frame)
						{
							result_granule -= MP3FRAMESIZE_half; // make it back

							uint32 save;

							if (!gr) save = (padding_a > MP3GRS) ? MP3GRS : padding_a;
							else     save = (padding_a > MP3GRS) ? (MP3GRS - (MP31152 - padding_a)) : 0;

							result_granule       += (save << 2); // * 2ch * 16bit
							padding_skip_counter += MP3GRS - save;
						}
						else if (frames_loop > padding_a_frame)
						{
							result_granule -= MP3FRAMESIZE_half; // make it back

							padding_skip_counter += MP3GRS;
						}
					}
				}
			}

			continue_frame = frames_loop; // provide Continue() mode

			settings.out._data = runner.p;    // debug info
			settings.out._size = runner.last; // debug info

			settings.out._padding        = padding_skip_counter; // debug info
			settings.out._padding_before = padding_before;		 // debug info
			settings.out._padding_after  = padding_after;		 // debug info

			// FIX OUT DATA (вычитаем из ожидаемого сколько кадров пропустили и сколько семплов выбросили в связи с LAME-паддингом)

			settings.out.size_out -= frames_skipped * MP3FRAMESIZE;
			settings.out.frames   -= frames_skipped;
			settings.out.samples  -= frames_skipped * MP31152;
			settings.out.time     -= frames_skipped * frame_time;
			settings.out._skipped  = frames_skipped; // debug info

			settings.out.size_out -= padding_skip_counter << 2; // * 2ch * 16bit
			settings.out.samples  -= padding_skip_counter;
			settings.out.time     -= (float)padding_skip_counter / settings.out.nSamplesPerSec;

			if (settings.out.time < 0.00002f) // 0.00002 s * 48000 KHz = 0.96 s
				settings.out.time = 0;

			// ERROR : unexpected <out.samples>
			if ((int32)settings.out.samples < 0)
				{ STATUS = eMP3ConvStatus_OutSamples; return false; }

			// ERROR : ?
			//if (frames < stop_frame)
			//	return false;

			//printf("\nMP3 CONV DONE");

			return true;
		}

	private:
		//>> Проверка на первичные ошибки запуска
		bool CheckRun()
		{
			if (settings.in.data == nullptr)
				{ STATUS = eMP3ConvStatus_NoInputPtr; return false; }

			if (settings.in.size == 0)
				{ STATUS = eMP3ConvStatus_NoInputSz;  return false; }

			if (settings.in.size > MAXMP3SIZE)
				{ STATUS = eMP3ConvStatus_MaxInput;   return false; }

			if (settings.in.start_time < 0)								{
				warning |= eMP3ConvWarning_StartTime;
				settings.in.start_time = abs(settings.in.start_time);	}

			if (settings.in.time < 0)									{
				warning |= eMP3ConvWarning_Time;
				settings.in.time = abs(settings.in.time);				}

			return true;
		}

		//>> Установка начальных значений
		bool PreSet(bool continue_mode)
		{
			if (!continue_mode) // Run() mode
			{
				ZeroMemory(storehs,     sizeof(storehs));     // HybridSynth
				ZeroMemory(storess,     sizeof(storess));     // SubbandSynth
				ZeroMemory(storess_pos, sizeof(storess_pos)); // SubbandSynth

				MPEGD._Reset_fast(); // reset phys. frame borders

				if (settings.in.data_mem != settings.in.data ||
					settings.in.size_mem != settings.in.size)
				{
					padding_b       = 0; // Source changed, reset padding memory
					padding_a       = 0; // .
					padding_b_frame = 0; // .
					padding_a_frame = 0; // .
					padding_before  = 0; // .
					padding_after   = 0; // .
				}

				settings.in.data_mem = settings.in.data;
				settings.in.size_mem = settings.in.size;

				frame0_position = 0;
				start_position  = MPG_ERROR;
				start_frame     = MPG_ERROR;
				total_frames    = 0;
				stop_frame      = MPG_ERROR;
				frame_time      = MPG_ERROR;

				settings.out.Reset();
			}
			else // Continue() mode
			{
				// ERROR : unsafe modification of input data
				if (settings.in.data_mem != settings.in.data ||
					settings.in.size_mem != settings.in.size)
					{ STATUS = eMP3ConvStatus_InputMod; return false; }

				// ERROR : converter should be initialized with Run() before Contimue()
				if (frame_time == MPG_ERROR)
					{ STATUS = eMP3ConvStatus_NoRun; return false; }

			//	frame0_position = 0;			
				start_position  = MPG_ERROR;
				start_frame     = MPG_ERROR;
			//	total_frames    = 0;
				stop_frame      = MPG_ERROR;
			//	frame_time      = MPG_ERROR;

				settings.out.Reset_continue();
			}

			///////////////////////////////////////////////////////
		
			if (!continue_mode) // Run() mode : calc everything from the beginning
			{
				runner.p    = settings.in.data;
				runner.end  = runner.p + settings.in.size;
				runner.pos  = 0;
				runner.last = settings.in.size;

				if (!SkipID3(runner.p, runner.last, frame0_position)) // EOF
					{ STATUS = eMP3ConvStatus_EOF_ID3; return false; }

				RunnerShift(frame0_position);

				// init calculations & validation process
				for (;; total_frames++)
				{
					uint32 mem_start_pos = runner.pos;

					if (!MPEG._Read(runner.p, runner.last)) // EOF
						break;                              // .

					uint32 frame_size = MPEG._Frame();

					if (frame_size == MPG_ERROR) // incorrect input
						break;                   // .

					// move to next frame  OR  break if cur frame data is not full
					if (!RunnerShift(mem_start_pos + frame_size))
						break;

					if (!_Check(MPEG, false)) // not MPEG-1 Layer 3 frame
						{ STATUS = eMP3ConvStatus_DiffFrames; return false; }

					if (!MPEG.RUNFRAMECALC()) // incorrect frame data
						{ STATUS = eMP3ConvStatus_HeaderFail; return false; }

					// do once
					if (!total_frames) 
					{
						settings.out.nChannels      = 2; // MPEG.FRAME.channels;
						settings.out.nSamplesPerSec = MPEG.FRAME.frequency;
						settings.out.wBitsPerSample = 16;
						settings.out.wFormatTag     = 1;
						settings.out.fsz_samples    = MP31152;
						settings.out.fsz_bytes      = MP3FRAMESIZE;
						settings.out.fsz_time       = MP31152 * (1.f / MPEG.FRAME.frequency);

						frame_time = (float)MP31152 / MPEG.FRAME.frequency;

						if (settings.in.use_frame)
							 start_frame = settings.in.start_frame;
						else start_frame = (uint32)(settings.in.start_time / frame_time);
					}

					// ERROR : all frames should have same frequency
					if (settings.out.nSamplesPerSec != MPEG.FRAME.frequency)
						{ STATUS = eMP3ConvStatus_DiffFreq; return false; }

					// get start position
					if (total_frames == start_frame)
						start_position = mem_start_pos;
				}
			}
			else // Continue() mode : recalc some
			{
				start_position = runner.pos;
				start_frame    = continue_frame;
			}

			///////////////////////////////////////////////////////

			// ERROR : invalid start_time or start_frame
			if (start_position == MPG_ERROR)
				{ STATUS = eMP3ConvStatus_StartPosErr; return false; }

			// move to start position
			RunnerShift(start_position);

			// count frames to proceed
			if (settings.in.time == 0)
				stop_frame = total_frames - start_frame;
			else
			{
				uint32 requested_frames;
				float  delta_start_time; // difference between requested and real start point
				float  settings_in_time;

				if (!settings.in.use_frame)
					 delta_start_time = settings.in.start_time - start_frame * frame_time;
				else delta_start_time = 0.f;

				// DEBUG check
				if (delta_start_time < 0)
					printf("\nWARNING MP3conv : delta_start_time < 0");

				// добавить расхождение к заказанному времени
				settings_in_time = settings.in.time + delta_start_time;

				if (settings_in_time < frame_time) // auto fix small request
					requested_frames = 1;
				else
				{                         
					float requested_frames_f = settings_in_time / frame_time;

					if (settings.in.maximize)
						 requested_frames = (uint32) ceil (requested_frames_f);
					else 
						 requested_frames = (uint32)       requested_frames_f;  // floor (requested_frames_f);
				}

				uint32 available_frames = total_frames - start_frame;			

				if (!settings.in.loop && available_frames < requested_frames)
					 stop_frame = available_frames;
				else stop_frame = requested_frames;
			}

			// ERROR : there is no available data AND it's not a loop
			if (stop_frame == 0)
				{ STATUS = eMP3ConvStatus_BufferEnd; return false; }

			return true;
		}

		//>> Подготовка выходных данных
		bool CalcOut()
		{
			// NOTE : some out data may be fixed due convertion in case of eMPEGDStatus_PhysFrame (skipped frames)

			////// OUT PCM BUF //////
		
			{
				uint32 target_size = stop_frame * MP3FRAMESIZE;

				if (settings.out.data_out_mem != settings.out.data_out)
				{
					warning |= eMP3ConvWarning_OutMod;
				}

				if (settings.out.data_out == nullptr)
				{
					settings.out.size_out     = target_size;
					settings.out.data_out     = (byte*) malloc(target_size);
					settings.out.size_out_mem = target_size;
					settings.out.data_out_mem = settings.out.data_out;
				}
				else if (settings.out.size_out_mem < target_size) // realloc
				{
					free(settings.out.data_out);
					settings.out.size_out     = target_size;
					settings.out.data_out     = (byte*) malloc(target_size);
					settings.out.size_out_mem = target_size;
					settings.out.data_out_mem = settings.out.data_out;
				}
				else // no need to alloc memory, just use old one
				{
					settings.out.size_out = target_size;
				}

				// <settings.out.size_out> will be fixed later by <frames_skipped>

				// ERROR : can't allocate memory
				if (settings.out.data_out == nullptr)
					{ STATUS = eMP3ConvStatus_AllocOutErr; return false; }
			}

			////// OTHER OUT //////
	
			settings.out.start_frame = start_frame;
			settings.out.start_time  = start_frame * frame_time;

			settings.out.frames  = stop_frame;						// fixed later by <frames_skipped>
			settings.out.time    = stop_frame * frame_time;			// fixed later by <frames_skipped>
			settings.out.samples = stop_frame * MP31152;			// fixed later by <frames_skipped>

			settings.out.total_frames   = total_frames;
			settings.out.end_frame      = start_frame + stop_frame;
			settings.out.loop_end_frame = settings.out.end_frame % total_frames;

			return true;
		}

		//>> Запуск MP3 конвертора
		eMP3ConvStatus Run(bool continue_mode)
		{
			STATUS  = eMP3ConvStatus_Normal;
			warning = eMP3ConvWarning_Normal;								
										// POSSIBLE ERRORS
			if (CheckRun())				// error input
			if (PreSet(continue_mode))	// EOF ? error input
			if (CalcOut())				// alloc memory fail
			ConvertMP3();				// internal convertion error

			return STATUS;
		}

	public:
		//>> Запуск MP3 конвертора с начала (инициализирующий пуск)
		eMP3ConvStatus Run()
		{
			return Run(false);
		}

		//>> Продолжение работы с последней точки останова (нет сброса статических данных)
		eMP3ConvStatus Continue()
		{
			return Run(true);
		}

	private:
		static
		//>> Сообщает позицию начала данных, пропуская ID3 заголовок :: возвращает <false> в случае EOF
		bool SkipID3(byte * data, uint32 size, uint32 & position)
		{
			ID3TAG ID3;

			for (;;)
			{
				if (!ID3._Read(data + position, size - position)) // EOF
					return false;

				if (ID3.sign == ID3_SIGN) // skip ID3
				{
					if (ID3.size + ID3_SIZE > size) // EOF
						return false;

					position += ID3.size + ID3_SIZE;

					for (;; position++) // skip NULL segment if presented
					{
						if (position >= size) return false;
						if (*(data + position)) break;
					}
				}
				else // ID3 not found
				{
					return true;
				}
			}
		}

		static
		//>> Проверка заголовка на возможность работы с кадром
		bool _Check(MPEGHeader & MPEG, bool deep_test)
		{
			if (MPEG.sync != MPG_SYNC) // sync FAIL
				return false;

			if (MPEG.mpeg != eMPEGType1) // MPEG-1 only
				return false;

			if (MPEG.layer != eMPEGLayer3) // Layer-3 only
				return false;

			if (MPEG.emph != eMPEGEmphasisNONE) // NONE-Emphasis only
				return false;

			if (deep_test)
			{
				if (MPEG._Bitrate() == MPG_ERROR) // ERROR bitrate
					return false;

				if (MPEG._Samples() == MPG_ERROR) // ERROR samples per frame
					return false;

				if (MPEG._Frequency() == MPG_ERROR) // ERROR samples per second
					return false;

			//	if (MPEG._CheckLayerII() == MPG_ERROR) // LAYER 2 ERROR
			//		return false;
			}

			return true;
		}

	public:
		static
		//>> Анализ длительности
		bool _Duration(byte * data, uint32 size, float & out_seconds)
		{
			uint32 position = 0;
			uint32 frames   = 0;

			if (!SkipID3(data, size, position)) // EOF
				return false;                   // .

			////////////////////////////////////

			MPEGHeader MPEG;

			if (!MPEG._Read(data + position, size - position)) // EOF
				return false;                                  // .

			if (!MPEG.RUNFRAMECALC()) // incorrect frame data
				return false;         // .

			uint32 frequency  = MPEG.FRAME.frequency;
			uint32 frame_size = MPEG.FRAME.size;
			float  frame_time = (float)MP31152 / frequency;		

			////////////////////////////////////

			for (frames = 1;; frames++)
			{
				position += frame_size;

				if (position >= size) // EOF
					break;            // .

				if (!MPEG._Read(data + position, size - position)) // EOF
					break;                                         // .

				if (MPG_ERROR == (frame_size = MPEG._Frame())) // incorrect input
					break;                                     // .

				if (frequency != MPEG._Frequency()) // critical error
					return false;                   // .
			}

			////////////////////////////////////

			out_seconds = frame_time * frames;

			return true;
		}

		static
		//>> Проверка MP3
		bool _Check(byte * data, uint32 size)
		{
			ID3TAG      ID3;
			MPEGHeader MPEG;

			uint32 position = 0;

			if (size > MAXMP3SIZE) // ignore too big files
			{
				wprintf(L"\nToo big mp3 file: %i MB (max %i MB)",
					_BYTESTOMB(size), _BYTESTOMB(MAXMP3SIZE));
				return false;
			}

			if (size > MAXMP3CHECK) // check only first MAXMP3CHECK bytes
				size = MAXMP3CHECK;

			///////// ID3 check /////////

			if (!SkipID3(data, size, position))
				return false;

			///////// MPEG check /////////

			if (!MPEG._Read(data + position, size - position)) // EOF
				return false;

			if (!_Check(MPEG, true))
				return false;

			position += MPG_SIZE;

			///////// MPEG-1 Layer-3 first frame size check /////////

			uint32 frame = MPEG._Frame();

			if (frame == MPG_ERROR ||      // ERROR
				frame > (size - position)) // EOF
				return false;

			return true;
		}

		static
		//>> Проверка MP3 : DEBUG
		bool _Check(FILE * fp)
		{
			uint32 fsize = (uint32) _filelengthi64(_fileno(fp));

			if (fsize > MAXMP3SIZE) // ignore too big files
			{
				wprintf(L"\nToo big mp3 file: %i MB (max %i MB)",
					_BYTESTOMB(fsize), _BYTESTOMB(MAXMP3SIZE));
				return false;
			}

			TBUFFER <byte, uint32> data;
			bool hRes = data.Create((fsize > MAXMP3CHECK) ? MAXMP3CHECK : fsize);

			if (!hRes) // mem alloc failed
				return false; 

			rewind(fp);
			fread(data.buf, 1, data.count, fp);

			return _Check(data.buf, data.count);
		}

	};

}

#endif // _MP3_H