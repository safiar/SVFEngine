// ----------------------------------------------------------------------- //
//
// MODULE  : UI_dx9_pshader.fx
//
// PURPOSE : Шейдер для вывода элементов интерфейса
//
// CREATED : SavF. | Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

sampler TexDiffuse : register(s0); // index 0  current color 2D map

struct PS_INPUT
{
   vector color     : COLOR0;
   vector texcoord  : TEXCOORD0;
};

struct PS_OUTPUT
{
   vector diffuse   : COLOR0;
};

PS_OUTPUT MainFunc(PS_INPUT input)
{
   PS_OUTPUT output = (PS_OUTPUT)0;

   vector texcolor = tex2D(TexDiffuse, input.texcoord.xy);
   output.diffuse = texcolor * input.color;

   return output;
};

