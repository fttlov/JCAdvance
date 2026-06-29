#Requires AutoHotkey v2.0
#SingleInstance Force
Persistent

; Строка 5: Создание объекта Gui (обязательна, иначе будет ошибка)
Overlay := Gui("+LastFound +AlwaysOnTop -Caption +ToolWindow +E0x02000000")
Overlay.BackColor := 0x111111
Overlay.SetFont("s14 q5", "Consolas")

; Создание текстовых полей внутри Overlay
TxtTriggers := Overlay.Add("Text", "w350 cFF9F00", "LT  :   0      | RT  :   0     ")
TxtLS       := Overlay.Add("Text", "w350 cFF9F00 y+5", "LS X:   0      | Y   :   0     ")
TxtRS       := Overlay.Add("Text", "w350 cFF9F00 y+5", "RS X:   0      | Y   :   0     ")

WinSetTransColor(0x111111, Overlay)

CenterX := (A_ScreenWidth - 350) / 2
Overlay.Show("x" CenterX " y20 NoActivate")

SetTimer(WatchXInput, 16.6)

WatchXInput() {
    XINPUT_STATE := Buffer(16, 0)
    
    if DllCall("XInput1_4\XInputGetState", "UInt", 0, "Ptr", XINPUT_STATE) = 0 {
        LT := NumGet(XINPUT_STATE, 6, "UChar")
        RT := NumGet(XINPUT_STATE, 7, "UChar")
        LS_X := NumGet(XINPUT_STATE, 8, "Short")
        LS_Y := NumGet(XINPUT_STATE, 10, "Short")
        RS_X := NumGet(XINPUT_STATE, 12, "Short")
        RS_Y := NumGet(XINPUT_STATE, 14, "Short")
        
        StrTriggers := Format("LT  :  {:-6}   RT  :  {:-6}", LT, RT)
        StrLS       := Format("LS X:  {:-6}   LS Y:  {:-6}", LS_X, LS_Y)
        StrRS       := Format("RS X:  {:-6}   RS Y:  {:-6}", RS_X, RS_Y)
        
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
}