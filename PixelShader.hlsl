#include "ShaderStructs.hlsli"
#include "Lighting.hlsli"

Texture2D Albedo : register(t0); // "t" registers for textures
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
Texture2D ShadowMap : register(t4);

SamplerState BasicSampler : register(s0); // "s" registers for samplers
SamplerComparisonState ShadowSampler : register(s1);
// Constant buffer bound to the 0 indexed buffer
// (b0); b = buffer; 0 = index
// name is arbitrary
// layout MUST match struct
cbuffer PixelShaderData : register(b0)
{
    float3 colorTint;
    float time;
    float2 uvScale;
    float2 uvOffset;
    float inputRoughness;
    float3 camPos;
    float3 ambientColor;
    float pad;
    Light lights[5];
}


// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Adjust UV coords
    input.uv = input.uv * uvScale + uvOffset;
    
    // normalize input
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    
    // Normal Mapping
    input.normal = NormalMapping(NormalMap, BasicSampler, input.uv, input.normal, input.tangent);

    // Texture color
    float3 surfaceColor = pow(Albedo.Sample(BasicSampler, input.uv).rgb, 2.2f);
    surfaceColor *= colorTint;
    
    // Roughness Map
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    
    // Metalness Map
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;
    
    // Specular color determination -----------------
    // Assume albedo texture is actually holding specular color where metalness == 1
    // Note the use of lerp here - metal is generally 0 or 1, but might be in between
    // because of linear texture sampling, so we lerp the specular color to match
    float3 specularColor = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);
    
    // Before lighting, check shadowMap
    input.shadowMapPos /= input.shadowMapPos.w;
    
    // Convert the normalized device coordinates to UVs for sampling
    float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
    shadowUV.y = 1 - shadowUV.y; // Flip the Y
    
    // Grab the distances we need: light-to-pixel and closest-surface
    float distToLight = input.shadowMapPos.z;
    
    // Get a ratio of comparison results using SampleCmpLevelZero()
    float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, distToLight).r;
    
    float3 totalLight;
    
    // Calculating Diffuse Lighting
    for (int i = 0; i < 5; i++)
    {
        Light light = lights[i];
        
        switch (lights[i].type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                float3 lightResult = DirectionalLight(light, input.normal, input.worldPos, camPos, roughness, metalness, surfaceColor, specularColor);
            
                if (i == 0)
                {
                    lightResult *= shadowAmount;
                }
            
                totalLight += lightResult;
                    break;
            case LIGHT_TYPE_POINT:
                totalLight += PointLight(light, input.normal, input.worldPos, camPos, roughness, metalness, surfaceColor, specularColor);
                break;
            case LIGHT_TYPE_SPOT:
                totalLight += SpotLight(light, input.normal, input.worldPos, camPos, roughness, metalness, surfaceColor, specularColor);
                break;
        }
       
    }
    
    // Gamma Correction 
    return float4(pow(totalLight, 1.0f / 2.2f), 1);
}