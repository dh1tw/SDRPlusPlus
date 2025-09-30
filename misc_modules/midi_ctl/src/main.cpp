#include <imgui.h>
#include <module.h>
#include <gui/gui.h>
#include <core.h>
#include <rtmidi/RtMidi.h>
#include <string>
#include <vector>
#include <map>
#include <signal_path/signal_path.h>
#include <signal_path/vfo_manager.h>


SDRPP_MOD_INFO{
    /* Name:            */ "midi_ctl",
    /* Description:     */ "MIDI control module for SDR++",
    /* Author:          */ "Tobias Wellnitz, DH1TW",
    /* Version:         */ 0, 1, 0,
    /* Max instances    */ -1
};

class MidiControlModule : public ModuleManager::Instance {


public:
    MidiControlModule(std::string name) {
        this->name = name;

        // Initialize RtMidi
        try {
            midiIn = new RtMidiIn();
        }
        catch (RtMidiError& error) {
            flog::error("Error creating RtMidi instance: {0}", error.getMessage());
            return;
        }

        // Get available MIDI ports
        refreshDeviceList();

        gui::menu.registerEntry(name, menuHandler, this, NULL);
    }

    ~MidiControlModule() {
        gui::menu.removeEntry(name);
        if (midiIn) {
            midiIn->closePort();
            delete midiIn;
        }
    }

    void postInit() {}

    void enable() {
        enabled = true;
    }

    void disable() {
        enabled = false;
        if (midiIn && midiIn->isPortOpen()) {
            midiIn->closePort();
        }
    }

    bool isEnabled() {
        return enabled;
    }

private:
    static void midiCallback(double deltatime, std::vector<unsigned char>* message, void* userData) {
        MidiControlModule* _this = (MidiControlModule*)userData;
        if (!_this->enabled) return;

        // Process MIDI messages
        if (message->size() >= 3) {
            unsigned char status = message->at(0);
            unsigned char data1 = message->at(1);
            unsigned char data2 = message->at(2);

            flog::info("MIDI message - Status: 0x{:02X}, Data1: 0x{:02X}, Data2: 0x{:02X}", status, data1, data2);
        }
    }

    void refreshDeviceList() {
        deviceList.clear();
        int portCount = midiIn->getPortCount();
        for (int i = 0; i < portCount; i++) {
            try {
                deviceList.push_back(midiIn->getPortName(i));
            }
            catch (RtMidiError& error) {
                continue;
            }
        }
    }

    static void menuHandler(void* ctx) {
        MidiControlModule* _this = (MidiControlModule*)ctx;

        if (ImGui::Begin("MIDI Control", &_this->enabled)) {
            // Device selection
            if (ImGui::Button("Refresh Devices")) {
                _this->refreshDeviceList();
            }

            ImGui::SameLine();

            if (ImGui::BeginCombo("MIDI Device", _this->selectedDevice.c_str())) {
                for (const auto& device : _this->deviceList) {
                    bool isSelected = (_this->selectedDevice == device);
                    if (ImGui::Selectable(device.c_str(), isSelected)) {
                        if (_this->midiIn->isPortOpen()) {
                            _this->midiIn->closePort();
                        }

                        // Find device index
                        for (int i = 0; i < _this->midiIn->getPortCount(); i++) {
                            if (_this->midiIn->getPortName(i) == device) {
                                try {
                                    _this->midiIn->openPort(i);
                                    _this->midiIn->setCallback(&MidiControlModule::midiCallback, _this);
                                    _this->selectedDevice = device;
                                }
                                catch (RtMidiError& error) {
                                    std::cerr << "Error opening MIDI port: " << error.getMessage() << std::endl;
                                }
                                break;
                            }
                        }
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Control mappings info
            ImGui::Separator();
            ImGui::Text("Control Mappings:");
            ImGui::Text("CC 0x10: VFO Control (±100 Hz per tick)");
        }
        ImGui::End();
    }

    std::string name;
    bool enabled = true;
    RtMidiIn* midiIn = nullptr;
    std::vector<std::string> deviceList;
    std::string selectedDevice = "No device selected";
};

MOD_EXPORT void _INIT_() {
    // Nothing here
}

MOD_EXPORT ModuleManager::Instance* _CREATE_INSTANCE_(std::string name) {
    return new MidiControlModule(name);
}

MOD_EXPORT void _DELETE_INSTANCE_(void* instance) {
    delete (MidiControlModule*)instance;
}

MOD_EXPORT void _END_() {
    // Nothing here
}