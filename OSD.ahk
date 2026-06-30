Persistent

global hMapFile := DllCall("OpenFileMapping", "UInt", 4, "Int", 0, "Str", "JCAdvanceTelemetry", "Ptr")
global pBuf := 0
if (hMapFile)
    pBuf := DllCall("MapViewOfFile", "Ptr", hMapFile, "UInt", 4, "UInt", 0, "UInt", 0, "UPtr", 256, "Ptr")

Overlay := Gui("+LastFound +AlwaysOnTop -Caption +ToolWindow +E0x02000000")
Overlay.BackColor := 0x111111
Overlay.SetFont("s14 q5", "Consolas")

TxtTriggers := Overlay.Add("Text", "w370 cFF9F00",     "LT  :      0 | RT   :      0")
TxtLS       := Overlay.Add("Text", "w370 cFF9F00 y+5", "LS X:      0 | Y    :      0")
TxtRS       := Overlay.Add("Text", "w370 cFF9F00 y+5", "RS X:      0 | Y    :      0")
TxtLine1    := Overlay.Add("Text", "w370 c555555 y+5", "------------------------------")
TxtGyro1    := Overlay.Add("Text", "w370 c00FFFF y+5", "Calib:    0% | Steady: NO ")
TxtGyro2    := Overlay.Add("Text", "w370 c00FFFF y+5", "BiasX:  0.00 | BiasY:  0.00")
TxtLine2    := Overlay.Add("Text", "w370 c555555 y+5", "------------------------------")
TxtHW1      := Overlay.Add("Text", "w370 c00FF00 y+5", "Waiting for device...         ")
TxtHW2      := Overlay.Add("Text", "w370 c00FF00 y+2", "                              ")

WinSetTransColor(0x111111, Overlay)

CenterX := (A_ScreenWidth - 870) / 2
Overlay.Show("x" CenterX " y20 NoActivate")

SetTimer(WatchXInput, 33.3)

; Функция перевода ID в строку (используем 15 символов с пробелами для ровной таблицы)
GetDevName(devID) {
    if (devID == 1)
        return "Joy-Con (L)    "
    if (devID == 2)
        return "Joy-Con (R)    "
    if (devID == 3)
        return "Pro Controller "
    if (devID == 4)
        return "DualShock 4    "
    if (devID == 5)
        return "DualSense      "
    return "Unknown Device "
}

WatchXInput() {
    XINPUT_STATE := Buffer(16, 0)
    if DllCall("XInput1_4\XInputGetState", "UInt", 0, "Ptr", XINPUT_STATE) = 0 {
        LT := NumGet(XINPUT_STATE, 6, "UChar")
        RT := NumGet(XINPUT_STATE, 7, "UChar")
        LS_X := NumGet(XINPUT_STATE, 8, "Short")
        LS_Y := NumGet(XINPUT_STATE, 10, "Short")
        RS_X := NumGet(XINPUT_STATE, 12, "Short")
        RS_Y := NumGet(XINPUT_STATE, 14, "Short")
        
        StrTriggers := Format("LT  : {:-6} | RT   : {:-6}", LT, RT)
        StrLS       := Format("LS X: {:-6} | Y    : {:-6}", LS_X, LS_Y)
        StrRS       := Format("RS X: {:-6} | Y    : {:-6}", RS_X, RS_Y)
        
        if (TxtTriggers.Value !== StrTriggers)
            TxtTriggers.Value := StrTriggers
        if (TxtLS.Value !== StrLS)
            TxtLS.Value := StrLS
        if (TxtRS.Value !== StrRS)
            TxtRS.Value := StrRS
    } else {
        TxtTriggers.Value := "Gamepad not found"
        TxtLS.Value := ""
        TxtRS.Value := ""
    }

    if (pBuf) {
        conf   := NumGet(pBuf, 0, "Float") * 100
        steady := NumGet(pBuf, 4, "Float") ? "YES" : "NO "
        biasX  := NumGet(pBuf, 8, "Float")
        biasY  := NumGet(pBuf, 12, "Float")
        
        hz1    := NumGet(pBuf, 16, "Float")
        hz2    := NumGet(pBuf, 20, "Float")
        bat1   := NumGet(pBuf, 24, "Float")
        bat2   := NumGet(pBuf, 28, "Float")
        type1  := NumGet(pBuf, 32, "Float")
        type2  := NumGet(pBuf, 36, "Float")
        
        StrGyro1 := Format("Calib: {:4.0f}% | Steady: {}", conf, steady)
        StrGyro2 := Format("BiasX: {:5.2f} | BiasY: {:5.2f}", biasX, biasY)
        
        StrHW1 := Format("{}: {:3.0f}Hz | {:3.0f}%", GetDevName(type1), hz1, bat1)
        if (hz2 > 0)
            StrHW2 := Format("{}: {:3.0f}Hz | {:3.0f}%", GetDevName(type2), hz2, bat2)
        else
            StrHW2 := ""
        
        if (TxtGyro1.Value !== StrGyro1)
            TxtGyro1.Value := StrGyro1
        if (TxtGyro2.Value !== StrGyro2)
            TxtGyro2.Value := StrGyro2
        if (TxtHW1.Value !== StrHW1)
            TxtHW1.Value := StrHW1
        if (TxtHW2.Value !== StrHW2)
            TxtHW2.Value := StrHW2
    }
}