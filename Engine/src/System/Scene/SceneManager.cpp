#include "pch.h"
#include "SceneManager.hpp"
#include<ECS/Entity/EntityManager.hpp>
namespace Ecse::System
{
    // ------------------------------------------------------------------
    //  Push（起動時専用・即時）
    // ------------------------------------------------------------------
    void SceneManager::Push(std::unique_ptr<IScene> scene)
    {
        scene->OnEnter(false);
        mSceneStack.push(std::move(scene));
    }

    // ------------------------------------------------------------------
    //  遷移要求（フレーム末尾で処理）
    // ------------------------------------------------------------------
    void SceneManager::RequestPush(
        std::unique_ptr<IScene>      scene,
        std::shared_ptr<ITransition> transition)
    {
        SceneCommand cmd;
        cmd.type = SceneCommandType::Push;
        cmd.scene = std::move(scene);
        cmd.transition = std::move(transition);
        mCommandQueue.push(std::move(cmd));
    }

    void SceneManager::RequestPop(std::shared_ptr<ITransition> transition)
    {
        SceneCommand cmd;
        cmd.type = SceneCommandType::Pop;
        cmd.transition = std::move(transition);
        mCommandQueue.push(std::move(cmd));
    }

    void SceneManager::RequestChange(
        std::unique_ptr<IScene>      scene,
        std::shared_ptr<ITransition> transition)
    {
        SceneCommand cmd;
        cmd.type = SceneCommandType::Change;
        cmd.scene = std::move(scene);
        cmd.transition = std::move(transition);
        mCommandQueue.push(std::move(cmd));
    }

    // ------------------------------------------------------------------
    //  Update
    // ------------------------------------------------------------------
    void SceneManager::PreUpdate(float deltaTime)
    {
        // --- 通常Update ---
        if (!mSceneStack.empty())
        {
            mSceneStack.top()->PreUpdate(deltaTime);
        }
    }

    void SceneManager::Update(float deltaTime)
    {
        // --- 通常Update ---
        if (!mSceneStack.empty())
        {
            mSceneStack.top()->Update(deltaTime);
        }

    }

    void SceneManager::PostUpdate(float deltaTime)
    {
        // --- トランジション中 ---
        if (mCurrentTransition)
        {
            mCurrentTransition->Update(deltaTime);

            // alpha = 1.0（完全に隠れた）瞬間にシーン切替を実行
            if (!mSwitchExecuted && mCurrentTransition->IsReadyToSwitch())
            {
                if (!mCommandQueue.empty())
                {
                    ExecuteCommand(mCommandQueue.front());
                    mCommandQueue.pop();
                }
                mSwitchExecuted = true;
            }

            // 演出終了でトランジション解放
            if (mCurrentTransition->IsFinished())
            {
                mCurrentTransition = nullptr;
                mSwitchExecuted = false;
            }

            return; // トランジション中は新しいコマンドを受け付けない
        }

        // --- 通常Update ---
        if (!mSceneStack.empty())
        {
            mSceneStack.top()->PostUpdate(deltaTime);
        }

        auto em = System::ServiceLocator::Get<ECS::EntityManager>();
        if (em) {
            em->Update(); // ここで mDestroyQueue を処理
        }

        // フレーム末尾でコマンド処理
        FlushCommands();
    }

    // ------------------------------------------------------------------
    //  Draw
    // ------------------------------------------------------------------
    void SceneManager::Draw()
    {
        if (mSceneStack.empty()) return;

        // unique_ptr はコピー不可なので raw pointer で描画リストを作る
        // stack の内部コンテナ（deque）を bottom → top の順に走査する
        std::vector<IScene*> drawList;
        drawList.reserve(mSceneStack.size());

        // std::stack の内部は c() でアクセスできないため
        // 一時的に top → bottom へ pop して raw ptr を積み、後で戻す
        std::vector<std::unique_ptr<IScene>> temp;
        temp.reserve(mSceneStack.size());

        while (!mSceneStack.empty())
        {
            temp.push_back(std::move(mSceneStack.top()));
            mSceneStack.pop();
        }
        // temp[0] = 元の top（最前面）、temp[last] = 元の bottom

        // 描画対象を選出（top から走査して IsTransparent でない所まで）
        for (auto& s : temp)
        {
            drawList.push_back(s.get());
            if (!s->IsTransparent()) break;
        }

        // スタックを元に戻す（bottom → top の順で push）
        for (int i = static_cast<int>(temp.size()) - 1; i >= 0; --i)
        {
            mSceneStack.push(std::move(temp[i]));
        }

        // bottom → top の順（下のシーンから）描画
        for (int i = static_cast<int>(drawList.size()) - 1; i >= 0; --i)
        {
            drawList[i]->Draw();
        }

        // トランジション演出を最前面に描画
        if (mCurrentTransition)
        {
            mCurrentTransition->Draw();
        }
    }

    // ------------------------------------------------------------------
//  IsEmpty
// ------------------------------------------------------------------
    bool SceneManager::IsEmpty() const
    {
        return mSceneStack.empty();
    }

    // ------------------------------------------------------------------
    //  FlushCommands（トランジションなしのコマンドを即時処理）
    // ------------------------------------------------------------------
    void SceneManager::FlushCommands()
    {
        if (mCommandQueue.empty()) return;

        auto& cmd = mCommandQueue.front();

        // トランジションありの場合は演出を開始してフレームごとに進める
        if (cmd.transition)
        {
            mCurrentTransition = cmd.transition;
            mSwitchExecuted = false;
            // ExecuteCommand は IsReadyToSwitch() が true になってから呼ばれる
            return;
        }

        // トランジションなし → 即時切替
        ExecuteCommand(cmd);
        mCommandQueue.pop();
    }

    // ------------------------------------------------------------------
    //  ExecuteCommand（実際のシーン切替）
    // ------------------------------------------------------------------
    void SceneManager::ExecuteCommand(SceneCommand& cmd)
    {
        auto em = System::ServiceLocator::Get<ECS::EntityManager>();


        switch (cmd.type)
        {
        case SceneCommandType::Push:
        {
            // 現在シーンを一時停止
            if (!mSceneStack.empty())
                mSceneStack.top()->OnExit(true); // isPaused = true

            // ※ Push はエンティティを消さない（一時停止なので）
            cmd.scene->OnEnter(false);
            mSceneStack.push(std::move(cmd.scene));
            break;
        }
        case SceneCommandType::Pop:
        {
            if (!mSceneStack.empty())
            {
                mSceneStack.top()->OnExit(false); // isPaused = false
                if (em)
                {
                    em->ClearLocalEntities();
                }
                mSceneStack.pop();
            }

            // 下のシーンを再開
            if (!mSceneStack.empty())
                mSceneStack.top()->OnEnter(true); // isResumed = true
            break;
        }
        case SceneCommandType::Change:
        {
            if (!mSceneStack.empty())
            {
                mSceneStack.top()->OnExit(false);
                if (em)
                {
                    em->ClearLocalEntities();
                }
                mSceneStack.pop();
            }

            cmd.scene->OnEnter(false);
            mSceneStack.push(std::move(cmd.scene));
            break;
        }
        }
    }
}