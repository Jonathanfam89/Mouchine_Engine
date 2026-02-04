/* Start Header ************************************************************************/
/*!
\file       TilemapEditor.h
\author     Khalil Belabadia [100% - 112 lines]
\par        khalil.b@digipen.edu
\date       November, 22, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef USE_IMGUI
#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <GL/glew.h>
#include "MathLib.h"
#include "ImGuiSystem.h"
#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

/**
 * @struct TileData
 * @brief Represents a single tile in the tilemap
 */
struct TileData {
    int tileID;              // Tile index from tileset (-1 = empty)
    bool isCollidable;       // Does this tile have collision?

    TileData() : tileID(-1), isCollidable(false) {}
};

/**
 * @struct TilesetData
 * @brief Information about a loaded tileset
 */
struct TilesetData {
    std::string name;
    std::string texturePath;
    GLuint textureID;        // OpenGL texture ID
    int tileWidth;           // Width of each tile in pixels
    int tileHeight;          // Height of each tile in pixels
    int tilesPerRow;         // Number of tiles per row in the tileset
    int tilesPerColumn;      // Number of tiles per column
    int totalTiles;          // Total number of tiles

    TilesetData()
        : textureID(0), tileWidth(64), tileHeight(64),
        tilesPerRow(8), tilesPerColumn(8), totalTiles(64) {
    }
};

/**
 * @class TilemapEditor
 * @brief Standalone ImGui window for editing tilemaps
 */
class TilemapEditor {
public:
    TilemapEditor();
    ~TilemapEditor();

    /**
     * @brief Render the tilemap editor window
     */
    void Render();

    /**
     * @brief Update tilemap editor logic
     * @param dt Delta time
     */
    void Update(float dt);

    /**
     * @brief Show/hide the tilemap editor window
     */
    void SetVisible(bool visible) { isVisible = visible; }
    bool IsVisible() const { return isVisible; }

    /**
     * @brief Create a new tilemap
     * @param width Width in tiles
     * @param height Height in tiles
     */
    void CreateNewTilemap(int width, int height);
    /**
     * @brief Resize tilemap (preserves existing tiles)
     * @param width Width in tiles
     * @param height Height in tiles
     */
    void ResizeTilemap(int width, int height);


    /**
     * @brief Load a tileset image
     * @param filepath Path to tileset image
     * @return True if successful
     */
    bool LoadTileset(const std::string& filepath);

    /**
     * @brief Save current tilemap to file
     * @param filepath File to save to
     * @return True if successful
     */
    bool SaveTilemap(const std::string& filepath);

    /**
     * @brief Load tilemap from file
     * @param filepath File to load from
     * @return True if successful
     */
    bool LoadTilemap(const std::string& filepath);

    /**
     * @brief Get the tilemap data (3D structure: [layer][y][x])
     */
    const std::vector<std::vector<std::vector<TileData>>>& GetTilemapData() const { return tilemap; }

    /**
     * @brief Get tilemap dimensions
     */
    int GetTilemapWidth() const { return tilemapWidth; }
    int GetTilemapHeight() const { return tilemapHeight; }
    int GetNumLayers() const { return maxLayers; }

    /**
     * @brief Get tile at specific position and layer
     */
    const TileData* GetTile(int layer, int x, int y) const
    {
        if (layer >= 0 && layer < maxLayers && x >= 0 && x < tilemapWidth && y >= 0 && y < tilemapHeight)
            return &tilemap[layer][y][x];
        return nullptr;
    }

    /**
     * @brief Clear the entire tilemap
     */
    void ClearTilemap();

    /**
     * @brief Load a tilemap and create a SINGLE GameObject (not thousands)
     */
    static GameObject* LoadTilemapAsGameObject(
        const std::string& filepath,
        const vec2& tilemapPosition = vec2(0.0f, 0.0f),
        const std::string& name = "Tilemap");

    /**
     * @brief Render a tilemap GameObject
     */
    static void RenderTilemapGameObject(GameObject* tilemapObj);

private:
    // Rendering functions
    void RenderTilesetPanel();
    void RenderTilemapCanvas();
    void RenderToolsPanel();
    void RenderPropertiesPanel();
    void RenderLayersPanel();
    void RenderMenuBar();

    // Tool functions
    void PaintTile(int gridX, int gridY);
    void EraseTile(int gridX, int gridY);
    void FloodFill(int gridX, int gridY, int targetTile);

