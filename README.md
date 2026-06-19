<p align="center">
  <img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/JCAdvance_logo6.png" alt="Logo" width="300"/>
</p>
<h1 align="center">JCAdvance</h1>

[![EN](https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/en.png)](README.md)
[![RU](Icon/ru.png)](README_RU.md)
← Choose your language

## What is it
**Joy-Con Advance** is an XBOX/DS4 gamepad emulator featuring advanced options for Joy-Cons, Pro Controllers, DualShock 4, and DualSense Edge. It is based on the DSAdvance project by r57zone.

## Basic features of the original:
- Assign gamepad buttons to emulate XBOX buttons or keyboard/mouse keys (a separate profile for each) <br>
- Gyro-based modes: mouse/right stick emulation for looking/aiming; wheel emulation for driving; Aircraft <br>
- HardCoded hotkeys for switching modes and profiles in real time <br>
- Customizable sensitivity, deadzones, inverting and left/right stick/trigger switching <br>
- Rumble support for Sony/Nintendo gamepads <br>
- Dual Cons combined into a single virtual XBOX controller<br>
- Magic Wheel feature: use the gyro movements in different directions as additional buttons/actions<br>
- Support for Sony DualSense adaptive triggers (pistol, rifle, sniper rifle, bow, car pedal) <br>
- Support for two gamepads (second gamepad is limited to basic functions)<br>
- External pedal: connect pedals to emulate triggers or sticks (limited device support) <br>
- Minimal memory and CPU usage (0.20% - 0.50%)<br>
 
## Key Differences in JCAdvance:
- **Gyro Concept:** Separate gyro motion approaches and fine-tuning for Joy-Cons and two-handed gamepads
- **Digital Trigger Bypass for Joy-Cons:** utilize all 6 virtual Xbox analog axes (4 mapped to sticks + 2 to gyro)
- **GUI:** New config tool and redesigned user-friendly main interface
- **Bug Fixes**, **improvements** and some **new features**

<details>
  <summary><h4>Learn more about Gyro Сoncept and Digital Trigger Bypass</h4></summary>

While *DSAdvance* was primarily designed for two-handed Sony controllers with Joy-Con support, **JCAdvance** focuses on making Joy-Cons easy and intuitive to use. It introduces flexible gyroscope adjustments via the **Gyro Space** setting for different controllers and ability to use all analog axes of the virtual controller on the Joy-Cons.

The core philosophy of Gyro Motion differs between controller types:
* **Two-handed controllers:** Gyro is best used for fine-tuning and adjusting classic stick aiming.
* **Joy-Cons:** True, free-hand full gyro motion control. Using the right stick for aiming in FPS/TPS becomes obsolete.<br>
*Since the right analog stick is now completely free on Joy-cons*: <br>
* **JCAdvance allows you to use the right stick's Y-axis as analog triggers**. These will work alongside the digital buttons you have already assigned to the triggers (like ZL ZR), and can be fully controlled (on/off) in real time using a customisable hotkey. <br>

Examples of use stick as triggers:<br>
* **RDR 2:** Slowly pull the right stick UP to fill the draw meter in duels (bypassing the digital trigger issue); smoothly cock your revolver's hammer and fire or just rapid fire by digital trigger <br>
* **GTA V:** Using the right stick (up/down) for analog gas/brakes in vehicles; progressive trigger actions on foot.

