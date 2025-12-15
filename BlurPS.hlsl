cbuffer externalData : register(b0)
{
    float pixelWidth;
    float pixelHeight;
    int blurRadius;
}

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D Pixels : register(t0);
SamplerState ClampSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    // Track the total color and number of samples
    float3 color = float3(0, 0, 0);
    int sampleCount = 0;
    
    // Loop through the "box"
    for (int x = -blurRadius; x <= blurRadius; x++)
    {
        for (int y = -blurRadius; y <= blurRadius; y++)
        {
            // Calculate the uv for this sample
            float2 uv = input.uv;
            uv += float2(x * pixelWidth, y * pixelHeight);
            
            // Add this color to the running total
            color += Pixels.Sample(ClampSampler, uv).rgb;
            sampleCount++;
        }
    }
    
    // Return the average
    return float4(color / sampleCount, 1);
}