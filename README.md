<p align="center">
  <img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/JCAdvance_logo6.png" alt="Logo" width="300"/>
</p>
<h1 align="center">JCAdvance</h1>

[![EN](https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/en.png)](README.md)
[![RU](Icon/ru.png)](README_RU.md)
← Choose your language

## What is it
**Joy-Con Advance** is an XBOX/DS4 gamepad emulator featuring advanced options for Joy-Cons, Pro Controllers, DualShock 4, and DualSense Edge. It's based on the DSAdvance project by r57zone.

## Basic features of the original:
- Assign gamepad buttons to emulate XBOX buttons or keyboard/mouse keys (a separate profile for each) <br>
- Gyro-based modes: mouse/right stick emulation for looking/aiming; steering wheel emulation; Aircraft <br>
- HardCoded hotkeys for switching modes and profiles in real time <br>
- Customizable sensitivity, deadzones, inverting and left/right stick/trigger switching <br>
- Rumble support for Sony/Nintendo gamepads <br>
- Joy-Cons combined into a single virtual XBOX controller<br>
- Support for Sony DualSense adaptive triggers (pistol, rifle, sniper rifle, bow, car pedal) <br>
- Support for two gamepads (second gamepad is limited to basic functions)<br>
- Magic Wheel feature: use the gyro movements in different directions as additional buttons/actions<br>
- External pedal: connect pedals to emulate triggers or sticks (limited device support) <br>
- Minimal memory and CPU usage (0.20% - 0.50%)<br>
 
