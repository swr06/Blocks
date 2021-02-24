#include "Player.h"

namespace Blocks
{
	Player::Player() : Camera(60.0f, 800.0f / 600.0f, 0.1f, 1000.0f)
	{

	}

	void Player::OnUpdate(GLFWwindow* window)
	{
		float camera_speed = 0.05f;

		Camera.ResetAcceleration();

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			// Take the cross product of the camera's right and up.
			glm::vec3 front = -glm::cross(Camera.GetRight(), Camera.GetUp());
			Camera.ApplyAcceleration(front * camera_speed);
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			glm::vec3 back = glm::cross(Camera.GetRight(), Camera.GetUp());
			Camera.ApplyAcceleration(back * camera_speed);
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			Camera.ApplyAcceleration(-(Camera.GetRight() * camera_speed));
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			Camera.ApplyAcceleration(Camera.GetRight() * camera_speed);
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			Camera.ApplyAcceleration(Camera.GetUp() * camera_speed);
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			Camera.ApplyAcceleration(-(Camera.GetUp() * camera_speed));
		}

		Camera.OnUpdate();
		Camera.Refresh();
	}
}