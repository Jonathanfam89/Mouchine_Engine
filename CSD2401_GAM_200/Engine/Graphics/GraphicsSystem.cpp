/* Start Header ************************************************************************/
/*!
\file       GraphicsSystem.cpp
\author     Guan Shao Jun [85%]
            Lee Hwee Min [ 1% - 8 lines]
\par        shaojun.g@digipen.edu
\date       November 07 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GraphicsSystem.h"
#include "Animator.hpp"

// Global constant for configuration file path (currently unused in this file)
const char* configFile = "../Logs/config.txt";
// Global constant for 2*Pi, used for circle approximation
const double two_pi = 6.28318530717958647692528676655900576;

/**
 * \brief The GraphicsSystem class manages all OpenGL rendering, GLFW window creation,
 * and the 2D batch rendering system.
 */

 /**
  * \brief Default constructor for GraphicsSystem.
  * Initializes GLFW, creates the main rendering window, and sets up window callbacks.
  */
GraphicsSystem::GraphicsSystem()
    : window(nullptr), vao(0), vbo(0), Wwidth(800), Wheight(600), tex(0), vp(), batchVAO(0), batchVBO(0), batchEBO(0), textVAO(0), textVBO(0),
    frameBuffer(0), frameBufferTexture(0), depthRenderBuffer(0),
    frameBufferWidth(800), frameBufferHeight(600), editorMode(false), isFullscreen(false)
{
    // Initialize GLFW library (required before creating a window/context).
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    // Create a resizable window with fixed default size (800x600 for now).
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(Wwidth, Wheight, "Game Engine", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return;
    }

    // Store pointer to this GraphicsSystem instance in the GLFW window,
    // so we can retrieve it inside callbacks.
    glfwSetWindowUserPointer(window, this);

    // Set callback for window resizing (updates viewport + stored dimensions).
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int width, int height)
        {
            glViewport(0, 0, width, height);

            // Update stored window width/height in GraphicsSystem.
            auto* gs = static_cast<GraphicsSystem*>(glfwGetWindowUserPointer(win));
            if (gs) {
                gs->Wwidth = width;
                gs->Wheight = height;

                if (gs->editorMode) {
                    gs->ResizeFrameBuffer(width, height);
                }

            } });
    
            // Make OpenGL context current for this window.
            glfwMakeContextCurrent(window);

            // Query the framebuffer size (accounts for HiDPI displays).
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
            // Set the initial OpenGL viewport to match the actual framebuffer size
            glViewport(0, 0, fbWidth, fbHeight);

            // Store the actual framebuffer size.
            Wwidth = fbWidth;
            Wheight = fbHeight;
}

/**
 * \brief Overloaded constructor for GraphicsSystem.
 * Allows specifying initial window dimensions.
 * \param w Initial window width.
 * \param h Initial window height.
 */
GraphicsSystem::GraphicsSystem(GLint w, GLint h)
    : window(nullptr), vao(0), vbo(0), Wwidth(w), Wheight(h), vp(), batchVAO(0), batchVBO(0), batchEBO(0), textVAO(0), textVBO(0),
    frameBuffer(0), frameBufferTexture(0), depthRenderBuffer(0),
    frameBufferWidth(800), frameBufferHeight(600), editorMode(false), isFullscreen(false)
    // Same as default constructor, but allows user to specify window size.
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(Wwidth, Wheight, "Game Engine", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return;
    }

    // Store pointer to this GraphicsSystem instance in the GLFW window,
    // so we can retrieve it inside callbacks.
    glfwSetWindowUserPointer(window, this);

    // Set callback for window resizing (updates viewport + stored dimensions).
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int width, int height)
        {
            glViewport(0, 0, width, height);

            // Update stored window width/height in GraphicsSystem.
            auto* gs = static_cast<GraphicsSystem*>(glfwGetWindowUserPointer(win));
            if (gs) {
                gs->Wwidth = width;
                gs->Wheight = height;
            } });

            // Make OpenGL context current for this window.
            glfwMakeContextCurrent(window);

            // Query the framebuffer size (accounts for HiDPI displays).
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
            // Set the initial OpenGL viewport to match the actual framebuffer size
            glViewport(0, 0, fbWidth, fbHeight);

            // Store the actual framebuffer size.
            Wwidth = fbWidth;
            Wheight = fbHeight;
}

/**
 * \brief Destructor for GraphicsSystem.
 * Cleans up OpenGL resources (Frame Buffer, VBOs, VAOs) and terminates GLFW.
 */
GraphicsSystem::~GraphicsSystem()
{
    if (window) {
        glfwSetCursor(window, nullptr); // Reset to default cursor
    }

    // Clean up Frame Buffer resources
    DeleteFrameBuffer();

    if (normalCursor) {
        glfwDestroyCursor(normalCursor);
        normalCursor = nullptr;
    }
    if (clickCursor) {
        glfwDestroyCursor(clickCursor);
        clickCursor = nullptr;
    }

    // Clean up resources on destruction.
    if (window)
    {
        glfwDestroyWindow(window); // Destroy GLFW window
        glfwTerminate();           // Terminate GLFW (frees global resources)
    }
}

/**
 * \brief Initializes OpenGL state, GLEW, shaders, and main rendering buffers (VAOs/VBOs).
 */
