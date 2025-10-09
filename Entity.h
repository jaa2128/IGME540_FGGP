#pragma once
#include "Mesh.h"
#include "Transform.h"
#include "Material.h"
#include <memory>
class Entity
{
public:
	Entity(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material);
	~Entity();

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Material> GetMaterial();

	void SetMaterial(std::shared_ptr<Material> _material);

	void Draw();

private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Material> material;
};

