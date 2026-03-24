#pragma once

namespace Ecse::System
{
    /// <summary>
    /// シーンのインターフェース
    /// 全シーンはこれを継承する
    /// </summary>
    class IScene
    {
    public:
        virtual ~IScene() = default;

        /// <summary>
        /// シーンに入るとき
        /// </summary>
        /// <param name="isResumed">
        /// true  : Pop によって上のシーンが消えて再表示された
        /// false : Push / Change による新規表示
        /// </param>
        virtual void OnEnter(bool isResumed) = 0;

        /// <summary>
        /// シーンから出るとき
        /// </summary>
        /// <param name="isPaused">
        /// true  : 上に別シーンが Push されて一時停止
        /// false : Pop / Change による完全な終了
        /// </param>
        virtual void OnExit(bool isPaused) = 0;

        /// <summary>
        /// 更新（毎フレーム）
        /// </summary>
        virtual void PreUpdate(float dt) {}
        virtual void Update(float deltaTime) {};
        virtual void PostUpdate(float dt) {}

        /// <summary>
        /// 描画（毎フレーム）
        /// </summary>
        virtual void Draw() {};

        /// <summary>
        /// スタック下のシーンも描画するか
        /// true にするとポーズ画面などで下シーンが透けて見える
        /// デフォルトは false
        /// </summary>
        virtual bool IsTransparent() const { return false; }
    };
}