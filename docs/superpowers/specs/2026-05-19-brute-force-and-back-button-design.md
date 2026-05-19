# BruteForce 遍历模式 + btnB 返回按钮设计

## 背景

当前红外扫描系统只有 btnA（短按选择/长按确认），且必须手持原装遥控器才能学习信号。用户提出两个功能需求：

1. **不拿遥控器也能尝试控制设备** — 通过生成式遍历常见品牌码库
2. **btnB 返回按钮** — 类似浏览器 Back，支持层级回退

## 目标

- 新增 `BruteForceMode`（菜单名 `"Find"`），无需遥控器，自动遍历常见红外码组合
- 新增 btnB 全局支持，短按返回上一级，长按返回主菜单
- 所有修改最小侵入现有代码，遵循现有命名空间和状态机风格

---

## 1. BruteForce 遍历模式

### 1.1 定位

新增第 5 个主菜单项 `"Find"`，与 Scan / Ctrl / Mgmt / Set 并列。

### 1.2 状态机

```
PROTOCOL_SELECT --(btnA长)--> BRUTING --(btnA长/命中)--> SAVE_CONFIRM
       ^                            |                            |
       |                        (btnB短)                     (btnA长)
       |                            |                            |
   (btnB短/长)                  PROTOCOL_SELECT              BRUTING
   /MAIN_MENU                     (btnB短)                   (btnB短)
                              /MAIN_MENU(btnB长)           /BRUTING继续
```

| 状态 | 屏幕内容 | btnA 短按 | btnA 长按 | btnB 短按 | btnB 长按 |
|------|---------|----------|----------|----------|----------|
| **PROTOCOL_SELECT** | 协议列表（NEC/Samsung/LG/Sony/Panasonic），选中高亮 | 下一个协议 | 确认，进入 BRUTING | 回主菜单 | 回主菜单 |
| **BRUTING** | 协议名、地址、命令、进度（如 `23/480`）、状态 `Trying...` | 加速步进（0.5s/次） | **保存当前码**（命中） | 回 PROTOCOL_SELECT（停止） | 回主菜单（停止） |
| **SAVE_CONFIRM** | 分类选择（TV/AC/FAN/STB/Custom），名称列表 | 下一个选项 | 确认保存 | 回 BRUTING（取消） | 回主菜单 |

### 1.3 码库策略

不存储具体设备品牌映射，而是按协议预置**常见地址列表**和**常见命令列表**的笛卡尔积。数据量控制在 **~500 组合以内**，最坏遍历约 **8 分钟**。

#### NEC 协议（常见地址 × 常见命令）

```cpp
// 24 个常见 NEC 地址（基于社区经验和常见品牌）
const std::vector<uint32_t> NEC_ADDRS = {
  0x0000, 0x00FF, 0x01FE, 0x02FD, 0x04FB,
  0x08F7, 0x10EF, 0x20DF, 0x40BF, 0x807F,
  0xA55A, 0xAD52, 0xBD42, 0xC03F, 0xD02F,
  0xE01F, 0xE0E0, 0xE110, 0xE190, 0xE210,
  0xE290, 0xE310, 0xE390, 0xFF00
};
// 10 个常见命令（主要是电源、音量、频道）
const std::vector<uint32_t> NEC_CMDS = {
  0x02, 0x07, 0x09, 0x0C, 0x10,
  0x1E, 0x40, 0x43, 0x44, 0x45
};
// 组合数: 24 × 10 = 240
```

#### Samsung 协议

```cpp
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
// 组合数: 20 × 10 = 200
```

#### LG 协议

```cpp
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
// 组合数: 16 × 10 = 160
```

#### Sony 协议（12-bit 命令）

```cpp
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
// 组合数: 16 × 10 = 160
```

#### Panasonic 协议（64-bit）

```cpp
const std::vector<uint32_t> PANASONIC_ADDRS = {
  0x4004, 0x8008, 0xBFAE, 0xBFAD,
  0xBFAC, 0xBFAB, 0xBFAA, 0xBFA9
};
const std::vector<uint32_t> PANASONIC_CMDS = {
  0x0100, 0x0190, 0x0191, 0x0192, 0x0193,
  0x0194, 0x0195, 0x0196, 0x0197, 0x0198
};
// 组合数: 8 × 10 = 80
```

**总计：** 最大协议（NEC）240 组合，最小（Panasonic）80 组合。遍历间隔 1 秒（BRUTING 下 btnA 短按可加速到 0.5 秒）。

