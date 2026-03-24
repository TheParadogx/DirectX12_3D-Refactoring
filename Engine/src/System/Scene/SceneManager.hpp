#pragma once
#include<Utility/Singleton/Singleton.hpp>
#include<System/Scene/IScene.hpp>
#include<System/Scene/ITransition.hpp>

#include <memory>
#include <stack>
#include <queue>

namespace Ecse::System
{

    /// <summary>
    /// シーンコマンドの種別
    /// </summary>
    enum class SceneCommandType
    {
        Push,   // 現在のシーンを残して上に積む（ポーズ画面など）
        Pop,    // 一番上のシーンを取り除く
        Change, // 一番上のシーンを差し替える
    };

    /// <summary>
    /// フレーム末尾で処理するシーン遷移コマンド
    /// </summary>
    struct SceneCommand
    {
        SceneCommandType         type;
        std::unique_ptr<IScene>  scene;      // Push / Change 時のみ有効
        std::shared_ptr<ITransition> transition; // nullptr なら演出なし
    };

    /// <summary>
    /// シーンスタックの管理
    ///
    /// 使い方:
    ///   // 起動時
    ///   SceneManager::GetInstance().Push(std::make_unique<TitleScene>());
    ///
    ///   // メインループ
    ///   SceneManager::GetInstance().Update(dt);
    ///   SceneManager::GetInstance().Draw();
    ///
    ///   // シーン内から遷移要求
    ///   SceneManager::GetInstance().RequestChange(
    ///       std::make_unique<GameScene>(),
    ///       std::make_shared<FadeTransition>(0.5f));
    /// </summary>
	class SceneManager : public Utility::Singleton<SceneManager>
	{
		SINGLETON_CLASS(SceneManager);
	public:
		SINGLETON_ACCESSOR(SceneManager);

        // ----------------------------------------------------------
        //  起動時の初回シーン登録（コマンドキューを介さず即時）
        // ----------------------------------------------------------
        void Push(std::unique_ptr<IScene> scene);

        // ----------------------------------------------------------
        //  シーン遷移要求（フレーム末尾で処理される）
        // ----------------------------------------------------------
        void RequestPush(
            std::unique_ptr<IScene>      scene,
            std::shared_ptr<ITransition> transition = nullptr);

        void RequestPop(
            std::shared_ptr<ITransition> transition = nullptr);

        void RequestChange(
            std::unique_ptr<IScene>      scene,
            std::shared_ptr<ITransition> transition = nullptr);

        // ----------------------------------------------------------
        //  メインループから毎フレーム呼ぶ
        // ----------------------------------------------------------
        void PreUpdate(float deltaTime);
        void Update(float deltaTime);
        void PostUpdate(float deltaTime);
        void Draw();

        /// <summary>
        /// シーンスタックが空か（アプリ終了判定に使う）
        /// </summary>
        bool IsEmpty() const;

    private:

        // フレーム末尾でコマンドキューを処理する
        void FlushCommands();

        // 実際のシーン切替処理
        void ExecuteCommand(SceneCommand& cmd);

    private:

        std::stack<std::unique_ptr<IScene>> mSceneStack;
        std::queue<SceneCommand>            mCommandQueue;
        std::shared_ptr<ITransition>        mCurrentTransition;

        // トランジション中にシーン切替を済ませたか
        bool mSwitchExecuted = false;
    };
}