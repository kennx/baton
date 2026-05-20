#ifdef UNIT_TEST

#include <unity.h>
#include "DeviceDatabase.h"

void test_database_has_categories() {
  size_t catCount = DeviceDatabase::getCategoryCount();
  TEST_ASSERT_TRUE(catCount > 0);
  
  const char* firstCatName = DeviceDatabase::getCategoryName(0);
  TEST_ASSERT_NOT_NULL(firstCatName);
}

void test_database_profile_access() {
  size_t pCount = DeviceDatabase::getProfileCount(0);
  TEST_ASSERT_TRUE(pCount > 0);
  
  const DeviceProfileStatic* p = DeviceDatabase::getProfile(0, 0);
  TEST_ASSERT_NOT_NULL(p);
  TEST_ASSERT_NOT_NULL(p->brand);
  TEST_ASSERT_NOT_NULL(p->protocol);
  TEST_ASSERT_TRUE(p->commandCount > 0);
  
  DeviceProfile dyn = DeviceDatabase::toDynamic(p);
  TEST_ASSERT_EQUAL_STRING(p->brand, dyn.brand.c_str());
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_database_has_categories);
  RUN_TEST(test_database_profile_access);
  return UNITY_END();
}

#endif
