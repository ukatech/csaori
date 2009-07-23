[en.US_UTF-8 (English)]
Simple SQLite3 database SAORI - sqlori
SQLori v1.1.10+
Author: 時原砂, Roy
License: This SAORI is for testing and teaching, anyone can use and modify it freely.

1. Update
Date     Version  Log
20090723 1.1.10+  + add back SQLite 3.6.16 source code as SQLite is Public Domain
                  ! Fix: when no filepath is specified, default file name is be "sqlori.db" but not "sqlori._db"
                  ! Fix missing SAORI return status
                  ! move real CSAORI implementation from csaori.cpp/caori.h to csaori_interface.cpp for easier update of CSAORI
                  ! update CSAORI to svn rev 88
20090723 1.1.10   ! remove GET_TABLE command, integrated into EXEC command
                  ! add OPEN and CLOSE command for opening database from other file name
                  * Fix demo code
                  * Support UTF-8
20090720 1.0.0    # Initial release

2. Introduction
In simple words, SQLite + CSAORI = SQLori.

SQLori is a very simple, but it is far better than the old SAORI debeso.dll.
Using SQLite 3.6.16
For simple coding, SQLori doesn't use Value[n].

3. Commands

(1) OPEN
argument[0]: OPEN
argument[1]: [%(filepath)]

Open database file. If no filepath is specified, default file name is be "sqlori.db"

(2) EXEC
argument[0]: EXEC
argument[1]: %(SQL_request)

Execute SQLite statements.

(3) CLOSE
argument[0]: CLOSE

Close database file.

3. Application

As the return value is complex which needs processing in SHIORI side,
there is a AYA processing script and Unit Test demo.

Code:
// sqlori.dll may be vary, change may be needed
#define SQLORI_PATH 'saori\sqlori.dll'
#define SQLITE_OK 0
#define SQLITE_ERROR 1

SQLori
{
    SQLori_result = SQLITE_OK;
    SQLori_fields = IARRAY;
    SQLori_values = IARRAY;
    SQLori_col = 0;
    SQLori_row = 0;
   
    _ret =  FUNCTIONEX(SQLORI_PATH, _argv[0], _argv[1]);

    _data = SPLIT(_ret[0], CHR(0x1));
    SQLori_result = TOINT(_data[0]);
   
    if SQLori_result != SQLITE_OK {
        SQLori_fields[0] = 'Error';
        SQLori_values[0] = _data[1];
        SQLori_col = 1;
        SQLori_row = 1;
    }
    else {
        if (ARRAYSIZE(_data) >= 3) {
            SQLori_fields = SPLIT(_data[1], CHR(0x2));
            SQLori_values = SPLIT(_data[2], CHR(0x2));
            SQLori_col = ARRAYSIZE(SQLori_fields);
            if SQLori_col != 0
                SQLori_row = ARRAYSIZE(SQLori_values) / SQLori_col;
        }
    }
}

SQLori.value
{
    _ret = '';
    if SQLori_col < 1 {
        return;
    }
    elseif SQLori_col == 1 {
        _ret = SQLori_values[_argv[0]];
    }
    else {
        _ret = SQLori_values[_argv[0] * SQLori_col + _argv[1]];
    }
   
    _ret;
}

// Unit Test
UnitTest
{
    _res = '';
    SQLori('OPEN', 'sqliori.db');

    SQLori('EXEC', "CREATE TABLE data (ID integer primary key, name, age);");
    _res += SQLori_result + '\n';
    SQLori('EXEC', "INSERT INTO data VALUES(null, 'San Cheung', '20');");
    _res += SQLori_result + '\n';
    SQLori('EXEC', "INSERT INTO data VALUES(null, 'Shi Lee', '18'); INSERT INTO data VALUES(null, 'Ng Wong', '25'); INSERT INTO data VALUES(null, 'Hui Kung Kai', '90');");
    _res += SQLori_result + '\n';
    SQLori('EXEC', "UPDATE data SET age = '23' WHERE name == 'San Cheung';");
    _res += SQLori_result + '\n';
    SQLori('EXEC', "SELECT * FROM data;");
    _res += SQLori_result + '\n';
    for _i = 0; _i < SQLori_col; _i++ {
        _res += SQLori_fields[_i] + ' ';
    }
    _res += '\n';
    for _i = 0; _i < SQLori_row; _i++ {
        for _j = 0; _j < SQLori_col; _j++ {
            _res += SQLori.value(_i, _j) + ' ';
        }
        _res += '\n';
    }
    SQLori('EXEC', "DELETE FROM data WHERE ID < 3;");
    _res += SQLori_result + '\n';
    SQLori('EXEC', "SELECT count(*) FROM data;");
    _res += SQLori_fields[0] + ' = ' + SQLori_values[0] + '\n';
    SQLori('EXEC', "DROP TABLE data;");
    _res += SQLori_result + '\n';

    SQLori('CLOSE');

    _res;
}

(End of en.US_UTF-8 Readme)