### 1.4 发射逻辑

```cpp
// BRUTING 状态下，update() 中每间隔发射
if (millis() - lastSendTime >= stepIntervalMs) {
    lastSendTime = millis();
    
    // 构造临时 Signal
    Signal testSignal;
    testSignal.protocol = currentProtocol;
    testSignal.address = uint32ToHex(addrList[addrIndex]);
    testSignal.command = uint32ToHex(cmdList[cmdIndex]);
    
    // 通过 IRController 发射
    ir.sendSignal(testSignal);
    
    // 前进到下一个组合
    advanceCombination();
}
```

### 1.5 保存流程

命中后进入 SAVE_CONFIRM，复用 SignalManager 的预设名称库：

- 分类选择：TV / AC / FAN / STB / Custom（同 SignalManager）
- 名称选择：各分类下的预设名称列表（同 SignalManager）
- 保存格式：`{Category}-{Name}`
- 保存方式：调用 `storage.addSignal(signal)`，signal 的 `rawData` 为空（纯协议发射即可，不需要 raw 回退）

### 1.6 UI 绘制

BRUTING 状态下使用 `UIScreen::drawSignalInfo` 显示：
- name: `{Protocol} [{progress}]`
- protocol: 当前协议
- address: 当前地址
- command: 当前命令
- status: `"Trying..."` / `"Hit! Save?"`

SAVE_CONFIRM 状态下使用 `UIScreen::drawMenu` 显示分类/名称列表。

---

## 2. btnB 智能混合返回

### 2.1 全局交互约定

| 按钮 | 短按 | 长按 |
|------|------|------|
| **btnA** | 向下选择 / 循环 | 确认 / 进入 / 执行 |
| **btnB** | **返回上一级** | **返回主菜单** |

### 2.2 AppStateMachine 修改

在 `checkButton()` 中增加 btnB 检测，与 btnA 平级：

```cpp
bool btnBPressed = M5.BtnB.isPressed();
// btnB 的按下/释放/长按检测逻辑与 btnA 相同
```

新增 `onBtnBShortPress()` 和 `onBtnBLongPress()`，分发到各模式：

```cpp
void onBtnBShortPress() {
  switch (state_) {
    case AppState::MAIN_MENU: /* 无操作 */ break;
    case AppState::SCAN_MODE: ScanMode::onBtnBShortPress(ui_, storage_, ir_); break;
    case AppState::CONTROL_MODE: ControlMode::onBtnBShortPress(ui_, storage_); break;
    case AppState::SIGNAL_MANAGER: SignalManager::onBtnBShortPress(ui_, storage_); break;
    case AppState::SETTINGS: SettingsMode::onBtnBShortPress(ui_, storage_); break;
    case AppState::BRUTE_FORCE: BruteForceMode::onBtnBShortPress(ui_, storage_, ir_); break;
  }
}

void onBtnBLongPress() {
  switch (state_) {
    case AppState::MAIN_MENU: /* 无操作 */ break;
    case AppState::SCAN_MODE: /* 停止接收 */ changeState(AppState::MAIN_MENU); break;
    case AppState::CONTROL_MODE: changeState(AppState::MAIN_MENU); break;
    case AppState::SIGNAL_MANAGER: changeState(AppState::MAIN_MENU); break;
    case AppState::SETTINGS: changeState(AppState::MAIN_MENU); break;
    case AppState::BRUTE_FORCE: /* 停止遍历 */ changeState(AppState::MAIN_MENU); break;
  }
}
```

### 2.3 各模式回退路径

#### ScanMode

| 当前状态 | btnB 短按 | btnB 长按 |
|---------|----------|----------|
| IDLE / RECEIVED_* / FULL | 回主菜单（停止接收） | 回主菜单（停止接收） |

#### ControlMode

| 当前状态 | btnB 短按 | btnB 长按 |
|---------|----------|----------|
| LIST | 回主菜单 | 回主菜单 |
| CATEGORY_SELECT | 回 LIST | 回主菜单 |
| SENDING | 回 LIST（中断） | 回主菜单 |

#### SignalManager

