#include "Entity.h"
#include <iostream>
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
	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();

	ps->SetFloat3("colorTint", material->GetColorTint());
	ps->SetFloat("roughness", material->GetRoughness());
	ps->SetFloat3("cameraPosition", camera->GetTransform()->GetPosition());
	for (auto& t : material->GetSRVs()) { ps->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : material->GetSamplers()) { ps->SetSamplerState(s.first.c_str(), s.second); }
	ps->SetFloat("textureScale", material->GetTextureScale());
	ps->SetFloat("textureOffset", material->GetTextureOffset());


	vs->SetMatrix4x4("world", transform->GetWorldMatrix()); 
	vs->SetMatrix4x4("worldInvTranspose", transform->GetWorldInverseTransposeMatrix());
	vs->SetMatrix4x4("view", camera->GetView());
	vs->SetMatrix4x4("projection", camera->GetProjection()); 
	
	vs->CopyAllBufferData();
	ps->CopyAllBufferData();
	mesh->Draw();
}
