#include "pch.h"
#include "SpriteRenderer.hpp"
#include<Graphics/Sprite/Pipeline/SpritePipeline.hpp>
#include<Graphics/VertexBuffer/VertexBuffer.hpp>
#include<Graphics/IndexBuffer/IndexBuffer.hpp>
#include<Graphics/Sprite/Sprite.hpp>
#include<Graphics/Data/GraphicsData.hpp>

namespace Ecse::Graphics
{


	SpriteRenderer::SpriteRenderer()
		:mSharedVB(nullptr)
		, mSharedIB(nullptr)
		, mPipeline(nullptr)
	{
		mPipeline = std::make_unique<SpritePipeline>();
		mSharedVB = std::make_unique<VertexBuffer>();
		mSharedIB = std::make_unique<IndexBuffer>();
	}

	/// <summary>
	/// 板ポリゴンの作成
	/// </summary>
	/// <returns></returns>
	bool SpriteRenderer::CreatePolygon()
	{
		// 共通の１x１四角形（中心原点）
		SpriteVertex vertices[4] = {
			{ {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f}, {1,1,1,1} },
			{ { 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f}, {1,1,1,1} },
			{ {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}, {1,1,1,1} },
			{ { 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}, {1,1,1,1} }
		};

		bool ret = false;
		// VB
		ret = mSharedVB->Create(sizeof(vertices), sizeof(SpriteVertex));
		if (ret == false) return false;
		mSharedVB->Update(vertices, sizeof(vertices));

		// IB
		uint16_t indices[6] = { 0, 1, 2, 2, 1, 3 };
		ret = mSharedIB->Create(sizeof(indices), eIndexBufferFormat::Uint16);
		if (ret == false) return false;
		mSharedIB->Update(indices, sizeof(indices));

		return true;
	}

	/// <summary>
	/// PSOの作成と共通バッファの構築
	/// </summary>
	/// <returns></returns>
	bool SpriteRenderer::Initialize()
	{
		bool ret = false;
		
		// パイプライン
		ret = mPipeline->Create();
		if (ret == false)
		{
			return false;
		}

		// VBとIB作成（板ポリ）
		ret = CreatePolygon();
		if (ret == false)
		{
			return false;
		}

		return true;
	}

	/// <summary>
	/// フレーム開始の準備
	/// </summary>
	/// <param name="CmdList"></param>
	void SpriteRenderer::Begin(ID3D12GraphicsCommandList* CmdList)
	{
		// パイプラインステートのセット（1回のみ）
		CmdList->SetGraphicsRootSignature(mPipeline->GetRootSignature());
		CmdList->SetPipelineState(mPipeline->GetPipelineState());
		CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 共有バッファをバインド（1回のみ）
		mSharedVB->Set(CmdList);
		mSharedIB->Set(CmdList);
	}

	/// <summary>
	/// 描画実行
	/// </summary>
	/// <param name="CmdList"></param>
	/// <param name="sprite"></param>
	void SpriteRenderer::Draw(ID3D12GraphicsCommandList* CmdList, const Sprite& Sprite)
	{
		// 描画命令
		CmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}
}
