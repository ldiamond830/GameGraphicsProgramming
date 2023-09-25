#pragma once
#include "Transform.h"
#include <DirectXMath.h>
#include <memory>
class Camera
{
private:
	std::shared_ptr<Transform> transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
	float fov;
	float nearClip;
	float farClip;
	bool isPerspective;
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(DirectX::XMFLOAT2 aspectRatio);

public:
	Camera(DirectX::XMFLOAT2 aspectRatio, DirectX::XMFLOAT3 initialPosition, DirectX::XMFLOAT3 initialRotation, float _fov, float _nearClip, float _farClip);
	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
};

