/* Start Header************************************************************************ /
/*!
\file       ComponentSerialization.cpp
\author     Lee Hwee Min [100% - 245 lines]
\par        l.hweemin@digipen.edu
\date       November, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ComponentSerialization.h"
#include "Animator.hpp"
#include "logic.h"

std::unordered_map<GLuint, std::string> s_TexturePathByID;
std::unordered_map<std::string, GLuint> s_IDByTexturePath;

bool Serialization::Prefab_DragAndDropped;
bool Serialization::Prefab_Reinstantiate;

bool DND_firstObject = true;
float DND_rootJsonX = 0.0f;
float DND_rootJsonY = 0.0f;

// ------------------- Serialize -------------------
/**
 * @brief Serializes the TransformComponent into JSON.
 * @param j JSON object to write into.
 * @param t Pointer to the TransformComponent.
 */
void ComponentSerialization::SerializeTransform(json &j, const TransformComponent *t)
{
    j["TransformComponent"] = {
        {"pos", {t->posX, t->posY}},
        {"scale", {t->scaleX, t->scaleY}},
        {"rotation", t->GetRotation()}};
}

/**
 * @brief Serializes the RenderComponent into JSON, including color and texture data.
 * @param j JSON object to write into.
 * @param r Pointer to the RenderComponent.
 */
void ComponentSerialization::SerializeRender(json &j, RenderComponent *r)
{
    std::string texPath = r->GetTexturePath();

    if (texPath.empty())
    {
        auto it = s_TexturePathByID.find(r->GetTextureID());
        texPath = (it != s_TexturePathByID.end()) ? it->second : "";
    }

    j["RenderComponent"] = {
        {"shapeType", r->GetShapeTypeName()},
        {"texturePath", texPath},
        {"color", {r->GetColor().x, r->GetColor().y, r->GetColor().z, r->GetColor().w}}};
}

/**
 * @brief Serializes the Spritesheet component into JSON.
 * @param j JSON object to write into.
 * @param s Pointer to the Spritesheet.
 */
void ComponentSerialization::SerializeSpritesheet(json &j, Spritesheet *s)
{
    json animArray = json::array();

    for (const auto &pair : s->GetAnimationRegistry())
    {
        const std::string &name = pair.first;
        const auto &data = pair.second;

        // Calculate back the Row and Col from the linear Index
        // Row = Index / TotalCols
        // Col = Index % TotalCols
        int startRow = 0;
        int startCol = 0;
        int endRow = 0;
        int endCol = 0;

        if (data.col > 0)
        {
            startRow = data.startframe / data.col;
            startCol = data.startframe % data.col;
            endRow = data.endframe / data.col;
            endCol = data.endframe % data.col;
        }

        json animNode = {
            {"name", name},
            {"filepath", data.filepath},
            {"rows", data.row},
            {"cols", data.col},
            {"startRow", startRow},
            {"startCol", startCol},
            {"endRow", endRow},
            {"endCol", endCol},
            {"fps", data.fps}};

        animArray.push_back(animNode);
    }
    j["Spritesheet"] = {
        {"defaultAnimation", s->GetCurrentAnimationName()},
        {"animations", animArray}};
}

/**
 * @brief Serializes the PhysicsComponent into JSON.
 * @param j JSON object to write into.
 * @param p Pointer to the PhysicsComponent.
 */
void ComponentSerialization::SerializePhysics(json &j, PhysicsComponent *p)
{
    j["PhysicsComponent"] = {
        {"mass", p->GetMass()},
        {"isStatic", p->IsStatic()},
        {"useGravity", p->useGravity},
        {"isGrounded", p->IsGrounded()},
        {"velocity", {p->velocityX, p->velocityY}}};
}

/**
 * @brief Serializes the ColliderComponent into JSON.
 * @param j JSON object to write into.
 * @param c Pointer to the ColliderComponent.
 */
void ComponentSerialization::SerializeCollider(json &j, ColliderComponent *c)
{
    j["ColliderComponent"] = {
        {"shape", EnumConverters::ToString(c->GetType())},
        {"layer", EnumConverters::ToString(c->GetLayer())}};
}

