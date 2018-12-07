#include "Overlay.hpp"
#include "Engine/Debug/Profile/Report.hpp"
#include "Engine/Debug/Profile/Profiler.hpp"
#include "Engine/Renderer/Geometry/Mesher.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Application/Window.hpp"
#include "Engine/Math/Primitives/AABB2.hpp"
#include <stack>
#include "Engine/Renderer/Font.hpp"
#include "Engine/Debug/Console/Command.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/Math/MathUtils.hpp"
#define WM_CHAR                 0x0102
#define WM_KEYDOWN              0x0100
#define WM_KEYUP                0x0101
#define JUST_KEYDOWN_STATE      0x80000000



static const Rgba kBoundColor;
static const Rgba kBackgroundColor;


namespace Profile {
  class Overlay {
  public:
    Overlay(aabb2 bound);

    void onInput();

    Report::eViewOption viewType = Report::VIEW_TREE;
    Report::eSortOption sortType = Report::SORT_SELF_TIME;
    void update();
    void render() const;
    void toggle() { mVisible = !mVisible; };
    void toggle(bool visible) {
      static bool mouseLocked = true;
      if(visible) {
        mouseLocked = Input::Get().isMouseLocked();
        Input::Get().mouseLockCursor(false);
      } else {
        Input::Get().mouseLockCursor(mouseLocked);
      }
      mVisible = visible;
    };
    bool visible() const { return mVisible; }
    static const float kFontSize;
    static const Rgba  kFontColor;
    static const Rgba  kFontHighlightColor;
  protected:

    struct Chart {

      //----------- Config ---------------
      static const Rgba kBoundColor;
      static const Rgba kBackgroundColor;

      static const Gradient kChartFillColor;
      static const Rgba kChartBorderColor;
      static const Rgba kChartSelectedBlendColor;
      static const Rgba kChartSelectedBorderColor;

      enum eSelectState {
        SELECT_CLEAR,
        SELECT_SELECTING,
        SELECT_SELECTED,
      };
      //---------------------------------

      Chart(Overlay& o) : overlay(o) {}

      void onInput();
      void update();
      void render() const;
      aabb2 bound;
      Transform transform;
      Range<float> xDataRange{ 0.f };
      Range<float> yDataRange{ 0.f, (float)MAX_FRAME_RECORDED };
      std::array<vec2, MAX_FRAME_RECORDED> data;
      Overlay& overlay;
      Mesh* graph = nullptr;
      eSelectState selectState = SELECT_CLEAR;
      Range<float> mSelectedRange{ 0 };
      Range<uint> frameRange{ 0,0 };
      vec2 PADDING{ 10.f };

    };

    aabb2 mBound;
    Chart mChart;
    Mesh* mText = nullptr;
    Camera* mCamera = nullptr;
    Transform mTransform;
    bool mVisible = false;
    vec2 PADDING{ 10.f };
  };

}

Profile::Overlay* gOverlay = nullptr;

void Profile::initOverlay() {
  EXPECTS(gOverlay == nullptr);

  gOverlay = new Profile::Overlay(Window::Get()->bounds());
}

const Rgba     Profile::Overlay::Chart::kBoundColor = Rgba(255, 255, 255, 255);
const Rgba     Profile::Overlay::Chart::kBackgroundColor = Rgba(0, 0, 0, 128);
const Gradient Profile::Overlay::Chart::kChartFillColor = { Rgba(0, 255, 0, 255), Rgba(255, 0, 0, 255) };
const Rgba     Profile::Overlay::Chart::kChartBorderColor = Rgba::white;
const Rgba     Profile::Overlay::Chart::kChartSelectedBlendColor = Rgba(0, 0, 200, 50);
const Rgba     Profile::Overlay::Chart::kChartSelectedBorderColor = Rgba(0, 0, 255, 200);

const float Profile::Overlay::kFontSize = 16.f;
const Rgba  Profile::Overlay::kFontColor = Rgba(180, 180, 180, 255);
const Rgba  Profile::Overlay::kFontHighlightColor = Rgba(255, 255, 255, 255);

Profile::Overlay::Overlay(aabb2 bound): mBound(bound), mChart(*this) {

  mBound.maxs -= 2 * PADDING;
  vec2 containerSize = mBound.size();

  mTransform.localTranslate(vec3(PADDING, 0.f));
  mChart.transform.parent() = &mTransform;
  mChart.transform.localTranslate({ vec2(0, containerSize.y * 0.8f), 0.f });

  mChart.bound = aabb2{ {0, 0}, { containerSize.x, containerSize.y*.2f } };
  aabb2 bounds = Window::Get()->bounds();
  mCamera = new Camera();
  mCamera->setProjection(mat44::makeOrtho(0, bounds.width(), 0, bounds.height(), -1.f, 1.f));
  mCamera->setColorTarget(Renderer::Get()->getDefaultColorTarget());
}

