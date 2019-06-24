// ----------------------------------------------------------------------- //
//
// MODULE  : helper.h
//
// PURPOSE : Вспомогательные классы и функции
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _BASE_HELPER_H
#define _BASE_HELPER_H

#include "defines.h"
#include "types.h"

#include "vecdata.h"
#include "thread.h"
#include "system.h"

#define UNIONMAX UNION512 // default UNION### max. size (AVX-512 for now)

namespace SAVFGAME
{
	template <bool...>
		struct check_template_types ;

	template <>
	struct check_template_types <>
		: std::true_type {};

	template <bool... T_types>
	struct check_template_types <false, T_types...>
		: std::false_type {};

	template <bool... T_types>
	struct check_template_types <true,  T_types...>
		: check_template_types <T_types...> {};

	#define CHECK_T_TYPES_IS_SAME(T_type, valid_type) check_template_types< std::is_same< std::decay_t<T_type>, valid_type >::value...>::value
	#define CHECK_T_TYPES_IS_SAME_2(T_type, valid_type_1, valid_type_2) \
		(CHECK_T_TYPES_IS_SAME(T_type, valid_type_1)) || (CHECK_T_TYPES_IS_SAME(T_type, valid_type_2))

	//>> integer 32-bit auto bitmasks union
	//>> sizeof(UNION32) = 4 bytes
	union UNION32
	{
		uint32 u32;		// 0x FFFFFFFF
		uint16 u16[2];	// 0x FFFF[1] FFFF[0]
		uint8  u8[4];	// 0x FF[3] FF[2] FF[1] FF[0]

		struct
		{
			uint16 u16_1; // 0000FFFF : u16[0]
			uint16 u16_2; // FFFF0000 : u16[1]
		};
		struct
		{
			uint8 u8_1; // 000000FF : u8[0]
			uint8 u8_2; // 0000FF00 : u8[1]
			uint8 u8_3; // 00FF0000 : u8[2]
			uint8 u8_4; // FF000000 : u8[3]
		};
		struct
		{
			unsigned u4_1 : 4; // 0000000F : BIT  0 ..  3 (0..31) / BIT  1 ..  4 (1..32)
			unsigned u4_2 : 4; // 000000F0 : BIT  4 ..  7 (0..31) / BIT  5 ..  8 (1..32)
			unsigned u4_3 : 4; // 00000F00 : BIT  8 .. 11 (0..31) / BIT  9 .. 12 (1..32)
			unsigned u4_4 : 4; // 0000F000 : BIT 12 .. 15 (0..31) / BIT 13 .. 16 (1..32)
			unsigned u4_5 : 4; // 000F0000 : BIT 16 .. 19 (0..31) / BIT 17 .. 20 (1..32)
			unsigned u4_6 : 4; // 00F00000 : BIT 20 .. 23 (0..31) / BIT 21 .. 24 (1..32)
			unsigned u4_7 : 4; // 0F000000 : BIT 24 .. 27 (0..31) / BIT 25 .. 28 (1..32)
			unsigned u4_8 : 4; // F0000000 : BIT 28 .. 31 (0..31) / BIT 29 .. 32 (1..32)
		};
		struct
		{
			unsigned u1_1_1 : 1; // 0000000F : 0001 _ BIT 0 (0..31) _ BIT 1 (1..32)
			unsigned u1_1_2 : 1; // 0000000F : 0010 _ BIT 1 (0..31) _ BIT 2 (1..32)
			unsigned u1_1_3 : 1; // 0000000F : 0100 _ BIT 2 (0..31) _ BIT 3 (1..32)
			unsigned u1_1_4 : 1; // 0000000F : 1000 _ BIT 3 (0..31) _ BIT 4 (1..32)

			unsigned u1_2_1 : 1; // 000000F0 : 0001 _ BIT 4 (0..31) _ BIT 5 (1..32)
			unsigned u1_2_2 : 1; // 000000F0 : 0010 _ BIT 5 (0..31) _ BIT 6 (1..32)
			unsigned u1_2_3 : 1; // 000000F0 : 0100 _ BIT 6 (0..31) _ BIT 7 (1..32)
			unsigned u1_2_4 : 1; // 000000F0 : 1000 _ BIT 7 (0..31) _ BIT 8 (1..32)

