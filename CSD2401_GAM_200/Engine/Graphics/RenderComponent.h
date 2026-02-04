/* Start Header ************************************************************************/
/*!
\file       GraphicsSystem.h
\author     Guan Shao Jun [50%]
\co-author
\par        shaojun.g@digipen.edu,
\date       October 02 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include "Component.h"
#include <GL/glew.h>
#include "MathLib.h"
#include <iostream>

/*! @class RenderComponent
    @brief Component for rendering shapes or textured objects.
*/
class RenderComponent : public IComponent
{
public:
    /*! @enum ShapeType
        @brief Types of shapes supported by RenderComponent.
    */
    enum class ShapeType
    {
        Square, /*!< Square shape */
        Circle, /*!< Circle shape */
        Line,   /*!< Line shape */
        Point,   /*!< Point shape */
        Empty
    };

    /*! @struct LineData
        @brief Data structure for a line (start/end points and color).
    */
    struct LineData
    {
        vec2 start; /*!< Start position of the line */
        vec2 end;   /*!< End position of the line */
        vec4 color; /*!< Line color */
    };

    /*! @struct PointData
        @brief Data structure for a point (position, color, size).
    */
    struct PointData
    {
        vec2 pos;       /*!< Position of the point */
        vec4 color;     /*!< Point color */
        float size = 0; /*!< Point size */
    };

    RenderComponent(GameObject *owner)
        : IComponent(owner), shape(ShapeType::Square), textureID(0), color(1.0f, 0.0f, 0.0f, 1.0f), line(), point(), layer(0), uvScale(1.0f, 1.0f), uvOffset(0.0f, 0.0f)
    {
    }
    /*! @brief Constructor for a textured object
        @param owner The GameObject this component belongs to
        @param texID OpenGL texture ID
    */
    RenderComponent(GameObject *owner, GLuint texID)
        : IComponent(owner), shape(ShapeType::Square), textureID(texID), color(1.0f), line(), point(), layer(0), uvScale(1.0f, 1.0f), uvOffset(0.0f, 0.0f)
    {
    }

    /*! @brief Constructor for a shape
        @param owner The GameObject this component belongs to
        @param type Shape type
        @param color Shape color
    */
    RenderComponent(GameObject *owner, ShapeType type, const vec4 &color)
        : IComponent(owner), shape(type), textureID(0), color(color), line(), point(), layer(0), uvScale(1.0f, 1.0f), uvOffset(0.0f, 0.0f)
    {
    }

    /*! @brief Lifecycle override: initialize component */
    void Initialize() override {}

    /*! @brief Lifecycle override: destroy component */
    void Destroy() override {}

    /*! @brief Get the component name
        @return Name of the component
    */
    std::string GetName() const override { return "RenderComponent"; }

    /*! @brief Clone this component for a new GameObject
        @param NewClone Pointer to the new GameObject
        @return Unique pointer to the cloned component
    */
    std::unique_ptr<IComponent> Clone(GameObject *NewClone) const override;

    /*! @brief Get texture ID
        @return OpenGL texture ID
    */
    GLuint GetTextureID() const { return textureID; }

    /*! @brief Set texture ID
        @param newID New OpenGL texture ID
    */
    void SetTextureID(GLuint newID) { textureID = newID; }