    // Helper functions
    int ScreenToGridX(float screenX) const;
    int ScreenToGridY(float screenY) const;
    vec2 GridToScreen(int gridX, int gridY) const;
    bool IsValidGridPos(int gridX, int gridY) const;

    // Tileset UV calculation
    ImVec2 GetTileUV0(int tileID) const;
    ImVec2 GetTileUV1(int tileID) const;

    // Helper functions for undo/redo
    void RecordTileEdit(int layer, int x, int y, const TileData& oldTile, const TileData& newTile);
    void PerformUndo();
    void PerformRedo();

private:
    bool isVisible;

    // Tilemap data
    std::vector<std::vector<std::vector<TileData>>> tilemap;  // [layer][y][x] grid
    int tilemapWidth;
    int tilemapHeight;

    // Tileset
    TilesetData currentTileset;
    int selectedTileID;
    bool tilesetLoaded;
    bool flipTilesetVertical;  // Option to flip tileset if loaded upside down

    // Layers
    int currentLayer;
    std::vector<bool> layerVisibility;
    std::vector<std::string> layerNames;
    int maxLayers;

    // Tools
    enum class Tool {
        Brush,
        Eraser,
        Fill,
        Eyedropper
    };
    Tool currentTool;
    bool paintCollision;  // If true, painted tiles will have collision

    // Canvas view
    ImVec2 canvasOffset;
    int tileDisplaySize;  // Size to display tiles at (controlled by slider in Properties)

    // Grid settings
    bool showGrid;
    bool showCollisionOverlay;
    ImVec4 gridColor;
    ImVec4 collisionColor;

    // UI state
    bool isPanning;
    ImVec2 lastMousePos;

    // File dialog
    char saveFilepath[256];
    char loadFilepath[256];
    bool showSaveDialog;
    bool showLoadDialog;

    // Tileset loading
    char tilesetPath[256] = "";
    bool showTilesetLoadDialog;

    // New tilemap dialog
    bool showNewTilemapDialog;
    int tempMapWidth;
    int tempMapHeight;

    // Layer highlighting
    bool highlightCurrentLayer;

    // Undo/Redo system
    struct TileEditAction
    {
        int layer;
        int x;
        int y;
        TileData oldTile;  // Tile state before the edit
        TileData newTile;  // Tile state after the edit
    };

    std::vector<TileEditAction> undoStack;
    std::vector<TileEditAction> redoStack;
    static constexpr int MAX_UNDO_STACK_SIZE = 100;
};

/**
 * @struct LoadedTilemapData
 * @brief Stores complete tilemap information for efficient rendering
 */
struct LoadedTilemapData {
    std::string filepath;
    int width;
    int height;
    int tileWidth;
    int tileHeight;
    int numLayers;

    std::string tilesetPath;
    GLuint tilesetTexture;
    int tilesPerRow;
    int tilesPerColumn;

    // All tile data: [layer][y][x]
    std::vector<std::vector<std::vector<TileData>>> tiles;

    LoadedTilemapData()
        : width(0), height(0), tileWidth(64), tileHeight(64),
        numLayers(1), tilesetTexture(0), tilesPerRow(0), tilesPerColumn(0) {
    }

    ~LoadedTilemapData()
    {
    }
};

/**
 * @class TilemapDataHolder
 * @brief Component that holds tilemap data (inherits from IComponent)
 */
class TilemapDataHolder : public IComponent
{
public:
    LoadedTilemapData* data;

    TilemapDataHolder(GameObject* owner)
        : IComponent(owner), data(nullptr) {
    }

    ~TilemapDataHolder()
    {
        if (data)
        {
            delete data;
            data = nullptr;
        }
    }

    // IComponent interface
    std::string GetName() const override { return "TilemapDataHolder"; }

    std::unique_ptr<IComponent> Clone(GameObject* newOwner) const override
    {
        auto clone = std::make_unique<TilemapDataHolder>(newOwner);
        if (data && !data->filepath.empty())
        {
            clone->LoadTilemapFromFile(data->filepath);
        }
        return clone;
    }

    // Tilemap specific
    std::string GetTilemapFilePath() const
    {
        return data ? data->filepath : "";
    }

    void LoadTilemapFromFile(const std::string& filepath);
};

extern bool isTileMapFocused;
#endif