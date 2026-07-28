#Requires AutoHotkey v2.0
#SingleInstance Force

; ==============================================================================
; 1. СТАТИЧНЫЕ ГЛОБАЛЬНЫЕ МАССИВЫ (не имеют зависимостей)
; ==============================================================================
Global XboxMapping := ["UP", "DOWN", "LEFT", "RIGHT", "BACK", "START", "LS", "RS", "LB", "RB", "A", "B", "X", "Y", "LT", "RT"]

Global JoyconMapping := ["UP", "DOWN", "LEFT", "RIGHT", "L3", "R3", "L", "R", "ZL", "ZR", "B", "A", "Y", "X", "MINUS", "PLUS", "SL", "SR", "CAPTURE", "HOME"]

Global SonyMapping := ["UP", "DOWN", "LEFT", "RIGHT", "L3", "R3", "L1", "R1", "L2", "R2", "CROSS", "CIRCLE", "SQUARE", "TRIANGLE", "SHARE", "OPTIONS", "L4", "R4"]

Global WheelMapping := ["WHEEL-UP", "WHEEL-DOWN", "WHEEL-LEFT", "WHEEL-RIGHT"]

Global RsButtonMapping := ["RS-UP", "RS-DOWN", "RS-LEFT", "RS-RIGHT"]

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
        try SmartIniWrite(ActiveProfile, A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile")
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

; --- НОВЫЙ МАССИВ ДЛЯ ВЕРХНЕГО БЛОКА XBOX ---
Global MainLayoutKeys := []
for k in LayoutKeys {
    if (k != "SL" && k != "SR" && k != "CAPTURE" && k != "HOME" && k != "L4" && k != "R4") {
        MainLayoutKeys.Push(k)
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
Global IsUnsavedChanges := false
MainGui := Gui("-MaximizeBox", "JCAdvance Config Editor")
;MainGui.OnEvent("Close", (*) => ExitApp())
MainGui.OnEvent("Close", ConfirmExit)

; --- ГЛОБАЛЬНЫЙ ШРИФТ ---
;MainGui.SetFont("s10")
MainGui.SetFont("s10 q5", "Segoe UI")

; --- ДИНАМИЧЕСКИЙ СПИСОК ВКЛАДОК ---
Global TabList := ["Xbox"]
if (Layout == "Sony") {
    TabList.Push("Sony")
} else {
    TabList.Push("Joy-Con")
}
for tabName in ["Special", "Hotkeys", "Gyro", "Analog", "Steering", "Profiles", "Second", "Settings"] {
    TabList.Push(tabName)
}

Tabs := MainGui.Add("Tab3", "x10 y10 w830", TabList)

; =========================================
; HELPERS
; =========================================

AddToggle(iniFile, sec, key, desc, pos := "") {
    val := IniRead(A_ScriptDir "\" iniFile, sec, key, "0")
    chkOpt := (val = "1") ? " Checked1" : ""
    opt := (pos != "") ? pos " " chkOpt : "xs+15 y+10 " chkOpt
    
    chk := MainGui.Add("Checkbox", opt, desc)
    CtrlSettings[iniFile "_" sec "_" key] := {type: "chk", ctrl: chk, file: iniFile, sec: sec, key: key}
}

AddInput(iniFile, sec, key, desc, defaultVal := "0", pos := "", labelWidth := 220, editWidth := 80, hasUpDown := true, udRange := "0-100", layoutMode := 0) {
    val := IniRead(A_ScriptDir "\" iniFile, sec, key, defaultVal)
    
    upDownVal := IsNumber(val) ? Integer(val) : 0
    
    if (layoutMode == 0)
    {
        txtOpt := (pos != "") ? pos " w" labelWidth : "xs+15 y+10 w" labelWidth
        MainGui.Add("Text", txtOpt, desc)
        
        edt := MainGui.Add("Edit", "x+10 yp-3 w" editWidth, val)
        
        if (hasUpDown)
        {
            MainGui.Add("UpDown", "Range" udRange, upDownVal)
        }
    }
    else if (layoutMode == 1)
    {
        anchorOpt := (pos != "") ? pos " w0 h0" : "xs+15 y+10 w0 h0"
        MainGui.Add("Text", anchorOpt, "") 
        
        edt := MainGui.Add("Edit", "xp yp-3 w" editWidth, val)
        
        if (hasUpDown)
        {
            MainGui.Add("UpDown", "Left Range" udRange, upDownVal)
        }
        
        MainGui.Add("Text", "x+10 yp+3 w" labelWidth, desc) 
    }
    else if (layoutMode == 2)
    {
        txtOpt := (pos != "") ? pos " w" labelWidth : "xs+15 y+10 w" labelWidth
        MainGui.Add("Text", txtOpt, desc)
        
        edt := MainGui.Add("Edit", "x+10 yp-3 w" editWidth, val)
        
        if (hasUpDown)
        {
            MainGui.Add("UpDown", "Left Range" udRange, upDownVal)
        }
    }
    
    CtrlSettings[iniFile "_" sec "_" key] := {type: "edt", ctrl: edt, file: iniFile, sec: sec, key: key}
}

AddMappedDropdown(iniFile, sec, key, desc, optionsArray, valMap, pos := "", labelWidth := 220, ddlWidth := 150) {
    val := IniRead(A_ScriptDir "\" iniFile, sec, key, "0")
    
    selectedText := optionsArray[1]
    for k, v in valMap {
        if (v == val) {
            selectedText := k
            break
        }
    }
    
    txtOpt := (pos != "") ? pos " w" labelWidth : "xs+15 y+10 w" labelWidth
    MainGui.Add("Text", txtOpt, desc)
    ddl := MainGui.Add("DropDownList", "x+10 yp-3 w" ddlWidth " Choose1", optionsArray)
    ddl.Text := selectedText
    
    CtrlSettings[iniFile "_" sec "_" key] := {type: "mapped_ddl", ctrl: ddl, file: iniFile, sec: sec, valMap: valMap, key: key}
}

AddHotkey(iniFile, sec, key, desc, listKeys, bindFunc, pos := "", labelWidth := 220, ddlWidth := 130) {
    val := IniRead(A_ScriptDir "\" iniFile, sec, key, "NONE")
    
    txtOpt := (pos != "") ? pos " w" labelWidth : "xs+15 y+10 w" labelWidth
    MainGui.Add("Text", txtOpt, desc)
    ddl := MainGui.Add("ComboBox", "x+10 yp-3 w" ddlWidth " Choose1", listKeys)
    SetDdlValue(ddl, val)
    
    btn := MainGui.Add("Button", "x+5 yp w60 h20", "Bind")
    btn.OnEvent("Click", bindFunc.Bind(ddl))
    
    CtrlSettings[iniFile "_" sec "_" key] := {type: "ddl", ctrl: ddl, file: iniFile, sec: sec, key: key}
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
            SmartIniWrite("default.ini", A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile")
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

;MainGui.SetFont("Bold")
MainGui.Add("Text", "x15 y73 w810 Center", T("Mapping your gamepad buttons to Xbox virtual buttons"))
;MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

; УМНЫЙ СТАРТОВЫЙ ФИЛЬТР: Загружаем только бинды текущей активной раскладки
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

MainGui.Add("Picture", "x270 y195 w300 h-1", A_ScriptDir "\Icons\xbox.png")

; LEFT
XboxMapLeft := ["LT", "LB", "BACK", "LS", "UP", "DOWN", "LEFT", "RIGHT"]
yPosLeft := 135
for key in XboxMapLeft {
    val := SavedXboxMap.Has(key) ? SavedXboxMap[key] : "NONE"
    
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x40 y" (yPosLeft+1) " w45 +Right", key ":")
	MainGui.SetFont("Norm")
    
    ;ddl := MainGui.Add("ComboBox", "x90 y" yPosLeft " w115 Choose1", LayoutKeys)
    ddl := MainGui.Add("ComboBox", "x90 y" yPosLeft " w115 Choose1", MainLayoutKeys)
	SetDdlValue(ddl, val)
    CtrlXbox[key] := ddl
    
    btn := MainGui.Add("Button", "x215 y" (yPosLeft-1) " w50 h22", "Bind")
    btn.OnEvent("Click", BindGamepad.Bind(ddl))
    
    yPosLeft += 42 
}

; RIGHT
XboxMapRight := ["RT", "RB", "START", "RS", "Y", "X", "B", "A"]
yPosRight := 135
for key in XboxMapRight {
    val := SavedXboxMap.Has(key) ? SavedXboxMap[key] : "NONE"
    
    ;ddl := MainGui.Add("ComboBox", "x635 y" yPosRight " w115 Choose1", LayoutKeys)
    ddl := MainGui.Add("ComboBox", "x635 y" yPosRight " w115 Choose1", MainLayoutKeys)
	SetDdlValue(ddl, val)
    CtrlXbox[key] := ddl
    
    btn := MainGui.Add("Button", "x575 y" (yPosRight-1) " w50 h22", "Bind")
    btn.OnEvent("Click", BindGamepad.Bind(ddl))
    
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x755 y" (yPosRight+1) " w45", ":" key)
    MainGui.SetFont("Norm")
	
    yPosRight += 42
}

MainGui.SetFont("Bold")
MainGui.Add("GroupBox", "x160 y490 w530 h145 cBlue Center", T("Extra Buttons"))
MainGui.SetFont("Norm")

; Загружаем сохраненные значения из INI-файла
SavedExtraMap := Map()
for key in ["SL", "SR", "HOME", "CAPTURE"] {
    SavedExtraMap[key] := IniRead(A_ScriptDir "\" XboxIni, "JOYCONS", key, "NONE")
}
for key in ["L4", "R4"] {
    SavedExtraMap[key] := IniRead(A_ScriptDir "\" XboxIni, "DUALSENSE-EDGE", key, "NONE")
}

; LEFT 2
yExtra := 525
for key in ["SL", "CAPTURE", "L4"] {
    val := SavedExtraMap[key]
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x200 y" (yExtra+2) " w65 +Right", key ":")
	MainGui.SetFont("Norm")
    ddl := MainGui.Add("DropDownList", "x275 y" yExtra " w115 Choose1", XboxKeys)
    SetDdlValue(ddl, val)
    CtrlExtraXbox[key] := ddl
    yExtra += 32
}

; RIGHT 2
yExtra := 525
for key in ["SR", "HOME", "R4"] {
    val := SavedExtraMap[key]
    ddl := MainGui.Add("DropDownList", "x460 y" yExtra " w115 Choose1", XboxKeys)
    SetDdlValue(ddl, val)
    CtrlExtraXbox[key] := ddl
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x585 y" (yExtra+2) " w65", ":" key) ; Отражаем двоеточие для симметрии
	MainGui.SetFont("Norm")
    yExtra += 32
}

MainGui.Add("Text", "x20 y662 w810 Center cRed", T("* Only digital buttons can be successfully remapped"))

; =========================================
; TAB 2: JOY-CON
; =========================================
if (Layout == "Nintendo") {
    Tabs.UseTab("Joy-Con")
    MainGui.Add("Text", "x20 y60 w810 Center", T("Emulate keyboard/mouse keys using the Joy-Cons buttons"))

    ; Смещаем картинки в центр
    MainGui.Add("Picture", "x15 y105 w100 h-1", A_ScriptDir "\Icons\joycon_left.png")
    MainGui.Add("Picture", "x720 y105 w100 h-1", A_ScriptDir "\Icons\joycon_right.png")

    ; --- ЛЕВАЯ КОЛОНКА JOY-CON (Рядом с левой картинкой) ---
    JcMapLeft := ["ZL", "L", "MINUS", "L3", "UP", "DOWN", "LEFT", "RIGHT", "SL", "CAPTURE"]
    yPosLeft := 100
    for key in JcMapLeft {
        val := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key, "NONE")
        
		MainGui.SetFont("Bold")
        MainGui.Add("Text", "x120 y" (yPosLeft+4) " w60 +Right", key ":")
		MainGui.SetFont("Norm")
        
        ddl := MainGui.Add("ComboBox", "x190 y" (yPosLeft+2) " w140 Choose1", KbmKeys)
        SetDdlValue(ddl, val)
        CtrlJoyCon[key] := ddl
        
        btn := MainGui.Add("Button", "x340 y" (yPosLeft+0) " w50 h24", "Bind")
        btn.OnEvent("Click", BindKbm.Bind(ddl))
        
        yPosLeft += 35
    }

    ; --- ПРАВАЯ КОЛОНКА JOY-CON (Рядом с правой картинкой) ---
    JcMapRight := ["ZR", "R", "PLUS", "R3", "X", "Y", "A", "B", "SR", "HOME"]
    yPosRight := 100
    for key in JcMapRight {
        val := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key, "NONE")
        
        ddl := MainGui.Add("ComboBox", "x510 y" (yPosRight+2) " w140 Choose1", KbmKeys)
        SetDdlValue(ddl, val)
        CtrlJoyCon[key] := ddl
        
        btn := MainGui.Add("Button", "x450 y" (yPosRight+0) " w50 h24", "Bind")
        btn.OnEvent("Click", BindKbm.Bind(ddl))
        
		MainGui.SetFont("Bold")
        MainGui.Add("Text", "x660 y" (yPosRight+3) " w60", ":" key)
		MainGui.SetFont("Norm")
        
        yPosRight += 35
    }

; --- РЕЖИМЫ СТИКОВ (Analog Sticks directions emulation) ---
    MainGui.SetFont("cBlue Bold q5", "Segoe UI")
    MainGui.Add("GroupBox", "x55 y470 w720 h215 Center Section", T("Analog Sticks modes to emulate KB/M"))
    MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

    StickModes := ["NONE", "WASD", "ARROWS", "MOUSE-LOOK", "MOUSE-WHEEL", "NUMPAD-ARROWS", "CUSTOM-BUTTONS"]

    ; ROW 1: MODES
    valLSM := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "LS-MODE", "CUSTOM-BUTTONS")
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x120 y504 w60 +Right", "LS-MODE:")
	MainGui.SetFont("Norm")
	
    ddlLSM := MainGui.Add("DropDownList", "x190 y502 w140 Choose1", StickModes)
    SetDdlValue(ddlLSM, valLSM)
    CtrlSettings["LS-MODE"] := {type: "txt", ctrl: ddlLSM, file: XboxIni, sec: "KEYBOARD-MOUSE"}

    valRSM := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "RS-MODE", "CUSTOM-BUTTONS")
    ddlRSM := MainGui.Add("DropDownList", "x510 y502 w140 Choose1", StickModes)
    SetDdlValue(ddlRSM, valRSM)
    CtrlSettings["RS-MODE"] := {type: "txt", ctrl: ddlRSM, file: XboxIni, sec: "KEYBOARD-MOUSE"}
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x660 y503 w70", ":RS-MODE")
	MainGui.SetFont("Norm")

    ; ROW 2: UP
    valLS_U := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "LS-UP", "NONE")
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x120 y539 w60 +Right", "LS-UP:")
	MainGui.SetFont("Norm")
    ddlLS_U := MainGui.Add("ComboBox", "x190 y537 w140 Choose1", KbmKeys)
    SetDdlValue(ddlLS_U, valLS_U)
    CtrlSettings["LS-UP"] := {type: "txt", ctrl: ddlLS_U, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnLSU := MainGui.Add("Button", "x340 y535 w50 h24", "Bind")
    btnLSU.OnEvent("Click", BindKbm.Bind(ddlLS_U))

    valRS_U := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "RS-UP", "NONE")
    btnRSU := MainGui.Add("Button", "x450 y535 w50 h24", "Bind")
    ddlRS_U := MainGui.Add("ComboBox", "x510 y537 w140 Choose1", KbmKeys)
    SetDdlValue(ddlRS_U, valRS_U)
    CtrlSettings["RS-UP"] := {type: "txt", ctrl: ddlRS_U, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnRSU.OnEvent("Click", BindKbm.Bind(ddlRS_U))
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x660 y538 w70", ":RS-UP")
	MainGui.SetFont("Norm")

    ; ROW 3: DOWN
    valLS_D := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "LS-DOWN", "NONE")
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x120 y574 w60 +Right", "LS-DOWN:")
	MainGui.SetFont("Norm")
    ddlLS_D := MainGui.Add("ComboBox", "x190 y572 w140 Choose1", KbmKeys)
    SetDdlValue(ddlLS_D, valLS_D)
    CtrlSettings["LS-DOWN"] := {type: "txt", ctrl: ddlLS_D, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnLSD := MainGui.Add("Button", "x340 y572 w50 h24", "Bind")
    btnLSD.OnEvent("Click", BindKbm.Bind(ddlLS_D))

    valRS_D := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "RS-DOWN", "NONE")
    btnRSD := MainGui.Add("Button", "x450 y570 w50 h24", "Bind")
    ddlRS_D := MainGui.Add("ComboBox", "x510 y572 w140 Choose1", KbmKeys)
    SetDdlValue(ddlRS_D, valRS_D)
    CtrlSettings["RS-DOWN"] := {type: "txt", ctrl: ddlRS_D, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnRSD.OnEvent("Click", BindKbm.Bind(ddlRS_D))
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x660 y572 w70", ":RS-DOWN")
	MainGui.SetFont("Norm")

    ; ROW 4: LEFT
    valLS_L := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "LS-LEFT", "NONE")
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x120 y609 w60 +Right", "LS-LEFT:")
	MainGui.SetFont("Norm")
    ddlLS_L := MainGui.Add("ComboBox", "x190 y607 w140 Choose1", KbmKeys)
    SetDdlValue(ddlLS_L, valLS_L)
    CtrlSettings["LS-LEFT"] := {type: "txt", ctrl: ddlLS_L, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnLSL := MainGui.Add("Button", "x340 y605 w50 h24", "Bind")
    btnLSL.OnEvent("Click", BindKbm.Bind(ddlLS_L))

    valRS_L := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "RS-LEFT", "NONE")
    btnRSL := MainGui.Add("Button", "x450 y605 w50 h24", "Bind")
    ddlRS_L := MainGui.Add("ComboBox", "x510 y607 w140 Choose1", KbmKeys)
    SetDdlValue(ddlRS_L, valRS_L)
    CtrlSettings["RS-LEFT"] := {type: "txt", ctrl: ddlRS_L, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnRSL.OnEvent("Click", BindKbm.Bind(ddlRS_L))
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x660 y607 w70", ":RS-LEFT")
	MainGui.SetFont("Norm")

    ; ROW 5: RIGHT
    valLS_R := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "LS-RIGHT", "NONE")
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x120 y645 w60 +Right", "LS-RIGHT:")
	MainGui.SetFont("Norm")
    ddlLS_R := MainGui.Add("ComboBox", "x190 y642 w140 Choose1", KbmKeys)
    SetDdlValue(ddlLS_R, valLS_R)
    CtrlSettings["LS-RIGHT"] := {type: "txt", ctrl: ddlLS_R, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnLSR := MainGui.Add("Button", "x340 y640 w50 h24", "Bind")
    btnLSR.OnEvent("Click", BindKbm.Bind(ddlLS_R))

    valRS_R := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "RS-RIGHT", "NONE")
    btnRSR := MainGui.Add("Button", "x450 y640 w50 h24", "Bind")
    ddlRS_R := MainGui.Add("ComboBox", "x510 y642 w140 Choose1", KbmKeys)
    SetDdlValue(ddlRS_R, valRS_R)
    CtrlSettings["RS-RIGHT"] := {type: "txt", ctrl: ddlRS_R, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnRSR.OnEvent("Click", BindKbm.Bind(ddlRS_R))
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x660 y643 w70", ":RS-RIGHT")
	MainGui.SetFont("Norm")
}

;yPos += 5
;MainGui.Add("Text", "x110 y" yPos " w830 cBlue", T("*Also you can configure Analog Sticks directions to emulate Keyboard keys and Mouse in XboxProfiles\*.ini"))

; =========================================
; TAB 2.1: SONY
; =========================================
if (Layout == "Sony") {
    Tabs.UseTab("Sony")
    MainGui.Add("Text", "x20 y50 w810 Center", T("Emulate keyboard/mouse keys using the Sony gamepad buttons"))

    MainGui.Add("Picture", "x290 y180 w250 h-1", A_ScriptDir "\Icons\Sony.png")

    ; --- ЛЕВАЯ КОЛОНКА SONY ---
    SonyMapLeft := ["L2", "L1", "SHARE", "L3", "UP", "DOWN", "LEFT", "RIGHT", "L4"]
    yPosSonyL := 110
    for key in SonyMapLeft {
        val := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key, "NONE")
		
        MainGui.SetFont("Bold")
        MainGui.Add("Text", "x40 y" (yPosSonyL+4) " w50 +Right", key ":")
        MainGui.SetFont("Norm")
		
        ddl := MainGui.Add("ComboBox", "x100 y" yPosSonyL " w120 Choose1", KbmKeys)
        SetDdlValue(ddl, val)
        CtrlSony[key] := ddl
        
        btn := MainGui.Add("Button", "x230 y" (yPosSonyL-1) " w50 h24", "Bind")
        btn.OnEvent("Click", BindKbm.Bind(ddl))
        
        yPosSonyL += 38
    }

    ; --- ПРАВАЯ КОЛОНКА SONY ---
    SonyMapRight := ["R2", "R1", "OPTIONS", "R3", "TRIANGLE", "SQUARE", "CIRCLE", "CROSS", "R4"]
    yPosSonyR := 110
    for key in SonyMapRight {
        val := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key, "NONE")
        
        ddl := MainGui.Add("ComboBox", "x610 y" yPosSonyR " w120 Choose1", KbmKeys)
        SetDdlValue(ddl, val)
        CtrlSony[key] := ddl
        
        btn := MainGui.Add("Button", "x550 y" (yPosSonyR-1) " w50 h24", "Bind")
        btn.OnEvent("Click", BindKbm.Bind(ddl))
        
		MainGui.SetFont("Bold")
        MainGui.Add("Text", "x740 y" (yPosSonyR+4) " w50", key)
        MainGui.SetFont("Norm")
		
        yPosSonyR += 38
    }

   ; --- РЕЖИМЫ СТИКОВ (Analog Sticks directions emulation) ---
    MainGui.SetFont("cBlue Bold q5", "Segoe UI")
    MainGui.Add("GroupBox", "x20 y470 w800 h210 Center Section", T("Analog Sticks modes to emulate KB/M"))
    MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

    StickModes := ["NONE", "WASD", "ARROWS", "MOUSE-LOOK", "MOUSE-WHEEL", "NUMPAD-ARROWS", "CUSTOM-BUTTONS"]

    ; ROW 1: MODES
    valLSM := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "LS-MODE", "CUSTOM-BUTTONS")
    MainGui.Add("Text", "x30 y505 w60 +Right", "LS-MODE:")
    ddlLSM := MainGui.Add("DropDownList", "x100 y500 w120 Choose1", StickModes)
    SetDdlValue(ddlLSM, valLSM)
    CtrlSettings["LS-MODE"] := {type: "txt", ctrl: ddlLSM, file: XboxIni, sec: "KEYBOARD-MOUSE"}

    valRSM := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "RS-MODE", "CUSTOM-BUTTONS")
    ddlRSM := MainGui.Add("DropDownList", "x610 y500 w120 Choose1", StickModes)
    SetDdlValue(ddlRSM, valRSM)
    CtrlSettings["RS-MODE"] := {type: "txt", ctrl: ddlRSM, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    MainGui.Add("Text", "x740 y505 w70", ":RS-MODE")

    ; ROW 2: UP
    valLS_U := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "LS-UP", "NONE")
    MainGui.Add("Text", "x30 y540 w60 +Right", "LS-UP:")
    ddlLS_U := MainGui.Add("ComboBox", "x100 y535 w120 Choose1", KbmKeys)
    SetDdlValue(ddlLS_U, valLS_U)
    CtrlSettings["LS-UP"] := {type: "txt", ctrl: ddlLS_U, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnLSU := MainGui.Add("Button", "x230 y535 w50 h24", "Bind")
    btnLSU.OnEvent("Click", BindKbm.Bind(ddlLS_U))

    valRS_U := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "RS-UP", "NONE")
    btnRSU := MainGui.Add("Button", "x550 y535 w50 h24", "Bind")
    ddlRS_U := MainGui.Add("ComboBox", "x610 y535 w120 Choose1", KbmKeys)
    SetDdlValue(ddlRS_U, valRS_U)
    CtrlSettings["RS-UP"] := {type: "txt", ctrl: ddlRS_U, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnRSU.OnEvent("Click", BindKbm.Bind(ddlRS_U))
    MainGui.Add("Text", "x740 y540 w70", ":RS-UP")

    ; ROW 3: DOWN
    valLS_D := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "LS-DOWN", "NONE")
    MainGui.Add("Text", "x30 y575 w60 +Right", "LS-DOWN:")
    ddlLS_D := MainGui.Add("ComboBox", "x100 y570 w120 Choose1", KbmKeys)
    SetDdlValue(ddlLS_D, valLS_D)
    CtrlSettings["LS-DOWN"] := {type: "txt", ctrl: ddlLS_D, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnLSD := MainGui.Add("Button", "x230 y570 w50 h24", "Bind")
    btnLSD.OnEvent("Click", BindKbm.Bind(ddlLS_D))

    valRS_D := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "RS-DOWN", "NONE")
    btnRSD := MainGui.Add("Button", "x550 y570 w50 h24", "Bind")
    ddlRS_D := MainGui.Add("ComboBox", "x610 y570 w120 Choose1", KbmKeys)
    SetDdlValue(ddlRS_D, valRS_D)
    CtrlSettings["RS-DOWN"] := {type: "txt", ctrl: ddlRS_D, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnRSD.OnEvent("Click", BindKbm.Bind(ddlRS_D))
    MainGui.Add("Text", "x740 y575 w70", ":RS-DOWN")

    ; ROW 4: LEFT
    valLS_L := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "LS-LEFT", "NONE")
    MainGui.Add("Text", "x30 y610 w60 +Right", "LS-LEFT:")
    ddlLS_L := MainGui.Add("ComboBox", "x100 y605 w120 Choose1", KbmKeys)
    SetDdlValue(ddlLS_L, valLS_L)
    CtrlSettings["LS-LEFT"] := {type: "txt", ctrl: ddlLS_L, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnLSL := MainGui.Add("Button", "x230 y605 w50 h24", "Bind")
    btnLSL.OnEvent("Click", BindKbm.Bind(ddlLS_L))

    valRS_L := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "RS-LEFT", "NONE")
    btnRSL := MainGui.Add("Button", "x550 y605 w50 h24", "Bind")
    ddlRS_L := MainGui.Add("ComboBox", "x610 y605 w120 Choose1", KbmKeys)
    SetDdlValue(ddlRS_L, valRS_L)
    CtrlSettings["RS-LEFT"] := {type: "txt", ctrl: ddlRS_L, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnRSL.OnEvent("Click", BindKbm.Bind(ddlRS_L))
    MainGui.Add("Text", "x740 y610 w70", ":RS-LEFT")

    ; ROW 5: RIGHT
    valLS_R := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "LS-RIGHT", "NONE")
    MainGui.Add("Text", "x30 y645 w60 +Right", "LS-RIGHT:")
    ddlLS_R := MainGui.Add("ComboBox", "x100 y640 w120 Choose1", KbmKeys)
    SetDdlValue(ddlLS_R, valLS_R)
    CtrlSettings["LS-RIGHT"] := {type: "txt", ctrl: ddlLS_R, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnLSR := MainGui.Add("Button", "x230 y640 w50 h24", "Bind")
    btnLSR.OnEvent("Click", BindKbm.Bind(ddlLS_R))

    valRS_R := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "RS-RIGHT", "NONE")
    btnRSR := MainGui.Add("Button", "x550 y640 w50 h24", "Bind")
    ddlRS_R := MainGui.Add("ComboBox", "x610 y640 w120 Choose1", KbmKeys)
    SetDdlValue(ddlRS_R, valRS_R)
    CtrlSettings["RS-RIGHT"] := {type: "txt", ctrl: ddlRS_R, file: XboxIni, sec: "KEYBOARD-MOUSE"}
    btnRSR.OnEvent("Click", BindKbm.Bind(ddlRS_R))
    MainGui.Add("Text", "x740 y645 w70", ":RS-RIGHT")
}

; =========================================
; TAB 3: SPECIAL (WHEEL)
; =========================================

Tabs.UseTab("Special")

; 1. Начальная координата Y
yPos := 50

; --- Группа 1: WHEEL ---
MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x180 y" yPos " w500 h223 Center Section", "Wheel")
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

; Описание (обычный цвет текста)
MainGui.Add("Text", "x200 y" (yPos+20) " w475", T("Gyro Wheel gestures for additional Xbox/KB+M buttons mapping. `nQuick press and release WHEEL-ACTIVATION button when gyro move:"))

yPos += 63

keyAct := "WHEEL-ACTIVATION"
valAct := IniRead(A_ScriptDir "\" XboxIni, "Motion", keyAct, "NONE")
MainGui.SetFont("Bold")
MainGui.Add("Text", "x200 y" (yPos+4) " w195", keyAct T(" Button:"))
MainGui.SetFont("Norm")
btnAct := MainGui.Add("Button", "x+0 y" (yPos-1) " w60 h22", "Bind")
ddlAct := MainGui.Add("ComboBox", "x+45 y" yPos " w150 Choose1", LayoutKeys)
SetDdlValue(ddlAct, valAct)
CtrlWheel[keyAct] := {ddl: ddlAct} 
btnAct.OnEvent("Click", BindGamepad.Bind(ddlAct))

yPos += 25

; 3. Отрисовка всех направлений WheelMapping (шаг 30px для компактности по высоте)
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
	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x200 y" (yPos+4) " w150", key ":")
    MainGui.SetFont("Norm")
	
    chkXbox := isKbm ? "" : " Checked1"
    chkKbm := isKbm ? " Checked1" : ""
    
    radXbox := MainGui.Add("Radio", "x330 y" (yPos+3) chkXbox, "Xbox")
    radKbm := MainGui.Add("Radio", "x400 y" (yPos+3) chkKbm, "KB/M")
    
    ddl := MainGui.Add("ComboBox", "x500 y" yPos " w150 Choose1", isKbm ? KbmKeys : XboxKeys)
    SetDdlValue(ddl, val)
    CtrlWheel[key] := {ddl: ddl, rXbox: radXbox, rKbm: radKbm}

    radXbox.OnEvent("Click", ChangeWheelList.Bind(ddl, XboxKeys))
    radKbm.OnEvent("Click", ChangeWheelList.Bind(ddl, KbmKeys))
    
    yPos += 25
}

; 4. MotionWheelButtonsDeadZone
yPos += 1
keyDead := "MotionWheelButtonsDeadZone"
valDead := IniRead(A_ScriptDir "\" ConfigIni, "Motion", keyDead, "12")
MainGui.SetFont("Bold")
MainGui.Add("Text", "x200 y" (yPos+4) " w250", T("Wheel Gesture DeadZone") ":")
MainGui.SetFont("Norm")
edtDead := MainGui.Add("Edit", "x400 y" yPos " w45", valDead)

try {
    MainGui.Add("UpDown", "Range0-100", Integer(valDead)) ; Диапазон от 0 до 100
} catch {
    MainGui.Add("UpDown", "Range0-100", 0)
}

CtrlSettings[keyDead] := {type: "edt", ctrl: edtDead, file: ConfigIni, sec: "Motion"}


; --- Группа 4: MELEE ---
yPos += 34

MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x180 y" yPos " w500 h120 Center Section", "Melee")
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

; Описание (обычный цвет текста)
MainGui.Add("Text", "x200 y" (yPos+20) " w475", T("Special 'Melee' gesture. Make a gesture: a punch, `na hook or a blow hammer to press virtual button:"))

yPos += 60
	
; 6. MELEE-GESTURE
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

MainGui.SetFont("Bold")
MainGui.Add("Text", "x200 y" (yPos+4) " w150", "MELEE-GESTURE:")
MainGui.SetFont("Norm")
 
chkXboxMelee := isKbmMelee ? "" : " Checked1"
chkKbmMelee := isKbmMelee ? " Checked1" : ""

radXboxMelee := MainGui.Add("Radio", "x330 y" (yPos+3) chkXboxMelee, "Xbox")
radKbmMelee := MainGui.Add("Radio", "x400 y" (yPos+3) chkKbmMelee, "KB/M")

ddlMelee := MainGui.Add("ComboBox", "x500 y" yPos " w150 Choose1", isKbmMelee ? KbmKeys : XboxKeys) 
SetDdlValue(ddlMelee, valMelee)
CtrlWheel["MELEE-GESTURE"] := {ddl: ddlMelee, rXbox: radXboxMelee, rKbm: radKbmMelee}

radXboxMelee.OnEvent("Click", ChangeWheelList.Bind(ddlMelee, XboxKeys))
radKbmMelee.OnEvent("Click", ChangeWheelList.Bind(ddlMelee, KbmKeys))

; 7. MeleeGForce
yPos += 25
valForce := IniRead(A_ScriptDir "\" ConfigIni, "Motion", "MeleeGForce", "5.0")
MainGui.SetFont("Bold")
MainGui.Add("Text", "x200 y" (yPos+3) " w250", T("Melee Gesture Force (g)") ":")
MainGui.SetFont("Norm")
edtForce := MainGui.Add("Edit", "x400 y" yPos " w40", valForce)
CtrlSettings["MeleeGForce"] := {type: "edt", ctrl: edtForce, file: ConfigIni, sec: "Motion"}

ChangeWheelList(ddl, listArray, *) {
    val := ddl.Text
    ddl.Delete()
    ddl.Add(listArray)
    SetDdlValue(ddl, val)
}


; --- Группа 2: Left STICK ---

yPos += 35

MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x180 y" yPos " w500 h123 Center Section", "Left Stick")
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

MainGui.Add("Text", "x200 y" (yPos+20) " w475", T("The virtual button will be held down when stick is tilted to a certain degree. `nFor example: Assign the run/sprint button to the stick's full travel"))

yPos += 63

; 1. AutoSprintButton
valXboxSprint := IniRead(A_ScriptDir "\" XboxIni, "Xbox", "AutoSprintButton", "NONE")
valKbmSprint := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "AutoSprintButton", "NONE")

isKbmSprint := false
valSprint := "NONE"

if (valKbmSprint != "NONE" && valKbmSprint != "") {
    valSprint := valKbmSprint
    isKbmSprint := true
} else if (valXboxSprint != "NONE" && valXboxSprint != "") {
    valSprint := valXboxSprint
}

MainGui.SetFont("Bold")
MainGui.Add("Text", "x200 y" (yPos+2) " w150", "AutoSprintButton:")
MainGui.SetFont("Norm")

chkXboxSprint := isKbmSprint ? "" : " Checked1"
chkKbmSprint := isKbmSprint ? " Checked1" : ""

radXboxSprint := MainGui.Add("Radio", "x330 y" (yPos+3) chkXboxSprint, "Xbox")
radKbmSprint := MainGui.Add("Radio", "x400 y" (yPos+3) chkKbmSprint, "KB/M")

ddlSprint := MainGui.Add("ComboBox", "x500 y" yPos " w150 Choose1", isKbmSprint ? KbmKeys : XboxKeys) 
SetDdlValue(ddlSprint, valSprint)
CtrlWheel["AutoSprintButton"] := {ddl: ddlSprint, rXbox: radXboxSprint, rKbm: radKbmSprint}

radXboxSprint.OnEvent("Click", ChangeWheelList.Bind(ddlSprint, XboxKeys))
radKbmSprint.OnEvent("Click", ChangeWheelList.Bind(ddlSprint, KbmKeys))

; 2. AutoPressStickValue
yPos += 26
valStick := IniRead(A_ScriptDir "\" XboxIni, "SETTINGS", "AutoPressStickValue", "90")
MainGui.SetFont("Bold")
MainGui.Add("Text", "x200 y" (yPos+3) " w250", T("Auto Press Stick Value (%)") ":")
MainGui.SetFont("Norm")
edtStick := MainGui.Add("Edit", "x400 y" yPos " w45", valStick)

try {
    MainGui.Add("UpDown", "Range0-100", Integer(valStick)) ; Диапазон от 0 до 100
} catch {
    MainGui.Add("UpDown", "Range0-100", 0)
}

CtrlSettings["AutoPressStickValue"] := {type: "edt", ctrl: edtStick, file: XboxIni, sec: "SETTINGS"}

; --- Группа 3: Right STICK ---
yPos += 35

MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x180 y" yPos " w500 h172 Center Section", "Right Stick")
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

; Описание (обычный цвет текста)
MainGui.Add("Text", "x200 y" (yPos+20) " w475", T("Map virtual Xbox/KB+M buttons to Right Stick directions. `nOnly for Right Stick modes: as buttons/as triggers ('Analog' tab)"))

yPos += 63

; 5. Отрисовка всех направлений RsButtonMapping
for key in RsButtonMapping {
    valXbox := IniRead(A_ScriptDir "\" XboxIni, "Xbox", key, "NONE")
    valKbm := IniRead(A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key, "NONE")
    
    isKbm := false
    val := "NONE"
    
    if (valKbm != "NONE" && valKbm != "") {
        val := valKbm
        isKbm := true
    } else if (valXbox != "NONE" && valXbox != "") {
        val := valXbox
    }

	MainGui.SetFont("Bold")
    MainGui.Add("Text", "x200 y" (yPos+4) " w150", key ":")
    MainGui.SetFont("Norm")
	
    chkXbox := isKbm ? "" : " Checked1"
    chkKbm := isKbm ? " Checked1" : ""
    
    radXbox := MainGui.Add("Radio", "x330 y" (yPos+3) chkXbox, "Xbox")
    radKbm := MainGui.Add("Radio", "x400 y" (yPos+3) chkKbm, "KB/M")
    
    ddl := MainGui.Add("ComboBox", "x500 y" yPos " w150 Choose1", isKbm ? KbmKeys : XboxKeys)
    SetDdlValue(ddl, val)
    CtrlWheel[key] := {ddl: ddl, rXbox: radXbox, rKbm: radKbm}

    radXbox.OnEvent("Click", ChangeWheelList.Bind(ddl, XboxKeys))
    radKbm.OnEvent("Click", ChangeWheelList.Bind(ddl, KbmKeys))
	
	yPos += 25
}



; =========================================
; TAB 4: HOTKEYS (config.ini)
; =========================================
Tabs.UseTab("Hotkeys")

; --- БОЛЬШАЯ ГРУППА 1: Gamepad Hotkeys ---
MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x180 y50 w490 h335 Center Section", T("Gamepad Hotkeys"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI") ; Сброс на стандартный шрифт

; Подгруппа 1.1: Aiming
MainGui.SetFont("Bold")
MainGui.Add("Text", "xs+30 ys+20 w450", T("Aiming"))
MainGui.SetFont("Norm s10")

; Хоткеи подгруппы Aiming (позиционируются автоматически друг под другом)
AddHotkey(ConfigIni, "Motion", "AimingToggleButton", T("Gyro Motion (On/Off)"), LayoutKeys, BindGamepad, "xs+30 y+10")
AddHotkey(ConfigIni, "Motion", "AimingModeToggleButton", T("Mode switching (Mouse/Stick)"), LayoutKeys, BindGamepad, "xs+30 y+10")
AddHotkey(XboxIni, "SETTINGS", "AimingButton", T("Motion Control (Ratchet) button"), LayoutKeys, BindGamepad, "xs+30 y+10")
AddHotkey(ConfigIni, "Motion", "AimingPressModeToggleButton", T("Control button behavior switching"), LayoutKeys, BindGamepad, "xs+30 y+10", 220)

; тонкая горизонтальная линия-разделитель
MainGui.Add("Text", "xs+30 y+15 w430 h2 0x10")

; Подгруппа 1.2: Driving
MainGui.SetFont("Bold")
MainGui.Add("Text", "xs+30 y+10 w450", T("Driving"))
MainGui.SetFont("Norm s10")

AddHotkey(ConfigIni, "Motion", "DrivingToggleButton", T("Driving Mode (On/Off)"), LayoutKeys, BindGamepad, "xs+30 y+10")
AddHotkey(ConfigIni, "Motion", "DrivingCalibrationButton", T("Wheel Centering / Recalibration"), LayoutKeys, BindGamepad, "xs+30 y+10")

MainGui.Add("Text", "xs+30 y+15 w430 h2 0x10")

MainGui.SetFont("Bold")
MainGui.Add("Text", "xs+30 y+10 w450", T("Misc"))
MainGui.SetFont("Norm s10")
AddHotkey(ConfigIni, "Gamepad", "StickAsTriggerToggleButton", T("Right stick as triggers (On/Off)"), LayoutKeys, BindGamepad, "xs+30 y+10")

; --- БОЛЬШАЯ ГРУППА 2: Keyboard Hotkeys ---
MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x180 y+35 w490 h162 Center Section", T("Keyboard Hotkeys"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

AddHotkey(ConfigIni, "SETTINGS", "ResetKey", T("Reset/Research Gamepad"), KbmKeys, BindKbm, "xs+30 ys+40")
;yPos += 30
AddHotkey(ConfigIni, "SETTINGS", "GyroCalibrateKey", T("Gyroscope Recalibration*"), KbmKeys, BindKbm, "xs+30 ys+67")
AddHotkey(ConfigIni, "SETTINGS", "AccelCalibrateKey", T("Accelerometer Recalibration*"), KbmKeys, BindKbm, "xs+30 ys+94")
AddHotkey(ConfigIni, "SETTINGS", "OSDKey", T("On-screen display (on/off)"), KbmKeys, BindKbm, "xs+30 ys+121")

MainGui.Add("Text", "x25 y+27 w820 cRed", T("Pro Tip:"))
MainGui.Add("Text", "x25 y+3 w820", T("- To assign a two-button gamepad combination (like R+HOME), you can manually type it into the field above"))
MainGui.Add("Text", "x25 y+5 w820", T("- If your hand tires quickly, bind Motion Control Button to non-aiming Joy-Con to prevent muscle tension"))

MainGui.Add("Text", "x25 y+7 w820 cRed", T("*Manual Software Recalibration:"))
MainGui.Add("Text", "x25 y+5 w820", T("- Gyroscope: Leave gamepad still (in any position), press the button, and wait for the beep"))
MainGui.Add("Text", "x25 y+5 w820", T("- Accelerometer: Use only on a flat surface. For Joy-Cons use Switch or Grip to prevent tilting"))


; =========================================
; TAB 5: GYRO (config.ini)
; =========================================
Tabs.UseTab("Gyro")

; --- Группа 1: Поведение и общие настройки ---
MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x210 y50 w410 h165 Center Section", T("Behavior and Settings"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

AddMappedDropdown(XboxIni, "SETTINGS", "AimingMode", T("Gyro mode by default"), [T("Right Stick"), T("Mouse")], Map(T("Right Stick"), "0", T("Mouse"), "1"), "xs+15 ys+25")
AddMappedDropdown(XboxIni, "SETTINGS", "AimingByPressingMode", T("Press Control (Ratchet) button to"), [T("stop motion tracking"), T("start motion tracking")], Map(T("stop motion tracking"), "0", T("start motion tracking"), "1"))
AddMappedDropdown(ConfigIni, "Motion", "GyroFromLeft", T("Gyro data in combined mode from"), [T("Right Joy-Con"), T("Left Joy-Con")], Map(T("Right Joy-Con"), "0", T("Left Joy-Con"), "1"))
AddMappedDropdown(ConfigIni, "SETTINGS", "SleepTimeOut", T("Polling rate (33.3 Hz for example)"), ["33.3 Hz", "66.7 Hz", "125 Hz", "250 Hz"], Map("33.3 Hz", "30", "66.7 Hz", "15", "125 Hz", "8", "250 Hz", "4"))
AddMappedDropdown(ConfigIni, "Motion", "GyroSpace", T("Gyro Motion Space *"), ["0", "1", "2", "3"], Map("0", "0", "1", "1", "2", "2", "3", "3"))

; --- Группа 2: Чувствительность и фильтрация ---
MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x210 y225 w410 h175 Center Section", T("Sensitivity and Filters"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

AddInput(XboxIni, "SETTINGS", "MouseSensX", T("Mouse X :"), "180", "xs+71 ys+30", 60, 50, true, "0-999")
AddInput(XboxIni, "SETTINGS", "MouseSensY", T("Mouse Y :"), "170", "xs+71 ys+58", 60, 50, true, "0-999")

; Включаем левый спиннер (последний параметр - true)
AddInput(XboxIni, "SETTINGS", "JoySensX", T(": Stick X"), "120", "xs+224 ys+30", 55, 50, true, "0-999", 1)
AddInput(XboxIni, "SETTINGS", "JoySensY", T(": Stick Y"), "120", "xs+224 ys+58", 55, 50, true, "0-999", 1)

AddInput(ConfigIni, "Motion", "RatchetDelayTime", T("Ratchet Delay"), "150", "xs+71 y+10", 110, 50, true, "0-999", 0)
MainGui.Add("Text", "xs255 y312", T("in milliseconds"))
AddInput(ConfigIni, "Motion", "Tightening", T("Tightening**"), "5.0", "xs+70 y+9", 111, 50,  true, "0-999", 0)
MainGui.Add("Text", "xs255 y338", T("no latency filter"))


AddInput(ConfigIni, "Motion", "MouseSmooth", T("EMA*** Mouse :"), , "xs+36 ys+142", 98, 50, true, "0-100")
AddInput(ConfigIni, "Motion", "JoySmooth", T(": EMA*** Stick"), , "xs+224 ys+142", 98, 50, true, "0-100", 1)

; --- Сноски и примечания (внизу вкладки) ---
MainGui.Add("Text", "x20 y+15 w820 cRed", "* Gyro Space (by Jibb Smart):")
MainGui.Add("Text", "x20 y+0 w820", T("This setting controls how gyroscope data from hand movements is processed and translated into cursor or stick input."))

; --- Выделяем "For two-handed controllers" ---
; Старая строка: MainGui.Add("Text", "x20 y+3 w820", T("For two-handed controllers, the difference only affects horizontal (X-axis) aiming. To move the cursor/stick left or right:"))
MainGui.SetFont("bold")
MainGui.Add("Text", "x20 y+3", T("For two-handed"))
MainGui.SetFont("norm")
MainGui.Add("Text", "x+0 w820", T(" controllers, the difference only affects horizontal (X-axis) aiming. To move the cursor/stick left or right:"))

MainGui.Add("Text", "x20 y+3 w820", T("0 — Turn the controller like a car steering wheel (Roll, only Gyro is used)"))
MainGui.Add("Text", "x20 y+2 w820", T("2 — Twist the controller like tank steering levers (Yaw, Gyro + Accel mode)"))

; --- Выделяем "For Joy-Cons" ---
; Старая строка: MainGui.Add("Text", "x20 y+4 w830", T("For Joy-Cons different rules apply. This setting dictates how wrist angle (clockwise/counter-clockwise Roll) `nand grip (horizontal or vertical) will skew cursor/stick movement relative to your arm's motion. Choose one:"))
MainGui.SetFont("bold")
MainGui.Add("Text", "x20 y+5", T("For Joy-Cons"))
MainGui.SetFont("norm")
MainGui.Add("Text", "x+0 w820", T(" different rules apply. This setting dictates how wrist angle (clockwise/counter-clockwise Roll) and"))
MainGui.Add("Text", "x20 y+0 w820", T("grip (horizontal or vertical) will skew cursor/stick movement relative to your arm's motion. Choose one:"))

MainGui.Add("Text", "x20 y+3 w820", T("0 — Wrist angle always affects to the cursor/stick’s movement relative to the movement of the hand (only Gyro is used)"))
MainGui.Add("Text", "x20 y+2 w820", T("1 — Wrist angle between -90 90 degrees has no effect. In vertical grip X-axis is controlled by screwdriver gesture (+Accel)"))
;MainGui.Add("Text", "x20 y+5 w820", T("Mode '1' and a horizontal grip (ZR pointing at the screen) provide the best accuracy and predictability of control for Joy-Con"))
;MainGui.Add("Text", "x20 y+2 w820", T("3 — Modified Mode 1: no interference with the control of the X-axis when rotating the hand (Roll) during a vertical grip"))
MainGui.Add("Text", "x20 y+2 w820", T("3 — Wrist angle has no effect. No 'screwdriver' effect in any grip! Best accuracy and predictability for gyro aiming (+Accel)"))

MainGui.Add("Text", "x20 y+4 w820 cRed", T("** Tightening:"))
MainGui.Add("Text", "x20 y+0 w820", T("Is a zero-latency, velocity-based threshold filter (by Jibb Smart) that attenuates micro-movements to eliminate hand `ntremors, pulse twitches and hardware sensor noise. 0 - disabled; 1 - 2 for Sony gamepads, 2 - 5 for Joy-Cons"))

MainGui.Add("Text", "x20 y+5 w820 cRed", T(" *** EMA Filter:"))
MainGui.Add("Text", "x20 y+0 w820", T("Smoothing time to reach 100% and decelerate to 0% speed (value - rise/fall time): 25   ~2.7ms;   50   ~8ms;   75   ~24ms"))

; =========================================
; TAB 6:Analog (config.ini)
; =========================================
Tabs.UseTab("Analog")

; --- Группа 1: LEFT HAND (Левая колонка сверху) ---
MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x30 y50 w380 h255 Center Section", T("LEFT"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

AddInput(XboxIni, "SETTINGS", "DeadZoneLeftTrigger", T("DeadZone Trigger :"), , "xs+110 ys+20", 120, 42, true)
AddInput(XboxIni, "SETTINGS", "DeadZoneLeftStickX", T("DeadZone Stick X :"), , "xs+110 y+6", 120, 42, true)
AddInput(XboxIni, "SETTINGS", "DeadZoneLeftStickY", T("DeadZone Stick Y :"), , "xs+110 y+6", 120, 42)
AddInput(XboxIni, "SETTINGS", "AntiDeadZoneLeftX", T("AntiDeadZone Stick X :"), , "xs+85 y+6", 145, 42)
AddInput(XboxIni, "SETTINGS", "AntiDeadZoneLeftY", T("AntiDeadZone Stick Y :"), , "xs+85 y+6", 145, 42)
AddInput(XboxIni, "SETTINGS", "LinearityLeftStickX", T("Linearity* Stick X :"), "50", "xs+115 y+6", 115, 42)
AddInput(XboxIni, "SETTINGS", "LinearityLeftStickY", T("Linearity* Stick Y :"), "50", "xs+115 y+6", 115, 42)
AddToggle(XboxIni, "SETTINGS", "InvertLeftStickX", T("Invert Stick X"), "xs+85 y275")
AddToggle(XboxIni, "SETTINGS", "InvertLeftStickY", T("Invert Stick Y"), "xs+191 y275")

; --- Группа 2: RIGHT HAND (Правая колонка сверху) ---
MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x425 y50 w380 h255 Center Section", T("RIGHT"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

; Включаем левый спиннер (true в самом конце)
AddInput(XboxIni, "SETTINGS", "DeadZoneRightTrigger", "    " T(": DeadZone Trigger"), , "xs+90 ys+20", 195, 42, true, "0-100", 1)
AddInput(XboxIni, "SETTINGS", "DeadZoneRightStickX", "    " T(": DeadZone Stick X"), , "xs+90 y+11", 195, 42, true, "0-100", 1)
AddInput(XboxIni, "SETTINGS", "DeadZoneRightStickY", "    " T(": DeadZone Stick Y"), , "xs+90 y+11", 195, 42, true, "0-100", 1)
AddInput(XboxIni, "SETTINGS", "AntiDeadZoneRightX", "    " T(": AntiDeadZone Stick X"), , "xs+90 y+11", 195, 42, true, "0-100", 1)
AddInput(XboxIni, "SETTINGS", "AntiDeadZoneRightY", "    " T(": AntiDeadZone Stick Y"), , "xs+90 y+11", 195, 42, true, "0-100", 1)
AddInput(XboxIni, "SETTINGS", "LinearityRightStickX", "    " T(": Linearity* Stick X"), "50", "xs+90 y+11", 195, 42, true, "0-100", 1)
AddInput(XboxIni, "SETTINGS", "LinearityRightStickY", "    " T(": Linearity* Stick Y"), "50", "xs+90 y+11", 195, 42, true, "0-100", 1)
AddToggle(XboxIni, "SETTINGS", "InvertRightStickX", T("Invert Stick X"), "xs+90 y275")
AddToggle(XboxIni, "SETTINGS", "InvertRightStickY", T("Invert Stick Y"), "xs+198 y275")

; --- Группа 3: HARDWARE SWAPS (Центрирована, построчно) ---
MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x175 y+17 w480 h195 Center Section", T("Hardware Swaps"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

AddToggle(XboxIni, "SETTINGS", "SWAP-STICKS", T("Swap Left and Right Sticks"), "xs+145 ys+25")
AddToggle(XboxIni, "SETTINGS", "SWAP-TRIGGERS", T("Swap Left and Right Triggers"), "xs+145 y+8")
AddToggle(XboxIni, "SETTINGS", "GyroApplyAntiDeadZone", T("Apply AntiDeadZone to Gyro stick"), "xs+130 y+8")
AddToggle(XboxIni, "SETTINGS", "GyroApplyLinearity", T("Apply Linearity curve to Gyro stick"), "xs+130 y+8")

; Настройка режима левого стика
valLSMode := IniRead(A_ScriptDir "\" XboxIni, "SETTINGS", "LeftStickMode", "0")
MainGui.Add("Text", "xs+115 y+10 w155", T("Left Stick Mode**") ":")
ddlLSMode := MainGui.Add("DropDownList", "x+10 yp-3 w110 Choose1", [T("default"), T("1 - front"), T("2 - all")])
LSModeMap := Map(T("default"), "0", T("1 - front"), "1", T("2 - all"), "2")
CtrlSettings["LeftStickMode"] := {type: "mapped_ddl", ctrl: ddlLSMode, file: XboxIni, sec: "SETTINGS", valMap: LSModeMap}

; Синхронизация текущего значения LeftStickMode
selectedTextLS := T("default")
for k, v in LSModeMap {
    if (v == valLSMode) {
        selectedTextLS := k
        break
    }
}
ddlLSMode.Text := selectedTextLS

valRSMode := IniRead(A_ScriptDir "\" XboxIni, "SETTINGS", "RightStickMode", "0")
MainGui.Add("Text", "xs+115 y+10 w155", T("Right Stick Mode") ":")
ddlRSMode := MainGui.Add("DropDownList", "x+10 yp-3 w110 Choose1", [T("default"), T("as triggers"), T("as buttons")])
RSModeMap := Map(T("default"), "0", T("as triggers"), "1", T("as buttons"), "2")
CtrlSettings["RightStickMode"] := {type: "mapped_ddl", ctrl: ddlRSMode, file: XboxIni, sec: "SETTINGS", valMap: RSModeMap}

; Синхронизация текущего значения RightStickMode
selectedText := T("default")
for k, v in RSModeMap {
    if (v == valRSMode) {
        selectedText := k
        break
    }
}

ddlRSMode.Text := selectedText

; --- Группа 4: OTHERS (Под группой Swaps) ---
MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x175 y+22 w480 Center h60 Section", T("Misc"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

AddInput(ConfigIni, "Gamepad", "RumbleStrength", T("Rumble strength :"), , "xs+165 ys+25", 110, 50)

; --- Сноски и примечания (внизу вкладки) ---
MainGui.Add("Text", "x25 y+12 w800 cRed", T("* Linearity"))
MainGui.Add("Text", "x25 y+0 w820", T("Adjusts stick sensitivity curve:"))
MainGui.Add("Text", "x25 y+1 w820", T("0: Lower sensitivity near the center for precise aiming (Exponential)"))
MainGui.Add("Text", "x25 y+1 w820", T("50 (Default): Perfectly linear response"))
MainGui.Add("Text", "x25 y+1 w820", T("100: Higher sensitivity near the center for instant response (Logarithmic)"))
MainGui.Add("Text", "x25 y+2 w800 cRed", T("** Left Stick Mode:"))
MainGui.Add("Text", "x25 y+0 w820", T("'AutoSprintButton' held down when stick direction in: default - none; 1 - front hemisphere (45 degrees); 2 - all directions"))

; =========================================
; TAB 7: STEERING
; =========================================
Tabs.UseTab("Steering")



; --- ГРУППА 1: Wheel settings (Driving mode) ---
MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x175 y65 w480 h120 Center Section", T("Wheel settings (Driving mode)"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

AddInput(XboxIni, "SETTINGS", "SteeringWheelAngle", T("Steering Wheel Angle"), , "xs+135 ys+40", 160, 50, true, "0-360")
AddInput(XboxIni, "SETTINGS", "LinearityWheel", T("Steering Wheel Linearity"), , "xs+135 y+10", 160, 50)

; --- ГРУППА 2: External Pedals Settings ---
MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x175 y+50 w480 h225 Center Section", T("External Pedals Settings"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

AddMappedDropdown(ConfigIni, "ExternalPedals", "DInput", T("DirectInput Search"), [T("On"), T("Off")], Map(T("On"), "1", T("Off"), "0"), "xs+135 ys+40", 160, 55)

; Список поддерживаемых осей педалей
AxesList := ["X", "Y", "Z", "R", "U", "V", "Z-ROTATION", "X-ROTATION", "Y-ROTATION", "DIAL"]
AxesMap := Map()
for axis in AxesList {
    AxesMap[axis] := axis
}

; Выбор осей для Педали 1 и Педали 2
AddMappedDropdown(ConfigIni, "ExternalPedals", "Pedal1Axis", T("Pedal 1 Axis"), AxesList, AxesMap, "xs+135 y+15", 110, 105)
AddMappedDropdown(ConfigIni, "ExternalPedals", "Pedal2Axis", T("Pedal 2 Axis"), AxesList, AxesMap, "xs+135 y+15", 110, 105)

;AddInput(ConfigIni, "ExternalPedals", "DeviceName", T("Device Name"), "AUTO", "xs+15 y+40", 110, 340)
MainGui.Add("Text", "xs+135 y+15 w120", T("Device Name:"))
AddInput(ConfigIni, "ExternalPedals", "DeviceName", T(""), "AUTO", "xs+125 y+15", 0, 225, false)

MainGui.Add("Text", "x20 y+188 w820 cRed", T("External Pedals:"))
MainGui.Add("Text", "x20 y+5 w820", T("Use pedals as analog triggers. Note: This is an experimental feature; proper functioning is not guaranteed."))
MainGui.Add("Text", "x20 y+5 w820", T("Connect your wheel/pedals, set DirectInput search to On and launch JCAdvance. If you see message: `n'[Pedals Search] ID 0: Found device 'Your wheel/pedlas name' -> APPROVED!', configure the correct pedal axes and you've golden."))
MainGui.Add("Text", "x20 y+5 w820", T("If you can't see your wheel/pedals name, replace AUTO with your device's name exactly as it appears in joy.cpl"))

; =========================================
; TAB 8: PROFILES
; =========================================
Tabs.UseTab("Profiles")


MainGui.Add("Text", "x20 y65 w810 Center", T("Profile Manager"))

MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x125 y120 w610 h140 Center cBlue", T("Load Delete Profile"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

MainGui.Add("Text", "x140 y160 w450", T("Current Active Profile: ") ActiveProfile)

ProfileList := []
Loop Files, A_ScriptDir "\XboxProfiles\*.ini", "F" {
    ProfileList.Push(A_LoopFileName)
}
MainGui.Add("Text", "x140 y203 w130", T("Select Profile:"))
ProfileDdl := MainGui.Add("DropDownList", "x270 y202 w180 Choose1", ProfileList)
ProfileDdl.Text := ActiveProfile

LoadProfileBtn := MainGui.Add("Button", "x470 y198 w120 h26", T("Load Profile"))
LoadProfileBtn.OnEvent("Click", (*) => LoadProfileEvent(ProfileDdl.Text))

DeleteProfileBtn := MainGui.Add("Button", "x600 y198 w120 h26", T("Delete Profile"))
DeleteProfileBtn.OnEvent("Click", (*) => DeleteProfileEvent(ProfileDdl.Text))

LoadProfileEvent(selectedProfile) {
    if (selectedProfile == "")
        return
    SmartIniWrite(selectedProfile, A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile")
    Reload()
}

MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x125 y280 w610 h140 Center cBlue", T("Create New Profile"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

MainGui.Add("Text", "x140 y330 w400", T("Enter Name for the New Profile:"))

NewProfileEdit := MainGui.Add("Edit", "x140 y365 w310")
CreateProfileBtn := MainGui.Add("Button", "x470 y363 w175 h26", T("Create and Load Profile"))
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
                    SmartIniWrite("NONE", targetFile, "JOYCONS", key)
                } else if (key == "L4" || key == "R4") {
                    SmartIniWrite("NONE", targetFile, "DUALSENSE-EDGE", key)
                } else {
                    SmartIniWrite("NONE", targetFile, "Xbox", key)
                }
                SmartIniWrite("NONE", targetFile, "KEYBOARD-MOUSE", key)
            }
        }
        
        SmartIniWrite(profileName, A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile")
        MsgBox(T("Profile '") profileName T("' created successfully!"), T("Success"), "Iconi")
        Reload()
    } catch as err {
        MsgBox(T("Failed to create profile!`nDetails: ") err.Message, T("Error"), "IconX")
    }
}


; =========================================
; TAB 9: SECOND (Secondary Gamepad)
; =========================================
Tabs.UseTab("Second")

; --- ГЛАВНЫЙ ПЕРЕКЛЮЧАТЕЛЬ И ЯРКОСТЬ (По центру вверху) ---
;MainGui.SetFont("cBlue Bold q5", "Segoe UI")
;MainGui.Add("GroupBox", "x225 y50 w380 h90 Center Section", T("Secondary Gamepad Status"))
;MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

; Enabled (On/Off)
AddMappedDropdown(ConfigIni, "SecondaryGamepad", "Enabled", T("Secondary Gamepad"), [T("Off"), T("On")], Map(T("Off"), "0", T("On"), "1"), "x280 y85", 140, 100)


; --- НАСТРОЙКИ СТИКОВ (Две колонки, как в Analog) ---

; ЛЕВАЯ КОЛОНКА
MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x25 y160 w380 h185 Center Section", T("Left Hand"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

AddInput(ConfigIni, "SecondaryGamepad", "DeadZoneLeftTrigger", T("DeadZone Trigger :"), "0", "xs+100 ys+40", 125, 42)
AddInput(ConfigIni, "SecondaryGamepad", "DeadZoneLeftStickX", T("DeadZone Stick X :"), "5", "xs+100 y+6", 125, 42)
AddInput(ConfigIni, "SecondaryGamepad", "DeadZoneLeftStickY", T("DeadZone Stick Y :"), "5", "xs+100 y+6", 125, 42)
AddToggle(ConfigIni, "SecondaryGamepad", "InvertLeftStickX", T("Invert Stick X"), "xs+80 y298")
AddToggle(ConfigIni, "SecondaryGamepad", "InvertLeftStickY", T("Invert Stick Y"), "xs+206 y298")

; ПРАВАЯ КОЛОНКА
MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x425 y160 w380 h185 Center Section", T("Right Hand"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

AddInput(ConfigIni, "SecondaryGamepad", "DeadZoneRightTrigger", "   " T(": DeadZone Trigger"), "0", "xs+100 ys+40", 185, 42, true, "0-100", 1)
AddInput(ConfigIni, "SecondaryGamepad", "DeadZoneRightStickX", "   " T(": DeadZone Stick X"), "5", "xs+100 y+11", 185, 42, true, "0-100", 1)
AddInput(ConfigIni, "SecondaryGamepad", "DeadZoneRightStickY", "   " T(": DeadZone Stick Y"), "5", "xs+100 y+11", 185, 42, true, "0-100", 1)
AddToggle(ConfigIni, "SecondaryGamepad", "InvertRightStickX", T("Invert Stick X"), "xs+80 y298")
AddToggle(ConfigIni, "SecondaryGamepad", "InvertRightStickY", T("Invert Stick Y"), "xs+206 y298")


; --- СНОСКА ВНИЗУ ---
MainGui.SetFont("cRed Bold q5", "Segoe UI")
MainGui.Add("Text", "x25 y660 w800", T("Note:"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")
MainGui.Add("Text", "x25 y+3 w800", T("The second gamepad supports only the basic features of the XBOX/DS4 virtual controller"))

; =========================================
; TAB 10: SETTINGS
; =========================================
Tabs.UseTab("Settings")

MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x175 y+30 w480 h175 Center cBlue", T("General Settings"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

MainGui.Add("Text", "x300 y110 w85", T("Layout:"))
chkNintendo := (Layout == "Nintendo") ? " Checked1" : ""
chkSony := (Layout == "Sony") ? " Checked1" : ""
radNintendo := MainGui.Add("Radio", "x385 y110" chkNintendo, "Nintendo")
radSony := MainGui.Add("Radio", "x475 y110" chkSony, "Sony")

radNintendo.OnEvent("Click", (*) => SetLayout("Nintendo"))
radSony.OnEvent("Click", (*) => SetLayout("Sony"))

SetLayout(value, *) {
    SmartIniWrite(value, A_ScriptDir "\" ConfigIni, "ConfigGUI", "Layout")
    Reload()
}

AddMappedDropdown(ConfigIni, "Gamepad", "EmulatedController", T("Emulated Controller *"), ["XBOX", "DS4"], Map("XBOX", "XBOX", "DS4", "DS4"), "x300 y+20", 155, 60)

LanguagesList := ["English"]
Loop Files, A_ScriptDir "\Language\*.ini", "F" {
    LanguagesList.Push(StrReplace(A_LoopFileName, ".ini", ""))
}

MainGui.Add("Text", "x300 y185 w75", T("Language:"))
LangDdl := MainGui.Add("DropDownList", "x420 y185 w105", LanguagesList)
LangDdl.Text := CurrentLang
LangDdl.OnEvent("Change", (ctrl, *) => ChangeLanguageEvent(ctrl.Text))

ChangeLanguageEvent(selectedLang) {
    SmartIniWrite(selectedLang, A_ScriptDir "\" ConfigIni, "ConfigGUI", "Language")
    Reload()
}

; --- Группа 2: Misc Settings ---
MainGui.SetFont("cBlue Bold q5", "Segoe UI")
MainGui.Add("GroupBox", "x175 y270 w480 h180 Center Section", T("Special"))
MainGui.SetFont("cDefault Norm s10 q5", "Segoe UI")

MainGui.Add("Text", "xs+65 ys+30 w380", T("While the emulator is running, you can terminate a process `nby the hotkey — for example, a game that has frozen"))

AddHotkey(ConfigIni, "Settings", "KillHotkey", T("Kill Process by Hotkey"), KbmKeys, BindKbm, "xs+65 ys+85", 140, 150)

;MainGui.Add("Text", "xs+45 y+10 w120", T("Exe Name:"))

AddInput(ConfigIni, "Settings", "KillProcessName", T("Exe Name:"), "", "xs+65 y+20", 75, 280, false)

MainGui.Add("Text", "x20 y+225 w820 cRed", T("* Emulated Controller: DS4 Mode for Nintendo gamepads only"))
MainGui.Add("Text", "x20 y+5 w820", T("For DirectInput games, like Half-Life 2, F.E.A.R., NFS classic series, you can change the controller type to DS4. When you launch JCAdvacne, ‘Wireless Controller’ will appear instead of ‘Xbox 360 Controller’"))


; =========================================
; GLOBAL BUTTONS & EVENTS
; =========================================
Tabs.UseTab() 

; Главная кнопка Save All
SaveBtn := MainGui.Add("Button", "x720 y5 w120 h25 Default", T("Save All"))
SaveBtn.OnEvent("Click", SaveAllConfigs)

; --- АВТО-ОТСЛЕЖИВАНИЕ ИЗМЕНЕНИЙ ---
SetUnsaved(ctrl?, info?) {
    Global IsUnsavedChanges := true
}

for hwnd, ctrl in MainGui {
    cType := ctrl.Type
    
    if (cType = "Button" || cType = "Tab3" || cType = "Text" || cType = "GroupBox" || cType = "Picture")
        continue
        
    try {
        ctrl.OnEvent("Change", SetUnsaved, 1)
    } catch {
        try ctrl.OnEvent("Change", SetUnsaved)
    }
    
    try {
        ctrl.OnEvent("Click", SetUnsaved, 1)
    } catch {
        try ctrl.OnEvent("Click", SetUnsaved)
    }
}

; --- ФУНКЦИЯ УМНОГО ВЫХОДА ---
ConfirmExit(guiObj?) {
    Global IsUnsavedChanges
    if (!IsUnsavedChanges)
        ExitApp()
        
    res := MsgBox(T("You have unsaved changes. Do you want to save them before exiting?"), T("Unsaved Changes"), "YesNoCancel Icon?")
    
    if (res == "Yes") {
        SaveAllConfigs()
        ExitApp()
    } else if (res == "No") {
        ExitApp()
    } else {
        return 1 ; Отменяет закрытие окна
    }
}

; Привязываем умный выход к крестику главного окна
MainGui.OnEvent("Close", ConfirmExit)

; =========================================
; SCROLLING (for low resolutions & high DPI)
; =========================================
MainGui.Show("Hide") 

Tabs.GetPos(&tx, &ty, &tw, &th)
ReqW := tw + 14
ReqH := th + 14

MonitorGetWorkArea(1, &WL, &WT, &WR, &WB)

; Вычисляем масштаб DPI (100% = 1.0, 175% = 1.75, 200% = 2.0)
DPIScale := A_ScreenDPI / 96

; Переводим физические пиксели экрана в логические (понятные для GUI)
MaxH_Physical := WB - WT - 40
MaxH := MaxH_Physical / DPIScale

if (ReqH > MaxH) {
    Global Viewport := Gui("-MaximizeBox", MainGui.Title)
    ; ВАЖНО: Привязываем умный выход к крестику окна скролла
    Viewport.OnEvent("Close", ConfirmExit) 
    Viewport.Opt("+0x200000") 
    
    MainGui.Opt("-Caption +Parent" Viewport.Hwnd)
    MainGui.Show("x0 y0 w" ReqW " h" ReqH)
    
    ScrollbarW := SysGet(2) 
    Viewport.Show("w" (ReqW + ScrollbarW) " h" MaxH)
    
    Global ScrollMax := ReqH
    Global ScrollPage := MaxH
    Global ScrollPos := 0
    
    UpdateScrollInfo()
    OnMessage(0x0115, OnVScroll)
    OnMessage(0x020A, OnMouseWheel)
} else {
    MainGui.Show("w" ReqW " h" ReqH)
}

; --- Функции движка прокрутки ---
UpdateScrollInfo() {
    Global ScrollMax, ScrollPage, ScrollPos, Viewport
    si := Buffer(28, 0)
    NumPut("UInt", 28, si, 0)
    NumPut("UInt", 0x1 | 0x2 | 0x4, si, 4) ; SIF_RANGE | SIF_PAGE | SIF_POS
    NumPut("Int", 0, si, 8)
    NumPut("Int", ScrollMax - 1, si, 12)
    NumPut("UInt", ScrollPage, si, 16)
    NumPut("Int", ScrollPos, si, 20)
    DllCall("SetScrollInfo", "Ptr", Viewport.Hwnd, "Int", 1, "Ptr", si, "Int", 1)
}

OnVScroll(wParam, lParam, msg, hwnd) {
    Global ScrollMax, ScrollPage, ScrollPos, Viewport, MainGui
    if (hwnd != Viewport.Hwnd)
        return
        
    action := wParam & 0xFFFF
    oldPos := ScrollPos
    
    if (action = 0)      ; Вверх
        ScrollPos -= 40
    else if (action = 1) ; Вниз
        ScrollPos += 40
    else if (action = 2) ; Клик выше
        ScrollPos -= ScrollPage
    else if (action = 3) ; Клик ниже
        ScrollPos += ScrollPage
    else if (action = 5) { ; Перетаскивание
        si := Buffer(28, 0)
        NumPut("UInt", 28, si, 0)
        NumPut("UInt", 0x10, si, 4)
        DllCall("GetScrollInfo", "Ptr", hwnd, "Int", 1, "Ptr", si)
        ScrollPos := NumGet(si, 24, "Int")
    }
    
    limit := ScrollMax - ScrollPage
    if (ScrollPos > limit)
        ScrollPos := limit
    if (ScrollPos < 0)
        ScrollPos := 0
        
    if (ScrollPos != oldPos) {
        MainGui.Move(, -ScrollPos)
        UpdateScrollInfo()
    }
}

OnMouseWheel(wParam, lParam, msg, hwnd) {
    Global Viewport
    dir := (wParam >> 16) > 0x7FFF ? 1 : -1
    if (dir = 1)
        OnVScroll(1, 0, 0, Viewport.Hwnd)
    else
        OnVScroll(0, 0, 0, Viewport.Hwnd)
}

; --- ФУНКЦИЯ УМНОГО СОХРАНЕНИЯ ---
SmartIniWrite(Value, Filename, Section, Key) {
    ; Читаем текущее значение из файла (если ключа нет, возвращаем спец. строку)
    oldVal := IniRead(Filename, Section, Key, "@@@NULL@@@")
    
    ; Переводим новое значение в строку для точного сравнения
    newVal := String(Value)
    
    ; Если значения отличаются — физически перезаписываем файл
    if (oldVal != newVal) {
        IniWrite(newVal, Filename, Section, Key)
    }
}

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
                        val := Trim(parts[2])         ; Виртуальная кнопка Xbox
                        keyToClear := Trim(parts[1])  ; Физическая кнопка геймпада
                        
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

                        ; --- УМНАЯ ПРОВЕРКА ---
                        ; Если интерфейс всё еще содержит этот же бинд, не трогаем его!
                        stillMapped := false
                        if (CtrlXbox.Has(val) && CtrlXbox[val].Text == keyToClear)
                            stillMapped := true
                        if (CtrlExtraXbox.Has(keyToClear) && CtrlExtraXbox[keyToClear].Text == val)
                            stillMapped := true

                        if (isXboxBtn && shouldClear && !stillMapped)
                            SmartIniWrite("NONE", A_ScriptDir "\" XboxIni, sec, keyToClear)
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
                    SmartIniWrite(key, A_ScriptDir "\" XboxIni, "Xbox", btn) 
                }
            }
        }

        ; --- Запись дополнительных кнопок напрямую в их системные секции ---
        for key in ["SL", "SR", "HOME", "CAPTURE"] {
            ctrl := CtrlExtraXbox[key]
            val := (ctrl.Text != "") ? ctrl.Text : "NONE"
            SmartIniWrite(val, A_ScriptDir "\" XboxIni, "JOYCONS", key)
        }
        for key in ["L4", "R4"] {
            ctrl := CtrlExtraXbox[key]
            val := (ctrl.Text != "") ? ctrl.Text : "NONE"
            SmartIniWrite(val, A_ScriptDir "\" XboxIni, "DUALSENSE-EDGE", key)
        }

        ; --- 3 и 4: Сохраняем активный Layout, НЕ стирая уникальные кнопки другого ---
        if (Layout == "Sony") {
            for key in SonyMapping {
                ctrl := CtrlSony[key]
                val := (ctrl.Text != "") ? ctrl.Text : "NONE"
                SmartIniWrite(val, A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key)
            }
        } else {
            for key in JoyconMapping {
                ctrl := CtrlJoyCon[key]
                val := (ctrl.Text != "") ? ctrl.Text : "NONE"
                SmartIniWrite(val, A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key)
            }
        }
		
        ; --- 5. Вкладка WHEEL ---
        valAct := (CtrlWheel["WHEEL-ACTIVATION"].ddl.Text != "") ? CtrlWheel["WHEEL-ACTIVATION"].ddl.Text : "NONE"
        SmartIniWrite(valAct, A_ScriptDir "\" XboxIni, "Motion", "WHEEL-ACTIVATION")
        
		; Сохранение нового параметра MELEE-GESTURE в зависимости от выбранного режима (Xbox или KB/M)
        if (CtrlWheel.Has("MELEE-GESTURE")) {
            obj := CtrlWheel["MELEE-GESTURE"]
            valMelee := (obj.ddl.Text != "") ? obj.ddl.Text : "NONE"
            if (obj.rKbm.Value == 1) {
                SmartIniWrite(valMelee, A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "MELEE-GESTURE")
                SmartIniWrite("NONE", A_ScriptDir "\" XboxIni, "Motion", "MELEE-GESTURE")
            } else {
                SmartIniWrite(valMelee, A_ScriptDir "\" XboxIni, "Motion", "MELEE-GESTURE")
                SmartIniWrite("NONE", A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "MELEE-GESTURE")
            }
        }
		
		; Сохранение AutoSprintButton
        if (CtrlWheel.Has("AutoSprintButton")) {
            obj := CtrlWheel["AutoSprintButton"]
            valSprint := (obj.ddl.Text != "") ? obj.ddl.Text : "NONE"
            if (obj.rKbm.Value == 1) {
                SmartIniWrite(valSprint, A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "AutoSprintButton")
                SmartIniWrite("NONE", A_ScriptDir "\" XboxIni, "Xbox", "AutoSprintButton")
            } else {
                SmartIniWrite(valSprint, A_ScriptDir "\" XboxIni, "Xbox", "AutoSprintButton")
                SmartIniWrite("NONE", A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", "AutoSprintButton")
            }
        }
		
        for key in WheelMapping {
            obj := CtrlWheel[key]
            val := (obj.ddl.Text != "") ? obj.ddl.Text : "NONE"
            if (obj.rKbm.Value == 1) {
                SmartIniWrite(val, A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key)
                SmartIniWrite("NONE", A_ScriptDir "\" XboxIni, "Motion", key)
            } else {
                SmartIniWrite(val, A_ScriptDir "\" XboxIni, "Motion", key)
                SmartIniWrite("NONE", A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key)
            }
        }
        
		; Сохранение направлений правого стика (в секцию Xbox или KEYBOARD-MOUSE)
        for key in RsButtonMapping {
            obj := CtrlWheel[key]
            val := (obj.ddl.Text != "") ? obj.ddl.Text : "NONE"
            if (obj.rKbm.Value == 1) {
                SmartIniWrite(val, A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key)
                SmartIniWrite("NONE", A_ScriptDir "\" XboxIni, "Xbox", key)
            } else {
                SmartIniWrite(val, A_ScriptDir "\" XboxIni, "Xbox", key)
                SmartIniWrite("NONE", A_ScriptDir "\" XboxIni, "KEYBOARD-MOUSE", key)
            }
        }
		
		; --- 6. Вкладки SETTINGS ---
        for id, obj in CtrlSettings {
            if (obj.type == "chk")
                val := obj.ctrl.Value ? "1" : "0"
            else if (obj.type == "mapped_ddl")
                val := obj.valMap[obj.ctrl.Text]
            else
                val := (obj.ctrl.Text != "") ? obj.ctrl.Text : "NONE"
                
            ; Если есть свойство key — берем его, если нет — используем старый id!
            realKey := HasProp(obj, "key") ? obj.key : id
            SmartIniWrite(val, A_ScriptDir "\" obj.file, obj.sec, realKey)
        }
        
        SmartIniWrite(Layout, A_ScriptDir "\" ConfigIni, "ConfigGUI", "Layout")
        SmartIniWrite(ActiveProfile, A_ScriptDir "\" ConfigIni, "ConfigGUI", "LayoutProfile")
		
        Global IsUnsavedChanges := false
        ;MsgBox(T("Settings successfully saved!"), T("Success"))
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
		Global IsUnsavedChanges := true
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
	
		; Проверяем, принадлежит ли кнопка Bind верхнему блоку вкладки Xbox
        isMainXboxTab := false
        for k, v in CtrlXbox {
            if (v == ddl) {
                isMainXboxTab := true
                break
            }
        }
        
        ; Если это верхний блок, строго блокируем Extra-кнопки
        if (isMainXboxTab && (detectedBtn == "SL" || detectedBtn == "SR" || detectedBtn == "HOME" || detectedBtn == "CAPTURE" || detectedBtn == "L4" || detectedBtn == "R4")) {
            MsgBox(T("Please use the 'Extra Buttons' section below to bind this button!"), T("Notice"), "Iconi")
            return
        }
	
        SetDdlValue(ddl, detectedBtn)
		Global IsUnsavedChanges := true
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
        return (Layout == "Sony") ? "OPTIONS" : "PLUS"
    if (mask & 0x00020)
        return (Layout == "Sony") ? "SHARE" : "MINUS"
        
    if (mask & 0x00040)
        return "L3"
    if (mask & 0x00080)
        return "R3"
        
    if (mask & 0x00100)
        return (Layout == "Sony") ? "L1" : "L"
    if (mask & 0x00200)
        return (Layout == "Sony") ? "R1" : "R"
        
    ; Аналоговые курки (ZL / ZR и L2 / R2)
    if (mask & 0x00400)
        return (Layout == "Sony") ? "L2" : "ZL"
    if (mask & 0x000800)
        return (Layout == "Sony") ? "R2" : "ZR"
        
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