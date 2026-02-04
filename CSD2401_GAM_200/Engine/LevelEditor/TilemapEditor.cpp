/* Start Header ************************************************************************/
/*!
\file       TilemapEditor.cpp
\author     Khalil Belabadia [100% - 968 lines]
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
#include "factory.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "transform.h"
#include "GraphicsSystem.h"

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
    showNewTilemapDialog(false),
    tempMapWidth(500),
    tempMapHeight(500),
    highlightCurrentLayer(true)
{
    // Initialize filepath buffers
    std::memset(saveFilepath, 0, sizeof(saveFilepath));
    std::memset(loadFilepath, 0, sizeof(loadFilepath));
    //std::memset(tilesetPath, 0, sizeof(tilesetPath));
    strcpy(tilesetPath, "<empty>");

    // Initialize layer names, 3 layers: Background, Main, Foreground
    layerNames = { "Background", "Main", "Foreground" };
    layerVisibility = { true, true, true };
    maxLayers = 3;

    // Create initial tilemap
    CreateNewTilemap(tilemapWidth, tilemapHeight);

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

        // Left column - Canvas
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

    // Sync visibility state
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
        if (tempMapWidth > 2000) tempMapWidth = 2000;

        ImGui::Text("Height (tiles):");
        ImGui::InputInt("##mapheight", &tempMapHeight);
        if (tempMapHeight < 1) tempMapHeight = 1;
        if (tempMapHeight > 2000) tempMapHeight = 2000;

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
    (void)dt;
    if (!isVisible)
        return;

    // Handle undo/redo keyboard shortcuts
    ImGuiIO& io = ImGui::GetIO();

    // Ctrl+Z for Undo
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z))
    {
        PerformUndo();
    }

    // Ctrl+Y for Redo
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y))
    {
        PerformRedo();
    }
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

            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, !undoStack.empty()))
            {
                PerformUndo();
            }

            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, !redoStack.empty()))
            {
                PerformRedo();
            }

            ImGui::Separator();

            ImGui::Text("Undo Stack: %zu", undoStack.size());
            ImGui::Text("Redo Stack: %zu", redoStack.size());

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

    // Draw tilemap background
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

    // Draw tiles
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

                // HIGHLIGHT CURRENT LAYER TILES
                if (highlightCurrentLayer && layer == currentLayer)
                {
                    // Draw a colored outline around tiles on the current layer
                    ImU32 highlightColor = IM_COL32(255, 255, 0, 150); // Yellow with transparency
                    float thickness = 2.0f;

                    drawList->AddRect(tilePos, tileEnd, highlightColor, 0.0f, 0, thickness);
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

            // Eyedropper on left click
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && currentTool == Tool::Eyedropper)
            {
                if (tilemap[currentLayer][gridY][gridX].tileID >= 0)
                {
                    selectedTileID = tilemap[currentLayer][gridY][gridX].tileID;
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

            std::string draggedTiles{};
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No tileset loaded");

            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Specify directory or Drag&Drop here");
            ImGui::InputText("##tilesetpath", tilesetPath, sizeof(tilesetPath));
            if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
            {
                draggedTiles = tilesetPath;
            }

            ReceiveDragAndDrop(SpritePayloadID, draggedTiles);
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

        int tileSize = 24;  // Display size for palette tiles
        int padding = 2;    // Padding between tiles

        // Use the actual tileset grid dimensions (like Tiled does)
        int tilesPerRow = currentTileset.tilesPerRow;
        int tilesPerColumn = currentTileset.tilesPerColumn;

        // Calculate total size needed to display all tiles in their original grid
        float totalWidth = static_cast<float>(tilesPerRow) * (tileSize + padding);
        float totalHeight = static_cast<float>(tilesPerColumn) * (tileSize + padding);

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
                // Draw actual tile from tileset
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

        // [WIP] make key binds only work when hovering over tilemap 
        if (ImGui::RadioButton("Brush", currentTool == Tool::Brush) || (ImGui::IsKeyPressed(ImGuiKey_B)))
            currentTool = Tool::Brush;
        CreateHintText("(B) Draw Tool");

        ImGui::SameLine();
        if (ImGui::RadioButton("Eraser", currentTool == Tool::Eraser) || (ImGui::IsKeyPressed(ImGuiKey_D)))
            currentTool = Tool::Eraser;
        CreateHintText("(D) Remove Tool");

        if (ImGui::RadioButton("Fill", currentTool == Tool::Fill) || (ImGui::IsKeyPressed(ImGuiKey_F)))
            currentTool = Tool::Fill;
        CreateHintText("(F) Fill Space Tool");

        ImGui::SameLine();
        if (ImGui::RadioButton("Eyedropper", currentTool == Tool::Eyedropper) || (ImGui::IsKeyPressed(ImGuiKey_G)))
            currentTool = Tool::Eyedropper;
        CreateHintText("(G) Color Picker Tool");

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
            if (newWidth > 2000) newWidth = 2000;
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
            if (newHeight > 2000) newHeight = 2000;
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

        ImGui::Checkbox("Highlight Current Layer", &highlightCurrentLayer);
        ImGui::Separator();

        // Draw layers in reverse order so they appear bottom-to-top visually
        // but still render in the correct order
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

            // Visibility checkbox
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
    std::string tsPath = j["tileset"];
    //std::string tilesetPath = j["tileset"];
    int tileWidth = j["tileWidth"];
    int tileHeight = j["tileHeight"];

    std::cout << "[TilemapEditor] Loading tilemap's tileset: " << tsPath << std::endl;

    // Set tile size before loading tileset
    currentTileset.tileWidth = tileWidth;
    currentTileset.tileHeight = tileHeight;

    // Load the tileset
    if (!LoadTileset(tsPath))
    {
        std::cerr << "[TilemapEditor] WARNING: Failed to load tileset. Tiles may appear incorrect!" << std::endl;
        std::cerr << "[TilemapEditor]          Expected tileset: " << tsPath << std::endl;
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

    // RECORD OLD STATE
    TileData oldTile = tilemap[currentLayer][gridY][gridX];

    // Apply new tile
    tilemap[currentLayer][gridY][gridX].tileID = selectedTileID;
    tilemap[currentLayer][gridY][gridX].isCollidable = paintCollision;

    // RECORD NEW STATE
    TileData newTile = tilemap[currentLayer][gridY][gridX];

    // RECORD FOR UNDO
    RecordTileEdit(currentLayer, gridX, gridY, oldTile, newTile);
}

/**
 * @brief Erase a tile at grid position
 */
