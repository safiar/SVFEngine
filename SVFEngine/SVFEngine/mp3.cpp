// ----------------------------------------------------------------------- //
//
// MODULE  : mp3.cpp
//
// PURPOSE : Декодер MPEG-1 Layer-3 -- генерация статических таблиц
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#include "mp3.h"

#define MP3FORMULA(i,x) (1.f / (2.f * cos ((2.f * i + 1.f) * (MathPI / (2.f * x)))))

#define SAMPLE_POWER (4.f / 3.f)
#define POW2A_COUNT (1 + 2 * (MP3MAXSCAL + MP3MAXPRET)) // == 1 + 38 == 1 + <scale_variants 2> * <max_sum 15 + 4>
#define POW2B_COUNT (1 + 256 * 8) // 1 + 2048 = 1 + <max global_gain 256> * <max subblock_gain 8>
#define COS1TAB_COUNT (1 + 12 * 6)  // p < 12 && m < 6   : cos(PI / (12 * 2) * (2 * p + ((12 / 2) + 1)) * (2 * m + 1));
#define COS2TAB_COUNT (1 + 36 * 18) // p < 36 && m < 18  : cos(PI / (36 * 2) * (2 * p + ((36 / 2) + 1)) * (2 * m + 1));

using namespace SAVFGAME;

namespace SAVFGAME
{
	float CMP3Converter_hint::pow43[MP3MAXHUFF];  // набор значений семплов, возведённых в степень 4/3 : ret = sample^(4/3)
	float CMP3Converter_hint::pow2A[POW2A_COUNT]; // набор значений множителя x1 == 2^A, где A = -scale * (scalefac_l + pretab) или A = -scale * scalefac_s
	float CMP3Converter_hint::pow2B[POW2B_COUNT]; // набор значений множителя x2 == 2^B, где B = 1/4 * (global_gain - 210) или B = 1/4 * (global_gain - 210 - 8 * sub_gain)

	float CMP3Converter_hint::ratio_l[7]; // left  audio rate
	float CMP3Converter_hint::ratio_r[7]; // right audio rate
	UNIONMAX   CMP3Converter_hint::uSqrt2inv;						 // Process stereo SSE/AVX/AVX-256F opt. table
	UNION512 & CMP3Converter_hint::uSqrt2inv512 = uSqrt2inv;		 // Process stereo SSE/AVX/AVX-256F opt.
	UNION256 & CMP3Converter_hint::uSqrt2inv256 = uSqrt2inv.u256[0]; // Process stereo SSE/AVX/AVX-256F opt.
	UNION128 & CMP3Converter_hint::uSqrt2inv128 = uSqrt2inv.u128[0]; // Process stereo SSE/AVX/AVX-256F opt.

	UNION512 CMP3Converter_hint::ucs2; // SSE opt. cs antialias constants (x2)
	UNION512 CMP3Converter_hint::uca2; // SSE opt. ca antialias constants (x2)
	UNION256 & CMP3Converter_hint::ucs = CMP3Converter_hint::ucs2.u256[0]; // SSE opt. cs antialias constants
	UNION256 & CMP3Converter_hint::uca = CMP3Converter_hint::uca2.u256[0]; // SSE opt. ca antialias constants
//	UNION256 CMP3Converter_hint::ucs; // SSE opt. cs antialias constants
//	UNION256 CMP3Converter_hint::uca; // SSE opt. ca antialias constants
//	float CMP3Converter_hint::cs[MP3ANTIALI]; // cs antialias constants
//	float CMP3Converter_hint::ca[MP3ANTIALI]; // ca antialias constants

	CMP3Converter_hint::
	UNION_C_SIN CMP3Converter_hint::u_IMDCT_sin[eMPEGBlockT_ENUM_MAX]; // IMDCT sin tables
	float *     CMP3Converter_hint::  IMDCT_sin[eMPEGBlockT_ENUM_MAX]; // IMDCT sin tables
//	float CMP3Converter_hint::IMDCT_sin[eMPEGBlockT_ENUM_MAX][MP3LINES * 2]; // IMDCT sin tables
	float CMP3Converter_hint::IMDCT_cos1tab[COS1TAB_COUNT]; // IMDCT (simple ver.) cos table [1]
	float CMP3Converter_hint::IMDCT_cos2tab[COS2TAB_COUNT]; // IMDCT (simple ver.) cos table [2]

