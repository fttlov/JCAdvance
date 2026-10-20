#Requires AutoHotkey v2.0
Persistent

; ==============================================================================
; НАСТРОЙКА ЧАСТОТЫ ОБНОВЛЕНИЯ (в миллисекундах)
; 16 = ~60 FPS | 33 = ~30 FPS | 10 = 100 FPS
; ==============================================================================
global UpdateInterval := 33.3
; ==============================================================================

DllCall("Winmm\timeBeginPeriod", "UInt", 1)

global hMapFile := DllCall("OpenFileMapping", "UInt", 4, "Int", 0, "Str", "JCAdvanceTelemetry", "Ptr")
global pBuf := 0
if (hMapFile)
    pBuf := DllCall("MapViewOfFile", "Ptr", hMapFile, "UInt", 4, "UInt", 0, "UInt", 0, "UPtr", 256, "Ptr")

; +E0x20 — сквозные клики мыши, +E0x02000000 — защита от мерцания
global Overlay := Gui("+AlwaysOnTop -Caption +ToolWindow +E0x20 +E0x02000000")
Overlay.BackColor := "111111"
Overlay.SetFont("s14 q5", "Consolas")

global TxtTriggers := Overlay.Add("Text", "w370 cFF9F00",     "LT  :      0 | RT   :      0")
;global TxtLS       := Overlay.Add("Text", "w370 cFF9F00 y+5", "LS X:      0 | LS Y :      0")
;global TxtRS       := Overlay.Add("Text", "w370 cFF9F00 y+5", "RS X:      0 | RS Y :      0")
global TxtLS       := Overlay.Add("Text", "w370 cFF9F00 y+5", "LS X:      0 | LS Y :      0")
global TxtRS       := Overlay.Add("Text", "w370 cFF9F00 y+5", "RS X:      0 | RS Y :      0")
;global TxtVirtHz   := Overlay.Add("Text", "w370 cFF00FF y+5", "Virt Xbox:      0 Hz")
global TxtLine1    := Overlay.Add("Text", "w370 c555555 y+5", "------------------------------")
global TxtGyro1    := Overlay.Add("Text", "w370 c00FFFF y+5", "Calib:    0% | Steady: NO ")
global TxtGyro2    := Overlay.Add("Text", "w370 c00FFFF y+5", "BiasX:  0.00 | BiasY:  0.00")
global TxtVel      := Overlay.Add("Text", "w370 c00FFFF y+5", Format("Gyro:  0 {1}/s | Peak:   0 {1}/s", Chr(176)))
global TxtAccel    := Overlay.Add("Text", "w370 c00FFFF y+5", "Shake: 0.000 | MinAc: 0.000")
global TxtLine2    := Overlay.Add("Text", "w370 c555555 y+5", "------------------------------")
global TxtHW1      := Overlay.Add("Text", "w370 c00FF00 y+5", "Waiting for device...         ")
global TxtHW2      := Overlay.Add("Text", "w370 c00FF00 y+2", "                              ")

WinSetTransColor("111111", Overlay)

global CenterX := (A_ScreenWidth - 370) / 2
Overlay.Show("x" CenterX " y20 NoActivate")

