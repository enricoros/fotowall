@ECHO OFF
SET TSFILES=
FOR %%i IN (*.ts) DO call :addtotsfilelist %%i

linguist.exe %TSFILES%

:addtotsfilelist
set TSFILES=%1 %TSFILES%
goto :eof
