// ----------------------------------------------------------------------- //
//
// MODULE  : light_N_dx9_pshader.fx
//
// PURPOSE : Шейдер света с дополнениями:
//           - Parallax normal mapping
//           - Environment reflecting
//           - Distance disappearing
//           - Fog control
//
// CREATED : SavF. | Savenkov Filipp A. (2017) 
//
// ----------------------------------------------------------------------- //

// https://ru.wikipedia.org/wiki/Parallax_mapping
// http://steps3d.narod.ru/tutorials/parallax-mapping-tutorial.html
// http://www.rastertek.com/dx11tut20.html
// http://ogldev.atspace.co.uk/www/tutorial26/tutorial26.html

// TODO: почитать http://steps3d.narod.ru/tutorials/lpp-tutorial.html
//       HDR Lighting

//////////////////////////////////////////////////////////////////////////////////////////////////////

#define  MAXLIGHTS     8    // 16
#define  LIGHT_DIRECT  0.f
#define  LIGHT_POINT   1.f
#define  LIGHT_SPOT    2.f

//////////////////////////////////////////////////////////////////////////////////////////////////////

float3 CameraPosition;  // позиция камеры в мире

//////////////////////////////////////////////////////////////////////////////////////////////////////

                        // [0,1,2]      [3,4,5]      [6,7,8]       [9]    [10]   [11]     [12]
float  MTL[13];         // ambient.rgb  diffuse.rgb  specular.rgb  alpha  power  TexDiff  TexNorm

#define  MTL_Ka_r               MTL[0]
#define  MTL_Ka_g               MTL[1]
#define  MTL_Ka_b               MTL[2]
#define  MTL_Kd_r               MTL[3]
#define  MTL_Kd_g               MTL[4]
#define  MTL_Kd_b               MTL[5]
#define  MTL_Ks_r               MTL[6]
#define  MTL_Ks_g               MTL[7]
#define  MTL_Ks_b               MTL[8]
#define  MTL_Alpha              MTL[9]
#define  MTL_Power              MTL[10]
#define  isColorTexturePresent  MTL[11]
#define  isBumpTexturePresent   MTL[12]

///////////////////////////////////////////////////////////////////////////////////////////////////////

                                // [0]      [1]       [2]        [3]
vector   VLHT [MAXLIGHTS] [4];  // diffuse, specular, direction, position

#define  Light_diffuse(i)    VLHT[i][0]
#define  Light_specular(i)   VLHT[i][1]
#define  Light_direction(i)  VLHT[i][2]
#define  Light_position(i)   VLHT[i][3]

///////////////////////////////////////////////////////////////////////////////////////////////////////

                                // [0]   [1]    [2-3-4]   [5]      [6]        [7]
float    FLHT [MAXLIGHTS] [8];  // type, range, att0-1-2, cos_phi, cos_theta, falloff

#define  Light_type(i)       FLHT[i][0]
#define  Light_range(i)      FLHT[i][1]
#define  Light_att0(i)       FLHT[i][2]
#define  Light_att1(i)       FLHT[i][3]
#define  Light_att2(i)       FLHT[i][4]
#define  Light_cos_phi(i)    FLHT[i][5]
#define  Light_cos_theta(i)  FLHT[i][6]
#define  Light_falloff(i)    FLHT[i][7]

///////////////////////////////////////////////////////////////////////////////////////////////////////

int    Light_count;
vector Light_ambient;
float  Parallax_scale;

sampler TexDiffuse : register(s0); // index 0  current color 2D map
sampler TexNormal  : register(s1); // index 1  current normal 2D map
sampler TexLight   : register(s2); // index 2  current shadow 2D map
sampler TexEnvCube : register(s7); // index 7  current env 3D cubemap

struct PS_INPUT
{
   vector normal    : COLOR0;
   vector tangent   : COLOR1;
   vector binormal  : COLOR2;
   vector world     : COLOR3;	 // позиция вершины в мире
   vector texcoord  : TEXCOORD0;
};

struct PS_OUTPUT
{
   vector diffuse   : COLOR0;
};

/////////////////////////////////////////////////////////////
/////////// Вычисление количества падающего света ///////////
/////////////////////////////////////////////////////////////

