#pragma once
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"
class Material
{
private:
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
public:
	Material(DirectX::XMFLOAT4 _colorTint, std::shared_ptr<SimplePixelShader> _pixelShader, std::shared_ptr<SimpleVertexShader> _vertexShader);

	void SetColorTint(DirectX::XMFLOAT4 newTint);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> newPixelShader);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> newVertexShader);

	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
};

