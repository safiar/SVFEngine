// ----------------------------------------------------------------------- //
//
// MODULE  : Skybox.h
//
// PURPOSE : Создание и подготовка skybox
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _SKYBOX_H
#define _SKYBOX_H

#include "stdafx.h"
#include "Model.h"

using namespace SAVFGAME;

namespace SAVFGAME
{
	class CSkybox
	{
	protected:
		TBUFFER <MODELVERTEX, uint32>	vertices;		// массив вершин
		TBUFFER <uint32,      uint32>	indices;		// массив индексов
		TBUFFER <byte,        int32 >	texture;		// загруженная текстура
		float							radius;			// радиус модели сферы
		bool							cullmode_CCW;	// направление отброса невидимых граней
		bool							isInit;
	public:
		CSkybox(const CSkybox& src)				= delete;
		CSkybox(CSkybox&& src)					= delete;
		CSkybox& operator=(CSkybox&& src)		= delete;
		CSkybox& operator=(const CSkybox& src)	= delete;
	public:
		CSkybox() : isInit(false) {};
		~CSkybox() { Close(); };
		float GetSphereRadius() { return radius; }
	protected:
		void DeleteVI()  { vertices.Close(); indices.Close(); }
		void DeleteTEX() { texture.Close(); }
	public:
		void Close()
		{
			if (isInit)
			{
				DeleteVI();
				DeleteTEX();
				isInit = false;
			}
		}
	
		//>> Инициализация - загрузка текстуры и создание сферы
		bool SetSphereTexture(const wchar_t* gamePath, const wchar_t* texName, float sphere_radius, bool cull_CCW)
		{
			if (isInit) { _MBM(ERROR_InitAlready);  return false; }

			wchar_t syspath[256], error[256], p1[256], p2[256];
			GetCurrentDirectory(256,syspath);
			wsprintf(error, L"%s: %s", ERROR_ReadFile, texName);
			wsprintf(p1,L"%s\\%s\\%s",gamePath,DIRECTORY_TEXTURESSKY,texName);
			wsprintf(p2,L"%s\\%s\\%s",syspath,DIRECTORY_TEXTURESSKY,texName);

			if (!LoadCubeTexture(p1))
				if (!LoadCubeTexture(p2)) { _MBM(error); return false; }

			cullmode_CCW = cull_CCW;
			CreateSphere(sphere_radius);

			return isInit = true;
		}

	protected:
		//>> Загрузка 3d текстуры с HDD в RAM
		bool LoadCubeTexture(const wchar_t* path)
		{
			if (texture.buf != nullptr) return false;
			ReadFileInMemory(path, texture, 0);
			if (texture.buf == nullptr) return false;
			return true;
		}

		//>> Генерация сферы
		void CreateSphere(float sphere_radius)
		{
			if (vertices.buf != nullptr) return;

			if (sphere_radius < 0)
				sphere_radius *= -1;
			radius = sphere_radius;
			int nLatitude = 8;				// вертикальные линии
			int nLongitude = 8;				// горизонтальные линии
			int nFaceVerts = 4;				// прямоугольник
			int nPitch = nLongitude + 1;
			int idx = 0;

			float pitchInc = TORADIANS(180.0f / static_cast<float>(nPitch));
			float rotInc   = TORADIANS(360.0f / static_cast<float>(nLatitude));

			vertices.Create(2 + nLatitude * nLongitude);
			indices.Create(vertices.count * (nFaceVerts - 2) * 3);
	
			// Генерация координат вертексов
			int fv = 1; // fix первого вертекса
			vertices.buf[0].P._set(MATH3DVEC(0,  radius, 0));
			vertices.buf[1].P._set(MATH3DVEC(0, -radius, 0));

			for(int i=1; i<nPitch; i++)
			{
				float y  = radius * cos(static_cast<float>(i) * pitchInc);
				float xz = radius * sin(static_cast<float>(i) * pitchInc);
				if (xz < 0)
					xz *= -1;
				for(int n=0; n<nLatitude; n++)
				{
					float x = xz * cos(static_cast<float>(n) * rotInc);
					float z = xz * sin(static_cast<float>(n) * rotInc);
					vertices.buf[fv + 1 + (i - 1)*nLatitude + n].P._set(MATH3DVEC(x, y, z));
					//printf("\n%2i:%2i = x %f y %f z %f",i,n,x,y,z);
				}
			} //printf("\nnVertices %i",nVertices);

			// Индексы пар треугольников (прямоугольник), связанных самым нижним и самым верхним вертексом
			int last = fv + (nLatitude * (nLongitude-1));
			for(int i=0; i<nLatitude; i++, idx += 6)
			{
				int j = (i==nLatitude-1) ? -1 : i;
				if(cullmode_CCW)
				{
					indices.buf[idx]   = fv-1;		indices.buf[idx+3] = fv;
					indices.buf[idx+1] = fv+j+2;	indices.buf[idx+4] = i+1+last;
					indices.buf[idx+2] = fv+i+1;	indices.buf[idx+5] = j+2+last;
				}
				else
				{
					indices.buf[idx]   = fv-1;		indices.buf[idx+3] = fv;
					indices.buf[idx+2] = fv+j+2;	indices.buf[idx+5] = i+1+last;
					indices.buf[idx+1] = fv+i+1;	indices.buf[idx+4] = j+2+last;
				}
			}
			
			// Остальные индексы пар треугольников (прямоугольник)
			for(int i=1; i<nPitch-1; i++)
			for(int n=0; n<nLatitude; n++, idx+=6)
			{
				//printf("\nnIndices %i idx %i->%i",nIndices,idx,idx + 6);
				int t1 = i*nLatitude + n;
				int t2 = (n==nLatitude-1) ? t1-nLatitude : t1;
				int p0 = fv+t1+1-nLatitude;
				int p1 = fv+t2+2-nLatitude;
				int p2 = fv+t2+2;
				int p3 = fv+t1+1;
				if(cullmode_CCW)
				{
					indices.buf[idx]   = p0;	indices.buf[idx+3] = p0;
					indices.buf[idx+1] = p1;	indices.buf[idx+4] = p2;
					indices.buf[idx+2] = p2;	indices.buf[idx+5] = p3;
				}
				else
				{
					indices.buf[idx]   = p0;	indices.buf[idx+3] = p0;
					indices.buf[idx+2] = p1;	indices.buf[idx+5] = p2;
					indices.buf[idx+1] = p2;	indices.buf[idx+4] = p3;
				}
			}
		}
	
