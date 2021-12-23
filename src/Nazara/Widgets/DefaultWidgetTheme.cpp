// Copyright (C) 2021 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Engine - Widgets module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Widgets/DefaultWidgetTheme.hpp>
#include <Nazara/Graphics/BasicMaterial.hpp>
#include <Nazara/Graphics/Material.hpp>
#include <Nazara/Graphics/MaterialPass.hpp>
#include <Nazara/Widgets/SimpleWidgetStyles.hpp>
#include <Nazara/Widgets/Widgets.hpp>
#include <Nazara/Widgets/Debug.hpp>

namespace Nz
{
	namespace
	{
		const UInt8 ButtonImage[] = {
			#include <Nazara/Widgets/Resources/DefaultTheme/Button.png.h>
		};

		const UInt8 ButtonHoveredImage[] = {
			#include <Nazara/Widgets/Resources/DefaultTheme/ButtonHovered.png.h>
		};

		const UInt8 ButtonPressedImage[] = {
			#include <Nazara/Widgets/Resources/DefaultTheme/ButtonPressed.png.h>
		};

		const UInt8 ButtonPressedHoveredImage[] = {
			#include <Nazara/Widgets/Resources/DefaultTheme/ButtonPressedHovered.png.h>
		};

		const UInt8 CheckboxBackgroundImage[] = {
			#include <Nazara/Widgets/Resources/DefaultTheme/CheckboxBackground.png.h>
		};

		const UInt8 CheckboxBackgroundHoveredImage[] = {
			#include <Nazara/Widgets/Resources/DefaultTheme/CheckboxBackgroundHovered.png.h>
		};

		const UInt8 CheckboxCheckImage[] = {
			#include <Nazara/Widgets/Resources/DefaultTheme/CheckboxCheck.png.h>
		};

		const UInt8 CheckboxTristateImage[] = {
			#include <Nazara/Widgets/Resources/DefaultTheme/CheckboxTristate.png.h>
		};
	}

	DefaultWidgetTheme::DefaultWidgetTheme()
	{
		TextureParams texParams;
		texParams.renderDevice = Graphics::Instance()->GetRenderDevice();
		texParams.loadFormat = PixelFormat::RGBA8_SRGB;

		auto CreateMaterialFromTexture = [](std::shared_ptr<Texture> texture)
		{
			std::shared_ptr<MaterialPass> buttonMaterialPass = std::make_shared<MaterialPass>(BasicMaterial::GetSettings());
			buttonMaterialPass->EnableDepthBuffer(true);
			buttonMaterialPass->EnableDepthWrite(false);
			buttonMaterialPass->EnableBlending(true);
			buttonMaterialPass->SetBlendEquation(BlendEquation::Add, BlendEquation::Add);
			buttonMaterialPass->SetBlendFunc(BlendFunc::SrcAlpha, BlendFunc::InvSrcAlpha, BlendFunc::One, BlendFunc::One);

			std::shared_ptr<Material> material = std::make_shared<Material>();
			material->AddPass("ForwardPass", buttonMaterialPass);

			BasicMaterial buttonBasicMat(*buttonMaterialPass);
			buttonBasicMat.SetDiffuseMap(texture);

			return material;
		};
		
		// Button material
		m_buttonMaterial = CreateMaterialFromTexture(Texture::LoadFromMemory(ButtonImage, sizeof(ButtonImage), texParams));
		m_buttonHoveredMaterial = CreateMaterialFromTexture(Texture::LoadFromMemory(ButtonHoveredImage, sizeof(ButtonHoveredImage), texParams));
		m_buttonPressedMaterial = CreateMaterialFromTexture(Texture::LoadFromMemory(ButtonPressedImage, sizeof(ButtonPressedImage), texParams));
		m_buttonPressedHoveredMaterial = CreateMaterialFromTexture(Texture::LoadFromMemory(ButtonPressedHoveredImage, sizeof(ButtonPressedHoveredImage), texParams));

		// Checkbox material
		m_checkboxBackgroundMaterial = CreateMaterialFromTexture(Texture::LoadFromMemory(CheckboxBackgroundImage, sizeof(CheckboxBackgroundImage), texParams));
		m_checkboxBackgroundHoveredMaterial = CreateMaterialFromTexture(Texture::LoadFromMemory(CheckboxBackgroundHoveredImage, sizeof(CheckboxBackgroundHoveredImage), texParams));
		m_checkboxCheckMaterial = CreateMaterialFromTexture(Texture::LoadFromMemory(CheckboxCheckImage, sizeof(CheckboxCheckImage), texParams));
		m_checkboxTristateMaterial = CreateMaterialFromTexture(Texture::LoadFromMemory(CheckboxTristateImage, sizeof(CheckboxTristateImage), texParams));
	}

	std::unique_ptr<ButtonWidgetStyle> DefaultWidgetTheme::CreateStyle(ButtonWidget* buttonWidget) const
	{
		SimpleButtonWidgetStyle::StyleConfig styleConfig;
		styleConfig.cornerSize = 20.f;
		styleConfig.cornerTexCoords = 20.f / 128.f;
		styleConfig.hoveredMaterial = m_buttonHoveredMaterial;
		styleConfig.material = m_buttonMaterial;
		styleConfig.pressedHoveredMaterial = m_buttonPressedHoveredMaterial;
		styleConfig.pressedMaterial = m_buttonPressedMaterial;

		return std::make_unique<SimpleButtonWidgetStyle>(buttonWidget, styleConfig);
	}

	std::unique_ptr<CheckboxWidgetStyle> DefaultWidgetTheme::CreateStyle(CheckboxWidget* checkboxWidget) const
	{
		SimpleCheckboxWidgetStyle::StyleConfig styleConfig;
		styleConfig.backgroundCornerSize = 10.f;
		styleConfig.backgroundCornerTexCoords = 10.f / 64.f;
		styleConfig.backgroundHoveredMaterial = m_checkboxBackgroundHoveredMaterial;
		styleConfig.backgroundMaterial = m_checkboxBackgroundMaterial;
		styleConfig.checkMaterial = m_checkboxCheckMaterial;
		styleConfig.tristateMaterial = m_checkboxTristateMaterial;

		return std::make_unique<SimpleCheckboxWidgetStyle>(checkboxWidget, styleConfig);
	}

	std::unique_ptr<LabelWidgetStyle> DefaultWidgetTheme::CreateStyle(LabelWidget* labelWidget) const
	{
		return std::make_unique<SimpleLabelWidgetStyle>(labelWidget, Widgets::Instance()->GetTransparentMaterial());
	}
}