void TilemapEditor::EraseTile(int gridX, int gridY)
{
    if (!IsValidGridPos(gridX, gridY))
        return;

    // RECORD OLD STATE
    TileData oldTile = tilemap[currentLayer][gridY][gridX];

    // Erase tile
    tilemap[currentLayer][gridY][gridX].tileID = -1;
    tilemap[currentLayer][gridY][gridX].isCollidable = false;

    // RECORD NEW STATE
    TileData newTile = tilemap[currentLayer][gridY][gridX];

    // RECORD FOR UNDO
    RecordTileEdit(currentLayer, gridX, gridY, oldTile, newTile);
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
 * @brief Get UV0 for a tile
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

    // Always flip individual tiles to keep them right-side up when default
    if (!flipTilesetVertical)
    {
        return ImVec2(u0, v1);
    }

    return ImVec2(u0, v0);
}

/**
 * @brief Get UV1 for a tile
 */
ImVec2 TilemapEditor::GetTileUV1(int tileID) const
{
    int col = tileID % currentTileset.tilesPerRow;
    int row = tileID / currentTileset.tilesPerRow;

    // Flip row order when NOT enabled
    if (!flipTilesetVertical)
    {
        row = currentTileset.tilesPerColumn - 1 - row;
    }

    float u1 = static_cast<float>(col + 1) / currentTileset.tilesPerRow;
    float v0 = static_cast<float>(row) / currentTileset.tilesPerColumn;
    float v1 = static_cast<float>(row + 1) / currentTileset.tilesPerColumn;

    // Always flip individual tiles to keep them right-side up when default
    if (!flipTilesetVertical)
    {
        return ImVec2(u1, v0);
    }

    return ImVec2(u1, v1);
}

