#include <M5Unified.h>
#include "AppStateMachine.h"

AppStateMachine app;

void setup() {
  app.begin();
}

void loop() {
  app.update();
  delay(10);
}
