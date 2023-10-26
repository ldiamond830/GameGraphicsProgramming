#pragma once
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"
#include <unordered_map>
#include <string>
class Material
{
private:
	DirectX::XMFLOAT3 colorTint;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
	float roughness;
	float textureScale;
	float textureOffset;
public:
	Material(DirectX::XMFLOAT3 _colorTint, std::shared_ptr<SimplePixelShader> _pixelShader, std::shared_ptr<SimpleVertexShader> _vertexShader);

	void SetColorTint(DirectX::XMFLOAT3 newTint);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> newPixelShader);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> newVertexShader);

	DirectX::XMFLOAT3 GetColorTint();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	float GetRoughness();

	void AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView);
	void AddSample(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> GetSamplers();
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> GetSRVs();
	void SetTextureScale(float newScale);
	void SetTetxureOffset(float newOffset);
	float GetTextureScale();
	float GetTextureOffset();
};

