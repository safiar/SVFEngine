// ----------------------------------------------------------------------- //
//
// MODULE  : skybox_dx9_pshader.fx
//
// PURPOSE : Шейдер неба
//
// CREATED : SavF. | Savenkov Filipp A. (2017) 
//
// ----------------------------------------------------------------------- //

// Semantics Supported in Direct3D 9 and Direct3D 10 and later
// https://msdn.microsoft.com/ru-ru/library/windows/desktop/bb509647(v=vs.85).aspx#PS

// D3D9   sampler Name = SamplerType{   Texture = <texture_variable>;   [state_name = state_value;]   ... };
// D3D10  SamplerType Name[Index]{   [state_name = state_value;]   ... };

//sampler SkyBoxTexture = sampler_state  // sampler, sampler1D, sampler2D, sampler3D, samplerCUBE
//{
//   // Filter = MIN_MAG_MIP_LINEAR;  D3D10
//   // ComparisonFunc = LESS;        D3D10
//
//   magfilter = LINEAR;
//   minfilter = LINEAR;
//   mipfilter = LINEAR;
//   AddressU = CLAMP; // Mirror; WRAP;
//   AddressV = CLAMP; // Mirror; WRAP;
//};

sampler SkyBoxTexture : register(s7); // index 7  current env 3D cubemap

struct PShaderInput
{
    float3 TexCoord : TEXCOORD0;
};

float4 MainFunc(PShaderInput inp) : COLOR0
{
    return texCUBE(SkyBoxTexture, normalize(inp.TexCoord));
};

// Inputs //////////////////////////////////////////////////////////////
//
// COLOR[n]		float4
// TEXCOORD[n]		float4
// VFACE		float		SV_IsFrontFace in D3D10
//	Floating-point scalar that indicates a back-facing primitive.
//	A negative value faces backwards, while a positive value faces the camera.
// VPOS			float2		The pixel location (x,y) in screen space.
//	To convert a Direct3D 9 shader (that uses this semantic) to a Direct3D 10 and
//	later shader, see : Direct3D 9 VPOS and Direct3D 10 SV_Position)
//
// Outputs //////////////////////////////////////////////////////////////
//
// COLOR[n]		float4
// DEPTH[n]		float
//
