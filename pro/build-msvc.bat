@SET BRANCH=%1
@IF "%BRANCH%"=="" (
SET GIT_BRANCH=
SET NAME_BRANCH=
)
@IF NOT "%BRANCH%"=="" (
SET GIT_BRANCH=-b %BRANCH%
SET NAME_BRANCH=_%BRANCH%
)
@SET BUILDROOT=vsedit-build

@pushd "%TEMP%"
@rd /q /s %BUILDROOT%
@mkdir %BUILDROOT%
git clone %GIT_BRANCH% https://bitbucket.org/mystery_keeper/vapoursynth-editor.git vsedit-build
@cd /d %BUILDROOT%\pro

@SET ORIGINAL_PATH=%PATH%

:: MSVC 64 bit build
@ECHO === SETTING UP ENVIRONMENT ===
@call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
@SET PATH=%PATH%;S:\sdk\qt\5.12.0\msvc2017_64\bin\
@ECHO === RUNNING QMAKE === 
qmake.exe -nocache "CONFIG+=release" pro.pro
@ECHO === BUILDING ===
nmake.exe release -f Makefile
@ECHO === ARCHIVING ===
@cd /d ../build/release-64bit-msvc
@del vc_redist.x64.exe
@7z a -y -m0=LZMA -mx9 "%~dp0/vsedit%NAME_BRANCH%.7z" "*"

:: Clean up the environment
@SET PATH=%ORIGINAL_PATH%

@popd