			unsigned u1_3_1 : 1; // 00000F00 : 0001 _ BIT 8 (0..31) _ BIT 9 (1..32)
			unsigned u1_3_2 : 1; // 00000F00 : 0010 _ BIT 9 (0..31) _ BIT 10 (1..32)
			unsigned u1_3_3 : 1; // 00000F00 : 0100 _ BIT 10 (0..31) _ BIT 11 (1..32)
			unsigned u1_3_4 : 1; // 00000F00 : 1000 _ BIT 11 (0..31) _ BIT 12 (1..32)
			
			unsigned u1_4_1 : 1; // 0000F000 : 0001 _ BIT 12 (0..31) _ BIT 13 (1..32)
			unsigned u1_4_2 : 1; // 0000F000 : 0010 _ BIT 13 (0..31) _ BIT 14 (1..32)
			unsigned u1_4_3 : 1; // 0000F000 : 0100 _ BIT 14 (0..31) _ BIT 15 (1..32)
			unsigned u1_4_4 : 1; // 0000F000 : 1000 _ BIT 15 (0..31) _ BIT 16 (1..32)

			unsigned u1_5_1 : 1; // 000F0000 : 0001 _ BIT 16 (0..31) _ BIT 17 (1..32)
			unsigned u1_5_2 : 1; // 000F0000 : 0010 _ BIT 17 (0..31) _ BIT 18 (1..32)
			unsigned u1_5_3 : 1; // 000F0000 : 0100 _ BIT 18 (0..31) _ BIT 19 (1..32)
			unsigned u1_5_4 : 1; // 000F0000 : 1000 _ BIT 19 (0..31) _ BIT 20 (1..32)

			unsigned u1_6_1 : 1; // 00F00000 : 0001 _ BIT 20 (0..31) _ BIT 21 (1..32)
			unsigned u1_6_2 : 1; // 00F00000 : 0010 _ BIT 21 (0..31) _ BIT 22 (1..32)
			unsigned u1_6_3 : 1; // 00F00000 : 0100 _ BIT 22 (0..31) _ BIT 23 (1..32)
			unsigned u1_6_4 : 1; // 00F00000 : 1000 _ BIT 23 (0..31) _ BIT 24 (1..32)

			unsigned u1_7_1 : 1; // 0F000000 : 0001 _ BIT 24 (0..31) _ BIT 25 (1..32)
			unsigned u1_7_2 : 1; // 0F000000 : 0010 _ BIT 25 (0..31) _ BIT 26 (1..32)
			unsigned u1_7_3 : 1; // 0F000000 : 0100 _ BIT 26 (0..31) _ BIT 27 (1..32)
			unsigned u1_7_4 : 1; // 0F000000 : 1000 _ BIT 27 (0..31) _ BIT 28 (1..32)

			unsigned u1_8_1 : 1; // F0000000 : 0001 _ BIT 28 (0..31) _ BIT 29 (1..32)
			unsigned u1_8_2 : 1; // F0000000 : 0010 _ BIT 29 (0..31) _ BIT 30 (1..32)
			unsigned u1_8_3 : 1; // F0000000 : 0100 _ BIT 30 (0..31) _ BIT 31 (1..32)
			unsigned u1_8_4 : 1; // F0000000 : 1000 _ BIT 31 (0..31) _ BIT 32 (1..32)
		};
	};

	//>> __m128 union
	//>> sizeof(UNION128) = 16 bytes
	union UNION128
	{
		UNION32  u32[4];
		float      f[4];
		double     d[2];
#ifdef ENGINE_SSE
		__m128     m128; // 4 float
#endif
#ifdef ENGINE_SSE2
		__m128d   m128d; // 2 double
		__m128i   m128i; // 4 integer
#endif
	};

	//>> __m256 union
	//>> sizeof(UNION256) = 32 bytes
	union UNION256
	{
///////////////////////////////////////////  UNION128 x 2
		UNION32  u32[8]; // .
		float      f[8]; // .
		double     d[4]; // .
#ifdef ENGINE_SSE
		__m128     m128[2]; // 4 float x2
#endif
#ifdef ENGINE_SSE2
		__m128d   m128d[2]; // 2 double x2
		__m128i   m128i[2]; // 4 integer x2
#endif
/////////////////////////////////////////// end
		UNION128 u128[2]; // .
#ifdef ENGINE_AVX
		__m256   m256;  // 8 float
		__m256d  m256d; // 4 double
		__m256i  m256i; // 8 integer
#endif
	};

