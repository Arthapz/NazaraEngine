// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Nazara Engine - Graphics module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Graphics/Material.hpp>
#include <Nazara/Core/ErrorFlags.hpp>
#include <Nazara/Graphics/BasicMaterial.hpp>
#include <Nazara/Renderer/CommandBufferBuilder.hpp>
#include <Nazara/Renderer/Renderer.hpp>
#include <Nazara/Renderer/RenderFrame.hpp>
#include <Nazara/Renderer/UploadPool.hpp>
#include <Nazara/Utility/MaterialData.hpp>
#include <Nazara/Graphics/Debug.hpp>

namespace Nz
{
	/*!
	* \ingroup graphics
	* \class Nz::Material
	* \brief Graphics class that represents a material
	*/

	/*!
	* \brief Constructs a Material object with default states
	*
	* \see Reset
	*/
	Material::Material(std::shared_ptr<const MaterialSettings> settings) :
	m_settings(std::move(settings)),
	m_enabledConditions(0),
	m_pipelineUpdated(false),
	m_shaderBindingUpdated(false),
	m_shadowCastingEnabled(true)
	{
		m_pipelineInfo.settings = m_settings;

		const auto& shaders = m_settings->GetShaders();
		for (std::size_t i = 0; i < ShaderStageTypeCount; ++i)
			m_pipelineInfo.shaders[i].uberShader = shaders[i];

		const auto& textureSettings = m_settings->GetTextures();
		const auto& uboSettings = m_settings->GetUniformBlocks();

		m_textures.resize(m_settings->GetTextures().size());

		m_uniformBuffers.reserve(m_settings->GetUniformBlocks().size());
		for (const auto& uniformBufferInfo : m_settings->GetUniformBlocks())
		{
			auto& uniformBuffer = m_uniformBuffers.emplace_back();

			uniformBuffer.buffer = Graphics::Instance()->GetRenderDevice()->InstantiateBuffer(Nz::BufferType::Uniform);
			if (!uniformBuffer.buffer->Initialize(uniformBufferInfo.blockSize, BufferUsage::Dynamic))
				throw std::runtime_error("failed to initialize UBO memory");

			assert(uniformBufferInfo.defaultValues.size() <= uniformBufferInfo.blockSize);

			uniformBuffer.data.resize(uniformBufferInfo.blockSize);
			std::memcpy(uniformBuffer.data.data(), uniformBufferInfo.defaultValues.data(), uniformBufferInfo.defaultValues.size());
		}

		UpdateShaderBinding();
	}

	bool Material::Update(RenderFrame& renderFrame, CommandBufferBuilder& builder)
	{
		bool shouldRegenerateCommandBuffer = false;
		if (!m_shaderBindingUpdated)
		{
			shouldRegenerateCommandBuffer = true;

			renderFrame.PushForRelease(std::move(m_shaderBinding));
			m_shaderBinding.reset();

			UpdateShaderBinding();
		}

		UploadPool& uploadPool = renderFrame.GetUploadPool();

		for (auto& ubo : m_uniformBuffers)
		{
			if (ubo.dataInvalidated)
			{
				auto& allocation = uploadPool.Allocate(ubo.data.size());
				std::memcpy(allocation.mappedPtr, ubo.data.data(), ubo.data.size());

				builder.CopyBuffer(allocation, ubo.buffer.get());

				ubo.dataInvalidated = false;
			}
		}

		return shouldRegenerateCommandBuffer;
	}

	void Material::UpdateShaderBinding()
	{
		assert(!m_shaderBinding);

		const auto& textureSettings = m_settings->GetTextures();
		const auto& uboSettings = m_settings->GetUniformBlocks();

		// TODO: Use StackVector
		std::vector<ShaderBinding::Binding> bindings;

		// Textures
		for (std::size_t i = 0; i < m_textures.size(); ++i)
		{
			const auto& textureSetting = textureSettings[i];
			const auto& textureSlot = m_textures[i];

			if (!textureSlot.sampler)
			{
				TextureSamplerCache& samplerCache = Graphics::Instance()->GetSamplerCache();
				textureSlot.sampler = samplerCache.Get(textureSlot.samplerInfo);
			}

			//TODO: Use "missing" texture
			if (textureSlot.texture)
			{
				bindings.push_back({
					textureSetting.bindingIndex,
					ShaderBinding::TextureBinding {
						textureSlot.texture.get(), textureSlot.sampler.get()
					}
				});
			}
		}

		// Shared UBO (TODO)

		// Owned UBO
		for (std::size_t i = 0; i < m_uniformBuffers.size(); ++i)
		{
			const auto& uboSetting = uboSettings[i];
			const auto& uboSlot = m_uniformBuffers[i];

			bindings.push_back({
				uboSetting.bindingIndex,
				ShaderBinding::UniformBufferBinding {
					uboSlot.buffer.get(), 0, uboSlot.buffer->GetSize()
				}
			});
		}

		m_shaderBinding = m_settings->GetRenderPipelineLayout()->AllocateShaderBinding(Graphics::MaterialBindingSet);
		m_shaderBinding->Update(bindings.data(), bindings.size());

		m_shaderBindingUpdated = true;
	}
}
