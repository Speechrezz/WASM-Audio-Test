#pragma once

#define GUI_WIDTH 500
#define GUI_HEIGHT 500
#if __APPLE__
#define GUI_API CLAP_WINDOW_API_COCOA
#endif

// Forward Declaration
class MyPlugin;
namespace xynth
{
struct SynthleGuiPimpl;
class SynthleGui
{
    SynthleGuiPimpl *m_pimpl;
    MyPlugin *m_plugin = nullptr;

  public:
    SynthleGui(MyPlugin *plugin);
    ~SynthleGui();

    void paint();
    void updateGuiValues();
    void setParent(const void *parent);
    void setVisible(bool show);
};
} // namespace xynth
