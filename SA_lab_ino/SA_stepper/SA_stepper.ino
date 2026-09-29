#include <Stepper.h>

const int stepsPerRev = 2048;
long currentPos = 0;
int currentRPM = 10;

// Pin order IN1, IN3, IN2, IN4 required for correct coil sequence on 28BYJ-48
Stepper myStepper(stepsPerRev, 8, 10, 9, 11);

void setup() {
  myStepper.setSpeed(currentRPM);
  Serial.begin(115200);

  Serial.println("--- Stepper Motor Control ---");
  Serial.println("Comandos:");
  Serial.println("  T[int]  -> Vai para posicao absoluta    (Ex: T2048)");
  Serial.println("  R[int]  -> Move N passos relativos      (Ex: R512, R-512)");
  Serial.println("  V[int]  -> Altera velocidade em RPM     (Ex: V15)");
  Serial.println("  H       -> Define posicao atual como ZERO (Home)");
  Serial.println("  P       -> Imprime posicao atual");
  Serial.print("Velocidade atual: "); Serial.print(currentRPM); Serial.println(" RPM");
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.peek();

    // --- T: Absolute position ---
    if (c == 'T' || c == 't') {
      Serial.read();
      long targetPos = Serial.parseInt();
      long moveRequired = targetPos - currentPos;

      Serial.print(">>> Alvo T: "); Serial.print(targetPos);
      Serial.print(" | Movendo "); Serial.print(moveRequired); Serial.println(" passos.");

      myStepper.step(moveRequired);

      currentPos = targetPos;
      Serial.println(">>> Alvo atingido.");
    }
    // --- R: Relative move ---
    else if (c == 'R' || c == 'r') {
      Serial.read();
      long relSteps = Serial.parseInt();

      Serial.print(">>> Relativo: "); Serial.print(relSteps); Serial.println(" passos.");

      myStepper.step(relSteps);

      currentPos += relSteps;
      Serial.print(">>> Posicao atual: "); Serial.println(currentPos);
    }
    // --- V: Speed ---
    else if (c == 'V' || c == 'v') {
      Serial.read();
      int newRPM = Serial.parseInt();
      if (newRPM > 0 && newRPM <= 20) {
        currentRPM = newRPM;
        myStepper.setSpeed(currentRPM);
        Serial.print(">>> Velocidade: "); Serial.print(currentRPM); Serial.println(" RPM");
      } else {
        Serial.println("!!! Erro: Velocidade entre 1 e 20 RPM.");
      }
    }
    // --- H: Home ---
    else if (c == 'H' || c == 'h') {
      Serial.read();
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
