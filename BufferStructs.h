#pragma once
#include <DirectXMath.h>

struct TintAndOffset {
	DirectX::XMFLOAT4 colorTint; // 4 floats = 16 bytes
	DirectX::XMFLOAT3 posOffset; // 3 floats = 12 bytes

	// Total bytes: 28 bytes
};