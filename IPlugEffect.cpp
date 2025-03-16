#include "IPlugEffect.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

#include"exprtk.hpp"
#include <ctime>


  iplug::sample* LUT = new sample[10];
  typedef exprtk::symbol_table<iplug::sample> symbol_table_t;
  typedef exprtk::expression<iplug::sample> expression_t;
  typedef exprtk::parser<iplug::sample> parser_t;


  const iplug::sample r0 = iplug::sample(0);
  const iplug::sample r1 = iplug::sample(1);

  symbol_table_t symbol_table;

  expression_t expression;
  parser_t parser;

  const iplug::sample delta = iplug::sample(1.0 / 100.0);


  iplug::sample x;


  void registerEquation(const std::string userInput)
  {
    symbol_table.add_variable("x", x);

    expression.register_symbol_table(symbol_table);

    parser.compile(userInput, expression);
  }


  iplug::sample ProcessEquation(const iplug::sample currentSample)
  {

    x = currentSample;


    for (x = r0; x <= r1; x += delta)
    {
      return expression.value();
    }
    
    return 0;
  }


  

  void createLUT()
  {
    sample value;
    for (size_t i = 0; i < 10; i++)
    {
      registerEquation("2x");
      value = ProcessEquation(-1 + (0.2 * i));
      LUT[i] = value;
      //LUT[i] = std::numeric_limits<sample>().min() + ((std::numeric_limits<sample>().max() / 5) * i);
    }
  }


  clock_t begin;
  IEditableTextControl* textInput;

   IPlugEffect::IPlugEffect(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitDouble("Gain", 2., 2., 7.0, 0.01, "%");

#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    //pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_WHITE);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);


    const IVStyle style{
      true, // Show label
      true, // Show value
      {
        DEFAULT_BGCOLOR,          // Background
        COLOR_WHITE,          // Foreground
        DEFAULT_PRCOLOR,          // Pressed
        COLOR_BLACK,              // Frame
        DEFAULT_HLCOLOR,          // Highlight
        DEFAULT_SHCOLOR,          // Shadow
        COLOR_BLACK,              // Extra 1
        DEFAULT_X2COLOR,          // Extra 2
        DEFAULT_X3COLOR           // Extra 3
      },                          // Colors
      IText(12.f, EAlign::Center) // Label text
    };


    const IBitmap backdrop = pGraphics->LoadBitmap(PNGBACKDROP_FN);

    const IRECT b = pGraphics->GetBounds();
    //const IRECT c = pGraphics->GetBounds();

    begin = clock();
    
    //pGraphics->AttachControl(new ISVGKnobControl(b.GetCentredInside(100), knobSVG, kGain), kNoTag, "svgcontrols");

    std::string text = "x";
    auto c = new IBitmapControl(b, backdrop);
    IRECT textboxRect = b.GetCentredInside(100).GetHShifted(-200).FracRectHorizontal(3.1f).FracRectVertical(0.6f).GetVShifted(60);
    //(-70,-240,-320,-50)
    textInput = new IEditableTextControl(textboxRect, text.c_str(), IText(35), COLOR_GRAY);
    
    //auto knob = new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kGain, "Dry/Wet", style);
    //auto knob2 = new IVKnobControl(b.GetCentredInside(100).GetVShifted(100), kPenis);

    pGraphics->AttachControl(c);
    pGraphics->AttachControl(textInput);
    //pGraphics->AttachControl(knob);
    //pGraphics->AttachControl(knob2);

    //pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-500), kPenis));


    textInput->SetTextEntryLength(100);
    textInput->GetText().mTextEntryFGColor.FromColorCode(COLOR_WHITE.ToColorCode());

    registerEquation("2x");
    createLUT();
    
  };
#endif
}

#if IPLUG_DSP
sample copy;

template <class result_t = std::chrono::milliseconds, class clock_t = std::chrono::steady_clock, class duration_t = std::chrono::milliseconds>
auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
  return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}

void IPlugEffect::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() * 0.5f;
  const int nChans = NOutChansConnected();
  const double secondsElapsed = (double(clock() - begin) / CLOCKS_PER_SEC);
  
  

  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {

      
      for (size_t i = 0; i < 10; i++)
      {
        if (inputs[c][s] >= LUT[i])
        {
          //copy = LUT[i];
          break;
        }
        
      }
      //x*((sin(t*20)+1)*0.5)
      outputs[c][s] = ((sin(secondsElapsed * 20.0) + 1) * 0.5) * inputs[c][s];
      //outputs[c][s] = inputs[c][s] + (pow(inputs[c][s], 1.3));
      //outputs[c][s] = inputs[c][s];
      //textInput->SetText(sin(since(*begin).count()));
    }
  }
}
#endif