	//>> __m512 union
	//>> sizeof(UNION512) = 64 bytes
	union UNION512
	{
///////////////////////////////////////////  UNION128 x 4
		UNION32  u32[16]; // .
		float      f[16]; // .
		double     d[8]; // .
#ifdef ENGINE_SSE
		__m128     m128[4]; // 4 float x4
#endif
#ifdef ENGINE_SSE2
		__m128d   m128d[4]; // 2 double x4
		__m128i   m128i[4]; // 4 integer x4
#endif
/////////////////////////////////////////// UNION256 x 2
		UNION128 u128[4]; // .
#ifdef ENGINE_AVX
		__m256   m256[2];  // 8 float x2
		__m256d  m256d[2]; // 4 double x2
		__m256i  m256i[2]; // 8 integer x2
#endif
/////////////////////////////////////////// end
		UNION256 u256[2]; // .
#ifdef ENGINE_AVX512F
		__m512   m512;  // 16 float
		__m512d  m512d; // 8  double
		__m512i  m512i; // 16 integer
#endif
	};

	//>> __m1024 union
	//>> sizeof(UNION1024) = 128 bytes
	union UNION1024
	{
///////////////////////////////////////////  UNION128 x 8
		UNION32  u32[32]; // .
		float      f[32]; // .
		double     d[16]; // .
#ifdef ENGINE_SSE
		__m128     m128[8]; // 4 float x8
#endif
#ifdef ENGINE_SSE2
		__m128d   m128d[8]; // 2 double x8
		__m128i   m128i[8]; // 4 integer x8
#endif
/////////////////////////////////////////// UNION256 x 4
		UNION128 u128[8]; // .
#ifdef ENGINE_AVX
		__m256   m256[4];  // 8 float x4
		__m256d  m256d[4]; // 4 double x4
		__m256i  m256i[4]; // 8 integer x4
#endif
/////////////////////////////////////////// UNION512 x 2
		UNION256 u256[4]; // .
#ifdef ENGINE_AVX512F
		__m512   m512[2];  // 16 float x2
		__m512d  m512d[2]; // 8  double x2
		__m512i  m512i[2]; // 16 integer x2
#endif
/////////////////////////////////////////// end
		UNION512 u512[2]; // .
#ifdef ENGINE_WAITING_IN_FUTURE_1024
		__m1024   m1024;  // 32 float
		__m1024d  m1024d; // 16 double
		__m1024i  m1024i; // 32 integer
#endif
	};

	//>> __m2048 union
	//>> sizeof(UNION2048) = 256 bytes
	union UNION2048
	{
///////////////////////////////////////////  UNION128 x 16
		UNION32  u32[64]; // .
		float      f[64]; // .
		double     d[32]; // .
#ifdef ENGINE_SSE
		__m128     m128[16]; // 4 float x16
#endif
#ifdef ENGINE_SSE2
		__m128d   m128d[16]; // 2 double x16
		__m128i   m128i[16]; // 4 integer x16
#endif
/////////////////////////////////////////// UNION256 x 8
		UNION128 u128[16]; // .
#ifdef ENGINE_AVX
		__m256   m256[8];  // 8 float x8
		__m256d  m256d[8]; // 4 double x8
		__m256i  m256i[8]; // 8 integer x8
#endif
/////////////////////////////////////////// UNION512 x 4
		UNION256 u256[8]; // .
#ifdef ENGINE_AVX512F
		__m512   m512[4];  // 16 float x4
		__m512d  m512d[4]; // 8  double x4
		__m512i  m512i[4]; // 16 integer x4
#endif
/////////////////////////////////////////// UNION1024 x 2
		UNION512 u512[4]; // .
#ifdef ENGINE_WAITING_IN_FUTURE_1024
		__m1024   m1024[2];  // 32 float x2
		__m1024d  m1024d[2]; // 16 double x2
		__m1024i  m1024i[2]; // 32 integer x2
#endif
/////////////////////////////////////////// end
		UNION1024 u1024[2]; // .
#ifdef ENGINE_WAITING_IN_FUTURE_2048
		__m2048   m2048;  // 64 float
		__m2048d  m2048d; // 32 double
		__m2048i  m2048i; // 64 integer
#endif
	};

	//>> UINT32 with INT32 & FLOAT union + UNION32
	union UFLOAT
	{
		UFLOAT()                   { u32 = 0;   }
		UFLOAT(uint32 src)         { u32 = src; }

