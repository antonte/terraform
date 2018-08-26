#include "bot.hpp"
#include "stone.hpp"
#include "terrain.hpp"
#include "world.hpp"
#include <coeff/coefficient_registry.hpp>
#include <log/log.hpp>
#include <sdlpp/sdlpp.hpp>
#include <shade/library.hpp>
#include <shade/shader_program.hpp>
#include <shade/var.hpp>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

COEFF(CamMoveSpeed, 0.002f);
COEFF(CamZoomSpeed, 0.1f);
COEFF(MapXK, 0.1f);
COEFF(MapYK, 0.1f);
COEFF(MapYBalance, 50);
COEFF(MaxCamZ, 250.0f);
COEFF(MinCamZ, 20.0f);

int main()
{
  CoefficientRegistry::instance().load();
  sdl::Init init(SDL_INIT_EVERYTHING);
  const auto Width = 720 * 720 / 1280;
  const auto Height = 720;
  sdl::Window win("Terraform", 64, 100, Width, Height, SDL_WINDOW_OPENGL);
  sdl::Renderer rend(win.get(), -1, 0);
  // set up OpenGL flags
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  Var<glm::mat4> proj("proj");
  Var<glm::mat4> view("view");
  Var<glm::mat4> mvp("mvp");
  ShaderProgram shad("shad", "shad", mvp, proj, view);
  shad.use();
  proj = glm::perspective(glm::radians(45.0f), 1.0f * Width / Height, 0.1f, 1000.0f);
  auto camX = 0.0f;
  auto camY = 0.0f;
  auto camZ = 20.0f;
  Library lib(rend.get());
  World world(lib);
  for (int x = -10; x < 10; x += 3)
    for (int y = -10; y < 10; y += 3)
      world.add(std::make_unique<Bot>(world, x, y));
  for (auto i = 0; i < 300000; ++i)
    world.add(
      std::make_unique<Stone>(world,
                              rand() % (Terrain::Width * 10) / 10.0f - Terrain::Width / 2,
                              rand() % (Terrain::Height * 10) / 10.0f - Terrain::Height / 2));
  //  world.add(std::make_unique<Stone>(world, 3.0f, 3.0f));
  auto done = false;
  sdl::EventHandler evHand;
  evHand.quit = [&done](const SDL_QuitEvent &) { done = true; };
  evHand.mouseMotion = [&camX, &camY, &camZ](const SDL_MouseMotionEvent &e) {
    if ((e.state & SDL_BUTTON_MMASK) == 0)
      return;
    camX -= CamMoveSpeed * e.xrel * camZ;
    camY += CamMoveSpeed * e.yrel * camZ;
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

  while (!done)
  {
    while (evHand.poll()) {}
    glClearColor(0.2f, 0.5f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shad.use();
    view = glm::lookAt(glm::vec3(camX, camY - camZ, camZ),
                       glm::vec3(camX, camY, 0.0f), // and looks at the origin
                       glm::vec3(0, 0, 1));
    view.update();
    proj.update();
    view.update();

    world.draw(mvp,
               camX - camZ * MapXK,
               camX + camZ * MapXK,
               camY - camZ * MapYK * MapYBalance / 100,
               camY + camZ * MapYK);
    rend.present();
    ++frames;
    if (SDL_GetTicks() > nextMeasure)
    {
      LOG("fps", frames);
      frames = 0;
      nextMeasure += 1000;
    }
    while (SDL_GetTicks() > tickTime)
    {
      world.tick();
      tickTime += 10;
    }
  }
}
