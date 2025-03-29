#include "BasicShaderHeader.hlsli"

Texture2D<float4> main_tex : register(t0);
SamplerState smp : register(s0);

float4 ps(BasicOutput i) : SV_TARGET
{
    float4 color = main_tex.Sample(smp, i.uv);
    return color;
}