    /*! @brief Gets the resolution of the texture image.
    @param mip_level The scaling of resolution to world size. Val of 0 is 1-1 resolution to world space. Nice tested value is around 2-3!
    */
    vec2 GetTextureResolution(int mip_level)
    {
        int x{}, y{};
        glBindTexture(GL_TEXTURE_2D, textureID);            // bind
        glGetTexLevelParameteriv(GL_TEXTURE_2D, mip_level, GL_TEXTURE_WIDTH, &x);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, mip_level, GL_TEXTURE_HEIGHT, &y);
        glBindTexture(GL_TEXTURE_2D, 0);                    // unbind
        return { float(x),float(y) };
    }

    /*! @brief Get shape type
        @return ShapeType of this component
    */
    ShapeType GetShapeType() const { return shape; }

    void SetShapeType(ShapeType newShape) { shape = newShape; }

    /*! @brief Get the current color
        @return Color as vec4
    */
    vec4 GetColor() const { return color; }

    /*! @brief Set the color of the shape
        @param newColor New color
    */
    void SetColor(vec4 newColor) { color = newColor; }

    /*! @brief Set alpha (transparency) of the shape
        @param newAlpha New alpha value
    */
    void setAlpha(float newAlpha) { color.w = newAlpha; } // renamed from a -> w

    /*! @brief Set line data
        @param start Start position
        @param end End position
    */
    void SetLine(const vec2 &start, const vec2 &end) { line = {start, end, color}; }

    /*! @brief Set point data
        @param pos Position of the point
        @param size Point size
    */
    void SetPoint(const vec2 &pos, float size) { point = {pos, color, size}; }

    /*! @brief Get line data
        @return LineData structure
    */
    LineData GetLine() const { return line; }

    /*! @brief Get point data
        @return PointData structure
    */
    PointData GetPoint() const { return point; }

    /**
     * @brief Return the shape's name as a c string
     */
    const char *GetShapeTypeName()
    {
        switch (static_cast<int>(GetShapeType())) // Return corresponding name depending on object's shape enum
        {
        case static_cast<int>(ShapeType::Square):
            return "Square";
        case static_cast<int>(ShapeType::Circle):
            return "Circle";
        case static_cast<int>(ShapeType::Line):
            return "Line";
        case static_cast<int>(ShapeType::Point):
            return "Point";
        default:
            return "Square";
        }

    }

    int GetLayer() const { return layer; };
    void SetLayer(int l) { layer = l; };
    void SetTexturePath(const std::string &path) { texturePath = path; }
    std::string GetTexturePath() const { return texturePath; }

    void SetUVScale(const vec2& scale) { uvScale = scale; }
    vec2 GetUVScale() const { return uvScale; }

    void SetUVOffset(const vec2& offset) { uvOffset = offset; }
    vec2 GetUVOffset() const { return uvOffset; }

private:
    ShapeType shape;  /*!< Shape type of this component */
    GLuint textureID; /*!< OpenGL texture ID */
    vec4 color; /*!< Color of the shape */
    int layer;
    std::string texturePath;
    vec2 uvScale;   // Size of the UV region
    vec2 uvOffset;
    LineData line;   /*!< Line data if ShapeType::Line */
    PointData point; /*!< Point data if ShapeType::Point */
};

namespace RenderComponentTools
{
    /**
     * @brief Overloaded function that converts shape type enum to string and vice versa. Overloaded for std::string and ShapeType enum!
     */

    inline RenderComponent::ShapeType ShapeEnumToString(std::string shapeName)
    {
        if (shapeName == "Square")
        {
            return RenderComponent::ShapeType::Square;
        }
        else if (shapeName == "Circle")
        {
            return RenderComponent::ShapeType::Circle;
        }
        else if (shapeName == "Line")
        {
            return RenderComponent::ShapeType::Line;
        }
        else if (shapeName == "Point")
        {
            return RenderComponent::ShapeType::Point;
        }
        else
            std::cout << "No matching shape name!\n";
        return RenderComponent::ShapeType::Point; // Replace as runtime error!
    }

    inline std::string ShapeEnumToString(RenderComponent::ShapeType shapeEnum)
    {
        switch (shapeEnum)
        {
        case RenderComponent::ShapeType::Square:
            return "Square";
            break;
        case RenderComponent::ShapeType::Circle:
            return "Circle";
            break;
        case RenderComponent::ShapeType::Line:
            return "Line";
            break;
        case RenderComponent::ShapeType::Point:
            return "Point";
            break;

        default:
            std::cout << "No matching shape enum!\n";
            break;
        }
    }

    inline void ShapeEnumToString(std::string &shapeName, RenderComponent::ShapeType &shapeEnum)
    {
        if (!shapeName.empty()) // If theres a name in shapeName, find a match and assign the enum int the corresponding shape value
        {
            if (shapeName == "Square")
            {
                shapeEnum = RenderComponent::ShapeType::Square;
            }
            else if (shapeName == "Circle")
            {
                shapeEnum = RenderComponent::ShapeType::Circle;
            }
            else if (shapeName == "Line")
            {
                shapeEnum = RenderComponent::ShapeType::Line;
            }
            else if (shapeName == "Point")
            {
                shapeEnum = RenderComponent::ShapeType::Point;
            }
            else
                std::cout << "No matching shape name!\n";
            return;
        }
        else // If string is empty, look to the enum int to assign the string the corresponding shape's name
        {
            switch (shapeEnum)
            {
            case RenderComponent::ShapeType::Square:
                shapeName = "Square";
                break;
            case RenderComponent::ShapeType::Circle:
                shapeName = "Circle";
                break;
            case RenderComponent::ShapeType::Line:
                shapeName = "Line";
                break;
            case RenderComponent::ShapeType::Point:
                shapeName = "Point";
                break;

            default:
                std::cout << "No matching shape enum!\n";
                break;
            }
        }
    }

}
