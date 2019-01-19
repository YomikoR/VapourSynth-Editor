CD %~dp0
@SET ORIGINAL_PATH=%PATH%

:: MSVC 64 bit build
@ECHO === SETTING UP ENVIRONMENT ===
@call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
@SET PATH=%PATH%;S:\sdk\qt\5.12.0\msvc2017_64\bin\
@ECHO === CLEANING UP BEFORE BUILDING ===
nmake.exe distclean -f Makefile
@ECHO === RUNNING QMAKE === 
qmake.exe -nocache "CONFIG+=release" pro.pro
@ECHO === BUILDING ===
nmake.exe release -f Makefile

:: Clean up the environment
@SET PATH=%ORIGINAL_PATH%