## Key Differences in JCAdvance:
- **Gyro Concept:** Separate gyro motion approaches and fine-tuning for Joy-Cons and two-handed gamepads
- **Gyro Calibration:** Advanced calibration for Joy-con to compensate for thermal IMU drift (see [Technical Details](#tech-info))
- **Digital Trigger Bypass for Joy-Cons:** utilize all 6 virtual Xbox analog axes (4 mapped to sticks + 2 to gyro)
- **GUI:** New config tool and redesigned user-friendly main interface
- **Bug Fixes**, **improvements** and some **new features**

<details>
  <summary><h4>Learn more about Gyro Сoncept and Digital Trigger Bypass</h4></summary>

While *DSAdvance* was primarily designed for two-handed Sony controllers with Joy-Con support, **JCAdvance** focuses on making Joy-Cons easy and intuitive to use. It introduces flexible gyroscope adjustments via the **Gyro Space**, **Tightening** setting (by JibbSmart) for different controllers and ability to use all analog axes of the XBOX virtual controller for Joy-Cons.

The core philosophy of Gyro Motion differs between controller types:
* **Two-handed controllers:** Gyro is best used for fine-tuning and adjusting classic stick aiming.
* **Joy-Cons:** True, free-hand full gyro motion control. Using the right stick for aiming in FPS/TPS becomes obsolete.<br>
*Since the physical right analog stick is now completely free on Joy-cons*: <br>
* **JCAdvance allows you to use the right stick's Y-axis as virtual analog triggers**. These will work alongside the digital buttons you have already assigned to the triggers (like ZL ZR), and can be fully controlled (on/off) in real time using a customisable hotkey. <br>

Examples of use stick as triggers:<br>

**RDR 2:** Slowly pull the right stick UP to fill the draw meter in duels (bypassing the digital trigger issue); smoothly cock your revolver's hammer and fire or just rapid fire by digital trigger <br>
**GTA V:** Using the right stick (up/down) for analog gas/brakes in vehicles; progressive trigger actions on foot.

* **Stick as buttons** - Don't use the right stick as triggers? Use them as buttons! Assign any virtual Xbox, keyboard or mouse buttons to one of the stick's four directions. <br>

Note: <br>
In "Stick as trigger" mode, you can only assign two buttons to the free X-axis (stick left-right directions).<br>
"Stick as trigger"  mode takes priority when activated via a hotkey.

</details>
 
## All new features:
- **Config tool:** all primary settings, Gyro options, button mapping, and hotkeys now can be configured via a GUI

<table>
  <tr>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config1_en.png" width="150" alt="Config Tab 1"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config2_en.png" width="150" alt="Config Tab 2"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config3_en.png" width="150" alt="Config Tab 3"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config4_en.png" width="150" alt="Config Tab 4"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config5_en.png" width="150" alt="Config Tab 5"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config6_en.png" width="150" alt="Config Tab 6"></td>
  </tr>
</table>

- **Universal Mapping:** Map any digital Nintendo/Sony gamepad button to emulate any XBOX button, keyboard key, or mouse action within a single profile
- **Auto-Bind:** Quickly assign buttons using the "Bind" or select them manually from a drop-down list
- **Profile Manager:** Create and manage profiles within a dedicated tab in the configurator
- **Custom Hotkeys:** Activate modes with customizable key combinations (e.g., `R + HOME`)
- **Gyro Ratchet button:** hold down to pause (classic mode + delay option) or hold down to enable gyro movement
- **Gyro Melee Gesture:** Perform physical punching, hooking, or hammering gestures to trigger virtual buttons
- **Gyro Space Option:** A crucial setting for Gyro Mouse/Stick modes (see [Technical Details](#tech-info) for more information)
- **Gyro Tightening Adjustment:** filter to eliminate hand tremors and hardware sensor noise by JibbSmart
- **Smart Sensitivity Adjustment:** Change the sensitivity by hotkeys in game and view the latest values in the console
- **Сalibration indicator:** successful first auto-calibration by the beep
- **Manual recalibrating:** Place device on a flat surface, press customizable hotkey and wait for the beep
- **Left handed mode:** Option to read Gyro data from the left Joy-Con in combined mode
- **Polling Rate Option:** Increase the polling rate for smoother motion response
- **Right Stick as triggers mode:** utilize all 6 virtual controller analog axes (mainly for Joy-Con) 
- **Right Stick as buttons mode:** using the stick directions as virtual buttons
- **Non-Linear Response:** Non-linear stick and steering wheel sensitivity options
- **EMA Smoothing Filter:** Exponential Moving Average (see [Technical Details](#tech-info) for more information)
- **OSD info**: current values from sticks, triggers and gyroscope; current battery status and device polling rate
- **DirectInput Emulation:** Option to emulate a DirectInput controller instead of a virtual XBOX 360 controller
- **Improved Driving Mode:** Eliminated sudden steering wheel jerks to the opposite side at maximum angles
- Added a hotkey for manual steering wheel recalibration/centering
- **Pedal Compatibility:** External pedal feature now works with almost all standard DirectInput wheels/pedals
- **Clean Main Menu:** Displays active settings and hotkeys at a glance

<table align="center">
  <tr>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Main1_en.png" width="150" alt="Main Interface"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Main2_en.png" width="150" alt="Main Interface"></td>
  </tr>
</table>


## Requirements
- [ViGEm Bus Driver](https://github.com/nefarius/ViGEmBus) — Virtual Gamepad Emulation Framework by nefarius
- [Microsoft Visual C++ Redistributable 2017 (x86/x64)](https://learn.microsoft.com/en-us/answers/questions/4137965/download-link-for-microsoft-visual-c-2017-redistri) or newer

## How to Use
1. Download the latest [Releases](https://github.com/fttlov/JCAdvance/releases)
2. Unzip the archive to any folder
3. Open `Config.exe`, choose gamepad layout in `Settings tab` and setup other options
4. Run `JCAdvance.exe`, connect your gamepad, and enjoy!

## Important Note
To prevent double-input issues in games (where a game detects both your physical controller and the virtual XBOX/DS4 controller simultaneously), you should hide your physical gamepad.
Best way - using the [HidHide](https://github.com/nefarius/HidHide) utility

<details>
  <summary><b>Quick Setup Guide</b></summary>
  <br>
  Download and install HidHide. Open the HidHide Configuration Client and:
  
  1. Add `JCAdvance.exe` and `Config.exe` to the Applications list
  2. Select your physical gamepad in the Devices tab
  3. Select the **"Enable device hiding"** option

  <table align="center">
    <tr>
      <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/HidHide1.png" width="150" alt="HidHide Setup 1"></td>
      <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/HidHide2.png" width="150" alt="HidHide Setup 2"></td>
    </tr>
  </table>

  For a complete guide, visit the [official HidHide Setup Guide](https://docs.nefarius.at/projects/HidHide/Simple-Setup-Guide/).
</details>  

__________

<details>
 
<summary><h2 id="tech-info">Technical details, instructions and bug fixes</h2> (Click to open)</summary>
  
  ### Interface, Settings and Profiles

  
**Interface**: A new 3-layer menu:
- **Layer 0:** Shown before connecting devices
- **Layer 1:** Active after controllers are connected
- **Layer 2:** Hotkey menu

  <details>
  <summary><b>🎮 All Controller & Keyboard Hotkeys Reference (Click to expand)</b></summary>
    
    
    ### 💻 Keyboard Hotkeys
    *   `ALT + Esc` — Exit application.
    *   `ALT + V` — Swap Primary and Secondary gamepad slots.
    *   `ALT + I` — Display controller battery status on the screen.
    *   `ALT + F9` — Toggle dead zones diagnostics overlay.
    *   `ALT + 1` *(or Gamepad Hotkey)* — Toggle Driving Mode On / Off **`[Configurable]`**.
    *   `ALT + 2` *(or Gamepad Hotkey)* — Unlock / Lock Gyro Motion On / Off **`[Configurable]`**.
    *   `ALT + A` *(or Gamepad Hotkey)* — Switch Gyro Aiming Mode (Mouse vs Stick) **`[Configurable]`**.
    *   `ALT + D` *(or Gamepad Hotkey)* — Toggle Right Stick as Analog Triggers mode **`[Configurable]`**.
    *   `ALT + S` — Toggle Left Stick Auto-Press Emulation mode.
    *   `ALT + F` — Toggle Control button behavior (start/stop gyro motion)
    *   `ALT + B` — Toggle controller backlight (Sony only).
    *   `ALT + W` — Toggle Sony Touchpad click mode-switching behavior.
    *   `ALT + Up / Down` — Switch active profile.
    *   `ALT + < / >` — Adjust rumble strength.
    *   `ALT + C` *(or keyboard Hotkey)* — Calibrate gyro manually (keep controller flat on a table) **`[Configurable]`**.
    *   `ALT + G` *(or keyboard Hotkey)* — Calibrate accel manually (keep controller flat on a table) **`[Configurable]`**.
    *   `CTRL + R` *(or keyboard Hotkey)* — Reset and re-search connected controllers **`[Configurable]`**.
    
    ---
    
    ### 🟦 Sony Controller Hotkeys (Modifier: `PS` Button)
    *   `PS` *(single tap)* — Open Windows Xbox Game Bar.
    *   `PS + Triangle` — Increase Gyro Sensitivity by 10 units (Sens +) **`[Configurable]`**.
    *   `PS + Cross` — Decrease Gyro Sensitivity by 10 units (Sens -) **`[Configurable]`**.
    *   `PS + R3 (Right Stick Click)` — Reset Gyro Sensitivity to default.
    *   `PS + Square` — Windows Volume Down.
    *   `PS + Circle` — Windows Volume Up.
    *   `PS + R1` — Take Screenshot (single tap) / Record Video (hold) **`[Configurable]`**.
    *   `PS + L1` — Toggle Lightbar backlight On / Off.
    *   `PS + L3 (Left Stick Click)` — Switch Left Stick "Auto-Sprint" Mode.
    *   `PS + Share` — Toggle Touchpad click mode-switching behavior.
    *   `PS + Options` — Adjust rumble strength (0% to 100% in 10% steps).
    *   `PS + DPAD Up / Down` — Switch active profile.
    
    ---
    
    ### 🟥 Nintendo Controller Hotkeys (Modifier: `Capture` Button)
    *   `Capture + Home` — Open Windows Xbox Game Bar.
    *   `Capture + X` — Increase Gyro Sensitivity by 10 units (Sens +) **`[Configurable]`**.
    *   `Capture + B` — Decrease Gyro Sensitivity by 10 units (Sens -) **`[Configurable]`**.
    *   `Capture + R3 (Right Stick Click)` — Reset Gyro Sensitivity to default.
    *   `Capture + Y` — Windows Volume Down.
    *   `Capture + A` — Windows Volume Up.
    *   `Capture + R` — Take Screenshot (single tap) / Record Video (hold) **`[Configurable]`**.
    *   `Capture + Plus` — Adjust rumble strength (0% to 100% in 10% steps).
    *   `HOME + L3 (Left Stick Click)` — Switch Left Stick "Auto-Sprint" Mode.
    *   `HOME + DPAD Up / Down` — Switch active profile.
    
    ---
    
    ### 🟪 Sony Touchpad Actions (DualSense / DualShock 4)
    *   **Left Area Touch/Click** — Activate **Driving Mode** (gyro steering wheel).
    *   **Right Area Touch/Click** — Activate **Aiming Mode** (gyro mouse/stick).
    *   **Center Area Touch/Click** — Reset to **Default Mode** (and briefly displays battery levels on Lightbar).
    *   **Center-Top Edge Slide** — Adjust controller LED backlight brightness dynamically.
    *   **Center-Bottom Touch/Click** — Switch to **Desktop Mode** (loads mouse and keyboard navigation profile).
    
    <table align="center">
      <tr>
        <td><img src="https://github.com/fttlov/JCAdvance_test/blob/main/Icon/DSAdvance_Touchpad.png" width="333" alt="Main Interface"></td>
      </tr>
    </table>
    
    <sub> Picture from **[DSADvance official github](https://github.com/r57zone/DSAdvance)**</sub>

  </details>

**Settings**: All settings relating to dead zones, inversion, default gyroscope mode, steering and other profile-specific options have now been moved to the \XboxProfile\\*.ini file, within the [Settings] section, instead of config.ini and applied without restarting the emulator.

**Profiles:** The original code strictly separated Xbox profiles (`.ini` files in the `XboxProfile` folder) and Keyboard/Mouse profiles (`KMProfile`). This prevented users from emulating both Xbox and keyboard actions in one profile. JCAdvance resolves this: the main `XboxProfile` folder now supports mixed emulation, and profiles are easily managed via `Config.exe`. However, you can still switch between profiles using hotkeys within the XboxProfile folder. Note that the profile change only applies to the current session

  ### OSD

The OSD now displays real-time sticks, triggers and gyroscope telemetry.<br>
**For Triggers**: raw data from 0 to 255. <br>
**For Sticks**: raw data fromm -32768 to 32768 <br>
Gyro telemetry: This helps you understand how the JoyShockLibrary auto-calibration handles thermal gyro drift (see more in "Calibration") <br>
**Calib (0-100%)**: Algorithm confidence. Usually stays at 100%, meaning the baseline noise is known. It drops to 0% only during a manual reset (ALT+C)<br>
**Steady (YES / NO)**: Physical stillness detector. When it says YES, the controller is perfectly still, and the background auto-calibration is actively collecting data.<br>
**BiasX / BiasY**: The actual hardware drift offsets (in degrees/second). Sensors naturally drift. These numbers show the raw error the emulator is suppressing during calibration to keep crosshair perfectly still.<br>

Pro Tip: If you hold the gamepad for a few minutes and then put it on a table, you might see the Bias values jump or slightly fluctuate (e.g., from 1.09 to 1.02). This is normal! It proves the software calibration is actively recalculating the thermal drift.

Also added current battery status and device poling rate info

  ### New Smart Gyro Sensitivity Adjustment

How it works: Launch the game, use the in-game settings to configure the controls, then, if necessary, use hotkeys to fine-tune the gyro (aiming) sensitivity (+- 5 units). After exiting the game, you’ll see a full log of the sensitivity changes in the console window; take the latest value and save it to Config.exe 

  ## Calibration

⚠️ Hardware calibration is described to help you understand how the sensors work and is only necessary if the factory calibration has been compromised (e.g., the device was dropped, a failed firmware update, etc.). If there are no specific issues, you don’t need to do anything. "If it works, don’t touch it."  <br>
You'll most likely have to deal with software calibration

  ### Hardware Calibration
For motion controls to work flawlessly, your controller must be properly calibrated at the hardware level. <br>
The Gyroscope measures rotation. To calibrate it, the controller only needs absolute stillness. <br>
The Accelerometer measures Earth's gravity. To calibrate it, the controller must be placed on a perfectly flat and level surface. <br>

Hardware Calibration Methods: <br>
- Sony (DualShock 4 / DualSense): Calibrated automatically at the factory. They usually do not require manual calibration <br>
- Original Nintendo (Joy-Con / Pro Controller): Calibrate them by connecting to a Nintendo Switch console (System Settings -> Controllers and Sensors -> Calibrate Motion Controls). If you don't own a Switch, use the free PC tool Joy-Con Toolkit. JCAdvance automatically reads these precise offsets from the controller's internal memory <br>
- Third-Party Clones (Mobapad, IINE, NYXI, etc.): These controllers usually have a built-in hardware shortcut to recalibrate sensors (e.g., holding R + X + HOME for right [Mobapad M6](https://www.youtube.com/watch?v=DRcDEmGya6M&t=2s)). Check your controller's manual

⚠️ For succsesfull calibrating **Accelerometer sensor** on separated Joy-Cons, attach them to the Switch console or use the charging Grip. If you lay a bare Joy-Con on a table, it will tilt due to the protruding SL/SR buttons, resulting in a crooked calibration. <br>
You can use a smartphone simple bubble-level app (or pro free app like Phyphox) to verify your desk or floor is actually flat <br>

Practical tip for Mobapad M6: Launch Phyphox, select Acceleration with g > Simple > place your smartphone on top, and put something under the gamepad to achieve values close to "0" for the Accelerometer X and Y parameters, then complete the calibration. For perfectionists only :)

  ### Software Calibration (correction) & Drift Prevention
  Due to imperfections in MEMS sensors (such as temperature drift—the sensor heating up), particularly in the Joy-Con controllers, a cumulative **gyroscope** drift effect may occur over time — a slight deviation from zero that manifests as random movement of the in-game camera. After the temperature rises during the first few minutes of a gaming session (due to the battery, the palm of the hand, or the crystal’s own heat generation) and then stabilizes, the drift generally stops increasing, but it needs to be compensated for. To do this JCAdvacne features a smart calibration system (by JibbSmart) to keep your gyro aiming perfectly accurate and eliminate "cursor/stick drift"<br> 

⚠️ Software corrections affects only the gyroscope sensor and works when the emulator is running

#### Automatic Gyro Calibration
By default, the emulator recalibrates your gyro sensor automaticaly when you place the controller on a flat surface for about 3-5 second. The algorithm detects the silence and instantly recalculates the absolute zero point. <br> 
The first calibration have the beep indication. Subsequent calibrations can be monitored using the "Steady" setting in the OSD or the BackgroundCalibSound (=1) setting in config.ini

#### Manual Gyro Calibration (by hotkey)
If you prefer to control when calibration happens, you can disable autocalibration in config.ini and force it manually at any time:
Press Alt + C (or your mapped Key). You will hear a low beep. Place the controller on a flat surface immediately. Your aiming axes will be temporarily muted. Wait for the success signal - double beep.<br>
Note: If you move the controller too much during this process, you will hear a low error beep after 5 seconds, meaning calibration failed

#### Manual Accelerometer Calibration
This is only necessary in rare cases when you want to use World Gyro Space mode (more on this below) but are unable to perform a hardware calibration of the accelerometer

#### Config.ini Settings
AutoCalibrationEnabled=1 — (Default) Continuous background calibration is ON <br>
AutoCalibrationEnabled=0 — calibration is possible only in manual mode by hotkey <br>
BackgroundCalibSound=1 — debug beep for each successful auto-calibration
  
  <details>
  <summary>Under the hood</summary>

IMU heating Issues: <br>
The gyroscope measures velocity (degrees per second). To determine the camera’s rotation angle in the game, the software continuously adds this velocity (integrates it). If the gyroscope starts to give off inaccurate readings by just 0.1 degrees/sec due to heating, then after 10 seconds the crosshairs will drift 1 degree off course, and after a minute—6 degrees. The error snowballs!

  By default, Joyshocklibrary uses "universal" software calibration settings for Sony and Nintendo controllers.
The code is written in such a way that Sony controllers can be automatically calibrated, as it were, in the background. It’s a heavy, 250-gram two-handed controller with high-quality gyroscope and accelerometer sensors, which is held with both hands (resulting in less shaking) and, for example, during cutscenes, is placed on the player’s lap—at which point automatic calibration occurs seamlessly and is applied smoothly. <br>
The Joy-Con is a lightweight controller for one-handed use, with sensors of significantly lower quality. It is impossible to calibrate it properly whilst holding it in your hand. Therefore, the only option is to calibrate it “on a table.” <br>
In JCadvance the settings for Joy-Con calibration have been adjusted to allow for faster and more reliable calibration in 2–3 seconds.

Example of use in real-world conditions (MobaPad M6S): <br>
Launch the emulator, connect the device, place it on a surface, wait for the first successful calibration (beep), and start playing. The temperature gradually rises, and drift increases. After a few minutes, place the device on the surface for a 2-3 sec. and continue playing. As a rule, a couple of recalibrations are enough to then play for an hour or more without ever letting go of the controller and without any drift (temperature has stabilized)

Added auto-calibration settings from Joyshocklibrary to config.ini for fine-tuning (Nintendo only) :<br>

1. MaxStillnessError (Default: 2.0) — The absolute maximum noise/error limit the algorithm will tolerate. If the noise exceeds this value, calibration is immediately aborted. Values greater than 4 will allow you to calibrate the Joy-Con while holding it in your hand, but this may cause drift right after calibration <br>
2. MinStillnessCollectionTime (Default: 0.5) — The initial time the controller must remain still to collect baseline noise data. Once reached, the algorithm evaluates this data to confirm if the controller is truly at rest. Recomended default value <br>
3. MinStillnessCorrectionTime (Default: 2.0) — The total continuous stillness time required to actually apply the new calibration to the gyroscope. The timer doesn't reset; it seamlessly continues from the collection phase.<br>
4. StillnessCalibrationEaseInTime (Default: 3.0) — The duration (in seconds) over which the newly calculated gyro bias is blended in. Lowering this (e.g., to 0.1 - 1.0) makes the drift stop abruptly and noticeably, while higher values smooth the transition to prevent sudden camera jerks if you are holding the controller.
Joy-Cons calibrate well only on flat surfaces, so feel free to use low values <br>
    
Since the JoyshockLibrary code is quite complex, it is not yet possible to fully understand the calibration logic. Among the unclear points:

- There is clearly a calibration process using the accelerometer, but it is not yet clear exactly how it works. Sometimes the values reset (drift decreases) during complex, smooth movements at a constant speed (for example, when drawing an infinity symbol with a wrist rotation).
  
- ~~In rare cases, auto-calibration fails and stops working even when the gamepad is completely stationary (Steady is always set to “No” in the OSD). The cause of this issue is not yet clear: it could be either a software bug in the library or a hardware issue with Bluetooth. If the drift increases and does not reset, first try manual calibration by hotkey; if that doesn’t help, press Ctrl + R; if that doesn’t help again, restart the emulator~~ <br>
The issue has been resolved by adding "Adaptive Noise Threshold" in the GamepadMotion.hpp. Tested on a Joy-Con (Mobapad) during an extended gaming session. Read more here [8. Auto-calibration fix ](https://github.com/fttlov/JoyShockLibrary/blob/main/README.md)
  
  </details>

  ### Gyro Motion Space

This option controls how the gyroscope interprets hand movements into mouse/stick movements depending on the tilt of your wrist (clockwise or counter-clockwise) and how you hold the gamepad (face buttons pointing toward you or horizontally). In DSAdvance, "0" is a hard-coded value. Now we have all 3 modes from the JoyShockLibrary creator: <br>

* **0 (Local Space):** Relies entirely on the gyroscope. Movement is calculated relative to the controller's plastic body, ignoring gravity <br>
* **1 (World Space):** Relies heavily on a perfectly calibrated accelerometer. It uses real-world gravity to separate horizontal and vertical aiming <br>
* **2 (Player Space):** uses accelerometer too

⚠️ For modes 1 or 2: If your in-game crosshair moves diagonally when you swipe your hands horizontally (cross-talk), your accelerometer is miscalibrated. Calibrate the accelerometer correctly (see harware calibration). If you cannot perform a hardware calibration of the accelerometer, use software calibration (hotkey) or Local Space mode only. <br>

In short: for two-handed gamepads, the recommended values are 0 or 2. For Joy-Con: 1 or 0.

**For two-handed gamepads:** let’s take the example of the standard grip, where the L1 and R1 buttons are positioned at an angle of roughly 45 degrees from us. To move the mouse cursor up and down, rotate the gamepad around its axis, with L1 and R1 moving from the ceiling toward the screen and back. This applies to all modes (0, 2). The difference begins with left-right movements. To move the cursor to the left: <br>
0 — "steering wheel" movement to the left <br>
2 — tilt the right side of the gamepad (R1) away from you while bringing the left side (L1) closer. If you hold the gamepad horizontally (which is uncomfortable), the "steering wheel" movement returns. <br>

**For Joy-Cons** the situation is different. Since you hold a single Joy-Con in a free hand, you control the cursor either by twisting your wrist (faster but less precise) or by moving your entire forearm (slower but more precise). Two main factors negatively impact how accurately the cursor tracks your hand's actual movement vector: <br>
a) wrist rotation (clockwise/counter-clockwise, Z-axis Roll, where the SL and SR buttons point to the floor or ceiling) <br>
b) controller orientation - horizontal, with R and ZR pointing at the screen, or vertical, with them pointing to the ceiling. <br>

Differences between modes: <br>
0 — Wrist rotation always affects aiming regardless of the controller's orientation. This means that to move the cursor perfectly horizontally to the left, you must move your wrist or entire arm to the left without twisting your hand at all. <br>
1 — Wrist rotation does not matter (within 180 degrees, i.e. the range of rotation of the SL and SR buttons from floor to ceiling), but your grip does. <br>
With a relatively horizontal grip (R and ZR pointing at the screen), the cursor will strictly follow your hand's movement vector - best way to use gyro aiming on the Joy-Cons. The downside of this mode is that with a vertical grip (R and ZR pointing at the ceiling), twisting your wrist will start controlling the cursor X-axis <br>

Conclusion: Mode "1" and the horizontal "grip" provide the best accuracy and predictability for Joy-Con gyro motion, provided the accelerometer is calibrated correctly

  ### Tightening (Dynamic Smoothing)

**How it works:**
Tightening is a zero-latency, velocity-based threshold filter that attenuates micro-movements to eliminate hand tremors and pulse twitches and natural hardware sensor noise.<br>
Unlike a traditional "deadzone" that blocks small inputs entirely, Tightening smoothly dampens the sensitivity only when the controller is moving very slowly or being held still. 
When you move the controller quickly (fast flicks), the filter automatically disengages, giving you 1:1 raw and responsive input

**Recommended Values:**
* **`0.0` (Disabled):** Best for hardcore competitive players with perfectly steady hands. Provides the absolute rawest input, but you might notice micro-jitters from your own pulse.
* ** `1.0 - 2.0` (Default):** Ideal for most players with high-quality controllers (like **DualSense** or **DualShock 4**). It completely removes stationary crosshair jitter while keeping micro-adjustments (like sniper aiming) incredibly smooth and responsive.
* **`2.0` - `5.0` (For Joy-Cons & Shaky Hands):** Nintendo **Joy-Cons** have inherently "noisier" and cheaper MEMS sensors compared to Sony controllers. Values in this range perfectly anchor the crosshair and hide the hardware noise, making Joy-Cons feel incredibly stable.
* **`10.0+`:** Setting this value too high will make the gyro feel "muddy" or cause stuttering when tracking moving targets, as the speed constantly dips below the dampening threshold

### Polling Rate & Performance

Due to certain limitations within some functions in the code and bugs in JoyShockLibrary, the developer of DSAdvance was forced to use SleepTimeout=15, which corresponds to 66.6 Hz — a clearly insufficient rate for smooth movement, especially for Gyro Mouse. <br>
What limitations? The Wheel function did not work properly when SleepTimeout < 15 and has been rewritten, adding WheelXboxHoldTimer. <br>
*Note:* For details on the updated library, visit the [JoyShockLibrary Fork](https://github.com/fttlov/JoyShockLibrary)
    
 Default program polling rate is now 250 Hz (sleepTimeout=4 in config.ini; 1 sec = 1000ms / 4). CPU usage even at 250 Hz is only 0.30% to 0.60% :) The app uses a surprisingly small amount of PC resources
  
<details>
<summary><b>Why exactly 250 Hz</b></summary>

For example, the Mobapad M6S (a Joy-Con equivalent) is polled by the system via Bluetooth at a frequency of **125 Hz** (with a communication interval of 8 ms, as specified by Windows). You can check your device's polling frequency in the OSD.

#### Asynchronous Bluetooth Polling

Left and Right Joy-Cons are completely independent Bluetooth devices. They transmit their data packets asynchronously (staggered in time) rather than at the exact same millisecond. 
* The Left Joy-Con might transmit its reports at `0 ms`, `8 ms`, `16 ms`, and `24 ms`
* The Right Joy-Con might transmit its reports at `4 ms`, `12 ms`, `20 ms`, and `28 ms`

While your Bluetooth adapter doesn't "overclock" its hardware, its radio module naturally manages independent time-slots for both devices simultaneously. From the Windows operating system's perspective, new controller data arrives in the queue **every 4 milliseconds** (resulting in a combined throughput of **250 Hz**)

#### Eliminating Input Lag

If you keep your emulator's loop at **125 Hz** (`SleepTimeOut = 8`), the program only checks the Windows input queue every 8 ms. This means the Right Joy-Con's aiming data (arriving at `4 ms`) is forced to wait in the OS buffer for 4 ms before being processed at `8 ms`.

By setting the emulator's polling rate to **250 Hz** (`SleepTimeOut = 4`):
1. The engine queries the input queue every 4 ms
2. It intercepts and processes the Left Joy-Con's packet at `0 ms` and the Right Joy-Con's aiming packet almost instantly at `4 ms`
3. This effectively **halves the average input lag** of your aiming hand, delivering the most responsive gyro controls possible

</details>

Note: For single controllers (Switch Pro Controller or DualSense), it's simple: just set the polling rate shown in the OSD

### 🎮 Right Stick as Analog Triggers

Because Nintendo Switch Pro and Joy-Con controllers feature digital ZL and ZR buttons, playing games that rely on analog trigger sensitivity (such as progressive throttle in driving, target lock-on thresholds, or weapon cocking mechanics) is traditionally difficult. However, since the Pro Controller is a classic two-handed controller, it doesn't offer the same flexibility for gyro motion, so we're not considering it.

`JCAdvance` resolves this by utilizing the right analog stick. Since looking and aiming are handled completely by the gyroscope, the right stick's Y-axis is mapped to act as a dual analog trigger:<br>
* **Stick UP (Y+)** smoothly controls the virtual **Right Trigger (RT)** from 0 to 255
* **Stick DOWN (Y-)** smoothly controls the virtual **Left Trigger (LT)** from 0 to 255

To ensure a comfortable experience, the right stick's X-axis is completely disabled globally when this mode is active. This eliminates accidental horizontal camera twitching when pushing the stick up or down.

This feature can be compared to the concept of a flick stick by Jibb Smart, but in a slightly different way and only for separated Joy-cons

#### Practical Use Cases:
1. **Red Dead Redemption 2 (Duels & Weapon Cocking):**<br>
   In RDR2, digital buttons immediately register as a 100% trigger pull, which fails the duel mini-game. With this mode active, you can slowly push the right stick UP to fill the "Draw" meter progressively. In standard combat, you can slowly push the stick UP to draw or cock the hammer of your revolver, then click the physical ZR button to fire instantly. br>
2. **GTA V (On-Foot & Driving):**<br>
   You can hold physical ZL to instantly lock-on/aim with your left hand, and use the right stick UP to smoothly manage progressive trigger actions. When entering a vehicle, the right stick Y-axis automatically acts as a high-precision analog gas (Y+) and brake (Y-) pedal, allowing you to manage vehicle traction without wheel spin.
   
In other words, full analogue control is now available to most users, and features such as driving mode and external pedals further expand the vehicle’s control options, but more on that below
   
4. **Other Action Games:**
   Works for games with zoom thresholds (like *Metal Gear Solid V*) or focus-aiming mechanics (like *Hitman*), where a half-press on the trigger alters the aim perspective or stabilizes the sniper scope

 ### 🎮 Right Stick as buttons (added after right stick as triggers)

As we already know, When gyro-aiming is active, the right analog stick is completely freed from camera looking duties (for Joy-cons). Letting it sit idle is a waste of a physical input. `JCAdvance` solves this by introducing **Right Stick Mode** (`RightStickMode`), which allows you to repurpose the right stick into a versatile custom input modifier tailored to your profile's needs.

Now the right stick can be configured into three distinct profiles via the AHK configurator or the profile's `.ini` file:

* **0 — Default (Camera Mode):** The right stick functions as a standard analog stick for camera looking or aiming
* **1 — Analog Triggers (as triggers):** 
  * The vertical Y-axis (Up/Down) smoothly controls the virtual **Right Trigger (RT)** and **Left Trigger (LT)** from 0 to 255. Takes priority over other modes when activated via a hotkey
* **2 — Directional Buttons (as buttons):** 
  * Transforms the entire right analog stick into a virtual 4-directional D-pad (`RS-UP`, `RS-DOWN`, `RS-LEFT`, `RS-RIGHT`) mapped to custom Xbox buttons or KB/M keys in your active profile. <br>
Note: <br>
In "Stick as trigger" mode, you can only assign two buttons to the free X-axis (stick left-right directions).<br>
Stick as triggers mode takes priority when activated via a hotkey.

<details>
<summary><b>Axis Isolation & Diagonal Filtering</b></summary>

To ensure a highly responsive, error-free experience in both Mode 1 and Mode 2, `JCAdvance` utilizes real-time mathematical filtering. 

When you push a sensitive analog stick, your thumb rarely moves in a perfectly straight line—there is always a slight diagonal tilt. To prevent accidental double-inputs (such as triggering a horizontal shortcut button while trying to push the stick vertical), the C++ engine compares the absolute values of the axes on every frame using `fabs()`:

$$\text{Vertical Dominates} \implies |ry| \ge |rx|$$
$$\text{Horizontal Dominates} \implies |rx| > |ry|$$

- **In Mode 2 (as buttons):** The engine dynamically isolates the dominant axis. If the vertical axis dominates, the horizontal buttons are temporarily ignored (and vice versa). The stick behaves like a crisp, tactile mechanical D-pad.
- **In Mode 1 (as triggers):** If the vertical axis dominates, the stick smoothly controls `RT` or `LT`, completely ignoring horizontal buttons. If the horizontal axis dominates, the engine disables trigger inputs and lets you trigger `RS-LEFT` or `RS-RIGHT` buttons cleanly, completely separating trigger control from digital button presses.

</details>

  ### 🎮 Left stick: "Auto-Sprint" Mode

In many games, running or sprinting is assigned to a separate button. JCAdvance allows you to assign this button to the left stick directions. You can set the trigger threshold to AutoPressStickValue=90 (stick deflected by 90%) and select one of the following modes:
- 0: Default (nothing happens)
- 1: The button activates if the stick > AutoPressStickValue only in the front half (45 degrees)
- 2: The button activates if the stick > AutoPressStickValue in any direction (for old games)

### Other Features and Capabilities

  #### Split Mode & Joy-Con Mapping
  Added Split Mode for Joy-Cons and XY-axis swapping for horizontal grip. Joy-Con buttons (`SL`, `SR`, `HOME`, `CAPTURE`) can be mapped to a secondary virtual controller. When `SplitJoycons = 1` in `config.ini`, the Left Joy-Con acts as Player 1, and the Right acts as Player 2

  #### Ratchet Delay

  With classic ratcheting (hold to mute gyro motion button), the camera jerks suddenly when you release the button due to the residual movement of your hand. Default setting: 150 ms. 
  In hold to move mode, the delay does not apply

  #### Gyro Melee Gesture
  A gesture-recognition feature designed primarily for Joy-Cons. Swings (straight punch, hook, or hammer motion) can emulate any keyboard key or controller button. This lets you perform melee actions in-game without occupying a physical button. The practical use for an accelerometer. You can also adjust the impact force (G-force).

  #### EMA Smoothing Filter
The EMA (Exponential Moving Average) filter does not add traditional input lag. When you move the controller, the in-game camera starts moving instantly (0ms delay). Instead, EMA acts like a rubber band or inertia. <br>
Example (EMA = 50 / ~8ms): When you make a quick swipe, the crosshair moves immediately, but it takes about 8 milliseconds to "catch up" and reach the full speed of your hand. This completely irons out micro-tremors from your hands, but makes the crosshair feel slightly "heavier" or smoother. The higher the value, the stronger the rubber band effect. Set it to 0 for raw, unfiltered input.

Smoothing Time (time to reach 100% speed, where 100% like without filter) : 
- Value 25 (~2.7ms to reach full speed)
- Value 50 (~8.0ms to reach full speed)
- Value 75 (~24.0ms to reach full speed)

#### DualShock Emulation
  Added a feature for Nintendo controllers. When enabled, JCAdvance emulates a DirectInput Wireless Controller instead of an Xbox 360 controller. This is highly useful for legacy DirectInput games (e.g., F.E.A.R., Half-Life, classic *Need for Speed* titles)

#### Improved Driving Mode
  The `CalcMotionStick` logic was rewritten to prevent the virtual wheel from snapping in the opposite direction when reaching maximum steering angles. Added manual calibration: if the wheel gets off-center, hold your controller in a comfortable position and press the calibration hotkey to reset the center

#### External Pedals Support
  Originally designed for custom Arduino-based pedals (and a few others), this feature has been expanded to support standard DirectInput wheels/pedals.
  
  *Setup:* Connect your device, enable **"Dinput Search"** in the Steering tab of `Config.exe`, and launch `JCAdvance.exe`. If you see your dedice name `[Pedals Search] ID 0: Found device 'Your Device Name' -> APPROVED!` in the console, it is configured correctly. If inputs do not register, adjust the `PedalAxis` options in the Configurator. If automatic detection fails, try entering the name manually: launch joy.cpl via Run or cmd and replace ‘AUTO’ with the exact name of your steering wheel/pedals from joy.cpl.  <br>

I tested this feature using an old "Logitech Wingman" wheel and it f@cking works! 

### Fixes & Adjustments
  - **Gyro Stick Fix:** Resolved an issue where moving the gyro on the Y-axis caused the stick to erratically snap to the center (a JoyShockLibrary fork for DSAdvance bug). <br>
  - **Joy-Con Rumble:** Patched rumble logic for Joy-Cons (added `PacketCounter2`, flood protection, etc.)
  - **Connection Stability:** Faster connection/disconnection handling, especially for the secondary Joy-Con
  - **Crash Fixes:** Fixed a crash occurring when disconnecting two Joy-Cons simultaneously
  - **Reconnection Fix:** Fixed an issue where disconnecting Joy-Con (1) and connecting Joy-Con (2) resulted in no input registration
  - **Battery Info:** Fixed battery tracking (`Alt+I`) for the second Joy-Con

### Testing & Debugging Limitations
  - **Sony Controllers:** The developer currently lacks access to physical DualShock/DualSense controllers. While the original emulation code remains intact, some untested issues may occur.
  - **Haptic Rumble:** Tested on Mobapad M6S controllers. Due to simplified motors, full HD Rumble compatibility could not be verified.
  - **Pedals:** Tested only on a legacy Logitech steering wheel. Broad compatibility with all modern pedals cannot be guaranteed.
</details>

__________

## Potential Issues
- ~~DPI / Resolution Scaling: `Config.exe` is built using AutoHotkey. High DPI settings or unusual Windows resolutions may cause UI elements to overlap or cut off. If this happens, temporarily lower your OS scaling, change resolution.~~ Fixed
- **Antivirus Flags:** Some antivirus software may flag `Config.exe` as a false positive due to DLL calls. The source code is entirely open-source, but if you prefer, you can configure everything manually in the `.ini` files
- Steam Input conflict. Disable it (for Switch, Playstation .etc) or use HidHide
- **Bluetooth Jitter:** If you experience connection drops or infinite rumble loops while using two Joy-Cons simultaneously, your Bluetooth adapter may be struggling Known reliable adapters include the ASUS USB-BT400 and cheaper alternatives based on the same BCM20702 chip, as well as some Bluetooth 4.0 adapters from Ugreen. There are several threads on Reddit discussing this issue

### The list of supported controllers is limited by Joyshocklibrary
And will not be expanded until the transition to SDL, which is a long way off

## Credits
* [DSAdvance](https://github.com/r57zone/DSAdvance) - that was the starting point for me. r57zone has done a really great job and I thank him for that
* [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) for a cool gamepad library that makes it easy to get controller rotation. Also uses some code from this library and [JibbSmart snippet](https://gist.github.com/JibbSmart/8cbaba568c1c2e1193771459aa5385df) for aiming.
* [ViGEm](https://github.com/nefarius/ViGEmBus) for the ability to emulate various gamepads and [HidHide](https://github.com/nefarius/HidHide/) for hiding them.
* [HIDAPI library](https://github.com/signal11/hidapi) with [fixes](https://github.com/libusb/hidapi) for the library to work with a USB devices. The project uses this [fork](https://github.com/r57zone/hidapi).
* DS4Windows[[1]](https://github.com/Jays2Kings/DS4Windows)[[2]](https://github.com/Ryochan7/DS4Windows) for the battery level.
* [JoyCon-Driver](https://github.com/fossephate/JoyCon-Driver/blob/main/joycon-driver/include/Joycon.hpp) for Joy-Cons rumble.
* [Valkirie](https://github.com/Valkirie/JoyShockLibrary/commits/HDRumble) for adaptive triggers over Bluetooth.

<details>
  <summary><h3>Building, Editing, Translating</h3> (Click to open)</summary>
  <br>

### Building
0. If you're new to programming, just like me, follow the instructions below carefully:
1. Download the source code and unzip
2. Download Visual Studio 17 and [install](https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/VS17_Install.png) with these components
3. Download Windows SDK 10.0.1776.x and [install](https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/SDK_Install.png) with these components (select x64 if you need) <br>
If you have newer SDK don't forget to retarget the project
4. Choose the `Release` build type , either `x86` or `x64`, and compile the project
5. To compile the Config tool, use Ahk2exe with the base file: v2 U32 or U64. The script reads JoyShockLibrary.dll and icons from the `\Icon` folder <br>
JCadvance, Joyshocklibrary.dll and Config.exe must be the same architecture

### Editing
Added configuration and support files for editing the code in modern VS Code with clangd.

### Translating
You can easily translate the JCAdvance configurator and console interface into any language without recompiling the program. See \Language folder in Release 
</details>

__________

## Support the Project

Enjoying your favorite game with J.C. Advance? Buy me a 🍺

<!--
 👉 **[Lava.top (Apple Pay / PayPal / Visa / Mastercard)](https://app.lava.top/4003151013?tabId=donate)** <sub> (No registration, enter email for receipt & history only)</sub>
-->

👉 **[Donationalerts](https://dalink.to/fttlov)** <sub> (Very high fee - 12% 😢)</sub><table align="center">
<details>
  <summary>📷 QR code </summary>
<img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/dalink-qr-code.png" width="160" alt="CloudTips QR" />
</details>
 
### 🪙 Cryptocurrency (Direct Transfer)
<details>
<summary><b>Click to expand Crypto addresses & QR Codes</b></summary>
<br>

Please ensure you send your transaction through the **correct network** listed inside each option!

<details>
<summary>🟢 <b>USDT (BSC / BEP-20) — Recommended (Low Fee)</b></summary>
<br>
<ul>
  <li><b>Network:</b> BNB Smart Chain (BEP-20)</li>
  <li><b>Address:</b> <code>0x7bd7bb2a21d3489a6bce6de29d9e504eb6bb1429</code></li>
</ul>
<img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/BSC%20(BEP20)%200x7bd7bb2a21d3489a6bce6de29d9e504eb6bb1429.png" width="160" alt="USDT BEP-20 QR" />
<br><br>
</details>

<details>
<summary>🟢 <b>USDT (Tron / TRC-20) — Classic</b></summary>
<br>
<ul>
  <li><b>Network:</b> Tron (TRC-20)</li>
  <li><b>Address:</b> <code>TXAdZL5Y4FqhUdZP5TeShMyXPk9hBWh27o</code></li>
</ul>
<img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Tron%20(TRC20)%20TXAdZL5Y4FqhUdZP5TeShMyXPk9hBWh27o.png" width="160" alt="USDT TRC-20 QR" />
<br><br>
</details>

<details>
<summary>🔵 <b>TON (Toncoin) — Recommended (Instant & Low Fee)</b></summary>
<br>
<ul>
  <li><b>Network:</b> TON Chain</li>
  <li><b>Address:</b> <code>UQC0uPYhCF5R3OZKC_HKsNi84oLtVvXBneI8fKVwhF2Ykcro</code> (👉 <b><a href="https://tonkeeper.app/transfer/UQC0uPYhCF5R3OZKC_HKsNi84oLtVvXBneI8fKVwhF2Ykcro">Open in Wallet</a></b>)</li>
  <li><b>Important:</b> No Memo / Tag required! (Direct personal deposit address).</li>
</ul>
<img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/TON%20(TON)%20UQC0uPYhCF5R3OZKC_HKsNi84oLtVvXBneI8fKVwhF2Ykcro.png" width="160" alt="TON QR" />
<br><br>
</details>

<details>
<summary>🪙 <b>LTC (Litecoin) — Low Fee</b></summary>
<br>
<ul>
  <li><b>Network:</b> Litecoin (LTC)</li>
  <li><b>Address:</b> <code>Lb3GnY7u8aKYsFQi7nY4gi5QeWb9Y8QDeR</code></li>
</ul>
<img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/LTC%20(LTC)%20Lb3GnY7u8aKYsFQi7nY4gi5QeWb9Y8QDeR.png" width="160" alt="LTC QR" />
<br><br>
</details>

<details>
<summary>🔶 <b>BTC (Bitcoin) — Classic</b></summary>
<br>
<ul>
  <li><b>Network:</b> Bitcoin</li>
  <li><b>Address:</b> <code>1MVqQdFdf8WCGyyZP6nqCE314nZj7mDGYR</code></li>
</ul>
<img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/BTC%20(BTC)%201MVqQdFdf8WCGyyZP6nqCE314nZj7mDGYR.png" width="160" alt="BTC QR" />
<br><br>
</details>

<details>
<summary>🔷 <b>ETH (Ethereum) — ERC-20</b></summary>
<br>
<ul>
  <li><b>Network:</b> Ethereum (ERC-20)</li>
  <li><b>Address:</b> <code>0x7bd7bb2a21d3489a6bce6de29d9e504eb6bb1429</code></li>
</ul>
<img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/ETH%20(ERC20)%200x7bd7bb2a21d3489a6bce6de29d9e504eb6bb1429.png" width="160" alt="ETH QR" />
<br><br>
</details>

</details>

__________

<sub>🇷🇺 Для пользователей из РФ/РБ: **[Donate via CloudTips / МИР СБП](https://pay.cloudtips.ru/p/3ae0e7e5)**</sub>

## Feedback
`fttlkov@gmail.com`
