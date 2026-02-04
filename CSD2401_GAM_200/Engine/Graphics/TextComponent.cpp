/* Start Header ************************************************************************/
/*!
\file       TextComponent.cpp
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       November 07 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "TextComponent.h"
#include "GraphicsSystem.h"
#include "ResourceManager.h"
#include <iostream>

// FreeType global variables (FT_Library definition is assumed to be included via another header,
// like the one defining Character/FontResource, or indirectly via ResourceManager).
// Note: It is best practice to manage FreeType initialization in a central place like GraphicsSystem or ResourceManager.
static FT_Library ft = nullptr;
static int ftRetCount = 0;

TextComponent::TextComponent(GameObject* owner, const char* fontPath, GLuint fontSize, const std::string& text, vec4 color, bool screenSpace)
	:IComponent(owner), text(text), color(color), scale(1.f), fontPath(fontPath), fontSize(fontSize), screenSpace(screenSpace), fontResource(nullptr) {

	LoadFont(fontPath, fontSize);
}

TextComponent::~TextComponent() {
	// The FontResource is managed by the ResourceManager, so we only nullify the pointer.
	fontResource = nullptr;
}

void TextComponent::LoadFont(const char* fPath, GLuint fSize) {
	// create unique resource name based on font path and size
	fontResourceName = std::string(fPath) + "_" + std::to_string(fSize);

	// load or get existing font from resource manager
	// Assumes ResourceManager::Instance().LoadFont() is a specialized function for FontResource
	fontResource = ResourceManager::Instance().LoadFont(fontResourceName, fPath, fSize);

	if (!fontResource) {
		std::cerr << "ERROR::TextComponent: Failed to load font: " << fPath << std::endl;
	}
	else {
		std::cout << "TextComponent using font: " << fPath << " (size: " << fSize << ")" << std::endl;
	}
}

const std::map<char, Character>& TextComponent::GetCharacters() const {
	if (fontResource) {
		// Return the map of characters from the shared resource
		return fontResource->GetCharacters();
	}

	// return static empty map if font not loaded to prevent crashes
	static std::map<char, Character> emptyMap;
	return emptyMap;
}

void TextComponent::SetText(const std::string& newText) {
	text = newText;
}

std::unique_ptr<IComponent> TextComponent::Clone(GameObject* newOwner) const {
	// The constructor handles linking the font resource automatically.
	auto cloned = std::make_unique<TextComponent>(
		newOwner,
		fontPath.c_str(),
		fontSize,
		text,
		color,
		screenSpace
	);
	// Clone remaining state
	cloned->SetScale(scale);
	cloned->SetScreenSpace(screenSpace);
	cloned->SetEnabled(IsEnabled());
	return cloned;
}