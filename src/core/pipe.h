#pragma once
#ifdef __unix__
#include "linux/pipe.h"
#else
#include "win32/pipe.h"
#endif
