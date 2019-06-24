// ----------------------------------------------------------------------- //
//
// MODULE  : shared.h
//
// PURPOSE : Общее разное
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _SHARED_H
#define _SHARED_H

namespace SAVFGAME
{
	enum eClickStatus	// interactive element status
	{
		CLICK_OFF,		// interactive is OFF
		CLICK_ON,		// interactive is ON

		CLICK_MISSED,	// missed   interactive element [interactive is ON]
		CLICK_TARGETED,	// targeted interactive element [interactive is ON]
		CLICK_CLICKED,	// clicked  interactive element [interactive is ON]

		CLICK_NO_CHANGES, // no changes from last state

		ENUM_CLICK_MAX
	};
};

#endif // _SHARED_H