/* Start Header ************************************************************************/
/*!
\file       ResourceManager.h
\author     Leck Xin Yee [100% - 130 lines]
\par        xinyee.leck@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <map>
#include <memory>
#include <iostream>
#include <vector>
#include "System.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <GL/glew.h>

class AudioEngine;
class GLSLShader;

namespace FMOD {
    class Sound;
}

struct Character {
    GLuint TextureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    GLuint advance;
};

/**
 * @class Resource
 * @brief Base class for all resource types (textures, audio, etc.)
 */
class Resource {
public:
    virtual ~Resource() = default;

    /**
     * @brief Loads a resource from a file
     * @param filepath Path to the resource file
     * @return True if loading was successful, false otherwise
     */
    virtual bool Load(const std::string& filepath) = 0;

    /**
     * @brief Unloads the resource and frees memory
     */
    virtual void Unload() = 0;

    /**
     * @brief Checks if the resource is currently loaded
     * @return True if loaded, false otherwise
     */
    virtual bool IsLoaded() const = 0;

    /**
     * @brief Gets the name identifier of the resource
     * @return Reference to the resource name string
     */
    const std::string& GetName() const { return name; }

    /**
     * @brief Gets the filepath of the resource
     * @return Reference to the filepath string
     */
    const std::string& GetFilePath() const { return filepath; }

    /**
     * @brief Sets the name identifier for the resource
     * @param resourceName Name to assign to the resource
     */
    void SetName(const std::string& resourceName) { name = resourceName; }

protected:
    std::string name;
    std::string filepath;
    bool loaded = false;
};

/**
 * @class TextureResource
 * @brief Manages texture resource loading and unloading
 */
class TextureResource : public Resource {
public:
    /**
     * @brief Constructs a TextureResource
     */
    TextureResource() : textureID(0) {}

    /**
     * @brief Destructs the TextureResource and unloads texture
     */
    ~TextureResource() override;

    /**
     * @brief Loads a texture from a file using OpenGL
     * @param filepath Path to the texture file
     * @return True if loading was successful, false otherwise
     */
    bool Load(const std::string& filepath) override;

    /**
     * @brief Unloads the texture and deletes OpenGL texture object
     */
    void Unload() override;

    /**
     * @brief Checks if the texture is currently loaded
     * @return True if loaded, false otherwise
     */
    bool IsLoaded() const override { return loaded; }

    /**
     * @brief Gets the OpenGL texture ID
     * @return The texture ID
     */
    unsigned int GetTextureID() const { return textureID; }

private:
    unsigned int textureID;
};
    
/**
 * @class AudioResource
 * @brief Manages audio resource loading and unloading through AudioEngine
 */
class AudioResource : public Resource {
public:
    /**
     * @brief Constructs an AudioResource
     */
    AudioResource() : audioEngine(nullptr), loop(false) {}

    /**
     * @brief Destructs the AudioResource and unloads audio
     */
    ~AudioResource() override;

    /**
     * @brief Loads an audio file through the AudioEngine
     * @param filepath Path to the audio file
     * @return True if loading was successful, false otherwise
     */
    bool Load(const std::string& filepath) override;

    /**
     * @brief Unloads the audio resource
     */
    void Unload() override;

    /**
     * @brief Checks if the audio is currently loaded
     * @return True if loaded, false otherwise
     */
    bool IsLoaded() const override { return loaded; }

    /**
     * @brief Sets whether the audio should loop
     * @param shouldLoop True to enable looping, false otherwise
     */
    void SetLoop(bool shouldLoop) { loop = shouldLoop; }

    /**
     * @brief Checks if the audio is set to loop
     * @return True if looping is enabled, false otherwise
     */
    bool IsLoop() const { return loop; }

    /**
     * @brief Sets the AudioEngine to use for loading
     * @param engine Pointer to the AudioEngine
     */
    void SetAudioEngine(AudioEngine* engine) { audioEngine = engine; }

private:
    AudioEngine* audioEngine;
    bool loop;
};

/**
 * @class FontResource
 * @brief Manages font resource loading and character glyph caching
 */
class FontResource : public Resource {
public:
    /**
     * @brief Constructor - initialise font resource
     */
    FontResource() : fontSize(0) {}

    /**
     * @brief Destructor - unload font resource
     */
    ~FontResource() override;

    /**
     * @brief Load font using FreeType
     */
    bool Load(const std::string& filepath) override;

    /**
     * @brief Load font with specified size
     */
    bool Load(const std::string& filepath, GLuint size);

    /**
     * @brief Unload font and glyph data
     */
    void Unload() override;

    /**
     * @brief Check if font is loaded
     */
    bool IsLoaded() const override { return loaded; }

    /**
     * @brief Get character glyph map
     */
    const std::map<char, Character>& GetCharacters() const { return characters; }

    /**
     * @brief Get font size
     */
    GLuint GetFontSize() const { return fontSize; }

    /**
     * @brief Set font size
     */
    void SetFontSize(GLuint size) { fontSize = size; }

private:
    std::map<char, Character> characters;
    GLuint fontSize;

    /**
     * @brief Load ASCII characters (0-127)
     */
    void LoadCharacters(FT_Face face);
};

/**
 * @class ShaderResource
 * @brief Manages shader resource loading and unloading
 */
class ShaderResource : public Resource {
public:
    ShaderResource();
    ~ShaderResource() override;

    bool Load(const std::string& filepath) override;
    bool LoadShaderPair(const std::string& vertexPath, const std::string& fragmentPath);
    bool LoadShaderFiles(const std::vector<std::pair<GLenum, std::string>>& shaderFiles);
    void Unload() override;
    bool IsLoaded() const override { return loaded; }

