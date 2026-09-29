#include <AccelStepper.h>

// Half-step mode: 64 internal steps × 64 gearbox = 4096 steps/rev
const int stepsPerRev = 4096;
long currentPos = 0;
int currentRPM = 10;

// HALF4WIRE + interleaved pin order IN1, IN3, IN2, IN4
AccelStepper myStepper(AccelStepper::HALF4WIRE, 8, 10, 9, 11);

// Helper: convert RPM to steps/second for AccelStepper
float rpmToStepsPerSec(int rpm) {
  return (float)rpm * stepsPerRev / 60.0;
}

void setup() {
  Serial.begin(115200);

  myStepper.setMaxSpeed(rpmToStepsPerSec(20)); // absolute ceiling
  myStepper.setAcceleration(500.0);            // steps/sec^2
  myStepper.setSpeed(rpmToStepsPerSec(currentRPM));

  Serial.println("--- Stepper Motor Control (HALF4WIRE / AccelStepper) ---");
  Serial.println("Comandos:");
  Serial.println("  T[int]  -> Vai para posicao absoluta    (Ex: T4096)");
  Serial.println("  R[int]  -> Move N passos relativos      (Ex: R1024, R-1024)");
  Serial.println("  V[int]  -> Altera velocidade em RPM     (Ex: V15)");
  Serial.println("  H       -> Define posicao atual como ZERO (Home)");
  Serial.println("  P       -> Imprime posicao atual");
  Serial.print("Velocidade atual: "); Serial.print(currentRPM); Serial.println(" RPM");
}

void loop() {
  // AccelStepper requires runToPosition() for blocking moves,
  // which matches the original behaviour of Stepper.step().
  if (Serial.available() > 0) {
    char c = Serial.peek();

    // --- T: Absolute position ---
    if (c == 'T' || c == 't') {
      Serial.read();
      long targetPos = Serial.parseInt();
      long moveRequired = targetPos - currentPos;

      Serial.print(">>> Alvo T: "); Serial.print(targetPos);
      Serial.print(" | Movendo "); Serial.print(moveRequired); Serial.println(" passos.");

      myStepper.move(moveRequired);
      myStepper.runToPosition(); // blocking, same behaviour as Stepper.step()

      currentPos = targetPos;
      Serial.println(">>> Alvo atingido.");
    }
    // --- R: Relative move ---
    else if (c == 'R' || c == 'r') {
      Serial.read();
      long relSteps = Serial.parseInt();

      Serial.print(">>> Relativo: "); Serial.print(relSteps); Serial.println(" passos.");

      myStepper.move(relSteps);
      myStepper.runToPosition();

      currentPos += relSteps;
      Serial.print(">>> Posicao atual: "); Serial.println(currentPos);
    }
    // --- V: Speed ---
    else if (c == 'V' || c == 'v') {
      Serial.read();
      int newRPM = Serial.parseInt();
      if (newRPM > 0 && newRPM <= 20) {
        currentRPM = newRPM;
        myStepper.setMaxSpeed(rpmToStepsPerSec(currentRPM));
        myStepper.setSpeed(rpmToStepsPerSec(currentRPM));
        Serial.print(">>> Velocidade: "); Serial.print(currentRPM); Serial.println(" RPM");
      } else {
        Serial.println("!!! Erro: Velocidade entre 1 e 20 RPM.");
      }
    }
    // --- H: Home ---
    else if (c == 'H' || c == 'h') {
      Serial.read();
      myStepper.setCurrentPosition(0); // resets AccelStepper's internal counter too
      currentPos = 0;
      Serial.println(">>> Home definido (0).");
    }
    // --- P: Print position ---
    else if (c == 'P' || c == 'p') {
      Serial.read();
      Serial.print(">>> Posicao atual: "); Serial.print(currentPos);
      Serial.print(" passos | ");
      Serial.print((float)currentPos / stepsPerRev * 360.0, 2);
      Serial.println(" graus");
    }
    else {
      Serial.read();
    }

    while (Serial.available() > 0) Serial.read(); // Limpa buffer
  }
}
