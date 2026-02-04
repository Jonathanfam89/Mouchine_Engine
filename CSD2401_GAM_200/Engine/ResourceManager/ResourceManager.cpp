/* Start Header ************************************************************************/
/*!
\file       ResourceManager.cpp
\author     Leck Xin Yee [100% - 384 lines]
\par        xinyee.leck@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ResourceManager.h"
#include "GraphicsSystem.h"
#include "audio.h"
#include "glslshader.h"

// Texture Resource Implementation
TextureResource::~TextureResource()
{
    Unload();
}

/**
 * @brief Loads a texture from file using GraphicsSystem
 */
bool TextureResource::Load(const std::string &fpath)
{
    if (loaded)
    {
        std::cout << "Texture already loaded: " << fpath << std::endl;
        return true;
    }


    this->filepath = fpath;

    textureID = GraphicsSystem::LoadTexture(filepath.c_str());

    if (textureID == 0)
    {
        std::cerr << "failed to load texture: " << filepath << std::endl;
        return false;
    }

    loaded = true;
    std::cout << "Texture loaded successfully: " << filepath << " (id: " << textureID << ")" << std::endl;

    return true;
}

/**
 * @brief Unloads the texture and deletes OpenGL texture object
 */
void TextureResource::Unload()
{
    if (loaded && textureID != 0)
    {
        glDeleteTextures(1, &textureID);
        textureID = 0;
        loaded = false;
        std::cout << "Texture unloaded: " << filepath << std::endl;
    }
}

// Audio Resource Implementation
AudioResource::~AudioResource()
{
    Unload();
}

/**
 * @brief Loads an audio file through the AudioEngine
 */
bool AudioResource::Load(const std::string &fpath)
{
    if (loaded)
    {
        std::cout << "Audio already loaded: " << fpath << std::endl;
        return true;
    }

    if (!audioEngine)
    {
        std::cerr << "Cannot load audio: AudioEngine not set" << std::endl;
        return false;
    }

    this->filepath = fpath;

    // Use audio engine load sound
    bool success = audioEngine->loadSound(name, filepath, loop);

    if (!success)
    {
        std::cerr << "Failed to load audio through audio engine: " << filepath << std::endl;
        return false;
    }

    loaded = true;
    std::cout << "Audio loaded successfully through audio engine: " << filepath << std::endl;
    return true;
}

/**
 * @brief Unloads the audio resource
 */
void AudioResource::Unload()
{
    if (loaded && audioEngine)
    {
        // Use audio engine unload sound
        loaded = false;
        std::cout << "Audio unloaded: " << filepath << std::endl;
    }
}

// Font Resource Implementation
FontResource::~FontResource()
{
    Unload();
}

/**
 * @brief Loads a font from file using default size
 */
bool FontResource::Load(const std::string& fpath)
{
    return Load(fpath, 48);
}

/**
 * @brief Loads a font from file with specified size
 */
bool FontResource::Load(const std::string& fpath, GLuint size)
{
    if (loaded)
    {
        std::cout << "Font already loaded: " << fpath << std::endl;
        return true;
    }

    // FreeType is initialized
    if (!ResourceManager::Instance().InitialiseFreeType())
    {
        std::cerr << "Failed to initialize FreeType library" << std::endl;
        return false;
    }

    this->filepath = fpath;
    this->fontSize = size;

    FT_Face face;
    FT_Library ft = ResourceManager::Instance().GetFreeTypeLibrary();

    if (FT_New_Face(ft, filepath.c_str(), 0, &face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font: " << filepath << std::endl;
        ResourceManager::Instance().ShutdownFreeType();
        return false;
    }

    FT_Set_Pixel_Sizes(face, 0, fontSize);
    LoadCharacters(face);
    FT_Done_Face(face);

    loaded = true;
    std::cout << "Font loaded successfully: " << filepath << " (size: " << fontSize << ")" << std::endl;

    return true;
}

/**
 * @brief Loads all ASCII characters (0-127) from the font
 */
void FontResource::LoadCharacters(FT_Face face)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FREETYPE: Failed to load Glyph for character: " << (int)c << std::endl;
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0, GL_RED, GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)
        };
        characters.insert(std::pair<char, Character>(c, character));
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

/**
 * @brief Unloads the font and frees all character textures
 */
void FontResource::Unload()
{
    if (loaded)
    {
        for (auto& pair : characters)
        {
            glDeleteTextures(1, &pair.second.TextureID);
        }
        characters.clear();

        ResourceManager::Instance().ShutdownFreeType();

        loaded = false;
        std::cout << "Font unloaded: " << filepath << std::endl;
    }
}

