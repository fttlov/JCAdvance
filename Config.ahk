#Requires AutoHotkey v2.0
#SingleInstance Force

; ==============================================================================
; 1. СТАТИЧНЫЕ ГЛОБАЛЬНЫЕ МАССИВЫ (не имеют зависимостей)
; ==============================================================================
Global XboxMapping := ["UP", "DOWN", "LEFT", "RIGHT", "BACK", "START", "LS", "RS", "LB", "RB", "A", "B", "X", "Y", "LT", "RT"]

Global JoyconMapping := ["UP", "DOWN", "LEFT", "RIGHT", "L3", "R3", "L", "R", "ZL", "ZR", "B", "A", "Y", "X", "MINUS", "PLUS", "SL", "SR", "CAPTURE", "HOME"]

Global SonyMapping := ["UP", "DOWN", "LEFT", "RIGHT", "L3", "R3", "L1", "R1", "L2", "R2", "CROSS", "CIRCLE", "SQUARE", "TRIANGLE", "SHARE", "OPTIONS", "L4", "R4"]

Global WheelMapping := ["WHEEL-UP", "WHEEL-DOWN", "WHEEL-LEFT", "WHEEL-RIGHT"]

Global JslKeys := [
    "NONE", 
    "UP", "DOWN", "LEFT", "RIGHT", "L3", "R3", "L", "R", "ZL", "ZR", 
    "A", "B", "X", "Y", "MINUS", "PLUS", "SL", "SR", "CAPTURE", "HOME", 
    "CROSS", "CIRCLE", "SQUARE", "TRIANGLE", "SHARE", "OPTIONS", "L1", "R1", "L2", "R2", "L4", "R4"
]

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

Global XboxKeys := [
    "NONE", "UP", "DOWN", "LEFT", "RIGHT", "BACK", "START", "LS", "RS", "LB", "RB", "A", "B", "X", "Y", "LT", "RT",
    "LS-UP", "LS-DOWN", "LS-LEFT", "LS-RIGHT", "RS-UP", "RS-DOWN", "RS-LEFT", "RS-RIGHT"
]

Global CtrlXbox := Map()
Global CtrlJoyCon := Map()
Global CtrlSony := Map()
Global CtrlWheel := Map()
Global CtrlSettings := Map()
Global CtrlExtraXbox := Map()

