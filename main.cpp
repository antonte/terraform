#include "bot.hpp"
#include "bot_class.hpp"
#include "screen.hpp"
#include "stone.hpp"
#include "terrain.hpp"
#include "tree.hpp"
#include "world.hpp"
#include <coeff/coefficient_registry.hpp>
#include <iomanip>
#include <log/log.hpp>
#include <sdlpp/sdlpp.hpp>
#include <shade/library.hpp>
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>
#include <shade/text.hpp>
#include <shade/var.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>

COEFF(CamMoveSpeed, 0.002f);
COEFF(CamInertia, 0.002f);
COEFF(CamZoomSpeed, 0.1f);
COEFF(MaxCamZ, 250.0f);
COEFF(MinCamZ, 20.0f);
COEFF(CamSpeed, 0.001f);

int main()
{
  CoefficientRegistry::instance().load();
  sdl::Init init(SDL_INIT_EVERYTHING);
  sdl::Window win("Terraform", 64, 100, ScreenWidth, ScreenHeight, SDL_WINDOW_OPENGL);
  sdl::Renderer rend(win.get(), -1, 0);
  // set up OpenGL flags
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  auto camX = 0.0f;
  auto camY = 0.0f;
  auto camPrevX = 0.0f;
  auto camPrevY = 0.0f;
  auto camVX = 0.0f;
  auto camVY = 0.0f;
  auto camZ = 180.0f;
  Library lib(rend.get());

  World world(lib);
  for (auto i = 0; i < 100000;)
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
  bool mouseDown = false;
  evHand.mouseMotion = [&camX, &camY, &camZ, &mouseDown](const SDL_MouseMotionEvent &e) {
    mouseDown = ((e.state & SDL_BUTTON_LMASK) != 0);
    if (!mouseDown)
      return;
    camX -= CamMoveSpeed * e.xrel * camZ;
    camY += CamMoveSpeed * e.yrel * camZ;
    camX = glm::clamp(camX, -Terrain::Width / 2.0f, Terrain::Width / 2.0f);
    camY = glm::clamp(camY, -Terrain::Height / 2.0f, Terrain::Height / 2.0f);
  };
  evHand.mouseWheel = [&camZ](const SDL_MouseWheelEvent &e) {
    camZ = camZ * exp(e.y * CamZoomSpeed);
    camZ = glm::clamp(camZ, MinCamZ, MaxCamZ);
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
  ShaderProgram uiShad("ui", "ui", proj, mvp);

  Obj* lab = lib.getObj("lab");
  Obj* planet = lib.getObj("planet");
  // Obj* cross = lib.getObj("cross");
  Obj* menu = lib.getObj("menu");

  Text text(lib, "font");
  int fps = 0;

  bool botPressed = false;

  evHand.mouseButtonDown = [&botPressed, &mouseDown](const SDL_MouseButtonEvent &e) {
    LOG("mouse button down", e.x, e.y);
    if (e.button != SDL_BUTTON_LEFT)
      return;
    mouseDown = true;
    if (abs(e.x - ScreenWidth / 2) > 40 || abs(e.y - (ScreenHeight - 60)) > 40)
      return;
    botPressed = true;
  };
  evHand.mouseButtonUp = [&botPressed, &world, &camX, &camY, &mouseDown](
                           const SDL_MouseButtonEvent &e) {
    botPressed = false;
    LOG("mouse button up", e.x, e.y);
    if (e.button != SDL_BUTTON_LEFT)
      return;
    mouseDown = false;
    if (abs(e.x - ScreenWidth / 2) > 40 || abs(e.y - (ScreenHeight - 60)) > 40)
      return;
    if (world.money < 100'000'000)
      return;
    BotSpecs specs;
    specs.maxAge = 20000;
    specs.batteryCapacity = 2000;
    specs.ram = Program::Default.data();
    world.money -= 100'000'000;
    world.add(std::make_unique<Bot>(
      world, camX + rand() % 200 / 10.f - 10.0f, camY + rand() % 200 / 10.f - 10.0f, specs));
  };

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
    }

    // UI section

    proj = glm::ortho(0.0f, 1.0f * ScreenWidth, 1.0f * ScreenHeight, 0.0f, -1000.0f, 1000.0f);
    uiShad.use();
    mvp = glm::translate(glm::vec3(ScreenWidth / 2, ScreenHeight - 60.0f, 0.0f)) *
          glm::scale(glm::vec3(20.0f, 20.0f, 20.0f) * (botPressed ? 1.3f : 1.0f));
    mvp.update();
    world.botClass->idleAnim[SDL_GetTicks() * 30 / 1000 % world.botClass->idleAnim.size()]->draw();
    mvp = glm::translate(glm::vec3(30.0f, 30.0f, 1.0f)) *
          glm::rotate(SDL_GetTicks() / 300.0f, glm::vec3(0.0f, 1.0f, 0.0f)) *
          glm::scale(glm::vec3(20.0f, 20.0f, 20.0f));
    mvp.update();
    planet->draw();

    mvp = glm::translate(glm::vec3(30.0f, ScreenHeight - 60.0f, 1.0f)) *
          glm::rotate(SDL_GetTicks() / 300.0f, glm::vec3(0.0f, 1.0f, 0.0f)) *
          glm::scale(glm::vec3(20.0f, 20.0f, 20.0f));
    mvp.update();
    lab->draw();

    mvp = glm::translate(glm::vec3(ScreenWidth - 30.0f, ScreenHeight - 60.0f, 0.0f)) *
          glm::scale(glm::vec3(20.0f, 20.0f, 20.0f));
    mvp.update();
    menu->draw();

    glDisable(GL_DEPTH_TEST);
    color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    textShad.use();

    {
      mvp =
        glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(20.0f, 20.0f, 20.0f));
      mvp.update();
      text.setText("FPS: " + std::to_string(fps));
      text.draw();
    }

    {
      mvp =
        glm::translate(glm::vec3(60.0f, 10.0f, 0.0f)) * glm::scale(glm::vec3(30.0f, 30.0f, 30.0f));
      mvp.update();
      std::ostringstream stats;
      stats << std::fixed
            << 100.0f * (world.getO2Level() + world.getH2OLevel() + world.getTreeLevel()) / 3.0f
            << "%";
      text.setText(stats.str());
      text.draw();
    }

    {
      mvp = glm::translate(glm::vec3(ScreenWidth / 2, 10.0f, 0.0f)) *
            glm::scale(glm::vec3(30.0f, 30.0f, 30.0f));
      mvp.update();
      std::ostringstream stats;
      stats << "$" << std::fixed << world.money << "\n" //
            << "$" << std::fixed << world.getIncome() << "/sec";
      text.setText(stats.str());
      text.draw();
    }

    rend.present();

    auto currentTick = SDL_GetTicks();
    while (currentTick > tickTime)
    {
      world.tick();
      tickTime += 10;
      camVX = (1.0f - CamInertia) * camVX + CamInertia * (camX - camPrevX);
      camVY = (1.0f - CamInertia) * camVY + CamInertia * (camY - camPrevY);

      if (!mouseDown)
      {
        camX += camVX * 0.99f;
        camY += camVY * 0.99f;
      }

      camPrevX = camX;
      camPrevY = camY;
    }
    if (currentTick > tickTime + 100)
      tickTime = currentTick;
  }
}
