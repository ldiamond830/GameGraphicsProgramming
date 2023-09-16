#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> _mesh)
{
	mesh = _mesh;
	transform = std::make_shared<Transform>(Transform());
}

std::shared_ptr<Mesh> Entity::GetMesh()
{
	return mesh;
}

std::shared_ptr<Transform> Entity::GetTransform()
{
	return transform;
}

void Entity::Draw(VertexShaderExternalData vsData, Microsoft::WRL::ComPtr <ID3D11Buffer> vsConstantBuffer, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	vsData.worldMatrix = transform->GetWorldMatrix();
	//color tint is set before being passed in to have easier acess to ImGui data

	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	context->Unmap(vsConstantBuffer.Get(), 0);
	context->VSSetConstantBuffers(
		0, // Which slot (register) to bind the buffer to?
		1, // How many are we activating? Can do multiple at once
		vsConstantBuffer.GetAddressOf());

	mesh->Draw();
}
