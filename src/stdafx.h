#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winternl.h>
#include <imagehlp.h>

#include "psapi.h"
#include <iostream>
#include "memoryapi.h"
#include "processthreadsapi.h"

#include "../src/Defs.h"