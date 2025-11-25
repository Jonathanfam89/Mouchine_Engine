/* Start Header ************************************************************************/
/*!
\file       Component.h
\author     Jonathan Fam Qian Wei [100% - 19 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

/*==================================================================================
/Brief
this file is the blueprint for all components where it has the class icomponent
with this file it gives all gameobject where the component are being store
must haves like the component must haves
-Initialize();
-Update();
-Destroy();
-GetName() - the component must have a name
-enable and disable
===================================================================================*/
#pragma once
#include <string>
#include <memory>
#include <vector>
#include "imgui.h"

class GameObject;
void ReceiveDragAndDrop(std::string const& _expectedPayload,std::string const& _receivedData);

class PropertiesSlot
{
public:
    ~PropertiesSlot() { --_totalID; }

    PropertiesSlot(std::string const& displayText, std::string const& expectedPayload) : _displayText(displayText), _expectedPayload(expectedPayload), _id(++_totalID), _receivedData{} {}

    void RenderImGui()
    {
        ImGui::PushID(std::to_string(_id).c_str());
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(_displayText.c_str());
        ImGui::SameLine();

        if (ImGui::Button(_receivedData.empty() ? "None" : _receivedData.c_str(), ImVec2(200, 0)))
        {
            // do something on click?
        }
        std::string received;
        ReceiveDragAndDrop(_expectedPayload, received);

        if (!received.empty())
            _receivedData = received;
        ImGui::PopID();
    }
    void SetDisplayText(std::string const& text) { _displayText = text; }

    unsigned int GetID() const { return _id; }
    std::string GetDisplayText() const { return _displayText; }
    std::string GetExpectedPayload() const { return _expectedPayload; }
private:
    unsigned int _id;
    std::string _displayText;
    std::string _expectedPayload;
    std::string _receivedData;
    inline static unsigned int _totalID = 0;
};

// Base Component class - contains both data and behavior
class IComponent
{
public:
    /**
     * @brief Constructs the base component.
     * @param owner GameObject that owns this component.
     */
    IComponent(GameObject *owner) : owner(owner), enabled(true) {}

    /**
     * @brief Default virtual destructor for proper cleanup of derived component classes.
     */
    virtual ~IComponent() = default;

    /**
     * @brief Initializes the component's state. Called once upon component creation.
     */
    virtual void Initialize() {}

    /**
     * @brief Called every game frame to update the component's logic.
     * @param deltaTime The time elapsed since the last frame.
     */
    virtual void Update(float) {}

    /**
     * @brief Cleans up the component's resources. Called once before component deletion.
     */
    virtual void Destroy() {}

    // Component info
    /**
     * @brief Gets the name of the component.
     * @return A string representing the component's name.
     */
    virtual std::string GetName() const = 0;

    // cloning virtual
    /**
     * @brief Creates a deep copy of the component for a new GameObject clone.
     * @param NewClone The GameObject that will own the new component.
     * @return A unique pointer to the cloned component.
     */
    virtual std::unique_ptr<IComponent> Clone(GameObject *NewClone) const = 0;

    // Enable/disable component
    /**
     * @brief Sets the enabled state of the component.
     * @param enable If true, the component is enabled; otherwise, it's disabled.
     */
    void SetEnabled(bool enable) { enabled = enable; }

    /**
     * @brief Add a new drag and drop slot on the component tab in the properties tab.
     * @param displayText       Name to display.
     * @param expectedPayload   Payload type its expecting to receive.
     */
    void AddDragDropSlot(std::string const& displayText, std::string const& expectedPayload) { _propertySlot.push_back(PropertiesSlot(displayText, expectedPayload)); }
    void AddDragDropSlot(PropertiesSlot const& ps) { _propertySlot.push_back(ps); }

    std::vector<PropertiesSlot> const& GetPropertySlots() { return _propertySlot; }

    /**
     * @brief Checks if the component is currently enabled.
     * @return true if the component is enabled, false otherwise.
     */
    bool IsEnabled() const { return enabled; }

protected:
    GameObject *owner;
    bool enabled;
    std::vector<PropertiesSlot> _propertySlot;
private:
    //class PropertiesSlot;
};