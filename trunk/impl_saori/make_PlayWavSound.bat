mkdir tmp\

del /F /S /Q tmp\PlayWavSound\*

mkdir tmp\PlayWavSound\
copy /B /Y .\PlayWavSound\Release\PlayWavSound.dll tmp\PlayWavSound\PlayWavSound.dll
upx --best tmp\PlayWavSound\PlayWavSound.dll

copy /B /Y .\PlayWavSound\readme.txt tmp\PlayWavSound\readme.txt
copy /B /Y .\PlayWavSound\license.txt tmp\PlayWavSound\license.txt

del /F /S /Q tmp\PlayWavSound.zip

chdir tmp\PlayWavSound\
zip -r -9 -q ..\PlayWavSound.zip *
chdir ..\..