    GLSLShader* GetShader() { return shader.get(); }
    GLuint GetProgramHandle() const;

private:
    std::unique_ptr<GLSLShader> shader;
};


/**
 * @class ResourceManager
 * @brief Singleton manager for all game resources (textures, audio, etc.)
 */
class ResourceManager{
public:
    /**
     * @brief Gets the singleton instance of ResourceManager
     * @return Reference to the ResourceManager instance
     */
    static ResourceManager& Instance() {
        static ResourceManager instance;
        return instance;
    }

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    /**
     * @brief Loads a texture resource
     * @param name Identifier for the texture
     * @param filepath Path to the texture file
     * @return Pointer to the loaded TextureResource, or nullptr if failed
     */
    TextureResource* LoadTexture(const std::string& name, const std::string& filepath);

    /**
     * @brief Retrieves a loaded texture resource
     * @param name Identifier of the texture
     * @return Pointer to the TextureResource, or nullptr if not found
     */
    TextureResource* GetTexture(const std::string& name);

    /**
     * @brief Gets the filepath of a texture by its OpenGL texture ID
     * @param textureID The OpenGL texture ID
     * @return The filepath if found, empty string otherwise
     */
    std::string GetTextureFilePath(unsigned int textureID);

    /**
     * @brief Unloads a specific texture resource
     * @param name Identifier of the texture to unload
     */
    void UnloadTexture(const std::string& name);

    /**
     * @brief Loads an audio resource
     * @param name Identifier for the audio
     * @param filepath Path to the audio file
     * @param loop Whether the audio should loop
     * @return Pointer to the loaded AudioResource, or nullptr if failed
     */
    AudioResource* LoadAudio(const std::string& name, const std::string& filepath, bool loop = false);

    /**
     * @brief Retrieves a loaded audio resource
     * @param name Identifier of the audio
     * @return Pointer to the AudioResource, or nullptr if not found
     */
    AudioResource* GetAudio(const std::string& name);

    /**
     * @brief Unloads a specific audio resource
     * @param name Identifier of the audio to unload
     */
    void UnloadAudio(const std::string& name);

    /**
     * @brief Sets the AudioEngine reference for loading audio
     * @param engine Pointer to the AudioEngine
     */
    void SetAudioEngine(AudioEngine* engine);

    /**
     * @brief Gets the current AudioEngine pointer
     * @return Pointer to the AudioEngine
     */
    AudioEngine* GetAudioEngine() const { return audioEngine; }

    /**
     * @brief Loads a font resource
     * @param name Identifier for the font
     * @param filepath Path to the font file
     * @param fontSize Font size to load
     * @return Pointer to the loaded FontResource, or nullptr if failed
     */
    FontResource* LoadFont(const std::string& name, const std::string& filepath, GLuint fontSize);

    /**
     * @brief Retrieves a loaded font resource
     * @param name Identifier of the font
     * @return Pointer to the FontResource, or nullptr if not found
     */
    FontResource* GetFont(const std::string& name);

    /**
     * @brief Unloads a specific font resource
     * @param name Identifier of the font to unload
     */
    void UnloadFont(const std::string& name);

    ShaderResource* LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
    ShaderResource* LoadShaderFiles(const std::string& name, const std::vector<std::pair<GLenum, std::string>>& shaderFiles);
    ShaderResource* GetShader(const std::string& name);


    void UnloadShader(const std::string& name);

    /**
     * @brief Unloads all resources (textures and audio)
     */
    void UnloadAll();

    /**
     * @brief Unloads all texture resources
     */
    void UnloadAllTextures();

    /**
     * @brief Unloads all texture resources
     */
    void UnloadAllAudio();

    /**
     * @brief Unloads all font resources
     */
    void UnloadAllFonts();

    void UnloadAllShaders();

    /**
     * @brief Gets the count of loaded textures
     * @return Number of loaded textures
     */
    size_t GetTextureCount() const { return textures.size(); }

    /**
     * @brief Gets the count of loaded audio resources
     * @return Number of loaded audio resources
     */
    size_t GetAudioCount() const { return audioResources.size(); }

    /**
     * @brief Gets the count of loaded font resources
     * @return Number of loaded font resources
     */
    size_t GetFontCount() const { return fonts.size(); }

    size_t GetShaderCount() const { return shaders.size(); }

    /**
     * @brief Prints information about all loaded resources
     */
    void PrintLoadedResources() const;

private:
    ResourceManager();
    ~ResourceManager() {
        UnloadAll();
        std::cout << "Resource manager destroyed" << std::endl;
    }

    std::map<std::string, std::unique_ptr<TextureResource>> textures;
    std::map<std::string, std::unique_ptr<AudioResource>> audioResources;
    std::map<std::string, std::unique_ptr<FontResource>> fonts;
    std::map<std::string, std::unique_ptr<ShaderResource>> shaders;

    AudioEngine* audioEngine = nullptr;
    FT_Library ftLibrary = nullptr;
    int ftRefCount = 0;

    /**
     * @brief Initialises the FreeType library if not already initialised
     * @return True if initialisation succeeded, false otherwise
     */
    bool InitialiseFreeType();

    /**
     * @brief Shuts down the FreeType library if no fonts are using it
     */
    void ShutdownFreeType();

    /**
     * @brief Gets the FreeType library instance
     * @return Pointer to the FreeType library instance
     */
    FT_Library GetFreeTypeLibrary() const { return ftLibrary; }

    /**
     * @brief Template helper to retrieve resources from maps
     * @param name Identifier of the resource
     * @param resourceMap Map containing the resources
     * @return Pointer to the resource, or nullptr if not found
     */
    template<typename Resource>
    Resource* GetResource(const std::string& name, std::map<std::string, std::unique_ptr<Resource>>& resourceMap);

    friend class FontResource;  // allow font resource to access FreeType library
};
