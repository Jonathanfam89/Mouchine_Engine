/* Start Header ************************************************************************/
/*!
\file       TilemapEditor.cpp
\author     Khalil Belabadia
\par        khalil.b@digipen.edu
\date       November, 22, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef USE_IMGUI
#include "TilemapEditor.h"
#include "ResourceManager.h"
#include <fstream>
#include <iostream>
#include <queue>
#include <cstring>

bool isTileMapFocused;

/**
 * @brief Constructor - initialize tilemap editor
 */
TilemapEditor::TilemapEditor()
    : isVisible(true),
    tilemapWidth(20),
    tilemapHeight(15),
    selectedTileID(0),
    tilesetLoaded(false),
    flipTilesetVertical(false),  // Default OFF - normal orientation is correct
    currentLayer(0),
    maxLayers(3),
    currentTool(Tool::Brush),
    paintCollision(false),
    canvasOffset(100.0f, 100.0f),
    tileDisplaySize(32),
    showGrid(true),
    showCollisionOverlay(false),
    gridColor(0.3f, 0.3f, 0.3f, 0.5f),
    collisionColor(1.0f, 0.0f, 0.0f, 0.3f),
    isPanning(false),
    showSaveDialog(false),
    showLoadDialog(false),
    showTilesetLoadDialog(false),
    tempTileWidth(64),
    tempTileHeight(64),
    showNewTilemapDialog(false),
    tempMapWidth(20),
    tempMapHeight(15)
{
    // Initialize filepath buffers
    std::memset(saveFilepath, 0, sizeof(saveFilepath));
    std::memset(loadFilepath, 0, sizeof(loadFilepath));
    //std::memset(tilesetPath, 0, sizeof(tilesetPath));
    strcpy(tilesetPath, "<empty>");

    // Initialize layer names - 3 layers: Background (0), Main (1), Foreground (2)
    layerNames = { "Background", "Main", "Foreground" };
    layerVisibility = { true, true, true };
    maxLayers = 3;

    // Create initial tilemap
    CreateNewTilemap(tilemapWidth, tilemapHeight);

    // No auto-load - user must load tileset manually via File -> Load Tileset
    std::cout << "[TilemapEditor] Initialized. Use File -> Load Tileset to load a tileset." << std::endl;
}

/**
 * @brief Destructor
 */
TilemapEditor::~TilemapEditor()
{
    // Cleanup handled by ResourceManager
}

/**
 * @brief Main render function
 */