* **Stick as buttons** - Don't use the sticks as triggers? Use them as buttons! Assign any virtual Xbox, keyboard or mouse buttons to one of the stick's four directions. <br>
Note: <br>
In "Stick as trigger" mode, you can only assign two buttons to the free X-axis (stick left-right directions).<br>
Stick as triggers mode takes priority when activated via a hotkey.

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
- **Auto-Bind:** Quickly bind buttons using the "Bind" or select them manually from a drop-down list
- **Profile Manager:** Create and manage profiles within a dedicated tab in the configurator
- **Custom Hotkeys:** Activate Gyro modes with customizable key combinations (e.g., `R + HOME`)
- **Gyro control options:** (clutch/ratcheting) to start/stop motion tracking by pressing a mapped button.
- **Gyro Melee Gesture:** Perform physical punching, hooking, or hammering gestures to trigger virtual buttons
- **Gyro Space Option:** A crucial setting for Gyro Mouse/Stick modes (see [Technical Details](#technical-details-and-bug-fixes) for more information)
- **Left handed mode:** Option to read Gyro data from the left Joy-Con in combined mode
- Added customizable hotkey for manually recalibrating the gyroscope (place it on a flat surface and press hotkey)
- **Right Stick as triggers mode:** utilize all 6 virtual controller analog axes for Joy-cons
- **Right Stick as buttons mode:** using the stick directions as virtual buttons for Joy-cons
- **Polling Rate Option:** Increase the polling rate for smoother motion response
- **EMA Filter:** Smooths out jittery movements
- **Non-Linear Response:** Non-linear stick and steering wheel sensitivity options
- **DirectInput Emulation:** Option to emulate a DirectInput controller instead of a virtual XBOX 360 controller
- **Improved Driving Mode:** Eliminated sudden steering wheel jerks to the opposite side at maximum angles
- Added a hotkey for manual steering wheel recalibration/centering
- **Pedal Compatibility:** External pedal feature now works with almost all standard DirectInput wheels/pedals
- **Clean Main Menu:** Displays active settings and hotkeys at a glance

 <table align="center">
  <tr>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/Main1_en.png" width="150" alt="Main Interface"></td>
  </tr>
</table>
 
## Requirements
- [ViGEm Bus Driver](https://github.com/nefarius/ViGEmBus) — Virtual Gamepad Emulation Framework by nefarius.
- [Microsoft Visual C++ Redistributable 2017 (x86/x64)](https://learn.microsoft.com/en-us/answers/questions/4137965/download-link-for-microsoft-visual-c-2017-redistri) or newer.

## How to Use
1. Download the latest release from the [Releases](https://github.com/fttlov/JCAdvance/releases) page.
2. Unzip the archive to any folder.
3. Open `Config.exe` to configure your buttons and hotkeys.
4. Run `JCAdvance.exe`, connect your gamepad, and enjoy!

## Important Note
To prevent double-input issues in games (where a game detects both your physical controller and the virtual XBOX/DS4 controller simultaneously), you should hide your physical gamepad.
Best way - using the [HidHide](https://github.com/nefarius/HidHide) utility.

<details>
  <summary><b>Quick Setup Guide</b></summary>
  <br>
  Download and install HidHide. Open the HidHide Configuration Client and:
  
  1. Add `JCAdvance.exe` and `Config.exe` to the Applications list.
  2. Select your physical gamepad in the Devices tab.
  3. Select the **"Enable device hiding"** option.

  <table align="center">
    <tr>
      <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/HidHide1.png" width="150" alt="HidHide Setup 1"></td>
      <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/HidHide2.png" width="150" alt="HidHide Setup 2"></td>
    </tr>
  </table>

  For a complete guide, visit the [official HidHide Setup Guide](https://docs.nefarius.at/projects/HidHide/Simple-Setup-Guide/).
</details>  

<details>
  <summary><h2>Technical details and bug fixes</h2> (Click to open)</summary>
  <br>

  ### Interface and Profiles
  A new 3-layer menu system has been introduced:
  - **Layer 0:** Shown before connecting devices.
  - **Layer 1:** Active after controllers are connected.
  - **Layer 2:** Classic detailed menu (retained for compatibility, touchpad hotkey info, and legacy profile management).

  *Profiles:* The original code strictly separated Xbox profiles (`.ini` files in the `XboxProfile` folder) and Keyboard/Mouse profiles (`KMProfile`). This prevented users from emulating both Xbox and keyboard actions in one profile. JCAdvance resolves this: the main `XboxProfile` folder now supports mixed emulation, and profiles are easily managed via `Config.exe`. Legacy `KMProfiles` are retained for backward compatibility.
   
  ### Polling Rate & Performance
  Default program polling rate is now 250 Hz (sleepTimeout=4 in config.ini; 1 sec = 1000ms / 4). CPU usage even at 250 Hz is only 0.30% to 0.60% :) The app uses a surprisingly small amount of PC resources. 

  ### Why 250 Hz (SleepTimeOut = 4) is Beneficial for Combined Joy-Cons

A single Nintendo Joy-Con controller natively operates at a **125 Hz** polling rate (an 8 ms connection interval negotiated with Windows). However, when you connect a combined pair of Joy-Cons, the system-level behavior changes dramatically. You can test this yourself by using [library](https://github.com/fttlov/JCAdvance_test/raw/refs/heads/main/Icon/JoyShockLibrary_debug.dll)  with debug output to the console (rename it to JoyShockLibrary.dll and replace the current one).

#### Asynchronous Bluetooth Polling

Left and Right Joy-Cons are completely independent Bluetooth devices. They transmit their data packets asynchronously (staggered in time) rather than at the exact same millisecond. 
* The Left Joy-Con might transmit its reports at `0 ms`, `8 ms`, `16 ms`, and `24 ms`.
* The Right Joy-Con might transmit its reports at `4 ms`, `12 ms`, `20 ms`, and `28 ms`.

While your Bluetooth adapter doesn't "overclock" its hardware, its radio module naturally manages independent time-slots for both devices simultaneously. From the Windows operating system's perspective, new controller data arrives in the queue **every 4 milliseconds** (resulting in a combined throughput of **250 Hz**).

#### Eliminating Input Lag

If you keep your emulator's loop at **125 Hz** (`SleepTimeOut = 8`), the program only checks the Windows input queue every 8 ms. This means the Right Joy-Con's aiming data (arriving at `4 ms`) is forced to wait in the OS buffer for 4 ms before being processed at `8 ms`.

By setting the emulator's polling rate to **250 Hz** (`SleepTimeOut = 4`):
1. The engine queries the input queue every 4 ms.
2. It intercepts and processes the Left Joy-Con's packet at `0 ms` and the Right Joy-Con's aiming packet almost instantly at `4 ms`.
3. This effectively **halves the average input lag** of your aiming hand, delivering the most responsive gyro controls possible.

*Note: For single controllers (like the Switch Pro Controller or DualSense), keeping the rate at 125 Hz (`SleepTimeOut = 8`) is optimal, as polling faster than their 8 ms interval will only result in duplicate empty frames. This won't make things any worse; it's just that some of the work will be wasted.
  
Due to certain limitations within some functions in the code and bugs in JoyShockLibrary, the developer of DSAdvance was forced to use SleepTimeout=15, which corresponds to 66.6 Hz — a clearly insufficient rate for smooth movement, especially for Gyro Mouse. <br>
What limitations? The Wheel function did not work properly when SleepTimeout < 15 and has been rewritten, adding WheelXboxHoldTimer. <br>
  *Note:* For details on the updated library, visit the [JoyShockLibrary Fork](https://github.com/fttlov/JoyShockLibrary).

  ### Fixes & Adjustments
  - **Gyro Stick Fix:** Resolved an issue where moving the gyro on the Y-axis caused the stick to erratically snap to the center (a JoyShockLibrary fork for DSAdvance bug). <br>
  - **EMA Smoothing Filter:** Smooths out shaky hands. *Note:* Adds slight latency (e.g., at 60fps: value 25 ~2.7ms, value 50 ~8ms, value 75 ~24ms).
  - **Joy-Con Rumble:** Patched rumble logic for Joy-Cons (added `PacketCounter2`, flood protection, etc.).
  - **Connection Stability:** Faster connection/disconnection handling, especially for the secondary Joy-Con.
  - **Crash Fixes:** Fixed a crash occurring when disconnecting two Joy-Cons simultaneously.
  - **Reconnection Fix:** Fixed an issue where disconnecting Joy-Con (1) and connecting Joy-Con (2) resulted in no input registration.
  - **Battery Info:** Fixed battery tracking (`Alt+I`) for the second Joy-Con.

  ### 🎮 Right Stick as Analog Triggers

Because Nintendo Switch Pro and Joy-Con controllers feature digital ZL and ZR buttons, playing games that rely on analog trigger sensitivity (such as progressive throttle in driving, target lock-on thresholds, or weapon cocking mechanics) is traditionally difficult. However, since the Pro Controller is a classic two-handed controller, it doesn't offer the same flexibility for gyro motion, so we're not considering it.

`JCAdvance` resolves this by utilizing the right analog stick. Since looking and aiming are handled completely by the gyroscope, the right stick's Y-axis is mapped to act as a dual analog trigger:<br>
* **Stick UP (Y+)** smoothly controls the virtual **Right Trigger (RT)** from 0 to 255.
* **Stick DOWN (Y-)** smoothly controls the virtual **Left Trigger (LT)** from 0 to 255.

To ensure a comfortable experience, the right stick's X-axis is completely disabled globally when this mode is active. This eliminates accidental horizontal camera twitching when pushing the stick up or down.

This feature can be compared to the concept of a flick stick by Jibb Smart, but in a slightly different way and only for separated Joy-cons.

#### Practical Use Cases:
1. **Red Dead Redemption 2 (Duels & Weapon Cocking):**<br>
   In RDR2, digital buttons immediately register as a 100% trigger pull, which fails the duel mini-game. With this mode active, you can slowly push the right stick UP to fill the "Draw" meter progressively. In standard combat, you can slowly push the stick UP to draw or cock the hammer of your revolver, then click the physical ZR button to fire instantly. br>
2. **GTA V (On-Foot & Driving):**<br>
   You can hold physical ZL to instantly lock-on/aim with your left hand, and use the right stick UP to smoothly manage progressive trigger actions. When entering a vehicle, the right stick Y-axis automatically acts as a high-precision analog gas (Y+) and brake (Y-) pedal, allowing you to manage vehicle traction without wheel spin.
   
In other words, full analogue control is now available to most users, and features such as driving mode and external pedals further expand the vehicle’s control options, but more on that below.
   
4. **Other Action Games:**
   Works for games with zoom thresholds (like *Metal Gear Solid V*) or focus-aiming mechanics (like *Hitman*), where a half-press on the trigger alters the aim perspective or stabilizes the sniper scope.

 ### 🎮 Right Stick as buttons (added after right stick as triggers)

As we already know, When gyro-aiming is active, the right analog stick is completely freed from camera looking duties (for Joy-cons). Letting it sit idle is a waste of a highly precise physical input. `JCAdvance` solves this by introducing **Right Stick Mode** (`RightStickMode`), which allows you to repurpose the right stick into a versatile custom input modifier tailored to your profile's needs.

Now the right stick can be configured into three distinct profiles via the AHK configurator or the profile's `.ini` file:

* **0 — Default (Camera Mode):** The right stick functions as a standard analog stick for camera looking or aiming.
* **1 — Analog Triggers (as triggers):** 
  * The vertical Y-axis (Up/Down) smoothly controls the virtual **Right Trigger (RT)** and **Left Trigger (LT)** from 0 to 255. Takes priority over other modes when activated via a hotkey.
* **2 — Directional Buttons (as buttons):** 
  * Transforms the entire right analog stick into a virtual 4-directional D-pad (`RS-UP`, `RS-DOWN`, `RS-LEFT`, `RS-RIGHT`) mapped to custom Xbox buttons or KB/M keys in your active profile.
Note: Since the horizontal X-axis is not used for stick as triggers (mode "1"), you can still bind two virtual buttons to the left and right directions of the stick.

---

### Axis Isolation & Diagonal Filtering

To ensure a highly responsive, error-free experience in both Mode 1 and Mode 2, `JCAdvance` utilizes real-time mathematical filtering. 

When you push a sensitive analog stick, your thumb rarely moves in a perfectly straight line—there is always a slight diagonal tilt. To prevent accidental double-inputs (such as triggering a horizontal shortcut button while trying to push the stick vertical), the C++ engine compares the absolute values of the axes on every frame using `fabs()`:

$$\text{Vertical Dominates} \implies |ry| \ge |rx|$$
$$\text{Horizontal Dominates} \implies |rx| > |ry|$$

* **In Mode 2 (as buttons):** The engine dynamically isolates the dominant axis. If the vertical axis dominates, the horizontal buttons are temporarily ignored (and vice versa). The stick behaves like a crisp, tactile mechanical D-pad.
* **In Mode 1 (as triggers):** If the vertical axis dominates, the stick smoothly controls `RT` or `LT`, completely ignoring horizontal buttons. If the horizontal axis dominates, the engine disables trigger inputs and lets you trigger `RS-LEFT` or `RS-RIGHT` buttons cleanly, completely separating trigger control from digital button presses.

#### Practical Combined Combos:
1. **Action & RPG Games (Mode 2):**  
   Use the right stick as a dedicated weapon wheel, item hotbar, or spell selection pad. Since aiming is handled entirely by the free-hand gyro, you can swap weapons or use potions instantly without taking your hands off the controls.
2. **GTA V & Cyberpunk 2077 (Mode 1):**  
   When driving, use the right stick's Y-axis for progressive, spin-free analog acceleration (Y+) and braking (Y-). Meanwhile, use the horizontal X-axis (`RS-LEFT` / `RS-RIGHT`) for on-the-fly digital shortcuts—such as switching radio stations, checking the map, or throwing grenades—without any input clashing or camera twitching.
 
  ### Gyro Motion Space
  This option controls how the gyroscope interprets hand movements into mouse/stick movements depending on the tilt of your wrist (clockwise or counter-clockwise) and how you hold the gamepad (face buttons pointing toward you or horizontally). In DSAdvance, "0" is a hard-coded value. Now we have all 3 modes from the JoyShockLibrary creator: 0, 1, 2. <br>

In short: for two-handed gamepads, the recommended values are 0 or 2. For Joy-Con: 1 or 0.

It is hard to explain, but I will try. <br>
For two-handed gamepads: let’s take the example of the standard grip, where the L1 and R1 buttons are positioned at an angle of roughly 45 degrees from us. To move the mouse cursor up and down, rotate the gamepad around its axis, with L1 and R1 moving from the ceiling toward the screen and back. This applies to all modes (0, 2). The difference begins with left-right movements. To move the cursor to the left: <br>
0 — "steering wheel" movement to the left <br>
2 — tilt the right side of the gamepad (R1) away from you while bringing the left side (L1) closer. If you hold the gamepad horizontally (which is uncomfortable), the "steering wheel" movement returns. <br>

For Joy-Cons, the situation is different. Since you hold a single Joy-Con in a free hand, you control the cursor either by twisting your wrist (faster but less precise) or by moving your entire forearm (slower but more precise). Two main factors negatively impact how accurately the cursor tracks your hand's actual movement vector: a) wrist rotation (clockwise/counter-clockwise, Z-axis Roll, where the SL and SR buttons point to the floor or ceiling), and b) controller orientation - horizontal, with R and ZR pointing at the screen, or vertical, with them pointing to the ceiling. <br>
0 — Wrist rotation always affects aiming regardless of the controller's orientation. This means that to move the cursor perfectly horizontally to the left, you must move your wrist or entire arm to the left without twisting your hand at all. <br>
1 — Wrist rotation does not matter (within 180 degrees, i.e. the range of rotation of the SL and SR buttons from floor to ceiling), but your grip does. <br>
With a relatively horizontal grip (R and ZR pointing at the screen), the cursor will strictly follow your hand's movement vector - best way to use gyro aiming on the Joy-Cons. The downside of this mode is that with a vertical grip (R and ZR pointing at the ceiling), twisting your wrist will start controlling the cursor X-axis <br>

Reading this description might make it seem like playing this way is impossible because every mode has its downsides. But that is not the case — your brain and muscle memory adapt quickly, and all modes are highly playable (except for Joy-Con on mode 2). Test them out, find what works best for you, and you're good to go!<br>

  ### Split Mode & Joy-Con Mapping
  Added Split Mode for Joy-Cons and XY-axis swapping for horizontal grip. Joy-Con buttons (`SL`, `SR`, `HOME`, `CAPTURE`) can be mapped to a secondary virtual controller. When `SplitJoycons = 1` in `config.ini`, the Left Joy-Con acts as Player 1, and the Right acts as Player 2.

  ### Gyro Melee Gesture
  A gesture-recognition feature designed primarily for Joy-Cons. Swings (straight punch, hook, or hammer motion) can emulate any keyboard key or controller button. This lets you perform melee actions in-game without occupying a physical button.

  ### DualShock Emulation
  Added a feature for Nintendo controllers. When enabled, JCAdvance emulates a DirectInput Wireless Controller instead of an Xbox 360 controller. This is highly useful for legacy DirectInput games (e.g., older *Need for Speed* titles).

  ### Improved Driving Mode
  The `CalcMotionStick` logic was rewritten to prevent the virtual wheel from snapping in the opposite direction when reaching maximum steering angles. Added manual calibration: if the wheel gets off-center, hold your controller in a comfortable position and press the calibration hotkey to reset the center.

  ### External Pedals Support
  Originally designed for custom Arduino-based pedals (and a few others), this feature has been expanded to support standard DirectInput wheels/pedals.
  
  *Setup:* Connect your device, enable **"Dinput Search"** in the Steering tab of `Config.exe`, and launch `JCAdvance.exe`. If you see your dedice name `[Pedals Search] ID 0: Found device 'Your Device Name' -> APPROVED!` in the console, it is configured correctly. If inputs do not register, adjust the `PedalAxis` options in the Configurator. If automatic detection fails, try entering the name manually: launch joy.cpl via Run or cmd and replace ‘AUTO’ with the exact name of your steering wheel/pedals from joy.cpl.  <br>

I tested this feature using an old "Logitech Wingman" wheel and it f@cking works! 

  ### Testing & Debugging Limitations
  - **Sony Controllers:** The developer currently lacks access to physical DualShock/DualSense controllers. While the original emulation code remains intact, some untested issues may occur.
  - **Haptic Rumble:** Tested on Mobapad M6S controllers. Due to simplified motors, full HD Rumble compatibility could not be verified.
  - **Pedals:** Tested only on a legacy Logitech steering wheel. Broad compatibility with all modern pedals cannot be guaranteed.
</details>

## Potential Issues
- ~~DPI / Resolution Scaling: `Config.exe` is built using AutoHotkey. High DPI settings or unusual Windows resolutions may cause UI elements to overlap or cut off. If this happens, temporarily lower your OS scaling, change resolution.~~ Fixed
- **Antivirus Flags:** Some antivirus software may flag `Config.exe` as a false positive due to DLL calls. The source code is entirely open-source, but if you prefer, you can configure everything manually in the `.ini` files.
- Steam Input conflict. Disable it (for Switch, Playstation .etc) or use HidHide.
- **Bluetooth Jitter:** If you experience connection drops or infinite rumble loops while using two Joy-Cons simultaneously, your Bluetooth adapter may be struggling. Known reliable adapters include the ASUS USB-BT400 and cheaper alternatives based on the same BCM20702 chip, as well as some Bluetooth 4.0 adapters from Ugreen. There are several threads on Reddit discussing this issue.

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
3. Download Windows SDK 10.0.1776.x and [install](https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/SDK_Install.png) with these components <br>
If you have newer SDK don't forget to retarget the project
4. Choose the `Release` build type , either `x86` or `x64`, and compile the project
5. To compile the Config tool, use Ahk2exe with the base file: v2 U32 or U64. The script reads JoyShockLibrary.dll and icons from the `\Icon` folder.
JCadvance, Joyshocklibrary.dll and Config.exe must be the same architecture

### Editing
Added configuration and support files for editing the code in modern VS Code with clangd.

### Translating
You can easily translate the JCAdvance configurator and console interface into any language without recompiling the program. See \Language folder in Release 
</details>

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