	const float CMP3Converter_hint::c_sqrt32 = (float)sqrt(3) / 2.f;             // IMDCT (fast)
	const float CMP3Converter_hint::c_cos9   = cos(MathPI / 9.f);                // IMDCT (fast)
	const float CMP3Converter_hint::c_cos94  = cos((4.f * MathPI) / 9.f);        // IMDCT (fast)
	const float CMP3Converter_hint::c_cos92  = cos((2.f * MathPI) / 9.f);        // IMDCT (fast)
	const float CMP3Converter_hint::c_sin0   = sin((2 * 0 + 1)*(MathPI / 18.f)); // IMDCT (fast)
	const float CMP3Converter_hint::c_sin1   = sin((2 * 1 + 1)*(MathPI / 18.f)); // IMDCT (fast)
	const float CMP3Converter_hint::c_sin2   = sin((2 * 2 + 1)*(MathPI / 18.f)); // IMDCT (fast)
	const float CMP3Converter_hint::c_sin3   = sin((2 * 3 + 1)*(MathPI / 18.f)); // IMDCT (fast)
	UNION128    CMP3Converter_hint::c_sin;

	UNION1024 CMP3Converter_hint::u_form_18_36; // (IMDCT) mp3 formula (i,36)   form_18_36 [18]
	UNION512  CMP3Converter_hint::u_form_9_18;  // (IMDCT) mp3 formula (i,18)	form_9_18  [9]	
	UNION256  CMP3Converter_hint::u_form_6_12;  // (IMDCT) mp3 formula (i,12)   form_6_12  [7]
	UNION128  CMP3Converter_hint::u_form_6_12_3;
	UNION128  CMP3Converter_hint::u_form_4_9;   // (IMDCT) mp3 formula (i,9)    form_4_9   [4]
	UNION128  CMP3Converter_hint::u_form_3_6;   // (IMDCT) mp3 formula (i,6)    form_3_6   [4]
	float * CMP3Converter_hint::form_18_36;     // (IMDCT) mp3 formula (i,36)   form_18_36 [18]
	float * CMP3Converter_hint::form_9_18;      // (IMDCT) mp3 formula (i,18)   form_9_18  [9]
	float * CMP3Converter_hint::form_6_12;      // (IMDCT) mp3 formula (i,12)   form_6_12  [7]
	float * CMP3Converter_hint::form_4_9;       // (IMDCT) mp3 formula (i,9)    form_4_9   [4]
	float * CMP3Converter_hint::form_3_6;       // (IMDCT) mp3 formula (i,6)    form_3_6   [4]
//	float CMP3Converter_hint::form_18_36 [18];  // (IMDCT) mp3 formula (i,36)	
//	float CMP3Converter_hint::form_9_18  [9];   // (IMDCT) mp3 formula (i,18)
//	float CMP3Converter_hint::form_6_12  [7];   // (IMDCT) mp3 formula (i,12)
//	float CMP3Converter_hint::form_4_9   [4];   // (IMDCT) mp3 formula (i,9)
//	float CMP3Converter_hint::form_3_6   [4];   // (IMDCT) mp3 formula (i,6)

	uint16     CMP3Converter_hint::freqInvIdx[MP3FREQINV];					// FreqInversion SSE/AVX/AVX-256F opt. table
	UNIONMAX   CMP3Converter_hint::freqInvMinus;							// FreqInversion SSE/AVX/AVX-256F opt.
	UNION512 & CMP3Converter_hint::freqInvMinus512 = freqInvMinus;			// FreqInversion SSE/AVX/AVX-256F opt.
	UNION256 & CMP3Converter_hint::freqInvMinus256 = freqInvMinus.u256[0];	// FreqInversion SSE/AVX/AVX-256F opt.
	UNION128 & CMP3Converter_hint::freqInvMinus128 = freqInvMinus.u128[0];	// FreqInversion SSE/AVX/AVX-256F opt.

