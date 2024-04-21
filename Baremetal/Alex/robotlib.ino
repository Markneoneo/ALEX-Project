#include "MotorLib.cpp"



// Move Alex forward "dist" cm at speed "speed".
// "speed" is expressed as a percentage. E.g. 50 is
// move forward at half speed.
// Specifying a distance of 0 means Alex will
// continue moving forward indefinitely.

void move(float speed, int direction)
{ 
  startMotor();
  int speed_scaled = (speed/100.0) * 255;
  setSpeed(speed_scaled);
  switch(direction)
    {
      case BACK:
        runFL(BACKWARD);
        runFR(FORWARD);
        runBL(BACKWARD);
        runBR(FORWARD); 
      break;
      case GO: //WE CHANGED THIS
        runFL(FORWARD);
        runFR(BACKWARD);
        runBL(FORWARD);
        runBR(BACKWARD); 
      break;
      case CCW:
        runFL(BACKWARD);
        runFR(BACKWARD);
        runBL(BACKWARD);
        runBR(BACKWARD); 
      break;
      case CW:
        runFL(FORWARD);
        runFR(FORWARD);
        runBL(FORWARD);
        runBR(FORWARD); 
      break;
      case STOP:
      default:
        runFL(STOP);
        runFR(STOP);
        runBL(STOP);
        runBR(STOP); 
    }
}

void forward(float dist, float speed)
{ 
  if(dist > 0) {
    deltaDist = dist;
  }
  else {
    deltaDist = 999999;
  }
  newDist = forwardDist + deltaDist;
  dir = (TDirection) FORWARD;
  move(speed, FORWARD);
}

void backward(float dist, float speed)
{
  if(dist > 0) {
    deltaDist = dist;
  } 
  else {
    deltaDist = 999999;
  }
  newDist = reverseDist + deltaDist;
  dir = (TDirection) BACKWARD;
  move(speed, BACKWARD);
}

void ccw(float ang, float speed)
{
  dir = (TDirection) LEFT;
  move(speed, CCW);
}

void cw(float ang, float speed)
{
  dir = (TDirection) RIGHT;
  move(speed, CW);
}

void stop()
{
  dir = (TDirection) STOP;
  move(0, STOP);
}

void left(float ang, float speed)
{
  if(ang == 0) {
    deltaTicks = 999999;
  }
  else {
    deltaTicks = computeDeltaTicks(ang);
  }
  targetTicks = leftReverseTicksTurns + deltaTicks;
  ccw(ang, speed);
}

void right(float ang, float speed)
{
  if(ang == 0) {
    deltaTicks = 999999;
  }
  else {
    deltaTicks = computeDeltaTicks(ang);
  }
  targetTicks = rightReverseTicksTurns + deltaTicks;
  cw(ang, speed);
}
