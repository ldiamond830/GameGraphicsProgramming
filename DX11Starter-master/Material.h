#pragma once
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"
class Material
{
private:
	DirectX::XMFLOAT3 colorTint;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	float roughness;
public:
	Material(DirectX::XMFLOAT3 _colorTint, std::shared_ptr<SimplePixelShader> _pixelShader, std::shared_ptr<SimpleVertexShader> _vertexShader);

	void SetColorTint(DirectX::XMFLOAT3 newTint);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> newPixelShader);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> newVertexShader);

	DirectX::XMFLOAT3 GetColorTint();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	float GetRoughness();
};

