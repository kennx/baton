#include <unity.h>
#include "SignalStorage.h"

void test_storage_json_roundtrip() {
  Signal s;
  s.id = 5;
  s.name = "电视-电源";
  s.category = "电视";
  s.protocol = "NEC";
  s.address = "0x20DF";
  s.command = "0x10";
  s.rawLength = 67;
  s.createdAt = "2024-01-15T10:30:00";

  std::string json = SignalStorage::signalToJson(s);
  Signal restored = SignalStorage::jsonToSignal(json);

  TEST_ASSERT_EQUAL(5, restored.id);
  TEST_ASSERT_EQUAL_STRING("电视-电源", restored.name.c_str());
  TEST_ASSERT_EQUAL_STRING("NEC", restored.protocol.c_str());
}

void test_storage_index_roundtrip() {
  std::vector<Signal> signals;
  Signal a{.id = 0, .name = "Signal-1", .protocol = "NEC", .address = "0x20DF", .command = "0x10"};
  Signal b{.id = 1, .name = "Signal-2", .protocol = "Sony", .address = "0x01", .command = "0x15"};
  signals.push_back(a);
  signals.push_back(b);

  std::string json = SignalStorage::indexToJson(signals);
  std::vector<Signal> restored = SignalStorage::jsonToIndex(json);

  TEST_ASSERT_EQUAL(2, restored.size());
  TEST_ASSERT_EQUAL_STRING("NEC", restored[0].protocol.c_str());
  TEST_ASSERT_EQUAL_STRING("Sony", restored[1].protocol.c_str());
}

void test_storage_duplicate_detection() {
  SignalStorage storage;
  storage.begin();

  Signal a{.protocol = "NEC", .address = "0x20DF", .command = "0x10"};
  Signal b{.protocol = "NEC", .address = "0x20DF", .command = "0x11"};

  storage.addSignal(a);

  TEST_ASSERT_TRUE(storage.isDuplicate("NEC", "0x20DF", "0x10"));
  TEST_ASSERT_FALSE(storage.isDuplicate("NEC", "0x20DF", "0x11"));
  TEST_ASSERT_TRUE(storage.getCount() == 1);
}

void test_storage_generate_name() {
  SignalStorage storage;
  storage.begin();
  TEST_ASSERT_EQUAL_STRING("Signal-1", storage.generateNextName().c_str());
  storage.addSignal(Signal{.protocol = "NEC"});
  TEST_ASSERT_EQUAL_STRING("Signal-2", storage.generateNextName().c_str());
}

// Forward declarations from test_signal.cpp
extern void test_signal_default_values();
extern void test_signal_equality();
extern void test_signal_raw_data();

void setUp() {}
void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_signal_default_values);
  RUN_TEST(test_signal_equality);
  RUN_TEST(test_signal_raw_data);
  RUN_TEST(test_storage_json_roundtrip);
  RUN_TEST(test_storage_index_roundtrip);
  RUN_TEST(test_storage_duplicate_detection);
  RUN_TEST(test_storage_generate_name);
  UNITY_END();
}
