#pragma once

#include "D2Types.h"

inline BOOL IsD2R(DWORD dwVersion) { return dwVersion >= 0x61; }
inline BOOL IsPtr24AndAbove(DWORD dwVersion) { return dwVersion >= 0x62; }
inline BOOL IsPtr25AndAbove(DWORD dwVersion) { return dwVersion >= 0x63; }
inline BOOL IsRotWAndAbove(DWORD dwVersion) { return dwVersion >= 0x64; }	// 术士君临版本 (Reign of the Warlock)
inline BOOL IsPtr31AndAbove(DWORD dwVersion) { return dwVersion >= 0x69; }	// 3.1版本 (Name[16]被移除, NamePTR扩展)

inline BOOL IsValidVersion(DWORD dwVersion) {
	switch (dwVersion)
	{
		case 0x69:	// 3.1 (Reign of the Warlock 3.1.x)
		case 0x64:	// 术士君临 (Reign of the Warlock)
		case 0x63:
		case 0x62:
		case 0x61:
		case 0x60:
		case 0x5C:
		case 0x59:
		case 0x57:
		case 0x47:return TRUE;
		default:
			return FALSE;
	}
}
