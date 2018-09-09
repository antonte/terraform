#pragma once
#include <glm/vec3.hpp>
#include <memory>
#include <shade/var.hpp>
#include <vector>

class ArrayBuffer;
class BotClass;
class Library;
class Obj;
class ShaderProgram;
class StoneClass;

class Rend
{
public:
  Rend(Library &);
  ~Rend();
  std::unique_ptr<BotClass> botClass;
  std::unique_ptr<StoneClass> stoneClass;
  std::vector<glm::vec3> stoneMvps;

  Var<float> o2Level;
  Var<float> h2OLevel;
  Var<float> time;
  Var<glm::mat4> proj;
  Var<glm::mat4> view;

  Var<glm::mat4> mvp;
  Var<glm::vec4> color;

  std::unique_ptr<ShaderProgram> shad;
  std::unique_ptr<ShaderProgram> botShad;
  std::unique_ptr<ShaderProgram> terrainShad;
  std::unique_ptr<ShaderProgram> buildShad;
  std::unique_ptr<ShaderProgram> stoneShad;
  std::unique_ptr<ShaderProgram> textShad;
  std::unique_ptr<ShaderProgram> uiShad;

  std::unique_ptr<Obj> o2PlantObj;
  std::unique_ptr<Obj> h2OPlantObj;
  std::unique_ptr<Obj> treeObj;

  std::unique_ptr<ArrayBuffer> waterMesh;
  std::unique_ptr<ShaderProgram> waterShad;
  std::unique_ptr<ShaderProgram> planeShad;
};
