R""(
Texture2D<float4> tex2D : register(t0);
SamplerState samplerState : register(s0);

cbuffer constantBuffer
{
    float2 xyOffset;
    float time;
    float intensity;
};

float4 PS_GlowH(float4 sum, float2 inputCoord, float widthScale) : COLOR0
{
    sum += tex2D.Sample(samplerState, float2(inputCoord.x - 6.0f * widthScale, inputCoord.y)) * 0.010f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x - 5.0f * widthScale, inputCoord.y)) * 0.020f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x - 4.0f * widthScale, inputCoord.y)) * 0.030f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x - 3.0f * widthScale, inputCoord.y)) * 0.041f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x - 2.0f * widthScale, inputCoord.y)) * 0.053f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x - widthScale, inputCoord.y))        * 0.063f;
    sum += tex2D.Sample(samplerState, inputCoord)                                             * 0.066f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x + widthScale, inputCoord.y))        * 0.063f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x + 2.0f * widthScale, inputCoord.y)) * 0.053f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x + 3.0f * widthScale, inputCoord.y)) * 0.041f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x + 4.0f * widthScale, inputCoord.y)) * 0.030f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x + 5.0f * widthScale, inputCoord.y)) * 0.020f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x + 6.0f * widthScale, inputCoord.y)) * 0.010f;

    return sum;
}

float4 PS_GlowV(float4 sum, float2 inputCoord, float heightScale) : COLOR0
{
    sum += tex2D.Sample(samplerState, float2(inputCoord.x, inputCoord.y - 6.0f * heightScale)) * 0.010f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x, inputCoord.y - 5.0f * heightScale)) * 0.020f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x, inputCoord.y - 4.0f * heightScale)) * 0.030f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x, inputCoord.y - 3.0f * heightScale)) * 0.041f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x, inputCoord.y - 2.0f * heightScale)) * 0.053f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x, inputCoord.y - heightScale))        * 0.063f;
    sum += tex2D.Sample(samplerState, inputCoord)                                              * 0.066f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x, inputCoord.y + heightScale))        * 0.063f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x, inputCoord.y + 2.0f * heightScale)) * 0.053f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x, inputCoord.y + 3.0f * heightScale)) * 0.041f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x, inputCoord.y + 4.0f * heightScale)) * 0.030f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x, inputCoord.y + 5.0f * heightScale)) * 0.020f;
    sum += tex2D.Sample(samplerState, float2(inputCoord.x, inputCoord.y + 6.0f * heightScale)) * 0.010f;

    return sum;
}

float4 PS_Glow_Gauss(float4 color : COLOR0, float2 texCoord : TEXCOORD0) : SV_Target0
{
    float4 src = tex2D.Sample(samplerState, texCoord);

    float2 dimensions;
    tex2D.GetDimensions(dimensions.x, dimensions.y);
    dimensions = rcp(dimensions);
    
    if (!(isfinite(dimensions.x) && isfinite(dimensions.y)))
    {
        return src;
    }
    
    float2 texCoordOffset = float2(texCoord.x + xyOffset.x * dimensions.x, texCoord.y - xyOffset.y * dimensions.y);

    float mul = clamp(intensity, 1.0f, 2.0f);
    float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 result = PS_GlowV(PS_GlowH(sum, texCoordOffset, dimensions.x * mul), texCoordOffset, dimensions.y * mul);
    return result * (1.0f - sqrt(src.a)) * intensity * 0.5f + src;
}

float4 PS_Saturate(float4 color : COLOR0, float2 texCoord : TEXCOORD0) : SV_Target0
{
    float3 weights = float3(0.299f, 0.587f, 0.114f);

    float4 result = mul(tex2D.Sample(samplerState, texCoord), color);
    float lumi = dot(result.xyz, weights);

    result = lerp(float4(lumi, lumi, lumi, result.a), result, intensity);

    return result;
}

float4 PS_Intensity(float4 color : COLOR0, float2 texCoord : TEXCOORD0) : SV_Target0
{
    float4 result = PS_Glow_Gauss(color, texCoord);

    float3 weights = float3(0.299f, 0.587f, 0.114f);
    float lumi = dot(result.xyz, weights);

    result = lerp(float4(lumi, lumi, lumi, result.a), result, saturate(intensity));

    return float4(result.r * color.r, result.g * color.g, result.b * color.b, result.a * color.a);
}

float4 PS_Mirror(float4 mirrorData : COLOR0, float2 texCoord : TEXCOORD0) : SV_Target0
{
    return tex2D.Sample(samplerState, abs(float2(mirrorData.x - texCoord.x, mirrorData.y - texCoord.y)));
}
)""