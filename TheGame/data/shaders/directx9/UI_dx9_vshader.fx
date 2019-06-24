// ----------------------------------------------------------------------- //
//
// MODULE  : UI_dx9_vshader.fx
//
// PURPOSE : Шейдер для вывода элементов интерфейса
//
// CREATED : SavF. | Savenkov Filipp A. (2017) 
//
// ----------------------------------------------------------------------- //

matrix World;
matrix ViewProj;

struct VS_INPUT 
{ 
   vector position  : POSITION0;
   vector color     : COLOR0;
   float2 texcoord  : TEXCOORD0;
};

struct VS_OUTPUT 
{ 
   vector position  : POSITION0;
   vector color     : COLOR0;
   float2 texcoord  : TEXCOORD0;
};

VS_OUTPUT MainFunc(VS_INPUT input) 
{
   VS_OUTPUT output = (VS_OUTPUT)0;

   vector PosW      = mul(input.position, World);      // float4 * float4x4
   output.position  = mul(PosW,           ViewProj);   // float4 * float4x4

   output.texcoord  = input.texcoord;
   output.color     = input.color;

   return output;
};