		operator uint32 & () { return u32; }

		uint32 u32;
		int32  i32;
		float  f32;

		UNION32 U;
	};

	//>> UINT64 with INT64 & DOUBLE union
	union UDOUBLE
	{
		UDOUBLE()                    { u64 = 0; }
		UDOUBLE(uint64 src)          { u64 = src; }

		operator uint64 & () { return u64; }
		operator UFLOAT & () { return U_1; }

		struct { uint32 u32_1;
			     uint32 u32_2; };
		struct {  int32 i32_1;
			      int32 i32_2; };
		struct {  float f32_1;
			      float f32_2; };
		struct { UFLOAT  U_1;
			     UFLOAT  U_2;  };

		uint64 u64;
		 int64 i64;
		double f64;
	};

	/////////////////////////////////////////////////////////////////////

	// 64 бита комплексной идентификации
	struct uint64_cid
	{
		uint64_cid            (const uint64 &  src) { _u64 = src; }
		uint64_cid            (      uint64 && src) { _u64 = src; }
		uint64_cid & operator=(const uint64 &  src) { _u64 = src; return *this; }
		uint64_cid & operator=(      uint64 && src) { _u64 = src; return *this; }

		operator uint64() { return _u64; }

		uint64_cid() { _u64 = 0; }

		union
		{
			uint64 _u64;
			struct
			{
				unsigned e : 32; // 00000000FFFFFFFF uint32 element id
				unsigned g : 32; // FFFFFFFF00000000 uint32 group id			
			} ge;
			struct
			{
				unsigned e : 32; // 00000000FFFFFFFF uint32 element id
				unsigned s : 16; // 0000FFFF00000000 uint16 subgroup id
				unsigned g : 16; // FFFF000000000000 uint16 group id
			} gse;
			struct
			{
				unsigned e : 16; // 000000000000FFFF uint16 element id
				unsigned c : 16; // 00000000FFFF0000 uint16 complex id
				unsigned s : 16; // 0000FFFF00000000 uint16 subgroup id
				unsigned g : 16; // FFFF000000000000 uint16 group id
			} gsce;
		};
	};

	// 64 бита групповой идентификации
	struct uint64_gid
	{
		uint64_gid           (const uint64 &  src) { _u64 = src; }
		uint64_gid           (      uint64 && src) { _u64 = src; }
		uint64_gid& operator=(const uint64 &  src) { _u64 = src; return *this; }
		uint64_gid& operator=(      uint64 && src) { _u64 = src; return *this; }

		operator uint64() { return _u64; }

		uint64_gid() { _u64 = 0; }
	
		union
		{
			uint64 _u64;
			struct
			{
				uint32 element; // 00000000FFFFFFFF
				uint32 group;   // FFFFFFFF00000000
			};
		};
	};

	/////////////////////////////////////////////////////////////////////

	template <class T, typename SZ>
	struct TBUFFER_PROTO // Прототип-интерфейс оболочки для содержания простых буферов
	{
		TBUFFER_PROTO() : buf(nullptr), count(0) {};
		virtual ~TBUFFER_PROTO() { }

		virtual bool Create(SZ num) = 0;
		virtual void Close() = 0;

		T *	buf;	// буфер данных
		SZ	count;	// количество данных
	};

	template <class T, typename SZ>
	struct TBUFFER : public TBUFFER_PROTO <T, SZ> // Оболочка для содержания простых буферов
	{
		TBUFFER() : TBUFFER_PROTO() {};
		~TBUFFER() override { Close(); }	
		
		//>> Проверка <false> если (buf == nullptr) иначе <true>
		bool Check()
		{
			if (buf == nullptr)
				return false;
			return true;
		}
		
		//>> Стирает старый и создаёт новый буфер в num элементов :: <false> если не удалось выделить память, иначе <true>
		bool Create(SZ num) override
		{
			Close();	
			buf = new /*_NEW_NOTHROW*/ T [num];
			if (buf != nullptr)
			{
				count = num;
				return true;
			}
			else
				return false;
		}
		
		//>> Очистка в ноль  - memset(NULL)
		void Clear()
		{
			if (buf)
			ZeroMemory(buf, sizeof(T)*count);
		}
		
		//>> Очистка в <val> - memset(val)
		void Clear(byte val)
		{
			if (buf)
			memset(buf, val, sizeof(T)*count);
		}
		
