@echo off 

echo @echo off > config.status.bat

echo echo running: configure.bat %* >> config.status.bat
echo configure.bat %* >> config.status.bat

rem The default is debug build
set cps_plus=debug
set cps_minus=release

set arguments=

:LOOP
if '%1' == ''                      goto QMAKE
if '%1' == '--enable-debug'        goto DEBUG
if '%1' == '--disable-debug'       goto RELEASE
if '%1' == '--enable-release'      goto RELEASE
if '%1' == '--disable-release'     goto DEBUG

set arguments=%arguments% %1
shift
goto LOOP

:DEBUG
:RELEASE
shift
goto LOOP

:QMAKE
cd copasi
del /S Makefile*
del commandline\CConfigurationFile.obj
del UI\copasiui3window.obj 
del UI\CQSplashWidget.obj 
del CopasiUI\main.obj 
del CopasiSE\CopasiSE.obj

if '%QT4DIR%' == ''   goto QT3
set QTDIR=%QT4DIR%

:QT3 
echo executing in copasi:
echo   qmake "CONFIG-=release" "CONFIG-=debug" %arguments%
%QTDIR%\bin\qmake "CONFIG-=release" "CONFIG-=debug" %arguments%

nmake qmake_all

cd libs
nmake qmake_all
cd ..

rem force relink

cd ..

rem Build the semantic test suite wrapper
cd semantic-test-suite
echo executing in semantic-test-suite:
rem  echo   for %%d in (%subdirs%) do del %%d\.qmake.internal.cache
for %%d in (%subdirs%) do del %%d\.qmake.internal.cache
echo   qmake "CONFIG-=release" "CONFIG-=debug" %arguments%
%QTDIR%\bin\qmake "CONFIG-=release" "CONFIG-=debug" %arguments%

cd ..

rem Build the stochastic test suuite wrapper
cd stochastic-testsuite
echo executing in stochastic-testsuite:
rem  echo   for %%d in (%subdirs%) do del %%d\.qmake.internal.cache
for %%d in (%subdirs%) do del %%d\.qmake.internal.cache
echo   qmake "CONFIG-=release" "CONFIG-=debug" %arguments%
%QTDIR%\bin\qmake "CONFIG-=release" "CONFIG-=debug" %arguments%

cd ..
