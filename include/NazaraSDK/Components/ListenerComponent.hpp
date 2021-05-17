// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Nazara Development Kit"
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#pragma once

#ifndef NDK_COMPONENTS_LISTENERCOMPONENT_HPP
#define NDK_COMPONENTS_LISTENERCOMPONENT_HPP

#include <NazaraSDK/ClientPrerequisites.hpp>
#include <NazaraSDK/Component.hpp>

namespace Ndk
{
	class ListenerComponent;

	using ListenerComponentHandle = Nz::ObjectHandle<ListenerComponent>;

	class NDK_CLIENT_API ListenerComponent : public Component<ListenerComponent>
	{
		public:
			inline ListenerComponent();
			~ListenerComponent() = default;

			inline bool IsActive() const;
			inline void SetActive(bool active = true);

			static ComponentIndex componentIndex;

		private:
			bool m_isActive;
	};
}

#include <NazaraSDK/Components/ListenerComponent.inl>

#endif // NDK_COMPONENTS_LISTENERCOMPONENT_HPP
