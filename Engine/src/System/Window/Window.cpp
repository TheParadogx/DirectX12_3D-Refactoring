#include "pch.h"
#include<System/Window/Window.hpp>

/// <summary>
/// ウィンドウ
/// </summary>
/// <returns></returns>
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// TODO:ImGui
	// TODO:Input

    switch (msg)
    {
    case WM_CLOSE:
        DestroyWindow(hWnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

namespace Ecse::System
{
    void Window::OnCreate()
    {
        // メンバーの初期化
        mHandle = nullptr;
        mWindowClass = {};
        mWidth = 0;
        mHeight = 0;
        mVirtualWidth = 0;
        mVirtualHeight = 0;
        mIsQuitRequested = false;
        mIsCursorVisible = true;

        ECSE_LOG(ELogLevel::Log, "Window Created.");
    }

    void Window::OnDestroy()
    {
        if (mHandle != nullptr)
        {
            ::UnregisterClass(mWindowClass.lpszClassName, mWindowClass.hInstance);
            ECSE_LOG(ELogLevel::Log, "Window Deleted.");
        }
    }

    /// <summary>
    /// ウィンドウの初期化
    /// </summary>
    /// <param name="Setting">生成時の設定</param>
    /// <returns>true:成功</returns>
    bool Window::Initiliaze(const WindowSetting& Setting)
    {
        //  ウィンドウサイズ
        mWidth = Setting.Width;
        mHeight = Setting.Height;
        mVirtualWidth = Setting.VirtualWidth;
        mVirtualHeight = Setting.VirtualHeight;

        //  ウィンドウ設定
        mWindowClass.cbSize = sizeof(WNDCLASSEX);
        mWindowClass.style = CS_HREDRAW | CS_VREDRAW;
        mWindowClass.lpfnWndProc = WndProc;
        mWindowClass.cbClsExtra = 0;
        mWindowClass.cbWndExtra = 0;
        mWindowClass.hInstance = GetModuleHandle(nullptr);
        mWindowClass.hIcon = LoadIcon(nullptr, IDC_ARROW);
        mWindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        mWindowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        mWindowClass.lpszMenuName = nullptr;
        mWindowClass.lpszClassName = Setting.Title.c_str();
        mWindowClass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

        //  ウィンドウの登録
        if (::RegisterClassExW(&mWindowClass) == FALSE)
        {
            ECSE_LOG(ELogLevel::Fatal, "Failed Register WindowClass.");
            return false;
        }

        //  ウィンドウのスタイルと表示座標と領域設定
        //  フルスクにするかどうか
        DWORD style = Setting.IsFullScreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;
        int x = CW_USEDEFAULT;
        int y = CW_USEDEFAULT;

        if (Setting.IsFullScreen == true)
        {
            //  モニターの解像度を物理サイズとして上書き
            mWidth = GetSystemMetrics(SM_CXSCREEN);
            mHeight = GetSystemMetrics(SM_CYSCREEN);
            x = 0;
            y = 0;
        }
        else
        {
            //  クライアント領域(中身のこと)を指定サイズにする
            RECT wr = { 0,0,mWidth,mHeight };
            ::AdjustWindowRect(&wr, style, FALSE);
            mWidth = wr.right - wr.left;
            mHeight = wr.bottom - wr.top;
        }

        //  ウィンドウの作成
        mHandle = ::CreateWindowExW(
            0,
            mWindowClass.lpszClassName,
            Setting.Title.c_str(),
            style,
            x, y,
            mWidth, mHeight,
            nullptr,
            nullptr,
            mWindowClass.hInstance,
            nullptr
        );

        if (mHandle == nullptr)
        {
            ECSE_LOG(ELogLevel::Fatal, "Failed Create WindowHandle.");
            return false;
        }

        //  表示とフォーカス
        ::ShowWindow(mHandle, SW_SHOW);
        ::UpdateWindow(mHandle);

        ::SetForegroundWindow(mHandle);
        ::SetFocus(mHandle);

        //  カーソルの初期状態
        SetCursorVisible(Setting.ShowCursor);

        ECSE_LOG(ELogLevel::Log, "Window Created Successfully: {}x{}", mWidth, mHeight);

        return true;
    }

    /// <summary>
    /// メッセージループの処理
    /// </summary>
    void Window::ProcessMessages()
    {
        MSG msg = {};
        while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                mIsQuitRequested = true;
            }
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    /// <summary>
    /// 終了フラグの確認
    /// </summary>
    /// <returns>true:App終了</returns>
    bool Window::IsQuitRequested() const
    {
        return mIsQuitRequested;
    }

    /// <summary>
    /// ウィンドウハンドル取得
    /// </summary>
    /// <returns></returns>
    HWND Window::GetHWND()const
    {
        return mHandle;
    }

    /// <summary>
    /// カーソルの表示・非表示を変更する
    /// </summary>
    /// <param name="isVisible">true:表示する</param>
    void Window::SetCursorVisible(bool isVisible)
    {
        if (mIsCursorVisible == isVisible) return;

        mIsCursorVisible = isVisible;
        if (isVisible)
        {
            //  カーソルの数値が＋(表示)になるまで呼び出す
            while (::ShowCursor(TRUE) < 0);
        }
        else
        {
            //  カーソルの数値がー(非表示)になるまで呼び出す
            while (::ShowCursor(FALSE) >= 0);
        }

    }

    /// <summary>
    /// カーソルの表示状態かどうかの判定
    /// </summary>
    /// <returns>true:表示中</returns>
    bool Window::IsCursorVisible()const
    {
        return mIsCursorVisible;
    }

    /// <summary>
    /// 実際のウィンドウの横サイズ
    /// </summary>
    /// <returns></returns>
    int Window::GetWidth()const
    {
        return mWidth;
    }

    /// <summary>
    /// 実際のウィンドウの縦サイズ
    /// </summary>
    /// <returns></returns>
    int Window::GetHeight()const
    {
        return mHeight;
    }

    /// <summary>
    /// 内部で設定した仮想の横サイズ
    /// </summary>
    /// <returns></returns>
    int Window::GetVirtualWidth() const
    {
        return mVirtualWidth;
    }

    /// <summary>
    /// 内部で設定した仮想の縦サイズ
    /// </summary>
    /// <returns></returns>
    int Window::GetVirtualHeight() const
    {
        return mVirtualHeight;
    }

    /// <summary>
    /// 実サイズの割合と仮想サイズ（横）
    /// </summary>
    /// <returns></returns>
    float Window::GetScaleX() const
    {
        return static_cast<float>(mWidth) / mVirtualWidth;
    }

    /// <summary>
    /// 実サイズの割合と仮想サイズ（縦）
    /// </summary>
    /// <returns></returns>
    float Window::GetScaleY() const
    {
        return static_cast<float>(mHeight) / mVirtualHeight;
    }
} 