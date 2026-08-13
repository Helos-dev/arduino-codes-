/*
  Progetto: Motore controllato da Joystick con display LCD
  Kit: Elegoo Super Starter Kit UNO R3

  Funzionamento:
  - Il joystick (asse X) controlla la direzione del motore
  - Spingere a destra  -> motore gira in senso ORARIO
  - Spingere a sinistra -> motore gira in senso ANTIORARIO
  - Al centro           -> motore fermo
  - La velocità aumenta più il joystick viene spinto ai lati
  - Il display LCD 1602 mostra lo stato attuale e la velocità
*/

#include <LiquidCrystal.h>

// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

// Pin driver motore L293D (ora sui pin 2-5)
const int motorEnable = 5;  // PWM - controllo velocità
const int motorIn1 = 2;
const int motorIn2 = 3;

// Pin joystick
const int joyX = A0;

const int centro = 512;   // valore centrale del joystick (0-1023)
const int soglia = 100;   // zona morta per evitare falsi movimenti

String statoPrecedente = "";

void setup() {
  lcd.begin(16, 2);

  pinMode(motorEnable, OUTPUT);
  pinMode(motorIn1, OUTPUT);
  pinMode(motorIn2, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("Motore fermo");
}

void loop() {
  int valX = analogRead(joyX);

  // Calcola la velocità in base a quanto è spinto il joystick
  int velocita = map(abs(valX - centro), 0, 512, 0, 255);
  velocita = constrain(velocita, 0, 255);

  if (valX > centro + soglia) {
    // Senso ORARIO
    digitalWrite(motorIn1, HIGH);
    digitalWrite(motorIn2, LOW);
    analogWrite(motorEnable, velocita);
    aggiornaDisplay("Senso ORARIO", velocita);
  }
  else if (valX < centro - soglia) {
    // Senso ANTIORARIO
    digitalWrite(motorIn1, LOW);
    digitalWrite(motorIn2, HIGH);
    analogWrite(motorEnable, velocita);
    aggiornaDisplay("ANTIORARIO", velocita);
  }
  else {
    // Fermo
    digitalWrite(motorIn1, LOW);
    digitalWrite(motorIn2, LOW);
    analogWrite(motorEnable, 0);
    aggiornaDisplay("Motore fermo", 0);
  }

  delay(100);
}

void aggiornaDisplay(String stato, int vel) {
  // Aggiorna la prima riga solo se lo stato è cambiato (evita sfarfallio)
  if (stato != statoPrecedente) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(stato);
    statoPrecedente = stato;
  }

  lcd.setCursor(0, 1);
  lcd.print("Vel: ");
  lcd.print(vel);
  lcd.print("    "); // pulisce residui di caratteri precedenti
}
