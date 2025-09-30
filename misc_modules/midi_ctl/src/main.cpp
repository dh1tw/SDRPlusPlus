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

        if (message->size() < 3) {
            return;
        }

        unsigned char cmd = message->at(0);
        unsigned char group = message->at(1);
        unsigned char value = message->at(2);

        flog::info("MIDI message - Cmd: 0x{}, Group: 0x{}, Value: 0x{}", cmd, group, value);

        flog::info("MIDI message - Cmd: {}", cmd);
        flog::info("MIDI message - Group: {}", group);
        flog::info("MIDI message - Value: {}", value);

        std::string vfoName = gui::waterfall.selectedVFO;
        double centerFrequency = gui::waterfall.getCenterFrequency();
        double offset = sigpath::vfoManager.getOffset(vfoName);

        if (cmd == 176 && group == 6) {

            if (value <= 10) {
                offset += 100 * value;
            }
            else {
                offset -= 100 * (128 - value);
            }
            sigpath::vfoManager.setOffset(vfoName, offset);
            // flog::info("New Freq: {}", sigpath::vfoManager.getFrequency(vfoName);
        }

        if (cmd == 128 && group == 02) {
            sigpath::vfoManager.setOffset(vfoName, 0);
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