[zh.TW_UTF-8 (正體中文)]
簡易資料庫SAORI - sqlori
SQLori v1.1.10+
作者：時原砂、Roy
授權：這個SAORI純為測試與教學之用，其他人可隨意使用或修改此原始碼

1. 更新
Date     Version  Log
20090723 1.1.10+  + 補回SQLite 3.6.16的源碼，SQLite是Public Domain的關係一起包進來也沒關係。
                  ! 修正：不指定filepath時，預設資料庫檔名為sqlori.db
                  ! 修正沒有設定 SAORI return status的問題
                  ! 有關 CSAORI 執行實體自csaori.cpp/caori.h中抽出成csaori_interface.cpp，使 CSAORI 可以獨立更新
                  ! 更新 CSAORI 至 svn rev 88
20090723 1.1.10   ! 刪除GET_TABLE指令，其功能整合至EXEC之中
                  ! 追加OPEN、CLOSE指令，允許讀取指定的資料庫檔案
                  * 修正範例程式碼
                  * 支援UTF-8
20090720 1.0.0    # 初版發佈

2. 說明

簡單的說就是sqlite + csaori = sqlori (炸)

SQLori在功能上極為陽春
不過絕對比debeso.dll這個N年前的SAORI好的多...XD
使用sqlite版本為3.6.16
為求簡便，只返回單一返值而不使用Value[n]格式

3. 指令

(1) OPEN
argument[0]: OPEN
argument[1]: [%(filepath)]

開啟資料庫的連線。若不指定filepath時，預設資料庫檔名為sqlori.db。

(2) EXEC
argument[0]: EXEC
argument[1]: %(SQL_request)

執行資料庫指令的核心函式。

(3) CLOSE
argument[0]: CLOSE

關閉資料庫連線。

3. 應用

由於格式較為複雜，需要在人格端進行一些處理
這裡提供一個AYA版的處理函式及基本的單元測試範例

代碼:
// sqlori.dll的位置若有不同的話請另行更改
#define SQLORI_PATH 'saori\sqlori.dll'
#define SQLITE_OK 0
#define SQLITE_ERROR 1

SQLori
{
    SQLori_result = SQLITE_OK;
    SQLori_fields = IARRAY;
    SQLori_values = IARRAY;
    SQLori_col = 0;
    SQLori_row = 0;
   
    _ret =  FUNCTIONEX(SQLORI_PATH, _argv[0], _argv[1]);

    _data = SPLIT(_ret[0], CHR(0x1));
    SQLori_result = TOINT(_data[0]);
   
    if SQLori_result != SQLITE_OK {
        SQLori_fields[0] = 'Error';
        SQLori_values[0] = _data[1];
        SQLori_col = 1;
        SQLori_row = 1;
    }
    else {
        if (ARRAYSIZE(_data) >= 3) {
            SQLori_fields = SPLIT(_data[1], CHR(0x2));
            SQLori_values = SPLIT(_data[2], CHR(0x2));
            SQLori_col = ARRAYSIZE(SQLori_fields);
            if SQLori_col != 0
                SQLori_row = ARRAYSIZE(SQLori_values) / SQLori_col;
        }
    }
}

SQLori.value
{
    _ret = '';
    if SQLori_col < 1 {
        return;
    }
    elseif SQLori_col == 1 {
        _ret = SQLori_values[_argv[0]];
    }
    else {
        _ret = SQLori_values[_argv[0] * SQLori_col + _argv[1]];
    }
   
    _ret;
}

// 單元測試
UnitTest
{
    _res = '';
    SQLori('OPEN', 'sqliori.db');

    SQLori('EXEC', "CREATE TABLE data (ID integer primary key, 姓名, 年齡);");
    _res += SQLori_result + '\n';
    SQLori('EXEC', "INSERT INTO data VALUES(null, '張三', '20');");
    _res += SQLori_result + '\n';
    SQLori('EXEC', "INSERT INTO data VALUES(null, '李四', '18'); INSERT INTO data VALUES(null, '王五', '25'); INSERT INTO data VALUES(null, '許功蓋', '90');");
    _res += SQLori_result + '\n';
    SQLori('EXEC', "UPDATE data SET 年齡 = '23' WHERE 姓名 == '張三';");
    _res += SQLori_result + '\n';
    SQLori('EXEC', "SELECT * FROM data;");
    _res += SQLori_result + '\n';
    for _i = 0; _i < SQLori_col; _i++ {
        _res += SQLori_fields[_i] + ' ';
    }
    _res += '\n';
    for _i = 0; _i < SQLori_row; _i++ {
        for _j = 0; _j < SQLori_col; _j++ {
            _res += SQLori.value(_i, _j) + ' ';
        }
        _res += '\n';
    }
    SQLori('EXEC', "DELETE FROM data WHERE ID < 3;");
    _res += SQLori_result + '\n';
    SQLori('EXEC', "SELECT count(*) FROM data;");
    _res += SQLori_fields[0] + ' = ' + SQLori_values[0] + '\n';
    SQLori('EXEC', "DROP TABLE data;");
    _res += SQLori_result + '\n';

    SQLori('CLOSE');

    _res;
}

(End of zh.TW_UTF-8 Readme)