float Calculate_Light_Normal_Factor ( vector normal,
                                      vector direction )
{
   float3 dir   = -1 * direction.xyz;
   float  power = max(0,dot(dir, normal.xyz));

   return power; // множитель количества света
};

///////////////////////////////////////////////////////////////
/////////// Вычисление количества отражённого света ///////////
///////////////////////////////////////////////////////////////

float Calculate_Light_Specular_Factor ( vector normal,
                                        vector direction,
                                        float3 viewer,
                                        float  power )
{
   float3 dir_ref  = reflect(direction.xyz, normal.xyz);   // через отражённый луч
   float  specular = max(0,dot(viewer, dir_ref));

   //float3 halfVec  = normalize(-1 * direction.xyz + viewer); // через полувектор
   //float  specular = max(0,dot(normal.xyz, halfVec));

   if (specular > 0) specular = pow(specular, power); 

   return specular;  // множитель количества отражения
};

////////////////////////////////////////////////////////////////
/////////// Вычисления затухания света от расстояния ///////////
////////////////////////////////////////////////////////////////

float Calculate_Light_Distance_Factor ( vector vertex_position,
                                        vector position,
                                        float  maximum_range,
                                        float  att0,
                                        float  att1,
                                        float  att2 )
{
   float range = distance(vertex_position, position);
   float att   = 0;

   if (maximum_range == 0 ||  // нет ограничения дальности  или
       maximum_range > range) // вершина в области воздействия
   {
         att = 1 / (att0 + att1 * range + att2 * pow(range,2));
   };

   return att; // множитель затухания на расстоянии
};

////////////////////////////////////////////////////////////////////
/////////// Вычисления затухания света от угла освещения ///////////
////////////////////////////////////////////////////////////////////

float Calculate_Light_Angle_Factor ( vector central_direction, // normalized
                                     vector vertex_direction,  // normalized
                                     float  cos_phi,
                                     float  cos_theta,
                                     float  falloff )
{
   float cos_angle = dot(vertex_direction, central_direction);
   float spot;

   if ( cos_angle <= cos_phi )   spot = 0.0f; else
   if ( cos_angle >  cos_theta ) spot = 1.0f; else
   {
   // spot = smoothstep(cos_phi, cos_theta, cos_angle);      // alt.1
   // spot = (cos_angle - cos_phi) / (cos_theta - cos_phi);  // alt.2
      spot = 1-(1-cos_angle)/(1-cos_phi);                    // alt.3
      spot = pow(abs(spot), falloff);
   };

   return spot; // множитель затухания от угла
};

/////////////////////////////////////////////////////////////
/////////// Вычисление рельефного текстурирования ///////////
/////////////////////////////////////////////////////////////

float2 Calculate_Bumped_Coord ( float3 viewer,
                                float2 texcoord )
{
   vector bump = tex2D(TexNormal, texcoord);

   const float  nstep  = 40;
         float  height = 1.0;
         float  step = 1.0 / nstep;
         float2 coord = texcoord;
         float2 coord_step = (viewer.xy * Parallax_scale) / (nstep * viewer.z);

   if (bump.w == height) return coord;

   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);}}}}}}}}}}  // 10

   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);}}}}}}}}}}  // 20

   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);}}}}}}}}}}  // 30

   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);
   if (bump.w<height) {height-=step; coord+=coord_step; bump=tex2D(TexNormal, coord);}}}}}}}}}}  // 40

   //float2 _d  = 0.5f * coord_step;
   //float2 mid = coord - _d;

   //_d*=0.5f; bump=tex2D(TexNormal,mid); if (bump.w<height) mid+=_d; else mid-=_d;
   //_d*=0.5f; bump=tex2D(TexNormal,mid); if (bump.w<height) mid+=_d; else mid-=_d;
   //_d*=0.5f; bump=tex2D(TexNormal,mid); if (bump.w<height) mid+=_d; else mid-=_d;
   //_d*=0.5f; bump=tex2D(TexNormal,mid); if (bump.w<height) mid+=_d; else mid-=_d;
   //_d*=0.5f; bump=tex2D(TexNormal,mid); if (bump.w<height) mid+=_d; else mid-=_d;
   //_d*=0.5f; bump=tex2D(TexNormal,mid); if (bump.w<height) mid+=_d; else mid-=_d;
   //_d*=0.5f; bump=tex2D(TexNormal,mid); if (bump.w<height) mid+=_d; else mid-=_d;
   //_d*=0.5f; bump=tex2D(TexNormal,mid); if (bump.w<height) mid+=_d; else mid-=_d;
   //_d*=0.5f; bump=tex2D(TexNormal,mid); if (bump.w<height) mid+=_d; else mid-=_d;
   //_d*=0.5f; bump=tex2D(TexNormal,mid); if (bump.w<height) mid+=_d; else mid-=_d;

   //coord = mid;

   float  hStop  = bump.w;
   float2 prev   = coord - coord_step;
          bump   = tex2D(TexNormal, prev);
   float  hPrev  = bump.w - (height + step);
   float  hCur   = hStop - height;
   float  weight = hCur / (hCur - hPrev);

   coord = weight * prev + (1 - weight) * coord;  // interpolate

   return coord;
};

