/* Start Header ************************************************************************/
/*!
\file       ImGuiSystem.cpp
\author     Guan Shao Jun [10% - 200 lines]
\co-author	Muhammad Ashfer [87% - 1941 lines]
            Hwee Min [3% - 82 lines]
\par        shaojun.g@digipen.edu,
\date       October 02 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef USE_IMGUI
#include "ImGuiSystem.h"
#include "PrefabManager.h"
#include <iostream>
#include <chrono>
#include <map>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cctype>  
#include "core.h"
#include "RenderComponent.h"
#include <typeinfo>
#include <type_traits>
#include "LevelEditor.h"
#include "TilemapEditor.h"
#include "ColliderComponent.h"
#include "physics.h"
#include "AudioManager.h"
#include "logic.h"
#include "ScriptComponent.h"

// Keep externs as ImGuiSystem::Update now correctly relies on these globals
// being populated by CoreEngine *before* ImGui's display pass.
extern std::map<std::string, double> g_systemTimings;
extern double g_totalLoopTime;

/**
 * @brief Struct to hold Data for GO Creation
 * @param name		Name of GO
 * @param shpae		Name of shape of GO
 * @param pos[2]		(x,y) of GOs
 * @param scale[2]	(Width, Height)
 * @param rot		Rotation of GO
 * @param color		Vec4 holding colour of GO
 * @param collider
 */
struct TempGOData
{
    char name[128]{""};
    // std::string shape;
    int shapeIndex{0};
    float pos[2]{0.f, 0.f};
    float scale[2]{50.f, 50.f};
    float rot{0.f}; // = transform->GetRotation();
    ImVec4 color{0.f, 0.f, 0.f, 1.f};

    bool addCollider = false;
    int colType = 0; // 0 for Square, 1 for Circle
    float colSize[2] = { 100.0f, 100.0f };
    int colLayer = 5; // Default index for "Default" layer
};

// GO CREATION MENU VARS
TempGOData tempGO{"Game Object"};
static int current_shape = 0;

// Flags for IMGUI (current selection in drop down for GO Creator, and showing of some screens)
bool ShowCreateGOMenu;
bool ShowLoadPrefabMenu;

// SAVE/ LOAD EDITOR VARS
std::vector<std::string> editorLayoutList;
char saveLayoutName[128]{""};

// PREFAB VARS
std::vector<std::string> prefabList;
TempGOData tempPrefab;
static int current_prefab = 0;
static int prefab_renameIndex = -1;
static bool prefab_renameFocus = false;
char rename_str[128]{""};
std::string prefab_oldname{};

// HIERARCHY VARS
static int hierarchy_renameIndex = -1;
static bool hierarchy_renameFocus = false;
char hierarchy_rename_str[128]{ "" };
std::string hierarchy_oldname{};

// CONTAINERS
std::vector < std::tuple<ComponentType, std::string, ImTextureID, std::function<void(GameObject*)>>>  AllComponents;
std::unordered_map<std::string, std::vector<EntityID>> prefabInstances;

// STATIC BOOL FLAGS
static bool greyOutGOSettings = false;
static bool openLayoutSavePopup = false;
static bool openLayoutLoadPopup = false;
static bool saveNamePopupClear = false;
static bool renderLoadedLayout = false;
static bool showAudioAssetsPanel = false;
static bool g_showAudioErrorPopup = false;
static std::string g_audioErrorMessage;
static bool showTilemapEditor = true;
static std::unique_ptr<TilemapEditor> tilemapEditor = nullptr;

// VIEWPORT
bool isHoveringOverViewport;
bool isViewportFocused;
int gizmo_type = -1;
ImVec2 viewportSize;
ImVec2 viewportPos;
std::unordered_set<EntityID> lockedGizmosGOs{};

// Animator Preview
bool isAnimPreviewFocused;

// PROPERTIES
char draggedSprite_str[128]{"<Drag Image Here>"};

// IMGUI COLOURS
ImVec4 color_icon_highlighted{ 0.5f, 0.5f, 0.5f, .7f };
ImVec4 color_icon_normal{ 1.f, 1.f, 1.f, .7f };
ImVec4 color_icon_bg{ 1.f, 1.f, 1.f, 1.f };

// Vars for asset manager interface
const float paddingSize = 16.f;
const float cellSize = icon_size.x + paddingSize;
DirectoryNavigator AssetBrowserWindow;

// DRAG AND DROP VARS
ImVec2 viewportCursorPos;
vec2 Prefab_Reinstantiate_Pos;

// LAYOUT LOADING 
std::string loadLayoutPath{};

// TEXTURE IDS FOR ICONS
GLuint icon_folder_ui;
GLuint icon_file_ui;
GLuint icon_sprite_ui;
GLuint icon_plus_ui;
GLuint icon_prefab_ui;
GLuint icon_audio_ui;
GLuint icon_json_ui;
GLuint icon_giz_select_ui;
GLuint icon_giz_translate_ui;
GLuint icon_giz_scale_ui;
GLuint icon_giz_rotate_ui;
GLuint icon_giz_resize_ui;
GLuint icon_active_ui;
GLuint icon_inactive_ui;

// selected game object for editing
static EntityID selectedGameObjectID = 0;

// stress test 
static bool stressTestActive = false;

/**
 * @brief Ctor - Inits window variable
 */
ImGuiSystem::ImGuiSystem(GLFWwindow *window) : m_window(window), factory(GameObjectFactory::Instance()), selectedGameObjectID(0), showPropertyEditor(true) {}

/**
 * @brief Dtor - Calls Funct to terminate properly
 */
ImGuiSystem::~ImGuiSystem()
{
    Shutdown();
}

/**
 * @brief Initialises ImGUI
 */
void ImGuiSystem::Initialize()
{
    IMGUI_CHECKVERSION();   // Verify ImGui version
    ImGui::CreateContext(); // Create ImGui context
    // IO object (moved to EditorLayout_Load)
    ImGui::StyleColorsDark();                                     // Set dark theme
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);                 // Initialize GLFW backend
    ImGui_ImplOpenGL3_Init("#version 450");                       // Initialize OpenGL backend
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    // ReloadPrefabsList();

    ShowCreateGOMenu = false; // Hide create menu initially

    // init default docking positions
    // TODO make this serialisable, so it remembers which one to load
    std::string layoutPath = LayoutsFilePath + "DefaultLayout.ini";
    ImGuiIO &io = ImGui::GetIO();
    //io.IniFilename = layoutPath.c_str();
    io.IniFilename = nullptr;
    ImGui::LoadIniSettingsFromDisk(layoutPath.c_str());

    // Imgui flags
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    tilemapEditor = std::make_unique<TilemapEditor>();

    // ASSET MANAGER/ DIRECTORY NAVIGATOR VARIABLES
    // Vars for ImGui::ImageButton
    icon_folder_ui = GraphicsSystem::LoadTexture(icon_folder);
    icon_file_ui = GraphicsSystem::LoadTexture(icon_file);
    icon_sprite_ui = GraphicsSystem::LoadTexture(icon_sprite);
    icon_plus_ui = GraphicsSystem::LoadTexture(icon_plus);
    icon_prefab_ui = GraphicsSystem::LoadTexture(icon_prefab);
    icon_audio_ui = GraphicsSystem::LoadTexture(icon_audio);
    icon_json_ui = GraphicsSystem::LoadTexture(icon_json);
    icon_giz_select_ui = GraphicsSystem::LoadTexture(icon_giz_select);
    icon_giz_translate_ui = GraphicsSystem::LoadTexture(icon_giz_translate);
    icon_giz_scale_ui = GraphicsSystem::LoadTexture(icon_giz_scale);
    icon_giz_rotate_ui = GraphicsSystem::LoadTexture(icon_giz_rotate);
    icon_giz_resize_ui = GraphicsSystem::LoadTexture(icon_giz_resize);
    icon_active_ui = GraphicsSystem::LoadTexture(icon_active);
    icon_inactive_ui = GraphicsSystem::LoadTexture(icon_inactive);

    PopulateComponentList();
}

/**
 * @brief Update - Runs each frame to update IMGui screens
 * @param dt		  Delta Time
 */
