#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "BufferStructs.h"
#include "Camera.h"
#include <memory>
class Entity
{
private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
public:
	Entity(std::shared_ptr<Mesh> _mesh);
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	void Draw(DirectX::XMFLOAT4 colorTint, Microsoft::WRL::ComPtr <ID3D11Buffer> vsConstantBuffer, Microsoft::WRL::ComPtr<ID3D11DeviceContext>	context, std::shared_ptr<Camera> camera);
};

