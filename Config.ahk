#Requires AutoHotkey v2.0
#SingleInstance Force

; --- GLOBAL SETTINGS ---
Global XboxIni := "XboxProfiles\default.ini"
Global ConfigIni := "config.ini"

; 2. Упорядоченный список геймпада (Common, Nintendo, Sony)
Global JslKeys := [
    "NONE", 
    "UP", "DOWN", "LEFT", "RIGHT", "L3", "R3", "L", "R", "ZL", "ZR", 
    "A", "B", "X", "Y", "MINUS", "PLUS", "SL", "SR", "CAPTURE", "HOME", 
    "CROSS", "CIRCLE", "SQUARE", "TRIANGLE", "SHARE", "OPTIONS", "L1", "R1", "L4", "R4"
]

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
Tabs := MainGui.Add("Tab3", "x10 y10 w830 h700", ["Xbox", "Joy-Con", "Wheel", "Hotkeys", "Gyro", "Settings"])

; =========================================
; TAB 1: XBOX
; =========================================
Tabs.UseTab(1)
MainGui.Add("Text", "x20 y90 w810 Center", "Mapping Nintendo\Sony buttons to XBOX virtual buttons")
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
MainGui.Add("Text", "x230 y" (yPosLeft+30) " w450 cRed", "* Note: Remapping Triggers (LT RT) for Nintendo gamepads only")

; =========================================
; TAB 2: JOY-CON
; =========================================
Tabs.UseTab(2)
MainGui.Add("Text", "x20 y45 w810 Center", "Emulate keyboard/mouse keys using the Joy-Con buttons")

MainGui.Add("Picture", "x60 y110 w134 h-1", A_ScriptDir "\Icon\joycon_left.png")
MainGui.Add("Picture", "x656 y110 w134 h-1", A_ScriptDir "\Icon\joycon_right.png")

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
MainGui.Add("Text", "x80 y" yPos " w830 cBlue", "* Also you can configure Analog Sticks directions to emulate Keyboard/Mouse keys in XboxProfiles\Default.ini")

; =========================================
; TAB 3: WHEEL
; =========================================
Tabs.UseTab(3)
MainGui.Add("Text", "x20 y45 w700", "Gyro Wheel for additional Xbox/KBM buttons mapping `nQuick press and release WHEEL-ACTIVATION button when gyro move `nSee more: https://github.com/r57zone/DSAdvance ")

