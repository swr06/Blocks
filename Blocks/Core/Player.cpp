#include "Player.h"

namespace Blocks
{
	Player::Player() : Camera(60.0f, 800.0f / 600.0f, 0.1f, 1000.0f)
	{

	}

	bool Player::OnUpdate(GLFWwindow* window)
	{
		bool moved = false;
		float camera_speed = 0.05f;

		Camera.ResetAcceleration();

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			moved = true;

			// Take the cross product of the camera's right and up.
			glm::vec3 front = -glm::cross(Camera.GetRight(), Camera.GetUp());
			Camera.ApplyAcceleration(front * camera_speed);
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			moved = true;
			glm::vec3 back = glm::cross(Camera.GetRight(), Camera.GetUp());
			Camera.ApplyAcceleration(back * camera_speed);
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			moved = true;
			Camera.ApplyAcceleration(-(Camera.GetRight() * camera_speed));
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			moved = true;
			Camera.ApplyAcceleration(Camera.GetRight() * camera_speed);
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			moved = true;
			Camera.ApplyAcceleration(Camera.GetUp() * camera_speed);
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			moved = true;
			Camera.ApplyAcceleration(-(Camera.GetUp() * camera_speed));
		}

		Camera.OnUpdate();
		Camera.Refresh();
		PlayerViewFrustum.Update(Camera.GetViewProjection());

		return moved;
	}
}