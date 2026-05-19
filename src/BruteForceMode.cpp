#include "BruteForceMode.h"
#include "PresetNames.h"
#include <M5Unified.h>

namespace {

  enum class BruteState { PROTOCOL_SELECT, BRUTING, SAVE_CONFIRM, DONE };
  BruteState bState = BruteState::PROTOCOL_SELECT;

  const std::vector<std::string> PROTOCOLS = {"NEC", "SAMSUNG", "LG", "SONY", "PANASONIC"};
  int protocolIndex = 0;

  // NEC: 24 addrs x 10 cmds = 240
  const std::vector<uint32_t> NEC_ADDRS = {
    0x0000, 0x00FF, 0x01FE, 0x02FD, 0x04FB,
    0x08F7, 0x10EF, 0x20DF, 0x40BF, 0x807F,
    0xA55A, 0xAD52, 0xBD42, 0xC03F, 0xD02F,
    0xE01F, 0xE0E0, 0xE110, 0xE190, 0xE210,
    0xE290, 0xE310, 0xE390, 0xFF00
  };
  const std::vector<uint32_t> NEC_CMDS = {
    0x02, 0x07, 0x09, 0x0C, 0x10,
    0x1E, 0x40, 0x43, 0x44, 0x45
  };

  // Samsung: 20 addrs x 10 cmds = 200
  const std::vector<uint32_t> SAMSUNG_ADDRS = {
    0xE0E0, 0xE110, 0xE190, 0xE210, 0xE290,
    0xE310, 0xE390, 0xE410, 0xE490, 0xE510,
    0xE590, 0xE610, 0xE690, 0xE710, 0xE790,
    0xE810, 0xE890, 0xE910, 0xE990, 0xEA10
  };
  const std::vector<uint32_t> SAMSUNG_CMDS = {
    0x02, 0x07, 0x0B, 0x0C, 0x0D,
    0x1E, 0x40, 0x43, 0x44, 0x45
  };

  // LG: 16 addrs x 10 cmds = 160
  const std::vector<uint32_t> LG_ADDRS = {
    0x20DF, 0x2FD4, 0x2FD5, 0x2FD6, 0x2FD7,
    0x2FD8, 0x2FD9, 0x2FDA, 0x2FDB, 0x2FDC,
    0x2FDD, 0x2FDE, 0x2FDF, 0x2FE0, 0x2FE1,
    0x2FE2
  };
  const std::vector<uint32_t> LG_CMDS = {
    0x10, 0x14, 0x15, 0x16, 0x17,
    0x43, 0x44, 0x45, 0x58, 0x5C
  };

  // Sony: 16 addrs x 10 cmds = 160
  const std::vector<uint32_t> SONY_ADDRS = {
    0x0000, 0x0001, 0x0010, 0x0011, 0x0100,
    0x0101, 0x0110, 0x0111, 0x1000, 0x1001,
    0x1010, 0x1011, 0x1100, 0x1101, 0x1110,
    0x1111
  };
  const std::vector<uint32_t> SONY_CMDS = {
    0xA90, 0xA91, 0xA92, 0xA93, 0xA94,
    0x490, 0x491, 0xC90, 0x290, 0x090
  };

  // Panasonic: 8 addrs x 10 cmds = 80
  const std::vector<uint32_t> PANASONIC_ADDRS = {
    0x4004, 0x8008, 0xBFAE, 0xBFAD,
    0xBFAC, 0xBFAB, 0xBFAA, 0xBFA9
  };
  const std::vector<uint32_t> PANASONIC_CMDS = {
    0x0100, 0x0190, 0x0191, 0x0192, 0x0193,
    0x0194, 0x0195, 0x0196, 0x0197, 0x0198
  };

  struct ProtocolData {
    const std::vector<uint32_t>* addrs;
    const std::vector<uint32_t>* cmds;
  };

