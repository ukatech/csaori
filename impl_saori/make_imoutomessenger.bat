mkdir tmp\

del /F /S /Q tmp\imoutomessenger\*

mkdir tmp\imoutomessenger\
copy /B /Y .\imoutomessenger\Release\imoutomessenger.dll tmp\imoutomessenger\imoutomessenger.dll
upx --best tmp\imoutomessenger\imoutomessenger.dll

copy /B /Y .\imoutomessenger\readme.txt tmp\imoutomessenger\readme.txt
copy /B /Y .\imoutomessenger\license.txt tmp\imoutomessenger\license.txt

del /F /S /Q tmp\imoutomessenger.zip

chdir tmp\imoutomessenger\
zip -r -9 -q ..\imoutomessenger.zip *
chdir ..\..

