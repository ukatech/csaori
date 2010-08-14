mkdir tmp\

del /F /S /Q tmp\window_info\*

mkdir tmp\window_info\
copy /B /Y .\window_info\Release\window_info.dll tmp\window_info\window_info.dll
upx --best tmp\window_info\window_info.dll

copy /B /Y .\window_info\readme.txt tmp\window_info\readme.txt
copy /B /Y .\window_info\license.txt tmp\window_info\license.txt

del /F /S /Q tmp\window_info.zip

chdir tmp\window_info\
zip -r -9 -q ..\window_info.zip *
chdir ..\..

