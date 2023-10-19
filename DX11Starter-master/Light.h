#pragma once
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#include <DirectXMath.h>
struct Light {
	int type;
	DirectX::XMFLOAT3 direction;
	float range;
	DirectX::XMFLOAT3 position;
	float intensity;
	DirectX::XMFLOAT3 color;
	float spotFalloff;
	DirectX::XMFLOAT3 padding;
};