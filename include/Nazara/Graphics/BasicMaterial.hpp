// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Engine - Graphics module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NAZARA_GRAPHICS_BASICMATERIAL_HPP
#define NAZARA_GRAPHICS_BASICMATERIAL_HPP

#include <Nazara/Prerequisites.hpp>
#include <Nazara/Graphics/MaterialPass.hpp>

namespace Nz
{
	class FieldOffsets;

	class NAZARA_GRAPHICS_API BasicMaterial
	{
		friend class MaterialPipeline;

		public:
			struct UniformOffsets;

			BasicMaterial(MaterialPass& material);
			~BasicMaterial() = default;

			inline void EnableAlphaTest(bool alphaTest);

			inline const std::shared_ptr<Texture>& GetAlphaMap() const;
			inline const TextureSamplerInfo& GetAlphaSampler() const;
			float GetAlphaTestThreshold() const;
			Color GetDiffuseColor() const;
			inline const std::shared_ptr<Texture>& GetDiffuseMap() const;
			inline const TextureSamplerInfo& GetDiffuseSampler() const;

			inline bool IsAlphaTestEnabled() const;

			inline bool HasAlphaMap() const;
			inline bool HasAlphaTest() const;
			inline bool HasAlphaTestThreshold() const;
			inline bool HasDiffuseColor() const;
			inline bool HasDiffuseMap() const;

			inline void SetAlphaMap(std::shared_ptr<Texture> alphaMap);
			inline void SetAlphaSampler(TextureSamplerInfo alphaSampler);
			void SetAlphaTestThreshold(float alphaThreshold);
			void SetDiffuseColor(const Color& diffuse);
			inline void SetDiffuseMap(std::shared_ptr<Texture> diffuseMap);
			inline void SetDiffuseSampler(TextureSamplerInfo diffuseSampler);

			static inline const UniformOffsets& GetOffsets();
			static inline const std::shared_ptr<MaterialSettings>& GetSettings();

			struct UniformOffsets
			{
				std::size_t alphaThreshold;
				std::size_t diffuseColor;
				std::size_t totalSize;
			};

		protected:
			struct OptionIndexes
			{
				std::size_t alphaTest;
				std::size_t hasAlphaMap;
				std::size_t hasDiffuseMap;
			};

			struct TextureIndexes
			{
				std::size_t alpha;
				std::size_t diffuse;
			};

			static MaterialSettings::Builder Build(const UniformOffsets& offsets, std::vector<UInt8> defaultValues, std::vector<std::shared_ptr<UberShader>> uberShaders, std::size_t* uniformBlockIndex = nullptr, OptionIndexes* optionIndexes = nullptr, TextureIndexes* textureIndexes = nullptr);
			static std::vector<std::shared_ptr<UberShader>> BuildShaders();
			static std::pair<UniformOffsets, FieldOffsets> BuildUniformOffsets();

		private:
			static bool Initialize();
			static void Uninitialize();

			MaterialPass& m_material;
			std::size_t m_uniformBlockIndex;
			OptionIndexes m_optionIndexes;
			TextureIndexes m_textureIndexes;
			UniformOffsets m_uniformOffsets;

			static std::shared_ptr<MaterialSettings> s_materialSettings;
			static std::size_t s_uniformBlockIndex;
			static OptionIndexes s_optionIndexes;
			static TextureIndexes s_textureIndexes;
			static UniformOffsets s_uniformOffsets;
	};
}

#include <Nazara/Graphics/BasicMaterial.inl>

#endif // NAZARA_GRAPHICS_BASICMATERIAL_HPP
