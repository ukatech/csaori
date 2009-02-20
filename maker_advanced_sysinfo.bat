mkdir tmp\

del /F /S /Q tmp\advanced_sysinfo\*

mkdir tmp\advanced_sysinfo\
copy /B /Y .\advanced_sysinfo\Release\advanced_sysinfo.dll tmp\advanced_sysinfo\advanced_sysinfo.dll
upx --best tmp\advanced_sysinfo\advanced_sysinfo.dll

copy /B /Y .\advanced_sysinfo\readme.txt tmp\advanced_sysinfo\readme.txt
copy /B /Y .\advanced_sysinfo\license.txt tmp\advanced_sysinfo\license.txt

del /F /S /Q tmp\advanced_sysinfo.zip

chdir tmp\advanced_sysinfo\
zip -r -9 -q ..\advanced_sysinfo.zip *
chdir ..\..

