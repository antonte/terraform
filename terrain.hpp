#pragma once
#include <fastnoise/FastNoise.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

class ArrayBuffer;
class Library;
class Rend;

namespace sdl
{
  class Texture;
}

struct TerrainChunk
{
  static const int ChunkSize = 100;
  int idx;
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
  static float getZ(float x, float y);
  void draw(Rend &,
            int minX,
            int maxX,
            int minY,
            int maxY);

private:
  static FastNoise noise;
  sdl::Texture *terrainTex;
  mutable std::vector<glm::vec3> tmpVertices;
  mutable std::vector<glm::vec2> tmpUvs;
  mutable std::vector<glm::vec3> tmpNormals;
  enum { MaxCacheSize = 250 };
  using AgeCache = std::list<TerrainChunk>;
  using Cache = std::unordered_map<int, AgeCache::iterator>;
  Cache cache;
  AgeCache ageCache;
  TerrainChunk generateChunk(int x, int y) const;
};
