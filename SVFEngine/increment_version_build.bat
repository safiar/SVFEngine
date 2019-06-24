for /f "eol=/ tokens=3* delims= " %%x in (SVFEngine\version_revision.h) do set _REVIS_=%%x
for /f "eol=/ tokens=3* delims= " %%x in (SVFEngine\version_build.h)    do set _BUILD_=%%x

set /A _BUILD_=_BUILD_+1
set /A _REVIS_OLD_=_REVIS_

if %_BUILD_% GEQ 65535 (

   set /A _BUILD_=1
   set /A _REVIS_=_REVIS_+1

)

set _BUILD_STR_=#define VERSION_BUILD %_BUILD_%  // uint16
set _REVIS_STR_=#define VERSION_REVISION %_REVIS_%  // uint16


ECHO %_BUILD_STR_% > SVFEngine\version_build.h


if %_REVIS_OLD_% NEQ %_REVIS_% (

   ECHO %_REVIS_STR_% > SVFEngine\version_revision.h

)