void Profile::Overlay::onInput() {
  if (!mVisible) return;
  if (Input::Get().isKeyJustDown('V')) {
    gOverlay->viewType = gOverlay->viewType ==
      Profile::Report::VIEW_TREE ? Profile::Report::VIEW_FLAT : Profile::Report::VIEW_TREE;
  }

  if (Input::Get().isKeyJustDown('L')) {
    gOverlay->sortType = gOverlay->sortType ==
      Profile::Report::SORT_TOTAL_TIME ? Profile::Report::SORT_SELF_TIME : Profile::Report::SORT_TOTAL_TIME;
  }

  if (Input::Get().isKeyJustDown('M')) {
    bool locked = Input::Get().isMouseLocked();

    while(locked == Input::Get().isMouseLocked()) {
      Input::Get().mouseLockCursor(!locked);
    }
  }

  mChart.onInput();
}

void Profile::Overlay::update() {
  if (!mVisible) return;

  std::array<Report*, MAX_FRAME_RECORDED> reports{ nullptr };

  mChart.yDataRange.min = 0;
  mChart.yDataRange.max = 0;
  for(uint i = 1; i<= MAX_FRAME_RECORDED; i++) {
    prof_sample_t* sample = dump(i);

    uint drawIndex = MAX_FRAME_RECORDED - i;
    Report*& report = reports[drawIndex];
    report = (sample == nullptr) ? nullptr : &sample->dump(viewType);

    mChart.data[drawIndex].y = (report==nullptr)
                        ? 0.f
                        : (float)report->totalFrameTime();

    mChart.data[drawIndex].x = (float)drawIndex;
    mChart.yDataRange.grow(mChart.data[drawIndex].y);

  }
  // update other params for chart;
  mChart.update();

  // update the report view

  // sort all report
  // for(Report* report: reports) {
  //   if(report) report->sort(sortType);
  // }

  Report* report = nullptr;
  if(mChart.selectState == Chart::SELECT_CLEAR) {
    report = reports[MAX_FRAME_RECORDED - 1];
  } else {
    report = (Report*)_alloca(sizeof(Report));
    report = new(report) Report();

    for(uint i = mChart.frameRange.min; i <= mChart.frameRange.max; i++) {
      prof_sample_t* sample = dump(MAX_FRAME_RECORDED - i);
      if(sample != nullptr) report->accumlateSample(sample, viewType);
    }
  }

  if (report) report->sort(sortType);

  Mesher ms;
  ms.begin(DRAW_TRIANGES);

  struct Iter {
    const Report::Entry* entry = nullptr;
    uint depth = 0;
  };

  std::stack<Iter> toProcess;
  auto font = Font::Default();

  float lingHeight = font->lineHeight(kFontSize);
  float lineStep = -lingHeight;
  vec3 position(vec2(mBound.mins.x, mBound.maxs.y * .8f - font->ascender(kFontSize) - PADDING.y), 0.f);
  
  if(reports[MAX_FRAME_RECORDED - 1] != nullptr) {
    toProcess.push({ &report->self(), 0 });
    ms.text(
      Stringf("FPS: %.2lf    Frame time: %s ", 
      1.0 / reports[MAX_FRAME_RECORDED - 1]->self().totalTime, 
      beautifySeconds(reports[MAX_FRAME_RECORDED - 1]->self().totalTime).c_str()),
      18, font.get(), vec3(PADDING, 0));
  }

  ms.color(kFontColor);
  ms.text(Stringf(
    "[ ]%-*s%-10s%-25s%-25s%-25s%-25s", 
    57, "Function Name", "Call", 
    sortType == Report::SORT_TOTAL_TIME ? "--Total(Time)--" : "Total(Time)",
    sortType == Report::SORT_SELF_TIME ? "--Self(Time)--" : "Self(Time)",
    "Average Total Time", "Average Self Time"),
    kFontSize, font.get(), position);
  position.y += lineStep;
  while(!toProcess.empty()) {
    Iter& top = toProcess.top();
    uint currentDepth = top.depth;

    // char str[1000];
    // sprintf_s(str, 1000, "%s", top.entry->name.data());

    // vec3 indentedPosition = position;
    // indentedPosition.x += font->advance(' ', ' ', kFontSize) * float(top.depth) ;

    std::string text = Stringf(
      "%-*s%-*s%-10u%-25s%-25s%-25s%-25s", 
      top.depth+3, top.entry->children().empty() ? "   " : "[-]", 
      57 - top.depth, top.entry->name.data(), 
      top.entry->callCount, 
      beautifySeconds(top.entry->totalTime).c_str(), 
      beautifySeconds(top.entry->selfTime).c_str(),
      beautifySeconds(top.entry->totalTimeAveragePerCall).c_str(),
      beautifySeconds(top.entry->selfTimeAveragePerCall).c_str());

    ms.text(text, kFontSize, font.get(), position);

    position.y += lineStep;
    toProcess.pop();

    for(auto& [_, v]: top.entry->children()) {
      EXPECTS(v != nullptr);
      toProcess.push({ v, currentDepth + 1 });
    }
  }


  ms.end();

  SAFE_DELETE(mText);
  mText = ms.createMesh<vertex_pcu_t>();
  
}

