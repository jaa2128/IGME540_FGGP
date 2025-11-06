#ifndef __GGP_LIGHTING__
#define __GGP_LIGHTING__

#define MAX_SPECULAR_EXPONENT 256.0f

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

// Struct that defines a light
struct Light
{
    int type;
    float3 direction;
    float range;
    float3 position;
    float intensity;
    float3 color;
    float spotInnerAngle;
    float spotOuterAngle;
    float2 padding;
};

float DiffuseTerm(float3 normal, float3 directionToLight)
{
    return saturate(dot(normal, directionToLight));

}

float SpecularTerm(float3 normal, float3 directionToLight, float3 vectorToCameraPos, float roughness)
{
    //Calculate reflection vector
    float3 reflection = reflect(-directionToLight, normal);
    
    if (roughness == 1)
    {
        return 0.0f;
    }
    else
    {
        return pow(max(dot(reflection, vectorToCameraPos), 0.0f), (1 - roughness) * MAX_SPECULAR_EXPONENT);
    }
        

}

float3 DirectionalLight(Light light, float3 normal, float3 worldPosition, float3 cameraPosition, float roughness, float3 surfaceColor)
{
    // Get the normalized direction of the surface to the light (reverse of incoming direction)
    float3 directionToLight = normalize(-light.direction);
    float3 vectorToCamera = normalize(cameraPosition - worldPosition);
    
    // get the Diffuse term
    float diffuseTerm = DiffuseTerm(normal, directionToLight);
   
    // get the specular term
    float spec = SpecularTerm(normal, directionToLight, vectorToCamera, roughness);
    
    // perform calculation
    return (diffuseTerm * surfaceColor + spec) * light.color * light.intensity;
}

#endif