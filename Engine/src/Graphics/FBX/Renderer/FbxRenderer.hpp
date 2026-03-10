#pragma once
#include <d3d12.h>
#include <memory>
#include <vector>
#include <entt/entt.hpp>

namespace Ecse::Graphics
{
	class FbxPipeline;
	class ConstantBuffer;

	/// <summary>
	/// FBXモデルのレンダリング管理
	/// </summary>
	class FbxRenderer
	{
	public:
		FbxRenderer();
		virtual ~FbxRenderer();

		bool Initialize();

		/// <summary>
		/// 描画予約の開始
		/// </summary>
		void Begin();

		/// <summary>
		/// registryを走査して描画が必要なモデルをリストに登録
		/// </summary>
		void UpdateAndDraw(entt::registry& registry);

		/// <summary>
		/// 溜まった予約リストをもとに、実際にGPUへコマンドを発行
		/// </summary>
		void End(ID3D12GraphicsCommandList* cmdList);

	private:

	};
}

