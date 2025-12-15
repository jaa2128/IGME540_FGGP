cbuffer externalData : register(b0)
{
    int pixelSize;
    float pixelWidth;
    float pixelHeight;
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
    // Snap UVs to the nearest pixel block in UV Space
    float2 blockSize = float2(pixelSize * pixelWidth, pixelSize * pixelHeight);
    float2 snappedUV = floor(input.uv / blockSize) * blockSize;
    
    // Return the average
    return Pixels.Sample(ClampSampler, snappedUV);
}