mkdir tmp\

del /F /S /Q tmp\gainer\*

mkdir tmp\gainer\
copy /B /Y .\gainer\Release\gainer.dll tmp\gainer\gainer.dll
upx --best tmp\gainer\gainer.dll

copy /B /Y .\gainer\readme.txt tmp\gainer\readme.txt
copy /B /Y .\gainer\license.txt tmp\gainer\license.txt

del /F /S /Q tmp\gainer.zip

chdir tmp\gainer\
zip -r -9 -q ..\gainer.zip *
chdir ..\..

