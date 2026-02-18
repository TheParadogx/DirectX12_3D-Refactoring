#pragma once
#include<DirectXMath.h>
#include<Graphics/Color/Color.hpp>
#include<memory>

namespace Ecse::Graphics
{
	class Texture;
	class ConstantBuffer;
}

namespace Ecse::ECS
{
	struct Sprite
	{
		// 色
		Graphics::Color Color = Graphics::Color::White;
		// 基準点
		DirectX::XMFLOAT2 Pivot = {0.5,0.5};
		// サイズ
		DirectX::XMFLOAT2 Size = {0,0};
		// 表示倍率
		DirectX::XMFLOAT2 DrawScale = {1.0f,1.0f};
		// 反転
		DirectX::XMFLOAT2 Flip = {1.0f,1.0f };
		// 定数バッファ
		std::unique_ptr<Graphics::ConstantBuffer> CB = nullptr;
		// テクスチャリソース（参照用ポインタ）
		Graphics::Texture* Texture = nullptr;
		// 光度
		float Intensity = 1.0f;
		// 表示フラグ（true:表示 false:非表示）
		bool IsVisible = true;

		Sprite() = default;
		Sprite(Graphics::Texture* Texture);
	};
}