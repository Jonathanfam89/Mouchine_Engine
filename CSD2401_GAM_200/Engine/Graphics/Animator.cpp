/* Start Header ************************************************************************/
/*!
\file       Animator.cpp
\author     Muhammad Ashfer [51% - 117 lines]
			Jonathan Fam [49% - 109 lines]
\par        m.binafzalkhan@digipen.edu
			Fam.q@digipen.edu
\date       October, 3, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Animator.hpp"

/**
 * @brief Ctor with GO, texture file path, amount of rows and cols for that spritesheet
 */
Spritesheet::Spritesheet(GameObject *owner, const char *textureFile, int numRows, int numCols)
	: IComponent(owner),
	  rows(numRows),
	  cols(numCols),
	  currRow(0),
	  currCol(0),
	  uv_width(1.0f / (float)numCols),	// Calculate 1/Cols
	  uv_height(1.0f / (float)numRows), // Calculate 1/Rows
	  timePerFrame(0.0f),
	  currentFrameTime(0.0f),
	  startFrame(0),
	  endFrame(0),
	  textureID(GraphicsSystem::LoadTexture(textureFile)),
	  SpriteFile(textureFile),
	  alpha(0),
	  ownsTexture(true)
{
}

Spritesheet::Spritesheet(GameObject *owner, GLuint existingTextureID, int numRows, int numCols)
	: IComponent(owner),
	  rows(numRows),
	  cols(numCols),
	  currRow(0),
	  currCol(0),
	  uv_width(1.0f / (float)numCols),	// Calculate 1/Cols
	  uv_height(1.0f / (float)numRows), // Calculate 1/Rows
	  timePerFrame(0.0f),
	  currentFrameTime(0.0f),
	  startFrame(0),
	  endFrame(0),
	  textureID(existingTextureID),
	  SpriteFile(0),
	  alpha(0),
	  ownsTexture(false)
{
}

/**
 * @brief Ctor with ref to existing spritesheet object, and a GO
 */
Spritesheet::Spritesheet(Spritesheet const &rhs, GameObject *NewClone)
	: IComponent(NewClone), // Assign to the new GameObject
	  rows(rhs.rows),
	  cols(rhs.cols),
	  currRow(rhs.currRow),
	  currCol(rhs.currCol),
	  uv_width(rhs.uv_width),
	  uv_height(rhs.uv_height),
	  timePerFrame(rhs.timePerFrame),
	  currentFrameTime(rhs.currentFrameTime),
	  startFrame(rhs.startFrame),
	  endFrame(rhs.endFrame),
	  textureID(rhs.textureID),
	  SpriteFile(0),
	  alpha(0),
	  ownsTexture(false)
{
}

/**
 * @brief Unique ptr for when the object is cloned!
 */
std::unique_ptr<IComponent> Spritesheet::Clone(GameObject *NewClone) const
{
	return std::make_unique<Spritesheet>(*this, NewClone);
}

/**
 * @brief Update spritesheet timers
 */
void Spritesheet::Update(float dt)
{
	if (startFrame > endFrame)
		return; // Invalid!

	currentFrameTime += dt; // Incr timers

	if (currentFrameTime >= timePerFrame) // timer has reached beyond time per frame, change frames
	{
		currentFrameTime -= timePerFrame;

		int currentFrameIndex = currRow * cols + currCol; // Readjust rows and cols
		int nextFrameIndex = currentFrameIndex + 1;

		if (nextFrameIndex > endFrame)
		{
            if (!animationQueue.empty())
            {
                std::string nextAnimation = animationQueue.front();
                animationQueue.pop();
                PlayAnimation(nextAnimation);
            }
            else
            {
                nextFrameIndex = startFrame;
            }
		}
		currRow = nextFrameIndex / cols; // Reassign new rows and cols, showing the next frame of animation
		currCol = nextFrameIndex % cols;
	}
}

void Spritesheet::AddToQueue(const std::string& animationName)
{
    animationQueue.push(animationName);
}

/**
 * @brief Get UV scale of 'mesh'. For drawing texture on
 */
vec2 Spritesheet::GetUVScale() const
{
	return vec2(uv_width, uv_height);
}

/**
 * @brief Padding for spritesheets
 */
vec2 Spritesheet::GetUVOffset() const
{
	return vec2(uv_width * currCol, uv_height * currRow);
}

/**
 * @brief Loading new spritesheet
 */
