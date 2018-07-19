mkdir tmp\

del /F /S /Q tmp\proxy_ex\*

mkdir tmp\proxy_ex\
copy /B /Y .\proxy_ex\Release\proxy_ex.dll tmp\proxy_ex\proxy_ex.dll
upx --best tmp\proxy_ex\proxy_ex.dll

copy /B /Y .\proxy_ex\readme.txt tmp\proxy_ex\readme.txt
copy /B /Y .\proxy_ex\license.txt tmp\proxy_ex\license.txt

del /F /S /Q tmp\proxy_ex.zip

chdir tmp\proxy_ex\
zip -r -9 -q ..\proxy_ex.zip *
chdir ..\..