void ImGuiSystem::Update(float dt)
{
    if (renderLoadedLayout)
    {
        // reload layouts next frame, do this so won't crash/ have inconsistent frame end times
        EditorLayout_LoadLayout(loadLayoutPath);
        renderLoadedLayout = false;                 // set flags that will open imgui windows
        openLayoutLoadPopup = true;
        return;
    }
    if (InputManager::KeyStateImgui)
    {
        // KEPT HERE FOR OUR REFERENCE - BUT MOVED THIS TO CORE ALREADY
        // ImGui_ImplOpenGL3_NewFrame(); // Start new OpenGL frame
        // ImGui_ImplGlfw_NewFrame();    // Start new GLFW frame
        // ImGui::NewFrame();        // Start ImGui frame
        if (ImGui::IsKeyPressed(ImGuiKey_T) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
        {
            showTilemapEditor = !showTilemapEditor;
        }
        // Full window docking
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        // Flags & Docking Settings
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("Window Dock", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("DockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();

        TopMenuBar_Render();

        LevelEditor* levelEditor = CORE->GetLevelEditor();
        bool isPlaying = (levelEditor && levelEditor->GetEditorMode() == EditorMode::Play);

        if (!isPlaying)
        {
            CORE->GetGraphicsSystem()->SetEditorMode(true);
            // GAME VIEWPORT - Always visible (outside conditional so it shows even in play mode)
            {
                TileMap_Render(dt);                       // Rendering first so viewport shows first by default
                RenderAnimationPreviewer();

                ImGui::SetNextWindowSize(ImVec2(800.0f, 600.0f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowPos(ImVec2(50.0f, 50.0f), ImGuiCond_FirstUseEver);

                ImGui::Begin("Game Viewport", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus);
                viewportSize = ImGui::GetContentRegionAvail();                                   // Viewport checks how much space there is at sets it as size
                CORE->GetGraphicsSystem()->RenderGameViewportImGui(viewportSize.x, viewportSize.y);

                //ImGui::SetItemAllowOverlap();                                                           // to drag and click without moving window
                ImGui::Image((void *)(intptr_t)CORE->GetGraphicsSystem()->GetFrameBufferTexture(), viewportSize, ImVec2(0, 1), ImVec2(1, 0));           // Viewport 

                // Render gizmos on viewport
                if (!isTileMapFocused && !isAnimPreviewFocused)
                {
                    // RENDER SECTION
                    Viewport_RenderGizmoToolbar();
                }
                // LOGIC SECTION
                Viewport_ManageGizmos();

                // If hovering over viewport, you can click to select a GO
                // TODO implement outline for selected GO
                if (ImGui::IsItemHovered())
                {
                    Viewport_ClickToSelect();
                    isHoveringOverViewport = true;
                }
                else
                {
                    isHoveringOverViewport = false;
                }

                std::string draggedPrefab{};
                // Viewport accepts Prefabs for drag and drop
                ReceiveDragAndDrop(PrefabPayloadID, draggedPrefab);

                if (!draggedPrefab.empty())
                {
                    // filter for type of data here
                    //std::cout << draggedPrefab << std::endl;                         // Prints the data sent via drag and drop
                    //Serialization::LoadPrefab(draggedPrefab);

                   // Load prefab AND retrieve the root object created
                    GameObject* root = Serialization::LoadPrefabWithReturn(draggedPrefab);

                    if (root)
                    {
                        // Record Undo Entries - Undo Create on Drag and Drop
                        LevelEditor* editor = CORE->GetLevelEditor();
                        if (editor)
                        {
                            EditorUndoCommand cmd;
                            cmd.action = EditorUndoCommand::ActionType::ObjectCreated;
                            cmd.entityID = root->GetID();

                            // Save object's entire state for redo
                            cmd.afterJson = Serialization::SerializeGameObject(root);

                            editor->RecordUndo(cmd);
                        }
                    }
                
                }

                // Viewport accepts Sprites for drag and drop
                std::string draggedSprite{};
                ReceiveDragAndDrop(SpritePayloadID, draggedSprite);

                if (!draggedSprite.empty())
                {
                    std::cout << "[Viewport] Dropped sprite: " << draggedSprite << std::endl;

                    // Extract sprite name from path
                    fs::path spritePath(draggedSprite);
                    std::string spriteName = spritePath.stem().string();

                    // Create GameObject
                    GameObject* newGO = factory.CreateGameObject(spriteName);

                    if (newGO)
                    {
                        float defaultVal{ 64.f };
                        vec2 size{ defaultVal, defaultVal };           // default
                        // Add Transform at cursor position
                        auto* transform = newGO->AddComponent<TransformComponent>();
                        // checks and adjustments below to let everything load before changing pos and scale

                        // Add Render with texture
                        auto* render = newGO->AddComponent<RenderComponent>();
                        if (render)
                        {
                            auto& resourceMgr = ResourceManager::Instance();
                            std::string textureKey = spriteName + "_texture";

                            if (resourceMgr.LoadTexture(textureKey, draggedSprite))
                            {
                                TextureResource* texRes = resourceMgr.GetTexture(textureKey);
                                if (texRes && texRes->IsLoaded())
                                {
                                    render->SetTextureID(texRes->GetTextureID());
                                    render->SetTexturePath(draggedSprite);
                                    render->SetShapeType(RenderComponent::ShapeType::Square);
                                    render->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
                                }
                                size = render->GetTextureResolution(3);
                                if (!size.x && !size.y)                                         // If invalid value found, set back to default value of 64,64. Else keep the aspect ratio of texture
                                    size = { defaultVal , defaultVal };
                                //std::cout << "\nMIP LEVEL 3 res: " << size.x << ' ' << size.y << "\n";
                            }
                        }

                        if (transform)
                        {
                            transform->SetPosition(viewportCursorPos.x, viewportCursorPos.y);
                            transform->SetScale(size.x, size.y); // Default size
                        }

                        // Add Square Collider - CORRECTED CONSTRUCTOR CALL!
                        // Constructor: ColliderComponent(owner, type, width, height, layer)
                        newGO->AddComponent<ColliderComponent>(
                            ColliderComponent::Type::Square,
                            size.x,
                            size.y,
                            CollisionLayer::Default
                        );

                        // Select the new GameObject
                        SetSelectedGameObject(newGO->GetID());

                        std::cout << "[Viewport] Created sprite GameObject: " << spriteName << std::endl;
                    }
                }
                // gets cursor pos WRT world pos
                viewportPos = ImGui::GetItemRectMin();
                viewportCursorPos = CursorToWorldPos(viewportPos, viewportSize);

                ImGui::Text("(World Space) Hovering at: (%.1f, %.1f)", viewportCursorPos.x, viewportCursorPos.y);
                ImGui::End();
            }

            // Only show editor UI when not in play mode
            Hierarchy_Render();
            AssetBrowserWindow.RenderImGui();

            // drawing highlight when a GO is selected
            if (selectedGameObjectID != 0)
            {
                Viewport_HighlightGO();
            }
            
            // only show audio assets if toggled on
            if (showAudioAssetsPanel)
            {
                AudioAssetsPanel_Render();
            }

            // render property editor for selected game object
            if (showPropertyEditor)
            {
                RenderPropertyEditor();
            }
            // Render profiler! Make it toggleable?
            Profiler_Render();
        }
        // if IS in play mode
        else
        {
            // toggle off other debug/ editor stuff.
            CORE->GetGraphicsSystem()->ClearEditorSelection();
        }

        if (CORE->GetLevelEditor()->GetEditorMode() != EditorMode::Play)
        {
            /**
             * @brief Game Object Creator Menu IMGui Window
             */
            if (ShowCreateGOMenu)
            {
                GOCreationMenu_Render();
            }

            /**
                * @brief Prefab Loader Menu IMGui Window
                */
            if (ShowLoadPrefabMenu)
            {
                PrefabMenu_Render();
            }
        }
        
        // Flag with 1 frame delay on whether something is being drag and dropped, used for spawning prefabs via DND
        Serialization::Prefab_DragAndDropped = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
    }
}

/**
 * @brief Terminates and releases resources allocated for imgui
 */
void ImGuiSystem::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown(); // Shutdown OpenGL3 backend
    ImGui_ImplGlfw_Shutdown();    // Shutdown GLFW backend
    ImGui::DestroyContext();      // Destroy ImGui context
}

/**
 * @brief Get System Name
 */
std::string ImGuiSystem::GetName()
{
    return "ImGui_System"; // Return descriptive system name
}

ImVec2 CursorToWorldPos(ImVec2 vpPos, ImVec2 vpSize)
{
    ImVec2 cursorScreenPos = ImGui::GetMousePos();
    const float camZoom = CORE->GetLevelEditor()->GetCameraZoom();
    const vec2 camPos = { CORE->GetLevelEditor()->GetCameraX(), CORE->GetLevelEditor()->GetCameraY() };

    // Evil function that took me like an hour to figure out and prolly took even more years off my lifespan
    // but basically the gist is
    // [from top left] local cursor pos - viewports position - viewportsize/2 (so we can have 0,0 be centered) / camerazoom + camera position
    // camera zoom represents how zoomed in we are, so cursor needs to scale off that. Camera pos is alr in world space so just add it onto!
    float worldx = (cursorScreenPos.x - vpPos.x - (vpSize.x / 2.f)) / camZoom + camPos.x;
    float worldy = ((vpSize.y / 2.f) - (cursorScreenPos.y - vpPos.y)) / camZoom + camPos.y;
    return ImVec2(worldx, worldy);
}

/**
 * @brief Manages logic for clicking to select GOs and DNDing to move them around
 */
void ImGuiSystem::Viewport_ClickToSelect()
{
    // Loop for all GOs in scene
    int highestLayer = std::numeric_limits<int>::min();
    GameObject* mostFrontGO = nullptr;
    for (auto& go : GameObjectFactory::Instance().GetAllGameObjects())
    {
        if (!go->IsActive()) continue;

        auto* transform = go->GetComponent<TransformComponent>();           // GOs with no transform will not be interactable
        if (!transform) continue;

        vec2 pos{ 0.f, 0.f }, scale{ 0.f, 0.f };                            // Get position and scale
        transform->GetPosition(pos.x, pos.y);
        transform->GetScale(scale.x, scale.y);

        // if GO has a collider
        // Calculates if cursor is within area of GO
        // Updated post M3 to fix issues with layering
        if (viewportCursorPos.x > pos.x - scale.x / 2 && viewportCursorPos.x < pos.x + scale.x / 2 && viewportCursorPos.y > pos.y - scale.y / 2 && viewportCursorPos.y < pos.y + scale.y / 2
            && !ImGuizmo::IsUsing())
        {
            int currLayer{};
            auto* render = go->GetComponent<RenderComponent>();
            if (render)
            {
                currLayer = render->GetLayer();
            }
            if (currLayer > highestLayer)
            {
                highestLayer = currLayer;
                mostFrontGO = go.get();
            }
        }
    }

    if (mostFrontGO && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsMouseDragging(ImGuiMouseButton_Left))            // changed back to on mouse click, since we have gizmos, wont have accidental moving anymore
    {
        SetSelectedGameObject(mostFrontGO->GetID());                         // Set GO to be selected

        std::cout << "[has collider]clicked on " << mostFrontGO->GetName() << std::endl;
    }
}

#pragma region Profiler ImGui
/**
 * @brief Performance Profiler IMGui Window
 */
void Profiler_Render()
{
    ImGui::Begin("=== Performance Profiler ===");

    static std::map<std::string, float> smoothedMs;
    static std::map<std::string, float> smoothedPercent;

    const float lerpSpeed = static_cast<float>(0.05); // smaller = smoother

    std::map<std::string, double>::iterator it;
    for (it = g_systemTimings.begin(); it != g_systemTimings.end(); ++it)
    {
        const std::string &name = it->first;
        double ms = it->second;
        double percent = (g_totalLoopTime > 0.0) ? (ms / g_totalLoopTime) * 100.0 : 0.0;

        // Initalize if first time
        if (smoothedMs.find(name) == smoothedMs.end())
        {
            smoothedMs[name] = (float)ms;
            smoothedPercent[name] = (float)percent;
        }

        // Lerp towards raw ms and percent
        smoothedMs[name] += (float)(ms - smoothedMs[name]) * lerpSpeed;
        smoothedPercent[name] += (float)(percent - smoothedPercent[name]) * lerpSpeed;

        // Add a small ImGui::Text displaying the total frame time first for context
        if (it == g_systemTimings.begin())
        {
            ImGui::Text("Total System Time: %.3f ms", g_totalLoopTime);
            ImGui::Text("Total Frame Per Seconds: %f ms", CoreEngine::GetFPS());
            ImGui::Text("Total Delta Time: %f ms", CoreEngine::GetDeltaTime());
            ImGui::Separator();
        }

        ImGui::Text("%-20s %.4f ms (%.1f%%)", name.c_str(), smoothedMs[name], smoothedPercent[name]);
        ImGui::ProgressBar((float)(smoothedPercent[name] / 100.0f), ImVec2(0.0f, 0.0f));
    }
    ImGui::End();
}
#pragma endregion

#pragma region Game Object Creation ImGui
/**
 * @brief Renders the game object creation menu and handles logic as well. I know it doesnt only render...but its the most intuitive naming convention btwn our programmers :)
 */
void GOCreationMenu_Render()
{
    ImGui::Begin("Game Object Creator", &ShowCreateGOMenu); // Mark beginning of window
    {
        // Game Object Text Field
        ImGui::Text("Name");
        if (ImGui::InputText("#", tempGO.name, sizeof(tempGO.name)))
        {
        } // Text input field for GO Name

        // Very ugly sorry, future impl can include a check for empty GOs
        const char *ShapeTypes[5] = {"Square", "Circle", "Line", "Point", "Empty"}; // Type of shapes
        ImGui::Text("Shape Type");
        if (ImGui::BeginCombo("##", ShapeTypes[current_shape]))
        { // Drop down menu for each shape
            for (int i = 0; i < 5; i++)
            {
                bool is_selected = (current_shape == i);
                if (ImGui::Selectable(ShapeTypes[i], is_selected))
                    current_shape = i;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();

                // If selected shape is empty, block out colour selection
                greyOutGOSettings = (!strcmp(ShapeTypes[current_shape], "Empty")) ? true : false;
            }
            ImGui::EndCombo(); // End drop down menu
        }

        if (greyOutGOSettings)
        {
            ImGui::BeginDisabled(true);
        }

        //----- will be disabled if shape selected is empty -----

        // Shape Colour Picker
        ImGui::Text("Colour");
        if (ImGui::ColorEdit3(" ", (float *)&tempGO.color))
        {
        } // Colour picker to later store in Vec4

        //----- will be disabled if shape selected is empty -----

        if (greyOutGOSettings)
        {
            ImGui::EndDisabled();
        }

        // Game Object Position
        ImGui::Text("Position");
        if (ImGui::DragFloat2("##", tempGO.pos, 0.1f))
        {
        } // Slider for GO position

        // Game Object Scale
        ImGui::Text("Scale");
        if (ImGui::DragFloat2("###", tempGO.scale, 0.1f))
        {
        } // Slider for GO scale

        // Game Object Rotation
        ImGui::Text("Rotation");
        if (ImGui::DragFloat("####", &tempGO.rot, 0.1f))
        {
        } // Slider for GO rot

        ImGui::Separator();

       // Game Object Collider
        ImGui::Checkbox("Add Collider", &tempGO.addCollider);

        if (tempGO.addCollider)
        {
            ImGui::Indent();

            const char* colTypes[] = { "Square", "Circle" };
            ImGui::Combo("Type", &tempGO.colType, colTypes, IM_ARRAYSIZE(colTypes));

            ImGui::DragFloat2("Size", tempGO.colSize, 1.0f);

            const char* layers[] = { "Player", "Ground", "Wall", "Enemy", "Platform", "Default" };
            ImGui::Combo("Layer", &tempGO.colLayer, layers, IM_ARRAYSIZE(layers));

            ImGui::Unindent();
        }

        // Generate Game Object
        ImGui::Text("  ");
        if (ImGui::Button("Generate")) // Button to create new GO
        {
            GOCreationMenu_Spawn();
        }
    }
    ImGui::End(); // End of GO Creation
}


/**
 * @brief New cool hierarchy! Now handles DND and other scene related features
 */
void ImGuiSystem::Hierarchy_Render()
{
    ImGui::SetNextWindowPos(ImVec2(10, 370), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 600), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Hierarchy##LevelEditor", nullptr, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }

    // Render top part to show adding of GOs
    if (ImGui::ImageButton("##Create GO Button", ImTextureID(icon_plus_ui), icon_size_small, {0, 1}, {1, 0},
                           (ImVec4(1.f, 1.f, 1.f, .7f)), (ImVec4(1, 1, 1, 1))))
    {
        // this is a file, not sure if we wanna do anything yet
        ShowCreateGOMenu = (!ShowCreateGOMenu) ? true : false;
    }
    CreateHintText("Create Game Object");

    ImGui::SameLine();
    // Render top part to show adding of Prefabs
    if (ImGui::ImageButton("##Spawn Prefab", ImTextureID(icon_prefab_ui), icon_size_small, {0, 1}, {1, 0},
                           (ImVec4(1.f, 1.f, 1.f, .7f)), (ImVec4(1, 1, 1, 1))))
    {
        // this is a file, not sure if we wanna do anything yet
        ReloadPrefabsList();
        ShowLoadPrefabMenu = (!ShowLoadPrefabMenu) ? true : false;
    }
    CreateHintText("Spawn Prefab");

    ImGui::SameLine();
    // Audio Assets toggle button
    if (ImGui::ImageButton("##AudioAssetsToggle", ImTextureID(icon_audio_ui),
        { icon_size_small.x, icon_size_small.y },
        { 0, 1 }, { 1, 0 },
        ImVec4(1.f, 1.f, 1.f, 0.7f), ImVec4(1, 1, 1, 1)))
    {
        showAudioAssetsPanel = !showAudioAssetsPanel;
    }
    CreateHintText("Toggle Audio Assets panel");

    // dear god help us all
    if (!stressTestActive)
    {
        if (ImGui::Button("Run Stress Test"))
        {
            stressTestActive = true;
            SpawnStressObjects();   // your spawn function
        }
    }
    else
    {
        if (ImGui::Button("Delete Stress Test"))
        {
            stressTestActive = false;
            DestroyStressTestGameObjects();
        }
    }

    // Undo Feature
    LevelEditor* editor = CORE->GetLevelEditor();
    bool canUndo = !editor->undoStack.empty();

    if (!canUndo) ImGui::BeginDisabled();
    if (ImGui::Button("Undo"))
    {
        if (canUndo) {
            CORE->GetLevelEditor()->PerformUndo();       
        } 
    }
    if (!canUndo) ImGui::EndDisabled();

    // display current scene name
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
    if (auto* levelEditor = CORE->GetLevelEditor())
    {
        ImGui::Text("Scene: %s", CORE->GetLevelEditor()->GetCurrentFilename().c_str());
    }
    else
    {
        ImGui::Text("Scene: Unknown");
    }
    ImGui::PopStyleColor();
    ImGui::Separator();
        
    // display object count
    auto &allObjects = factory.GetAllGameObjects();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::Text("Game Objects: %zu", allObjects.size());
    ImGui::PopStyleColor();
    ImGui::Separator();

    // list all game objects, if they have no parent (root), if not render the tree display!
    for (const auto &objPtr : allObjects)
    {
        if (!objPtr->GetParent())
        {
            Hierarchy_GameObjectTree_Renderer(objPtr.get(), 0, selectedGameObjectID, factory);
        }
    }
    // remove selected GO when clicking on empty space
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (!ImGui::IsAnyItemHovered())
        {
            ResetSelectedGameObject();
        }
    }

    ImVec2 spaceAvail = ImGui::GetContentRegionAvail();
    ImGui::Dummy(spaceAvail);
    if (spaceAvail.y > 0.f)
    {
        std::string draggedID{};
        ReceiveDragAndDrop(HierarchyPayloadID, draggedID);
        if (!draggedID.empty())
        {
            EntityID deliveredID = EntityID(std::stoi(draggedID));       // Convert payload data to EntityID
            GameObject* go = factory.GetGameObject(deliveredID);

            if (!go) return;
            GameObject* goParent = go->GetParent();
            if (goParent)
            {
                goParent->RemoveChild(go);
            }
        }
    }
    ImGui::End();

    // Press F to focus onto GO
    // TODO can make this like a nice panning if we want, but naw
    if (ImGui::IsKeyPressed(ImGuiKey_F) && selectedGameObjectID != 0)
    {
        // TODO set camera focus to that game object
        auto* transform = GetSelectedGameObject()->GetComponent<TransformComponent>();
        if (!transform) return;
        vec2 newPos{};
        float newZoom{ (viewportSize.x - transform->scaleX)/viewportSize.x };
        transform->GetPosition(newPos.x, newPos.y);
        CORE->GetLevelEditor()->SetCameraPos(newPos);
        CORE->GetLevelEditor()->SetCameraZoom(newZoom);
    }
}

void ImGuiSystem::Hierarchy_GameObjectTree_Renderer(GameObject *obj, int depth, EntityID &selectedGOID, GameObjectFactory &p_factory)
{
    // Just commented this out in case want to revert, but now we can show inactive GOs but greyed out
    //if (!obj->IsActive())
    //    return;
    if (!obj) return;
    EntityID id = obj->GetID();

    // Set tree node flags
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (selectedGOID == id)
        flags |= ImGuiTreeNodeFlags_Selected;

    // Apply indentation based on depth
    ImGui::Indent(depth * 10.0f);
    bool nodeOpen{};

    // gotta do this for proper ids, was causing crashes
    std::string buttonName = "##Toggle Active" + std::to_string(int(id));

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));             // remove bg for image [PUSH]
    if (obj->IsActive())
    {
        if (ImGui::ImageButton(buttonName.c_str(), ImTextureID(icon_active_ui), icon_size_smallest, {0, 1}, {1, 0},
            ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
        {
            obj->SetActive(false);
        }
    }
    else
    {
        if (ImGui::ImageButton(buttonName.c_str(), ImTextureID(icon_inactive_ui), icon_size_smallest, { 0, 1 }, { 1, 0 },
            ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
        {
            obj->SetActive(true);
        }
    }
    ImGui::PopStyleColor();                                                 // remove bg for image [POP]
    ImGui::SameLine();

    // Grey out text if inactive [PUSH]
    if (!obj->IsActive())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    }

    ImGui::AlignTextToFramePadding();
    // if renaming, set the name of the tree node as an input text field
    if (hierarchy_renameIndex == (int)id)
    {
        ImGui::PushID(id);
        ImGuiTreeNodeFlags nameFlags = flags | ImGuiTreeNodeFlags_Leaf;

        nodeOpen = ImGui::TreeNodeEx("##RenamingNode", nameFlags);
        ImGui::SameLine();
        if (hierarchy_renameFocus)
        {
            ImGui::SetKeyboardFocusHere();
            hierarchy_renameFocus = false;
        }
        if (ImGui::InputText("##Renaming", rename_str, IM_ARRAYSIZE(rename_str), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
        {
            GetSelectedGameObject()->SetName(std::string(rename_str));
            hierarchy_renameIndex = -1;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_Enter))
        {
            GetSelectedGameObject()->SetName(std::string(rename_str));
            hierarchy_renameIndex = -1;
        }

        ImGui::PopID();
    }
    else
    {
        // Display the tree node
        nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)id, flags, "%s (ID: %u)", obj->GetName().c_str(), id);
    }

    // Grey out text if inactive [POP]
    if (!obj->IsActive())
    {
        ImGui::PopStyleColor();
    }

    if (!ImGui::IsItemFocused() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        hierarchy_renameIndex = -1;
    }

    // If has children, make it expandable
    if (obj->GetAllChildren().empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (selectedGOID == id)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // Drag and drop for Parenting
    if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        SendDragAndDrop(HierarchyPayloadID, obj->GetID(), obj->GetName());
    }

    {
        // receiving drag and drop
        std::string draggedID{};
        ReceiveDragAndDrop(HierarchyPayloadID, draggedID);
        // Do something with DnD data
        if (!draggedID.empty())
        {
            std::vector<EntityID> familyTree{};
            GetFamilyTree(obj, familyTree);                         // Get all descendent game objects (child of child wtv) underneath the parrent GO

            /*std::cout <<  obj->GetName() <<"'s GetFamilyTree() has: \n";
            for (auto const &r : familyTree)
            {
                std::cout << p_factory.GetGameObject(r)->GetName() << std::endl;
            }*/

            EntityID deliveredID = EntityID(std::stoi(draggedID));       // Convert payload data to EntityID
            auto check = std::find(familyTree.begin(), familyTree.end(), deliveredID);

            std::vector<EntityID> selfParentingTree{};
            GetFamilyTree(p_factory.GetGameObject(deliveredID), selfParentingTree);
            auto checkSelfParenting = std::find(selfParentingTree.begin(), selfParentingTree.end(), obj->GetID());

            if (check == familyTree.end() && checkSelfParenting == selfParentingTree.end())
            {
                GameObject *deliveredGO = p_factory.GetGameObject(deliveredID);
                std::cout << "[Hierarchy] Setting " << deliveredGO->GetName() << " to be child of " << obj->GetName() << std::endl;
                deliveredGO->SetParent(obj);                                        // Set Dragged GO to be parent of receiver GO

                if (!nodeOpen && !obj->GetAllChildren().empty())
                {
                    // Sets tree node to be expanded then renders it 
                    ImGuiStorage *storage = ImGui::GetStateStorage();
                    storage->SetInt(ImGui::GetID((void *)(intptr_t)id), 1);
                }
            }
            else
            {
                std::cout << "[Hierarchy] Invalid Drag and Drop source!\n";
            }
        }
    }

    // Set selected item
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        SetSelectedGameObject(id);
    }

    EntityID deleteID{0};                                                        // fast check if game object should be deleted
    if (isHoveringOverViewport && selectedGameObjectID != 0 && ImGui::IsKeyPressed(ImGuiKey_Delete))
        deleteID = selectedGameObjectID;

    // Right Click event (for specific GO in hierarchy)
    if (ImGui::BeginPopupContextItem())
    {
        SetSelectedGameObject(id);                                          // right click auto selects them, but sure let them click select also 
        if (ImGui::MenuItem("Select"))
        {
            SetSelectedGameObject(id);
        }
        if (ImGui::MenuItem("Rename"))
        {
            // rename shiz here
            hierarchy_renameIndex = id;
            strncpy(rename_str, GetSelectedGameObject()->GetName().c_str(), sizeof(hierarchy_rename_str));
            prefab_oldname = GetSelectedGameObject()->GetName();
            hierarchy_renameFocus = true;
        }
        if (ImGui::MenuItem("Save as Prefab"))
        {
            Serialization::SavePrefab(*obj);
        }
        if (ImGui::MenuItem("Clone"))
        {
            p_factory.CloneGameObject(id);
        }
        // find way to unparent by drag and drop in the future
        if (obj->GetParent() && ImGui::MenuItem("Make Root"))
        {
            obj->GetParent()->RemoveChild(obj);
        }

        if (ImGui::MenuItem("Delete"))
            deleteID = id;

        ImGui::EndPopup();
    }

    if (deleteID)
    {
        // TODO - block out the deleting of main camera - crashes the game

        // Record Undo Entries - Undo Delete
        if (factory.GetGameObject(deleteID)->GetName() == "MainCamera")
            return;

        LevelEditor* editor = CORE->GetLevelEditor();
        GameObject* gameObj = factory.GetGameObject(deleteID);

        if (gameObj)
        {       
            EditorUndoCommand undocmd;
            undocmd.action = EditorUndoCommand::ActionType::ObjectDeleted;
            undocmd.entityID = deleteID;
            //cmd.beforeState = editor->SerialiseGameObject(obj);
            undocmd.beforeJson = Serialization::SerializeGameObject(gameObj);

            // Store Parent
            GameObject* parent = gameObj->GetParent();
            undocmd.parentID = parent ? parent->GetID() : 0;

            if (parent) // Get child order
            {
                for (GameObject* c : parent->GetAllChildren())
                    undocmd.siblingOrderIDs.push_back(c->GetID()); // Order of children
            }
            else { // Get root order
                auto& allObjs = GameObjectFactory::Instance().GetAllGameObjects();
                for (auto& u : allObjs) {
                    undocmd.rootOrderIDs.push_back(u->GetID()); // Order of root objects
                }
            }

            // Store Children
            for (GameObject* c : gameObj->GetAllChildren())
                undocmd.childIDs.push_back(c->GetID());

            editor->RecordUndo(undocmd);

            factory.DestroyAllRelatedGameObjects(deleteID);  // handles cases for both individual GOs and parented GOs

            // Clear selection if it was selected
            if (selectedGameObjectID == deleteID)
                ResetSelectedGameObject();
        }
    }

    // Render all children of opened parent GO
    if (nodeOpen)
    {
        // repeats loop again
        for (auto &childPtr : obj->GetAllChildren())
        {
            Hierarchy_GameObjectTree_Renderer(childPtr, depth+1, selectedGOID, p_factory);
        }
        ImGui::TreePop();
    }
    ImGui::Unindent(depth * 10.0f);
}

/**
 * @brief Within the game object creation menu, handles the spawning of GOs with the specified properties
 */
void GOCreationMenu_Spawn()
{
    // Determine creation type based on shape
    CreateObjectMessage::ObjectType objType;
    RenderComponent::ShapeType shapeType = static_cast<RenderComponent::ShapeType>(current_shape);      // get shape type enum

    // Special case for empty GOs
    if (shapeType == RenderComponent::ShapeType::Empty)
    {
        objType = CreateObjectMessage::ObjectType::Empty;
    }
    else
    {
        objType = CreateObjectMessage::ObjectType::Basic;
    }

    
    // Gather GO data to send
    float pos[2] = { tempGO.pos[0], tempGO.pos[1] };
    float scale[2] = { tempGO.scale[0], tempGO.scale[1] };
    float rot = tempGO.rot;
    vec4 color = vec4(tempGO.color.x, tempGO.color.y, tempGO.color.z, 1.f);
    std::string name = tempGO.name;
    ColliderComponent::Type cType = (tempGO.colType == 0) ? ColliderComponent::Type::Square : ColliderComponent::Type::Circle;
    CollisionLayer cLayer = static_cast<CollisionLayer>(tempGO.colLayer);

    // Construct message and send
    CreateObjectMessage msg(
        name,                   // GameObject name
        objType,                // Shape if is Empty or Basic
        shapeType,              // ShapeType
        pos, scale, rot,        // Transform
        color,               // Colour
        tempGO.addCollider,
        cType,
        tempGO.colSize[0], tempGO.colSize[1],
        cLayer
    );

    std::cout << "[ImGuiSystem] Sending CreateObjectMessage for " << tempGO.name << std::endl;
    //msg.Dispatch(*CORE);
    CORE->GetMessageBus().Publish(msg);
}

/**
 * @brief Set the selected game object
 */
void ImGuiSystem::SetSelectedGameObject(EntityID id)
{
    selectedGameObjectID = id;
    CORE->GetGraphicsSystem()->ClearEditorSelection();              // need to clear after resassigning so newly selected GO can have the correct outlines drawn
}

void ImGuiSystem::ResetSelectedGameObject()
{
    selectedGameObjectID = 0;
    CORE->GetGraphicsSystem()->ClearEditorSelection();
}

/**
 * @brief Remove a game object from the level
 */
void ImGuiSystem::RemoveGameObject(EntityID id)
{
    factory.DestroyGameObject(id);
}

#pragma endregion

#pragma region Prefab ImGui

/**
 * @brief Renders the prefab window on screen
 */
void ImGuiSystem::PrefabMenu_Render()
{
    ImGui::Begin("Prefab Menu", &ShowLoadPrefabMenu);

    ImGui::Text("Prefab Menu");
    ImGui::BeginGroup();
    if (ImGui::BeginListBox("##", ImVec2(200, 150))) // Draw selectable box for which prefabs are found
    {
        if (prefabList.empty())
        {
            ImGui::Text("No saved Prefabs found.");
        }
        else
        {
            // Draw list of prefabs
            for (int i = 0; i < prefabList.size(); ++i) // Loop for every prefab in list
            {
                if (prefab_renameIndex != i)
                {
                    bool is_selected = (current_prefab == i);
                    if (ImGui::Selectable(prefabList[i].c_str(), &is_selected))
                    {
                        current_prefab = i;         // Set selected
                    }
                    else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    {
                        current_prefab = i;         // Right click to select and open menu too
                        ImGui::OpenPopup("Prefab_RightClickEvent");
                    }

                    // If item is selected and being held, drag and drop event is taking place
                    if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    {
                        SendDragAndDrop(PrefabPayloadID, prefabList[i], prefabList[i]);
                    }
                    if (i != prefabList.size() - 1)
                    {
                        ImGui::Separator();
                    }
                }
                else
                {
                    // For renaming of prefab files
                    ImGui::PushID(i);
                    if (prefab_renameFocus)
                    {
                        ImGui::SetKeyboardFocusHere();
                        prefab_renameFocus = false;
                    }
                    if (ImGui::InputText("##", rename_str, IM_ARRAYSIZE(rename_str), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                    {
                        prefabList[i] = std::string(rename_str);
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_Enter))
                    {
                        prefab_renameIndex = -1;
                        Serialization::RenamePrefab(prefab_oldname, rename_str);
                    }

                    ImGui::PopID();
                }
            }
        }

        if (ImGui::BeginPopup("Prefab_RightClickEvent"))            // If item in the prefab lists gets right clicked
        {
            if (ImGui::MenuItem("Rename"))                          // If rename button clicked, get name of input text field and set it to new name
            {
                prefab_renameIndex = current_prefab;
                strncpy(rename_str, prefabList[current_prefab].c_str(), sizeof(rename_str));
                prefab_renameFocus = true;
                prefab_oldname = prefabList[current_prefab];
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete"))
            {
                Serialization::RemovePrefab(prefabList[current_prefab]);        // Remove from file, reload list afterwards
                ReloadPrefabsList();
            }
            ImGui::EndPopup();
        }

        ImGui::EndListBox();
    }
    ImGui::EndGroup();

    ImGui::SameLine(); // This so its on the right side of the selection box

    // draw preview
    // TODO doesnt actually work yet
    ImGui::BeginGroup();
    ImVec2 boxSize = ImVec2(256, 256);
    ImGui::Text("[WIP] Preview Render:");
    ImGui::Dummy(boxSize); // Reserves space for preview

    ImDrawList* drawList = ImGui::GetWindowDrawList(); // Imgui rendering for box with white outline
    drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 255, 255));
    ImGui::EndGroup();


    if (ImGui::Button("Spawn -> Load Prefab"))
    {
        PrefabMenu_Spawn();                     // Spawn selected prefab as GO
    }
    CreateHintText("Spawn prefab as Game Object in scene.");

    ImGui::Dummy({30, 30}); // Reserves space for preview
    std::string dispText;
    bool bothSelected{ selectedGameObjectID && current_prefab };
    if (!selectedGameObjectID && !current_prefab)
    {
        dispText = "Select GO and prefab to override";
    }
    else
    {
        std::string goName{}, prefabName{};

        if (selectedGameObjectID) { goName = factory.GetGameObject(selectedGameObjectID)->GetName(); }
            else goName = "_____";
        if (current_prefab) { prefabName = prefabList[current_prefab]; }
            else prefabName = "_____";

        dispText = "Override " + prefabName + " with " + goName;
    }
    // looks kinda ugly but it makes it more obvious
    if (!bothSelected)
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), dispText.c_str());
    else
        ImGui::Text(dispText.c_str());

    if (!bothSelected)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1)),
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 0.3f, 0.3f, 1)),
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0, 0, 1));
    }
    if (ImGui::Button("Override"))
    {
        // takes the selected GO and replace the selected prefab with all the GO's data
        if (bothSelected)
        {
            PrefabMenu_Override();
        }
        else
        {
            std::cout << "Select GO in Scene + prefab in Prefab Menu!\n";
        }
    }
    CreateHintText("Override an existing prefab with a selected Game Object.");
    ImGui::SameLine();
    if (ImGui::Button("Update all instances"))
    {
        if (bothSelected)
        {
            // tweaked to work with spawned prefab objects or any game object
            // Gets selected prefab/ game object, replaces ALL newly spawned prefabs to be of that newly edited game prefab/ game object
            PrefabMenu_Reinstantiate();
        }
        else
        {
            std::cout << "Select GO in Scene + prefab in Prefab Menu!\n";
        }
    }
    CreateHintText("Update all newly created prefabs of selected type.");
    if (!bothSelected)
    {
        ImGui::PopStyleColor(3);
    }

    ImGui::End(); // End of Prefab Search Menu
}

