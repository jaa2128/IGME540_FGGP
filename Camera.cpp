#include "Camera.h"

Camera::Camera(DirectX::XMFLOAT3 position,
	float _cameraSpeed, 
	float _mouseLookSpeed,
	float _fov,
	float _aspectRatio,
	float _nearClipDistance, 
	float _farClipDistance, ProjectionType type) :
	cameraSpeed(_cameraSpeed), 
	mouseLookSpeed(_mouseLookSpeed),
	nearClipDistance(_nearClipDistance),
	farClipDistance(_farClipDistance),
	projectionType(type),
	aspectRatio(_aspectRatio),
	fov(_fov),
	orthographicWidth(10.0f)
{
	// set position of camera
	transform = std::make_shared<Transform>();
	transform->SetPosition(position);

	// update view and projection matrix
	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

Camera::~Camera()
{
}

void Camera::UpdateProjectionMatrix(float _aspectRatio)
{
	// set aspectRatio to the defined new aspectRatio
	aspectRatio = _aspectRatio;

	// Create projection Matrix 
	DirectX::XMMATRIX _projectionMatrix = DirectX::XMMatrixIdentity();

	// depending on type use different matrix/parameters
	switch (projectionType)
	{
	case PERSPECTIVE:
		_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
			fov, // Field of View Angle in Radians
			aspectRatio, // Aspect Ratio
			nearClipDistance, // Near Clip Distance
			farClipDistance // Far Clip Distance
		);
		break;
	case ORTHOGRAPHIC:
		_projectionMatrix = DirectX::XMMatrixOrthographicLH(
			orthographicWidth, // Projection width in world units
			orthographicWidth / aspectRatio, // projection height
			nearClipDistance, // Near Clip Distance
			farClipDistance // Far Clip Distance
		);
		break;
	default:
		break;
	}

	DirectX::XMStoreFloat4x4(&projectionMatrix, _projectionMatrix);
}

void Camera::UpdateViewMatrix()
{
	// get position and forward vector of camera from transform
	DirectX::XMFLOAT3 position = transform->GetPosition();
	DirectX::XMFLOAT3 forwardVec = transform->GetForward();

	// create view matrix
	DirectX::XMMATRIX _viewMatrix = DirectX::XMMatrixLookToLH(
		DirectX::XMLoadFloat3(&position),
		DirectX::XMLoadFloat3(&forwardVec),
		DirectX::XMVectorSet(0, 1, 0, 0)); // World up axis
	
	// store it
	DirectX::XMStoreFloat4x4(&viewMatrix, _viewMatrix);
}

void Camera::Update(float dt)
{
	// set a speed according to deltaTime and camera speed
	float speed = dt * cameraSpeed;

	// Key Controls
	if (Input::KeyDown('W')) { transform->MoveRelative(0, 0, speed); }
	if (Input::KeyDown('A')) { transform->MoveRelative(-speed, 0, 0); }
	if (Input::KeyDown('S')) { transform->MoveRelative(0, 0, -speed); }
	if (Input::KeyDown('D')) { transform->MoveRelative(speed, 0, 0); }
	if (Input::KeyDown(' ')) { transform->MoveAbsolute(0, speed, 0); }
	if (Input::KeyDown('X')) {transform->MoveAbsolute(0, -speed, 0);}

	// Mouse Controls
	if (Input::MouseLeftDown()) {

		// calculate cursor change
		float cursorMovementX = mouseLookSpeed * Input::GetMouseXDelta();
		float cursorMovementY = mouseLookSpeed * Input::GetMouseYDelta();

		// rotate the transform accordingly
		transform->Rotate(cursorMovementY, cursorMovementX, 0);

		// Clamp rotation to prevent camera from going upside down
		DirectX::XMFLOAT3 rotation = transform->GetPitchYawRoll();
		if (rotation.x > DirectX::XM_PIDIV2) rotation.x = DirectX::XM_PIDIV2;
		if (rotation.x < -DirectX::XM_PIDIV2) rotation.x = -DirectX::XM_PIDIV2;
		transform->SetRotation(rotation);
	}

	// Update View Matrix
	UpdateViewMatrix();
}

DirectX::XMFLOAT4X4 Camera::GetView()
{
	return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjection()
{
	return projectionMatrix;
}

std::shared_ptr<Transform> Camera::GetTransform()
{
	return transform;
}

float Camera::GetAspectRatio()
{
	return aspectRatio;
}

float Camera::GetFieldOfView()
{
	return fov;
}

void Camera::SetFieldOfView(float _fov)
{
	fov = _fov;
	UpdateProjectionMatrix(aspectRatio);
}

float Camera::GetNearClip()
{
	return nearClipDistance;
}

void Camera::SetNearClip(float distance)
{
	nearClipDistance = distance;
	UpdateProjectionMatrix(aspectRatio);
}

float Camera::GetFarClip()
{
	return farClipDistance;
}

void Camera::SetFarClip(float distance)
{
	farClipDistance = distance;
	UpdateProjectionMatrix(aspectRatio);
}

float Camera::GetOrthoGraphicWidth()
{
	return orthographicWidth;
}

void Camera::SetOrthoGraphicWidth(float width)
{
	orthographicWidth = width;
	UpdateProjectionMatrix(aspectRatio);
}

ProjectionType Camera::GetProjectionType()
{
	return projectionType;
}

void Camera::SetProjectionType(ProjectionType type)
{
	projectionType = type;
	UpdateProjectionMatrix(aspectRatio);
}

float Camera::GetCameraSpeed()
{
	return cameraSpeed;
}

void Camera::SetCameraSpeed(float speed)
{
	cameraSpeed = speed;
}

float Camera::GetMouseLookSpeed()
{
	return mouseLookSpeed;
}

void Camera::SetMouseLookSpeed(float speed)
{
	mouseLookSpeed = speed;
}
