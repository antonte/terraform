#include "terrain.hpp"

#include <log/log.hpp>
#include <sdlpp/sdlpp.hpp>
#include <shade/array_buffer.hpp>
#include <shade/library.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/normal.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vec_swizzle.hpp>

Terrain::Terrain(Library &lib) : terrainTex(lib.getTexture("terrain"))
{
  noise.SetNoiseType(FastNoise::SimplexFractal);
}

Terrain::~Terrain()
{}

float Terrain::getZ(int x, int y) const
{
  x = x + Width / 2;
  y = y + Height / 2;
  if (x < 0 || y < 0 || x >= Width || y >= Height)
    return 0.0f;
  auto idx = x + y * Width;
  auto it = z.find(idx);
  if (it == std::end(z))
  {
    bool res;
    std::tie(it, res) = z.emplace(idx, 10.0f * noise.GetNoise(x, y));
  }
  return it->second;
}

void Terrain::draw(Var<glm::mat4> &mvp, int minX, int maxX, int minY, int maxY)
{
  if (minX != lastMinX || maxX != lastMaxX || minY != lastMinY || maxY != lastMaxY || !vertices ||
      !uvs || !normals)
  {
    lastMinX = minX;
    lastMaxX = maxX;
    lastMinY = minY;
    lastMaxY = maxY;
    tmpVertices.clear();
    tmpUvs.clear();
    tmpNormals.clear();
    auto yStep = std::max(1, (maxY - minY) / 100);
    auto xStep = std::max(1, (maxX - minX) / 100);
    for (int y = minY; y < maxY; y += yStep)
      for (int x = minX; x < maxX; x += xStep)
      {
        {
          auto p1 = glm::vec3(x, y, getZ(x, y));
          tmpVertices.push_back(p1);
          auto p2 = glm::vec3(x + xStep, y, getZ(x + xStep, y));
          tmpVertices.push_back(p2);
          auto p3 = glm::vec3(x, y + yStep, getZ(x, y + yStep));
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
          auto p2 = glm::vec3(x, y + yStep, getZ(x, y + yStep));
          tmpVertices.push_back(p2);
          auto p3 = glm::vec3(x - xStep, y + yStep, getZ(x - xStep, y + yStep));
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
    vertices = std::make_unique<ArrayBuffer>(tmpVertices.data(), tmpVertices.size(), 0);
    uvs = std::make_unique<ArrayBuffer>(tmpUvs.data(), tmpUvs.size(), 1);
    normals = std::make_unique<ArrayBuffer>(tmpNormals.data(), tmpNormals.size(), 2);
  }
  vertices->activate();
  uvs->activate();
  normals->activate();
  mvp = glm::translate(glm::vec3(0.0f, 0.0, 0.0f)) *
        glm::rotate(-3.1415926f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
  mvp.update();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  terrainTex->glBind(nullptr, nullptr);
  glDrawArrays(GL_TRIANGLES, 0, tmpVertices.size());
}