yPos := 100
keyAct := "WHEEL-ACTIVATION"
valAct := IniRead(A_ScriptDir "\" XboxIni, "Motion", keyAct, "NONE")
MainGui.Add("Text", "x20 y" (yPos+4) " w140", keyAct ":")
ddlAct := MainGui.Add("ComboBox", "x280 y" yPos " w120 Choose1", JslKeys)
SetDdlValue(ddlAct, valAct)
CtrlWheel[keyAct] := {ddl: ddlAct} 
btnAct := MainGui.Add("Button", "x410 y" (yPos-1) " w60 h24", "Bind")
btnAct.OnEvent("Click", BindGamepad.Bind(ddlAct))

yPos += 40

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

AddInput(iniFile, sec, key, desc) {
    global yPos
    val := IniRead(A_ScriptDir "\" iniFile, sec, key, "0")
    MainGui.Add("Text", "x20 y" (yPos+3) " w250", desc ":") ; было desc " (" key "):")
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
; TAB 4: HOTKEYS (config.ini)
; =========================================
Tabs.UseTab(4)
yPos := 50

MainGui.Add("Text", "x20 y" yPos " w450 cBlue", "--- [Motion] (Config.ini) ---")
yPos += 30
AddHotkey(ConfigIni, "Motion", "AimingToggleButton", "Gyro Aiming On\Off  ", JslKeys, BindGamepad)
yPos += 5
AddHotkey(ConfigIni, "Motion", "AimingButton", "Pressing button to Gyro move", JslKeys, BindGamepad)
yPos += 5
AddHotkey(ConfigIni, "Motion", "AimingModeToggleButton", "Switch Aiming Mode (Mouse\Stcik)", JslKeys, BindGamepad)
yPos += 5
AddHotkey(ConfigIni, "Motion", "DrivingToggleButton", "Driving Mode On\Off", JslKeys, BindGamepad)

yPos += 20
MainGui.Add("Text", "x20 y" yPos " w450 cBlue", "--- [Gamepad] (Config.ini) ---")
yPos += 30
AddHotkey(ConfigIni, "Gamepad", "ResetKey", "Reset/Research Gamepad (Keyboard)", KbmKeys, BindKbm)

yPos += 40
MainGui.Add("Text", "x20 y" yPos " w450 cRed", "* Note: To assign a two-button combination (like R+HOME),`nyou can manually type it into the field above and click Save All")

; =========================================
; TAB 5: SETTINGS (config.ini)
; =========================================
Tabs.UseTab(5)
yPos := 50

MainGui.Add("Text", "x20 y" yPos " w450 cBlue", "Gyro Motion")
yPos += 35
AddMappedDropdown(ConfigIni, "Motion", "AimingMode", "Gyro mode by default", ["Right Stick", "Mouse"], Map("Right Stick", "0", "Mouse", "1"))
yPos += 5
AddMappedDropdown(ConfigIni, "Motion", "AimingByPressingMode", "Gyro behavior by default", ["always", "by pressing button"], Map("always", "0", "by pressing button", "1"))

yPos += 20
AddToggle(ConfigIni, "Motion", "GyroFromLeft", "Read Gyro from Left Joy-Con (Default - Right)")

yPos += 20
MainGui.Add("Text", "x20 y" yPos " w450 cBlue", "Sensitivity")
yPos += 30
AddInput(ConfigIni, "Motion", "MouseSensX", "Gyro Sensitivity for Mouse X")
AddInput(ConfigIni, "Motion", "MouseSensY", "Gyro Sensitivity for Mouse Y")
yPos += 10
AddInput(ConfigIni, "Motion", "JoySensX", "Gyro Sensitivity for Stick X")
AddInput(ConfigIni, "Motion", "JoySensY", "Gyro Sensitivity for Stick Y")
yPos += 10
AddInput(ConfigIni, "Motion", "MouseSmooth", "EMA smooth filter for Mouse")
AddInput(ConfigIni, "Motion", "StickSmooth", "EMA smooth filter for Stick")
yPos += 10
AddInput(ConfigIni, "Motion", "SteeringWheelAngle (Driving Mode)", "Steering wheel angle")
yPos += 40
MainGui.Add("Text", "x20 y" yPos " w600 cRed", "* Caution: EMA smooth filter add input latency. For 60fps games `n(value - latency): 25   ~2.7ms;  50   ~8ms;  75   ~24ms")

; =========================================
; TAB 6: SETTINGS 2 (config.ini)
; =========================================
Tabs.UseTab(6)
yPos := 50

MainGui.Add("Text", "x20 y" yPos " w450 cBlue", "--- [Gamepad] (Config.ini) ---")
yPos += 30
AddInput(ConfigIni, "Gamepad", "RumbleStrength", "Rumble strength (0 to 100)")
AddInput(ConfigIni, "Gamepad", "DeadZoneLeftStickX", "DeadZone Left Stick X")
AddInput(ConfigIni, "Gamepad", "DeadZoneLeftStickY", "DeadZone Left Stick Y")
AddInput(ConfigIni, "Gamepad", "DeadZoneRightStickX", "DeadZone Right Stick X")
AddInput(ConfigIni, "Gamepad", "DeadZoneRightStickY", "DeadZone Right Stick Y")
AddInput(ConfigIni, "Gamepad", "DeadZoneLeftTrigger", "DeadZone Left Trigger")
AddInput(ConfigIni, "Gamepad", "DeadZoneRightTrigger", "DeadZone Right Trigger")

yPos += 15
AddToggle(ConfigIni, "Gamepad", "InvertLeftStickX", "Invert Left Stick X")
AddToggle(ConfigIni, "Gamepad", "InvertLeftStickY", "Invert Left Stick Y")
AddToggle(ConfigIni, "Gamepad", "InvertRightStickX", "Invert Right Stick X")
AddToggle(ConfigIni, "Gamepad", "InvertRightStickY", "Invert Right Stick Y")

yPos += 20
MainGui.Add("Text", "x20 y" yPos " w450 cBlue", "--- [SETTINGS] (default.ini) ---")
yPos += 30
AddToggle(XboxIni, "SETTINGS", "SWAP-STICKS", "Swap Left and Right Sticks")
AddToggle(XboxIni, "SETTINGS", "SWAP-TRIGGERS", "Swap Left and Right Triggers")

; Отключаем привязку ко вкладкам, чтобы кнопка стала общей для всего окна
Tabs.UseTab() 

; КНОПКА SAVE СДВИНУТА В САМЫЙ НИЗ, ПОД ВКЛАДКИ
SaveBtn := MainGui.Add("Button", "x700 y720 w120 h40 Default", "Save All")
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
                        if (isXboxBtn)
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
            
        ; --- 4. Вкладка WHEEL ---
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
            
        ; --- 5. Вкладки SETTINGS ---
        for key, obj in CtrlSettings {
            if (obj.type == "chk")
                val := obj.ctrl.Value ? "1" : "0"
            else if (obj.type == "mapped_ddl")
                val := obj.valMap[obj.ctrl.Text]
            else
                val := (obj.ctrl.Text != "") ? obj.ctrl.Text : "NONE"
            IniWrite(val, A_ScriptDir "\" obj.file, obj.sec, key)
        }
        
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
    if !FileExist(A_ScriptDir "\JoyShockLibrary.dll") {
        MsgBox("JoyShockLibrary.dll not found in the script folder!", "Error")
        return
    }

    hModule := DllCall("LoadLibrary", "Str", A_ScriptDir "\JoyShockLibrary.dll", "Ptr")
    if (hModule == 0) {
        errCode := A_LastError
        MsgBox("Windows blocked DLL loading!`nError code: " errCode "`n(If 193 - Bitness mismatch, compile to x32!)", "Critical DLL Error")
        return
    }

    DllCall("JoyShockLibrary.dll\JslConnectDevices", "Cdecl")
    
    handles := Buffer(16, 0)
    count := DllCall("JoyShockLibrary.dll\JslGetConnectedDeviceHandles", "Ptr", handles, "Int", 4, "Cdecl Int")
    
    if (count == 0) {
        MsgBox("No gamepads found!`nEnsure JCAdvance is closed and gamepad is connected.", "Warning")
        DllCall("FreeLibrary", "Ptr", hModule)
        return
    }
    
    deviceId := NumGet(handles, 0, "Int")

    bGui := Gui("+AlwaysOnTop -SysMenu +ToolWindow", "Waiting for input...")
    bGui.Add("Text", "w250 h50 Center +0x0200", "Press any Gamepad button...`n(Timeout: 5 sec)")
    bGui.Show("NoActivate")

    detectedBtn := ""
    timeout := A_TickCount + 5000
    
    Loop {
        if (A_TickCount > timeout)
            break
            
        stateMask := DllCall("JoyShockLibrary.dll\JslGetButtons", "Int", deviceId, "Cdecl Int")
        if (stateMask > 0) {
            detectedBtn := ParseJslMask(stateMask)
            if (detectedBtn != "")
                break
        }
        Sleep(50)
    }
    
    bGui.Destroy()
    DllCall("JoyShockLibrary.dll\JslDisconnectAndDisposeAll", "Cdecl")
    DllCall("FreeLibrary", "Ptr", hModule)
    
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
    if (mask & 0x00010)
        return "PLUS"
    if (mask & 0x00020)
        return "MINUS"
    if (mask & 0x00040)
        return "L3"
    if (mask & 0x00080)
        return "R3"
    if (mask & 0x00100)
        return "L"
    if (mask & 0x00200)
        return "R"
    if (mask & 0x00400)
        return "ZL"
    if (mask & 0x00800)
        return "ZR"
    if (mask & 0x01000) 
		return "B" 
    if (mask & 0x02000) 
		return "A" 
    if (mask & 0x04000) 
		return "Y" 
    if (mask & 0x08000) 
		return "X" 
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