		//>> Удаляет всё
		void Close() override
		{
			_DELETEN(buf);
			count = 0;
		}
		
		T& operator[](size_t idx) { return buf[idx]; }
		T* operator()()           { return buf; }
	};

	template <class T, typename SZ>
	struct TBUFFERNEXT : public TBUFFER < T, SZ >
	{
		TBUFFERNEXT() : TBUFFER() {};
		~TBUFFERNEXT() override { };

		SZ cur;
		SZ prev;

		void SetNext(const T & value)
		{
			if (count < 2) { _MBM(ERROR_BUFLIMIT); return; }

			SZ current = cur + 1;
			if (current >= count)
				current = 0;

			prev = cur;
			cur  = current;

			buf[current] = value;
		}
		void Reset(const T & value)
		{
			for (SZ i=0; i<count; i++)
				buf[i] = value;
		}
		
		bool Equal()   { return ( buf[cur] == buf[prev] ); }
		T & Current()  { return buf[cur];  }
		T & Previous() { return buf[prev]; }
	};

	template <class T, typename SZ>
	struct TBUFFERS : public TBUFFER <T, SZ> // Оболочка для содержания простых буферов (версия со связанным счетчиком)
	{
		TBUFFERS() : TBUFFER(), owners(nullptr) {};
		~TBUFFERS() override { Close(); }
		
		//>> Стирает старый и создаёт новый буфер в num элементов :: <false> если не удалось выделить память, иначе <true>
		bool Create(SZ num) override
		{
			Close();

			buf = new /*_NEW_NOTHROW*/ T [num];

			if (buf != nullptr)
			{
				owners = new /*_NEW_NOTHROW*/ uint32;
				if (owners != nullptr)
				{
					count = num;
					(*owners) = 1;
					return true;
				}
				else
				{
					_DELETEN(buf);
					return false;
				}
			}
			else
				return false;
		}

		//>> Удаляет всё
		void Close() override
		{
			if (owners)
			{
				uint32 & num_owners = *owners;

				if (num_owners < 2)
				{
					_DELETE(owners);
					_DELETEN(buf);
					count = 0;
				}
				else
				{
					num_owners--;

					owners = nullptr;
					buf    = nullptr;
					count  = 0;
				}
			}
			else // нет данных по владельцам
			{
				_DELETEN(buf);
				count = 0;
			}
		}
		
		//>> Связывает другого с данным на общее владение
		bool Link(TBUFFERS & other)
		{
			if (buf == nullptr) return false; // NONE

			if (owners == nullptr)
			{
				owners = new /*_NEW_NOTHROW*/ uint32;

				if (owners == nullptr) // malloc error
					return false;      // .

				(*owners) = 1;
			}

			other.Close(); // сброс другого

			other.buf    = buf;
			other.count  = count;
			other.owners = owners;

			uint32 & num_owners = *owners;

			if  (num_owners < 2)
				 num_owners = 2;
			else num_owners++;

			return true;
		}
		
		//>> Сброс старого и установка нового, включая внутренний счетчик владельцев
		bool Reset(T * _buf, SZ _count)
		{
			Close();

			{
				owners = new /*_NEW_NOTHROW*/ uint32;

				if (owners == nullptr) // malloc error
					return false;      // .

				(*owners) = 1;
			}

			buf   = _buf;
			count = _count;

			return true;
		}
		
	private:
		uint32 * owners; // кол-во владельцев (других TBUFFERS)
	};

	/////////////////////////////////////////////////////////////////////

