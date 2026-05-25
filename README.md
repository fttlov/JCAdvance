[![EN](https://github.com/fttlov/JCAdvance/blob/main/Icon/en-US.png)](https://github.com/fttlov/JCAdvance/) 
[![RU](Icon/ru-RU.png)](README_RU.md)
← Choose language

## What is it
Joy-con Advance is an Xbox gamepad emulator and advanced features for Joy-cons, Pro Controller, DualShock 4, and DualSense Edge, based on [DSAdvance](https://github.com/r57zone/DSAdvance) by r57zone. <br>

## Some basic features:
- Customizable buttons: assign gamepad buttons to emulate Xbox, keyboard, and mouse keys <br>
- Gyro-based modes: mouse/stick emulation for looking/aiming; wheel emulation for driving, Aircraft <br>
- Hotkeys for switching Gyro modes in real time<br>
- Customizable sensitivity, deadzones, and left/right stick switching
- Rumble support for Sony/Nintendo gamepads
- Dual Joy-Cons combined into a single virtual Xbox controller
- Magic Wheel feature: use the gyroscope and movements in different directions as additional buttons/actions
- Support for Sony DualSense adaptive triggers (pistol, rifle, sniper rifle, bow, car pedal);<br>
- Various emulation modes for games with adaptive triggers;<br>
- Support for two gamepads <br>
- (Experimental) External pedal: connect your wheel/pedals and emulate triggers or sticks.
- Minimal memory and CPU usage (0.20% - 0.50%)
 
## What's the difference:
In short: in the concept of using Gyro Motion, user-friendly GUIs, and bug fixes. <br>
DSAdvance was designed primarily for Sony's two-handed controllers. JCAdvance, as you might guess, focuses on Joy-con's and Gyro aiming features. <br>
I assume that Gyro aiming with two-handed gamepads and Joy-con's is based on different concepts. <br>
Two-handed: Gyro aiming is used for fine-tuning and adjusting classic stick aiming. <br>
Joy-con's: it is free-hand full Gyro motion control — the right stick for looking/aiming is a relic of the past.

## What's new:
- Config.exe: all primary settings, Gyro options, button mapping, and hotkeys can now be configured via a GUI
<table>
  <tr>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/main/Icon/Config1_en.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/main/Icon/Config2_en.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/main/Icon/Config3_en.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/main/Icon/Config4_en.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/main/Icon/Config5_en.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/main/Icon/Config6_en.png" width="150"></td>
  </tr>
</table>

- You can map <u> any digital Nintendo/Sony gamepad button </u> to emulate <u>any Xbox, keyboard, or mouse key</u> <br>
- One main profile for all. You can still switch between profiles and create new ones
- Fully configurable hotkeys to activate Gyro modes with support for two-button combinations (like R+HOME)
- Added Gyro Space option — an important setting for aiming (see more in Technical changes)
- Added Polling rate option (higher value means smoother motion)
- Added option to read Gyro data from the left Joy-con (by default, only the right one is used)
- Added Smooth motion filter
- (Experimental) External pedal now works with almost all known dinput wheels/pedals (not just Arduino)
- A new, user-friendly main menu with info about the current settings and hotkeys (press Alt+Z for the full menu)
<table>
  <tr>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/refs/heads/main/Icon/Main1.png" width="150"></td>
    <td><img src="https://raw.githubusercontent.com/fttlov/JCAdvance/refs/heads/main/Icon/Screenshot_Main2.png" width="150"></td>
  </tr>
</table>

## Requires
[ViGEm Bus Driver](https://github.com/nefarius/ViGEmBus) - Virtual Gamepad Emulation Framework by nefarius <br>
[Microsoft Visual C++ Redistributable 2017](https://learn.microsoft.com/en-us/answers/questions/4137965/download-link-for-microsoft-visual-c-2017-redistri)  or newer 

## How to use
1. Download the latest version from the [Releases](https://github.com/fttlov/JCAdvance/releases) page.
2. Unzip the archive to any folder.
3. Run `JCAdvance.exe`, connect your gamepad and enjoy!

## Important
To ensure compatibility with some games, you will need to hide your physical gamepad. Otherwise, the game might detect both your physical controller and the virtual Xbox controller at the same time (causing double-input issues). <br>
To prevent this, we recommend using the [HidHide](https://github.com/nefarius/HidHide) utility by nefarius.
See the simple installation and setup [instructions](https://docs.nefarius.at/projects/HidHide/Simple-Setup-Guide/).


<details>
  <summary><h2>Technical changes and bug fixes</h2> (Click to open)</summary>
  <br>
- Default program polling rate is now 125 Hz (sleepTimeout=8 in config.ini; 1 sec = 1000ms / 8). CPU usage even at 250 Hz is only 0.30% to 0.60% :) The app uses a surprisingly small amount of PC resources. <br>
Due to certain limitations within some functions in the code and bugs in JoyShockLibrary, the developer of DSAdvance was forced to use SleepTimeout=15, which corresponds to 66.6 Hz — a clearly insufficient rate for smooth movement, especially for Gyro Mouse. <br>
What limitations? The Wheel function did not work properly when SleepTimeout < 15 and has been rewritten, adding WheelXboxHoldTimer. For the full changelog of JoyShockLibrary, see the fork page: https://github.com/fttlov/JoyShockLibrary
- Fixed a Gyro Joystick mode issue where the stick would be randomly pulled toward the center when moving the gyroscope up or down (Y-axis), caused by bugs in JoyShockLibrary.
- Added EMA motion smoothing filter. Note: this introduces input latency. For 60fps games (value - latency): 25 ~2.7ms; 50 ~8ms; 75 ~24ms.
- Rumble code fixes for Joy-Cons. Added PacketCounter2, flood protection, etc.
- Improved connection and disconnection times, especially for the second Joy-Con.
- Fixed a crash that occurred when two Joy-Cons were disconnected at the same time.
- Fixed an issue where connecting Joy-Con (1), disconnecting it, and then connecting Joy-Con (2) resulted in no input response.
- Fixed Battery Info (Alt+I) for the second Joy-Con.

- Gyro Motion Space. This option controls how the gyroscope interprets hand movements into mouse/stick movements depending on the tilt of your wrist (clockwise or counter-clockwise) and how you hold the gamepad (face buttons pointing toward you or horizontally). In DSAdvance, "0" is a hard-coded value. Now we have all 3 modes from the JoyShockLibrary creator: 0, 1, 2. <br>

In short: for two-handed gamepads, the recommended values are 0 or 2. For Joy-Con: 1 or 0.

It is hard to explain, but I will try. For two-handed gamepads: hold the gamepad in front of you with the L2 and R2 buttons facing the ceiling. To move the mouse cursor up and down, rotate the gamepad around its axis, with L2 and R2 moving from the ceiling toward the screen and back. This applies to all modes (0, 2). The difference begins with left-right movements. To move the cursor to the left: 0 — "steering wheel" movement to the left; 2 — tilt the right side of the gamepad (R2) away from you while bringing the left side (L2) closer. If you hold the gamepad horizontally (which is uncomfortable), the "steering wheel" movement returns. <br>

For Joy-Con: We have a free hand. The cursor is moved by rotating the wrist or moving the entire arm (using the forearm, which makes aiming more precise) up, down, left, and right. In this case, rotating the wrist around its axis (clockwise and counter-clockwise) has a negative effect on cursor movement. 0 — the angle of wrist rotation (clockwise and counter-clockwise) always affects movement, meaning you will need to consciously control your wrist position to keep your physical hand movements aligned with the cursor on the screen. 1 — the only mode where this effect is neutralized. If you hold the Joy-Con more or less horizontally, interference from other axes is minimal.

- Experimental fixes for the ExternalPedals function. I expect the most issues to occur here. <br>
The original code was made for an Arduino pedal project (I do not know what it is) and DirectInput (DInput) devices, but I could not get other steering wheels/pedals to work. The code has been rewritten to support all DInput wheels/pedals. <br>
Set DInput=1 in config, plug in your USB DInput wheel/pedals, connect your Nintendo/Sony gamepad, and... Xbox triggers are now controlled by your pedals. If not, try changing the settings in the config section [ExternalPedals]: 
a) Pedal1Axis: in Windows, the default pedal mapping is Z and Z-rotation. Try other axes.
b) change DeviceName: The 'Auto' value acts as a 'smart filter' to block gamepads — which, of course, isn't actually that smart. However, you can try entering the name of your wheel/pedals manually. Open the command prompt (cmd), run 'joy.cpl', press Enter, and copy/type the exact name of your steering wheel/pedals instead of 'Auto'. <br>
I tested this feature using an old "Logitech Wingman" wheel, mapping its pedals to the trigger axes. </details>


## Potential issues
- If you’re experiencing poor connectivity or vibration issues when two Joy-Cons are connected at the same time, try a different Bluetooth adapter. Known reliable adapters include the ASUS USB-BT400 and cheaper alternatives based on the same BCM20702 chip, as well as some Bluetooth 4.0 adapters from Ugreen. There are several threads on Reddit discussing this issue.
- Currently, I do not have any Sony gamepads for testing. The original code for them has not been changed, so everything should be fine <br>
- It’s much the same with the pedals: testing the code on just one device is clearly not enough. <br>
- Joy-Con rumble: I only have a Mobapad M6S with basic rumble, so I do not know how HD Rumble behaves, or if it works at all.

## The list of supported controllers is limited by Joyshocklibrary (by JibbSmart) and will not be expanded.
## If you’d like to explore all the features like Sony Touchpad and Nintendo Home/Capture hotkeys, old profile switching and more - visit https://github.com/r57zone/DSAdvance


## Credits
* Sony and Nintendo for the most advanced gamepads and investment in innovation, and for driving innovation in games.
* [ViGEm](https://github.com/nefarius/ViGEmBus) for the ability to emulate various gamepads and [HidHide](https://github.com/nefarius/HidHide/) for hiding them.
* [HIDAPI library](https://github.com/signal11/hidapi) with [fixes](https://github.com/libusb/hidapi) for the library to work with a USB devices. The project uses this [fork](https://github.com/r57zone/hidapi).
* [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) for a cool gamepad library that makes it easy to get controller rotation. Also uses some code from this library and [JibbSmart snippet](https://gist.github.com/JibbSmart/8cbaba568c1c2e1193771459aa5385df) for aiming.
* DS4Windows[[1]](https://github.com/Jays2Kings/DS4Windows)[[2]](https://github.com/Ryochan7/DS4Windows) for the battery level.
* [JoyCon-Driver](https://github.com/fossephate/JoyCon-Driver/blob/main/joycon-driver/include/Joycon.hpp) for Joy-Cons rumble.
* [Valkirie](https://github.com/Valkirie/JoyShockLibrary/commits/HDRumble) for adaptive triggers over Bluetooth.

## Building
1. Download the source code and unzip
2. Download Visual Studio 17 and [install](https://raw.githubusercontent.com/fttlov/JCAdvance/refs/heads/main/Icon/VS17_Install.png) with these components
3. Download Windows SDK 10.0.1776.x and [install](https://raw.githubusercontent.com/fttlov/JCAdvance/refs/heads/main/Icon/VS17_Install.png) with these components <br>
If you have newer SDK don't forget to retarget the project
4. Choose the `Release` build type , either `x86` or `x64`, and compile the project. Remember about JoyshockLibrary architecture
5. To compile the Config tool, use Ahk2exe with the base file: v2 U32 or U64 (depends on JoyshockLibrary). The script reads JoyShockLibrary.dll and the icon from the `\Icon` folder.

## Editing
Added configuration and support files for editing the code in modern VS Code with clangd.

## Translation
To translate Config.exe 

## Feedback
`fttlkov@gmail.com`
