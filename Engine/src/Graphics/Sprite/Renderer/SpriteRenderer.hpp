#pragma once

#include<Utility/Types/EcseTypes.hpp>

namespace Ecse::Graphics
{

	class VertexBuffer;
	class IndexBuffer;
	class SpritePipeline;
	class Sprite;

	/// <summary>
	/// スプライトの描画管理
	/// </summary>
	class SpriteRenderer
	{
		/// <summary>
		/// 板ポリゴンの作成
		/// </summary>
		/// <returns></returns>
		bool CreatePolygon();
	public:
		SpriteRenderer();
		virtual ~SpriteRenderer();

		/// <summary>
		/// PSOの作成と共通バッファの構築
		/// </summary>
		/// <returns></returns>
		bool Initialize();

		/// <summary>
		/// フレーム開始の準備
		/// </summary>
		/// <param name="CmdList"></param>
		void Begin(ID3D12GraphicsCommandList* CmdList);

		/// <summary>
		/// 描画実行
		/// </summary>
		/// <param name="CmdList"></param>
		/// <param name="sprite"></param>
		void Draw(ID3D12GraphicsCommandList* CmdList, const Sprite& Sprite);

		/// <summary>
		/// フレームの終了
		/// </summary>
		/// <param name="CmdList"></param>
		void End(ID3D12GraphicsCommandList* CmdList);


		/// <summary>
		/// 後でenttのviewからspriteを取得して描画をするメソッドを作成します。
		/// </summary>

	private:
		/// <summary>
		/// 板ポリゴン用の共通リソース
		/// </summary>
		std::unique_ptr<VertexBuffer> mSharedVB;
		std::unique_ptr<IndexBuffer>  mSharedIB;

		/// <summary>
		/// Sprite用のパイプライン
		/// </summary>
		std::unique_ptr<SpritePipeline> mPipeline;
	};

}


