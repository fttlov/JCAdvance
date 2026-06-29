#Requires AutoHotkey v2.0
#SingleInstance Force
Persistent

; œÓ‰ÍÎ˛˜‡ÂÏÒˇ Í Shared Memory, ÍÓÚÓÛ˛ ÒÓÁ‰‡Î JCAdvance
global hMapFile := DllCall("OpenFileMapping", "UInt", 4, "Int", 0, "Str", "JCAdvanceTelemetry", "Ptr")
global pBuf := 0
if (hMapFile)
    pBuf := DllCall("MapViewOfFile", "Ptr", hMapFile, "UInt", 4, "UInt", 0, "UInt", 0, "UPtr", 256, "Ptr")

Overlay := Gui("+LastFound +AlwaysOnTop -Caption +ToolWindow +E0x02000000")
Overlay.BackColor := 0x111111
Overlay.SetFont("s14 q5", "Consolas")

TxtTriggers := Overlay.Add("Text", "w350 cFF9F00",     "LT  :      0 | RT   :      0")
TxtLS       := Overlay.Add("Text", "w350 cFF9F00 y+5", "LS X:      0 | Y    :      0")
TxtRS       := Overlay.Add("Text", "w350 cFF9F00 y+5", "RS X:      0 | Y    :      0")
TxtLine     := Overlay.Add("Text", "w350 c555555 y+5", "----------------------------")
TxtGyro1    := Overlay.Add("Text", "w350 c00FFFF y+5", "Calib:    0% | Steady: NO ")
TxtGyro2    := Overlay.Add("Text", "w350 c00FFFF y+5", "BiasX:  0.00 | BiasY:  0.00")

WinSetTransColor(0x111111, Overlay)

CenterX := (A_ScreenWidth - 350) / 2
Overlay.Show("x" CenterX " y20 NoActivate")

SetTimer(WatchXInput, 16.6)

WatchXInput() {
    ; --- ◊“≈Õ»≈ √≈…Ãœ¿ƒ¿ ---
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

    ; --- ◊“≈Õ»≈ √»–Œ— Œœ¿ (Shared Memory) ---
    if (pBuf) {
        conf   := NumGet(pBuf, 0, "Float") * 100
        steady := NumGet(pBuf, 4, "Float") ? "YES" : "NO "
        biasX  := NumGet(pBuf, 8, "Float")
        biasY  := NumGet(pBuf, 12, "Float")
        
        StrGyro1 := Format("Calib:  {:3.0f}% | Steady: {}", conf, steady)
        StrGyro2 := Format("BiasX: {:5.2f} | BiasY: {:5.2f}", biasX, biasY)
        
        if (TxtGyro1.Value !== StrGyro1)
            TxtGyro1.Value := StrGyro1
        if (TxtGyro2.Value !== StrGyro2)
            TxtGyro2.Value := StrGyro2
    }
}