#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> _mesh, std::shared_ptr <Material> _material)
{
	mesh = _mesh;
	material = _material;
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

std::shared_ptr<Material> Entity::GetMaterial()
{
	return material;
}

void Entity::SetMaterial(std::shared_ptr<Material> _material)
{
	material = _material;
}

void Entity::Draw(DirectX::XMFLOAT4 _colorTint, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera)
{
	//VertexShaderExternalData vsData;
	//vsData.colorTint = _colorTint;
	//vsData.worldMatrix = transform->GetWorldMatrix();
	//vsData.viewMatrix = camera->GetView();
	//vsData.projectionMatrix = camera->GetProjection();
	//

	//D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	//context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	//memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	//context->Unmap(vsConstantBuffer.Get(), 0);
	//context->VSSetConstantBuffers(
	//	0, // Which slot (register) to bind the buffer to?
	//	1, // How many are we activating? Can do multiple at once
	//	vsConstantBuffer.GetAddressOf());
	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();
	ps->SetFloat4("colorTint", material->GetColorTint()); // Strings here MUST
	vs->SetMatrix4x4("world", transform->GetWorldMatrix()); // match variable
	vs->SetMatrix4x4("view", camera->GetView()); // names in your
	vs->SetMatrix4x4("projection", camera->GetProjection()); // shader’s cbuffer!

	vs->CopyAllBufferData();
	ps->CopyAllBufferData();
	mesh->Draw();
}
