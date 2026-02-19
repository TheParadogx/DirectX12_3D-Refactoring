#pragma once
#include<DirectXMath.h>
#include<Graphics/Color/Color.hpp>
#include<Utility/Export/Export.hpp>
#include<memory>

namespace Ecse::Graphics
{
	class Texture;
	class ConstantBuffer;
}

namespace Ecse::ECS
{
	/// <summary>
	/// 描画の順番
	/// </summary>
	enum class ENGINE_API RenderLayer : int
	{
		Background = 0,
		Map = 1000,
		Character = 2000,
		Effect = 3000,
		UI = 4000,
		System = 5000
	};


	struct ENGINE_API Sprite
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
		// テクスチャリソース（参照用ポインタ）
		Graphics::Texture* Texture = nullptr;

		// 光度
		float Intensity = 1.0f;
		// 描画の順番（０に行くほど前にする）
		int Layer = static_cast<int>(RenderLayer::Character);

		// 表示フラグ（true:表示 false:非表示）
		bool IsVisible = true;

		Sprite() = default;
		Sprite(Graphics::Texture* Texture);

		/// <summary>
		/// layer設定用ヘルパー
		/// </summary>
		/// <param name="Base">種別</param>
		/// <param name="Offset">種別内のソート用</param>
		void SetLayer(RenderLayer Base, int Offset = 0);
	};
}