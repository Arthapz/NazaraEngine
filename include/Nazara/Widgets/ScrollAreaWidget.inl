// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Engine - Widgets module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Widgets/Debug.hpp>

namespace Nz
{
	inline bool ScrollAreaWidget::HasScrollbar() const
	{
		return m_hasScrollbar;
	}

	inline bool ScrollAreaWidget::IsScrollbarEnabled() const
	{
		return m_isScrollbarEnabled;
	}

	inline bool ScrollAreaWidget::IsScrollbarVisible() const
	{
		return HasScrollbar() && IsScrollbarEnabled();
	}

	inline void ScrollAreaWidget::ScrollToHeight(float height)
	{
		float contentHeight = m_content->GetHeight();
		ScrollToRatio(height / contentHeight);
	}
}

#include <Nazara/Widgets/DebugOff.hpp>
