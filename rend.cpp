#include "rend.hpp"

#include "bot_class.hpp"
#include "stone_class.hpp"
#include "terrain.hpp"
#include <shade/shader_program.hpp>
#include <shade/obj.hpp>

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
    proj("proj"),
    view("view"),
    mvp("mvp"),
    color("textColor"),
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
    textShad(std::make_unique<ShaderProgram>("text", "text", proj, mvp, color)),
    uiShad(std::make_unique<ShaderProgram>("ui", "ui", proj, mvp)),

    o2PlantObj(std::make_unique<Obj>(lib, "o2_plant")),
    h2OPlantObj(std::make_unique<Obj>(lib, "h2o_plant")),
    treeObj(std::make_unique<Obj>(lib, "tree")),
    waterMesh(std::make_unique<ArrayBuffer>(getWaterMesh(), 0)),
    waterShad(std::make_unique<ShaderProgram>("water", "water", mvp, proj, view, h2OLevel, time)),
    planeShad(std::make_unique<ShaderProgram>("plane", "plane", mvp, proj, time))
{
}

Rend::~Rend() {}
