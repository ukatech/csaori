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
argument[4]: [multiline? 0 or 1]

show an input box. return user input string, \r\n will be replaced to CHR(2). return default value if user press cancel button.

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
argument[4]: [是否多行? 0 或 1]

顯示輸入框，回傳使用者輸入的文字，\r\n會替換為CHR(2)。如果使用者按 Cancel 按鈕則會回傳預設文字。

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

(End of zh.TW_UTF-8 Readme)