#include "terrain.hpp"

#include "world.hpp"
#include <log/log.hpp>
#include <sdlpp/sdlpp.hpp>
#include <shade/array_buffer.hpp>
#include <shade/library.hpp>
#include <shade/shader_program.hpp>
#include <coeff/coefficient_registry.hpp>

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

void Terrain::draw(World &world,
                   int minX,
                   int maxX,
                   int minY,
                   int maxY)
{
  world.terrainShad->use();
  world.mvp = glm::translate(glm::vec3(0.0f, 0.0, 0.0f)) *
               glm::rotate(-3.1415926f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
  world.mvp.update();
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
        bool res;
        std::tie(it, res) = cache.insert(std::make_pair(getChunkIdx(x, y), generateChunk(x, y)));
      }
      it->second.vertices->activate();
      it->second.uvs->activate();
      it->second.normals->activate();
      glDrawArrays(GL_TRIANGLES, 0, tmpVertices.size());
    }
}

TerrainChunk Terrain::generateChunk(int xx, int yy) const
{
  tmpVertices.clear();
  tmpUvs.clear();
  tmpNormals.clear();
  const auto Sz = TerrainChunk::ChunkSize;
  auto x1 = (xx + Terrain::Width / 2) / Sz * Sz - Terrain::Width / 2;
  auto y1 = (yy + Terrain::Height / 2) / Sz * Sz - Terrain::Height / 2;
  for (int y = y1; y < y1 + Sz; ++y)
    for (int x = x1; x < x1 + Sz; ++x)
    {
      {
        auto p1 = glm::vec3(x, y, getZ(x, y));
        tmpVertices.push_back(p1);
        auto p2 = glm::vec3(x + 1, y, getZ(x + 1, y));
        tmpVertices.push_back(p2);
        auto p3 = glm::vec3(x, y + 1, getZ(x, y + 1));
        tmpVertices.push_back(p3);
        auto n = glm::triangleNormal(p1, p2, p3);
        tmpNormals.push_back(n);
        tmpNormals.push_back(n);
        tmpNormals.push_back(n);
        tmpUvs.push_back(glm::xy(p1) / 100.0f);
        tmpUvs.push_back(glm::xy(p2) / 100.0f);
        tmpUvs.push_back(glm::xy(p3) / 100.0f);
      }
      {
        auto p1 = glm::vec3(x, y, getZ(x, y));
        tmpVertices.push_back(p1);
        auto p2 = glm::vec3(x, y + 1, getZ(x, y + 1));
        tmpVertices.push_back(p2);
        auto p3 = glm::vec3(x - 1, y + 1, getZ(x - 1, y + 1));
        tmpVertices.push_back(p3);
        auto n = glm::triangleNormal(p1, p2, p3);
        tmpNormals.push_back(n);
        tmpNormals.push_back(n);
        tmpNormals.push_back(n);
        tmpUvs.push_back(glm::xy(p1) / 100.0f);
        tmpUvs.push_back(glm::xy(p2) / 100.0f);
        tmpUvs.push_back(glm::xy(p3) / 100.0f);
      }
    }
  TerrainChunk chunk;
  chunk.vertices = std::make_unique<ArrayBuffer>(tmpVertices.data(), tmpVertices.size(), 0);
  chunk.uvs = std::make_unique<ArrayBuffer>(tmpUvs.data(), tmpUvs.size(), 1);
  chunk.normals = std::make_unique<ArrayBuffer>(tmpNormals.data(), tmpNormals.size(), 2);
  return chunk;
}
