// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Engine - Platform module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NAZARA_PLATFORM_CURSOR_HPP
#define NAZARA_PLATFORM_CURSOR_HPP

#include <NazaraUtils/Prerequisites.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Platform/Config.hpp>
#include <Nazara/Platform/Enums.hpp>
#include <Nazara/Utility/Image.hpp>
#include <NazaraUtils/EnumArray.hpp>
#include <array>

namespace Nz
{
	class CursorImpl;

	class NAZARA_PLATFORM_API Cursor
	{
		friend class Platform;
		friend class WindowImpl;

		public:
			Cursor();
			Cursor(const Image& cursor, const Vector2i& hotSpot, SystemCursor placeholder);
			Cursor(const Cursor&) = delete;
			Cursor(Cursor&&) noexcept;
			~Cursor();

			bool Create(const Image& cursor, const Vector2i& hotSpot, SystemCursor placeholder);
			void Destroy();

			inline SystemCursor GetSystemCursor() const;

			inline bool IsValid() const;

			Cursor& operator=(const Cursor&) = delete;
			Cursor& operator=(Cursor&&) noexcept;

			static inline std::shared_ptr<Cursor>& Get(SystemCursor cursor);

		private:
			explicit Cursor(SystemCursor systemCursor);

			bool Create(SystemCursor cursor);

			static bool Initialize();
			static void Uninitialize();

			SystemCursor m_systemCursor;
			std::unique_ptr<CursorImpl> m_impl;

			static EnumArray<SystemCursor, std::shared_ptr<Cursor>> s_systemCursors;
	};
}

#include <Nazara/Platform/Cursor.inl>

#endif // NAZARA_PLATFORM_CURSOR_HPP
