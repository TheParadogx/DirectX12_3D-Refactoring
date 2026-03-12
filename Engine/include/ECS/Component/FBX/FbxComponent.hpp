#pragma once
#include<vector>
#include<string>
#include<DirectXMath.h>
#include<Utility/Export/Export.hpp>

namespace Ecse::Graphics {
    class FbxResource;
}

namespace Ecse::ECS {

    /// <summary>
    /// FBXのインスタンス状態を管理するコンポーネント
    /// </summary>
    struct ENGINE_API FbxComponent {

        FbxComponent() = default;

        /// <summary>
        /// 使用するFBXリソースへの読み取り専用ポインタ
        /// </summary>
        const Graphics::FbxResource* Resource = nullptr;

        /// <summary>
        /// 計算済みの最終スキニング行列配列
        /// 演算効率を重視し、ストレージ型ではなく計算型の XMMATRIX を保持
        /// </summary>
        std::vector<DirectX::XMFLOAT4X4> BoneTransforms;

        /// <summary>
        /// リソースをセットし、ボーン数に合わせて行列配列を初期化する
        /// </summary>
        void SetResource(const Graphics::FbxResource* res);

    };

    /// <summary>
	/// /アニメーション用コンポーネント
    /// </summary>
    struct ENGINE_API AnimatorComponent {
        std::string CurrentAnimationName = "";
        std::string NextAnimationName = "";

        float CurrentTime = 0.0f;
        float PlaySpeed = 1.0f;
        bool IsLoop = true;
        bool IsPlaying = true;

        float TransitionTime = 0.0f;
        float TransitionDuration = 0.2f;

        // 再生命令などのヘルパー関数
        void Play(const std::string& name, bool loop = true, float speed = 1.0f);
    };
}