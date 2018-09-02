#include "bot.hpp"
#include "stone.hpp"
#include "terrain.hpp"
#include "world.hpp"
#include "tree.hpp"
#include <coeff/coefficient_registry.hpp>
#include <log/log.hpp>
#include <sdlpp/sdlpp.hpp>
#include <shade/library.hpp>
#include <shade/shader_program.hpp>
#include <shade/var.hpp>
#include <shade/text.hpp>
#include <iomanip>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>

COEFF(CamMoveSpeed, 0.002f);
COEFF(CamZoomSpeed, 0.1f);
COEFF(MaxCamZ, 250.0f);
COEFF(MinCamZ, 20.0f);
COEFF(CamSpeed, 0.001f);

int main()
{
  CoefficientRegistry::instance().load();
  sdl::Init init(SDL_INIT_EVERYTHING);
  sdl::Window win("Terraform", 64, 100, World::ScreenWidth, World::ScreenHeight, SDL_WINDOW_OPENGL);
  sdl::Renderer rend(win.get(), -1, 0);
  // set up OpenGL flags
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  auto camX = 0.0f;
  auto camY = 0.0f;
  auto camZ = 180.0f;
  Library lib(rend.get());

  World world(lib);
  world.add(std::make_unique<Bot>(world, 0, 0, 2000, 200, 20000));
  for (auto i = 0; i < 100000 / 25;)
  {
    auto x = rand() % (Terrain::Width * 10) / 10.0f - Terrain::Width / 2;
    auto y = rand() % (Terrain::Height * 10) / 10.0f - Terrain::Height / 2;
    auto p = Terrain::getZ(x, y) + 20;
    if (rand() % 40 < p)
    {
      world.add(std::make_unique<Stone>(world, x, y));
      ++i;
    }
  }
  auto done = false;
  sdl::EventHandler evHand;
  evHand.quit = [&done](const SDL_QuitEvent &) { done = true; };
  evHand.mouseMotion = [&camX, &camY, &camZ](const SDL_MouseMotionEvent &e) {
    if ((e.state & SDL_BUTTON_MMASK) == 0)
      return;
    camX -= CamMoveSpeed * e.xrel * camZ;
    camY += CamMoveSpeed * e.yrel * camZ;
    camX = std::max(std::min(camX, Terrain::Width / 2.0f), -Terrain::Width / 2.0f);
    camY = std::max(std::min(camY, Terrain::Height / 2.0f), -Terrain::Height / 2.0f);
  };
  evHand.mouseWheel = [&camZ](const SDL_MouseWheelEvent &e) {
    camZ = camZ * exp(e.y * CamZoomSpeed);
    camZ = std::max(MinCamZ, std::min(camZ, MaxCamZ));
    LOG("camZ", camZ);
  };
  evHand.keyDown = [](const SDL_KeyboardEvent &keyEv) {
    if (CoefficientRegistry::instance().onKeyDown(keyEv.keysym.sym))
    {
      // coeffText.setText(CoefficientRegistry::instance().display());
      return;
    }
  };

  int frames = 0;
  auto nextMeasure = SDL_GetTicks() + 1000U;
  auto tickTime = SDL_GetTicks();

  Var<glm::mat4> proj("proj");
  Var<glm::mat4> mvp("mvp");
  Var<glm::vec4> color("textColor");
  ShaderProgram textShad("text", "text", proj, mvp, color);

  Text text(lib, "font");
  int fps = 0;

  const Bot *botCam = nullptr;

  while (!done)
  {
    while (evHand.poll()) {}

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    world.draw(camX, camY, camZ);

    ++frames;
    if (SDL_GetTicks() > nextMeasure)
    {
      LOG("fps", frames);
      fps = frames;
      frames = 0;
      nextMeasure += 1000;
      if (!world.isAlive(*botCam))
        botCam = world.getFirstBot();
    }

    glDisable(GL_DEPTH_TEST);
    proj = glm::ortho(0.0f, 1.0f * World::ScreenWidth, 1.0f * World::ScreenHeight, 0.0f);
    textShad.use();

    mvp = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) * scale(glm::vec3(20.0f, 20.0f, 1.0f));
    color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    mvp.update();
    color.update();
    text.setText("FPS: " + std::to_string(fps));
    text.draw();

    mvp = glm::translate(glm::vec3(0.0f, 20.0f, -1.0f)) * scale(glm::vec3(20.0f, 20.0f, 1.0f));
    color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    mvp.update();
    color.update();
    std::ostringstream stats;
    stats << "O2: " << world.getO2Level()        //
          << "\nH2O: " << world.getH2OLevel()    //
          << "\nTrees: " << world.getTreeLevel() //
      ;
    text.setText(stats.str());
    text.draw();

    rend.present();

    auto currentTick = SDL_GetTicks();
    while (currentTick > tickTime)
    {
      world.tick();
      tickTime += 2;
      if (world.isAlive(*botCam) && botCam)
      {
        camX = (1.0f - CamSpeed) * camX + CamSpeed * botCam->getX();
        camY = (1.0f - CamSpeed) * camY + CamSpeed * botCam->getY();
      }
    }
    if (currentTick > tickTime + 100)
      tickTime = currentTick;
  }
}
