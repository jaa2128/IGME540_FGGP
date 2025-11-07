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
    
    // if Roughness is 1 there is no specular term
    if (roughness == 1)
    {
        return 0.0f;
    }
    // Otherwise calculate it
    else
    {
        return pow(max(dot(reflection, vectorToCameraPos), 0.0f), (1 - roughness) * MAX_SPECULAR_EXPONENT);
    }
        

}

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.range * light.range)));
    return att * att;
}

float3 DirectionalLight(Light light, float3 normal, float3 worldPosition, float3 cameraPosition, float roughness, float3 surfaceColor)
{
    // Get the normalized direction of the surface to the light (reverse of incoming direction)
    float3 directionToLight = normalize(-light.direction);
    float3 vectorToCamera = normalize(cameraPosition - worldPosition);
    
    // get the Diffuse term
    float diffuseTerm = DiffuseTerm(normal, directionToLight);
   
    // get the specular term
    float specularTerm = SpecularTerm(normal, directionToLight, vectorToCamera, roughness);
    
    // perform calculation
    return (diffuseTerm * surfaceColor + specularTerm) * light.color * light.intensity;
}

float3 PointLight(Light light, float3 normal, float3 worldPosition, float3 cameraPosition, float roughness, float3 surfaceColor)
{
    // Get the normalized direction of the surface to the light (position of light - world position of surface)
    float3 directionToLight = normalize(light.position - worldPosition);
    float3 vectorToCamera = normalize(cameraPosition - worldPosition);
    
    // get the Diffuse term
    float diffuseTerm = DiffuseTerm(normal, directionToLight);
   
    // get the specular term
    float specularTerm = SpecularTerm(normal, directionToLight, vectorToCamera, roughness);
    
    // get attenuation value
    float attenuation = Attenuate(light, worldPosition);
    
    // perform calculation
    return (diffuseTerm * surfaceColor + specularTerm) * attenuation * light.color * light.intensity;
}

float3 SpotLight(Light light, float3 normal, float3 worldPosition, float3 cameraPosition, float roughness, float3 surfaceColor)
{
     // Get the normalized direction of the surface to the light (position of light - world position of surface)
    float3 directionToLight = normalize(light.position - worldPosition);
    
    // Get the angle between the surface and the light's direction (reverse of directionToLight)
    float pixelAngle = saturate(dot(-directionToLight, light.direction));
    
    // Get cosines of angles and calculate range for the fall off
    float cosOuter = cos(light.spotOuterAngle);
    float cosInner = cos(light.spotInnerAngle);
    float falloffRange = cosOuter - cosInner;
    
    // Linear falloff over the range, clamp 0-1, apply to light calc
    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
    
    return PointLight(light, normal, worldPosition, cameraPosition, roughness, surfaceColor) * spotTerm;

}

#endif