// ----------------------------------------------------------------------- //
//
// MODULE  : hashs.h
//
// PURPOSE : Хеш-суммы
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _HASHS_H
#define _HASHS_H

class HCRC32
{
protected:
	unsigned int table[256];
public:
	unsigned int m_crc32;
public:
	HCRC32()
	{
		const unsigned int CRC_POLY = 0xEDB88320;
		unsigned int i, j, r;
		for (i = 0; i < 256; i++){
			for (r = i, j = 8; j; j--)
				r = r & 1 ? (r >> 1) ^ CRC_POLY : r >> 1;
			table[i] = r;
		}
		m_crc32 = 0;
	}
	~HCRC32(){}
	void ProcessCRC(void* pData, int nLen)
	{
		const unsigned int CRC_MASK = 0xD202EF8D;
		register unsigned char* pdata = reinterpret_cast <unsigned char*> (pData);
		register unsigned int crc = m_crc32;
		while (nLen--){
			crc = table[static_cast <unsigned char> (crc) ^ *pdata++] ^ crc >> 8;
			crc ^= CRC_MASK;
		}
		m_crc32 = crc;
	}
	void ProcessCRC(void* pData, int nLen, unsigned int CRC_MASK)
	{
		register unsigned char* pdata = reinterpret_cast <unsigned char*> (pData);
		register unsigned int crc = m_crc32;
		while (nLen--){
			crc = table[static_cast <unsigned char> (crc) ^ *pdata++] ^ crc >> 8;
			crc ^= CRC_MASK;
		}
		m_crc32 = crc;
	}
};

#endif  // _HASHS_H