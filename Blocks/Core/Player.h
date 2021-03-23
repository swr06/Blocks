#pragma once

#include <glm/glm.hpp>
#include "FPSCamera.h"
#include "World.h"
#include "ViewFrustum.h"

namespace Blocks
{
	Block GetWorldBlock(const glm::ivec3& block);
	Block* GetWorldBlockPtr(const glm::ivec3& block);

	class Player
	{
	public :

		Player();
		bool OnUpdate(GLFWwindow* window);

		FPSCamera Camera;
		ViewFrustum PlayerViewFrustum;
		bool TestBlockCollision(const glm::vec3& position);


		bool InWater = false;
	};
}