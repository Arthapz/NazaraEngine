// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Engine - Core module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Core/Win32/Utils.hpp>
#include <Nazara/Core/Debug.hpp>

namespace Nz
{
	time_t FileTimeToTime(FILETIME* time)
	{
		SYSTEMTIME stUTC, stLocal;

		FileTimeToSystemTime(time, &stUTC);
		SystemTimeToTzSpecificLocalTime(nullptr, &stUTC, &stLocal);

		std::tm timeinfo;
		timeinfo.tm_sec = stLocal.wSecond;
		timeinfo.tm_min = stLocal.wMinute;
		timeinfo.tm_hour = stLocal.wHour;
		timeinfo.tm_mday = stLocal.wDay;
		timeinfo.tm_mon = stLocal.wMonth-1;
		timeinfo.tm_year = stLocal.wYear-1900;
		timeinfo.tm_isdst = -1;

		return std::mktime(&timeinfo);
	}
}

#include <Nazara/Core/AntiWindows.hpp>
