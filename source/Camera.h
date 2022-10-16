#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

#include <iostream>;

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			//assert(false && "Not Implemented Yet");
			Vector3 right = Vector3::Cross(Vector3::UnitY, forward);
			Vector3 up = Vector3::Cross(forward, right);

			return Matrix(right,up,forward,origin);;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//todo: W2
			//assert(false && "Not Implemented Yet");
			if(pKeyboardState[SDL_SCANCODE_W])
			{
				origin += deltaTime * forward;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin += deltaTime * -forward;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += deltaTime * right;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin += deltaTime * -right;
			}
			if (pKeyboardState[SDL_SCANCODE_Q])
			{
				fovAngle *= 1.1f;
			}
			if (pKeyboardState[SDL_SCANCODE_E])
			{
				fovAngle *= 0.9f;
			}
			float rx = - mouseX * deltaTime * 0.01f;
			float ry = -mouseY * deltaTime * 0.01f;
			Matrix mouseYRot = Matrix({ cosf(rx),0,sinf(rx) }, Vector3::UnitY, { -sinf(rx) ,0,cosf(rx) }, { 0,0,0 });
			Matrix mouseXRot = Matrix({ 1,0,0 }, { 0,cosf(ry) ,-sinf(ry) }, {0,sinf(ry) ,cosf(ry) }, { 0, 0, 0 });
			forward += mouseYRot.TransformVector(forward);
			forward += mouseXRot.TransformVector(forward);

			forward.Normalize();
		}
	};
}
