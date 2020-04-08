int waterLevel(int container) {
  //0: puffer 1: watering
  //return 0 if both sensors 0
  //return 1 if bottom sensor 1
  //return 2 if both sensors 1
  bool upper = false;
  bool lower = false;

  if (container == 1) {//If we should get the level of watering tank
    lower = waterLower; //If we have water on the lower sensor, store it
    upper = waterUpper;
  }
  else { //Otherwise get data from buffer tank
    lower = false;//bufferLower; //If we have water on the lower sensor, store it
    upper = bufferUpper;
  }

  if (!lower && !upper) return 0;
  if (lower && !upper) return 1;
  if (lower && upper) return 2;

  if (!lower && upper) {
    if (container) error(true,1300);
    else {
      return 2;
      //error(true,1310);
    }
  }
}
