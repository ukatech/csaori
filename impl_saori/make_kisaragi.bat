mkdir tmp\

del /F /S /Q tmp\kisaragi\*

mkdir tmp\kisaragi\
copy /B /Y .\kisaragi\Release\kisaragi.dll tmp\kisaragi\kisaragi.dll
upx --best tmp\kisaragi\kisaragi.dll

copy /B /Y .\kisaragi\readme.txt tmp\kisaragi\readme.txt
copy /B /Y .\kisaragi\license.txt tmp\kisaragi\license.txt

mkdir tmp\kisaragi\dic\
copy /B /Y .\kisaragi\mecab\dic\char.bin tmp\kisaragi\dic\char.bin
copy /B /Y .\kisaragi\mecab\dic\dicrc tmp\kisaragi\dic\dicrc
copy /B /Y .\kisaragi\mecab\dic\matrix.bin tmp\kisaragi\dic\matrix.bin
copy /B /Y .\kisaragi\mecab\dic\sys.dic tmp\kisaragi\dic\sys.dic
copy /B /Y .\kisaragi\mecab\dic\unk.dic tmp\kisaragi\dic\unk.dic

copy /B /Y .\kisaragi\mecab\sdk\libmecab.dll tmp\kisaragi\libmecab.dll

del /F /S /Q tmp\kisaragi.zip

chdir tmp\kisaragi\
zip -r -9 -q ..\kisaragi.zip *
chdir ..\..

