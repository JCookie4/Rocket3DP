const int redLED     = 13;
const int yellowLED  = 12;
const int yellowLED2 = 11;
const int greenLED   = 10;

const int armButton    = 9;
const int launchButton = 6;
const int abortButton  = 7;
const int buzzer       = 5;


const int firePin = 4;

const unsigned long FIRE_DURATION_MS = 5000;

const int COUNTDOWN_SECONDS = 10;

enum State { IDLE, ARMED, COUNTDOWN, LAUNCHED, ABORTED };
State currentState = IDLE;

unsigned long countdownStart = 0;
int lastSecond = -1;

bool lastArm = HIGH, lastLaunch = HIGH, lastAbort = HIGH;

void setup() {
  Serial.begin(9600);

  pinMode(firePin, OUTPUT);
  digitalWrite(firePin, LOW); // Initial State Check to ensure rocket does not launch prematurely. 

  pinMode(redLED,     OUTPUT);
  pinMode(yellowLED,  OUTPUT);
  pinMode(yellowLED2, OUTPUT);
  pinMode(greenLED,   OUTPUT);
  pinMode(buzzer,     OUTPUT);

  pinMode(armButton,    INPUT_PULLUP);
  pinMode(launchButton, INPUT_PULLUP);
  pinMode(abortButton,  INPUT_PULLUP);

  Serial.println("IDLE - Press ARM");
}

void loop() {
  if (currentState != COUNTDOWN) {
    digitalWrite(firePin, LOW);
  }

  bool a = digitalRead(armButton);
  bool l = digitalRead(launchButton);
  bool b = digitalRead(abortButton);

  bool armEdge    = (a == LOW && lastArm    == HIGH);
  bool launchEdge = (l == LOW && lastLaunch == HIGH);
  bool abortEdge  = (b == LOW && lastAbort  == HIGH);

  lastArm = a;
  lastLaunch = l;
  lastAbort = b;

  switch (currentState) {

    // ================= IDLE =================
    case IDLE:

      digitalWrite(redLED,     HIGH);
      digitalWrite(yellowLED,  HIGH);
      digitalWrite(yellowLED2, HIGH);
      digitalWrite(greenLED,   HIGH);

      if (armEdge) {

        // ARMING BEEP
        tone(buzzer, 1500, 250);

        currentState = ARMED;
        Serial.println("ARMED - Press LAUNCH");
      }

      break;

    // ================= ARMED =================
    case ARMED: {

      bool blink = (millis() % 600) < 300;

      digitalWrite(redLED,     LOW);
      digitalWrite(yellowLED,  blink);
      digitalWrite(yellowLED2, blink);
      digitalWrite(greenLED,   blink);

      if (abortEdge) {
        currentState = IDLE;
        Serial.println("IDLE - Press ARM");
      }

      if (launchEdge) {
        countdownStart = millis();
        lastSecond = -1;
        currentState = COUNTDOWN;
      }

      break;
    }

    // ================= COUNTDOWN =================
    case COUNTDOWN: {

      int t = COUNTDOWN_SECONDS - (int)((millis() - countdownStart) / 1000);

      // COUNTDOWN BEEPS
      if (t != lastSecond) {

        lastSecond = t;

        Serial.print("T-");
        Serial.println(t);

        // Normal countdown beep
        if (t > 3) {
          tone(buzzer, 1200, 120);
          delay(200);
          tone(buzzer, 850, 120);
        }

        // FAST FINAL COUNTDOWN
        else if (t > 0) {

          for (int i = 0; i < 4; i++) {
            tone(buzzer, 2200, 40);
            delay(70);
          }
        }
      }

      digitalWrite(yellowLED,  HIGH);
      digitalWrite(yellowLED2, (millis() % 150) < 75);
      digitalWrite(redLED, LOW);
      digitalWrite(greenLED, LOW);

      // ABORT
      if (abortEdge) {

        // --- NEW: explicit safety line ---
        // Redundant with the firePin staying LOW throughout countdown,
        // but explicit here so it's visually obvious this path is safe.
        digitalWrite(firePin, LOW);

        noTone(buzzer);

        digitalWrite(yellowLED,  LOW);
        digitalWrite(yellowLED2, LOW);
        digitalWrite(redLED,     HIGH);

        tone(buzzer, 300, 1000);

        currentState = ABORTED;
        Serial.println("ABORTED");
      }

      // IGNITION
      if (t <= 0) {
        digitalWrite(firePin, HIGH);
        delay(FIRE_DURATION_MS);
        digitalWrite(firePin, LOW);

        Serial.println("IGNITION!");

        // Rising launch sound
        for (int f = 400; f < 2500; f += 15) {
          tone(buzzer, f);
          delay(6);
        }

        // LONG FINAL BLAST
        tone(buzzer, 600);
        delay(1500);

        noTone(buzzer);

        digitalWrite(yellowLED,  LOW);
        digitalWrite(yellowLED2, LOW);
        digitalWrite(redLED,     LOW);

        currentState = LAUNCHED;
      }

      break;
    }

    // ================= LAUNCHED =================
    case LAUNCHED: {

      bool blink = (millis() % 200) < 100;

      digitalWrite(greenLED,   blink);
      digitalWrite(yellowLED,  LOW);
      digitalWrite(yellowLED2, LOW);
      digitalWrite(redLED,     LOW);

      break;
    }

    // ================= ABORTED =================
    case ABORTED:

      digitalWrite(redLED,     (millis() % 300) < 150);
      digitalWrite(greenLED,   LOW);
      digitalWrite(yellowLED,  LOW);
      digitalWrite(yellowLED2, LOW);

      break;
  }
}