// Shader Resource Implementation
ShaderResource::ShaderResource() : shader(std::make_unique<GLSLShader>())
{}

ShaderResource::~ShaderResource()
{
    Unload();
}

// loads a shader assuming vertex and fragment shader with .vert and .frag
bool ShaderResource::Load(const std::string& fpath)
{
    if (loaded)
    {
        std::cout << "Shader already loaded: " << fpath << std::endl;
        return true;
    }

    // Assume the filepath is the base path, and we'll append .vert and .frag
    std::string vertexPath = fpath + ".vert";
    std::string fragmentPath = fpath + ".frag";

    return LoadShaderPair(vertexPath, fragmentPath);
}

// loads a shader from vertex and fragment shader files with explicit paths
bool ShaderResource::LoadShaderPair(const std::string& vertexPath, const std::string& fragmentPath)
{
    if (loaded)
    {
        std::cout << "Shader already loaded" << std::endl;
        return true;
    }

    std::vector<std::pair<GLenum, std::string>> shaderFiles;
    shaderFiles.push_back(std::make_pair(GL_VERTEX_SHADER, vertexPath));
    shaderFiles.push_back(std::make_pair(GL_FRAGMENT_SHADER, fragmentPath));

    return LoadShaderFiles(shaderFiles);
}

// loads a shader from multiple shader files
bool ShaderResource::LoadShaderFiles(const std::vector<std::pair<GLenum, std::string>>& shaderFiles)
{
    if (loaded)
    {
        std::cout << "Shader already loaded" << std::endl;
        return true;
    }

    if (!shader)
    {
        std::cerr << "Shader object not initialized" << std::endl;
        return false;
    }

    if (!shader->CompileLinkValidate(shaderFiles))
    {
        std::cerr << "Failed to compile, link, and validate shader: " << shader->GetLog() << std::endl;
        return false;
    }

    // Store the first file path as the filepath
    if (!shaderFiles.empty())
    {
        this->filepath = shaderFiles[0].second;
    }

    loaded = true;
    std::cout << "Shader loaded successfully: " << name << " from " << filepath << std::endl;
    return true;
}

// unloads the shader and deletes shader program
void ShaderResource::Unload()
{
    if (loaded && shader)
    {
        shader->DeleteShaderProgram();
        loaded = false;
        std::cout << "Shader unloaded: " << filepath << std::endl;
    }
}

// gets the shader program handle
GLuint ShaderResource::GetProgramHandle() const
{
    if (shader)
    {
        return shader->GetHandle();
    }
    return 0;
}

// Resource Manager Implementation
ResourceManager::ResourceManager()
{}

/**
 * @brief Loads a texture resource or returns existing one if already loaded
 */
TextureResource *ResourceManager::LoadTexture(const std::string &name, const std::string &filepath)
{
    // Check if already loaded
    auto it = textures.find(name);
    if (it != textures.end())
    {
        std::cout << "Texture '" << name << "' already loaded, returning existing resource" << std::endl;
        return it->second.get();
    }

    // Create new texture resource
    auto texture = std::make_unique<TextureResource>();
    texture->SetName(name);

    if (!texture->Load(filepath))
    {
        std::cerr << "faailed to load texture: " << name << " from " << filepath << std::endl;
        return nullptr;
    }

    TextureResource *ptr = texture.get();
    textures[name] = std::move(texture);

    return ptr;
}

/**
 * @brief Retrieves a loaded texture resource by name
 */
TextureResource *ResourceManager::GetTexture(const std::string &name)
{
    return GetResource<TextureResource>(name, textures);
}

/**
 * @brief Gets the filepath of a texture by its OpenGL texture ID
 */
std::string ResourceManager::GetTextureFilePath(unsigned int textureID)
{
    for (const auto& pair : textures)
    {
        if (pair.second->GetTextureID() == textureID)
        {
            return pair.second->GetFilePath();
        }
    }
    return ""; // return empty string if not found
}

/**
 * @brief Unloads a specific texture resource from the manager
 */
void ResourceManager::UnloadTexture(const std::string &name)
{
    auto it = textures.find(name);
    if (it != textures.end())
    {
        textures.erase(it);
        std::cout << "Texture '" << name << "' unloaded from resource manager" << std::endl;
    }
}

/**
 * @brief Loads an audio resource or returns existing one if already loaded
 */