void TilemapEditor::Render()
{
    if (!isVisible)
        return;

    ImGui::SetNextWindowSize(ImVec2(1200, 800), ImGuiCond_FirstUseEver);

    bool windowOpen = isVisible;  // Track if window should be open
    isTileMapFocused = false;
    if (ImGui::Begin("Tilemap Editor", &windowOpen, ImGuiWindowFlags_MenuBar))
    {
        if (!ImGui::IsWindowFocused(ImGuiHoveredFlags_ChildWindows))
            isTileMapFocused = false;
        RenderMenuBar();

        // Main content area with two columns
        ImGui::Columns(2, "TilemapColumns", true);

        // Left column - Canvas (80% width)
        static bool init = true;
        if (init)
        {
            ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.75f);
            init = false;
        }

        RenderTilemapCanvas();

        ImGui::NextColumn();

        // Right column - Tileset palette and tools
        RenderToolsPanel();
        RenderTilesetPanel();
        RenderLayersPanel();
        RenderPropertiesPanel();

        ImGui::Columns(1);
        isTileMapFocused |= ImGui::IsWindowFocused(ImGuiHoveredFlags_ChildWindows);
    }
    ImGui::End();

    // Sync visibility state (handle window close button)
    isVisible = windowOpen;

    // Handle dialogs
    if (showSaveDialog)
    {
        ImGui::OpenPopup("Save Tilemap");
        showSaveDialog = false;
    }

    if (showLoadDialog)
    {
        ImGui::OpenPopup("Load Tilemap");
        showLoadDialog = false;
    }

    if (showTilesetLoadDialog)
    {
        ImGui::OpenPopup("Load Tileset");
        showTilesetLoadDialog = false;
    }

    if (showNewTilemapDialog)
    {
        ImGui::OpenPopup("New Tilemap");
        showNewTilemapDialog = false;
    }

    // Save dialog
    if (ImGui::BeginPopupModal("Save Tilemap", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Enter filename:");
        ImGui::InputText("##savefilepath", saveFilepath, sizeof(saveFilepath));

        if (ImGui::Button("Save", ImVec2(120, 0)))
        {
            std::string filepath = std::string("../Assets/Tiles/") + saveFilepath;
            if (!filepath.ends_with(".tilemap"))
                filepath += ".tilemap";

            if (SaveTilemap(filepath))
            {
                std::cout << "Tilemap saved: " << filepath << std::endl;
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // Load dialog
    if (ImGui::BeginPopupModal("Load Tilemap", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Enter filename:");
        ImGui::InputText("##loadfilepath", loadFilepath, sizeof(loadFilepath));

        if (ImGui::Button("Load", ImVec2(120, 0)))
        {
            std::string filepath = std::string("../Assets/Tiles/") + loadFilepath;
            if (!filepath.ends_with(".tilemap"))
                filepath += ".tilemap";

            if (LoadTilemap(filepath))
            {
                std::cout << "Tilemap loaded: " << filepath << std::endl;
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // New Tilemap dialog
    if (ImGui::BeginPopupModal("New Tilemap", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Tilemap Dimensions:");
        ImGui::Separator();

        ImGui::Text("Width (tiles):");
        ImGui::InputInt("##mapwidth", &tempMapWidth);
        if (tempMapWidth < 1) tempMapWidth = 1;
        if (tempMapWidth > 200) tempMapWidth = 200;

        ImGui::Text("Height (tiles):");
        ImGui::InputInt("##mapheight", &tempMapHeight);
        if (tempMapHeight < 1) tempMapHeight = 1;
        if (tempMapHeight > 200) tempMapHeight = 200;

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Warning: This will clear the current tilemap!");
        ImGui::Spacing();

        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            CreateNewTilemap(tempMapWidth, tempMapHeight);
            std::cout << "[TilemapEditor] Created new tilemap: " << tempMapWidth << "x" << tempMapHeight << std::endl;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

/**
 * @brief Update function
 */
void TilemapEditor::Update(float dt)
{
    // Update logic if needed
}

/**
 * @brief Render menu bar
 */
void TilemapEditor::RenderMenuBar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Tilemap"))
            {
                showNewTilemapDialog = true;
            }

            if (ImGui::MenuItem("Save Tilemap"))
            {
                showSaveDialog = true;
            }

            if (ImGui::MenuItem("Load Tilemap"))
            {
                showLoadDialog = true;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Load Tileset"))
            {
                showTilesetLoadDialog = true;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Clear Tilemap"))
            {
                ClearTilemap();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Show Grid", nullptr, &showGrid);
            ImGui::MenuItem("Show Collision Overlay", nullptr, &showCollisionOverlay);

            ImGui::Separator();

            if (ImGui::MenuItem("Reset View"))
            {
                canvasOffset = ImVec2(100.0f, 100.0f);
                tileDisplaySize = 32;  // Reset to default tile display size
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

/**
 * @brief Render tilemap canvas
 */
void TilemapEditor::RenderTilemapCanvas()
{
    // Enable horizontal and vertical scrollbars for large tilemaps
    ImGui::BeginChild("Canvas", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    // Calculate tile size and tilemap dimensions
    float scaledTileSize = static_cast<float>(tileDisplaySize);
    float totalWidth = tilemapWidth * scaledTileSize;
    float totalHeight = tilemapHeight * scaledTileSize;

    // Reserve space for the entire tilemap to enable scrolling
    ImGui::Dummy(ImVec2(totalWidth, totalHeight));

    // Draw tilemap background (gray rectangle exactly the size of the tilemap)
    drawList->AddRectFilled(
        ImVec2(canvasPos.x + canvasOffset.x, canvasPos.y + canvasOffset.y),
        ImVec2(canvasPos.x + canvasOffset.x + totalWidth, canvasPos.y + canvasOffset.y + totalHeight),
        IM_COL32(40, 40, 45, 255)
    );

    // Handle panning
    if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Middle))
    {
        if (!isPanning)
        {
            isPanning = true;
            lastMousePos = ImGui::GetMousePos();
        }

        ImVec2 currentMousePos = ImGui::GetMousePos();
        ImVec2 delta = ImVec2(currentMousePos.x - lastMousePos.x, currentMousePos.y - lastMousePos.y);
        canvasOffset.x += delta.x;
        canvasOffset.y += delta.y;
        lastMousePos = currentMousePos;
    }
    else
    {
        isPanning = false;
    }

    // Draw tiles - layer by layer from background to foreground for proper depth
    for (int layer = 0; layer < maxLayers; ++layer)
    {
        // Skip if layer is not visible
        if (!layerVisibility[layer])
            continue;

        for (int y = 0; y < tilemapHeight; ++y)
        {
            for (int x = 0; x < tilemapWidth; ++x)
            {
                const TileData& tile = tilemap[layer][y][x];

                // Skip if tile is empty
                if (tile.tileID < 0)
                    continue;

                vec2 screenPos = GridToScreen(x, y);
                ImVec2 tilePos(canvasPos.x + screenPos.x, canvasPos.y + screenPos.y);
                ImVec2 tileEnd(tilePos.x + scaledTileSize, tilePos.y + scaledTileSize);

                // Draw actual tile texture or colored placeholder
                if (tilesetLoaded && currentTileset.textureID > 0)
                {
                    ImVec2 uv0 = GetTileUV0(tile.tileID);
                    ImVec2 uv1 = GetTileUV1(tile.tileID);
                    drawList->AddImage((void*)(intptr_t)currentTileset.textureID,
                        tilePos, tileEnd, uv0, uv1);
                }
                else
                {
                    // Draw placeholder colored squares
                    ImU32 tileColor = IM_COL32(
                        100 + (tile.tileID * 37) % 155,
                        100 + (tile.tileID * 71) % 155,
                        100 + (tile.tileID * 113) % 155,
                        255
                    );
                    drawList->AddRectFilled(tilePos, tileEnd, tileColor);
                }

                // Draw collision overlay
                if (showCollisionOverlay && tile.isCollidable)
                {
                    ImU32 colColor = ImGui::ColorConvertFloat4ToU32(collisionColor);
                    drawList->AddRectFilled(tilePos, tileEnd, colColor);
                }
            }
        }
    }

    // Draw grid
    if (showGrid)
    {
        ImU32 gridColorU32 = ImGui::ColorConvertFloat4ToU32(gridColor);

        for (int y = 0; y <= tilemapHeight; ++y)
        {
            vec2 start = GridToScreen(0, y);
            vec2 end = GridToScreen(tilemapWidth, y);
            drawList->AddLine(
                ImVec2(canvasPos.x + start.x, canvasPos.y + start.y),
                ImVec2(canvasPos.x + end.x, canvasPos.y + end.y),
                gridColorU32
            );
        }

        for (int x = 0; x <= tilemapWidth; ++x)
        {
            vec2 start = GridToScreen(x, 0);
            vec2 end = GridToScreen(x, tilemapHeight);
            drawList->AddLine(
                ImVec2(canvasPos.x + start.x, canvasPos.y + start.y),
                ImVec2(canvasPos.x + end.x, canvasPos.y + end.y),
                gridColorU32
            );
        }
    }

    // Handle mouse input for painting
    if (ImGui::IsWindowHovered() && !isPanning)
    {
        ImVec2 mousePos = ImGui::GetMousePos();
        int gridX = ScreenToGridX(mousePos.x - canvasPos.x);
        int gridY = ScreenToGridY(mousePos.y - canvasPos.y);

        // Highlight hovered tile
        if (IsValidGridPos(gridX, gridY))
        {
            vec2 screenPos = GridToScreen(gridX, gridY);
            ImVec2 tilePos(canvasPos.x + screenPos.x, canvasPos.y + screenPos.y);
            ImVec2 tileEnd(tilePos.x + scaledTileSize, tilePos.y + scaledTileSize);
            drawList->AddRect(tilePos, tileEnd, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);

            // Paint/erase on left click
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                if (currentTool == Tool::Brush)
                {
                    PaintTile(gridX, gridY);
                }
                else if (currentTool == Tool::Eraser)
                {
                    EraseTile(gridX, gridY);
                }
            }

            // Eyedropper on left click - pick tile from canvas
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && currentTool == Tool::Eyedropper)
            {
                if (tilemap[currentLayer][gridY][gridX].tileID >= 0)
                {
                    selectedTileID = tilemap[currentLayer][gridY][gridX].tileID;
                    // Stay in eyedropper mode so you can pick multiple tiles
                }
            }

            // Flood fill on click
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && currentTool == Tool::Fill)
            {
                FloodFill(gridX, gridY, tilemap[currentLayer][gridY][gridX].tileID);
            }
        }
    }

    ImGui::EndChild();
}

/**
 * @brief Render tileset palette panel
 */
void TilemapEditor::RenderTilesetPanel()
{
    if (ImGui::CollapsingHeader("Tileset Palette", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (!tilesetLoaded)
        {
            ImGui::Text("Tile Size:");
            ImGui::InputInt("Width##tilewidth", &currentTileset.tileWidth);
            ImGui::InputInt("Height##tileheight", &currentTileset.tileHeight);
            currentTileset.tileWidth = tempTileWidth;
            currentTileset.tileHeight = tempTileHeight;

            std::string draggedTiles{};
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No tileset loaded");

            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Specify directory or Drag&Drop here");
            ImGui::InputText("##tilesetpath", tilesetPath, sizeof(tilesetPath));
            if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
            {
                draggedTiles = tilesetPath;
            }

            ReceiveDragAndDrop(TilesetPayloadID, draggedTiles);
            if (!draggedTiles.empty())
            {
                strcpy(tilesetPath, draggedTiles.c_str());
                std::cout << draggedTiles << std::endl;
                if (LoadTileset(draggedTiles))
                {
                    std::cout << "[TilemapEditor] Tileset loaded successfully!" << std::endl;
                }
                else
                {
                    std::cout << "[TilemapEditor] Failed to load tileset!" << std::endl;
                }
            }
            return;
        }

        // Show current tileset info
        ImGui::Text("Current Tileset:");
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s", currentTileset.name.c_str());
        ImGui::Text("Tile Size: %dx%d", currentTileset.tileWidth, currentTileset.tileHeight);
        ImGui::Text("Grid: %dx%d (%d tiles)", currentTileset.tilesPerRow, currentTileset.tilesPerColumn, currentTileset.totalTiles);

        // Check if there are any tiles painted
        bool hasTiles = false;
        for (int layer = 0; layer < maxLayers && !hasTiles; ++layer)
        {
            for (int y = 0; y < tilemapHeight && !hasTiles; ++y)
            {
                for (int x = 0; x < tilemapWidth && !hasTiles; ++x)
                {
                    if (tilemap[layer][y][x].tileID >= 0)
                    {
                        hasTiles = true;
                    }
                }
            }
        }

        // Button to load a different tileset
        if (ImGui::Button("Change Tileset", ImVec2(-1, 0)))
        {
            if (hasTiles)
            {
                std::cout << "[TilemapEditor] WARNING: Changing tileset will affect existing tiles!" << std::endl;
            }
            tilesetLoaded = false;
        }

        if (hasTiles)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Warning: Changing tileset");
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "will change tile graphics!");
        }

        ImGui::Separator();

        // Add flip option
        ImGui::Checkbox("Flip Vertical", &flipTilesetVertical);

        // Create a scrollable child window for the tileset palette
        ImGui::Text("Tileset:");
        ImGui::BeginChild("TilesetPalette", ImVec2(0, 400), true, ImGuiWindowFlags_HorizontalScrollbar);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 palettePos = ImGui::GetCursorScreenPos();

        int tileSize = 24;  // Display size for palette tiles (compact view)
        int padding = 2;    // Padding between tiles

        // Use the actual tileset grid dimensions (like Tiled does)
        int tilesPerRow = currentTileset.tilesPerRow;
        int tilesPerColumn = currentTileset.tilesPerColumn;

        // Calculate total size needed to display all tiles in their original grid
        float totalWidth = tilesPerRow * (tileSize + padding);
        float totalHeight = tilesPerColumn * (tileSize + padding);

        // Reserve space for the entire tileset grid (enables scrolling)
        ImGui::Dummy(ImVec2(totalWidth, totalHeight));

        // Draw tiles in their original grid layout
        for (int row = 0; row < tilesPerColumn; ++row)
        {
            for (int col = 0; col < tilesPerRow; ++col)
            {
                int tileID = row * tilesPerRow + col;

                // Skip if this tile index is beyond the total tiles
                if (tileID >= currentTileset.totalTiles)
                    break;

                ImVec2 tilePos(palettePos.x + col * (tileSize + padding), palettePos.y + row * (tileSize + padding));
                ImVec2 tileEnd(tilePos.x + tileSize, tilePos.y + tileSize);

                // Draw actual tile texture or colored placeholder
                // Draw actual tile from tileset (tileset is always loaded in this function)
                ImVec2 uv0 = GetTileUV0(tileID);
                ImVec2 uv1 = GetTileUV1(tileID);
                drawList->AddImage((void*)(intptr_t)currentTileset.textureID,
                    tilePos, tileEnd, uv0, uv1);

                // Highlight selected tile
                if (tileID == selectedTileID)
                {
                    drawList->AddRect(tilePos, tileEnd, IM_COL32(255, 255, 0, 255), 0.0f, 0, 3.0f);
                }

                // Handle click
                if (ImGui::IsMouseHoveringRect(tilePos, tileEnd))
                {
                    drawList->AddRect(tilePos, tileEnd, IM_COL32(255, 255, 255, 128), 0.0f, 0, 2.0f);

                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                    {
                        selectedTileID = tileID;
                    }
                }
            }
        }

        ImGui::EndChild();
    }
}

/**
 * @brief Render tools panel
 */
void TilemapEditor::RenderToolsPanel()
{
    if (ImGui::CollapsingHeader("Tools", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Tool:");

        if (ImGui::RadioButton("Brush", currentTool == Tool::Brush))
            currentTool = Tool::Brush;

        ImGui::SameLine();
        if (ImGui::RadioButton("Eraser", currentTool == Tool::Eraser))
            currentTool = Tool::Eraser;

        if (ImGui::RadioButton("Fill", currentTool == Tool::Fill))
            currentTool = Tool::Fill;

        ImGui::SameLine();
        if (ImGui::RadioButton("Eyedropper", currentTool == Tool::Eyedropper))
            currentTool = Tool::Eyedropper;

        ImGui::Separator();

        ImGui::Text("Tile Properties:");
        ImGui::Checkbox("Paint Collision", &paintCollision);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("When enabled, painted tiles will have collision");
        }
    }
}

/**
 * @brief Render properties panel
 */
void TilemapEditor::RenderPropertiesPanel()
{
    if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Tilemap Size:");

        // Make width editable
        int newWidth = tilemapWidth;
        if (ImGui::InputInt("Width (tiles)##mapwidth", &newWidth))
        {
            if (newWidth < 1) newWidth = 1;
            if (newWidth > 200) newWidth = 200;
            if (newWidth != tilemapWidth)
            {
                ResizeTilemap(newWidth, tilemapHeight);
            }
        }

        // Make height editable
        int newHeight = tilemapHeight;
        if (ImGui::InputInt("Height (tiles)##mapheight", &newHeight))
        {
            if (newHeight < 1) newHeight = 1;
            if (newHeight > 200) newHeight = 200;
            if (newHeight != tilemapHeight)
            {
                ResizeTilemap(tilemapWidth, newHeight);
            }
        }

        ImGui::Separator();

        ImGui::Text("Display Tile Size:");
        ImGui::SliderInt("##displaytilesize", &tileDisplaySize, 1, 64);

        ImGui::Separator();

        ImGui::Checkbox("Show Grid", &showGrid);
        ImGui::Checkbox("Show Collision", &showCollisionOverlay);
    }
}

/**
 * @brief Render layers panel
 */
void TilemapEditor::RenderLayersPanel()
{
    if (ImGui::CollapsingHeader("Layers", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Draw order: Bottom to Top");
        ImGui::Separator();

        // Draw layers in reverse order so they appear bottom-to-top visually
        // but still render in the correct order (0=background on bottom)
        for (int i = maxLayers - 1; i >= 0; --i)
        {
            ImGui::PushID(i);

            bool isCurrentLayer = (i == currentLayer);

            // Highlight the current layer
            if (isCurrentLayer)
            {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.5f, 0.8f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.6f, 0.9f, 0.7f));
            }

            // Layer name as selectable
            if (ImGui::Selectable(layerNames[i].c_str(), isCurrentLayer, 0, ImVec2(0, 20)))
            {
                currentLayer = i;
            }

            if (isCurrentLayer)
            {
                ImGui::PopStyleColor(2);
            }

            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);

            // Visibility checkbox (eye icon)
            bool visible = layerVisibility[i];
            const char* eyeIcon = visible ? "[O]" : "[X]";
            ImGui::PushStyleColor(ImGuiCol_Text, visible ? ImVec4(0.3f, 0.8f, 0.3f, 1.0f) : ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
            if (ImGui::SmallButton(eyeIcon))
            {
                layerVisibility[i] = !visible;
            }
            ImGui::PopStyleColor();

            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip(visible ? "Hide layer" : "Show layer");
            }

            ImGui::PopID();
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "Active: %s", layerNames[currentLayer].c_str());
    }
}

/**
 * @brief Create a new tilemap
 */
void TilemapEditor::CreateNewTilemap(int width, int height)
{
    // Update to new dimensions
    tilemapWidth = width;
    tilemapHeight = height;

    // Always create a fresh, empty tilemap
    tilemap.clear();
    tilemap.resize(maxLayers);
    for (int layer = 0; layer < maxLayers; ++layer)
    {
        tilemap[layer].resize(height);
        for (int y = 0; y < height; ++y)
        {
            tilemap[layer][y].resize(width);
            for (int x = 0; x < width; ++x)
            {
                tilemap[layer][y][x] = TileData();
            }
        }
    }

    // Center the canvas view on the tilemap
    // Start with offset that shows tilemap near top-left with some padding
    canvasOffset.x = 50.0f;
    canvasOffset.y = 50.0f;

    // Reset display size to default for consistent view
    tileDisplaySize = 32;

    std::cout << "[TilemapEditor] Created new tilemap: " << width << "x" << height << " with " << maxLayers << " layers" << std::endl;
}

/**
 * @brief Resize tilemap while preserving existing tiles
 */
void TilemapEditor::ResizeTilemap(int width, int height)
{
    // Store old dimensions
    int oldWidth = tilemapWidth;
    int oldHeight = tilemapHeight;

    // Update to new dimensions
    tilemapWidth = width;
    tilemapHeight = height;

    // Create new 3D structure with new size
    auto newTilemap = std::vector<std::vector<std::vector<TileData>>>(maxLayers);
    for (int layer = 0; layer < maxLayers; ++layer)
    {
        newTilemap[layer].resize(height);
        for (int y = 0; y < height; ++y)
        {
            newTilemap[layer][y].resize(width);
            for (int x = 0; x < width; ++x)
            {
                // Copy old tile data if it exists, otherwise use default
                if (x < oldWidth && y < oldHeight && !tilemap.empty())
                {
                    newTilemap[layer][y][x] = tilemap[layer][y][x];
                }
                else
                {
                    newTilemap[layer][y][x] = TileData();
                }
            }
        }
    }

    // Replace old tilemap with new one
    tilemap = std::move(newTilemap);

    std::cout << "[TilemapEditor] Resized tilemap: " << width << "x" << height
        << " (tiles preserved from " << oldWidth << "x" << oldHeight << ")" << std::endl;
}

/**
 * @brief Load tileset
 */
bool TilemapEditor::LoadTileset(const std::string& filepath)
{
    std::cout << "[TilemapEditor] Attempting to load: " << filepath << std::endl;
    // Check if we're changing tilesets and warn user
    if (tilesetLoaded && currentTileset.texturePath != filepath)
    {
        std::cout << "[TilemapEditor] WARNING: Changing tileset! Existing tiles will reference different tile indices." << std::endl;
        std::cout << "[TilemapEditor]          Consider saving your work first." << std::endl;

        // Clear the tilemap to prevent confusion
        ClearTilemap();
        std::cout << "[TilemapEditor]          Tilemap has been cleared to prevent tile ID mismatches." << std::endl;
    }


    // Use the filepath as the resource name to avoid caching conflicts
    // This allows switching between different tilesets
    TextureResource* tex = ResourceManager::Instance().LoadTexture(filepath, filepath);
    if (!tex || !tex->IsLoaded())
    {
        std::cerr << "[TilemapEditor] Failed to load tileset: " << filepath << std::endl;
        return false;
    }

    currentTileset.textureID = tex->GetTextureID();
    currentTileset.texturePath = filepath;
    currentTileset.name = filepath.substr(filepath.find_last_of("/\\") + 1);

    // Get actual texture dimensions from OpenGL
    int textureWidth = 0;
    int textureHeight = 0;

    glBindTexture(GL_TEXTURE_2D, currentTileset.textureID);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (textureWidth <= 0 || textureHeight <= 0)
    {
        std::cerr << "[TilemapEditor] Failed to get texture dimensions!" << std::endl;
        return false;
    }

    // Calculate tiles per row/column based on texture size and tile size
    currentTileset.tilesPerRow = textureWidth / currentTileset.tileWidth;
    currentTileset.tilesPerColumn = textureHeight / currentTileset.tileHeight;
    currentTileset.totalTiles = currentTileset.tilesPerRow * currentTileset.tilesPerColumn;

    tilesetLoaded = true;

    std::cout << "[TilemapEditor] SUCCESS: Tileset loaded!" << std::endl;
    std::cout << "[TilemapEditor]    Texture ID: " << currentTileset.textureID << std::endl;
    std::cout << "[TilemapEditor]    Texture Size: " << textureWidth << "x" << textureHeight << std::endl;
    std::cout << "[TilemapEditor]    Tile Size: " << currentTileset.tileWidth << "x" << currentTileset.tileHeight << std::endl;
    std::cout << "[TilemapEditor]    Tiles: " << currentTileset.tilesPerRow << "x"
        << currentTileset.tilesPerColumn << " = " << currentTileset.totalTiles << std::endl;
    std::cout << "[TilemapEditor]    tilesetLoaded flag: " << (tilesetLoaded ? "true" : "false") << std::endl;

    return true;
}

/**
 * @brief Save tilemap to JSON
 */
bool TilemapEditor::SaveTilemap(const std::string& filepath)
{
    json j;

    j["width"] = tilemapWidth;
    j["height"] = tilemapHeight;
    j["tileset"] = currentTileset.texturePath;
    j["tileWidth"] = currentTileset.tileWidth;
    j["tileHeight"] = currentTileset.tileHeight;
    j["numLayers"] = maxLayers;

    json tilesArray = json::array();
    for (int layer = 0; layer < maxLayers; ++layer)
    {
        for (int y = 0; y < tilemapHeight; ++y)
        {
            for (int x = 0; x < tilemapWidth; ++x)
            {
                const TileData& tile = tilemap[layer][y][x];
                if (tile.tileID >= 0)  // Only save non-empty tiles
                {
                    json tileJson;
                    tileJson["x"] = x;
                    tileJson["y"] = y;
                    tileJson["layer"] = layer;
                    tileJson["id"] = tile.tileID;
                    tileJson["collision"] = tile.isCollidable;
                    tilesArray.push_back(tileJson);
                }
            }
        }
    }

    j["tiles"] = tilesArray;

    std::ofstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "[TilemapEditor] Failed to save tilemap: " << filepath << std::endl;
        return false;
    }

    file << j.dump(2);
    file.close();

    return true;
}

/**
 * @brief Load tilemap from JSON
 */
bool TilemapEditor::LoadTilemap(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "[TilemapEditor] Failed to load tilemap: " << filepath << std::endl;
        return false;
    }

    json j;
    file >> j;
    file.close();

    tilemapWidth = j["width"];
    tilemapHeight = j["height"];

    // Load the tileset that was used for this tilemap
    std::string tilesetPath = j["tileset"];
    int tileWidth = j["tileWidth"];
    int tileHeight = j["tileHeight"];

    std::cout << "[TilemapEditor] Loading tilemap's tileset: " << tilesetPath << std::endl;

    // Set tile size before loading tileset
    currentTileset.tileWidth = tileWidth;
    currentTileset.tileHeight = tileHeight;

    // Load the tileset
    if (!LoadTileset(tilesetPath))
    {
        std::cerr << "[TilemapEditor] WARNING: Failed to load tileset. Tiles may appear incorrect!" << std::endl;
        std::cerr << "[TilemapEditor]          Expected tileset: " << tilesetPath << std::endl;
    }

    CreateNewTilemap(tilemapWidth, tilemapHeight);

    for (const auto& tileJson : j["tiles"])
    {
        int x = tileJson["x"];
        int y = tileJson["y"];
        int layer = tileJson["layer"];

        if (IsValidGridPos(x, y) && layer >= 0 && layer < maxLayers)
        {
            tilemap[layer][y][x].tileID = tileJson["id"];
            tilemap[layer][y][x].isCollidable = tileJson["collision"];
        }
    }

    std::cout << "[TilemapEditor] Tilemap loaded successfully!" << std::endl;

    return true;
}

/**
 * @brief Clear the tilemap
 */
void TilemapEditor::ClearTilemap()
{
    for (int layer = 0; layer < maxLayers; ++layer)
    {
        for (int y = 0; y < tilemapHeight; ++y)
        {
            for (int x = 0; x < tilemapWidth; ++x)
            {
                tilemap[layer][y][x] = TileData();
            }
        }
    }
}

/**
 * @brief Paint a tile at grid position
 */
void TilemapEditor::PaintTile(int gridX, int gridY)
{
    if (!IsValidGridPos(gridX, gridY))
        return;

    tilemap[currentLayer][gridY][gridX].tileID = selectedTileID;
    tilemap[currentLayer][gridY][gridX].isCollidable = paintCollision;
}

/**
 * @brief Erase a tile at grid position
 */
void TilemapEditor::EraseTile(int gridX, int gridY)
{
    if (!IsValidGridPos(gridX, gridY))
        return;

    tilemap[currentLayer][gridY][gridX].tileID = -1;
}

/**
 * @brief Flood fill algorithm
 */
void TilemapEditor::FloodFill(int gridX, int gridY, int targetTile)
{
    if (!IsValidGridPos(gridX, gridY))
        return;

    if (targetTile == selectedTileID)
        return;  // Nothing to fill

    std::queue<std::pair<int, int>> queue;
    queue.push({ gridX, gridY });

    while (!queue.empty())
    {
        auto [x, y] = queue.front();
        queue.pop();

        if (!IsValidGridPos(x, y))
            continue;

        if (tilemap[currentLayer][y][x].tileID != targetTile)
            continue;

        tilemap[currentLayer][y][x].tileID = selectedTileID;

        // Add neighbors
        queue.push({ x + 1, y });
        queue.push({ x - 1, y });
        queue.push({ x, y + 1 });
        queue.push({ x, y - 1 });
    }
}

/**
 * @brief Convert screen X to grid X
 */
int TilemapEditor::ScreenToGridX(float screenX) const
{
    float scaledTileSize = static_cast<float>(tileDisplaySize);
    return static_cast<int>((screenX - canvasOffset.x) / scaledTileSize);
}

/**
 * @brief Convert screen Y to grid Y
 */
int TilemapEditor::ScreenToGridY(float screenY) const
{
    float scaledTileSize = static_cast<float>(tileDisplaySize);
    return static_cast<int>((screenY - canvasOffset.y) / scaledTileSize);
}

/**
 * @brief Convert grid position to screen position
 */
vec2 TilemapEditor::GridToScreen(int gridX, int gridY) const
{
    float scaledTileSize = static_cast<float>(tileDisplaySize);
    return vec2(
        canvasOffset.x + gridX * scaledTileSize,
        canvasOffset.y + gridY * scaledTileSize
    );
}

/**
 * @brief Check if grid position is valid
 */
bool TilemapEditor::IsValidGridPos(int gridX, int gridY) const
{
    return gridX >= 0 && gridX < tilemapWidth && gridY >= 0 && gridY < tilemapHeight;
}

/**
 * @brief Get UV0 for a tile (top-left corner in texture space)
 */
ImVec2 TilemapEditor::GetTileUV0(int tileID) const
{
    int col = tileID % currentTileset.tilesPerRow;
    int row = tileID / currentTileset.tilesPerRow;

    // Flip row order when NOT enabled because OpenGL HAS TO BE LIKE THAT FOR SOME REASON????
    if (!flipTilesetVertical)
    {
        row = currentTileset.tilesPerColumn - 1 - row;
    }

    float u0 = static_cast<float>(col) / currentTileset.tilesPerRow;
    float v0 = static_cast<float>(row) / currentTileset.tilesPerColumn;
    float v1 = static_cast<float>(row + 1) / currentTileset.tilesPerColumn;

    // Always flip individual tiles to keep them right-side up when default (unchecked)
    if (!flipTilesetVertical)
    {
        return ImVec2(u0, v1);
    }

    return ImVec2(u0, v0);
}

/**
 * @brief Get UV1 for a tile (bottom-right corner in texture space)
 */
ImVec2 TilemapEditor::GetTileUV1(int tileID) const
{
    int col = tileID % currentTileset.tilesPerRow;
    int row = tileID / currentTileset.tilesPerRow;

    // Flip row order when NOT enabled (inverted logic - flip is the default correct behavior)
    if (!flipTilesetVertical)
    {
        row = currentTileset.tilesPerColumn - 1 - row;
    }

    float u1 = static_cast<float>(col + 1) / currentTileset.tilesPerRow;
    float v0 = static_cast<float>(row) / currentTileset.tilesPerColumn;
    float v1 = static_cast<float>(row + 1) / currentTileset.tilesPerColumn;

    // Always flip individual tiles to keep them right-side up when default (unchecked)
    if (!flipTilesetVertical)
    {
        return ImVec2(u1, v0);
    }

    return ImVec2(u1, v1);
}
#endif