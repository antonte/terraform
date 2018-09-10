#include "terrain.hpp"

#include "o2_plant.hpp"
#include "pi.hpp"
#include "rend.hpp"
#include "tree.hpp"
#include <shade/array_buffer.hpp>
#include <shade/library.hpp>
#include <shade/shader_program.hpp>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/normal.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vec_swizzle.hpp>

FastNoise Terrain::noise;

Terrain::Terrain(Library &lib) : terrainTex(lib.getTexture("terrain"))
{
  terrainTex->glBind(nullptr, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  noise.SetNoiseType(FastNoise::SimplexFractal);
}

Terrain::~Terrain()
{}

float Terrain::getZ(float x, float y)
{
  return 20.0f * noise.GetNoise(x, y);
}

static int getChunkIdx(int x, int y)
{
  return (x + Terrain::Width / 2) / TerrainChunk::ChunkSize +
         ((y + Terrain::Height / 2) / TerrainChunk::ChunkSize) *
           (Terrain::Width / TerrainChunk::ChunkSize);
}

void Terrain::draw(Rend &rend,
                   int minX,
                   int maxX,
                   int minY,
                   int maxY)
{
  rend.terrainShad->use();
  rend.mvp = glm::translate(glm::vec3(0.0f, 0.0, 0.0f)) *
               glm::rotate(-PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
  rend.mvp.update();
  terrainTex->glBind(nullptr, nullptr);

  const auto Sz = TerrainChunk::ChunkSize;
  for (int y = minY - Sz; y < maxY + Sz; y += Sz)
    for (int x = minX - Sz; x < maxX + Sz; x += Sz)
    {
      if (x < -Terrain::Width / 2 || x > Terrain::Width / 2 || y < -Terrain::Height / 2 ||
          y > Terrain::Height / 2)
        continue;
      auto it = cache.find(getChunkIdx(x, y));
      if (it == std::end(cache))
      {
        ageCache.push_front(generateChunk(x, y));
        while (ageCache.size() > MaxCacheSize)
        {
          auto it = ageCache.end();
          --it;
          auto idx = it->idx;
          ageCache.erase(it);
          cache.erase(idx);
        }
      }
      else
      {
        auto chank = std::move(*(it->second));
        ageCache.erase(it->second);
        ageCache.push_front(std::move(chank));
        cache.erase(getChunkIdx(x, y));
      }
      bool res;
      std::tie(it, res) = cache.insert(std::make_pair(getChunkIdx(x, y), std::begin(ageCache)));
      auto ageIt = it->second;
      ageIt->vertices->activate();
      ageIt->uvs->activate();
      ageIt->normals->activate();
      glDrawArrays(GL_TRIANGLES, 0, tmpVertices.size());
    }
}

static const int GridSz = 8;

static glm::vec3 norm(bool isUp, int x, int y)
{
  if (isUp)
  {
    int xx0 = x;
    int yy0 = y;
    int xx1 = x + GridSz;
    int yy1 = y + GridSz;
    int xx2 = x;
    int yy2 = y + GridSz;
    return glm::triangleNormal(glm::vec3(1.0f * xx0, 1.0f * yy0, Terrain::getZ(xx0, yy0)),
                               glm::vec3(1.0f * xx1, 1.0f * yy1, Terrain::getZ(xx1, yy1)),
                               glm::vec3(1.0f * xx2, 1.0f * yy2, Terrain::getZ(xx2, yy2)));
  }
  else
  {
    int xx0 = x;
    int yy0 = y;
    int xx1 = x + GridSz;
    int yy1 = y;
    int xx2 = x + GridSz;
    int yy2 = y + GridSz;
    return glm::triangleNormal(glm::vec3(1.0f * xx0, 1.0f * yy0, Terrain::getZ(xx0, yy0)),
                               glm::vec3(1.0f * xx1, 1.0f * yy1, Terrain::getZ(xx1, yy1)),
                               glm::vec3(1.0f * xx2, 1.0f * yy2, Terrain::getZ(xx2, yy2)));
  }
}

static glm::vec3 getNormal(int x, int y)
{
  glm::vec3 sum = glm::vec3(0.0f, 0.0f, 0.0f);

  static int dxys[][2] = {{0, 0}, {0, 0}, {-1, 0}, {-1, -1}, {-1, -1}, {0, -1}};
  static bool ups[] = {false, true, false, true, false, true};

  for (int i = 0; i < 6; ++i)
    sum += norm(ups[i], x + dxys[i][0], y + dxys[i][1]);

  return sum / 6.0f;
}

TerrainChunk Terrain::generateChunk(int xx, int yy) const
{
  tmpVertices.clear();
  tmpUvs.clear();
  tmpNormals.clear();
  const auto Sz = TerrainChunk::ChunkSize;
  int x1 = (xx + Terrain::Width / 2) / Sz * Sz - Terrain::Width / 2;
  int y1 = (yy + Terrain::Height / 2) / Sz * Sz - Terrain::Height / 2;
  for (int y = y1; y < y1 + Sz; y += GridSz)
    for (int x = x1; x < x1 + Sz; x += GridSz)
    {
      {
        auto p1 = glm::vec3(1.0f * x, 1.0f * y, getZ(x, y));
        tmpVertices.push_back(p1);
        auto p2 = glm::vec3(1.0f * x + GridSz, 1.0f * y, getZ(x + GridSz, y));
        tmpVertices.push_back(p2);
        auto p3 = glm::vec3(1.0f * x, 1.0f * y + GridSz, getZ(x, y + GridSz));
        tmpVertices.push_back(p3);

        tmpUvs.push_back(glm::xy(p1) / 100.0f);
        tmpUvs.push_back(glm::xy(p2) / 100.0f);
        tmpUvs.push_back(glm::xy(p3) / 100.0f);

        tmpNormals.push_back(getNormal(x, y));
        tmpNormals.push_back(getNormal(x + GridSz, y));
        tmpNormals.push_back(getNormal(x, y + GridSz));
      }
      {
        auto p1 = glm::vec3(1.0f * x, 1.0f * y, getZ(x, y));
        tmpVertices.push_back(p1);
        auto p2 = glm::vec3(1.0f * x, 1.0f * y + GridSz, getZ(x, y + GridSz));
        tmpVertices.push_back(p2);
        auto p3 = glm::vec3(1.0f * x - GridSz, 1.0f * y + GridSz, getZ(x - GridSz, y + GridSz));
        tmpVertices.push_back(p3);

        tmpUvs.push_back(glm::xy(p1) / 100.0f);
        tmpUvs.push_back(glm::xy(p2) / 100.0f);
        tmpUvs.push_back(glm::xy(p3) / 100.0f);

        tmpNormals.push_back(getNormal(x, y));
        tmpNormals.push_back(getNormal(x, y + GridSz));
        tmpNormals.push_back(getNormal(x - GridSz, y + GridSz));
      }
    }
  TerrainChunk chunk;
  chunk.vertices = std::make_unique<ArrayBuffer>(tmpVertices.data(), tmpVertices.size(), 0);
  chunk.uvs = std::make_unique<ArrayBuffer>(tmpUvs.data(), tmpUvs.size(), 1);
  chunk.normals = std::make_unique<ArrayBuffer>(tmpNormals.data(), tmpNormals.size(), 2);
  chunk.idx = getChunkIdx(xx, yy);
  return chunk;
}
