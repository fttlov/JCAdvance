#Requires AutoHotkey v2.0
#SingleInstance Force

; --- GLOBAL SETTINGS ---
Global ConfigIni := "config.ini"
Global hModule := DllCall("LoadLibrary", "Str", A_ScriptDir "\JoyShockLibrary.dll", "Ptr")
Global CurrentLang := "English"
try CurrentLang := IniRead(A_ScriptDir "\" ConfigIni, "ConfigGUI", "Language", "English")

; For translate
T(str) {
    global CurrentLang
    ; Если переменная еще не создана в памяти, возвращаем английский текст
    if (!IsSet(CurrentLang) || CurrentLang == "English")
        return str
        
    return IniRead(A_ScriptDir "\Config_lang\" CurrentLang ".ini", "Translations", str, str)
}

Global Layout := "Nintendo"
try Layout := IniRead(A_ScriptDir "\" ConfigIni, "ConfigGUI", "Layout", "Nintendo")

; Читаем активный профиль из config.ini (по умолчанию default.ini)
Global ActiveProfile := "default.ini"
try ActiveProfile := IniRead(A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile", "default.ini")
Global XboxIni := "XboxProfiles\" ActiveProfile

; 2. Упорядоченный список геймпада (Common, Nintendo, Sony)
Global JslKeys := [
    "NONE", 
    "UP", "DOWN", "LEFT", "RIGHT", "L3", "R3", "L", "R", "ZL", "ZR", 
    "A", "B", "X", "Y", "MINUS", "PLUS", "SL", "SR", "CAPTURE", "HOME", 
    "CROSS", "CIRCLE", "SQUARE", "TRIANGLE", "SHARE", "OPTIONS", "L1", "R1", "L2", "R2", "L4", "R4"
]

Global SonyMapping := ["UP", "DOWN", "LEFT", "RIGHT", "L3", "R3", "L1", "R1", "L2", "R2", "CROSS", "CIRCLE", "SQUARE", "TRIANGLE", "SHARE", "OPTIONS", "L4", "R4"]

; 4 & 6. Полный список Клавиатуры и Мыши
Global KbmKeys := [
    "NONE", "MOUSE-LEFT", "MOUSE-RIGHT", "MOUSE-MIDDLE", "MOUSE-WHEEL-UP", "MOUSE-WHEEL-DOWN",
    "ESCAPE", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
    "~", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=",
    "TAB", "CAPS-LOCK", "SHIFT", "LSHIFT", "RSHIFT", "CTRL", "LCTRL", "RCTRL",
    "WIN", "ALT", "LALT", "RALT", "SPACE", "ENTER", "BACKSPACE",
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]",
    "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "APOSTROPHE", "\",
    "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?",
    "PRINTSCREEN", "SCROLL-LOCK", "PAUSE", "INSERT", "HOME", "DELETE", "END", "PAGE-UP", "PAGE-DOWN",
    "UP", "DOWN", "LEFT", "RIGHT",
    "NUM-LOCK", "NUMPAD0", "NUMPAD1", "NUMPAD2", "NUMPAD3", "NUMPAD4", "NUMPAD5", "NUMPAD6", "NUMPAD7", "NUMPAD8", "NUMPAD9",
    "NUMPAD-DIVIDE", "NUMPAD-MULTIPLY", "NUMPAD-MINUS", "NUMPAD-PLUS", "NUMPAD-DEL", "NUMPAD-ENTER"
]

; 5. Полный список Xbox кнопок
Global XboxKeys := [
    "NONE", "UP", "DOWN", "LEFT", "RIGHT", "BACK", "START", "LS", "RS", "LB", "RB", "A", "B", "X", "Y", "LT", "RT",
    "LS-UP", "LS-DOWN", "LS-LEFT", "LS-RIGHT", "RS-UP", "RS-DOWN", "RS-LEFT", "RS-RIGHT"
]

Global CtrlXbox := Map()
Global CtrlJoyCon := Map()
Global CtrlSony := Map()
Global CtrlWheel := Map()
Global CtrlSettings := Map()

SetDdlValue(ddl, val) {
    if (val == "")
        return
    try {
        ddl.Text := val
    } catch {
        ddl.Add([val])
        ddl.Text := val
    }
}

; =========================================
; MAIN WINDOW CREATION
; =========================================
MainGui := Gui("-MaximizeBox", "JCAdvance Config Editor")
MainGui.OnEvent("Close", (*) => ExitApp())

; --- ГЛОБАЛЬНЫЙ ШРИФТ ---
; "s10" означает размер шрифта 10. Если нужно больше - ставь "s11" или "s12".
; Чтобы изменить сам стиль шрифта, напиши так: MainGui.SetFont("s10", "Segoe UI")
MainGui.SetFont("s10")

; Вкладки подняты на самый верх (y10)
Tabs := MainGui.Add("Tab3", "x10 y10 w830 h700", ["Xbox", "Joy-Con", "Sony", "Wheel", "Hotkeys", "Gyro", "Analog", "Profiles"])

; =========================================
; TAB 1: XBOX
; =========================================
Tabs.UseTab(1)
MainGui.Add("Text", "x20 y90 w810 Center", T("Mapping Nintendo\Sony buttons to XBOX virtual buttons"))
XboxMapping := ["UP", "DOWN", "LEFT", "RIGHT", "BACK", "START", "LS", "RS", "LB", "RB", "A", "B", "X", "Y", "LT", "RT"]

SavedXboxMap := Map()
For sec in ["Xbox", "JOYCONS", "DUALSENSE-EDGE"] {
    secText := ""
    try secText := IniRead(A_ScriptDir "\" XboxIni, sec)
    if (secText != "") {
        Loop Parse secText, "`n", "`r" {
            parts := StrSplit(A_LoopField, "=")
            if (parts.Length == 2)
                SavedXboxMap[Trim(parts[2])] := Trim(parts[1])
        }
    }
}

; Картинка XBOX сдвинута вниз, строго по центру вертикали и горизонтали
MainGui.Add("Picture", "x275 y220 w300 h-1", A_ScriptDir "\Icon\xbox.png")

; --- ЛЕВАЯ КОЛОНКА (Опущена вниз к центру экрана) ---
XboxMapLeft := ["LT", "LB", "BACK", "LS", "UP", "DOWN", "LEFT", "RIGHT"]
yPosLeft := 160
for key in XboxMapLeft {
    val := SavedXboxMap.Has(key) ? SavedXboxMap[key] : "NONE"
    
    MainGui.Add("Text", "x45 y" (yPosLeft+4) " w45 +Right", key ":")
    
    ddl := MainGui.Add("ComboBox", "x95 y" yPosLeft " w115 Choose1", JslKeys)
    SetDdlValue(ddl, val)
    CtrlXbox[key] := ddl
    
    btn := MainGui.Add("Button", "x220 y" (yPosLeft-1) " w50 h24", "Bind")
    btn.OnEvent("Click", BindGamepad.Bind(ddl))
    
    yPosLeft += 42 
}

; --- ПРАВАЯ КОЛОНКА (Опущена вниз, отзеркалена) ---
XboxMapRight := ["RT", "RB", "START", "RS", "Y", "X", "B", "A"]
yPosRight := 160
for key in XboxMapRight {
    val := SavedXboxMap.Has(key) ? SavedXboxMap[key] : "NONE"
    
    ddl := MainGui.Add("ComboBox", "x640 y" yPosRight " w115 Choose1", JslKeys)
    SetDdlValue(ddl, val)
    CtrlXbox[key] := ddl
    
    btn := MainGui.Add("Button", "x580 y" (yPosRight-1) " w50 h24", "Bind")
    btn.OnEvent("Click", BindGamepad.Bind(ddl))
    
    MainGui.Add("Text", "x760 y" (yPosRight+4) " w45", key)
    
    yPosRight += 42
}

; Текст-предупреждение сдвинут с красивым отступом от левого столбца
MainGui.Add("Text", "x245 y" (yPosLeft+45) " w450 cRed", T("* Only digital buttons can be successfully remapped"))

; =========================================
; TAB 2: JOY-CON
; =========================================
Tabs.UseTab(2)
MainGui.Add("Text", "x25 y45 w810 Center", T("Emulate keyboard/mouse keys using the Joy-Con's buttons"))

MainGui.Add("Picture", "x60 y140 w134 h-1", A_ScriptDir "\Icon\joycon_left.png")
MainGui.Add("Picture", "x656 y140 w134 h-1", A_ScriptDir "\Icon\joycon_right.png")

JoyconMapping := ["UP", "DOWN", "LEFT", "RIGHT", "L3", "R3", "L", "R", "ZL", "ZR", "B", "A", "Y", "X", "MINUS", "PLUS", "SL", "SR", "CAPTURE", "HOME"]
yPos := 70
for key in JoyconMapping {
    val := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key, "NONE")
    MainGui.Add("Text", "x290 y" (yPos+4) " w70", key ":")
    ddl := MainGui.Add("ComboBox", "x370 y" yPos " w150 Choose1", KbmKeys)
    SetDdlValue(ddl, val)
    CtrlJoyCon[key] := ddl
    btn := MainGui.Add("Button", "x530 y" (yPos-1) " w60 h24", "Bind")
    btn.OnEvent("Click", BindKbm.Bind(ddl))
    yPos += 30
}

yPos += 10
MainGui.Add("Text", "x75 y" yPos " w830 cBlue", T("*Also you can configure Analog Sticks directions to emulate Keyboard keys and Mouse in XboxProfiles\*.ini"))

; =========================================
; TAB 3: SONY
; =========================================
Tabs.UseTab(3)
MainGui.Add("Text", "x20 y90 w810 Center", T("Emulate keyboard/mouse keys using the Sony gamepad buttons"))

; Размещаем прозрачную картинку по центру
MainGui.Add("Picture", "x290 y220 w250 h-1", A_ScriptDir "\Icon\Sony.png")

; --- ЛЕВАЯ КОЛОНКА SONY ---
SonyMapLeft := ["L2", "L1", "SHARE", "L3", "UP", "DOWN", "LEFT", "RIGHT", "L4"]
yPosLeft := 160
for key in SonyMapLeft {
    val := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key, "NONE")
    
    MainGui.Add("Text", "x40 y" (yPosLeft+4) " w50 +Right", key ":")
    
    ddl := MainGui.Add("ComboBox", "x100 y" yPosLeft " w120 Choose1", KbmKeys)
    SetDdlValue(ddl, val)
    CtrlSony[key] := ddl
    
    btn := MainGui.Add("Button", "x230 y" (yPosLeft-1) " w50 h24", "Bind")
    btn.OnEvent("Click", BindKbm.Bind(ddl))
    
    yPosLeft += 38
}

; --- ПРАВАЯ КОЛОНКА SONY ---
SonyMapRight := ["R2", "R1", "OPTIONS", "R3", "TRIANGLE", "SQUARE", "CIRCLE", "CROSS", "R4"]
yPosRight := 160
for key in SonyMapRight {
    val := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key, "NONE")
    
    ddl := MainGui.Add("ComboBox", "x610 y" yPosRight " w120 Choose1", KbmKeys)
    SetDdlValue(ddl, val)
    CtrlSony[key] := ddl
    
    btn := MainGui.Add("Button", "x550 y" (yPosRight-1) " w50 h24", "Bind")
    btn.OnEvent("Click", BindKbm.Bind(ddl))
    
    MainGui.Add("Text", "x740 y" (yPosRight+4) " w50", key)
    
    yPosRight += 38
}

; =========================================
; TAB 4: WHEEL
; =========================================
Tabs.UseTab(4)
MainGui.Add("Text", "x20 y55 w700", T("Gyro Wheel for additional Xbox/KBM buttons mapping `nQuick press and release WHEEL-ACTIVATION button when gyro move"))
;yPos += 15
;MainGui.Add("Text", "x20 y55 w700", T("See more: https://github.com/r57zone/DSAdvance"))
yPos := 140
keyAct := "WHEEL-ACTIVATION"
valAct := IniRead(A_ScriptDir "\" XboxIni, "Motion", keyAct, "NONE")
MainGui.Add("Text", "x20 y" (yPos+4) " w140", keyAct ":")
ddlAct := MainGui.Add("ComboBox", "x280 y" yPos " w120 Choose1", JslKeys)
SetDdlValue(ddlAct, valAct)
CtrlWheel[keyAct] := {ddl: ddlAct} 
btnAct := MainGui.Add("Button", "x410 y" (yPos-1) " w60 h24", "Bind")
btnAct.OnEvent("Click", BindGamepad.Bind(ddlAct))

yPos += 60

WheelMapping := ["WHEEL-DEFAULT", "WHEEL-UP", "WHEEL-DOWN", "WHEEL-LEFT", "WHEEL-RIGHT"]
for key in WheelMapping {
    valXbox := IniRead(A_ScriptDir "\" XboxIni, "Motion", key, "NONE")
    valKbm := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key, "NONE")
    
    isKbm := false
    val := "NONE"
    
    if (valKbm != "NONE" && valKbm != "") {
        val := valKbm
        isKbm := true
    } else if (valXbox != "NONE" && valXbox != "") {
        val := valXbox
    }

    MainGui.Add("Text", "x20 y" (yPos+4) " w140", key ":")
    
    chkXbox := isKbm ? "" : " Checked1"
    chkKbm := isKbm ? " Checked1" : ""
    
    radXbox := MainGui.Add("Radio", "x160 y" (yPos+3) chkXbox, "Xbox")
    radKbm := MainGui.Add("Radio", "x220 y" (yPos+3) chkKbm, "KB/M")
    
    ddl := MainGui.Add("ComboBox", "x280 y" yPos " w120 Choose1", isKbm ? KbmKeys : XboxKeys)
    SetDdlValue(ddl, val)
    CtrlWheel[key] := {ddl: ddl, rXbox: radXbox, rKbm: radKbm}

    radXbox.OnEvent("Click", ChangeWheelList.Bind(ddl, XboxKeys))
    radKbm.OnEvent("Click", ChangeWheelList.Bind(ddl, KbmKeys))
    
    yPos += 35
}

ChangeWheelList(ddl, listArray, *) {
    val := ddl.Text
    ddl.Delete()
    ddl.Add(listArray)
    SetDdlValue(ddl, val)
}

; =========================================
; HELPERS
; =========================================

AddToggle(iniFile, sec, key, desc) {
    global yPos
    val := IniRead(A_ScriptDir "\" iniFile, sec, key, "0")
    chkOpt := (val = "1") ? " Checked1" : ""
    chk := MainGui.Add("Checkbox", "x20 y" yPos chkOpt, desc) ; было desc " (" key ")") - чтобы не выводить параметр рядом с названием
    CtrlSettings[key] := {type: "chk", ctrl: chk, file: iniFile, sec: sec}
    yPos += 28
}

AddMappedDropdown(iniFile, sec, key, desc, optionsArray, valMap) {
    global yPos
    val := IniRead(A_ScriptDir "\" iniFile, sec, key, "0")
    
    ; Ищем, какой текст соответствует цифре из INI
    selectedText := optionsArray[1]
    for k, v in valMap {
        if (v == val) {
            selectedText := k
            break
        }
    }
    
    MainGui.Add("Text", "x20 y" (yPos+3) " w210", desc ":") ; было desc " (" key "):")
    ddl := MainGui.Add("DropDownList", "x240 y" yPos " w150 Choose1", optionsArray)
    ddl.Text := selectedText
    
    ; valMap сохраняем в объект, чтобы при записи знать, как перевести текст обратно в цифру
    CtrlSettings[key] := {type: "mapped_ddl", ctrl: ddl, file: iniFile, sec: sec, valMap: valMap}
    yPos += 30
}

AddInput(iniFile, sec, key, desc, defaultVal := "0") {
    global yPos
    val := IniRead(A_ScriptDir "\" iniFile, sec, key, defaultVal)
    MainGui.Add("Text", "x20 y" (yPos+3) " w250", desc ":")
    edt := MainGui.Add("Edit", "x280 y" yPos " w80", val)
    CtrlSettings[key] := {type: "edt", ctrl: edt, file: iniFile, sec: sec}
    yPos += 28
}

AddHotkey(iniFile, sec, key, desc, listKeys, bindFunc) {
    global yPos
    val := IniRead(A_ScriptDir "\" iniFile, sec, key, "NONE")
    MainGui.Add("Text", "x20 y" (yPos+3) " w210", desc ":") ; было desc " (" key "):")
    ddl := MainGui.Add("ComboBox", "x240 y" yPos " w130 Choose1", listKeys)
    SetDdlValue(ddl, val)
    btn := MainGui.Add("Button", "x380 y" (yPos-1) " w60 h22", "Bind")
    btn.OnEvent("Click", bindFunc.Bind(ddl))
    CtrlSettings[key] := {type: "ddl", ctrl: ddl, file: iniFile, sec: sec}
    yPos += 32
}

; =========================================
; TAB 5: HOTKEYS (config.ini)
; =========================================
Tabs.UseTab(5)
yPos := 55

MainGui.Add("Text", "x20 y" yPos " w450 cBlue", "--- [Motion] (Config.ini) ---")
yPos += 35
AddHotkey(ConfigIni, "Motion", "AimingToggleButton", "Gyro Aiming On\Off  ", JslKeys, BindGamepad)
yPos += 10
AddHotkey(ConfigIni, "Motion", "AimingButton", "Gyro move button", JslKeys, BindGamepad)
yPos += 10
AddHotkey(ConfigIni, "Motion", "AimingModeToggleButton", "Switch Aiming Mode (Mouse\Stcik)", JslKeys, BindGamepad)
yPos += 15
AddHotkey(ConfigIni, "Motion", "DrivingToggleButton", "Driving Mode On\Off", JslKeys, BindGamepad)

yPos += 20
MainGui.Add("Text", "x20 y" yPos " w450 cBlue", "--- [Gamepad] (Config.ini) ---")
yPos += 40
AddHotkey(ConfigIni, "Gamepad", "ResetKey", "Reset/Research Gamepad (Keyboard)", KbmKeys, BindKbm)

yPos += 40
MainGui.Add("Text", "x20 y" yPos " w820 cRed", "* Note:")
yPos += 20
MainGui.Add("Text", "x20 y" yPos " w450", T("To assign a two-button combination (like R+HOME), you `ncan manually type it into the field above and click Save All"))

; =========================================
; TAB 6: GYRO (config.ini)
; =========================================
Tabs.UseTab(6)
yPos := 55

MainGui.Add("Text", "x20 y" yPos " w450 cBlue", "--- [Motion] (Config.ini) ---")
yPos += 30
AddMappedDropdown(ConfigIni, "Motion", "AimingMode", "Gyro mode by default", ["Right Stick", "Mouse"], Map("Right Stick", "0", "Mouse", "1"))
;yPos += 5
AddMappedDropdown(ConfigIni, "Motion", "AimingByPressingMode", "Mouse\Stick moves when", ["not pressing AimButton", "pressing AimButton"], Map("not pressing AimButton", "0", "pressing AimButton", "1"))
;yPos += 5
AddMappedDropdown(ConfigIni, "Motion", "GyroFromLeft", "Read Gyro data from", ["Right Joy-Con", "Left Joy-Con"], Map("Right Joy-Con", "0", "Left Joy-Con", "1"))
;yPos += 5
AddMappedDropdown(ConfigIni, "Gamepad", "SleepTimeOut", "Polling rate (33.3 Hz for example)", ["33.3 Hz", "66.7 Hz", "125 Hz", "250 Hz"], Map("33.3 Hz", "30", "66.7 Hz", "15", "125 Hz", "8", "250 Hz", "4"))
;yPos += 5
AddMappedDropdown(ConfigIni, "Motion", "GyroSpace", "Gyro Space *", ["0", "1", "2"], Map("0", "0", "1", "1", "2", "2"))

;AddToggle(ConfigIni, "Motion", "GyroFromLeft", "Read Gyro from Left Joy-Con (Default - Right)")

;yPos += 10
;MainGui.Add("Text", "x20 y" yPos " w450 cBlue", "--- Sensitivity ---")
yPos += 10
AddInput(ConfigIni, "Motion", "MouseSensX", "Gyro Sensitivity for Mouse X")
AddInput(ConfigIni, "Motion", "MouseSensY", "Gyro Sensitivity for Mouse Y")
yPos += 15
AddInput(ConfigIni, "Motion", "JoySensX", "Gyro Sensitivity for Stick X")
AddInput(ConfigIni, "Motion", "JoySensY", "Gyro Sensitivity for Stick Y")
yPos += 15
AddInput(ConfigIni, "Motion", "MouseSmooth", "EMA** smooth filter for Mouse")
AddInput(ConfigIni, "Motion", "StickSmooth", "EMA** smooth filter for Stick")
yPos += 15
AddInput(ConfigIni, "Motion", "SteeringWheelAngle", "Steering wheel angle (Driving Mode)")
yPos += 5
MainGui.Add("Text", "x20 y" yPos " w820 cRed", "* Gyro Motion Space:")
yPos += 20
MainGui.Add("Text", "x20 y" yPos " w820", T("This option controls how the gyroscope converts hand moves into mouse\stick movements depending  on the `ntilt of the wrist (roll - clockwise or anti-clockwise) and how you hold a gamepad (face buttons to you or horizontal)"))
;MainGui.Add("Text", "x20 y" yPos " w820", T("0 - Movement depends on the rotation of the hand (default in DSAdvance) for all grip"))
yPos += 35
MainGui.Add("Text", "x20 y" yPos " w820", T("For two-hands gamepads use 0 or 2:"))
yPos += 15
MainGui.Add("Text", "x20 y" yPos " w820", T("0 — Steer left like a wheel to move cursor/stcik to the left"))
yPos += 15
MainGui.Add("Text", "x20 y" yPos " w820", T("2 — Push your right hand forward to move cursor/stick to the left"))
yPos += 20
MainGui.Add("Text", "x20 y" yPos " w820", T("For Joy-cons use 1 or 0:"))
yPos += 15
MainGui.Add("Text", "x20 y" yPos " w820", T("1 — Ignores wrist rotation (roll) within -90 to 90 degrees; Minimal interference from other axes in horizontal grip"))
yPos += 15
MainGui.Add("Text", "x20 y" yPos " w820", T("0 — Movement is affected by wrist rotation, regardless of your grip"))
;yPos += 20
;MainGui.Add("Text", "x20 y" yPos " w820", T("1 - Movement not affected by the rotation of the hand (-90 90 degrees) for horizontal grip"))
;yPos += 20
;MainGui.Add("Text", "x20 y" yPos " w820", T("2 - ? Try each mode and choose the one. For example: 1 - for joy-con's, 0 - for others"))
yPos += 25
MainGui.Add("Text", "x20 y" yPos " w820 cRed", T("** Caution:"))
yPos += 20
MainGui.Add("Text", "x20 y" yPos " w820", T("EMA smooth filter add input latency. For 60fps games (value - latency): 25   ~2.7ms;  50   ~8ms;  75   ~24ms"))

; =========================================
; TAB 7: SETTINGS 2 (config.ini)
; =========================================
Tabs.UseTab(7)

y1 := 90 ; Высота элементов для левой колонки
y2 := 90 ; Высота элементов для правой колонки

; --- Локальные функции для авторазметки левой колонки ---
AddInputCol1(key, desc, defaultVal := "0") {
    global y1
    val := IniRead(A_ScriptDir "\" ConfigIni, "Gamepad", key, defaultVal)
    MainGui.Add("Text", "x20 y" (y1+3) " w210", desc ":")
    edt := MainGui.Add("Edit", "x240 y" y1 " w60", val)
    CtrlSettings[key] := {type: "edt", ctrl: edt, file: ConfigIni, sec: "Gamepad"}
    y1 += 30
}

AddToggleCol1(iniFile, sec, key, desc) {
    global y1
    val := IniRead(A_ScriptDir "\" iniFile, sec, key, "0")
    chkOpt := (val = "1") ? " Checked1" : ""
    chk := MainGui.Add("Checkbox", "x20 y" y1 chkOpt, desc)
    CtrlSettings[key] := {type: "chk", ctrl: chk, file: iniFile, sec: sec}
    y1 += 30
}

; --- Локальные функции для авторазметки правой колонки ---
AddInputCol2(key, desc, defaultVal := "0") {
    global y2
    val := IniRead(A_ScriptDir "\" ConfigIni, "Gamepad", key, defaultVal)
    MainGui.Add("Text", "x430 y" (y2+3) " w210", desc ":")
    edt := MainGui.Add("Edit", "x650 y" y2 " w60", val)
    CtrlSettings[key] := {type: "edt", ctrl: edt, file: ConfigIni, sec: "Gamepad"}
    y2 += 30
}

AddToggleCol2(iniFile, sec, key, desc) {
    global y2
    val := IniRead(A_ScriptDir "\" iniFile, sec, key, "0")
    chkOpt := (val = "1") ? " Checked1" : ""
    chk := MainGui.Add("Checkbox", "x430 y" y2 chkOpt, desc)
    CtrlSettings[key] := {type: "chk", ctrl: chk, file: iniFile, sec: sec}
    y2 += 30
}

; ЛЕВАЯ КОЛОНКА (Левая рука, Вибрация и Свопы)
MainGui.Add("Text", "x20 y55 w350 cBlue", "--- LEFT HAND (Config.ini) ---")

AddInputCol1("DeadZoneLeftStickX", "DeadZone Left Stick X")
AddInputCol1("DeadZoneLeftStickY", "DeadZone Left Stick Y")
y1 += 10
AddInputCol1("LinearityLeftStickX", "Linearity* Left Stick X (0-100)", "50")
AddInputCol1("LinearityLeftStickY", "Linearity* Left Stick Y (0-100)", "50")
y1 += 10
AddInputCol1("DeadZoneLeftTrigger", "DeadZone Left Trigger") ; Сюда переехал левый триггер

y1 += 10 ; Небольшой отступ перед чекбоксами левой руки
AddToggleCol1(ConfigIni, "Gamepad", "InvertLeftStickX", "Invert Left Stick X")
AddToggleCol1(ConfigIni, "Gamepad", "InvertLeftStickY", "Invert Left Stick Y")

y1 += 10 ; Небольшой отступ перед силой вибрации
AddInputCol1("RumbleStrength", "Rumble strength (0 to 100)") ; Вибрация ушла вниз блока Config.ini

y1 += 15 ; Небольшой отступ перед секцией default.ini
MainGui.Add("Text", "x20 y" y1 " w350 cBlue", "--- HARDWARE SWAPS (default.ini) ---")
y1 += 30
AddToggleCol1(XboxIni, "SETTINGS", "SWAP-STICKS", "Swap Left and Right Sticks")
AddToggleCol1(XboxIni, "SETTINGS", "SWAP-TRIGGERS", "Swap Left and Right Triggers")

y1 += 30
MainGui.Add("Text", "x20 y" y1 " w820 cRed", T("* Stick Linearity"))
y1 += 25
MainGui.Add("Text", "x20 y" y1 " w820", T("Adjusts stick sensitivity curve:"))
y1 += 25
MainGui.Add("Text", "x20 y" y1 " w820", T("0: Lower sensitivity near the center for precise aiming (Exponential)"))
y1 += 25
MainGui.Add("Text", "x20 y" y1 " w820", T("50 (Default): Perfectly linear response."))
y1 += 25
MainGui.Add("Text", "x20 y" y1 " w820", T("100: Higher sensitivity near the center for instant response (Logarithmic)"))

; ПРАВАЯ КОЛОНКА (Правая рука и Правый триггер)
MainGui.Add("Text", "x430 y55 w350 cBlue", "--- RIGHT HAND (Config.ini) ---")

AddInputCol2("DeadZoneRightStickX", "DeadZone Right Stick X")
AddInputCol2("DeadZoneRightStickY", "DeadZone Right Stick Y")
y2 += 10
AddInputCol2("LinearityRightStickX", "Linearity* Right Stick X (0-100)", "50")
AddInputCol2("LinearityRightStickY", "Linearity* Right Stick Y (0-100)", "50")
y2 += 10
AddInputCol2("DeadZoneRightTrigger", "DeadZone Right Trigger") ; Здесь остался правый триггер

y2 += 10 ; Небольшой отступ перед чекбоксами правой руки
AddToggleCol2(ConfigIni, "Gamepad", "InvertRightStickX", "Invert Right Stick X")
AddToggleCol2(ConfigIni, "Gamepad", "InvertRightStickY", "Invert Right Stick Y")

; =========================================
; TAB 8: PROFILES
; =========================================
Tabs.UseTab(8)
MainGui.Add("Text", "x20 y65 w810 Center", "Profile Manager")

; --- БЛОК 1: ТЕКУЩИЙ ПРОФИЛЬ И ВЫБОР ---
MainGui.Add("GroupBox", "x20 y120 w790 h140 cBlue", T("Select Active Profile"))
MainGui.Add("Text", "x40 y160 w450", T("Current Active Profile:  " ActiveProfile))

; Сканируем папку XboxProfiles и собираем список файлов .ini
ProfileList := []
Loop Files, A_ScriptDir "\XboxProfiles\*.ini", "F" {
    ProfileList.Push(A_LoopFileName)
}

MainGui.Add("Text", "x40 y203 w130", T("Select Profile to Load:"))
ProfileDdl := MainGui.Add("DropDownList", "x170 y200 w180 Choose1", ProfileList)
ProfileDdl.Text := ActiveProfile

LoadProfileBtn := MainGui.Add("Button", "x370 y198 w120 h26", "Load Profile")
LoadProfileBtn.OnEvent("Click", (*) => LoadProfileEvent(ProfileDdl.Text))

LoadProfileEvent(selectedProfile) {
    if (selectedProfile == "")
        return
    IniWrite(selectedProfile, A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile")
    Reload() ; Мгновенный перезапуск для чистого обновления всего интерфейса
}

; --- БЛОК 2: СОЗДАНИЕ НОВОГО ПРОФИЛЯ ---
MainGui.Add("GroupBox", "x20 y280 w790 h140 cBlue", T("Create New Profile"))
MainGui.Add("Text", "x40 y330 w400", T("Enter Name for the New Profile:"))

NewProfileEdit := MainGui.Add("Edit", "x40 y365 w250")
CreateProfileBtn := MainGui.Add("Button", "x310 y363 w150 h26", "Create and Load Profile")
CreateProfileBtn.OnEvent("Click", (*) => CreateProfileEvent(NewProfileEdit.Text))

CreateProfileEvent(profileName) {
    profileName := Trim(profileName)
    if (profileName == "") {
        MsgBox("Please enter a valid profile name!", "Error", "Icon!")
        return
    }
    
    ; Добавляем .ini если пользователь забыл его ввести
    if (SubStr(profileName, -4) != ".ini")
        profileName .= ".ini"
        
    targetFile := A_ScriptDir "\XboxProfiles\" profileName
    
    if FileExist(targetFile) {
        MsgBox("Profile with this name already exists!", "Error", "Icon!")
        return
    }
    
    try {
        ; Копируем default.ini со всей его структурой и твоими комментариями
        FileCopy(A_ScriptDir "\XboxProfiles\default.ini", targetFile, 1)
        
        ; Обнуляем все кнопки во всех маппинг-секциях
        For key in JslKeys {
            if (key != "NONE") {
                IniWrite("NONE", targetFile, "Xbox", key)
                IniWrite("NONE", targetFile, "JOYCONS", key)
                IniWrite("NONE", targetFile, "DUALSENSE-EDGE", key)
                IniWrite("NONE", targetFile, "KEYBOARD-MOUSE", key)
            }
        }
        
        ; Сразу записываем его как активный и перезапускаем
        IniWrite(profileName, A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile")
        MsgBox("Profile '" profileName "' created successfully!", "Success", "Iconi")
        Reload()
    } catch as err {
        MsgBox("Failed to create profile!`nDetails: " err.Message, "Error", "IconX")
    }
}

; =========================================
; Other stuff
; =========================================
; Отключаем привязку ко вкладкам, чтобы кнопка стала общей для всего окна
Tabs.UseTab() 

; Вспомогательная функция для безопасной смены раскладки в AHK v2
SetLayout(value, *) {
    global Layout
    Layout := value
}

; Переключатели раскладки кнопок (Nintendo / Sony) слева внизу
MainGui.Add("Text", "x20 y725 w220", "Layout for Bind buttons:")

chkNintendo := (Layout == "Nintendo") ? " Checked1" : ""
chkSony := (Layout == "Sony") ? " Checked1" : ""

radNintendo := MainGui.Add("Radio", "x175 y725" chkNintendo, "Nintendo")
radSony := MainGui.Add("Radio", "x255 y725" chkSony, "Sony")

; Назначаем события клика через вызов нашей функции
radNintendo.OnEvent("Click", (*) => SetLayout("Nintendo"))
radSony.OnEvent("Click", (*) => SetLayout("Sony"))

; Сканируем папку Languages и собираем список доступных языков
LanguagesList := ["English"]
Loop Files, A_ScriptDir "\Config_lang\*.ini", "F" {
    ; Берем имя файла без расширения .ini
    LanguagesList.Push(StrReplace(A_LoopFileName, ".ini", ""))
}

; Выбор языка внизу посередине экрана
MainGui.Add("Text", "x390 y725 w80", T("Language:"))
LangDdl := MainGui.Add("DropDownList", "x460 y723 w120", LanguagesList)
LangDdl.Text := CurrentLang
LangDdl.OnEvent("Change", (ctrl, *) => ChangeLanguageEvent(ctrl.Text))

ChangeLanguageEvent(selectedLang) {
    IniWrite(selectedLang, A_ScriptDir "\" ConfigIni, "ConfigGUI", "Language")
    Reload() ; Перезапускаем скрипт для мгновенного применения языка
}

; КНОПКА SAVE СДВИНУТА В САМЫЙ НИЗ, ПОД ВКЛАДКИ
SaveBtn := MainGui.Add("Button", "x720 y720 w120 h25 Default", "Save All")
SaveBtn.OnEvent("Click", SaveAllConfigs)

MainGui.Show()

; =========================================
; SAVE LOGIC
; =========================================
SaveAllConfigs(*) {
    try {
        ; --- 1. Очистка старых биндов Xbox (ИН-ПЛЕЙС, БЕЗ УДАЛЕНИЯ СТРОК!) ---
        For sec in ["Xbox", "JOYCONS", "DUALSENSE-EDGE"] {
            secText := ""
            try secText := IniRead(A_ScriptDir "\" XboxIni, sec)
            if (secText != "") {
                Loop Parse secText, "`n", "`r" {
                    parts := StrSplit(A_LoopField, "=")
                    if (parts.Length == 2) {
                        val := Trim(parts[2])
                        keyToClear := Trim(parts[1])
                        
                        isXboxBtn := false
                        for x in XboxMapping {
                            if (val == x) {
                                isXboxBtn := true
                                break
                            }
                        }
                        
                        ; Умная проверка: очищаем кнопку только если она принадлежит текущей активной раскладке
                        shouldClear := false
                        if (Layout == "Sony") {
                            for k in SonyMapping {
                                if (keyToClear == k) {
                                    shouldClear := true
                                    break
                                }
                            }
                        } else {
                            for k in JoyconMapping {
                                if (keyToClear == k) {
                                    shouldClear := true
                                    break
                                }
                            }
                        }

                        if (isXboxBtn && shouldClear)
                            IniWrite("NONE", A_ScriptDir "\" XboxIni, sec, keyToClear)
                    }
                }
            }
        }
        
        ; --- 2. Запись Вкладки XBOX ---
        for key in XboxMapping {
            ctrl := CtrlXbox[key]
            btn := ctrl.Text
            if (btn != "NONE" && btn != "") {
                if (btn == "SL" || btn == "SR" || btn == "HOME" || btn == "CAPTURE")
                    IniWrite(key, A_ScriptDir "\" XboxIni, "JOYCONS", btn)
                else if (btn == "L4" || btn == "R4")
                    IniWrite(key, A_ScriptDir "\" XboxIni, "DUALSENSE-EDGE", btn)
                else
                    IniWrite(key, A_ScriptDir "\" XboxIni, "Xbox", btn) 
            }
        }

        ; --- 3. Вкладка JOY-CON ---
        for key in JoyconMapping {
            ctrl := CtrlJoyCon[key]
            val := (ctrl.Text != "") ? ctrl.Text : "NONE"
            IniWrite(val, A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key)
        }
        
		; --- 4. Вкладка SONY ---
        for key in SonyMapping {
            ctrl := CtrlSony[key]
            val := (ctrl.Text != "") ? ctrl.Text : "NONE"
            IniWrite(val, A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key)
        }
		
        ; --- 5. Вкладка WHEEL ---
        valAct := (CtrlWheel["WHEEL-ACTIVATION"].ddl.Text != "") ? CtrlWheel["WHEEL-ACTIVATION"].ddl.Text : "NONE"
        IniWrite(valAct, A_ScriptDir "\" XboxIni, "Motion", "WHEEL-ACTIVATION")
        
        for key in WheelMapping {
            obj := CtrlWheel[key]
            val := (obj.ddl.Text != "") ? obj.ddl.Text : "NONE"
            if (obj.rKbm.Value == 1) {
                IniWrite(val, A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key)
                IniWrite("NONE", A_ScriptDir "\" XboxIni, "Motion", key)
            } else {
                IniWrite(val, A_ScriptDir "\" XboxIni, "Motion", key)
                IniWrite("NONE", A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key)
            }
        }
            
        ; --- 6. Вкладки SETTINGS ---
        for key, obj in CtrlSettings {
            if (obj.type == "chk")
                val := obj.ctrl.Value ? "1" : "0"
            else if (obj.type == "mapped_ddl")
                val := obj.valMap[obj.ctrl.Text]
            else
                val := (obj.ctrl.Text != "") ? obj.ctrl.Text : "NONE"
            IniWrite(val, A_ScriptDir "\" obj.file, obj.sec, key)
        }
        
		; Сохраняем выбранную раскладку
        IniWrite(Layout, A_ScriptDir "\" ConfigIni, "ConfigGUI", "Layout")
		
		; Сохраняем имя текущего активного профиля
        IniWrite(ActiveProfile, A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile")
		
        MsgBox("Settings successfully saved!", "Success")
    } catch as err {
        MsgBox("Error writing to INI file!`nDetails: " err.Message, "Save Error")
    }
}

; =========================================
; BIND FUNCTIONS
; =========================================

TranslateAhkKey(k) {
    if (k == "LCONTROL")
        return "LCTRL"
    if (k == "RCONTROL")
        return "RCTRL"
    if (k == "LWIN" || k == "RWIN")
        return "WIN"
    if (k == "LALT")
        return "LALT"
    if (k == "RALT")
        return "RALT"
    if (k == "RETURN")
        return "ENTER"
    if (k == "SPACE")
        return "SPACE"
    if (k == "CAPITAL")
        return "CAPS-LOCK"
    if (k == "OEM_3" || k == "``")
        return "~"
    if (k == "OEM_MINUS")
        return "-"
    if (k == "OEM_PLUS")
        return "="
    if (k == "OEM_4")
        return "["
    if (k == "OEM_6")
        return "]"
    if (k == "OEM_1")
        return ":"
    if (k == "OEM_7")
        return "APOSTROPHE"
    if (k == "OEM_5")
        return "\"
    if (k == "OEM_COMMA")
        return "<"
    if (k == "OEM_PERIOD")
        return ">"
    if (k == "OEM_2")
        return "?"
    if (k == "PRIOR")
        return "PAGE-UP"
    if (k == "NEXT")
        return "PAGE-DOWN"
    if (k == "SCROLL")
        return "SCROLL-LOCK"
    if (k == "NUMPADINS" || k == "NUMPAD0")
        return "NUMPAD0"
    if (k == "NUMPADEND" || k == "NUMPAD1")
        return "NUMPAD1"
    if (k == "NUMPADDOWN" || k == "NUMPAD2")
        return "NUMPAD2"
    if (k == "NUMPADPGDN" || k == "NUMPAD3")
        return "NUMPAD3"
    if (k == "NUMPADLEFT" || k == "NUMPAD4")
        return "NUMPAD4"
    if (k == "NUMPADCLEAR" || k == "NUMPAD5")
        return "NUMPAD5"
    if (k == "NUMPADRIGHT" || k == "NUMPAD6")
        return "NUMPAD6"
    if (k == "NUMPADHOME" || k == "NUMPAD7")
        return "NUMPAD7"
    if (k == "NUMPADUP" || k == "NUMPAD8")
        return "NUMPAD8"
    if (k == "NUMPADPGUP" || k == "NUMPAD9")
        return "NUMPAD9"
    if (k == "NUMPADDEL")
        return "NUMPAD-DEL"
    if (k == "NUMPADDIV")
        return "NUMPAD-DIVIDE"
    if (k == "NUMPADMULT")
        return "NUMPAD-MULTIPLY"
    if (k == "NUMPADADD")
        return "NUMPAD-PLUS"
    if (k == "NUMPADSUB")
        return "NUMPAD-MINUS"
    if (k == "NUMPADENTER")
        return "NUMPAD-ENTER"
    return k
}

BindKbm(ddl, *) {
    bGui := Gui("+AlwaysOnTop -SysMenu +ToolWindow", "Waiting for input...")
    bGui.Add("Text", "w250 h50 Center +0x0200", "Press any Keyboard or Mouse key`n(ESC to cancel)")
    bGui.Show("NoActivate")

    ih := InputHook("V")
    ih.KeyOpt("{All}", "E")
    ih.Start()
    
    result := ""
    Loop {
        if GetKeyState("LButton", "P") {
            result := "MOUSE-LEFT"
            break
        }
        if GetKeyState("RButton", "P") {
            result := "MOUSE-RIGHT"
            break
        }
        if GetKeyState("MButton", "P") {
            result := "MOUSE-MIDDLE"
            break
        }
        if GetKeyState("WheelUp", "P") {
            result := "MOUSE-WHEEL-UP"
            break
        }
        if GetKeyState("WheelDown", "P") {
            result := "MOUSE-WHEEL-DOWN"
            break
        }
        
        if (ih.InProgress = 0) {
            if (ih.EndKey != "" && ih.EndKey != "Escape") {
                result := TranslateAhkKey(StrUpper(ih.EndKey))
            }
            break
        }
        Sleep(20)
    }
    ih.Stop()
    bGui.Destroy()

    if (result != "") {
        SetDdlValue(ddl, result)
    }
}

BindGamepad(ddl, *) {
    if (hModule == 0) {
        MsgBox("JoyShockLibrary.dll not found in the script folder or blocked by Windows!", "Error")
        return
    }

    ; 1. МГНОВЕННО создаем и показываем окно статуса
    bGui := Gui("+AlwaysOnTop -SysMenu +ToolWindow", "Connecting...")
    infoText := bGui.Add("Text", "w250 h50 Center +0x0200", "Connecting to gamepads...`n(Please wait up to 5s)")
    bGui.Show("NoActivate")
    
    ; Даем окну 50мс, чтобы физически прорисоваться на экране до блокирующего вызова DLL
    Sleep(50) 

    ; 2. Запускаем тяжелый поиск устройств в DLL
    DllCall("JoyShockLibrary.dll\JslConnectDevices", "Cdecl")
    
    handles := Buffer(16, 0)
    count := DllCall("JoyShockLibrary.dll\JslGetConnectedDeviceHandles", "Ptr", handles, "Int", 4, "Cdecl Int")
    
    if (count == 0) {
        bGui.Destroy()
        MsgBox("No gamepads found!`nEnsure JCAdvance is closed and gamepad is connected.", "Warning")
        return
    }
    
    ; 3. Обновляем заголовок и текст окна на ожидание нажатия кнопки
    bGui.Title := "Waiting for input..."
    infoText.Text := "Press any Gamepad button...`n(Timeout: 5 sec)"

    detectedBtn := ""
    timeout := A_TickCount + 5000
    
    Loop {
        if (A_TickCount > timeout)
            break
            
        Loop count {
            idx := A_Index - 1
            devId := NumGet(handles, idx * 4, "Int")
            stateMask := DllCall("JoyShockLibrary.dll\JslGetButtons", "Int", devId, "Cdecl Int")
            if (stateMask > 0) {
                detectedBtn := ParseJslMask(stateMask)
                if (detectedBtn != "")
                    break 2
            }
        }
        Sleep(50)
    }
    
    bGui.Destroy()
    DllCall("JoyShockLibrary.dll\JslDisconnectAndDisposeAll", "Cdecl")
    
    if (detectedBtn != "") {
        SetDdlValue(ddl, detectedBtn)
    }
}

ParseJslMask(mask) {
    if (mask & 0x00001)
        return "UP"
    if (mask & 0x00002)
        return "DOWN"
    if (mask & 0x00004)
        return "LEFT"
    if (mask & 0x00008)
        return "RIGHT"
        
    ; Системные кнопки (+ / - и Options / Share)
    if (mask & 0x00010)
        return (Layout == "Sony") ? "OPTIONS" : "PLUS"
    if (mask & 0x00020)
        return (Layout == "Sony") ? "SHARE" : "MINUS"
        
    if (mask & 0x00040)
        return "L3"
    if (mask & 0x00080)
        return "R3"
        
    ; Бамперы (L / R и L1 / R1)
    if (mask & 0x00100)
        return (Layout == "Sony") ? "L1" : "L"
    if (mask & 0x00200)
        return (Layout == "Sony") ? "R1" : "R"
        
    ; Аналоговые курки (ZL / ZR и L2 / R2)
    if (mask & 0x00400)
        return (Layout == "Sony") ? "L2" : "ZL"
    if (mask & 0x000800)
        return (Layout == "Sony") ? "R2" : "ZR"
        
    ; Четыре основные кнопки действия (ABXY и Крест/Круг/Квадрат/Треугольник)
    if (mask & 0x01000) 
        return (Layout == "Sony") ? "CROSS" : "B" 
    if (mask & 0x02000) 
        return (Layout == "Sony") ? "CIRCLE" : "A" 
    if (mask & 0x04000) 
        return (Layout == "Sony") ? "SQUARE" : "Y" 
    if (mask & 0x08000) 
        return (Layout == "Sony") ? "TRIANGLE" : "X" 
        
    if (mask & 0x10000)
        return "HOME"
    if (mask & 0x20000)
        return "CAPTURE"
    if (mask & 0x40000)
        return "SL"
    if (mask & 0x80000)
        return "SR"
    return ""
}