	// DELETED : use std's as in example comparing floats
	// std::vector<std::pair<float, uint64>> map;
	// std::sort(map.begin(), map.end(), std::less_equal<>());
	// OR
	// std::map<float, uint64> map;
	// map.insert(std::pair<float, uint64>(...));

/*	template < typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type >
	//>> type T should be arithmetic, overwise it will not compile
	struct sortable_data
	{
		struct DATAKEY
		{
			     T d;
			uint64 k;
		};
		struct IDKEY
		{
			uint64 id;
			uint64 k;
		};

		sortable_data() : _size(0) {};
		~sortable_data(){};
		void create(uint64 capacity)
		{
			_data.Create(capacity);
			_ID.Close();
			_size = 0;
		}
		uint64 capacity()
		{
			return _data.count;
		}
		uint64 size()
		{
			return _size;
		}
		template <typename _Ty, typename _Key>
		bool put(_Ty && data, _Key && key)
		{
			static_assert (
				!std::is_same<_Ty, T>::value,
				"sortable_data : Type _Ty should be same as T"
			);
			static_assert (
				!std::is_same<_Key, uint64>::value,
				"sortable_data : Type _Key should be uint64"
			);

			if (_size == _data.count) return false;
			_data[_size].d = std::forward<_Ty>(data);
			_data[_size].k = std::forward<_Key>(key);
			_size++;
			return true;
		}
		void sort(bool from_min_to_max)
		{
			TBUFFER <uint64, uint64> _XX;
			_ID.Create(_size);
			_XX.Create(_size);
			_XX.Clear();

			if (from_min_to_max)					{
				for(uint64 i=0;   i<_size; i++)
				for(uint64 n=i+1; n<_size; n++)
					if (_data[i].d > _data[n].d)
						_XX[i]++; else _XX[n]++;	}
			else									{
				for(uint64 i=0;   i<_size; i++)
				for(uint64 n=i+1; n<_size; n++)
					if (_data[i].d < _data[n].d)
						_XX[i]++; else _XX[n]++;	}

			for(uint64 i=0; i<_size; i++)
			{ 
				_ID[_XX[i]].id = i;				// so _ID[0].id  will send us to _data[id] which is first (min or max)
				_ID[_XX[i]].k  = _data[i].k;	// copy keys from <_data> in correct order for quick grabbing
			}
		}
		bool is_sorted()
		{
			if (!_ID.count) return false;
			return (_size == _ID.count);
		}

		     T& operator[](uint64 get_data_at_sorted_id) { return _data[_ID[get_data_at_sorted_id].id].d; }
		uint64& operator()(uint64 get_key_at_sorted_id)  { return       _ID[get_key_at_sorted_id].k;      }

	private:
		TBUFFER <DATAKEY, uint64> _data;	// RAW original
		TBUFFER <IDKEY,   uint64> _ID;		// sorted id / keys
		uint64                    _size;	// count of presented RAW blocks
	}; //*/

	///////////////////////////////////////////////////////

	template <class CHOLD, class CFRIEND> 
	struct THOLDER							// CFRIEND - источник CHOLD, метод Set(), прочие могут только Get()
	{
		struct
		{
			friend struct THOLDER;
		private:
			CHOLD var;
		} _;
		friend CFRIEND;
	private:
		void Set(CHOLD hold_var) { _.var = hold_var; }
	public:
		const CHOLD Get() { return _.var; }
	};

	///////////////////////////////////////////////////////

	//>> int32 to limited float
	inline void I2F (UFLOAT & value)
	{
		const float LIMIT = 1.f / 0x7FFFFFFF;

		value.f32 = _CLAMP((LIMIT * value.i32), -1.f, +1.f);
	}

	//>> limited float to int32
	inline void F2I (UFLOAT & value)
	{
		const int32 LIMIT = 0x7FFFFFFF;

		value.i32 = (int32)(_CLAMP(value.f32, -1.f, +1.f) * LIMIT);
	}

	///////////////////////////////////////////////////////

	//>> Читает следующие bits_to_read биты c позиции байта position_bit {01234567} в uint64
	inline uint64 ReadBits(byte *& ptr, byte & position_bit, byte bits_to_read)
	{
		if (!bits_to_read)
			return 0;

		if (bits_to_read > 64) // you want too much
			bits_to_read = 64;

		if (position_bit > 7) // shift
		{
			ptr += position_bit >> 3;
			position_bit %= 8;
		}
		
		uint64 RET_VALUE = 0;
		byte BITS_DONE = 0;
		byte BITS_FORWARD = 8 - position_bit;
	
		while (BITS_DONE < bits_to_read)
		{
			byte BITS_NOT_DONE = bits_to_read - BITS_DONE;
			byte BITS_ROUND;

			if (BITS_FORWARD > BITS_NOT_DONE)
				BITS_ROUND = BITS_NOT_DONE;
			else
				BITS_ROUND = BITS_FORWARD;

			RET_VALUE <<= BITS_ROUND;
			RET_VALUE |= ((*ptr) >> (BITS_FORWARD - BITS_ROUND)) & ((1 << BITS_ROUND) - 1);

			BITS_FORWARD -= BITS_ROUND;
			BITS_DONE    += BITS_ROUND;
			position_bit += BITS_ROUND;

			if (!BITS_FORWARD)		{
				BITS_FORWARD = 8;
				position_bit = 0;
				ptr++;				}
		}

		return RET_VALUE;
	}