void GraphicsSystem::Initialize()
{
    // Initialize GLEW (required to access modern OpenGL functions).
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW\n";
        return;
    }
    LoadDefaultCursor("../Assets/Mouse.png");
    clickCursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);

    // Initialize cursors with null checks
    //normalCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    //clickCursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);

    //std::cout << "Normal cursor created: " << (normalCursor ? "SUCCESS" : "FAILED") << std::endl;
    //std::cout << "Click cursor created: " << (clickCursor ? "SUCCESS" : "FAILED") << std::endl;

    // Set initial cursor - only if creation was successful
    //if (normalCursor) {
    //    glfwSetCursor(window, clickCursor);
    //}
    //else {
    //    std::cerr << "WARNING: Failed to create normal cursor, using default" << std::endl;
    //}

    // Improved mouse button callback with better error handling
    //glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int mods) {
    //    auto* gs = static_cast<GraphicsSystem*>(glfwGetWindowUserPointer(win));
    //    if (!gs) {
    //        return;
    //    }

    //    if (button == GLFW_MOUSE_BUTTON_LEFT) {
    //        if (action == GLFW_PRESS) {
    //            // Safe cursor switching
    //            GLFWcursor* clickCursor = gs->GetClickCursor();
    //            if (clickCursor) {
    //                glfwSetCursor(win, clickCursor);
    //            }
    //        }
    //        else if (action == GLFW_RELEASE) {
    //            // Safe cursor switching back to normal
    //            GLFWcursor* normalCursor = gs->GetNormalCursor();
    //            if (normalCursor) {
    //                glfwSetCursor(win, normalCursor);
    //            }
    //            else {
    //                // Fallback to default cursor if custom normal cursor is unavailable
    //                glfwSetCursor(win, nullptr);
    //            }
    //        }
    //    }
    //    });

    glfwSetWindowFocusCallback(window, [](GLFWwindow* win, int focused)
        {
            if (focused == GLFW_FALSE)
            {
                CORE->SetGamePaused(true);
            }
            else
            {
                CORE->SetGamePaused(false);
            }
        });

    // Enable blending (allows transparency in textures and colors).
    glEnable(GL_BLEND);
    // Standard blend function for alpha transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Compile and link the default shader program for rendering.
    // Requires "vertices.vert" (vertex shader) and "fragment.frag" (fragment shader).
    shader.CompileLinkValidate({ {GL_VERTEX_SHADER, "../Shaders/vertices.vert"},
                                {GL_FRAGMENT_SHADER, "../Shaders/fragment.frag"} });

    // Compile and link the debug shader program for lines and points
    debugShader.CompileLinkValidate({ {GL_VERTEX_SHADER, "../Shaders/debug.vert"},
                                {GL_FRAGMENT_SHADER, "../Shaders/debug.frag"} });
    std::cout << "Debug shader compiled successfully\n\n\n\n" << std::endl;

    // Define vertex structure for a textured quad.
    struct Vertex
    {
        vec3 pos; // Position (x, y, z)
        vec2 tex; // Texture coordinates (u, v)
    };

    // Quad geometry (two triangles forming a square).
    // Coordinates are in NDC space, centered at origin.
    Vertex vertices[] = {
        {{-0.5f, -0.5f, 0.f}, {0.f, 0.f}}, // bottom-left
        {{0.5f, -0.5f, 0.f}, {1.f, 0.f}},  // bottom-right
        {{-0.5f, 0.5f, 0.f}, {0.f, 1.f}},  // top-left
        {{0.5f, 0.5f, 0.f}, {1.f, 1.f}}    // top-right
    };

    // === VAO (Vertex Array Object) + VBO (Vertex Buffer Object) setup for single draw ===

    // Create a VAO to store the state of vertex attribute bindings.
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create a VBO and upload vertex data into GPU memory.
    glCreateBuffers(1, &vbo);
    // Use immutable storage for performance (data is static)
    glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);

    // Bind the VBO to the VAO at binding point 0.
    // Each vertex is sizeof(Vertex) apart.
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));

    // --- Attribute 0: Position (vec3) ---
    glEnableVertexArrayAttrib(vao, 0); // enable attribute location 0
    // Define the format and offset for the position attribute
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
    glVertexArrayAttribBinding(vao, 0, 0); // bind attribute 0 to binding point 0

    // --- Attribute 1: Texture coordinates (vec2) ---
    glEnableVertexArrayAttrib(vao, 1); // enable attribute location 1
    // Define the format and offset for the texture coordinate attribute
    glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex));
    glVertexArrayAttribBinding(vao, 1, 0); // bind attribute 1 to binding point 0

    // === Batch Rendering Buffers Setup ===
    // Initialise batch rendering buffers
    glCreateVertexArrays(1, &batchVAO);
    glCreateBuffers(1, &batchVBO); // Vertex data buffer
    glCreateBuffers(1, &batchEBO); // Index data buffer

    // Allocate large buffers for batch rendering
    // Use GL_DYNAMIC_STORAGE_BIT because the data changes every frame (SubData is used)
    glNamedBufferStorage(batchVBO, MAX_BATCH_VERTICES * sizeof(BatchVertex), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(batchEBO, MAX_BATCH_INDICES * sizeof(GLuint), nullptr, GL_DYNAMIC_STORAGE_BIT);

    // bind buffers to VAO
    glVertexArrayVertexBuffer(batchVAO, 0, batchVBO, 0, sizeof(BatchVertex));
    glVertexArrayElementBuffer(batchVAO, batchEBO);

    // Configure vertex attributes for the BatchVertex structure
    // Attribute 0: Position (vec3)
    glEnableVertexArrayAttrib(batchVAO, 0);
    glVertexArrayAttribFormat(batchVAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(BatchVertex, pos));
    glVertexArrayAttribBinding(batchVAO, 0, 0);

    // Attribute 1: Texture Coords (vec2)
    glEnableVertexArrayAttrib(batchVAO, 1);
    glVertexArrayAttribFormat(batchVAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(BatchVertex, tex));
    glVertexArrayAttribBinding(batchVAO, 1, 0);

    // Attribute 2: Color (vec4)
    glEnableVertexArrayAttrib(batchVAO, 2);
    glVertexArrayAttribFormat(batchVAO, 2, 4, GL_FLOAT, GL_FALSE, offsetof(BatchVertex, color));
    glVertexArrayAttribBinding(batchVAO, 2, 0);

    // Attribute 3: UV Scale (vec2)
    glEnableVertexArrayAttrib(batchVAO, 3);
    glVertexArrayAttribFormat(batchVAO, 3, 2, GL_FLOAT, GL_FALSE, offsetof(BatchVertex, uvScale));
    glVertexArrayAttribBinding(batchVAO, 3, 0);

    // Attribute 4: UV Offset (vec2)
    glEnableVertexArrayAttrib(batchVAO, 4);
    glVertexArrayAttribFormat(batchVAO, 4, 2, GL_FLOAT, GL_FALSE, offsetof(BatchVertex, uvOffset));
    glVertexArrayAttribBinding(batchVAO, 4, 0);

    // Attribute 5: Transform (vec3) - Stores (x, y, rotation) for the quad center
    glEnableVertexArrayAttrib(batchVAO, 5);
    glVertexArrayAttribFormat(batchVAO, 5, 3, GL_FLOAT, GL_FALSE, offsetof(BatchVertex, transform));
    glVertexArrayAttribBinding(batchVAO, 5, 0);

    // Attribute 6: Scale (vec2) - Stores (scaleX, scaleY)
    glEnableVertexArrayAttrib(batchVAO, 6);
    glVertexArrayAttribFormat(batchVAO, 6, 2, GL_FLOAT, GL_FALSE, offsetof(BatchVertex, scale));
    glVertexArrayAttribBinding(batchVAO, 6, 0);

    // === Text Rendering Buffers Setup ===
    // Compile and link the text shader
    textShader.CompileLinkValidate({ {GL_VERTEX_SHADER, "../Shaders/text.vert"},
                                    {GL_FRAGMENT_SHADER, "../Shaders/text.frag"} });

    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    // Allocate buffer space for a single glyph (6 vertices, 4 floats per vertex)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    // Attribute 0: vertex position and texture coordinates (x, y, u, v)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float flashQuad[]{
        -1.f, -1.f,
        1.f, -1.f,
        -1.f, 1.f,
        1.f, 1.f
    };

    glGenVertexArrays(1, &flashVAO);
    glGenBuffers(1, &flashVBO);

    glBindVertexArray(flashVAO);
    glBindBuffer(GL_ARRAY_BUFFER, flashVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(flashQuad), flashQuad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);

    // Initialize the off-screen framebuffer for the editor viewport
    CreateFrameBuffer(800, 600);

    ToggleFullScreen();
}

