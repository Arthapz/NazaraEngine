// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Engine - Core module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NAZARA_CORE_CALLONEXIT_HPP
#define NAZARA_CORE_CALLONEXIT_HPP

#include <Nazara/Prerequisites.hpp>
#include <functional>

namespace Nz
{
	class CallOnExit
	{
		using Func = std::function<void()>;

		public:
			CallOnExit(Func func = nullptr);
			CallOnExit(const CallOnExit&) = delete;
			CallOnExit(CallOnExit&&) noexcept = delete;
			~CallOnExit();

			void CallAndReset(Func func = nullptr);
			void Reset(Func func = nullptr);

			CallOnExit& operator=(const CallOnExit&) = delete;
			CallOnExit& operator=(CallOnExit&&) noexcept = default;

		private:
			Func m_func;
	};
}

#include <Nazara/Core/CallOnExit.inl>

#endif // NAZARA_CORE_CALLONEXIT_HPP