struct NMDATA
{
   vector   normal;
   float2   texcoord;
};

NMDATA Calculate_Bumped_Data ( float3x3 TBN,
                               float2   texcoord,
                               float3   viewer    )
{
   NMDATA outdata;

   viewer = mul(viewer, TBN);

   outdata.texcoord = Calculate_Bumped_Coord (viewer, texcoord);

   float3 bump = 2 * (float3)tex2D(TexNormal, outdata.texcoord) - 1;   // [0;1] -> [-1;+1]

   outdata.normal.xyz = mul(TBN, bump);
   outdata.normal.w   = 0;
   outdata.normal     = normalize(outdata.normal);

   return outdata;
};

///////////////////////////////////////////////////////////////
/////////// Главная процедура, точка входа в шейдер ///////////
///////////////////////////////////////////////////////////////

PS_OUTPUT MainFunc(PS_INPUT input)
{
   PS_OUTPUT output = (PS_OUTPUT)0;

   /////////////////////////////////////////////////////////////

   float3 ViewEye_dir = normalize(CameraPosition - (float3)input.world);  // взгляд от камеры на вершину

   int i;

   vector diffuse_all  = vector(0,0,0,1);  // результирующий рассеянный свет
   vector specular_all = vector(0,0,0,1);  // результирующий отражённый свет
   vector env_refl     = vector(0,0,0,1);  // отражение от окружения

   ////////////////// PARALLAX NORMAL MAPPING ///////////////////

   if (isBumpTexturePresent) // если есть карта нормалей
   {
      float3x3 TBN = float3x3 ( input.tangent.x, input.binormal.x, input.normal.x,
                                input.tangent.y, input.binormal.y, input.normal.y,
                                input.tangent.z, input.binormal.z, input.normal.z  );

      NMDATA calc = Calculate_Bumped_Data (TBN, input.texcoord.xy, ViewEye_dir);

      input.normal      = calc.normal;
      input.texcoord.xy = calc.texcoord;
   };

   ////////////////// ENVIRONMENT REFLECTION ///////////////////

   // Convert dir to spherical coordinates       vec2 longlat = vec2(atan(dir.y,dir.x),acos(dir.z));
   // Normalize, lookup in equirectangular map   return texture2D(sampler,longlat/vec2(2.0*PI,PI)).xyz;	
   
   env_refl.xyz = reflect(ViewEye_dir, input.normal.xyz);
   env_refl.rgb = (float3)texCUBE(TexEnvCube, -env_refl.xyz);

   ////////////////////// DIRECT LIGHTS /////////////////////////

   for(i=0; i<Light_count; i++)
   if ( Light_type(i) == LIGHT_DIRECT )
   {
      vector Light_to_vertex = normalize(Light_direction(i));

      float  power = Calculate_Light_Normal_Factor  (input.normal, Light_to_vertex );
      float spower = Calculate_Light_Specular_Factor(input.normal, Light_to_vertex, ViewEye_dir, MTL_Power);

      diffuse_all  += Light_diffuse(i)  * power;
      specular_all += Light_specular(i) * power * spower;
   };

   ////////////////////// POINT LIGHTS /////////////////////////

   for(i=0; i<Light_count; i++)
   if ( Light_type(i) == LIGHT_POINT )
   {
      vector Light_to_vertex = normalize(input.world - Light_position(i));

      float  power = Calculate_Light_Normal_Factor  (input.normal, Light_to_vertex);
      float spower = Calculate_Light_Specular_Factor(input.normal, Light_to_vertex, ViewEye_dir, MTL_Power);
      float dpower = Calculate_Light_Distance_Factor
           (input.world, Light_position(i), Light_range(i), Light_att0(i), Light_att1(i), Light_att2(i));

      diffuse_all  += Light_diffuse(i)  * power * dpower;
      specular_all += Light_specular(i) * power * dpower * spower;
   };

   ////////////////////// SPOT LIGHTS /////////////////////////

   for(i=0; i<Light_count; i++)
   if ( Light_type(i) == LIGHT_SPOT )
   {
      vector Light_to_vertex = normalize(input.world - Light_position(i));
      vector Light_to_center = normalize(Light_direction(i));

      float  power = Calculate_Light_Normal_Factor  (input.normal, Light_to_vertex);
      float spower = Calculate_Light_Specular_Factor(input.normal, Light_to_vertex, ViewEye_dir, MTL_Power);
      float dpower = Calculate_Light_Distance_Factor
            (input.world, Light_position(i), Light_range(i), Light_att0(i), Light_att1(i), Light_att2(i));
      float apower = Calculate_Light_Angle_Factor
            (Light_to_center, Light_to_vertex, Light_cos_phi(i), Light_cos_theta(i), Light_falloff(i));

      diffuse_all  += Light_diffuse(i)  * power * dpower * apower;
      specular_all += Light_specular(i) * power * dpower * apower * spower;
   };

   ////////////////////// CALC RESULT MTL+LIGHT COLOR /////////////////////////
   //
   // OUT = GLOGAL ambient + 
   //       DIRECT diffuse + DIRECT specular +
   //       POINT  diffuse + POINT  specular +
   //       SPOT   diffuse + SPOT   specular
   
   env_refl.rgb *= max(0.2, MTL_Power/50.0);  // TODO: Подумать, как лучше смешивать

   vector diffuse;

   diffuse.r = MTL_Ka_r * Light_ambient.r + //* env_refl.r +
               MTL_Kd_r * diffuse_all.r +
               MTL_Ks_r * specular_all.r;

   diffuse.g = MTL_Ka_g * Light_ambient.g + //* env_refl.g +
               MTL_Kd_g * diffuse_all.g +
               MTL_Ks_g * specular_all.g;

   diffuse.b = MTL_Ka_b * Light_ambient.b + //* env_refl.b +
               MTL_Kd_b * diffuse_all.b +
               MTL_Ks_b * specular_all.b;

   diffuse.rgb += env_refl.rgb;

   diffuse.a = MTL_Alpha;

   if (isColorTexturePresent) // если есть текстура
   {
      vector texcolor = tex2D(TexDiffuse, input.texcoord.xy);
      output.diffuse.rgb = diffuse.rgb * texcolor.rgb;
      output.diffuse.a   = texcolor.a;
   }
   else output.diffuse = diffuse;

   ////////////////////// DISTANCE DISAPPEARING //////////////////////

   // Надо сверху включать ALPHABLENDENABLE
   // TODO: написать шаблон для пропадания объекта со сцены и оптимизации

      float CamToVertDist = distance(input.world.xyz, CameraPosition);
      float DisappDist    = 30.0;
      float DisappFactor  = 1.0 - saturate(CamToVertDist / DisappDist); // 1-clamp(x,0,1)
   
   // output.diffuse.a = DisappFactor;

   ////////////////////// FOG //////////////////////

   // TODO: настройки тумана задавать сверху
   // TODO: попробовать нелинейный туман (1/e^(dist))

   vector FogColor   = vector(0.5, 0.5, 0.5, 1.0);
   float  FogDist    = 40.0;
   float  DF         = saturate(CamToVertDist / FogDist); // clamp(x,0,1)
   vector DistFactor = vector(DF, DF, DF, DF);

   // output.diffuse = lerp(output.diffuse, FogColor, DistFactor); // interpolate (линейно)

   return output;
};

