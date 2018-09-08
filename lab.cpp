#include "lab.hpp"

#include "button.hpp"
#include "rend.hpp"
#include "screen.hpp"
#include "ui.hpp"
#include <coeff/coefficient_registry.hpp>
#include <shade/library.hpp>
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>
#include <shade/text.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

COEFF(LabFontSize, 40.0f);

Lab::Lab(Rend &rend, Library &lib, Ui &aUi)
  : crossIcon(lib.getObj("cross")),
    closeBtn(std::make_unique<Button>(ScreenWidth - 50 - 20, 80.0f - 20, 40, 40)),
    researchBtn(std::make_unique<Button>(100, 100, ScreenWidth - 200, 40)),
    ui(&aUi)
{
  closeBtn->onDraw = [this, &rend](bool pressed) {
    rend.mvp = glm::translate(glm::vec3(ScreenWidth - 50.0f, 80.0f, 1.0f)) *
               glm::scale(glm::vec3(20.0f, 20.0f, 20.0f) * (pressed ? 1.3f : 1.0f));
    rend.mvp.update();
    crossIcon->draw();
  };
  closeBtn->onClick = [this]() { closed = true; };

  researchBtn->onDraw = [&rend, &lib](bool pressed) {
    rend.textShad->use();
    Text text(lib, "font");
    auto tmpTxt = "Increase bot's robustness\n"
                  "(120 sec + 1%) $100000\n"
                  "Increase battery capacity\n"
                  "(10000 mAh + 1%) $100000\n"
                  "Better solar panels\n"
                  "(100 mA + 1%) $100000\n"
                  "Increase bot's speed\n"
                  "(3 MPH + 1%) $100000\n"
                  "Reduce build time\n"
                  "(10 sec - 1%) $100000\n"
                  "Build stronger O2 plant\n"
                  "(120 sec + 1%) $100000\n"
                  "Build stronger H2O plant\n"
                  "(120 sec + 1%) $100000\n"
                  "Make plants produce more O2\n"
                  "(0.0001%/sec + 1%) $100000\n"
                  "Make plants produce more H2O\n"
                  "(0.0002%/sec + 1%) $100000\n";
    auto w = text.getWidth(tmpTxt);
    rend.mvp =
      glm::translate(glm::vec3((ScreenWidth - w * LabFontSize) / 2.0f, 80.0f, 0.0f)) *
      glm::scale(glm::vec3(LabFontSize, LabFontSize, LabFontSize) * (pressed ? 1.3f : 1.0f));
    rend.mvp.update();
    text.setText(tmpTxt);

    text.draw();
  };
  
  ui->add(*closeBtn);
  ui->add(*researchBtn);
}

Lab::~Lab()
{
  ui->remove(*closeBtn);
  ui->remove(*researchBtn);
}