/**
 * @brief Load tilemap data into TilemapDataHolder component
 */
void TilemapDataHolder::LoadTilemapFromFile(const std::string& filepath)
{
    try
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "[TilemapDataHolder] Failed to open: " << filepath << std::endl;
            return;
        }

        json j;
        file >> j;
        file.close();

        if (!j.contains("width") || !j.contains("height") ||
            !j.contains("tileset") || !j.contains("tiles"))
        {
            std::cerr << "[TilemapDataHolder] Invalid tilemap format!" << std::endl;
            return;
        }

        // Create data structure
        data = new LoadedTilemapData();
        data->filepath = filepath;
        data->width = j["width"];
        data->height = j["height"];
        data->tileWidth = j["tileWidth"];
        data->tileHeight = j["tileHeight"];
        data->numLayers = j.contains("numLayers") ? (int)j["numLayers"] : 1;
        data->tilesetPath = j["tileset"];

        // Load tileset texture
        data->tilesetTexture = GraphicsSystem::LoadTexture(data->tilesetPath.c_str());
        if (data->tilesetTexture == 0)
        {
            std::cerr << "[TilemapDataHolder] Failed to load tileset: " << data->tilesetPath << std::endl;
            delete data;
            data = nullptr;
            return;
        }

        // Get tileset dimensions
        int texWidth, texHeight;
        glBindTexture(GL_TEXTURE_2D, data->tilesetTexture);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);

        data->tilesPerRow = texWidth / data->tileWidth;
        data->tilesPerColumn = texHeight / data->tileHeight;

        // Initialize tile grid
        data->tiles.resize(data->numLayers);
        for (int layer = 0; layer < data->numLayers; ++layer)
        {
            data->tiles[layer].resize(data->height);
            for (int y = 0; y < data->height; ++y)
            {
                data->tiles[layer][y].resize(data->width);
            }
        }

        // Load tile data
        for (const auto& tileJson : j["tiles"])
        {
            int x = tileJson["x"];
            int y = tileJson["y"];
            int layer = tileJson.contains("layer") ? (int)tileJson["layer"] : 0;
            int tileID = tileJson["id"];
            bool collision = tileJson.contains("collision") ? (bool)tileJson["collision"] : false;

            if (x >= 0 && x < data->width &&
                y >= 0 && y < data->height &&
                layer >= 0 && layer < data->numLayers &&
                tileID >= 0)
            {
                data->tiles[layer][y][x].tileID = tileID;
                data->tiles[layer][y][x].isCollidable = collision;
            }
        }

        std::cout << "[TilemapDataHolder] Loaded: " << data->width << "x"
            << data->height << " (" << data->numLayers << " layers)" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[TilemapDataHolder] Error: " << e.what() << std::endl;
        if (data)
        {
            delete data;
            data = nullptr;
        }
    }
}

/**
 * @brief Load tilemap and create ONE GameObject with TilemapDataHolder
 */
