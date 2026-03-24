#pragma once

namespace Ecse::System
{
    /// <summary>
    /// シーントランジションのインターフェース
    /// フェード・クロスフェードなどの演出はこれを継承する
    /// </summary>
    class ITransition
    {
    public:
        ITransition() = default;
        virtual ~ITransition() = default;

        /// <summary>
        /// 更新（毎フレーム）
        /// </summary>
        virtual void Update(float deltaTime) = 0;

        /// <summary>
        /// 演出の描画（毎フレーム）
        /// シーン描画の上に重ねて呼ぶ
        /// </summary>
        virtual void Draw() = 0;

        /// <summary>
        /// シーンを切り替えるタイミングか
        /// オーバーレイ方式では alpha = 1.0（完全に隠れた）瞬間に true を返す
        /// SceneManager はこれが true になった時点で
        /// OnExit / ClearLocalEntities / OnEnter を実行する
        /// </summary>
        virtual bool IsReadyToSwitch() const = 0;

        /// <summary>
        /// トランジション演出が完全に終わったか
        /// true になったら SceneManager は mCurrentTransition を解放する
        /// </summary>
        virtual bool IsFinished() const = 0;
    };
}