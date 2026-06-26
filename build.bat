@echo off
setlocal

set MINGW=C:\msys64\mingw64
set GPP=%MINGW%\bin\g++.exe
set SFML_INC=%MINGW%\include
set SFML_LIB=%MINGW%\lib

if not exist build mkdir build

echo Compiling Battleship...

"%GPP%" -std=c++20 -O2 ^
    -I"%SFML_INC%" ^
    src/main.cpp ^
    -L"%SFML_LIB%" ^
    -lsfml-graphics -lsfml-window -lsfml-system ^
    -o build/battleship.exe ^
    -mwindows 2>&1

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [BUILD FAILED] See errors above.
    pause
    exit /b 1
)

echo.
echo [BUILD OK] -- build\battleship.exe
echo.
echo Copying SFML DLLs...
copy /Y "%MINGW%\bin\sfml-graphics-3.dll"  build\ >nul 2>&1
copy /Y "%MINGW%\bin\sfml-window-3.dll"    build\ >nul 2>&1
copy /Y "%MINGW%\bin\sfml-system-3.dll"    build\ >nul 2>&1
copy /Y "%MINGW%\bin\freetype-6.dll"       build\ >nul 2>&1
copy /Y "%MINGW%\bin\openal32.dll"         build\ >nul 2>&1

echo Done. Run:  build\battleship.exe
