#include "Transform.h"
using namespace DirectX;
void Transform::SetDirectionVectors()
{
	XMVECTOR rotationQuat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
	XMVECTOR rotatedForward = XMVector3Rotate(XMVECTOR{ 0,0,1 }, rotationQuat);
	XMStoreFloat3(&forward, rotatedForward);

	XMVECTOR rotatedUp = XMVector3Rotate(XMVECTOR{ 0,1,0 }, rotationQuat);
	XMStoreFloat3(&up, rotatedUp);

	XMVECTOR rotatedRight = XMVector3Rotate(XMVECTOR{ 1,0,0 }, rotationQuat);
	XMStoreFloat3(&right, rotatedRight);
}
Transform::Transform()
{
	position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	SetDirectionVectors();

	DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&worldInverseTranspose, DirectX::XMMatrixIdentity());
}

void Transform::SetPosition(float x, float y, float z)
{
	position = DirectX::XMFLOAT3(x, y, z);
}

void Transform::SetPosition(DirectX::XMFLOAT3 _position)
{
	position = _position;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation = DirectX::XMFLOAT3(pitch, yaw, roll);
}

void Transform::SetRotation(DirectX::XMFLOAT3 _rotation)
{
	rotation = _rotation;
	SetDirectionVectors();
}

void Transform::SetScale(float x, float y, float z)
{
	scale = DirectX::XMFLOAT3(x, y, z);
}

void Transform::SetScale(DirectX::XMFLOAT3 _scale)
{
	scale = _scale;
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	return rotation;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	DirectX::XMMATRIX posMatrix = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX worldMatrix = scaleMatrix * rotMatrix * posMatrix;

	XMStoreFloat4x4(&world, worldMatrix);
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixInverse(0, XMMatrixTranspose(worldMatrix)));

	return world;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	return worldInverseTranspose;
}

DirectX::XMFLOAT3 Transform::GetForward()
{

	return forward;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	return up;
}

DirectX::XMFLOAT3 Transform::GetRight()
{
	return right;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	position = DirectX::XMFLOAT3(position.x + x, position.y + y, position.z + z);
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	position = DirectX::XMFLOAT3(position.x + offset.x, position.y + offset.y, position.z + offset.z);
}

void Transform::MoveRelative(float x, float y, float z)
{
	DirectX::XMVECTOR unrotatedDirection = { x, y, z };

	float tempX = rotation.x;
	float tempY = rotation.y;
	float tempZ = rotation.z;
	DirectX::XMVECTOR rotation = DirectX::XMQuaternionRotationRollPitchYaw(tempX, tempY, tempZ);

	DirectX::XMVECTOR rotatedDirection = DirectX::XMVector3Rotate(unrotatedDirection, rotation);

	XMVECTOR tempPosition = { position.x, position.y, position.z };
	XMVECTOR updatedPosition = tempPosition + rotatedDirection;

	DirectX::XMStoreFloat3(&position, updatedPosition);
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	rotation = DirectX::XMFLOAT3(rotation.x + pitch, rotation.y + yaw, rotation.z + roll);
	SetDirectionVectors();
}

void Transform::Rotate(DirectX::XMFLOAT3 _rotation)
{
	rotation = DirectX::XMFLOAT3(rotation.x + _rotation.x, rotation.y + _rotation.y, rotation.z + _rotation.z);
	SetDirectionVectors();
}

void Transform::Scale(float x, float y, float z)
{
	scale = DirectX::XMFLOAT3(scale.x * x, scale.y * y, scale.z * z);
}

void Transform::Scale(DirectX::XMFLOAT3 _scale)
{
	scale = DirectX::XMFLOAT3(scale.x * _scale.x, scale.y * _scale.y, scale.z * _scale.z);
}