[![EN](https://github.com/fttlov/JCAdvance/blob/main/Icon/en-US.png)](https://github.com/fttlov/JCAdvance/) 
[![RU](https://github.com/fttlov/JCAdvance/blob/main/Icon/ru-RU.png)](https://github.com/fttlov/JCAdvance/blob/master/README.RU.md)
← Choose language | Выберите язык

Joy-con Advance it's a fork of DSAdvance from r57zone https://github.com/r57zone/DSAdvance <br>
r57zone has done a really great job and thank him for that.

## What is it
Just like DSAdvance, JCAdvance is an Xbox gamepad emulator with advanced features for Nintendo Joy-cons, Pro controller, DualShock 4 and DualSense Edge.<br>
All of DSAdvance's features, like a profiles, hotkeys, Magic Wheel, Air Craft, External pedals etc. have been retained.

## What's the difference:
In conception of use. <br>
DSAdvance was designed primarily for SONY's two-handed controllers. JCAdvance, as you might guess, focuses on the Joy-Con's and Gyro aiming features. <br>
You have to admit, Gyro aiming with a two-handed gamepad and with Joy-Cons is based on different concepts. Two handed: "Fine-tuning", adjustment by gyro for classic aiming. Joy-con's - free hand full Gyro aiming.

## What's new:
- Config.exe. All primary settings, mapping buttons and hotkeys now can be configured via GUI
<details>
  <summary><b>Spoiler</b></summary>
  
  <img src="https://github.com/fttlov/JCAdvance/blob/main/Icon/Screenshot_Config.png"/>
</details>

- A new, user-friendly main menu with info about the current settings and hotkeys (press Alt+Z to old menu)
<details>
  <summary><b>Spoiler</b></summary>
  
  <img src="https://github.com/fttlov/JCAdvance/blob/main/Icon/Screenshot_Main.png"/>
</details>

- You can map <u> any Joy-con button </u> to emulate <u>any of Xbox, Keyboard or Mouse key</u> in one main profile <br>
<sub>For Sony gamepads you can do the same wihou GUI, by editing XboxProfiles\Default.ini</sub>

- Fully configurable hotkeys to activate Gyro modes with two-buttons combination support (R+HOME)
- Add option to read Gyro data from left Joy-con (by defaul - only for right)

## <b> Technical changes and bug fixes</b>

- Default program polling rate is now 125 Hz (sleepTimeout=8 in config.ini; 1sec = 1000ms\8). CPU usage has risen dramatically from 0.30% to 0.40% :) App uses a surprisingly small amount of PC resources<br>
Due to certain limitations by some functions in code, the developer of DSAdvance was forced to use SleepTimeout=15, which corresponds to 66.6 Hz — clearly insufficient rate for smooth movement of the Gyro Mouse <br>
Limitations: Wheel and Tightening function, both has been rewritten. For Wheel added WheelXboxHoldTimer, for Tightening see below:
- Fixed Gyro Joysctick issue when the stick being randomly pulled towards the centre when Gyro move up or down (Y axis).
Tightening code has been rewritten (Gyro Stick part only) with add ImuState function
- Added EMA filter (not necessary, but why not?)
- Rumble code fixes for Joy-cons. Add PacketCounter2, flood protection, .etc
- Fixed connect and disconnect time, specialy for second joy-con
- Fixed a crash that occurred when two joy-cons were disconnected at the same time
- Fixed: If connect joy-con(1), disconnect them and connect joycon(2) - it will not respond to input
- Fiexed Battery Info (ALT+I) for 2nd Joy-con

# If you’d like to explore all the features, please visit https://github.com/r57zone/DSAdvance

## Credits
* Sony and Nintendo for the most advanced gamepads and investment in innovation, and for driving innovation in games.
* [ViGEm](https://github.com/nefarius/ViGEmBus) for the ability to emulate various gamepads and [HidHide](https://github.com/nefarius/HidHide/) for hiding them.
* [HIDAPI library](https://github.com/signal11/hidapi) with [fixes](https://github.com/libusb/hidapi) for the library to work with a USB devices. The project uses this [fork](https://github.com/r57zone/hidapi).
* [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) for a cool gamepad library that makes it easy to get controller rotation. Also uses some code from this library and [JibbSmart snippet](https://gist.github.com/JibbSmart/8cbaba568c1c2e1193771459aa5385df) for aiming.
* DS4Windows[[1]](https://github.com/Jays2Kings/DS4Windows)[[2]](https://github.com/Ryochan7/DS4Windows) for the battery level.
* [JoyCon-Driver](https://github.com/fossephate/JoyCon-Driver/blob/main/joycon-driver/include/Joycon.hpp) for Joy-Cons rumble.
* [Valkirie](https://github.com/Valkirie/JoyShockLibrary/commits/HDRumble) for adaptive triggers over Bluetooth.

## Building
1. Download the sources and unzip them
2. Douwnload VIsual Studio 17 and [install](https://github.com/fttlov/JCAdvance/blob/main/Icon/VS17_Install.png) with these components
3. Download Windows SDK 10.0.1776.x and [install](https://github.com/fttlov/JCAdvance/blob/main/Icon/SDK_Install.png) with these components
4. Choose the `Release` build type  and `x86`, then compile the project
5. For compile Config use Ahk2exe, base file: v2 U32 (x86 only). Script reading JoyShockLibrary.dll and icon ftom \Icon folder

## Editing
Added support files for use modern VS Code with clangd for editing

## Feedback
`fttlkov[at]gmail.com`
