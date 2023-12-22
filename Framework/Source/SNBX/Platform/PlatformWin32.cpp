// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#include "SNBX/Defines.hpp"
#include "Platform.hpp"

#ifdef SNBX_WIN

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace SNBX
{
	CPtr Platform::LoadSharedLib(const StringView& library)
	{
		char buffer[MAX_PATH];
		int  i      = 0;
		for (; i < library.size(); ++i)
		{
			buffer[i] = library[i];
		}
		buffer[i++] = '.';
		buffer[i++] = 'd';
		buffer[i++] = 'l';
		buffer[i++] = 'l';
		buffer[i++] = '\0';
		return LoadLibraryA(buffer);
	}

	CPtr Platform::GetAddress(CPtr lib, const char* funcName)
	{
		return (CPtr) GetProcAddress((HMODULE) lib, funcName);
	}
}

#endif