  ProtocolData getProtocolData(const std::string& protocol) {
    if (protocol == "NEC") return {&NEC_ADDRS, &NEC_CMDS};
    if (protocol == "SAMSUNG") return {&SAMSUNG_ADDRS, &SAMSUNG_CMDS};
    if (protocol == "LG") return {&LG_ADDRS, &LG_CMDS};
    if (protocol == "SONY") return {&SONY_ADDRS, &SONY_CMDS};
    return {&PANASONIC_ADDRS, &PANASONIC_CMDS};
  }

  const std::vector<uint32_t>* currentAddrList = nullptr;
  const std::vector<uint32_t>* currentCmdList = nullptr;
  int addrIndex = 0;
  int cmdIndex = 0;
  int totalCombos = 0;
  int currentCombo = 0;

  unsigned long lastSendTime = 0;
  unsigned long stepIntervalMs = 1000;

  Signal hitSignal;
  int saveCatIndex = 0;
  const std::vector<std::string>* currentNameList = nullptr;

  std::string uint32ToHex(uint32_t value) {
    if (value == 0) return "0x0";
    char buf[16];
    snprintf(buf, sizeof(buf), "0x%04X", value);
    return std::string(buf);
  }

  void drawProtocolSelect(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("[B]", storage.getCount(), "[F]FIND");
    ui.drawMenu(PROTOCOLS, protocolIndex, "Pick protocol");
    ui.drawFooter("Short:NXT Long:GO");
  }

  void drawBruting(UIScreen& ui, SignalStorage& storage) {
    std::string name = PROTOCOLS[protocolIndex] + " [" +
                       std::to_string(currentCombo) + "/" +
                       std::to_string(totalCombos) + "]";
    std::string addr = (currentAddrList && addrIndex < static_cast<int>(currentAddrList->size()))
                       ? uint32ToHex((*currentAddrList)[addrIndex]) : "--";
    std::string cmd = (currentCmdList && cmdIndex < static_cast<int>(currentCmdList->size()))
                      ? uint32ToHex((*currentCmdList)[cmdIndex]) : "--";

    ui.drawStatusBar("[B]", storage.getCount(), "[F]FIND");
    ui.drawSignalInfo(name, PROTOCOLS[protocolIndex], addr, cmd, "Trying...");
    ui.drawFooter("Short:FAST Long:HIT");
  }

  void drawDone(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("[B]", storage.getCount(), "[F]FIND");
    ui.drawSignalInfo("Done", "No match", "", "", "Try another protocol");
    ui.drawFooter("btnB:Back");
  }

  void drawSaveConfirm(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> cats = PresetNames::CATEGORIES;
    ui.drawStatusBar("[B]", storage.getCount(), "[F]SAVE");
    ui.drawMenu(cats, saveCatIndex, "Pick category");
    ui.drawFooter("Short:NXT Long:SAVE");
  }

  void advanceCombination() {
    cmdIndex++;
    currentCombo++;
    if (cmdIndex >= static_cast<int>(currentCmdList->size())) {
      cmdIndex = 0;
      addrIndex++;
      if (addrIndex >= static_cast<int>(currentAddrList->size())) {
        bState = BruteState::DONE;
      }
    }
  }

} // namespace

namespace BruteForceMode {

  void enter(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    bState = BruteState::PROTOCOL_SELECT;
    protocolIndex = 0;
    currentAddrList = nullptr;
    currentCmdList = nullptr;
    addrIndex = 0;
    cmdIndex = 0;
    totalCombos = 0;
    currentCombo = 0;
    lastSendTime = 0;
    stepIntervalMs = 1000;
    drawProtocolSelect(ui, storage);
  }

  void update(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (bState == BruteState::BRUTING) {
      if (millis() - lastSendTime >= stepIntervalMs) {
        lastSendTime = millis();

        if (currentAddrList && currentCmdList &&
            addrIndex < static_cast<int>(currentAddrList->size()) &&
            cmdIndex < static_cast<int>(currentCmdList->size())) {

          Signal test;
          test.protocol = PROTOCOLS[protocolIndex];
          test.address = uint32ToHex((*currentAddrList)[addrIndex]);
          test.command = uint32ToHex((*currentCmdList)[cmdIndex]);

          ir.sendSignal(test);
          drawBruting(ui, storage);
          advanceCombination();
        }
      }
    }
  }