GameObject* TilemapEditor::LoadTilemapAsGameObject(
    const std::string& filepath,
    const vec2& tilemapPosition,
    const std::string& name)
{
    try
    {
        std::cout << "[TilemapEditor] Loading tilemap: " << filepath << std::endl;

        // Create ONE GameObject
        auto& factory = GameObjectFactory::Instance();
        GameObject* tilemapObj = factory.CreateGameObject(name);

        if (!tilemapObj)
        {
            std::cerr << "[TilemapEditor] Failed to create GameObject!" << std::endl;
            return nullptr;
        }

        // Add transform
        auto* transform = tilemapObj->GetComponent<TransformComponent>();
        if (!transform)
        {
            transform = tilemapObj->AddComponent<TransformComponent>(
                tilemapPosition.x, tilemapPosition.y);
        }
        else
        {
            transform->SetPosition(tilemapPosition.x, tilemapPosition.y);
        }

        // Add TilemapDataHolder component
        auto* holder = tilemapObj->AddComponent<TilemapDataHolder>();
        if (!holder)
        {
            std::cerr << "[TilemapEditor] Failed to add TilemapDataHolder!" << std::endl;
            factory.DestroyGameObject(tilemapObj->GetID());
            return nullptr;
        }

        // Load the tilemap data
        holder->LoadTilemapFromFile(filepath);

        if (!holder->data)
        {
            std::cerr << "[TilemapEditor] Failed to load tilemap data!" << std::endl;
            factory.DestroyGameObject(tilemapObj->GetID());
            return nullptr;
        }

        std::cout << "[TilemapEditor] Created tilemap GameObject: "
            << holder->data->width << "x" << holder->data->height << std::endl;

        return tilemapObj;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[TilemapEditor] Error: " << e.what() << std::endl;
        return nullptr;
    }
}

/**
 * @brief Render a tilemap GameObject by adding tiles to the batch system
 */
void TilemapEditor::RenderTilemapGameObject(GameObject* tilemapObj)
{
    if (!tilemapObj) return;

    auto* holder = tilemapObj->GetComponent<TilemapDataHolder>();
    if (!holder || !holder->data) return;

    LoadedTilemapData* data = holder->data;

    auto* transform = tilemapObj->GetComponent<TransformComponent>();
    if (!transform) return;

    vec2 position = vec2(transform->posX, transform->posY);
    vec2 scale = vec2(transform->scaleX, transform->scaleY);
    float rotation = transform->GetRotation();

    // Get GraphicsSystem
    auto* gs = CORE->GetGraphicsSystem();
    if (!gs) return;

    // Access batches
    auto& batches = gs->batches;

    // Render each layer
    for (int layer = 0; layer < data->numLayers; ++layer)
    {
        int renderLayer = -100 + (layer * 25);

        // Find existing batch index
        int batchIndex = -1;
        for (size_t i = 0; i < batches.size(); ++i)
        {
            if (batches[i].textureID == data->tilesetTexture &&
                !batches[i].isCircle &&
                batches[i].layer == renderLayer)
            {
                batchIndex = static_cast<int>(i);
                break;
            }
        }

        // Create new batch if needed
        if (batchIndex == -1)
        {
            batches.emplace_back(data->tilesetTexture, false, renderLayer);
            batchIndex = static_cast<int>(batches.size() - 1);
        }

        // NOW get the pointer (after all potential reallocations)
       RenderBatch* targetBatch = &batches[batchIndex];

        // Add all tiles in this layer
        for (int y = 0; y < data->height; ++y)
        {
            for (int x = 0; x < data->width; ++x)
            {
                const TileData& tile = data->tiles[layer][y][x];

                if (tile.tileID < 0) continue;

                // Calculate world position
                float tileX = position.x + (x * data->tileWidth * scale.x);
                float tileY = position.y + ((data->height - 1 - y) * data->tileHeight * scale.y);

                float tileSX = static_cast<float>(data->tileWidth) * scale.x;
                float tileSY = static_cast<float>(data->tileHeight) * scale.y;

                // Calculate UV
                int col = tile.tileID % data->tilesPerRow;
                int row = tile.tileID / data->tilesPerRow;

                float uv_width = 1.0f / data->tilesPerRow;
                float uv_height = 1.0f / data->tilesPerColumn;

                row = (data->tilesPerColumn - 1) - row;

                float uv_x = uv_width * col;
                float uv_y = uv_height * row;

                // Add 4 vertices for this tile
                GLuint startIndex = static_cast<GLuint>(targetBatch->vertices.size());

                vec3 localPos[4] = {
                    {-0.5f, -0.5f, 0.0f}, {0.5f, -0.5f, 0.0f},
                    {0.5f, 0.5f, 0.0f}, {-0.5f, 0.5f, 0.0f}
                };

                vec2 texCoords[4] = {
                    {0.0f, 0.0f}, {1.0f, 0.0f},
                    {1.0f, 1.0f}, {0.0f, 1.0f}
                };

                for (int i = 0; i < 4; ++i)
                {
                    
                    BatchVertex vertex;
                    vertex.pos = localPos[i];
                    vertex.tex = texCoords[i];
                    vertex.color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
                    vertex.uvScale = vec2(uv_width, uv_height);
                    vertex.uvOffset = vec2(uv_x, uv_y);
                    vertex.transform = vec3(tileX, tileY, rotation);
                    vertex.scale = vec2(tileSX, tileSY);

                    targetBatch->vertices.push_back(vertex);
                }

                // Add 6 indices (2 triangles)
                targetBatch->indices.push_back(startIndex + 0);
                targetBatch->indices.push_back(startIndex + 1);
                targetBatch->indices.push_back(startIndex + 2);

                targetBatch->indices.push_back(startIndex + 2);
                targetBatch->indices.push_back(startIndex + 3);
                targetBatch->indices.push_back(startIndex + 0);
            }
        }
    }
}

