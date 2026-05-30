// Arduino Uno + CNC Shield v3
// Команды: START, STOP, HOME, POS

#define X_STEP 2
#define X_DIR  5
#define Y_STEP 3
#define Y_DIR  6
#define ENABLE 8

const float SCAN_X_MM = 180;
const float SCAN_Y_MM = 170;

const int MOTOR_STEPS_PER_REV = 200;
const float BELT_PITCH = 2.0;
const float PULLEY_TEETH = 20.0;

const float MM_PER_REV = BELT_PITCH * PULLEY_TEETH;
const float STEPS_PER_MM = MOTOR_STEPS_PER_REV / MM_PER_REV;

const int stepDelay = 800;
const int measureDelay = 500;

int X_POINTS = 0;
int Y_POINTS = 0;

long xMotorSteps = 0;
long yMotorSteps = 0;

long currentXSteps = 0;
long currentYSteps = 0;

bool stopRequested = false;

void setup() {
  Serial.begin(115200);

  pinMode(X_STEP, OUTPUT);
  pinMode(X_DIR, OUTPUT);
  pinMode(Y_STEP, OUTPUT);
  pinMode(Y_DIR, OUTPUT);
  pinMode(ENABLE, OUTPUT);

  digitalWrite(ENABLE, LOW);

  delay(2000);

  Serial.println("=== SCANNER READY ===");
  Serial.println("Commands:");
  Serial.println("START - begin scan");
  Serial.println("STOP  - stop scan");
  Serial.println("HOME  - return to start");
  Serial.println("POS   - show position");

  Serial.println("Enter X points:");
  X_POINTS = readIntFromSerial();

  Serial.println("Enter Y points:");
  Y_POINTS = readIntFromSerial();

  calculateSteps();

  printSettings();
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "START") {
      stopRequested = false;
      Serial.println("START SCAN");
      scanSnake();
      Serial.println("SCAN FINISHED");
    }

    else if (command == "STOP") {
      stopRequested = true;
      Serial.println("STOP REQUESTED");
    }

    else if (command == "HOME") {
      stopRequested = false;
      returnHome();
    }

    else if (command == "POS") {
      printPosition();
    }

    else {
      Serial.println("Unknown command");
    }
  }
}

int readIntFromSerial() {
  while (true) {
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();

      if (input.length() > 0) {
        int value = input.toInt();

        if (value > 1) {
          return value;
        }

        Serial.println("Enter number > 1");
      }
    }
  }
}

void calculateSteps() {
  float xStepMm = SCAN_X_MM / (X_POINTS - 1);
  float yStepMm = SCAN_Y_MM / (Y_POINTS - 1);

  xMotorSteps = round(xStepMm * STEPS_PER_MM);
  yMotorSteps = round(yStepMm * STEPS_PER_MM);
}

void printSettings() {
  Serial.println("========== SETTINGS ==========");
  Serial.print("SCAN_X_MM = ");
  Serial.println(SCAN_X_MM);

  Serial.print("SCAN_Y_MM = ");
  Serial.println(SCAN_Y_MM);

  Serial.print("X_POINTS = ");
  Serial.println(X_POINTS);

  Serial.print("Y_POINTS = ");
  Serial.println(Y_POINTS);

  Serial.print("X motor steps between points = ");
  Serial.println(xMotorSteps);

  Serial.print("Y motor steps between points = ");
  Serial.println(yMotorSteps);

  Serial.println("==============================");
}

void scanSnake() {
  bool xForward = true;

  for (int y = 0; y < Y_POINTS; y++) {
    for (int xIndex = 0; xIndex < X_POINTS; xIndex++) {
      checkSerialDuringScan();

      if (stopRequested) {
        Serial.println("SCAN STOPPED");
        printPosition();
        return;
      }

      int xCoord = xForward ? xIndex : X_POINTS - 1 - xIndex;

      Serial.print("MEASURE X=");
      Serial.print(xCoord);
      Serial.print(" Y=");
      Serial.println(y);

      printPosition();

      delay(measureDelay);

      if (xIndex < X_POINTS - 1) {
        setDirection(X_DIR, xForward);

        if (moveMotor(X_STEP, xMotorSteps)) {
          currentXSteps += xForward ? xMotorSteps : -xMotorSteps;
        }
      }
    }

    if (y < Y_POINTS - 1) {
      setDirection(Y_DIR, true);

      if (moveMotor(Y_STEP, yMotorSteps)) {
        currentYSteps += yMotorSteps;
      }
    }

    xForward = !xForward;
  }

  returnHome();
}

void returnHome() {
  Serial.println("RETURN HOME");

  if (currentXSteps > 0) {
    setDirection(X_DIR, false);
    moveMotor(X_STEP, currentXSteps);
    currentXSteps = 0;
  } else if (currentXSteps < 0) {
    setDirection(X_DIR, true);
    moveMotor(X_STEP, -currentXSteps);
    currentXSteps = 0;
  }

  if (currentYSteps > 0) {
    setDirection(Y_DIR, false);
    moveMotor(Y_STEP, currentYSteps);
    currentYSteps = 0;
  } else if (currentYSteps < 0) {
    setDirection(Y_DIR, true);
    moveMotor(Y_STEP, -currentYSteps);
    currentYSteps = 0;
  }

  Serial.println("HOME");
  printPosition();
}

void printPosition() {
  float xMm = currentXSteps / STEPS_PER_MM;
  float yMm = currentYSteps / STEPS_PER_MM;

  Serial.print("POSITION X_mm=");
  Serial.print(xMm);
  Serial.print(" Y_mm=");
  Serial.print(yMm);

  Serial.print(" X_steps=");
  Serial.print(currentXSteps);
  Serial.print(" Y_steps=");
  Serial.println(currentYSteps);
}

bool moveMotor(int stepPin, long steps) {
  for (long i = 0; i < steps; i++) {
    checkSerialDuringScan();

    if (stopRequested) {
      return false;
    }

    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }

  return true;
}

void setDirection(int dirPin, bool forward) {
  digitalWrite(dirPin, forward ? HIGH : LOW);
}

void checkSerialDuringScan() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "STOP") {
      stopRequested = true;
    }

    else if (command == "POS") {
      printPosition();
    }
  }
}
