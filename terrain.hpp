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

struct TerrainChunk
{
  static const int ChunkSize = 50;
  std::unique_ptr<ArrayBuffer> vertices;
  std::unique_ptr<ArrayBuffer> uvs;
  std::unique_ptr<ArrayBuffer> normals;
};

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
  FastNoise noise;
  sdl::Texture *terrainTex;
  mutable std::vector<glm::vec3> tmpVertices;
  mutable std::vector<glm::vec2> tmpUvs;
  mutable std::vector<glm::vec3> tmpNormals;
  std::unordered_map<int, TerrainChunk> cache;
  TerrainChunk generateChunk(int x, int y) const;
};