SetTimer(WatchXInput, UpdateInterval)

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
    ; объвление всех static-переменных в начале функции
    static NotFoundTime    := 0
    static NoProcessTime   := 0
    static NoDeviceTime    := 0
    
    static lastHzTime      := 0
    static currentHz       := 0
    static lastPacketForHz := 0
    
    static activePad       := 0
    static lastActivePad   := 0
    static padPackets      := [0, 0, 0, 0]
	
	static peakVel         := 0.0	;углы в сек.
    static peakTime        := 0

    ; --- 1. Проверка процесса эмулятора JCadvance.exe ---
    if (!ProcessExist("JCadvance.exe")) {
        if (NoProcessTime == 0)
            NoProcessTime := A_TickCount
        else if (A_TickCount - NoProcessTime > 3000)
            ExitApp()
    } else {
        NoProcessTime := 0
    }

    tempState := Buffer(16, 0)
    Loop 4 {
        if DllCall("XInput1_4\XInputGetState", "UInt", A_Index - 1, "Ptr", tempState) = 0 {
            pNum := NumGet(tempState, 0, "UInt")
            if (pNum != padPackets[A_Index]) {
                activePad := A_Index - 1
                padPackets[A_Index] := pNum
            }
        }
    }

    XINPUT_STATE := Buffer(16, 0)
    if DllCall("XInput1_4\XInputGetState", "UInt", activePad, "Ptr", XINPUT_STATE) = 0
    {
        NotFoundTime := 0

        packetNum := NumGet(XINPUT_STATE, 0, "UInt")
        now := A_TickCount

        if (activePad != lastActivePad) {
            lastHzTime := now
            lastPacketForHz := packetNum
            lastActivePad := activePad
            currentHz := 0
        }

        if (lastHzTime == 0)
        {
            lastHzTime := now
            lastPacketForHz := packetNum
        }

        elapsed := now - lastHzTime
        
        if (elapsed >= 500)
        {
            diff := (packetNum >= lastPacketForHz) ? (packetNum - lastPacketForHz) : (0xFFFFFFFF - lastPacketForHz + packetNum)
            currentHz := Round(diff * 1000 / elapsed)
            lastPacketForHz := packetNum
            lastHzTime := now
        }

        LT := NumGet(XINPUT_STATE, 6, "UChar")
        RT := NumGet(XINPUT_STATE, 7, "UChar")
        LS_X := NumGet(XINPUT_STATE, 8, "Short")
        LS_Y := NumGet(XINPUT_STATE, 10, "Short")
        RS_X := NumGet(XINPUT_STATE, 12, "Short")
        RS_Y := NumGet(XINPUT_STATE, 14, "Short")
        
        StrTriggers := Format("LT  : {:-6} | RT  : {:-6}", LT, RT)
        ;StrLS       := Format("LS X: {:-6} | LS Y : {:-6}", LS_X, LS_Y)
        ;StrRS       := Format("RS X: {:-6} | RS Y : {:-6}", RS_X, RS_Y)
		StrLS       := Format("LS X: {:-6} | RS X: {:-6}", LS_X, RS_X)
        StrRS       := Format("LS Y: {:-6} | RS Y: {:-6}", LS_Y, RS_Y)
        ;StrVirtHz   := Format("Virt Xbox: {:-4} Hz [Slot {}]", currentHz, activePad)
        
        if (TxtTriggers.Value !== StrTriggers)
            TxtTriggers.Value := StrTriggers
        if (TxtLS.Value !== StrLS)
            TxtLS.Value := StrLS
        if (TxtRS.Value !== StrRS)
            TxtRS.Value := StrRS
        ;if (TxtVirtHz.Value !== StrVirtHz)
            ;TxtVirtHz.Value := StrVirtHz
    }
    else
    {
        lastHzTime := 0
        currentHz  := 0

        TxtTriggers.Value := "Gamepad not found"
        TxtLS.Value := ""
        TxtRS.Value := ""
        ;TxtVirtHz.Value := "Virt Xbox:      0 Hz"
        
        ; Если виртуальный геймпад не найден (3 сек)
        if (NotFoundTime == 0)
            NotFoundTime := A_TickCount
        else if (A_TickCount - NotFoundTime > 3000)
            ExitApp()
    }

    if (pBuf)
    {
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
        
        shake  := NumGet(pBuf, 40, "Float")
        minAc  := NumGet(pBuf, 44, "Float")
		
		angVel := Abs(NumGet(pBuf, 48, "Float"))
        if (angVel >= peakVel) {
            peakVel := angVel
            peakTime := A_TickCount
        } else if (A_TickCount - peakTime > 500) {
            peakVel := angVel
        }

        ; --- 2. Проверка физического отключения контроллера (3 сек) ---
        if (type1 <= 0 || hz1 <= 0) {
            if (NoDeviceTime == 0)
                NoDeviceTime := A_TickCount
            else if (A_TickCount - NoDeviceTime > 3000)
                ExitApp()
        } else {
            NoDeviceTime := 0
        }
        
        StrGyro1 := Format("Calib: {:4.0f}% | Steady: {}", conf, steady)
        StrGyro2 := Format("BiasX: {:5.2f} | BiasY: {:5.2f}", biasX, biasY)
		
		StrVel := Format("Gyro:{2:4.0f}{1}/s | Peak:{3:4.0f} {1}/s", Chr(176), angVel, peakVel)
        
        StrAccel := Format("Shake: {:5.3f} | MinAc: {:5.3f}", shake, minAc)
        
        StrHW1 := Format("{}: {:3.0f}Hz | {:3.0f}%", GetDevName(type1), hz1, bat1)
        if (hz2 > 0)
            StrHW2 := Format("{}: {:3.0f}Hz | {:3.0f}%", GetDevName(type2), hz2, bat2)
        else
            StrHW2 := ""
        
        if (TxtGyro1.Value !== StrGyro1)
            TxtGyro1.Value := StrGyro1
        if (TxtGyro2.Value !== StrGyro2)
            TxtGyro2.Value := StrGyro2
		if (TxtVel.Value !== StrVel)
            TxtVel.Value := StrVel	
        if (TxtAccel.Value !== StrAccel)
            TxtAccel.Value := StrAccel	
        if (TxtHW1.Value !== StrHW1)
            TxtHW1.Value := StrHW1
        if (TxtHW2.Value !== StrHW2)
            TxtHW2.Value := StrHW2
			

    }
}