/**
 * \brief The main rendering loop function.
 * Handles framebuffer binding, screen clearing, game object accumulation into batches,
 * batch drawing, and drawing non-batched items (lines, points, text, debug shapes).
 * \param deltaTime The time elapsed since the last frame (not used in this rendering logic).
 */
void GraphicsSystem::Update(float)
{
    // std::cout << "Debug Mode: " << (debugMode ? "ON" : "OFF") << std::endl;

    if (!window)
        return; // Skip if no valid window.

    // === Framebuffer and Viewport Setup ===
    if (editorMode)
    {
        // Render to the custom framebuffer texture for the editor viewport
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glViewport(0, 0, frameBufferWidth, frameBufferHeight);
    }
    else
    {
        // Render directly to the default window framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        int currentWidth, currentHeight;
        glfwGetFramebufferSize(window, &currentWidth, &currentHeight);

        // Update stored dimensions if they changed
        if (currentWidth != Wwidth || currentHeight != Wheight)
        {
            Wwidth = currentWidth;
            Wheight = currentHeight;
        }
        glViewport(0, 0, Wwidth, Wheight);
    }

    // === Clear Screen ===
    glClearColor(0.6f, 0.5f, 0.45f, 0.0f);              // Background color (RGBA)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color buffer and depth buffer
    glLineWidth(2.f);                                   // Set line thickness for wireframes/debug
    glPointSize(50.f);                                  // Set point size (for debug points)

    // Use main shader program
    shader.Use();

    // === View-Projection Matrix Calculation ===
    float renderWidth = editorMode ? (float)(frameBufferWidth) : float(Wwidth);
    float renderHeight = editorMode ? (float)(frameBufferHeight) : float(Wheight);
#ifdef USE_IMGUI
    if (levelEditor && (levelEditor->GetEditorMode() == EditorMode::Stop || levelEditor->GetEditorMode() == EditorMode::Pause))
    {
        // EDITOR MODE: Use free editor camera for level editing
        // This allows designers to navigate and edit the level
        vp = levelEditor->GetEditorViewProjection((float)renderWidth, (float)renderHeight);
    }
    else
    {
        // GAME MODE: Use the primary game camera
        vp = CameraManager::Instance().GetViewProjectMatrix(renderWidth, renderHeight);
        vec2 camPos = CameraManager::Instance().GetCameraPosition();
    }
#endif

    vp = CameraManager::Instance().GetViewProjectMatrix(renderWidth, renderHeight);
    vec2 camPos = CameraManager::Instance().GetCameraPosition();

    // Accumulate all geometry into batches
    batches.clear();

    auto* textObj = GameObjectFactory::Instance().FindGameObject("Text FPS");
    if (textObj) {
        auto* textFPS = textObj->GetComponent<TextComponent>();
#ifdef USE_IMGUI
        if (InputManager::ShowFPS && CORE->GetLevelEditor()->GetEditorMode() == EditorMode::Play) {
#else
        if (InputManager::ShowFPS){
#endif
            textFPS->SetEnabled(true);
            textFPS->SetText("FPS: "+std::to_string(static_cast<int>(CORE->GetFPS())));
        }
        else {
            textFPS->SetText("FPS");
            textFPS->SetEnabled(false);
        }
    }

    auto& gameObjects = GameObjectFactory::Instance().GetAllGameObjects();
    // === Batch Accumulation Loop ===
    for (auto& obj : gameObjects)
    {
        if (!obj->IsActive())
        {
            continue;
        }

        auto* transform = obj->GetComponent<TransformComponent>();
        if (!transform)
        {
            continue;
        }

        auto* rend = obj->GetComponent<RenderComponent>();

        if (!rend)
        {
            continue;
        }
        // Only batch Square and Circle for now
        if (rend->GetShapeType() != RenderComponent::ShapeType::Circle && rend->GetShapeType() != RenderComponent::ShapeType::Square)
        {
            continue; // Skip non-batchable shapes (handled later)
        }

        // Gather transform data
        float x, y, sx, sy;
        transform->GetPosition(x, y);
        transform->GetScale(sx, sy);
        float rot = transform->GetRotation();

        // Gather render data
        GLuint texID = rend->GetTextureID();
        vec4 color = vec4(rend->GetColor());
        bool isCircle = (rend->GetShapeType() == RenderComponent::ShapeType::Circle);
        int layer = rend->GetLayer(); // For draw order sorting

        // Gather UV data for spritesheets/animation/tiles
        vec2 uvScale = vec2(1.0f);
        vec2 uvOffset = vec2(0.0f);

        if (auto* animator = obj->GetComponent<Spritesheet>())
        {
            // Animator takes priority (for animated sprites)
            uvScale = animator->GetUVScale();
            uvOffset = animator->GetUVOffset();
        }
        else if (auto* renderComp = obj->GetComponent<RenderComponent>())
        {
            // Use RenderComponent UV (for tiles/static sprites)
            uvScale = renderComp->GetUVScale();
            uvOffset = renderComp->GetUVOffset();
        }

        // Find an existing batch that matches the criteria (TextureID, ShapeType, Layer)
        RenderBatch* targetBatch = nullptr;
        for (auto& batch : batches)
        {
            if (batch.textureID == texID && batch.isCircle == isCircle && batch.layer == layer)
            {
                targetBatch = &batch;
                break;
            }
        }

        // If no matching batch found, create a new one
        if (!targetBatch)
        {
            batches.emplace_back(texID, isCircle, layer);
            targetBatch = &batches.back();

            // Reserve space to minimize reallocations
            targetBatch->vertices.reserve(20000);
            targetBatch->indices.reserve(30000);

            // std::cout << "Created batch - TexID: " << texID
            //     << ", Layer: " << layer
            //     << ", IsCircle: " << isCircle << std::endl;
        }

        // Local coordinates for a unit quad (centered at 0,0)
        vec3 localVerts[4] = {
            {-0.5f, -0.5f, 0.f},
            {0.5f, -0.5f, 0.f},
            {-0.5f, 0.5f, 0.f},
            {0.5f, 0.5f, 0.f} };

        // Texture coordinates for the unit quad
        vec2 texCoords[4] = {
            {0.f, 0.f},
            {1.f, 0.f},
            {0.f, 1.f},
            {1.f, 1.f} };

        GLuint baseIndex = (GLuint)targetBatch->vertices.size(); // Starting index for the new quad

        // Add 4 vertices to batch
        for (int i = 0; i < 4; ++i)
        {
            // Instead of calculating the full model matrix and world position here,
            // we store the local vertex and the quad's transformation data in the BatchVertex struct.
            // The transformation is applied in the vertex shader.
            BatchVertex vert;
            vert.pos = localVerts[i]; // Local position
            vert.tex = texCoords[i]; // Texture UV
            vert.color = color;
            vert.uvScale = uvScale;
            vert.uvOffset = uvOffset;
            vert.transform = vec3(x, y, rot); // Translation (x,y) and Rotation (rot)
            vert.scale = vec2(sx, sy); // Scale (sx, sy)

            targetBatch->vertices.push_back(vert);
        }

        // add 6 indices for 2 triangles (quad) using the base index
        targetBatch->indices.push_back(baseIndex + 0);
        targetBatch->indices.push_back(baseIndex + 1);
        targetBatch->indices.push_back(baseIndex + 2);
        targetBatch->indices.push_back(baseIndex + 2);
        targetBatch->indices.push_back(baseIndex + 1);
        targetBatch->indices.push_back(baseIndex + 3);
    }

    // Sort batches by rendering layer (lower layer draws first)
    std::sort(batches.begin(), batches.end(), [](const RenderBatch& a, const RenderBatch& b)
        { return a.layer < b.layer; });
    // auto afterSort = std::chrono::high_resolution_clock::now();

    // Draw all accumulated batches
    DrawBatches();
    // auto afterDraw = std::chrono::high_resolution_clock::now();

    // Draw Non-batched shapes (Lines, Points)
    for (auto& obj : gameObjects)
    {
        if (!obj->IsActive())
        {
            continue;
        }

        auto* rend = obj->GetComponent<RenderComponent>();
        if (!rend)
        {
            continue;
        }

        // Draw Lines and Points (not batched, drawn via immediate-mode-like temporary buffers)
        switch (rend->GetShapeType())
        {
        case RenderComponent::ShapeType::Line:
        {
            auto line = rend->GetLine();
            DrawLine(line.start, line.end, line.color, vp);
            break;
        }
        case RenderComponent::ShapeType::Point:
        {
            auto point = rend->GetPoint();
            DrawPoint(point.pos, point.color, vp);
            break;
        }
        default:
            break; // Batched shapes (Square/Circle) are already drawn
        }

        // Draw debug outlines for ALL shapes
        if (debugMode)
        {
            // std::cout << "went here" << std::endl;
            auto* transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                float x, y, sx, sy;
                transform->GetPosition(x, y);
                transform->GetScale(sx, sy);

                // Draw outlines based on the shape type
                if (rend->GetShapeType() == RenderComponent::ShapeType::Circle)
                {
                    // For circles, the scale is interpreted as diameter, so radius is scale*0.5
                    DrawCircleOutline(x, y, static_cast<float>(sx * 0.5), static_cast<int>(sy), { 0, 1, 0, 1 }, vp);
                }
                else if (rend->GetShapeType() == RenderComponent::ShapeType::Square)
                {
                    //std::cout << "X: " << x << " Y: " << y << " SX: " << sx << " SY: " << sy << std::endl;
                    DrawRectOutline(x, y, sx, sy, { 0, 1, 0, 1 }, vp);
                }
            }
        }

        // Text rendering
        if (auto* textComp = obj->GetComponent<TextComponent>())
        {
            if (textComp->IsEnabled())
            {
                if (auto* transform = obj->GetComponent<TransformComponent>())
                {
                    float x, y;
                    transform->GetPosition(x, y);
                    // Render text using the dedicated text rendering function
                    RenderText(textComp->GetCharacters(), textComp->GetText(), x, y,
                        textComp->GetScale(), textComp->GetColor(), textComp->isScreenSpace());
                }
            }
        }
    }

    // === Debug System Shapes Drawing ===
    if (debugMode)
    {
        auto& debugShapes = CORE->GetDebugSystem()->GetShapes();
        for (auto& shape : debugShapes)
        {
            switch (shape.type)
            {
            case DebugSystem::DebugShape::LINE:
            {
                DrawLine(shape.p1, shape.p2, shape.color, vp);
                break;
            }
            case DebugSystem::DebugShape::POINT:
            {
                DrawPoint(shape.p1, shape.color, vp);
                break;
            }
            case DebugSystem::DebugShape::RECT:
            {
                // p2 here stores width/height
                DrawRectOutline(shape.p1.x, shape.p1.y, shape.p2.x, shape.p2.y, shape.color, vp);
                break;
            }
            case DebugSystem::DebugShape::CIRCLE:
            {
                DrawCircleOutline(shape.p1.x, shape.p1.y, shape.radius, shape.segments, shape.color, vp);
                break;
            }
            }
        }
    }

    auto* activeCamera = CameraManager::Instance().GetActiveCamera();
    if (activeCamera) {
        if (activeCamera->IsFlashing()) {
            DrawFlashOverlay(activeCamera->GetFlashColor());
        }
    }

    if (currentSelection_ColliderOnly.exist) {
        DrawRectOutline(currentSelection_ColliderOnly.x, currentSelection_ColliderOnly.y, currentSelection_ColliderOnly.sx, currentSelection_ColliderOnly.sy, { 1,1,0.4,1 }, vp);       // draw yellow outline
    }
    if (currentSelection.exist) {
        DrawRectOutline(currentSelection.x, currentSelection.y, currentSelection.sx, currentSelection.sy, { 1,1,1,1 }, vp);         // draw white outline
    }

    // Unbind the custom framebuffer to prevent accidental drawing to it
    if (editorMode)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

/**
 * \brief Draws a line segment between two points in world space.
 * Uses a temporary VAO/VBO with the debug shader.
 * \param start The starting point of the line.
 * \param end The ending point of the line.
 * \param color The color of the line.
 * \param viewProjection The combined view-projection matrix (MVP in shader).
 */
void GraphicsSystem::DrawLine(const vec2& start, const vec2& end,
    const vec4& color, const mat4& viewProjection)
{
    // Tell shader this is a line draw
    debugShader.Use();
    debugShader.SetUniform("uColor", color);

    // Define 2 vertices for the line
    vec3 verts[2] = {
        {start.x, start.y, 0.0f},
        {end.x, end.y, 0.0f} };

    // Create temporary VAO/VBO for this line (immediate-mode like drawing)
    GLuint tempVBO, tempVAO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);

    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

    // Attribute 0 = vertex position (vec3 in shader)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

    // Send MVP matrix
    debugShader.SetUniform("uMVP", viewProjection);

    // Draw the line
    glDrawArrays(GL_LINES, 0, 2);

    // Cleanup temp objects
    glDeleteBuffers(1, &tempVBO);
    glDeleteVertexArrays(1, &tempVAO);
}

