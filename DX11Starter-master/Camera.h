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
	float mouseLookSpeed;
	float moveSpeed;
	bool isPerspective;
	void UpdateViewMatrix();
	

public:
	Camera(float aspectRatio, DirectX::XMFLOAT3 initialPosition, DirectX::XMFLOAT3 initialRotation, float _fov, float _nearClip, float _farClip, float _mouseLookSpeed, float _moveSpeed);
	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
	void Update(float dt);
	void UpdateProjectionMatrix(float aspectRatio);
	std::shared_ptr<Transform> GetTransform();
	float GetFov();
	float GetMoveSpeed();
};

