#ifndef __GGP_LIGHTING__
#define __GGP_LIGHTING__

#define MAX_SPECULAR_EXPONENT 256.0f

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

// PBR Constants:
// fresnel value for non metals
static const float F0_NON_METAL = 0.04f;

static const float MIN_ROUGHNESS = 0.0000001f;

static const float PI = 3.14159265359f;
static const float TWO_PI = PI * 2.0f;
static const float HALF_PI = PI / 2.0f;
static const float QUARTER_PI = PI / 4.0f;

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

// Normal Distribution Func
float D_GGX(float3 normal, float3 halfVector, float roughness)
{
    // Pre-calculations
    float NdotH = saturate(dot(normal, halfVector));
    float NdotH2 = NdotH * NdotH;
    float a = roughness * roughness; // Remapping roughness
    float a2 = max(a * a, MIN_ROUGHNESS);
    
    // Denominator to be squared is ((n dot h)^2 * (a^2 - 1) + 1)
    float denomToSquare = NdotH2 * (a2 - 1) + 1;
    
    return a2 / (PI * denomToSquare * denomToSquare);
}

// Geometric Shadowing
float G_SchlickGGX(float3 normal, float3 viewVector, float roughness)
{
    float k = pow(roughness + 1, 2) / 8.0f; // End result of remaps
    float NdotV = saturate(dot(normal, viewVector));
    return 1 / (NdotV * (1 - k) + k);
}

// Fresnel
float3 F_Schlick(float3 viewVector, float3 halfVector, float3 f0)
{
    float VdotH = saturate(dot(viewVector, halfVector));
    return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

// PBR Specular Term using Cook-Torrance MicrofacetBDRF
float3 SpecularTerm(float3 normal, float3 directionToLight, float3 vectorToCameraPos, float roughness, float3 f0, out float3 F_out)
{
    // Calculate halfVector
    float3 halfVector = normalize(vectorToCameraPos + directionToLight);

    // get denominators
    float D = D_GGX(normal, halfVector, roughness);
    float3 F = F_Schlick(vectorToCameraPos, halfVector, f0);
    float G = G_SchlickGGX(normal, vectorToCameraPos, roughness) * G_SchlickGGX(normal, directionToLight, roughness);
    
    // Pass the Fresnel term out for Diffuse balance later
    F_out = F;
    
    // Specular term:
    float3 spec = (D * F * G) / 4;
    
    return spec * saturate(dot(normal, directionToLight));
}

// Diffuse Energy Conservation for metals
float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
    return diffuse * (1 - F) * (1 - metalness);
}

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.range * light.range)));
    return att * att;
}

float3 DirectionalLight(Light light, float3 normal, float3 worldPosition, float3 cameraPosition, float roughness, float metalness, float3 surfaceColor, float3 specularColor)
{
    // Get the normalized direction of the surface to the light (reverse of incoming direction)
    float3 directionToLight = normalize(-light.direction);
    float3 vectorToCamera = normalize(cameraPosition - worldPosition);
    
    // get the Diffuse term
    float diffuseTerm = DiffuseTerm(normal, directionToLight);
    
    // Fresnel Calculation
    float3 F;
    
    // get the specular term
    // Fresnel Term calulated here will be pushed out to F
    float3 specularTerm = SpecularTerm(normal, directionToLight, vectorToCamera, roughness, specularColor, F);
    

    // Calculate Diffuse with energy conservation
    float3 balancedDiffuse = DiffuseEnergyConserve(diffuseTerm, F, metalness);
    
    // perform calculation
    return (balancedDiffuse * surfaceColor + specularTerm) * light.color * light.intensity;
}

float3 PointLight(Light light, float3 normal, float3 worldPosition, float3 cameraPosition, float roughness, float metalness, float3 surfaceColor, float3 specularColor)
{
    // Get the normalized direction of the surface to the light (position of light - world position of surface)
    float3 directionToLight = normalize(light.position - worldPosition);
    float3 vectorToCamera = normalize(cameraPosition - worldPosition);
    
    // get the Diffuse term
    float diffuseTerm = DiffuseTerm(normal, directionToLight);
   
    // Fresnel Calculation
    float3 F;
    
    // get the specular term
    float3 specularTerm = SpecularTerm(normal, directionToLight, vectorToCamera, roughness, specularColor, F);
    
    // get attenuation value
    float attenuation = Attenuate(light, worldPosition);
   
    
    // Calculate Diffuse with energy conservation
    float3 balancedDiffuse = DiffuseEnergyConserve(diffuseTerm, F, metalness);
    
    // perform calculation
    return (balancedDiffuse * surfaceColor + specularTerm) * attenuation * light.color * light.intensity;
}

float3 SpotLight(Light light, float3 normal, float3 worldPosition, float3 cameraPosition, float roughness, float metalness, float3 surfaceColor, float3 specularColor)
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
    
    return PointLight(light, normal, worldPosition, cameraPosition, roughness, metalness, surfaceColor, specularColor) * spotTerm;

}

// Normal Mapping unpacking and tangent transformation

// Sample then unpack values
float3 UnpackNormalMap(Texture2D map, SamplerState _sampler, float2 uv)
{
    return map.Sample(_sampler, uv).rgb * 2.0f - 1.0f;
}

// Convert from Tangent Space to Normal Space
float3 NormalMapping(Texture2D map, SamplerState _sampler, float2 uv, float3 normal, float3 tangent)
{
    // get the unpacked normals
    float3 unpackedNormal = normalize(UnpackNormalMap(map, _sampler, uv));
    
    // create TBN Matrix
    float3 Normal = normalize(normal);
    float3 Tangent = normalize(tangent - dot(tangent, Normal) * Normal);
    float3 Bitangent = cross(Tangent, Normal);
    
    float3x3 TBNmatrix = float3x3(Tangent, Bitangent, Normal);
    
    //Transform the normal from Vertex Shader with TBN matrix
    return normalize(mul(unpackedNormal, TBNmatrix));

}

#endif