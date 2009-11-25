[en.US_UTF-8 (English)]
Setting Helper SAORI for MAKOTO - MakotoUtil

MakotoUtil is a SAORI for MAKOTO(AYA as MAKOTO/KAWARI as MAKOTO) for adding 
shell only setting ability by simple input box and key press check.

Changelog:
20091125 - Initial release

Commands:

(1) keydown
argument[0]: keydown
argument[1]: KeyCode

detect KeyCode is pressed. return 1 if KeyCode is pressed, return 0 if not.

(2) inputbox
argument[0]: inputbox
argument[1]: [prompt]
argument[2]: [title]
argument[3]: [default value]
argument[4]: [flags = 0 - 3]

flags:
0 - single line (default)
1 - multi line
2 - single line integer only
3 - single line '*' masked

show an input box. return user input string, \r\n will be replaced to CHR(2). return default value if user press cancel button.

(3) messagebox
argument[0]: messagebox
argument[1]: [message]
argument[2]: [title]
argument[3]: [flags = 0]

flags:
(Please refer to "Appendix - MessageBox flags")

show a message box. return button value pressed by user.

Credits:
Using Dialog Templates to create an InputBox() in C++
http://www.codeproject.com/KB/dialog/w32inputbox_1.aspx

Appendix:
- Common KeyCodes
Backspace = 8
Tab = 9
Enter = 13
Shift = 16
Control = 17
Alt = 18
Pause = 19
Caps Lock = 20
Page Up = 33
Page Down = 34
End = 35
Home = 36
Insert = 45
Delete = 46
F1 = 112
F2 = 113
F3 = 114
F4 = 115
F5 = 116
F6 = 117
F7 = 118
F8 = 119
F9 = 120
F10 = 121
F11 = 122
F12 = 123
Num Lock = 144
Scroll Lock = 145

- MessageBox flags
#define MB_OK                       0
#define MB_OKCANCEL                 1
#define MB_ABORTRETRYIGNORE         2
#define MB_YESNOCANCEL              3
#define MB_YESNO                    4
#define MB_RETRYCANCEL              5
#define MB_CANCELTRYCONTINUE        6

#define MB_ICONHAND                 16
#define MB_ICONQUESTION             32
#define MB_ICONEXCLAMATION          48
#define MB_ICONASTERISK             64


(End of en.US_UTF-8 Readme)

[zh.TW_UTF-8 (正體中文)]
MAKOTO用設定助手SAORI - MakotoUtil

MakotoUtil 是一個 MAKOTO(AYA as MAKOTO/KAWARI as MAKOTO) 用的 SAORI 以簡單輸入框和按鍵檢測來提供 Shell 專用設定

更新：
20091125 - 初版發佈

指令:

(1) keydown
argument[0]: keydown
argument[1]: KeyCode

按鍵檢測。如果 KeyCode 被按下回傳1，否則回傳0。

(2) inputbox
argument[0]: inputbox
argument[1]: [輸入提示]
argument[2]: [標題]
argument[3]: [預設文字]
argument[4]: [旗標 = 0 - 3]

顯示輸入框，回傳使用者輸入的文字，\r\n會替換為CHR(2)。如果使用者按 Cancel 按鈕則會回傳預設文字。

(3) messagebox
argument[0]: messagebox
argument[1]: [信息]
argument[2]: [標題]
argument[3]: [旗標 = 0]

顯示信息框，回傳使用者按下的按鈕數值。

旗標:
(請參照「附錄 - MessageBox 旗標」)

鳴謝:
Using Dialog Templates to create an InputBox() in C++
http://www.codeproject.com/KB/dialog/w32inputbox_1.aspx

附錄:
- 常用 KeyCode
Backspace = 8
Tab = 9
Enter = 13
Shift = 16
Control = 17
Alt = 18
Pause = 19
Caps Lock = 20
Page Up = 33
Page Down = 34
End = 35
Home = 36
Insert = 45
Delete = 46
F1 = 112
F2 = 113
F3 = 114
F4 = 115
F5 = 116
F6 = 117
F7 = 118
F8 = 119
F9 = 120
F10 = 121
F11 = 122
F12 = 123
Num Lock = 144
Scroll Lock = 145

- MessageBox 旗標
#define MB_OK                       0
#define MB_OKCANCEL                 1
#define MB_ABORTRETRYIGNORE         2
#define MB_YESNOCANCEL              3
#define MB_YESNO                    4
#define MB_RETRYCANCEL              5
#define MB_CANCELTRYCONTINUE        6

#define MB_ICONHAND                 16
#define MB_ICONQUESTION             32
#define MB_ICONEXCLAMATION          48
#define MB_ICONASTERISK             64

(End of zh.TW_UTF-8 Readme)