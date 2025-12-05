struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D srcTex : register(t0);
SamplerState smp : register(s0);

VSOut vs(uint id : SV_VertexID)
{
    VSOut output;
    float2 positions[3] = { float2(-1, -1), float2(-1, 3), float2(3, -1) };
    float2 uvs[3] = { float2(0, 1), float2(0, -1), float2(2, 1) };
    output.pos = float4(positions[id], 0, 1);
    output.uv = uvs[id];
    return output;
}

float4 monochrome(float4 In)
{
    // モノクロ変換
    float gray = dot(In.rgb, float3(0.299, 0.587, 0.114));
    return float4(gray, gray, gray, In.a);
}

float4 sepia(float4 In)
{
    // セピア変換
    float3 sepiaColor;
    sepiaColor.r = dot(In.rgb, float3(0.393, 0.769, 0.189));
    sepiaColor.g = dot(In.rgb, float3(0.349, 0.686, 0.168));
    sepiaColor.b = dot(In.rgb, float3(0.272, 0.534, 0.131));
    return float4(sepiaColor, In.a);
}

float4 negative(float4 In)
{
    // ネガポジ反転
    return float4(1.0 - In.rgb, In.a);
}

float noise(float2 uv, float scale)
{
    // シンプルノイズ生成
    float2 pos = uv * scale;
    return frac(sin(dot(pos, float2(12.9898, 78.233))) * 43758.5453);
}

float4 ps(VSOut input) : SV_TARGET
{
    float4 color = srcTex.Sample(smp, input.uv);
    //return color;

    //return monochrome(color);
    //return sepia(color);
    //return negative(color);
    return srcTex.Sample(smp, input.uv + 0.01f * noise(input.uv, 10.0));
}
