bool bufferUpper, bufferLower, waterUpper, waterLower, groundWater, bufferLowerChange, bufferUpperChange, waterLowerChange, waterUpperChange, prevTapSwitchSt;
unsigned long forSenseMillis, forBufferLowerMillis, forBufferUpperMillis, forWaterLowerMillis, forWaterUpperMillis;

void sense() { //Read data from temperature and humidity sensors and write it to variables so other functions can use them
  tapFlowControl();
}

int levelOf(int container) {
  //0: puffer 1: watering
  //return 0 if both sensors 0
  //return 1 if bottom sensor 1
  //return 2 if both sensors 1

  bool upper = container ? waterUpper : bufferUpper;
  bool lower = container ? waterLower : bufferLower;

  if (!lower && !upper) return 0;
  if (lower && !upper) return 1;
  if (lower && upper) return 2;

  if (!lower && upper) {
    if (container) error(1300);
    else {
      return 0;
      error(1310);
    }
  }
  return 0;
}
