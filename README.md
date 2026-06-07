<p align="center">
  <img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/JCAdvance_logo6.png" alt="Logo" width="300"/>
</p>
<h1 align="center">JCAdvance</h1>

[![EN](https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/en.png)](README.md)
[![RU](Icon/ru.png)](README_RU.md)
← Choose your language

## What is it
**Joy-Con Advance * is an Xbox/DS4 gamepad emulator featuring advanced options for Joy-Cons, Pro Controllers, DualShock 4, and DualSense Edge. It is based on the [DSAdvance](https://github.com/r57zone/DSAdvance) project by r57zone.

## Basic features of the original:
- Assign gamepad buttons to emulate Xbox buttons or keyboard/mouse keys (a separate profile for each) <br>
- Gyro-based modes: mouse/stick emulation for looking/aiming; wheel emulation for driving; Aircraft <br>
- HardCoded hotkeys for switching modes and profiles in real time <br>
- Customizable sensitivity, deadzones, inverting and left/right stick/trigger switching <br>
- Rumble support for Sony/Nintendo gamepads <br>
- Dual Joy-Cons combined into a single virtual Xbox controller<br>
- Magic Wheel feature: use the gyro movements in different directions as additional buttons/actions<br>
- Support for Sony DualSense adaptive triggers (pistol, rifle, sniper rifle, bow, car pedal) <br>
- Support for two gamepads (second gamepad is limited to basic functions)<br>
- External pedal: connect pedals to emulate triggers or sticks (limited device support) <br>
- Minimal memory and CPU usage (0.20% - 0.50%)<br>
 
## Key Differences in JCAdvance
- **New Gyro Concept:** Focuses on a completely revised approach to Gyro Motion.
- **Modern GUI:** Completely redesigned, user-friendly configuration interface.
- **Bug Fixes:** Resolved original issues and added stability improvements.

While *DSAdvance* was primarily designed for two-handed Sony controllers (with basic Joy-Con support), *JCAdvance* focuses on making Joy-Cons easy and intuitive to use. It introduces flexible gyroscope adjustments via the **Gyro Space** setting. 

The core philosophy of Gyro Motion differs between controller types:
* **Two-handed controllers:** Gyro is best used for fine-tuning and adjusting classic stick aiming.
* **Joy-Cons:** True, free-hand full gyro motion control. With Joy-Cons, using the right stick for aiming in FPS/TPS becomes obsolete.

## What's New
- **Configurator App (`Config.exe`):** Configure all primary settings, Gyro options, button mapping, and hotkeys via a graphical user interface.

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

- **Universal Mapping:** Map any digital Nintendo/Sony gamepad button to emulate any Xbox button, keyboard key, or mouse action within a single profile.
- **Auto-Bind:** Quickly bind buttons using the "Bind" feature or select them manually from a drop-down list.
- **Gyro Melee Gesture:** Perform physical punching, hooking, or hammering gestures to trigger virtual buttons.
- **Profile Manager:** Create and manage profiles within a dedicated tab in the configurator.
- **Custom Hotkeys:** Activate Gyro modes with customizable key combinations (e.g., `R + HOME`).
- **Gyro Space Option:** A crucial setting for Gyro Mouse/Stick modes (see *Technical Details* for more information).
- **Polling Rate Option:** Increase the polling rate for smoother motion response.
- **Left Joy-Con Gyro:** Option to read Gyro data from the left Joy-Con in combined mode.
- **EMA Filter:** Smooths out jittery movements (adds a small amount of latency).
- **Non-Linear Response:** Non-linear stick and steering wheel sensitivity options.
- **DirectInput Emulation:** Option to emulate a DirectInput controller instead of a standard virtual Xbox 360 controller.
- **Improved Driving Mode:** Eliminated sudden steering wheel jerks to the opposite side at maximum angles. Added a hotkey for manual calibration.
- **Pedal Compatibility:** External pedal feature now works with almost all standard DirectInput wheels/pedals.
- **Clean Main Menu:** Displays active settings and hotkeys at a glance (toggle with `Alt+Z`).

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
To prevent double-input issues (where a game detects both your physical controller and the virtual Xbox controller simultaneously), you should hide your physical gamepad.

We highly recommend using the [HidHide](https://github.com/nefarius/HidHide) utility by nefarius.

<details>
  <summary><b>Quick Setup Guide</b></summary>
  <br>
  Download and install HidHide. Open the HidHide Configuration Client and:
  
  1. Add `JCAdvance.exe` and `Config.exe` to the Applications list.
  2. Select your physical gamepad in the Devices tab.
  3. Enable the **"Enable device hiding"** option.

  <table align="center">
    <tr>
      <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/HidHide1.png" width="150" alt="HidHide Setup 1"></td>
      <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance_test/refs/heads/main/Icon/HidHide2.png" width="150" alt="HidHide Setup 2"></td>
    </tr>
  </table>

  For a complete guide, visit the [official HidHide Setup Guide](https://docs.nefarius.at/projects/HidHide/Simple-Setup-Guide/).
</details>  

<details>
  <summary><b>Technical Details and Bug Fixes</b> (Click to expand)</summary>
  <br>

  ### Interface and Profiles
  A new 3-layer menu system has been introduced:
  - **Layer 0:** Shown before connecting devices.
  - **Layer 1:** Active after controllers are connected.
  - **Layer 2:** Classic detailed menu (retained for compatibility, touchpad hotkey info, and legacy profile management).

  *Profiles:* The original code strictly separated Xbox profiles (`.ini` files in the `XboxProfile` folder) and Keyboard/Mouse profiles (`KMProfile`). This prevented users from emulating both Xbox and keyboard actions in one profile. JCAdvance resolves this: the main `XboxProfile` folder now supports mixed emulation, and profiles are easily managed via `Config.exe`. Legacy `KMProfiles` are retained for backward compatibility.
   
  ### Polling Rate & Performance
  The default polling rate is set to **125 Hz** (`sleepTimeout=8` in `config.ini`). Even at 250 Hz, CPU usage remains incredibly low (around 0.30% – 0.60%). 
  
  Due to JoyShockLibrary limitations, the original developer was forced to use `SleepTimeout=15` (66.6 Hz), which caused choppy gyro mouse movement. The driving wheel functions have been rewritten to support lower sleep timeout values, and a `WheelXboxHoldTimer` has been added.
  
  *Note:* For details on the updated library, visit the [JoyShockLibrary Fork](https://github.com/fttlov/JoyShockLibrary).

  ### Fixes & Adjustments
  - **Gyro Stick Fix:** Resolved an issue where moving the gyro on the Y-axis caused the stick to erratically snap to the center (a JoyShockLibrary bug).
  - **EMA Smoothing Filter:** Smooths out shaky hands. *Note:* Adds slight latency (e.g., at 60fps: value 25 ~2.7ms, value 50 ~8ms, value 75 ~24ms).
  - **Joy-Con Rumble:** Patched rumble logic for Joy-Cons (added `PacketCounter2`, flood protection, etc.).
  - **Connection Stability:** Faster connection/disconnection handling, especially for the secondary Joy-Con.
  - **Crash Fixes:** Fixed a crash occurring when disconnecting two Joy-Cons simultaneously.
  - **Reconnection Fix:** Fixed an issue where disconnecting Joy-Con (1) and connecting Joy-Con (2) resulted in no input registration.
  - **Battery Info:** Fixed battery tracking (`Alt+I`) for the second Joy-Con.

  ### Gyro Motion Space
  This option defines how hand rotation translates to on-screen movement based on wrist tilt (Roll) and how you hold the controller. JCAdvance includes all 3 modes from the creator of JoyShockLibrary (0, 1, and 2):
  - **Mode 0 / 2:** Recommended for two-handed controllers.
  - **Mode 0 / 1:** Recommended for Joy-Cons.

  **Understanding the Modes:**
  * **Two-handed controllers:** To aim vertically, you tilt the controller forward/backward. Horizontal aiming behaves as follows:
    * **Mode 0:** Steer the controller like a steering wheel.
    * **Mode 2:** Yaw-based rotation (tilting the right side away while pulling the left side closer). If held completely flat, it reverts to steering behavior.
  * **Joy-Cons:** Since a single Joy-Con is held freely in one hand, precision is affected by wrist roll and controller tilt.
    * **Mode 0:** Wrist roll affects horizontal aiming. Aiming straight requires keeping your wrist completely straight.
    * **Mode 1:** Wrist roll is ignored (within a 180-degree range). With a horizontal grip (triggers pointing at the screen), the cursor precisely follows your physical movement vector. If held vertically, however, wrist twisting will affect the horizontal axis.

  While these descriptions sound complex, muscle memory adapts very quickly. Most setups are highly playable (except for Joy-Cons on Mode 2). Experiment to find your preference!

  ### Split Mode & Joy-Con Mapping
  Added Split Mode for Joy-Cons and XY-axis swapping for horizontal grip. Joy-Con buttons (`SL`, `SR`, `HOME`, `CAPTURE`) can be mapped to a secondary virtual controller. When `SplitJoycons = 1` in `config.ini`, the Left Joy-Con acts as Player 1, and the Right acts as Player 2.

  ### Gyro Melee Gesture
  A gesture-recognition feature designed primarily for Joy-Cons. Swings (straight punch, hook, or hammer motion) can emulate any keyboard key or controller button. This lets you perform melee actions in-game without occupying a physical button.

  ### DualShock Emulation
  Added a feature for Nintendo controllers. When enabled, JCAdvance emulates a DirectInput Wireless Controller instead of an Xbox 360 controller. This is highly useful for legacy DirectInput games (e.g., older *Need for Speed* titles).

  ### Improved Driving Mode
  The `CalcMotionStick` logic was rewritten to prevent the virtual wheel from snapping in the opposite direction when reaching maximum steering angles. Added manual calibration: if the wheel gets off-center, hold your controller in a comfortable position and press the calibration hotkey to reset the center.

  ### External Pedals Support
  Originally designed for custom Arduino-based pedals, this feature has been expanded to support standard DirectInput wheels/pedals.
  
  *Setup:* Connect your device, enable **"Dinput Search"** in the Steering tab of `Config.exe`, and launch `JCAdvance.exe`. If you see `[Pedals Search] ID 0: Found device 'Your Device Name' -> APPROVED!` in the console, it is configured correctly. If inputs do not register, adjust the `Pedal1Axis` and `Pedal2Axis` options in the Configurator. If automatic detection fails, replace `AUTO` with your exact device name (found via `joy.cpl`) in the configuration file.

  ### Testing & Debugging Limitations
  - **Sony Controllers:** The developer currently lacks access to physical DualShock/DualSense controllers. While the original emulation code remains intact, some untested issues may occur.
  - **Haptic Rumble:** Tested on Mobapad M6S controllers. Due to simplified motors, full HD Rumble compatibility could not be verified.
  - **Pedals:** Tested only on a legacy Logitech steering wheel. Broad compatibility with all modern pedals cannot be guaranteed.
</details>

## Potential Issues
- **DPI / Resolution Scaling:** `Config.exe` is built using AutoHotkey. High DPI settings or unusual Windows resolutions may cause UI elements to overlap or cut off. If this happens, temporarily lower your OS scaling or manually edit the `.ini` files.
- **Antivirus Flags:** Some antivirus software may flag `Config.exe` as a false positive due to DLL calls. The source code is entirely open-source, but if you prefer, you can configure everything manually in the `.ini` files.
- **Bluetooth Jitter:** If you experience connection drops or infinite rumble loops while using two Joy-Cons simultaneously, your Bluetooth adapter may be struggling. Highly reliable adapters include the ASUS USB-BT400 (BCM20702 chip) and Bluetooth 4.0 adapters from Ugreen.

### Supported Controllers
The list of supported controllers is limited by the underlying [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) and will remain so until a planned transition to SDL is implemented.

## Credits
* [DSAdvance](https://github.com/r57zone/DSAdvance) — The original project by r57zone which served as the foundation.
* [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) — Gamepad library for controller rotation tracking, and [JibbSmart's aiming snippet](https://gist.github.com/JibbSmart/8cbaba568c1c2e1193771459aa5385df).
* [ViGEm](https://github.com/nefarius/ViGEmBus) — Virtual controller emulation framework, and [HidHide](https://github.com/nefarius/HidHide/) for hiding physical devices.
* [HIDAPI Library](https://github.com/signal11/hidapi) (with [USB fixes](https://github.com/libusb/hidapi)) — The project utilizes r57zone's [hidapi fork](https://github.com/r57zone/hidapi).
* [DS4Windows](https://github.com/Ryochan7/DS4Windows) — Battery level logic.
* [JoyCon-Driver](https://github.com/fossephate/JoyCon-Driver) — Joy-Con rumble implementation.
* [Valkirie's JoyShockLibrary fork](https://github.com/Valkirie/JoyShockLibrary/commits/HDRumble) — Adaptive triggers over Bluetooth.

<details>
  <summary><b>Building, Editing, and Translating</b> (Click to expand)</summary>
  <br>

  ### Building
  1. Download and extract the source code.
  2. Install **Visual Studio 2017** with the required C++ build tools.
  3. Install **Windows SDK 10.0.1776.x** (if you use a newer SDK, remember to retarget the project in VS).
  4. Select the `Release` build configuration (`x86` or `x64`) and build the project.
  5. To compile the Config tool, use `Ahk2exe` with the `v2 U32` or `v2 U64` base. Note that `JCAdvance.exe`, `JoyShockLibrary.dll`, and `Config.exe` must share the same architecture.

  ### Editing
  We have included configuration files to easily edit the codebase in VS Code with the `clangd` extension.

  ### Translating
  You can translate the interface and console output of the configurator without recompiling. Simply refer to the `\Language` folder in the release directory.
</details>

## Support the Project
Enjoying your experience with JCAdvance? Consider buying me a 🍺!

### 🌐 International:
👉 **[Lava.top (Apple Pay / PayPal / Visa / Mastercard)](https://app.lava.top/4003151013?tabId=donate)** *(No registration required, enter your email for receipts only)*

### 🪙 Cryptocurrency (Direct Transfer)
<details>
<summary><b>Click to expand Crypto addresses & QR Codes</b></summary>
<br>

Please ensure you send your transaction through the **correct network**!

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
  <li><b>Important:</b> No Memo / Tag required!</li>
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

---

<sub>🇷🇺 Для пользователей из РФ/РБ: **[Donate via CloudTips / МИР СБП](https://pay.cloudtips.ru/p/3ae0e7e5)**</sub>

## Feedback
📧 `fttlkov@gmail.com`
