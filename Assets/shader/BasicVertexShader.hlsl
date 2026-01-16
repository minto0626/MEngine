#include "BasicShaderHeader.hlsli"

cbuffer ScreenCB : register(b0)
{
    matrix viewProjection;
};

cbuffer Transform : register(b1)
{
    matrix world;
};

BasicOutput vs(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    BasicOutput o;
    o.sv_pos = mul(mul(viewProjection, world), pos);
    o.uv = uv;
	return o;
}
