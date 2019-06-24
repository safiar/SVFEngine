// ----------------------------------------------------------------------- //
//
// MODULE  : version_conv.h
//
// PURPOSE : Макрос конвертации текста
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _VERSION_CONV_H
#define _VERSION_CONV_H

#define TOWSTRING_(s) L#s
#define TOWSTRING(s) TOWSTRING_(s)

#define TOSTRING_(s) #s
#define TOSTRING(s) TOSTRING_(s)

#endif // _VERSION_CONV_H
