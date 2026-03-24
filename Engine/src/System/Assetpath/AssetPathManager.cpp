#include "pch.h"
#include "AssetPathManager.hpp"

namespace Ecse::System
{
    // ------------------------------------------------------------------
    //  GetExeDir: 実行ファイルのディレクトリ
    // ------------------------------------------------------------------
    static std::filesystem::path GetExeDir()
    {
#ifdef _WIN32
        wchar_t buf[MAX_PATH]{};
        GetModuleFileNameW(nullptr, buf, MAX_PATH);
        return std::filesystem::path(buf).parent_path();
#else
        return std::filesystem::canonical("/proc/self/exe").parent_path();
#endif
    }

    // ------------------------------------------------------------------
    //  SearchUpward
    //  startDir から親方向へ辿りながら、各階層で predicate(dir) を試す。
    //  predicate が空でないパスを返したらそこで停止。
    // ------------------------------------------------------------------
    static std::filesystem::path SearchUpward(
        const std::filesystem::path& startDir,
        std::function<std::filesystem::path(const std::filesystem::path&)> predicate,
        int maxLevels = 8)
    {
        auto dir = std::filesystem::weakly_canonical(startDir);
        for (int i = 0; i < maxLevels; ++i)
        {
            auto result = predicate(dir);
            if (!result.empty())
                return result;

            auto parent = dir.parent_path();
            if (parent == dir) break;   // ドライブルートに達した
            dir = parent;
        }
        return {};
    }

    // ------------------------------------------------------------------
    //  FindEngineSystemAssets
    //
    //  探索戦略（各階層で優先順に試す）:
    //    1. dir/Engine/SystemAssets   ← 兄弟フォルダパターン
    //                                    Game/ の親から Engine/ が見える
    //    2. dir/SystemAssets          ← Engine フォルダ内から実行時
    //
    //  対象フォルダ構成:
    //    DirectX12_3D-Refactoring/
    //    ├── Engine/
    //    │   └── SystemAssets/    ← ここを見つける
    //    └── Game/                ← cwd / exe がここにある
    // ------------------------------------------------------------------
    static std::filesystem::path FindEngineSystemAssets(
        const std::filesystem::path& startDir)
    {
        return SearchUpward(startDir, [](const std::filesystem::path& dir)
            -> std::filesystem::path
            {
                auto p1 = dir / "Engine" / "Assets";
                if (std::filesystem::is_directory(p1)) return p1;

                auto p2 = dir / "Assets";
                if (std::filesystem::is_directory(p2)) return p2;

                return {};
            });
    }

    // ------------------------------------------------------------------
    //  FindGameContent
    //
    //  探索戦略:
    //    1. dir/Content
    //    2. dir/Game/Content
    // ------------------------------------------------------------------
    static std::filesystem::path FindGameContent(
        const std::filesystem::path& startDir)
    {
        return SearchUpward(startDir, [](const std::filesystem::path& dir)
            -> std::filesystem::path
            {
                auto p1 = dir / "Content";
                if (std::filesystem::is_directory(p1)) return p1;

                auto p2 = dir / "Game" / "Content";
                if (std::filesystem::is_directory(p2)) return p2;

                return {};
            });
    }

    // ------------------------------------------------------------------
    //  Initialize
    // ------------------------------------------------------------------
    void AssetPathManager::Initialize(
        const std::filesystem::path& gameContentDir,
        const std::filesystem::path& engineRootDir)
    {
        auto exeDir = GetExeDir();
        auto cwd = std::filesystem::current_path();

        // --- /Engine/ ルート ---
        std::filesystem::path engineAssets;

        if (!engineRootDir.empty())
        {
            engineAssets = engineRootDir / "SystemAssets";
        }
        else
        {
            // exe → cwd の順で探索
            // cwd = Game/  → 上に上がると DirectX12_3D-Refactoring/
            //              → Engine/SystemAssets が見える
            engineAssets = FindEngineSystemAssets(exeDir);
            if (engineAssets.empty())
                engineAssets = FindEngineSystemAssets(cwd);
        }

        if (engineAssets.empty())
        {
            std::cerr << "[AssetPathManager] WARNING: Engine/SystemAssets not found.\n"
                << "  Searched from exe : " << exeDir << "\n"
                << "  Searched from cwd : " << cwd << "\n";
        }
        else
        {
            mRoots["Engine"] = engineAssets;
        }

        // --- /Game/ ルート ---
        std::filesystem::path gameContent;

        if (!gameContentDir.empty())
        {
            gameContent = gameContentDir;
        }
        else
        {
            gameContent = FindGameContent(cwd);
            if (gameContent.empty())
                gameContent = FindGameContent(exeDir);
        }

        if (gameContent.empty())
        {
            std::cerr << "[AssetPathManager] WARNING: Game Content not found.\n";
        }
        else
        {
            mRoots["Game"] = gameContent;
        }

#ifdef _DEBUG
        DumpRoots();
#endif
    }

    // ------------------------------------------------------------------
    //  Split
    //  "/Engine/Shader/VS.hlsl" → root="Engine", sub="Shader/VS.hlsl"
    // ------------------------------------------------------------------
    bool AssetPathManager::Split(std::string_view virtualPath,
        std::string& outRoot,
        std::string& outSub) const
    {
        if (virtualPath.empty() || virtualPath[0] != '/')
            return false;

        auto rest = virtualPath.substr(1);      // "Engine/Shader/VS.hlsl"
        auto sep = rest.find('/');

        if (sep == std::string_view::npos)
        {
            outRoot = std::string(rest);
            outSub.clear();
        }
        else
        {
            outRoot = std::string(rest.substr(0, sep));   // "Engine"
            outSub = std::string(rest.substr(sep + 1));  // "Shader/VS.hlsl"
        }
        return !outRoot.empty();
    }

    // ------------------------------------------------------------------
    //  Resolve
    // ------------------------------------------------------------------
    std::filesystem::path AssetPathManager::Resolve(std::string_view virtualPath) const
    {
        std::string root, sub;

        if (!Split(virtualPath, root, sub))
            return std::filesystem::path(virtualPath);  // 生パスはそのまま返す

        auto it = mRoots.find(root);
        if (it == mRoots.end())
        {
            std::cerr << "[AssetPathManager] Unknown root: /" << root << "/\n"
                << "  Virtual path: " << virtualPath << "\n";
            return {};
        }

        auto resolved = sub.empty() ? it->second : (it->second / sub);

#ifdef _DEBUG
        if (!std::filesystem::exists(resolved))
            std::cerr << "[AssetPathManager] Path does not exist: " << resolved << "\n";
#endif

        return resolved;
    }

    // ------------------------------------------------------------------
    //  ResolveW
    // ------------------------------------------------------------------
    std::wstring AssetPathManager::ResolveW(std::string_view virtualPath) const
    {
        return Resolve(virtualPath).wstring();
    }

    // ------------------------------------------------------------------
    //  DumpRoots
    // ------------------------------------------------------------------
    void AssetPathManager::DumpRoots() const
    {
        std::cout << "[AssetPathManager] Registered roots:\n";
        for (auto& [name, path] : mRoots)
            std::cout << "  /" << name << "/  →  " << path << "\n";
    }

} // namespace Ecse::System