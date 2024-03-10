#include "Material.h"
#include "DX12Helper.h"
Material::Material(Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState, DirectX::XMFLOAT3 _colorTint, DirectX::XMFLOAT2 _uvScale, DirectX::XMFLOAT2 _uvOffset)
{
	pipelineState = _pipelineState;
	colorTint = _colorTint;
	uvScale = _uvScale;
	uvOffset = _uvOffset;
}

void Material::AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE srv, int slot)
{
	textureSRVsBySlot[slot] = srv;
}

void Material::FinalizeMaterial()
{
	if (!finalized) {
		finalGPUHandleForSRVs = DX12Helper::GetInstance().CopySRVsToDescriptorHeapAndGetGPUDescriptorHandle(textureSRVsBySlot[0], 1);

		for (unsigned int i = 1; i < 4; i++)
		{
			 DX12Helper::GetInstance().CopySRVsToDescriptorHeapAndGetGPUDescriptorHandle(textureSRVsBySlot[i], 1);
		}

		finalized = true;
	}
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> Material::GetPipelineState()
{
	return pipelineState;
}

D3D12_GPU_DESCRIPTOR_HANDLE Material::GetGPUHandleForSRVs()
{
	return finalGPUHandleForSRVs;
}

DirectX::XMFLOAT2 Material::GetUVScale()
{
	return uvScale;
}

DirectX::XMFLOAT2 Material::GetUVOffset()
{
	return uvOffset;
}

DirectX::XMFLOAT3 Material::GetColorTint()
{
	return colorTint;
}

