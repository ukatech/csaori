mkdir tmp\

del /F /S /Q tmp\xaoric\*

mkdir tmp\xaoric\
copy /B /Y .\XPath\Release\xaoric.dll tmp\xaoric\xaoric.dll
upx --best tmp\xaoric\xaoric.dll

copy /B /Y .\XPath\readme.txt tmp\xaoric\readme.txt
copy /B /Y .\XPath\license.txt tmp\xaoric\license.txt

del /F /S /Q tmp\xaoric.zip

chdir tmp\xaoric\
zip -r -9 -q ..\xaoric.zip *
chdir ..\..

