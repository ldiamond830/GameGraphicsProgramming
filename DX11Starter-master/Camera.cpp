#include "Camera.h"
#include "Input.h"
#include "Transform.h"
using namespace DirectX;

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 transformVal = transform->GetPosition();
	XMVECTOR tempPos = XMLoadFloat3(&transformVal);
	transformVal = transform->GetForward();
	XMVECTOR tempForward = XMLoadFloat3(&transformVal);
	XMVECTOR tempUp = { 0.0, 1.0, 0.0 };

	XMStoreFloat4x4(&viewMatrix, XMMatrixLookToLH(tempPos, tempForward, tempUp));
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	XMStoreFloat4x4(&projectionMatrix, XMMatrixPerspectiveFovLH(fov, aspectRatio, nearClip, farClip));
}

Camera::Camera(float aspectRatio, DirectX::XMFLOAT3 initialPosition, DirectX::XMFLOAT3 initialRotation, float _fov, float _nearClip, float _farClip, float _mouseLookSpeed, float _moveSpeed)
{
	transform = std::make_shared<Transform>(Transform());

	transform->SetPosition(initialPosition);
	transform->SetRotation(initialRotation);
	fov = _fov;
	nearClip = _nearClip;
	farClip = _farClip;
	mouseLookSpeed = _mouseLookSpeed;
	moveSpeed = _moveSpeed;

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

void Camera::Update(float dt)
{
	Input& input = Input::GetInstance();
	if (input.KeyDown('W')) {
		transform->MoveRelative(0.0f, 0.0f,moveSpeed * dt);
	}
	else if (input.KeyDown('S')) {
		transform->MoveRelative(0.0f, 0.0f, -moveSpeed * dt);
	}
	else if (input.KeyDown('A')) {
		transform->MoveRelative(-moveSpeed * dt, 0.0f, 0.0f);
	}
	else if (input.KeyDown('D')) {
		transform->MoveRelative(moveSpeed * dt, 0.0f, 0.0f);
	}
	else if (input.KeyDown(VK_SPACE)) {
		transform->MoveAbsolute(0.0f, moveSpeed * dt, 0.0f);
	}
	else if (input.KeyDown('X')) {
		transform->MoveAbsolute(0.0f, -moveSpeed * dt, 0.0f);
	}

	if (input.MouseLeftDown()) {
		float cursorMovementX = input.GetMouseXDelta() * mouseLookSpeed;
		float cursorMovementY = input.GetMouseYDelta() * mouseLookSpeed;

		transform->Rotate(cursorMovementY, cursorMovementX, 0.0f);

		XMFLOAT3 rotation = transform->GetPitchYawRoll();

		if (rotation.x > XM_1DIV2PI) {
			rotation.x = XM_1DIV2PI;
		}
		else if (rotation.x < -XM_1DIV2PI) {
			rotation.x = -XM_1DIV2PI;
		}

		transform->SetRotation(rotation);
	}

	UpdateViewMatrix();
}
