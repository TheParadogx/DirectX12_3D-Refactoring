#pragma once

#include<d3d12.h>
#include<memory>
#include<vector>
#include<entt/entt.hpp>
#include<Graphics/Data/GraphicsData.hpp>

namespace Ecse::ECS
{
	struct Transform2D;
	struct Sprite;
}

namespace Ecse::Graphics
{

	class VertexBuffer;
	class StructuredBuffer;
	class SpritePipeline;

	/// <summary>
	/// スプライトの描画管理
	/// </summary>
	class SpriteRenderer
	{
		/// <summary>
		/// 表示用の行列計算
		/// </summary>
		/// <param name="tr">トランスフォーム</param>
		/// <param name="sp">スプライト</param>
		/// <returns></returns>
		SpriteShaderData CalculateShaderData(const ECS::Transform2D& tr, const ECS::Sprite& sp);

	public:
		SpriteRenderer();
		virtual ~SpriteRenderer();


		/// <summary>
		/// 初期化
		/// </summary>
		/// <returns>true:成功</returns>
		bool Initialize();
		
		/// <summary>
		/// 前回の命令予約を削除
		/// </summary>
		void Begin();

		/// <summary>
		/// データとテクスチャのGPUハンドルを受け取る 
		/// </summary>
		/// <param name="data"></param>
		/// <param name="textureHandle"></param>
		void Draw(const SpriteShaderData& data, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

		/// <summary>
		/// 命令の発行（実描画）
		/// </summary>
		/// <param name="cmdList"></param>
		void End(ID3D12GraphicsCommandList* cmdList);

		/// <summary>
		/// viewを使って必要なコンポーネントを持っているオブジェクトだけを描画する
		/// </summary>
		/// <param name="registry"></param>
		void UpdateAndDraw(entt::registry& registry);

	private:
		/// <summary>
		/// 1フレームの最大数 
		/// </summary>
		static constexpr uint32_t MAX_SPRITE_COUNT = 4096;

		/// <summary>
		/// Sprite用パイプライン
		/// </summary>
		std::unique_ptr<SpritePipeline> mPipeline;
		/// <summary>
		/// SpriteShaderDataをシェーダに送るためのバッファ
		/// </summary>
		std::unique_ptr<StructuredBuffer> mInstanceBuffer;
		/// <summary>
		/// 頂点バッファ
		/// </summary>
		std::unique_ptr<VertexBuffer> mVB;

		/// <summary>
		/// 1回のDrawCallのデータ
		/// </summary>
		struct DrawCall {
			D3D12_GPU_DESCRIPTOR_HANDLE textureHandle;
			uint32_t instanceCount;
			uint32_t startIndex;
		};

		/// <summary>
		/// 頂点バッファに送るデータ
		/// </summary>
		std::vector<SpriteShaderData> mReservedData;
		/// <summary>
		/// DrawCallの全データ
		/// </summary>
		std::vector<DrawCall> mDrawCalls;
	};

}