/**
 * @brief Record a tile edit for undo/redo
 */
void TilemapEditor::RecordTileEdit(int layer, int x, int y, const TileData& oldTile, const TileData& newTile)
{
    // Don't record if nothing changed
    if (oldTile.tileID == newTile.tileID && oldTile.isCollidable == newTile.isCollidable)
        return;

    TileEditAction action;
    action.layer = layer;
    action.x = x;
    action.y = y;
    action.oldTile = oldTile;
    action.newTile = newTile;

    undoStack.push_back(action);

    // Limit stack size
    if (undoStack.size() > MAX_UNDO_STACK_SIZE)
    {
        undoStack.erase(undoStack.begin());
    }

    // Clear redo stack when new edit is made
    redoStack.clear();
}

/**
 * @brief Undo the last tile edit
 */
void TilemapEditor::PerformUndo()
{
    if (undoStack.empty())
    {
        std::cout << "[TilemapEditor] Nothing to undo" << std::endl;
        return;
    }
    
    TileEditAction action = undoStack.back();
    undoStack.pop_back();
    
    // Restore old tile state
    if (action.layer >= 0 && action.layer < maxLayers &&
        action.x >= 0 && action.x < tilemapWidth &&
        action.y >= 0 && action.y < tilemapHeight)
        {
            tilemap[action.layer][action.y][action.x] = action.oldTile;
        }
        
        // Add to redo stack
        redoStack.push_back(action);
        
        std::cout << "[TilemapEditor] Undo performed" << std::endl;
    }
    
    /**
     * @brief Redo the last undone tile edit
     */
    void TilemapEditor::PerformRedo()
    {
        if (redoStack.empty())
        {
            std::cout << "[TilemapEditor] Nothing to redo" << std::endl;
            return;
        }
        
        TileEditAction action = redoStack.back();
        redoStack.pop_back();
        
        // Restore new tile state
        if (action.layer >= 0 && action.layer < maxLayers &&
            action.x >= 0 && action.x < tilemapWidth &&
            action.y >= 0 && action.y < tilemapHeight)
            {
                tilemap[action.layer][action.y][action.x] = action.newTile;
            }
            
            // Add back to undo stack
            undoStack.push_back(action);
            
            std::cout << "[TilemapEditor] Redo performed" << std::endl;
        }
        
        #endif