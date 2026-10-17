<p align="center">
  <img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/JCAdvance_logo6.png" alt="Logo" width="300"/>
</p>
<h1 align="center">JCAdvance</h1>

[![EN](https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/en.png)](README.md)
[![RU](Icon/ru.png)](README_RU.md)
← Choose your language

## What is it

**Joy-Con Advance** is an XBOX/DS4 gamepad emulator featuring advanced options for Joy-Con, Pro Controller, DualShock 4, and DualSense. It's based on the DSAdvance project by r57zone.

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
- Minimal CPU/RAM usage

## Major differences in JCAdvance:

- Distinct [Gyro Motion](#gyro-info) concept for Joy-Cons and classic gamepads: independent gyro settings tailored for each type
- **Enhanced auto-calibration** pipeline for Joy-Con to mitigate 'Thermal Drift' (see [instructions](#calibration))
- **Parametric Gyro Motion acceleration** engine with an interactive real-time graph and tooltips (see [instructions](#parametric))
- '**Stick as Triggers**' mode: brings full analog trigger functionality to Joy-Cons, completely bypassing their digital-only limitation
- **Fully customizable hotkeys** with multi-button chord/combo support (e.g., `R + HOME`)
- '**Action Layers for buttons**': bind two distinct in-game actions to a single physical button (short tap vs hold)
- **'MUTE mode'** in Action Layers: never sacrifice a physical button for Motion Aiming Control (see [instructions](#action))
- **Real-time OSD telemetry** for sticks, triggers, gyro velocity, and calibration status (via AHK or injected directly into RTSS!) (see [instructions](#osd))
- Extensive core emulator and JoyShockLibrary bugfixes, stability improvements, and enhancements<br>
- **Graphical Configurator (GUI)**! Clean, intuitive Win32 configuration tool for all settings

<table>
  <tr>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config1_en.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config2_en.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config3_en.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config4_en.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config5_en.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Config6_en.png" width="150"></td>
  </tr>
</table>

<details>
  <summary><h4>Full list of new features</h4></summary>

- All physical gamepad buttons can now emulate Xbox, keyboard, and mouse inputs within a single unified profile
- One-click binding: bind inputs interactively via Xbox image, with the "Bind" button, or manually via dropdown menus
- Action Layers: assign two distinct in-game actions to a single button (tap vs long press)
- Action Layer 'MUTE' mode: activate motion controls on hold with 0 ms lag while keeping regular button tap functions intact
- Profile Manager: create, duplicate, switch, and delete layout profiles in a dedicated GUI tab<br>
- Fully customizable hotkeys for runtime modes with combo/chord support (e.g., `R + HOME`)
- Dedicated 'Gyro Activation Button' supporting both Hold to Aim and Hold to Pause behaviors
- Gyro Activation Delay: prevents camera twitches when releasing the gyro button during ratcheting
- Gyro Space selection: fine-tune coordinate frame projection for specific controllers (see [technical info](#tech-info))<br>
- Gyro Tightening: zero-latency velocity-based filter eliminating hand tremor and sensor noise at near-zero velocities
- Audio feedback (beep) on successful baseline gyro calibration + hotkeys for manual recalibration
- In-game runtime sensitivity tuning via controller hotkeys with console change logging (see [technical info](#tech-info))<br>
- Interactive Parametric Gyro Acceleration graph (Gain Curve / Raw Accel style) with dynamic tooltips
- Real-time OSD displaying stick/trigger deflection, gyro angular velocity, and IMU stability ([technical info](#tech-info))
- Configurable internal emulator polling rate (up to 500 Hz) for smooth tracking<br>
- Left-handed Gyro option: read motion sensors strictly from the Left Joy-Con in combined dual Joy-Con mode
- Gesture recognition: map virtual buttons to a physical punch/melee gesture with adjustable G-Force trigger threshold<br>
- 'Stick as Triggers' and 'Stick as Buttons' modes with custom hotkey mode cycling (A ⇄ B toggle or full cycle)
- Radial Anti-Deadzone for hardware analog sticks and optional pass-through for Gyro Stick
- Response curve shaping (Linearity) for physical sticks and virtual Gyro Steering Wheel
- Reworked Driving Mode: eliminated extreme-angle steering flips, added on-the-fly wheel re-centering button
- DirectInput DualShock 4 emulation option instead of XInput Xbox 360 (Nintendo controllers only) for classic PC games
- (Experimental) Support for external DirectInput steering wheels and pedal sets (see [technical info](#tech-info))
- Clean 3-tier console launcher menu with live status monitoring and hotkey reference

<table align="center">
  <tr>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Main1_en.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Main2_en.png" width="150"></td>
  </tr>
</table>

</details>

<details>
  <summary><h2 id="gyro-info">Different Approaches, the Gyro Motion Concept, and 'Stick as Triggers' Mode</h2></summary>

*DSAdvance* was originally developed as an emulator tailored for two-handed Sony controllers, with Joy-Con support added as a secondary feature.<br>

**JCAdvance** not only focuses on split Joy-Con ergonomics, but provides flexible gyro tuning (Gyro Space, Tightening, Parametric Acceleration) across various controller types , as well as a definitive solution to the Joy-Con's lack of analog triggers.<br>

### The Inherent Flaw of "Two-Handed" Gyro Gaming (DualSense / Pro Controller)

When people play shooters using motion controls on a monolithic gamepad, they rarely realize how biomechanically unnatural it actually is:

* **Muscle Interference (Cross-Hand Tension):** Both hands grip a single piece of plastic. Physical tension in your left hand (active strafing, dodging, jumping, or weapon swapping) inevitably travels through the controller body straight into your right hand, and vice versa.
* **Restricted Joint Mobility:** Wrist rotation is constrained because the other hand must mirror and adapt to that angle in real time. It is impossible to execute a sharp, clean flick with just your wrist without awkwardly twisting your opposite shoulder.

**With Split Joy-Cons, your hands are physically decoupled:** Your left hand with the analog stick can rest completely relaxed on the couch, while your right hand is 100% liberated in free space — only engaging your wrist and forearm. Zero parasite tension.

### The **Gyro Motion** Concept Across Controller Types and New Capabilities for Joy-Cons:

* **Two-handed controllers**: Gyro Motion is primarily used for micro-adjustments and precision aiming, complementing traditional right-stick camera controls.<br>
* **Joy-Cons:** Separated hands grant independent 6-DOF freedom. Gyroscope is used for primary camera looking and aiming in FPS/TPS games without touching the **right stick** at all.<br>
  *Since the physical right analog stick is now completely free on Joy-cons*: <br>
* **JCAdvance allows you to use the right stick's Y-axis as virtual analog triggers**. These will work alongside the digital buttons you have already assigned to the triggers (like ZL ZR), and can be fully controlled in real time using a customisable hotkey. <br>

Practical examples of 'Stick as Triggers':<br>

**Red Dead Redemption 2**: Gently push the right stick upward to progressively fill the "Draw" meter in duels (bypassing the digital trigger limitation); smoothly cock your revolver hammer and fire, or tap ZR for rapid hip-fire.<br>
**GTA V**: Use the right stick (Y+/Y-) for progressive analog throttle and braking while driving; trigger subtle walking/drawing animations on foot.<br>

* **'Stick as Buttons'**: If you don't need analog triggers, turn the right stick into 4 additional digital buttons! Assign any virtual Xbox, keyboard, or mouse action to the four cardinal directions.<br>

Notes:<br>

- In 'Stick as Triggers' mode, the horizontal X-axis remains available for 2 custom digital shortcut buttons (Left / Right).
- The right stick hotkey behavior (A ⇄ B toggle or full 3-mode cycle) can be customized directly in `Config.exe` under the 'Analog' tab.<br>

</details>

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

<details open>
  <summary><h1 id="tech-info">Technical information and instructions</h1></summary>


<details>
    <summary><h3>Interface, Settings, and Profiles</h3></summary>

The original DSAdvance menu was overly cluttered. A modernized 3-tier menu has been implemented:<br>

- **0** — Prior to device connection<br>

- **1** — After devices are connected<br>

- **2** — Runtime hotkey reference menu

  <details>
    <summary><b>🎮 Full Hotkey & Touchpad Reference (expand)</b></summary>

  ### 💻 Keyboard Hotkeys

  * `ALT + Esc` — Exit application.

  * `ALT + V` — Swap Player 1 and Player 2 controller slots.

  * `ALT + I` — Display controller battery levels on screen.

  * `ALT + F9` — Open live stick and trigger deadzone visualizer.

  * `ALT + F` — Toggle Gyro Activation Button behavior (Hold to Aim ⇄ Hold to Pause).

  * `ALT + 2` *(or controller hotkey)* — Lock/unlock gyroscope controls **`[Configurable]`**.

  * `ALT + A` *(or controller hotkey)* — Switch Gyro Emulation mode (Mouse ⇄ Stick) **`[Configurable]`**.

  * `ALT + 1` *(or controller hotkey)* — Toggle Driving Mode (Gyro Steering Wheel) **`[Configurable]`**.

  * `ALT + D` *(or controller hotkey)* — Cycle Right Stick Modes **`[Configurable]`**.

  * `ALT + S` — Cycle Left Stick Mode (AutoSprintButton).

  * `ALT + B` — Toggle controller lightbar (Sony only).

  * `ALT + W` — Toggle touchpad mode cycling via physical touchpad click (Sony only).

  * `ALT + Up / Down` — Switch active profile.

  * `ALT + < / >` — Adjust controller rumble strength.

  * `ALT + C` *(or keyboard key)* — Recalibrate gyroscope manually (place controller on table) **`[Configurable]`**.

  * `ALT + G` *(or keyboard key)* — Recalibrate accelerometer manually (place controller on a flat surface) **`[Configurable]`**.

  * `CTRL + R` *(or keyboard key)* — Reset controllers and restart device discovery **`[Configurable]`**.

    ---

  ### 🟦 Sony Controller Hotkeys (Modifier: `PS` button)

  * `PS` *(single tap)* — Open Windows Xbox Game Bar.

  * `PS + Triangle` — Increase gyro sensitivity by 10 units (Sens +) **`[Configurable]`**.

  * `PS + Cross` — Decrease gyro sensitivity by 10 units (Sens -) **`[Configurable]`**.

  * `PS + R3` *(right stick click)* — Reset gyro sensitivity to profile defaults.

  * `PS + Square` — Master Volume Down.

  * `PS + Circle` — Master Volume Up.

  * `PS + R1` — Take screenshot (single click) / Start video recording (hold) **`[Configurable]`**.

  * `PS + L1` — Toggle lightbar on/off.

  * `PS + L3` *(Left Stick Click)* — Toggle "AutoSprint" mode for left stick.

  * `PS + Share` — Toggle touchpad mode cycling on physical click.

  * `PS + Options` — Cycle vibration strength (0% to 100% in 10% steps).

  * `PS + DPAD Up / Down` — Switch active profile.

    ---

  ### 🟥 Nintendo Controller Hotkeys (Modifier: `Capture` button)

  * `Capture + Home` — Open Windows Xbox Game Bar.

  * `Capture + X` — Increase gyro sensitivity by 10 units (Sens +) **`[Configurable]`**.

  * `Capture + B` — Decrease gyro sensitivity by 10 units (Sens -) **`[Configurable]`**.

  * `Capture + R3` *(right stick click)* — Reset gyro sensitivity to profile defaults.

  * `Capture + Y` — Master Volume Down.

  * `Capture + A` — Master Volume Up.

  * `Capture + R` — Take screenshot (single click) / Start video recording (hold) **`[Configurable]`**.

  * `Capture + Plus` — Cycle vibration strength (0% to 100% in 10% steps).

  * `HOME + L3` *(AutoSprint)* — Toggle "AutoSprint" mode for left stick.

  * `HOME + DPAD Up / Down` — Switch active profile.

    ---

  ### 🟪 Sony Touchpad Zones (DualSense / DualShock 4)

  * **Left Touch/Click** — Activate **Driving Mode** (gyro steering wheel).
  * **Right Touch/Click** — Activate **Aiming Mode** (gyro mouse or gyro stick).
  * **Center Touch/Click** — Reset controls to **Default Mode** (and briefly indicate battery level via lightbar color).
  * **Swipe along top edge** — Smoothly adjust lightbar brightness.
  * **Bottom Touch/Click** — Switch to **Desktop Mode** (mouse & keyboard navigation layout).

  <table align="center">
    <tr>
      <td><img src="https://github.com/fttlov/JCAdvance_test/blob/main/Icon/DSAdvance_Touchpad.png" width="333" alt="Touchpad Zones"></td>
    </tr>
  </table>

  <sub> Picture from **[DSAdvance official repository](https://github.com/r57zone/DSAdvance)**</sub>

  </details>

  **Settings**: All profile-specific settings have been relocated from the global `config.ini` into the `[SETTINGS]` section of individual `XboxProfiles\*.ini` files.

  **Profiles**: In original DSAdvance, there was a strict separation between Xbox profiles (in `XboxProfile`) and Keyboard/Mouse profiles (`KMProfile`), making hybrid emulation impossible. JCAdvance consolidates everything into unified profiles under `XboxProfiles` with full management inside `Config.exe`. You can still hot-swap profiles on the fly via hotkeys (profile switch persists for the active session).

</details>

<details>
  <summary><h3 id="osd">On-Screen Display (OSD)</h3></summary>

Displays real-time telemetry for analog sticks, triggers, gyroscope angular velocity, accelerometer noise, and device polling frequency.<br>

Two OSD output engines are available in `Config.exe`:<br>

1) **AHK OSD (`OSD.exe`)**: Traditional desktop overlay. *Drawbacks:* Incompatible with Exclusive Fullscreen (FSE) games, uses modest background CPU.<br>
2) **RTSS Direct**: The emulator writes directly into the RivaTuner Statistics Server shared memory pipeline without intermediate wrappers. Zero overhead. *Drawbacks:* Requires a modern RTSS installation; text may overlap with MSI Afterburner / existing RTSS overlays.

⚠️ To eliminate overlay overlapping in RTSS, use the **RTSS OSD Pos X** and **Pos Y** sliders in `Config.exe` (values from 0 to 300, where 0,0 is the top-left corner).

* **Triggers**: Raw 8-bit input values (0 to 255).<br>
* **Sticks**: 16-bit signed deflection (-32768 to 32768).<br>

Telemetry parameters for tracking thermal drift and sensor auto-calibration (see "Software Calibration" below):<br>

* **Calib (0–100%)**: Algorithm confidence. After initial baseline lock, it remains at 100%, indicating sensor noise floor has been determined. Drops to 0% only during manual recalibration or device reconnection.<br>
* **Steady (YES / NO)**: Physical stillness detector. When indicating `YES`, the controller is resting stationary and background calibration is active.<br>
* **BiasX / BiasY**: Hardware sensor drift (angular velocity offset in °/s). Low-cost Joy-Con MEMS sensors naturally "drift" as internal crystal temperature rises. These values show the raw hardware offset actively subtracted by the emulator to prevent camera wandering.<br>
* **Gyro**: Real-time angular velocity of controller movement in degrees per second (°/s). Essential for tracking hand velocity during micro-adjustments and fast flick turns when tuning the acceleration curve in the Gyro 2 tab.<br>
* **Peak**: Peak gyro velocity with a 500 ms display hold for easy reading.<br>
* **Shake (Shakiness)**: Real-time hand tremor / environmental vibration index. Mobapad M6S typical metrics: `~0.003` (resting on table), `~0.005–0.01` (resting in hand on table), and `0.01–0.015` (held unsupported in mid-air).<br>
* **MinAc (MinDeltaAccel)**: Lowest recorded accelerometer noise threshold used as a baseline anchor for software auto-calibration. Typical Mobapad M6S reading: `0.003–0.005` (indicates healthy hardware ready for calibration). If your values drastically exceed this range accompanied by uncommanded camera drift, perform hardware calibration.

*Tip*: If you play for a few minutes and place the gamepad on a table, you may see Bias numbers slightly update (e.g., shifting from 1.09 to 1.02). This confirms the software calibration has detected thermal expansion and compensated for drift.<br>
Battery percentage and live polling rate (Hz) are also displayed.

</details>

<details>
  <summary><h3 id="calibration">Calibration</h3></summary>

⚠️ *Hardware calibration is described for diagnostic understanding and is only needed if factory sensor tables are corrupted (hard drops, bad firmware flashes). If you have no severe issues, "if it works, don't touch it." Software calibration, however, is a routine part of daily play.*

### Hardware Calibration

Motion gaming relies on two distinct sensors inside the IMU:

- **Gyroscope**: Measures angular velocity (degrees per second). Calibration requires only **absolute stillness** (any orientation).
- **Accelerometer**: Measures gravity to determine "down". Calibration requires a **strictly level, flat surface**.

Hardware calibration methods:<br>

- **Sony (DualShock 4 / DualSense)**: Factory-calibrated with high-precision sensors; rarely, if ever, requires manual user intervention.<br>
- **Original Nintendo (Joy-Con / Pro Controller)**: Use the built-in calibration utility on the Nintendo Switch console (*System Settings ➔ Controllers and Sensors ➔ Calibrate Motion Controls*). If a Switch is unavailable, use the PC tool *Joy-Con Toolkit*. Calibration tables write directly to internal SPI flash memory, which JCAdvance reads on connection.<br>
- **Third-Party Clones (Mobapad, IINE, NYXI, etc.)**: Many feature proprietary hardware sensor calibration shortcuts (e.g., `R + X + HOME` on the right [Mobapad M6](https://www.youtube.com/watch?v=DRcDEmGya6M&t=2s)). However, first try reflashing controller firmware and re-pairing via Windows Bluetooth settings; this resolves most issues.

⚠️ When calibrating Joy-Cons (and clones), always dock them into a Switch console or Joy-Con Comfort Grip to prevent roll/tilt along the Z-axis (SL/SR rails must remain strictly horizontal and face each other) on a perfectly level surface. Use smartphone leveling apps (or Phyphox via Wi-Fi remote access) to verify table flatness.

### Software Calibration (Thermal Drift Compensation)

MEMS gyroscopes suffer from **thermal drift** — internal crystal heating (from battery, hands, internal circuitry) shifts the sensor's electronic zero-point over the first 5–10 minutes of play, causing unwanted camera wandering. Once temperature stabilizes, drift halts, but that offset must be canceled.<br>
JCAdvance integrates an automated software calibration engine by Jibb Smart to maintain pixel-perfect stability without manual zeroing.<br>

⚠️ *Software calibration is active only while the emulator is running and adjusts gyroscope offsets dynamically.*

#### Automatic Gyroscope Calibration

Whenever you place the controller stationary on a flat surface for 3–5 seconds, the gyroscope recalibrates. The algorithm verifies absolute stillness and recalculates the zero-bias offset.<br>
The first successful calibration emits an audible beep. Subsequent background passes can be monitored via the `Steady` indicator in OSD or by enabling `BackgroundCalibSound=1` in `config.ini`.

#### Manual Gyroscope Calibration (Hotkey)

If you prefer full control, disable auto-calibration in `config.ini` and trigger it manually at any time:<br>
Press `Alt + C` (or your configured hotkey). A low beep will sound. Immediately place the controller motionless on a table. Aiming axes will be muted. Wait for the confirmation beep.<br>
*Note: If the controller is moved during this process, an error beep will sound after 5 seconds, indicating calibration failed.*

#### Manual Accelerometer Calibration

Only required in rare cases where the accelerometer is miscalibrated and you intend to use Gyro Space modes 1, 2, or 3, but cannot perform a hardware calibration.

⚙️ `Config.ini` settings:<br>

* `AutoCalibrationEnabled=1` — (Default) Automatic background calibration enabled.<br>
* `AutoCalibrationEnabled=0` — Manual calibration via hotkey only.<br>
* `BackgroundCalibSound=1` — Audible beep on every successful auto-calibration.

<details>
  <summary>Under the Hood (expand)</summary>

**Thermal Drift Physics:**<br>
Gyroscopes measure velocity (°/s). To determine camera angle, software continuously integrates (sums) this velocity over time. If sensor heating creates a minute error of just 0.1 °/s, in 10 seconds the crosshair wanders by 1 degree; in 1 minute, by 6 degrees. The error compounds exponentially!

In JoyShockLibrary, default calibration presets assume heavy 250g Sony controllers resting on a player's lap during cutscenes. Lightweight split Joy-Cons held in one hand have far higher jitter and less thermal mass; they cannot be calibrated reliably in-hand. JCAdvance adapts these thresholds for fast 2–3 second calibration specifically on table surfaces.

**Real-World Example (Mobapad M6S):**<br>
Launch the emulator, connect Joy-Cons, place on desk, listen for the initialization beep, and play. Over the first few minutes, temperature rises and drift develops. Placing the controller down for 2–3 seconds recalibrates the baseline. After 2–3 quick table rests during the first 5–10 minutes, the sensor temperature reaches equilibrium and you can play for hours without drift.

**IMU Library Improvements:**<br>

* In rare cases with stock JoyShockLibrary, auto-calibration could lock up and refuse to trigger (`Steady` stuck at "No"). Resolved via the new "Adaptive Noise Threshold" engine in `GamepadMotion.hpp`. See: [JoyShockLibrary Auto-calibration Fix](https://github.com/fttlov/JoyShockLibrary/blob/main/README.md).

#### Advanced `config.ini` Gyroscope Parameters (Nintendo only):

1. `MaxStillnessError`: Stillness tolerance threshold. Values above 4.0 may cause calibration to trigger while held in-hand, risking calibration errors (default: `2.0`).<br>
2. `MinStillnessCollectionTime`: Duration of the initial sensor noise profiling phase once motion ceases (default: `0.5` s).<br>
3. `MinStillnessCorrectionTime`: Verification timer ensuring the device is resting on a solid surface rather than held steady by hand. Moving the controller resets both timers (default: `2.0` s).<br>
4. `StillnessCalibrationEaseInTime`: Smooth transition time (in seconds) for applying newly calculated bias offsets. Joy-Cons calibrate strictly on tables, so minimal values work best (default: `3.0` s).

#### Advanced Accelerometer Parameters:

1. `GravityShakinessMin`: Minimum jitter threshold below which the controller is treated as completely stationary (`0.01`).<br>
2. `GravityShakinessMax`: Severe shake threshold where gravity vector correction smooths out to reject centrifugal forces during fast swings (`0.4`).<br>
3. `GravityStillSpeed`: Gravity vector convergence rate at rest (`1.0`).<br>
4. `GravityShakySpeed`: Gravity tracking rate during active motion; low values preserve level horizon during fast turns (`0.1`).

</details>

</details>

<details>
  <summary><h3>Gyro Motion Space</h3></summary>

Determines how physical controller rotation is projected into cursor, stick, or in-game camera coordinates. The original DSAdvance hardcoded this to "0", severely limiting aiming ergonomics.<br>

JCAdvance supports all 3 standard JSL Gyro Space modes:

* **0 (Local Space)**: Pure gyroscope mode. Motion is calculated strictly relative to the controller chassis, ignoring gravity/accelerometer entirely.<br>
* **1 (World Space)**: Gyroscope + accelerometer. Uses Earth gravity to decouple horizontal (yaw) and vertical (pitch) aiming regardless of controller tilt.<br>
* **2 (Player Space)**: Gyroscope + accelerometer. Designed for two-handed gamepads.

👌 **New mode added to JoyShockLibrary:**<br>

* **3 (Planar Space):** Relies on the gyroscope and accelerometer. It mathematically projects movement onto a 2D plane, completely ignoring wrist-roll ("screwdriver" effect) at any grip angle, but have a limits for extreme angles; otherwise, the direction of motion reverses. <br>
Designed specifically for the Joy-Con

⚠️ *If horizontal hand movements cause diagonal crosshair drift in modes 1, 2, or 3, your accelerometer requires calibration (see Hardware Calibration). If hardware calibration is impossible, use manual software calibration or stay on Local Space.*

#### Practical Mode Guide:

* **Two-handed controllers.** For standard grip:
  * **0**: Turn controller like a steering wheel (Roll)  (gyro only mode)
  * **2**: Turn controller like tank steering levers (Yaw) (gyro+accelerometer)

If you hold the gamepad horizontally (which is uncomfortable), the "steering wheel" movement returns in mode 2. <br>
 
* **Joy-Cons**: Hand movements can turn via wrist rotation or forearm sweeps. Two factors interfere with true vector tracking:
  * a) **Roll**: Wrist twisting clockwise/counter-clockwise ("screwdriver" gesture)
  * b) **Pitch**: Grip angle ('horizontal' with buttons facing screen or 'vertical' facing ceiling)

*Airplane Metaphor:* Imagine the Joy-Con is an airplane: ZR is the nose, ABXY is the roof. Controls: Pitch = nose up/down, Yaw = nose left/right, Roll = twisting around the nose.

* **0**: Roll > 0 skews pitch and yaw axes; requires keeping Joy-Con strictly level (gyro only mode)
* **1**: Roll < 120° avoids axis skewing; Pitch > 0 causes wrist twists (screwdriver) to influence Yaw (gyro+accelerometer)
* **3**: No axis skewing, no screwdriver effect, but aim locks when Pitch > 90° (gyro+accelerometer)

*Conclusion*: For standard horizontal grip, **Mode 3** provides the highest precision and predictability. <br>
You can see this for yourself by testing how the mouse cursor behaves in different modes. For example, try drawing an infinity symbol while tilting the gamepad at different angles.  <br>
For vertical grip, use **Mode 1**.

</details>

<details>
  <summary><h3>Tightening</h3></summary>

A dynamic, zero-latency velocity threshold filter by Jibb Smart designed to eliminate micro hand tremor, pulse twitches, and hardware sensor noise. Unlike a traditional deadzone (which bluntly cuts small movements), Tightening gently attenuates sensitivity only at near-zero speeds. Fast movements pass through completely untouched (1:1).

**Recommended values:**

* **`0.0` (Disabled)**: Raw, unfiltered sensor feed. Recommended only for competitive players with rock-steady hands.
* **`1.0 – 2.0` (Default)**: Ideal for high-grade controllers (**DualSense** / **DualShock 4**). Eliminates stationary crosshair jitter while preserving micro-adjustments.
* **`2.0 – 5.0` (Joy-Cons)**: Joy-Cons have noisier MEMS sensors and one-handed grip increases tremor. This range stabilizes the crosshair cleanly.
* **`10.0+`**: Overly high values cause small aiming adjustments to feel sluggish.

</details>


<details>
    <summary><h3 id="parametric">Parametric Gyro Acceleration</h3></summary>

#### 1. Why Acceleration is Necessary

When using pure linear gyro mouse/stick response at low sensitivity, it is easy to track targets and hit accurately at long ranges, but difficult to turn 180° quickly because the rotational angle of the wrist and arm as a whole is physically limited. At high sensitivity, it becomes difficult to aim precisely.

Parametric acceleration solves this dilemma, allowing you to simultaneously achieve:

- High accuracy and control during slow, smooth movements (aiming, micro-adjustments)
- Reaching sufficient speed without sweeping arm movements (quick turns, flicks)

#### 2. Shared Settings Overview

The configurator uses a unified set of parameters for both gyro-mouse and gyro-stick modes:<br>

**MouseSens and JoySens (Base Sens):** Controls sensitivity at 1:1 linearity.<br>

**GyroAccelStart:** Angular velocity threshold (°/s) after which acceleration begins. The entire zone below this threshold operates strictly linear 1:1.<br>

**GyroAccelEnd:** Angular velocity threshold (°/s) at which acceleration reaches its maximum.<br>

**GyroAccelCap:** <br>
• **For Mouse:** Final sensitivity acceleration multiplier <br>
• **For Stick:** Curve shape along which the stick approaches maximum deflection (32768)

#### 3. Gyro Mouse Mode

⚠️ Gyro mouse speed is completely decoupled from the emulator polling rate.

##### Acceleration and the GyroAccelEnd = 75 °/s Standard

**How acceleration works in practice:**<br>

Suppose we have the following settings:
`MouseSens = 100` <br>
`GyroAccelStart = 25` <br>
`GyroAccelEnd = 75` <br>
`GyroAccelCap = 2.0`<br>

While aiming smoothly (hand speed not exceeding 25 °/s), the mouse works without acceleration (at base `MouseSens` speed).<br>
Increasing hand movement speed: from 25 to 75 °/s, sensitivity smoothly scales up, doubling (2.0x) exactly at the 75 °/s mark. The in-game camera rotates as fast as it would at a hand speed of 150 °/s without acceleration.<br>
Snapping faster (for example, up to 200 °/s): this 2x boost is preserved — the camera rotates with the equivalent of 400 °/s with parametric acceleration disabled.

**Bottom line:** we maintain perfect pixel control on micro-movements, while achieving fast turns with significantly less wrist effort thanks to the `GyroAccelCap` multiplier.

**Why the GyroAccelEnd = 75 °/s Standard:**

Research by **Jibb Smart** (creator of JoyShockMapper and pioneer of modern gyro gaming standards) showed (generalized description):

- Everything **slower than 25–30 °/s** is guaranteed **aiming**. Turning on acceleration here is undesirable, otherwise aiming accurately becomes difficult (lower or raise `MouseSens` for comfortable aiming).
- Everything **faster than 75 °/s** is guaranteed a **turn**. The player is no longer aiming; they want to reorient the camera as fast as possible.

Therefore, he suggested **75 °/s** as the canonical benchmark for `GyroAccelEnd`, though nothing prevents you from setting it to 400 °/s if desired.

<details>
  <summary>📐 <b>Mouse Physics & Mathematics (for enthusiasts)</b></summary>

In the emulator, **`MouseSens = 100`** is not an arbitrary number, but a precise reference point tied to the natural biomechanics of the human wrist (**75 °/s**):

$$\text{Mouse Counts per Second (Counts/s)} = \omega \times 50 \times (\text{Sens} \times 0.009)$$

At base sensitivity `100` and wrist rotation at `75 °/s`:
$$75 \times 50 \times (100 \times 0.009) = 75 \times 50 \times 0.9 = \mathbf{3375\text{ Counts/s}}$$

#### 1. What do these 3375 counts mean?

These are **hardware motion units (mickeys/counts)** dispatched by the emulator to the Windows system API (`SendInput`).

* They are **not monitor "pixels"**: on the Windows desktop, the cursor moves in logical coordinates that factor in system display scaling (DPI).
* Inside **3D games**, these counts are captured directly by the game engine and translated into camera rotation angles via the in-game sensitivity slider.

#### 2. Polling Rate Invariance

Thanks to integration over real elapsed time (`loopDeltaTime`), the total count output (3375 counts per second) remains **mathematically invariant across any emulator frequency** (125, 250, or 500 Hz). Only packet density changes:

* At 250 Hz: dispatched in batches of `13.5` counts per frame.
* At 500 Hz: dispatched in batches of `6.75` counts per frame.
* At 125 Hz: dispatched in batches of `27.0` counts per frame.

    </details>

⚠️ The graph visualizer is built on the Raw Accel principle.

### 4. Gyro Stick Mode (Right Stick)

⚠️ Gyro stick speed is completely decoupled from the emulator polling rate.

A controller stick differs fundamentally from a mouse: it controls **camera rotation speed** rather than crosshair displacement. As long as the stick is deflected, the camera rotates. Furthermore, game developers typically do not make stick response linear by default — a non-linear response curve is already applied to physical stick travel. Most games also include their own internal stick acceleration settings.

❕ *Recommendation: before configuring parametric acceleration in JCAdvance, try disabling stick acceleration in the game settings, and set the in-game stick sensitivity to maximum.*

#### Gyro Stick Mechanics:

##### 1. Hardware Limit and Saturation

A virtual stick has a hard hardware ceiling: the axis values range strictly from **-32768 to +32768** (100% physical tilt).<br>

Base sensitivity **JoySens = 100** is calibrated so that the stick reaches its limit of 32768 exactly at a hand angular velocity of **75 °/s**.<br>

This introduces an inevitable physical boundary known as **Saturation**:
It is physically impossible to deflect the stick beyond 100%. In other words, you cannot turn the in-game camera using the gyro-stick faster than it turns when the physical stick is pushed completely to its maximum limit.

##### 2. Wrist Biomechanics: Why 75 °/s?

To keep the virtual stick held at maximum (32768), the hand must rotate continuously. However, the anatomical limit of comfortable human wrist rotation is only **70°–80°**, after which the joint hits a physical wall.

`JoySens = 100` and `GyroAccelEnd = 75 °/s` provide a balanced physiological equilibrium:

- **Natural Turn:** Rotating the wrist across comfortable 75° at 75 °/s takes exactly **1 second** of holding the virtual stick at maximum. In this second, the game manages to rotate the camera to the desired angle without joint strain.  
  *(This represents ideal conditions; in practice, to reliably hold maximum deflection (32768) for turning the camera at maximum speed, angular velocity should be slightly above 75 °/s.)*

- **If the threshold is set too high (e.g., 300 °/s):** The entire wrist range of motion is burned through in 0.25 seconds — the hand has already hit its anatomical limit, while the in-game camera barely had time to move.

- **If the threshold is set too low (e.g., 30 °/s):** The stick will slam into 32768 from minor hand twitches, depriving you of fine micro-adjustments.

  ⚠️ **Why swinging the hand at 250 °/s is useless:** The stick already hits its ceiling (32768) at a hand speed of 75 °/s. The game technically cannot turn the camera any faster (saturation), and you exhaust your entire available wrist angle in a third of a second.

#### How Acceleration Parameters Work in Practice

In gyro-stick mode, acceleration does not attempt to exceed the hardware ceiling (32768), but optimizes the curve progression up to the point of saturation:

- **JoySens (Base Sens):** Sensitivity at linear response (100 is the benchmark for saturation at 75 °/s).
- **GyroAccelStart:** Hand speed below which the stick behaves strictly linear (1:1).
- **GyroAccelEnd:** Hand speed (canonical 75 °/s) at which stick deflection reaches 100% (32768).
- **GyroAccelCap:** An important setting for parametric acceleration in gyro-stick mode. Here it is **not a multiplier**, but the **shape of the curve** along which the stick moves toward saturation (32768):<br>
  `1.0` — Pure linearity<br>
  `Below 1.0` — Deceleration (suppressed initial response)<br>
  `Above 1.0` — Acceleration (faster initial curve progression)

Test preset for Halo MCC (in-game controller settings: Look Sensitivity 10/10, Look Acceleration 1/10):<br>
`JoySens = 100`<br>
`GyroAccelStart = 1`<br>
`GyroAccelEnd = 75`<br>
`GyroAccelCap = 1.3 – 1.5`

</details>

<details>
  <summary><h3>Polling Rate and Performance</h3></summary>

Due to legacy DSAdvance architecture and JoyShockLibrary bugs, the original author utilized `sleepTimeout=15` (66.67 Hz), which is insufficient for smooth gyro mouse tracking.<br>
For example, the Magic Wheel feature previously dropped virtual button inputs at higher polling rates. This logic was completely rewritten with dedicated pulse timers for reliable button registration up to 250 Hz. CPU usage remains negligible (`~0.60% CPU`).<br>

- Fixed upstream bugs and improved JoyShockLibrary: [JoyShockLibrary Fork](https://github.com/fttlov/JoyShockLibrary).<br>
- Resolved an intermittent gyro stick issue where smooth hand movements caused sub-millisecond snaps toward center, and enabled full Gyro Motion Space support.<br>

Default rate is now **250 Hz** (`sleepTimeout=4` in `config.ini`).

<details>
  <summary>Why exactly 250 Hz</summary>

### 1. Bluetooth Timeslots and Radio Scheduling

Bluetooth radio transmission is quantized into discrete hardware time intervals:

1. **Time Slots:** Bluetooth airtime is sliced into **625 µs (0.625 ms)** intervals (1600 slots per second).
2. **Packet Transmission:** In Sniff Mode, gamepads wake up on schedule, dispatch one HID report (buttons, sticks, IMU), and return to low-power state.
3. **Interval Quantization:** A minimal polling cycle (adapter poll + gamepad response) takes 2 slots — **1.25 ms**. Polling intervals are strictly multiples of 1.25 ms:
   * **10 slots (6.25 ms)** $\rightarrow$ **160 Hz**
   * **12 slots (7.50 ms)** $\rightarrow$ **133.3 Hz**
   * **24 slots (15.0 ms)** $\rightarrow$ **66.7 Hz**

### 2. Expected OSD Polling Rates (Ideal Conditions)

| Device                             | Connection | Expected OSD Rate | Notes                                           |
|:---------------------------------- |:---------- |:----------------- |:----------------------------------------------- |
| **Sony DualShock 4**               | Bluetooth  | **~250 Hz**       | Native Sony Bluetooth protocol polls every 4 ms |
| **Sony DualSense**                 | Bluetooth  | **Testing**       | Hardware capable up to 1000 Hz                  |
| **Mobapad M6s / M6 HD**            | Bluetooth  | **133 – 160 Hz**  | Varies by PC Bluetooth host controller chip     |
| **Official Joy-Con (L / R)**       | Bluetooth  | **~66.7 Hz**      | 15 ms packet containing 3 internal IMU samples  |
| **Nintendo Switch Pro Controller** | Bluetooth  | **~66.7 Hz**      | Standard Nintendo Switch protocol interval      |

*Note: The official Joy-Con IMU runs internally at 200 Hz; each 66.7 Hz Bluetooth packet carries 3 discrete sensor frames processed by JoyShockLibrary.*

### 3. Bluetooth Adapters and Radio Interference

* **Broadcom Adapters (e.g., Asus USB-BT400):** Negotiate a tight 10-slot interval, delivering **150–160 Hz** in clean conditions.
* **Integrated Intel Modules (Wi-Fi/BT combo: 9260, AX200, AX210, etc.):** Share antennas between Wi-Fi and Bluetooth. Intel firmware caps polling at 12 slots, fixing rates at **133 Hz**.
* **Packet Drops:** The human body absorbs 2.4 GHz radio waves. Blocking line-of-sight between controller and adapter causes packet loss.

OSD displays the **real-time packet arrival rate (Hz)** processed by Windows.

---

### 4. Emulator Loop (`SleepTimeOut`) and Input Queues

Internal emulator cycle rates: **250 Hz** (4 ms), **125 Hz** (8 ms), **66.7 Hz** (15 ms), or **33.3 Hz** (30 ms).

Rule of thumb: **Emulator processing frequency should equal or exceed physical packet arrival frequency.**

* **When emulator loop is slower than device rate (e.g., 125 Hz loop with 160 Hz Mobapad):**
  Packets arrive every 6.25 ms, but are fetched only every 8 ms. Packets buffer unevenly, producing micro-stutter and unstable input latency.
* **Running at 250 Hz (4 ms loop):**
  The emulator immediately flushes every packet the moment it hits the OS buffer, delivering minimal input lag.

</details>

</details>

<details>
  <summary><h3 id="action">Action Layers (Regular / LongPress) + MUTE</h3></summary>

#### 1. Behavior and Setup

The configurator provides two button mapping layers: **`Regular`** and **`LongPress`**.
As long as `LongPress` is set to `NONE`, the button functions normally.

Assigning an action to `LongPress` splits button behavior:

* **Short Tap:** Virtual action triggers **on button release** if released before the threshold
* **Hold:** Virtual action from `LongPress` triggers once held past the threshold
* **Threshold:** Default threshold is **200 ms** (customizable via `LongPress Button delay` in the **Settings** tab)

*Example:* Classic *Dark Souls* controls — tap to roll, hold to sprint.

#### 2. MUTE Mode — The Solution for Gyro Aiming

Gyro aiming requires an activation button. To avoid sacrificing in-game button functions, JCAdvance provides the special **`MUTE`** action.

**Setup Workflow:**

1. Take any button in the Regular layer that is already used in your game (e.g., Joy-Con `X` for a flashlight or reload).
2. In `LongPress`, set the same button to **`MUTE`**.
3. In the **Hotkeys** tab, bind `X` as the `Motion Aiming Button`.

**Result:** A quick tap triggers the in-game action (flashlight/relaod), while holding activates Gyro Aiming Button **instantly (with zero lag, bypassing the 150 ms ratchet timer)** without triggering unwanted in-game button presses.<br>
Full gyro activation without losing a single gamepad button!

> ⚠️ *Note: Assigning any action to `LongPress` removes continuous holding capability from the `Regular` action — it always executes as a single click on release.*

</details>

<details>
  <summary><h3>Right Stick Modes</h3></summary>

The right analog stick supports three distinct operating modes:

* **0 — Standard (Camera Mode)**: Traditional analog stick for camera rotation / aiming.
* **1 — As Analog Triggers**
* **2 — As Digital Buttons**

In `Config.exe`, the "Next Right Stick Mode" setting controls runtime hotkey cycling. You can cycle all 3 modes sequentially or configure a two-way toggle (e.g., Standard ⇄ As Buttons).

#### 🎮 Right Stick as Triggers (for Joy-Con)

Nintendo Joy-Cons feature digital ZL/ZR switches, making games requiring analog trigger travel (racing throttles, lock-on thresholds, weapon cocking) difficult to play.

JCAdvance solves this using the right analog stick. Since looking and aiming are handled entirely by the gyroscope, the right stick's Y-axis drives virtual analog triggers:<br>

* **Stick UP (Y+)**: Progressively commands virtual **Right Trigger (RT)** from 0 to 255.
* **Stick DOWN (Y-)**: Progressively commands virtual **Left Trigger (LT)** from 0 to 255.

The horizontal X-axis is isolated to avoid accidental camera nudges when moving vertically.

#### Practical Examples:

1. **Red Dead Redemption 2 (Duels and Weapon Cocking):**<br>
   In duels, digital switches instantly trigger 100% draw, causing duel failures. In this mode, gently push the right stick up to fill the "Draw" meter. In regular combat, gently push up to cock the revolver hammer, press fully to fire, or tap ZR for rapid hip-fire.
2. **GTA V (Foot & Driving):**<br>
   Hold physical ZL to aim with gyro, and use stick up for analog throttle/trigger control. In vehicles, stick Y-axis provides progressive throttle (Y+) and braking (Y-).
3. **Other Action Titles:**<br>
   Ideal for games with zoom thresholds (*Metal Gear Solid V*) or focus-aim mechanics (*Hitman*), where a half-pull stabilizes sniper scope sway.

### 🎮 Right Stick as Buttons

Converts the stick into 4 digital buttons (`RS-UP`, `RS-DOWN`, `RS-LEFT`, `RS-RIGHT`) mapped to any virtual Xbox, keyboard, or mouse input.<br>
*Note: In Trigger mode (1), the horizontal X-axis remains available for two digital shortcut buttons (`RS-LEFT`, `RS-RIGHT`).*

<details>
  <summary>Axis Isolation & Diagonal Filtering</summary>

To prevent accidental diagonal cross-chatter, JCAdvance applies mathematical axis isolation in real-time via `fabs()`:

$$\text{Vertical Dominance} \implies |ry| \ge |rx|$$
$$\text{Horizontal Dominance} \implies |rx| > |ry|$$

* **In Mode 2 (As Buttons)**: The engine detects the dominant axis and mutes perpendicular directions. The stick behaves like a crisp, tactile 4-way mechanical D-pad.
* **In Mode 1 (As Triggers)**: When the vertical axis dominates, trigger deflection engages cleanly while horizontal buttons are muted. When the horizontal axis dominates, trigger outputs are suppressed, allowing clean shortcut button clicks.

</details>

</details>

<details>
  <summary><h3>Left Stick: AutoSprint Mode</h3></summary>

Many games assign sprint to a button. JCAdvance can map this action to left stick deflection. In `Config.exe`, configure the deflection threshold (`AutoPressStickValue=90` for 90% travel) and operation mode:

- **0**: Disabled.
- **1**: Triggers only within the forward 45° hemisphere.
- **2**: Triggers on stick deflection in all 360° directions (convenient for classic engines like *Doom 3* / *dhewm3*).

</details>

<details>
  <summary><h3>Other Features & Capabilities</h3></summary>

- **In-Game Sensitivity Tuning**

  Adjust sensitivity in your game menu as usual. If further adjustments are needed, fine-tune gyro sensitivity during gameplay via controller hotkeys (5-unit steps). When closing the game, all sensitivity adjustments are logged in the console. Take the final numbers and save them permanently to your profile in `Config.exe`.<br>
  *Especially useful when using Gyro Stick, as games frequently apply internal stick response curves and acceleration, making calibration tricky.*

❗ *Switching runtime modes refreshes the console display, which may clear previous on-screen log lines.*

- **Radial Anti-Deadzone:**<br>
  Compensates for hardcoded, unconfigurable game deadzones using an elliptical radial algorithm that preserves diagonal trajectory angles without geometric distortion.

  * `AntiDeadZoneLeftX/Y` & `AntiDeadZoneRightX/Y`: Pushes physical stick input outside the game's deadzone.
  * `GyroApplyAntiDeadZone`: Toggles whether anti-deadzone compensation applies to Gyro Stick.

- **EMA Smoothing Filter:**<br>
  An advanced smoothing filter designed to eliminate hand tremor without introducing traditional input lag. When you move the controller, the in-game camera begins moving instantly (0 ms initial latency). Instead of delaying your input, EMA acts like simulated physical inertia or a "stretched rubber band".<br>
  *Example (EMA = 50 / ~8 ms):* During a sudden flick, the crosshair reacts immediately, but takes approximately 8 milliseconds to smoothly reach the full tracking speed of your hand. This filters out micro-tremors and pulse twitches, giving the crosshair a slightly "heavier", remarkably stable feel. Higher values result in smoother acceleration and a longer deceleration "tail" (when physical motion ceases, the crosshair continues gliding to a halt for a split second).<br>
  Ramp-up time to 100% velocity and decay time back to 0% (where 100% and 0% represent unfiltered raw input):<br>
  * **Value 25:** `~2.7 ms`
  * **Value 50:** `~8 ms`
  * **Value 75:** `~24 ms`

- **Gyro Activation (Ratchet release) Delay:** Prevents camera twitch when releasing the gyro button during ratcheting (default: 150 ms; active only in Hold to Pause mode).

- **Melee Gyro Gesture:** Map physical punches or hammer swings to virtual buttons with adjustable G-force thresholds.

- **Joy-Con Split Mode:** Horizontal individual Joy-Con grip mode with SL/SR/HOME/CAPTURE mapping. Experimental and limited due to lack of inputs.

- **DualShock 4 DirectInput Emulation:** Creates a DirectInput Wireless Controller rather than XInput Xbox 360  for classic PC games like *F.E.A.R.* or *Need for Speed*. (Nintendo controllers only)

- **Reworked Driving Mode:** Overhauled `CalcMotionStick` logic preventing steering flips at extreme angles. Includes on-the-fly wheel re-centering button.

### External Pedals

*This is an experimental feature; proper functioning is not guaranteed.*

As far as can be understood, the original author of DSAdvance focused primarily on custom DIY pedals connected via Arduino and other prototype boards.<br>
The code has been completely rewritten to perform genuine DirectInput device enumeration via the Windows Registry, automatically capture device names, and filter out regular gamepads (isolating only steering wheels and pedal sets). In `Config.exe`, you can now manually specify the device name (from the registry or `joy.cpl`) and reassign axes, as different manufacturers map pedals to completely different axes. All 8 standard Windows DirectInput axes are available for manual configuration. Default axes: `Z` and `Z-ROTATION`.<br>

**How to connect and configure:**<br>
1. Connect your wheel/pedals to your PC.<br>
2. In `Config.exe` under the **Driving** tab, set **DirectInput Search** to **On**.<br>
3. Launch `JCAdvance.exe` and check the console window. If you see a line like:  
   `[Pedals Search] ID 0: Found device 'Your Wheel/Pedals Name' -> APPROVED!`  
   everything is working correctly — the device was recognized and routed into the pedal slot.<br>
4. Connect your main controller and test pedal input in `XInputTest` or Windows `joy.cpl`. If the virtual triggers do not respond to pressing the pedals, try selecting different axes for `Pedal1Axis` and `Pedal2Axis` in `Config.exe`.<br>
5. **Troubleshooting:** If the console does not display your device name or inputs do not register at all, open `joy.cpl` (press `Win + R`, type `joy.cpl` and hit Enter), copy the exact name of your wheel/pedals as it appears in the Windows controller list, and paste it into the **Device Name** field in `Config.exe` instead of `AUTO`.

### Bugfixes & Enhancements

- Improved Joy-Con rumble handling.<br>
- Optimized connect/disconnect stability when handling dual Joy-Cons.<br>
- Fixed crash when simultaneously disconnecting both Joy-Cons.<br>
- Fixed unresponsive inputs when disconnecting Joy-Con 1 and connecting Joy-Con 2.<br>
- Fixed missing battery reporting on secondary Joy-Con.<br>
- Multiple upstream fixes in [JoyShockLibrary Fork](https://github.com/fttlov/JoyShockLibrary/blob/main/README.md).

### Debugging Notes:<br>

Testing on Sony controllers is currently limited due to lack of test hardware.<br>
Joy-Con rumble tests conducted on Mobapad M6S with standard linear vibration.<br>
Pedal tests validated using a legacy Logitech Wingman setup.

</details>

</details>

__________

## Potential Issues

- ~~DPI / Resolution Scaling: `Config.exe` is built using AutoHotkey. High DPI settings or unusual Windows resolutions may cause UI elements to overlap or cut off. If this happens, temporarily lower your OS scaling, change resolution.~~ Fixed
- **Antivirus Flags:** Some antivirus software may flag `Config.exe` as a false positive due to DLL calls. The source code is entirely open-source, but if you prefer, you can configure everything manually in the `.ini` files
- Steam Input conflict. Disable it (for Switch, Playstation .etc) or use HidHide
- **Bluetooth Jitter:** If you experience connection drops or infinite rumble loops while using two Joy-Cons simultaneously, your Bluetooth adapter may be struggling.
  Known reliable adapters:  <br>
  (USB) ASUS BT400 and cheaper alternatives based on the same BCM20702 chip, some Bluetooth 4.0 adapters from Ugreen <br>
  (M.2) Intel 9260, AX200, AX210  <br>
  There are several threads on Reddit discussing this issue

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
* [AL2009man](https://github.com/AL2009man/Gamepad-Asset-Pack) for Xbox 360 controller interractive image.

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
