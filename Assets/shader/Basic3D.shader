{
    "shaders":
    {
        "vs": "Assets/shader/Basic3DShader.hlsl",
        "ps": "Assets/shader/Basic3DShader.hlsl"
    },
    "input_elements":
    [
        { "semantic": "POSITION", "format": "R32G32B32_FLOAT" },
        { "semantic": "NORMAL", "format": "R32G32B32_FLOAT" },
        { "semantic": "TEXCOORD", "format": "R32G32_FLOAT" }
    ],
    "rtv_formats":
    [
        "R8G8B8A8_UNORM", "R8G8B8A8_UNORM", "R32G32B32A32_FLOAT"
    ],
    "root_signature":
    {
        "params":
        [
            {
                "name": "sceneCB",
                "type": "cbv",
                "numDescriptors": 1,
                "shaderRegister": 0,
                "visibility": "all"
            },
            {
                "name": "worldMat",
                "type": "cbv",
                "numDescriptors": 1,
                "shaderRegister": 1,
                "visibility": "all"
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
