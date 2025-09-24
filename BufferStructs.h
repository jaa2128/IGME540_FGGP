#pragma once
#include <DirectXMath.h>

struct VertexShaderExternalData {
	DirectX::XMFLOAT4 colorTint; // 4 floats = 16 bytes
	DirectX::XMFLOAT4X4 world; // 3 floats = 12 bytes

	// Total bytes: 28 bytes
};
