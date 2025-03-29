#include "BasicShaderHeader.hlsli"

float4 ps(BasicOutput i) : SV_TARGET
{
	return float4(i.uv, 1.0f, 1.0f);
}