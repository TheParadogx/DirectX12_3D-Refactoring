#include "pch.h"
#include<ECS/Component/Sprite/SpriteComponent.hpp>
#include<Graphics/ConstantBuffer/ConstantBuffer.hpp>
#include<Graphics/Texture/Texture.hpp>
#include<Graphics/Data/GraphicsData.hpp>

namespace Ecse::ECS
{
	Sprite::Sprite(Graphics::Texture* Texture)
		:Color(Graphics::Color::White)
		, Pivot(0.0f,0.0f)
		, Size(0,0)
		, DrawScale(1,1)
		, Flip(1,1)
		, Texture(nullptr)
		, Intensity(1)
		, IsVisible(true)
	{
		this->Texture = Texture;
		if (Texture != nullptr)
		{
			Size = { Texture->GetWidth(),Texture->GetHeight()};
		}
	}

	void Sprite::SetLayer(RenderLayer Base, int Offset)
	{
		Layer = static_cast<int>(Base) + Offset;
	}
}