/**
 * \brief Draws a single point in world space.
 * Uses a temporary VAO/VBO with the debug shader.
 * \param ppos The position of the point.
 * \param color The color of the point.
 * \param viewProjection The combined view-projection matrix (MVP in shader).
 */
void GraphicsSystem::DrawPoint(const vec2& ppos,
    const vec4& color, const mat4& viewProjection)
{
    // Use the debug shader
    debugShader.Use();
    debugShader.SetUniform("uColor", color);

    // Position of the point (use vec3 to match shader input = vec3)
    vec3 pos = { ppos.x, ppos.y, 0.0f };

    // Create temporary VAO/VBO for this point
    GLuint tempVBO, tempVAO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);

    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pos), &pos, GL_DYNAMIC_DRAW);

    // Attribute 0 = vertex position (vec3 in shader)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

    // Send MVP matrix
    debugShader.SetUniform("uMVP", viewProjection);

    // Make point visible (default = 1px, often too small)
    glPointSize(5.0f);

    // Draw the point
    glDrawArrays(GL_POINTS, 0, 1);

    // Cleanup temp objects
    glDeleteBuffers(1, &tempVBO);
    glDeleteVertexArrays(1, &tempVAO);
}

/**
 * \brief Draws the outline of an axis-aligned rectangle.
 * Achieved by calling DrawLine four times.
 * \param x Center X coordinate.
 * \param y Center Y coordinate.
 * \param w Width of the rectangle.
 * \param h Height of the rectangle.
 * \param color The color of the outline.
 * \param viewProjection The combined view-projection matrix.
 */
