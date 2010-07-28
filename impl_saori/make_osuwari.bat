mkdir tmp\

del /F /S /Q tmp\Osuwari\*

mkdir tmp\Osuwari\
copy /B /Y .\OsuwariDll\Osuwari\Release\Osuwari.dll tmp\Osuwari\Osuwari.dll
upx --best tmp\Osuwari\Osuwari.dll

copy /B /Y .\OsuwariDll\Osuwari\Release\readme.txt tmp\Osuwari\readme.txt

del /F /S /Q tmp\Osuwari.zip

chdir tmp\Osuwari\
zip -r -9 -q ..\Osuwari.zip *
chdir ..\..

