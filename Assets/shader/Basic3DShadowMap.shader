{
    "shaders":
    {
        "vs": "Assets/shader/Basic3DShadowMap.hlsl",
        "ps": ""
    },
    "input_elements":
    [
        { "semantic": "POSITION", "format": "R32G32B32_FLOAT" },
        { "semantic": "NORMAL", "format": "R32G32B32_FLOAT" },
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
            }
        ],
        "samplers":
        [
        ]
    }
}