		//>> TODO
		void CreateIcosphere()
		{
			// https://schneide.wordpress.com/2016/07/15/generating-an-icosphere-in-c/

		/*	struct Triangle
			{
				Index vertex[3];
			};

			using TriangleList = std::vector<Triangle>;
			using VertexList = std::vector<v3>;

			namespace icosahedron
			{
				const float X = .525731112119133606f;
				const float Z = .850650808352039932f;
				const float N = 0.f;

				static const VertexList vertices =
				{
					{ -X, N, Z }, { X, N, Z }, { -X, N, -Z }, { X, N, -Z },
					{ N, Z, X }, { N, Z, -X }, { N, -Z, X }, { N, -Z, -X },
					{ Z, X, N }, { -Z, X, N }, { Z, -X, N }, { -Z, -X, N }
				};

				static const TriangleList triangles =
				{
					{ 0, 4, 1 }, { 0, 9, 4 }, { 9, 5, 4 }, { 4, 5, 8 }, { 4, 8, 1 },
					{ 8, 10, 1 }, { 8, 3, 10 }, { 5, 3, 8 }, { 5, 2, 3 }, { 2, 7, 3 },
					{ 7, 10, 3 }, { 7, 6, 10 }, { 7, 11, 6 }, { 11, 0, 6 }, { 0, 1, 6 },
					{ 6, 1, 10 }, { 9, 0, 11 }, { 9, 11, 2 }, { 9, 2, 5 }, { 7, 2, 11 }
				};
			} //*/

		/*	using Lookup = std::map<std::pair<Index, Index>, Index>;

			Index vertex_for_edge(Lookup& lookup,
				VertexList& vertices, Index first, Index second)
			{
				Lookup::key_type key(first, second);
				if (key.first>key.second)
					std::swap(key.first, key.second);

				auto inserted = lookup.insert({ key, vertices.size() });
				if (inserted.second)
				{
					auto& edge0 = vertices[first];
					auto& edge1 = vertices[second];
					auto point = normalize(edge0 + edge1);
					vertices.push_back(point);
				}

				return inserted.first->second;
			} //*/

		/*	TriangleList subdivide(VertexList& vertices,
				TriangleList triangles)
			{
				Lookup lookup;
				TriangleList result;

				for (auto&& each : triangles)
				{
					std::array<Index, 3> mid;
					for (int edge = 0; edge<3; ++edge)
					{
						mid[edge] = vertex_for_edge(lookup, vertices,
							each.vertex[edge], each.vertex[(edge + 1) % 3]);
					}

					result.push_back({ each.vertex[0], mid[0], mid[2] });
					result.push_back({ each.vertex[1], mid[1], mid[0] });
					result.push_back({ each.vertex[2], mid[2], mid[1] });
					result.push_back({ mid[0], mid[1], mid[2] });
				}

				return result;
			} //*/

		/*	using IndexedMesh = std::pair<VertexList, TriangleList>;

			IndexedMesh make_icosphere(int subdivisions)
			{
				VertexList vertices = icosahedron::vertices;
				TriangleList triangles = icosahedron::triangles;

				for (int i = 0; i<subdivisions; ++i)
				{
					triangles = subdivide(vertices, triangles);
				}

				return{ vertices, triangles };
			} //*/
		
		}
	};
}

#endif // _SKYBOX_H