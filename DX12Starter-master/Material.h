#pragma once
#include <DirectXMath.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <d3d12.h>
class Material
{
private:
	DirectX::XMFLOAT3 colorTint;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	bool finalized;
	std::shared_ptr<ID3D12PipelineState> pipelineState;
	D3D12_CPU_DESCRIPTOR_HANDLE textureSRVsBySlot[4];
	D3D12_GPU_DESCRIPTOR_HANDLE finalGPUHandleForSRVs;

public:
	Material(std::shared_ptr<ID3D12PipelineState>, DirectX::XMFLOAT3, DirectX::XMFLOAT2, DirectX::XMFLOAT2);
	void AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE, int);
	void FinalizeMaterial();
};