	template <byte JUMPS>
	//>> Читает следующие bits_to_read биты c позиции байта position_bit {01234567} в uint64 (мод. с прыжками)
	inline uint64 ReadBits(byte *& ptr, byte & position_bit, byte bits_to_read, byte ** jump)
	{
		if (!bits_to_read)
			return 0;

		if (bits_to_read > 64) // you want too much
			bits_to_read = 64;

		if (position_bit > 7) // shift
		{
			ptr += position_bit >> 3;
			position_bit %= 8;
		}
		
		uint64 RET_VALUE = 0;
		byte BITS_DONE = 0;
		byte BITS_FORWARD = 8 - position_bit;
	
		while (BITS_DONE < bits_to_read)
		{
			byte BITS_NOT_DONE = bits_to_read - BITS_DONE;
			byte BITS_ROUND;

			if (BITS_FORWARD > BITS_NOT_DONE)
				BITS_ROUND = BITS_NOT_DONE;
			else
				BITS_ROUND = BITS_FORWARD;

			RET_VALUE <<= BITS_ROUND;
			RET_VALUE |= ((*ptr) >> (BITS_FORWARD - BITS_ROUND)) & ((1 << BITS_ROUND) - 1);

			BITS_FORWARD -= BITS_ROUND;
			BITS_DONE    += BITS_ROUND;
			position_bit += BITS_ROUND;

			if (!BITS_FORWARD)		{
				BITS_FORWARD = 8;
				position_bit = 0;
				ptr++;

				for (uint32 jp=0; jp<JUMPS; jp+=2) // 0-1, 2-3, ...
				if (ptr == jump[jp])
				{
					ptr =  jump[jp+1];
					break;
				}
			}
		}

		return RET_VALUE;
	}

	template <uint16 MAXPOS>
	//>> Читает следующие bits_to_read биты c позиции байта position_bit {01234567} в uint64 (мод. с MAXPOS)
	inline uint64 ReadBits(byte * ptr, uint16 & position_bit, byte bits_to_read)
	{
		if (!bits_to_read)
			return 0;

		if (bits_to_read > 64) // you want too much
			bits_to_read = 64;

		uint64 RET_VALUE = 0;
		byte BITS_DONE = 0;
		byte BITS_FORWARD = 8 - (position_bit & 7);
		byte * DATA = ptr + ((position_bit >> 3) % MAXPOS);

		while (BITS_DONE < bits_to_read)
		{
			byte BITS_NOT_DONE = bits_to_read - BITS_DONE;
			byte BITS_ROUND;

			if (BITS_FORWARD > BITS_NOT_DONE)
				BITS_ROUND = BITS_NOT_DONE;
			else
				BITS_ROUND = BITS_FORWARD;

			RET_VALUE <<= BITS_ROUND;
			RET_VALUE |= ((*DATA) >> (BITS_FORWARD - BITS_ROUND)) & ((1 << BITS_ROUND) - 1);

			BITS_FORWARD -= BITS_ROUND;
			BITS_DONE    += BITS_ROUND;
			position_bit += BITS_ROUND;

			if (!BITS_FORWARD)				{
				BITS_FORWARD = 8;
				DATA++;
				
				if (position_bit == (MAXPOS << 3))
				{
					DATA = ptr;
					//position_bit = 0;
				}							
			}
		}

		return RET_VALUE;
	};

	///////////////////////////////////////////////////////

/*	//>> Проверка доступности файла по одному из путей
	inline wchar_t* LoadFileCheck(uint8 num, wchar_t* p, wchar_t* ...)
	{
		wchar_t* ret = nullptr;
		wchar_t** pp = &p;
		for (uint8 i=0; i<num; i++)
		{
			FILE *fp = nullptr;
			      fp = _wfsopen(*pp, L"rb", _SH_DENYNO);
			if (fp) {fclose(fp); ret=*pp; break;}
			pp++;
		}
		return ret;
	} //*/

	template<typename... T64>
	//>> Проверка доступности файла по одному из путей
	inline wchar_t* LoadFileCheck64(T64&&... args)
	{
		static_assert (
			CHECK_T_TYPES_IS_SAME(T64, int64),
			"LoadFileCheck64() : All values must be int64 type"
		);

		std::vector< int64 > && argvec = initializer_list < int64 > { args... };
		for (auto & cur : argvec)
		{
			FILE *fp = _wfsopen(reinterpret_cast<wchar_t*>(cur), L"rb", _SH_DENYNO);
			if (fp) { fclose(fp); return reinterpret_cast<wchar_t*>(cur); }
		}
		return nullptr;
	}