	float CMP3Converter_hint::synthesis_tab [MP3SUBBANDS * 2][MP3SUBBANDS]; // Subband synthesis table

	UNION512 CMP3Converter_hint::u_form_16_32; // (Subband synthesis) mp3 formula (i,32)  form_16_32 [16];
	UNION256 CMP3Converter_hint::u_form_8_16;  // (Subband synthesis) mp3 formula (i,16)  form_8_16  [8];
	UNION128 CMP3Converter_hint::u_form_4_8;   // (Subband synthesis) mp3 formula (i,8)   form_4_8   [4];
	float * CMP3Converter_hint::form_16_32;    // (Subband synthesis) mp3 formula (i,32)  form_16_32 [16];
	float * CMP3Converter_hint::form_8_16;     // (Subband synthesis) mp3 formula (i,16)  form_8_16  [8];
	float * CMP3Converter_hint::form_4_8;      // (Subband synthesis) mp3 formula (i,8)   form_4_8   [4];
//	float CMP3Converter_hint::form_16_32 [16]; // (Subband synthesis) mp3 formula (i,32)  form_8_16  [8];
//	float CMP3Converter_hint::form_8_16  [8];  // (Subband synthesis) mp3 formula (i,16)
//	float CMP3Converter_hint::form_4_8   [4];  // (Subband synthesis) mp3 formula (i,8)
	float CMP3Converter_hint::form_2_4   [2];  // (Subband synthesis) mp3 formula (i,4)

	float CMP3Converter_hint::cosij[2][2]; // (Subband synthesis) table cos((2 * j + 1) * i * (PI / 4))

	bool CMP3Converter_hint::opt_SSE;           // SSE                  possible
	bool CMP3Converter_hint::opt_SSE_only;      // SSE                  possible and others (AVX, AVX-512F) not possible
	bool CMP3Converter_hint::opt_AVX;           //       AVX            possible
	bool CMP3Converter_hint::opt_AVX_only;      // SSE + AVX            possible and others (AVX-512F) not possible
	bool CMP3Converter_hint::opt_AVX512F;       //             AVX-512F possible
	bool CMP3Converter_hint::opt_AVX512F_only;  // SSE + AVX + AVX-512F possible and others (??? 2019 year, NONE others!) not possible
	bool CMP3Converter_hint::opt_AVX512F_combo; //       AVX + AVX-512F possible