AudioResource *ResourceManager::LoadAudio(const std::string &name, const std::string &filepath, bool loop)
{
    auto it = audioResources.find(name);
    if (it != audioResources.end())
    {
        std::cout << "Audio '" << name << "' already loaded, returning existing resource" << std::endl;
        return it->second.get();
    }

    if (!audioEngine)
    {
        std::cerr << "Audio engine not set in resource manager" << std::endl;
        return nullptr;
    }

    auto audio = std::make_unique<AudioResource>();
    audio->SetName(name);
    audio->SetLoop(loop);
    audio->SetAudioEngine(audioEngine);

    if (!audio->Load(filepath))
    {
        std::cerr << "Failed to load audio: " << name << " from " << filepath << std::endl;
        return nullptr;
    }

    AudioResource *ptr = audio.get();
    audioResources[name] = std::move(audio);
    return ptr;
}

/**
 * @brief Retrieves a loaded audio resource by name
 */
AudioResource *ResourceManager::GetAudio(const std::string &name)
{
    return GetResource<AudioResource>(name, audioResources);
}

/**
 * @brief Unloads a specific audio resource from the manager
 */
void ResourceManager::UnloadAudio(const std::string &name)
{
    auto it = audioResources.find(name);
    if (it != audioResources.end())
    {
        audioResources.erase(it);
        std::cout << "Audio '" << name << "' unloaded from resource manager" << std::endl;
    }
}

/**
 * @brief Initializes the FreeType library if not already initialized
 */
bool ResourceManager::InitialiseFreeType()
{
    if (ftRefCount == 0)
    {
        if (FT_Init_FreeType(&ftLibrary))
        {
            std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return false;
        }
        std::cout << "FreeType library initialized" << std::endl;
    }
    ftRefCount++;
    return true;
}

/**
 * @brief Shuts down the FreeType library when no longer in use
 */
void ResourceManager::ShutdownFreeType()
{
    ftRefCount--;
    if (ftRefCount == 0 && ftLibrary)
    {
        FT_Done_FreeType(ftLibrary);
        ftLibrary = nullptr;
        std::cout << "FreeType library shut down" << std::endl;
    }
}

/**
 * @brief Loads a font resource or returns existing one if already loaded
 */
FontResource* ResourceManager::LoadFont(const std::string& name, const std::string& filepath, GLuint fontSize)
{
    // create unique key with font name and size
    std::string key = name + "_" + std::to_string(fontSize);

    auto it = fonts.find(key);
    if (it != fonts.end())
    {
        std::cout << "Font '" << name << "' (size: " << fontSize << ") already loaded, returning existing resource" << std::endl;
        return it->second.get();
    }

    auto font = std::make_unique<FontResource>();
    font->SetName(name);
    font->SetFontSize(fontSize);

    if (!font->Load(filepath, fontSize))
    {
        std::cerr << "Failed to load font: " << name << " from " << filepath << std::endl;
        return nullptr;
    }

    FontResource* ptr = font.get();
    fonts[key] = std::move(font);
    return ptr;
}

/**
 * @brief Retrieves a loaded font resource by name
 */
FontResource* ResourceManager::GetFont(const std::string& name)
{
    // try to find any font with this base name
    for (auto& pair : fonts)
    {
        if (pair.second->GetName() == name)
        {
            return pair.second.get();
        }
    }

    std::cerr << "Font not found: " << name << std::endl;
    return nullptr;
}

/**
 * @brief Unloads a specific font resource from the manager
 */
void ResourceManager::UnloadFont(const std::string& name)
{
    // unload all fonts with this base name
    auto it = fonts.begin();
    while (it != fonts.end())
    {
        if (it->second->GetName() == name)
        {
            it = fonts.erase(it);
            std::cout << "Font '" << name << "' unloaded from resource manager" << std::endl;
        }
        else
        {
            ++it;
        }
    }
}

// loads a shader resource or returns existing one if already loaded
ShaderResource* ResourceManager::LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath)
{
    auto it = shaders.find(name);
    if (it != shaders.end())
    {
        std::cout << "Shader '" << name << "' already loaded, returning existing resource" << std::endl;
        return it->second.get();
    }

    auto shader = std::make_unique<ShaderResource>();
    shader->SetName(name);

    if (!shader->LoadShaderPair(vertexPath, fragmentPath))
    {
        std::cerr << "Failed to load shader: " << name << " from " << vertexPath << " and " << fragmentPath << std::endl;
        return nullptr;
    }

    ShaderResource* ptr = shader.get();
    shaders[name] = std::move(shader);
    return ptr;
}

