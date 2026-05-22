[![EN](https://github.com/fttlov/JCAdvance/blob/main/Icon/en-US.png)](https://github.com/fttlov/JCAdvance/) 
[![RU](https://github.com/fttlov/JCAdvance/blob/main/Icon/ru-RU.png)](https://github.com/fttlov/JCAdvance/blob/master/README.RU.md)
← Choose language | Выберите язык

Joy-con Advance it's a fork of DSAdvance from r57zone https://github.com/r57zone/DSAdvance <br>

## What is it
JCAdvance is an Xbox gamepad emulator with advanced features for Nintendo Joy-cons, Pro controller, DualShock 4 and DualSense Edge.<br>
All of DSAdvance's features, like a profiles, hotkeys, Magic Wheel, Air Craft, External pedals etc. have been retained.

## What's the difference:
In conception of use Gyro Motion, user friendly GUI and bugfixes <br>
DSAdvance was designed primarily for SONY's two-handed controllers. JCAdvance, as you might guess, focuses on the Joy-Con's and Gyro aiming features. <br>
I assume that Gyro aiming with a two-handed gamepad's and Joy-Con's is based on different concepts. Two handed: Gyro aiming is a fine-tuning, adjustment for classic stick aiming. Joy-con's - it's free hand full Gyro aiming - right stick for looking(aiming) in FPS is a relic of the past.

## What's new:
- Config.exe. All primary settings, mapping buttons and hotkeys now can be configured via GUI
<details>
  <summary><b>Spoiler</b></summary>
  
  <img src="https://github.com/fttlov/JCAdvance/blob/main/Icon/Screenshot_Config.png"/>
</details>

- You can map <u> any Joy-con button </u> to emulate <u>any of Xbox, Keyboard or Mouse key</u> in one main profile <br>
<sub>For Sony gamepads you can do the same, but wihou GUI by editing [Keyboard-Mouse] section in \XboxProfile\Default.ini</sub>

- Fully configurable hotkeys to activate Gyro modes with two-buttons combinations support (R+HOME)
- Add Poling rate option (see more in Technical changes)  
- Add option to read Gyro data from left Joy-con (by defaul - only for right)
- Add Smooth Filter
- (Experimental) External pedal now working with almost known dinput wheels\pedals (not just Arduino) (realy experimental)
- A new, user-friendly main menu with info about the current settings and hotkeys (press Alt+Z to old menu)
<details>
  <summary><b>Spoiler</b></summary>
  
  <img src="https://github.com/fttlov/JCAdvance/blob/main/Icon/Screenshot_Main.png"/>
</details>

## <b> Technical changes and bug fixes</b>

- Default program polling rate is now 125 Hz (sleepTimeout=8 in config.ini; 1sec = 1000ms\8). CPU usage has risen dramatically from 0.30% to 0.40% :) App uses a surprisingly small amount of PC resources<br>
Due to certain limitations by some functions in code and bugs in JoyShokLibrary, the developer of DSAdvance was forced to use SleepTimeout=15, which corresponds to 66.6 Hz — clearly insufficient rate for smooth movement of the Gyro Mouse <br>
Limitations: Wheel function did not work properly when Sleeptimeout < 15 and has been rewritten , added WheelXboxHoldTimer 
- Fixed Gyro Joysctick mode issue when the stick being randomly pulled towards the centre when Gyro move up or down (Y axis) by fixed some bugs in JoyshokLibrary
- Added EMA motion smoothing filter. Careful: add input latency. For 60fps games (value - latency): 25   ~2.7ms;  50   ~8ms;  75   ~24ms
- Rumble code fixes for Joy-cons. Add PacketCounter2, flood protection, .etc
- Fixed connect and disconnect time, specialy for second joy-con
- Fixed a crash that occurred when two joy-cons were disconnected at the same time
- Fixed: If connect joy-con(1), disconnect them and connect joycon(2) - it will not respond to input
- Fixed Battery Info (ALT+I) for 2nd Joy-con

- (Experemental) fixes for ExternalPedals function. I expect to receive the highest number of issues reports right here.
Original code has made for Arduino pedal project(i du no what is it). I couldn't get the other steering wheels/pedals to work. Code has been rewritten for: set DInput=1 in config, plug-in your USB dinput wheel\pedals, connect Nintendo/Sony gamepad and..  XBOX triggers now controlled by your pedals. If is not, try to change settings  in config setction [ExternalPedals]: 
a) Pedal1Axis - in Widows by default pedals maapping to z, z-rotation. Try others axis
b) change DeviceName: Value 'Auto' is a sort of 'smart filter'—which, of course, isn't actually smart. But you can try entering the name of your Wheel/pedals yourself. Open cmd - joy.cpl - Enter and type the exact name of your steering wheel\pedals in place of 'Auto'.

I have old "Logitech Wingman Wheel" and testing pedals to triggers axis for them.  

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
2. Download VIsual Studio 17 and [install](https://raw.githubusercontent.com/fttlov/JCAdvance/refs/heads/main/Icon/VS17_Install.png?token=GHSAT0AAAAAADYRJAGYXVW5TUCNT2QJLEOU2QML4CA) with these components
3. Download Windows SDK 10.0.1776.x and [install](https://raw.githubusercontent.com/fttlov/JCAdvance/refs/heads/main/Icon/SDK_Install.png?token=GHSAT0AAAAAADYRJAGZXLP2JUHLXWREPIBM2QML43A) with these components
4. Choose the `Release` build type  and `x86`, then compile the project
5. For compile Config use Ahk2exe, base file: v2 U32 (x86 only). Script reading JoyShockLibrary.dll and icon ftom \Icon folder

## Editing
Added support files for edit code in modern VS Code + clangd

## Feedback
`fttlkov@gmail.com`
