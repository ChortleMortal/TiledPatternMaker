@echo off
setlocal
rem Generates version.cpp from version.cpp.in, baking in the current
rem timestamp. Used by TiledPatternMaker.pro (qmake/Qt Creator build) on
rem Windows -- mirrors what gen_version.cmake does for the CMake build.
rem
rem usage: gen_version.bat <version.cpp.in> <version.cpp>
set "IN=%~1"
set "OUT=%~2"
for /f "usebackq delims=" %%T in (`powershell -NoProfile -Command "Get-Date -Format 'yyyy-MM-dd HH:mm:ss'"`) do set "TS=%%T"
powershell -NoProfile -Command "(Get-Content -Raw -LiteralPath '%IN%') -replace '@BUILD_TIMESTAMP@', '%TS%' | Set-Content -NoNewline -LiteralPath '%OUT%'"
endlocal
