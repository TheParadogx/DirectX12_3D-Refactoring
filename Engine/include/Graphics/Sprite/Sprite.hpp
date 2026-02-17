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

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="Texture"></param>
		/// <returns>true:成功</returns>
		bool Create(Texture* Texture);

		/// <summary>
		/// 描画エントリ（GPUに上げる前まで済ませる。）
		/// </summary>
		void Draw();

		/*
		* アクセサ
		*/
		DEFINE_VEC2_PROPERTY(Positiopn, mPosition);
		DEFINE_VEC2_PROPERTY(Scale, mScale);
		DEFINE_VEC2_PROPERTY(Pivot, mPivot);

		DEFINE_PROPERTY(float, Angle, mAngle);
		DEFINE_PROPERTY(float, Intensity, mIntensity);
		DEFINE_PROPERTY(Graphics::Color, Color, mColor);

		DirectX::XMFLOAT2 GetSize() const;
		Texture* GetTexture() const;
		ConstantBuffer* GetCB() const;
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
		/// 座標
		/// </summary>
		DirectX::XMFLOAT2 mPosition;
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