| 当前状态 | btnB 短按 | btnB 长按 |
|---------|----------|----------|
| SIGNAL_LIST | 回主菜单 | 回主菜单 |
| ACTION_MENU | 回 SIGNAL_LIST | 回主菜单 |
| RENAME_CATEGORY | 回 ACTION_MENU | 回主菜单 |
| RENAME_NAME | 回 RENAME_CATEGORY | 回主菜单 |
| CATEGORY_SELECT | 回 ACTION_MENU | 回主菜单 |
| DELETE_CONFIRM | 回 ACTION_MENU（取消） | 回主菜单 |
| RAW_DATA | 回 SIGNAL_LIST | 回主菜单 |

#### SettingsMode

| 当前状态 | btnB 短按 | btnB 长按 |
|---------|----------|----------|
| MENU | 回主菜单 | 回主菜单 |
| CONFIRM_CLEAR | 回 MENU（取消） | 回主菜单 |
| ABOUT | 回 MENU | 回主菜单 |

#### BruteForceMode

| 当前状态 | btnB 短按 | btnB 长按 |
|---------|----------|----------|
| PROTOCOL_SELECT | 回主菜单 | 回主菜单 |
| BRUTING | 回 PROTOCOL_SELECT（停止） | 回主菜单（停止） |
| SAVE_CONFIRM | 回 BRUTING（取消） | 回主菜单 |

### 2.4 各模式 API 新增

每个模式命名空间新增两个函数签名：

```cpp
namespace XxxMode {
  // ... 原有函数
  void onBtnBShortPress(UIScreen& ui, SignalStorage& storage, ...);
  void onBtnBLongPress(UIScreen& ui, SignalStorage& storage, ...);
}
```

BruteForceMode 完整 API：

```cpp
namespace BruteForceMode {
  void enter(UIScreen& ui, SignalStorage& storage, IRController& ir);
  void update(UIScreen& ui, SignalStorage& storage, IRController& ir);
  void onShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir);
  bool onLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir);
  void onBtnBShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir);
  void onBtnBLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir);
}
```

---

## 3. 文件变更清单

| 文件 | 变更类型 | 说明 |
|------|---------|------|
| `src/AppStateMachine.h` | 修改 | 新增 AppState::BRUTE_FORCE，新增 btnB 检测和分发 |
| `src/AppStateMachine.cpp` | 修改 | 新增 MENU_ITEMS `"Find"`，新增 checkBtnB()、onBtnBShortPress()、onBtnBLongPress() |
| `src/ScanMode.h` | 修改 | 新增 onBtnBShortPress、onBtnBLongPress |
| `src/ScanMode.cpp` | 修改 | 实现 btnB 回调 |
| `src/ControlMode.h` | 修改 | 新增 onBtnBShortPress、onBtnBLongPress |
| `src/ControlMode.cpp` | 修改 | 实现 btnB 回调，处理子状态回退 |
| `src/SignalManager.h` | 修改 | 新增 onBtnBShortPress、onBtnBLongPress |
| `src/SignalManager.cpp` | 修改 | 实现 btnB 回调，处理多层子状态回退 |
| `src/SettingsMode.h` | 修改 | 新增 onBtnBShortPress、onBtnBLongPress |
| `src/SettingsMode.cpp` | 修改 | 实现 btnB 回调 |
| `src/BruteForceMode.h` | 新增 | 头文件 |
| `src/BruteForceMode.cpp` | 新增 | 完整实现 |
| `src/UIScreen.h/cpp` | 可能修改 | 如需新增绘制函数（如 drawBruteForceStatus） |
| `platformio.ini` | 不修改 | 无新增库依赖 |

---

## 4. 兼容性

- **ESP32 编译**：无新增库依赖，仅增加代码量（估计 +8~12KB Flash），仍在可用范围内
- **native 测试**：BruteForceMode 依赖 IRController（Arduino 库），不加入 native 编译。现有 native 测试不受影响
- **内存**：遍历状态的临时数据全部用静态局部变量，无动态分配

---

## 5. 边界情况

| 场景 | 处理 |
|------|------|
| BRUTING 遍历到末尾 | 自动停止，显示 `"Done! No match"`，btnB 返回 PROTOCOL_SELECT |
| BRUTING 中存储已满（50/50） | 尝试保存时显示 `"FULL!"`，2 秒后返回 BRUTING |
| SAVE_CONFIRM 时选择了已有名称 | 正常保存（SignalStorage::addSignal 会分配新 id），允许重复名称 |
| 长按 btnB 时正在发射红外 | 立即停止遍历并返回主菜单 |
| btnA 和 btnB 同时按下 | 各自独立处理，无特殊逻辑 |
