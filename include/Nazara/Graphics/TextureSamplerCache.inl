// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Engine - Graphics module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <functional>
#include <Nazara/Graphics/Debug.hpp>

namespace Nz
{
	inline TextureSamplerCache::TextureSamplerCache(std::shared_ptr<RenderDevice> device) :
	m_device(std::move(device))
	{
	}
}

#include <Nazara/Graphics/DebugOff.hpp>