/**
 * @brief Override existing prefab with edited prefab. Tweaked to also work for any GO in scene.
 */
void ImGuiSystem::PrefabMenu_Override()
{
    auto* selectedGO = factory.GetGameObject(selectedGameObjectID);
    if (!selectedGO) return;
    std::string selectedPrefabName = prefabList[current_prefab];

    // if same name, simply override 
    Serialization::SavePrefab(*selectedGO);
    if (selectedGO->GetName() != selectedPrefabName)                     // only delete prefab if names are different, if names are the same it gets overriden automatically
    {
        Serialization::RemovePrefab(selectedPrefabName);
        Serialization::RenamePrefab(selectedGO->GetName(), selectedPrefabName);
    }
}


/**
 * @brief Reinstantiated all newly spawned prefabs to be as the selected GO in scene.
 */
void ImGuiSystem::PrefabMenu_Reinstantiate()
{
    // If want to tweak for it to replace with only newly added prefabs, just do a check if the selectedGO is also in prefabInstances.
    // Tried to only replace components so we can keep the same entity IDs, but parenting is not a component and it causes issues. So we just delete and spawn again at specified location.

    Serialization::Prefab_Reinstantiate = true;                             // flag for specifying location of where to spawn prefab (Prefab_Reinstantiate_Pos)
    auto* selectedGO = factory.GetGameObject(selectedGameObjectID);
    std::string selectedPrefabName = prefabList[current_prefab];
    selectedGO->SetName(selectedPrefabName);                                // force set the name to be of selected prefab

    Serialization::SavePrefab(*selectedGO);                                 // Properly overrides the old prefab

    std::vector<EntityID> prefabReiterationList = prefabInstances[selectedPrefabName];          // make a copy because we will be deleting things from the active list

    for (EntityID p : prefabReiterationList)
    {
        GameObject* currPrefab = factory.GetGameObject(p);                                      // get GO
        if (!currPrefab || !currPrefab->GetComponent<TransformComponent>())
        {
            std::cout << "Could not find GO id: " << p << " or its transform. Skipping...\n";           // print if unable to modify GO, skip as well
            continue;
        }

        currPrefab->GetComponent<TransformComponent>()->GetPosition(Prefab_Reinstantiate_Pos.x, Prefab_Reinstantiate_Pos.y);            // get position for spawning of new prefab (handled in loadprefab)
        factory.DestroyGameObject(p);                                                               // destroy old GO and spawn new GO
        Serialization::LoadPrefab(selectedPrefabName);
    }
    Serialization::Prefab_Reinstantiate = false;                                // reset flag to false
}


