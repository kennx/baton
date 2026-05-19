#include <unity.h>
#include "SignalData.h"

void test_signal_default_values() {
  Signal s;
  TEST_ASSERT_EQUAL(-1, s.id);
  TEST_ASSERT_TRUE(s.name.empty());
  TEST_ASSERT_TRUE(s.rawData.empty());
}

void test_signal_equality() {
  Signal a{.id = 1, .name = "TV-Power", .protocol = "NEC", .address = "0x20DF", .command = "0x10"};
  Signal b{.id = 2, .name = "TV-VolUp", .protocol = "NEC", .address = "0x20DF", .command = "0x10"};
  Signal c{.id = 3, .name = "TV-Power", .protocol = "NEC", .address = "0x20DF", .command = "0x11"};

  TEST_ASSERT_TRUE(a == b);   // 相同协议/地址/命令
  TEST_ASSERT_FALSE(a == c);  // 不同命令
}

void test_signal_raw_data() {
  Signal s;
  s.rawData = {9000, 4500, 560, 560, 560, 1690};
  s.rawLength = s.rawData.size();
  TEST_ASSERT_EQUAL(6, s.rawLength);
  TEST_ASSERT_EQUAL(9000, s.rawData[0]);
  TEST_ASSERT_EQUAL(1690, s.rawData[5]);
}


