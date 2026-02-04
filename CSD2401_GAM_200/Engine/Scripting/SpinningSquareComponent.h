#pragma once
#include "ScriptComponentBase.h"

/**
 * @brief Component that runs the SpinningSquare C# script
 *
 * This component rotates its GameObject continuously.
 * Corresponds to the C# class "SpinningSquare" in the CSharpScript namespace.
 */
class SpinningSquareComponent : public ScriptComponentBase
{
public:
    SpinningSquareComponent(GameObject* owner) : ScriptComponentBase(owner) {}

    std::string GetName() const override
    {
        return "SpinningSquareComponent";
    }

    std::string GetScriptClassName() const override
    {
        return "SpinningSquare";
    }


    std::unique_ptr<IComponent> Clone(GameObject* newOwner) const override
    {
        auto cloned = std::make_unique<SpinningSquareComponent>(newOwner);
        return cloned;
    }
};