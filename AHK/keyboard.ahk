#Requires AutoHotkey v2.0
#SingleInstance Force

; ==============================================================================
; МГНОВЕННЫЙ КИЛЛ-СВИТЧ (ВЫЗОВ/ЗАКРЫТИЕ ПО ОДНОЙ КНОПКЕ ГЕЙМПАДА)
; ==============================================================================
global LockFile := A_Temp . "\osk_couch_state.lock"
if FileExist(LockFile) {
    try FileDelete(LockFile)
    ExitApp()
}
try FileAppend("1", LockFile)

; ==============================================================================
; НАСТРОЙКИ (CONFIG.INI И ЦВЕТОВАЯ ПАЛИТРА)
; ==============================================================================
global ConfigFile      := A_ScriptDir . "\Config.ini"
global ScaleFactor     := Float(IniRead(ConfigFile, "KEYBOARD", "ScaleFactor", "1.15"))
global KeyboardOpacity := Integer(IniRead(ConfigFile, "KEYBOARD", "Opacity", "235"))
global BaseFontSize    := Integer(IniRead(ConfigFile, "KEYBOARD", "FontSize", "14"))
global KeyBaseW        := Integer(IniRead(ConfigFile, "KEYBOARD", "KeyWidth", "40"))

global COLOR_BG        := "141414"  ; Фон окна
global COLOR_KEY       := "252526"  ; Фон обычной кнопки
global COLOR_HOVER     := "0078D7"  ; Подсветка под гиро-мышь (голубой)
global COLOR_ACTIVE    := "005A9E"  ; Вспышка клика
global COLOR_TEXT      := "FFFFFF"  ; Цвет букв