/**
 * @brief Ctor - Spawns the selected prefab as a game object in scene
 */
void ImGuiSystem::PrefabMenu_Spawn()
{
    Serialization::LoadPrefab(prefabList[current_prefab]);          // Deserialises the selected prefab
}
#pragma endregion

#pragma region Viewport/ Gizmos
void ImGuiSystem::Viewport_ManageGizmos()
{
    if (selectedGameObjectID != 0 && gizmo_type != -1)
    {
        auto* go = GetSelectedGameObject();

        if (lockedGizmosGOs.contains(go->GetID()))
        {
            // draw locked gizmo sign
            //ImGuizmo::Enable(false);
        }

        auto* transform = go->GetComponent<TransformComponent>();
        auto* camComp = CameraManager::Instance().GetActiveCamera();
        if (!camComp)
        {
            std::cout << "[ManageGizmos]NO CAMERA FOUND!\n";
            return;
        }
        if (!transform)
        {
            std::cout << "[ManageGizmos]NO TRANSFORM FOUND!\n";
            return;
        }

        ImVec2 vpSize{ ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y };
        ImVec2 vpPos{ ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMin().y };

        ImGuizmo::SetOrthographic(true);        // or false idk
        ImGuizmo::SetDrawlist();                                                                                                            // draws current window
        ImGuizmo::SetRect(vpPos.x, vpPos.y, vpSize.x, vpSize.y);             // set rect to viewport

        //mat4 view = camComp->GetViewMatrixUnshaken();
        mat4 view = mat4::identity();

        //mat4 projection = camComp->GetProjectionMatrix(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());                                // replaced with getVp() bc this doesnt scale with cam zoom etc
        mat4 mtx = transform->GetMatrix();
        ImGuizmo::Manipulate(value_ptr(view), value_ptr(CORE->GetGraphicsSystem()->getVp()), ImGuizmo::OPERATION(gizmo_type), ImGuizmo::WORLD, value_ptr(mtx));

        if (gizmo_type == (int)(ImGuizmo::OPERATION::BOUNDS))
        {
            float* deltaMat{};                                                  // we not gonna use this for anything, just a filler ptr so we can access bounds
            float bounds[6] = { -0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f };         // plots the cross section so we can get the middle edges
            ImGuizmo::Manipulate(value_ptr(view), value_ptr(CORE->GetGraphicsSystem()->getVp()), ImGuizmo::OPERATION::BOUNDS, ImGuizmo::WORLD, value_ptr(mtx), deltaMat, deltaMat, bounds);     // manipulate with bounds in mind
        }

        // Record Undo Entries - Undo Transform on Gizmo
        static bool gizmoEditing = false;
        static json gizmoBeforeJson;

        if (ImGuizmo::IsUsing() && !gizmoEditing)
        {
            // User STARTED dragging gizmo, record BEFORE state
            gizmoEditing = true;
            gizmoBeforeJson = Serialization::SerializeGameObject(go)["TransformComponent"];
        }

        if (!ImGuizmo::IsUsing() && gizmoEditing)
        {
            // User STOPPED dragging, record AFTER & push undo
            gizmoEditing = false;

            LevelEditor* editor = CORE->GetLevelEditor();

            EditorUndoCommand cmd;
            cmd.action = EditorUndoCommand::ActionType::ObjectTransform;
            cmd.entityID = go->GetID();
            cmd.beforeJson = gizmoBeforeJson;
            cmd.afterJson = Serialization::SerializeGameObject(go)["TransformComponent"];

            editor->RecordUndo(cmd);
        }

        if (ImGuizmo::IsUsing())
        {
            vec3 trans{}, scale{}, rot{};
            DecomposeMatrix(mtx, trans.x, trans.y, scale.x, scale.y, rot.x);

            //std::cout << "Decomposed rot: " << rot.x << " " << rot.y << " " << rot.z << "\n";
            // debugging the c# anim image flipping for some reason AAAAAAAAAAAAAAAAAAAAAAA

            // clamps so stuff doesnt get out of hand - only on gizmos bc its likely to be an accident here, will not check for clamps if edited via editor slider
            scale.x = (scale.x < 1.f) ? 1.f : scale.x;
            scale.y = (scale.y < 1.f) ? 1.f : scale.y;

            transform->SetPosition(trans.x, trans.y);
            transform->SetScale(scale.x, scale.y);
            transform->SetRotation(rot.x);
        }
    }
}

void ImGuiSystem::Viewport_RenderGizmoToolbar()
{
    // On screen GUI settings
    viewportPos = ImGui::GetWindowPos();
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowBgAlpha(0.75f);
    ImGui::SetNextWindowPos(ImVec2(viewportPos.x + 15, viewportPos.y + 35), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(58.0f, 260.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin("##Toolbar", nullptr, flags);

    if (ImGui::ImageButton("##Gizmo_Select", ImTextureID(icon_giz_select_ui), icon_size_medium, { 0, 1 }, { 1, 0 },
        (gizmo_type == -1) ? color_icon_highlighted : color_icon_normal, color_icon_bg) || ImGui::IsKeyPressed(ImGuiKey_Q) && isHoveringOverViewport)
    {
        gizmo_type = -1;
    }
    CreateHintText("(Q) Select");
    ImGui::Separator();
    if (ImGui::ImageButton("##Gizmo_Translate", ImTextureID(icon_giz_translate_ui), icon_size_medium, { 0, 1 }, { 1, 0 },
        (gizmo_type == int(ImGuizmo::OPERATION::TRANSLATE)) ? color_icon_highlighted : color_icon_normal, color_icon_bg) || ImGui::IsKeyPressed(ImGuiKey_W) && isHoveringOverViewport)
    {
        gizmo_type = int(ImGuizmo::OPERATION::TRANSLATE);
    }
    CreateHintText("(W) Move");
    ImGui::Separator();
    if (ImGui::ImageButton("##Gizmo_Scale", ImTextureID(icon_giz_scale_ui), icon_size_medium, { 0, 1 }, { 1, 0 },
        (gizmo_type == int(ImGuizmo::OPERATION::SCALE)) ? color_icon_highlighted : color_icon_normal, color_icon_bg) || ImGui::IsKeyPressed(ImGuiKey_E) && isHoveringOverViewport)
    {
        gizmo_type = int(ImGuizmo::OPERATION::SCALE);
    }
    CreateHintText("(E) Scale");
    ImGui::Separator();
    if (ImGui::ImageButton("##Gizmo_Rotate", ImTextureID(icon_giz_rotate_ui), icon_size_medium, { 0, 1 }, { 1, 0 },
        (gizmo_type == int(ImGuizmo::OPERATION::ROTATE)) ? color_icon_highlighted : color_icon_normal, color_icon_bg) || ImGui::IsKeyPressed(ImGuiKey_R) && isHoveringOverViewport)
    {
        gizmo_type = int(ImGuizmo::OPERATION::ROTATE);
    }
    CreateHintText("(R) Rotate");
    ImGui::Separator();
    if (ImGui::ImageButton("##Gizmo_Resize", ImTextureID(icon_giz_resize_ui), icon_size_medium, { 0, 1 }, { 1, 0 },
        (gizmo_type == int(ImGuizmo::OPERATION::BOUNDS)) ? color_icon_highlighted : color_icon_normal, color_icon_bg) || ImGui::IsKeyPressed(ImGuiKey_T) && isHoveringOverViewport)
    {
        gizmo_type = int(ImGuizmo::OPERATION::BOUNDS);
    }
    CreateHintText("(T) Resize");

    ImGui::End();
}

void ImGuiSystem::Viewport_HighlightGO()
{
    // TODO WIP FOR HIGHLIGHTING SELECTED GO, I GOT SO LOST IN THE SAUCE LMAO ITS ALMOST DONE THO I JUST DK HOW TO GET VP MATRIX
    if (!selectedGameObjectID) return;
    auto* transform = GameObjectFactory::Instance().GetGameObject(selectedGameObjectID)->GetComponent<TransformComponent>();
    auto* collider = GameObjectFactory::Instance().GetGameObject(selectedGameObjectID)->GetComponent<ColliderComponent>();

    if (transform)
    {
        float x, y, sx, sy;

        transform->GetPosition(x, y);
        transform->GetScale(sx, sy);
        if (CORE->GetGraphicsSystem())
        {
            CORE->GetGraphicsSystem()->SetEditorSelection(x, y, sx, sy);
        }
        if (collider)
        {
            vec2 colliderScale{ collider->GetSize() };

            if (CORE->GetGraphicsSystem())
            {
                CORE->GetGraphicsSystem()->SetEditorSelection_ColliderOnly(x, y, colliderScale.x, colliderScale.y);
            }
        }
    }

}
#pragma endregion



#pragma region Asset Browser/ Directory Navigator ImGui
/**
 * @brief Ctor - Inits Directory Navigator for asset manager
 */
DirectoryNavigator::DirectoryNavigator() : currentDir(assetPath) {}

/**
 * @brief Renders the asset manager section of the drag and drop
 */
void DirectoryNavigator::RenderImGui()
{
    ImGui::Begin("Project Navigator");
    // Currently project navigator only shows files in assets, not sure if we wanna do anything beyond that

    if (currentDir != fs::path(assetPath))
    {
        if (ImGui::Button("back"))                  // Go back button
        {
            currentDir = currentDir.parent_path();
        }
    }

    // Get amount of columns depending on space available
    int columns = static_cast<int>((ImGui::GetContentRegionAvail().x) / cellSize);
    if (columns <= 0)
        columns = 1;
    ImGui::Columns(columns, 0, false);

    // For each item in the current folder
    for (auto &iter : fs::directory_iterator(currentDir))
    {
        std::string path = iter.path().string();                            // Path we are currently in
        auto relativePath = fs::relative(iter.path(), assetPath);           // Path relative from where we started (assets) to where we are
        std::string fileName = relativePath.filename().string();            // name of the file alone
        std::string extension = iter.path().extension().string();

        // If the current item is a folder
        if (iter.is_directory())
        {
            // TODO add double click? or is single click fine
            if (ImGui::ImageButton(fileName.c_str(), (ImTextureID)(uintptr_t)(icon_folder_ui), icon_size, {0, 1}, {1, 0}))     // Draw image button with folder icon
            {
                currentDir /= iter.path().filename();           // Upon clicking, enter the file path
            }
        }
        // If current item is a file 
        else
        {
            // this is a file, not sure if we wanna do anything yet (when we click)
            // add double click to open in windows file explorer?

            // Json files
            if (extension == ".json")
            {
                if (ImGui::ImageButton(fileName.c_str(), ImTextureID(icon_json_ui), icon_size, { 0, 1 }, { 1, 0 }))
                {
                    // do nothin for now
                }
            }
            // Audio Files
            else if (extension == ".wav" || extension == ".mp3" || extension == ".ogg" || extension == ".flac")
            {
                if (ImGui::ImageButton(fileName.c_str(), ImTextureID(icon_audio_ui), icon_size, { 0, 1 }, { 1, 0 }))
                {
                    // do nothin for now
                }
            }
            // Sprite Files
            else if (extension == ".png" || extension == ".jpg")
            {
                if (ImGui::ImageButton(fileName.c_str(), ImTextureID(icon_sprite_ui), icon_size, { 0, 1 }, { 1, 0 }))
                {
                    // do nothin for now
                }
            }
            // Generic file
            else
            {
                if (ImGui::ImageButton(fileName.c_str(), ImTextureID(icon_file_ui), icon_size, { 0, 1 }, { 1, 0 }))
                {
                    // do nothin for now
                }
            }

            // Can only drag and drop files, not folders
            if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                // Depending on folder its in, send the respective payloadIDs to receive
                // changed from string() to generic_string() because of weird issues with front n back slashes
                if (iter.path().parent_path().filename() == "Scenes")
                {
                    SendDragAndDrop(ScenePayloadID, iter.path().generic_string(), fileName.c_str());            // For scenes files
                }
                else if (iter.path().parent_path().filename() == "Prefabs")
                {
                    SendDragAndDrop(PrefabPayloadID, iter.path().stem().generic_string(), fileName);            // For prefab files
                }
                else if (iter.path().parent_path().filename() == "Audio" || iter.path().parent_path().filename() == "audio"
                    || extension == ".wav" || extension == ".mp3" || extension == ".ogg" || extension == ".flac")
                {
                    SendDragAndDrop(AudioPayloadID, iter.path().generic_string(), fileName.c_str());            // For Audio files
                }
                else if (iter.path().parent_path().filename() == "Sprites" || iter.path().parent_path().filename() == "Icons" || iter.path().parent_path().filename() == "Tiles"
                    || extension == ".png" || extension == ".jpg")
                {
                    SendDragAndDrop(SpritePayloadID, iter.path().generic_string(), fileName.c_str());            // For Sprites/ Spritesheets files
                }
                else if (extension == ".cs")
                {
                    SendDragAndDrop(ScriptPayloadID, iter.path().generic_string(), fileName.c_str());
                }
                else
                {
                    SendDragAndDrop(AssetPayloadID, iter.path().generic_string(), fileName.c_str());            // Anything else considered an general asset
                }
            }
        }

        // Wraps text nicely underneath icon
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + icon_size.x);
        ImGui::TextWrapped(fileName.c_str());
        ImGui::PopTextWrapPos();

        ImGui::NextColumn(); // go next
    }
    ImGui::Columns(1);
    ImGui::End();
}
#pragma endregion

#pragma region Drag&Drop Imgui
/**
 * @brief For drag and drop services. This is the 'sender', takes in a payloadID(specifying what you want to send), type of file and a display text for visual feedback
 *        Overloaded for arithmetic types and strings
 */
// FOR ASSET BROWSER PURPOSES IS JUST FOR STRINGS CONTAINING FILE PATH!
template <typename DataType>
void SendDragAndDrop(std::string const &PayloadID, DataType const &dataToSend, std::string const &displayText)
{
    static_assert(std::is_arithmetic_v<DataType>, "FUNCTION ONLY TAKES STRINGS OR ARITHMETIC TYPES!!!\n");          // ONLY ACCEPT NUMBERS
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))                                           // For drag and drop SENDER
    {
        std::string data{std::to_string(dataToSend)};
        if (!data.empty())
        {
            ImGui::SetDragDropPayload(PayloadID.c_str(), data.c_str(), data.size() + 1, ImGuiCond_Once);            // Send payload
            ImGui::Text(displayText.c_str());                                                                       // Set display text for visual feedback
            ImGui::EndDragDropSource();
        }
        else
        {
            std::cout << "BAD CONVERSION OF " << typeid(DataType).name() << " to std::string!\n";                   // something went wrong with converting data type to string
            return;
        }
    }
}

