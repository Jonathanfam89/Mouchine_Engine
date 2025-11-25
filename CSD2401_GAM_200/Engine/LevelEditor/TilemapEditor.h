/* Start Header ************************************************************************/
/*!
\file       TilemapEditor.h
\author     Khalil Belabadia
\par        khalil.b@digipen.edu
\date       November, 22, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

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
        : textureID(0), tileWidth(32), tileHeight(32),
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
    int tempTileWidth;
    int tempTileHeight;

    // New tilemap dialog
    bool showNewTilemapDialog;
    int tempMapWidth;
    int tempMapHeight;
};

extern bool isTileMapFocused;