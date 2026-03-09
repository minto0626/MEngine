{
    "shaders":
    {
        "vs": "Assets/shader/PostProcess.hlsl",
        "ps": "Assets/shader/PostProcess.hlsl"
    },
    "input_elements":
    [
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
                "name": "srcTex",
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
