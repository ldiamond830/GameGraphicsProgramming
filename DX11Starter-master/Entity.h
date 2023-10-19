#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "BufferStructs.h"
#include "Camera.h"
#include <memory>
#include "Material.h"
#include "Light.h"
class Entity
{
private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
public:
	Entity(std::shared_ptr<Mesh> _mesh, std::shared_ptr <Material> _material);
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Material> GetMaterial();
	void SetMaterial(std::shared_ptr <Material> _material);
	void Draw(DirectX::XMFLOAT4 colorTint, Microsoft::WRL::ComPtr<ID3D11DeviceContext>	context, std::shared_ptr<Camera> camera, Light light);
};

