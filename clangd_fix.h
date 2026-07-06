#pragma once

// 1. Стандартная библиотека (STL)
#include <vector>
#include <string>
#include <thread>
#include <algorithm>
#include <iostream>
#include <mutex>
#include <atomic>

// 2. Базовый Windows API
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <joystickapi.h>

