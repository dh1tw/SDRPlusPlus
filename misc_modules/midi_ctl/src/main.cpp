#include <imgui.h>
#include <module.h>
#include <gui/gui.h>
#include <core.h>
#include <rtmidi/RtMidi.h>
#include <string>
#include <vector>
#include <map>


SDRPP_MOD_INFO{
    /* Name:            */ "midi_ctl",
    /* Description:     */ "MIDI control module for SDR++",
    /* Author:          */ "Tobias Wellnitz, DH1TW",
    /* Version:         */ 0, 1, 0,
    /* Max instances    */ -1
};

class DemoModule : public ModuleManager::Instance {


public:
    DemoModule(std::string name) {
        this->name = name;

        // Initialize RtMidi
        try {
            midiIn = new RtMidiIn();
            midiOut = new RtMidiOut();
        }
        catch (RtMidiError& error) {
            flog::error("Error creating RtMidi instance: {0}", error.getMessage());
            return;
        }

        gui::menu.registerEntry(name, menuHandler, this, NULL);
    }

    ~DemoModule() {
        gui::menu.removeEntry(name);
    }

    void postInit() {}

    void enable() {
        enabled = true;
    }

    void disable() {
        enabled = false;
    }

    bool isEnabled() {
        return enabled;
    }

private:
    static void menuHandler(void* ctx) {
        DemoModule* _this = (DemoModule*)ctx;
        ImGui::Text("Hello SDR++ ok, my name is Dude %s", _this->name.c_str());
    }

    std::string name;
    bool enabled = true;
    RtMidiIn* midiIn = nullptr;
    RtMidiOut* midiOut = nullptr;
    std::vector<std::string> deviceList;
    std::string selectedDevice = "No device selected";
};

MOD_EXPORT void _INIT_() {
    // Nothing here
}

MOD_EXPORT ModuleManager::Instance* _CREATE_INSTANCE_(std::string name) {
    return new DemoModule(name);
}

MOD_EXPORT void _DELETE_INSTANCE_(void* instance) {
    delete (DemoModule*)instance;
}

MOD_EXPORT void _END_() {
    // Nothing here
}