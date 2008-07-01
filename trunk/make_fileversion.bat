mkdir tmp\

del /F /S /Q tmp\FileVersion\*

mkdir tmp\FileVersion\
copy /B /Y .\SaoriTest\FileVersion\Release\FileVersion.dll tmp\FileVersion\FileVersion.dll
upx --best tmp\FileVersion\FileVersion.dll

copy /B /Y .\SaoriTest\FileVersion\readme.txt tmp\FileVersion\readme.txt
copy /B /Y .\SaoriTest\FileVersion\license.txt tmp\FileVersion\license.txt

del /F /S /Q tmp\FileVersion.zip

chdir tmp\FileVersion\
zip -r -9 -q ..\FileVersion.zip *
chdir ..\..

