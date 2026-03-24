#pragma once
#include <Utility/Singleton/Singleton.hpp>

#include <filesystem>
#include <string_view>
#include <unordered_map>

// ============================================================
//  AssetPathManager
//
//  仮想パス → 絶対パス 解決システム。
//
//  ルート一覧:
//    /Engine/  →  Engine/SystemAssets/   （自動探索）
//    /Game/    →  Game プロジェクトの Content フォルダ
//
//  使い方:
//    初期化（起動時に1回）:
//      AssetPathManager::GetInstance().Initialize();
//
//    パス取得:
//      ASSET_PATH("/Engine/Shader/VS_Texture.hlsl")
//      ASSET_PATH("/Game/Texture/player.png")
// ============================================================

namespace Ecse::System
{
    class AssetPathManager : public Utility::Singleton<AssetPathManager>
    {
        SINGLETON_CLASS(AssetPathManager);
    public:
        SINGLETON_ACCESSOR(AssetPathManager);

        // ----------------------------------------------------------
        //  Initialize
        //
        //  gameContentDir : /Game/ ルート。
        //                   省略時は exe の隣の "Content" フォルダを探す。
        //
        //  engineRootDir  : Engine リポジトリのルート（Engine.sln があるフォルダ）。
        //                   省略時は exe / cwd から自動探索。
        //                   ここに SystemAssets/ が存在すると想定する。
        // ----------------------------------------------------------
        void Initialize(
            const std::filesystem::path& gameContentDir = {},
            const std::filesystem::path& engineRootDir = {});

        // ----------------------------------------------------------
        //  Resolve / ResolveW
        //
        //  "/Engine/Shader/VS_Texture.hlsl"
        //      → "D:/.../Engine/SystemAssets/Shader/VS_Texture.hlsl"
        //
        //  "/Game/Texture/player.png"
        //      → "D:/.../Game/Content/Texture/player.png"
        //
        //  解決できない場合は空パスを返す（エラーログも出力）。
        // ----------------------------------------------------------
        std::filesystem::path Resolve(std::string_view virtualPath) const;
        std::wstring          ResolveW(std::string_view virtualPath) const;

        // デバッグ用: 登録済みルート一覧を標準出力へ
        void DumpRoots() const;

    private:
        // "/Engine/Shader/VS.hlsl" → root="Engine", sub="Shader/VS.hlsl"
        bool Split(std::string_view virtualPath,
            std::string& outRoot,
            std::string& outSub) const;

    private:
        // "Engine" → "D:/.../Engine/SystemAssets"
        // "Game"   → "D:/.../Game/Content"
        std::unordered_map<std::string, std::filesystem::path> mRoots;
    };

} // namespace Ecse::System

#define ASSET_PATH(vpath)   Ecse::System::AssetPathManager::GetInstance().Resolve(vpath)
#define ASSET_PATH_W(vpath) Ecse::System::AssetPathManager::GetInstance().ResolveW(vpath)