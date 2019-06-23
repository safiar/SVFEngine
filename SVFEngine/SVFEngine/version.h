// ----------------------------------------------------------------------- //
//
// MODULE  : version.h
//
// PURPOSE : Версия продукта
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _VERSION_H
#define _VERSION_H

#include "version_major.h"
#include "version_minor.h"
#include "version_revision.h"
#include "version_build.h"

#include "version_conv.h"


#define PRODUCT_VERSION			VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD
#define PRODUCT_VERSION_STR		    TOSTRING(VERSION_MAJOR)    \
								"." TOSTRING(VERSION_MINOR)    \
								"." TOSTRING(VERSION_REVISION) \
								"." TOSTRING(VERSION_BUILD)

#endif // _VERSION_H
