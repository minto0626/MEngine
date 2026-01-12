struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer SceneCB : register(b0)
{
    matrix view;
    matrix projection;
    float3 eyePos;
    float pad0;

    matrix lightViewProj;
    float3 lightVec;
    float pad1;
};

Texture2D<float4> albedo_tex : register(t0);
Texture2D<float4> normal_tex : register(t1);
Texture2D<float4> world_pos_tex : register(t2);
Texture2D<float> shadow_map : register(t3);

SamplerState smp : register(s0);
SamplerComparisonState shadow_smp : register(s1);

VSOut vs(uint id : SV_VertexID)
{
    VSOut output;
    float2 positions[3] = { float2(-1, -1), float2(-1, 3), float2(3, -1) };
    float2 uvs[3] = { float2(0, 1), float2(0, -1), float2(2, 1) };
    output.pos = float4(positions[id], 0, 1);
    output.uv = uvs[id];
    return output;
}

float4 ps(VSOut input) : SV_TARGET
{
    float4 albedo = albedo_tex.Sample(smp, input.uv);
    
    float4 normal = normal_tex.Sample(smp, input.uv);
    normal = normal * 2.0f - 1.0f;
    
    float4 world_pos = world_pos_tex.Sample(smp, input.uv);

    // Phongの拡散反射モデル
    
    float3 light = normalize(lightVec);
    
    float3 ref = reflect(light, normal.xyz);
    float3 toEye = normalize(eyePos - world_pos.xyz);
    float specular = pow(saturate(dot(ref, toEye)), 50);
    float ambient = 0.3f;
    float diffuse = saturate(dot(-light, normal.xyz));

    float4 finalColor = albedo * (diffuse + specular + ambient);
    
    // シャドウマップによる影付け
    // 投影変換後の座標を正規化デバイス座標に変換
    float4 light_view_pos = mul(lightViewProj, float4(world_pos.xyz, 1.0f));
    float3 shadowViewProj = light_view_pos.xyz / light_view_pos.w;
    float2 shadowMapUV = shadowViewProj.xy * float2(0.5f, -0.5f) + 0.5f;
    float shadowWeight = lerp(0.5f, 1.0f, shadow_map.SampleCmp(shadow_smp, shadowMapUV, shadowViewProj.z - 0.001f));
    finalColor.rgb *= shadowWeight;

    return finalColor;
}
