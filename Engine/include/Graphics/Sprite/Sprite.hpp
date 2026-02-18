#pragma once

#include<memory>
#include<Utility/Macro/Macro.hpp>
#include<Graphics/Color/Color.hpp>
#include<Utility/Export/Export.hpp>

namespace Ecse::Graphics
{
	class Texture;
	class ConstantBuffer;

	/// <summary>
	/// 2Dとして画像を表示するクラス
	/// </summary>
	class ENGINE_API Sprite
	{
	public:
		Sprite();
		virtual ~Sprite();


	private:
		/// <summary>
		/// 色
		/// </summary>
		Color mColor;

		/// <summary>
		/// 定数バッファ
		/// </summary>
		std::unique_ptr<ConstantBuffer> mConstantBuffer;
		/// <summary>
		/// テクスチャのポインタ
		/// </summary>
		Texture* mTexture;

		/// <summary>
		/// スケール
		/// </summary>
		DirectX::XMFLOAT2 mScale;
		/// <summary>
		/// 原点（0,0）が左上
		/// </summary>
		DirectX::XMFLOAT2 mPivot;
		/// <summary>
		/// 画像の本来のサイズ
		/// </summary>
		DirectX::XMFLOAT2 mSize;
		/// <summary>
		/// 回転
		/// </summary>
		float mAngle;
		/// <summary>
		/// 光度
		/// </summary>
		float mIntensity;

	};
}