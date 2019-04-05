#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers
/* Windows Header Files */
#include <windows.h>
#include <commctrl.h>

#include "resource.h"

extern "C" {
#include "mtrx_t.h"
#include "memrealloc.h"
}

/* C RunTime Header Files */
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