// overload for strings as data to send, generally same thing
void SendDragAndDrop(std::string const &PayloadID, std::string const &dataToSend, std::string const &displayText)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload(PayloadID.c_str(), dataToSend.c_str(), dataToSend.size() + 1, ImGuiCond_Once);
        ImGui::Text(displayText.c_str());
        ImGui::EndDragDropSource();
    }
}

/**
 * @brief For drag and drop services. This is the 'receiver', takes in a payloadID(specifying what you want to send)
 *        unfortunately cannot be templated bc BeginDragDropTarget only takes in c string...
 */
void ReceiveDragAndDrop(std::string const &PayloadID, std::string &dataToReceive)                                   // For drag and drop RECEIVER
{
    if (ImGui::BeginDragDropTarget())                                                                               // sets to true open hovering
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(PayloadID.c_str()))                          // sets to true when payload accepted
        {
            const char *droppedPath = (const char *)payload->Data;                                                  // can also change the lhs type and the rhs cast if need
            dataToReceive = droppedPath;
            // use droppedPath (load texture,model,etc)
            // droppedPath in this case is just the file path that we sent
            std::cout << "[DragNDrop] successfully Received\n";
        }
        ImGui::EndDragDropTarget();
    }
}

#pragma endregion


/**
 * @brief Renders the top menu bar that has application settings
 */
void TopMenuBar_Render()
{
    if (ImGui::BeginMainMenuBar()) // Begin the top menu toolbar thingy
    {
        // TODO largely WIP
        // File menu
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("[WIP]"))
            {
            }
            if (ImGui::MenuItem("Open"))
            {
            }
            if (ImGui::MenuItem("Save"))
            {
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Exit"))
            {
            }

            ImGui::EndMenu();
        }

        // Edit menu
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("[WIP]"))
            {
            }
            if (ImGui::MenuItem("Preferences"))
            {
                // key binds? Rebinding keys for shortcuts?
            }

            ImGui::EndMenu();
        }

        // Edit menu
        if (ImGui::BeginMenu("Window"))
        {
            if (ImGui::MenuItem("Save Editor Layout"))
            {
                openLayoutSavePopup = true;                     // if clicked, set flags for saving scenes
            }
            if (ImGui::MenuItem("Load Editor Layout"))
            {
                openLayoutLoadPopup = true;                     // if clicked, set flags for loading scenes
            }

            ImGui::Separator();  // Add a separator line

            ImGui::MenuItem("Tilemap Editor", "Ctrl+T", &showTilemapEditor);

            ImGui::EndMenu();
        }

        // Help menu
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("[WIP]"))
            {
            }
            if (ImGui::MenuItem("Naw"))
            {
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // POP UP CALLS HERE
    // Have to separate bc refreshes every frame and got some crashes with end popup, this keeps it up
    if (openLayoutSavePopup)
    {
        ImGui::OpenPopup("Save_Layout");                        // Open respective pop up
        EditorLayout_Refresh();
        openLayoutSavePopup = false;
    }
    if (openLayoutLoadPopup)
    {
        ImGui::OpenPopup("Load_Layout");                        // Open respective pop up
        EditorLayout_Refresh();
        openLayoutLoadPopup = false;
    }
    EditorLayout_SaveNamePopUp();                               // Update popup checks
    EditorLayout_LoadPopUp();
}

/**
 * @brief Save imgui layout to file
 */
void EditorLayout_SaveLayout(std::string const &fileName)
{
    // Saves docking layout to file, future implementation will have this be attached to a button (Prefences -> save editor layout etc)
    if (!fileName.empty())
    {
        // Save to a specific file
        std::string layoutPath = LayoutsFilePath + fileName + ".ini";               // Have to specify this if not it saves as generic file
        ImGui::SaveIniSettingsToDisk(layoutPath.c_str());                           // Save settings to ini file
        std::cout << "Saved window layout to: " << layoutPath << std::endl;
    }
}

/**
 * @brief Load imgui layout
 */
void EditorLayout_LoadLayout(std::string const &fileName)
{
    std::string layoutPath = LayoutsFilePath + fileName;                            // get the directory 
    if (fs::exists(layoutPath))
    {
        ImGui::LoadIniSettingsFromDisk(layoutPath.c_str());                         // Load the settings (gets refreshed in the top of update)

        std::cout << "Layout file at path: " << layoutPath << " loaded\n";
    }
    else
    {
        std::cout << "Layout file at path: " << layoutPath << " not found\n";
    }
}

/**
 * @brief Saving of layout POPUP, checks for illegal file names too
 */
