#include <LiquidCrystal.h>

// LCD
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Sensore ultrasuoni
const int trigPin = 6;
const int echoPin = 5;

// LED RGB
const int pinR = 2;
const int pinG = 3;
const int pinB = 4;

// Buzzer passivo
const int buzzerPin = 13;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pinR, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  lcd.begin(16, 2);
  Serial.begin(9600);
}

long leggiDistanza() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long durata = pulseIn(echoPin, HIGH, 30000);

  if (durata == 0) {
    return -1;
  }

  long distanza = durata * 0.034 / 2;
  return distanza;
}

void accendiColore(bool r, bool g, bool b) {
  digitalWrite(pinR, r);
  digitalWrite(pinG, g);
  digitalWrite(pinB, b);
}

void loop() {
  long distanza = leggiDistanza();
  Serial.print("Distanza: ");
  Serial.println(distanza);

  lcd.clear();

  if (distanza == -1 || distanza > 20) {
    // Nessun dato -> BLU, buzzer spento
    accendiColore(LOW, HIGH, LOW);
    noTone(buzzerPin);
    lcd.setCursor(0, 0);
    lcd.print("NESSUN DATO");
    lcd.setCursor(0, 1);
    lcd.print("RILEVATO");
  }
  else if (distanza <= 5) {
    // TROPPO VICINO -> ROSSO, buzzer allarme
    accendiColore(HIGH, LOW, LOW);
    lcd.setCursor(0, 0);
    lcd.print("TROPPO VICINO");

    tone(buzzerPin, 1000);   // frequenza 1000 Hz
    delay(150);
    noTone(buzzerPin);
    delay(100);
    return; // salta il delay finale, già fatto sopra
  }
  else {
    // Distanza normale -> VERDE, buzzer spento
    accendiColore(LOW, LOW, HIGH);
    noTone(buzzerPin);
    lcd.setCursor(0, 0);
    lcd.print("Distanza:");
    lcd.setCursor(0, 1);
    lcd.print(distanza);
    lcd.print(" cm");
  }

  delay(400);
}

