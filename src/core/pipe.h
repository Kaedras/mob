#pragma once
#ifdef __unix__
#include "linux/pipe_linux.h"
#else
#include "win32/pipe_win32.h"
#endif
