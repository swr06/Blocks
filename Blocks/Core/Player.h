#pragma once

#include <glm/glm.hpp>
#include "FPSCamera.h"
#include "World.h"

namespace Blocks
{
	class Player
	{
	public :

		Player();
		void OnUpdate(GLFWwindow* window);

		FPSCamera Camera;
	};
}