/* Start Header ************************************************************************/
/*!
\file       Animator.hpp
\author     Muhammad Ashfer [51% - 40 lines]
			Jonathan Fam [49% - 39 lines]
\par        m.binafzalkhan@digipen.edu
			Fam.q@digipen.edu
\date       October, 3, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "crtdbg.h"
#include "core.h"
#include "GraphicsSystem.h"
#include "RenderComponent.h"
#include "gameObject.h"
#include <filesystem>
#include <iostream>
#include <cctype>
#include <map>
#include <queue>
#include <string>

struct AnimationData {
	std::string filepath;
	int col, row;
	int startframe, endframe;
	float fps;

};


bool SupportedImageFile(const std::string& path);

/**
 * @brief Spritesheet class, derived from Components class, to get GO/ Comp IDs etc. But should not be viewed as a component (We can build upon this in the future)
 */
class Spritesheet : public IComponent
{
public:
	/**
	 * @brief Ctor, with GO ptr, and parameters specifying spritesheet details.
	 * @param textureFile Filepath for spritesheet
	 * @param intRows     Number of rows in spritesheet
	 * @param lintCols	 Number of cols in spritesheet
	 */
	Spritesheet(GameObject *owner, const char *textureFile, int numRows, int numCols);

	// ============ TO BE ADDED =========================
	Spritesheet(GameObject* owner, GLuint existingTextureID, int numRows, int numCols);

	/**
	 * @brief Ctor, with GO ptr, and parameters specifying spritesheet details.
	 * @param rhs		  reference to spritesheet
	 * @param newClone     Ptr to Game Object
	 */
	Spritesheet(Spritesheet const &rhs, GameObject *newClone);

	/**
	 * @brief Dtor. Will properly destruct the spritesheet.
	 */
	~Spritesheet();

	/**
	 * @brief Overloaded virtual functions, since derived from IComponents. (TBD if spritesheet will stay as a component in the future)
	 */
	std::string GetName() const override { return "Spritesheet"; }
	void Initialize() override {}
	void Update(float dt) override;
	void Destroy() override {}
	std::unique_ptr<IComponent> Clone(GameObject *NewClone) const override;

	/**
	 * @brief Return texture ID
	 */
	GLuint GetTextureID() const { return textureID; }

	void RegisterAnimation(const std::string& name, const std::string& path, int numRows, int numCols, int startRow, int startCol, int endRow, int endCol, float fps);

	void PlayAnimation(const std::string& name);

	void AddToQueue(const std::string& animationName);
	/**
	 * @brief Loads a new spritesheet file within the same object.
	 * @param textureFile Filepath for spritesheet
	 * @param intRows     Number of rows in spritesheet
	 * @param lintCols	 Number of cols in spritesheet
	 */
	void LoadNewSpritesheet(const char *textureFile, int numRows, int numCols);

	/**
	 * @brief Sets the animation of the object, within the same sprite sheet
	 * @param startFrameIdx     Starting frame of animation
	 * @param endFrameIdx	   End frame
	 * @param fps		       Time per frame
	 */
	void SetAnimation(int startFrameIdx, int endFrameIdx, float fps);

	/**
	 * @brief Frees up assigned data/mem for destructor (and maybe reloading if error occurs)
	 */
	void Unload();

	/**
	 * @brief Graphics related, how the spritesheet frame should stick to a 'mesh' (how to 'cut' the image from a spritesheet)
	 */
	vec2 GetUVScale() const;
	vec2 GetUVOffset() const;

	/**
	 * @brief Getters/ Setters for Rows and Cols
	 */
	int GetRows();
	void SetRows(int val);
	int GetCols();
	void SetCols(int val);

	/**
	 * @brief Getter for startFrame and amount of time per frame (frame in this context refers to spritesheet frames, not game fps)
	 */
	int GetCurrentStartFrame() const { return startFrame; }
	float GetTimePerFrame() const { return timePerFrame; }
	int GetCurrentEndFrame() const { return endFrame; }
	int GetCurrentRow() const { return currRow; }
	int GetCurrentCol() const { return currCol; } 
	const char* GetSpriteFile() const { return SpriteFile; }

	void SetCurrentFrame(int row, int col)
	{
		if (row >= 0 && row < rows && col >= 0 && col < cols)
		{
			currRow = row;
			currCol = col;
		}
	}

	const std::map<std::string, AnimationData>& GetAnimationRegistry() const {
		return animation_mList;
	}
	std::string GetCurrentAnimationName() const {
		return currentAnimName;
	}

private:
	/**
	 * @brief Private members to control the spritesheet and which frames are being displayed!
	 */
	int rows, cols;			   // total rows/cols
	int currRow, currCol;	   // current row/col timer is on, WRT timePerFrame
	float uv_width, uv_height; // Width/ Height of UV (for texture)
	float timePerFrame;		   // Timer between swapping to next frame
	float currentFrameTime;	   // Current timer
	int startFrame, endFrame;  // Starting and Ending frame of specific animation

	std::map<std::string, AnimationData> animation_mList;
	std::string currentAnimName;
    std::queue<std::string> animationQueue;

	const char *SpriteFile; // File path for spritesheet
	GLuint textureID;		// Personalised ID for the texture
	float alpha;			// Transparency
	bool ownsTexture;
};
