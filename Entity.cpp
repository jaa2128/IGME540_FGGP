#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material) : mesh(_mesh), 
transform(std::make_shared<Transform>()), material(_material)
{
}

Entity::~Entity()
{
}

std::shared_ptr<Mesh> Entity::GetMesh()
{
    return mesh;
}

std::shared_ptr<Transform> Entity::GetTransform()
{
    return transform;
}

std::shared_ptr<Material> Entity::GetMaterial()
{
    return material;
}

void Entity::SetMaterial(std::shared_ptr<Material> _material)
{
    material = _material;
}

void Entity::Draw()
{
    // set shaders to the current entity
    material->BindTexturesAndSamplers();
    Graphics::Context->VSSetShader(material->GetVertexShader().Get(), 0, 0);
    Graphics::Context->PSSetShader(material->GetPixelShader().Get(), 0, 0);
    mesh->Draw();
}