void Spritesheet::LoadNewSpritesheet(const char *textureFile, int numRows, int numCols)
{
	if (textureID != 0 && SpriteFile && strcmp(SpriteFile, textureFile) == 0)
	{
		// Just update rows/cols in case the animation changed but the sheet didn't
		rows = numRows;
		cols = numCols;
		uv_width = 1.0f / (float)numCols;
		uv_height = 1.0f / (float)numRows;

		// Reset frames logic
		currRow = 0;
		currCol = 0;
		startFrame = 0;
		endFrame = 0;
		return;
	}

	//  Unload only if we own it 
	if (ownsTexture && textureID)
	{
		Unload();
	}

	// Use Resource Manager to get the texture
	TextureResource* tex = ResourceManager::Instance().LoadTexture(textureFile, textureFile);

	if (tex)
	{
		textureID = tex->GetTextureID();
	}
	else
	{
		// Fallback
		textureID = GraphicsSystem::LoadTexture(textureFile);
	}

	// Update Tracking Variables
	SpriteFile = textureFile;
	ownsTexture = false; // Manager owns it now

	// Update UVs
	rows = numRows;
	cols = numCols;
	uv_width = 1.0f / (float)numCols;
	uv_height = 1.0f / (float)numRows;

	currRow = 0;
	currCol = 0;
	startFrame = 0;
	endFrame = 0;

	if (auto* render = owner->GetComponent<RenderComponent>())
	{
		render->SetTextureID(textureID);
	}
}

/**
 * @brief Set new animation
 */
void Spritesheet::SetAnimation(int startFrameIdx, int endFrameIdx, float fps)
{
	startFrame = startFrameIdx; // Set starting and ending frames and time between frames. (We're still on the existing spritesheet)
	endFrame = endFrameIdx;
	timePerFrame = 1.0f / fps;
	currentFrameTime = 0.0f;

	// Set initial frames to start frame
	currRow = startFrame / cols;
	currCol = startFrame % cols;
}

/**
 * @brief Dtor
 */
Spritesheet::~Spritesheet()
{
	if (ownsTexture)
	{
		Unload();
	}
}

/**
 * @brief Free memory!
 */
void Spritesheet::Unload()
{
	if (textureID)
	{
		glDeleteTextures(1, &textureID); // Delete texture and remove texture ID
		textureID = 0;
	}
}

/**
 * @brief Getter/ Setter for rows
 */
int Spritesheet::GetRows()
{
	return rows;
}
void Spritesheet::SetRows(int val)
{
	if (val < 0 || val >= rows) // Invalid range
	{
		_CrtDbgBreak();
	}
	currRow = val;
}

/**
 * @brief Getter/ Setter for Cols
 */
int Spritesheet::GetCols()
{
	return cols;
}
void Spritesheet::SetCols(int val)
{
	if (val < 0 || val >= cols) // Invalid range
	{
		_CrtDbgBreak();
	}
	currCol = val;
}

bool SupportedImageFile(const std::string& path) {

	std::filesystem::path p(path);
	std::string ext = p.extension().string();

	std::transform(ext.begin(), ext.end(), ext.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

	return (ext == ".png" || ext == ".jpeg");
}

void Spritesheet::RegisterAnimation(const std::string& name, const std::string& path, int numRows, int numCols, int startRow, int startCol, int endRow, int endCol, float fps)
{
	
	if (startRow >= numRows || startCol >= numCols || endRow >= numRows || endCol >= numCols) {
		std::cerr << "Error: Animation " << name << " has out of bounds coordinates!" << std::endl;
		return; 
	}

	int startIdx = (startRow * numCols) + startCol;
	int endIdx = (endRow * numCols) + endCol;

	AnimationData newState;
	newState.filepath = path;
	newState.row = numRows;
	newState.col = numCols; // This is the Total columns of the sheet
	newState.startframe = startIdx;
	newState.endframe = endIdx;
	newState.fps = fps;

	animation_mList[name] = newState;
}
void Spritesheet::PlayAnimation(const std::string& name)
{
	if (currentAnimName == name) return;

	// Check if this animation name exists in our registry
	auto it = animation_mList.find(name);
	if (it == animation_mList.end()) {
		return;
	}

	// Get the data we stored earlier
	const AnimationData& state = it->second;

	
	if (std::string(GetSpriteFile()) != state.filepath) {
		LoadNewSpritesheet(state.filepath.c_str(), state.row, state.col);
	}

	SetAnimation(state.startframe, state.endframe, state.fps);

	// Update current name
	currentAnimName = name;
}