// ----------------------------------------------------------------------- //
//
// MODULE  : light_N_dx9_vshader.fx
//
// PURPOSE : Шейдер света с дополнениями:
//           - Parallax normal mapping
//           - Environment reflecting
//
// CREATED : SavF. | Savenkov Filipp A. (2017) 
//
// ----------------------------------------------------------------------- //

matrix World;
matrix ViewProj;

struct VS_INPUT 
{ 
   vector position  : POSITION0;
   vector normal    : NORMAL0;
   float2 texcoord0 : TEXCOORD0;
   vector tangent   : TANGENT0;
   vector binormal  : BINORMAL0;
};

struct VS_OUTPUT 
{ 
   vector position  : POSITION0;
   vector normal    : COLOR0;
   vector tangent   : COLOR1;
   vector binormal  : COLOR2;
   vector world     : COLOR3;
   float2 texcoord0 : TEXCOORD0;
};

VS_OUTPUT MainFunc(VS_INPUT input) 
{
   VS_OUTPUT output = (VS_OUTPUT)0;

   vector PosW      = mul(input.position, World);      // float4 * float4x4
   output.position  = mul(PosW,           ViewProj);   // float4 * float4x4
   output.world     = PosW;

   output.texcoord0 = input.texcoord0;

   input.normal.w   = 0;
   input.tangent.w  = 0;
   input.binormal.w = 0;
   output.normal    = normalize(mul(input.normal,   World));
   output.tangent   = normalize(mul(input.tangent,  World));
   output.binormal  = normalize(mul(input.binormal, World));

   return output;
};