void EditorLayout_SaveNamePopUp()
{
    bool PushPopBgColor = false;
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Save_Layout", nullptr, ImGuiWindowFlags_AlwaysAutoResize))              // modal got that cool grey background
    {
        ImGui::Text("Name");
        if (saveNamePopupClear)
        {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255, 0, 0, 100));                          // red evil scary box for people trying to save weird file names
            PushPopBgColor = true;
        }
        if (ImGui::InputText("##SaveLayoutName", saveLayoutName, sizeof(saveLayoutName)))
        {
            saveNamePopupClear = false;                                                 // after getting illegal warning, type to clear the red box
        }
        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere(-1);

        if (PushPopBgColor)
        {
            ImGui::PopStyleColor();                                                     // pop the push
        }
        if (saveNamePopupClear)
        {
            ImGui::Text("Illegal name! Try something else");                            // if user sets an illegal file name, set flag to true which triggers this text
        }

        if (ImGui::Button("Save"))
        {
            std::string fileName = saveLayoutName;
            // LAMBDA LAMBDA LAMBDA LAMBDA LAMBDA I CANNOT ESCAPE THIS HELL
            fileName.erase(std::remove_if(fileName.begin(), fileName.end(), [](char ch)
                                          { return std::isspace(ch); }),fileName.end());

            // list of illegal chars in filename
            if (fileName.empty() || fileName.find_first_of("<>:\"/\\|?*") != std::string::npos || fileName.back() == ' ' || fileName.back() == '.')
            {
                saveNamePopupClear = true;
            }
            else
            {
                // Save to a specific file
                std::string layoutPath = LayoutsFilePath + fileName + ".ini";
                ImGui::SaveIniSettingsToDisk(layoutPath.c_str());
                std::cout << "Saved window layout to: " << layoutPath << std::endl;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

/**
 * @brief Loading of layout POPUP
 */
void EditorLayout_LoadPopUp()
{
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(800, 600));

    // basically the same as project/ asset navigator
    if (ImGui::BeginPopupModal("Load_Layout", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (fs::directory_iterator(LayoutsFilePath) == fs::directory_iterator())                    // if no layouts are found just show empty with text
        {
            ImGui::Text("No layout files found");
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
            return;
        }
        std::string IntroText{"Layouts Found in: " + LayoutsFilePath};
        ImGui::Text(IntroText.c_str());

        // Adjusting column position based on size of window
        int columns = static_cast<int>((ImGui::GetContentRegionAvail().x) / cellSize);
        if (columns <= 0)
        {
            columns = 1;
        }
        ImGui::Columns(columns, 0, false);

        // Loops through all items with .ini file type
        for (auto &iter : fs::directory_iterator(LayoutsFilePath))
        {
            if (iter.path().extension() != ".ini")
                continue;

            std::string path = iter.path().string();
            auto relativePath = fs::relative(iter.path(), LayoutsFilePath);
            std::string layoutFileName = relativePath.filename().string();

            // each item will have the file icon, on click set the layoutpath to string. Itll then be loaded by the layout loader function
            if (ImGui::ImageButton(layoutFileName.c_str(), (ImTextureID)(uintptr_t)(icon_file_ui), icon_size, {0, 1}, {1, 0}))
            {
                loadLayoutPath = layoutFileName;
                renderLoadedLayout = true;
            }

            // Wraps text nicely underneath icon
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + icon_size.x);
            ImGui::TextWrapped(layoutFileName.c_str());
            ImGui::PopTextWrapPos();

            ImGui::NextColumn(); // go next
        }
        ImGui::Columns(1);

        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

/**
 * @brief Refreshes the vector of layouts, used for loading. OLD CODE!
 */
void EditorLayout_Refresh()
{
    editorLayoutList.clear();

    if (!fs::exists(LayoutsFilePath))
        return;

    // Loops through all .ini files in the folder
    for (const auto &file : fs::directory_iterator(LayoutsFilePath))
    {
        if (file.is_regular_file() && file.path().extension() == ".ini")
        {
            editorLayoutList.push_back(file.path().filename().string());
            std::cout << file.path().string() << std::endl;
        }
    }
}


/**
 * @brief Wrapper to easily create Hints when hovering over Imgui stuff. Call directly under Imgui element! (Text, buttons etc.)
 */
void CreateHintText(std::string const &displayText)
{
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text(displayText.c_str());
        ImGui::EndTooltip();
    }
}

/**
 * @brief Get pointer to selected game object
 */
GameObject* ImGuiSystem::GetSelectedGameObject() const
{
    if (selectedGameObjectID == 0)
        return nullptr;

    return factory.GetGameObject(selectedGameObjectID);
}

/**
 * @brief Render property editor for Selected GOs. Will be empty by default, and then show details if a GO is selected.
 */
void ImGuiSystem::RenderPropertyEditor()
{
    ImGui::SetNextWindowPos(ImVec2(1400, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 900), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Properties##ImGuiSystem", &showPropertyEditor, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }

    GameObject* obj = GetSelectedGameObject();
    if (!obj)
    {
        // if selected go is nullptr
        ImGui::End();
        return;
    }

    ImGui::Text("GameObject: %s", obj->GetName().c_str());
    ImGui::Text("ID: %u", obj->GetID());
    ImGui::Separator();

    // Check box for is active
    // TODO add eye icon like unity on the side in hierarchy to enable/ disable. or right click option
    bool isActive = obj->IsActive();
    if (ImGui::Checkbox("Active", &isActive))
    {
        obj->SetActive(isActive);
    }

    ImGui::Spacing();
    // this is done, just need to attach to a component? so it'll belong to a specific component where users can drag and drop stuff
    //ps.RenderImGui();

    // GO transform screen
    if (auto* transform = obj->GetComponent<TransformComponent>())
    {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SameLine(ImGui::GetWindowWidth() - 30);
            bool removed = ImGui::Button("X##RemoveTransform", ImVec2(20, 20));
            if (removed) {
                // commented out bc no case scenario for GO not having transform
                //obj->RemoveComponent("TransformComponent");
            }
            else
            {
                bool isTransformEnabled = transform->IsEnabled();
                if (ImGui::Checkbox("Component Enabled ##Transform", &isTransformEnabled))
                {
                    transform->SetEnabled(isTransformEnabled);
                }

                // Record Undo Entries - Undo Transform
                LevelEditor* editor = CORE->GetLevelEditor();

                static bool editingTransform = false;
                static json beforeJson;   // JSON before edit

                // Helper lambdas (when on drag)
                auto BeginEdit = [&](void)
                    {
                        if (!editingTransform && ImGui::IsItemActivated())
                        {
                            editingTransform = true;
                            beforeJson = Serialization::SerializeGameObject(obj)["TransformComponent"];
                        }
                    };

                auto EndEdit = [&](void)
                    {
                        if (editingTransform && ImGui::IsItemDeactivatedAfterEdit())
                        {
                            editingTransform = false;

                            EditorUndoCommand cmd;
                            cmd.action = EditorUndoCommand::ActionType::ObjectTransform;
                            cmd.entityID = obj->GetID();
                            cmd.beforeJson = beforeJson;
                            cmd.afterJson = Serialization::SerializeGameObject(obj)["TransformComponent"];

                            editor->RecordUndo(cmd);
                        }
                    };

                // sliders for position, scale, rotation etc

                
                bool isGizmoLocked = lockedGizmosGOs.contains(obj->GetID());
                if (ImGui::Checkbox("Lock Gizmos ##Transform", &isGizmoLocked))
                {
                    if (isGizmoLocked)
                        lockedGizmosGOs.insert(obj->GetID());
                    else
                        lockedGizmosGOs.erase(obj->GetID());
                }
                ImGuizmo::Enable(!isGizmoLocked);

                float pos[2] = { 0.0f, 0.0f };
                transform->GetPosition(pos[0], pos[1]);
                if (ImGui::DragFloat2("Position", pos, 0.5f))
                {
                    transform->SetPosition(pos[0], pos[1]);

                }
                BeginEdit();
                EndEdit();

                float scale[2] = { 1.0f, 1.0f };
                transform->GetScale(scale[0], scale[1]);
                if (ImGui::DragFloat2("Scale", scale, 0.5f))
                {
                    transform->SetScale(scale[0], scale[1]);

                    // Sync collider
                    auto* collider = obj->GetComponent<ColliderComponent>();
                    if (collider) {
                        collider->SetSize(scale[0], scale[1]);
                    }
                }
                BeginEdit();
                EndEdit();

                float rotation = transform->GetRotation();
                if (ImGui::DragFloat("Rotation", &rotation, 0.01f))
                {
                    if (rotation > 2 * pi || rotation < -2 * pi)
                    {
                        rotation = 0;
                    }
                    transform->SetRotation(rotation);

                }
                BeginEdit();
                EndEdit();

                if (ImGui::Button("Press Here to add DND Slot"))
                {
                    transform->AddDragDropSlot("todayisaBADday", PrefabPayloadID);
                }

                if (transform->GetPropertySlots().size())
                {
                    for (auto& slot : transform->GetPropertySlots())
                    {
                        slot.RenderImGui();
                    }
                }
            }
        }

    }
        

    // Render component for GO
    if (auto* render = obj->GetComponent<RenderComponent>())
    {
        if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SameLine(ImGui::GetWindowWidth() - 30);
            bool renderRemove = ImGui::Button("X##RemoveRender", ImVec2(20, 20));
            if (renderRemove)
            {
                obj->RemoveComponent("RenderComponent");
            }
            else
            {

                bool isRenderEnabled = render->IsEnabled();
                if (ImGui::Checkbox("Component Enabled ##Render", &isRenderEnabled))
                {
                    render->SetEnabled(isRenderEnabled);
                }

                ImGui::Text("Shape Type: %s", GetShapeTypeName((int)render->GetShapeType()));
                static bool showTextureDND = false;

                // Show colour picker for GO colour (only if no texture id for custom texture/ sprite)
                if (render->GetTextureID() == 0)
                {
                    vec4 color = render->GetColor();
                    float colorArray[4] = { color.x, color.y, color.z, color.w };
                    if (ImGui::ColorEdit4("Color", colorArray))
                    {
                        render->SetColor(vec4(colorArray[0], colorArray[1], colorArray[2], colorArray[3]));
                    }

                    if (showTextureDND)
                    {
                        std::string draggedSprite{};
                        ImGui::InputText("##DragImageHere", draggedSprite_str, sizeof(draggedSprite_str));
                        if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
                        {
                            draggedSprite = draggedSprite_str;
                        }

                        ReceiveDragAndDrop(SpritePayloadID, draggedSprite);
                        if (!draggedSprite.empty())
                        {
                            std::cout << draggedSprite << std::endl;
                            render->SetTexturePath(draggedSprite);
                            GLuint newTexID = CORE->GetGraphicsSystem()->LoadTexture(draggedSprite.c_str());
                            if (newTexID)
                            {
                                render->SetColor(1.0f);                                 // removes colour and makes way for loading image
                                render->SetTextureID(newTexID);
                                strcpy(draggedSprite_str, "<Drag Image Here>");                   // reset
                                std::cout << "[Properties] New Texture loaded successfully!" << std::endl;
                            }
                            else
                            {
                                std::cout << "[Properties] Failed to load texture!" << std::endl;
                            }
                        }
                    }

                    // placed below here for cleaner transition when pressing the button
                    if (!showTextureDND && ImGui::Button("Add Texture"))
                    {
                        render->SetTextureID(GLuint{});
                        showTextureDND = true;
                    }
                }
                else
                {
                    ImGui::Text("Texture ID: %u", render->GetTextureID());
                    showTextureDND = false;
                    if (ImGui::Button("Remove Existing Texture"))
                    {
                        render->SetTextureID(GLuint{});
                        render->SetColor({ 1,0,1,1 });                    // set to pink to make it obvious if texture missing
                    }
                }
                int goRenderLayer{ render->GetLayer() };
                ImGui::Text("Rendering Layer");
                ImGui::SetNextItemWidth(100.0f);
                if (ImGui::InputInt("##Rendering Layer", &goRenderLayer))
                {
                    render->SetLayer(goRenderLayer);
                }
                CreateHintText("Ascending order for topmost rendering position");
            }
        }
    }

    // Physics component
    if (auto* physics = obj->GetComponent<PhysicsComponent>())
    {
        if (ImGui::CollapsingHeader("Physics"))
        {
            ImGui::SameLine(ImGui::GetWindowWidth() - 30);
            bool physicsRemove = ImGui::Button("X##RemovePhysic", ImVec2(20, 20));

            if (physicsRemove)
            {
                obj->RemoveComponent("PhysicsComponent");
            }
            else
            {
                // Physics sliders with respective functions
                bool isPhysEnabled = physics->IsEnabled();
                if (ImGui::Checkbox("Phsyics Enabled##Physics", &isPhysEnabled))
                {
                    physics->SetEnabled(isPhysEnabled);
                }

                float vel[2] = { 0.0f, 0.0f };
                physics->GetVelocity(vel[0], vel[1]);
                if (ImGui::DragFloat2("Velocity", vel, 0.1f))
                {
                    physics->SetVelocity(vel[0], vel[1]);
                }

                bool isStatic = physics->IsStatic();
                if (ImGui::Checkbox("Static", &isStatic))
                {
                    physics->SetStatic(isStatic);
                }

                bool hasGravity = physics->useGravity;
                if (ImGui::Checkbox("Gravity", &hasGravity))
                {
                    physics->SetGravity(hasGravity);
                }

                ImGui::Text("Mass: %.2f", physics->GetMass());
                ImGui::Text("Grounded: %s", physics->IsGrounded() ? "Yes" : "No");
            }
        }
    }

    // Collider component
    if (auto* collider = obj->GetComponent<ColliderComponent>())
    {
        if (ImGui::CollapsingHeader("Collider"))
        {
            ImGui::SameLine(ImGui::GetWindowWidth() - 30);
            bool colliderRemove = ImGui::Button("X##RemoveCollider", ImVec2(20, 20));
            if (colliderRemove)
            {
                obj->RemoveComponent("ColliderComponent");
            }
            else
            {
                // Editable Collider Type Dropdown
                bool isColEnabled = collider->IsEnabled();
                if (ImGui::Checkbox("Component Enabled##Collider", &isColEnabled))
                {
                    collider->SetEnabled(isColEnabled);
                }

                const char* colliderTypes[] = { "Square", "Circle" };
                int currentType = (collider->GetType() == ColliderComponent::Type::Circle) ? 1 : 0;

                if (ImGui::Combo("Type", &currentType, colliderTypes, 2))
                {
                    ColliderComponent::Type newType = (currentType == 1)
                        ? ColliderComponent::Type::Circle
                        : ColliderComponent::Type::Square;
                    collider->setColliShape(newType);
                    std::cout << "[Property Editor] Changed collider type to "
                        << colliderTypes[currentType] << std::endl;
                }

                // Display dimensions
                if (collider->GetType() == ColliderComponent::Type::Circle)
                {
                    Circle circle = collider->GetCircle();
                    ImGui::Text("Radius: %.2f", circle.radius);
                }
                else
                {
                    AABB aabb = collider->GetAABB();
                    float width = aabb.max.x - aabb.min.x;
                    float height = aabb.max.y - aabb.min.y;
                    ImGui::Text("Width: %.2f", width);
                    ImGui::Text("Height: %.2f", height);
                }

                // Editable Size (In case we need it)
                if (collider->GetType() == ColliderComponent::Type::Square)
                {
                    AABB aabb = collider->GetAABB();
                    float width = aabb.max.x - aabb.min.x;
                    float height = aabb.max.y - aabb.min.y;

                    float newSize[2] = { width, height };
                    if (ImGui::DragFloat2("Size", newSize, 1.0f, 1.0f, 5000.0f))
                    {
                        collider->SetSize(newSize[0], newSize[1]);
                        std::cout << "[Property Editor] Changed collider size to "
                            << newSize[0] << "x" << newSize[1] << std::endl;
                    }
                }
                else if (collider->GetType() == ColliderComponent::Type::Circle)
                {
                    Circle circle = collider->GetCircle();
                    float radius = circle.radius;

                    if (ImGui::DragFloat("Radius", &radius, 1.0f, 1.0f, 5000.0f))
                    {
                        collider->SetSize(radius * 2.0f, radius * 2.0f);
                        std::cout << "[Property Editor] Changed circle radius to "
                            << radius << std::endl;
                    }
                }


                // Editable Collision Layer Dropdown
                const char* layerNames[] = { "Player", "Ground", "Wall", "Enemy", "Platform", "Default" };
                int currentLayer = static_cast<int>(collider->GetLayer());

                if (ImGui::Combo("Layer", &currentLayer, layerNames, 6))
                {
                    collider->SetLayer(static_cast<CollisionLayer>(currentLayer));
                    std::cout << "[Property Editor] Changed collision layer to "
                        << layerNames[currentLayer] << std::endl;
                }
            }
        }
    }

    if (auto* events = obj->GetComponent<EventComponent>())
    {
        // Add event into GameObjects
        RenderEventComponent(events, obj);
    }

// Script component
if (auto* script = obj->GetComponent<ScriptComponent>())
{
    if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SameLine(ImGui::GetWindowWidth() - 30);
        bool colliderRemove = ImGui::Button("X##RemoveScript", ImVec2(20, 20));
        if (colliderRemove)
        {
            obj->RemoveComponent("ScriptComponent");
        }
        else
        {
            bool isScriptEnabled = script->IsEnabled();
            if (ImGui::Checkbox("Component Enabled ##Script", &isScriptEnabled))
            {
                script->SetEnabled(isScriptEnabled);
            }
            if (ImGui::Button("Add Script Slot"))
            {
                script->AddDragDropSlot("Drag Script Here", ScriptPayloadID);
            }

            if (script->GetPropertySlots().size())
            {
                for (auto& slot : script->GetPropertySlots())
                {
                    slot.RenderImGui();
                }
            }
        }
        ImGui::Text("Attached Scripts:");

        auto* scriptSystem = CORE->GetScriptSystem();
        if (scriptSystem)
        {
            auto* instances = scriptSystem->GetScriptInstances(obj);
            if (instances)
            {
                for (auto& instance : *instances)
                {
                    const char* className = mono_class_get_name(instance.scriptClass->monoClass);
                    if (ImGui::TreeNode(className))
                    {
                        auto fields = scriptSystem->GetPublicFields(instance);
                        for (auto& field : fields)
                        {
                            if (field.type == ScriptFieldType::Float)
                            {
                                float val = std::get<float>(field.value);
                                if (ImGui::DragFloat(field.name.c_str(), &val, 0.1f))
                                {
                                    scriptSystem->SetPublicField(instance, field.name, val);
                                }
                            }
                            else if (field.type == ScriptFieldType::Int)
                            {
                                int val = std::get<int>(field.value);
                                if (ImGui::DragInt(field.name.c_str(), &val))
                                {
                                    scriptSystem->SetPublicField(instance, field.name, val);
                                }
                            }
                            else if (field.type == ScriptFieldType::Bool)
                            {
                                bool val = std::get<bool>(field.value);
                                if (ImGui::Checkbox(field.name.c_str(), &val))
                                {
                                    scriptSystem->SetPublicField(instance, field.name, val);
                                }
                            }
                            else if (field.type == ScriptFieldType::String)
                            {
                                std::string val = std::get<std::string>(field.value);
                                char buffer[256];
                                strncpy(buffer, val.c_str(), sizeof(buffer));
                                buffer[sizeof(buffer) - 1] = 0;
                                if (ImGui::InputText(field.name.c_str(), buffer, sizeof(buffer)))
                                {
                                    scriptSystem->SetPublicField(instance, field.name, std::string(buffer));
                                }
                            }
                        }
                        ImGui::TreePop();
                    }
                }
            }
        }
    }
}

    // Add Component Button
    ImVec2 space = ImGui::GetContentRegionAvail();
    ImVec2 buttonSize{ 100,25 };
    ImGui::SetCursorPosX((space.x - buttonSize.x)/2);
    if (ImGui::Button("Add Component", buttonSize))
    {
        ImGui::OpenPopup("Add_Component");
    }
    if (ImGui::BeginPopup("Add_Component"))
    {
        {
            for (auto& comp : AllComponents)
            {
                // can use to sort by component type in the future when we got more components, and can add explanation text
                //ComponentType type      { std::get<0>(comp) };
                std::string& name       { std::get<1>(comp) };
                ImTextureID& textureID  { std::get<2>(comp) };
                auto& onclick           { std::get<3>(comp) };

                std::string idname = "##iconItem" + name;
                bool onClick = ImGui::Selectable(idname.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);
                ImGui::SameLine();

                // Draw the icon
                ImGui::Image(textureID, icon_size_smallest);

                ImGui::SameLine(40); // spacing between icon and label

                ImGui::Text(name.c_str());
                CreateHintText("Explanation text here.");

                if (onClick)
                {
                    // clicked 
                    onclick(GetSelectedGameObject());
                }
            }
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void ImGuiSystem::RenderEventComponent(EventComponent* ec, GameObject* obj)
{
    (void)obj; // not used yet

    // --- Begin UI ---
    if (ImGui::CollapsingHeader("Event Component", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Rules: %zu", ec->rules.size());
        ImGui::Separator();

        auto& allObjects = factory.GetAllGameObjects();
        std::vector<const char*> objectNames;
        std::vector<int> objectIds;

        objectNames.push_back("Any GameObjects"); // Trigger by Any Objects
        objectIds.push_back(-1);

        for (auto& o : allObjects)
        {
            objectNames.push_back(o->GetName().c_str());
            objectIds.push_back(o->GetID());
        }

        int index = 0;
        for (auto& rule : ec->rules)
        {
            ImGui::PushID(index);

            if (ImGui::TreeNodeEx("Rule", ImGuiTreeNodeFlags_DefaultOpen))
            {
                static const char* eventNames[] = {
                    "None",
                    "OnEnable",
                    "OnDisable",
                    "OnDestroy",
                    "OnCollisionEnter",
                    "OnCollisionStay",
                    "OnCollisionExit",
                };

                int evIndex = (int)rule.eventType;
                if (ImGui::Combo("Event", &evIndex, eventNames, IM_ARRAYSIZE(eventNames)))
                    rule.eventType = (EventType)evIndex;

                int currentIndex = -1;

                // find index of current ID
                for (int i = 0; i < (int)objectIds.size(); i++)
                {
                    if (objectIds[i] == rule.targetObjectId)
                        currentIndex = i;
                }

                if (ImGui::Combo("Trigger By", &currentIndex,
                    objectNames.data(), (int)objectNames.size()))
                {
                    rule.targetObjectId = objectIds[currentIndex];
                }

                static const char* actionNames[] = {
                    "None",
                    "EnableObject",
                    "DisableObject",
                    "DestroyObject",
                    "PlaySound",
                    "StopSound",
                    "Respawn",
                };

                int acIndex = (int)rule.action;
                if (ImGui::Combo("Action", &acIndex, actionNames, IM_ARRAYSIZE(actionNames)))
                    rule.action = (ActionType)acIndex;

                switch (rule.action)
                {
                case ActionType::PlaySound:
                {
                    auto* audioMgr = CORE->GetAudioManager();

                    // Fallback: If no sounds loaded, load global audio.json right now
                    if (audioMgr->GetSoundDatabase().empty())
                    {
                        std::string fallbackPath = "../Assets/Audio/audio.json";
                        std::ifstream f(fallbackPath);

                        if (f.is_open())
                        {
                            json audioRoot;
                            f >> audioRoot;

                            json wrapper;
                            wrapper["AudioManager"] = audioRoot["AudioManager"];

                            Serialization::DeserializeAudioManager(wrapper, audioMgr);
                        }
                        else
                        {
                            ImGui::TextColored(ImVec4(1, 0.2f, 0.2f, 1),
                                "ERROR: Could not open audio.json fallback.");
                            break;
                        }
                    }

                    // After fallback, soundDatabase MUST have entries
                    auto& soundDB = audioMgr->GetSoundDatabase();

                    if (soundDB.empty())
                    {
                        ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1),
                            "No sounds loaded (even after fallback).");
                        break;
                    }

                    std::vector<const char*> names;
                    names.reserve(soundDB.size());

                    for (auto& entry : soundDB)
                        names.push_back(entry.first.c_str());

                    // current selection
                    int idx = 0;
                    int i = 0;
                    for (auto& pair : soundDB)
                    {
                        if (pair.first == rule.stringValue)
                        {
                            idx = i;
                            break;
                        }
                        i++;
                    }

                    if (ImGui::Combo("Sound", &idx, names.data(), (int)names.size()))
                    {
                        int j = 0;
                        for (auto& pair : soundDB)
                        {
                            if (j == idx)
                            {
                                rule.stringValue = pair.first;   // ONLY save name
                                break;
                            }
                            j++;
                        }
                    }
                }
                break;
               
                case ActionType::StopSound: {                

                    ImGui::Text("Stop this GameObject Sound.");

                    const std::string& cur = obj->GetCurrentlyPlayingSound();

                    if (cur.empty())
                    {
                        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1),
                            "No sound currently playing.");
                    }
                }
                break;

                default:
                    break;
                }

                ImGui::Spacing();

                if (ImGui::Button("Remove Rule"))
                {
                    ec->rules.erase(ec->rules.begin() + index);
                    ImGui::TreePop();
                    ImGui::PopID();
                    break; // exit loop safely
                }

                ImGui::TreePop();
            }

            ImGui::PopID();
            index++;
        }

        ImGui::Separator();

        if (ImGui::Button("Add Rule"))
        {
            EventRule newRule;
            ec->rules.push_back(newRule);
        }
    }
}

//  generate unique sound name if one already exists
static std::string MakeUniqueSoundName(const std::string& baseName,
    const std::map<std::string, SoundData>& db)
{
    // if base not used, just use it
    if (db.find(baseName) == db.end())
        return baseName;

    int counter = 1;
    std::string candidate;
    do
    {
        candidate = baseName + "_" + std::to_string(counter++);
    } while (db.find(candidate) != db.end());

    return candidate;
}