global ShiftState      := 0         ; 0 = Выкл, 1 = Одиночный Shift, 2 = Caps Lock
global IsSymLayer      := false     ; Флаг 3-го слоя символов (#+=)
global CurrentLang     := "RU"
global KeyMap          := Map()
global KeyList         := []
global LastHover       := 0
global GripCtrl        := 0
global LangCtrl        := 0
global EnterCtrl       := 0
global SpaceCtrl       := 0
global ShiftCtrl       := 0
global SymCtrl         := 0

; Считывание параметров повтора клавиатуры из Windows
rawDelay := Buffer(4, 0), DllCall("SystemParametersInfo", "UInt", 0x0016, "UInt", 0, "Ptr", rawDelay, "UInt", 0)
global KeyRepeatDelay    := (NumGet(rawDelay, "UInt") + 1) * 250 ; Задержка перед повтором (мс)

rawSpeed := Buffer(4, 0), DllCall("SystemParametersInfo", "UInt", 0x000A, "UInt", 0, "Ptr", rawSpeed, "UInt", 0)
global KeyRepeatInterval := Integer(400 - (NumGet(rawSpeed, "UInt") * 367 / 31)) ; Интервал повтора (мс)

; ==============================================================================
; РАСКЛАДКА V2.2 (СТРОГИЙ ФОРМАТ 6 ЭЛЕМЕНТОВ: ВСЕ РЯДЫ РОВНО ПО 12.0u)
; [1:EN, 2:EN_Shift, 3:RU, 4:RU_Shift, 5:SYM, 6:Width]
; ==============================================================================
global Layout := [
    ; Ряд 1: Цифры / Слой символов: Esc, цифры, Tab (12.0u)
    [["``","~","ё","Ё","{Esc}",1.0], ["1","!","1","!","1",1.0], ["2","@","2",'"',"2",1.0], ["3","#","3","№","3",1.0],
     ["4","$","4",";","4",1.0], ["5","%","5","%","5",1.0], ["6","^","6",":","6",1.0], ["7","&","7","?","7",1.0],
     ["8","*","8","*","8",1.0], ["9","(","9","(","9",1.0], ["0",")","0",")","0",1.0], ["-","_","-","_","{Tab}",1.0]],

    ; Ряд 2: Буквы / Слой символов: скобки и математика (12.0u)
    [["q","Q","й","Й","[",1.0], ["w","W","ц","Ц","]",1.0], ["e","E","у","У","{",1.0], ["r","R","к","К","}",1.0],
     ["t","T","е","Е","<",1.0], ["y","Y","н","Н",">",1.0], ["u","U","г","Г","=",1.0], ["i","I","ш","Ш","+",1.0],
     ["o","O","щ","Щ","-",1.0], ["p","P","з","З","*",1.0], ["[","{","х","Х","/",1.0], ["]","}","ъ","Ъ","\",1.0]],

    ; Ряд 3: Буквы + Backspace ⌫ (1.0u) / Слой символов: спецсимволы + Backspace ⌫ (12.0u)
    [["a","A","ф","Ф","!",1.0], ["s","S","ы","Ы","@",1.0], ["d","D","в","В","#",1.0], ["f","F","а","А","$",1.0],
     ["g","G","п","П","%",1.0], ["h","H","р","Р","^",1.0], ["j","J","о","О","&",1.0], ["k","K","л","Л","_",1.0],
     ["l","L","д","Д","~",1.0], [";",":","ж","Ж",'"',1.0], ["'","`"","э","Э","'",1.0],
     ["{Backspace}","{Backspace}","{Backspace}","{Backspace}","{Backspace}",1.0]],

    ; Ряд 4: Shift 🠉 (1.5u) | Буквы + /| и \| | Enter ⮠ (1.5u) = 12.0u
    [["Shift","Shift","Shift","Shift","Shift",1.5],
     ["z","Z","я","Я","(",1.0], ["x","X","ч","Ч",")",1.0], ["c","C","с","С","№",1.0], ["v","V","м","М","₽",1.0],
     ["b","B","и","И","€",1.0], ["n","N","т","Т","£",1.0], ["m","M","ь","Ь","|",1.0], ["/","|","б","Б","«",1.0],
     ["\","\","ю","Ю","»",1.0],
     ["{Enter}","{Enter}","{Enter}","{Enter}","{Enter}",1.5]],

    ; Ряд 5: § (1.0u) | ! / ← (1.0u) | , / ↑ (1.0u) | Пробел ␣ (5.0u) | . / ↓ (1.0u) | ? / → (1.0u) | Язык (1.5u) | Ручка (0.5u) = 12.0u
    [["Sym","Sym","Sym","Sym","Sym",1.0],
     ["!","!","!","!","{Left}",1.0],
     [",","<",",","/","{Up}",1.0],
     ["{Space}","{Space}","{Space}","{Space}","{Space}",5.0],
     [".",">",".","\","{Down}",1.0],
     ["?","?","?","?","{Right}",1.0],
     ["Lang","Lang","Lang","Lang","Lang",1.5],
     ["⤡","⤡","⤡","⤡","⤡",0.5]]
]

; ==============================================================================
; ПОСТРОЕНИЕ ИНТЕРФЕЙСА
; ==============================================================================
global DpiRatio := A_ScreenDPI / 96
global Scale    := ScaleFactor * DpiRatio

; Опрос языка активного окна Windows ДО создания кнопок (устраняет мигание)
if (targetHwnd := WinExist("A")) {
    threadId := DllCall("GetWindowThreadProcessId", "Ptr", targetHwnd, "Ptr", 0)
    hkl := DllCall("GetKeyboardLayout", "UInt", threadId, "Ptr")
    CurrentLang := ((hkl & 0xFFFF) == 0x0419) ? "RU" : "EN"
}

VKGui := Gui("+AlwaysOnTop +ToolWindow -Caption +E0x08000000 -DPIScale", "OSK_Couch")
VKGui.BackColor := COLOR_BG

mX := Integer(10 * Scale), mY := Integer(10 * Scale)
VKGui.MarginX := mX, VKGui.MarginY := mY

BaseW  := Integer(KeyBaseW * Scale)
BaseH  := Integer(52 * Scale)
Margin := Integer(6 * Scale)
FSize  := Integer(BaseFontSize * ScaleFactor)

VKGui.SetFont("s" . FSize . " c" . COLOR_TEXT . " Bold", "Segoe UI")

; Расчет габаритов окна
KeysTotalW := Integer(BaseW * 12.0 + Margin * 11)
GuiW := KeysTotalW + mX * 2

; Верхняя планка: DragBar + Кнопка закрытия ✕
DragH  := Integer(20 * Scale)
CloseW := DragH
DragW  := KeysTotalW - CloseW - Margin

global DragBar := VKGui.Add("Text", "xm ym w" . DragW . " h" . DragH . " Center 0x200 +0x100 Background" . COLOR_KEY, "JC Keyboard")
DragBar.SetFont("s" . Integer(FSize * 0.7) . " c888888 Norm")

global BtnClose := VKGui.Add("Text", "x+" . Margin . " yp w" . CloseW . " h" . DragH . " Center 0x200 +0x100 Background" . COLOR_KEY, "✕")
BtnClose.SetFont("s" . Integer(FSize * 0.7) . " cAAAAAA Norm Bold")
BtnClose.OnEvent("Click", OnCloseClick)

; Создание клавиш с точным маппингом 6-го элемента (Width)
For rowIndex, row in Layout {
    For keyIndex, keyData in row {
        xPos := (keyIndex == 1) ? "xm" : "x+" . Margin
        yPos := (keyIndex == 1) ? "y+" . Margin : "yp"
        kWidth := Integer(BaseW * keyData[6] + (keyData[6] - 1) * Margin)

        ;lbl := (keyData[1] = "{Space}") ? "Space" : (keyData[1] = "{Backspace}") ? "⌫" : (keyData[1] = "{Enter}") ? "⮠" : (keyData[1] = "Shift") ? "🠉" : (keyData[1] = "Sym") ? "§" : (keyData[1] = "Lang") ? CurrentLang : (CurrentLang == "RU" ? keyData[3] : keyData[1])
		lbl := (keyData[1] = "{Space}") ? "␣" : (keyData[1] = "{Backspace}") ? "⌫" : (keyData[1] = "{Enter}") ? "⮠" : (keyData[1] = "Shift") ? "🠉" : (keyData[1] = "Sym") ? "§" : (keyData[1] = "Lang") ? CurrentLang : (CurrentLang == "RU" ? keyData[3] : keyData[1])

        ctrl := VKGui.Add("Text", xPos . " " . yPos . " w" . kWidth . " h" . BaseH . " Center 0x200 0x80 +0x100 Background" . COLOR_KEY, lbl)
        ctrl.RowIdx := rowIndex, ctrl.KeyIdx := keyIndex, ctrl.UWidth := keyData[6]

        if (keyData[1] == "⤡") {
            global GripCtrl := ctrl
            KeyMap[ctrl.Hwnd] := {EN: "⤡", EN_S: "⤡", RU: "⤡", RU_S: "⤡", SYM: "⤡", Ctrl: ctrl}
        } else if (keyData[1] == "Lang") {
            global LangCtrl := ctrl
            ctrl.OnEvent("Click", OnLangToggleClick)
            KeyMap[ctrl.Hwnd] := {EN: "Lang", EN_S: "Lang", RU: "Lang", RU_S: "Lang", SYM: "Lang", Ctrl: ctrl}
        } else if (keyData[1] == "Sym") {
            global SymCtrl := ctrl
            ctrl.OnEvent("Click", OnSymToggleClick)
            KeyMap[ctrl.Hwnd] := {EN: "Sym", EN_S: "Sym", RU: "Sym", RU_S: "Sym", SYM: "Sym", Ctrl: ctrl}
        } else if (keyData[1] == "{Enter}") {
            global EnterCtrl := ctrl
            ctrl.SetFont("s" . Integer(FSize * 1.45))
            ctrl.OnEvent("Click", OnKeyClick)
            ctrl.OnEvent("DoubleClick", OnKeyClick)
            KeyMap[ctrl.Hwnd] := {EN: keyData[1], EN_S: keyData[2], RU: keyData[3], RU_S: keyData[4], SYM: keyData[5], Ctrl: ctrl}
		} else if (keyData[1] == "{Space}") {
            global SpaceCtrl := ctrl
            ctrl.SetFont("s" . Integer(FSize * 1.3)) ; Настройка размера значка ␣ (меняй 1.3 на любое число)
            ctrl.OnEvent("Click", OnKeyClick)
            ctrl.OnEvent("DoubleClick", OnKeyClick)
            KeyMap[ctrl.Hwnd] := {EN: keyData[1], EN_S: keyData[2], RU: keyData[3], RU_S: keyData[4], SYM: keyData[5], Ctrl: ctrl}	
        } else if (keyData[1] == "Shift") {
            global ShiftCtrl := ctrl
            ctrl.OnEvent("Click", OnShiftClick)
            ctrl.OnEvent("DoubleClick", OnShiftDoubleClick)
            KeyMap[ctrl.Hwnd] := {EN: "Shift", EN_S: "Shift", RU: "Shift", RU_S: "Shift", SYM: "Shift", Ctrl: ctrl}
        } else {
            ctrl.OnEvent("Click", OnKeyClick)
            ctrl.OnEvent("DoubleClick", OnKeyClick)
            KeyMap[ctrl.Hwnd] := {EN: keyData[1], EN_S: keyData[2], RU: keyData[3], RU_S: keyData[4], SYM: keyData[5], Ctrl: ctrl}
        }
        KeyList.Push(ctrl)
    }
}

; Обработчики системных сообщений Win32
OnMessage(0x0021, WM_MOUSEACTIVATE)    ; Запрет воровать фокус
OnMessage(0x0200, WM_MOUSEMOVE)        ; Hover-подсветка
OnMessage(0x02A3, WM_MOUSELEAVE)       ; Сброс Hover
OnMessage(0x0201, WM_LBUTTONDOWN)      ; Нажатие (перетаскивание, ресайз, автоповтор)
OnMessage(0x0203, WM_LBUTTONDOWN)      ; Быстрый повторный клик на Backspace и стрелки
OnMessage(0x0020, WM_SETCURSOR)        ; Диагональная стрелочка на ручке ⤡
OnMessage(0x0232, WM_EXITSIZEMOVE)     ; Сохранение координат

; Автосинхронизация языка каждые 150 мс
SetTimer(SyncKeyboardLayout, 150)

GuiH := DragH + Margin + Integer(BaseH * 5 + Margin * 4 + mY * 2)
global AspectRatio := GuiW / GuiH

; Минимальный размер (ScaleFactor = 0.40)
global MinScaleFactor := 0.40
global MinGuiW := Integer((KeyBaseW * 12.0 + 6 * 11) * (MinScaleFactor * DpiRatio) + (10 * MinScaleFactor * DpiRatio) * 2)

VKGui.OnEvent("Size", OnGuiSize)

; Чтение координат с защитой от вылета за экран
MonitorGetWorkArea(, &left, &top, &right, &bottom)
savedX := IniRead(ConfigFile, "KEYBOARD", "PosX", "")
savedY := IniRead(ConfigFile, "KEYBOARD", "PosY", "")

if (savedX != "" && savedY != "" && savedX >= left && savedX + GuiW <= right && savedY >= top && savedY + GuiH <= bottom)
    posStr := "x" . savedX . " y" . savedY
else
    posStr := "xCenter y" . (bottom - GuiH - Integer(15 * DpiRatio))

VKGui.Show("NoActivate " . posStr . " w" . GuiW . " h" . GuiH)
WinSetTransparent(KeyboardOpacity, VKGui.Hwnd)

; Начальная синхронизация надписей
UpdateKeyLabels()

; ==============================================================================
; ОБРАБОТЧИКИ НАЖАТИЙ И ВВОДА
; ==============================================================================

OnKeyClick(GuiCtrl, *) {
    global ShiftState, CurrentLang, IsSymLayer
    info := KeyMap[GuiCtrl.Hwnd]
    
    ; Центральный диспетчер: блокировка любой печати текста служебными кнопками
    switch info.EN {
        case "Shift":
            OnShiftClick()
            return
        case "Lang":
            OnLangToggleClick()
            return
        case "Sym":
            OnSymToggleClick()
            return
        case "⤡":
            return
    }

    ; Клавиши с автоповтором отрабатывают мгновенно на mouse-down в WM_LBUTTONDOWN
    if (info.EN == "{Backspace}" || (IsSymLayer && (info.SYM == "{Left}" || info.SYM == "{Right}" || info.SYM == "{Up}" || info.SYM == "{Down}")))
        return

    try SoundPlay(A_WinDir . "\Media\Windows Navigation Start.wav")

    GuiCtrl.Opt("+Background" . COLOR_ACTIVE), GuiCtrl.Redraw()
    SetTimer(() => (KeyMap.Has(GuiCtrl.Hwnd) ? (GuiCtrl.Opt("+Background" . COLOR_KEY), GuiCtrl.Redraw()) : 0), -90)

    ; Выбор отправляемого значения
    if (IsSymLayer) {
        sendVal := info.SYM
    } else {
        sendVal := (CurrentLang == "RU") 
            ? (ShiftState ? info.RU_S : info.RU) 
            : (ShiftState ? info.EN_S : info.EN)
    }
    
    if RegExMatch(sendVal, "^\{.*\}$")
        SendInput(sendVal)
    else
        SendInput("{Text}" . sendVal)

    ; Одиночный Shift сбрасывается после 1 символа (Caps Lock не трогаем)
    if (ShiftState == 1 && !IsSymLayer) {
        ShiftState := 0
        UpdateKeyLabels()
    }
}

OnShiftClick(*) {
    global ShiftState, IsSymLayer
    try SoundPlay(A_WinDir . "\Media\Windows Navigation Start.wav")
    if (IsSymLayer)
        return
    ShiftState := (ShiftState == 2) ? 0 : (ShiftState == 1 ? 0 : 1)
    UpdateKeyLabels()
}

OnShiftDoubleClick(*) {
    global ShiftState, IsSymLayer
    try SoundPlay(A_WinDir . "\Media\Windows Navigation Start.wav")
    if (IsSymLayer)
        return
    ShiftState := 2
    UpdateKeyLabels()
}

OnLangToggleClick(*) {
    try SoundPlay(A_WinDir . "\Media\Windows Navigation Start.wav")
    SendInput("#{Space}")
    global CurrentLang := (CurrentLang == "RU") ? "EN" : "RU"
    UpdateKeyLabels()
}

OnSymToggleClick(*) {
    global IsSymLayer, ShiftState
    try SoundPlay(A_WinDir . "\Media\Windows Navigation Start.wav")
    IsSymLayer := !IsSymLayer
    if (IsSymLayer)
        ShiftState := 0
    UpdateKeyLabels()
}

UpdateKeyLabels() {
    global ShiftState, CurrentLang, IsSymLayer
    For hwnd, item in KeyMap {
        if (item.EN == "Shift") {
            item.Ctrl.Opt("+Background" . (ShiftState ? COLOR_HOVER : COLOR_KEY))
            item.Ctrl.Value := (IsSymLayer) ? "" : ((ShiftState == 2) ? "⇪" : "🠉")
            item.Ctrl.Redraw()
            continue
        }
        if (item.EN == "Lang") {
            item.Ctrl.Value := (CurrentLang == "RU") ? "РУС" : "ENG"
            item.Ctrl.Redraw()
            continue
        }
        if (item.EN == "Sym") {
            item.Ctrl.Opt("+Background" . (IsSymLayer ? COLOR_HOVER : COLOR_KEY))
            item.Ctrl.Value := IsSymLayer ? (CurrentLang == "RU" ? "Абв" : "abc") : "§"
            item.Ctrl.Redraw()
            continue
        }
        if (item.EN == "⤡")
            continue

        if (IsSymLayer) {
            val := item.SYM
        } else {
            val := (CurrentLang == "RU") ? (ShiftState ? item.RU_S : item.RU) : (ShiftState ? item.EN_S : item.EN)
        }

        ;lbl := (val = "{Space}") ? "Space" : (val = "{Backspace}") ? "⌫" : (val = "{Enter}") ? "⮠" : (val = "{Tab}") ? "Tab" : (val = "{Esc}") ? "Esc" : (val = "{Left}") ? "←" : (val = "{Right}") ? "→" : val
		;lbl := (val = "{Space}") ? "␣" : (val = "{Backspace}" || val = "Bksp") ? "⌫" : (val = "{Enter}" || val = "Enter") ? "⮠" : (val = "{Tab}" || val = "Tab") ? "Tab" : (val = "{Esc}") ? "Esc" : (val = "{Left}") ? "←" : (val = "{Right}") ? "→" : val
		lbl := (val = "{Space}") ? "␣" : (val = "{Backspace}" || val = "Bksp") ? "⌫" : (val = "{Enter}" || val = "Enter") ? "⮠" : (val = "{Tab}" || val = "Tab") ? "Tab" : (val = "{Esc}") ? "Esc" : (val = "{Left}") ? "←" : (val = "{Right}") ? "→" : (val = "{Up}") ? "↑" : (val = "{Down}") ? "↓" : val

        item.Ctrl.Value := lbl
    }
}

SyncKeyboardLayout() {
    global CurrentLang
    targetHwnd := WinExist("A")
    if (!targetHwnd || targetHwnd == VKGui.Hwnd)
        return

    threadId := DllCall("GetWindowThreadProcessId", "Ptr", targetHwnd, "Ptr", 0)
    hkl := DllCall("GetKeyboardLayout", "UInt", threadId, "Ptr")
    langId := hkl & 0xFFFF

    detectedLang := (langId == 0x0419) ? "RU" : "EN"
    if (detectedLang != CurrentLang) {
        CurrentLang := detectedLang
        UpdateKeyLabels()
    }
}

; ==============================================================================
; WIN32 СООБЩЕНИЯ, МЫШЬ, СТОРОЖ И ДИНАМИЧЕСКИЙ РЕСАЙЗ
; ==============================================================================

WM_MOUSEACTIVATE(wParam, lParam, msg, hwnd) => 3

WM_SETCURSOR(wParam, lParam, msg, hwnd) {
    global GripCtrl
    if (GripCtrl && hwnd == GripCtrl.Hwnd) {
        hCur := DllCall("LoadCursor", "Ptr", 0, "Int", 32642, "Ptr") ; IDC_SIZENWSE
        DllCall("SetCursor", "Ptr", hCur)
        return 1
    }
}

WM_MOUSEMOVE(wParam, lParam, msg, hwnd) {
    global LastHover, BtnClose, ShiftState, KeyMap
    if (IsSet(BtnClose) && LastHover == BtnClose.Hwnd && hwnd != BtnClose.Hwnd)
        BtnClose.Opt("+Background" . COLOR_KEY), BtnClose.Redraw()

    if (IsSet(BtnClose) && hwnd == BtnClose.Hwnd) {
        BtnClose.Opt("+BackgroundE81123"), BtnClose.Redraw()
        LastHover := hwnd
        SetTimer(CleanStuckHover, 50)
        return
    }

    if (hwnd != LastHover) {
        if (LastHover && KeyMap.Has(LastHover)) {
            isShiftBtn := (KeyMap[LastHover].EN == "Shift" && ShiftState)
            KeyMap[LastHover].Ctrl.Opt("+Background" . (isShiftBtn ? COLOR_HOVER : COLOR_KEY))
            KeyMap[LastHover].Ctrl.Redraw()
        }
        if (KeyMap.Has(hwnd)) {
            KeyMap[hwnd].Ctrl.Opt("+Background" . COLOR_HOVER)
            KeyMap[hwnd].Ctrl.Redraw()
            LastHover := hwnd
            SetTimer(CleanStuckHover, 50)
        } else {
            LastHover := 0
        }
    }
}

WM_MOUSELEAVE(wParam, lParam, msg, hwnd) {
    global LastHover, BtnClose, ShiftState, KeyMap
    SetTimer(CleanStuckHover, 0)
    if (IsSet(BtnClose) && LastHover == BtnClose.Hwnd) {
        BtnClose.Opt("+Background" . COLOR_KEY), BtnClose.Redraw()
        LastHover := 0
    }
    if (LastHover && KeyMap.Has(LastHover)) {
        isShiftBtn := (KeyMap[LastHover].EN == "Shift" && ShiftState)
        KeyMap[LastHover].Ctrl.Opt("+Background" . (isShiftBtn ? COLOR_HOVER : COLOR_KEY))
        KeyMap[LastHover].Ctrl.Redraw()
        LastHover := 0
    }
}

CleanStuckHover() {
    global LastHover, BtnClose, ShiftState, VKGui, KeyMap
    if (!LastHover) {
        SetTimer(CleanStuckHover, 0)
        return
    }

    CoordMode("Mouse", "Screen")
    MouseGetPos(&mX, &mY)
    try {
        WinGetPos(&wX, &wY, &wW, &wH, VKGui.Hwnd)
        ; Проверяем геометрию окна: курсор физически улетел за границы
        if (mX < wX || mX >= wX + wW || mY < wY || mY >= wY + wH) {
            if (IsSet(BtnClose) && LastHover == BtnClose.Hwnd) {
                BtnClose.Opt("+Background" . COLOR_KEY), BtnClose.Redraw()
            } else if (KeyMap.Has(LastHover)) {
                isShiftBtn := (KeyMap[LastHover].EN == "Shift" && ShiftState)
                KeyMap[LastHover].Ctrl.Opt("+Background" . (isShiftBtn ? COLOR_HOVER : COLOR_KEY))
                KeyMap[LastHover].Ctrl.Redraw()
            }
            LastHover := 0
            SetTimer(CleanStuckHover, 0)
        }
    }
}

WM_LBUTTONDOWN(wParam, lParam, msg, hwnd) {
    global GripCtrl, DragBar, ScaleFactor, KeyboardOpacity, BaseFontSize, ConfigFile, AspectRatio, MinGuiW, KeyList, KeyMap, IsSymLayer, ShiftState, KeyRepeatDelay, KeyRepeatInterval

    ; Автоповтор Backspace и стрелочек при удержании (со скоростью Windows)
    if (KeyMap.Has(hwnd)) {
        info := KeyMap[hwnd]
        targetKey := (IsSymLayer) ? info.SYM : info.EN
        if (targetKey == "{Backspace}" || targetKey == "{Left}" || targetKey == "{Right}" || targetKey == "{Up}" || targetKey == "{Down}") {
            try SoundPlay(A_WinDir . "\Media\Windows Navigation Start.wav")
            SendInput(targetKey)
            info.Ctrl.Opt("+Background" . COLOR_ACTIVE), info.Ctrl.Redraw()
            SetTimer(() => (KeyMap.Has(hwnd) ? (info.Ctrl.Opt("+Background" . COLOR_KEY), info.Ctrl.Redraw()) : 0), -90)

            if (ShiftState == 1 && !IsSymLayer) {
                ShiftState := 0
                UpdateKeyLabels()
            }

            startTime := A_TickCount
            while (GetKeyState("LButton", "P") && (A_TickCount - startTime < KeyRepeatDelay))
                Sleep(15)

            while GetKeyState("LButton", "P") {
                SendInput(targetKey)
                Sleep(KeyRepeatInterval)
            }
            return 0
        }
    }

    ; Перетаскивание СТРОГО за планку DragBar
    if (DragBar && hwnd == DragBar.Hwnd) {
        PostMessage(0xA1, 2, 0, VKGui.Hwnd)
        return
    }

    ; Плавный ресайз за ручку ⤡ с зажатой кнопкой мыши
    if (GripCtrl && hwnd == GripCtrl.Hwnd) {
        CoordMode("Mouse", "Screen")
        MouseGetPos(&sX, &sY)
        WinGetPos(&wx, &wy, &ww, &wh, VKGui.Hwnd)

        while GetKeyState("LButton", "P") {
            MouseGetPos(&cX, &cY)
            newW := Max(MinGuiW, ww + (cX - sX))
            newH := Integer(newW / AspectRatio)
            
            if (newW != ww) {
                VKGui.Move(,, newW, newH)
                OnGuiSize(VKGui, 0, newW, newH)
            }
            Sleep(10)
        }

        newFSize := Integer(BaseFontSize * ScaleFactor)
        VKGui.SetFont("s" . newFSize)
        For c in KeyList
            c.SetFont("s" . newFSize)
        if (IsSet(EnterCtrl) && EnterCtrl)
            EnterCtrl.SetFont("s" . Integer(newFSize * 1.45))
		if (IsSet(SpaceCtrl) && SpaceCtrl)
            SpaceCtrl.SetFont("s" . Integer(newFSize * 1.75))	
        DragBar.SetFont("s" . Integer(newFSize * 0.7))
        BtnClose.SetFont("s" . Integer(newFSize * 0.7))
            
        IniWrite(Round(ScaleFactor, 2), ConfigFile, "KEYBOARD", "ScaleFactor")
        IniWrite(KeyboardOpacity, ConfigFile, "KEYBOARD", "Opacity")
        IniWrite(wx, ConfigFile, "KEYBOARD", "PosX"), IniWrite(wy, ConfigFile, "KEYBOARD", "PosY")
        return
    }
}

OnGuiSize(guiObj, minMax, width, height) {
    if (minMax == -1 || width == 0)
        return
    global ScaleFactor, Scale, BaseW, BaseH, Margin, KeyBaseW
    try {
        mX := Integer(10 * (Scale ? Scale : (ScaleFactor * DpiRatio)))
        mY := Integer(10 * (Scale ? Scale : (ScaleFactor * DpiRatio)))
        Scale := (width - mX * 2) / (12.0 * KeyBaseW + 11 * 6)
        ScaleFactor := Scale / DpiRatio
        BaseW  := Integer(KeyBaseW * Scale), BaseH  := Integer(52 * Scale), Margin := Integer(6 * Scale)
        
        KeysTotalW := Integer(BaseW * 12.0 + Margin * 11)
        DragH  := Integer(20 * Scale)
        CloseW := DragH
        DragW  := KeysTotalW - CloseW - Margin

        DragBar.Move(mX, mY, DragW, DragH)
        BtnClose.Move(mX + DragW + Margin, mY, CloseW, DragH)

        curY := mY + DragH + Margin
        curRow := 1, curX := mX
        For ctrl in KeyList {
            if (ctrl.RowIdx != curRow) {
                curRow := ctrl.RowIdx
                curY += BaseH + Margin
                curX := mX
            }
            kW := Integer(BaseW * ctrl.UWidth + (ctrl.UWidth - 1) * Margin)
            
            ; Жесткая привязка последней клавиши каждого ряда к правому краю
            if (ctrl.KeyIdx == Layout[curRow].Length)
                kW := (mX + KeysTotalW) - curX

            ctrl.Move(curX, curY, kW, BaseH)
            curX += kW + Margin
        }
    }
}

WM_EXITSIZEMOVE(wParam, lParam, msg, hwnd) {
    if (hwnd == VKGui.Hwnd) {
        try {
            WinGetPos(&curX, &curY, , , VKGui.Hwnd)
            IniWrite(curX, ConfigFile, "KEYBOARD", "PosX")
            IniWrite(curY, ConfigFile, "KEYBOARD", "PosY")
        }
    }
}

OnCloseClick(*) {
    try FileDelete(LockFile)
    ExitApp()
}