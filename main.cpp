#include "asm.hpp"
#include "bot.hpp"
#include "bot_class.hpp"
#include "bot_specs.hpp"
#include "button.hpp"
#include "inst.hpp"
#include "lab.hpp"
#include "rend.hpp"
#include "screen.hpp"
#include "stone.hpp"
#include "terrain.hpp"
#include "ui.hpp"
#include "world.hpp"
#include <coeff/coeff_reg.hpp>
#include <memory>
#include <sdlpp/sdlpp.hpp>
#include <shade/library.hpp>
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>
#include <shade/text.hpp>
// workaround for recursive dependency bug
#include <SDL.h>

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
COEFF(FontSize, 20.0f);

int main()
{
  CoeffReg::instance().load();
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

  Inst inst(rend);
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
  };
  evHand.keyDown = [](const SDL_KeyboardEvent &keyEv) {
    if (CoeffReg::instance().onKeyDown(keyEv.keysym.sym))
    {
      // coeffText.setText(CoefficientRegistry::instance().display());
      return;
    }
  };

  int frames = 0;
  auto nextMeasure = SDL_GetTicks() + 1000U;
  auto tickTime = SDL_GetTicks();

  // Obj* cross = lib.getObj("cross");
  Text text(*inst.rend->lib, "font");
  int fps = 0;

  Ui ui;
  evHand.mouseButtonDown = [&ui, &mouseDown](const SDL_MouseButtonEvent &e) {
    if (e.button != SDL_BUTTON_LEFT)
      return;
    if (ui.onMouseDown(e.x, e.y))
      return;
    mouseDown = true;
  };
  evHand.mouseButtonUp =
    [&ui, &mouseDown](const SDL_MouseButtonEvent &e) {
      if (e.button != SDL_BUTTON_LEFT)
        return;
      if (ui.onMouseUp(e.x, e.y))
        return;
      mouseDown = false;
      if (abs(e.x - ScreenWidth / 2) > 40 || abs(e.y - (ScreenHeight - 60)) > 40)
        return;
    };

  Button botBtn(ScreenWidth / 2 - 30, ScreenHeight - 60.0f - 100, 60, 100);
  botBtn.onDraw = [&inst](bool pressed) {
    inst.rend->mvp = glm::translate(glm::vec3(ScreenWidth / 2, ScreenHeight - 60.0f, 0.0f)) *
          glm::scale(glm::vec3(20.0f, 20.0f, 20.0f) * (pressed ? 1.3f : 1.0f));
    inst.rend->mvp.update();
    inst.rend->botClass
      ->idleAnim[SDL_GetTicks() * 30 / 1000 % inst.rend->botClass->idleAnim.size()]
      ->draw();
  };
  botBtn.onClick = [&camX, &camY, &inst]() {
    if (inst.world->money < inst.world->specs.botPrice)
      return;
    inst.world->money -= inst.world->specs.botPrice;
    inst.world->add<Bot>(
      camX + rand() % 200 / 10.f - 10.0f, camY + rand() % 200 / 10.f - 10.0f, inst.world->specs);
  };

  ui.add(botBtn);

  Obj *planet = inst.rend->lib->getObj("planet");
  Button planetBtn(50 - 20, 30 - 20, 40, 40);
  planetBtn.onDraw = [&inst, &planet](bool pressed) {
    inst.rend->mvp = glm::translate(glm::vec3(50.0f, 30.0f, 1.0f)) *
          glm::rotate(SDL_GetTicks() / 300.0f, glm::vec3(0.0f, 1.0f, 0.0f)) *
          glm::scale(glm::vec3(20.0f, 20.0f, 20.0f) * (pressed ? 1.3f : 1.0f));
    inst.rend->mvp.update();
    planet->draw();
  };
  ui.add(planetBtn);

  Lab labDlg(*inst.rend->lib, inst, ui, inst.world->specs);

  Obj *lab = inst.rend->lib->getObj("lab");
  Button labBtn(50 - 20, ScreenHeight - 60.0f - 20, 40, 40);
  labBtn.onDraw = [&inst, &lab](bool pressed) {
    inst.rend->mvp = glm::translate(glm::vec3(50.0f, ScreenHeight - 60.0f, 1.0f)) *
                     glm::rotate(SDL_GetTicks() / 300.0f, glm::vec3(0.0f, 1.0f, 0.0f)) *
                     glm::scale(glm::vec3(20.0f, 20.0f, 20.0f) * (pressed ? 1.3f : 1.0f));
    inst.rend->mvp.update();
    lab->draw();
  };
  labBtn.onClick = [&labDlg]() {
    if (labDlg.isVisible())
      labDlg.hide();
    else
      labDlg.show();
  };
  ui.add(labBtn);

  Obj *menu = inst.rend->lib->getObj("menu");
  Button menuBtn(ScreenWidth - 50 - 20, ScreenHeight - 60.0f - 20, 40, 40);
  menuBtn.onDraw = [&inst, &menu](bool pressed) {
    inst.rend->mvp = glm::translate(glm::vec3(ScreenWidth - 50.0f, ScreenHeight - 60.0f, 1.0f)) *
          glm::scale(glm::vec3(20.0f, 20.0f, 20.0f) * (pressed ? 1.3f : 1.0f));
    inst.rend->mvp.update();
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

    inst.rend->proj =
      glm::perspective(glm::radians(45.0f), 1.0f * ScreenWidth / ScreenHeight, 0.1f, 1000.0f);
    inst.world->draw(*inst.rend, camX, camY, camZ);

    ++frames;
    if (SDL_GetTicks() > nextMeasure)
    {
      fps = frames;
      frames = 0;
      nextMeasure += 1000;
    }

    // UI section
    inst.rend->proj = glm::ortho(0.0f, 1.0f * ScreenWidth, 1.0f * ScreenHeight, 0.0f, -1000.0f, 1000.0f);
    inst.rend->uiShad->use();

    ui.draw();

    inst.rend->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    inst.rend->textShad->use();

    {
      text.setText("FPS: " + std::to_string(fps));
      auto w = text.getWidth() * 20.0f;
      inst.rend->mvp =
        glm::translate(glm::vec3(ScreenWidth - w, ScreenHeight - 20.0f, 0.0f)) *
        glm::scale(glm::vec3(20.0f, 20.0f, 20.0f));
      inst.rend->mvp.update();
      text.draw();
    }

    {
      inst.rend->mvp = glm::translate(glm::vec3(80.0f, 10.0f, 0.0f)) *
                       glm::scale(glm::vec3(FontSize, FontSize, FontSize));
      inst.rend->mvp.update();
      std::ostringstream stats;
      stats << std::fixed
            << 100.0f * (inst.world->getO2Level() + inst.world->getH2OLevel() + inst.world->getTreeLevel()) / 3.0f
            << "%";
      text.setText(stats.str());
      text.draw();
    }

    {
      std::ostringstream stats;
      stats.imbue(std::locale(""));
      stats << "$" << std::fixed << inst.world->money;
      text.setText(stats.str());
      auto w = text.getWidth();
      inst.rend->mvp = glm::translate(glm::vec3(ScreenWidth - w * FontSize - 50.0f, 10.0f, 0.0f)) *
                       glm::scale(glm::vec3(FontSize, FontSize, FontSize));
      inst.rend->mvp.update();
      text.draw();
    }
    {
      std::ostringstream stats;
      stats.imbue(std::locale(""));
      stats << "$" << std::fixed << inst.world->getIncome() << "/sec";
      text.setText(stats.str());
      auto w = text.getWidth();
      inst.rend->mvp =
        glm::translate(glm::vec3(ScreenWidth - w * FontSize - 50.0f, FontSize + 10.0f, 0.0f)) *
        glm::scale(glm::vec3(FontSize, FontSize, FontSize));
      inst.rend->mvp.update();
      text.draw();
    }
    {
      std::ostringstream stats;
      stats.imbue(std::locale(""));
      stats << "$" << inst.world->specs.botPrice << "/bot";
      text.setText(stats.str());
      auto w = text.getWidth();
      inst.rend->mvp = glm::translate(glm::vec3(
                         ScreenWidth / 2 - w * FontSize / 2.0f, ScreenHeight - 60.0f, 0.0f)) *
                       glm::scale(glm::vec3(FontSize, FontSize, FontSize));
      inst.rend->mvp.update();
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
