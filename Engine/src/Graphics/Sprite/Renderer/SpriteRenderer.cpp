#include "pch.h"
#include "SpriteRenderer.hpp"
#include<Graphics/Sprite/Pipeline/SpritePipeline.hpp>
#include<Graphics/VertexBuffer/VertexBuffer.hpp>
#include<Graphics/StructuredBuffer/StructuredBuffer.hpp>
#include<Graphics/Data/GraphicsData.hpp>
#include<Graphics/DX12/DX12.hpp>
#include<Graphics/GraphicsDescriptorHeap/GDescriptorHeapManager.hpp>
#include<Graphics/Texture/Texture.hpp>

#include<ECS/Component/Transform/TransformComponent.hpp>
#include<ECS/Component/Sprite/SpriteComponent.hpp>

namespace Ecse::Graphics
{
	/// <summary>
	/// 表示用の行列計算
	/// </summary>
	/// <param name="tr">トランスフォーム</param>
	/// <param name="sp">スプライト</param>
	/// <returns></returns>
	SpriteShaderData SpriteRenderer::CalculateShaderData(const ECS::Transform2D& tr, const ECS::Sprite& sp)
	{
		using namespace DirectX;

		SpriteShaderData data;

		// 基本サイズ
		float baseW = (sp.Size.x > 0.0f) ? sp.Size.x : sp.Texture->GetWidth();
		float baseH = (sp.Size.y > 0.0f) ? sp.Size.y : sp.Texture->GetHeight();

		// 表示倍率を適用
		float w = baseW * sp.DrawScale.x;
		float h = baseH * sp.DrawScale.y;

		// ピボット
		XMMATRIX pivotTrans = XMMatrixTranslation(
			(0.5f - sp.Pivot.x) * w,
			(sp.Pivot.y - 0.5f) * h,
			0.0f
		);

		// スケール
		XMMATRIX scale = XMMatrixScaling(w * sp.Flip.x, h * sp.Flip.y, 1.0f);

		// 回転
		XMMATRIX rot = XMMatrixRotationZ(tr.Rotation);

		// 配置
		XMMATRIX trans = XMMatrixTranslation(tr.Position.x, tr.Position.y, 0.0f);

		// 行列の合成
		XMMATRIX world = pivotTrans * scale * rot * trans;

		// ビュー・プロジェクション
		// カメラを作成したらここにカメラの行列をかける

		// 転置して格納（シェーダーの行優先に対応させるため）
		XMStoreFloat4x4(&data.WVP, XMMatrixTranspose(world));

		data.Color = sp.Color;
		data.Intensity = sp.Intensity;

		return data;
	}


	SpriteRenderer::SpriteRenderer()
		:mPipeline(nullptr)
		,mInstanceBuffer(nullptr)
		, mVB(nullptr)
		, mReservedData()
		, mDrawCalls()
	{ 
	}

	/// <summary>
	/// 初期化
	/// </summary>
	/// <returns>true:成功</returns>
	bool SpriteRenderer::Initialize()
	{

		// パイプライン
		mPipeline = std::make_unique<SpritePipeline>();
		if (mPipeline->Create() == false) return false;

		// StructuredBuffer
		mInstanceBuffer = std::make_unique<StructuredBuffer>();
		if (mInstanceBuffer->Create(sizeof(SpriteShaderData), MAX_SPRITE_COUNT) == false) return false;

		// 共通頂点データ
		SpriteVertex vertices[] = {
			{ { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } }, // 左上
			{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f } }, // 右上
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } }, // 左下
			{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } }, // 右下
		};

		// 頂点バッファ
		mVB = std::make_unique<VertexBuffer>();
		if (!mVB->Create(sizeof(vertices), sizeof(SpriteVertex))) return false;
		mVB->Update(vertices, sizeof(vertices));

		return true;
	}

	/// <summary>
	/// 前回の命令予約を削除
	/// </summary>
	void SpriteRenderer::Begin()
	{
		mReservedData.clear();
		mDrawCalls.clear();
	}

	/// <summary>
	/// データとテクスチャのGPUハンドルを受け取る 
	/// </summary>
	/// <param name="data"></param>
	/// <param name="textureHandle"></param>
	void SpriteRenderer::Draw(const SpriteShaderData& data, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle)
	{
		if (mReservedData.size() >= MAX_SPRITE_COUNT) return;

		// 同じテクスチャなら現在のバッチの個数を増やす（バッチ処理）
		if (!mDrawCalls.empty() && mDrawCalls.back().textureHandle.ptr == textureHandle.ptr)
		{
			mDrawCalls.back().instanceCount++;
		}
		else
		{
			mDrawCalls.push_back({ textureHandle, 1, static_cast<uint32_t>(mReservedData.size()) });
		}

		mReservedData.push_back(data);
	}

	/// <summary>
	/// 命令の発行 （実描画）
	/// </summary>
	/// <param name="cmdList"></param>
	void SpriteRenderer::End(ID3D12GraphicsCommandList* cmdList)
	{
		// 空なら何もしない
		if (mReservedData.empty()) return;

		// インスタンスデータの一括更新
		mInstanceBuffer->Update(mReservedData.data(), sizeof(SpriteShaderData) * mReservedData.size());

		// 基本状態のステートセット
		cmdList->SetPipelineState(mPipeline->GetPipelineState());
		cmdList->SetGraphicsRootSignature(mPipeline->GetRootSignature());

		// SRVセット
		auto heapManager = System::ServiceLocator::Get<GDescriptorHeapManager>();
		ID3D12DescriptorHeap* heaps[] = { heapManager->GetNativeHeap() };
		cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

		// 頂点バッファとインスタンスデータのバインド
		mVB->Set(cmdList, 0);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		// T0にStructuredBuffer の SRV ハンドルをセット
		cmdList->SetGraphicsRootDescriptorTable(0, mInstanceBuffer->GetGpuHandle());

		// バッチごとにDrawInstanced を発行
		for (const auto& call : mDrawCalls)
		{
			// T1 に テクスチャの SRV ハンドルをセット
			cmdList->SetGraphicsRootDescriptorTable(1, call.textureHandle);
			// オフセットを渡して適切なデータを取得
			cmdList->DrawInstanced(4, call.instanceCount, 0, call.startIndex);
		}
	}

	/// <summary>
	/// viewを使って必要なコンポーネントを持っているオブジェクトだけを描画する
	/// </summary>
	/// <param name="registry"></param>
	void SpriteRenderer::UpdateAndDraw(entt::registry& registry)
	{
		using namespace ECS;
		auto view = registry.view<Transform2D, Sprite>();
		struct RenderItem {
			const Transform2D* transform;
			const Sprite* psprite;
		};
		std::vector<RenderItem> items;
		items.reserve(view.size_hint());

		view.each([&](auto entity, Transform2D& transform, Sprite&sprite) 
			{
				if (!sprite.IsVisible || !sprite.Texture) return;
				items.push_back({ &transform ,&sprite });
			});

		// layer順にソート
		std::sort(items.begin(), items.end(), [](const RenderItem& a, const RenderItem& b) {
			return a.psprite->Layer < b.psprite->Layer;
			});


		// ソートされた順番にDrawを発行
		for (const auto& item : items) {
			// WVP行列の計算（後述）
			SpriteShaderData shaderData = CalculateShaderData(*item.transform, *item.psprite);

			// 前に作った Draw メソッドを呼ぶ
			this->Draw(shaderData, item.psprite->Texture->GetGpuHandle());
		}

	}
}
