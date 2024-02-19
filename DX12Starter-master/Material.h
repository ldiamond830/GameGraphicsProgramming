#pragma once
#include <DirectXMath.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <d3d12.h>
#include <wrl/client.h>
class Material
{
private:
	DirectX::XMFLOAT3 colorTint;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	bool finalized = false;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	D3D12_CPU_DESCRIPTOR_HANDLE textureSRVsBySlot[4];
	D3D12_GPU_DESCRIPTOR_HANDLE finalGPUHandleForSRVs;

public:
	Material(Microsoft::WRL::ComPtr<ID3D12PipelineState>, DirectX::XMFLOAT3, DirectX::XMFLOAT2, DirectX::XMFLOAT2);
	void AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE, int);
	void FinalizeMaterial();
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState();
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleForSRVs();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();
	DirectX::XMFLOAT3 GetColorTint();
};

