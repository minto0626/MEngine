{
    "shaders":
    {
        "vs": "Assets/shader/BasicVertexShader.hlsl",
        "ps": "Assets/shader/BasicPixelShader.hlsl"
    },
    "input_elements":
    [
        { "semantic": "POSITION", "format": "R32G32B32_FLOAT" },
        { "semantic": "TEXCOORD", "format": "R32G32_FLOAT" }
    ],
    "rtv_formats":
    [
        "R8G8B8A8_UNORM"
    ],
    "root_signature":
    {
        "params":
        [
            {
                "name": "canvasCB",
                "type": "cbv",
                "numDescriptors": 1,
                "shaderRegister": 0,
                "visibility": "vertex"
            },
            {
                "name": "worldMat",
                "type": "cbv",
                "numDescriptors": 1,
                "shaderRegister": 1,
                "visibility": "vertex"
            },
            {
                "name": "mainTex",
                "type": "srv",
                "numDescriptors": 1,
                "shaderRegister": 0,
                "visibility": "pixel"
            }
        ],
        "samplers":
        [
            {
                "shaderRegister": 0,
                "visibility": "pixel",
                "addressMode": "clamp",
                "comparisonFunc": "less_equal",
                "filter": "anisotropic"
            }
        ]
    }
}
