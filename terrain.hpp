#pragma once
#include <fastnoise/FastNoise.h>
#include <memory>
#include <shade/var.hpp>
#include <unordered_map>
#include <vector>

class Library;
class ArrayBuffer;
namespace sdl
{
  class Texture;
}

class Terrain
{
public:
  static const int Width = 5000;
  static const int Height = 5000;

  Terrain(Library &);
  ~Terrain();
  float getZ(int x, int y) const;
  void draw(Var<glm::mat4> &mvp, int minX, int maxX, int minY, int maxY);

private:
  mutable std::unordered_map<uint32_t, float> z;
  FastNoise noise;
  sdl::Texture *terrainTex;
  std::vector<glm::vec3> tmpVertices;
  std::vector<glm::vec2> tmpUvs;
  std::vector<glm::vec3> tmpNormals;
  std::unique_ptr<ArrayBuffer> vertices;
  std::unique_ptr<ArrayBuffer> uvs;
  std::unique_ptr<ArrayBuffer> normals;
  int lastMinX = 0;
  int lastMaxX = 0;
  int lastMinY = 0;
  int lastMaxY = 0;
};
