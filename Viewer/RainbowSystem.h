#pragma once

#include "vesper.h"

// std
#include <memory>
#include <random>
#include <vector>


VESPERENGINE_USING_NAMESPACE

class RainbowSystem 
{
public:
	RainbowSystem(float _flickerRate) : m_flickerRate{ _flickerRate } 
	{
		// initialize colors
		m_colors = {
			{.8f, .1f, .1f, 1.0f},
			{.1f, .8f, .1f, 1.0f},
			{.1f, .1f, .8f, 1.0f},
			{.8f, .8f, .1f, 1.0f},
			{.8f, .1f, .8f, 1.0f},
			{.1f, .8f, .8f, 1.0f},
		};
		m_elapsedTime = m_flickerRate;
	}

	// randomly select a color for each game object every mFlickerRate seconds
	void Update(float dt, std::vector<GameObject>& gameObjects) 
	{
		m_elapsedTime -= dt;
		if (m_elapsedTime < 0.f)
		{
			m_elapsedTime += m_flickerRate;
			std::uniform_int_distribution<int> randInt{ 0, static_cast<int>(m_colors.size()) - 1 };

			for (auto& obj : gameObjects) 
			{
				int randValue = randInt(m_rng);
				obj.m_color = m_colors[randValue];
			}
		}
	}

private:
	std::random_device rd;
	std::mt19937 m_rng{ rd() };

	std::vector<glm::vec4> m_colors;
	float m_flickerRate;
	float m_elapsedTime;
};