/**
 * @brief Serializes the TextComponent into JSON.
 * @param j JSON object to write into.
 * @param tc Pointer to the TextComponent.
 */
void ComponentSerialization::SerializeText(json &j, TextComponent *tc)
{
    j["TextComponent"] = {
        {"fontPath", tc->GetFontPath()},
        {"fontSize", tc->GetFontSize()},
        {"text", tc->GetText()},
        {"color", {tc->GetColor().x, tc->GetColor().y, tc->GetColor().z, tc->GetColor().w}},
        {"scale", tc->GetScale()},
        {"screenSpace", tc->isScreenSpace()}};
}

void ComponentSerialization::SerializeCamera(json &j, CameraComponent *cam)
{
    std::string modeStr;
    switch (cam->GetMode())
    {
    case CameraComponent::CameraMode::FIXED:
        modeStr = "FIXED";
        break;
    case CameraComponent::CameraMode::FOLLOW_TARGET:
        modeStr = "FOLLOW_TARGET";
        break;
    case CameraComponent::CameraMode::FREE_MOVE:
        modeStr = "FREE_MOVE";
        break;
    default:
        modeStr = "FIXED";
        break;
    }

    j["CameraComponent"] = {
        {"mode", modeStr},
        {"position", {cam->GetPosition().x, cam->GetPosition().y}},
        {"followOffset", {cam->GetFollowOffset().x, cam->GetFollowOffset().y}},
        {"smoothing", cam->IsSmoothingEnabled()},
        {"smoothingSpeed", cam->GetSmoothingSpeed()},
        {"followTargetName", cam->GetFollowTargetName()}};
}

void ComponentSerialization::SerializeEvent(json &j, EventComponent *ec)
{
    if (!ec)
        return;

    json rulesJson = json::array();

    for (const auto &rule : ec->rules)
    {
        json r;
        r["eventType"] = EventTypeToString(rule.eventType);
        r["targetFilter"] = rule.targetFilter;
        r["action"] = ActionTypeToString(rule.action);

        rulesJson.push_back(r);
    }

    j["EventComponent"] = rulesJson;
}

// ------------------- Deserialize -------------------
/**
 * @brief Deserializes TransformComponent data from JSON and attaches it to a GameObject.
 * @param j JSON object containing Transform data.
 * @param go Target GameObject to apply data to.
 */
void ComponentSerialization::DeserializeTransform(const json &j, GameObject *go)
{
    if (!j.contains("TransformComponent"))
        return;

    const auto &t = j["TransformComponent"];
    auto *tc = go->AddComponent<TransformComponent>();

#ifdef USE_IMGUI
    float jsonX = t["pos"][0];
    float jsonY = t["pos"][1];

    if (Serialization::Prefab_DragAndDropped)
    {
        /*std::cout << "[DeserializeTransform] Prefab transform set to cursor pos\n";
        tc->SetPosition(viewportCursorPos.x, viewportCursorPos.y);
        std::cout << "CREATED AT " << "(" << viewportCursorPos.x << ", " << viewportCursorPos.y << ")\n";
        */

        ImVec2 spawnPos = viewportCursorPos;

        if (DND_firstObject)
        {
            // ROOT JSON position
            DND_rootJsonX = jsonX;
            DND_rootJsonY = jsonY;

            tc->SetPosition(spawnPos.x, spawnPos.y);
            DND_firstObject = false;
        }
        else
        {
            // RELATIVE offset based on JSON
            float relX = jsonX - DND_rootJsonX;
            float relY = jsonY - DND_rootJsonY;

            tc->SetPosition(spawnPos.x + relX, spawnPos.y + relY);
        }
    }
    else if (Serialization::Prefab_Reinstantiate)
    {
        tc->SetPosition(Prefab_Reinstantiate_Pos.x, Prefab_Reinstantiate_Pos.y);
        std::cout << "REINSTANTIATED AT " << "(" << viewportCursorPos.x << ", " << viewportCursorPos.y << ")\n";
    }
    else
    {
        std::cout << "[DeserializeTransform] Prefab transform set to serialised pos\n";
        tc->SetPosition(t["pos"][0], t["pos"][1]);
    }
#else
    tc->SetPosition(t["pos"][0], t["pos"][1]);
#endif

    tc->SetScale(t["scale"][0], t["scale"][1]);
    tc->SetRotation(t["rotation"]);
}

