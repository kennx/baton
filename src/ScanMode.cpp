#include "ScanMode.h"
#include "SettingsMode.h"
#include <M5Unified.h>

namespace {
  enum class ScanState { IDLE, RECEIVED_NEW, RECEIVED_DUP, FULL };
  ScanState scanState = ScanState::IDLE;
  unsigned long stateEnterTime = 0;
  static constexpr unsigned long DISPLAY_DURATION_MS = 2000;

  Signal lastSignal;
  std::string statusMessage;
}

namespace ScanMode {

  void enter(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    scanState = ScanState::IDLE;
    statusMessage = "Waiting for IR...";
    ir.enableReceive();
    ui.drawStatusBar("[B]", storage.getCount(), "[*]SCAN");
    ui.drawSignalInfo("", "", "", "", statusMessage);
    ui.drawFooter("Short:SEL Long:MENU");
  }

  void update(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (scanState == ScanState::IDLE) {
      if (ir.hasReceived()) {
        Signal received = ir.decodeSignal();

        if (storage.getCount() >= SignalStorage::MAX_SIGNALS) {
          scanState = ScanState::FULL;
          statusMessage = "FULL! Del old sigs";
          stateEnterTime = millis();
        } else if (storage.isDuplicate(received.protocol, received.address, received.command)) {
          scanState = ScanState::RECEIVED_DUP;
          statusMessage = "DUP - skip";
          lastSignal = received;
          stateEnterTime = millis();
        } else {
          storage.addSignal(received);
          scanState = ScanState::RECEIVED_NEW;
          statusMessage = "OK New signal saved";
          lastSignal = received;
          stateEnterTime = millis();

          // 蜂鸣器提示
          if (SettingsMode::getBuzzerEnabled() && M5.Speaker.isEnabled()) {
            M5.Speaker.tone(2000, 50);
          }
        }

        ui.drawStatusBar("[B]", storage.getCount(), "[*]SCAN");
        ui.drawSignalInfo(lastSignal.name, lastSignal.protocol,
                          lastSignal.address, lastSignal.command,
                          statusMessage);
      }
    } else {
      // 非 IDLE 状态，2 秒后Auto回到 IDLE
      if (millis() - stateEnterTime >= DISPLAY_DURATION_MS) {
        scanState = ScanState::IDLE;
        statusMessage = "Waiting for IR...";
        lastSignal = Signal{};
        ui.drawSignalInfo("", "", "", "", statusMessage);
      }
    }
  }

  void onShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    // 扫描模式Down短按无操作（通过状态机长按回到MENU，再进入CTRL模式）
  }

  bool onLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    ir.disableReceive();
    return true;  // BackMENU
  }

  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    ir.disableReceive();
    return true;
  }
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    ir.disableReceive();
    return true;
  }

}
