//////////////////////// 
// TEST USING EFFECTS //
////////////////////////

// DirectX 8.0  - Shader Model 1.0 & 1.1
// DirectX 8.0a - Shader Model 1.3
// DirectX 8.1  - Shader Model 1.4
// DirectX 9.0  - Shader Model 2.0
// DirectX 9.0a - Shader Model 2.0a
// DirectX 9.0b - Shader Model 2.0b
// DirectX 9.0c - Shader Model 3.0
// DirectX 10.0 - Shader Model 4.0
// DirectX 10.1 - Shader Model 4.1
// DirectX 11.0 - Shader Model 5.0
// DirectX 11.1 - Shader Model 5.0
// DirectX 11.2 - Shader Model 5.0
// DirectX 12   - Shader Model 5.1

///////////////////////////////////////////////// v_shader /////

matrix World;		// float4x4
matrix ViewProj;	// float4x4
float3 CameraPosition;

struct VShaderInput
{
    float4 P  : POSITION0;   // 3D float expanded to (value, value, value, 1.)
};

struct VShaderOutput
{
    float4 Position : POSITION0;
    float3 TexCoord : TEXCOORD0;
};

VShaderOutput VS_MainFunc(VShaderInput inp)
{
    VShaderOutput outp;

    float4 PosW   = mul(inp.P, World);        // float4 * float4x4
    outp.Position = mul(PosW,  ViewProj);

    outp.TexCoord = (float3)PosW - CameraPosition;

    return outp;
};

///////////////////////////////////////////////// p_shader /////

sampler SkyBoxTexture : register(s7); // index 7  current env 3D cubemap

struct PShaderInput
{
    float3 TexCoord : TEXCOORD0;
};

float4 PS_MainFunc(PShaderInput inp) : COLOR0
{
    return texCUBE(SkyBoxTexture, normalize(inp.TexCoord));
};

///////////////////////////////////////////////// Techniques /////

vertexshader VS50 = compile vs_5_0 VS_MainFunc();	// 5.0 версия
pixelshader  PS50 = compile ps_5_0 PS_MainFunc();	// 5.0 версия

technique SkyboxTechniqueV50
{
    pass P0
    {
        VertexShader = (VS50);
        PixelShader  = (PS50);
    }
}