/**
 * @brief Deserializes RenderComponent data from JSON and attaches it to a GameObject.
 *
 * Handles texture loading through the ResourceManager when possible.
 * Falls back to default colors if textures are unavailable.
 *
 * @param j JSON object containing Render data.
 * @param go Target GameObject to apply data to.
 */
void ComponentSerialization::DeserializeRender(const json &j, GameObject *go)
{
    if (!j.contains("RenderComponent"))
        return;

    const auto &r = j["RenderComponent"];
    auto *render = go->AddComponent<RenderComponent>();

    std::string shapeType = r.value("shapeType", "Square");
    render->SetShapeType(RenderComponentTools::ShapeEnumToString(shapeType));

    std::string texPath = r.value("texturePath", "");
    GLuint texID = 0;

    if (!texPath.empty())
    {
        // try to get texture from resource manager
        auto &resourceMgr = ResourceManager::Instance();
        std::string goName = go->GetName();
        TextureResource *texResource = resourceMgr.GetTexture(goName);

        if (texResource && texResource->IsLoaded())
        {
            // use existing texture from resource manager
            texID = texResource->GetTextureID();
            std::cout << "Serialization using texture from resource manager: " << goName << std::endl;
        }
        else
        {
            // not in resource manager, load it and register it for future use
            std::cout << "Serialization texture not in ResourceManager, loading and registering: "
                      << goName << " -> " << texPath << std::endl;

            if (resourceMgr.LoadTexture(goName, texPath))
            {
                texResource = resourceMgr.GetTexture(goName);
                if (texResource && texResource->IsLoaded())
                {
                    texID = texResource->GetTextureID();
                }
            }
            else
            {
                // load directly
                std::cout << "Serialization warning: Failed to load via resource manager, directly loading" << std::endl;
                texID = GraphicsSystem::LoadTexture(texPath.c_str());
            }
        }

        if (texID != 0)
        {
            render->SetTextureID(texID);
            render->SetTexturePath(texPath);
            render->SetColor({1, 1, 1, 1});

            glBindTexture(GL_TEXTURE_2D, texID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            std::cerr << "[DeserializeRender] Warning: Texture ID is 0 for " << goName
                      << ", using fallback color instead.\n";
            render->SetTextureID(0);
            render->SetColor({1.0f, 0.5f, 0.5f, 1.0f}); // pink fallback
        }
    }
    else
    {
        if (r.contains("color"))
        {
            vec4 c(
                r["color"][0].get<float>(),
                r["color"][1].get<float>(),
                r["color"][2].get<float>(),
                r["color"][3].get<float>());
            render->SetColor(c);
        }
        else
        {
            // default color if no texture and no color given
            render->SetColor({1.0f, 0.5f, 0.5f, 1.0f});
        }

        render->SetTextureID(0);
        render->SetTexturePath("");
    }

    // --- Final safety check ---
    if (render->GetTextureID() == 0 && texPath.empty())
    {
        std::cout << "[DeserializeRender] No texture for " << go->GetName()
                  << ", rendering as pure color.\n";
    }
}

/**
 * @brief Deserializes Spritesheet data from JSON and attaches it to a GameObject.
 * @param j JSON object containing Spritesheet data.
 * @param go Target GameObject to apply data to.
 */
void ComponentSerialization::DeserializeSpritesheet(const json &j, GameObject *go)
{
    if (!j.contains("Spritesheet"))
        return;
    const auto &s = j["Spritesheet"];

    std::string initialPath = "";
    int initialRows = 1;
    int initialCols = 1;

    // Try to find a valid animation to initialize with
    if (s.contains("animations") && s["animations"].is_array() && !s["animations"].empty())
    {
        const auto &firstAnim = s["animations"][0];
        initialPath = firstAnim.value("filepath", "");
        initialRows = firstAnim.value("rows", 1);
        initialCols = firstAnim.value("cols", 1);
    }

    auto *spr = go->AddComponent<Spritesheet>(
        initialPath.c_str(),
        initialRows,
        initialCols);

    // 3. Loop through the "animations" array and Register them
    if (s.contains("animations") && s["animations"].is_array())
    {
        for (const auto &anim : s["animations"])
        {
            std::string name = anim.value("name", "Default");
            std::string path = anim.value("filepath", "");
            int rows = anim.value("rows", 1);
            int cols = anim.value("cols", 1);

            // READ NEW COORDINATE KEYS
            int startRow = anim.value("startRow", 0);
            int startCol = anim.value("startCol", 0);
            int endRow = anim.value("endRow", 0);
            int endCol = anim.value("endCol", 0);

            float fps = anim.value("fps", 1.0f);

            // Register using the new function
            spr->RegisterAnimation(name, path, rows, cols, startRow, startCol, endRow, endCol, fps);
        }
    }

    // 4. Start playing the default animation
    std::string defaultAnim = s.value("defaultAnimation", "");
    if (!defaultAnim.empty())
    {
        spr->PlayAnimation(defaultAnim);
    }
    std::cout << "collected SpriteAnimation" << std::endl;
}
/**
 * @brief Deserializes PhysicsComponent data from JSON and attaches it to a GameObject.
 * @param j JSON object containing Physics data.
 * @param go Target GameObject to apply data to.
 */
void ComponentSerialization::DeserializePhysics(const json &j, GameObject *go)
{
    if (!j.contains("PhysicsComponent"))
        return;

    const auto &p = j["PhysicsComponent"];
    auto *pc = go->AddComponent<PhysicsComponent>();

    pc->SetStatic(p.value("isStatic", false));
    pc->SetGravity(p.value("useGravity", true));
    pc->SetMass(p.value("mass", 1.0f));

    if (p.contains("velocity"))
        pc->SetVelocity(p["velocity"][0], p["velocity"][1]);

    pc->SetGrounded(p.value("isGrounded", false));
}

/**
 * @brief Deserializes ColliderComponent data from JSON and attaches it to a GameObject.
 * @param j JSON object containing Collider data.
 * @param go Target GameObject to apply data to.
 */
void ComponentSerialization::DeserializeCollider(const json &j, GameObject *go)
{
    if (!j.contains("ColliderComponent"))
        return;
    const auto &c = j["ColliderComponent"];

    ColliderComponent::Type type = EnumConverters::ToColliderType(c.value("shape", "Square"));
    CollisionLayer layer = EnumConverters::ToCollisionLayer(c.value("layer", "Default"));

    float width, height;
    if (j.contains("TransformComponent"))
    {
        const auto &t = j["TransformComponent"];
        if (t.contains("scale") && t["scale"].size() >= 2)
        {
            width = t["scale"][0].get<float>();
            height = t["scale"][1].get<float>();
        }
    }

    auto *collider = go->AddComponent<ColliderComponent>(type, width, height, layer);
}

/**
 * @brief Deserializes TextComponent data from JSON and attaches it to a GameObject.
 * @param j JSON object containing Text data.
 * @param go Target GameObject to apply data to.
 */
void ComponentSerialization::DeserializeText(const json &j, GameObject *go)
{
    if (!j.contains("TextComponent"))
    {
        return;
    }

    const auto &t = j["TextComponent"];

    std::string fontPath = t.value("fontPath", "../Assets/FrederickatheGreat-Regular.ttf");
    int fontSize = t.value("fontSize", 48);
    std::string text = t.value("text", "");

    vec4 color(
        t["color"][0].get<float>(),
        t["color"][1].get<float>(),
        t["color"][2].get<float>(),
        t["color"][3].get<float>());

    bool screenSpace = t.value("screenSpace", true);

    auto *textComp = go->AddComponent<TextComponent>(
        fontPath.c_str(),
        fontSize,
        text,
        color,
        screenSpace);

    textComp->SetScale(t.value("scale", 1.f));
}

/**
 * @brief Deserializes CameraComponent data from JSON and attaches it to a GameObject.
 * @param j JSON object containing Camera data.
 * @param go Target GameObject to apply data to.
 */
void ComponentSerialization::DeserializeCamera(const json &j, GameObject *go)
{
    if (!j.contains("CameraComponent"))
    {
        return;
    }

    const auto &c = j["CameraComponent"];

    auto *cam = go->AddComponent<CameraComponent>();

    std::string modeStr = c.value("mode", "FIXED");
    if (modeStr == "FIXED")
    {
        cam->SetMode(CameraComponent::CameraMode::FIXED);

        if (c.contains("position"))
        {
            cam->SetPosition(
                c["position"][0].get<float>(),
                c["position"][1].get<float>());
        }
    }
    else if (modeStr == "FOLLOW_TARGET")
    {
        cam->SetMode(CameraComponent::CameraMode::FOLLOW_TARGET);

        std::string targetName = c.value("followTargetName", "");
        GameObject *target = GameObjectFactory::Instance().FindGameObject(targetName);
        if (target)
        {
            cam->SetFollowTarget(target);
        }
        if (c.contains("followOffset"))
        {
            cam->SetFollowOffset(vec2(
                c["followOffset"][0].get<float>(),
                c["followOffset"][1].get<float>()));
        }
    }
    bool smoothing = c.value("smoothing", true);
    float smoothingSpeed = c.value("smoothingSpeed", 2.f);
    cam->SetSmoothing(smoothing, smoothingSpeed);
}

void ComponentSerialization::DeserializeEvent(const json &j, GameObject *go)
{
    if (!j.contains("EventComponent"))
        return;

    auto *ec = go->AddComponent<EventComponent>();
    const auto &rulesJson = j["EventComponent"];

    for (const auto &item : rulesJson)
    {
        EventRule rule;
        rule.eventType = StringToEventType(item.value("eventType", "None"));
        rule.targetFilter = item.value("targetFilter", "");
        rule.action = StringToActionType(item.value("action", "None"));

        ec->rules.push_back(rule);
    }
}

// ------------------- Serialize / Deserialize All Componenents -------------------
/**
 * @brief Serializes all components attached to a GameObject.
 * @param go Pointer to the GameObject.
 * @param j JSON object to write serialized component data into.
 */
void ComponentSerialization::SerializeAllComponents(GameObject *go, json &j)
{
    if (const auto *t = go->GetComponent<TransformComponent>())
        SerializeTransform(j, t);
    if (auto *r = go->GetComponent<RenderComponent>())
        SerializeRender(j, r);
    if (auto *s = go->GetComponent<Spritesheet>())
        SerializeSpritesheet(j, s);
    if (auto *p = go->GetComponent<PhysicsComponent>())
        SerializePhysics(j, p);
    if (auto *c = go->GetComponent<ColliderComponent>())
        SerializeCollider(j, c);
    if (auto *tc = go->GetComponent<TextComponent>())
        SerializeText(j, tc);
    // if (auto* cam = go->GetComponent<CameraComponent>())  SerializeCamera(j, cam);
    if (auto *t = go->GetComponent<EventComponent>())
        SerializeEvent(j, t);
}

/**
 * @brief Deserializes all components defined in JSON and attaches them to a GameObject.
 * @param go Pointer to the GameObject to populate.
 * @param j JSON object containing serialized component data.
 */
void ComponentSerialization::DeserializeAllComponents(GameObject *go, const json &j)
{
    DeserializeTransform(j, go);
    DeserializeRender(j, go);
    DeserializeSpritesheet(j, go);
    DeserializePhysics(j, go);
    DeserializeCollider(j, go);
    DeserializeText(j, go);
    // DeserializeCamera(j, go);
    DeserializeEvent(j, go);
}
