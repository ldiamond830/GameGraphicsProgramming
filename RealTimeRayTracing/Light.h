#pragma once
#define MAX_LIGHTS 128
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define MAX_LIGHT_COUNT 10
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