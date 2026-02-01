{
    "shaders":
    {
        "vs": "Assets/shader/SceneLighting.hlsl",
        "ps": "Assets/shader/SceneLighting.hlsl"
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
                "name": "sceneCB",
                "type": "cbv",
                "numDescriptors": 1,
                "shaderRegister": 0,
                "visibility": "all"
            },
            {
                "name": "albedoTex",
                "type": "srv",
                "numDescriptors": 1,
                "shaderRegister": 0,
                "visibility": "pixel"
            },
            {
                "name": "normalTex",
                "type": "srv",
                "numDescriptors": 1,
                "shaderRegister": 1,
                "visibility": "pixel"
            },
            {
                "name": "positionTex",
                "type": "srv",
                "numDescriptors": 1,
                "shaderRegister": 2,
                "visibility": "pixel"
            },
            {
                "name": "shadowMap",
                "type": "srv",
                "numDescriptors": 1,
                "shaderRegister": 3,
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
            },
            {
                "shaderRegister": 1,
                "visibility": "pixel",
                "addressMode": "clamp",
                "comparisonFunc": "less_equal",
                "filter": "comparison_min_mag_mip_linear"
            }
        ]
    }
}
