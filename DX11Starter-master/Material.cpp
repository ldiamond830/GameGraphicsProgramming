#include "Material.h"

Material::Material(DirectX::XMFLOAT3 _colorTint, std::shared_ptr<SimplePixelShader> _pixelShader, std::shared_ptr<SimpleVertexShader> _vertexShader)
{
    colorTint = _colorTint;
    pixelShader = _pixelShader;
    vertexShader = _vertexShader;
    roughness = 0.5f; // temp
}

void Material::SetColorTint(DirectX::XMFLOAT3 newTint)
{
    colorTint = newTint;
}

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> newPixelShader)
{
    pixelShader = newPixelShader;
}

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> newVertexShader)
{
    vertexShader = newVertexShader;
}

DirectX::XMFLOAT3 Material::GetColorTint()
{
    return colorTint;
}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
    return pixelShader;
}
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
    return vertexShader;
}

float Material::GetRoughness()
{
    return roughness;
}
