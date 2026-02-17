#include "pch.h"
#include<Graphics/Sprite/Sprite.hpp>
#include<Graphics/Texture/Texture.hpp>
#include<Graphics/ConstantBuffer/ConstantBuffer.hpp>
#include<Graphics/Data/GraphicsData.hpp>

namespace Ecse::Graphics
{

    Sprite::Sprite()
        :mColor(Color::White)
        , mTexture(nullptr)
        , mConstantBuffer(nullptr)
        , mPosition(0,0)
        , mScale(1,1)
        , mPivot(0, 0)
        , mSize(-1,-1)
        , mAngle(0)
        , mIntensity(1.0)
    {

    }

    Sprite::~Sprite()
    {
        mConstantBuffer.reset();
    }

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="Texture"></param>
    /// <returns>true:成功</returns>
    bool Sprite::Create(Texture* Texture)
    {
        if (Texture == nullptr) return false;

        //  サイズ
        mSize.x = Texture->GetWidth();
        mSize.y = Texture->GetHeight();

        //   定数バッファ
        mConstantBuffer = std::make_unique<ConstantBuffer>();
        return mConstantBuffer->Create(sizeof(SpriteShaderData));
    }

    /// <summary>
    /// 描画エントリ（GPUに上げる前まで済ませる。）
    /// </summary>
    void Sprite::Draw()
    {
        if (mTexture == nullptr) return;

        // オフセット

        //  拡大

        //  回転

        //  座標移動

        //  SRT純で合成

        //  Rendererから仮想サイズのProj行列取得

        //  定数バッファの更新

        // Rednererに描画を依頼（まだ描画はしない。）
    }

    DirectX::XMFLOAT2 Ecse::Graphics::Sprite::GetSize()const
    {
        return mSize;
    }
    Texture* Ecse::Graphics::Sprite::GetTexture() const
    {
        return mTexture;
    }
    ConstantBuffer* Sprite::GetCB() const
    {
        return mConstantBuffer.get();
    }
}