; ==============================================================================
; 2. GLOBAL SETTINGS & ЧТЕНИЕ КОНФИГОВ
; ==============================================================================
Global ConfigIni := "config.ini"
Global hModule := DllCall("LoadLibrary", "Str", A_ScriptDir "\JoyShockLibrary.dll", "Ptr")
Global CurrentLang := "English"
try CurrentLang := IniRead(A_ScriptDir "\" ConfigIni, "ConfigGUI", "Language", "English")

; Читаем активный профиль из config.ini (по умолчанию default.ini)
Global ActiveProfile := "default.ini"
try ActiveProfile := IniRead(A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile", "default.ini")

Global Layout := "Nintendo"
try Layout := IniRead(A_ScriptDir "\" ConfigIni, "ConfigGUI", "Layout", "Nintendo")

; --- ЗАЩИТА: Проверяем, существует ли файл профиля физически ---
if (!FileExist(A_ScriptDir "\XboxProfiles\" ActiveProfile)) {
    foundAlternative := ""
    Loop Files, A_ScriptDir "\XboxProfiles\*.ini", "F" {
        foundAlternative := A_LoopFileName
        break
    }
    
    if (foundAlternative != "") {
        ActiveProfile := foundAlternative
        try IniWrite(ActiveProfile, A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile")
    } else {
        ActiveProfile := "default.ini"
    }
}

; Собираем путь к активному профилю
Global XboxIni := "XboxProfiles\" ActiveProfile

; ==============================================================================
; 3. ДИНАМИЧЕСКИЙ СПИСОК КЛАВИШ (Зависит от уже созданных массивов)
; ==============================================================================
Global LayoutKeys := ["NONE"]
if (Layout == "Sony") {
    for k in SonyMapping {
        LayoutKeys.Push(k)
    }
} else {
    for k in JoyconMapping {
        LayoutKeys.Push(k)
    }
}

; ==============================================================================
; 4. СИСТЕМНЫЕ ФУНКЦИИ
; ==============================================================================
SetDdlValue(ddl, val) {
    if (val == "")
        return
    try {
        ddl.Text := val
    } catch {
        ; Если кнопки нет в списке (сменился Layout), сбрасываем на NONE
        try {
            ddl.Text := "NONE"
        } catch {
            ; На случай, если в списке нет даже NONE
        }
    }
}

T(str) {
    global CurrentLang
    if (!IsSet(CurrentLang) || CurrentLang == "English")
        return str
        
    keyStr := StrReplace(str, "`r`n", "\n")
    keyStr := StrReplace(keyStr, "`n", "\n")
    keyStr := StrReplace(keyStr, "`r", "\n")
    
    translated := IniRead(A_ScriptDir "\Language\" CurrentLang ".ini", "Config", keyStr, str)
    return StrReplace(translated, "\n", "`n")
}

; =========================================
; MAIN WINDOW CREATION
; =========================================
MainGui := Gui("-MaximizeBox", "JCAdvance Config Editor")
MainGui.OnEvent("Close", (*) => ExitApp())

; --- ГЛОБАЛЬНЫЙ ШРИФТ ---
MainGui.SetFont("s10")

; --- ДИНАМИЧЕСКИЙ СПИСОК ВКЛАДОК ---
Global TabList := ["Xbox"]
if (Layout == "Sony") {
    TabList.Push("Sony")
} else {
    TabList.Push("Joy-Con")
}
for tabName in ["Special", "Hotkeys", "Gyro", "Analog", "Steering", "Profiles"] {
    TabList.Push(tabName)
}

Tabs := MainGui.Add("Tab3", "x10 y10 w830 h700", TabList)

; =========================================
; HELPERS
; =========================================

AddToggle(iniFile, sec, key, desc) {
    global yPos
    val := IniRead(A_ScriptDir "\" iniFile, sec, key, "0")
    chkOpt := (val = "1") ? " Checked1" : ""
    chk := MainGui.Add("Checkbox", "x20 y" yPos chkOpt, desc)
    CtrlSettings[key] := {type: "chk", ctrl: chk, file: iniFile, sec: sec}
    yPos += 28
}

AddMappedDropdown(iniFile, sec, key, desc, optionsArray, valMap) {
    global yPos
    val := IniRead(A_ScriptDir "\" iniFile, sec, key, "0")
    
    selectedText := optionsArray[1]
    for k, v in valMap {
        if (v == val) {
            selectedText := k
            break
        }
    }
    
    MainGui.Add("Text", "x20 y" (yPos+3) " w210", desc ":")
    ddl := MainGui.Add("DropDownList", "x240 y" yPos " w150 Choose1", optionsArray)
    ddl.Text := selectedText
    
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
    MainGui.Add("Text", "x20 y" (yPos+3) " w210", desc ":")
    ddl := MainGui.Add("ComboBox", "x240 y" yPos " w130 Choose1", listKeys)
    SetDdlValue(ddl, val)
    btn := MainGui.Add("Button", "x380 y" (yPos-1) " w60 h22", "Bind")
    btn.OnEvent("Click", bindFunc.Bind(ddl))
    CtrlSettings[key] := {type: "ddl", ctrl: ddl, file: iniFile, sec: sec}
    yPos += 32
}

; --- Логика удаления профиля ---
DeleteProfileEvent(selectedProfile) {
    global ActiveProfile, ConfigIni
    
    if (selectedProfile == "")
        return
        
    if (selectedProfile == "default.ini") {
        MsgBox(T("You cannot delete the default profile!"), T("Error"), "Icon!")
        return
    }
    
    confirm := MsgBox(T("Are you sure you want to delete profile '") selectedProfile "'?", T("Confirm Deletion"), "YesNo Icon?")
    if (confirm == "No")
        return
        
    try {
        FileDelete(A_ScriptDir "\XboxProfiles\" selectedProfile)
        
        if (selectedProfile == ActiveProfile) {
            IniWrite("default.ini", A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile")
        }
        
        MsgBox(T("Profile deleted successfully!"), T("Success"), "Iconi")
        Reload()
    } catch as err {
        MsgBox(T("Failed to delete profile!`nDetails: ") err.Message, T("Error"), "IconX")
    }
}

; =========================================
; TAB 1: XBOX
; =========================================
Tabs.UseTab("Xbox")
MainGui.Add("Text", "x20 y90 w810 Center", T("Mapping Nintendo\Sony buttons to Xbox virtual buttons"))

; --- УМНЫЙ СТАРТОВЫЙ ФИЛЬТР: Загружаем только бинды текущей активной раскладки ---
SavedXboxMap := Map()
secText := ""
try secText := IniRead(A_ScriptDir "\" XboxIni, "Xbox") ; Считываем строго секцию Xbox! [1]
if (secText != "") {
    Loop Parse secText, "`n", "`r" {
        parts := StrSplit(A_LoopField, "=")
        if (parts.Length == 2) {
            physicalKey := Trim(parts[1])
            virtualXboxBtn := Trim(parts[2])
            
            ; Проверяем, принадлежит ли физическая кнопка текущему макету
            isKeyInCurrentLayout := false
            for k in LayoutKeys {
                if (physicalKey == k) {
                    isKeyInCurrentLayout := true
                    break
                }
            }
            
            ; Записываем в карту только если кнопка принадлежит активному макету!
            if (isKeyInCurrentLayout) {
                SavedXboxMap[virtualXboxBtn] := physicalKey
            }
        }
    }
}

MainGui.Add("Picture", "x275 y220 w300 h-1", A_ScriptDir "\Icon\xbox.png")

; LEFT
XboxMapLeft := ["LT", "LB", "BACK", "LS", "UP", "DOWN", "LEFT", "RIGHT"]
yPosLeft := 160
for key in XboxMapLeft {
    val := SavedXboxMap.Has(key) ? SavedXboxMap[key] : "NONE"
    
    MainGui.Add("Text", "x45 y" (yPosLeft+4) " w45 +Right", key ":")
    
    ddl := MainGui.Add("ComboBox", "x95 y" yPosLeft " w115 Choose1", LayoutKeys)
    SetDdlValue(ddl, val)
    CtrlXbox[key] := ddl
    
    btn := MainGui.Add("Button", "x220 y" (yPosLeft-1) " w50 h24", "Bind")
    btn.OnEvent("Click", BindGamepad.Bind(ddl))
    
    yPosLeft += 42 
}

; RIGHT
XboxMapRight := ["RT", "RB", "START", "RS", "Y", "X", "B", "A"]
yPosRight := 160
for key in XboxMapRight {
    val := SavedXboxMap.Has(key) ? SavedXboxMap[key] : "NONE"
    
    ddl := MainGui.Add("ComboBox", "x640 y" yPosRight " w115 Choose1", LayoutKeys)
    SetDdlValue(ddl, val)
    CtrlXbox[key] := ddl
    
    btn := MainGui.Add("Button", "x580 y" (yPosRight-1) " w50 h24", "Bind")
    btn.OnEvent("Click", BindGamepad.Bind(ddl))
    
    MainGui.Add("Text", "x760 y" (yPosRight+4) " w45", key)
    
    yPosRight += 42
}

MainGui.Add("GroupBox", "x150 y515 w530 h135 cBlue Center", T("Extra Buttons"))

; Загружаем сохраненные значения из INI-файла
SavedExtraMap := Map()
for key in ["SL", "SR", "HOME", "CAPTURE"] {
    SavedExtraMap[key] := IniRead(A_ScriptDir "\" XboxIni, "JOYCONS", key, "NONE")
}
for key in ["L4", "R4"] {
    SavedExtraMap[key] := IniRead(A_ScriptDir "\" XboxIni, "DUALSENSE-EDGE", key, "NONE")
}

; LEFT 2
yExtra := 540
for key in ["SL", "CAPTURE", "L4"] {
    val := SavedExtraMap[key]
    MainGui.Add("Text", "x180 y" (yExtra+4) " w65 +Right", key ":")
    ddl := MainGui.Add("DropDownList", "x255 y" yExtra " w115 Choose1", XboxKeys)
    SetDdlValue(ddl, val)
    CtrlExtraXbox[key] := ddl
    yExtra += 32
}

; RIGHT 2
yExtra := 540
for key in ["SR", "HOME", "R4"] {
    val := SavedExtraMap[key]
    ddl := MainGui.Add("DropDownList", "x440 y" yExtra " w115 Choose1", XboxKeys)
    SetDdlValue(ddl, val)
    CtrlExtraXbox[key] := ddl
    MainGui.Add("Text", "x565 y" (yExtra+4) " w65", ": " key) ; Отражаем двоеточие для симметрии
    yExtra += 32
}

MainGui.Add("Text", "x20 y670 w810 Center cRed", T("* Only digital buttons can be successfully remapped"))

; =========================================
; TAB 2: JOY-CON
; =========================================
if (Layout == "Nintendo") {
    Tabs.UseTab("Joy-Con")
    MainGui.Add("Text", "x25 y45 w810 Center", T("Emulate keyboard/mouse keys using the Joy-Cons buttons"))

    MainGui.Add("Picture", "x60 y140 w134 h-1", A_ScriptDir "\Icon\joycon_left.png")
    MainGui.Add("Picture", "x656 y140 w134 h-1", A_ScriptDir "\Icon\joycon_right.png")

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
}

; =========================================
; TAB 2.1: SONY
; =========================================
if (Layout == "Sony") {
    Tabs.UseTab("Sony")
    MainGui.Add("Text", "x20 y90 w810 Center", T("Emulate keyboard/mouse keys using the Sony gamepad buttons"))

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
}

; =========================================
; TAB 3: SPECIAL (WHEEL)
; =========================================
Tabs.UseTab("Special")
MainGui.Add("Text", "x20 y55 w700", T("Gyro Wheel gestures for additional Xbox/KB+M buttons mapping. `nQuick press and release WHEEL-ACTIVATION button when gyro move:"))

; 1. Начальная координата Y
yPos := 115

; 2. WHEEL-ACTIVATION
keyAct := "WHEEL-ACTIVATION"
valAct := IniRead(A_ScriptDir "\" XboxIni, "Motion", keyAct, "NONE")
MainGui.Add("Text", "x20 y" (yPos+4) " w180", keyAct T(" Button:"))
ddlAct := MainGui.Add("ComboBox", "x280 y" yPos " w120 Choose1", LayoutKeys)
SetDdlValue(ddlAct, valAct)
CtrlWheel[keyAct] := {ddl: ddlAct} 
btnAct := MainGui.Add("Button", "x410 y" (yPos-1) " w60 h24", "Bind")
btnAct.OnEvent("Click", BindGamepad.Bind(ddlAct))


yPos += 40

; 3. Отрисовка всех направлений WheelMapping
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

    MainGui.Add("Text", "x20 y" (yPos+4) " w150", key ":")
    
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

; 4. MotionWheelButtonsDeadZone
yPos += 5
keyDead := "MotionWheelButtonsDeadZone"
valDead := IniRead(A_ScriptDir "\" ConfigIni, "Motion", keyDead, "12")
MainGui.Add("Text", "x20 y" (yPos+4) " w250", T("Wheel Gesture DeadZone") ":")
edtDead := MainGui.Add("Edit", "x280 y" yPos " w120", valDead)
CtrlSettings[keyDead] := {type: "edt", ctrl: edtDead, file: ConfigIni, sec: "Motion"}

yPos += 65

MainGui.Add("Text", "x20 y" yPos " w450", T("Gyro Melee gesture. Mainly for Joy-cons. Make a gesture: a punch, a hook or a blow hammer to press virtual button:"))

yPos += 65
	
; 5. MELEE-GESTURE
valXboxMelee := IniRead(A_ScriptDir "\" XboxIni, "Motion", "MELEE-GESTURE", "NONE")
valKbmMelee := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "MELEE-GESTURE", "NONE")

isKbmMelee := false
valMelee := "NONE"

if (valKbmMelee != "NONE" && valKbmMelee != "") {
    valMelee := valKbmMelee
    isKbmMelee := true
} else if (valXboxMelee != "NONE" && valXboxMelee != "") {
    valMelee := valXboxMelee
}

MainGui.Add("Text", "x20 y" (yPos+4) " w150", "MELEE-GESTURE:")

chkXboxMelee := isKbmMelee ? "" : " Checked1"
chkKbmMelee := isKbmMelee ? " Checked1" : ""

radXboxMelee := MainGui.Add("Radio", "x160 y" (yPos+3) chkXboxMelee, "Xbox")
radKbmMelee := MainGui.Add("Radio", "x220 y" (yPos+3) chkKbmMelee, "KB/M")

ddlMelee := MainGui.Add("ComboBox", "x280 y" yPos " w120 Choose1", isKbmMelee ? KbmKeys : XboxKeys)
SetDdlValue(ddlMelee, valMelee)
CtrlWheel["MELEE-GESTURE"] := {ddl: ddlMelee, rXbox: radXboxMelee, rKbm: radKbmMelee}

radXboxMelee.OnEvent("Click", ChangeWheelList.Bind(ddlMelee, XboxKeys))
radKbmMelee.OnEvent("Click", ChangeWheelList.Bind(ddlMelee, KbmKeys))

; 6. MeleeGForce
yPos += 35
valForce := IniRead(A_ScriptDir "\" ConfigIni, "Motion", "MeleeGForce", "5.0")
MainGui.Add("Text", "x20 y" (yPos+3) " w250", T("Melee Gesture Force (g)") ":")
edtForce := MainGui.Add("Edit", "x280 y" yPos " w120", valForce)
CtrlSettings["MeleeGForce"] := {type: "edt", ctrl: edtForce, file: ConfigIni, sec: "Motion"}

ChangeWheelList(ddl, listArray, *) {
    val := ddl.Text
    ddl.Delete()
    ddl.Add(listArray)
    SetDdlValue(ddl, val)
}

; =========================================
; TAB 4: HOTKEYS (config.ini)
; =========================================
Tabs.UseTab("Hotkeys")
yPos := 55

MainGui.Add("Text", "x20 y" yPos " w450 cBlue", T("--- Gamepad Hotkeys ---"))
yPos += 55
AddHotkey(ConfigIni, "Motion", "AimingToggleButton", T("Gyro Motion (On/Off)"), LayoutKeys, BindGamepad)
yPos += 10
AddHotkey(ConfigIni, "Motion", "AimingButton", T("Motion Control button"), LayoutKeys, BindGamepad)
yPos += 10
AddHotkey(ConfigIni, "Motion", "AimingModeToggleButton", T("Switch mode (Mouse/Stick)"), LayoutKeys, BindGamepad)
yPos += 15
AddHotkey(ConfigIni, "Motion", "DrivingToggleButton", T("Driving Mode (On/Off)"), LayoutKeys, BindGamepad)
yPos += 15
AddHotkey(ConfigIni, "Motion", "DrivingCalibrationButton", T("Wheel Centering / Recalibration"), LayoutKeys, BindGamepad)

yPos += 45
MainGui.Add("Text", "x20 y" yPos " w450 cBlue", T("--- Keyboard Hotkeys ---"))
yPos += 55
AddHotkey(ConfigIni, "Gamepad", "ResetKey", T("Reset/Research Gamepad (Keyboard)"), KbmKeys, BindKbm)

yPos += 195
MainGui.Add("Text", "x20 y" yPos " w820 cRed", T("* Note:"))
yPos += 25
MainGui.Add("Text", "x20 y" yPos " w450", T("To assign a two-button combination (like R+HOME), you can manually type it into the field above and click Save All"))

; =========================================
; TAB 5: GYRO (config.ini)
; =========================================
Tabs.UseTab("Gyro")
yPos := 55

MainGui.Add("Text", "x20 y" yPos " w450 cBlue", T("--- Behavior and Settings ---"))
yPos += 30
AddMappedDropdown(ConfigIni, "Motion", "AimingMode", T("Gyro mode by default"), [T("Right Stick"), T("Mouse")], Map(T("Right Stick"), "0", T("Mouse"), "1"))
AddMappedDropdown(ConfigIni, "Motion", "AimingByPressingMode", T("Press Control button to"), [T("stop motion tracking"), T("start motion tracking")], Map(T("stop motion tracking"), "0", T("start motion tracking"), "1"))
AddMappedDropdown(ConfigIni, "Motion", "GyroFromLeft", T("Gyro data in combined mode from"), [T("Right Joy-Con"), T("Left Joy-Con")], Map(T("Right Joy-Con"), "0", T("Left Joy-Con"), "1"))
AddMappedDropdown(ConfigIni, "Gamepad", "SleepTimeOut", T("Polling rate (33.3 Hz for example)"), ["33.3 Hz", "66.7 Hz", "125 Hz", "250 Hz"], Map("33.3 Hz", "30", "66.7 Hz", "15", "125 Hz", "8", "250 Hz", "4"))
AddMappedDropdown(ConfigIni, "Motion", "GyroSpace", T("Gyro Motion Space *"), ["0", "1", "2"], Map("0", "0", "1", "1", "2", "2"))
yPos += 10
MainGui.Add("Text", "x20 y" yPos " w450 cBlue", T("--- Sensitivity ---"))
yPos += 35
AddInput(XboxIni, "SETTINGS", "MouseSensX", T("Mouse X"), "160")
AddInput(XboxIni, "SETTINGS", "MouseSensY", T("Mouse Y"), "150")
yPos += 10
AddInput(XboxIni, "SETTINGS", "JoySensX", T("Stick X"), "100")
AddInput(XboxIni, "SETTINGS", "JoySensY", T("Stick Y"), "90")
yPos += 10
AddInput(ConfigIni, "Motion", "MouseSmooth", T("EMA** smooth filter for Mouse"))
AddInput(ConfigIni, "Motion", "StickSmooth", T("EMA** smooth filter for Stick"))
yPos += 5
;AddInput(ConfigIni, "Motion", "SteeringWheelAngle", T("Steering wheel angle (Driving Mode)"))
MainGui.Add("Text", "x20 y" yPos " w820 cRed", "* Gyro Space:")
yPos += 20
MainGui.Add("Text", "x20 y" yPos " w820", T("This setting controls how gyroscope data from hand movements is processed and translated into cursor or stick input."))
yPos += 20
MainGui.Add("Text", "x20 y" yPos " w820", T("For two-handed controllers, the difference only affects horizontal (X-axis) aiming. To move the cursor/stick left or right:"))
yPos += 20
MainGui.Add("Text", "x20 y" yPos " w820", T("0 — Turn the controller like a car steering wheel (Roll)"))
yPos += 15
MainGui.Add("Text", "x20 y" yPos " w820", T("2 — Twist the controller like tank steering levers (Yaw)"))
yPos += 20
MainGui.Add("Text", "x20 y" yPos " w820", T("For Joy-Cons, different rules apply. This setting dictates how wrist angle (clockwise/counter-clockwise Roll) will skew `ncursor/stick movement relative to your arm's motion. In any case, rotating the wrist always has a negative effect."))
yPos += 30
MainGui.Add("Text", "x20 y" yPos " w820", T("The best way to use the Joy-Con for gyro aiming is the horizontal grip. This minimises interference from the other axes."))
yPos += 20
MainGui.Add("Text", "x20 y" yPos " w820", T("1 — Wrist angle between -90 and 90 degrees has no effect and the cursor/stick accurately follows your hand"))
yPos += 15
MainGui.Add("Text", "x20 y" yPos " w820", T("0 — Wrist angle always affects to the cursor/stick’s movement relative to the movement of the hand"))
yPos += 25
MainGui.Add("Text", "x20 y" yPos " w820 cRed", T("** Caution:"))
yPos += 20
MainGui.Add("Text", "x20 y" yPos " w820", T("EMA smooth filter add input latency. For 60fps games (value - latency): 25   ~2.7ms;  50   ~8ms;  75   ~24ms"))

; =========================================
; TAB 6:Analog (config.ini)
; =========================================
Tabs.UseTab("Analog")

y1 := 90
y2 := 90

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

MainGui.Add("Text", "x20 y55 w350 cBlue", T("--- LEFT HAND ---"))

AddInputCol1("DeadZoneLeftStickX", T("DeadZone Left Stick X"))
AddInputCol1("DeadZoneLeftStickY", T("DeadZone Left Stick Y"))
y1 += 10
AddInputCol1("DeadZoneLeftTrigger", T("DeadZone Left Trigger"))
y1 += 10
AddInputCol1("LinearityLeftStickX", T("Linearity* Left Stick X (0-100)"), "50")
AddInputCol1("LinearityLeftStickY", T("Linearity* Left Stick Y (0-100)"), "50")
y1 += 10
AddToggleCol1(ConfigIni, "Gamepad", "InvertLeftStickX", T("Invert Left Stick X"))
AddToggleCol1(ConfigIni, "Gamepad", "InvertLeftStickY", T("Invert Left Stick Y"))
y1 += 10
AddInputCol1("RumbleStrength", T("Rumble strength (0 to 100)"))
y1 += 15

MainGui.Add("Text", "x20 y" y1 " w350 cBlue", T("--- HARDWARE SWAPS ---"))

y1 += 30
AddToggleCol1(XboxIni, "SETTINGS", "SWAP-STICKS", T("Swap Left and Right Sticks"))
AddToggleCol1(XboxIni, "SETTINGS", "SWAP-TRIGGERS", T("Swap Left and Right Triggers"))
y1 += 100
MainGui.Add("Text", "x20 y" y1 " w820 cRed", T("* Linearity"))
y1 += 25
MainGui.Add("Text", "x20 y" y1 " w820", T("Adjusts stick sensitivity curve:"))
y1 += 25
MainGui.Add("Text", "x20 y" y1 " w820", T("0: Lower sensitivity near the center for precise aiming (Exponential)"))
y1 += 25
MainGui.Add("Text", "x20 y" y1 " w820", T("50 (Default): Perfectly linear response"))
y1 += 25
MainGui.Add("Text", "x20 y" y1 " w820", T("100: Higher sensitivity near the center for instant response (Logarithmic)"))

MainGui.Add("Text", "x430 y55 w350 cBlue", T("--- RIGHT HAND ---"))

AddInputCol2("DeadZoneRightStickX", T("DeadZone Right Stick X"))
AddInputCol2("DeadZoneRightStickY", T("DeadZone Right Stick Y"))
y2 += 10
AddInputCol2("DeadZoneRightTrigger", T("DeadZone Right Trigger"))
y2 += 10
AddInputCol2("LinearityRightStickX", T("Linearity* Right Stick X (0-100)"), "50")
AddInputCol2("LinearityRightStickY", T("Linearity* Right Stick Y (0-100)"), "50")
y2 += 10
AddToggleCol2(ConfigIni, "Gamepad", "InvertRightStickX", T("Invert Right Stick X"))
AddToggleCol2(ConfigIni, "Gamepad", "InvertRightStickY", T("Invert Right Stick Y"))

; =========================================
; TAB 7: STEERING
; =========================================
Tabs.UseTab("Steering")
yPos := 55

; Выбор эмулируемого геймпада (Xbox -> "Xbox", DS4 -> "DS4")
AddMappedDropdown(ConfigIni, "Gamepad", "EmulatedController", T("Emulated Controller *"), ["Xbox", "DS4"], Map("Xbox", "Xbox", "DS4", "DS4"))
yPos += 15

MainGui.Add("Text", "x20 y" yPos " w450 cBlue", T("--- Wheel settings (Driving mode) ---"))
yPos += 40
; Угол и линейность руля
AddInput(ConfigIni, "Motion", "SteeringWheelAngle", T("Steering Wheel Angle"))
yPos += 10
AddInput(ConfigIni, "Motion", "LinearityWheel", T("Steering Wheel Linearity"))

yPos += 20
MainGui.Add("Text", "x20 y" yPos " w450 cBlue", T("--- External Pedals Settings ---"))
yPos += 40

; Включение DirectInput (on -> "1", off -> "0")
AddMappedDropdown(ConfigIni, "ExternalPedals", "DInput", T("DirectInput Search"), [T("On"), T("Off")], Map(T("On"), "1", T("Off"), "0"))
yPos += 10
; Имя устройства педалей
;AddInput(ConfigIni, "ExternalPedals", "DeviceName", T("Device Name"))
valDevice := IniRead(A_ScriptDir "\" ConfigIni, "ExternalPedals", "DeviceName", "AUTO")
MainGui.Add("Text", "x20 y" (yPos+3) " w145", T("Device Name") ":")
edtDevice := MainGui.Add("Edit", "x170 y" yPos " w220", valDevice)
CtrlSettings["DeviceName"] := {type: "edt", ctrl: edtDevice, file: ConfigIni, sec: "ExternalPedals"}
yPos += 28
yPos += 10
; Список поддерживаемых осей
AxesList := ["X", "Y", "Z", "R", "U", "V", "Z-ROTATION", "X-ROTATION", "Y-ROTATION", "DIAL"]
AxesMap := Map()
for axis in AxesList {
    AxesMap[axis] := axis
}

; Выбор осей для Педали 1 и Педали 2
AddMappedDropdown(ConfigIni, "ExternalPedals", "Pedal1Axis", T("Pedal 1 Axis"), AxesList, AxesMap)
yPos += 10
AddMappedDropdown(ConfigIni, "ExternalPedals", "Pedal2Axis", T("Pedal 2 Axis"), AxesList, AxesMap)

yPos += 100

MainGui.Add("Text", "x20 y" yPos " w450 cRed", T("External Pedals:"))
yPos += 30
MainGui.Add("Text", "x20 y" yPos " w4780", T("Use pedals as analog triggers. Note: This is an experimental feature; proper functioning is not guaranteed."))
yPos += 20
MainGui.Add("Text", "x20 y" yPos " w820", T("Connect your wheel/pedals, set DirectInput search to On and launch JCAdvance. If you see message: `n'[Pedals Search] ID 0: Found device 'Your wheel/pedlas name' -> APPROVED!', configure the correct pedal axes and you've golden."))
yPos += 40
MainGui.Add("Text", "x20 y" yPos " w820", T("If you can't see your wheel/pedals name, replace AUTO with your device's name exactly as it appears in joy.cpl"))
yPos += 35

MainGui.Add("Text", "x20 y" yPos " w780 cRed", T("* Emulated Controller: DS4 Mode for Nintendo controllers only"))
yPos += 30
MainGui.Add("Text", "x20 y" yPos " w780", T("For DirectInput games, you can change the controller type to DS4. When you launch JCAdvacne, ‘Wireless Controller’ will appear instead of ‘Xbox 360 Controller’"))

; =========================================
; TAB 8: PROFILES
; =========================================
Tabs.UseTab("Profiles")

MainGui.Add("Text", "x20 y65 w810 Center", T("Profile Manager"))

MainGui.Add("GroupBox", "x20 y120 w790 h140 cBlue", T("Load Delete Profile"))
MainGui.Add("Text", "x40 y160 w450", T("Current Active Profile: ") ActiveProfile)

ProfileList := []
Loop Files, A_ScriptDir "\XboxProfiles\*.ini", "F" {
    ProfileList.Push(A_LoopFileName)
}

MainGui.Add("Text", "x40 y203 w130", T("Select Profile:"))
ProfileDdl := MainGui.Add("DropDownList", "x170 y200 w180 Choose1", ProfileList)
ProfileDdl.Text := ActiveProfile

LoadProfileBtn := MainGui.Add("Button", "x370 y198 w120 h26", T("Load Profile"))
LoadProfileBtn.OnEvent("Click", (*) => LoadProfileEvent(ProfileDdl.Text))

DeleteProfileBtn := MainGui.Add("Button", "x500 y198 w120 h26", T("Delete Profile"))
DeleteProfileBtn.OnEvent("Click", (*) => DeleteProfileEvent(ProfileDdl.Text))

LoadProfileEvent(selectedProfile) {
    if (selectedProfile == "")
        return
    IniWrite(selectedProfile, A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile")
    Reload()
}

MainGui.Add("GroupBox", "x20 y280 w790 h140 cBlue", T("Create New Profile"))
MainGui.Add("Text", "x40 y330 w400", T("Enter Name for the New Profile:"))

NewProfileEdit := MainGui.Add("Edit", "x40 y365 w250")
CreateProfileBtn := MainGui.Add("Button", "x310 y363 w150 h26", T("Create and Load Profile"))
CreateProfileBtn.OnEvent("Click", (*) => CreateProfileEvent(NewProfileEdit.Text))

CreateProfileEvent(profileName) {
    profileName := Trim(profileName)
    if (profileName == "") {
        MsgBox(T("Please enter a valid profile name!"), T("Error"), "Icon!")
        return
    }
    
    if (SubStr(profileName, -4) != ".ini")
        profileName .= ".ini"
        
    targetFile := A_ScriptDir "\XboxProfiles\" profileName
    
    if FileExist(targetFile) {
        MsgBox(T("Profile with this name already exists!"), T("Error"), "Icon!")
        return
    }
    
    try {
        FileCopy(A_ScriptDir "\XboxProfiles\default.ini", targetFile, 1)
        
        For key in JslKeys {
            if (key != "NONE") {
                if (key == "SL" || key == "SR" || key == "HOME" || key == "CAPTURE") {
                    IniWrite("NONE", targetFile, "JOYCONS", key)
                } else if (key == "L4" || key == "R4") {
                    IniWrite("NONE", targetFile, "DUALSENSE-EDGE", key)
                } else {
                    IniWrite("NONE", targetFile, "Xbox", key)
                }
                IniWrite("NONE", targetFile, "KEYBOARD-MOUSE", key)
            }
        }
        
        IniWrite(profileName, A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile")
        MsgBox(T("Profile '") profileName T("' created successfully!"), T("Success"), "Iconi")
        Reload()
    } catch as err {
        MsgBox(T("Failed to create profile!`nDetails: ") err.Message, T("Error"), "IconX")
    }
}

; =========================================
; Other stuff
; =========================================
Tabs.UseTab() 

; Дублирующая кнопка Save All в правом верхнем углу вкладок
SaveBtn2 := MainGui.Add("Button", "x720 y5 w120 h25", T("Save All"))
SaveBtn2.OnEvent("Click", SaveAllConfigs)

SetLayout(value, *) {
    IniWrite(value, A_ScriptDir "\" ConfigIni, "ConfigGUI", "Layout")
    Reload()
}

MainGui.Add("Text", "x20 y725 w220", T("Layout:"))

chkNintendo := (Layout == "Nintendo") ? " Checked1" : ""
chkSony := (Layout == "Sony") ? " Checked1" : ""

radNintendo := MainGui.Add("Radio", "x95 y725" chkNintendo, "Nintendo")
radSony := MainGui.Add("Radio", "x170 y725" chkSony, "Sony")

radNintendo.OnEvent("Click", (*) => SetLayout("Nintendo"))
radSony.OnEvent("Click", (*) => SetLayout("Sony"))

LanguagesList := ["English"]
Loop Files, A_ScriptDir "\Language\*.ini", "F" {
    LanguagesList.Push(StrReplace(A_LoopFileName, ".ini", ""))
}

MainGui.Add("Text", "x350 y725 w80", T("Language:"))
LangDdl := MainGui.Add("DropDownList", "x420 y723 w120", LanguagesList)
LangDdl.Text := CurrentLang
LangDdl.OnEvent("Change", (ctrl, *) => ChangeLanguageEvent(ctrl.Text))

ChangeLanguageEvent(selectedLang) {
    IniWrite(selectedLang, A_ScriptDir "\" ConfigIni, "ConfigGUI", "Language")
    Reload()
}

SaveBtn := MainGui.Add("Button", "x720 y720 w120 h25 Default", T("Save All"))
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
        
        ; --- 2. Запись Вкладки XBOX (Основные кнопки) ---
        for key in XboxMapping {
            ctrl := CtrlXbox[key]
            btn := ctrl.Text
            if (btn != "NONE" && btn != "") {
                if (btn != "SL" && btn != "SR" && btn != "HOME" && btn != "CAPTURE" && btn != "L4" && btn != "R4") {
                    IniWrite(key, A_ScriptDir "\" XboxIni, "Xbox", btn) 
                }
            }
        }

        ; --- Запись дополнительных кнопок напрямую в их системные секции ---
        for key in ["SL", "SR", "HOME", "CAPTURE"] {
            ctrl := CtrlExtraXbox[key]
            val := (ctrl.Text != "") ? ctrl.Text : "NONE"
            IniWrite(val, A_ScriptDir "\" XboxIni, "JOYCONS", key)
        }
        for key in ["L4", "R4"] {
            ctrl := CtrlExtraXbox[key]
            val := (ctrl.Text != "") ? ctrl.Text : "NONE"
            IniWrite(val, A_ScriptDir "\" XboxIni, "DUALSENSE-EDGE", key)
        }

        ; --- 3 и 4: Сохраняем активный Layout, НЕ стирая уникальные кнопки другого ---
        if (Layout == "Sony") {
            for key in SonyMapping {
                ctrl := CtrlSony[key]
                val := (ctrl.Text != "") ? ctrl.Text : "NONE"
                IniWrite(val, A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key)
            }
        } else {
            for key in JoyconMapping {
                ctrl := CtrlJoyCon[key]
                val := (ctrl.Text != "") ? ctrl.Text : "NONE"
                IniWrite(val, A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key)
            }
        }
		
        ; --- 5. Вкладка WHEEL ---
        valAct := (CtrlWheel["WHEEL-ACTIVATION"].ddl.Text != "") ? CtrlWheel["WHEEL-ACTIVATION"].ddl.Text : "NONE"
        IniWrite(valAct, A_ScriptDir "\" XboxIni, "Motion", "WHEEL-ACTIVATION")
        
		; Сохранение нового параметра MELEE-GESTURE в зависимости от выбранного режима (Xbox или KB/M)
        if (CtrlWheel.Has("MELEE-GESTURE")) {
            obj := CtrlWheel["MELEE-GESTURE"]
            valMelee := (obj.ddl.Text != "") ? obj.ddl.Text : "NONE"
            if (obj.rKbm.Value == 1) {
                IniWrite(valMelee, A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "MELEE-GESTURE")
                IniWrite("NONE", A_ScriptDir "\" XboxIni, "Motion", "MELEE-GESTURE")
            } else {
                IniWrite(valMelee, A_ScriptDir "\" XboxIni, "Motion", "MELEE-GESTURE")
                IniWrite("NONE", A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "MELEE-GESTURE")
            }
        }
		
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
        
        IniWrite(Layout, A_ScriptDir "\" ConfigIni, "ConfigGUI", "Layout")
        IniWrite(ActiveProfile, A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile")
		
        MsgBox(T("Settings successfully saved!"), "Success")
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
    bGui := Gui("+AlwaysOnTop -SysMenu +ToolWindow", T("Waiting for input.."))
    bGui.Add("Text", "w250 h50 Center +0x0200", T("Press any Keyboard or Mouse key `n(ESC to cancel)"))
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
    bGui := Gui("+AlwaysOnTop -SysMenu +ToolWindow", T("Connecting.."))
    infoText := bGui.Add("Text", "w250 h50 Center +0x0200", T("Connecting to gamepads..`n(Please wait up to 5s)"))
    bGui.Show("NoActivate")
    
    ; Даем окну 50мс, чтобы физически прорисоваться на экране до блокирующего вызова DLL
    Sleep(50) 

    ; 2. Запускаем тяжелый поиск устройств в DLL
    DllCall("JoyShockLibrary.dll\JslConnectDevices", "Cdecl")
    
    handles := Buffer(16, 0)
    count := DllCall("JoyShockLibrary.dll\JslGetConnectedDeviceHandles", "Ptr", handles, "Int", 4, "Cdecl Int")
    
    if (count == 0) {
        bGui.Destroy()
        MsgBox(T("No gamepads found!`nEnsure JCAdvance is closed and gamepad is connected"), "Warning")
        return
    }
    
    ; 3. Обновляем заголовок и текст окна на ожидание нажатия кнопки
    bGui.Title := T("Waiting for input..")
    infoText.Text := T("Press any Gamepad button..`n(Timeout: 5 sec)")

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
    if (mask & 0x80000)
        return "SL"
    if (mask & 0x100000)
        return "SR"
    return ""
}