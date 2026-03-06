#pragma once
#include<vector>
#include<string>
#include<DirectXMath.h>

namespace Ecse::Graphics {
    class FbxResource;
}

namespace Ecse::ECS {

    /// <summary>
    /// FBXのインスタンス状態を管理するコンポーネント
    /// </summary>
    struct FbxComponent {

        FbxComponent() = default;

        /// <summary>
        /// 使用するFBXリソースへの読み取り専用ポインタ
        /// </summary>
        const Graphics::FbxResource* Resource = nullptr;

        /// <summary>
        /// 現在再生中のアニメーション名（デバッグ・切り替え用）
        /// </summary>
        std::string CurrentAnimationName = "";

        /// <summary>
        /// 現在の再生時間（秒）
        /// </summary>
        float CurrentTime = 0.0f;

        /// <summary>
        /// 再生スピード（マイナスなら逆再生）
        /// </summary>
        float PlaySpeed = 1.0f;

        /// <summary>
        /// ループ設定
        /// </summary>
        bool IsLoop = true;

        /// <summary>
        /// 計算済みの最終スキニング行列配列
        /// 演算効率を重視し、ストレージ型ではなく計算型の XMMATRIX を保持
        /// </summary>
        std::vector<DirectX::XMMATRIX> BoneTransforms;

        /// <summary>
        /// リソースをセットし、ボーン数に合わせて行列配列を初期化する
        /// </summary>
        void SetResource(const Graphics::FbxResource* res);
    };
}