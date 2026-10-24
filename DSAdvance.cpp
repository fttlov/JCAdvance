// JCAdvance by fttlov - a fork of DSAdvance by r57zone
// Advanced Xbox controller emulation for DualSense, DualShock 4, Pro Controller, Joy-Cons
// https://github.com/fttlov/JCAdvance https://github.com/r57zone/DSAdvance
// @xxx key codes https://github.com/fttlov/JCAdvance_test/blob/main/README.md

#include "DSAdvance.h"
#include <atlstr.h>
#include <dbt.h>					// Для констант вроде DBT_DEVNODES_CHANGED
#include <mmsystem.h>				// Для PlaySound
#pragma comment(lib, "winmm.lib")	// Линковка для PlaySound

//@075 Direct Shared Memory ----------------
static HANDLE g_hMapRTSS = NULL;
static LPBYTE g_pRTSS = NULL;
static ULONGLONG g_lastRTSSRetry = 0;

inline const char* GetJslDevName(int deviceIndex) {
	if (deviceIndex == -1) return "None";
	int type = JslGetControllerType(deviceIndex);
	switch (type) {
	case JS_TYPE_JOYCON_LEFT:     return "Joy-Con L";
	case JS_TYPE_JOYCON_RIGHT:    return "Joy-Con R";
	case JS_TYPE_PRO_CONTROLLER:  return "SwitchPro";
	case JS_TYPE_DS:              return "DualSense";
	case JS_TYPE_DS4:             return "DS4";
	default:                      return "Gamepad";
	}
}

//@075
static DWORD g_rtssSlot = 0;

void ClearRTSS() {
	if (!g_pRTSS || *(DWORD*)g_pRTSS != 0x52545353 || g_rtssSlot == 0) return;

	DWORD entrySize = *(DWORD*)(g_pRTSS + 0x14);
	DWORD entryOffset = *(DWORD*)(g_pRTSS + 0x18);
	DWORD arrSize = *(DWORD*)(g_pRTSS + 0x1C);

	if (g_rtssSlot >= 1 && g_rtssSlot < arrSize) {
		LPBYTE ptr = g_pRTSS + entryOffset + g_rtssSlot * entrySize;
		((char*)ptr)[0] = '\0';
		((char*)(ptr + 256))[0] = '\0';
		if (entrySize >= 4608) ((char*)(ptr + 512))[0] = '\0';
		g_rtssSlot = 0;
	}
	(*(DWORD*)(g_pRTSS + 0x20))++;
}

void CloseRTSS() {
	ClearRTSS();
	if (g_pRTSS) { UnmapViewOfFile(g_pRTSS); g_pRTSS = NULL; }
	if (g_hMapRTSS) { CloseHandle(g_hMapRTSS); g_hMapRTSS = NULL; }
}


// Перехватчик закрытия консоли Windows (нажатие на [X] или Ctrl+C)
BOOL WINAPI ConsoleCtrlHandler(DWORD ctrlType) {
	if (ctrlType == CTRL_CLOSE_EVENT || ctrlType == CTRL_C_EVENT || ctrlType == CTRL_SHUTDOWN_EVENT) {
		if (AppStatus.OsdType == 1) {
			CloseRTSS();
		}
	}
	return FALSE;
}

void UpdateRTSS(const char* text) {
	if (!g_pRTSS) {
		ULONGLONG now = GetTickCount64();
		if (now - g_lastRTSSRetry < 3000) return; // Проверка RTSS раз в 3 сек
		g_lastRTSSRetry = now;

		g_hMapRTSS = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");
		if (!g_hMapRTSS) g_hMapRTSS = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "Global\\RTSSSharedMemoryV2");
		if (!g_hMapRTSS) return;

		g_pRTSS = (LPBYTE)MapViewOfFile(g_hMapRTSS, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (!g_pRTSS) return;
	}

	if (*(DWORD*)g_pRTSS != 0x52545353) return;

	DWORD entrySize = *(DWORD*)(g_pRTSS + 0x14);
	DWORD entryOffset = *(DWORD*)(g_pRTSS + 0x18);
	DWORD arrSize = *(DWORD*)(g_pRTSS + 0x1C);

	// Ищем свободный слот (не конфликтуя с OverlayEditor)
	if (g_rtssSlot == 0 || g_rtssSlot >= arrSize) {
		for (DWORD i = 1; i < arrSize; i++) {
			LPBYTE ptr = g_pRTSS + entryOffset + i * entrySize;
			if (((char*)ptr)[0] == '\0') {
				g_rtssSlot = i;
				break;
			}
		}
	}
	if (g_rtssSlot == 0) return;

	LPBYTE target = g_pRTSS + entryOffset + g_rtssSlot * entrySize;

	// Имя владельца оставляем пустым, чтобы RTSS не лепил заголовок на экран
	((char*)target)[0] = '\0';

	strncpy_s((char*)(target + 256), 256, text, _TRUNCATE);
	if (entrySize >= 4608) {
		strncpy_s((char*)(target + 512), 4096, text, _TRUNCATE);
	}

	(*(DWORD*)(g_pRTSS + 0x20))++;
}

void GamepadSearch(AdvancedGamepad &Gamepad, std::string SkipDevPath, std::string SkipDevPath2 = "") {
	struct hid_device_info *devs, *cur_dev;

	// Sony controllers
	devs = hid_enumerate(SONY_VENDOR, 0x0);
	cur_dev = devs;
	while (cur_dev) {
		if (!SkipDevPath.empty() && SkipDevPath == cur_dev->path) { cur_dev = cur_dev->next; continue; }
		if (cur_dev->product_id == SONY_DS5 ||
			cur_dev->product_id == SONY_DS5_EDGE ||
			cur_dev->product_id == SONY_DS4_USB ||
			cur_dev->product_id == SONY_DS4_V2_USB ||
			cur_dev->product_id == SONY_DS4_BT ||
			cur_dev->product_id == SONY_DS4_DONGLE)
		{
			Gamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			if (Gamepad.HidHandle == NULL) { cur_dev = cur_dev->next; continue; }
			Gamepad.DevicePath = cur_dev->path;
			hid_set_nonblocking(Gamepad.HidHandle, 1);

			if (cur_dev->product_id == SONY_DS5 || cur_dev->product_id == SONY_DS5_EDGE) {
				Gamepad.ControllerType = SONY_DUALSENSE;
				Gamepad.USBConnection = true;

				// BT detection https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/JoyShock.cpp
				unsigned char buf[64];
				memset(buf, 0, 64);
				hid_read_timeout(Gamepad.HidHandle, buf, 64, 100);
				if (buf[0] == 0x31)
					Gamepad.USBConnection = false;

			} else if (cur_dev->product_id == SONY_DS4_USB || cur_dev->product_id == SONY_DS4_V2_USB || cur_dev->product_id == SONY_DS4_DONGLE) {
				Gamepad.ControllerType = SONY_DUALSHOCK4;
				Gamepad.USBConnection = true;

				// JoyShock Library apparently sent something, so it worked without a package (needed for BT detection to work, does not affect USB)
				unsigned char checkBT[2] = { 0x02, 0x00 };
				hid_write(Gamepad.HidHandle, checkBT, sizeof(checkBT));

				// BT detection for compatible gamepads that output USB VID/PID on BT connection
				unsigned char buf[64];
				memset(buf, 0, sizeof(buf));
				int bytesRead = hid_read_timeout(Gamepad.HidHandle, buf, sizeof(buf), 100);
				if (bytesRead > 0 && buf[0] == 0x11)
					Gamepad.USBConnection = false;

				//printf("Detected device ID: 0x%X\n", cur_dev->product_id);
				//if (Gamepad.USBConnection) printf("USB"); else printf("Wireless");

			} else if (cur_dev->product_id == SONY_DS4_BT) { // ?
				Gamepad.ControllerType = SONY_DUALSHOCK4;
				Gamepad.USBConnection = false;
			}
			break;
		}
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);

	// Sony compatible controllers
	devs = hid_enumerate(BROOK_DS4_VENDOR, 0x0);
	cur_dev = devs;
	while (cur_dev) {
		if (!SkipDevPath.empty() && SkipDevPath == cur_dev->path) { cur_dev = cur_dev->next; continue; }
		if (cur_dev->product_id == BROOK_DS4_USB)
		{
			Gamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			if (Gamepad.HidHandle == NULL) { cur_dev = cur_dev->next; continue; }
			hid_set_nonblocking(Gamepad.HidHandle, 1);
			Gamepad.USBConnection = true;
			Gamepad.ControllerType = SONY_DUALSHOCK4;
			break;
		}
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);

	// Nintendo compatible controllers
	devs = hid_enumerate(NINTENDO_VENDOR, 0x0);
	cur_dev = devs;
	while (cur_dev) {
		if ((!SkipDevPath.empty() && SkipDevPath == cur_dev->path) || (!SkipDevPath2.empty() && SkipDevPath2 == cur_dev->path)) { cur_dev = cur_dev->next; continue; }
		if (cur_dev->product_id == NINTENDO_JOYCON_L)
		{
			if (Gamepad.HidHandle != NULL) { cur_dev = cur_dev->next; continue; }
			Gamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			if (Gamepad.HidHandle == NULL) { cur_dev = cur_dev->next; continue; }
			Gamepad.DevicePath = cur_dev->path;
			hid_set_nonblocking(Gamepad.HidHandle, 1);
			Gamepad.USBConnection = false;
			Gamepad.ControllerType = NINTENDO_JOYCONS;
		}
		else if (cur_dev->product_id == NINTENDO_JOYCON_R) {
			if (Gamepad.HidHandle != NULL && Gamepad.ControllerType != NINTENDO_JOYCONS) { cur_dev = cur_dev->next; continue; }
			Gamepad.HidHandle2 = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			if (Gamepad.HidHandle2 == NULL) { cur_dev = cur_dev->next; continue; }
			Gamepad.DevicePath2 = cur_dev->path;
			hid_set_nonblocking(Gamepad.HidHandle2, 1);
			Gamepad.USBConnection = false;
			Gamepad.ControllerType = NINTENDO_JOYCONS;
		}
		else if (cur_dev->product_id == NINTENDO_SWITCH_PRO) {
			if (Gamepad.HidHandle != NULL) { cur_dev = cur_dev->next; continue; }
			Gamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			if (Gamepad.HidHandle == NULL) { cur_dev = cur_dev->next; continue; }
			Gamepad.DevicePath = cur_dev->path;
			Gamepad.ControllerType = NINTENDO_SWITCH_PRO;
			hid_set_nonblocking(Gamepad.HidHandle, 1);
			//Gamepad.USBConnection = true;
			Gamepad.RumbleSkipCounter = 300;

			// Conflict with JoyShock Library ???
			unsigned char buf[64] = {};
			buf[0] = 0x80;
			buf[1] = 0x01;

			int written = hid_write(Gamepad.HidHandle, buf, 2);
			if (written > 0) {
				Gamepad.USBConnection = true;
				/*unsigned char buf[64];
				memset(buf, 0, sizeof(buf));
				int bytesRead = hid_read_timeout(Gamepad.HidHandle, buf, sizeof(buf), 100);
				if (bytesRead > 0 && (buf[0] == 0x81 || buf[0] == 0x21 || buf[0] == 0x30))
					Gamepad.USBConnection = true;*/
			} else
				Gamepad.USBConnection = false;

			//Gamepad.USBConnection = (cur_dev->serial_number != NULL);
		}
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);

	//printf("\nFound: %d\n", Gamepad.ControllerType);
}

static float g_MaxStillnessError = 2.0f;	//@062 
static float g_MinStillnessCollectionTime = 0.5f;
static float g_MinStillnessCorrectionTime = 2.0f;
static float g_StillnessCalibrationEaseInTime = 3.0f;
static float g_GravityShakinessMin = 0.01f;
static float g_GravityShakinessMax = 0.4f;
static float g_GravityStillSpeed = 1.0f;
static float g_GravityShakySpeed = 0.1f;

static std::string g_KillProcessName = "";		//@066
static int g_KillProcessHotkey = 0;

/*static float MotorFreqFromStrength(unsigned char motorValue) { // Dynamic frequency	//old Nintendo rumble code
	if (motorValue == 0) return 40.0f;
	return 40.0f + (motorValue / 255.0f) * (320.0f - 40.0f);
}

static void EncodeRumble(unsigned char* data, float freq, float amp) {
	if (freq < 41.0f) freq = 41.0f;
	if (freq > 1253.0f) freq = 1253.0f;
	if (amp < 0.0f) amp = 0.0f;
	if (amp > 1.0f) amp = 1.0f;

	uint16_t hf = (uint16_t)(320.0f * log2f(freq * 0.1f) + 0.5f);
	uint8_t hf_byte = (hf - (hf % 4)) / 4;
	uint8_t lf_byte = (uint8_t)((freq * 0.1f) / powf(2.0f, (hf_byte - 0x60) / 32.0f));

	uint16_t amp_enc = (uint16_t)(amp * 0x7FFF);
	uint8_t amp_hi = (amp_enc >> 8) & 0xFF;
	uint8_t amp_lo = amp_enc & 0xFF;

	data[0] = lf_byte;
	data[1] = hf_byte;
	data[2] = amp_lo;
	data[3] = amp_hi;
}*/

//@070 New Nintendo Rumble SDL3 code. Логарифмические таблицы амплитуд по спецификации Nintendo Switch
static const uint16_t JoyCon_HFA_Table[101][2] = {
	{0, 0x0},{514, 0x2},{775, 0x4},{921, 0x6},{1096, 0x8},{1303, 0x0a},{1550, 0x0c},
	{1843, 0x0e},{2192, 0x10},{2606, 0x12},{3100, 0x14},{3686, 0x16},{4383, 0x18},{5213, 0x1a},
	{6199, 0x1c},{7372, 0x1e},{7698, 0x20},{8039, 0x22},{8395, 0x24},{8767, 0x26},{9155, 0x28},
	{9560, 0x2a},{9984, 0x2c},{10426, 0x2e},{10887, 0x30},{11369, 0x32},{11873, 0x34},{12398, 0x36},
	{12947, 0x38},{13520, 0x3a},{14119, 0x3c},{14744, 0x3e},{15067, 0x40},{15397, 0x42},{15734, 0x44},
	{16079, 0x46},{16431, 0x48},{16790, 0x4a},{17158, 0x4c},{17534, 0x4e},{17918, 0x50},{18310, 0x52},
	{18711, 0x54},{19121, 0x56},{19540, 0x58},{19968, 0x5a},{20405, 0x5c},{20852, 0x5e},{21308, 0x60},
	{21775, 0x62},{22251, 0x64},{22739, 0x66},{23236, 0x68},{23745, 0x6a},{24265, 0x6c},{24797, 0x6e},
	{25339, 0x70},{25894, 0x72},{26460, 0x74},{27039, 0x76},{27630, 0x78},{28235, 0x7a},{28853, 0x7c},
	{29484, 0x7e},{30129, 0x80},{30789, 0x82},{31462, 0x84},{32151, 0x86},{32854, 0x88},{33573, 0x8a},
	{34307, 0x8c},{35058, 0x8e},{35825, 0x90},{36609, 0x92},{37422, 0x94},{38242, 0x96},{39079, 0x98},
	{39935, 0x9a},{40809, 0x9c},{41703, 0x9e},{42616, 0xa0},{43549, 0xa2},{44503, 0xa4},{45477, 0xa6},
	{46473, 0xa8},{47491, 0xaa},{48531, 0xac},{49593, 0xae},{50679, 0xb0},{51789, 0xb2},{52923, 0xb4},
	{54082, 0xb6},{55266, 0xb8},{56476, 0xba},{57713, 0xbc},{58977, 0xbe},{60268, 0xc0},{61588, 0xc2},
	{62936, 0xc4},{64315, 0xc6},{65535, 0xc8}
};

static const uint16_t JoyCon_LFA_Table[101][2] = {
	{0, 0x0040},{514, 0x8040},{775, 0x0041},{921, 0x8041},{1096, 0x0042},{1303, 0x8042},{1550, 0x0043},
	{1843, 0x8043},{2192, 0x0044},{2606, 0x8044},{3100, 0x0045},{3686, 0x8045},{4383, 0x0046},{5213, 0x8046},
	{6199, 0x0047},{7372, 0x8047},{7698, 0x0048},{8039, 0x8048},{8395, 0x0049},{8767, 0x8049},{9155, 0x004a},
	{9560, 0x804a},{9984, 0x004b},{10426, 0x804b},{10887, 0x004c},{11369, 0x804c},{11873, 0x004d},{12398, 0x804d},
	{12947, 0x004e},{13520, 0x804e},{14119, 0x004f},{14744, 0x804f},{15067, 0x0050},{15397, 0x8050},{15734, 0x0051},
	{16079, 0x8051},{16431, 0x0052},{16790, 0x8052},{17158, 0x0053},{17534, 0x8053},{17918, 0x0054},{18310, 0x8054},
	{18711, 0x0055},{19121, 0x8055},{19540, 0x0056},{19968, 0x8056},{20405, 0x0057},{20852, 0x8057},{21308, 0x0058},
	{21775, 0x8058},{22251, 0x0059},{22739, 0x8059},{23236, 0x005a},{23745, 0x805a},{24265, 0x005b},{24797, 0x805b},
	{25339, 0x005c},{25894, 0x805c},{26460, 0x005d},{27039, 0x805d},{27630, 0x005e},{28235, 0x805e},{28853, 0x005f},
	{29484, 0x805f},{30129, 0x0060},{30789, 0x8060},{31462, 0x0061},{32151, 0x8061},{32854, 0x0062},{33573, 0x8062},
	{34307, 0x0063},{35058, 0x8063},{35825, 0x0064},{36609, 0x8064},{37422, 0x0065},{38242, 0x8065},{39079, 0x0066},
	{39935, 0x8066},{40809, 0x0067},{41703, 0x8067},{42616, 0x0068},{43549, 0x8068},{44503, 0x0069},{45477, 0x8069},
	{46473, 0x006a},{47491, 0x806a},{48531, 0x006b},{49593, 0x806b},{50679, 0x006c},{51789, 0x806c},{52923, 0x006d},
	{54082, 0x806d},{55266, 0x006e},{56476, 0x806e},{57713, 0x006f},{58977, 0x806f},{60268, 0x0070},{61588, 0x8070},
	{62936, 0x0071},{64315, 0x8071},{65535, 0x0072}
};

static uint8_t GetJoyConHFAmp(uint16_t amp) {
	for (int i = 0; i < 101; i++) {
		if (amp <= JoyCon_HFA_Table[i][0]) return (uint8_t)JoyCon_HFA_Table[i][1];
	}
	return (uint8_t)JoyCon_HFA_Table[100][1];
}

static uint16_t GetJoyConLFAmp(uint16_t amp) {
	for (int i = 0; i < 101; i++) {
		if (amp <= JoyCon_LFA_Table[i][0]) return JoyCon_LFA_Table[i][1];
	}
	return JoyCon_LFA_Table[100][1];
}

// Правильная аппаратная упаковка 4 байт HD Rumble
static void EncodeRumbleModern(unsigned char* data, uint16_t hf, uint8_t hf_amp, uint8_t lf, uint16_t lf_amp) {
	if (hf_amp > 0 || lf_amp > 0x0040) {
		data[0] = (uint8_t)(hf & 0xFF);
		data[1] = (uint8_t)(hf_amp + ((hf >> 8) & 0xFF));
		data[2] = (uint8_t)(lf + ((lf_amp >> 8) & 0xFF));
		data[3] = (uint8_t)(lf_amp & 0xFF);
	}
	else {
		// Нейтральный пакет тишины
		data[0] = 0x00;
		data[1] = 0x01;
		data[2] = 0x40;
		data[3] = 0x40;
	}
}

// https://github.com/fossephate/JoyCon-Driver/blob/main/joycon-driver/include/Joycon.hpp
/*void JoyConSimpleRumble(hid_device* jcHandle, bool IsLeft, unsigned char MotorValue)	//old Nintendo rumble code
{
	unsigned char outputReport[64] = { 0 };

	outputReport[0] = 0x10;
	//outputReport[1] = PrimaryGamepad.PacketCounter++ & 0x0f;
	outputReport[1] = (IsLeft ? PrimaryGamepad.PacketCounter++ : PrimaryGamepad.PacketCounter2++) & 0x0f;	//@019 RumbleFix. Левый-PacketCounter, правый-PacketCounter2 из .h

	if (IsLeft) {
		if (MotorValue == 0) {
			outputReport[2] = 0x00;
			outputReport[3] = 0x01;
			outputReport[4] = 0x40;
			outputReport[5] = 0x40;
		}
		else
			EncodeRumble(&outputReport[2], MotorFreqFromStrength(MotorValue), (MotorValue * PrimaryGamepad.RumbleStrength * 0.9f) / 25500.0f); // It seems that values above 90% may cause wear on the motors of Nintendo controllers.
	}
	else { // Is right
		if (MotorValue == 0) {
			outputReport[6] = 0x00;
			outputReport[7] = 0x01;
			outputReport[8] = 0x40;
			outputReport[9] = 0x40;
		}
		else
			EncodeRumble(&outputReport[6], MotorFreqFromStrength(MotorValue), (MotorValue * PrimaryGamepad.RumbleStrength * 0.9f) / 25500.0f);
	}

	hid_write(jcHandle, outputReport, sizeof(outputReport));
}*/

/*void JoyConSimpleRumble(hid_device* jcHandle, bool IsLeft, unsigned char MotorValue) //fixed old N Rumble code
{
	unsigned char outputReport[64] = { 0 };

	outputReport[0] = 0x10;
	outputReport[1] = (IsLeft ? PrimaryGamepad.PacketCounter++ : PrimaryGamepad.PacketCounter2++) & 0x0f;	//@019 RumbleFix

	// ЖЕСТКО устанавливаем нейтральную вибрацию для ОБЕИХ сторон по умолчанию (защита от щелчков/игнора)
	outputReport[2] = 0x00; outputReport[3] = 0x01; outputReport[4] = 0x40; outputReport[5] = 0x40;
	outputReport[6] = 0x00; outputReport[7] = 0x01; outputReport[8] = 0x40; outputReport[9] = 0x40;

	// Если сигнал есть, перезаписываем только активную сторону
	if (MotorValue > 0) {
		if (IsLeft) {
			EncodeRumble(&outputReport[2], MotorFreqFromStrength(MotorValue), (MotorValue * PrimaryGamepad.RumbleStrength * 0.9f) / 25500.0f);
		}
		else { // Is right
			EncodeRumble(&outputReport[6], MotorFreqFromStrength(MotorValue), (MotorValue * PrimaryGamepad.RumbleStrength * 0.9f) / 25500.0f);
		}
	}

	hid_write(jcHandle, outputReport, 64);
}*/

void JoyConSimpleRumble(hid_device* jcHandle, bool IsLeft, unsigned char largeMotor, unsigned char smallMotor)	//@070
{
	unsigned char outputReport[64] = { 0 };

	outputReport[0] = 0x10;
	outputReport[1] = (IsLeft ? PrimaryGamepad.PacketCounter++ : PrimaryGamepad.PacketCounter2++) & 0x0f;

	// Резонансные частоты Nintendo Switch: 
	// Низкая (тяжелый бас) = 160 Гц (0x3D), Высокая (звонкий треск) = 320 Гц (0x0074)
	const uint16_t k_usHighFreq = 0x0074;
	const uint8_t  k_ucLowFreq = 0x3D;

	uint8_t  hf_amp = 0;
	uint16_t lf_amp = 0x0040; // 0x0040 означает нулевую громкость для LF

	// Если вибрация включена в настройках (>0%) и моторы активны
	if (PrimaryGamepad.RumbleStrength > 0 && (largeMotor > 0 || smallMotor > 0)) {
		// Переводим 0..255 с учетом ползунка RumbleStrength (0..100) в диапазон 0..65535
		uint16_t low_val = (uint16_t)(((uint32_t)largeMotor * PrimaryGamepad.RumbleStrength * 65535) / 25500);
		uint16_t high_val = (uint16_t)(((uint32_t)smallMotor * PrimaryGamepad.RumbleStrength * 65535) / 25500);

		lf_amp = GetJoyConLFAmp(low_val);
		hf_amp = GetJoyConHFAmp(high_val);
	}

	// Записываем пакет вибрации сразу в ОБА слота (байт 2..5 и байт 6..9).
	// Это гарантирует 100% совместимость с любыми китайскими Joy-Con (включая Mobapad),
	// независимо от того, какой слот ожидает прошивка контроллера!
	EncodeRumbleModern(&outputReport[2], k_usHighFreq, hf_amp, k_ucLowFreq, lf_amp);
	EncodeRumbleModern(&outputReport[6], k_usHighFreq, hf_amp, k_ucLowFreq, lf_amp);

	hid_write(jcHandle, outputReport, 64);
}

void GamepadSetState(AdvancedGamepad &Gamepad)
{
	if (Gamepad.HidHandle == NULL && Gamepad.HidHandle2 == NULL) return;
	if (Gamepad.ControllerType == SONY_DUALSENSE) { // https://www.reddit.com/r/gamedev/comments/jumvi5/dualsense_haptics_leds_and_more_hid_output_report/

		unsigned char PlayersDSPacket = 0;

		if (Gamepad.OutState.PlayersCount == 0) PlayersDSPacket = 0;
		else if (Gamepad.OutState.PlayersCount == 1) PlayersDSPacket = 4;
		else if (Gamepad.OutState.PlayersCount == 2) PlayersDSPacket = 2; // Center 2
		else if (Gamepad.OutState.PlayersCount == 5) PlayersDSPacket = 1; // Both 2
		else if (Gamepad.OutState.PlayersCount == 3) PlayersDSPacket = 5;
		else if (Gamepad.OutState.PlayersCount == 4) PlayersDSPacket = 3;

		Gamepad.OutState.LEDRed = (Gamepad.OutState.LEDColor >> 16) & 0xFF;
		Gamepad.OutState.LEDGreen = (Gamepad.OutState.LEDColor >> 8) & 0xFF;
		Gamepad.OutState.LEDBlue = Gamepad.OutState.LEDColor & 0xFF;

		if (Gamepad.USBConnection) {
			unsigned char outputReport[48];
			memset(outputReport, 0, 48);

			outputReport[0] = 0x02;
			outputReport[1] = 0xff;
			outputReport[2] = 0x15;
			outputReport[3] = (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 100;
			outputReport[4] = (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 100;
			outputReport[5] = 0xff;
			outputReport[6] = 0xff;
			outputReport[7] = 0xff;
			outputReport[8] = 0x0c;
			//outputReport[9] = OutState.MicLED;
			outputReport[38] = 0x07;
			outputReport[44] = PlayersDSPacket;
			outputReport[45] = std::clamp(Gamepad.OutState.LEDRed - Gamepad.OutState.LEDBrightness, 0, 255);
			outputReport[46] = std::clamp(Gamepad.OutState.LEDGreen - Gamepad.OutState.LEDBrightness, 0, 255);
			outputReport[47] = std::clamp(Gamepad.OutState.LEDBlue - Gamepad.OutState.LEDBrightness, 0, 255);

			// Adaptive triggers
			if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_RUMBLE_MODE) // Rumble translation
			{
				// Left trigger
				outputReport[21] = 0x06;   // Continuous Resistance
				outputReport[22] = (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 2300; // 2300 - softer
				outputReport[23] = 0x09;   // Начало триггера (почти с нуля)
				outputReport[24] = 0xFF;   // Конец триггера (100%)
				outputReport[25] = 0x00;   // Без вибрации

				// Right trigger
				outputReport[11] = 0x06;      // Pulse mode
				outputReport[12] = (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 2300; // 2300 - softer
				outputReport[13] = 3;          // старт чуть позже — не так резко
				outputReport[14] = 8;          // короткая серия импульсов
				outputReport[15] = 0x18;       // частота импульсов ниже — мягкая отдача
			}
			else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_PISTOL_MODE) // Pistol / Пистолет
			{
				// Пистолет: плавное сопротивление по всему ходу
				outputReport[11] = 0x02;   // Continuous Resistance
				outputReport[12] = 35;     // Средняя сила сопротивления
				outputReport[13] = 0x09;   // Начало триггера (почти с нуля)
				outputReport[14] = 0xFF;   // Конец триггера (100%)
				outputReport[15] = 0x00;   // Без вибрации

			}
			else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_AUTOMATIC_MODE) // Automatic / Machine Gun — серия коротких импульсов
			{
				outputReport[11] = 0x06;   // Pulse mode
				outputReport[12] = 15;     // Сила каждого импульса (легкая)
				outputReport[13] = 2;      // Старт почти сразу при лёгком нажатии
				outputReport[14] = 10;     // Конец короткой серии импульсов
				outputReport[15] = 0x20;   // Частота импульсов (выше — имитация очереди)

			}
			else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_RIFLE_MODE) // Sniper Rifle — винтовка с усилием
			{
				/* Более легкий вариант
				outputReport[11] = 0x26;   // Resistance + лёгкая вибрация
				outputReport[12] = 120;    // более сильное сопротивление
				outputReport[13] = 0x00;   // начало
				outputReport[14] = 0xE0;   // конец почти полной
				outputReport[15] = 0x05;   // частота вибрации
				outputReport[16] = 0xF0;   // короткий резкий толчок
				outputReport[17] = 0x40;   // сила толчка — ощущается реально*/

				outputReport[11] = 0x25;
				outputReport[12] = 0x04; // low (1<<2)
				outputReport[13] = 0x01; // high(1<<8)
				outputReport[14] = 0x06; // strength-1 (7-1)
				outputReport[15] = 0x00;
				outputReport[16] = 0x00;
				outputReport[17] = 0x00;
				outputReport[18] = 0x00;
				outputReport[19] = 0x00;
				outputReport[20] = 0x00;
				outputReport[21] = 0x00;

			}
			else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_BOW_MODE) // Лук — прогрессивное натяжение
			{
				outputReport[11] = 0x22;
				outputReport[12] = 0x01; // low (1<<0)
				outputReport[13] = 0x01; // high(1<<8)
				outputReport[14] = 0x33; // (strength-1) | ((snap-1)<<3) => (4-1)=3, (7-1)=6 => 0x03 | (0x06<<3)=0x33
				outputReport[15] = 0x00;
				outputReport[16] = 0x00;
				outputReport[17] = 0x00;
				outputReport[18] = 0x00;
				outputReport[19] = 0x00;
				outputReport[20] = 0x00;
				outputReport[21] = 0x00;

			}
			else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_CAR_MODE) // Педаль авто
			{
				/* Слишком сильное
				outputReport[11] = 0x02;   // Continuous resistance
				outputReport[12] = 0x10;   // слабое в начале
				outputReport[13] = 0xFF;   // конец хода
				outputReport[14] = 0x20;   // начальная сила
				outputReport[15] = 0xF0;   // максимальная сила — реально чувствуется
				*/

				outputReport[11] = 0x21;
				outputReport[12] = 0xFF; // активные зоны 0..9
				outputReport[13] = 0x03;
				outputReport[14] = 0x24; // amplitude zones для strength=5 (повтор "100")
				outputReport[15] = 0x92;
				outputReport[16] = 0x49;
				outputReport[17] = 0x24;
				outputReport[18] = 0x00;
				outputReport[19] = 0x00;
				outputReport[20] = 0x00;
				outputReport[21] = 0x00;

			}

			// Left trigger
			if (Gamepad.AdaptiveTriggersOutputMode > 1)
			{
				outputReport[21] = 0x02;   // Continuous Resistance
				outputReport[22] = 35;     // Средняя сила сопротивления
				outputReport[23] = 0x09;   // Начало триггера (почти с нуля)
				outputReport[24] = 0xFF;   // Конец триггера (100%)
				outputReport[25] = 0x00;   // Без вибрации

			}

			if (Gamepad.HidHandle != NULL)
				hid_write(Gamepad.HidHandle, outputReport, 48);
	
		}
		// DualSense BT
		else {
			unsigned char outputReport[79]; // https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/JoyShock.cpp (set_ds5_rumble_light_bt)
			memset(outputReport, 0, 79);

			outputReport[0] = 0xa2;
			outputReport[1] = 0x31;
			outputReport[2] = 0x02;
			outputReport[3] = 0x03;
			outputReport[4] = 0x54;
			outputReport[5] = (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 100;
			outputReport[6] = (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 100;
			outputReport[11] = 0x00; // Gamepad.OutState.MicLED - not working
			outputReport[41] = 0x02;
			outputReport[44] = 0x02;
			outputReport[45] = 0x02;
			outputReport[46] = PlayersDSPacket;
			//outputReport[46] &= ~(1 << 7);
			//outputReport[46] &= ~(1 << 8);
			outputReport[47] = std::clamp(Gamepad.OutState.LEDRed - Gamepad.OutState.LEDBrightness, 0, 255);
			outputReport[48] = std::clamp(Gamepad.OutState.LEDGreen - Gamepad.OutState.LEDBrightness, 0, 255);
			outputReport[49] = std::clamp(Gamepad.OutState.LEDBlue - Gamepad.OutState.LEDBrightness, 0, 255);

			// https://github.com/Valkirie/JoyShockLibrary/commit/f4fffb6faa53f0839130b093690ca292f23f115e
			if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_RUMBLE_MODE) // Rumble translation
			{
				// Left trigger (USB: 21..25) -> BT: 24..28
				outputReport[3] |= 0x08;              // dirty L2
				outputReport[24] = 0x06;              // Continuous Resistance
				outputReport[25] = (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 2300;
				outputReport[26] = 0x09;              // начало
				outputReport[27] = 0xFF;              // конец
				outputReport[28] = 0x00;              // без вибрации

				// Right trigger (USB: 11..15) -> BT: 13..17
				outputReport[3] |= 0x04;              // dirty R2
				outputReport[13] = 0x06;              // Pulse mode
				outputReport[14] = (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 2300;
				outputReport[15] = 3;                 // старт чуть позже
				outputReport[16] = 8;                 // короткая серия
				outputReport[17] = 0x18;              // частота
			}
			else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_PISTOL_MODE) // Пистолет
			{
				// Только правый, как по USB
				outputReport[3] |= 0x04;              // dirty R2
				outputReport[13] = 0x02;              // Continuous Resistance
				outputReport[14] = 35;                // сила
				outputReport[15] = 0x09;              // начало
				outputReport[16] = 0xFF;              // конец
				outputReport[17] = 0x00;              // без вибрации
			}
			else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_AUTOMATIC_MODE) // Automatic / очередь
			{
				outputReport[3] |= 0x04;              // dirty R2
				outputReport[13] = 0x06;              // Pulse mode
				outputReport[14] = 15;                // сила импульса
				outputReport[15] = 2;                 // старт
				outputReport[16] = 10;                // конец серии
				outputReport[17] = 0x20;              // частота
			}
			else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_RIFLE_MODE) // Винтовка
			{
				outputReport[3] |= 0x04;              // dirty R2
				outputReport[13] = 0x25;
				outputReport[14] = 0x04;
				outputReport[15] = 0x01;
				outputReport[16] = 0x06;
				outputReport[17] = 0x00;
				outputReport[18] = 0x00;
				outputReport[19] = 0x00;
				outputReport[20] = 0x00;
				outputReport[21] = 0x00;
				outputReport[22] = 0x00;
				outputReport[23] = 0x00;
			}
			else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_BOW_MODE) // Лук
			{
				outputReport[3] |= 0x04;              // dirty R2
				outputReport[13] = 0x22;
				outputReport[14] = 0x01;              // low
				outputReport[15] = 0x01;              // high
				outputReport[16] = 0x33;              // как по USB
				outputReport[17] = 0x00;
				outputReport[18] = 0x00;
				outputReport[19] = 0x00;
				outputReport[20] = 0x00;
				outputReport[21] = 0x00;
				outputReport[22] = 0x00;
				outputReport[23] = 0x00;
			}
			else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_CAR_MODE) // Педаль авто
			{
				outputReport[3] |= 0x04;              // dirty R2
				outputReport[13] = 0x21;
				outputReport[14] = 0xFF;              // активные зоны 0..9
				outputReport[15] = 0x03;
				outputReport[16] = 0x24;              // amplitude zones
				outputReport[17] = 0x92;
				outputReport[18] = 0x49;
				outputReport[19] = 0x24;
				outputReport[20] = 0x00;
				outputReport[21] = 0x00;
				outputReport[22] = 0x00;
				outputReport[23] = 0x00;
			}
			else
			{
				// режим 0 / неизвестный — сброс обоих триггеров
				outputReport[3] |= 0x0C;              // dirty R2+L2
			}

			// Left trigger
			if (Gamepad.AdaptiveTriggersOutputMode > 1)
			{
				outputReport[3] |= 0x08;              // dirty L2
				outputReport[24] = 0x02;              // Continuous Resistance
				outputReport[25] = 35;                // Средняя сила
				outputReport[26] = 0x09;              // Начало
				outputReport[27] = 0xFF;              // Конец
				outputReport[28] = 0x00;              // Без вибрации
			}

			uint32_t crc = crc_32(outputReport, 75);
			memcpy(&outputReport[75], &crc, 4);

			if (Gamepad.HidHandle != NULL)
				hid_write(Gamepad.HidHandle, &outputReport[1], 78);
		}

	}
	else if (Gamepad.ControllerType == SONY_DUALSHOCK4) { // JoyShockLibrary rumble working for USB DS4 ??? 
		Gamepad.OutState.LEDRed = (Gamepad.OutState.LEDColor >> 16) & 0xFF;
		Gamepad.OutState.LEDGreen = (Gamepad.OutState.LEDColor >> 8) & 0xFF;
		Gamepad.OutState.LEDBlue = Gamepad.OutState.LEDColor & 0xFF;

		if (Gamepad.USBConnection) {
			unsigned char outputReport[31];
			memset(outputReport, 0, 31);

			outputReport[0] = 0x05;
			outputReport[1] = 0xff;
			outputReport[4] = (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 100;
			outputReport[5] = (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 100;
			outputReport[6] = std::clamp(Gamepad.OutState.LEDRed - Gamepad.OutState.LEDBrightness, 0, 255);
			outputReport[7] = std::clamp(Gamepad.OutState.LEDGreen - Gamepad.OutState.LEDBrightness, 0, 255);
			outputReport[8] = std::clamp(Gamepad.OutState.LEDBlue - Gamepad.OutState.LEDBrightness, 0, 255);

			if (Gamepad.HidHandle != NULL)
				hid_write(Gamepad.HidHandle, outputReport, 31);

			// DualShock 4 BT
		}
		else { // https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/JoyShock.cpp (set_ds4_rumble_light_bt)
			unsigned char outputReport[79];
			memset(outputReport, 0, 79);

			outputReport[0] = 0xa2;
			outputReport[1] = 0x11;
			outputReport[2] = 0xc0;
			outputReport[3] = 0x20;
			outputReport[4] = 0x07;
			outputReport[5] = 0x00;
			outputReport[6] = 0x00;

			outputReport[7] = (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 100;
			outputReport[8] = (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 100;

			outputReport[9] = std::clamp(Gamepad.OutState.LEDRed - Gamepad.OutState.LEDBrightness, 0, 255);
			outputReport[10] = std::clamp(Gamepad.OutState.LEDGreen - Gamepad.OutState.LEDBrightness, 0, 255);
			outputReport[11] = std::clamp(Gamepad.OutState.LEDBlue - Gamepad.OutState.LEDBrightness, 0, 255);

			outputReport[12] = 0xff;
			outputReport[13] = 0x00;

			uint32_t crc = crc_32(outputReport, 75);
			memcpy(&outputReport[75], &crc, 4);

			if (Gamepad.HidHandle != NULL)
				hid_write(Gamepad.HidHandle, &outputReport[1], 78);
		}
	}
	/*else if (Gamepad.ControllerType == NINTENDO_JOYCONS && !Gamepad.USBConnection) {	//old Nintendo rumble code
		
		if (Gamepad.RumbleStrength != 0) {
			if (Gamepad.HidHandle != NULL)	//@019 RubleFix выбирать самый сильный сигнал, а не делить пополам
				//JoyConSimpleRumble(Gamepad.HidHandle, true, AppStatus.JoyconRumbleMerge == false ? Gamepad.OutState.LargeMotor : (Gamepad.OutState.LargeMotor + Gamepad.OutState.SmallMotor) / 2);			
				JoyConSimpleRumble(Gamepad.HidHandle, true, AppStatus.JoyconRumbleMerge == false ? Gamepad.OutState.LargeMotor : (Gamepad.OutState.LargeMotor > Gamepad.OutState.SmallMotor ? Gamepad.OutState.LargeMotor : Gamepad.OutState.SmallMotor));
			if (Gamepad.HidHandle2)
				//JoyConSimpleRumble(Gamepad.HidHandle2, false, AppStatus.JoyconRumbleMerge == false ? Gamepad.OutState.SmallMotor : (Gamepad.OutState.LargeMotor + Gamepad.OutState.SmallMotor) / 2);			
				JoyConSimpleRumble(Gamepad.HidHandle2, false, AppStatus.JoyconRumbleMerge == false ? Gamepad.OutState.SmallMotor : (Gamepad.OutState.LargeMotor > Gamepad.OutState.SmallMotor ? Gamepad.OutState.LargeMotor : Gamepad.OutState.SmallMotor));
		}
	}*/
	else if (Gamepad.ControllerType == NINTENDO_JOYCONS && !Gamepad.USBConnection) {	//@070
		if (Gamepad.RumbleStrength != 0) {
			// Если включен Merge — оба джойкона получают оба мотора (тяжелый и легкий).
			// Если выключен — Левый джойкон басит (LargeMotor), Правый трещит (SmallMotor), как на Xbox!
			unsigned char leftLM  = Gamepad.OutState.LargeMotor;
			unsigned char leftSM  = AppStatus.JoyconRumbleMerge ? Gamepad.OutState.SmallMotor : 0;

			unsigned char rightLM = AppStatus.JoyconRumbleMerge ? Gamepad.OutState.LargeMotor : 0;
			unsigned char rightSM = Gamepad.OutState.SmallMotor;

			if (Gamepad.HidHandle != NULL)
				JoyConSimpleRumble(Gamepad.HidHandle, true, leftLM, leftSM);

			if (Gamepad.HidHandle2 != NULL)
				JoyConSimpleRumble(Gamepad.HidHandle2, false, rightLM, rightSM);
		}
	}
	/*else if (Gamepad.ControllerType == NINTENDO_SWITCH_PRO) { // && !Gamepad.USBConnection
		//printf("rumble\n");
		//JslSetRumble(0, (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 100, (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 100);
		if (Gamepad.RumbleStrength != 0) {
			if (!Gamepad.USBConnection || Gamepad.RumbleSkipCounter == 0) { // Wireless or wired with skip JoyShockLibrary init
				unsigned char outputReport[64] = { 0 };
				outputReport[0] = 0x10;
				outputReport[1] = Gamepad.PacketCounter++ & 0x0f;
				EncodeRumble(&outputReport[2], MotorFreqFromStrength(Gamepad.OutState.SmallMotor), (Gamepad.OutState.SmallMotor / 255.0f) * (Gamepad.RumbleStrength / 100.0f));
				EncodeRumble(&outputReport[6], MotorFreqFromStrength(Gamepad.OutState.LargeMotor), (Gamepad.OutState.LargeMotor / 255.0f) * (Gamepad.RumbleStrength / 100.0f));
				if (Gamepad.HidHandle != NULL)
					hid_write(Gamepad.HidHandle, outputReport, 64);
			}
		}
	}*/
	else if (Gamepad.ControllerType == NINTENDO_SWITCH_PRO) { // && !Gamepad.USBConnection
		if (Gamepad.RumbleStrength != 0) {
			if (!Gamepad.USBConnection || Gamepad.RumbleSkipCounter == 0) { // Wireless or wired with skip JoyShockLibrary init
				unsigned char outputReport[64] = { 0 };
				outputReport[0] = 0x10;
				outputReport[1] = Gamepad.PacketCounter++ & 0x0f;

				const uint16_t k_usHighFreq = 0x0074; // 320 Hz
				const uint8_t  k_ucLowFreq  = 0x3D;   // 160 Hz

				// Масштабируем значения моторов под ползунок RumbleStrength (0..65535)
				uint16_t low_val  = (uint16_t)(((uint32_t)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength * 65535) / 25500);
				uint16_t high_val = (uint16_t)(((uint32_t)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength * 65535) / 25500);

				uint16_t lf_amp = (Gamepad.OutState.LargeMotor > 0) ? GetJoyConLFAmp(low_val)  : 0x0040;
				uint8_t  hf_amp = (Gamepad.OutState.SmallMotor > 0) ? GetJoyConHFAmp(high_val) : 0;

				// Стандарт Xbox:
				// Левый грип (байт 2..5)  = LargeMotor (тяжелый бас)
				// Правый грип (байт 6..9) = SmallMotor (высокие частоты)
				// Если включен JoyconRumbleMerge — оба грипа вибрируют обоими моторами на полную мощность
				uint8_t  left_hf_amp  = AppStatus.JoyconRumbleMerge ? hf_amp : 0;
				uint16_t left_lf_amp  = lf_amp;

				uint8_t  right_hf_amp = hf_amp;
				uint16_t right_lf_amp = AppStatus.JoyconRumbleMerge ? lf_amp : 0x0040;

				// Упаковываем Левый и Правый грип
				EncodeRumbleModern(&outputReport[2], k_usHighFreq, left_hf_amp, k_ucLowFreq, left_lf_amp);
				EncodeRumbleModern(&outputReport[6], k_usHighFreq, right_hf_amp, k_ucLowFreq, right_lf_amp);

				if (Gamepad.HidHandle != NULL)
					hid_write(Gamepad.HidHandle, outputReport, 64);
			}
		}
	}
}

void UpdateBatteryInfo(AdvancedGamepad &Gamepad) {
	if (Gamepad.HidHandle != NULL || Gamepad.HidHandle2 != NULL) {	//@018 add 2nd joycon
		if (Gamepad.ControllerType == SONY_DUALSENSE) {
			unsigned char buf[64];
			memset(buf, 0, 64);
			hid_read(Gamepad.HidHandle, buf, 64);
			if (Gamepad.USBConnection) {
				Gamepad.LEDBatteryLevel = (buf[53] & 0x0f) / 2 + 1; // "+1" for the LED to be responsible for 25%. Each unit of battery data corresponds to 10%, 0 = 0 - 9 % , 1 = 10 - 19 % , .. and 10 = 100 %
				//??? in charge mode, need to show animation within a few seconds 
				Gamepad.BatteryMode = ((buf[52] & 0x0f) & DS_STATUS_CHARGING) >> DS_STATUS_CHARGING_SHIFT; // 0x0 - discharging, 0x1 - full, 0x2 - charging, 0xa & 0xb - not-charging, 0xf - unknown
				//printf(" Battery status: %d\n", Gamepad.BatteryMode); // if there is charging, then we don't add 1 led
				Gamepad.BatteryLevel = (buf[53] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS_BATTERY_MAX;
			}
			else { // BT
				Gamepad.LEDBatteryLevel = (buf[54] & 0x0f) / 2 + 1;
				Gamepad.BatteryMode = ((buf[53] & 0x0f) & DS_STATUS_CHARGING) >> DS_STATUS_CHARGING_SHIFT;
				Gamepad.BatteryLevel = (buf[54] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS_BATTERY_MAX;
				//printf(" Battery status: %d\n", Gamepad.BatteryMode); // if there is charging, then we don't add 1 led
			}
			if (Gamepad.LEDBatteryLevel > 4) // min(data * 10 + 5, 100);
				Gamepad.LEDBatteryLevel = 4;
		}
		else if (Gamepad.ControllerType == SONY_DUALSHOCK4) {
			unsigned char buf[64];
			memset(buf, 0, 64);
			hid_read(Gamepad.HidHandle, buf, 64);
			if (Gamepad.USBConnection)
				Gamepad.BatteryLevel = (buf[30] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS4_USB_BATTERY_MAX;
			else
				Gamepad.BatteryLevel = (buf[32] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS_BATTERY_MAX;
		}
		else if (Gamepad.ControllerType == NINTENDO_JOYCONS || Gamepad.ControllerType == NINTENDO_SWITCH_PRO) {
			unsigned char buf[64];
			if (Gamepad.HidHandle != NULL) {	//@018 check to read (bytesRead > 0)
				memset(buf, 0, sizeof(buf));
				int bytesRead = hid_read(Gamepad.HidHandle, buf, 64);
				// Обновляем заряд, только если пакет реально пришел
				if (bytesRead > 0) { Gamepad.BatteryLevel = ((buf[2] >> 4) & 0x0F) * 100 / 8; }
			}

			if (Gamepad.HidHandle2 != NULL) {
				memset(buf, 0, sizeof(buf));
				int bytesRead = hid_read(Gamepad.HidHandle2, buf, 64);
				if (bytesRead > 0) { Gamepad.BatteryLevel2 = ((buf[2] >> 4) & 0x0F) * 100 / 8; }
			}
		}
		if (Gamepad.BatteryLevel > 100) Gamepad.BatteryLevel = 100; // It looks like something is not right, once it gave out 125%
	}
}

void GetBatteryInfo() {
	UpdateBatteryInfo(PrimaryGamepad);
	if (AppStatus.SecondaryGamepadEnabled && AppStatus.ControllerCount > 1 && SecondaryGamepad.DeviceIndex != -1)
		UpdateBatteryInfo(SecondaryGamepad);
}

void ShowBatteryLevels() {
	GetBatteryInfo(); if (AppStatus.BackOutStateCounter == 0) AppStatus.BackOutStateCounter = 40; // It is executed many times, so it is done this way, it is necessary to save the old brightness value for return
	if (AppStatus.ShowBatteryStatusOnLightBar) {
		// Primary gamepad
		if (AppStatus.BackOutStateCounter == 40) PrimaryGamepad.LastLEDBrightness = PrimaryGamepad.OutState.LEDBrightness; // Save on first click (tick)
		if (PrimaryGamepad.BatteryLevel >= 30) // Battery fine 30%-100%
			PrimaryGamepad.OutState.LEDColor = AppStatus.BatteryFineColor;
		else if (PrimaryGamepad.BatteryLevel >= 10) // Battery warning 10..29%
			PrimaryGamepad.OutState.LEDColor = AppStatus.BatteryWarningColor;
		else // battery critical 10%
			PrimaryGamepad.OutState.LEDColor = AppStatus.BatteryCriticalColor;
		PrimaryGamepad.OutState.LEDBrightness = PrimaryGamepad.DefaultLEDBrightness;

		// Secondary gamepad
		if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1) {
			if (AppStatus.BackOutStateCounter == 40) SecondaryGamepad.LastLEDBrightness = SecondaryGamepad.OutState.LEDBrightness; // Save on first click (tick)
			if (SecondaryGamepad.BatteryLevel >= 30) // Battery fine 30%-100%
				SecondaryGamepad.OutState.LEDColor = AppStatus.BatteryFineColor;
			else if (SecondaryGamepad.BatteryLevel >= 10) // Battery warning 10..29%
				SecondaryGamepad.OutState.LEDColor = AppStatus.BatteryWarningColor;
			else // battery critical 10%
				SecondaryGamepad.OutState.LEDColor = AppStatus.BatteryCriticalColor;
			SecondaryGamepad.OutState.LEDBrightness = SecondaryGamepad.DefaultLEDBrightness;
		}

	}
	PrimaryGamepad.OutState.PlayersCount = PrimaryGamepad.LEDBatteryLevel; // JslSetPlayerNumber(PrimaryGamepad.DeviceIndex, 5);
	if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1)
		SecondaryGamepad.OutState.PlayersCount = SecondaryGamepad.LEDBatteryLevel;
}

std::string GetJoystickOEMName(int joyId, const char* szRegKey) {	//@034 New externalPedals"
	if (szRegKey == nullptr || strlen(szRegKey) == 0) return "";

	HKEY hKey = NULL;
	char subKey[512];
	sprintf_s(subKey, "System\\CurrentControlSet\\Control\\MediaResources\\Joystick\\%s\\CurrentJoystickSettings", szRegKey);

	std::string oemKeyName = "";
	// Читаем ключ настроек джойстика
	if (RegOpenKeyExA(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		char valueName[64];
		sprintf_s(valueName, "Joystick%dOEMName", joyId + 1);

		char oemNameBuf[256];
		DWORD bufSize = sizeof(oemNameBuf);
		DWORD type = 0;
		if (RegQueryValueExA(hKey, valueName, NULL, &type, (LPBYTE)oemNameBuf, &bufSize) == ERROR_SUCCESS) {
			oemKeyName = oemNameBuf;
		}
		RegCloseKey(hKey);
	}

	if (oemKeyName.empty()) return "";

	std::string realName = "";
	sprintf_s(subKey, "System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\%s", oemKeyName.c_str());

	// Сначала ищем имя производителя в пользовательском реестре (HKCU)
	if (RegOpenKeyExA(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		char nameBuf[256];
		DWORD bufSize = sizeof(nameBuf);
		DWORD type = 0;
		if (RegQueryValueExA(hKey, "OEMName", NULL, &type, (LPBYTE)nameBuf, &bufSize) == ERROR_SUCCESS) {
			realName = nameBuf;
		}
		RegCloseKey(hKey);
	}

	// Если там пусто, ищем в глобальном реестре системы (HKLM)
	if (realName.empty()) {
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
			char nameBuf[256];
			DWORD bufSize = sizeof(nameBuf);
			DWORD type = 0;
			if (RegQueryValueExA(hKey, "OEMName", NULL, &type, (LPBYTE)nameBuf, &bufSize) == ERROR_SUCCESS) {
				realName = nameBuf;
			}
			RegCloseKey(hKey);
		}
	}

	return realName;
}

inline bool IsValidPedalDevice(const std::string& name, const std::string& configName) {
	std::string upperName = name;
	for (auto &c : upperName) c = toupper(c);

	std::string upperConfig = configName;
	for (auto &c : upperConfig) c = toupper(c);

	// Если пользователь вручную указал имя педалей в Config.ini (не AUTO) отключаем черные/белые списки и ищем точное совпадение
	if (!upperConfig.empty() && upperConfig != "AUTO") {
		return upperName.find(upperConfig) != std::string::npos;
	}

	// ИНАЧЕ: работает стандартный умный фильтр автоопределения
	// 1. БЕЛЫЙ СПИСОК (разрешаем рули и педали сразу)
	//if (upperName.find("LOGITECH") != std::string::npos) return true;

	// 2. ЧЕРНЫЙ СПИСОК (блокируем стандартные геймпады)
	if (upperName.find("GAMEPAD") != std::string::npos) return false;
	if (upperName.find("JOYSTICK") != std::string::npos) return false;
	if (upperName.find("DUALSHOCK") != std::string::npos) return false;
	if (upperName.find("DUALSENSE") != std::string::npos) return false;
	if (upperName.find("CONTROLLER (XBOX 360 FOR WINDOWS)") != std::string::npos) return false;
	if (upperName.find("CONTROLLER (XBOX 360 WIRELESS RECEIVER FOR WINDOWS)") != std::string::npos) return false;
	if (upperName.find("CONTROLLER (XBOX ONE FOR WINDOWS)") != std::string::npos) return false;
	if (upperName.find("XBOX WIRELESS CONTROLLER") != std::string::npos) return false;
	if (upperName.find("WIRELESS GAMEPAD") != std::string::npos) return false;	//Joy-con
	if (upperName.find("WIRELESS CONTROLLER") != std::string::npos) return false; // DualShock 4 & DualSense?
	if (upperName.find("PRO CONTROLLER") != std::string::npos) return false;      // Nintendo Switch Pro
	if (upperName.find("JOY-CON") != std::string::npos) return false;             // Любой из Joy-Con (L/R)
	if (upperName.find("LOGITECH GAMEPAD F310") != std::string::npos) return false;
	if (upperName.find("LOGITECH CORDLESS RUMBLEPAD 2") != std::string::npos) return false;

	return true;
}

void ExternalPedalsDInputSearch() {
	AppStatus.ExternalPedalsDInputConnected = false;
	printf("\n[Pedals Search] Scanning DirectInput devices...\n");

	for (int JoyID = 0; JoyID < 16; ++JoyID) {
		JOYCAPSA joyCapsA = {};
		if (joyGetPosEx(JoyID, &AppStatus.ExternalPedalsJoyInfo) == JOYERR_NOERROR &&
			joyGetDevCapsA(JoyID, &joyCapsA, sizeof(joyCapsA)) == JOYERR_NOERROR) {

			// Пытаемся прочитать реальное OEM-имя из реестра Windows
			std::string deviceName = GetJoystickOEMName(JoyID, joyCapsA.szRegKey);

			// Если реестр пуст, берем хотя бы имя драйвера
			if (deviceName.empty()) {
				deviceName = joyCapsA.szPname;
			}

			printf("[Pedals Search] ID %d: Found device '%s'", JoyID, deviceName.c_str());

			if (IsValidPedalDevice(deviceName, AppStatus.ExternalPedalsDeviceName)) {
				printf(" -> APPROVED!\n");
				AppStatus.ExternalPedalsJoyIndex = JoyID;
				AppStatus.ExternalPedalsDInputConnected = true;
				printf("[Pedals Search] Successfully matched pedals device '%s' on ID %d.\n", deviceName.c_str(), JoyID);
				break;
			}
			else {
				printf(" -> REJECTED (Not a pedal device)\n");
			}
		}
	}
	Sleep(2000);// Держим экран чтобы прочитать логи
}

void ExternalPedalsArduinoRead()
{
	DWORD bytesRead;

	while (AppStatus.ExternalPedalsArduinoConnected) {
		ReadFile(hSerial, &PedalsValues, sizeof(PedalsValues), &bytesRead, 0);

		if (PedalsValues[0] > 1.0 || PedalsValues[0] < 0 || PedalsValues[1] > 1.0 || PedalsValues[1] < 0)
		{
			PedalsValues[0] = 0;
			PedalsValues[1] = 0;

			PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
		}

		if (bytesRead == 0) Sleep(1);
	}
}

static std::mutex m;

//@044 Функция-транслятор отчета Xbox 360 в полноценный аппаратный отчет DualShock 4
void ConvertXusbToDs4(const XUSB_REPORT& x, DS4_REPORT& d, bool psPressed, bool touchPressed) {
	// 1. Конвертируем аналоговые стики из -32768..32767 в 0..255 (Y инвертируется для PS контроллеров)
	d.bThumbLX = (BYTE)(((int)x.sThumbLX + 32768) / 256);
	d.bThumbLY = (BYTE)(255 - (((int)x.sThumbLY + 32768) / 256));
	d.bThumbRX = (BYTE)(((int)x.sThumbRX + 32768) / 256);
	d.bThumbRY = (BYTE)(255 - (((int)x.sThumbRY + 32768) / 256));

	// 2. Копируем аналоговые триггеры
	d.bTriggerL = x.bLeftTrigger;
	d.bTriggerR = x.bRightTrigger;

	// 3. Конвертируем крестовину DPAD (Hat Switch: 0-7, 8 - нейтраль)
	BYTE dpad = 8;
	bool up = (x.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
	bool down = (x.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
	bool left = (x.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
	bool right = (x.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;

	if (up && right) dpad = 1;
	else if (down && right) dpad = 3;
	else if (down && left) dpad = 5;
	else if (up && left) dpad = 7;
	else if (up) dpad = 0;
	else if (right) dpad = 2;
	else if (down) dpad = 4;
	else if (left) dpad = 6;

	d.wButtons = dpad;

	// 4. Мапим основные кнопки (записываются побитно в wButtons)
	if (x.wButtons & XINPUT_GAMEPAD_X)              d.wButtons |= (1 << 4);  // Квадрат
	if (x.wButtons & XINPUT_GAMEPAD_A)              d.wButtons |= (1 << 5);  // Крест
	if (x.wButtons & XINPUT_GAMEPAD_B)              d.wButtons |= (1 << 6);  // Круг
	if (x.wButtons & XINPUT_GAMEPAD_Y)              d.wButtons |= (1 << 7);  // Треугольник
	if (x.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)   d.wButtons |= (1 << 8);  // L1
	if (x.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)  d.wButtons |= (1 << 9);  // R1
	//if (x.bLeftTrigger > 30)                        d.wButtons |= (1 << 10); // L2 (цифровой клик)
	//if (x.bRightTrigger > 30)                       d.wButtons |= (1 << 11); // R2 (цифровой клик)
	if (x.wButtons & XINPUT_GAMEPAD_BACK)           d.wButtons |= (1 << 12); // Share (Share / Back)
	if (x.wButtons & XINPUT_GAMEPAD_START)          d.wButtons |= (1 << 13); // Options (Options / Start)
	if (x.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)     d.wButtons |= (1 << 14); // L3
	if (x.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)    d.wButtons |= (1 << 15); // R3

	// 5. Специальные кнопки (PS Button и Клик тачпада)
	d.bSpecial = 0;
	if (psPressed)    d.bSpecial |= (1 << 0); // PS Button
	if (touchPressed) d.bSpecial |= (1 << 1); // Touchpad Click
}

// Приемник вибрации от игр для DualShock 4
VOID CALLBACK ds4_notification(
	PVIGEM_CLIENT Client,
	PVIGEM_TARGET Target,
	UCHAR LargeMotor,
	UCHAR SmallMotor,
	DS4_LIGHTBAR_COLOR LightbarColor,
	LPVOID UserData
)
{
	m.lock();
	int gamepadID = (int)(intptr_t)UserData;
	if (gamepadID == 1) {
		if (PrimaryGamepad.OutState.LargeMotor != LargeMotor || PrimaryGamepad.OutState.SmallMotor != SmallMotor) {
			PrimaryGamepad.OutState.LargeMotor = LargeMotor;
			PrimaryGamepad.OutState.SmallMotor = SmallMotor;
			GamepadSetState(PrimaryGamepad);
		}
	}
	else if (gamepadID == 2 && AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1) {
		if (SecondaryGamepad.OutState.LargeMotor != LargeMotor || SecondaryGamepad.OutState.SmallMotor != SmallMotor) {
			SecondaryGamepad.OutState.LargeMotor = LargeMotor;
			SecondaryGamepad.OutState.SmallMotor = SmallMotor;
			GamepadSetState(SecondaryGamepad);
		}
	}
	m.unlock();
}

VOID CALLBACK notification(
	PVIGEM_CLIENT Client,
	PVIGEM_TARGET Target,
	UCHAR LargeMotor,
	UCHAR SmallMotor,
	UCHAR LedNumber,
	LPVOID UserData
)
{
	m.lock();
	// PrimaryGamepad
	int gamepadID = (int)(intptr_t)UserData;
	if (gamepadID == 1) {	//@020 RumbleFix2	Защита от флуда: отправляем пакет если значения изменились
		if (PrimaryGamepad.OutState.LargeMotor != LargeMotor || PrimaryGamepad.OutState.SmallMotor != SmallMotor) {
		PrimaryGamepad.OutState.LargeMotor = LargeMotor;
		PrimaryGamepad.OutState.SmallMotor = SmallMotor;
		GamepadSetState(PrimaryGamepad);
		}

		// SecondaryGamepad
	}
	else if (gamepadID == 2 && AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1) {
		if (SecondaryGamepad.OutState.LargeMotor != LargeMotor || SecondaryGamepad.OutState.SmallMotor != SmallMotor) {
		SecondaryGamepad.OutState.LargeMotor = LargeMotor;
		SecondaryGamepad.OutState.SmallMotor = SmallMotor;
		GamepadSetState(SecondaryGamepad);
		}
	}
	m.unlock();
}

float accumulatedX = 0, accumulatedY = 0;
void MouseMove(float x, float y) { // Implementation from https://github.com/JibbSmart/JoyShockMapper/blob/master/JoyShockMapper/src/win32/InputHelpers.cpp
	accumulatedX += x;
	accumulatedY += y;

	int applicableX = (int)accumulatedX;
	int applicableY = (int)accumulatedY;

	accumulatedX -= applicableX;
	accumulatedY -= applicableY;

	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.mouseData = 0;
	input.mi.time = 0;
	input.mi.dx = applicableX;
	input.mi.dy = applicableY;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	SendInput(1, &input, sizeof(input));
}

void KMStickMode(AdvancedGamepad &Gamepad, bool DontResetInputState, bool StickIsLeft, float StickX, float StickY, int Mode) {
	// Порог отклонения стика (если 0 — ставим безопасные 50%)
	float threshold = (Gamepad.KMEmu.StickValuePressKey > 0.05f) ? Gamepad.KMEmu.StickValuePressKey : 0.5f;

	// РАЗДЕЛЕНИЕ ПАМЯТИ: Левый и правый стик используют РАЗНЫЕ структуры состояния, чтобы не спамить!
	Button* upBtn = StickIsLeft ? &Gamepad.ButtonsStates.Up : &Gamepad.ButtonsStates.RightStickUp;
	Button* downBtn = StickIsLeft ? &Gamepad.ButtonsStates.Down : &Gamepad.ButtonsStates.RightStickDown;
	Button* leftBtn = StickIsLeft ? &Gamepad.ButtonsStates.Left : &Gamepad.ButtonsStates.RightStickLeft;
	Button* rightBtn = StickIsLeft ? &Gamepad.ButtonsStates.Right : &Gamepad.ButtonsStates.RightStickRight;

	if (Mode == WASDStickMode) {
		KeyPress('W', DontResetInputState && StickY > threshold, upBtn, true);
		KeyPress('S', DontResetInputState && StickY < -threshold, downBtn, true);
		KeyPress('A', DontResetInputState && StickX < -threshold, leftBtn, true);
		KeyPress('D', DontResetInputState && StickX > threshold, rightBtn, true);
	}
	else if (Mode == ArrowsStickMode) {
		KeyPress(VK_UP, DontResetInputState && StickY > threshold, upBtn, true);
		KeyPress(VK_DOWN, DontResetInputState && StickY < -threshold, downBtn, true);
		KeyPress(VK_LEFT, DontResetInputState && StickX < -threshold, leftBtn, true);
		KeyPress(VK_RIGHT, DontResetInputState && StickX > threshold, rightBtn, true);
	}
	else if (Mode == MouseLookStickMode) {
		// ОЖИВЛЯЕМ МЫШЬ: Если чувствительность равна 0, берем нормальную базовую скорость 15.0f
		float sensX = (Gamepad.KMEmu.JoySensX > 0.0f) ? Gamepad.KMEmu.JoySensX : 15.0f;
		float sensY = (Gamepad.KMEmu.JoySensY > 0.0f) ? Gamepad.KMEmu.JoySensY : 15.0f;
		MouseMove(StickX * sensX, -StickY * sensY);
	}
	else if (Mode == MouseWheelStickMode) {
		mouse_event(MOUSEEVENTF_WHEEL, 0, 0, static_cast<DWORD>(static_cast<int>(StickY * 50.0f)), 0);
	}
	else if (Mode == NumpadsStickMode) {
		KeyPress(VK_NUMPAD8, DontResetInputState && StickY > threshold, upBtn, true);
		KeyPress(VK_NUMPAD2, DontResetInputState && StickY < -threshold, downBtn, true);
		KeyPress(VK_NUMPAD4, DontResetInputState && StickX < -threshold, leftBtn, true);
		KeyPress(VK_NUMPAD6, DontResetInputState && StickX > threshold, rightBtn, true);
	}
	else if (Mode == CustomStickMode) {
		if (StickIsLeft) {
			KeyPress(Gamepad.ButtonsStates.LeftStickUp.KeyCode, DontResetInputState && StickY > threshold, &Gamepad.ButtonsStates.LeftStickUp, true);
			KeyPress(Gamepad.ButtonsStates.LeftStickDown.KeyCode, DontResetInputState && StickY < -threshold, &Gamepad.ButtonsStates.LeftStickDown, true);
			KeyPress(Gamepad.ButtonsStates.LeftStickLeft.KeyCode, DontResetInputState && StickX < -threshold, &Gamepad.ButtonsStates.LeftStickLeft, true);
			KeyPress(Gamepad.ButtonsStates.LeftStickRight.KeyCode, DontResetInputState && StickX > threshold, &Gamepad.ButtonsStates.LeftStickRight, true);
		}
		else {
			KeyPress(Gamepad.ButtonsStates.RightStickUp.KeyCode, DontResetInputState && StickY > threshold, &Gamepad.ButtonsStates.RightStickUp, true);
			KeyPress(Gamepad.ButtonsStates.RightStickDown.KeyCode, DontResetInputState && StickY < -threshold, &Gamepad.ButtonsStates.RightStickDown, true);
			KeyPress(Gamepad.ButtonsStates.RightStickLeft.KeyCode, DontResetInputState && StickX < -threshold, &Gamepad.ButtonsStates.RightStickLeft, true);
			KeyPress(Gamepad.ButtonsStates.RightStickRight.KeyCode, DontResetInputState && StickX > threshold, &Gamepad.ButtonsStates.RightStickRight, true);
		}
	}
}

void LoadConfig() {	//@057 Realtime reading config (by modified file date) and applying (in main)
	CIniReader IniFile("config.ini");
	
	//@005 Двухкнопочный Binding для переключения режимов + чтение из Config, юзается новый парсинг в .h + условия активации toggle-функций в main (buttons & mask) == mask. )

	/*AppStatus.AimingByPressingMode = IniFile.ReadBoolean("Motion", "AimingByPressingMode", true);	//moved into profile
	AppStatus.AimingButtonName = IniFile.ReadString("Motion", "AimingButton", "NONE");
	AppStatus.AimingButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.AimingButtonName);
	AppStatus.AimingToggleButtonName = IniFile.ReadString("Motion", "AimingToggleButton", "NONE");
	AppStatus.AimingToggleButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.AimingToggleButtonName);
	AppStatus.AimingModeToggleButtonName = (IniFile.ReadString("Motion", "AimingModeToggleButton", "NONE"));
	AppStatus.AimingModeToggleButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.AimingModeToggleButtonName);
	AppStatus.DrivingToggleButtonName = IniFile.ReadString("Motion", "DrivingToggleButton", "NONE");
	AppStatus.DrivingToggleButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.DrivingToggleButtonName);	//@045
	AppStatus.DrivingCalibrationButtonName = IniFile.ReadString("Motion", "DrivingCalibrationButton", "NONE");
	AppStatus.DrivingCalibrationButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.DrivingCalibrationButtonName);
	AppStatus.AimingPressModeToggleButtonName = IniFile.ReadString("Motion", "AimingPressModeToggleButton", "NONE");	//@053
	AppStatus.AimingPressModeToggleButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.AimingPressModeToggleButtonName);
	AppStatus.RightStickModeButtonName = IniFile.ReadString("Gamepad", "RightStickModeButton", "NONE");
	AppStatus.RightStickModeButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.RightStickModeButtonName);*/	//@047

	AppStatus.MeleeGForce = IniFile.ReadFloat("Motion", "MeleeGForce", 3.0f); //@043
	AppStatus.GyroFromLeft = IniFile.ReadBoolean("Motion", "GyroFromLeft", false);		//@024 Gyro left hand

	PrimaryGamepad.Motion.Tightening = IniFile.ReadFloat("Motion", "Tightening", 2.0f); //@030
	PrimaryGamepad.Motion.RatchetDelayTime = IniFile.ReadFloat("Motion", "RatchetDelayTime", 150.0f);	//@058 Clutch Smoothing
	PrimaryGamepad.Motion.MotionWheelButtonsDeadZone = IniFile.ReadFloat("Motion", "MotionWheelButtonsDeadZone", 12.0f);
	
	//@067 Считаем готовые альфы при загрузке конфига:
	PrimaryGamepad.Motion.CachedMouseAlpha = (PrimaryGamepad.Motion.MouseSmooth > 0.0f) ? powf(PrimaryGamepad.Motion.MouseSmooth, AppStatus.FrameTime / 0.015f) : 0.0f;
	PrimaryGamepad.Motion.CachedJoyAlpha = (PrimaryGamepad.Motion.JoySmooth > 0.0f) ? powf(PrimaryGamepad.Motion.JoySmooth, AppStatus.FrameTime / 0.015f) : 0.0f;

	AppStatus.OsdType = IniFile.ReadInteger("SETTINGS", "OsdType", 0); //@075
	AppStatus.OsdPosX = IniFile.ReadInteger("SETTINGS", "OsdPosX", 20);
	AppStatus.OsdPosY = IniFile.ReadInteger("SETTINGS", "OsdPosY", 20);

	AppStatus.SplitJoycons = IniFile.ReadBoolean("Gamepad", "SplitJoycons", false);	//@040 Joy-con split Mode
	PrimaryGamepad.Sticks.InvertLeftXY = IniFile.ReadBoolean("Gamepad", "InvertLeftStickXY", false);	//@041
	PrimaryGamepad.Sticks.InvertRightXY = IniFile.ReadBoolean("Gamepad", "InvertRightStickXY", false);

	PrimaryGamepad.RumbleStrength = IniFile.ReadInteger("Gamepad", "RumbleStrength", 100);

	PrimaryGamepad.TouchSticksOn = IniFile.ReadBoolean("Gamepad", "TouchSticksOn", false);
	PrimaryGamepad.TouchSticks.LeftX = IniFile.ReadFloat("Gamepad", "TouchLeftStickSensX", 5.0f);
	PrimaryGamepad.TouchSticks.LeftY = IniFile.ReadFloat("Gamepad", "TouchLeftStickSensY", 5.0f);
	PrimaryGamepad.TouchSticks.RightX = IniFile.ReadFloat("Gamepad", "TouchRightStickSensX", 1.0f);
	PrimaryGamepad.TouchSticks.RightY = IniFile.ReadFloat("Gamepad", "TouchRightStickSensY", 1.0f);

	PrimaryGamepad.DefaultLEDBrightness = std::clamp((int)(255 - IniFile.ReadInteger("Gamepad", "DefaultBrightness", 100) * 2.55), 0, 255);
	PrimaryGamepad.OutState.LEDBrightness = PrimaryGamepad.DefaultLEDBrightness;

	/*PrimaryGamepad.Motion.AircraftEnabled = IniFile.ReadBoolean("Motion", "AircraftEnabled", false);	//not working
	PrimaryGamepad.Motion.AircraftPitchAngle = IniFile.ReadFloat("Motion", "AircraftPitchAngle", 45) / 2.0f;
	PrimaryGamepad.Motion.AircraftPitchInverted = IniFile.ReadBoolean("Motion", "AircraftPitchInverted", false) ? -1 : 1;
	PrimaryGamepad.Motion.AircraftRollSens = IniFile.ReadFloat("Motion", "AircraftRollSens", 100) * 0.11875f;*/

	AppStatus.LongPressTimeOut = IniFile.ReadInteger("Gamepad", "LongPressTimeOut", 300); //@071
	AppStatus.LockedChangeBrightness = IniFile.ReadBoolean("Gamepad", "LockChangeBrightness", false);
	AppStatus.ChangeModesWithClick = IniFile.ReadBoolean("Gamepad", "ChangeModesWithClick", true);
	AppStatus.ChangeModesWithoutAreas = IniFile.ReadBoolean("Gamepad", "ChangeModesWithoutAreas", false);
	AppStatus.JoyconChangeModesWithButton = SonyNintendoKeyNameToJoyShockKeyCode(IniFile.ReadString("Gamepad", "JoyconChangeModesWithButton", "NONE"));
	AppStatus.JoyconRumbleMerge = IniFile.ReadBoolean("Gamepad", "JoyconRumbleMerge", false);

	SecondaryGamepad.Sticks.DeadZoneLeftX = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneLeftStickX", 0) * 0.01f;	//@010
	SecondaryGamepad.Sticks.DeadZoneLeftX = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneLeftStickX", 0) * 0.01f;
	SecondaryGamepad.Sticks.DeadZoneLeftX = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneLeftStickX", 0) * 0.01f;
	SecondaryGamepad.Sticks.DeadZoneLeftY = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneLeftStickY", 0) * 0.01f;
	SecondaryGamepad.Sticks.DeadZoneRightX = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneRightStickX", 0) * 0.01f;
	SecondaryGamepad.Sticks.DeadZoneRightY = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneRightStickY", 0) * 0.01f;
	SecondaryGamepad.Triggers.DeadZoneLeft = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneLeftTrigger", 0) * 0.01f;

	SecondaryGamepad.Sticks.InvertLeftX = IniFile.ReadBoolean("SecondaryGamepad", "InvertLeftStickX", false);	//@041 +@042 + fix, в config были invert, тут нет
	SecondaryGamepad.Sticks.InvertLeftY = IniFile.ReadBoolean("SecondaryGamepad", "InvertLeftStickY", false);
	SecondaryGamepad.Sticks.InvertRightX = IniFile.ReadBoolean("SecondaryGamepad", "InvertRightStickX", false);
	SecondaryGamepad.Sticks.InvertRightY = IniFile.ReadBoolean("SecondaryGamepad", "InvertRightStickY", false);
	SecondaryGamepad.Sticks.InvertLeftXY = IniFile.ReadBoolean("SecondaryGamepad", "InvertLeftStickXY", false);
	SecondaryGamepad.Sticks.InvertRightXY = IniFile.ReadBoolean("SecondaryGamepad", "InvertRightStickXY", false);

	SecondaryGamepad.Triggers.DeadZoneRight = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneRightTrigger", 0);
	SecondaryGamepad.DefaultLEDBrightness = std::clamp((int)(255 - IniFile.ReadInteger("SecondaryGamepad", "DefaultBrightness", 100) * 2.55), 0, 255);
	SecondaryGamepad.OutState.LEDBrightness = SecondaryGamepad.DefaultLEDBrightness;
	SecondaryGamepad.DefaultModeColor = WebColorToRGB(IniFile.ReadString("SecondaryGamepad", "DefaultModeColor", "00ff00"));
	SecondaryGamepad.OutState.LEDColor = SecondaryGamepad.DefaultModeColor;

	// External pedals
	AppStatus.ExternalPedalsMode = IniFile.ReadInteger("ExternalPedals", "DefaultMode", 0);
	AppStatus.ExternalPedalsXboxModePedal1 = SonyNintendoKeyNameToJoyShockKeyCode(IniFile.ReadString("ExternalPedals", "AimingPedal1", "NONE"));
	AppStatus.ExternalPedalsXboxModePedal1Analog = (AppStatus.ExternalPedalsXboxModePedal1 == JSMASK_ZL) || (AppStatus.ExternalPedalsXboxModePedal1 == JSMASK_ZR);
	AppStatus.ExternalPedalsXboxModePedal2 = SonyNintendoKeyNameToJoyShockKeyCode(IniFile.ReadString("ExternalPedals", "AimingPedal2", "NONE"));
	AppStatus.ExternalPedalsXboxModePedal2Analog = (AppStatus.ExternalPedalsXboxModePedal2 == JSMASK_ZL) || (AppStatus.ExternalPedalsXboxModePedal2 == JSMASK_ZR);
	//AppStatus.ExternalPedalsValuePress = 65536 * ClampFloat(IniFile.ReadFloat("ExternalPedals", "PedalValuePress", 20.0f) * 0.01f, 0, 1.0f);
	AppStatus.ExternalPedalsValuePress = static_cast<DWORD>(65536.0f * ClampFloat(IniFile.ReadFloat("ExternalPedals", "PedalValuePress", 20.0f) * 0.01f, 0.0f, 1.0f));
	for (int i = 0; i < 16; ++i) AppStatus.ExternalPedalsButtons[i] = SonyNintendoKeyNameToJoyShockKeyCode(IniFile.ReadString("ExternalPedals", "Button" + std::to_string(i + 1), "NONE"));
	AppStatus.ExternalPedalsJoyInfo.dwFlags = JOY_RETURNALL;
	AppStatus.ExternalPedalsJoyInfo.dwSize = sizeof(AppStatus.ExternalPedalsJoyInfo);
	//в блок чтения настроек педалей:
	std::string p1AxisName = IniFile.ReadString("ExternalPedals", "Pedal1Axis", "V");
	std::string p2AxisName = IniFile.ReadString("ExternalPedals", "Pedal2Axis", "U");
	AppStatus.Pedal1Axis = ParseAxisName(p1AxisName);	//@034
	AppStatus.Pedal2Axis = ParseAxisName(p2AxisName);
	// Читаем имя устройства (если не задано, по умолчанию будет "AUTO")
	AppStatus.ExternalPedalsDeviceName = IniFile.ReadString("ExternalPedals", "DeviceName", "AUTO");	//@034
}

void LoadXboxProfile(std::string ProfileFile) {
	CIniReader IniFile("XboxProfiles\\" + ProfileFile);

	//Read settings from XboxProfile\*.ini
	PrimaryGamepad.Motion.SensX = IniFile.ReadFloat("SETTINGS", "MouseSensX", 100) * 0.009f;		//@046
	PrimaryGamepad.Motion.SensY = IniFile.ReadFloat("SETTINGS", "MouseSensY", 100) * 0.009f;
	PrimaryGamepad.Motion.SensAvg = (PrimaryGamepad.Motion.SensX + PrimaryGamepad.Motion.SensY) * 0.5f;
	PrimaryGamepad.Motion.JoySensX = IniFile.ReadFloat("SETTINGS", "JoySensX", 100) * 0.002f;
	PrimaryGamepad.Motion.JoySensY = IniFile.ReadFloat("SETTINGS", "JoySensY", 100) * 0.002f;
	PrimaryGamepad.Motion.JoySensAvg = (PrimaryGamepad.Motion.JoySensX + PrimaryGamepad.Motion.JoySensY) * 0.5f;
	PrimaryGamepad.Motion.MouseSmooth = ClampFloat(IniFile.ReadFloat("SETTINGS", "MouseSmooth", 0), 0, 99) * 0.01f; //@029 EMA Filter
	PrimaryGamepad.Motion.JoySmooth = ClampFloat(IniFile.ReadFloat("SETTINGS", "JoySmooth", 0), 0, 99) * 0.01f;
	//PrimaryGamepad.Motion.GyroApplyLinearity = IniFile.ReadBoolean("SETTINGS", "GyroApplyLinearity", true);	//@065
	PrimaryGamepad.Motion.GyroApplyAntiDeadZone = IniFile.ReadBoolean("SETTINGS", "GyroApplyAntiDeadZone", false);
	PrimaryGamepad.Motion.GyroAccelStart = IniFile.ReadFloat("SETTINGS", "GyroAccelStart", 50.0f);
	PrimaryGamepad.Motion.GyroAccelEnd = IniFile.ReadFloat("SETTINGS", "GyroAccelEnd", 0.0f); // @073 Parametric gyro acceleration
	PrimaryGamepad.Motion.GyroAccelCap = IniFile.ReadFloat("SETTINGS", "GyroAccelCap", 2.0f);

	PrimaryGamepad.Triggers.DeadZoneLeft = IniFile.ReadFloat("SETTINGS", "DeadZoneLeftTrigger", 0) * 0.01f;
	PrimaryGamepad.Triggers.DeadZoneRight = IniFile.ReadFloat("SETTINGS", "DeadZoneRightTrigger", 0) * 0.01f;
	PrimaryGamepad.Sticks.DeadZoneLeftX = IniFile.ReadFloat("SETTINGS", "DeadZoneLeftStickX", 0) * 0.01f;	//@010
	PrimaryGamepad.Sticks.DeadZoneLeftY = IniFile.ReadFloat("SETTINGS", "DeadZoneLeftStickY", 0) * 0.01f;
	PrimaryGamepad.Sticks.DeadZoneRightX = IniFile.ReadFloat("SETTINGS", "DeadZoneRightStickX", 0) * 0.01f;
	PrimaryGamepad.Sticks.DeadZoneRightY = IniFile.ReadFloat("SETTINGS", "DeadZoneRightStickY", 0) * 0.01f;
	PrimaryGamepad.Sticks.AntiDeadZoneLeftX = ClampFloat(IniFile.ReadFloat("SETTINGS", "AntiDeadZoneLeftX", 0), 0, 99) * 0.01f;	//@065
	PrimaryGamepad.Sticks.AntiDeadZoneLeftY = ClampFloat(IniFile.ReadFloat("SETTINGS", "AntiDeadZoneLeftY", 0), 0, 99) * 0.01f;
	PrimaryGamepad.Sticks.AntiDeadZoneRightX = ClampFloat(IniFile.ReadFloat("SETTINGS", "AntiDeadZoneRightX", 0), 0, 99) * 0.01f;
	PrimaryGamepad.Sticks.AntiDeadZoneRightY = ClampFloat(IniFile.ReadFloat("SETTINGS", "AntiDeadZoneRightY", 0), 0, 99) * 0.01f;

	PrimaryGamepad.Sticks.MaxLeftStickLimit = std::clamp(IniFile.ReadInteger("SETTINGS", "MaxLeftStickLimit", 32767), 1000, 32767);	//@074
	PrimaryGamepad.Sticks.MaxRightStickLimit = std::clamp(IniFile.ReadInteger("SETTINGS", "MaxRightStickLimit", 32767), 1000, 32767);

	PrimaryGamepad.Sticks.LinearityLeftX = IniFile.ReadFloat("SETTINGS", "LinearityLeftStickX", 50.0f); 	//@035 Linearity 1. Читаем из ini 
	PrimaryGamepad.Sticks.LinearityLeftY = IniFile.ReadFloat("SETTINGS", "LinearityLeftStickY", 50.0f);
	PrimaryGamepad.Sticks.LinearityRightX = IniFile.ReadFloat("SETTINGS", "LinearityRightStickX", 50.0f);
	PrimaryGamepad.Sticks.LinearityRightY = IniFile.ReadFloat("SETTINGS", "LinearityRightStickY", 50.0f);
	auto GetPower = [](float linearity) {	//@067 2. Сразу рассчитываем быстрые степени p для ускоренной математики:
		if (linearity == 50.0f) return 1.0f;
		return powf(2.0f, (50.0f - linearity) / 25.0f);
	};
	PrimaryGamepad.Sticks.p_LeftX = GetPower(PrimaryGamepad.Sticks.LinearityLeftX);
	PrimaryGamepad.Sticks.p_LeftY = GetPower(PrimaryGamepad.Sticks.LinearityLeftY);
	PrimaryGamepad.Sticks.p_RightX = GetPower(PrimaryGamepad.Sticks.LinearityRightX);
	PrimaryGamepad.Sticks.p_RightY = GetPower(PrimaryGamepad.Sticks.LinearityRightY);

	PrimaryGamepad.Sticks.InvertLeftX = IniFile.ReadBoolean("SETTINGS", "InvertLeftStickX", false);
	PrimaryGamepad.Sticks.InvertLeftY = IniFile.ReadBoolean("SETTINGS", "InvertLeftStickY", false);
	PrimaryGamepad.Sticks.InvertRightX = IniFile.ReadBoolean("SETTINGS", "InvertRightStickX", false);
	PrimaryGamepad.Sticks.InvertRightY = IniFile.ReadBoolean("SETTINGS", "InvertRightStickY", false);

	PrimaryGamepad.Motion.SteeringWheelAngle = IniFile.ReadFloat("SETTINGS", "SteeringWheelAngle", 150) / 2.0f;
	PrimaryGamepad.Motion.LinearityWheel = IniFile.ReadFloat("SETTINGS", "LinearityWheel", 50.0f);

	AppStatus.AimingButtonName = IniFile.ReadString("SETTINGS", "AimingButton", "NONE");
	AppStatus.AimingButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.AimingButtonName);
	AppStatus.AimMode = IniFile.ReadBoolean("SETTINGS", "AimingMode", AimMouseMode);
	AppStatus.AimingToggleButtonName = IniFile.ReadString("SETTINGS", "AimingToggleButton", "NONE");
	AppStatus.AimingToggleButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.AimingToggleButtonName);
	AppStatus.AimingModeToggleButtonName = (IniFile.ReadString("SETTINGS", "AimingModeToggleButton", "NONE"));
	AppStatus.AimingModeToggleButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.AimingModeToggleButtonName);
	AppStatus.DrivingToggleButtonName = IniFile.ReadString("SETTINGS", "DrivingToggleButton", "NONE");
	AppStatus.DrivingToggleButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.DrivingToggleButtonName);	//@045
	AppStatus.DrivingCalibrationButtonName = IniFile.ReadString("SETTINGS", "DrivingCalibrationButton", "NONE");
	AppStatus.DrivingCalibrationButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.DrivingCalibrationButtonName);
	AppStatus.AimingPressModeToggleButtonName = IniFile.ReadString("SETTINGS", "AimingPressModeToggleButton", "NONE");	//@053
	AppStatus.AimingPressModeToggleButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.AimingPressModeToggleButtonName);
	AppStatus.RightStickModeButtonName = IniFile.ReadString("SETTINGS", "RightStickModeButton", "NONE");
	AppStatus.RightStickModeButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.RightStickModeButtonName);	//@047
	//AppStatus.AimingByPressingMode = IniFile.ReadBoolean("SETTINGS", "AimingByPressingMode", true);
	bool newAimingByPressing = IniFile.ReadBoolean("SETTINGS", "AimingByPressingMode", true);
	if (AppStatus.AimingByPressingMode != newAimingByPressing) {
		AppStatus.AimingByPressingMode = newAimingByPressing;

		// Мгновенно обновляем текущий режим геймпада (если прицеливание сейчас активно)
		if (PrimaryGamepad.GamepadActionMode == MotionAimingMode || PrimaryGamepad.GamepadActionMode == MotionAimingModeOnlyPressed) {
			PrimaryGamepad.GamepadActionMode = AppStatus.AimingByPressingMode ? MotionAimingModeOnlyPressed : MotionAimingMode;
			PrimaryGamepad.LastMotionAIMMode = PrimaryGamepad.GamepadActionMode;
		}
	}
	
	CurrentXboxProfile.SwapSticksAxis = IniFile.ReadBoolean("SETTINGS", "SWAP-STICKS", false);
	CurrentXboxProfile.SwapTriggers = IniFile.ReadBoolean("SETTINGS", "SWAP-TRIGGERS", false);

	AppStatus.LeftStickMode = std::clamp(IniFile.ReadInteger("SETTINGS", "LeftStickMode", 0), 0, 2);
	PrimaryGamepad.AutoPressStickValue = IniFile.ReadFloat("SETTINGS", "AutoPressStickValue", 99) * 0.01f;
	CurrentXboxProfile.AutoSprintButton = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "AutoSprintButton", "LS"));

	CurrentXboxProfile.RightStickMode = IniFile.ReadInteger("SETTINGS", "RightStickMode", 0);	//@049
	CurrentXboxProfile.RightStickUp = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "RS-UP", "NONE"));
	CurrentXboxProfile.RightStickDown = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "RS-DOWN", "NONE"));
	CurrentXboxProfile.RightStickLeft = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "RS-LEFT", "NONE"));
	CurrentXboxProfile.RightStickRight = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "RS-RIGHT", "NONE"));

	std::string modesStr = IniFile.ReadString("SETTINGS", "RightStickAvailableModes", "0, 1, 2"); // Читаем список режимов для переключения (например, "2, 0" или "0, 1" или "0, 2, 1")
	CurrentXboxProfile.RightStickCycleModes.clear();

	for (char c : modesStr) {	// Простейший и пуленепробиваемый парсер: собирает любые цифры от 0 до 2, игнорируя запятые и пробелы!
		if (c >= '0' && c <= '2') {
			CurrentXboxProfile.RightStickCycleModes.push_back(c - '0');
		}
	}

	if (CurrentXboxProfile.RightStickCycleModes.empty()) {		// Если юзер написал херню — страхуемся дефолтом 0, 1, 2
		CurrentXboxProfile.RightStickCycleModes = { 0, 1, 2 };
	}

	CurrentXboxProfile.RightStickCycleIndex = 0;		// Синхронизируем индекс списка с текущим стартовым режимом RightStickMode
	for (size_t i = 0; i < CurrentXboxProfile.RightStickCycleModes.size(); i++) {
		if (CurrentXboxProfile.RightStickCycleModes[i] == CurrentXboxProfile.RightStickMode) {
			CurrentXboxProfile.RightStickCycleIndex = (int)i;
			break;
		}
	}


	//@031 + @071 fix Нужно для GUI Config и двух button Lyaouts. "_MISSING_" помогает отличить отсутствие ключа от явного "NONE"
	//УНИВЕРСАЛЬНОЕ ЯДРО ПАРСЕРА Читает сырую строку с учетом контроллера (allowFallback = false строго для _LONG)
	auto ReadRawKey = [&](const char* section, const std::string& nintendoKey, const std::string& sonyKey, const std::string& xboxKey, bool allowFallback) -> std::string {
		std::string val = "_MISSING_";
		if (PrimaryGamepad.ControllerType == SONY_DUALSENSE || PrimaryGamepad.ControllerType == SONY_DUALSHOCK4) {
			val = IniFile.ReadString(section, sonyKey, "_MISSING_");
			if (val == "_MISSING_" && allowFallback) val = IniFile.ReadString(section, nintendoKey, "_MISSING_");
		}
		else if (PrimaryGamepad.ControllerType == NINTENDO_JOYCONS || PrimaryGamepad.ControllerType == NINTENDO_SWITCH_PRO) {
			val = IniFile.ReadString(section, nintendoKey, "_MISSING_");
			if (val == "_MISSING_" && allowFallback) val = IniFile.ReadString(section, sonyKey, "_MISSING_");
		}
		else {
			val = IniFile.ReadString(section, xboxKey, "_MISSING_");
		}

		if (val == "_MISSING_" && allowFallback) {
			val = IniFile.ReadString(section, xboxKey, "_MISSING_");
		}
		return val;
	};

	// Чтение кнопки для виртуального XBOX (короткая или длинная)
	auto ReadXbox = [&](const std::string& n, const std::string& s, const std::string& x, const std::string& defVal, bool isLong = false) -> unsigned int {
		std::string val = ReadRawKey("XBOX", isLong ? n + "_LONG" : n, isLong ? s + "_LONG" : s, isLong ? x + "_LONG" : x, !isLong);
		if (val != "_MISSING_") return XboxKeyNameToXboxKeyCode(val);
		return isLong ? 0 : XboxKeyNameToXboxKeyCode(defVal);
	};

	// Чтение кнопки для KEYBOARD-MOUSE (короткая или длинная)
	auto ReadKbm = [&](const std::string& n, const std::string& s, const std::string& x, bool isLong = false) -> int {
		std::string val = ReadRawKey("KEYBOARD-MOUSE", isLong ? n + "_LONG" : n, isLong ? s + "_LONG" : s, isLong ? x + "_LONG" : x, !isLong);
		if (val != "_MISSING_") return KeyNameToKeyCode(val);
		return 0;
	};

	// Главный упаковщик: читает сразу Short Xbox, Long Xbox, Short KBM и Long KBM в 1 вызов!
	auto BindButton = [&](const std::string& n, const std::string& s, const std::string& x, const std::string& defXbox,
		unsigned int& outXboxShort, unsigned int& outXboxLong, Button& outKbm) {
		outXboxShort = ReadXbox(n, s, x, defXbox, false);
		outXboxLong = ReadXbox(n, s, x, "NONE", true);
		outKbm.KeyCode = ReadKbm(n, s, x, false);
		outKbm.LongKeyCode = ReadKbm(n, s, x, true);
	};


	//Virtual buttons. Читаем всё сразу (Xbox + KBM + LongPress):
	BindButton("L", "L1", "LB", "LB", CurrentXboxProfile.LeftBumper, CurrentXboxProfile.LeftBumperLong, PrimaryGamepad.ButtonsStates.LeftBumper);
	BindButton("R", "R1", "RB", "RB", CurrentXboxProfile.RightBumper, CurrentXboxProfile.RightBumperLong, PrimaryGamepad.ButtonsStates.RightBumper);
	BindButton("B", "CROSS", "A", "A", CurrentXboxProfile.A, CurrentXboxProfile.ALong, PrimaryGamepad.ButtonsStates.A);
	BindButton("A", "CIRCLE", "B", "B", CurrentXboxProfile.B, CurrentXboxProfile.BLong, PrimaryGamepad.ButtonsStates.B);
	BindButton("Y", "SQUARE", "X", "X", CurrentXboxProfile.X, CurrentXboxProfile.XLong, PrimaryGamepad.ButtonsStates.X);
	BindButton("X", "TRIANGLE", "Y", "Y", CurrentXboxProfile.Y, CurrentXboxProfile.YLong, PrimaryGamepad.ButtonsStates.Y);
	BindButton("UP", "UP", "UP", "UP", CurrentXboxProfile.DPADUp, CurrentXboxProfile.DPADUpLong, PrimaryGamepad.ButtonsStates.DPADUp);
	BindButton("DOWN", "DOWN", "DOWN", "DOWN", CurrentXboxProfile.DPADDown, CurrentXboxProfile.DPADDownLong, PrimaryGamepad.ButtonsStates.DPADDown);
	BindButton("LEFT", "LEFT", "LEFT", "LEFT", CurrentXboxProfile.DPADLeft, CurrentXboxProfile.DPADLeftLong, PrimaryGamepad.ButtonsStates.DPADLeft);
	BindButton("RIGHT", "RIGHT", "RIGHT", "RIGHT", CurrentXboxProfile.DPADRight, CurrentXboxProfile.DPADRightLong, PrimaryGamepad.ButtonsStates.DPADRight);
	BindButton("L3", "L3", "LS", "LS", CurrentXboxProfile.LeftStick, CurrentXboxProfile.LeftStickLong, PrimaryGamepad.ButtonsStates.LeftStick);
	BindButton("R3", "R3", "RS", "RS", CurrentXboxProfile.RightStick, CurrentXboxProfile.RightStickLong, PrimaryGamepad.ButtonsStates.RightStick);
	BindButton("MINUS", "SHARE", "BACK", "BACK", CurrentXboxProfile.Back, CurrentXboxProfile.BackLong, PrimaryGamepad.ButtonsStates.Back);
	BindButton("PLUS", "OPTIONS", "START", "START", CurrentXboxProfile.Start, CurrentXboxProfile.StartLong, PrimaryGamepad.ButtonsStates.Start);

	// Курки ZL / ZR (аналоговые на Xbox, цифровые для KBM)
	CurrentXboxProfile.ZL = ReadXbox("ZL", "L2", "LT", "LT", false);
	CurrentXboxProfile.ZR = ReadXbox("ZR", "R2", "RT", "RT", false);
	PrimaryGamepad.ButtonsStates.LeftTrigger.KeyCode = ReadKbm("ZL", "L2", "LT", false);
	PrimaryGamepad.ButtonsStates.LeftTrigger.LongKeyCode = ReadKbm("ZL", "L2", "LT", true);
	PrimaryGamepad.ButtonsStates.RightTrigger.KeyCode = ReadKbm("ZR", "R2", "RT", false);
	PrimaryGamepad.ButtonsStates.RightTrigger.LongKeyCode = ReadKbm("ZR", "R2", "RT", true);

	// Xbox: читаются из своих секций [JOYCONS] и [DUALSENSE-EDGE]
	auto ReadSpecialXbox = [&](const char* section, const char* key, unsigned int& outShort, unsigned int& outLong) {
		outShort = XboxKeyNameToXboxKeyCode(IniFile.ReadString(section, key, "NONE"));
		outLong = XboxKeyNameToXboxKeyCode(IniFile.ReadString(section, (std::string(key) + "_LONG").c_str(), "NONE"));
	};
	ReadSpecialXbox("JOYCONS", "SL", CurrentXboxProfile.JCSL, CurrentXboxProfile.JCSLLong);
	ReadSpecialXbox("JOYCONS", "SR", CurrentXboxProfile.JCSR, CurrentXboxProfile.JCSRLong);
	ReadSpecialXbox("JOYCONS", "HOME", CurrentXboxProfile.HOME, CurrentXboxProfile.HOMELong);
	ReadSpecialXbox("JOYCONS", "CAPTURE", CurrentXboxProfile.CAPTURE, CurrentXboxProfile.CAPTURELong);
	ReadSpecialXbox("DUALSENSE-EDGE", "L4", CurrentXboxProfile.DSEdgeL4, CurrentXboxProfile.DSEdgeL4Long);
	ReadSpecialXbox("DUALSENSE-EDGE", "R4", CurrentXboxProfile.DSEdgeR4, CurrentXboxProfile.DSEdgeR4Long);

	// KBM: читаются из секции [KEYBOARD-MOUSE]
	auto ReadSpecialKbm = [&](const std::string& n, const std::string& s, const std::string& u, Button& outKbm) {
		outKbm.KeyCode = ReadKbm(n, s, u, false);
		outKbm.LongKeyCode = ReadKbm(n, s, u, true);
	};
	ReadSpecialKbm("SL", "SL", "JCSL", PrimaryGamepad.ButtonsStates.JCSL);
	ReadSpecialKbm("SR", "SR", "JCSR", PrimaryGamepad.ButtonsStates.JCSR);
	ReadSpecialKbm("HOME", "PS", "HOME", PrimaryGamepad.ButtonsStates.HOME);
	ReadSpecialKbm("CAPTURE", "CAPTURE", "CAPTURE", PrimaryGamepad.ButtonsStates.CAPTURE);
	ReadSpecialKbm("L4", "L4", "DSEdgeL4", PrimaryGamepad.ButtonsStates.DSEdgeL4);
	ReadSpecialKbm("R4", "R4", "DSEdgeR4", PrimaryGamepad.ButtonsStates.DSEdgeR4);

	// Motion wheel XBOX
	CurrentXboxProfile.WheelActivationButton = SonyNintendoKeyNameToJoyShockKeyCode(IniFile.ReadString("MOTION", "WHEEL-ACTIVATION", "L2"));
	CurrentXboxProfile.WheelDefault = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-DEFAULT", "0"));
	CurrentXboxProfile.WheelUp = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-UP", "2"));
	CurrentXboxProfile.WheelLeft = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-LEFT", "1"));
	CurrentXboxProfile.WheelRight = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-RIGHT", "3"));
	CurrentXboxProfile.WheelDown = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-DOWN", "4"));

	CurrentXboxProfile.WheelUpLeft = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-UP-LEFT", "NONE"));
	CurrentXboxProfile.WheelUpRight = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-UP-RIGHT", "NONE"));
	CurrentXboxProfile.WheelDownLeft = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-DOWN-LEFT", "NONE"));
	CurrentXboxProfile.WheelDownRight = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-DOWN-RIGHT", "NONE"));
	CurrentXboxProfile.WheelAdvancedMode = !(CurrentXboxProfile.WheelUpLeft == 0 && CurrentXboxProfile.WheelUpRight == 0 && CurrentXboxProfile.WheelDownLeft == 0 && CurrentXboxProfile.WheelDownRight == 0);

	CurrentXboxProfile.MeleeGesture = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "MELEE-GESTURE", "NONE")); //@043

	// Wheel KB/M for XobxFrofile 
	PrimaryGamepad.ButtonsStates.WheelDefault.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "WHEEL-DEFAULT", "NONE"));
	PrimaryGamepad.ButtonsStates.WheelUp.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "WHEEL-UP", "NONE"));
	PrimaryGamepad.ButtonsStates.WheelLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "WHEEL-LEFT", "NONE"));
	PrimaryGamepad.ButtonsStates.WheelRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "WHEEL-RIGHT", "NONE"));
	PrimaryGamepad.ButtonsStates.WheelDown.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "WHEEL-DOWN", "NONE"));
	PrimaryGamepad.ButtonsStates.WheelUpLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "WHEEL-UP-LEFT", "NONE"));
	PrimaryGamepad.ButtonsStates.WheelUpRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "WHEEL-UP-RIGHT", "NONE"));
	PrimaryGamepad.ButtonsStates.WheelDownLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "WHEEL-DOWN-LEFT", "NONE"));
	PrimaryGamepad.ButtonsStates.WheelDownRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "WHEEL-DOWN-RIGHT", "NONE"));
	PrimaryGamepad.ButtonsStates.WheelAdvancedMode = !(PrimaryGamepad.ButtonsStates.WheelUpLeft.KeyCode == 0 && PrimaryGamepad.ButtonsStates.WheelUpRight.KeyCode == 0 && PrimaryGamepad.ButtonsStates.WheelDownLeft.KeyCode == 0 && PrimaryGamepad.ButtonsStates.WheelDownRight.KeyCode == 0);

	// Специфичные диагонали (они не имеют физических кнопок на Joy-Con, поэтому оставляем старые ключи)
	PrimaryGamepad.ButtonsStates.DPADUpLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "UP-LEFT", "NONE"));
	PrimaryGamepad.ButtonsStates.DPADUpRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "UP-RIGHT", "NONE"));
	PrimaryGamepad.ButtonsStates.DPADDownLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "DOWN-LEFT", "NONE"));
	PrimaryGamepad.ButtonsStates.DPADDownRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "DOWN-RIGHT", "NONE"));
	PrimaryGamepad.ButtonsStates.LeftStickUp.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "LS-UP", "NONE"));
	PrimaryGamepad.ButtonsStates.LeftStickLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "LS-LEFT", "NONE"));
	PrimaryGamepad.ButtonsStates.LeftStickRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "LS-RIGHT", "NONE"));
	PrimaryGamepad.ButtonsStates.LeftStickDown.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "LS-DOWN", "NONE"));
	PrimaryGamepad.ButtonsStates.RightStickUp.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "RS-UP", "NONE"));
	PrimaryGamepad.ButtonsStates.RightStickLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "RS-LEFT", "NONE"));
	PrimaryGamepad.ButtonsStates.RightStickRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "RS-RIGHT", "NONE"));
	PrimaryGamepad.ButtonsStates.RightStickDown.KeyCode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "RS-DOWN", "NONE"));

	PrimaryGamepad.KMEmu.LeftStickMode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "LS-MODE", "NONE"));
	PrimaryGamepad.KMEmu.RightStickMode = KeyNameToKeyCode(IniFile.ReadString("KEYBOARD-MOUSE", "RS-MODE", "NONE"));

	//PrimaryGamepad.ButtonsStates.MeleeGesture.KeyCode = ReadKbmKey("MELEE-GESTURE", "MELEE-GESTURE", "MELEE-GESTURE"); //@043
	//PrimaryGamepad.ButtonsStates.AutoSprint.KeyCode = ReadKbmKey("AutoSprintButton", "AutoSprintButton", "AutoSprintButton");	//@055
	PrimaryGamepad.ButtonsStates.MeleeGesture.KeyCode = ReadKbm("MELEE-GESTURE", "MELEE-GESTURE", "MELEE-GESTURE", false); //@043
	PrimaryGamepad.ButtonsStates.AutoSprint.KeyCode = ReadKbm("AutoSprintButton", "AutoSprintButton", "AutoSprintButton", false); //@055

	PrimaryGamepad.Motion.BaseSensX = PrimaryGamepad.Motion.SensX;	//@051
	PrimaryGamepad.Motion.BaseSensY = PrimaryGamepad.Motion.SensY;
	PrimaryGamepad.Motion.BaseJoySensX = PrimaryGamepad.Motion.JoySensX;
	PrimaryGamepad.Motion.BaseJoySensY = PrimaryGamepad.Motion.JoySensY;
}

void DefaultMainText() {
	if (AppStatus.ControllerCount < 1) { //@025 New menu Layer0 
		u8printf(T("Layer0_Connect", "\n Connect Joy-con(s), Pro controller, DualShock 4, DualSense or Press \"ALT + Esc\" to Exit.").c_str());
		return;
	}

	if (!AppStatus.ShowFullMenu) {	//	Layer1 Light menu for novice
		u8printf(T("Layer1_Connected", "\n Connected controllers: ").c_str());
		switch (PrimaryGamepad.ControllerType) {
		case SONY_DUALSENSE:
			u8printf(T("Layer1_DualSense", "\033[32m Sony DualSense\033[0m").c_str());
			break;
		case SONY_DUALSHOCK4:
			u8printf(T("Layer1_DualShock", "\033[32m Sony DualShock 4\033[0m").c_str());
			break;
		case NINTENDO_JOYCONS:
			u8printf(T("Layer1_Joy-Con(s)", "\033[32m Nintendo Joy-Con(s) -\033[0m").c_str());
			if (PrimaryGamepad.HidHandle != NULL && PrimaryGamepad.HidHandle2 != NULL) u8printf(T("Layer1_JC_Both", "\033[32m left & right\033[0m").c_str());
			else if (PrimaryGamepad.HidHandle != NULL) u8printf(T("Layer1_JC(L)", "\033[32m left\033[0m").c_str());
			else if (PrimaryGamepad.HidHandle2 != NULL) u8printf(T("Layer1_JC(R)", "\033[32m right\033[0m").c_str());
			//printf(") (\033[32m all functions)\033[0m");
			break;
		case NINTENDO_SWITCH_PRO:
			u8printf(T("Layer1_Pro", "\033[32m Nintendo Switch Pro\033[0m").c_str());
			break;
		default:
			break;
		}
		if (AppStatus.SecondaryGamepadEnabled) {
			if (SecondaryGamepad.DeviceIndex != -1) {
				printf(", ");
				switch (SecondaryGamepad.ControllerType) {
				case SONY_DUALSENSE:
					printf("Sony DualSense (simplified)");
					break;
				case SONY_DUALSHOCK4:
					printf("Sony DualShock 4 (simplified)");
					break;
				case NINTENDO_JOYCONS:
					printf("Nintendo Joy-Cons (");
					if (SecondaryGamepad.HidHandle != NULL && SecondaryGamepad.HidHandle2 != NULL) printf("left & right");
					else if (SecondaryGamepad.HidHandle != NULL) printf("left - limited input");
					else if (SecondaryGamepad.HidHandle2 != NULL) printf("right - limited input");
					printf(") (simplified)");
					break;
				case NINTENDO_SWITCH_PRO:
					printf("Nintendo Switch Pro Controller (simplified)");
					break;
				default:
					break;
				}
			}
		} else if (AppStatus.ControllerCount > 1 && SecondaryGamepad.DeviceIndex != -1) printf(", the second gamepad is disabled in the config");
		printf("\n");

		u8printf(T("Layer1_Reset", "\n Press \"\033[1m%s\033[0m\" or \"\033[1mCTRL + R\033[0m\" to reset/search for controllers, \"\033[1mALT + V\033[0m\" to swap the 1st and 2nd ones\n").c_str(), AppStatus.HotKeys.ResetKeyName.c_str());

		if (AppStatus.ControllerCount > 0 && AppStatus.ShowBatteryStatus) {
			printf(" Controller 1");
			if (PrimaryGamepad.USBConnection) printf(" wired");
			else printf(" wireless");
			if (PrimaryGamepad.ControllerType != NINTENDO_JOYCONS) printf(", battery charge: %d%%", PrimaryGamepad.BatteryLevel);
			else {
				if (PrimaryGamepad.HidHandle != NULL && PrimaryGamepad.HidHandle2 != NULL) printf(", battery charge: %d%%, %d%%", PrimaryGamepad.BatteryLevel, PrimaryGamepad.BatteryLevel2);
				else if (PrimaryGamepad.HidHandle != NULL) printf(", battery charge: %d%%", PrimaryGamepad.BatteryLevel);
				else if (PrimaryGamepad.HidHandle2 != NULL) printf(", battery charge: %d%%", PrimaryGamepad.BatteryLevel2);
			}
			if (PrimaryGamepad.BatteryMode == 0x2) printf(" (charging)");

			if (AppStatus.SecondaryGamepadEnabled && AppStatus.ControllerCount > 1 && SecondaryGamepad.DeviceIndex != -1) {
				printf(". Controller 2");
				if (SecondaryGamepad.USBConnection) printf(" wired");
				else printf(" wireless");
				if (SecondaryGamepad.ControllerType != NINTENDO_JOYCONS) printf(", battery charge: %d%%", SecondaryGamepad.BatteryLevel);
				else {
					if (SecondaryGamepad.HidHandle != NULL && SecondaryGamepad.HidHandle2 != NULL) printf(", battery level: %d%%, %d%%", SecondaryGamepad.BatteryLevel, SecondaryGamepad.BatteryLevel2);
					else if (SecondaryGamepad.HidHandle != NULL) printf(", battery level: %d%%", SecondaryGamepad.BatteryLevel);
					else if (SecondaryGamepad.HidHandle2 != NULL) printf(", battery level: %d%%", SecondaryGamepad.BatteryLevel2);
				}
				if (SecondaryGamepad.BatteryMode == 0x2) printf(" (charging)");
			}

			printf(".\n");
		}

		u8printf(T("Layer1_Descript", "\n \033[4mDescription\033[0m:").c_str());
		u8printf(T("Layer1_About", "\n JCAdvance is an Xbox gamepad emulator with advanced Gyro features. You can map most of any button on your \n"
		" gamepad to emulate any of Xbox, Keyboard or Mouse keys. Gyro modes are controlled in real time using hotkeys.\n" 
		" For setup primary setting use Config.exe. To manage all settings see config.ini and XboxProfile\\*.ini\n").c_str());
		
		u8printf(T("Layer1_Info", "\n \033[4mGyro info\033[0m: ").c_str());
		u8printf(T("Layer1_Calibrate", "\n Auto-calibration: place the device on a flat surface, wait for the beep, or press \"\033[1m%s\033[0m\" to do it manually\n").c_str(), AppStatus.HotKeys.GyroCalibrateKeyName.c_str());
		//u8printf(T("Layer1_Calibrate", "\n Auto-calibration: place the device on a flat surface and wait for the beep\n").c_str());
		u8printf(T("Layer1_Sense", "\n Press \"\033[1mCapture + X/B\033[0m\" or \"\033[1mPS + \xE2\x96\xB3/x\033[0m\" to change aiming sensitivity, \"Capture/PS + R3\" to reset\n").c_str());
		u8printf(T("Layer1_Gyro_On", "\n Press \"\033[1m%s\033[0m\" or \"\033[1mALT + 2\033[0m\" to unlock Gyro Motion (on/off)\n").c_str(), AppStatus.AimingToggleButtonName.c_str());

		if (AppStatus.AimMode == AimMouseMode) u8printf(T("Layer1_Mode_Mouse", "\n \033[1mControls\033[0m: \033[33mGyro Mouse\033[0m").c_str());
		else u8printf(T("Layer1_Mode_Stick", "\n \033[1mControls\033[0m: \033[36mGyro Stick\033[0m").c_str());
		u8printf(T("Layer1_Mode_Switch", ", to switch mode press \"\033[1m%s\033[0m\" or \"\033[1mALT + A\033[0m\"\n").c_str(), AppStatus.AimingModeToggleButtonName.c_str());
		u8printf(T("Layer1_Move_Button", "\n \033[1mControl Button\033[0m: \"\033[93m%s\033[0m\", %s. Press \"\033[93m%s\033[0m\" to change the behavior\n").c_str(),
			AppStatus.AimingButtonName.c_str(),
			AppStatus.AimingByPressingMode ?
			T("Layer1_START_MOVE", "hold to \033[4mstart\033[0m motion").c_str() :
			T("Layer1_STOP_MOVE", "hold to \033[4mstop\033[0m motion").c_str(),
			AppStatus.AimingPressModeToggleButtonName.c_str());

		u8printf(T("Layer1_Driving", "\n Press \"\033[1m%s\033[0m\" or \"\033[1mALT + 1\033[0m\" to activate Driving Mode (on/off), \"\033[1m%s\033[0m\" to recentering wheel\n").c_str(), AppStatus.DrivingToggleButtonName.c_str(), AppStatus.DrivingCalibrationButtonName.c_str());
		
		u8printf(T("Layer1_Misc", "\n \033[4mMiscellaneous\033[0m:").c_str());
		u8printf(T("Layer1_Profile", "\n Profile: \"\033[1m%s\033[0m\", press \"\033[1mHome/PS + DPAD Up/Down\033[0m\" or \"\033[1mALT + Up/Down\033[0m\" to change\n").c_str(), XboxProfiles[XboxProfileIndex].substr(0, XboxProfiles[XboxProfileIndex].size() - 4).c_str());
		u8printf(T("Layer1_StickAsTrigger", "\n Press \"\033[1m%s\033[0m\" or \"\033[1mALT + D\033[0m\" to switch Right Stick modes\n").c_str(), AppStatus.RightStickModeButtonName.c_str());
		u8printf(T("Layer1_Battery", "\n Press \"\033[1mALT + I\033[0m\" to view battery status, \"\033[1mALT + Z\033[0m\" to see other hotkeys, \"\033[1mALT + Esc\033[0m\" to Exit\n").c_str());
		//u8printf(T("Layer1_Full_Menu", "\n Press \"\033[1mALT + Z\033[0m\" to open full menu\n").c_str());
		//u8printf(T("Layer1_Exit", "\n Press \"\033[1mALT + Esc\033[0m\" to Exit\n").c_str());

		return;
	}

	u8printf(T("Layer3_Title", "\n \033[4mHotkey & Touchpad Reference Guide\033[0m:\n").c_str());

	// Group: System & Media
	u8printf(T("Layer3_GroupMedia", "\n [System & Media]\n").c_str());
	u8printf(T("Layer3_Volume", "  Volume:          Press \"Capture + Y/A\" or \"PS + \xE2\x96\xA1/\xE2\x97\x8B\" to adjust Windows volume.\n").c_str());
	u8printf(T("Layer3_Screen", "  Screenshots:     Press \"Capture + R\" or \"PS + R1\" to take screenshot (hold to record).\n").c_str());
	u8printf(T("Layer3_Gamebar", "  Xbox Game Bar:   Press \"Capture + Home\" or \"PS\" alone to open Game Bar.\n").c_str());

	// Group: Controller Settings
	u8printf(T("Layer3_GroupSettings", "\n [Controller Settings]\n").c_str());
	//u8printf(T("Layer1_StickAsTrigger", "\n Press \"\033[1m%s\033[0m\" or \"\033[1mALT + C\033[0m\" - Right Stick as Analog Triggers mode (on/off)\n").c_str(), AppStatus.DrivingCalibrationButtonName.c_str());
	u8printf(T("Layer3_AImMode", "  Gyro Behavior:   Press \"ALT + F\" to switching Control button behavior (start/stop motion).\n").c_str());
	u8printf(T("Layer3_Lstick", "  L-Stick Mode:    Press \"PS/HOME + L3\" or \"ALT + S\" to switch Left Stick modes (AutoSprintButton).\n").c_str());
	u8printf(T("Layer3_Rumble", "  Rumble Power:    Press \"Capture + Plus\" or \"PS + Options\" or \"ALT + </>\" to adjust rumble.\n").c_str());
	u8printf(T("Layer3_Calibrate", "  Calibrate:	   Press \"ALT + C\" or \"%s\" to calibrate gyroscope manually.\n").c_str(), AppStatus.HotKeys.GyroCalibrateKeyName.c_str());
	u8printf(T("Layer3_AccelCalibrate", "  Calibrate:	   Press \"ALT + G\" or \"%s\" to calibrate accelerometer manually.\n").c_str(), AppStatus.HotKeys.AccelCalibrateKeyName.c_str());
	u8printf(T("Layer3_Backlight", "  Backlight:       Press \"PS + L1\" or \"ALT + B\" to toggle controller backlight (Sony only).\n").c_str());
	u8printf(T("Layer3_Deadzones", "  Diagnostics:     Press \"ALT + F9\" to view stick and trigger dead zones.\n").c_str());

	// Group: Sony Touchpad Areas
	u8printf(T("Layer3_GroupTouch", "\n [Sony Touchpad Areas]\n").c_str());
	u8printf(T("Layer3_TouchLeft", "  Left Area:       Click/Touch to activate Driving Mode (motion wheel).\n").c_str());
	u8printf(T("Layer3_TouchRight", "  Right Area:      Click/Touch to activate Aiming Mode (gyro motion).\n").c_str());
	u8printf(T("Layer3_TouchCenter", "  Center Area:     Click/Touch to reset to Default Mode (shows battery level).\n").c_str());
	u8printf(T("Layer3_TouchSlide", "  Center-Top Edge: Slide left/right to adjust LED backlight brightness.\n").c_str());
	u8printf(T("Layer3_TouchBottom", "  Center-Bottom:   Click/Touch to switch to Desktop Mode controls.\n").c_str());
}

//void RussianMainText() {
//}

void MainTextUpdate() {
	system("cls");
	//if (AppStatus.Lang == LANG_RUSSIAN)	//@037 loacale больше не юзаем
	//	RussianMainText();
	//else
		DefaultMainText();
	//system("cls"); DefaultMainText();
}

void SwapGamepads()
{
	std::swap(PrimaryGamepad.HidHandle, SecondaryGamepad.HidHandle);
	std::swap(PrimaryGamepad.HidHandle2, SecondaryGamepad.HidHandle2);
	std::swap(PrimaryGamepad.DeviceIndex, SecondaryGamepad.DeviceIndex);
	std::swap(PrimaryGamepad.DeviceIndex2, SecondaryGamepad.DeviceIndex2);
	std::swap(PrimaryGamepad.ControllerType, SecondaryGamepad.ControllerType);
	GamepadSetState(PrimaryGamepad);
	GamepadSetState(SecondaryGamepad);
	MainTextUpdate();

	//@069 Перезагружаем профиль кнопок теперь, когда ControllerType точно известен!
	LoadXboxProfile(XboxProfiles[XboxProfileIndex]);
}

void OpenGamepadByJSL(AdvancedGamepad &Gamepad) {	//@021 RumbleFix3 -возможное устранение "вибрации не на том Joycon", брать точный системный путь устройства из библиотеки JoyShockLibrary
	if (Gamepad.DeviceIndex == -1) return;

	int type = JslGetControllerType(Gamepad.DeviceIndex);
	struct JSL_SETTINGS settings1 = JslGetControllerInfoAndSettings(Gamepad.DeviceIndex);
	std::string path1 = settings1.controllerPath;
	std::string path2 = "";

	if (Gamepad.DeviceIndex2 != -1) {
		struct JSL_SETTINGS settings2 = JslGetControllerInfoAndSettings(Gamepad.DeviceIndex2);
		path2 = settings2.controllerPath;
	}

	// Получаем список всех HID устройств
	struct hid_device_info *devs = hid_enumerate(0x0, 0x0);
	struct hid_device_info *cur_dev = devs;

	while (cur_dev) {
		// Если путь совпал с первым устройством из JSL
		if (path1 == cur_dev->path) {
			if (type == JS_TYPE_JOYCON_RIGHT) {	//@027 ritght всегда в HidHandle2
				if (Gamepad.HidHandle2 == NULL) {
					Gamepad.HidHandle2 = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
					Gamepad.DevicePath2 = cur_dev->path;
					if (Gamepad.HidHandle2) {
						hid_set_nonblocking(Gamepad.HidHandle2, 1);
						Gamepad.ControllerType = NINTENDO_JOYCONS;
						Gamepad.USBConnection = false;
					}
				}
			}
			else if (Gamepad.HidHandle == NULL) {
				Gamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
				Gamepad.DevicePath = cur_dev->path;
				if (Gamepad.HidHandle) {
					hid_set_nonblocking(Gamepad.HidHandle, 1);
					// Настраиваем тип устройства и проверяем Bluetooth
					if (type == JS_TYPE_DS) {
						Gamepad.ControllerType = SONY_DUALSENSE;
						Gamepad.USBConnection = true;
						unsigned char buf[64] = { 0 };
						hid_read_timeout(Gamepad.HidHandle, buf, 64, 100);
						if (buf[0] == 0x31) Gamepad.USBConnection = false;
					}
					else if (type == JS_TYPE_DS4) {
						Gamepad.ControllerType = SONY_DUALSHOCK4;
						Gamepad.USBConnection = true;
						unsigned char checkBT[2] = { 0x02, 0x00 };
						hid_write(Gamepad.HidHandle, checkBT, sizeof(checkBT));
						unsigned char buf[64] = { 0 };
						int bytesRead = hid_read_timeout(Gamepad.HidHandle, buf, sizeof(buf), 100);
						if (bytesRead > 0 && buf[0] == 0x11) Gamepad.USBConnection = false;
					}
					else if (type == JS_TYPE_PRO_CONTROLLER) {
						Gamepad.ControllerType = NINTENDO_SWITCH_PRO;
						Gamepad.RumbleSkipCounter = 300;
						unsigned char buf[64] = { 0x80, 0x01 };
						Gamepad.USBConnection = (hid_write(Gamepad.HidHandle, buf, 2) > 0);
					}
					else if (type == JS_TYPE_JOYCON_LEFT) {
						Gamepad.ControllerType = NINTENDO_JOYCONS;
						Gamepad.USBConnection = false;
					}
				}
			}
			// Если путь совпал с правым джойконом из JSL
		} else if (!path2.empty() && path2 == cur_dev->path && Gamepad.HidHandle2 == NULL) {
			Gamepad.HidHandle2 = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			Gamepad.DevicePath2 = cur_dev->path;
			if (Gamepad.HidHandle2) {
				hid_set_nonblocking(Gamepad.HidHandle2, 1);
				Gamepad.ControllerType = NINTENDO_JOYCONS;
				Gamepad.USBConnection = false;
			}
		}
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);
}

//@068 Функция считает контрольную сумму ТОЛЬКО реальных геймпадов
/*int GetRealControllersSignature() {
	int signature = 0;
	struct hid_device_info *devs = hid_enumerate(0x0, 0x0);
	struct hid_device_info *cur = devs;
	while (cur) {
		// 1. Nintendo (Joy-Con L, Joy-Con R, Switch Pro)
		if (cur->vendor_id == NINTENDO_VENDOR) {
			if (cur->product_id == NINTENDO_JOYCON_L || cur->product_id == NINTENDO_JOYCON_R || cur->product_id == NINTENDO_SWITCH_PRO) {
				signature += (cur->vendor_id + cur->product_id);
			}
		}
		// 2. Sony (DualSense, DS4, Brook)
		else if (cur->vendor_id == SONY_VENDOR) {
			// Если включена эмуляция DS4, виртуальный геймпад ViGEm имеет PID SONY_DS4_USB (0x05C4)!
			// Мы игнорируем его, чтобы не ловить ложные срабатывания от самого себя:
			if (AppStatus.EmulateDS4 && (cur->product_id == SONY_DS4_USB || cur->product_id == SONY_DS4_V2_USB)) {
				// Пропускаем виртуальный контроллер ViGEm
			}
			else if (cur->product_id == SONY_DS4_USB || cur->product_id == SONY_DS4_V2_USB ||
				cur->product_id == SONY_DS4_BT || cur->product_id == SONY_DS4_DONGLE ||
				cur->product_id == SONY_DS5 || cur->product_id == SONY_DS5_EDGE) {
				signature += (cur->vendor_id + cur->product_id);
			}
		}
		// 3. Brook адаптеры
		else if (cur->vendor_id == BROOK_DS4_VENDOR && cur->product_id == BROOK_DS4_USB) {
			signature += (cur->vendor_id + cur->product_id);
		}
		cur = cur->next;
	}
	hid_free_enumeration(devs);
	return signature;
}*/

//@068 Структура для детального анализа подключенного железа
struct ControllersPresence {
	int signature = 0;
	int joyconLCount = 0;
	int joyconRCount = 0;
	int standaloneCount = 0; // DualShock, DualSense, Switch Pro, Brook
};

//@068 Функция детально сканирует, КТО именно сейчас подключен к Windows (смотрит наружу - в Windows)
ControllersPresence GetRealControllersPresence() {
	ControllersPresence res;
	struct hid_device_info *devs = hid_enumerate(0x0, 0x0);
	struct hid_device_info *cur = devs;

	while (cur) {
		// 1. Nintendo
		if (cur->vendor_id == NINTENDO_VENDOR) {
			if (cur->product_id == NINTENDO_JOYCON_L) {
				res.signature += (cur->vendor_id + cur->product_id);
				res.joyconLCount++;
			}
			else if (cur->product_id == NINTENDO_JOYCON_R) {
				res.signature += (cur->vendor_id + cur->product_id);
				res.joyconRCount++;
			}
			else if (cur->product_id == NINTENDO_SWITCH_PRO) {
				res.signature += (cur->vendor_id + cur->product_id);
				res.standaloneCount++;
			}
		}
		// 2. Sony (DualSense, DS4, Brook)
		else if (cur->vendor_id == SONY_VENDOR) {
			if (AppStatus.EmulateDS4 && (cur->product_id == SONY_DS4_USB || cur->product_id == SONY_DS4_V2_USB)) {
				// Пропускаем виртуальный DS4 ViGEm
			}
			else if (cur->product_id == SONY_DS4_USB || cur->product_id == SONY_DS4_V2_USB ||
				cur->product_id == SONY_DS4_BT || cur->product_id == SONY_DS4_DONGLE ||
				cur->product_id == SONY_DS5 || cur->product_id == SONY_DS5_EDGE) {
				res.signature += (cur->vendor_id + cur->product_id);
				res.standaloneCount++;
			}
		}
		else if (cur->vendor_id == BROOK_DS4_VENDOR && cur->product_id == BROOK_DS4_USB) {
			res.signature += (cur->vendor_id + cur->product_id);
			res.standaloneCount++;
		}
		cur = cur->next;
	}
	hid_free_enumeration(devs);
	return res;
}

//@068 Функция считает сигнатуру ТОЛЬКО тех устройств, которые эмулятор открыл в слотах (смотрит внутрь -в эмулятор)
int GetActiveControllersSignature() {
	int signature = 0;

	// Слот 1 (PrimaryGamepad: DualShock, DualSense, Pro Controller или Левый Joy-Con)
	if (PrimaryGamepad.DeviceIndex != -1) {
		int t1 = JslGetControllerType(PrimaryGamepad.DeviceIndex);
		if (t1 == JS_TYPE_JOYCON_LEFT)            signature += (NINTENDO_VENDOR + NINTENDO_JOYCON_L);
		else if (t1 == JS_TYPE_JOYCON_RIGHT)      signature += (NINTENDO_VENDOR + NINTENDO_JOYCON_R);
		else if (t1 == JS_TYPE_PRO_CONTROLLER)    signature += (NINTENDO_VENDOR + NINTENDO_SWITCH_PRO);
		else if (t1 == JS_TYPE_DS)                signature += (SONY_VENDOR + SONY_DS5);
		else if (t1 == JS_TYPE_DS4)               signature += (SONY_VENDOR + SONY_DS4_USB);
	}

	// Слот 2 (PrimaryGamepad: Правый Joy-Con пары)
	if (PrimaryGamepad.DeviceIndex2 != -1) {
		int t2 = JslGetControllerType(PrimaryGamepad.DeviceIndex2);
		if (t2 == JS_TYPE_JOYCON_RIGHT)           signature += (NINTENDO_VENDOR + NINTENDO_JOYCON_R);
		else if (t2 == JS_TYPE_JOYCON_LEFT)       signature += (NINTENDO_VENDOR + NINTENDO_JOYCON_L);
	}

	// Второй игрок (SecondaryGamepad, если включен)
	if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1) {
		int t3 = JslGetControllerType(SecondaryGamepad.DeviceIndex);
		if (t3 == JS_TYPE_JOYCON_LEFT)            signature += (NINTENDO_VENDOR + NINTENDO_JOYCON_L);
		else if (t3 == JS_TYPE_JOYCON_RIGHT)      signature += (NINTENDO_VENDOR + NINTENDO_JOYCON_R);
		else if (t3 == JS_TYPE_PRO_CONTROLLER)    signature += (NINTENDO_VENDOR + NINTENDO_SWITCH_PRO);
		else if (t3 == JS_TYPE_DS)                signature += (SONY_VENDOR + SONY_DS5);
		else if (t3 == JS_TYPE_DS4)               signature += (SONY_VENDOR + SONY_DS4_USB);

		if (SecondaryGamepad.DeviceIndex2 != -1) {
			int t4 = JslGetControllerType(SecondaryGamepad.DeviceIndex2);
			if (t4 == JS_TYPE_JOYCON_RIGHT)       signature += (NINTENDO_VENDOR + NINTENDO_JOYCON_R);
			else if (t4 == JS_TYPE_JOYCON_LEFT)   signature += (NINTENDO_VENDOR + NINTENDO_JOYCON_L);
		}
	}

	return signature;
}

//std::mutex gamepadMutex;
void RefreshDevices() {
	//std::lock_guard<std::mutex> lock(gamepadMutex);
	std::lock_guard<std::mutex> lock(m); //@068 ConnectFix блокируем параллельный поток вибрации Vigem на время переподключения
	if (PrimaryGamepad.HidHandle) hid_close(PrimaryGamepad.HidHandle);
	if (PrimaryGamepad.HidHandle2) hid_close(PrimaryGamepad.HidHandle2);
	if (SecondaryGamepad.HidHandle) hid_close(SecondaryGamepad.HidHandle);
	if (SecondaryGamepad.HidHandle2) hid_close(SecondaryGamepad.HidHandle2);
	PrimaryGamepad.HidHandle = NULL;
	PrimaryGamepad.HidHandle2 = NULL;
	PrimaryGamepad.DeviceIndex = -1;
	PrimaryGamepad.DeviceIndex2 = -1;
	SecondaryGamepad.HidHandle = NULL;
	SecondaryGamepad.HidHandle2 = NULL;
	SecondaryGamepad.DeviceIndex = -1;
	SecondaryGamepad.DeviceIndex2 = -1;
	JslDisconnectAndDisposeAll();	//@068 ConnectFix убиваем фоновые потоки JSL (спам "Not a USB response")

	AppStatus.ControllerCount = JslConnectDevices();
	bool JoyconLeftFound = false;

	int jslHandles[8] = {0};	//@023 ConnectFix2 Получаем реальные "хэндлы" (ID) устройств из JSL, а не цифры
	int actualCount = JslGetConnectedDeviceHandles(jslHandles, 8);

	//First Pass: сalibrate and assign all controllers, EXCEPT the right Joy-Con, to the correct slots + @040 split mode
	for (int i = 0; i < actualCount; i++) {
		int handle = jslHandles[i];
		int ControllerType = JslGetControllerType(handle);

		if (AppStatus.EmulateDS4 && (ControllerType == JS_TYPE_DS4 || ControllerType == JS_TYPE_DS)) { //@044 Skip real Sony gamepads for virtual dinput DS4 controller 
			continue; // skip
		}

		// Calibrate only for real devices
		//JslSetAutomaticCalibration(handle, true);
		JslSetAutomaticCalibration(handle, AppStatus.AutoCalibrationEnabled); //@050 -  нет сартовой автокалибровки при "0"
		JslSetGyroSpace(handle, AppStatus.GyroSpace);	//@032 После bugfix в joyshocklib при "1" оси больше не меняются при скручивании кисти (до 90 градусов)

		// Распределяем по слотам
		if (ControllerType == JS_TYPE_DS || ControllerType == JS_TYPE_DS4 ||
			ControllerType == JS_TYPE_JOYCON_LEFT || ControllerType == JS_TYPE_PRO_CONTROLLER) {

			if (PrimaryGamepad.DeviceIndex == -1) PrimaryGamepad.DeviceIndex = handle;
			else if (SecondaryGamepad.DeviceIndex == -1) SecondaryGamepad.DeviceIndex = handle;
			else // Только два контроллера
				break;

			if (ControllerType == JS_TYPE_JOYCON_LEFT) JoyconLeftFound = true;
		}
	}

	// Second pass: only for right Joy-Con (split / merge)
	for (int i = 0; i < actualCount; i++) {
		int handle = jslHandles[i];
		int ControllerType = JslGetControllerType(handle);
		if (ControllerType != JS_TYPE_JOYCON_RIGHT) continue;

		// Split Mode
		if (AppStatus.SplitJoycons) {
			if (PrimaryGamepad.DeviceIndex == -1) PrimaryGamepad.DeviceIndex = handle;
			else if (SecondaryGamepad.DeviceIndex == -1) SecondaryGamepad.DeviceIndex = handle;
		}

		// Merge Mode
		else {
			if (JoyconLeftFound) {
				if (JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_LEFT) PrimaryGamepad.DeviceIndex2 = handle;
				else SecondaryGamepad.DeviceIndex2 = handle;
			}
			else {
				if (PrimaryGamepad.DeviceIndex == -1) PrimaryGamepad.DeviceIndex = handle;
				else if (SecondaryGamepad.DeviceIndex == -1) SecondaryGamepad.DeviceIndex = handle;
			}
		}
	}

	//Sleep(50);	//Temporarily, it may help not to crash in random cases with BT reset //@022 -fix более не нужен

	// Find first gamepad
	/*GamepadSearch(PrimaryGamepad, "");
	GamepadSetState(PrimaryGamepad);

	if (AppStatus.SecondaryGamepadEnabled && AppStatus.ControllerCount > 1) {
		GamepadSearch(SecondaryGamepad, PrimaryGamepad.DevicePath, PrimaryGamepad.DevicePath2);
		SyncGamepadsWithJSL();
		GamepadSetState(SecondaryGamepad);*/

	OpenGamepadByJSL(PrimaryGamepad);	// Привязываем HID-интерфейсы строго по путям из JoyShockLibrary
	GamepadSetState(PrimaryGamepad);

	if (AppStatus.SecondaryGamepadEnabled && AppStatus.ControllerCount > 1) {
		OpenGamepadByJSL(SecondaryGamepad);
		GamepadSetState(SecondaryGamepad);
	}

	if (AppStatus.ExternalPedalsDInputSearch)
		ExternalPedalsDInputSearch();
	
	PrimaryGamepad.Motion.AngleInitialized = false;	//@045 Сбрасываем инициализацию углов развертывания при каждом переподключении устройств
	PrimaryGamepad.Motion.PitchAngleInitialized = false;
	SecondaryGamepad.Motion.AngleInitialized = false;
	SecondaryGamepad.Motion.PitchAngleInitialized = false;

	PrimaryGamepad.Motion.IsManualCalibrated = false;	//@050 Сбрасываем флаг калибровки
	SecondaryGamepad.Motion.IsManualCalibrated = false;

	//AppStatus.StartupCalibrationFrozen = false;	//@050
	AppStatus.StartupCalibrationFrozen = !AppStatus.AutoCalibrationEnabled;	//@050 -  нет сартовой автокалибровки при "0"
	AppStatus.BTReset = false;

	//@062
	if (PrimaryGamepad.DeviceIndex != -1) JslSetStillnessSettings(PrimaryGamepad.DeviceIndex, g_MaxStillnessError, g_MinStillnessCollectionTime, g_MinStillnessCorrectionTime, g_StillnessCalibrationEaseInTime);
	if (PrimaryGamepad.DeviceIndex2 != -1) JslSetStillnessSettings(PrimaryGamepad.DeviceIndex2, g_MaxStillnessError, g_MinStillnessCollectionTime, g_MinStillnessCorrectionTime, g_StillnessCalibrationEaseInTime);
	if (SecondaryGamepad.DeviceIndex != -1) JslSetStillnessSettings(SecondaryGamepad.DeviceIndex, g_MaxStillnessError, g_MinStillnessCollectionTime, g_MinStillnessCorrectionTime, g_StillnessCalibrationEaseInTime);
	if (SecondaryGamepad.DeviceIndex2 != -1) JslSetStillnessSettings(SecondaryGamepad.DeviceIndex2, g_MaxStillnessError, g_MinStillnessCollectionTime, g_MinStillnessCorrectionTime, g_StillnessCalibrationEaseInTime);

	if (PrimaryGamepad.DeviceIndex != -1) JslSetGravitySettings(PrimaryGamepad.DeviceIndex, g_GravityShakinessMin, g_GravityShakinessMax, g_GravityStillSpeed, g_GravityShakySpeed);
	if (PrimaryGamepad.DeviceIndex2 != -1) JslSetGravitySettings(PrimaryGamepad.DeviceIndex2, g_GravityShakinessMin, g_GravityShakinessMax, g_GravityStillSpeed, g_GravityShakySpeed);
	if (SecondaryGamepad.DeviceIndex != -1) JslSetGravitySettings(SecondaryGamepad.DeviceIndex, g_GravityShakinessMin, g_GravityShakinessMax, g_GravityStillSpeed, g_GravityShakySpeed);
	if (SecondaryGamepad.DeviceIndex2 != -1) JslSetGravitySettings(SecondaryGamepad.DeviceIndex2, g_GravityShakinessMin, g_GravityShakinessMax, g_GravityStillSpeed, g_GravityShakySpeed);

	LoadXboxProfile(XboxProfiles[XboxProfileIndex]); //@069 Перезагружаем профиль кнопок теперь, когда ControllerType точно известен!
	AppStatus.LastControllersSignature = GetActiveControllersSignature(); //@068

	//system("pause"); // ВРЕМЕННЫЙ ДЕБАГ "Для продолжения нажмите любую клавишу . . ." 

	MainTextUpdate();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DEVICECHANGE: // Список оборудования изменился
		if (wParam == DBT_DEVNODES_CHANGED) {
			ControllersPresence presence = GetRealControllersPresence();

			//@077 1. ПОДКЛЮЧЕНИЕ: ровно 200 мс реального времени
			if (presence.signature > AppStatus.LastControllersSignature) {
				AppStatus.DeviceChangeDeadline = GetTickCount64() + 200;
			}
			// 2. ОТКЛЮЧЕНИЕ: ровно 2000 мс для чистого закрытия Bluetooth
			else {
				AppStatus.DeviceChangeDeadline = GetTickCount64() + 2000;
			}
		}
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//@056 Функция возвращает время последнего изменения файла в виде числа
uint64_t GetFileModifiedTime(const std::string& filePath) {
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;
	if (GetFileAttributesExA(filePath.c_str(), GetFileExInfoStandard, &fileInfo)) {
		ULARGE_INTEGER time;
		time.LowPart = fileInfo.ftLastWriteTime.dwLowDateTime;
		time.HighPart = fileInfo.ftLastWriteTime.dwHighDateTime;
		return time.QuadPart;
	}
	return 0; // Если файла нет
}

int main(int argc, char **argv)
{
	SetConsoleTitle("JCAdvance 4.0");
	WindowToCenter();

	SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE); //@075

	bool ForceEnLang = false;
	for (int i = 1; i < __argc; i++)
		if (strcmp(__argv[i], "-en") == 0) {
			ForceEnLang = true;
			break;
		}

	WNDCLASS AppWndClass = {};
	AppWndClass.lpfnWndProc = WindowProc;
	AppWndClass.hInstance = GetModuleHandle(NULL);
	AppWndClass.lpszClassName = "DSAdvanceApp";
	RegisterClass(&AppWndClass);
	HWND AppWindow = CreateWindowEx(0, AppWndClass.lpszClassName, "DSAdvanceApp", 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetModuleHandle(NULL), NULL);
	MSG WindowMsgs = {};

	// Config parameters
	CIniReader IniFile("config.ini");

	std::string selectedLang = IniFile.ReadString("ConfigGUI", "Language", "English");	//@037 Localization throught .ini files
	std::string lowerLang = selectedLang;
	std::transform(lowerLang.begin(), lowerLang.end(), lowerLang.begin(), ::tolower);
	if (!ForceEnLang && lowerLang != "english") {	// УНИВЕРСАЛЬНАЯ динамическая логика определения любого языка!
		AppStatus.LangFile = selectedLang; // Сюда запишется "Spanish", "Russian" и т.д.
	}
	else {
		AppStatus.LangFile = "english";
	}

	AppStatus.HotKeys.ResetKeyName = IniFile.ReadString("SETTINGS", "ResetKey", "PAUSE");
	AppStatus.HotKeys.ResetKey = KeyNameToKeyCode(AppStatus.HotKeys.ResetKeyName);
	AppStatus.HotKeys.OSDKey = KeyNameToKeyCode(IniFile.ReadString("SETTINGS", "OSDKey", "NONE"));		//@060
	AppStatus.HotKeys.GyroCalibrateKeyName = IniFile.ReadString("SETTINGS", "GyroCalibrateKey", "NONE");	//@050	keyboad
	AppStatus.HotKeys.GyroCalibrateKey = KeyNameToKeyCode(AppStatus.HotKeys.GyroCalibrateKeyName);
	AppStatus.GyroCalibrateButtonName = IniFile.ReadString("SETTINGS", "GyroCalibrateButton", "NONE");				//@050 gamepad
	AppStatus.GyroCalibrateButton = SonyNintendoKeyNameToJoyShockKeyCode(AppStatus.GyroCalibrateButtonName);
	AppStatus.HotKeys.AccelCalibrateKeyName = IniFile.ReadString("SETTINGS", "AccelCalibrateKey", "NONE");	//@063
	AppStatus.HotKeys.AccelCalibrateKey = KeyNameToKeyCode(AppStatus.HotKeys.AccelCalibrateKeyName);

	AppStatus.AutoCalibrationEnabled = IniFile.ReadBoolean("SETTINGS", "AutoCalibrationEnabled", true);	//@050
	AppStatus.BackgroundCalibSound = IniFile.ReadBoolean("SETTINGS", "BackgroundCalibSound", false);

	g_KillProcessName = IniFile.ReadString("SETTINGS", "KillProcessName", "");					//@066
	g_KillProcessHotkey = KeyNameToKeyCode(IniFile.ReadString("SETTINGS", "KillHotkey", "NONE"));

	g_MaxStillnessError = IniFile.ReadFloat("JOYCONS", "MaxStillnessError", 2.0f);	//@062
	g_MinStillnessCollectionTime = IniFile.ReadFloat("JOYCONS", "MinStillnessCollectionTime", 0.5f);
	g_MinStillnessCorrectionTime = IniFile.ReadFloat("JOYCONS", "MinStillnessCorrectionTime", 2.0f);
	g_StillnessCalibrationEaseInTime = IniFile.ReadFloat("JOYCONS", "StillnessCalibrationEaseInTime", 3.0f);

	if (PrimaryGamepad.DeviceIndex != -1) JslSetStillnessSettings(PrimaryGamepad.DeviceIndex, g_MaxStillnessError, g_MinStillnessCollectionTime, g_MinStillnessCorrectionTime, g_StillnessCalibrationEaseInTime);
	if (PrimaryGamepad.DeviceIndex2 != -1) JslSetStillnessSettings(PrimaryGamepad.DeviceIndex2, g_MaxStillnessError, g_MinStillnessCollectionTime, g_MinStillnessCorrectionTime, g_StillnessCalibrationEaseInTime);
	if (SecondaryGamepad.DeviceIndex != -1) JslSetStillnessSettings(SecondaryGamepad.DeviceIndex, g_MaxStillnessError, g_MinStillnessCollectionTime, g_MinStillnessCorrectionTime, g_StillnessCalibrationEaseInTime);
	if (SecondaryGamepad.DeviceIndex2 != -1) JslSetStillnessSettings(SecondaryGamepad.DeviceIndex2, g_MaxStillnessError, g_MinStillnessCollectionTime, g_MinStillnessCorrectionTime, g_StillnessCalibrationEaseInTime);

	g_GravityShakinessMin = IniFile.ReadFloat("JOYCONS", "GravityShakinessMin", 0.01f);
	g_GravityShakinessMax = IniFile.ReadFloat("JOYCONS", "GravityShakinessMax", 0.4f);
	g_GravityStillSpeed = IniFile.ReadFloat("JOYCONS", "GravityStillSpeed", 1.0f);
	g_GravityShakySpeed = IniFile.ReadFloat("JOYCONS", "GravityShakySpeed", 0.1f);

	if (PrimaryGamepad.DeviceIndex != -1) JslSetGravitySettings(PrimaryGamepad.DeviceIndex, g_GravityShakinessMin, g_GravityShakinessMax, g_GravityStillSpeed, g_GravityShakySpeed);
	if (PrimaryGamepad.DeviceIndex2 != -1) JslSetGravitySettings(PrimaryGamepad.DeviceIndex2, g_GravityShakinessMin, g_GravityShakinessMax, g_GravityStillSpeed, g_GravityShakySpeed);
	if (SecondaryGamepad.DeviceIndex != -1) JslSetGravitySettings(SecondaryGamepad.DeviceIndex, g_GravityShakinessMin, g_GravityShakinessMax, g_GravityStillSpeed, g_GravityShakySpeed);
	if (SecondaryGamepad.DeviceIndex2 != -1) JslSetGravitySettings(SecondaryGamepad.DeviceIndex2, g_GravityShakinessMin, g_GravityShakinessMax, g_GravityStillSpeed, g_GravityShakySpeed);

	AppStatus.ShowBatteryStatusOnLightBar = IniFile.ReadBoolean("Gamepad", "ShowBatteryStatusOnLightBar", true);
	AppStatus.SleepTimeOut = IniFile.ReadInteger("SETTINGS", "SleepTimeOut", 4);
	timeBeginPeriod(1);
	AppStatus.SkipPollTimeOut = SkipPollTimeOutMS / AppStatus.SleepTimeOut;
	AppStatus.PSReleasedTimeOut = PSReleasedTimeOutMS / AppStatus.SleepTimeOut;
	AppStatus.ButtonCheckTimeOut = ButtonReleasedTimeOutMS / AppStatus.SleepTimeOut;
	AppStatus.FrameTime = AppStatus.SleepTimeOut / 1000.0f;
	AppStatus.GyroSpace = IniFile.ReadInteger("Motion", "GyroSpace", 1);	//@032

	PrimaryGamepad.DefaultModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "DefaultModeColor", "0000ff"));
	PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.DefaultModeColor;
	PrimaryGamepad.DrivingModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "DrivingModeColor", "ff0000"));
	PrimaryGamepad.AimingModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "AimingModeColor", "00ff00"));
	PrimaryGamepad.AimingModeL2Color = WebColorToRGB(IniFile.ReadString("Gamepad", "AimingModeL2Color", "00ffff"));
	PrimaryGamepad.DesktopModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "DesktopModeColor", "ff00ff"));
	PrimaryGamepad.TouchSticksModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "TouchSticksModeColor", "ff00ff"));

	PrimaryGamepad.KMEmu.StickValuePressKey = IniFile.ReadFloat("KEYBOARD-MOUSE", "StickValuePressKey", 0.2f);
	PrimaryGamepad.KMEmu.TriggerValuePressKey = IniFile.ReadFloat("KEYBOARD-MOUSE", "TriggerValuePressKey", 0.2f);
	PrimaryGamepad.KMEmu.JoySensX = IniFile.ReadFloat("KEYBOARD-MOUSE", "JoySensX", 15.0f);
	PrimaryGamepad.KMEmu.JoySensY = IniFile.ReadFloat("KEYBOARD-MOUSE", "JoySensY", 15.0f);

	AppStatus.MicCustomKeyName = IniFile.ReadString("Gamepad", "MicCustomKey", "NONE");
	AppStatus.MicCustomKey = KeyNameToKeyCode(AppStatus.MicCustomKeyName);
	if (AppStatus.MicCustomKey == 0)
		AppStatus.ScreenshotMode = ScreenShotXboxGameBarMode; // If not set, then hide this mode
	else
		AppStatus.ScreenShotKey = AppStatus.MicCustomKey;
	AppStatus.SteamScrKeyName = IniFile.ReadString("Gamepad", "SteamScrKey", "NONE");
	AppStatus.SteamScrKey = KeyNameToKeyCode(AppStatus.SteamScrKeyName);

	AppStatus.SecondaryGamepadEnabled = IniFile.ReadBoolean("SecondaryGamepad", "Enabled", false);
	if (AppStatus.SplitJoycons) {					//@040 Joy-con split Mode
		AppStatus.SecondaryGamepadEnabled = true;
	}

	//@044 Считываем тип эмулируемого геймпада (Xbox или DS4)
	std::string controllerType = IniFile.ReadString("Gamepad", "EmulatedController", "Xbox");
	AppStatus.EmulateDS4 = (controllerType == "DS4" || controllerType == "ds4");

	AppStatus.ExternalPedalsDInputSearch = IniFile.ReadBoolean("ExternalPedals", "DInput", false);
	AppStatus.ExternalPedalsCOMPort = IniFile.ReadInteger("ExternalPedals", "COMPort", 0);

	if (AppStatus.ExternalPedalsDInputSearch) { // Dinput in priority
		//ExternalPedalsDInputSearch();			//	//@034 - нахера 3й раз [Pedals Search] Scanning DirectInput devices ?
	}
	else if (AppStatus.ExternalPedalsCOMPort != 0) {
		char sPortName[32];
		sprintf_s(sPortName, "\\\\.\\COM%d", AppStatus.ExternalPedalsCOMPort);

		hSerial = ::CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (hSerial != INVALID_HANDLE_VALUE && GetLastError() != ERROR_FILE_NOT_FOUND) {

			DCB dcbSerialParams = { 0 };
			dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

			if (GetCommState(hSerial, &dcbSerialParams))
			{
				dcbSerialParams.BaudRate = CBR_115200;
				dcbSerialParams.ByteSize = 8;
				dcbSerialParams.StopBits = ONESTOPBIT;
				dcbSerialParams.Parity = NOPARITY;

				if (SetCommState(hSerial, &dcbSerialParams))
				{
					AppStatus.ExternalPedalsArduinoConnected = true;
					PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
					pArduinoReadThread = new std::thread(ExternalPedalsArduinoRead);
				}
			}
		}
	}

	// Sound for switching profiles
	TCHAR ChangeEmuModeWav[MAX_PATH] = { 0 };
	GetSystemWindowsDirectory(ChangeEmuModeWav, sizeof(ChangeEmuModeWav));
	_tcscat_s(ChangeEmuModeWav, sizeof(ChangeEmuModeWav), _T("\\Media\\Windows Pop-up Blocked.wav"));

	// Search keyboard and mouse profiles
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	/*Find = FindFirstFile("KMProfiles\\*.ini", &ffd);		//@056KMProfiles больше не юзаем
	KMProfiles.push_back("Desktop.ini");
	KMProfiles.push_back("FPS.ini");
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (strcmp(ffd.cFileName, "Desktop.ini") && strcmp(ffd.cFileName, "FPS.ini")) // Already added to the top of the list
				KMProfiles.push_back(ffd.cFileName);
		} while (FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
	}
	LoadKMProfile(KMProfiles[KMProfileIndex]); // Loading a standard keyboard and mouse profile*/

	// Search Xbox profiles
	hFind = FindFirstFile("XboxProfiles\\*.ini", &ffd);
	XboxProfiles.push_back("Default.ini");
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (strcmp(ffd.cFileName, "Default.ini"))
				XboxProfiles.push_back(ffd.cFileName);
		} while (FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
	}

	//@036 for Profiles tab in Config.exe. Smart reading active profile from config.ini. 
	std::string ActiveProfile = IniFile.ReadString("ConfigGUI", "LayoutProfile", "Default.ini");	
	for (size_t i = 0; i < XboxProfiles.size(); i++) {
		if (_stricmp(XboxProfiles[i].c_str(), ActiveProfile.c_str()) == 0) {
			XboxProfileIndex = (int)i; // Синхронизируем индекс с выбранным файлом
			break;
		}
	}

	LoadConfig();	//056
	LoadXboxProfile(XboxProfiles[XboxProfileIndex]); // Loading a standard Xbox profile

	RefreshDevices();

	MOTION_STATE MotionState;
	TOUCH_STATE TouchState;

	const auto client = vigem_alloc();
	auto ret = vigem_connect(client);

	/*const auto x360 = vigem_target_x360_alloc();
	ret = vigem_target_add(client, x360);
	ret = vigem_target_x360_register_notification(client, x360, &notification, (void*)1);
	XUSB_REPORT report;

	const auto client2 = vigem_alloc();
	const auto x3602 = vigem_target_x360_alloc();
	if (AppStatus.SecondaryGamepadEnabled) {
		ret = vigem_connect(client2);
		ret = vigem_target_add(client2, x3602);
		ret = vigem_target_x360_register_notification(client2, x3602, &notification, (void*)2);
	}
	XUSB_REPORT report2;*/

	//@044
	PVIGEM_TARGET x360 = nullptr; // Будет использоваться как базовый таргет ViGEm для геймпада 1
	XUSB_REPORT report;
	DS4_REPORT ds4_report;

#pragma warning(push)
#pragma warning(disable: 4996) // Подавляем ошибку C4996 депрекации вызова ViGEm для геймпада 1

	if (AppStatus.EmulateDS4) {
		x360 = vigem_target_ds4_alloc();
		ret = vigem_target_add(client, x360);
		ret = vigem_target_ds4_register_notification(client, x360, &ds4_notification, (void*)1);
	}
	else {
		x360 = vigem_target_x360_alloc();
		ret = vigem_target_add(client, x360);
		ret = vigem_target_x360_register_notification(client, x360, &notification, (void*)1);
	}

#pragma warning(pop)

	const auto client2 = vigem_alloc();
	PVIGEM_TARGET x3602 = nullptr; // Будет использоваться как базовый таргет ViGEm для геймпада 2
	XUSB_REPORT report2;
	DS4_REPORT ds4_report2;

	if (AppStatus.SecondaryGamepadEnabled) {
		ret = vigem_connect(client2);

#pragma warning(push)
#pragma warning(disable: 4996)

		if (AppStatus.EmulateDS4) {
			x3602 = vigem_target_ds4_alloc();
			ret = vigem_target_add(client2, x3602);
			ret = vigem_target_ds4_register_notification(client2, x3602, &ds4_notification, (void*)2);
		}
		else {
			x3602 = vigem_target_x360_alloc();
			ret = vigem_target_add(client2, x3602);
			ret = vigem_target_x360_register_notification(client2, x3602, &notification, (void*)2);
		}

#pragma warning(pop)
	}

	//float velocityX, velocityY, velocityZ;
	float velocityX = 0.0f, velocityY = 0.0f, velocityZ = 0.0f;	//@033 добавил нолики
	TouchpadTouch FirstTouch, SecondTouch;

	//auto previous_time = std::chrono::high_resolution_clock::now();
	//static DWORD lastTime = GetTickCount();

	uint64_t LastConfigTime = GetFileModifiedTime("config.ini");	//@057 remember the time when the configurations change
	std::string CurrentProfilePath = "XboxProfiles\\" + XboxProfiles[XboxProfileIndex];
	uint64_t LastProfileTime = GetFileModifiedTime(CurrentProfilePath);
	//int HotReloadTimer = 10000 / (AppStatus.SleepTimeOut == 0 ? 1 : AppStatus.SleepTimeOut);	// Таймер, чтобы не дергать Windows слишком часто (10 сек.)
	ULONGLONG nextHotReloadTime = GetTickCount64() + 10000; // Честные 10 секунд по часам //@077

	//@060 Shared Memory for telemetry in OSD (AHK)
	HANDLE hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 256, "JCAdvanceTelemetry");
	float* pTelemetry = nullptr;
	if (hMapFile) pTelemetry = (float*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 256);

	// Shared Memory for RTSS (AIDA64 Hack)	//disable old code for new @139
	/*HANDLE hMapFileAIDA = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 65536, "AIDA64_SensorValues");
	char* pTelemetryAIDA = nullptr;
	if (hMapFileAIDA) pTelemetryAIDA = (char*)MapViewOfFile(hMapFileAIDA, FILE_MAP_ALL_ACCESS, 0, 0, 65536);*/

	while (!(GetAsyncKeyState(VK_LMENU) & 0x8000 && GetAsyncKeyState(VK_ESCAPE) & 0x8000))
	{
		if (PeekMessage(&WindowMsgs, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&WindowMsgs);
			DispatchMessage(&WindowMsgs);
			if (WindowMsgs.message == WM_QUIT) break;
		}

		/*if (AppStatus.DeviceChangeDebounce > 0) {//@022 ConnectFix Умное переподключение. ждем пока Windows закончит спамить событиями отключения
			AppStatus.DeviceChangeDebounce--;
			if (AppStatus.DeviceChangeDebounce == 0) {
				AppStatus.BTReset = true; // Триггерим чистый рефреш
			}
		}*/

		//@077 ex. @068
		if (AppStatus.DeviceChangeDeadline > 0 && GetTickCount64() >= AppStatus.DeviceChangeDeadline) {
			AppStatus.DeviceChangeDeadline = 0;
			int currentSignature = GetRealControllersPresence().signature;

			// Переподключаем ТОЛЬКО если изменились РЕАЛЬНЫЕ геймпады!
			if (currentSignature != AppStatus.LastControllersSignature) {
				AppStatus.BTReset = true;
			}
		}

		// Reset
		//if ((AppStatus.SkipPollCount == 0 && (IsKeyPressed(VK_CONTROL) && IsKeyPressed('R')) || IsKeyPressed(AppStatus.HotKeys.ResetKey)) || AppStatus.BTReset) //@061 fix critical bug when Resetkey=NONE
		if (AppStatus.BTReset || (AppStatus.SkipPollCount == 0 && ((IsKeyPressed(VK_CONTROL) && IsKeyPressed('R')) || (AppStatus.HotKeys.ResetKey != 0 && IsKeyPressed(AppStatus.HotKeys.ResetKey)))))
  		{
			RefreshDevices();
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

		//@057 HotRead ini and apply
		/*if (HotReloadTimer > 0) {
			HotReloadTimer--;
		}
		else {
			// Reset timer to 10 sec.
			HotReloadTimer = 5000 / (AppStatus.SleepTimeOut == 0 ? 1 : AppStatus.SleepTimeOut);*/
		if (GetTickCount64() >= nextHotReloadTime) {	//@077
			nextHotReloadTime = GetTickCount64() + 10000;

			// 1. Check Config.ini
			uint64_t currentConfigTime = GetFileModifiedTime("Config.ini");
			if (currentConfigTime != LastConfigTime && currentConfigTime != 0) {
				LastConfigTime = currentConfigTime;

				LoadConfig();

				u8printf("\n[HOT RELOAD] Config.ini updated!");
				Beep(1200, 100);
			}

			// 2. Check active profile (XboxProfile)
			uint64_t currentProfileTime = GetFileModifiedTime(CurrentProfilePath);
			if (currentProfileTime != LastProfileTime && currentProfileTime != 0) {
				LastProfileTime = currentProfileTime;

				// Remember old values BEFORE loading
				bool oldAimMode = AppStatus.AimMode;
				bool oldAimingByPressingMode = AppStatus.AimingByPressingMode;
				unsigned int oldAimingButton = AppStatus.AimingButton;

				// Loading (new values applying here)
				LoadXboxProfile(XboxProfiles[XboxProfileIndex]);

				// check to see if the modes displayed in the console header have changed
				if (oldAimMode != AppStatus.AimMode || oldAimingByPressingMode != AppStatus.AimingByPressingMode || oldAimingButton != AppStatus.AimingButton) {
					// clear and redraw the console ONLY if actually changed
					MainTextUpdate();
				}

				u8printf("\n[HOT RELOAD] %s updated!", XboxProfiles[XboxProfileIndex].c_str());
				Beep(1500, 100);
			}
		}

		// Swap gamepads
		if (AppStatus.SkipPollCount == 0 && IsKeyPressed(VK_MENU) && IsKeyPressed('V') && AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1) {
			SwapGamepads();
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

		XUSB_REPORT_INIT(&report);
		if (AppStatus.SecondaryGamepadEnabled)
			XUSB_REPORT_INIT(&report2);

		if (AppStatus.ControllerCount < 1) { // We don't process anything during idle time
			if (AppStatus.OsdType == 1) ClearRTSS(); //@075 Мгновенно гасим OSD в игре, если геймпад отключился
			report.sThumbLX = 1; // helps with crash, maybe power saving turns off the controller
			ret = vigem_target_x360_update(client, x360, report); // Vigem always mode only

			if (AppStatus.SecondaryGamepadEnabled) {
				report2.sThumbLX = 1;
				ret = vigem_target_x360_update(client2, x3602, report2);
			}

			//Sleep(AppStatus.SleepTimeOut);
			Sleep(50); // Холостой ход без геймпадов. спим по 50 мс
			continue;
		}

		//@043 Melee Gesture. Объявляем независимые переменные для жестов обоих рук
		MOTION_STATE msL, msR;
		float gyroLX = 0.0f, gyroLY = 0.0f, gyroLZ = 0.0f;
		float gyroRX = 0.0f, gyroRY = 0.0f, gyroRZ = 0.0f;

		//@073 Высокоточный замер реального времени кадра (защита мыши от неровного Sleep Windows)
		static auto lastLoopTime = std::chrono::steady_clock::now();
		auto currentLoopTime = std::chrono::steady_clock::now();
		float loopDeltaTime = std::chrono::duration<float>(currentLoopTime - lastLoopTime).count();
		lastLoopTime = currentLoopTime;
		if (loopDeltaTime <= 0.0f || loopDeltaTime > 0.1f) loopDeltaTime = AppStatus.FrameTime;

		// Primary controller
		if (PrimaryGamepad.DeviceIndex2 == -1) {
			PrimaryGamepad.InputState = JslGetSimpleState(PrimaryGamepad.DeviceIndex);
			MotionState = JslGetMotionState(PrimaryGamepad.DeviceIndex);
			JslGetAndFlushAccumulatedGyro(PrimaryGamepad.DeviceIndex, velocityX, velocityY, velocityZ);

			// На одиночном геймпаде жесты левой/правой руки дублируют друг друга
			msL = MotionState;
			gyroLX = velocityX; gyroLY = velocityY; gyroLZ = velocityZ;
		}
		else { // Split contoller (Joycons)
			PrimaryGamepad.InputState = JslGetSimpleState(PrimaryGamepad.DeviceIndex);
			JOY_SHOCK_STATE tempState = JslGetSimpleState(PrimaryGamepad.DeviceIndex2);

			// Считываем ускорения с обоих контроллеров параллельно
			msL = JslGetMotionState(PrimaryGamepad.DeviceIndex);
			msR = JslGetMotionState(PrimaryGamepad.DeviceIndex2);

			if (AppStatus.GyroFromLeft) {		//@024+@028 gyro левша + joy fix
				// Прицеливание с левого (DeviceIndex)
				JslGetAndFlushAccumulatedGyro(PrimaryGamepad.DeviceIndex, velocityX, velocityY, velocityZ);
				gyroLX = velocityX; gyroLY = velocityY; gyroLZ = velocityZ;

				// Жест удара с правого (DeviceIndex2)
				JslGetAndFlushAccumulatedGyro(PrimaryGamepad.DeviceIndex2, gyroRX, gyroRY, gyroRZ);

				// Назначаем состояние для движения камеры
				MotionState = msL;
			}
			else {
				// Прицеливание с правого (DeviceIndex2)
				JslGetAndFlushAccumulatedGyro(PrimaryGamepad.DeviceIndex2, velocityX, velocityY, velocityZ);
				gyroRX = velocityX; gyroRY = velocityY; gyroRZ = velocityZ;

				// Жест удара с левого (DeviceIndex)
				JslGetAndFlushAccumulatedGyro(PrimaryGamepad.DeviceIndex, gyroLX, gyroLY, gyroLZ);

				// Назначаем состояние для движения камеры
				MotionState = msR;
			}
			PrimaryGamepad.InputState.stickRX = tempState.stickRX;
			PrimaryGamepad.InputState.stickRY = tempState.stickRY;
			PrimaryGamepad.InputState.rTrigger = tempState.rTrigger;
			PrimaryGamepad.InputState.buttons |= tempState.buttons;
		}

		AppStatus.JustPressedButtons = PrimaryGamepad.InputState.buttons & ~AppStatus.PrevGamepadButtons;	//@072
		AppStatus.PrevGamepadButtons = PrimaryGamepad.InputState.buttons;

		//@043 Melee Gesture detection (PUNCH, HOOK, DOWNSTRIKE)
		if (AppStatus.ControllerCount >= 1 && PrimaryGamepad.DeviceIndex != -1) {
			// Уменьшаем кулдаун (таймаут повтора) и таймер удержания кнопки
			/*if (PrimaryGamepad.Motion.GestureXCooldown > 0) {
				PrimaryGamepad.Motion.GestureXCooldown--;
			}
			if (PrimaryGamepad.Motion.GestureXTimer > 0) {
				PrimaryGamepad.Motion.GestureXTimer--;
			}*/

			//@077
			int elapsedFrameMs = (int)roundf(loopDeltaTime * 1000.0f);
			if (elapsedFrameMs < 1) elapsedFrameMs = 1;

			if (PrimaryGamepad.Motion.GestureXCooldown > 0) {
				PrimaryGamepad.Motion.GestureXCooldown -= elapsedFrameMs;
			}
			if (PrimaryGamepad.Motion.GestureXTimer > 0) {
				PrimaryGamepad.Motion.GestureXTimer -= elapsedFrameMs;
			}

			bool isGestureTriggered = false;
			float punchLimit = AppStatus.MeleeGForce * 0.70f;
			float sweepLimit = AppStatus.MeleeGForce * 0.90f;

			if (PrimaryGamepad.DeviceIndex2 == -1) {
				// Одиночный геймпад (Pro Controller / DualSense)
				float absX = abs(msL.accelX);
				float absY = abs(msL.accelY);

				bool isPunch = (msL.accelZ > punchLimit) && (absX < 2.2f) && (absY < 2.2f);
				bool isHook = (absX > sweepLimit) && (abs(gyroLY) > 32.0f);
				bool isDownStrike = (gyroLX < -32.0f) && (absY > sweepLimit);

				isGestureTriggered = isPunch || isHook || isDownStrike;
			}
			else {
				// Раздельные Joy-Con (проверяем оба контроллера одновременно!)

				// 1. Проверка ЛЕВОЙ РУКИ (Left Joy-Con)
				float absXL = abs(msL.accelX);
				float absYL = abs(msL.accelY);
				bool isPunchL = (msL.accelZ > punchLimit) && (absXL < 2.2f) && (absYL < 2.2f);
				bool isHookL = (absXL > sweepLimit) && (abs(gyroLY) > 32.0f);
				bool isDownStrikeL = (gyroLX < -32.0f) && (absYL > sweepLimit);

				// 2. Проверка ПРАВОЙ РУКИ (Right Joy-Con)
				float absXR = abs(msR.accelX);
				float absYR = abs(msR.accelY);
				bool isPunchR = (msR.accelZ > punchLimit) && (absXR < 2.2f) && (absYR < 2.2f);
				bool isHookR = (absXR > sweepLimit) && (abs(gyroRY) > 32.0f);
				bool isDownStrikeR = (gyroRX < -32.0f) && (absYR > sweepLimit);

				// Жест срабатывает, если удар нанесен ЛЮБОЙ рукой
				isGestureTriggered = isPunchL || isHookL || isDownStrikeL || isPunchR || isHookR || isDownStrikeR;
			}

			// Если кулдаун равен нулю и распознан один из ударов
			/*if (PrimaryGamepad.Motion.GestureXCooldown == 0 && isGestureTriggered) {
				PrimaryGamepad.Motion.GestureXTimer = 15;        // Зажимаем назначенную кнопку на 15 кадров (~150 мс)
				PrimaryGamepad.Motion.GestureXCooldown = 40;     // Блокируем повтор на 40 кадров (~400 мс)
			}*/

			//@077
			if (PrimaryGamepad.Motion.GestureXCooldown <= 0 && isGestureTriggered) {
				PrimaryGamepad.Motion.GestureXTimer = 150;        // Честные 150 мс удержания кнопки в игре!
				PrimaryGamepad.Motion.GestureXCooldown = 400;     // Честные 400 мс кулдауна!
			}
		}

		// Secondary controller
		if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1) {
			if (SecondaryGamepad.DeviceIndex2 == -1) {
				SecondaryGamepad.InputState = JslGetSimpleState(SecondaryGamepad.DeviceIndex);
				//MotionState = JslGetMotionState(SecondaryGamepad.DeviceIndex);
				//JslGetAndFlushAccumulatedGyro(SecondaryGamepad.DeviceIndex, velocityX, velocityY, velocityZ);

			// Split contoller (Joycons)
			}
			else {
				SecondaryGamepad.InputState = JslGetSimpleState(SecondaryGamepad.DeviceIndex);
				JOY_SHOCK_STATE tempState = JslGetSimpleState(SecondaryGamepad.DeviceIndex2);
				//MotionState = JslGetMotionState(SecondaryGamepad.DeviceIndex2);
				SecondaryGamepad.InputState.stickRX = tempState.stickRX;
				SecondaryGamepad.InputState.stickRY = tempState.stickRY;
				SecondaryGamepad.InputState.rTrigger = tempState.rTrigger;
				SecondaryGamepad.InputState.buttons |= tempState.buttons;
				//JslGetAndFlushAccumulatedGyro(SecondaryGamepad.DeviceIndex2, velocityX, velocityY, velocityZ);
			}
		}

		//@050 Auto-calibration on/off + indication //@076 переносим вниз в сервисный блок
		/*int aimingHandle = PrimaryGamepad.DeviceIndex;
		if (PrimaryGamepad.DeviceIndex2 != -1 && !AppStatus.GyroFromLeft) {
			aimingHandle = PrimaryGamepad.DeviceIndex2;
		}

		if (aimingHandle != -1) {
			static bool wasSteadyAndConfident = false;
			JSL_AUTO_CALIBRATION autoCal = JslGetAutoCalibrationStatus(aimingHandle);
			bool isSteadyAndConfident = (autoCal.isSteady && autoCal.confidence >= 1.0f);

			if (isSteadyAndConfident && !wasSteadyAndConfident) {

				// 1. Обработка ПЕРВОЙ (стартовой) калибровки при AutoCalibrationEnabled 0 и 1
				if (!AppStatus.StartupCalibrationFrozen) {
					// Если в конфиге калибровка выключена - жестко замораживаем её
					if (!AppStatus.AutoCalibrationEnabled) {
						JslSetAutomaticCalibration(aimingHandle, false);
					}
					AppStatus.StartupCalibrationFrozen = true; // Стартовый этап пройден
					PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);	//success beep
				}

				// 2. Обработка ФОНОВЫХ калибровок
				else if (AppStatus.AutoCalibrationEnabled) {
					if (AppStatus.BackgroundCalibSound) PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
				}
			}
			wasSteadyAndConfident = isSteadyAndConfident;
		}*/

		//Manual calibration by hokey + indication (success/fail)
		/*if (AppStatus.SkipPollCount == 0 && (
			(AppStatus.HotKeys.GyroCalibrateKey != 0 && IsKeyPressed(AppStatus.HotKeys.GyroCalibrateKey)) || (IsKeyPressed(VK_MENU) && IsKeyPressed('C')) // Дублирующий хардкод хоткея ALT + C
			) && !AppStatus.IsManualCalibrating) {*/

		if (AppStatus.SkipPollCount == 0 && (
			(AppStatus.GyroCalibrateButton != 0 && (PrimaryGamepad.InputState.buttons & AppStatus.GyroCalibrateButton) == AppStatus.GyroCalibrateButton)
			|| (AppStatus.HotKeys.GyroCalibrateKey != 0 && IsKeyPressed(AppStatus.HotKeys.GyroCalibrateKey)) || (IsKeyPressed(VK_MENU) && IsKeyPressed('C'))
			) && !AppStatus.IsManualCalibrating) {

			AppStatus.IsManualCalibrating = true;

			// Теперь это не таймер ожидания, а ТАЙМАУТ (5 секунд). Если за 5 сек не найдем ноль - выдадим ошибку.
			//AppStatus.ManualCalibrationTimer = 5000 / (AppStatus.SleepTimeOut == 0 ? 1 : AppStatus.SleepTimeOut);
			static ULONGLONG manualCalibDeadline = 0;
			AppStatus.ManualCalibDeadline = GetTickCount64() + 5000; //@077 Ровно 5 секунд по системным часам

			// Сигнал старта калибровки (низкий тон)
			Beep(800, 150);

			// Сбрасываем старый ноль, чтобы JSL начал замер с чистого листа
			JslResetContinuousCalibration(PrimaryGamepad.DeviceIndex);
			if (PrimaryGamepad.DeviceIndex2 != -1) {
				JslResetContinuousCalibration(PrimaryGamepad.DeviceIndex2);
			}

			// Если фоновая калибровка отключена в конфиге, временно включаем её для замера
			if (!AppStatus.AutoCalibrationEnabled) {
				JslSetAutomaticCalibration(PrimaryGamepad.DeviceIndex, true);
				if (PrimaryGamepad.DeviceIndex2 != -1) JslSetAutomaticCalibration(PrimaryGamepad.DeviceIndex2, true);
			}

			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

		if (AppStatus.IsManualCalibrating) {
			//AppStatus.ManualCalibrationTimer--;	//@077

			velocityX = 0.0f; velocityY = 0.0f; velocityZ = 0.0f;	// Принудительно глушим оси, пока геймпад укладывают на стол

			// Определяем прицельный геймпад для проверки математики
			int aimingHandle = PrimaryGamepad.DeviceIndex;
			if (PrimaryGamepad.DeviceIndex2 != -1 && !AppStatus.GyroFromLeft) {
				aimingHandle = PrimaryGamepad.DeviceIndex2;
			}

			JSL_AUTO_CALIBRATION autoCal = JslGetAutoCalibrationStatus(aimingHandle);
			bool isSuccess = (autoCal.isSteady && autoCal.confidence >= 1.0f);
			//bool isTimeout = (AppStatus.ManualCalibrationTimer <= 0);
			bool isTimeout = (GetTickCount64() >= AppStatus.ManualCalibDeadline);	//@077

			// Если JSL поймал ИДЕАЛЬНЫЙ НОЛЬ (успех) ИЛИ вышло время в 5 секунд (провал)
			if (isSuccess || isTimeout) {

				// Если автокалибровка глобально выключена в конфиге, снова замораживаем её
				if (!AppStatus.AutoCalibrationEnabled) {
					JslSetAutomaticCalibration(PrimaryGamepad.DeviceIndex, false);
					if (PrimaryGamepad.DeviceIndex2 != -1) JslSetAutomaticCalibration(PrimaryGamepad.DeviceIndex2, false);
				}

				AppStatus.IsManualCalibrating = false;

				if (isSuccess) {
					// Success beep
					//Beep(1500, 50); Sleep(50); Beep(1200, 100);
					// Async success beep (без блокировки главного потока ViGEm)
					std::thread([]() {
						Beep(1500, 50);
						Sleep(50);
						Beep(1200, 100);
					}).detach();
					
					//AppStatus.CalibRumbleTimer = 200 / AppStatus.SleepTimeOut; //будет двойной вибро, но для калибровки такое себе
				}
				else {
					// Fail beep (low sound). Геймпад трясли в руках все 5 секунд.
					Beep(300, 400);
				}
			}
		}

		//@063 Software Accelerometer Calibration (ALT + G)
		if (AppStatus.SkipPollCount == 0 && (
			(AppStatus.HotKeys.AccelCalibrateKey != 0 && IsKeyPressed(AppStatus.HotKeys.AccelCalibrateKey)) || (IsKeyPressed(VK_MENU) && IsKeyPressed('G')))) {
			if (PrimaryGamepad.DeviceIndex != -1) {
				JslResetAccelerometerCalibration(PrimaryGamepad.DeviceIndex);
				if (PrimaryGamepad.DeviceIndex2 != -1) JslResetAccelerometerCalibration(PrimaryGamepad.DeviceIndex2);
			}
			if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1) {
				JslResetAccelerometerCalibration(SecondaryGamepad.DeviceIndex);
				if (SecondaryGamepad.DeviceIndex2 != -1) JslResetAccelerometerCalibration(SecondaryGamepad.DeviceIndex2);
			}

			// Асинхронный звуковой сигнал успеха (двойной короткий писк)
			std::thread([]() {
				Beep(1200, 50);
				Sleep(50);
				Beep(1500, 100);
			}).detach();

			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

		//Вибро-помощник
		/*if (AppStatus.CalibRumbleTimer > 0) {
			AppStatus.CalibRumbleTimer--;

			// Вычисляем, сколько реальных миллисекунд осталось до конца таймера
			int timeLeftMs = AppStatus.CalibRumbleTimer * (AppStatus.SleepTimeOut == 0 ? 1 : AppStatus.SleepTimeOut);

			if (timeLeftMs > 150 || (timeLeftMs > 0 && timeLeftMs <= 100)) {

				if (PrimaryGamepad.DeviceIndex2 != -1) {
					// Если раздельные Joy-Con: включаем вибрацию только на прицельном
					if (AppStatus.GyroFromLeft) {
						PrimaryGamepad.OutState.LargeMotor = 255; // Левый
						PrimaryGamepad.OutState.SmallMotor = 0;
					}
					else {
						PrimaryGamepad.OutState.LargeMotor = 0;
						PrimaryGamepad.OutState.SmallMotor = 255; // Правый
					}
				}
				else {
					// Если одиночный геймпад (DualSense, Pro Controller)
					PrimaryGamepad.OutState.SmallMotor = 255;
					PrimaryGamepad.OutState.LargeMotor = 255;
				}

			}
			else {
				PrimaryGamepad.OutState.SmallMotor = 0;
				PrimaryGamepad.OutState.LargeMotor = 0;
			}
			GamepadSetState(PrimaryGamepad);
		}*/

		// Stick dead zones 
		if (AppStatus.SkipPollCount == 0 && IsKeyPressed(VK_MENU) && IsKeyPressed(VK_F9) != 0)
		{
			AppStatus.DeadZoneMode = !AppStatus.DeadZoneMode;
			if (AppStatus.DeadZoneMode == false) MainTextUpdate(); else { system("cls"); printf("\n"); }
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}
		/*if (AppStatus.DeadZoneMode) {	//@076 переносим вниз в сервисный блок
			if (AppStatus.Lang == LANG_RUSSIAN) {
				printf(" Левый стик X=%.2f, ", abs(PrimaryGamepad.InputState.stickLX));
				printf("Y=%.2f | ", abs(PrimaryGamepad.InputState.stickLY));
				printf("Правый стик X=%.2f, ", abs(PrimaryGamepad.InputState.stickRX));
				printf("Y=%.2f | ", abs(PrimaryGamepad.InputState.stickRY));
				printf("Левый триггер=%.2f | ", abs(PrimaryGamepad.InputState.lTrigger));
				printf("Правый триггер=%.2f\n", abs(PrimaryGamepad.InputState.rTrigger));
			} else {
				printf(" Left stick X=%.2f, ", abs(PrimaryGamepad.InputState.stickLX));
				printf("Y=%.2f | ", abs(PrimaryGamepad.InputState.stickLY));
				printf("Right stick X=%.2f, ", abs(PrimaryGamepad.InputState.stickRX));
				printf("Y=%.2f | ", abs(PrimaryGamepad.InputState.stickRY));
				printf("Left trigger=%.2f | ", abs(PrimaryGamepad.InputState.lTrigger));
				printf("Right trigger=%.2f \n", abs(PrimaryGamepad.InputState.rTrigger));
			}
		}*/

		//@007 AimingMode (mouse/stick)
		/*if (AppStatus.SkipPollCount == 0 && ((AppStatus.AimingModeToggleButton != 0 && (PrimaryGamepad.InputState.buttons & AppStatus.AimingModeToggleButton) == AppStatus.AimingModeToggleButton) || (IsKeyPressed(VK_MENU) && IsKeyPressed('A')))) {
			AppStatus.AimMode = !AppStatus.AimMode;
			MainTextUpdate();
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}*/

		if (IsHotkeyTriggered(AppStatus.AimingModeToggleButton, VK_MENU, 'A')) {	//@072
			AppStatus.AimMode = !AppStatus.AimMode;
			MainTextUpdate();
		}

		// Switch modes by pressing or touching
		if (AppStatus.SkipPollCount == 0 && ((IsKeyPressed(VK_MENU) && IsKeyPressed('W')) ||
			((JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS4) &&
			(PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_SHARE))))
		{
			AppStatus.ChangeModesWithClick = !AppStatus.ChangeModesWithClick;
			MainTextUpdate();
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

		// Switch left stick mode
		if (AppStatus.SkipPollCount == 0 && ((IsKeyPressed(VK_MENU) != 0 && IsKeyPressed('S')) || (PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_LCLICK)))
		{
			AppStatus.LeftStickMode++;
			if (AppStatus.LeftStickMode > 2) AppStatus.LeftStickMode = 0; // Зацикливаем: 0, 1, 2

			MainTextUpdate();
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

		if (AppStatus.SkipPollCount == 0 && IsKeyPressed(VK_MENU) && IsKeyPressed('Z')) {	//@025 Switch menu Layer 2/3
			AppStatus.ShowFullMenu = !AppStatus.ShowFullMenu;
			MainTextUpdate();
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}
		
		//@060 OSD hotkey
		/*if (AppStatus.SkipPollCount == 0 && AppStatus.HotKeys.OSDKey != 0 && IsKeyPressed(AppStatus.HotKeys.OSDKey)) {
			AppStatus.IsOsdActive = !AppStatus.IsOsdActive;
			if (AppStatus.IsOsdActive) {
				ShellExecuteA(NULL, "open", "OSD.exe", NULL, NULL, SW_SHOWNORMAL);
			}
			else {
				system("taskkill /IM OSD.exe /F > nul 2>&1");
			}
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}*/

		//@075 Toggle OSD
		if (AppStatus.SkipPollCount == 0 && AppStatus.HotKeys.OSDKey != 0 && IsKeyPressed(AppStatus.HotKeys.OSDKey)) {
			AppStatus.IsOsdActive = !AppStatus.IsOsdActive;

			if (AppStatus.OsdType == 0) {
				if (AppStatus.IsOsdActive) {
					ShellExecuteA(NULL, "open", "OSD.exe", NULL, NULL, SW_SHOWNORMAL);
				}
				else {
					//HWND hOsdWnd = FindWindowA(NULL, "OSD");
					//if (hOsdWnd) PostMessage(hOsdWnd, WM_CLOSE, 0, 0);
					system("taskkill /IM OSD.exe /F > nul 2>&1");
				}
			}
			else {
				if (!AppStatus.IsOsdActive) {
					ClearRTSS();
				}
			}
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

		//@066 KillProcess: Закрытие зависшей игры по хоткею
		if (AppStatus.SkipPollCount == 0 && g_KillProcessHotkey != 0 && IsKeyPressed(g_KillProcessHotkey)) {
			if (!g_KillProcessName.empty()) {
				std::string killCmd = "taskkill /IM \"" + g_KillProcessName + "\" /F > nul 2>&1";
				system(killCmd.c_str());
				Beep(500, 300); // Звуковой сигнал, чтобы знать, что команда отправлена
			}
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

		// Switch screenshot mode
		if (AppStatus.SkipPollCount == 0 && IsKeyPressed(VK_MENU) && IsKeyPressed('X'))
		{
			AppStatus.ScreenshotMode++; if (AppStatus.ScreenshotMode > ScreenShotMaxModes) AppStatus.ScreenshotMode = AppStatus.MicCustomKey == 0 ? ScreenShotXboxGameBarMode : ScreenShotCustomKeyMode;
			if (AppStatus.ScreenshotMode == ScreenShotCustomKeyMode) AppStatus.ScreenShotKey = AppStatus.MicCustomKey;
			else if (AppStatus.ScreenshotMode == ScreenShotXboxGameBarMode) AppStatus.ScreenShotKey = VK_GAMEBAR_SCREENSHOT;
			else if (AppStatus.ScreenshotMode == ScreenShotSteamMode) AppStatus.ScreenShotKey = VK_STEAM_SCREENSHOT;
			else if (AppStatus.ScreenshotMode == ScreenShotMultiMode) AppStatus.ScreenShotKey = VK_MULTI_SCREENSHOT;
			MainTextUpdate();
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

		// Enable or disable lightbar
		if (AppStatus.SkipPollCount == 0 && ((IsKeyPressed(VK_MENU) && IsKeyPressed('B')) || (PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_L)))
		{
			if (PrimaryGamepad.OutState.LEDBrightness == 255) PrimaryGamepad.OutState.LEDBrightness = PrimaryGamepad.DefaultLEDBrightness;
			else {
				if (AppStatus.LockedChangeBrightness == false && PrimaryGamepad.OutState.LEDBrightness > 4) // 5 is the minimum brightness
					PrimaryGamepad.DefaultLEDBrightness = PrimaryGamepad.OutState.LEDBrightness; // Save the new selected value as default
				PrimaryGamepad.OutState.LEDBrightness = 255;
			}
			GamepadSetState(PrimaryGamepad);
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

		// Switch profile
		if (AppStatus.SkipPollCount == 0 && (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse || AppStatus.GamepadEmulationMode == EmuGamepadEnabled))
			if ((PrimaryGamepad.InputState.buttons & JSMASK_PS && (PrimaryGamepad.InputState.buttons & JSMASK_UP || PrimaryGamepad.InputState.buttons & JSMASK_DOWN)) ||
				((IsKeyPressed(VK_MENU) && (IsKeyPressed(VK_UP) || IsKeyPressed(VK_DOWN))) && GetConsoleWindow() == GetForegroundWindow()))
			{
				AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
				if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled) {
					//if (IsKeyPressed(VK_UP) || PrimaryGamepad.InputState.buttons & JSMASK_UP) if (XboxProfileIndex > 0) XboxProfileIndex--; else XboxProfileIndex = XboxProfiles.size() - 1;
					//if (IsKeyPressed(VK_DOWN) || PrimaryGamepad.InputState.buttons & JSMASK_DOWN) if (XboxProfileIndex < XboxProfiles.size() - 1) XboxProfileIndex++; else XboxProfileIndex = 0;
					if (IsKeyPressed(VK_UP) || PrimaryGamepad.InputState.buttons & JSMASK_UP) if (XboxProfileIndex > 0) XboxProfileIndex--; else XboxProfileIndex = static_cast<int>(XboxProfiles.size()) - 1;
					if (IsKeyPressed(VK_DOWN) || PrimaryGamepad.InputState.buttons & JSMASK_DOWN) if (XboxProfileIndex < static_cast<int>(XboxProfiles.size()) - 1) XboxProfileIndex++; else XboxProfileIndex = 0;
					LoadXboxProfile(XboxProfiles[XboxProfileIndex]);

				/*} else {	//dsiable KMProfiles
					if (!AppStatus.IsDesktopMode) { // EmuKeyboardAndMouse game mode
						if (IsKeyPressed(VK_UP) || PrimaryGamepad.InputState.buttons & JSMASK_UP) if (KMProfileIndex > 0) KMProfileIndex--; else KMProfileIndex = KMProfiles.size() - 1;
						if (IsKeyPressed(VK_DOWN) || PrimaryGamepad.InputState.buttons & JSMASK_DOWN) if (KMProfileIndex < KMProfiles.size() - 1) KMProfileIndex++; else KMProfileIndex = 0;
					}
					LoadKMProfile(KMProfiles[KMProfileIndex]);
					KMGameProfileIndex = KMProfileIndex;*/
				}
				
				MainTextUpdate();
				PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
			}

		// Switch external pedals mode
		if (AppStatus.SkipPollCount == 0 && ((IsKeyPressed(VK_MENU) != 0 && IsKeyPressed('E'))))
		{
			if (AppStatus.ExternalPedalsMode == ExPedalsAlwaysRacing)
				AppStatus.ExternalPedalsMode = ExPedalsDependentMode;
			else
				AppStatus.ExternalPedalsMode = ExPedalsAlwaysRacing;
			MainTextUpdate();
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

		// Changing the Rumble strength
		if (AppStatus.SkipPollCount == 0 && IsKeyPressed(VK_MENU) && (IsKeyPressed(VK_OEM_COMMA) || IsKeyPressed(VK_OEM_PERIOD)))
		{
			if (IsKeyPressed(VK_OEM_COMMA) && PrimaryGamepad.RumbleStrength > 0)
				PrimaryGamepad.RumbleStrength -= 10;
			if (IsKeyPressed(VK_OEM_PERIOD) && PrimaryGamepad.RumbleStrength < 100)
				PrimaryGamepad.RumbleStrength += 10;
			MainTextUpdate();
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

		// Switch adaptive triigers mode
		if (AppStatus.SkipPollCount == 0 && IsKeyPressed(VK_MENU) && ((IsKeyPressed('3') || IsKeyPressed('4'))))
		{
			if (IsKeyPressed('3')) {
				PrimaryGamepad.AdaptiveTriggersMode++;
				if (PrimaryGamepad.AdaptiveTriggersMode > ADAPTIVE_TRIGGERS_MODE_MAX)
					PrimaryGamepad.AdaptiveTriggersMode = 0;
			}
			else { //if (IsKeyPressed('4'))
				PrimaryGamepad.AdaptiveTriggersMode--;
				if (PrimaryGamepad.AdaptiveTriggersMode < 0)
					PrimaryGamepad.AdaptiveTriggersMode = ADAPTIVE_TRIGGERS_MODE_MAX;
			}
			if (PrimaryGamepad.AdaptiveTriggersMode > 3)
				PrimaryGamepad.AdaptiveTriggersOutputMode = PrimaryGamepad.AdaptiveTriggersMode - 3; // Output skips "dependent" - "- 3"
			else if (PrimaryGamepad.AdaptiveTriggersMode == 0)
				PrimaryGamepad.AdaptiveTriggersOutputMode = 0;
			else if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_1)
					PrimaryGamepad.AdaptiveTriggersOutputMode = ADAPTIVE_TRIGGERS_PISTOL_MODE;
				else if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_2)
					PrimaryGamepad.AdaptiveTriggersOutputMode = ADAPTIVE_TRIGGERS_AUTOMATIC_MODE;
				else if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_3)
					PrimaryGamepad.AdaptiveTriggersOutputMode = ADAPTIVE_TRIGGERS_RIFLE_MODE;
			GamepadSetState(PrimaryGamepad);

			MainTextUpdate();
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

		bool IsCombinedRumbleChange = false;
		if ((JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS4) &&
			(PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_OPTIONS))
			IsCombinedRumbleChange = true;
		if ((JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_RIGHT || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_PRO_CONTROLLER) &&
			(PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_PLUS))
			IsCombinedRumbleChange = true;
		if (AppStatus.SkipPollCount == 0 && IsCombinedRumbleChange) {
			if (PrimaryGamepad.RumbleStrength == 100)
				PrimaryGamepad.RumbleStrength = 0;
			else
				PrimaryGamepad.RumbleStrength += 10;
			MainTextUpdate();
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

		// Switch modes by touchpad & PS button
		if (JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS4) {

			// Regular controllers with touchpads
			if (AppStatus.ChangeModesWithoutAreas == false) {

				TouchState = JslGetTouchState(PrimaryGamepad.DeviceIndex);

				if (AppStatus.LockChangeBrightness == false && TouchState.t0Down && TouchState.t0Y <= 0.1 && TouchState.t0X > TOUCHPAD_LEFT_AREA && TouchState.t0X < TOUCHPAD_RIGHT_AREA) { // Brightness change
					PrimaryGamepad.OutState.LEDBrightness = 255 - std::clamp((int)((TouchState.t0X - TOUCHPAD_LEFT_AREA - 0.020) * 255 * 4), 0, 255);
					//printf("%5.2f %d\n", (TouchState.t0X - TOUCHPAD_LEFT_AREA - 0.020) * 255 * 4, PrimaryGamepad.GamepadOutState.LEDBrightness);
					GamepadSetState(PrimaryGamepad);
				}

				if ((PrimaryGamepad.InputState.buttons & JSMASK_TOUCHPAD_CLICK && AppStatus.ChangeModesWithClick) || (TouchState.t0Down && AppStatus.ChangeModesWithClick == false)) {

					// [O--] - Driving mode
					if (TouchState.t0X > 0 && TouchState.t0X <= TOUCHPAD_LEFT_AREA && PrimaryGamepad.GamepadActionMode != TouchpadSticksMode && !AppStatus.DisableDriving) {
						PrimaryGamepad.GamepadActionMode = MotionDrivingMode;

						PrimaryGamepad.Motion.OffsetAxisX = atan2f(MotionState.gravX, MotionState.gravZ);
						PrimaryGamepad.Motion.OffsetAxisY = atan2f(MotionState.gravY, MotionState.gravZ);

						//@045 Сбрасываем историю углов развертывания и выключаем прецизионный режим
						PrimaryGamepad.Motion.AngleInitialized = false;
						PrimaryGamepad.Motion.PitchAngleInitialized = false;
						PrimaryGamepad.Motion.IsManualCalibrated = false;

						PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.DrivingModeColor;

						if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_1 || PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_2 || PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_3)
							PrimaryGamepad.AdaptiveTriggersOutputMode = ADAPTIVE_TRIGGERS_CAR_MODE;

						// [-O-] // Default & touch sticks modes
					}
					else if (TouchState.t0X > TOUCHPAD_LEFT_AREA && TouchState.t0X < TOUCHPAD_RIGHT_AREA) {

						// Brightness area
						if (TouchState.t0Y <= 0.1) {

							if (AppStatus.SkipPollCount == 0) {
								AppStatus.BrightnessAreaPressed++;
								if (AppStatus.BrightnessAreaPressed > 1) {
									if (AppStatus.LockedChangeBrightness) {
										if (PrimaryGamepad.OutState.LEDBrightness == 255) PrimaryGamepad.OutState.LEDBrightness = PrimaryGamepad.DefaultLEDBrightness; else PrimaryGamepad.OutState.LEDBrightness = 255;
									}
									else
										AppStatus.LockChangeBrightness = !AppStatus.LockChangeBrightness;
									AppStatus.BrightnessAreaPressed = 0;
								}
								AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
							}
							PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.DefaultModeColor;

							// Default mode
						}
						else if (TouchState.t0Y > 0.1 && TouchState.t0Y < 0.7) {
							PrimaryGamepad.GamepadActionMode = GamepadDefaultMode;
							// Show battery level
							ShowBatteryLevels();
							AppStatus.ShowBatteryStatus = true;
							if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1)
								GamepadSetState(SecondaryGamepad);
							MainTextUpdate();
							//printf(" %d %d\n", PrimaryGamepad.LastLEDBrightness, PrimaryGamepad.GamepadOutState.LEDBrightness);

							if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_1 || PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_2 || PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_3)
								PrimaryGamepad.AdaptiveTriggersOutputMode = 0;

							// Desktop / Touch sticks mode
						}
						else {
							if (PrimaryGamepad.TouchSticksOn) {
								PrimaryGamepad.GamepadActionMode = TouchpadSticksMode;
								PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.TouchSticksModeColor;

							}
							else if (!PrimaryGamepad.SwitchedToDesktopMode && AppStatus.SkipPollCount == 0) {
								PrimaryGamepad.GamepadActionMode = DesktopMode;
								PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.DesktopModeColor;
								if (AppStatus.GamepadEmulationMode != EmuKeyboardAndMouse)
									AppStatus.LastGamepadEmulationMode = AppStatus.GamepadEmulationMode;
								AppStatus.GamepadEmulationMode = EmuKeyboardAndMouse;

								//KMProfileIndex = 0;
								//LoadKMProfile(KMProfiles[0]); // First profile Desktop.ini
								PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
								PrimaryGamepad.SwitchedToDesktopMode = true;
								AppStatus.IsDesktopMode = true;
								MainTextUpdate();
								AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
								//printf("Desktop turn on\n");
							}

							if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_1 || PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_2 || PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_3)
								PrimaryGamepad.AdaptiveTriggersOutputMode = 0;
						}

						// [--O] Aiming mode
					}
					else if (TouchState.t0X > TOUCHPAD_RIGHT_AREA && TouchState.t0X <= 1 && PrimaryGamepad.GamepadActionMode != TouchpadSticksMode && !AppStatus.DisableAiming) {

						// Switch motion aiming mode
						if (AppStatus.SkipPollCount == 0 && TouchState.t0Y < 0.3) {
							PrimaryGamepad.GamepadActionMode = PrimaryGamepad.GamepadActionMode != MotionAimingMode ? MotionAimingMode : MotionAimingModeOnlyPressed;
							PrimaryGamepad.LastMotionAIMMode = PrimaryGamepad.GamepadActionMode;
							AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
						}

						// Motion aiming
						if (TouchState.t0Y >= 0.3 && TouchState.t0Y <= 1) {
							PrimaryGamepad.GamepadActionMode = PrimaryGamepad.LastMotionAIMMode;
							PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.AimingModeColor;
						}

						PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.GamepadActionMode == MotionAimingMode ? PrimaryGamepad.AimingModeColor : PrimaryGamepad.AimingModeL2Color;

						if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_1)
							PrimaryGamepad.AdaptiveTriggersOutputMode = ADAPTIVE_TRIGGERS_PISTOL_MODE;
						else if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_2)
							PrimaryGamepad.AdaptiveTriggersOutputMode = ADAPTIVE_TRIGGERS_AUTOMATIC_MODE;
						else if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_3)
							PrimaryGamepad.AdaptiveTriggersOutputMode = ADAPTIVE_TRIGGERS_RIFLE_MODE;
					}

					// Reset desktop mode and return action mode
					if (!PrimaryGamepad.TouchSticksOn && PrimaryGamepad.GamepadActionMode != DesktopMode && PrimaryGamepad.SwitchedToDesktopMode) {
						AppStatus.GamepadEmulationMode = AppStatus.LastGamepadEmulationMode;
						MainTextUpdate();
						PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
						PrimaryGamepad.SwitchedToDesktopMode = false;
						AppStatus.IsDesktopMode = false;
						//printf("Desktop turn off\n");
					}

					// Reset lock brightness if clicked in another area
					if (!(TouchState.t0Y <= 0.1 && TouchState.t0X > TOUCHPAD_LEFT_AREA && TouchState.t0X < TOUCHPAD_RIGHT_AREA)) {
						AppStatus.BrightnessAreaPressed = 0;
						if (AppStatus.LockChangeBrightness == false) AppStatus.LockChangeBrightness = true;
					}

					GamepadSetState(PrimaryGamepad);
					//printf("current mode = %d\r\n", PrimaryGamepad.GamepadActionMode);
					if (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving && PrimaryGamepad.GamepadActionMode != MotionDrivingMode) AppStatus.XboxGamepadReset = true; // Reset last state
				}

				// Controllers without touchpads (AppStatus.ChangeModesWithoutAreas == true)
			}
			else if ((PrimaryGamepad.InputState.buttons & JSMASK_TOUCHPAD_CLICK) && AppStatus.SkipPollCount == 0) {

				// Aiming & driving
				if (!AppStatus.DisableDriving && !AppStatus.DisableAiming)
					PrimaryGamepad.GamepadActionMode = PrimaryGamepad.GamepadActionMode == PrimaryGamepad.LastMotionAIMMode ? MotionDrivingMode : PrimaryGamepad.LastMotionAIMMode;

				// Aiming & driving disabled
				else if (AppStatus.DisableDriving && AppStatus.DisableAiming) {
					PrimaryGamepad.GamepadActionMode = GamepadDefaultMode;

					// Show battery level
					ShowBatteryLevels();
					AppStatus.ShowBatteryStatus = true;
					GamepadSetState(PrimaryGamepad);
					if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1)
						GamepadSetState(SecondaryGamepad);
					MainTextUpdate();

					// Only aiming
				}
				else if (AppStatus.DisableDriving)
					PrimaryGamepad.GamepadActionMode = PrimaryGamepad.GamepadActionMode == PrimaryGamepad.LastMotionAIMMode ? GamepadDefaultMode : PrimaryGamepad.LastMotionAIMMode;
				// Only driving
				else if (AppStatus.DisableAiming)
					PrimaryGamepad.GamepadActionMode = PrimaryGamepad.GamepadActionMode == MotionDrivingMode ? GamepadDefaultMode : MotionDrivingMode;

				AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
			}

			// GameBar & multi keys
			// PS without any keys
			if (PrimaryGamepad.PSReleasedCount == 0 && PrimaryGamepad.InputState.buttons == JSMASK_PS) { PrimaryGamepad.PSOnlyCheckCount = AppStatus.ButtonCheckTimeOut; PrimaryGamepad.PSOnlyPressed = true; }
			if (PrimaryGamepad.PSOnlyCheckCount > 0) {
				if (PrimaryGamepad.PSOnlyCheckCount == 1 && PrimaryGamepad.PSOnlyPressed)
					PrimaryGamepad.PSReleasedCount = AppStatus.PSReleasedTimeOut; // Timeout to release the PS button and don't execute commands
				PrimaryGamepad.PSOnlyCheckCount--;
				if (PrimaryGamepad.InputState.buttons != JSMASK_PS && PrimaryGamepad.InputState.buttons != 0) { PrimaryGamepad.PSOnlyPressed = false; PrimaryGamepad.PSOnlyCheckCount = 0; }
			}
			if (PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons != JSMASK_PS) PrimaryGamepad.PSReleasedCount = AppStatus.PSReleasedTimeOut; // printf("PS + any button\n"); }
			if (PrimaryGamepad.PSReleasedCount > 0) PrimaryGamepad.PSReleasedCount--;
		}

		if (AppStatus.SkipPollCount == 0 && IsKeyPressed(VK_MENU) && IsKeyPressed('I') && GetConsoleWindow() == GetForegroundWindow())
		{
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
			ShowBatteryLevels();
			GamepadSetState(PrimaryGamepad);
			if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1)
				GamepadSetState(SecondaryGamepad);
			/*GetBatteryInfo(); if (AppStatus.BackOutStateCounter == 0) AppStatus.BackOutStateCounter = 40; // ↑
			if (AppStatus.BackOutStateCounter == 40) {
				PrimaryGamepad.LastLEDBrightness = PrimaryGamepad.OutState.LEDBrightness; // Save on first click (tick)
				if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1)
					SecondaryGamepad.LastLEDBrightness = SecondaryGamepad.OutState.LEDBrightness; // Save on first click (tick)
			}*/
			AppStatus.ShowBatteryStatus = true;
			MainTextUpdate();
		}

		//printf("%5.2f\t%5.2f\r\n", PrimaryGamepad.InputState.stickLX, DeadZoneAxis(PrimaryGamepad.InputState.stickLX, PrimaryGamepad.Sticks.DeadZoneLeftX));
		/*report.sThumbLX = PrimaryGamepad.Sticks.InvertLeftX == false ? DeadZoneAxis(PrimaryGamepad.InputState.stickLX, PrimaryGamepad.Sticks.DeadZoneLeftX) * 32767 : DeadZoneAxis(-PrimaryGamepad.InputState.stickLX, PrimaryGamepad.Sticks.DeadZoneLeftX) * 32767;
		report.sThumbLY = PrimaryGamepad.Sticks.InvertLeftX == false ? DeadZoneAxis(PrimaryGamepad.InputState.stickLY, PrimaryGamepad.Sticks.DeadZoneLeftY) * 32767 : DeadZoneAxis(-PrimaryGamepad.InputState.stickLY, PrimaryGamepad.Sticks.DeadZoneLeftY) * 32767;
		report.sThumbRX = PrimaryGamepad.Sticks.InvertRightX == false ? DeadZoneAxis(PrimaryGamepad.InputState.stickRX, PrimaryGamepad.Sticks.DeadZoneRightX) * 32767 : DeadZoneAxis(-PrimaryGamepad.InputState.stickRX, PrimaryGamepad.Sticks.DeadZoneRightX) * 32767;
		report.sThumbRY = PrimaryGamepad.Sticks.InvertRightY == false ? DeadZoneAxis(PrimaryGamepad.InputState.stickRY, PrimaryGamepad.Sticks.DeadZoneRightY) * 32767 : DeadZoneAxis(-PrimaryGamepad.InputState.stickRY, PrimaryGamepad.Sticks.DeadZoneRightY) * 32767;*/

		float lx = DeadZoneAxis(PrimaryGamepad.InputState.stickLX, PrimaryGamepad.Sticks.DeadZoneLeftX);
		float ly = DeadZoneAxis(PrimaryGamepad.InputState.stickLY, PrimaryGamepad.Sticks.DeadZoneLeftY);
		float rx = DeadZoneAxis(PrimaryGamepad.InputState.stickRX, PrimaryGamepad.Sticks.DeadZoneRightX);
		float ry = DeadZoneAxis(PrimaryGamepad.InputState.stickRY, PrimaryGamepad.Sticks.DeadZoneRightY);

		//@035 Linearity Stick  Применяем искривление линейности (Response Curve)
		/*lx = ApplyLinearity(lx, PrimaryGamepad.Sticks.LinearityLeftX);
		ly = ApplyLinearity(ly, PrimaryGamepad.Sticks.LinearityLeftY);
		rx = ApplyLinearity(rx, PrimaryGamepad.Sticks.LinearityRightX);
		ry = ApplyLinearity(ry, PrimaryGamepad.Sticks.LinearityRightY);*/

		lx = ApplyLinearityFast(lx, PrimaryGamepad.Sticks.p_LeftX);
		ly = ApplyLinearityFast(ly, PrimaryGamepad.Sticks.p_LeftY);
		rx = ApplyLinearityFast(rx, PrimaryGamepad.Sticks.p_RightX);
		ry = ApplyLinearityFast(ry, PrimaryGamepad.Sticks.p_RightY);

		//@065 Left Stick: Сложная (радиальная/эллиптическая) Anti-Deadzone
		if (PrimaryGamepad.Sticks.AntiDeadZoneLeftX > 0.0f || PrimaryGamepad.Sticks.AntiDeadZoneLeftY > 0.0f) {
			float mag = sqrtf(lx * lx + ly * ly);
			if (mag > 0.0001f) {
				float dirX = lx / mag;
				float dirY = ly / mag;
				lx = (dirX * PrimaryGamepad.Sticks.AntiDeadZoneLeftX) + lx * (1.0f - PrimaryGamepad.Sticks.AntiDeadZoneLeftX);
				ly = (dirY * PrimaryGamepad.Sticks.AntiDeadZoneLeftY) + ly * (1.0f - PrimaryGamepad.Sticks.AntiDeadZoneLeftY);
			}
		}

		// Локально проверяем, активен ли гироскоп прямо сейчас
		/*bool isAimActiveCheck = (AppStatus.AimingButton != 0) && (
			(AppStatus.AimingButton == JSMASK_ZL && DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) > 0) ||
			(AppStatus.AimingButton == JSMASK_ZR && DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) > 0) ||
			(PrimaryGamepad.InputState.buttons & AppStatus.AimingButton)
			);
		bool isGyroActiveCheck = (PrimaryGamepad.GamepadActionMode == MotionAimingMode && !isAimActiveCheck) ||
			(PrimaryGamepad.GamepadActionMode == MotionAimingModeOnlyPressed && isAimActiveCheck);

		//@065 Right Stick: Сложная (радиальная/эллиптическая) Anti-Deadzone, применяется здесь ТОЛЬКО если гироскоп ВЫКЛЮЧЕН или тумблер ADZ гироскопа = false
		if (!(isGyroActiveCheck && PrimaryGamepad.Motion.GyroApplyAntiDeadZone) &&
			(PrimaryGamepad.Sticks.AntiDeadZoneRightX > 0.0f || PrimaryGamepad.Sticks.AntiDeadZoneRightY > 0.0f)) {
			float mag = sqrtf(rx * rx + ry * ry);
			if (mag > 0.0001f) {
				//float dirX = rx / mag;
				//float dirY = ry / mag;
				float invMag = 1.0f / mag; //@067 Делаем ВСЕГО ОДНО деление
				float dirX = lx * invMag;  // Заменяем деление на умножение
				float dirY = ly * invMag;  // Заменяем деление на умножение
				rx = (dirX * PrimaryGamepad.Sticks.AntiDeadZoneRightX) + rx * (1.0f - PrimaryGamepad.Sticks.AntiDeadZoneRightX);
				ry = (dirY * PrimaryGamepad.Sticks.AntiDeadZoneRightY) + ry * (1.0f - PrimaryGamepad.Sticks.AntiDeadZoneRightY);
			}
		}*/

		//@078 исправленный @065, были перепутаны l и r. Новая логира работы ADZ при совместной работе стика и гиро стика
		if (PrimaryGamepad.Sticks.AntiDeadZoneRightX > 0.0f || PrimaryGamepad.Sticks.AntiDeadZoneRightY > 0.0f) {
			float mag = sqrtf(rx * rx + ry * ry);
			if (mag > 0.0001f) {
				float invMag = 1.0f / mag;
				float dirX = rx * invMag; // Исправлено: честный rx вместо lx!
				float dirY = ry * invMag; // Исправлено: честный ry вместо ly!
				rx = (dirX * PrimaryGamepad.Sticks.AntiDeadZoneRightX) + rx * (1.0f - PrimaryGamepad.Sticks.AntiDeadZoneRightX);
				ry = (dirY * PrimaryGamepad.Sticks.AntiDeadZoneRightY) + ry * (1.0f - PrimaryGamepad.Sticks.AntiDeadZoneRightY);
			}
		}

		//@041 Смена осей (транспонирование XY)
		if (PrimaryGamepad.Sticks.InvertLeftXY) {
			std::swap(lx, ly);
			lx = -lx; // Инвертируем новую ось X (бывшую Y)
		}
		if (PrimaryGamepad.Sticks.InvertRightXY) {
			std::swap(rx, ry);
			ry = -ry;
		}

		// Передаем значения виртуальному Xbox с учетом инверсии осей
		/*report.sThumbLX = PrimaryGamepad.Sticks.InvertLeftX == false ? lx * 32767 : -lx * 32767;
		report.sThumbLY = PrimaryGamepad.Sticks.InvertLeftY == false ? ly * 32767 : -ly * 32767;
		report.sThumbRX = PrimaryGamepad.Sticks.InvertRightX == false ? rx * 32767 : -rx * 32767;
		report.sThumbRY = PrimaryGamepad.Sticks.InvertRightY == false ? ry * 32767 : -ry * 32767;*/

		float rawLX = PrimaryGamepad.Sticks.InvertLeftX == false ? lx * 32767.0f : -lx * 32767.0f;
		float rawLY = PrimaryGamepad.Sticks.InvertLeftY == false ? ly * 32767.0f : -ly * 32767.0f;
		float rawRX = PrimaryGamepad.Sticks.InvertRightX == false ? rx * 32767.0f : -rx * 32767.0f;
		float rawRY = PrimaryGamepad.Sticks.InvertRightY == false ? ry * 32767.0f : -ry * 32767.0f;

		/*report.sThumbLX = static_cast<SHORT>(std::clamp(static_cast<int>(roundf(rawLX)), -32768, 32767));
		report.sThumbLY = static_cast<SHORT>(std::clamp(static_cast<int>(roundf(rawLY)), -32768, 32767));
		report.sThumbRX = static_cast<SHORT>(std::clamp(static_cast<int>(roundf(rawRX)), -32768, 32767));
		report.sThumbRY = static_cast<SHORT>(std::clamp(static_cast<int>(roundf(rawRY)), -32768, 32767));*/

		report.sThumbLX = static_cast<SHORT>(std::clamp(static_cast<int>(roundf(rawLX)), -PrimaryGamepad.Sticks.MaxLeftStickLimit, PrimaryGamepad.Sticks.MaxLeftStickLimit));	//@074
		report.sThumbLY = static_cast<SHORT>(std::clamp(static_cast<int>(roundf(rawLY)), -PrimaryGamepad.Sticks.MaxLeftStickLimit, PrimaryGamepad.Sticks.MaxLeftStickLimit));
		report.sThumbRX = static_cast<SHORT>(std::clamp(static_cast<int>(roundf(rawRX)), -PrimaryGamepad.Sticks.MaxRightStickLimit, PrimaryGamepad.Sticks.MaxRightStickLimit));
		report.sThumbRY = static_cast<SHORT>(std::clamp(static_cast<int>(roundf(rawRY)), -PrimaryGamepad.Sticks.MaxRightStickLimit, PrimaryGamepad.Sticks.MaxRightStickLimit));

		if (CurrentXboxProfile.SwapSticksAxis) {
			std::swap(report.sThumbLX, report.sThumbRX);
			std::swap(report.sThumbLY, report.sThumbRY);
		}

		int activeRSMode = CurrentXboxProfile.RightStickMode;	//@047 + @049. UPD: Хоткей теперь циклически переключает режимы 0,1,2
		//if (AppStatus.StickAsTriggerEnabled) {
		//	activeRSMode = 1; // Хоткей принудительно переключает в режим аналоговых триггеров
		//}

		if (activeRSMode == 1 || activeRSMode == 2) {
			report.sThumbRX = 0;
			report.sThumbRY = 0;
		}

		// Auto stick pressing when value is exceeded
		/*if (PrimaryGamepad.GamepadActionMode != MotionDrivingMode) { // Exclude driving mode
			if (AppStatus.LeftStickMode != LeftStickDefaultMode && (sqrt(PrimaryGamepad.InputState.stickLX * PrimaryGamepad.InputState.stickLX + PrimaryGamepad.InputState.stickLY * PrimaryGamepad.InputState.stickLY) >= PrimaryGamepad.AutoPressStickValue)) {
				if (AppStatus.LeftStickMode == LeftStickAutoPressMode)
					//report.wButtons |= JSMASK_LCLICK;
					report.wButtons |= CurrentXboxProfile.AutoSprintButton; // <-- ИЗМЕНЕНО: Нажимаем кастомную кнопку спринта (Hold)
				else { // LeftStickPressOnceMode
					if (AppStatus.LeftStickPressOnce == false) {
						//report.wButtons |= JSMASK_LCLICK;
						report.wButtons |= CurrentXboxProfile.AutoSprintButton; // <-- ИЗМЕНЕНО: Нажимаем кастомную кнопку спринта (Click)
						AppStatus.LeftStickPressOnce = true;
						//printf(" LeftStickPressOnce\n");
					}
				}
			} else
				AppStatus.LeftStickPressOnce = false;
		}*/
		
		bool isAutoSprintTriggered = false; //@055 new Auto stick pressing для клавиатуры и Xbox

		if (PrimaryGamepad.GamepadActionMode != MotionDrivingMode) {
			if (AppStatus.LeftStickMode != 0) {
				float finalX = report.sThumbLX / 32767.0f;
				float finalY = report.sThumbLY / 32767.0f;

				if (sqrt(finalX * finalX + finalY * finalY) >= PrimaryGamepad.AutoPressStickValue) {
					if (AppStatus.LeftStickMode == 1) {
						if (finalY > fabs(finalX)) isAutoSprintTriggered = true; // Конус 45 градусов; if (finalY > 0.0f) - Максимально широкая полусфера (Все 180° спереди) 
					}
					else if (AppStatus.LeftStickMode == 2) {
						isAutoSprintTriggered = true; // Во все стороны
					}
				}
			}
		}

		// Если сработало — нажимаем кнопку виртуального Xbox
		if (isAutoSprintTriggered) {
			report.wButtons |= CurrentXboxProfile.AutoSprintButton;
		}

		//@044 Вычисляем, активны ли сейчас педали в качестве аналоговых триггеров Xbox L2/R2
		bool isLeftPedalAnalogActive = AppStatus.ExternalPedalsDInputConnected && (
			AppStatus.ExternalPedalsMode == ExPedalsAlwaysRacing ||
			(AppStatus.ExternalPedalsMode == ExPedalsDependentMode && PrimaryGamepad.GamepadActionMode == MotionDrivingMode)
			);
		bool isRightPedalAnalogActive = AppStatus.ExternalPedalsDInputConnected && (
			AppStatus.ExternalPedalsMode == ExPedalsAlwaysRacing ||
			(AppStatus.ExternalPedalsMode == ExPedalsDependentMode && PrimaryGamepad.GamepadActionMode == MotionDrivingMode)
			);

		// Проверяем, зажаты ли физические триггеры Joy-Con и назначены ли они на LT/RT в профиле
		bool isPhysicalTriggerActiveL = (PrimaryGamepad.ControllerType == NINTENDO_JOYCONS || PrimaryGamepad.ControllerType == NINTENDO_SWITCH_PRO) ?
			(CurrentXboxProfile.ZL == XINPUT_GAMEPAD_LEFT_TRIGGER && DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) != 0) :
			(DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) != 0);

		bool isPhysicalTriggerActiveR = (PrimaryGamepad.ControllerType == NINTENDO_JOYCONS || PrimaryGamepad.ControllerType == NINTENDO_SWITCH_PRO) ?
			(CurrentXboxProfile.ZR == XINPUT_GAMEPAD_RIGHT_TRIGGER && DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) != 0) :
			(DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) != 0);

		//@045 ХОТКЕЙ БЫСТРОЙ РЕКАЛИБРОВКИ ЦЕНТРА
		if (AppStatus.ControllerCount >= 1 && PrimaryGamepad.DeviceIndex != -1 &&
			PrimaryGamepad.GamepadActionMode == MotionDrivingMode &&
			AppStatus.DrivingCalibrationButton != 0 &&
			(PrimaryGamepad.InputState.buttons & AppStatus.DrivingCalibrationButton) == AppStatus.DrivingCalibrationButton)
		{
			// Принудительно калибруем новый центр руля и самолета в текущем положении рук (высокоточный компенсированный метод)
			PrimaryGamepad.Motion.OffsetAxisX = GetCompensatedAngle(MotionState.gravX, MotionState.gravZ, MotionState.gravY);
			PrimaryGamepad.Motion.OffsetAxisY = GetCompensatedAngle(MotionState.gravY, MotionState.gravZ, MotionState.gravX);

			// Переключаем расчет стиков в прецизионный компенсированный режим
			PrimaryGamepad.Motion.IsManualCalibrated = true;

			// Обязательно сбрасываем историю углов развертывания, чтобы начать расчет центра с чистого листа
			PrimaryGamepad.Motion.AngleInitialized = false;
			PrimaryGamepad.Motion.PitchAngleInitialized = false;

			// Проигрываем подтверждающий системный звук
			PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
		}

		//@048 Упорядоченный код для триггеров (все изменения + сохранение оригинального кода для аналогов)
		//1. Подготавливаем входящие аналоговые значения датчиков
		float physLTrigger = PrimaryGamepad.InputState.lTrigger;
		float physRTrigger = PrimaryGamepad.InputState.rTrigger;

		// Для геймпадов Nintendo проверяем переназначение в профиле, замена костылей @008
		if (PrimaryGamepad.ControllerType == NINTENDO_JOYCONS || PrimaryGamepad.ControllerType == NINTENDO_SWITCH_PRO) {
			if (CurrentXboxProfile.ZL != XINPUT_GAMEPAD_LEFT_TRIGGER) {
				physLTrigger = 0.0f; // Если ZL переназначена в профиле, физический курок не нажимается
			}
			if (CurrentXboxProfile.ZR != XINPUT_GAMEPAD_RIGHT_TRIGGER) {
				physRTrigger = 0.0f;
			}
		}

		//report.bLeftTrigger = DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) * 255;
		//report.bRightTrigger = DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) * 255;
		//report.bLeftTrigger = DeadZoneAxis(physLTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) * 255;
		//report.bRightTrigger = DeadZoneAxis(physRTrigger, PrimaryGamepad.Triggers.DeadZoneRight) * 255;
		float rawLT = DeadZoneAxis(physLTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) * 255.0f;
		float rawRT = DeadZoneAxis(physRTrigger, PrimaryGamepad.Triggers.DeadZoneRight) * 255.0f;

		report.bLeftTrigger = static_cast<BYTE>(std::clamp(static_cast<int>(roundf(rawLT)), 0, 255));
		report.bRightTrigger = static_cast<BYTE>(std::clamp(static_cast<int>(roundf(rawRT)), 0, 255));

		// 3. Перехват триггеров педалями (если они подключены и активны)
		if (isLeftPedalAnalogActive) {
			//report.bLeftTrigger = GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal1Axis) / 256;
			report.bLeftTrigger = static_cast<BYTE>(std::clamp(static_cast<int>(GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal1Axis) / 256), 0, 255));
		}
		if (isRightPedalAnalogActive) {
			//report.bRightTrigger = GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal2Axis) / 256;
			report.bRightTrigger = static_cast<BYTE>(std::clamp(static_cast<int>(GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal2Axis) / 256), 0, 255));
		}

		// 4. Перехват триггеров правым аналоговым стиком (Stick-as-Triggers)
		if (activeRSMode == 1) {
			if (ry > 0.05f) {
				//report.bRightTrigger = (BYTE)(ry * 255);
				report.bRightTrigger = static_cast<BYTE>(std::clamp(static_cast<int>(roundf(ry * 255.0f)), 0, 255));
			}
			else if (ry < -0.05f) {
				//report.bLeftTrigger = (BYTE)(-ry * 255);
				report.bLeftTrigger = static_cast<BYTE>(std::clamp(static_cast<int>(roundf(-ry * 255.0f)), 0, 255));
			}
		}

		// External pedals
		if (AppStatus.ExternalPedalsDInputConnected) {
			if (joyGetPosEx(AppStatus.ExternalPedalsJoyIndex, &AppStatus.ExternalPedalsJoyInfo) == JOYERR_NOERROR) {

				// Always racing mode - analog triggers
				if (AppStatus.ExternalPedalsMode == ExPedalsAlwaysRacing) { 
					//if (DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) == 0) {	//@044 во всем меняем if (DeadZoneAxis на if (!isPhysicalTriggerActive
					if (!isPhysicalTriggerActiveL) {
						//report.bLeftTrigger = AppStatus.ExternalPedalsJoyInfo.dwVpos / 256;
						//report.bLeftTrigger = GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal1Axis) / 256;	//@034 во всем блоки заменяем dwVpos dwUpos
						report.bLeftTrigger = static_cast<BYTE>(std::clamp(static_cast<int>(GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal1Axis) / 256), 0, 255));
						PrimaryGamepad.InputState.lTrigger = report.bLeftTrigger / 255.0f;
					}
					//if (DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) == 0) {
					if (!isPhysicalTriggerActiveR) {
						//report.bRightTrigger = AppStatus.ExternalPedalsJoyInfo.dwUpos / 256;
						//report.bRightTrigger = GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal2Axis) / 256;
						report.bRightTrigger = static_cast<BYTE>(std::clamp(static_cast<int>(GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal2Axis) / 256), 0, 255));
						PrimaryGamepad.InputState.rTrigger = report.bRightTrigger / 255.0f;
					}

				// ExPedalsModeDependent
				} else { 
				 // In motion driving mode - analog triggers
					if (PrimaryGamepad.GamepadActionMode == MotionDrivingMode) {
						//if (DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) == 0) {
						if (!isPhysicalTriggerActiveL) {
							//report.bLeftTrigger = AppStatus.ExternalPedalsJoyInfo.dwVpos / 256;
							//report.bLeftTrigger = GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal1Axis) / 256;
							report.bLeftTrigger = static_cast<BYTE>(std::clamp(static_cast<int>(GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal1Axis) / 256), 0, 255));
							PrimaryGamepad.InputState.lTrigger = report.bLeftTrigger / 255.0f;
						}
						//if (DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) == 0) {
						if (!isPhysicalTriggerActiveR) {
							//report.bRightTrigger = AppStatus.ExternalPedalsJoyInfo.dwUpos / 256;
							//report.bRightTrigger = GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal2Axis) / 256;
							report.bRightTrigger = static_cast<BYTE>(std::clamp(static_cast<int>(GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal2Axis) / 256), 0, 255));
							PrimaryGamepad.InputState.rTrigger = report.bRightTrigger / 255.0f;
						}

					} else {
						// Pedal 1
						if (!AppStatus.ExternalPedalsXboxModePedal1Analog) { // Pedal 1 button
							//if (AppStatus.ExternalPedalsJoyInfo.dwVpos > AppStatus.ExternalPedalsValuePress)
							if (GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal1Axis) > AppStatus.ExternalPedalsValuePress)
								if ((PrimaryGamepad.InputState.buttons & AppStatus.ExternalPedalsXboxModePedal1) == 0)
									PrimaryGamepad.InputState.buttons |= AppStatus.ExternalPedalsXboxModePedal1;
						}
						else {
							if (AppStatus.ExternalPedalsXboxModePedal1 == JSMASK_ZL) {
								//if (DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) == 0) {
								if (!isPhysicalTriggerActiveL) {
									//report.bLeftTrigger = AppStatus.ExternalPedalsJoyInfo.dwVpos / 256;
									//report.bLeftTrigger = GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal1Axis) / 256;
									report.bLeftTrigger = static_cast<BYTE>(std::clamp(static_cast<int>(GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal1Axis) / 256), 0, 255));
									PrimaryGamepad.InputState.lTrigger = report.bLeftTrigger / 255.0f;
								}
							}
							else {
								//if (DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) == 0) {
								if (!isPhysicalTriggerActiveR) {
									//report.bRightTrigger = AppStatus.ExternalPedalsJoyInfo.dwVpos / 256;
									//report.bRightTrigger = GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal1Axis) / 256;
									report.bRightTrigger = static_cast<BYTE>(std::clamp(static_cast<int>(GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal1Axis) / 256), 0, 255));
									PrimaryGamepad.InputState.rTrigger = report.bRightTrigger / 255.0f;
								}
							}
						}

						// Pedal 2
						if (!AppStatus.ExternalPedalsXboxModePedal2Analog) { // Pedal 2 button
							//if (AppStatus.ExternalPedalsJoyInfo.dwUpos > AppStatus.ExternalPedalsValuePress)
							if (GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal2Axis) > AppStatus.ExternalPedalsValuePress)
								if ((PrimaryGamepad.InputState.buttons & AppStatus.ExternalPedalsXboxModePedal2) == 0)
									PrimaryGamepad.InputState.buttons |= AppStatus.ExternalPedalsXboxModePedal2;
						}
						else {
							if (AppStatus.ExternalPedalsXboxModePedal2 == JSMASK_ZL) {
								//if (DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) == 0) {
								if (!isPhysicalTriggerActiveL) {
									//report.bLeftTrigger = AppStatus.ExternalPedalsJoyInfo.dwUpos / 256;
									//report.bLeftTrigger = GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal2Axis) / 256;
									report.bLeftTrigger = static_cast<BYTE>(std::clamp(static_cast<int>(GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal2Axis) / 256), 0, 255));
									PrimaryGamepad.InputState.lTrigger = report.bLeftTrigger / 255.0f;
								}
							}
							else {
								//if (DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) == 0) {
								if (!isPhysicalTriggerActiveR) {
									//report.bRightTrigger = AppStatus.ExternalPedalsJoyInfo.dwUpos / 256;
									//report.bRightTrigger = GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal2Axis) / 256;
									report.bRightTrigger = static_cast<BYTE>(std::clamp(static_cast<int>(GetAxisValue(AppStatus.ExternalPedalsJoyInfo, AppStatus.Pedal2Axis) / 256), 0, 255));
									PrimaryGamepad.InputState.rTrigger = report.bRightTrigger / 255.0f;
								}
							}
						}
					}
				}

				// Press buttons
				for (int i = 0; i < 16; ++i)
					if (AppStatus.ExternalPedalsJoyInfo.dwButtons & (1 << (i))) { //JOY_BUTTON1-32

						if (AppStatus.ExternalPedalsButtons[i] == JSMASK_ZL) {
							report.bLeftTrigger = 255;
							PrimaryGamepad.InputState.lTrigger = report.bLeftTrigger / 255.0f;
						}
						else if (AppStatus.ExternalPedalsButtons[i] == JSMASK_ZR) {
							report.bRightTrigger = 255;
							PrimaryGamepad.InputState.rTrigger = report.bRightTrigger / 255.0f;
						}
						else if ((PrimaryGamepad.InputState.buttons & AppStatus.ExternalPedalsButtons[i]) == 0)
							PrimaryGamepad.InputState.buttons |= AppStatus.ExternalPedalsButtons[i];
					}

			} else
				AppStatus.ExternalPedalsDInputConnected = false;

#pragma warning(push)
#pragma warning(disable: 4244)

		} else if (AppStatus.ExternalPedalsArduinoConnected) {
			if (DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) == 0) {
				report.bLeftTrigger = PedalsValues[0] * 255;
				PrimaryGamepad.InputState.lTrigger = report.bLeftTrigger / 255.0f;
			}
			if (DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) == 0) {
				report.bRightTrigger = PedalsValues[1] * 255;
				PrimaryGamepad.InputState.rTrigger = report.bRightTrigger / 255.0f;
			}
		}

#pragma warning(pop)

		//@064 Создаем 32-битный контейнер ЗАРАНЕЕ, чтобы 17-й и 18-й биты (LT/RT) не обрезались для Minus Plus
		DWORD XboxButtons = report.wButtons;

		/*if (JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS4) {
			if (!(PrimaryGamepad.InputState.buttons & JSMASK_PS)) {
				//report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_SHARE ? CurrentXboxProfile.Back : 0;	//@064
				//report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_OPTIONS ? CurrentXboxProfile.Start : 0;
				XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_SHARE ? CurrentXboxProfile.Back : 0;
				XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_OPTIONS ? CurrentXboxProfile.Start : 0;
			}
		}
		else if (JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_PRO_CONTROLLER || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_RIGHT) {
			if (!(PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE) && !(PrimaryGamepad.InputState.buttons & JSMASK_HOME)) { // Защита от протекания кнопок в игру при использовании хоткеев (CAPTURE / HOME)
				//report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_MINUS ? CurrentXboxProfile.Back : 0;	//@064
				//report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_PLUS ? CurrentXboxProfile.Start : 0;
				XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_MINUS ? CurrentXboxProfile.Back : 0;
				XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_PLUS ? CurrentXboxProfile.Start : 0;
			}
		}*/

		//int sleepMs = AppStatus.SleepTimeOut == 0 ? 1 : AppStatus.SleepTimeOut;	//@071
		int sleepMs = (int)roundf(loopDeltaTime * 1000.0f);	//@077
		if (sleepMs < 1) sleepMs = 1;

		bool isBackPressed = false;
		bool isStartPressed = false;

		if (JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS4) {
			if (!(PrimaryGamepad.InputState.buttons & JSMASK_PS)) {
				isBackPressed = (PrimaryGamepad.InputState.buttons & JSMASK_SHARE) != 0;
				isStartPressed = (PrimaryGamepad.InputState.buttons & JSMASK_OPTIONS) != 0;
			}
		}
		else if (JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_PRO_CONTROLLER || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_RIGHT) {
			if (!(PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE) && !(PrimaryGamepad.InputState.buttons & JSMASK_HOME)) { // Защита от протекания кнопок
				isBackPressed = (PrimaryGamepad.InputState.buttons & JSMASK_MINUS) != 0;
				isStartPressed = (PrimaryGamepad.InputState.buttons & JSMASK_PLUS) != 0;
			}
		}

		ProcessSmartButton(isBackPressed, CurrentXboxProfile.Back, CurrentXboxProfile.BackLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.Back, sleepMs, AppStatus.LongPressTimeOut);
		ProcessSmartButton(isStartPressed, CurrentXboxProfile.Start, CurrentXboxProfile.StartLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.Start, sleepMs, AppStatus.LongPressTimeOut);

		//@072 ДЕТЕКТОР ОКНА КОМБО (CHORD WINDOW 50 MS) для Capture/Home
		sleepMs = AppStatus.SleepTimeOut == 0 ? 1 : AppStatus.SleepTimeOut;

		// Проверяем: нажата ли любая ДРУГАЯ кнопка, кроме системных (X, Y, B, A, D-Pad, бамперы и т.д.)?
		bool hasOtherButtonsPressed = (PrimaryGamepad.InputState.buttons & ~(JSMASK_CAPTURE | JSMASK_HOME)) != 0;

		auto ProcessChordModifier = [](bool isPhysPressed, bool hasOtherButtons, int sleepMs, int &chordTimer, bool &comboFired) -> bool {
			if (isPhysPressed) {
				if (hasOtherButtons) {
					comboFired = true; // Засекли комбо! Глушим одиночное действие
				}
				else if (!comboFired) {
					chordTimer += sleepMs;
					if (chordTimer >= 50) return true; // 50 мс прошло, комбо нет -> разрешаем одиночное действие!
				}
			}
			else {
				bool wasQuickSoloTap = (chordTimer > 0 && !comboFired);
				chordTimer = 0;
				comboFired = false;
				if (wasQuickSoloTap) return true; // Разрешаем быстрый тап при отпускании!
			}
			return false;
		};

		static int captureChordTimer = 0;
		static bool captureComboFired = false;
		bool allowCaptureSolo = ProcessChordModifier((PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE) != 0, hasOtherButtonsPressed, sleepMs, captureChordTimer, captureComboFired);

		static int homeChordTimer = 0;
		static bool homeComboFired = false;
		bool allowHomeSolo = ProcessChordModifier((PrimaryGamepad.InputState.buttons & JSMASK_HOME) != 0, hasOtherButtonsPressed, sleepMs, homeChordTimer, homeComboFired);

		unsigned int mappedButtons = PrimaryGamepad.InputState.buttons;		//@051 Новый код модификаторов PS HOME и Capture через mappedButtons
		//if (mappedButtons & JSMASK_PS || mappedButtons & JSMASK_CAPTURE) {	//@064
		if (mappedButtons & JSMASK_PS || mappedButtons & JSMASK_CAPTURE || mappedButtons & JSMASK_HOME) {
			unsigned int hotkeyMask = JSMASK_UP | JSMASK_DOWN | JSMASK_LEFT | JSMASK_RIGHT | JSMASK_N | JSMASK_S | JSMASK_W | JSMASK_E | JSMASK_L | JSMASK_R | JSMASK_LCLICK | JSMASK_RCLICK | JSMASK_SHARE;
			mappedButtons &= ~hotkeyMask; // Стираем кнопки хоткеев из маски для игры
		}

		{ // Mapping standard game buttons using the safe masked layout
			//DWORD XboxButtons = report.wButtons;
			/*XboxButtons |= mappedButtons & JSMASK_L ? CurrentXboxProfile.LeftBumper : 0;
			XboxButtons |= mappedButtons & JSMASK_R ? CurrentXboxProfile.RightBumper : 0;
			XboxButtons |= mappedButtons & JSMASK_LCLICK ? CurrentXboxProfile.LeftStick : 0;
			XboxButtons |= mappedButtons & JSMASK_RCLICK ? CurrentXboxProfile.RightStick : 0;
			XboxButtons |= mappedButtons & JSMASK_UP ? CurrentXboxProfile.DPADUp : 0;
			XboxButtons |= mappedButtons & JSMASK_DOWN ? CurrentXboxProfile.DPADDown : 0;
			XboxButtons |= mappedButtons & JSMASK_LEFT ? CurrentXboxProfile.DPADLeft : 0;
			XboxButtons |= mappedButtons & JSMASK_RIGHT ? CurrentXboxProfile.DPADRight : 0;
			XboxButtons |= mappedButtons & JSMASK_N ? CurrentXboxProfile.Y : 0;
			XboxButtons |= mappedButtons & JSMASK_W ? CurrentXboxProfile.X : 0;
			XboxButtons |= mappedButtons & JSMASK_S ? CurrentXboxProfile.A : 0;
			XboxButtons |= mappedButtons & JSMASK_E ? CurrentXboxProfile.B : 0;*/

			//@071
			ProcessSmartButton((mappedButtons & JSMASK_L) != 0, CurrentXboxProfile.LeftBumper, CurrentXboxProfile.LeftBumperLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.L, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton((mappedButtons & JSMASK_R) != 0, CurrentXboxProfile.RightBumper, CurrentXboxProfile.RightBumperLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.R, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton((mappedButtons & JSMASK_LCLICK) != 0, CurrentXboxProfile.LeftStick, CurrentXboxProfile.LeftStickLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.L3, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton((mappedButtons & JSMASK_RCLICK) != 0, CurrentXboxProfile.RightStick, CurrentXboxProfile.RightStickLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.R3, sleepMs, AppStatus.LongPressTimeOut);

			ProcessSmartButton((mappedButtons & JSMASK_UP) != 0, CurrentXboxProfile.DPADUp, CurrentXboxProfile.DPADUpLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.DPADUp, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton((mappedButtons & JSMASK_DOWN) != 0, CurrentXboxProfile.DPADDown, CurrentXboxProfile.DPADDownLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.DPADDown, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton((mappedButtons & JSMASK_LEFT) != 0, CurrentXboxProfile.DPADLeft, CurrentXboxProfile.DPADLeftLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.DPADLeft, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton((mappedButtons & JSMASK_RIGHT) != 0, CurrentXboxProfile.DPADRight, CurrentXboxProfile.DPADRightLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.DPADRight, sleepMs, AppStatus.LongPressTimeOut);

			ProcessSmartButton((mappedButtons & JSMASK_N) != 0, CurrentXboxProfile.Y, CurrentXboxProfile.YLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.Y, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton((mappedButtons & JSMASK_W) != 0, CurrentXboxProfile.X, CurrentXboxProfile.XLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.X, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton((mappedButtons & JSMASK_S) != 0, CurrentXboxProfile.A, CurrentXboxProfile.ALong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.A, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton((mappedButtons & JSMASK_E) != 0, CurrentXboxProfile.B, CurrentXboxProfile.BLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.B, sleepMs, AppStatus.LongPressTimeOut);

			// Additional buttons
			/*if (PrimaryGamepad.ControllerType == SONY_DUALSENSE) { // Edge
				XboxButtons |= mappedButtons & JSMASK_FNL ? CurrentXboxProfile.DSEdgeL4 : 0;
				XboxButtons |= mappedButtons & JSMASK_FNR ? CurrentXboxProfile.DSEdgeR4 : 0;
			}
			else if (PrimaryGamepad.ControllerType == NINTENDO_JOYCONS) {
				XboxButtons |= mappedButtons & JSMASK_SL ? CurrentXboxProfile.JCSL : 0;
				XboxButtons |= mappedButtons & JSMASK_SR ? CurrentXboxProfile.JCSR : 0;
				XboxButtons |= (mappedButtons & JSMASK_ZL) ? CurrentXboxProfile.ZL : 0;	//@009
				XboxButtons |= (mappedButtons & JSMASK_ZR) ? CurrentXboxProfile.ZR : 0;
				XboxButtons |= mappedButtons & JSMASK_HOME ? CurrentXboxProfile.HOME : 0;
				XboxButtons |= mappedButtons & JSMASK_CAPTURE ? CurrentXboxProfile.CAPTURE : 0;
			}*/

			if (PrimaryGamepad.ControllerType == SONY_DUALSENSE) { // Edge
				ProcessSmartButton((mappedButtons & JSMASK_FNL) != 0, CurrentXboxProfile.DSEdgeL4, CurrentXboxProfile.DSEdgeL4Long, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.DSEdgeL4, sleepMs, AppStatus.LongPressTimeOut);
				ProcessSmartButton((mappedButtons & JSMASK_FNR) != 0, CurrentXboxProfile.DSEdgeR4, CurrentXboxProfile.DSEdgeR4Long, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.DSEdgeR4, sleepMs, AppStatus.LongPressTimeOut);
			}
			else if (PrimaryGamepad.ControllerType == NINTENDO_JOYCONS) {
				ProcessSmartButton((mappedButtons & JSMASK_SL) != 0, CurrentXboxProfile.JCSL, CurrentXboxProfile.JCSLLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.JCSL, sleepMs, AppStatus.LongPressTimeOut);
				ProcessSmartButton((mappedButtons & JSMASK_SR) != 0, CurrentXboxProfile.JCSR, CurrentXboxProfile.JCSRLong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.JCSR, sleepMs, AppStatus.LongPressTimeOut);
				ProcessSmartButton((mappedButtons & JSMASK_ZL) != 0, CurrentXboxProfile.ZL, 0, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.L, sleepMs, AppStatus.LongPressTimeOut);	// ZL/ZR оставляем как есть, для них нет лонг пресса
				ProcessSmartButton((mappedButtons & JSMASK_ZR) != 0, CurrentXboxProfile.ZR, 0, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.R, sleepMs, AppStatus.LongPressTimeOut);
				//ProcessSmartButton((mappedButtons & JSMASK_HOME) != 0, CurrentXboxProfile.HOME, CurrentXboxProfile.HOMELong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.HOME, sleepMs, AppStatus.LongPressTimeOut);
				//ProcessSmartButton((mappedButtons & JSMASK_CAPTURE) != 0, CurrentXboxProfile.CAPTURE, CurrentXboxProfile.CAPTURELong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.CAPTURE, sleepMs, AppStatus.LongPressTimeOut);
				ProcessSmartButton(allowHomeSolo, CurrentXboxProfile.HOME, CurrentXboxProfile.HOMELong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.HOME, sleepMs, AppStatus.LongPressTimeOut);	//@072
				ProcessSmartButton(allowCaptureSolo, CurrentXboxProfile.CAPTURE, CurrentXboxProfile.CAPTURELong, &XboxButtons, nullptr, false, PrimaryGamepad.Trackers.CAPTURE, sleepMs, AppStatus.LongPressTimeOut);
			}

			if (PrimaryGamepad.Motion.GestureXTimer > 0 && CurrentXboxProfile.MeleeGesture != 0) {
				XboxButtons |= CurrentXboxProfile.MeleeGesture;
			}

			if (PrimaryGamepad.Motion.GestureXTimer > 0 && CurrentXboxProfile.MeleeGesture != 0) {	//@043
				XboxButtons |= CurrentXboxProfile.MeleeGesture;
			}

			if (activeRSMode == 2) {	//@049
				float absX = fabs(rx); // Используем fabs для работы с float
				float absY = fabs(ry);

				if (absY >= absX) { // Вертикальное отклонение доминирует
					if (ry > 0.5f) {
						XboxButtons |= CurrentXboxProfile.RightStickUp;
					}
					else if (ry < -0.5f) {
						XboxButtons |= CurrentXboxProfile.RightStickDown;
					}
				}
				else { // Горизонтальная ось доминирует
					if (rx > 0.5f) {
						XboxButtons |= CurrentXboxProfile.RightStickRight;
					}
					else if (rx < -0.5f) {
						XboxButtons |= CurrentXboxProfile.RightStickLeft;
					}
				}
			}
			else if (activeRSMode == 1) {
				// В режиме аналоговых триггеров (1) горизонтальная ось X свободна.
				// Разрешаем использовать её для цифровых кнопок влево/вправо (RS-LEFT / RS-RIGHT)
				float absX = fabs(rx); // Используем fabs для работы с float
				float absY = fabs(ry);

				if (absX > absY) { // Горизонтальное отклонение доминирует над триггерами (осью Y)
					if (rx > 0.5f) {
						XboxButtons |= CurrentXboxProfile.RightStickRight;
					}
					else if (rx < -0.5f) {
						XboxButtons |= CurrentXboxProfile.RightStickLeft;
					}
				}
			}

			// Custom keys
			if (XboxButtons & XINPUT_GAMEPAD_LEFT_TRIGGER) { XboxButtons &= ~XINPUT_GAMEPAD_LEFT_TRIGGER; report.bLeftTrigger = 255; }
			if (XboxButtons & XINPUT_GAMEPAD_RIGHT_TRIGGER) { XboxButtons &= ~XINPUT_GAMEPAD_RIGHT_TRIGGER; report.bRightTrigger = 255; }

			if (XboxButtons & XINPUT_GAMEPAD_LEFT_STICK_UP) { XboxButtons &= ~XINPUT_GAMEPAD_LEFT_STICK_UP; report.sThumbLY = 32767; }
			if (XboxButtons & XINPUT_GAMEPAD_LEFT_STICK_DOWN) { XboxButtons &= ~XINPUT_GAMEPAD_LEFT_STICK_DOWN; report.sThumbLY = -32767; }
			if (XboxButtons & XINPUT_GAMEPAD_LEFT_STICK_LEFT) { XboxButtons &= ~XINPUT_GAMEPAD_LEFT_STICK_LEFT; report.sThumbLX = -32767; }
			if (XboxButtons & XINPUT_GAMEPAD_LEFT_STICK_RIGHT) { XboxButtons &= ~XINPUT_GAMEPAD_LEFT_STICK_RIGHT; report.sThumbLX = 32767; }

			if (XboxButtons & XINPUT_GAMEPAD_RIGHT_STICK_UP) { XboxButtons &= ~XINPUT_GAMEPAD_RIGHT_STICK_UP; report.sThumbRY = 32767; }
			if (XboxButtons & XINPUT_GAMEPAD_RIGHT_STICK_DOWN) { XboxButtons &= ~XINPUT_GAMEPAD_RIGHT_STICK_DOWN; report.sThumbRY = -32767; }
			if (XboxButtons & XINPUT_GAMEPAD_RIGHT_STICK_LEFT) { XboxButtons &= ~XINPUT_GAMEPAD_RIGHT_STICK_LEFT; report.sThumbRX = -32767; }
			if (XboxButtons & XINPUT_GAMEPAD_RIGHT_STICK_RIGHT) { XboxButtons &= ~XINPUT_GAMEPAD_RIGHT_STICK_RIGHT; report.sThumbRX = 32767; }

			if (CurrentXboxProfile.SwapTriggers)
				std::swap(report.bLeftTrigger, report.bRightTrigger);

			report.wButtons = (WORD)XboxButtons;
		}
		// Nintendo controllers buttons: Capture & Home - changing working mode + another controllers (with additional buttons with keyboard emulation)
		//if ((IsKeyPressed(VK_MENU) && IsKeyPressed('1')) || (IsKeyPressed(VK_MENU) && IsKeyPressed('2')) ||		//@038 -Hotkeys for all gamepads
			//JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_PRO_CONTROLLER ||
			//JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_LEFT ||
			//JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_RIGHT) {

		//Driving Mode Hotkey двухкнопочный бинд новый парсинг	//@011
		/*if (AppStatus.SkipPollCount == 0 && ((AppStatus.DrivingToggleButton != 0 && (PrimaryGamepad.InputState.buttons & AppStatus.DrivingToggleButton) == AppStatus.DrivingToggleButton && AppStatus.JoyconChangeModesWithButton == 0) || (IsKeyPressed(VK_MENU) && IsKeyPressed('1')))) {
			//if (AppStatus.GamepadEmulationMode != EmuKeyboardAndMouse) {	// Зачем вообще эта дичь в KMProfiles?? Художнику виднее )
				if (PrimaryGamepad.GamepadActionMode == 1) PrimaryGamepad.GamepadActionMode = GamepadDefaultMode;
				else PrimaryGamepad.GamepadActionMode = MotionDrivingMode;
			//}
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}*/

		if (IsHotkeyTriggered(AppStatus.DrivingToggleButton, VK_MENU, '1')) {	//@072
			if (PrimaryGamepad.GamepadActionMode == 1) PrimaryGamepad.GamepadActionMode = GamepadDefaultMode;
			else PrimaryGamepad.GamepadActionMode = MotionDrivingMode;
		}

		//@012 AimingToggleButton (Gyro on\off ) двухкнопочный bind новый парсинг и AimingByPressingMode (больше не жмем Cature 2 раза) в Config	
		/*if (AppStatus.SkipPollCount == 0 && ((AppStatus.AimingToggleButton != 0 && (PrimaryGamepad.InputState.buttons & AppStatus.AimingToggleButton) == AppStatus.AimingToggleButton && AppStatus.JoyconChangeModesWithButton == 0) || (IsKeyPressed(VK_MENU) && IsKeyPressed('2')))) {
			int targetAimingMode = AppStatus.AimingByPressingMode ? MotionAimingModeOnlyPressed : MotionAimingMode;

			if (PrimaryGamepad.GamepadActionMode == targetAimingMode) {
				PrimaryGamepad.GamepadActionMode = GamepadDefaultMode;
			}
			else {
				PrimaryGamepad.GamepadActionMode = targetAimingMode;
				if (AppStatus.AimingByPressingMode) {
					PrimaryGamepad.LastMotionAIMMode = MotionAimingModeOnlyPressed;
				}
			}
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}*/

		if (IsHotkeyTriggered(AppStatus.AimingToggleButton, VK_MENU, '2')) {	//@072
			int targetAimingMode = AppStatus.AimingByPressingMode ? MotionAimingModeOnlyPressed : MotionAimingMode;
			if (PrimaryGamepad.GamepadActionMode == targetAimingMode) {
				PrimaryGamepad.GamepadActionMode = GamepadDefaultMode;
			}
			else {
				PrimaryGamepad.GamepadActionMode = targetAimingMode;
				if (AppStatus.AimingByPressingMode) {
					PrimaryGamepad.LastMotionAIMMode = MotionAimingModeOnlyPressed;
				}
			}
		}

		//@053 Toggle AimingByPressingMode (Pressing vs Always-on Gyro)
		/*if (AppStatus.SkipPollCount == 0 && (
			(AppStatus.AimingPressModeToggleButton != 0 && (PrimaryGamepad.InputState.buttons & AppStatus.AimingPressModeToggleButton) == AppStatus.AimingPressModeToggleButton && AppStatus.JoyconChangeModesWithButton == 0) || (IsKeyPressed(VK_MENU) && IsKeyPressed('F'))))
		{
			AppStatus.AimingByPressingMode = !AppStatus.AimingByPressingMode;

			// ИСПРАВЛЕНИЕ: Мгновенно обновляем текущий режим геймпада, если прицеливание сейчас активно
			if (PrimaryGamepad.GamepadActionMode == MotionAimingMode || PrimaryGamepad.GamepadActionMode == MotionAimingModeOnlyPressed) {
				PrimaryGamepad.GamepadActionMode = AppStatus.AimingByPressingMode ? MotionAimingModeOnlyPressed : MotionAimingMode;
				PrimaryGamepad.LastMotionAIMMode = PrimaryGamepad.GamepadActionMode;
			}

			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
			PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
			MainTextUpdate();
		}*/

		if (IsHotkeyTriggered(AppStatus.AimingPressModeToggleButton, VK_MENU, 'F')) {	//@072
			AppStatus.AimingByPressingMode = !AppStatus.AimingByPressingMode;
			if (PrimaryGamepad.GamepadActionMode == MotionAimingMode || PrimaryGamepad.GamepadActionMode == MotionAimingModeOnlyPressed) {
				PrimaryGamepad.GamepadActionMode = AppStatus.AimingByPressingMode ? MotionAimingModeOnlyPressed : MotionAimingMode;
				PrimaryGamepad.LastMotionAIMMode = PrimaryGamepad.GamepadActionMode;
			}
			PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
			MainTextUpdate();
		}

		/*if (AppStatus.SkipPollCount == 0 && (	//@047
			(AppStatus.StickAsTriggerToggleButton != 0 && (PrimaryGamepad.InputState.buttons & AppStatus.StickAsTriggerToggleButton) == AppStatus.StickAsTriggerToggleButton) || (IsKeyPressed(VK_MENU) && IsKeyPressed('D')))) {
			AppStatus.StickAsTriggerEnabled = !AppStatus.StickAsTriggerEnabled;
			MainTextUpdate();
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}*/

		/*if (AppStatus.SkipPollCount == 0 && (	//@047 + @049
			(AppStatus.RightStickModeButton != 0 && (PrimaryGamepad.InputState.buttons & AppStatus.RightStickModeButton) == AppStatus.RightStickModeButton) || (IsKeyPressed(VK_MENU) && IsKeyPressed('D'))))
		{
			// 1. Крутим режим по кругу: 0 -> 1 -> 2 -> 0
			//CurrentXboxProfile.RightStickMode++;	//old cycle mode
			//if (CurrentXboxProfile.RightStickMode > 2) CurrentXboxProfile.RightStickMode = 0;
			if (!CurrentXboxProfile.RightStickCycleModes.empty()) {
				CurrentXboxProfile.RightStickCycleIndex = (CurrentXboxProfile.RightStickCycleIndex + 1) % CurrentXboxProfile.RightStickCycleModes.size();
				CurrentXboxProfile.RightStickMode = CurrentXboxProfile.RightStickCycleModes[CurrentXboxProfile.RightStickCycleIndex];
			}

			// 2. Выбираем высоту писка
			int freq = 450; // Низкий тон для режима 0 (default)
			if (CurrentXboxProfile.RightStickMode == 1) freq = 800;       // Средний тон для режима 1 (as triggetrs)
			else if (CurrentXboxProfile.RightStickMode == 2) freq = 1200; // Высокий тон для режима 2 (as buttons)

			// 3. Пищим в микро-потоке (0 мс задержки для эмулятора!)
			std::thread([freq]() {
				Beep(freq, 65);
			}).detach();

			MainTextUpdate();
			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}*/

		if (IsHotkeyTriggered(AppStatus.RightStickModeButton, VK_MENU, 'D')) {	//@072
			if (!CurrentXboxProfile.RightStickCycleModes.empty()) {
				CurrentXboxProfile.RightStickCycleIndex = (CurrentXboxProfile.RightStickCycleIndex + 1) % CurrentXboxProfile.RightStickCycleModes.size();
				CurrentXboxProfile.RightStickMode = CurrentXboxProfile.RightStickCycleModes[CurrentXboxProfile.RightStickCycleIndex];
			}

			int freq = 450;
			if (CurrentXboxProfile.RightStickMode == 1) freq = 800;
			else if (CurrentXboxProfile.RightStickMode == 2) freq = 1200;

			std::thread([freq]() { Beep(freq, 65); }).detach();
			MainTextUpdate();
		}

		// Change modes on one Joycon
		if (AppStatus.SkipPollCount == 0 && AppStatus.JoyconChangeModesWithButton != 0 && (PrimaryGamepad.InputState.buttons & AppStatus.JoyconChangeModesWithButton)) {
			if (PrimaryGamepad.GamepadActionMode == MotionDrivingMode) {

				if (PrimaryGamepad.GamepadActionMode == MotionAimingMode)
				{
					PrimaryGamepad.GamepadActionMode = MotionAimingModeOnlyPressed; PrimaryGamepad.LastMotionAIMMode = MotionAimingModeOnlyPressed;
				}
				else {
					PrimaryGamepad.GamepadActionMode = MotionAimingMode; PrimaryGamepad.LastMotionAIMMode = MotionAimingMode;
				}

			} else
				PrimaryGamepad.GamepadActionMode = MotionDrivingMode;

			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
		}

			// Sony
		if (JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS4) {
			// GameBar & multi keys
			// PS without any keys
			if (PrimaryGamepad.PSReleasedCount == 0 && PrimaryGamepad.InputState.buttons == JSMASK_PS) { PrimaryGamepad.PSOnlyCheckCount = AppStatus.ButtonCheckTimeOut; PrimaryGamepad.PSOnlyPressed = true; }
			if (PrimaryGamepad.PSOnlyCheckCount > 0) {
				if (PrimaryGamepad.PSOnlyCheckCount == 1 && PrimaryGamepad.PSOnlyPressed)
					PrimaryGamepad.PSReleasedCount = AppStatus.PSReleasedTimeOut; // Timeout to release the PS button and don't execute commands
				PrimaryGamepad.PSOnlyCheckCount--;
				if (PrimaryGamepad.InputState.buttons != JSMASK_PS && PrimaryGamepad.InputState.buttons != 0) { PrimaryGamepad.PSOnlyPressed = false; PrimaryGamepad.PSOnlyCheckCount = 0; }
			}
			if (PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons != JSMASK_PS) PrimaryGamepad.PSReleasedCount = AppStatus.PSReleasedTimeOut; // printf("PS + any button\n"); }
			if (PrimaryGamepad.PSReleasedCount > 0) PrimaryGamepad.PSReleasedCount--;
		}
		// Gamebar
		KeyPress(VK_GAMEBAR, (PrimaryGamepad.PSOnlyCheckCount == 1 && PrimaryGamepad.PSOnlyPressed) || (PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_HOME), &PrimaryGamepad.ButtonsStates.PS, false);

		// Volume
		if (JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_PRO_CONTROLLER || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_RIGHT) {
			KeyPress(VK_VOLUME_DOWN2, PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_W, &PrimaryGamepad.ButtonsStates.VolumeDown, false);
			KeyPress(VK_VOLUME_UP2, PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_E, &PrimaryGamepad.ButtonsStates.VolumeUp, false);
		}
		else {
			KeyPress(VK_VOLUME_DOWN2, PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_W, &PrimaryGamepad.ButtonsStates.VolumeDown, false);
			KeyPress(VK_VOLUME_UP2, PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_E, &PrimaryGamepad.ButtonsStates.VolumeUp, false);
		}

		// Screenshot / record key
		bool IsSharePressed = PrimaryGamepad.InputState.buttons & JSMASK_MIC || ((PrimaryGamepad.InputState.buttons & JSMASK_PS || PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE) && PrimaryGamepad.InputState.buttons & JSMASK_R); //@051 Change JSMASK_N to _R  + DualShock 4 & Nintendo
		bool IsScreenshotPressed = false;
		bool IsRecordPressed = false;

		// Microphone (screenshots / record video) detect / Определение нажатия скриншота / записи видео
		if (AppStatus.ScreenshotMode != ScreenShotCustomKeyMode) {

			if (IsSharePressed && PrimaryGamepad.ShareHandled == false && PrimaryGamepad.ShareOnlyCheckCount == 0) {
				PrimaryGamepad.ShareHandled = true;
				PrimaryGamepad.ShareOnlyCheckCount = AppStatus.ButtonCheckTimeOut;
				PrimaryGamepad.ShareCheckUnpressed = false;
				// printf(" Start\n");
			}

			if (PrimaryGamepad.ShareOnlyCheckCount > 0) { // Checking start
				if (IsSharePressed == false) {
					PrimaryGamepad.ShareCheckUnpressed = true;
					PrimaryGamepad.ShareOnlyCheckCount = 1; // Skip timeout if button is released
				}

				if (PrimaryGamepad.ShareOnlyCheckCount == 1) {
					// Screenshot
					if (PrimaryGamepad.ShareCheckUnpressed) {
						IsScreenshotPressed = true;
						//printf(" Screenshot\n");

					// Record
					}
					else {
						IsRecordPressed = true;
						//printf(" Record\n");
						//GamepadOutState.MicLED = PrimaryGamepad.ShareIsRecording ? MIC_LED_PULSE : MIC_LED_OFF; // doesn't work via BT :(
						//GamepadSetState(PrimaryGamepad);
					}
				}

				PrimaryGamepad.ShareOnlyCheckCount--;
			}

			if (!IsSharePressed && PrimaryGamepad.ShareHandled && PrimaryGamepad.ShareOnlyCheckCount == 0)
				PrimaryGamepad.ShareHandled = false;
		}

		//@051 Custom sens (±5 steps with averages recalculation). Removed all "* PrimaryGamepad.Motion.CustomMulSens" from snippet below
		if (AppStatus.SkipPollCount == 0 && (PrimaryGamepad.InputState.buttons & JSMASK_PS || PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE) && PrimaryGamepad.InputState.buttons & JSMASK_N) {
			PrimaryGamepad.Motion.SensX += 0.045f;     // +5 в масштабе INI (5 * 0.009)
			PrimaryGamepad.Motion.SensY += 0.045f;
			PrimaryGamepad.Motion.JoySensX += 0.010f;  // +5 в масштабе INI (5 * 0.002)
			PrimaryGamepad.Motion.JoySensY += 0.010f;

			// Ограничение максимума (250 единиц)
			PrimaryGamepad.Motion.SensX = ClampFloat(PrimaryGamepad.Motion.SensX, 0.09f, 2.25f);
			PrimaryGamepad.Motion.SensY = ClampFloat(PrimaryGamepad.Motion.SensY, 0.09f, 2.25f);
			PrimaryGamepad.Motion.JoySensX = ClampFloat(PrimaryGamepad.Motion.JoySensX, 0.02f, 0.5f);
			PrimaryGamepad.Motion.JoySensY = ClampFloat(PrimaryGamepad.Motion.JoySensY, 0.02f, 0.5f);

			PrimaryGamepad.Motion.SensAvg = (PrimaryGamepad.Motion.SensX + PrimaryGamepad.Motion.SensY) * 0.5f;
			PrimaryGamepad.Motion.JoySensAvg = (PrimaryGamepad.Motion.JoySensX + PrimaryGamepad.Motion.JoySensY) * 0.5f;

			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
			Beep(1200, 100);
			u8printf("\n[Sens Change] Mouse X: %.0f, Y: %.0f | Joy X: %.0f, Y: %.0f",
				PrimaryGamepad.Motion.SensX / 0.009f, PrimaryGamepad.Motion.SensY / 0.009f,
				PrimaryGamepad.Motion.JoySensX / 0.002f, PrimaryGamepad.Motion.JoySensY / 0.002f);
		}
		if (AppStatus.SkipPollCount == 0 && (PrimaryGamepad.InputState.buttons & JSMASK_PS || PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE) && PrimaryGamepad.InputState.buttons & JSMASK_S) {
			PrimaryGamepad.Motion.SensX -= 0.045f;     // -5 в масштабе INI
			PrimaryGamepad.Motion.SensY -= 0.045f;
			PrimaryGamepad.Motion.JoySensX -= 0.010f;  // -5 в масштабе INI
			PrimaryGamepad.Motion.JoySensY -= 0.010f;

			// Ограничение минимума (10 единиц)
			PrimaryGamepad.Motion.SensX = ClampFloat(PrimaryGamepad.Motion.SensX, 0.09f, 2.25f);
			PrimaryGamepad.Motion.SensY = ClampFloat(PrimaryGamepad.Motion.SensY, 0.09f, 2.25f);
			PrimaryGamepad.Motion.JoySensX = ClampFloat(PrimaryGamepad.Motion.JoySensX, 0.02f, 0.5f);
			PrimaryGamepad.Motion.JoySensY = ClampFloat(PrimaryGamepad.Motion.JoySensY, 0.02f, 0.5f);

			PrimaryGamepad.Motion.SensAvg = (PrimaryGamepad.Motion.SensX + PrimaryGamepad.Motion.SensY) * 0.5f;
			PrimaryGamepad.Motion.JoySensAvg = (PrimaryGamepad.Motion.JoySensX + PrimaryGamepad.Motion.JoySensY) * 0.5f;

			AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
			Beep(800, 100);
			u8printf("\n[Sens Change] Mouse X: %.0f, Y: %.0f | Joy X: %.0f, Y: %.0f",
				PrimaryGamepad.Motion.SensX / 0.009f, PrimaryGamepad.Motion.SensY / 0.009f,
				PrimaryGamepad.Motion.JoySensX / 0.002f, PrimaryGamepad.Motion.JoySensY / 0.002f);
		}
		if ((PrimaryGamepad.InputState.buttons & JSMASK_PS || PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE) && PrimaryGamepad.InputState.buttons & JSMASK_RCLICK) {
			if (PrimaryGamepad.Motion.SensX != PrimaryGamepad.Motion.BaseSensX || PrimaryGamepad.Motion.JoySensX != PrimaryGamepad.Motion.BaseJoySensX) {
				PrimaryGamepad.Motion.SensX = PrimaryGamepad.Motion.BaseSensX;
				PrimaryGamepad.Motion.SensY = PrimaryGamepad.Motion.BaseSensY;
				PrimaryGamepad.Motion.JoySensX = PrimaryGamepad.Motion.BaseJoySensX;
				PrimaryGamepad.Motion.JoySensY = PrimaryGamepad.Motion.BaseJoySensY;

				PrimaryGamepad.Motion.SensAvg = (PrimaryGamepad.Motion.SensX + PrimaryGamepad.Motion.SensY) * 0.5f;
				PrimaryGamepad.Motion.JoySensAvg = (PrimaryGamepad.Motion.JoySensX + PrimaryGamepad.Motion.JoySensY) * 0.5f;

				u8printf("\n[Sens Reset] Mouse X: %.0f, Y: %.0f | Joy X: %.0f, Y: %.0f",
					PrimaryGamepad.Motion.SensX / 0.009f, PrimaryGamepad.Motion.SensY / 0.009f,
					PrimaryGamepad.Motion.JoySensX / 0.002f, PrimaryGamepad.Motion.JoySensY / 0.002f);
			}
		}

		// Gamepad modes

		//@038  Проверяем нажатие кнопки прицеливания (поддерживаем аналоговый опрос для ZL/L2 и ZR/R2):
		bool isAimingButtonPressed = (AppStatus.AimingButton != 0) && (
			((AppStatus.AimingButton & JSMASK_ZL) && DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) > 0) ||
			((AppStatus.AimingButton & JSMASK_ZR) && DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) > 0) ||
			(PrimaryGamepad.InputState.buttons & AppStatus.AimingButton)
		);

		//@058 Ratchet delay - a gyro motion delay after release Control button in ms 
		bool isGyroAimingActive = (PrimaryGamepad.GamepadActionMode == MotionAimingMode && !isAimingButtonPressed) ||
			(PrimaryGamepad.GamepadActionMode == MotionAimingModeOnlyPressed && isAimingButtonPressed);

		// СБРОС АМОРТИЗАТОРА: Если прицел сейчас выключен - сбрасываем флаг
		if (!isGyroAimingActive) {
			PrimaryGamepad.Motion.WasGyroActive = false;
		}

		// Motion racing  [O--]
		if (PrimaryGamepad.GamepadActionMode == MotionDrivingMode) {

			// Steering wheel
			if (!PrimaryGamepad.Motion.AircraftEnabled) {
				//report.sThumbLX = (SHORT)(CalcMotionStick(MotionState.gravX, MotionState.gravZ, PrimaryGamepad.Motion.SteeringWheelAngle, PrimaryGamepad.Motion.OffsetAxisX) * 32767);	//@045
				report.sThumbLX = (SHORT)(CalcMotionStick(MotionState.gravX, MotionState.gravZ, MotionState.gravY, PrimaryGamepad.Motion.SteeringWheelAngle, PrimaryGamepad.Motion.OffsetAxisX, PrimaryGamepad.Motion.PrevAngleRad, PrimaryGamepad.Motion.CumulativeOffsetRad, PrimaryGamepad.Motion.AngleInitialized, PrimaryGamepad.Motion.IsManualCalibrated, PrimaryGamepad.Motion.LinearityWheel) * 32767);
			}
			// Aircraft
			/*else {	//not working
				const float InputSize = sqrtf(velocityX * velocityX + velocityY * velocityY + velocityZ * velocityZ);
				float tighteningFactor = 1.0f;

				// Вычисляем коэффициент вязкости
				if (PrimaryGamepad.Motion.Tightening > 0.0f && InputSize < PrimaryGamepad.Motion.Tightening) {
					tighteningFactor = InputSize / PrimaryGamepad.Motion.Tightening;
				}

				// Применяем вязкость к оси Y
				float effVelY = velocityY * tighteningFactor;

				report.sThumbLX = std::clamp((int)(ClampFloat(-(effVelY * PrimaryGamepad.Motion.AircraftRollSens * AppStatus.FrameTime * PrimaryGamepad.Motion.JoySensX), -1.0f, 1.0f) * 32767 + report.sThumbLX), -32767, 32767);
				report.sThumbLY = (SHORT)(CalcMotionStick(MotionState.gravY, MotionState.gravZ, MotionState.gravX, PrimaryGamepad.Motion.AircraftPitchAngle, PrimaryGamepad.Motion.OffsetAxisY, PrimaryGamepad.Motion.PitchPrevAngleRad, PrimaryGamepad.Motion.PitchCumulativeOffsetRad, PrimaryGamepad.Motion.PitchAngleInitialized, PrimaryGamepad.Motion.IsManualCalibrated, 50.0f) * 32767) * PrimaryGamepad.Motion.AircraftPitchInverted;
			}*/

		}
		
		//@039 Always меняем на button not pressed
		else if ((PrimaryGamepad.GamepadActionMode == MotionAimingMode && !isAimingButtonPressed) || (PrimaryGamepad.GamepadActionMode == MotionAimingModeOnlyPressed && isAimingButtonPressed)) {

			// Snippet by JibbSmart https://gist.github.com/JibbSmart/8cbaba568c1c2e1193771459aa5385df

			//@028 + @054 + @058 + @073: Tightened в config + EMA Filter + Button relase delay + parametric gyro acceleration 
			if (!PrimaryGamepad.Motion.WasGyroActive) {	//start Ratchetdelay 
				PrimaryGamepad.Motion.WasGyroActive = true;

				//@058 Включаем delay для MotionAimingMode
				if (PrimaryGamepad.GamepadActionMode == MotionAimingMode) {
					//PrimaryGamepad.Motion.RatchetDelayMaxTimer = PrimaryGamepad.Motion.RatchetDelayTime / (AppStatus.SleepTimeOut == 0 ? 1 : AppStatus.SleepTimeOut);
					PrimaryGamepad.Motion.RatchetDelayMaxTimer = static_cast<int>(roundf(PrimaryGamepad.Motion.RatchetDelayTime / (AppStatus.SleepTimeOut == 0 ? 1 : AppStatus.SleepTimeOut)));
				}
				else {
					PrimaryGamepad.Motion.RatchetDelayMaxTimer = 0;	//В режиме OnlyPressed отклик должен быть мгновенным!
				}
				PrimaryGamepad.Motion.RatchetDelayTimer = PrimaryGamepad.Motion.RatchetDelayMaxTimer;

				// ЖЕСТКО сбрасываем EMA-фильтр, чтобы убить "старую" инерцию
				PrimaryGamepad.Motion.EmaGyroX = 0.0f;
				PrimaryGamepad.Motion.EmaGyroY = 0.0f;
				PrimaryGamepad.Motion.EmaGyroZ = 0.0f;
			}

			float effGyroX = velocityX;//Считываем кумулятивные данные один раз для режимов
			float effGyroY = velocityY;
			float effGyroZ = velocityZ;

			// Вычисляем, насколько сильно геймпад отклонен от горизонтали (нагрузка на оси X и Z)
			/*float tiltMagnitude = sqrtf(MotionState.gravX * MotionState.gravX + MotionState.gravZ * MotionState.gravZ);

			// Вычисляем сырую скорость вращения (в градусах в секунду)
			float rawSpeed = sqrtf(effGyroX * effGyroX + effGyroY * effGyroY + effGyroZ * effGyroZ);

			// Если геймпад наклонен (tiltMagnitude > 0.2f) И скорость микроскопическая (менее 1.5 град/сек)
			if (tiltMagnitude > 0.2f && rawSpeed < 1.0f) {
				effGyroX = 0.0f;
				effGyroY = 0.0f;
				effGyroZ = 0.0f;
			}*/

			// Если таймер амортизатора еще тикает и функция не отключена в конфиге
			if (PrimaryGamepad.Motion.RatchetDelayTimer > 0 && PrimaryGamepad.Motion.RatchetDelayMaxTimer > 0) {
				PrimaryGamepad.Motion.RatchetDelayTimer--;

				// Вычисляем прогресс от 0.0 до 1.0
				float progress = 1.0f - ((float)PrimaryGamepad.Motion.RatchetDelayTimer / (float)PrimaryGamepad.Motion.RatchetDelayMaxTimer);

				// Квадратичная кривая (progress * progress) для мягкого старта
				float easeInFactor = progress * progress;

				// Глушим текущую скорость
				effGyroX *= easeInFactor;
				effGyroY *= easeInFactor;
				effGyroZ *= easeInFactor;
			}

			//@029 2.EMA
			float timeCorrectedAlpha = (AppStatus.AimMode == AimMouseMode) ? PrimaryGamepad.Motion.CachedMouseAlpha : PrimaryGamepad.Motion.CachedJoyAlpha;

			if (timeCorrectedAlpha > 0.0f) {
				PrimaryGamepad.Motion.EmaGyroX = effGyroX * (1.0f - timeCorrectedAlpha) + PrimaryGamepad.Motion.EmaGyroX * timeCorrectedAlpha;
				PrimaryGamepad.Motion.EmaGyroY = effGyroY * (1.0f - timeCorrectedAlpha) + PrimaryGamepad.Motion.EmaGyroY * timeCorrectedAlpha;
				PrimaryGamepad.Motion.EmaGyroZ = effGyroZ * (1.0f - timeCorrectedAlpha) + PrimaryGamepad.Motion.EmaGyroZ * timeCorrectedAlpha;

				effGyroX = PrimaryGamepad.Motion.EmaGyroX;
				effGyroY = PrimaryGamepad.Motion.EmaGyroY;
				effGyroZ = PrimaryGamepad.Motion.EmaGyroZ;
			}
			else {
				PrimaryGamepad.Motion.EmaGyroX = effGyroX;
				PrimaryGamepad.Motion.EmaGyroY = effGyroY;
				PrimaryGamepad.Motion.EmaGyroZ = effGyroZ;
			} //end of EMA

			//Это уже град/сек!
			const float InputSize = sqrtf(effGyroX * effGyroX + effGyroY * effGyroY + effGyroZ * effGyroZ);
			float tighteningFactor = 1.0f;

			//Вычисляем Tightening по JibbSmart
			if (PrimaryGamepad.Motion.Tightening > 0.0f && InputSize < PrimaryGamepad.Motion.Tightening) {
				tighteningFactor = InputSize / PrimaryGamepad.Motion.Tightening;
			}

			//Применяем tightening НАПРЯМУЮ к осям гироскопа
			effGyroX *= tighteningFactor;
			effGyroY *= tighteningFactor;

			//@073 Parametric gyro acceleration как в STEAM / JSM
			if (PrimaryGamepad.Motion.GyroAccelEnd > PrimaryGamepad.Motion.GyroAccelStart) {

				float gyroSpeed = sqrtf(effGyroX * effGyroX + effGyroY * effGyroY);

				if (gyroSpeed > PrimaryGamepad.Motion.GyroAccelStart) {
					// Нормализуем прогресс скорости в диапазоне от 0.0 до 1.0
					float progress = std::clamp((gyroSpeed - PrimaryGamepad.Motion.GyroAccelStart) / (PrimaryGamepad.Motion.GyroAccelEnd - PrimaryGamepad.Motion.GyroAccelStart), 0.0f, 1.0f);
					float accelMult = 1.0f;

					if (AppStatus.AimMode == AimMouseMode) {
						// Gyro Mouse: Линейная акселерация (канонический Steam Input)
						accelMult = 1.0f + progress * (PrimaryGamepad.Motion.GyroAccelCap - 1.0f);
					}
					else {
						// Gyro Stick: Soft Knee Range Expander (Плавное расширение диапазона)
						//float baseMultJoy = 16.66f * AppStatus.FrameTime;
						const float baseMultJoy = 1.0f / 15.0f; //  0.066667f (1.0 при 75 град/сек и JoySens=100)

						// Текущий чистый сигнал
						//float rawMagCurrent = gyroSpeed * baseMultJoy * PrimaryGamepad.Motion.JoySensAvg;
						// Сигнал на границе Start
						//float rawMagStart = PrimaryGamepad.Motion.GyroAccelStart * baseMultJoy * PrimaryGamepad.Motion.JoySensAvg;

						//@080 Честный вектор отклика стика с учетом асимметричной чувствительности (JoySensX != JoySensY)
						float unaccelStickX = -effGyroY * baseMultJoy * PrimaryGamepad.Motion.JoySensX;
						float unaccelStickY = effGyroX * baseMultJoy * PrimaryGamepad.Motion.JoySensY;
						float rawMagCurrent = sqrtf(unaccelStickX * unaccelStickX + unaccelStickY * unaccelStickY);

						// Сигнал на пороге Start по текущей траектории движения
						float rawMagStart = (gyroSpeed > 0.0001f) ? (rawMagCurrent / gyroSpeed) * PrimaryGamepad.Motion.GyroAccelStart : 0.0f;

						if (rawMagCurrent > rawMagStart && rawMagStart < 1.0f) {
							// Сколько диапазона стика осталось до 1.0 (например, от 0.6 до 1.0)
							float remainingOutput = 1.0f - rawMagStart;

							// Выпуклая кривая Ease-Out. 
							// При Cap = 1.0 (прямая) - будет казаться медленно.
							// При Cap = 2.0 или 3.0 - быстро набирает ход, но плавно смягчает удар в 1.0
							float curveProgress = 1.0f - powf(1.0f - progress, PrimaryGamepad.Motion.GyroAccelCap);

							// Целевой сигнал стика
							float targetMag = rawMagStart + curveProgress * remainingOutput;

							accelMult = targetMag / rawMagCurrent;
						}
					}

					effGyroX *= accelMult;
					effGyroY *= accelMult;
				}
			}

			//базовые множители
			//float baseMultMouse = 50.0f * AppStatus.FrameTime;
			//float baseMultJoy = 16.66f * AppStatus.FrameTime; // Уменьшен в ~3 раза, чтобы шкала конфига снова стала комфортной в районе 120
			
			//@073 Базовые множители: мышь интегрирует реальное время, стик оперирует чистой скоростью
			float baseMultMouse = 50.0f * loopDeltaTime;
			const float baseMultJoy = 1.0f / 15.0f; // 1.0 при 75 град/сек и JoySens=100

			if (AppStatus.AimMode == AimMouseMode) { //Mouse
				MouseMove(-effGyroY * baseMultMouse * PrimaryGamepad.Motion.SensX,
					-effGyroX * baseMultMouse * PrimaryGamepad.Motion.SensY);
			}
			else { // Joystick + @079
				// Динамический выбор целевого стика (поддержка раскладки левши Southpaw при SWAP-STICKS=1)
				SHORT& targetThumbX = CurrentXboxProfile.SwapSticksAxis ? report.sThumbLX : report.sThumbRX;
				SHORT& targetThumbY = CurrentXboxProfile.SwapSticksAxis ? report.sThumbLY : report.sThumbRY;
				int maxStickLimit = CurrentXboxProfile.SwapSticksAxis ? PrimaryGamepad.Sticks.MaxLeftStickLimit : PrimaryGamepad.Sticks.MaxRightStickLimit;
				float adzX = CurrentXboxProfile.SwapSticksAxis ? PrimaryGamepad.Sticks.AntiDeadZoneLeftX : PrimaryGamepad.Sticks.AntiDeadZoneRightX;
				float adzY = CurrentXboxProfile.SwapSticksAxis ? PrimaryGamepad.Sticks.AntiDeadZoneLeftY : PrimaryGamepad.Sticks.AntiDeadZoneRightY;

				float gyroRX = ClampFloat(-effGyroY * baseMultJoy * PrimaryGamepad.Motion.JoySensX, -1.0f, 1.0f);
				float gyroRY = ClampFloat(effGyroX * baseMultJoy * PrimaryGamepad.Motion.JoySensY, -1.0f, 1.0f);

				float curStickX = targetThumbX / 32767.0f;
				float curStickY = targetThumbY / 32767.0f;
				float stickMag = sqrtf(curStickX * curStickX + curStickY * curStickY);

				// Если физический стик в центре, а гироскоп движется:
				// выталкиваем гироскоп из мертвой зоны на соответствующий пьедестал ADZ!
				if (PrimaryGamepad.Motion.GyroApplyAntiDeadZone && stickMag < 0.0001f && (adzX > 0.0f || adzY > 0.0f)) {
					float gMag = sqrtf(gyroRX * gyroRX + gyroRY * gyroRY);
					if (gMag > 0.0001f) {
						float invG = 1.0f / gMag;
						gyroRX = (gyroRX * invG * adzX) + gyroRX * (1.0f - adzX);
						gyroRY = (gyroRY * invG * adzY) + gyroRY * (1.0f - adzY);
					}
				}

				// Плавное сложение без двойного наложения пьедестала
				float combinedX = gyroRX + curStickX;
				float combinedY = gyroRY + curStickY;

				targetThumbX = static_cast<SHORT>(std::clamp(static_cast<int>(roundf(combinedX * 32767.0f)), -maxStickLimit, maxStickLimit));
				targetThumbY = static_cast<SHORT>(std::clamp(static_cast<int>(roundf(combinedY * 32767.0f)), -maxStickLimit, maxStickLimit));
			}
		}

		// [-_-] Touchpad sticks
		else if (PrimaryGamepad.GamepadActionMode == TouchpadSticksMode) {

			if (TouchState.t0Down) {
				if (FirstTouch.Touched == false) {
					FirstTouch.InitAxisX = TouchState.t0X;
					FirstTouch.InitAxisY = TouchState.t0Y;
					FirstTouch.Touched = true;
				}
				FirstTouch.AxisX = TouchState.t0X - FirstTouch.InitAxisX;
				FirstTouch.AxisY = TouchState.t0Y - FirstTouch.InitAxisY;


#pragma warning(push)
#pragma warning(disable: 4244)

				if (FirstTouch.InitAxisX < 0.5) {
					report.sThumbLX = ClampFloat(FirstTouch.AxisX * PrimaryGamepad.TouchSticks.LeftX, -1, 1) * 32767;
					report.sThumbLY = ClampFloat(-FirstTouch.AxisY * PrimaryGamepad.TouchSticks.LeftY, -1, 1) * 32767;
					if (PrimaryGamepad.InputState.buttons & JSMASK_TOUCHPAD_CLICK) report.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;
				}
				else {
					report.sThumbRX = ClampFloat((TouchState.t0X - FirstTouch.LastAxisX) * PrimaryGamepad.TouchSticks.RightX * 200, -1, 1) * 32767;
					report.sThumbRY = ClampFloat(-(TouchState.t0Y - FirstTouch.LastAxisY) * PrimaryGamepad.TouchSticks.RightY * 200, -1, 1) * 32767;
					FirstTouch.LastAxisX = TouchState.t0X; FirstTouch.LastAxisY = TouchState.t0Y;
					if (PrimaryGamepad.InputState.buttons & JSMASK_TOUCHPAD_CLICK) report.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB;
				}
			}
			else {
				FirstTouch.AxisX = 0;
				FirstTouch.AxisY = 0;
				FirstTouch.Touched = false;
			}

			if (TouchState.t1Down) {
				if (SecondTouch.Touched == false) {
					SecondTouch.InitAxisX = TouchState.t1X;
					SecondTouch.InitAxisY = TouchState.t1Y;
					SecondTouch.Touched = true;
				}
				SecondTouch.AxisX = TouchState.t1X - SecondTouch.InitAxisX;
				SecondTouch.AxisY = TouchState.t1Y - SecondTouch.InitAxisY;

				if (SecondTouch.InitAxisX < 0.5) {
					report.sThumbLX = ClampFloat(SecondTouch.AxisX * PrimaryGamepad.TouchSticks.LeftX, -1, 1) * 32767;
					report.sThumbLY = ClampFloat(-SecondTouch.AxisY * PrimaryGamepad.TouchSticks.LeftY, -1, 1) * 32767;
				}
				else {
					report.sThumbRX = ClampFloat((TouchState.t1X - SecondTouch.LastAxisX) * PrimaryGamepad.TouchSticks.RightX * 200, -1, 1) * 32767;
					report.sThumbRY = ClampFloat(-(TouchState.t1Y - SecondTouch.LastAxisY) * PrimaryGamepad.TouchSticks.RightY * 200, -1, 1) * 32767;
					SecondTouch.LastAxisX = TouchState.t1X; SecondTouch.LastAxisY = TouchState.t1Y;
				}
			}
			else {
				SecondTouch.AxisX = 0;
				SecondTouch.AxisY = 0;
				SecondTouch.Touched = false;
			}
		}

#pragma warning(pop)

		// Keyboard and mouse mode
		bool DontResetInputState = !( // Reset clicks when activating some actions / Сброс нажатий при активации некоторых действий
			(PrimaryGamepad.InputState.buttons & JSMASK_PS) && !(PrimaryGamepad.InputState.buttons & JSMASK_HOME) || //@013 иначе на HOME не эмулируются кнопки KM в XboxProfile.	
			IsSharePressed ||
			//IsRecordPressed);
			IsRecordPressed ||
			IsKeyPressed(VK_MENU)
			);

		if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse || AppStatus.GamepadEmulationMode == EmuGamepadEnabled) { //@014  add для эмуляции KM в XboxProfiles

			if (PrimaryGamepad.GamepadActionMode == MotionDrivingMode) {

				//float MotionAxisX = CalcMotionStick(MotionState.gravX, MotionState.gravZ, PrimaryGamepad.Motion.SteeringWheelAngle, PrimaryGamepad.Motion.OffsetAxisX);	//@045
				float MotionAxisX = CalcMotionStick(MotionState.gravX, MotionState.gravZ, MotionState.gravY, PrimaryGamepad.Motion.SteeringWheelAngle, PrimaryGamepad.Motion.OffsetAxisX, PrimaryGamepad.Motion.PrevAngleRad, PrimaryGamepad.Motion.CumulativeOffsetRad, PrimaryGamepad.Motion.AngleInitialized, PrimaryGamepad.Motion.IsManualCalibrated, PrimaryGamepad.Motion.LinearityWheel);

				if (PrimaryGamepad.InputState.buttons & JSMASK_LEFT) PrimaryGamepad.InputState.buttons &= ~JSMASK_LEFT;
				if (PrimaryGamepad.InputState.buttons & JSMASK_LEFT) PrimaryGamepad.InputState.buttons &= ~JSMASK_LEFT;
				if (PrimaryGamepad.InputState.buttons & JSMASK_RIGHT) PrimaryGamepad.InputState.buttons &= ~JSMASK_RIGHT;

				if (fabs(MotionAxisX) < PrimaryGamepad.KMEmu.SteeringWheelDeadZone) {
					PrimaryGamepad.KMEmu.MaxLeftAxisX = 0.0f;
					PrimaryGamepad.KMEmu.MaxRightAxisX = 0.0f;
				}
				else {

					// Left
					if (MotionAxisX < -PrimaryGamepad.KMEmu.SteeringWheelDeadZone) {
						// Update the maximum
						if (MotionAxisX < PrimaryGamepad.KMEmu.MaxLeftAxisX) PrimaryGamepad.KMEmu.MaxLeftAxisX = MotionAxisX;

						// Retention of at least 95% of the peak
						if (MotionAxisX <= PrimaryGamepad.KMEmu.MaxLeftAxisX * (1.0f - PrimaryGamepad.KMEmu.SteeringWheelReleaseThreshold))
							if (PrimaryGamepad.KMEmu.SteeringWheelUseDPAD)
								PrimaryGamepad.InputState.buttons |= JSMASK_LEFT;
							else
								PrimaryGamepad.InputState.stickLX = -1.0f;
					}

					// Right
					if (MotionAxisX > PrimaryGamepad.KMEmu.SteeringWheelDeadZone) {
						if (MotionAxisX > PrimaryGamepad.KMEmu.MaxRightAxisX) PrimaryGamepad.KMEmu.MaxRightAxisX = MotionAxisX;

						if (MotionAxisX >= PrimaryGamepad.KMEmu.MaxRightAxisX * (1.0f - PrimaryGamepad.KMEmu.SteeringWheelReleaseThreshold)) {
							if (PrimaryGamepad.KMEmu.SteeringWheelUseDPAD)
								PrimaryGamepad.InputState.buttons |= JSMASK_RIGHT;
							else
								PrimaryGamepad.InputState.stickLX = 1.0f;
						}
					}
				}
			}

			//KeyPress(PrimaryGamepad.ButtonsStates.LeftTrigger.KeyCode, DontResetInputState && PrimaryGamepad.InputState.lTrigger > PrimaryGamepad.KMEmu.TriggerValuePressKey, &PrimaryGamepad.ButtonsStates.LeftTrigger, true);
			//KeyPress(PrimaryGamepad.ButtonsStates.RightTrigger.KeyCode, DontResetInputState && PrimaryGamepad.InputState.rTrigger > PrimaryGamepad.KMEmu.TriggerValuePressKey, &PrimaryGamepad.ButtonsStates.RightTrigger, true);

			// Same - JSMASK_CAPTURE 0x020000 - JSMASK_TOUCHPAD_CLICK 0x020000
			/*if (PrimaryGamepad.ControllerType == SONY_DUALSENSE || PrimaryGamepad.ControllerType == SONY_DUALSHOCK4)
				KeyPress(PrimaryGamepad.ButtonsStates.Back.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_SHARE, &PrimaryGamepad.ButtonsStates.Back, true);
			else if (PrimaryGamepad.ControllerType == NINTENDO_JOYCONS || PrimaryGamepad.ControllerType == NINTENDO_JOYCONS)
				KeyPress(PrimaryGamepad.ButtonsStates.Back.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE, &PrimaryGamepad.ButtonsStates.Back, true);
			
			KeyPress(PrimaryGamepad.ButtonsStates.Start.KeyCode, DontResetInputState && (PrimaryGamepad.InputState.buttons & JSMASK_OPTIONS || PrimaryGamepad.InputState.buttons & JSMASK_HOME), &PrimaryGamepad.ButtonsStates.Start, true);*/

			/*KeyPress(PrimaryGamepad.ButtonsStates.LeftBumper.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_L, &PrimaryGamepad.ButtonsStates.LeftBumper, true);
			KeyPress(PrimaryGamepad.ButtonsStates.RightBumper.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_R, &PrimaryGamepad.ButtonsStates.RightBumper, true);

			//@015  Разделене BACK START для SONY и Nintendo
			if (PrimaryGamepad.ControllerType == SONY_DUALSENSE || PrimaryGamepad.ControllerType == SONY_DUALSHOCK4) {
				KeyPress(PrimaryGamepad.ButtonsStates.Back.KeyCode, DontResetInputState && (PrimaryGamepad.InputState.buttons & JSMASK_SHARE), &PrimaryGamepad.ButtonsStates.Back, true);
				KeyPress(PrimaryGamepad.ButtonsStates.Start.KeyCode, DontResetInputState && (PrimaryGamepad.InputState.buttons & JSMASK_OPTIONS), &PrimaryGamepad.ButtonsStates.Start, true);
			}
			else {
				KeyPress(PrimaryGamepad.ButtonsStates.Back.KeyCode, DontResetInputState && (PrimaryGamepad.InputState.buttons & JSMASK_MINUS), &PrimaryGamepad.ButtonsStates.Back, true);
				KeyPress(PrimaryGamepad.ButtonsStates.Start.KeyCode, DontResetInputState && (PrimaryGamepad.InputState.buttons & JSMASK_PLUS), &PrimaryGamepad.ButtonsStates.Start, true);
			}

			if (PrimaryGamepad.ButtonsStates.DPADAdvancedMode == false) { // Regular mode  ↑ → ↓ ←
				KeyPress(PrimaryGamepad.ButtonsStates.DPADUp.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_UP, &PrimaryGamepad.ButtonsStates.DPADUp, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADDown.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_DOWN, &PrimaryGamepad.ButtonsStates.DPADDown, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADLeft.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_LEFT, &PrimaryGamepad.ButtonsStates.DPADLeft, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADRight.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_RIGHT, &PrimaryGamepad.ButtonsStates.DPADRight, true);

			}
			else { // Advanced mode ↑ ↗ → ↘ ↓ ↙ ← ↖ for switching in retro games
				KeyPress(PrimaryGamepad.ButtonsStates.DPADUp.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_UP && !(PrimaryGamepad.InputState.buttons & JSMASK_LEFT) && !(PrimaryGamepad.InputState.buttons & JSMASK_RIGHT), &PrimaryGamepad.ButtonsStates.DPADUp, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADLeft.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_LEFT && !(PrimaryGamepad.InputState.buttons & JSMASK_UP) && !(PrimaryGamepad.InputState.buttons & JSMASK_DOWN), &PrimaryGamepad.ButtonsStates.DPADLeft, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADRight.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_RIGHT && !(PrimaryGamepad.InputState.buttons & JSMASK_UP) && !(PrimaryGamepad.InputState.buttons & JSMASK_DOWN), &PrimaryGamepad.ButtonsStates.DPADRight, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADDown.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_DOWN && !(PrimaryGamepad.InputState.buttons & JSMASK_LEFT) && !(PrimaryGamepad.InputState.buttons & JSMASK_RIGHT), &PrimaryGamepad.ButtonsStates.DPADDown, true);

				KeyPress(PrimaryGamepad.ButtonsStates.DPADUpLeft.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_UP && PrimaryGamepad.InputState.buttons & JSMASK_LEFT, &PrimaryGamepad.ButtonsStates.DPADUpLeft, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADUpRight.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_UP && PrimaryGamepad.InputState.buttons & JSMASK_RIGHT, &PrimaryGamepad.ButtonsStates.DPADUpRight, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADDownLeft.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_DOWN && PrimaryGamepad.InputState.buttons & JSMASK_LEFT, &PrimaryGamepad.ButtonsStates.DPADDownLeft, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADDownRight.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_DOWN && PrimaryGamepad.InputState.buttons & JSMASK_RIGHT, &PrimaryGamepad.ButtonsStates.DPADDownRight, true);
			}

			KeyPress(PrimaryGamepad.ButtonsStates.Y.KeyCode, DontResetInputState &&  PrimaryGamepad.InputState.buttons & JSMASK_N, &PrimaryGamepad.ButtonsStates.Y, true);
			KeyPress(PrimaryGamepad.ButtonsStates.A.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_S, &PrimaryGamepad.ButtonsStates.A, true);
			KeyPress(PrimaryGamepad.ButtonsStates.X.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_W, &PrimaryGamepad.ButtonsStates.X, true);
			KeyPress(PrimaryGamepad.ButtonsStates.B.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_E, &PrimaryGamepad.ButtonsStates.B, true);

			KeyPress(PrimaryGamepad.ButtonsStates.LeftStick.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_LCLICK, &PrimaryGamepad.ButtonsStates.LeftStick, true);
			KeyPress(PrimaryGamepad.ButtonsStates.RightStick.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_RCLICK, &PrimaryGamepad.ButtonsStates.RightStick, true);*/

			//@071

			bool isLeftTriggerPressed = (PrimaryGamepad.InputState.lTrigger > PrimaryGamepad.KMEmu.TriggerValuePressKey) || ((PrimaryGamepad.InputState.buttons & JSMASK_ZL) != 0);
			bool isRightTriggerPressed = (PrimaryGamepad.InputState.rTrigger > PrimaryGamepad.KMEmu.TriggerValuePressKey) || ((PrimaryGamepad.InputState.buttons & JSMASK_ZR) != 0);

			ProcessSmartButton(isLeftTriggerPressed, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.LeftTrigger, DontResetInputState, PrimaryGamepad.Trackers.ZL, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton(isRightTriggerPressed, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.RightTrigger, DontResetInputState, PrimaryGamepad.Trackers.ZR, sleepMs, AppStatus.LongPressTimeOut);

			ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_L) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.LeftBumper, DontResetInputState, PrimaryGamepad.Trackers.L, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_R) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.RightBumper, DontResetInputState, PrimaryGamepad.Trackers.R, sleepMs, AppStatus.LongPressTimeOut);

			//@015  Разделене BACK START для SONY и Nintendo
			if (PrimaryGamepad.ControllerType == SONY_DUALSENSE || PrimaryGamepad.ControllerType == SONY_DUALSHOCK4) {
				ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_SHARE) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.Back, DontResetInputState, PrimaryGamepad.Trackers.Back, sleepMs, AppStatus.LongPressTimeOut);
				ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_OPTIONS) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.Start, DontResetInputState, PrimaryGamepad.Trackers.Start, sleepMs, AppStatus.LongPressTimeOut);
			}
			else {
				ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_MINUS) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.Back, DontResetInputState, PrimaryGamepad.Trackers.Back, sleepMs, AppStatus.LongPressTimeOut);
				ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_PLUS) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.Start, DontResetInputState, PrimaryGamepad.Trackers.Start, sleepMs, AppStatus.LongPressTimeOut);
			}

			if (PrimaryGamepad.ButtonsStates.DPADAdvancedMode == false) { // Regular mode  ^ > v <
				ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_UP) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.DPADUp, DontResetInputState, PrimaryGamepad.Trackers.DPADUp, sleepMs, AppStatus.LongPressTimeOut);
				ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_DOWN) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.DPADDown, DontResetInputState, PrimaryGamepad.Trackers.DPADDown, sleepMs, AppStatus.LongPressTimeOut);
				ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_LEFT) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.DPADLeft, DontResetInputState, PrimaryGamepad.Trackers.DPADLeft, sleepMs, AppStatus.LongPressTimeOut);
				ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_RIGHT) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.DPADRight, DontResetInputState, PrimaryGamepad.Trackers.DPADRight, sleepMs, AppStatus.LongPressTimeOut);
			}
			else { // Advanced mode ^ ? > ? v ? < ? for switching in retro games
				KeyPress(PrimaryGamepad.ButtonsStates.DPADUp.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_UP && !(PrimaryGamepad.InputState.buttons & JSMASK_LEFT) && !(PrimaryGamepad.InputState.buttons & JSMASK_RIGHT), &PrimaryGamepad.ButtonsStates.DPADUp, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADLeft.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_LEFT && !(PrimaryGamepad.InputState.buttons & JSMASK_UP) && !(PrimaryGamepad.InputState.buttons & JSMASK_DOWN), &PrimaryGamepad.ButtonsStates.DPADLeft, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADRight.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_RIGHT && !(PrimaryGamepad.InputState.buttons & JSMASK_UP) && !(PrimaryGamepad.InputState.buttons & JSMASK_DOWN), &PrimaryGamepad.ButtonsStates.DPADRight, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADDown.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_DOWN && !(PrimaryGamepad.InputState.buttons & JSMASK_LEFT) && !(PrimaryGamepad.InputState.buttons & JSMASK_RIGHT), &PrimaryGamepad.ButtonsStates.DPADDown, true);

				KeyPress(PrimaryGamepad.ButtonsStates.DPADUpLeft.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_UP && PrimaryGamepad.InputState.buttons & JSMASK_LEFT, &PrimaryGamepad.ButtonsStates.DPADUpLeft, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADUpRight.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_UP && PrimaryGamepad.InputState.buttons & JSMASK_RIGHT, &PrimaryGamepad.ButtonsStates.DPADUpRight, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADDownLeft.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_DOWN && PrimaryGamepad.InputState.buttons & JSMASK_LEFT, &PrimaryGamepad.ButtonsStates.DPADDownLeft, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADDownRight.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_DOWN && PrimaryGamepad.InputState.buttons & JSMASK_RIGHT, &PrimaryGamepad.ButtonsStates.DPADDownRight, true);
			}

			ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_N) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.Y, DontResetInputState, PrimaryGamepad.Trackers.Y, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_S) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.A, DontResetInputState, PrimaryGamepad.Trackers.A, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_W) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.X, DontResetInputState, PrimaryGamepad.Trackers.X, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_E) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.B, DontResetInputState, PrimaryGamepad.Trackers.B, sleepMs, AppStatus.LongPressTimeOut);

			ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_LCLICK) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.LeftStick, DontResetInputState, PrimaryGamepad.Trackers.L3, sleepMs, AppStatus.LongPressTimeOut);
			ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_RCLICK) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.RightStick, DontResetInputState, PrimaryGamepad.Trackers.R3, sleepMs, AppStatus.LongPressTimeOut);

			// Aditional buttons
			if (PrimaryGamepad.ControllerType == SONY_DUALSENSE) { // Edge
				ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_FNL) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.DSEdgeL4, DontResetInputState, PrimaryGamepad.Trackers.DSEdgeL4, sleepMs, AppStatus.LongPressTimeOut);
				ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_FNR) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.DSEdgeR4, DontResetInputState, PrimaryGamepad.Trackers.DSEdgeR4, sleepMs, AppStatus.LongPressTimeOut);
			}
			else if (PrimaryGamepad.ControllerType == NINTENDO_JOYCONS) {
				ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_SL) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.JCSL, DontResetInputState, PrimaryGamepad.Trackers.JCSL, sleepMs, AppStatus.LongPressTimeOut);
				ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_SR) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.JCSR, DontResetInputState, PrimaryGamepad.Trackers.JCSR, sleepMs, AppStatus.LongPressTimeOut);
				//ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_HOME) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.HOME, DontResetInputState, PrimaryGamepad.Trackers.HOME, sleepMs, AppStatus.LongPressTimeOut);
				//ProcessSmartButton((PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE) != 0, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.CAPTURE, DontResetInputState, PrimaryGamepad.Trackers.CAPTURE, sleepMs, AppStatus.LongPressTimeOut);
				ProcessSmartButton(allowHomeSolo, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.HOME, DontResetInputState, PrimaryGamepad.Trackers.HOME, sleepMs, AppStatus.LongPressTimeOut);	//@072
				ProcessSmartButton(allowCaptureSolo, 0, 0, nullptr, &PrimaryGamepad.ButtonsStates.CAPTURE, DontResetInputState, PrimaryGamepad.Trackers.CAPTURE, sleepMs, AppStatus.LongPressTimeOut);
			}

			KMStickMode(PrimaryGamepad, DontResetInputState, true, DeadZoneAxis(PrimaryGamepad.InputState.stickLX, PrimaryGamepad.Sticks.DeadZoneLeftX), DeadZoneAxis(PrimaryGamepad.InputState.stickLY, PrimaryGamepad.Sticks.DeadZoneLeftY), PrimaryGamepad.KMEmu.LeftStickMode);
			KMStickMode(PrimaryGamepad, DontResetInputState, false, DeadZoneAxis(PrimaryGamepad.InputState.stickRX, PrimaryGamepad.Sticks.DeadZoneRightX), DeadZoneAxis(PrimaryGamepad.InputState.stickRY, PrimaryGamepad.Sticks.DeadZoneRightY), PrimaryGamepad.KMEmu.RightStickMode);

			// Aditional buttons
			if (PrimaryGamepad.ControllerType == SONY_DUALSENSE) { // Edge
				KeyPress(PrimaryGamepad.ButtonsStates.DSEdgeL4.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_FNL, &PrimaryGamepad.ButtonsStates.DSEdgeL4, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DSEdgeR4.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_FNR, &PrimaryGamepad.ButtonsStates.DSEdgeR4, true);
			} else if (PrimaryGamepad.ControllerType == NINTENDO_JOYCONS) {
				KeyPress(PrimaryGamepad.ButtonsStates.JCSL.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_SL, &PrimaryGamepad.ButtonsStates.JCSL, true);
				KeyPress(PrimaryGamepad.ButtonsStates.JCSR.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_SR, &PrimaryGamepad.ButtonsStates.JCSR, true);
				KeyPress(PrimaryGamepad.ButtonsStates.HOME.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons&JSMASK_HOME, &PrimaryGamepad.ButtonsStates.HOME, true);
				KeyPress(PrimaryGamepad.ButtonsStates.CAPTURE.KeyCode, DontResetInputState&&PrimaryGamepad.InputState.buttons&JSMASK_CAPTURE, &PrimaryGamepad.ButtonsStates.CAPTURE, true);//@016
			}

			//@017 УНИВЕРСАЛЬНЫЙ БЛОК WHEEL Для Xbox и KM + WheelWheelXboxHoldTimer  для  SleepTimeOut<15
			int currentWheelActivationBtn = (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) ? PrimaryGamepad.ButtonsStates.WheelActivationGamepadButton.KeyCode : CurrentXboxProfile.WheelActivationButton;

			// Сброс
			PrimaryGamepad.ButtonsStates.WheelDefault.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelUp.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelDown.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelLeft.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelRight.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelUpLeft.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelUpRight.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelDownLeft.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelDownRight.IsPressed = false;

			if (currentWheelActivationBtn != 0 && (PrimaryGamepad.InputState.buttons & currentWheelActivationBtn)) {
				if (PrimaryGamepad.Motion.WheelCounter == 0) {
					PrimaryGamepad.Motion.WheelCounter = AppStatus.SkipPollTimeOut;
					PrimaryGamepad.Motion.WheelActive = true;
					PrimaryGamepad.Motion.WheelAccumX = 0.0f;
					PrimaryGamepad.Motion.WheelAccumY = 0.0f;
				}
				if (AppStatus.GamepadEmulationMode != EmuKeyboardAndMouse) {
					report.wButtons &= ~CurrentXboxProfile.WheelDefault;
				}
			}

			if (!(PrimaryGamepad.InputState.buttons & currentWheelActivationBtn) && PrimaryGamepad.Motion.WheelCounter > 0) {
				if (PrimaryGamepad.Motion.WheelCounter == 1) {
					float MotionLength = sqrtf(PrimaryGamepad.Motion.WheelAccumX * PrimaryGamepad.Motion.WheelAccumX + PrimaryGamepad.Motion.WheelAccumY * PrimaryGamepad.Motion.WheelAccumY);

					WORD xboxButtonToPress = 0; // Сюда собираем нужную кнопку Xbox

					if (MotionLength < PrimaryGamepad.Motion.MotionWheelButtonsDeadZone) {
						xboxButtonToPress = CurrentXboxProfile.WheelDefault;
						PrimaryGamepad.ButtonsStates.WheelDefault.IsPressed = true;
					}
					else {
						float MotionWheelAngle = atan2f(PrimaryGamepad.Motion.WheelAccumY, PrimaryGamepad.Motion.WheelAccumX) * 57.29578f;
						bool advancedMode = (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) ? PrimaryGamepad.ButtonsStates.WheelAdvancedMode : CurrentXboxProfile.WheelAdvancedMode;

						if (!advancedMode) {
							if (fabs(MotionWheelAngle) < 25.f) {
								xboxButtonToPress = CurrentXboxProfile.WheelUp;
								PrimaryGamepad.ButtonsStates.WheelUp.IsPressed = true;
							}
							else if (MotionWheelAngle > 45.f && MotionWheelAngle < 135.f) {
								xboxButtonToPress = CurrentXboxProfile.WheelLeft;
								PrimaryGamepad.ButtonsStates.WheelLeft.IsPressed = true;
							}
							else if (MotionWheelAngle < -45.f && MotionWheelAngle > -135.f) {
								xboxButtonToPress = CurrentXboxProfile.WheelRight;
								PrimaryGamepad.ButtonsStates.WheelRight.IsPressed = true;
							}
							else if (MotionWheelAngle > 165.f || MotionWheelAngle < -165.f) {
								xboxButtonToPress = CurrentXboxProfile.WheelDown;
								PrimaryGamepad.ButtonsStates.WheelDown.IsPressed = true;
							}
						}
						else {
							if (MotionWheelAngle >= -22.5f && MotionWheelAngle < 22.5f) {
								xboxButtonToPress = CurrentXboxProfile.WheelUp;
								PrimaryGamepad.ButtonsStates.WheelUp.IsPressed = true;
							}
							else if (MotionWheelAngle >= 22.5f && MotionWheelAngle < 67.5f) {
								xboxButtonToPress = CurrentXboxProfile.WheelUpLeft;
								PrimaryGamepad.ButtonsStates.WheelUpLeft.IsPressed = true;
							}
							else if (MotionWheelAngle >= 67.5f && MotionWheelAngle < 112.5f) {
								xboxButtonToPress = CurrentXboxProfile.WheelLeft;
								PrimaryGamepad.ButtonsStates.WheelLeft.IsPressed = true;
							}
							else if (MotionWheelAngle >= 112.5f && MotionWheelAngle < 157.5f) {
								xboxButtonToPress = CurrentXboxProfile.WheelDownLeft;
								PrimaryGamepad.ButtonsStates.WheelDownLeft.IsPressed = true;
							}
							else if (MotionWheelAngle >= 157.5f || MotionWheelAngle < -157.5f) {
								xboxButtonToPress = CurrentXboxProfile.WheelDown;
								PrimaryGamepad.ButtonsStates.WheelDown.IsPressed = true;
							}
							else if (MotionWheelAngle >= -157.5f && MotionWheelAngle < -112.5f) {
								xboxButtonToPress = CurrentXboxProfile.WheelDownRight;
								PrimaryGamepad.ButtonsStates.WheelDownRight.IsPressed = true;
							}
							else if (MotionWheelAngle >= -112.5f && MotionWheelAngle < -67.5f) {
								xboxButtonToPress = CurrentXboxProfile.WheelRight;
								PrimaryGamepad.ButtonsStates.WheelRight.IsPressed = true;
							}
							else if (MotionWheelAngle >= -67.5f && MotionWheelAngle < -22.5f) {
								xboxButtonToPress = CurrentXboxProfile.WheelUpRight;
								PrimaryGamepad.ButtonsStates.WheelUpRight.IsPressed = true;
							}
						}
					}

					// Если нужно нажать кнопку Xbox, задаем её и запускаем таймер, "33"мс - 2 кадра для обработки при 60fps, для подстраховки - увеличить
					if (xboxButtonToPress != 0 && AppStatus.GamepadEmulationMode != EmuKeyboardAndMouse) {
						PrimaryGamepad.Motion.WheelXboxHoldButton = xboxButtonToPress; //кладем кнопку  в "память"
						PrimaryGamepad.Motion.WheelXboxHoldTimer = (33 / (AppStatus.SleepTimeOut == 0 ? 1 : AppStatus.SleepTimeOut)) + 1;  //мс в циклы, поодстраховка + 1
					}

					PrimaryGamepad.Motion.WheelActive = false;
				}
				PrimaryGamepad.Motion.WheelCounter--;
			}

			// таймер удержания кнопки Xbox
			if (PrimaryGamepad.Motion.WheelXboxHoldTimer > 0) {	//if >0, идет процесс удержания кнопки.
				if (AppStatus.GamepadEmulationMode != EmuKeyboardAndMouse) {
					report.wButtons |= PrimaryGamepad.Motion.WheelXboxHoldButton;  //добавляем нашу кнопку к уже нажатым
				}
				PrimaryGamepad.Motion.WheelXboxHoldTimer--;
			}
			else {
				PrimaryGamepad.Motion.WheelXboxHoldButton = 0;
			}

			if (PrimaryGamepad.Motion.WheelActive) {
				PrimaryGamepad.Motion.WheelAccumX += velocityX * AppStatus.FrameTime;
				PrimaryGamepad.Motion.WheelAccumY += velocityY * AppStatus.FrameTime;
			}

			KeyPress(PrimaryGamepad.ButtonsStates.WheelDefault.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelDefault.IsPressed, &PrimaryGamepad.ButtonsStates.WheelDefault, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelUp.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelUp.IsPressed, &PrimaryGamepad.ButtonsStates.WheelUp, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelUpLeft.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelUpLeft.IsPressed, &PrimaryGamepad.ButtonsStates.WheelUpLeft, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelUpRight.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelUpRight.IsPressed, &PrimaryGamepad.ButtonsStates.WheelUpRight, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelDown.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelDown.IsPressed, &PrimaryGamepad.ButtonsStates.WheelDown, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelDownLeft.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelDownLeft.IsPressed, &PrimaryGamepad.ButtonsStates.WheelDownLeft, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelDownRight.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelDownRight.IsPressed, &PrimaryGamepad.ButtonsStates.WheelDownRight, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelLeft.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelLeft.IsPressed, &PrimaryGamepad.ButtonsStates.WheelLeft, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelRight.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelRight.IsPressed, &PrimaryGamepad.ButtonsStates.WheelRight, true);
			KeyPress(PrimaryGamepad.ButtonsStates.MeleeGesture.KeyCode, DontResetInputState && (PrimaryGamepad.Motion.GestureXTimer > 0), &PrimaryGamepad.ButtonsStates.MeleeGesture, true);	//@043
			KeyPress(PrimaryGamepad.ButtonsStates.AutoSprint.KeyCode, DontResetInputState && isAutoSprintTriggered, &PrimaryGamepad.ButtonsStates.AutoSprint, true);	//@055
		}

		// After releasing all the buttons you can click on screenshots, so it's here / После отпускания всех кнопок можно нажимать скриншоты, поэтому это здесь
		// Microphone (custom key)
		if (AppStatus.ScreenshotMode == ScreenShotCustomKeyMode)
			KeyPress(AppStatus.ScreenShotKey, IsSharePressed, &PrimaryGamepad.ButtonsStates.Screenshot, false);

		// Microphone (screenshots / record)
		else {
			KeyPress(AppStatus.ScreenShotKey, IsScreenshotPressed, &PrimaryGamepad.ButtonsStates.Screenshot, false);
			KeyPress(VK_GAMEBAR_RECORD, IsRecordPressed, &PrimaryGamepad.ButtonsStates.Record, false);
		}

#pragma warning(push)
#pragma warning(disable: 4244)

		if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1) {
			if (AppStatus.GamepadEmulationMode != EmuKeyboardAndMouse) {
				report2.sThumbLX = SecondaryGamepad.Sticks.InvertLeftX == false ? DeadZoneAxis(SecondaryGamepad.InputState.stickLX, SecondaryGamepad.Sticks.DeadZoneLeftX) * 32767 : DeadZoneAxis(-SecondaryGamepad.InputState.stickLX, SecondaryGamepad.Sticks.DeadZoneLeftX) * 32767;
				report2.sThumbLY = SecondaryGamepad.Sticks.InvertLeftY == false ? DeadZoneAxis(SecondaryGamepad.InputState.stickLY, SecondaryGamepad.Sticks.DeadZoneLeftY) * 32767 : DeadZoneAxis(-SecondaryGamepad.InputState.stickLY, SecondaryGamepad.Sticks.DeadZoneLeftY) * 32767;	//была ошибка: "InvertLeftX"
				report2.sThumbRX = SecondaryGamepad.Sticks.InvertRightX == false ? DeadZoneAxis(SecondaryGamepad.InputState.stickRX, SecondaryGamepad.Sticks.DeadZoneRightX) * 32767 : DeadZoneAxis(-SecondaryGamepad.InputState.stickRX, SecondaryGamepad.Sticks.DeadZoneRightX) * 32767;
				report2.sThumbRY = SecondaryGamepad.Sticks.InvertRightY == false ? DeadZoneAxis(SecondaryGamepad.InputState.stickRY, SecondaryGamepad.Sticks.DeadZoneRightY) * 32767 : DeadZoneAxis(-SecondaryGamepad.InputState.stickRY, SecondaryGamepad.Sticks.DeadZoneRightY) * 32767;

				//@041+042 Считываем значения осей второго контроллера
				float s_lx = DeadZoneAxis(SecondaryGamepad.InputState.stickLX, SecondaryGamepad.Sticks.DeadZoneLeftX);
				float s_ly = DeadZoneAxis(SecondaryGamepad.InputState.stickLY, SecondaryGamepad.Sticks.DeadZoneLeftY);
				float s_rx = DeadZoneAxis(SecondaryGamepad.InputState.stickRX, SecondaryGamepad.Sticks.DeadZoneRightX);
				float s_ry = DeadZoneAxis(SecondaryGamepad.InputState.stickRY, SecondaryGamepad.Sticks.DeadZoneRightY);

				if (SecondaryGamepad.Sticks.InvertLeftXY) {
					std::swap(s_lx, s_ly);
					//s_lx = -s_lx; // Инвертируем новую ось X (бывшую Y)
				}
				if (SecondaryGamepad.Sticks.InvertRightXY) {
					std::swap(s_rx, s_ry);
					//s_rx = -s_rx; // Инвертируем новую ось X (бывшую Y)
					s_ry = -s_ry;
				}

				report2.sThumbLX = SecondaryGamepad.Sticks.InvertLeftX == false ? s_lx * 32767 : -s_lx * 32767;
				report2.sThumbLY = SecondaryGamepad.Sticks.InvertLeftX == false ? s_ly * 32767 : -s_ly * 32767;
				report2.sThumbRX = SecondaryGamepad.Sticks.InvertRightX == false ? s_rx * 32767 : -s_rx * 32767;
				report2.sThumbRY = SecondaryGamepad.Sticks.InvertRightY == false ? s_ry * 32767 : -s_ry * 32767;

				report2.bLeftTrigger = DeadZoneAxis(SecondaryGamepad.InputState.lTrigger, SecondaryGamepad.Triggers.DeadZoneLeft) * 255;
				report2.bRightTrigger = DeadZoneAxis(SecondaryGamepad.InputState.rTrigger, SecondaryGamepad.Triggers.DeadZoneRight) * 255;

#pragma warning(pop)

				if (!(SecondaryGamepad.InputState.buttons & JSMASK_PS && SecondaryGamepad.InputState.buttons & JSMASK_CAPTURE && SecondaryGamepad.InputState.buttons & JSMASK_CAPTURE)) { // During special functions, nothing is pressed in the game
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_L ? XINPUT_GAMEPAD_LEFT_SHOULDER : 0;
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_R ? XINPUT_GAMEPAD_RIGHT_SHOULDER : 0;
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_LCLICK ? XINPUT_GAMEPAD_LEFT_THUMB : 0;
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_RCLICK ? XINPUT_GAMEPAD_RIGHT_THUMB : 0;
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_UP ? XINPUT_GAMEPAD_DPAD_UP : 0;
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_DOWN ? XINPUT_GAMEPAD_DPAD_DOWN : 0;
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_LEFT ? XINPUT_GAMEPAD_DPAD_LEFT : 0;
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_RIGHT ? XINPUT_GAMEPAD_DPAD_RIGHT : 0;
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_N ? XINPUT_GAMEPAD_Y : 0;
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_W ? XINPUT_GAMEPAD_X : 0;
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_S ? XINPUT_GAMEPAD_A : 0;
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_E ? XINPUT_GAMEPAD_B : 0;
					if (SecondaryGamepad.ControllerType == NINTENDO_JOYCONS) {		//@042
						report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_SL ? CurrentXboxProfile.JCSL : 0;
						report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_SR ? CurrentXboxProfile.JCSR : 0;
						report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_HOME ? CurrentXboxProfile.HOME : 0;
						report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_CAPTURE ? CurrentXboxProfile.CAPTURE : 0;
					}
				}

				if (JslGetControllerType(SecondaryGamepad.DeviceIndex) == JS_TYPE_DS || JslGetControllerType(SecondaryGamepad.DeviceIndex) == JS_TYPE_DS4) {
					if (!(SecondaryGamepad.InputState.buttons & JSMASK_PS)) {
						report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_SHARE ? XINPUT_GAMEPAD_BACK : 0;
						report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_OPTIONS ? XINPUT_GAMEPAD_START : 0;
					}

					if (AppStatus.SkipPollCount == 0 && (SecondaryGamepad.InputState.buttons & JSMASK_PS && SecondaryGamepad.InputState.buttons & JSMASK_L))
					{
						if (SecondaryGamepad.OutState.LEDBrightness == SecondaryGamepad.DefaultLEDBrightness)
							SecondaryGamepad.OutState.LEDBrightness = 255;
						else
							SecondaryGamepad.OutState.LEDBrightness = SecondaryGamepad.DefaultLEDBrightness;
						GamepadSetState(SecondaryGamepad);
						AppStatus.SkipPollCount = AppStatus.SkipPollTimeOut;
					}
				}
				else if (JslGetControllerType(SecondaryGamepad.DeviceIndex) == JS_TYPE_PRO_CONTROLLER || JslGetControllerType(SecondaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(SecondaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_RIGHT) {
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_MINUS ? XINPUT_GAMEPAD_BACK : 0;
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_PLUS ? XINPUT_GAMEPAD_START : 0;
				}

				if (SecondaryGamepad.RumbleSkipCounter > 0)
					SecondaryGamepad.RumbleSkipCounter--;
			}
			// Secondary gamepad keyboard & mouse
			else {
				KeyPress(SecondaryGamepad.ButtonsStates.LeftTrigger.KeyCode, DontResetInputState && SecondaryGamepad.InputState.lTrigger > SecondaryGamepad.KMEmu.TriggerValuePressKey, &SecondaryGamepad.ButtonsStates.LeftTrigger, true);
				KeyPress(SecondaryGamepad.ButtonsStates.RightTrigger.KeyCode, DontResetInputState && SecondaryGamepad.InputState.rTrigger > SecondaryGamepad.KMEmu.TriggerValuePressKey, &SecondaryGamepad.ButtonsStates.RightTrigger, true);

				KeyPress(SecondaryGamepad.ButtonsStates.LeftBumper.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_L, &SecondaryGamepad.ButtonsStates.LeftBumper, true);
				KeyPress(SecondaryGamepad.ButtonsStates.RightBumper.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_R, &SecondaryGamepad.ButtonsStates.RightBumper, true);

				KeyPress(SecondaryGamepad.ButtonsStates.Back.KeyCode, DontResetInputState && (SecondaryGamepad.InputState.buttons & JSMASK_SHARE || SecondaryGamepad.InputState.buttons & JSMASK_CAPTURE), &SecondaryGamepad.ButtonsStates.Back, true);
				KeyPress(SecondaryGamepad.ButtonsStates.Start.KeyCode, DontResetInputState && (SecondaryGamepad.InputState.buttons & JSMASK_OPTIONS || SecondaryGamepad.InputState.buttons & JSMASK_HOME), &SecondaryGamepad.ButtonsStates.Start, true);

				if (SecondaryGamepad.ButtonsStates.DPADAdvancedMode == false) { // Regular mode  ↑ → ↓ ←
					KeyPress(SecondaryGamepad.ButtonsStates.DPADUp.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_UP, &SecondaryGamepad.ButtonsStates.DPADUp, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADDown.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_DOWN, &SecondaryGamepad.ButtonsStates.DPADDown, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADLeft.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_LEFT, &SecondaryGamepad.ButtonsStates.DPADLeft, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADRight.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_RIGHT, &SecondaryGamepad.ButtonsStates.DPADRight, true);

				}
				else { // Advanced mode ↑ ↗ → ↘ ↓ ↙ ← ↖ for switching in retro games
					KeyPress(SecondaryGamepad.ButtonsStates.DPADUp.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_UP && !(SecondaryGamepad.InputState.buttons & JSMASK_LEFT) && !(SecondaryGamepad.InputState.buttons & JSMASK_RIGHT), &SecondaryGamepad.ButtonsStates.DPADUp, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADLeft.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_LEFT && !(SecondaryGamepad.InputState.buttons & JSMASK_UP) && !(SecondaryGamepad.InputState.buttons & JSMASK_DOWN), &SecondaryGamepad.ButtonsStates.DPADLeft, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADRight.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_RIGHT && !(SecondaryGamepad.InputState.buttons & JSMASK_UP) && !(SecondaryGamepad.InputState.buttons & JSMASK_DOWN), &SecondaryGamepad.ButtonsStates.DPADRight, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADDown.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_DOWN && !(SecondaryGamepad.InputState.buttons & JSMASK_LEFT) && !(SecondaryGamepad.InputState.buttons & JSMASK_RIGHT), &SecondaryGamepad.ButtonsStates.DPADDown, true);

					KeyPress(SecondaryGamepad.ButtonsStates.DPADUpLeft.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_UP && SecondaryGamepad.InputState.buttons & JSMASK_LEFT, &SecondaryGamepad.ButtonsStates.DPADUpLeft, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADUpRight.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_UP && SecondaryGamepad.InputState.buttons & JSMASK_RIGHT, &SecondaryGamepad.ButtonsStates.DPADUpRight, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADDownLeft.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_DOWN && SecondaryGamepad.InputState.buttons & JSMASK_LEFT, &SecondaryGamepad.ButtonsStates.DPADDownLeft, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADDownRight.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_DOWN && SecondaryGamepad.InputState.buttons & JSMASK_RIGHT, &SecondaryGamepad.ButtonsStates.DPADDownRight, true);
				}

				KeyPress(SecondaryGamepad.ButtonsStates.Y.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_N, &SecondaryGamepad.ButtonsStates.Y, true);
				KeyPress(SecondaryGamepad.ButtonsStates.A.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_S, &SecondaryGamepad.ButtonsStates.A, true);
				KeyPress(SecondaryGamepad.ButtonsStates.X.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_W, &SecondaryGamepad.ButtonsStates.X, true);
				KeyPress(SecondaryGamepad.ButtonsStates.B.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_E, &SecondaryGamepad.ButtonsStates.B, true);

				KeyPress(SecondaryGamepad.ButtonsStates.LeftStick.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_LCLICK, &SecondaryGamepad.ButtonsStates.LeftStick, true);
				KeyPress(SecondaryGamepad.ButtonsStates.RightStick.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_RCLICK, &SecondaryGamepad.ButtonsStates.RightStick, true);

				KMStickMode(SecondaryGamepad, DontResetInputState, true, DeadZoneAxis(SecondaryGamepad.InputState.stickLX, SecondaryGamepad.Sticks.DeadZoneLeftX), DeadZoneAxis(SecondaryGamepad.InputState.stickLY, SecondaryGamepad.Sticks.DeadZoneLeftY), SecondaryGamepad.KMEmu.LeftStickMode);
				KMStickMode(SecondaryGamepad, DontResetInputState, false, DeadZoneAxis(SecondaryGamepad.InputState.stickRX, SecondaryGamepad.Sticks.DeadZoneRightX), DeadZoneAxis(SecondaryGamepad.InputState.stickRY, SecondaryGamepad.Sticks.DeadZoneRightY), SecondaryGamepad.KMEmu.RightStickMode);

				if (SecondaryGamepad.ControllerType == NINTENDO_JOYCONS) {	//@042
					KeyPress(SecondaryGamepad.ButtonsStates.JCSL.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_SL, &SecondaryGamepad.ButtonsStates.JCSL, true);
					KeyPress(SecondaryGamepad.ButtonsStates.JCSR.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_SR, &SecondaryGamepad.ButtonsStates.JCSR, true);
					KeyPress(SecondaryGamepad.ButtonsStates.HOME.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_HOME, &SecondaryGamepad.ButtonsStates.HOME, true);
					KeyPress(SecondaryGamepad.ButtonsStates.CAPTURE.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_CAPTURE, &SecondaryGamepad.ButtonsStates.CAPTURE, true);
					KeyPress(SecondaryGamepad.ButtonsStates.MeleeGesture.KeyCode, DontResetInputState && (SecondaryGamepad.Motion.GestureXTimer > 0), &SecondaryGamepad.ButtonsStates.MeleeGesture, true);	//@043
				}
			}
		}

		if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled || (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving && PrimaryGamepad.GamepadActionMode == MotionDrivingMode) || AppStatus.XboxGamepadReset) {
			if (AppStatus.XboxGamepadReset) { AppStatus.XboxGamepadReset = false; XUSB_REPORT_INIT(&report); }
			//if (AppStatus.XboxGamepadAttached)
				//ret = vigem_target_x360_update(client, x360, report);
		}

		if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) { // Temporary hack(Vigem always, no removal)
			XUSB_REPORT_INIT(&report);
			report.sThumbLX = 1; // Maybe the crash is due to power saving? temporary test
			if (AppStatus.SecondaryGamepadEnabled) {
				XUSB_REPORT_INIT(&report2);
				report2.sThumbLX = 1; // Maybe the crash is due to power saving? temporary test
			}
		}
		/*ret = vigem_target_x360_update(client, x360, report);
		if (AppStatus.SecondaryGamepadEnabled)
			ret = vigem_target_x360_update(client2, x3602, report2);*/

			//@044 Выполняем апдейт виртуальных устройств (с конвертацией в DS4 при необходимости)
		if (AppStatus.EmulateDS4) {
			ConvertXusbToDs4(report, ds4_report, (PrimaryGamepad.InputState.buttons & JSMASK_PS) != 0, (PrimaryGamepad.InputState.buttons & JSMASK_TOUCHPAD_CLICK) != 0);
			ret = vigem_target_ds4_update(client, x360, ds4_report);
		}
		else {
			ret = vigem_target_x360_update(client, x360, report);
		}

		if (AppStatus.SecondaryGamepadEnabled) {
			if (AppStatus.EmulateDS4) {
				ConvertXusbToDs4(report2, ds4_report2, (SecondaryGamepad.InputState.buttons & JSMASK_PS) != 0, (SecondaryGamepad.InputState.buttons & JSMASK_TOUCHPAD_CLICK) != 0);
				ret = vigem_target_ds4_update(client2, x3602, ds4_report2);
			}
			else {
				ret = vigem_target_x360_update(client2, x3602, report2);
			}
		}

		// Battery level display
		if (AppStatus.BackOutStateCounter > 0) {
			if (AppStatus.BackOutStateCounter == 1) {
				PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.DefaultModeColor;
				PrimaryGamepad.OutState.PlayersCount = 0;
				if (AppStatus.ShowBatteryStatusOnLightBar) PrimaryGamepad.OutState.LEDBrightness = PrimaryGamepad.LastLEDBrightness;
				GamepadSetState(PrimaryGamepad);

				if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1) {
					SecondaryGamepad.OutState.LEDColor = SecondaryGamepad.DefaultModeColor;
					SecondaryGamepad.OutState.PlayersCount = 0;
					if (AppStatus.ShowBatteryStatusOnLightBar) SecondaryGamepad.OutState.LEDBrightness = SecondaryGamepad.LastLEDBrightness;
					GamepadSetState(SecondaryGamepad);
				}

				AppStatus.ShowBatteryStatus = false;
				MainTextUpdate();
			}
			AppStatus.BackOutStateCounter--;
		}

		if (PrimaryGamepad.RumbleSkipCounter > 0)
			PrimaryGamepad.RumbleSkipCounter--;

		if (AppStatus.SkipPollCount > 0) AppStatus.SkipPollCount--;

		//@060 ТЕЛЕМЕТРИЯ в OSD AHK
		// Вычисляем частоту эмулятора во float (например, 1000.0 / 4 мс = 250.0 Гц)
		/*float loopHz = 1000.0f / (AppStatus.SleepTimeOut == 0 ? 1 : AppStatus.SleepTimeOut);

		//@067 точная частота обновления OSD
		float targetOsdHz = 33.3f;

		// Округляем до ближайшего целого количества пропускаемых кадров
		int maxSkip = static_cast<int>(roundf(loopHz / targetOsdHz));
		if (maxSkip < 1) maxSkip = 1;

		static int telemetrySkipCount = 0;
		telemetrySkipCount++;

		if (telemetrySkipCount >= maxSkip) {
			telemetrySkipCount = 0;

			if (pTelemetry && PrimaryGamepad.DeviceIndex != -1) {
				int aimingHandle = PrimaryGamepad.DeviceIndex;
				if (PrimaryGamepad.DeviceIndex2 != -1 && !AppStatus.GyroFromLeft) {
					aimingHandle = PrimaryGamepad.DeviceIndex2;
				}

				JSL_AUTO_CALIBRATION autoCal = JslGetAutoCalibrationStatus(aimingHandle);
				float bx, by, bz;
				JslGetCalibrationOffset(aimingHandle, bx, by, bz);

				float shakiness = 0.0f, minDeltaAccel = 0.0f;
				JslGetAccelerometerTelemetry(aimingHandle, shakiness, minDeltaAccel);

				float angularVelocity = JslGetAngularVelocity(aimingHandle);

				pTelemetry[0] = autoCal.confidence;
				pTelemetry[1] = autoCal.isSteady ? 1.0f : 0.0f;
				pTelemetry[2] = bx;
				pTelemetry[3] = by;
				pTelemetry[4] = JslGetPollRate(PrimaryGamepad.DeviceIndex);
				pTelemetry[5] = PrimaryGamepad.DeviceIndex2 != -1 ? JslGetPollRate(PrimaryGamepad.DeviceIndex2) : 0.0f;
				pTelemetry[6] = JslGetBattery(PrimaryGamepad.DeviceIndex);
				pTelemetry[7] = PrimaryGamepad.DeviceIndex2 != -1 ? JslGetBattery(PrimaryGamepad.DeviceIndex2) : -1.0f;
				pTelemetry[8] = (float)JslGetControllerType(PrimaryGamepad.DeviceIndex);
				pTelemetry[9] = PrimaryGamepad.DeviceIndex2 != -1 ? (float)JslGetControllerType(PrimaryGamepad.DeviceIndex2) : 0.0f;
				pTelemetry[10] = shakiness;
				pTelemetry[11] = minDeltaAccel;
				pTelemetry[12] = angularVelocity;

				//@075 Нативный RTSS Direct OSD
				if (AppStatus.OsdType == 1 && AppStatus.IsOsdActive) {
					static float peakAngularVel = 0.0f;
					static ULONGLONG peakTimestamp = 0;
					ULONGLONG nowMs = GetTickCount64();

					if (angularVelocity >= peakAngularVel) {
						peakAngularVel = angularVelocity;
						peakTimestamp = nowMs;
					}
					else if (nowMs - peakTimestamp > 800) {
						peakAngularVel = angularVelocity;
					}

					char rtssBuf[1024];
					sprintf_s(rtssBuf, sizeof(rtssBuf),
						"<P=%d,%d><C=FF9F00>LT  : %-6d | RT   : %-6d\n"
						"LS X: %-6d | Y    : %-6d\n"
						"RS X: %-6d | Y    : %-6d\n"
						"<C=555555>------------------------------\n"
						"<C=00FFFF>Calib: %4.0f%% | Steady: %s\n"
						"BiasX: %5.2f | BiasY: %5.2f\n"
						//"Gyro : %4.0f deg/s | Peak: %4.0f deg/s\n"
						"Gyro:%4.0f\xB0/s | Peak:%4.0f\xB0/s\n"
						"Shake: %5.3f | MinAc: %5.3f\n"
						"<C=555555>------------------------------\n"
						"<C=00FF00>%s: %3.0fHz | Bat: %3.0f%%%s",
						AppStatus.OsdPosX, AppStatus.OsdPosY,
						report.bLeftTrigger, report.bRightTrigger,
						report.sThumbLX, report.sThumbLY,
						report.sThumbRX, report.sThumbRY,
						autoCal.confidence * 100.0f,
						autoCal.isSteady ? "YES" : "NO ",
						bx, by,
						angularVelocity, peakAngularVel,
						shakiness, minDeltaAccel,
						GetDevShortName(PrimaryGamepad.ControllerType),
						pTelemetry[4], pTelemetry[6],
						(PrimaryGamepad.DeviceIndex2 != -1) ? "\n<C=00FF00>Joy-Con R" : ""
					);

					UpdateRTSS(rtssBuf);
				}
			}
		}*/
		//@076 СЕРВИСНЫЙ КОНТУР 33.3 Гц (Slow Path: Телеметрия, автокалибровка (чтоб не дёргал часто mutex), Диагностика)
		float loopHz = 1000.0f / (AppStatus.SleepTimeOut == 0 ? 1 : AppStatus.SleepTimeOut);
		float targetServiceHz = 33.3f;
		int maxServiceSkip = static_cast<int>(roundf(loopHz / targetServiceHz));
		if (maxServiceSkip < 1) maxServiceSkip = 1;

		static int serviceSkipCount = 0;
		serviceSkipCount++;

		if (serviceSkipCount >= maxServiceSkip) {
			serviceSkipCount = 0;

			// Сервис 1: Диагностика мёртвых зон стиков (Alt + F9)
			if (AppStatus.DeadZoneMode) {
				printf("\rLS: X=%5.2f Y=%5.2f | RS: X=%5.2f Y=%5.2f | LT=%4.2f RT=%4.2f ",
					abs(PrimaryGamepad.InputState.stickLX), abs(PrimaryGamepad.InputState.stickLY),
					abs(PrimaryGamepad.InputState.stickRX), abs(PrimaryGamepad.InputState.stickRY),
					abs(PrimaryGamepad.InputState.lTrigger), abs(PrimaryGamepad.InputState.rTrigger));
			}

			//@050 Сервис 2. Мониторинг калибровки и фоновые звуки
			int aimingHandle = PrimaryGamepad.DeviceIndex;
			if (PrimaryGamepad.DeviceIndex2 != -1 && !AppStatus.GyroFromLeft) {
				aimingHandle = PrimaryGamepad.DeviceIndex2;
			}

			if (aimingHandle != -1) {
				static bool wasSteadyAndConfident = false;
				JSL_AUTO_CALIBRATION autoCal = JslGetAutoCalibrationStatus(aimingHandle);
				bool isSteadyAndConfident = (autoCal.isSteady && autoCal.confidence >= 1.0f);

				if (isSteadyAndConfident && !wasSteadyAndConfident) {
					if (!AppStatus.StartupCalibrationFrozen) {
						if (!AppStatus.AutoCalibrationEnabled) {
							JslSetAutomaticCalibration(aimingHandle, false);
						}
						AppStatus.StartupCalibrationFrozen = true;
						PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
					}
					else if (AppStatus.AutoCalibrationEnabled && AppStatus.BackgroundCalibSound) {
						PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
					}
				}
				wasSteadyAndConfident = isSteadyAndConfident;

				// --- Сервис 3: Запись телеметрии (AHK Shared Memory + RTSS Direct) ---
				float bx, by, bz;
				JslGetCalibrationOffset(aimingHandle, bx, by, bz);
				float shakiness = 0.0f, minDeltaAccel = 0.0f;
				JslGetAccelerometerTelemetry(aimingHandle, shakiness, minDeltaAccel);
				float angularVelocity = JslGetAngularVelocity(aimingHandle);

				if (pTelemetry) {
					pTelemetry[0] = autoCal.confidence;
					pTelemetry[1] = autoCal.isSteady ? 1.0f : 0.0f;
					pTelemetry[2] = bx;
					pTelemetry[3] = by;
					pTelemetry[4] = JslGetPollRate(PrimaryGamepad.DeviceIndex);
					pTelemetry[5] = PrimaryGamepad.DeviceIndex2 != -1 ? JslGetPollRate(PrimaryGamepad.DeviceIndex2) : 0.0f;
					pTelemetry[6] = JslGetBattery(PrimaryGamepad.DeviceIndex);
					pTelemetry[7] = PrimaryGamepad.DeviceIndex2 != -1 ? JslGetBattery(PrimaryGamepad.DeviceIndex2) : -1.0f;
					pTelemetry[8] = (float)JslGetControllerType(PrimaryGamepad.DeviceIndex);
					pTelemetry[9] = PrimaryGamepad.DeviceIndex2 != -1 ? (float)JslGetControllerType(PrimaryGamepad.DeviceIndex2) : 0.0f;
					pTelemetry[10] = shakiness;
					pTelemetry[11] = minDeltaAccel;
					pTelemetry[12] = angularVelocity;
				}

				if (AppStatus.OsdType == 1 && AppStatus.IsOsdActive) {
					static float peakAngularVel = 0.0f;
					static ULONGLONG peakTimestamp = 0;
					ULONGLONG nowMs = GetTickCount64();

					if (angularVelocity >= peakAngularVel) {
						peakAngularVel = angularVelocity;
						peakTimestamp = nowMs;
					}
					else if (nowMs - peakTimestamp > 800) {
						peakAngularVel = angularVelocity;
					}

					// Формируем строку для второго контроллера (если подключена пара Joy-Con)
					char dev2Buf[128] = "";
					if (PrimaryGamepad.DeviceIndex2 != -1) {
						sprintf_s(dev2Buf, sizeof(dev2Buf),
							"\n<C=00FF00>%-9s: %3.0fHz | Bat: %3.0f%%",
							GetJslDevName(PrimaryGamepad.DeviceIndex2),
							pTelemetry ? pTelemetry[5] : JslGetPollRate(PrimaryGamepad.DeviceIndex2),
							pTelemetry ? pTelemetry[7] : JslGetBattery(PrimaryGamepad.DeviceIndex2)
						);
					}

					char rtssBuf[1024];
					sprintf_s(rtssBuf, sizeof(rtssBuf),
						"<P=%d,%d><C=FF9F00>LT  : %-6d | RT  : %-6d\n"
						"LS X: %-6d | RS X: %-6d\n"
						"LS Y: %-6d | RS Y: %-6d\n"
						"<C=555555>------------------------------\n"
						"<C=00FFFF>Calib: %4.0f%% | Steady: %s\n"
						"BiasX: %5.2f | BiasY: %5.2f\n"
						"Gyro:%4.0f\xB0/s | Peak:%4.0f\xB0/s\n"
						"Shake: %5.3f | MinAc: %5.3f\n"
						"<C=555555>------------------------------\n"
						"<C=00FF00>%-9s: %3.0fHz | Bat: %3.0f%%%s",
						AppStatus.OsdPosX, AppStatus.OsdPosY,
						report.bLeftTrigger, report.bRightTrigger,
						report.sThumbLX, report.sThumbLY,
						report.sThumbRX, report.sThumbRY,
						autoCal.confidence * 100.0f,
						autoCal.isSteady ? "YES" : "NO ",
						bx, by,
						angularVelocity, peakAngularVel,
						shakiness, minDeltaAccel,
						GetJslDevName(PrimaryGamepad.DeviceIndex),
						pTelemetry ? pTelemetry[4] : JslGetPollRate(PrimaryGamepad.DeviceIndex),
						pTelemetry ? pTelemetry[6] : JslGetBattery(PrimaryGamepad.DeviceIndex),
						dev2Buf
					);

					UpdateRTSS(rtssBuf);
				}
			}
		}
		Sleep(AppStatus.SleepTimeOut);
	}

	timeEndPeriod(1);

	/*if (AppStatus.IsOsdActive) {
		system("taskkill /IM OSD.exe /F > nul 2>&1");
	}*/

	if (AppStatus.IsOsdActive) {	// Завершение OSD через Win32 API без консольных пауз (taskkill)
		HWND hOsdWnd = FindWindowA(NULL, "OSD");
		if (hOsdWnd) {
			PostMessage(hOsdWnd, WM_CLOSE, 0, 0);
		}
		else {
			system("taskkill /IM OSD.exe /F > nul 2>&1");
		}
	}

	// Reset keyboard motion driving
	if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse && PrimaryGamepad.GamepadActionMode == MotionDrivingMode) {
		KeyPress(PrimaryGamepad.ButtonsStates.DPADLeft.KeyCode, false, &PrimaryGamepad.ButtonsStates.DPADLeft, true);
		KeyPress(PrimaryGamepad.ButtonsStates.DPADRight.KeyCode, false, &PrimaryGamepad.ButtonsStates.DPADRight, true);
	}

	// Reset adaptive triggers
	if (PrimaryGamepad.AdaptiveTriggersMode > 0) {
		PrimaryGamepad.AdaptiveTriggersOutputMode = 0;
		GamepadSetState(PrimaryGamepad);
	}

	JslDisconnectAndDisposeAll();
	if (PrimaryGamepad.HidHandle != NULL)
		hid_close(PrimaryGamepad.HidHandle);

	if (AppStatus.ExternalPedalsArduinoConnected) {
		AppStatus.ExternalPedalsArduinoConnected = false;
		pArduinoReadThread->join();
		delete pArduinoReadThread;
		pArduinoReadThread = nullptr;
		CloseHandle(hSerial);
	}

	/*if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled || (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving)) {
	vigem_target_x360_unregister_notification(x360);
	vigem_target_remove(client, x360);
	vigem_target_free(x360);
	}*/

	// Очистка первого таргета ViGEm с учетом типа контроллера
	if (x360) {
		vigem_target_remove(client, x360);
		vigem_target_free(x360);
	}

	vigem_disconnect(client);
	vigem_free(client);

	/*if (AppStatus.SecondaryGamepadEnabled) {
		vigem_target_x360_unregister_notification(x3602);
		vigem_target_remove(client2, x3602);
		vigem_target_free(x3602);

		vigem_disconnect(client2);
		vigem_free(client2);
		if (pTelemetry) UnmapViewOfFile(pTelemetry); //060
		if (hMapFile) CloseHandle(hMapFile);
		if (pTelemetryAIDA) UnmapViewOfFile(pTelemetryAIDA);
		if (hMapFileAIDA) CloseHandle(hMapFileAIDA);
	}*/

	// Очистка второго таргета ViGEm
	if (AppStatus.SecondaryGamepadEnabled && x3602) {
		vigem_target_remove(client2, x3602);
		vigem_target_free(x3602);

		vigem_disconnect(client2);
		vigem_free(client2);
	}

	// Глобальное освобождение Shared Memory (для любого числа геймпадов)
	if (pTelemetry) UnmapViewOfFile(pTelemetry);
	if (hMapFile) CloseHandle(hMapFile);
	//if (pTelemetryAIDA) UnmapViewOfFile(pTelemetryAIDA);
	//if (hMapFileAIDA) CloseHandle(hMapFileAIDA);
	if (AppStatus.OsdType == 1) {	//@075
		CloseRTSS();
	}

	return 0;
}