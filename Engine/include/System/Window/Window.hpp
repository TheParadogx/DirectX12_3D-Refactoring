#pragma once
#include <Windows.h>
#include<filesystem>

#include<Utility/Export/Export.hpp>
#include<System/Service/ServiceLocator.hpp>
#include<System/Service/ServiceProvider.hpp>


namespace Ecse::System
{

	/// <summary>
	/// ウィンドウの設定
	/// </summary>
	struct WindowSetting
	{
		//	ウィンドウ名
		std::filesystem::path Title = "Ecse Engine";
		//	物理的なウィンドウサイズ（デスクトップ上）
		int Width = 1280;
		int Height = 720;
		// ゲームロジック・描画用の仮想サイズ（内部の仮想サイズ）
		int VirtualWidth = 1920;
		int VirtualHeight = 1080;
		//	フルスクにするかどうか true:フルスク
		bool IsFullScreen = false;
		//	カーソルを表示するかどうか true:表示する
		bool ShowCursor = true;
	};

	/// <summary>
	/// OSイベント系とウィンドウの管理
	/// </summary>
	class ENGINE_API Window : public ServiceProvider<Window>
	{
	protected:
		//	生成・破棄
		virtual void OnCreate() override;
		virtual void OnDestroy() override;
	public:
		/// <summary>
		/// ウィンドウの初期化
		/// </summary>
		/// <param name="Setting">生成時の設定</param>
		/// <returns>true:成功</returns>
		bool Initiliaze(const WindowSetting& Setting);

		/// <summary>
		/// メッセージループの処理
		/// </summary>
		void ProcessMessages();

		/// <summary>
		/// 終了フラグの確認
		/// </summary>
		/// <returns>true:App終了</returns>
		bool IsQuitRequested() const;

		/// <summary>
		/// ウィンドウハンドル取得
		/// </summary>
		/// <returns></returns>
		HWND GetHWND() const;

		/// <summary>
		/// カーソルの表示・非表示を変更する
		/// </summary>
		/// <param name="isVisible">true:表示する</param>
		void SetCursorVisible(bool isVisible);

		/// <summary>
		/// カーソルの表示状態かどうかの判定
		/// </summary>
		/// <returns>true:表示中</returns>
		bool IsCursorVisible()const;


		//	実際のウィンドウのサイズ
		int GetWidth() const;
		int GetHeight() const;

		//	内部の仮想ウィンドウサイズ
		int GetVirtualWidth() const;
		int GetVirtualHeight() const;

		//	仮想サイズから実サイズのスケール比
		float GetScaleX() const;
		float GetScaleY() const;
	private:

		/// <summary>
		/// ウィンドウクラス
		/// </summary>
		WNDCLASSEX mWindowClass;

		/// <summary>
		/// ウィンドウのハンドル
		/// </summary>
		HWND mHandle;

		/// <summary>
		/// 実際のウィンドウの横サイズ
		/// </summary>
		int mWidth;
		/// <summary>
		/// 実際のウィンドウの縦サイズ
		/// </summary>
		int mHeight;
		/// <summary>
		/// 仮想の横サイズ
		/// </summary>
		int mVirtualWidth;
		/// <summary>
		/// 仮想の縦サイズ
		/// </summary>
		int mVirtualHeight;

		/// <summary>
		/// ウィンドウを閉じるかどうか
		/// true:閉じる false:続行！
		/// </summary>
		bool mIsQuitRequested;

		/// <summary>
		/// カーソルを表示するかどうか。Windowsは標準で表示されるようなのでtrueから
		/// </summary>
		bool mIsCursorVisible;

	};

}