	//>> Проверка доступности файла по одному из путей
	inline wchar_t* LoadFileCheck(const initializer_list < wchar_t* > & args)
	{
		for (auto & cur : args)
		{
			FILE *fp = _wfsopen(cur, L"rb", _SH_DENYNO);
			if (fp) { fclose(fp); return cur; }
		}
		return nullptr;
	}

	template<typename T1, typename T2>
	//>> Вспомогательная функция сверки 2х массивов	:: возвращает <true> если одинаковые
	inline bool Compare(const T1* array1, const T2* array2, const uint64 size)
	{
		static_assert (
			sizeof(T1) == sizeof(T2),
			"Compare<T1,T2>() : Types T1,T2 are not size-identical"
		);

		for (uint64 i=0;; array1++, array2++)	{
			if (*array1 ^ *array2) return 0;
			if (!(++i^size))       break;		}
		return 1;
	}

	//>> Взятие пути без имени файла
	inline std::wstring GetPrenamePath(const wchar_t* source)
	{
		std::wstring path(source);
		size_t last = path.find_last_of(L"/\\");
		return path.substr(0, last);
	}

	//>> Взятие имени файла из пути
	inline std::wstring GetFilename(const wchar_t* source)
	{
		std::wstring path(source);
		size_t last = path.find_last_of(L"/\\");
		return path.substr(last+1, path.size());
	}

	template <typename SZ>
	//>> Копирование файла с HDD в RAM
	inline bool ReadFileInMemory(const wchar_t* in_filepath, TBUFFER_PROTO <byte, SZ> & out, bool show_error)
	{
		static_assert (
			std::is_integral<SZ>::value,
			"ReadFileInMemory<SZ>() : Type SZ should be integral"
		);

		wchar_t error[MAX_PATH * 2];
		out.Close();

		wsprintf(error, L"%s %s", ERROR_OpenFile, in_filepath);
		FILE *fp = _wfsopen(in_filepath, L"rb", _SH_DENYNO);		if (!fp) { if (show_error) _MBM(error); return false; }

		out.Create(static_cast<SZ>(_filelengthi64(_fileno(fp))));
		fread(out.buf, sizeof(byte), out.count, fp);
		fclose(fp);

		return true;
	}

	///////////////////////////////////////////////////

	//>> Обёртка для системной функции ShowWindow()
	inline int ShowWindowOS(void * handler, int settings)
	{
		int hRes = false;

		#if _OS_WINDOWS
		hRes = ShowWindow (
			std::move((HWND)handler),
			std::move(settings)
		);
		#endif

		return hRes;
	}

	//>> Обёртка для системной функции SetWindowPos()
	inline int SetWindowPosOS(void * handler, void * handler_after, int X, int Y, int cx, int cy, unsigned int flags)
	{
		int hRes = false;

		#if _OS_WINDOWS
		hRes = SetWindowPos (
			std::move((HWND)handler),
			std::move((HWND)handler_after),
			std::move(X),
			std::move(Y),
			std::move(cx),
			std::move(cy),
			std::move(flags)
		);
		#endif

		return hRes;
	}

	//>> Обёртка для системной функции GetSystemMetricsOS()
	inline int GetSystemMetricsOS(int flags)
	{
		int hRes = 0;

		#if _OS_WINDOWS
		hRes = GetSystemMetrics(std::move(flags));
		#endif

		return hRes;
	}

	///////////////////////////////////////////////////

	//>> debug
	inline void Makedirs(char* path)
	{
		char temp[0xff];
		size_t length = strlen(path);
		for (size_t x = 0; x<length; x++)
		{
			if (path[x]=='/'||path[x]=='\\')
			{
				memset(temp,0,sizeof(temp));
				strncat_s(temp,path,x);
				_mkdir(temp);
			};
		};
	}

	//>> debug
	inline void Makedump(char* buf, long size, char* name)
	{
		Makedirs(name);
		FILE *fp = _fsopen(name, "wb", _SH_DENYNO);
		fwrite(buf, size, 1, fp);
		fclose(fp);
	}
}

#endif // _BASE_HELPER_H