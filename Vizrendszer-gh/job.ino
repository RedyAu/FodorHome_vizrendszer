bool directJobs() {
  /* Combined function for getting water from tap and dumping from the tanks.
   * Dumping empties both tanks and than finishes.
   * Tap flow only empties watering tank, and than continues indefinetely from well.
   */

  if (directTap) {
    currentJob = waterJob{NoStopNext, 0, toTap};
    return End;
  } else if (directGrey) {
    currentJob = waterJob{NoStopNext, 0, toGrey};
    return End;
  } else if (directPink) {
    currentJob = waterJob{NoStopNext, 0, toPink};
    return End;
  } else if (directGreen) {
    currentJob = waterJob{NoStopNext, 0, toGreen};
    return End;
  } else if (directBlue) {
    currentJob = waterJob{NoStopNext, 0, toBlue};
    return End;
  } else if (directRed) {
    currentJob = waterJob{NoStopNext, 0, toRed};
    return End;
  } else {
    currentJob = waterJob{StopNext};
  }
  
  return Continue;
}

bool water(bool bufferDumping = false);

void job() { /////////////////////////////////////////////////////////////////
  if (pause) {
    currentJob = waterJob{StopNext};
    return;
  }
  
  //Tap/Dump
  if (directJobs()) return;
  
  //Water
  if (water()) return;

  if (currentJob.from == AllValves) return;
  //If end of tree reached, stop and reset.
  currentJob = waterJob{StopNext};
}