  void onShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (bState == BruteState::PROTOCOL_SELECT) {
      protocolIndex = (protocolIndex + 1) % PROTOCOLS.size();
      drawProtocolSelect(ui, storage);
    } else if (bState == BruteState::BRUTING) {
      // toggle fast mode
      stepIntervalMs = (stepIntervalMs == 1000) ? 500 : 1000;
      drawBruting(ui, storage);
    } else if (bState == BruteState::SAVE_CONFIRM) {
      saveCatIndex = (saveCatIndex + 1) % PresetNames::CATEGORIES.size();
      drawSaveConfirm(ui, storage);
    } else if (bState == BruteState::DONE) {
      // no-op
    }
  }

  bool onLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (bState == BruteState::PROTOCOL_SELECT) {
      // Start bruting
      ProtocolData pd = getProtocolData(PROTOCOLS[protocolIndex]);
      currentAddrList = pd.addrs;
      currentCmdList = pd.cmds;
      addrIndex = 0;
      cmdIndex = 0;
      currentCombo = 1;
      totalCombos = static_cast<int>(currentAddrList->size() * currentCmdList->size());
      stepIntervalMs = 1000;
      lastSendTime = millis();
      bState = BruteState::BRUTING;
      drawBruting(ui, storage);
      return false;
    } else if (bState == BruteState::BRUTING) {
      // HIT! Save this code
      hitSignal.protocol = PROTOCOLS[protocolIndex];
      // Step back one to get the code we just sent
      int savedAddrIdx = addrIndex;
      int savedCmdIdx = cmdIndex;
      if (savedCmdIdx > 0) {
        savedCmdIdx--;
      } else if (savedAddrIdx > 0) {
        savedAddrIdx--;
        savedCmdIdx = static_cast<int>(currentCmdList->size()) - 1;
      }
      if (currentAddrList && currentCmdList) {
        hitSignal.address = uint32ToHex((*currentAddrList)[savedAddrIdx]);
        hitSignal.command = uint32ToHex((*currentCmdList)[savedCmdIdx]);
      }
      saveCatIndex = 0;
      bState = BruteState::SAVE_CONFIRM;
      drawSaveConfirm(ui, storage);
      return false;
    } else if (bState == BruteState::SAVE_CONFIRM) {
      // Confirm save
      if (storage.getCount() >= SignalStorage::MAX_SIGNALS) {
        // Show full message briefly, then go back to bruting
        bState = BruteState::BRUTING;
        drawBruting(ui, storage);
        return false;
      }
      std::string cat = PresetNames::CATEGORIES[saveCatIndex];
      hitSignal.name = cat + "-Power";
      hitSignal.category = cat;
      hitSignal.id = -1;
      storage.addSignal(hitSignal);
      bState = BruteState::BRUTING;
      drawBruting(ui, storage);
      return false;
    } else if (bState == BruteState::DONE) {
      return true;  // back to main menu
    }
    return true;
  }

  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (bState == BruteState::PROTOCOL_SELECT) {
      return true;  // back to main menu
    } else if (bState == BruteState::BRUTING) {
      bState = BruteState::PROTOCOL_SELECT;
      drawProtocolSelect(ui, storage);
      return false;
    } else if (bState == BruteState::SAVE_CONFIRM) {
      bState = BruteState::BRUTING;
      drawBruting(ui, storage);
      return false;
    } else if (bState == BruteState::DONE) {
      bState = BruteState::PROTOCOL_SELECT;
      drawProtocolSelect(ui, storage);
      return false;
    }
    return true;
  }

  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    return true;  // always back to main menu
  }

} // namespace BruteForceMode