#pragma region Audio Assets ImGui
// Renders the Audio Assets panel
void AudioAssetsPanel_Render()
{
    // use the flag to control visibility
    if (!showAudioAssetsPanel)
        return;

    bool open = true;

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Audio Manager", &open))
    {
        // even if collapsed still need to handle closing
        if (!open)
        {
            showAudioAssetsPanel = false;
        }
            
        ImGui::End();
        return;
    }

    if (!open)
    {
        showAudioAssetsPanel = false;
        ImGui::End();
        return;
    }

    AudioManager* audioMgr = CORE->GetAudioManager();
    if (!audioMgr)
    {
        ImGui::TextDisabled("AudioManager not available");
        ImGui::End();
        return;
    }

    ImGui::TextWrapped("Drag .wav / .mp3 / .ogg files from Project Navigator into this window to add them to the game");
    ImGui::Separator();

    // global controls
    if (ImGui::Button("Stop All Sounds"))
    {
        audioMgr->StopAllSounds();
    }
    ImGui::SameLine();

    // Fade controls
    static float fadeDuration = 1.0f;
    static int selectedCategory = 0;
    static const char* categories[] = { "music", "sfx", "voice", "ambient" };
    static float fadeTargetVolume = 1.0f;

    ImGui::Separator();
    ImGui::Text("Fade Controls:");

    ImGui::PushItemWidth(100.0f);
    ImGui::InputFloat("Fade Duration (s)", &fadeDuration, 0.1f, 1.0f, "%.1f");
    if (fadeDuration < 0.1f) fadeDuration = 0.1f;
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(100.0f);
    ImGui::Combo("Category", &selectedCategory, categories, 4);
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(100.0f);
    ImGui::SliderFloat("Target Volume", &fadeTargetVolume, 0.0f, 1.0f, "%.1f");
    ImGui::PopItemWidth();

    if (ImGui::Button("Fade In"))
    {
        audioMgr->FadeInCategory(categories[selectedCategory], fadeDuration, fadeTargetVolume);
        std::cout << "[Audio] Fading in " << categories[selectedCategory]
                  << " over " << fadeDuration << "s to volume " << fadeTargetVolume << std::endl;
    }
    ImGui::SameLine();

    if (ImGui::Button("Fade Out"))
    {
        audioMgr->FadeOutCategory(categories[selectedCategory], fadeDuration);
        std::cout << "[Audio] Fading out " << categories[selectedCategory]
                  << " over " << fadeDuration << "s" << std::endl;
    }
    ImGui::SameLine();

    if (ImGui::Button("Fade Out All"))
    {
        audioMgr->FadeOutAll(fadeDuration);
        std::cout << "[Audio] Fading out all categories over " << fadeDuration << "s" << std::endl;
    }

    ImGui::Separator();
    ImGui::Text("Drop zone:");

    ImVec2 dropSize = ImVec2(ImGui::GetContentRegionAvail().x, 120.0f);
    ImGui::Dummy(dropSize);     // big area as the target

    // draw an outline so it's visible... to change ltr on
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    dl->AddRect(min, max, IM_COL32(200, 200, 200, 255));

    if (ImGui::BeginDragDropTarget())
    {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(AudioPayloadID.c_str());
        if (!payload)
            payload = ImGui::AcceptDragDropPayload(AssetPayloadID.c_str());
        if (!payload)
            payload = ImGui::AcceptDragDropPayload(ScenePayloadID.c_str());
        if (!payload)
            payload = ImGui::AcceptDragDropPayload(PrefabPayloadID.c_str());
        if (!payload)
            payload = ImGui::AcceptDragDropPayload(HierarchyPayloadID.c_str());

        if (payload)
        {
            const char* droppedPath = (const char*)payload->Data;
            std::string droppedPathStr = droppedPath;

            fs::path p(droppedPathStr);
            std::string ext = p.extension().string();

            // normalise extension to lowercase
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            if (ext == ".wav" || ext == ".mp3" || ext == ".ogg")
            {
                // get current database so we can avoid name clashes
                auto& db = audioMgr->GetSoundDatabase();

                std::string baseName = p.stem().string();                   // "bgm"
                std::string uniqueName = MakeUniqueSoundName(baseName, db); // "bgm1"

                SoundData sound{};
                sound.filepath = droppedPathStr;
                sound.name = p.stem().string();
                sound.loop = false;
                sound.volume = 1.0f;
                sound.pitch = 1.0f;
                sound.category = "sfx";

                audioMgr->AddSound(sound);

                if (audioMgr->LoadSound(sound))
                {
                    std::cout << "[Audio Assets] Added and loaded sound: " << sound.name << std::endl;
                }
                else
                {
                    std::cout << "[Audio Assets] Failed to load sound: " << sound.name << std::endl;
                }
            }
            else
            {
                // trigger imgui error popup
                g_audioErrorMessage = "Unsupported audio file type: " + ext + "\n\nSupported formats: .wav, .mp3, .ogg";
                g_showAudioErrorPopup = true;
                std::cout << "[Audio Assets] Unsupported audio type: " << ext << std::endl;
            }
        }
        ImGui::EndDragDropTarget();
    }

    std::string soundToRemove;
    // show current audio database
    auto& db = audioMgr->GetSoundDatabase();
    if (db.empty())
    {
        ImGui::TextDisabled("No sounds added yet");
    }
    else
    {
        if (ImGui::BeginTable("AudioAssetsTable", 6,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Category");
            ImGui::TableSetupColumn("Loop");
            ImGui::TableSetupColumn("Volume");
            ImGui::TableSetupColumn("Pitch");
            ImGui::TableSetupColumn("Actions");
            ImGui::TableHeadersRow();

            for (auto& pair : db)
            {
                SoundData& sound = pair.second;

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(sound.name.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(sound.category.c_str());

                // loop
                ImGui::TableSetColumnIndex(2);
                {
                    bool loop = sound.loop;
                    if (ImGui::Checkbox(("##loop_" + sound.name).c_str(), &loop))
                    {
                        sound.loop = loop;
                        audioMgr->ReloadSound(sound.name);

                        std::cout << "[Audio Assets] Updated loop for " << sound.name << " = " << (sound.loop ? "true" : "false") << std::endl;
                    }
                }

                // volume
                ImGui::TableSetColumnIndex(3);
                {
                    float volume = sound.volume;
                    ImGui::SetNextItemWidth(-60.0f);
                    if (ImGui::SliderFloat(("##vol_" + sound.name).c_str(),
                        &volume, 0.0f, 1.0f))
                    {
                        volume = std::round(volume * 10.0f) / 10.0f;
                        sound.volume = volume;
                        audioMgr->SetSoundVolume(sound.name, volume);
                        std::cout << "[Audio Assets] Updated volume for " << sound.name << " = " << sound.volume << std::endl;
                    }
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(50.0f);
                    float volDisplay = sound.volume;
                    ImGui::InputFloat(("##vol_val_" + sound.name).c_str(), &volDisplay, 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_ReadOnly);
                }

                //pitch
                ImGui::TableSetColumnIndex(4);
                {
                    float pitch = (sound.pitch <= 0.0f ? 1.0f : sound.pitch);
                    ImGui::SetNextItemWidth(-60.0f);
                    if (ImGui::SliderFloat(("##pitch_" + sound.name).c_str(),
                        &pitch, 0.5f, 2.0f, "%.2f"))
                    {
                        pitch = std::round(pitch * 10.0f) / 10.0f;
                        sound.pitch = pitch;
                        audioMgr->SetSoundPitch(sound.name, pitch);
                        std::cout << "[Audio Assets] Updated pitch for " << sound.name << " = " << sound.pitch << std::endl;
                    }
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(50.0f);
                    float pitchDisplay = sound.pitch;
                    ImGui::InputFloat(("#pitch_val_" + sound.name).c_str(), &pitchDisplay, 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_ReadOnly);
                }
                /* actions */
                // play button
                ImGui::TableSetColumnIndex(5);
                std::string playLabel = "Play##" + sound.name;
                if (ImGui::SmallButton(playLabel.c_str()))
                {
                    audioMgr->PlaySound(sound.name);
                }
                ImGui::SameLine();

                // stop button
                std::string stopLabel = "Stop##" + sound.name;
                if (ImGui::SmallButton(stopLabel.c_str()))
                {
                    audioMgr->StopSound(sound.name + "_channel");
                }

                // remove sound button
                std::string removeLabel = "Remove##" + sound.name;
                if (ImGui::SmallButton(removeLabel.c_str()))
                {
                    soundToRemove = sound.name;
                }
            }

            ImGui::EndTable();
        }
        if (!soundToRemove.empty())
        {
            audioMgr->RemoveSound(soundToRemove);
        }
    }

    // audio file error popup centered and top most
    if (g_showAudioErrorPopup)
    {
        ImGui::OpenPopup("Audio File Error");
        g_showAudioErrorPopup = false; // only open once
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 center = viewport->GetCenter();
    ImGui::SetNextWindowSize(ImVec2(320, 180), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Audio File Error", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse))
    {
        ImGui::TextWrapped("%s", g_audioErrorMessage.c_str());
        ImGui::Separator();

        ImGui::Dummy(ImVec2(0.0f, 4.0f));
        ImGui::SetCursorPosX(
            ImGui::GetCursorPosX()
            + (ImGui::GetContentRegionAvail().x - 120.0f) * 0.5f
        );
        if (ImGui::Button("OK", ImVec2(120.0f, 0.0f)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::End();

    if (!open)
    { 
        showAudioAssetsPanel = false;
    }
}
#pragma endregion

#pragma region Animation Previewer ImGui

void ImGuiSystem::RenderAnimationPreviewer()
{
    // Draft (file) preview state
    static std::string draftPath;
    static GLuint draftTexID = 0;
    static int draftRows = 1, draftCols = 1;

    // Playback - linear frame index so it can clamp to a range
    static int   draftFrameLinear = 0;
    static float draftTimer = 0.0f;
    static bool  isPlayingDraft = false;

    // GameObject (left) preview state
    static int   currentRowIdx = 0, currentColIdx = 0;
    static float currentTimer = 0.0f;
    static bool  isPlayingCurrent = false;

    // Clip authoring state (saved in editor as "draft clips")
    struct DraftClip
    {
        std::string name;
        int startR = 0, startC = 0;
        int endR = 0, endC = 0;
        float fps = 12.0f;
        bool loop = true;
    };

    static std::vector<DraftClip> draftClips;
    // -1 means "<New / Unsaved>"
    static int selectedDraftClip = -1;

    // Editable inputs
    static char  actionName[64] = "Idle";
    static int   startR = 0, startC = 0;
    static int   endR = 0, endC = 0;
    static float actionFPS = 12.0f;
    static bool  actionLoop = true;

    // Get context
    GameObject* obj = factory.GetGameObject(selectedGameObjectID);
    Spritesheet* ss = obj ? obj->GetComponent<Spritesheet>() : nullptr;
    bool hasExistingSS = (ss != nullptr);

    ImGui::Begin("Animation Preview");
    isAnimPreviewFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);// || ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

    // Mode switches (From File vs From Game Object)
    static bool fromFile = true;
    static bool fromGO = false;

    if (ImGui::Checkbox("From File", &fromFile)) {
        if (fromFile) fromGO = false;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("From Game Object", &fromGO)) {
        if (fromGO) fromFile = false;
    }

    ImGui::Separator();

    // One column for file preview, two columns when comparing with a GameObject.
    int columnCount = (fromGO && obj) ? 2 : 1;
    ImGui::Columns(columnCount, "PreviewSplit", true);

    // LEFT COLUMN: Current GameObject preview
    if (fromGO)
    {
        if (!obj)
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "PLEASE SELECT A GAMEOBJECT");
        }
        else
        {
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "CURRENT GAMEOBJECT");

            if (hasExistingSS)
            {
                // Get current grid from the object's spritesheet
                const int goRows = std::max(1, ss->GetRows());
                const int goCols = std::max(1, ss->GetCols());
                float uW = 1.0f / (float)goCols;
                float vH = 1.0f / (float)goRows;

                // Sync on first view
                if (!isPlayingCurrent && currentRowIdx == 0 && currentColIdx == 0)
                {
                    currentRowIdx = ss->GetCurrentRow();
                    currentColIdx = ss->GetCurrentCol();
                }

                // Simple stepping for left preview
                if (isPlayingCurrent)
                {
                    currentTimer += ImGui::GetIO().DeltaTime;
                    if (currentTimer > 0.12f)
                    {
                        currentColIdx = (currentColIdx + 1) % goCols;
                        currentTimer = 0.0f;
                    }
                }

                // Compute UVs for current cell
                ImVec2 uv0 = ImVec2(currentColIdx * uW, (currentRowIdx + 1) * vH);
                ImVec2 uv1 = ImVec2((currentColIdx + 1) * uW, currentRowIdx * vH);

                ImGui::Image((void*)(intptr_t)ss->GetTextureID(),
                    ImVec2(180, 180), uv0, uv1,
                    ImVec4(1, 1, 1, 1),
                    ImVec4(1, 1, 1, 0.2f));

                // Action dropdown
                {
                    const auto& reg = ss->GetAnimationRegistry();
                    std::vector<const char*> names;
                    names.reserve(reg.size());
                    for (const auto& kv : reg) names.push_back(kv.first.c_str());

                    if (!names.empty())
                    {
                        // Find current animation name inside the list
                        int curIdx = 0;
                        std::string curName = ss->GetCurrentAnimationName();
                        for (int i = 0; i < (int)names.size(); ++i)
                        {
                            if (curName == names[i]) { curIdx = i; break; }
                        }

                        // Selecting from dropdown calls PlayAnimation
                        if (ImGui::Combo("Action##GO", &curIdx, names.data(), (int)names.size()))
                        {
                            ss->PlayAnimation(names[curIdx]);
                            currentRowIdx = ss->GetCurrentRow();
                            currentColIdx = ss->GetCurrentCol();
                            currentTimer = 0.0f;
                        }
                    }
                    else
                    {
                        ImGui::TextDisabled("No actions registered.");
                    }
                }

                if (ImGui::Button(isPlayingCurrent ? "STOP##Cur" : "PLAY##Cur", ImVec2(180, 0)))
                    isPlayingCurrent = !isPlayingCurrent;
            }
            else
            {
                ImGui::BeginChild("NoExisting", ImVec2(180, 180), true);
                ImGui::TextWrapped("\n\n NO EXISTING\n SPRITESHEET");
                ImGui::EndChild();
            }

            ImGui::Text("GAMEOBJECT: %s", obj->GetName().c_str());
            ImGui::Spacing();
        }
        ImGui::NextColumn();
    }

    // RIGHT COLUMN: Draft preview + authoring
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), draftTexID ? "EDIT / PREVIEW" : "DRAG SPRITE HERE");

    std::string dropped;

    // Helper - stop playing + reset timers
    auto ResetDraftPlayback = [&]()
        {
            isPlayingDraft = false;
            draftTimer = 0.0f;
            draftFrameLinear = 0;
        };

    // If have a draft texture loaded, show preview + play controls
    if (draftTexID != 0)
    {
        // Active playback range
        int activeStartR = startR, activeStartC = startC;
        int activeEndR = endR, activeEndC = endC;
        float activeFPS = std::max(0.001f, actionFPS);

        draftRows = std::max(1, draftRows);
        draftCols = std::max(1, draftCols);

        // Clamp the range within grid bounds
        activeStartR = std::clamp(activeStartR, 0, draftRows - 1);
        activeEndR = std::clamp(activeEndR, 0, draftRows - 1);
        activeStartC = std::clamp(activeStartC, 0, draftCols - 1);
        activeEndC = std::clamp(activeEndC, 0, draftCols - 1);

        // Convert to linear indices
        int startIdx = (activeStartR * draftCols) + activeStartC;
        int endIdx = (activeEndR * draftCols) + activeEndC;
        if (endIdx < startIdx) std::swap(startIdx, endIdx);

        if (draftFrameLinear < startIdx || draftFrameLinear > endIdx)
            draftFrameLinear = startIdx;

        // Draft playback (range-based)
        if (isPlayingDraft)
        {
            draftTimer += ImGui::GetIO().DeltaTime;
            const float frameDur = 1.0f / activeFPS;
            if (draftTimer >= frameDur)
            {
                draftTimer -= frameDur;
                draftFrameLinear++;

                // Loop within the chosen range
                if (draftFrameLinear > endIdx)
                {
                    if (actionLoop) draftFrameLinear = startIdx;
                    else { draftFrameLinear = endIdx; isPlayingDraft = false; }
                }
            }
        }

        // Convert linear index back for UV sampling
        int showRow = draftFrameLinear / draftCols;
        int showCol = draftFrameLinear % draftCols;

        float duW = 1.0f / (float)draftCols;
        float dvH = 1.0f / (float)draftRows;

        // Draw preview
        ImGui::BeginGroup();
        ImGui::Image((void*)(intptr_t)draftTexID, ImVec2(180, 180),
            ImVec2(showCol * duW, (showRow * dvH) + dvH),
            ImVec2((showCol + 1) * duW, showRow * dvH),
            ImVec4(1, 1, 1, 1),
            ImVec4(1, 1, 0, 1));
        ImGui::EndGroup();

        ReceiveDragAndDrop(SpritePayloadID, dropped);

        if (ImGui::Button(isPlayingDraft ? "STOP PREVIEW##Drf" : "PLAY PREVIEW##Drf", ImVec2(180, 0)))
        {
            isPlayingDraft = !isPlayingDraft;
            if (isPlayingDraft) draftTimer = 0.0f;
        }

        ImGui::NewLine();
        ImGui::Text("FILE: %s", draftPath.substr(draftPath.find_last_of("\\/") + 1).c_str());
    }
    else
    {
        ImGui::BeginChild("DropZone", ImVec2(180, 180), true);
        ImGui::TextDisabled("\n\n   DRAG SPRITE HERE\n   TO START EDITING");
        ImGui::EndChild();

        ReceiveDragAndDrop(SpritePayloadID, dropped);
    }

    // Drag drop load
    if (!dropped.empty() && SupportedImageFile(dropped))
    {
        draftPath = dropped;
        draftTexID = GraphicsSystem::LoadTexture(dropped.c_str());

        // Reset
        draftRows = 1;
        draftCols = 1;
        startR = startC = endR = endC = 0;
        actionFPS = 12.0f;
        actionLoop = true;
        std::strncpy(actionName, "Idle", sizeof(actionName) - 1);
        actionName[sizeof(actionName) - 1] = '\0';

        // Start in "<New / Unsaved>" mode
        selectedDraftClip = -1;
        ResetDraftPlayback();
    }

    // Controls
    if (draftTexID)
    {
        // Grid configuration
        ImGui::NewLine();
        ImGui::Separator();
        ImGui::Text("GRID CONFIGURATION:");
        ImGui::PushItemWidth(80);
        bool gridChanged = false;
        if (ImGui::InputInt("ROWS", &draftRows)) { draftRows = std::max(1, draftRows); gridChanged = true; }
        ImGui::SameLine();
        if (ImGui::InputInt("COLS", &draftCols)) { draftCols = std::max(1, draftCols); gridChanged = true; }
        ImGui::PopItemWidth();
        if (gridChanged) ResetDraftPlayback();

        // Draft clip dropdown
        ImGui::NewLine();
        ImGui::Separator();
        ImGui::Text("DRAFT CLIPS:");

        std::vector<std::string> clipLabels;
        clipLabels.reserve(draftClips.size() + 1);
        clipLabels.push_back("<New / Unsaved>");
        for (auto& c : draftClips) clipLabels.push_back(c.name.empty() ? "<unnamed>" : c.name);

        std::vector<const char*> clipLabelPtrs;
        clipLabelPtrs.reserve(clipLabels.size());
        for (auto& s : clipLabels) clipLabelPtrs.push_back(s.c_str());

        // Selecting a clip loads its data into editable inputs
        int comboIndex = selectedDraftClip + 1;
        if (ImGui::Combo("Draft Clip", &comboIndex, clipLabelPtrs.data(), (int)clipLabelPtrs.size()))
        {
            selectedDraftClip = comboIndex - 1;
            ResetDraftPlayback();

            if (selectedDraftClip >= 0 && selectedDraftClip < (int)draftClips.size())
            {
                const auto& c = draftClips[selectedDraftClip];
                std::strncpy(actionName, c.name.c_str(), sizeof(actionName) - 1);
                actionName[sizeof(actionName) - 1] = '\0';
                startR = c.startR; startC = c.startC;
                endR = c.endR; endC = c.endC;
                actionFPS = c.fps;
                actionLoop = c.loop;
            }
            else
            {
                // "<New / Unsaved>" -> clear the name so user sees it's a fresh slot
                std::strncpy(actionName, "", sizeof(actionName) - 1);
                actionName[sizeof(actionName) - 1] = '\0';
            }
        }

        // "New" button to return to "<New / Unsaved>" mode
        ImGui::SameLine();
        if (ImGui::Button("New##DraftClip"))
        {
            selectedDraftClip = -1;
            ResetDraftPlayback();
            std::strncpy(actionName, "", sizeof(actionName) - 1);
            actionName[sizeof(actionName) - 1] = '\0';
        }

        // Define action range (row/col start and end + fps + loop)
        ImGui::NewLine();
        ImGui::Separator();
        ImGui::Text("DEFINE ACTION RANGE:");

        ImGui::InputText("Name", actionName, IM_ARRAYSIZE(actionName));
        ImGui::Checkbox("Loop", &actionLoop);
        ImGui::SameLine();
        ImGui::PushItemWidth(120);
        ImGui::InputFloat("FPS", &actionFPS, 0.5f, 1.0f, "%.2f");
        ImGui::PopItemWidth();
        actionFPS = std::max(0.001f, actionFPS);

        // Clamp
        startR = std::clamp(startR, 0, draftRows - 1);
        endR = std::clamp(endR, 0, draftRows - 1);
        startC = std::clamp(startC, 0, draftCols - 1);
        endC = std::clamp(endC, 0, draftCols - 1);

        if (ImGui::SliderInt("Start Row", &startR, 0, draftRows - 1)) ResetDraftPlayback();
        if (ImGui::SliderInt("Start Col", &startC, 0, draftCols - 1)) ResetDraftPlayback();
        if (ImGui::SliderInt("End Row", &endR, 0, draftRows - 1)) ResetDraftPlayback();
        if (ImGui::SliderInt("End Col", &endC, 0, draftCols - 1)) ResetDraftPlayback();

        auto FindClipIndexByName = [&](const std::string& name) -> int
            {
                for (int i = 0; i < (int)draftClips.size(); ++i)
                    if (draftClips[i].name == name) return i;
                return -1;
            };

        std::string nameStr(actionName);
        bool hasName = !nameStr.empty();

        if (ImGui::Button("Add / Update Clip"))
        {
            if (!hasName)
            {
                std::cerr << "[Previewer] Cannot add clip: empty name.\n";
            }
            else
            {
                int idx = FindClipIndexByName(nameStr);
                DraftClip c;
                c.name = nameStr;
                c.startR = startR; c.startC = startC;
                c.endR = endR; c.endC = endC;
                c.fps = actionFPS;
                c.loop = actionLoop;

                if (idx >= 0)
                {
                    // Update existing
                    draftClips[idx] = c;
                    selectedDraftClip = idx;
                }
                else
                {
                    // Add new
                    draftClips.push_back(c);
                    selectedDraftClip = (int)draftClips.size() - 1;
                }
            }
        }

        // Remove the currently selected clip
        ImGui::SameLine();
        if (ImGui::Button("Remove Clip"))
        {
            if (selectedDraftClip >= 0 && selectedDraftClip < (int)draftClips.size())
            {
                draftClips.erase(draftClips.begin() + selectedDraftClip);
                selectedDraftClip = -1;
                ResetDraftPlayback();
                std::strncpy(actionName, "", sizeof(actionName) - 1);
                actionName[sizeof(actionName) - 1] = '\0';
            }
        }
    }

    // APPLY
    if (obj && draftTexID)
    {
        ImGui::NewLine();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f)); // Green button for apply
        if (ImGui::Button("APPLY TO SELECTED OBJECT", ImVec2(-1, 35)))
        {
            if (!draftPath.empty())
            {
                // Check if a RenderComponent exists
                RenderComponent* render = obj->GetComponent<RenderComponent>();
                if (!render)
                {
                    obj->AddComponent<RenderComponent>();
                    render = obj->GetComponent<RenderComponent>();
                }

                // Check if the Spritesheet component exist
                if (ss)
                {
                    ss->LoadNewSpritesheet(draftPath.c_str(), draftRows, draftCols);
                }
                else
                {
                    obj->AddComponent<Spritesheet>(draftPath.c_str(), draftRows, draftCols);
                    ss = obj->GetComponent<Spritesheet>();
                }

                // Checking texture is visible + serializes properly
                if (render && ss)
                {
                    render->SetTextureID(ss->GetTextureID());
                    render->SetTexturePath(draftPath);
                    render->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
                }

                // Register animations into Spritesheet component
                if (ss)
                {
                    if (!draftClips.empty())
                    {
                        for (const auto& c : draftClips)
                        {
                            if (c.name.empty()) continue;
                            ss->RegisterAnimation(c.name, draftPath, draftRows, draftCols,
                                c.startR, c.startC, c.endR, c.endC, c.fps);
                        }

                        // Play currently selected draft clip, else play first
                        if (selectedDraftClip >= 0 && selectedDraftClip < (int)draftClips.size())
                            ss->PlayAnimation(draftClips[selectedDraftClip].name);
                        else
                            ss->PlayAnimation(draftClips.front().name);
                    }
                    else
                    {
                        // Register a single clip from current inputs
                        std::string oneName(actionName);
                        if (!oneName.empty())
                        {
                            ss->RegisterAnimation(oneName, draftPath, draftRows, draftCols,
                                startR, startC, endR, endC, actionFPS);
                            ss->PlayAnimation(oneName);
                        }
                    }
                }
            }

            // Reset preview
            ResetDraftPlayback();
            currentRowIdx = 0;
            currentColIdx = 0;
            currentTimer = 0.0f;
        }

        ImGui::PopStyleColor();
    }

    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::End();
}
#pragma endregion