void Profile::Overlay::render() const {
  if (!mVisible) return;
  // set ui solid material
  // draw background, container

  Renderer::Get()->setCamera(mCamera);
  Renderer::Get()->setModelMatrix(mat44::identity);

  Renderer::Get()->setMaterial(Resource<Material>::get("material/ui/default").get());
  // Renderer::Get()->drawAABB2({ Input::Get().mouseClientPositon(), Input::Get().mouseClientPositon()+ vec2(5.f) }, Rgba::red);
  Renderer::Get()->drawAABB2(Window::Get()->bounds(), Rgba(0, 0, 128, 128));
  // Renderer::Get()->drawAABB2(mBound, Rgba::yellow, false);
  mChart.render();

  // set ui text material
  Renderer::Get()->setModelMatrix(mTransform.localToWorld());
  Renderer::Get()->setMaterial(Resource<Material>::get("material/ui/font").get());
  Renderer::Get()->drawMesh(*mText);
}

void Profile::Overlay::Chart::onInput() {
  vec2 mousePosition = Input::Get().mouseClientPositon();
  mousePosition.y = Window::Get()->bounds().height() - mousePosition.y;
  vec2 positionInChart = (transform.worldToLocal() * vec4(mousePosition, 0, 1)).xy();

  switch(selectState) { 
    case SELECT_CLEAR: {
      if (bound.contains(positionInChart)) {
        positionInChart.x = clamp(positionInChart.x, 0.f, bound.width());
        mSelectedRange.min = positionInChart.x;
        mSelectedRange.max = positionInChart.x + .1f;
        if (Input::Get().isKeyJustDown(MOUSE_LBUTTON)) {
          Profile::pause();
          selectState = SELECT_SELECTING;
        }
      } else {
        mSelectedRange.min = mSelectedRange.max;
      }
    }
    break;
    case SELECT_SELECTING: {
      positionInChart.x = clamp(positionInChart.x, 0.f, bound.width());
      mSelectedRange.max = positionInChart.x;
      if(Input::Get().isKeyJustUp(MOUSE_LBUTTON)) {
        if (mSelectedRange.size() < .1f && mSelectedRange.size() > -.1f) {
          mSelectedRange.max = mSelectedRange.min + 1.f;
        }
        selectState = SELECT_SELECTED;
      }
    }
    break;
    case SELECT_SELECTED: {
      if(Input::Get().isKeyJustDown(MOUSE_RBUTTON)) {
        Profile::resume();
        selectState = SELECT_CLEAR;
      }
    }
    break;
    default: ;
  }
  
}

void Profile::Overlay::Chart::update() {
  Mesher ms;

  std::array<vec3, MAX_FRAME_RECORDED> dataToDraw;

  float stepSize = bound.width() / MAX_FRAME_RECORDED;

  for(uint i = 0; i<data.size(); i++) {
    dataToDraw[i].x = data[i].x * stepSize;
    dataToDraw[i].z = data[i].y / yDataRange.size(); // use z to cache the precent
    dataToDraw[i].y = data[i].y / yDataRange.size() * bound.height();

    ENSURES(yDataRange.size() == 0 || dataToDraw[i].z <= 1.f);
  }
  // ----- draw graph filled color ----- 
  // bound
  ms.begin(DRAW_TRIANGES);
  ms.color(kBackgroundColor);
  ms.quad2(bound);

  // draw bar

  if(yDataRange.size() != 0.f) {
    float heightStandard = (1.f / 5.f) / yDataRange.size() * bound.height();
    for(const vec3& d: dataToDraw) {
      ms.color(kChartFillColor.evaluate(d.y / heightStandard));
      ms.quad2({ {d.x, 0}, {d.x + stepSize, d.y} });
    }
  }

  float min = std::min(mSelectedRange.min, mSelectedRange.max);
  float max = std::max(mSelectedRange.min, mSelectedRange.max);

  frameRange.min = uint(min / (bound.width() / (float)MAX_FRAME_RECORDED));
  frameRange.max = uint(max / (bound.width() / (float)MAX_FRAME_RECORDED));
  if(mSelectedRange.size() != 0) {
    ms.color(kChartSelectedBlendColor);
    ms.quad2({ { min, 0 }, { max, bound.height() } });
  }
    // ms.quad2()
  ms.end();

  // ----- draw borders ----- 
  ms.begin(DRAW_LINES);
  ms.color(kBoundColor);
  ms.quad2(bound);

  if (mSelectedRange.size() != 0) {
    ms.color(kChartSelectedBorderColor);
    ms.quad2({ { min, 0 },{ max, bound.height() } });
  }
  ms.end();

  SAFE_DELETE(graph);

  graph = ms.createMesh<vertex_pcu_t>();
}

void Profile::Overlay::Chart::render() const {
  Renderer::Get()->setMaterial(Resource<Material>::get("material/ui/default").get());
  Renderer::Get()->setModelMatrix(transform.localToWorld());
  Renderer::Get()->drawMesh(*graph);
}

void Profile::updateOverlay() {
  gOverlay->onInput();
  gOverlay->update();
}

void Profile::renderOverlay() {
  gOverlay->render();
}

COMMAND_REG("profiler", "bool: 1/0/true/false", "toggle profiler display") (Command& cmd){
  gOverlay->toggle(cmd.arg<0, bool>());
  return true;
}