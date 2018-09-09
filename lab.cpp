#include "lab.hpp"

#include "bot_specs.hpp"
#include "button.hpp"
#include "inst.hpp"
#include "rend.hpp"
#include "research_item.hpp"
#include "screen.hpp"
#include "text_button.hpp"
#include "ui.hpp"
#include "world.hpp"
#include <coeff/coefficient_registry.hpp>
#include <shade/library.hpp>
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>
#include <shade/text.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

COEFF(LabFontSize, 40.0f);

static std::vector<glm::vec3> getPlaneMesh()
{
  std::vector<glm::vec3> res;
  res.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
  res.push_back(glm::vec3(1.0f, 1.0f, 0.0f));
  res.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

  res.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
  res.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
  res.push_back(glm::vec3(1.0f, 1.0f, 0.0f));
  return res;
}


class BgPlane: public Widget
{
public:
  BgPlane(Rend &aRend, int x, int y, int w, int h)
    : Widget(x, y, w, h), rend(&aRend), planeMesh(getPlaneMesh(), 0)
  {
  }
  void draw() override
  {
    rend->mvp = glm::translate(glm::vec3(x, y, -2.0f)) * glm::scale(glm::vec3(w, h, 1.0f));
    rend->time = SDL_GetTicks();
    rend->planeShad->use();
    planeMesh.activate();
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }
private:
  Rend* rend;
  ArrayBuffer planeMesh;
};

Lab::Lab(Library &lib, Inst &inst, Ui &aUi, BotSpecs &specs)
  : crossIcon(lib.getObj("cross")),
    closeBtn(std::make_unique<Button>(ScreenWidth - 30 - 20, 70.0f - 20, 40, 40)),
    bgPlane(
      std::make_unique<BgPlane>(*inst.rend, 30, 70, ScreenWidth - 60, ScreenHeight - 180 - 80)),
    ui(&aUi)
{
  closeBtn->onDraw = [this, &inst](bool pressed) {
    inst.rend->mvp = glm::translate(glm::vec3(ScreenWidth - 30.0f, 70.0f, 20.0f)) *
               glm::scale(glm::vec3(20.0f, 20.0f, 20.0f) * (pressed ? 1.3f : 1.0f));
    inst.rend->uiShad->use();
    crossIcon->draw();
  };
  closeBtn->onClick = [this]() { hide(); };

  int y = 65;
  auto const BtnHeight = TextButton::FontSize * 2 * 110 / 100;
  auto createResearchItem = [&](int &value,
                                const std::string &description,
                                float k,
                                const std::string &unit,
                                float improvement = 5.0f) {
    researchItems.push_back(std::make_pair(
      std::make_unique<ResearchItem>(value, description, k, unit, improvement),
      std::make_unique<TextButton>(*inst.rend, lib, 60, y, ScreenWidth - 120, BtnHeight)));
    auto &&item = researchItems.back().first.get();
    auto &&btn = researchItems.back().second.get();
    btn->setText(item->getText());
    btn->onClick = [item, &inst, btn, &specs]() {
      if (inst.world->money < item->cost)
        return;
      inst.world->money-= item->cost;
      item->research();
      btn->setText(item->getText());
      specs.botPrice = specs.botPrice * 1010 / 1000;
    };
    y += BtnHeight;
  };

  createResearchItem(specs.lifeSpan, "Increase bot's robustness", 1.0f / 100.0f, " s");
  createResearchItem(specs.batteryCapacity, "Increase battery capacity", 5.0f, " mAh");
  createResearchItem(specs.chargeRate, "Better solar panels", 0.25f, " mA");
  createResearchItem(specs.speed, "Increase bot's speed", 3.0f / 1100.0f, " MPH");
  createResearchItem(specs.buildTime, "Reduce build time", 1.0f / 100.0f, " s", -5.0f);
  createResearchItem(specs.o2PlantLifeSpan, "Build stronger O2 plant", 1.0f / 100.0f, " s");
  createResearchItem(specs.h2OPlantLifeSpan, "Build stronger H2O plant", 1.0f / 100.0f, " s");
  createResearchItem(specs.o2PlantProdRate, "Make plants produce more O2", 0.01f, " t/s");
  createResearchItem(specs.h2OPlantProdRate, "Make plants produce more H2O", 0.01f, " t/s");
}

void Lab::show()
{
  ui->add(*bgPlane);
  ui->add(*closeBtn);
  for (auto &&research : researchItems)
    ui->add(*research.second);
  visible = true;
}

void Lab::hide()
{
  ui->remove(*bgPlane);
  ui->remove(*closeBtn);
  for (auto &&research : researchItems)
    ui->remove(*research.second);
  visible = false;
}

Lab::~Lab()
{
  hide();
}

bool Lab::isVisible() const
{
  return visible;
}
