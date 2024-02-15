#include "Material.h"

Material::Material(std::shared_ptr<ID3D12PipelineState> _pipelineState, DirectX::XMFLOAT3 _colorTint, DirectX::XMFLOAT2 _uvScale, DirectX::XMFLOAT2 _uvOffset)
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

		for (unsigned int i = 1; i < 4; i++)
		{

		}
	}
}