void GraphicsSystem::DrawRectOutline(float x, float y, float w, float h,
    const vec4& color, const mat4& viewProjection)
{
    // Half extents (distance from center to edge)
    float halfW = w / 2;
    float halfH = h / 2;

    // Compute 4 corner points of the rectangle
    vec2 p1(x - halfW, y - halfH); // bottom-left
    vec2 p2(x + halfW, y - halfH); // bottom-right
    vec2 p3(x + halfW, y + halfH); // top-right
    vec2 p4(x - halfW, y + halfH); // top-left

    // Connect corners with lines (in order)
    DrawLine(p1, p2, color, viewProjection);
    DrawLine(p2, p3, color, viewProjection);
    DrawLine(p3, p4, color, viewProjection);
    DrawLine(p4, p1, color, viewProjection);
}

/**
 * \brief Draws the outline of a circle approximated by line segments.
 * \param cx Center X coordinate.
 * \param cy Center Y coordinate.
 * \param r Radius of the circle.
 * \param segments The number of line segments to use for approximation.
 * \param color The color of the outline.
 * \param viewProjection The combined view-projection matrix.
 */
void GraphicsSystem::DrawCircleOutline(float cx, float cy, float r, int segments,
    const vec4& color, const mat4& viewProjection)
{
    // Clamp to a minimum number of segments (otherwise circle looks too blocky)
    if (segments < 8)
        segments = 8;

    // Angle step per segment (full 360 degrees / segments)
    float step = static_cast<float>(two_pi) / segments;

    // Generate connected line segments around the circle
    for (int i = 0; i < segments; i++)
    {
        float theta1 = step * i;
        float theta2 = step * (i + 1);

        // Two consecutive points on the circle’s circumference (using polar coordinates)
        vec2 p1 = { cx + r * cos(theta1), cy + r * sin(theta1) };
        vec2 p2 = { cx + r * cos(theta2), cy + r * sin(theta2) };

        // Draw line between them
        DrawLine(p1, p2, color, viewProjection);
    }
}

