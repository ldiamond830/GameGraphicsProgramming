#include "Camera.h"
#include "Input.h"

void Camera::UpdateViewMatrix()
{

}

void Camera::UpdateProjectionMatrix(DirectX::XMFLOAT2 aspectRatio)
{

}

Camera::Camera(DirectX::XMFLOAT2 aspectRatio, DirectX::XMFLOAT3 initialPosition, DirectX::XMFLOAT3 initialRotation, float _fov, float _nearClip, float _farClip)
{
	transform = std::make_shared<Transform>(Transform());

	transform->SetPosition(initialPosition);
	transform->SetRotation(initialRotation);
	fov = _fov;
	nearClip = _nearClip;
	farClip = _farClip;

	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

DirectX::XMFLOAT4X4 Camera::GetView()
{
	return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjection()
{
	return projectionMatrix;
}
