mkdir tmp\

del /F /S /Q tmp\textcopy2\*

mkdir tmp\textcopy2\
copy /B /Y .\textcopy2\Release\textcopy2.dll tmp\textcopy2\textcopy2.dll
upx --best tmp\textcopy2\textcopy2.dll

copy /B /Y .\textcopy2\readme.txt tmp\textcopy2\readme.txt
copy /B /Y .\textcopy2\license.txt tmp\textcopy2\license.txt

del /F /S /Q tmp\textcopy2.zip

chdir tmp\textcopy2\
zip -r -9 -q ..\textcopy2.zip *
chdir ..\..

