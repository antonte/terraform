#include "rend.hpp"
#include "bot_class.hpp"
#include "screen.hpp"
#include "stone_class.hpp"
#include "terrain.hpp"
#include <shade/array_buffer.hpp>
#include <shade/library.hpp>
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

static std::vector<glm::vec3> getWaterMesh()
{
  std::vector<glm::vec3> res;
  res.push_back(glm::vec3(-1.0f * Terrain::Width / 2.0f, -1.0f * Terrain::Height / 2.0f, 0.0f));
  res.push_back(glm::vec3(1.0f * Terrain::Width / 2.0f, 1.0f * Terrain::Height / 2.0f, 0.0f));
  res.push_back(glm::vec3(1.0f * Terrain::Width / 2.0f, -1.0f * Terrain::Height / 2.0f, 0.0f));

  res.push_back(glm::vec3(-1.0f * Terrain::Width / 2.0f, -1.0f * Terrain::Height / 2.0f, 0.0f));
  res.push_back(glm::vec3(-1.0f * Terrain::Width / 2.0f, 1.0f * Terrain::Height / 2.0f, 0.0f));
  res.push_back(glm::vec3(1.0f * Terrain::Width / 2.0f, 1.0f * Terrain::Height / 2.0f, 0.0f));
  return res;
}

Rend::Rend(Library &lib)
  : botClass(std::make_unique<BotClass>(lib)),
    stoneClass(std::make_unique<StoneClass>(lib)),

    o2Level("o2Level", 0.0f),
    h2OLevel("h2OLevel", 0.0f),
    time("time"),
    proj("proj",
         glm::perspective(glm::radians(45.0f), 1.0f * ScreenWidth / ScreenHeight, 0.1f, 1000.0f)),
    view("view"),
    mvp("mvp"),
    shad(std::make_unique<ShaderProgram>("shad", "shad", mvp, proj, view)),
    botShad(std::make_unique<ShaderProgram>("bot",
                                            "bot",
                                            mvp,
                                            proj,
                                            view,
                                            botClass->energy,
                                            botClass->matter)),
    terrainShad(
      std::make_unique<ShaderProgram>("terrain", "terrain", mvp, proj, view, o2Level, h2OLevel)),
    buildShad(std::make_unique<ShaderProgram>("build", "build", mvp, proj, view)),
    stoneShad(std::make_unique<ShaderProgram>("stone", "stone", proj, view)),

    o2PlantObj(std::make_unique<Obj>(lib, "o2_plant")),
    h2OPlantObj(std::make_unique<Obj>(lib, "h2o_plant")),
    treeObj(std::make_unique<Obj>(lib, "tree")),
    waterMesh(std::make_unique<ArrayBuffer>(getWaterMesh(), 0)),
    waterShad(std::make_unique<ShaderProgram>("water", "water", mvp, proj, view, h2OLevel, time))
{
}

Rend::~Rend() {}
