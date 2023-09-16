#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "BufferStructs.h"
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
	void Draw(VertexShaderExternalData constantBuffer, Microsoft::WRL::ComPtr <ID3D11Buffer> vsConstantBuffer, Microsoft::WRL::ComPtr<ID3D11DeviceContext>	context);
};