/**
 * \brief Draws all accumulated batches.
 * Iterates through the sorted list of RenderBatches, uploads their data to the
 * GPU buffers (VBO/EBO), sets uniforms, binds the texture, and issues a single draw call.
 */
void GraphicsSystem::DrawBatches()
{
    if (batches.empty())
    {
        return;
    }

    shader.Use();
    // Signal the shader to use the batch rendering pipeline (instance data)
    shader.SetUniform("uUseBatching", 1);

    glBindVertexArray(batchVAO);


    for (auto& batch : batches)
    {
        if (batch.vertices.empty())
        {
            continue;
        }

        // === ADD THIS SAFETY CHECK ===
        // Ensure batch size doesn't exceed pre-allocated buffer size
        if (batch.vertices.size() > MAX_BATCH_VERTICES)
        {
            std::cerr << "ERROR: Batch exceeds vertex buffer size! "
                << batch.vertices.size() << " > " << MAX_BATCH_VERTICES << std::endl;
            continue;
        }
        if (batch.indices.size() > MAX_BATCH_INDICES)
        {
            std::cerr << "ERROR: Batch exceeds index buffer size! "
                << batch.indices.size() << " > " << MAX_BATCH_INDICES << std::endl;
            continue;
        }
        // === END SAFETY CHECK ===

        // Upload vertex data (sub-data for dynamic updates)
        glNamedBufferSubData(batchVBO, 0, batch.vertices.size() * sizeof(BatchVertex), batch.vertices.data());

        // Upload index data (sub-data for dynamic updates)
        glNamedBufferSubData(batchEBO, 0, batch.indices.size() * sizeof(GLuint), batch.indices.data());

        // Set Uniforms that apply to the whole batch
        shader.SetUniform("uMVP", vp);
        shader.SetUniform("uIsCircle", batch.isCircle ? 1 : 0);

        // Bind Texture
        if (batch.textureID > 0)
        {
            shader.SetUniform("uHasTexture", 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, batch.textureID);
            shader.SetUniform("uTexture", 0); // Set sampler to texture unit 0
        }
        else
        {
            // Disable texturing if no texture is provided
            shader.SetUniform("uHasTexture", 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // Issue a single draw call for all elements in the batch
        glDrawElements(GL_TRIANGLES, (GLsizei)batch.indices.size(), GL_UNSIGNED_INT, 0);
    }
    // Restore shader state to non-batching mode
    shader.SetUniform("uUseBatching", 0);
}

/**
 * \brief Renders text using a font atlas (Characters map).
 * \param characters Map of character data (texture, bearing, size, advance).
 * \param text The string to render.
 * \param x Starting X position.
 * \param y Starting Y position.
 * \param scale Scaling factor for the text size.
 * \param color Color of the text.
 * \param screenSpace True if projection should be orthographic screen-space, false for world-space.
 */
void GraphicsSystem::RenderText(const std::map<char, Character>& characters, const std::string& text, float x, float y, float scale, const vec4& color, bool screenSpace)
{
    textShader.Use();
    textShader.SetUniform("textColor", color);

    mat4 projection;
    float renderWidth = editorMode ? (float)(frameBufferWidth) : float(Wwidth);
    float renderHeight = editorMode ? (float)(frameBufferHeight) : float(Wheight);
    if (screenSpace)
    {
        // Screen-space projection (origin bottom-left of viewport)
        projection = mat4::ortho(0.0f, (float)renderWidth, 0.0f, (float)renderHeight);
    }
    else
    {
        // World-space projection (use the camera VP matrix)
        projection = vp;
    }
    textShader.SetUniform("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    // Iterate through all characters in the string
    for (char c : text)
    {
        auto it = characters.find(c);
        if (it == characters.end())
        {
            continue; // Skip characters not found in the font atlas
        }

        const Character& ch = it->second;

        // Calculate the quad position and size based on the character metrics (bearing, size) and scale
        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;
        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        // Update VBO for each character (4 vertices: pos.x, pos.y, tex.x, tex.y)
        float vertices[6][4] = {
            {xpos, ypos + h, 0.f, 0.f},
            {xpos, ypos, 0.f, 1.f},
            {xpos + w, ypos, 1.f, 1.f},

            {xpos, ypos + h, 0.f, 0.f},
            {xpos + w, ypos, 1.f, 1.f},
            {xpos + w, ypos + h, 1.f, 0.f} };

        // Render the glyph texture
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update the VBO data with the current glyph's quad vertices
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render the quad (6 vertices, 2 triangles)
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Advance the cursor position for the next glyph (advance is in 1/64 pixels)
        x += (ch.advance >> 6) * scale;
    }

    // Cleanup
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * \brief Creates an OpenGL Frame Buffer Object (FBO) for off-screen rendering.
 * \param width The width of the framebuffer's color and depth attachments.
 * \param height The height of the framebuffer's color and depth attachments.
 */
void GraphicsSystem::CreateFrameBuffer(int width, int height)
{
    frameBufferWidth = width;
    frameBufferHeight = height;

    // 1. Generate and bind the FBO
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    // 2. Create the Color Attachment (Texture)
    glGenTextures(1, &frameBufferTexture);
    glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
    // Allocate storage for the color texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Attach the texture to the FBO's color attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture, 0);

    // 3. Create the Depth/Stencil Attachment (Renderbuffer)
    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    // Allocate storage for the depth and stencil buffer
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    // Attach the renderbuffer to the FBO's depth/stencil attachment point
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

    // 4. Check for FBO completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR: Framebuffer is not complete! \n";
    }

    // 5. Unbind FBO (return to default framebuffer)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::cout << "framebuffer created " << width << "x" << height << std::endl;
}

/**
 * \brief Deletes all OpenGL resources associated with the Frame Buffer Object (FBO).
 */
void GraphicsSystem::DeleteFrameBuffer()
{
    if (frameBuffer)
    {
        // Delete the FBO and its attachments
        glDeleteFramebuffers(1, &frameBuffer);
        glDeleteTextures(1, &frameBufferTexture);
        glDeleteRenderbuffers(1, &depthRenderBuffer);
        // Reset IDs to 0
        frameBuffer = 0;
        frameBufferTexture = 0;
        depthRenderBuffer = 0;
    }
}

/**
 * \brief Resizes the Frame Buffer and its attachments.
 * Calls DeleteFrameBuffer() followed by CreateFrameBuffer(width, height).
 * \param width The new width.
 * \param height The new height.
 */
void GraphicsSystem::ResizeFrameBuffer(int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        return;
    }
    // Delete existing attachments
    DeleteFrameBuffer();
    // Recreate with new dimensions
    CreateFrameBuffer(width, height);
}

/**
 * \brief Updates the framebuffer size to match the provided viewport dimensions.
 * Intended to be called from an ImGui rendering loop for a game viewport panel.
 * \param viewportSizeX Reference to the desired viewport width (updated if needed).
 * \param viewportSizeY Reference to the desired viewport height (updated if needed).
 */
 // apologies for messy code, used 2 float refs instead of ImVec2 because i didnt wanna include imgui.h here and slow compile time
void GraphicsSystem::RenderGameViewportImGui(float& viewportSizeX, float& viewportSizeY)
{
    //if (!editorMode)
    //{
    //    return;
    //}

    // Clamp minimum viewport size to prevent issues
    if (viewportSizeX < 100.0f)
        viewportSizeX = 800.0f;
    if (viewportSizeY < 100.0f)
        viewportSizeY = 600.0f;

    // Check if a resize is necessary and perform it
    if ((int)viewportSizeX != frameBufferWidth || (int)viewportSizeY != frameBufferHeight)
    {
        if (viewportSizeX > 0 && viewportSizeY > 0)
        {
            ResizeFrameBuffer((int)viewportSizeX, (int)viewportSizeY);
        }
    }
}

void GraphicsSystem::DrawFlashOverlay(const vec4& color) {
    debugShader.Use();
    debugShader.SetUniform("uColor", color);
    debugShader.SetUniform("uMVP", mat4::identity());

    glBindVertexArray(flashVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void GraphicsSystem::ToggleFullScreen() {
    if (!window) {
        return;
    }

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    if (!isFullscreen) {
        glfwGetWindowPos(window, &windowedPosX, &windowedPosY);
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        isFullscreen = true;

        std::cout << "[GraphicsSystem] Switched to fullscreen: " << mode->width << "x" << mode->height << std::endl;
    }
    else {

        glfwSetWindowMonitor(window, nullptr, windowedPosX, windowedPosY, windowedWidth, windowedHeight, GLFW_DONT_CARE);

        isFullscreen = false;

        std::cout << "[GraphicsSytem] Switched to windowed " << Wwidth << "x" << Wheight << std::endl;
    }
}

void GraphicsSystem::LoadClickCursors(const char* normalPath, const char* clickPath, int normalHotX, int normalHotY, int clickHotX, int clickHotY) {
    if (window) {
        glfwSetCursor(window, nullptr);
    }

    if (normalCursor) {
        glfwDestroyCursor(normalCursor);
        normalCursor = nullptr;
    }

    if (clickCursor) {
        glfwDestroyCursor(clickCursor);
        clickCursor = nullptr;
    }

    int width, height, channels;
    unsigned char* pixels = stbi_load(normalPath, &width, &height, &channels, 4);
    if (pixels) {
        GLFWimage image;
        image.width = width;
        image.height = height;
        image.pixels = pixels;
        normalCursor = glfwCreateCursor(&image, normalHotX, normalHotY);
        stbi_image_free(pixels);
        std::cout << "Loaded normal cursor: " << normalPath << std::endl;
    }
    else {
        std::cerr << "Failed to load normal cursor: " << normalPath << std::endl;
        normalCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    }

    pixels = stbi_load(clickPath, &width, &height, &channels, 4);
    if (pixels) {
        GLFWimage image;
        image.width = width;
        image.height = height;
        image.pixels = pixels;
        clickCursor = glfwCreateCursor(&image, clickHotX, clickHotY);
        std::cout << "Loaded click cursor: " << clickPath << std::endl;
    }
    else {
        std::cerr << "Failed to load click cursor: " << clickPath << std::endl;
        clickCursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    }

    if (normalCursor) {
        glfwSetCursor(window, normalCursor);
    }
}

void GraphicsSystem::LoadDefaultCursor(const char* cursorPath, int hotX, int hotY) {
    // Reset to default cursor before changing
    if (window) {
        glfwSetCursor(window, nullptr);
    }

    // Destroy old cursor if it exists
    if (normalCursor) {
        glfwDestroyCursor(normalCursor);
        normalCursor = nullptr;
    }

    int width, height, channels;
    unsigned char* pixels = stbi_load(cursorPath, &width, &height, &channels, 4);
    if (pixels) {
        GLFWimage image;
        image.width = width;
        image.height = height;
        image.pixels = pixels;
        normalCursor = glfwCreateCursor(&image, hotX, hotY);
        stbi_image_free(pixels);
        std::cout << "Loaded custom cursor: " << cursorPath << std::endl;
    }
    else {
        std::cerr << "Failed to load custom cursor: " << cursorPath
            << " - Using default arrow cursor" << std::endl;
        normalCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    }

    // Set the new cursor
    if (normalCursor && window) {
        glfwSetCursor(window, normalCursor);
    }
}

/**
 * \brief Returns the name of the system.
 * \return A string representing the system's name.
 */
std::string GraphicsSystem::GetName()
{
    return "Graphics System";
}

/**
 * \brief Loads a 2D texture from a file using stb_image and creates an OpenGL texture object.
 * \param filepath Path to the image file.
 * \return The OpenGL texture ID (GLuint), or 0 if loading fails.
 */
GLuint GraphicsSystem::LoadTexture(const char* filepath)
{
    // Tell stb_image to flip images vertically (so origin is bottom-left, like OpenGL expects)
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    // Force load with 4 channels (RGBA) regardless of source image format
    unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
    if (!data)
    {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        return 0; // return 0 to signal failure
    }

    // Generate a texture object and bind it
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Upload pixel data to GPU (level 0, no border, RGBA format)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, data);

    // Generate mipmaps for scaling
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture wrapping mode (repeat when UV > 1.0 or < 0.0)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // X axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Y axis

    // Set filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // smooth with mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);               // smooth (no mipmaps for magnification)

    // Free image memory (data is now on GPU)
    stbi_image_free(data);

    // Return texture ID for later use
    return texture;
}

/**
 * \brief Clones the RenderComponent for a new GameObject.
 * \param NewClone The new GameObject that will own the cloned component.
 * \return A unique pointer to the newly cloned RenderComponent.
 */
std::unique_ptr<IComponent> RenderComponent::Clone(GameObject* NewClone) const
{

    // 1. Determine which constructor to use based on the original component's state.
    std::unique_ptr<RenderComponent> clonedComponent;

    // Check if the component uses a texture (textureID is non-zero)
    if (this->textureID != 0)
    {
        // Use the constructor for textured objects:
        // RenderComponent(owner, GLuint texID)
        clonedComponent = std::make_unique<RenderComponent>(NewClone, this->textureID);
    }
    else
    {
        // Use the constructor for shape/color:
        // RenderComponent(owner, ShapeType type, const vec4& color)
        clonedComponent = std::make_unique<RenderComponent>(NewClone, this->shape, this->color);
    }

    // 2. Clone specialized data for Line or Point types.
    if (this->shape == RenderComponent::ShapeType::Line)
    {
        // Copy the line data using the public setter
        clonedComponent->SetLine(this->line.start, this->line.end);
    }
    else if (this->shape == RenderComponent::ShapeType::Point)
    {
        // Copy the point data using the public setter
        clonedComponent->SetPoint(this->point.pos, this->point.size);
    }

    // 3. Clone the enabled state and layer.
    clonedComponent->SetEnabled(this->IsEnabled());

    clonedComponent->SetLayer(this->layer);

    return clonedComponent;
}

/**
 * \brief Handles a message to create a new object with a RenderComponent.
 * \param msg The CreateObjectMessage containing object details.
 */
void GraphicsSystem::HandleMessage(const CreateObjectMessage& msg)
{
    // Find the target GameObject by name
    GameObject* go = GameObjectFactory::Instance().FindByName(msg.name);
    if (!go)
        return;

    // Add a new RenderComponent to the object
    RenderComponent* render = go->AddComponent<RenderComponent>();
    // Configure the component with data from the message
    render->SetShapeType(msg.shapeType);
    render->SetColor({ msg.color.x, msg.color.y, msg.color.z, msg.color.w });

    std::cout << "[GraphicsSystem] Added RenderComponent to: " << msg.name << "\n";
}