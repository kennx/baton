#include "ControlMode.h"
#include <M5Unified.h>

namespace {
  enum class CState { DEVICE_SELECT, COMMAND_CYCLE };
  CState cState = CState::DEVICE_SELECT;

  std::vector<DeviceProfile> savedProfiles;
  int deviceIndex = 0;
  
  DeviceProfile activeProfile;
  int commandIndex = 0;
  
  unsigned long sendDisplayTime = 0;
  std::string sendStatusMsg = "";

  void drawDeviceSelect(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("[B]", storage.getCount(), "CTRL");
    if (savedProfiles.empty()) {
      ui.drawSignalInfo("No Devices", "Go to Match Mode", "", "", "to add devices");
      ui.drawFooter("btnB: Back");
      return;
    }
    
    std::vector<std::string> names;
    for (const auto& p : savedProfiles) {
      names.push_back(p.brand);
    }
    ui.drawMenu(names, deviceIndex, "Select Device");
    ui.drawFooter("NXT          SEL");
  }

  void drawCommandCycle(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("[B]", storage.getCount(), activeProfile.brand);
    if (activeProfile.commands.empty()) {
      ui.drawSignalInfo("Error", "No commands", "", "", "");
      return;
    }
    
    DeviceCommand cmd = activeProfile.commands[commandIndex];
    std::string statusMsg = !sendStatusMsg.empty() ? sendStatusMsg : "Ready";
    ui.drawSignalInfo(activeProfile.brand, activeProfile.protocol, activeProfile.address, cmd.name, statusMsg);
    ui.drawFooter("SEND         NXT");
  }
} // namespace

namespace ControlMode {

  void enter(UIScreen& ui, SignalStorage& storage) {
    cState = CState::DEVICE_SELECT;
    savedProfiles = storage.getAllProfiles();
    deviceIndex = 0;
    sendStatusMsg = "";
    drawDeviceSelect(ui, storage);
  }

  void update(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (!sendStatusMsg.empty() && (millis() - sendDisplayTime > 500)) {
      sendStatusMsg = "";
      if (cState == CState::COMMAND_CYCLE) {
        drawCommandCycle(ui, storage);
      }
    }
  }

  void onShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (cState == CState::DEVICE_SELECT) {
      if (savedProfiles.empty()) return;
      deviceIndex = (deviceIndex + 1) % savedProfiles.size();
      drawDeviceSelect(ui, storage);
    } else if (cState == CState::COMMAND_CYCLE) {
      if (activeProfile.commands.empty()) return;
      
      DeviceCommand cmd = activeProfile.commands[commandIndex];
      Signal s;
      s.protocol = activeProfile.protocol;
      s.address = activeProfile.address;
      s.command = cmd.hexCode;
      
      bool success = ir.sendSignal(s);
      
      sendStatusMsg = success ? "SENDING..." : "Unsupported!";
      sendDisplayTime = millis();
      drawCommandCycle(ui, storage);
    }
  }

  bool onLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (cState == CState::DEVICE_SELECT) {
      if (savedProfiles.empty()) return true; // go back
      activeProfile = savedProfiles[deviceIndex];
      commandIndex = 0;
      cState = CState::COMMAND_CYCLE;
      drawCommandCycle(ui, storage);
      return false;
    } else if (cState == CState::COMMAND_CYCLE) {
      // Long press in Command Cycle: NEXT COMMAND
      if (!activeProfile.commands.empty()) {
        commandIndex = (commandIndex + 1) % activeProfile.commands.size();
        drawCommandCycle(ui, storage);
      }
      return false;
    }
    return true;
  }

  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage) {
    if (cState == CState::DEVICE_SELECT) {
      return true; // back to main menu
    } else if (cState == CState::COMMAND_CYCLE) {
      cState = CState::DEVICE_SELECT;
      drawDeviceSelect(ui, storage);
      return false;
    }
    return true;
  }

  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage) {
    return true; // always back to main menu
  }

}
