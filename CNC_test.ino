// CNC Shield v3 + Arduino Uno
// X отдельно
// Y и A вместе (дублирование Y)

#define X_STEP 2
#define X_DIR  5

#define Y_STEP 3
#define Y_DIR  6

// A на CNC shield дублирует Y через перемычки
// поэтому отдельно A управлять не надо

#define ENABLE 8

int stepDelay = 800;
int stepsCount = 400;

void setup() {
  pinMode(X_STEP, OUTPUT);
  pinMode(X_DIR, OUTPUT);

  pinMode(Y_STEP, OUTPUT);
  pinMode(Y_DIR, OUTPUT);

  pinMode(ENABLE, OUTPUT);

  digitalWrite(ENABLE, LOW); // включить драйверы

  // направление вперед
  digitalWrite(X_DIR, HIGH);
  digitalWrite(Y_DIR, HIGH);
}

void loop() {

  // Движение X
  moveMotor(X_STEP, stepsCount);
  delay(1000);

  // Движение Y + A
  // A поедет автоматически вместе с Y
  moveMotor(Y_STEP, stepsCount);
  delay(1000);
}

void moveMotor(int stepPin, int steps) {
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);

    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }
}