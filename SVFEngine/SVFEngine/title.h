// ----------------------------------------------------------------------- //
//
// MODULE  : title.h
//
// PURPOSE : Название и описание продукта, имя производителя
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _TITLE_H
#define _TITLE_H

#ifdef _DEBUG
	#define PRODUCT_TITLE_ L"SVF Engine alpha prototype /DEBUG/"
#else
	#define PRODUCT_TITLE_ L"SVF Engine alpha prototype"
#endif

#define PRODUCT_AUTHOR			L"Savenkov F.A."						// PRODUCT COMPANY
#define PRODUCT_TITLE			PRODUCT_TITLE_							// GAME TITLE
#define PRODUCT_DESCRIPTION		L"SVF Game Engine - Alpha Prototype"	// FILE DESCRIPTION
#define PRODUCT_FILENAME		L"SVFEngine.exe"						// INTERNAL NAME
#define PRODUCT_NAME			L"SVFEngine"							// GAME NAME
#define PRODUCT_COPYRIGHT		L"Savenkov F.A. (C) 2017-2018"			// LEGALS

#endif
