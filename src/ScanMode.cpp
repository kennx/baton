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
    statusMessage = "等待红外信号...";
    ir.enableReceive();
    ui.drawStatusBar("🔋", storage.getCount(), "🔴扫描中");
    ui.drawSignalInfo("", "", "", "", statusMessage);
    ui.drawFooter("短按:控制模式 长按:主菜单");
  }

  void update(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (scanState == ScanState::IDLE) {
      if (ir.hasReceived()) {
        Signal received = ir.decodeSignal();

        if (storage.getCount() >= SignalStorage::MAX_SIGNALS) {
          scanState = ScanState::FULL;
          statusMessage = "存储已满！请删除旧信号";
          stateEnterTime = millis();
        } else if (storage.isDuplicate(received.protocol, received.address, received.command)) {
          scanState = ScanState::RECEIVED_DUP;
          statusMessage = "重复信号 — 跳过";
          lastSignal = received;
          stateEnterTime = millis();
        } else {
          storage.addSignal(received);
          scanState = ScanState::RECEIVED_NEW;
          statusMessage = "✓ 新信号已保存";
          lastSignal = received;
          stateEnterTime = millis();

          // 蜂鸣器提示
          if (SettingsMode::getBuzzerEnabled() && M5.Speaker.isEnabled()) {
            M5.Speaker.tone(2000, 50);
          }
        }

        ui.drawStatusBar("🔋", storage.getCount(), "🔴扫描中");
        ui.drawSignalInfo(lastSignal.name, lastSignal.protocol,
                          lastSignal.address, lastSignal.command,
                          statusMessage);
      }
    } else {
      // 非 IDLE 状态，2 秒后自动回到 IDLE
      if (millis() - stateEnterTime >= DISPLAY_DURATION_MS) {
        scanState = ScanState::IDLE;
        statusMessage = "等待红外信号...";
        lastSignal = Signal{};
        ui.drawSignalInfo("", "", "", "", statusMessage);
      }
    }
  }

  void onShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    // 扫描模式下短按无操作（通过状态机长按回到主菜单，再进入控制模式）
  }

  bool onLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    ir.disableReceive();
    return true;  // 返回主菜单
  }

}
