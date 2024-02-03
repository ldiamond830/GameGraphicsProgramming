#pragma once
#include <memory>
#include "Mesh.h"
#include "Transform.h"
class Entity
{
private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;

public:
	Entity(std::shared_ptr<Mesh> _mesh);

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();

	void SetMesh(std::shared_ptr<Mesh> _mesh);

	void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	commandList);
};

