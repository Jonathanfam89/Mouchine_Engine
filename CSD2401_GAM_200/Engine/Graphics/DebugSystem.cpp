/* Start Header ************************************************************************/
/*!
\file       DebugSystem.cpp
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       October 02 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "DebugSystem.h"

void DebugSystem::Initialize() {
	// Initialize the DebugSystem (currently empty)
}

void DebugSystem::Update(float) {
	// Update the DebugSystem each frame (currently empty)
}

void DebugSystem::DrawLine(const vec2& start, const vec2& end, const vec4& color) {
	// Add a line shape to the debug draw list
	shapes.push_back({ DebugShape::LINE, start, end, 0.0f, color, 0 });
}

void DebugSystem::DrawRect(const vec2& pos, const vec2& size, const vec4& color) {
	// Add a rectangle shape to the debug draw list
	shapes.push_back({ DebugShape::RECT, pos, size, 0.0f, color, 0 });
}

void DebugSystem::DrawCircle(const vec2& center, float radius, const vec4& color, int segments) {
	// Add a circle shape to the debug draw list
	shapes.push_back({ DebugShape::CIRCLE, center, {}, radius, color, segments });
}

void DebugSystem::DrawPoint(const vec2& ppos, vec4 color) {
	// Add a point shape to the debug draw list
	shapes.push_back({ DebugShape::POINT, ppos, {}, 0.0f, color, 0 });
}

void DebugSystem::Clear() {
	shapes.clear();
}

std::string DebugSystem::GetName() {
	// Return the system name
	return "Debug System";
}
