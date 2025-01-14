// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Engine - Core module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Core/Log.hpp>
#include <Nazara/Core/Debug.hpp>

namespace Nz
{
	template<typename T>
	ModuleBase<T>::ModuleBase(std::string moduleName, T* pointer) :
	ModuleBase(std::move(moduleName), pointer, NoLog{})
	{
		LogInit();
	}

	template<typename T>
	ModuleBase<T>::ModuleBase(std::string moduleName, T* pointer, NoLog) :
	m_moduleName(std::move(moduleName))
	{
		NazaraAssert(T::s_instance == nullptr, "only one instance of " + m_moduleName + " can exist at a given time");
		T::s_instance = pointer;
	}

	template<typename T>
	ModuleBase<T>::~ModuleBase()
	{
		LogUninit();
		T::s_instance = nullptr;
	}

	template<typename T>
	T* ModuleBase<T>::Instance()
	{
		return T::s_instance;
	}

	template<typename T>
	void ModuleBase<T>::LogInit()
	{
		NazaraNotice("Initializing " + m_moduleName + "...");
	}

	template<typename T>
	void ModuleBase<T>::LogUninit()
	{
		if (m_moduleName.empty())
			return;

		NazaraNotice("Uninitialized " + m_moduleName);
		m_moduleName.clear();
	}
}

#include <Nazara/Core/DebugOff.hpp>
