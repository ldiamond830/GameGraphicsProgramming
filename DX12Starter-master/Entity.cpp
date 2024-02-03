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

void Entity::SetMesh(std::shared_ptr<Mesh> _mesh)
{
    mesh = _mesh;
}

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
{
    D3D12_VERTEX_BUFFER_VIEW vbView;
    D3D12_INDEX_BUFFER_VIEW ibView;

    vbView.StrideInBytes = sizeof(Vertex);
    vbView.SizeInBytes = sizeof(Vertex) * mesh->GetIndexCount(); // needs update
    vbView.BufferLocation = mesh->GetVertexBuffer()->GetGPUVirtualAddress();
    commandList->IASetVertexBuffers(0, 1, &vbView);

    ibView.Format = DXGI_FORMAT_R32_UINT;
    ibView.SizeInBytes = sizeof(unsigned int) * mesh->GetIndexCount();
    ibView.BufferLocation = mesh->GetIndexBuffer()->GetGPUVirtualAddress();
    commandList->IASetIndexBuffer(&ibView);

    // Draw
    commandList->DrawIndexedInstanced(mesh->GetIndexCount(), 1, 0, 0, 0);
}
