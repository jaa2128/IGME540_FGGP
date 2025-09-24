#include "Transform.h"

// Set Position, Rotation, Scale to 1, 1, 1
// Then Set both matrices to the identity matrix
Transform::Transform() :
	position(0, 0, 0),
	rotation(0, 0, 0),
	scale(1, 1, 1)
{
	DirectX::XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&worldInverseTransposeMatrix, DirectX::XMMatrixIdentity());

	// dirty flag to use when calling World Matrix
	dirtyMatrices = true;
}

Transform::~Transform()
{
}

void Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;

	// Matrix was changed
	dirtyMatrices = true;

}

void Transform::SetPosition(DirectX::XMFLOAT3 _position)
{
	position = _position;

	// Matrix was changed
	dirtyMatrices = true;

}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation.x = pitch;
	rotation.y = yaw;
	rotation.z = roll;

	// Matrix was changed
	dirtyMatrices = true;

}

void Transform::SetRotation(DirectX::XMFLOAT3 _rotation)
{
	rotation = _rotation;

	// Matrix was changed
	dirtyMatrices = true;

}

void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;

	// Matrix was changed
	dirtyMatrices = true;

}

void Transform::SetScale(DirectX::XMFLOAT3 _scale)
{
	scale = _scale;

	// Matrix was changed
	dirtyMatrices = true;
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
	CalculateMatrices();

	// return the world matrix
	return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	CalculateMatrices();

	// return the world inverse transpose matrix
	return worldInverseTransposeMatrix;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	// create DirectX Math Types for the Transform's Position and the offset passed in
	DirectX::XMVECTOR posVec = DirectX::XMLoadFloat3(&position);
	DirectX::XMVECTOR offsetVec = DirectX::XMVectorSet(x, y, z, 0);

	// Add them together
	posVec = DirectX::XMVectorAdd(posVec, offsetVec);

	// store the result back into the DirectX Storage type
	DirectX::XMStoreFloat3(&position, posVec);

	// Matrix was changed
	dirtyMatrices = true;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	// create DirectX Math Types for the Transform's Position and the offset passed in
	DirectX::XMVECTOR posVec = DirectX::XMLoadFloat3(&position);
	DirectX::XMVECTOR offsetVec = DirectX::XMLoadFloat3(&offset);

	// Add them together
	posVec = DirectX::XMVectorAdd(posVec, offsetVec);

	// store the result back into the DirectX Storage type
	DirectX::XMStoreFloat3(&position, posVec);

	// Matrix was changed
	dirtyMatrices = true;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	// create DirectX Math Types for the Transform's Position and the offset passed in
	DirectX::XMVECTOR rotVec = DirectX::XMLoadFloat3(&rotation);
	DirectX::XMVECTOR offsetVec = DirectX::XMVectorSet(pitch, yaw, roll, 0);

	// Add them together
	rotVec = DirectX::XMVectorAdd(rotVec, offsetVec);

	// store the result back into the DirectX Storage type
	DirectX::XMStoreFloat3(&rotation, rotVec);

	// Matrix was changed
	dirtyMatrices = true;
}

void Transform::Rotate(DirectX::XMFLOAT3 _rotation)
{
	// create DirectX Math Types for the Transform's Position and the offset passed in
	DirectX::XMVECTOR rotVec = DirectX::XMLoadFloat3(&rotation);
	DirectX::XMVECTOR offsetVec = DirectX::XMLoadFloat3(&_rotation);

	// Add them together
	rotVec = DirectX::XMVectorAdd(rotVec, offsetVec);

	// store the result back into the DirectX Storage type
	DirectX::XMStoreFloat3(&rotation, rotVec);

	// Matrix was changed
	dirtyMatrices = true;
}

void Transform::Scale(float x, float y, float z)
{
	// create DirectX Math Types for the Transform's Position and the offset passed in
	DirectX::XMVECTOR scaleVec = DirectX::XMLoadFloat3(&scale);
	DirectX::XMVECTOR scaling = DirectX::XMVectorSet(x, y, z, 0);

	// Multiply them together
	scaleVec = DirectX::XMVectorMultiply(scaleVec, scaling);

	// store the result back into the DirectX Storage type
	DirectX::XMStoreFloat3(&scale, scaleVec);

	// Matrix was changed
	dirtyMatrices = true;
}

void Transform::Scale(DirectX::XMFLOAT3 _scale)
{
	// create DirectX Math Types for the Transform's Position and the offset passed in
	DirectX::XMVECTOR scaleVec = DirectX::XMLoadFloat3(&scale);
	DirectX::XMVECTOR scaling = DirectX::XMLoadFloat3(&_scale);

	// Multiply them together
	scaleVec = DirectX::XMVectorMultiply(scaleVec, scaling);

	// store the result back into the DirectX Storage type
	DirectX::XMStoreFloat3(&scale, scaleVec);

	// Matrix was changed
	dirtyMatrices = true;
}

void Transform::CalculateMatrices() {
	// If the Matrix Change
	if (dirtyMatrices) {

		// Recalculate the translation, rotation, and scale matrices
		DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&position));
		DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
		DirectX::XMMATRIX scalingMatrix = DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&scale));

		// Calculate World Matrix first scale, then rotation, then translation
		DirectX::XMMATRIX world = DirectX::XMMatrixMultiply(XMMatrixMultiply(scalingMatrix, rotationMatrix), translationMatrix);

		// Store Math types to the storage types as well as the inverse transpose
		DirectX::XMStoreFloat4x4(&worldMatrix, world);
		DirectX::XMStoreFloat4x4(&worldInverseTransposeMatrix,
			XMMatrixInverse(0, XMMatrixTranspose(world)));

		// no longer dirty
		dirtyMatrices = false;
	}
}