	//>> Инициализация таблиц констант
	void CMP3Converter_hint::Init()
	{
		static bool isInit = false; // флаг готовности (генерируется единожды для всех экземпляров конвертора)

		if (!isInit)
		{
			c_sin.f[0] = c_sin0;
			c_sin.f[1] = c_sin1;
			c_sin.f[2] = c_sin2;
			c_sin.f[3] = c_sin3;

			//////////////////////////
			// Таблицы "Requantize" //
			//////////////////////////

			for (int i = 0; i < MP3MAXHUFF;  i++) pow43[i] = (float) pow(i, SAMPLE_POWER);
			for (int i = 0; i < POW2A_COUNT; i++) pow2A[i] = (float) pow(2, -0.5f * i); // [38]: 0, 0.5, 1.0, ..., 18.5, 19.0
			for (int i = 0; i < POW2B_COUNT; i++) pow2B[i] = (float) pow(2,  0.25f * ((i % 256) - 210 - (i / 256) * 8));

			//////////////////////////////
			// Таблицы "Process stereo" //
			//////////////////////////////

			for (int i = 0; i < MP3STEREOR-1; i++)
			{
				float ratio = tan((MathPI * i) / 12);
				ratio_r[i] = 1.f / (1.f + ratio);
				ratio_l[i] = ratio * ratio_r[i];
			}
			ratio_l[MP3STEREOR-1] = 1.f; // tan(PI/2)
			ratio_r[MP3STEREOR-1] = 0.f; // 

			for (int i = 0; i < _ELEMENTS(uSqrt2inv.f); i++)
				uSqrt2inv.f[i] = MathSqrt2inv;

			/////////////////////////
			// Таблицы "Antialias" //
			/////////////////////////

			const float ci[MP3ANTIALI] = { -0.6f, -0.535f, -0.33f, -0.185f, -0.095f, -0.041f, -0.0142f, -0.0037f };
			for (int i = 0; i < MP3ANTIALI; i++)
			{
				//cs[i] = 
					ucs.f[i] = 1.f / sqrt(1.f + ci[i] * ci[i]);
				//ca[i] = 
					uca.f[i] = ci[i] * ucs.f[i];

				ucs2.u256[1].f[i] = ucs.f[i]; // get x2 copy for AVX-512F opt.
				uca2.u256[1].f[i] = uca.f[i]; // .
			}

			////////////////////////////////
			// Таблицы "Hybrid synthesis" //
			////////////////////////////////

			IMDCT_sin[0] = u_IMDCT_sin[0].f;
			IMDCT_sin[1] = u_IMDCT_sin[1].f;
			IMDCT_sin[2] = u_IMDCT_sin[2].f;
			IMDCT_sin[3] = u_IMDCT_sin[3].f;

			const int i_03 = 0;
			const int i_13 = MP3LINES * 1 / 3;
			const int i_23 = MP3LINES * 2 / 3;
			const int i_33 = MP3LINES * 3 / 3;
			const int i_43 = MP3LINES * 4 / 3;
			const int i_53 = MP3LINES * 5 / 3;
			const int i_63 = MP3LINES * 6 / 3;

			// block_type == 0 (eMPEGBlockT_Reserved)      0-6

			for (int i = i_03; i<i_63; i++) IMDCT_sin[0][i] = sin(MathPI / 36 * (i + 0.5f));

			// block_type == 1 (eMPEGBlockT_StartBlock)    0-3, 3-4, 4-5, 5-6

			for (int i = i_03; i<i_33; i++) IMDCT_sin[1][i] = sin(MathPI / 36 * (i + 0.5f));
			for (int i = i_33; i<i_43; i++) IMDCT_sin[1][i] = 1;
			for (int i = i_43; i<i_53; i++) IMDCT_sin[1][i] = sin(MathPI / 12 * (i + 0.5f - 18));
			for (int i = i_53; i<i_63; i++) IMDCT_sin[1][i] = 0;

			// block_type == 2 (eMPEGBlockT_3ShortWindow)  0-2, 2-6

			for (int i = i_03; i<i_23; i++) IMDCT_sin[2][i] = sin(MathPI / 12 * (i + 0.5f));
			for (int i = i_23; i<i_63; i++) IMDCT_sin[2][i] = 0;

			// block_type == 3 (eMPEGBlockT_EndBlock)      0-1, 1-2, 2-3, 3-6

			for (int i = i_03; i<i_13; i++) IMDCT_sin[3][i] = 0;
			for (int i = i_13; i<i_23; i++) IMDCT_sin[3][i] = sin(MathPI / 12 * (i + 0.5f - 6));
			for (int i = i_23; i<i_33; i++) IMDCT_sin[3][i] = 1;
			for (int i = i_33; i<i_63; i++) IMDCT_sin[3][i] = sin(MathPI / 36 * (i + 0.5f));

			// IMDCT cos tables

			for (int i = 0; i < COS1TAB_COUNT; i++)
				IMDCT_cos1tab[i] = cos(MathPI / (12 * 2) * (2 * (i % 12) + ((12 / 2) + 1)) * (2 * (i / 12) + 1));

			for (int i = 0; i < COS2TAB_COUNT; i++)
				IMDCT_cos2tab[i] = cos(MathPI / (36 * 2) * (2 * (i % 36) + ((36 / 2) + 1)) * (2 * (i / 36) + 1));

			// MP3 formula constants

			form_18_36 = u_form_18_36.f;
			form_9_18  = u_form_9_18.f;
			form_6_12  = u_form_6_12.f;
			form_4_9   = u_form_4_9.f;
			form_3_6   = u_form_3_6.f;
			for (int i = 0; i < 18; i++) form_18_36 [i] = MP3FORMULA(i, 36.f);
			for (int i = 0; i < 9;  i++) form_9_18  [i] = MP3FORMULA(i, 18.f);
		    for (int i = 0; i < 6;  i++) form_6_12  [i] = MP3FORMULA(i, 12.f);  form_6_12 [6] = 0.f;
			for (int i = 0; i < 4;  i++) form_4_9   [i] = MP3FORMULA(i,  9.f);	
			for (int i = 0; i < 3;  i++) form_3_6   [i] = MP3FORMULA(i,  6.f);  form_3_6  [3] = 0.f;			
			u_form_6_12_3.f[0] = u_form_6_12.f[3];
			u_form_6_12_3.f[1] = u_form_6_12.f[4];
			u_form_6_12_3.f[2] = u_form_6_12.f[5];
			u_form_6_12_3.f[3] = 0.f;

			/////////////////////////////
			// Таблицы "FreqInversion" //
			/////////////////////////////

			// 144 = 4 x36 (SSE) or 8 x18 (AVX) or 16 x9 (AVX-512F)
			for (uint16 i=0, subband = 1; subband < MP3SUBBANDS; subband += 2) // (16) 1,3,5,7__9,11,13,15__17,19,21,23__25,27,29,31
			{
				uint16 level = subband * MP3LINES;
				for (uint16 line = 1; line < MP3LINES; line += 2) // (9) 1,3,5,7__9,11,13,15__17
				{
					freqInvIdx[i++] = level + line;
				}
			}

			for (int i = 0; i < _ELEMENTS(freqInvMinus.f); i++)
				freqInvMinus.f[i] = -1.f;	

			/////////////////////////////////
			// Таблицы "Subband synthesis" //
			/////////////////////////////////

			for (int i = 0; i < MP3SUBBANDS * 2; i++)
			for (int j = 0; j < MP3SUBBANDS;     j++)
				synthesis_tab[i][j] = cos((MathPI / (MP3SUBBANDS * 2)) * (16.f + i) * (2.f * j + 1));

			// MP3 formula constants

			form_16_32 = u_form_16_32.f;
			form_8_16  = u_form_8_16.f;
			form_4_8   = u_form_4_8.f;
			for (int i = 0; i < 16; i++) form_16_32 [i] = MP3FORMULA(i, 32.f);
			for (int i = 0; i < 8;  i++) form_8_16  [i] = MP3FORMULA(i, 16.f);
			for (int i = 0; i < 4;  i++) form_4_8   [i] = MP3FORMULA(i,  8.f);
			for (int i = 0; i < 2;  i++) form_2_4   [i] = MP3FORMULA(i,  4.f);		

			// cos table

			for (int i = 0; i < 2;  i++)
			for (int j = 0; j < 2;  j++)
				cosij[i][j] = cos((2 * j + 1) * i * (MathPI / 4));

			////////////////////////////////////////////
			// Выяснение возможностей для оптимизации //
			////////////////////////////////////////////

			opt_SSE           = CSystem::GetCap(eCPUCap_SSE);
			opt_AVX           = CSystem::GetCap(eCPUCap_AVX);
			opt_AVX512F       = CSystem::GetCap(eCPUCap_AVX512F);
			opt_SSE_only      = opt_SSE && !opt_AVX && !opt_AVX512F;
			opt_AVX_only      = opt_SSE &&  opt_AVX && !opt_AVX512F;
			opt_AVX512F_only  = opt_SSE &&  opt_AVX &&  opt_AVX512F;
			opt_AVX512F_combo =             opt_AVX &&  opt_AVX512F;

			isInit = true;
		}
	}
}

#undef MP3FORMULA
#undef SAMPLE_POWER
#undef POW2A_COUNT
#undef POW2B_COUNT
#undef COS1TAB_COUNT
#undef COS2TAB_COUNT