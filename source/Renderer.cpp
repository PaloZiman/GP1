//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float cx = (2 * (px + 0.5f) / (float)m_Width - 1) * ((float)m_Width / (float)m_Height);
			float cy = 1 - 2 * py / (float)m_Height;
			Vector3 cameraSpaceDir = {cx ,cy,1 };

			Ray viewRay{ camera.origin, camera.CalculateCameraToWorld().TransformVector(cameraSpaceDir) };

			ColorRGB finalColor{};
			HitRecord closestHit{};
			//Plane testPlane{ {0,-50.f,0.f},{0,1.f,0},0 };
			//GeometryUtils::HitTest_Plane(testPlane, viewRay, closestHit);
			//Sphere testSphere{ {0.f,0.f,100.f},50.f,0 };
			//if(px > m_Width/2)
			//GeometryUtils::HitTest_Sphere(testSphere, viewRay, closestHit);
			pScene->GetClosestHit(viewRay, closestHit);
			if (closestHit.didHit) {
				/*const float scaled_t = (closestHit.t - 50) / 40;
				finalColor = { scaled_t,scaled_t,scaled_t };*/
				for (Light light : lights)
				{
					Ray lightRay{ closestHit.origin,(light.origin-closestHit.origin).Normalized(),0.001f,(closestHit.origin - light.origin).Magnitude() };
					if(pScene->DoesHit(lightRay))
					{
						finalColor = materials[closestHit.materialIndex]->Shade()*0.5f;
					}
					else
					{
						finalColor = materials[closestHit.materialIndex]->Shade();
					}
				}

			}
			//ColorRGB finalColor{ (2 * (px) / (float)m_Width - 1) * ((float)m_Width / (float)m_Height),1 - 2 * py / (float)m_Height ,1 };
			
			/*float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			ColorRGB finalColor{ gradient, gradient, gradient };
			*/
			//Update Color in Buffer
			//finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
		//std::cout << (2 * (px) / (float)m_Width - 1) * ((float)m_Width / (float)m_Height) << "\n";
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
