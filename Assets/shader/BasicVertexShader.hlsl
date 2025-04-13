#include "BasicShaderHeader.hlsli"

cbuffer Transform : register(b0)
{
    matrix world;
};

BasicOutput vs(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    BasicOutput o;
    o.sv_pos = mul(world, pos);
    o.uv = uv;
	return o;
}