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
			float fov = tanf( TO_RADIANS*camera.fovAngle * 0.5f);
			float cx = ((2 * (px + 0.5f) / (float)m_Width - 1) * ((float)m_Width / (float)m_Height)) * fov;
			float cy =( 1 - 2 * py / (float)m_Height )* fov;
			Vector3 cameraSpaceDir = {cx ,cy,1 };

			Ray viewRay{ camera.origin, camera.CalculateCameraToWorld().TransformVector(cameraSpaceDir) };

			ColorRGB finalColor{};
			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);
			if (closestHit.didHit) {
				ColorRGB incidentRadiance;
				Vector3 incidentLightDir;
				for (Light light : lights)
				{

					Ray lightRay{closestHit.origin,(light.origin - closestHit.origin).Normalized(),0.001f,(closestHit.origin - light.origin).Magnitude()};
					const float dot_n_lightDir = Vector3::Dot(lightRay.direction, closestHit.normal);
					ColorRGB BRDFColor = materials[closestHit.materialIndex]->Shade(closestHit, lightRay.direction, viewRay.direction.Normalized());

						if(m_ShadowsEnabled)
						{
							if(!pScene->DoesHit(lightRay))
							{
								switch (m_CurrentLightingMode)
								{
								case LightingMode::ObservedArea:
									if (dot_n_lightDir >= 0)
										finalColor += ColorRGB{1,1,1} * dot_n_lightDir;
									break;
								case LightingMode::Radiance:
									finalColor += LightUtils::GetRadiance(light, closestHit.origin);
									break;
								case LightingMode::BRDF:
									finalColor += BRDFColor;
									break;
								case LightingMode::Combined:
									if (dot_n_lightDir >= 0)
										finalColor += BRDFColor * LightUtils::GetRadiance(light, closestHit.origin) * dot_n_lightDir;
									break;
								}
							}
						}
						else
						{
							switch (m_CurrentLightingMode)
							{
							case LightingMode::ObservedArea:
								if (dot_n_lightDir >= 0)
									finalColor += ColorRGB{ 1,1,1 } * dot_n_lightDir;
								break;
							case LightingMode::Radiance:
								finalColor += LightUtils::GetRadiance(light, closestHit.origin);
								break;
							case LightingMode::BRDF:
								finalColor += BRDFColor;
								break;
							case LightingMode::Combined:
								if (dot_n_lightDir >= 0)
									finalColor += BRDFColor * LightUtils::GetRadiance(light, closestHit.origin) * dot_n_lightDir;
								break;
							}
						}
					
				}
				finalColor.MaxToOne();
			}

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightMode()
{
	switch (m_CurrentLightingMode)
	{
	case LightingMode::Combined:
		m_CurrentLightingMode = LightingMode::BRDF;
		std::cout << "BRDF" << std::endl;
		break;
	case LightingMode::BRDF:
		m_CurrentLightingMode = LightingMode::ObservedArea;
		std::cout << "ObservedArea" << std::endl;
		break;
	case LightingMode::ObservedArea:
		m_CurrentLightingMode = LightingMode::Radiance;
		std::cout << "Radiance" << std::endl;
		break;
	case LightingMode::Radiance:
		m_CurrentLightingMode = LightingMode::Combined;
		std::cout << "Combined" << std::endl;
		break;
	}
}
