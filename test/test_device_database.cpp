#ifdef UNIT_TEST

#include <unity.h>
#include "DeviceDatabase.h"

void test_database_has_profiles() {
  auto profiles = DeviceDatabase::getAllProfiles();
  TEST_ASSERT_TRUE(profiles.size() > 0);
}

void test_samsung_profile() {
  auto p = DeviceDatabase::getProfile("Samsung TV");
  TEST_ASSERT_EQUAL_STRING("Samsung TV", p.brand.c_str());
  TEST_ASSERT_EQUAL_STRING("SAMSUNG", p.protocol.c_str());
  TEST_ASSERT_EQUAL_STRING("0x0707", p.address.c_str());
  TEST_ASSERT_TRUE(p.commands.size() > 0);
  TEST_ASSERT_EQUAL_STRING("Power", p.commands[0].name.c_str());
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_database_has_profiles);
  RUN_TEST(test_samsung_profile);
  return UNITY_END();
}

#endif
