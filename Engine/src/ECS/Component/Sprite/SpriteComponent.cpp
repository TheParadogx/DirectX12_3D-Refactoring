#include "pch.h"
#include<ECS/Component/Sprite/SpriteComponent.hpp>
#include<Graphics/ConstantBuffer/ConstantBuffer.hpp>
#include<Graphics/Texture/Texture.hpp>
#include<Graphics/Data/GraphicsData.hpp>

namespace Ecse::ECS
{
	Sprite::Sprite(Graphics::Texture* Texture)
		:Color(Graphics::Color::White)
		, Pivot(0.5f,0.5f)
		, Size(0,0)
		, DrawScale(1,1)
		, Flip(1,1)
		, CB(nullptr)
		, Texture(nullptr)
		, Intensity(1)
		, IsVisible(true)
	{
		this->Texture = Texture;
		if (Texture != nullptr)
		{
			Size = { Texture->GetWidth(),Texture->GetHeight()};
		}
		if (CB == nullptr)
		{
			CB = std::make_unique<Graphics::ConstantBuffer>();
			CB->Create(sizeof(Graphics::SpriteShaderData));
		}
	}
}
