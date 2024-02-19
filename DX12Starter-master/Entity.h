#pragma once
#include <memory>
#include "Mesh.h"
#include "Transform.h"
#include "Material.h"
class Entity
{
private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Material> material;

public:
	Entity(std::shared_ptr<Mesh> _mesh);

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Material> GetMaterial();

	void SetMesh(std::shared_ptr<Mesh> _mesh);
	void SetMaterial(std::shared_ptr<Material>);

	void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	commandList);
};

