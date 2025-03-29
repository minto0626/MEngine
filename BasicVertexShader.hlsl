#include "BasicShaderHeader.hlsli"

BasicOutput vs(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    BasicOutput o;
    o.sv_pos = pos;
    o.uv = uv;
	return o;
}