// Helpers
// cover your eyes this will be hardcoded based off what we have rn
// [WIP] Consider adding explanation text in the tuple for specific explanation.
void PopulateComponentList()
{
    //transform physics collider camera render text animator
    // stores Display Text, icon texture, std::function to check if component is alr present, if not add it.
    // ^ might have to rethink this because we might want multiple of the same components 

    // push_back to vector the component details
        // lambda for adding component checks if already has component
            // if dont have, then add
    AllComponents.push_back({ ComponentType::Transform, "Transform Component" , ImTextureID(icon_active_ui), [](GameObject* go){ 
        if (!go->GetComponent<TransformComponent>()){
            go->AddComponent<TransformComponent>();     std::cout << "Added Transform Component to " << go->GetName() << std::endl; } } });
                
    AllComponents.push_back({ ComponentType::Transform, "Physics Component"   , ImTextureID(icon_active_ui), [](GameObject* go){ 
            if (!go->GetComponent<PhysicsComponent>()){ 
                go->AddComponent<PhysicsComponent>();   std::cout << "Added Physics Component to " << go->GetName() << std::endl; } } });

    AllComponents.push_back({ ComponentType::Transform, "Collider Component"  , ImTextureID(icon_active_ui), [](GameObject* go){ 
            if (!go->GetComponent<ColliderComponent>()) { 
                go->AddComponent<ColliderComponent>();  std::cout << "Added Collider Component to " << go->GetName() << std::endl; } } });

    // Camera component doesnt work like that LMAO
    //AllComponents.push_back({ ComponentType::Camera, "Camera Component"    , ImTextureID(icon_active_ui), [](GameObject* go){ 
    //        if (!go->GetComponent<CameraComponent>()) { 
    //            go->AddComponent<CameraComponent>();    std::cout << "Added Camera Component to " << go->GetName() << std::endl; } } });

    AllComponents.push_back({ ComponentType::Render, "Render Component"    , ImTextureID(icon_active_ui), [](GameObject* go){ 
            if (!go->GetComponent<RenderComponent>()) { 
                go->AddComponent<RenderComponent>();    std::cout << "Added Render Component to " << go->GetName() << std::endl; } } });
    AllComponents.push_back({ ComponentType::Event, "Event Component"    , ImTextureID(icon_active_ui), [](GameObject* go) {
            if (!go->GetComponent<EventComponent>()) {
                go->AddComponent<EventComponent>();    std::cout << "Added Event Component to " << go->GetName() << std::endl; } } });

    AllComponents.push_back({ ComponentType::Script, "Script Component"    , ImTextureID(icon_active_ui), [](GameObject* go) {
        if (!go->GetComponent<ScriptComponent>()) {
            go->AddComponent<ScriptComponent>();    std::cout << "Added Script Component to " << go->GetName() << std::endl;
} } });

    //AllComponents.push_back({ "Text Component"      , ImTextureID(icon_active_ui), [](GameObject* go) { go->AddComponent<TextComponent>(); } });
    //AllComponents.push_back({ "Animator Component"  , ImTextureID(icon_active_ui), [](GameObject* go) { go->AddComponent<Spritesheet>(); } });
    // cant add these because theyre abit special, not just a simple slap on component
}

/**
 * @brief Refreshes vector of prefabs
 */
void ReloadPrefabsList()
{
    prefabList = Serialization::GetAllPrefabs();
}

/**
 * @brief Get text of shape type
 */
const char* ImGuiSystem::GetShapeTypeName(int type)
{
    switch (type)
    {
    case 0:
        return "Square";
    case 1:
        return "Circle";
    case 2:
        return "Line";
    case 3:
        return "Point";
    default:
        return "Unknown";
    }
}

/**
 * @brief Get Collision layer string
 */
const char* ImGuiSystem::GetCollisionLayerName(int layer)
{
    switch (layer)
    {
    case 0:
        return "Player";
    case 1:
        return "Ground";
    case 2:
        return "Wall";
    case 3:
        return "Enemy";
    case 4:
        return "Platform";
    case 5:
        return "Default";
    default:
        return "Unknown";
    }
}

void TileMap_Render(float dt)
{
    // Renders the tilemap - putting up here so game opens showing viewport first
    if (showTilemapEditor)
    {
        tilemapEditor->SetVisible(true);
        tilemapEditor->Update(dt);
        tilemapEditor->Render();

        //handle window close button
        if (!tilemapEditor->IsVisible())
        {
            showTilemapEditor = false;
        }
    }
    else
    {
        tilemapEditor->SetVisible(false);
    }
}
#endif
