#include "Material.h"
#include <string>
Material::Material(DirectX::XMFLOAT3 _colorTint, std::shared_ptr<SimplePixelShader> _pixelShader, std::shared_ptr<SimpleVertexShader> _vertexShader)
{
    colorTint = _colorTint;
    pixelShader = _pixelShader;
    vertexShader = _vertexShader;
    roughness = 0.5f;
    textureScale = 1.0f;
    textureOffset = 0.0f;
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

void Material::SetRoughness(float newRoughness)
{
    roughness = newRoughness;
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

void Material::AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView)
{
    textureSRVs.insert({ name, shaderResourceView });
}

void Material::AddSample(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
    samplers.insert({ name, sampler });
}

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> Material::GetSamplers()
{
    return samplers;
}

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> Material::GetSRVs()
{
    return textureSRVs;
}

void Material::SetTextureScale(float newScale)
{
    textureScale = newScale;
}

void Material::SetTetxureOffset(float newOffset)
{
    textureOffset = newOffset;
}

float Material::GetTextureScale()
{
    return textureScale;
}

float Material::GetTextureOffset()
{
    return textureOffset;
}