// loads a shader resource from multiple shader files
ShaderResource* ResourceManager::LoadShaderFiles(const std::string& name, const std::vector<std::pair<GLenum, std::string>>& shaderFiles)
{
    auto it = shaders.find(name);
    if (it != shaders.end())
    {
        std::cout << "Shader '" << name << "' already loaded, returning existing resource" << std::endl;
        return it->second.get();
    }

    auto shader = std::make_unique<ShaderResource>();
    shader->SetName(name);

    if (!shader->LoadShaderFiles(shaderFiles))
    {
        std::cerr << "Failed to load shader: " << name << std::endl;
        return nullptr;
    }

    ShaderResource* ptr = shader.get();
    shaders[name] = std::move(shader);
    return ptr;
}

// retrieves a loaded shader resource by name
ShaderResource* ResourceManager::GetShader(const std::string& name)
{
    return GetResource<ShaderResource>(name, shaders);
}

// unloads a specific shader resource from the manager
void ResourceManager::UnloadShader(const std::string& name)
{
    auto it = shaders.find(name);
    if (it != shaders.end())
    {
        shaders.erase(it);
        std::cout << "Shader '" << name << "' unloaded from resource manager" << std::endl;
    }
}

/**
 * @brief Unloads all textures and audio resources
 */
void ResourceManager::UnloadAll()
{
    std::cout << "Unloading all resources..." << std::endl;
    UnloadAllTextures();
    UnloadAllAudio();
    UnloadAllFonts();
    UnloadAllShaders();
}

/**
 * @brief Unloads all texture resources and clears the texture map
 */
void ResourceManager::UnloadAllTextures()
{
    std::cout << "Unloading " << textures.size() << " textures" << std::endl;
    textures.clear();
}

/**
 * @brief Unloads all audio resources and clears the audio map
 */
void ResourceManager::UnloadAllAudio()
{
    std::cout << "Unloading " << audioResources.size() << " audio resources" << std::endl;
    audioResources.clear();
}

/**
 * @brief Unloads all font resources and clears the font map
 */
void ResourceManager::UnloadAllFonts()
{
    std::cout << "Unloading " << fonts.size() << " fonts" << std::endl;
    fonts.clear();
}

// unloads all shader resources and clears the shader map
void ResourceManager::UnloadAllShaders()
{
    std::cout << "Unloading " << shaders.size() << " shaders" << std::endl;
    shaders.clear();
}

/**
 * @brief Prints detailed information about all loaded resources
 */
void ResourceManager::PrintLoadedResources() const
{
    std::cout << "\n=== Loaded Resources ===" << std::endl;

    std::cout << "Textures (" << textures.size() << "):" << std::endl;
    for (const auto &pair : textures)
    {
        std::cout << "  - " << pair.first
                  << " (ID: " << pair.second->GetTextureID() << ")"
                  << " -> " << pair.second->GetFilePath() << std::endl;
    }

    std::cout << "Audio (" << audioResources.size() << "):" << std::endl;
    for (const auto &pair : audioResources)
    {
        std::cout << "  - " << pair.first << " -> " << pair.second->GetFilePath() << std::endl;
    }

    std::cout << "Fonts (" << fonts.size() << "):" << std::endl;
    for (const auto& pair : fonts)
    {
        std::cout << "  - " << pair.second->GetName()
            << " (Size: " << pair.second->GetFontSize() << ")"
            << " -> " << pair.second->GetFilePath() << std::endl;
    }

    std::cout << "Shaders (" << shaders.size() << "):" << std::endl;
    for (const auto& pair : shaders)
    {
        std::cout << "  - " << pair.first
            << " (Handle: " << pair.second->GetProgramHandle() << ")"
            << " -> " << pair.second->GetFilePath() << std::endl;
    }
}

/**
 * @brief Sets the AudioEngine reference for loading audio resources
 */
void ResourceManager::SetAudioEngine(AudioEngine *engine)
{
    audioEngine = engine;
    if (audioEngine)
    {
        std::cout << "Audio engine set in resource manager" << std::endl;
    }
}

// Template Helper
/**
 * @brief Template helper to retrieve resources from resource maps
 */
template <typename Resource>
Resource *ResourceManager::GetResource(const std::string &name, std::map<std::string, std::unique_ptr<Resource>> &resourceMap)
{
    auto it = resourceMap.find(name);
    if (it != resourceMap.end())
    {
        return it->second.get();
    }

    std::cerr << "Resource not found: " << name << std::endl;
    return nullptr;
}
