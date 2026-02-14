#pragma once

/*
* Ecsc の全体の構成フラグの管理
*/

//// デバック用機能
#if defined(_DEBUG)
    // デバッグビルドならデフォルトで有効
    #define ECSE_DEV_TOOL_ENABLED  (1)
    #define ECSE_ENABLE_ASSERT     (1)
    #define ECSE_DEBUG_DRAW_COLLISION (ECSE_DEV_TOOL_ENABLED)
#else
    // リリースビルドでも開発ツールを使いたい場合はここを (1) にする
    #define ECSE_DEV_TOOL_ENABLED  (0)
    #define ECSE_ENABLE_ASSERT     (0)
    #define ECSE_DEBUG_DRAW_COLLISION (0)
#endif