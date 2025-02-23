// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Engine - Utility module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Utility/Joint.hpp>
#include <Nazara/Utility/Debug.hpp>

namespace Nz
{
	void Joint::InvalidateNode(Invalidation invalidation)
	{
		Node::InvalidateNode(invalidation);

		m_skinningMatrixUpdated = false;
	}
}
