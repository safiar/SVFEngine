// ----------------------------------------------------------------------- //
//
// MODULE  : skybox_dx9_vshader.fx
//
// PURPOSE : Шейдер неба
//
// CREATED : SavF. | Savenkov Filipp A. (2017) 
//
// ----------------------------------------------------------------------- //

// Semantics Supported in Direct3D 9 and Direct3D 10 and later
// https://msdn.microsoft.com/ru-ru/library/windows/desktop/bb509647(v=vs.85).aspx#PS
// Intrinsic Functions
// https://msdn.microsoft.com/ru-ru/library/windows/desktop/ff471376(v=vs.85).aspx
// Variable Syntax
// https://msdn.microsoft.com/ru-ru/library/windows/desktop/bb509706(v=vs.85).aspx

matrix World;		// float4x4
matrix ViewProj;	// float4x4
float3 CameraPosition;

struct VShaderInput
{
    float4 P  : POSITION0;   // 3D float expanded to (value, value, value, 1.)
//  float4 N  : NORMAL0;
//  float4 T  : TEXCOORD0;   // 2D float expanded to (value, value, 0., 1.)
//  float4 T2 : TEXCOORD1;
//  float4 T3 : TEXCOORD2;
};
 
struct VShaderOutput
{
    float4 Position : POSITION0;
    float3 TexCoord : TEXCOORD0;
};

VShaderOutput MainFunc(VShaderInput inp)
{
    VShaderOutput outp;

    float4 PosW   = mul(inp.P, World);        // float4 * float4x4
    outp.Position = mul(PosW,  ViewProj);

    outp.TexCoord = (float3)PosW - CameraPosition;

    return outp;
};

// Inputs //////////////////////////////////////////////////////////////
//
// BINORMAL[n]		float4
// BLENDINDICES[n]	uint
// BLENDWEIGHT[n]	float
// COLOR[n]		float4		Diffuse and specular color
// NORMAL[n]		float4
// POSITION[n]		float4		Vertex position in object space.
// POSITIONT		float4		Transformed vertex position.
// PSIZE[n]		float		Point size
// TANGENT[n]		float4
// TEXCOORD[n]		float4
//
// Outputs //////////////////////////////////////////////////////////////
//
// COLOR[n]		float4		Diffuse or specular color
// FOG			float		Vertex fog
// POSITION[n]		float4		Position of a vertex in homogenous space.	
//	Compute position in screen-space by dividing (x,y,z) by w.
//	Every vertex shader must write out a parameter with this semantic.
// PSIZE		float		Point size
// TESSFACTOR[n]	float		Tessellation factor
// TEXCOORD[n]		float4		Texture coordinates
//
