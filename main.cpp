#include "bot.hpp"
#include "bot_class.hpp"
#include "button.hpp"
#include "inst.hpp"
#include "rend.hpp"
#include "screen.hpp"
#include "stone.hpp"
#include "terrain.hpp"
#include "tree.hpp"
#include "ui.hpp"
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

  Inst inst(lib);
  for (auto i = 0; i < 100000;)
  {
    auto x = rand() % (Terrain::Width * 10) / 10.0f - Terrain::Width / 2;
    auto y = rand() % (Terrain::Height * 10) / 10.0f - Terrain::Height / 2;
    auto p = Terrain::getZ(x, y) + 20;
    if (rand() % 40 < p)
    {
      inst.world->add<Stone>(x, y);
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

  // Obj* cross = lib.getObj("cross");
  Text text(lib, "font");
  int fps = 0;

  Ui ui;
  evHand.mouseButtonDown = [&ui, &mouseDown](const SDL_MouseButtonEvent &e) {
    LOG("mouse button down", e.x, e.y);
    if (e.button != SDL_BUTTON_LEFT)
      return;
    if (ui.onMouseDown(e.x, e.y))
      return;
    mouseDown = true;
  };
  evHand.mouseButtonUp =
    [&ui, &mouseDown](const SDL_MouseButtonEvent &e) {
      LOG("mouse button up", e.x, e.y);
      if (e.button != SDL_BUTTON_LEFT)
        return;
      if (ui.onMouseUp(e.x, e.y))
        return;
      mouseDown = false;
      if (abs(e.x - ScreenWidth / 2) > 40 || abs(e.y - (ScreenHeight - 60)) > 40)
        return;
    };

  Button botBtn(ScreenWidth / 2 - 30, ScreenHeight - 60.0f - 100, 60, 100);
  botBtn.onDraw = [&mvp, &inst](bool pressed) {
    mvp = glm::translate(glm::vec3(ScreenWidth / 2, ScreenHeight - 60.0f, 0.0f)) *
          glm::scale(glm::vec3(20.0f, 20.0f, 20.0f) * (pressed ? 1.3f : 1.0f));
    mvp.update();
    inst.rend->botClass
      ->idleAnim[SDL_GetTicks() * 30 / 1000 % inst.rend->botClass->idleAnim.size()]
      ->draw();
  };
  botBtn.onClick = [&camX, &camY, &inst]() {
    if (inst.world->money < 100'000'000)
      return;
    BotSpecs specs;
    specs.maxAge = 20000;
    specs.batteryCapacity = 2000;
    specs.ram = Program::Default.data();
    inst.world->money -= 100'000'000;
    inst.world->add<Bot>(
      camX + rand() % 200 / 10.f - 10.0f, camY + rand() % 200 / 10.f - 10.0f, specs);
  };

  ui.add(botBtn);

  Obj *planet = lib.getObj("planet");
  Button planetBtn(30 - 20, 30 - 20, 40, 40);
  planetBtn.onDraw = [&mvp, &planet](bool pressed) {
    mvp = glm::translate(glm::vec3(30.0f, 30.0f, 1.0f)) *
          glm::rotate(SDL_GetTicks() / 300.0f, glm::vec3(0.0f, 1.0f, 0.0f)) *
          glm::scale(glm::vec3(20.0f, 20.0f, 20.0f) * (pressed ? 1.3f : 1.0f));
    mvp.update();
    planet->draw();
  };
  ui.add(planetBtn);

  Obj *lab = lib.getObj("lab");
  Button labBtn(30 - 20, ScreenHeight - 60.0f - 20, 40, 40);
  labBtn.onDraw = [&mvp, &lab](bool pressed) {
    mvp = glm::translate(glm::vec3(30.0f, ScreenHeight - 60.0f, 1.0f)) *
          glm::rotate(SDL_GetTicks() / 300.0f, glm::vec3(0.0f, 1.0f, 0.0f)) *
          glm::scale(glm::vec3(20.0f, 20.0f, 20.0f) * (pressed ? 1.3f : 1.0f));
    mvp.update();
    lab->draw();
  };
  ui.add(labBtn);

  Obj *menu = lib.getObj("menu");
  Button menuBtn(ScreenWidth - 30 - 20, ScreenHeight - 60.0f - 20, 40, 40);
  menuBtn.onDraw = [&mvp, &menu](bool pressed) {
    mvp = glm::translate(glm::vec3(ScreenWidth - 30.0f, ScreenHeight - 60.0f, 1.0f)) *
          glm::scale(glm::vec3(20.0f, 20.0f, 20.0f) * (pressed ? 1.3f : 1.0f));
    mvp.update();
    menu->draw();
  };
  ui.add(menuBtn);

  while (!done)
  {
    while (evHand.poll()) {}

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    inst.world->draw(*inst.rend, camX, camY, camZ);

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

    ui.draw();

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
            << 100.0f * (inst.world->getO2Level() + inst.world->getH2OLevel() + inst.world->getTreeLevel()) / 3.0f
            << "%";
      text.setText(stats.str());
      text.draw();
    }

    {
      mvp = glm::translate(glm::vec3(ScreenWidth / 2, 10.0f, 0.0f)) *
            glm::scale(glm::vec3(30.0f, 30.0f, 30.0f));
      mvp.update();
      std::ostringstream stats;
      stats << "$" << std::fixed << inst.world->money << "\n" //
            << "$" << std::fixed << inst.world->getIncome() << "/sec";
      text.setText(stats.str());
      text.draw();
    }

    rend.present();

    auto currentTick = SDL_GetTicks();
    while (currentTick > tickTime)
    {
      inst.world->tick();
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
