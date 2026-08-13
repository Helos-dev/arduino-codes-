/*
  Sistema di controllo accessi con sensore ad ultrasuoni
  - Distanza tra 2 e 10 cm  -> Accesso autorizzato (LED verde, servo a 90°)
  - Distanza inferiore a 2 cm -> Accesso negato (LED rosso, buzzer, servo fermo)
  - Distanza oltre 10 cm -> Stato di riposo (tutto spento, servo a 0°)

  Elegoo Super Starter Kit - Arduino UNO
  NB: buzzer PASSIVO -> serve tone()/noTone(), non digitalWrite()
*/

#include <LiquidCrystal.h>
#include <Servo.h>

// --- Pin sensore ultrasuoni ---
const int trigPin = 2;
const int echoPin = 3;

// --- Pin LED ---
const int ledVerde = 4;
const int ledRosso = 5;

// --- Pin servo ---
const int servoPin = 6;
Servo mioServo;

// --- Pin LCD (RS, E, D4, D5, D6, D7) ---
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// --- Pin buzzer passivo ---
const int buzzerPin = 13;
const int buzzerFrequenza = 1000; // Hz, tono dell'allarme

// Variabile per ricordare lo stato precedente (evita di riscrivere l'LCD ogni ciclo)
int statoPrecedente = -1; // -1 = nessuno, 0 = riposo, 1 = autorizzato, 2 = negato

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledRosso, OUTPUT);
  pinMode(buzzerPin, OUTPUT); // tone() lo gestisce comunque, ma non fa male

  mioServo.attach(servoPin);
  mioServo.write(0); // posizione di riposo

  lcd.begin(16, 2);
  lcd.print("Sistema pronto");

  Serial.begin(9600);
  delay(1000);
  lcd.clear();
}

long leggiDistanza() {
  // Invia impulso di trigger
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Legge il tempo di ritorno dell'eco
  long durata = pulseIn(echoPin, HIGH, 30000); // timeout 30ms per evitare blocchi

  // Calcola la distanza in cm (velocità del suono 0.034 cm/us, andata e ritorno /2)
  long distanza = durata * 0.034 / 2;

  return distanza;
}

void loop() {
  long distanza = leggiDistanza();

  Serial.print("Distanza: ");
  Serial.print(distanza);
  Serial.println(" cm");

  if (distanza > 0 && distanza < 2) {
    // --- ACCESSO NEGATO ---
    if (statoPrecedente != 2) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Accesso negato");
      statoPrecedente = 2;
    }
    digitalWrite(ledRosso, HIGH);
    digitalWrite(ledVerde, LOW);
    tone(buzzerPin, buzzerFrequenza); // avvia il tono del buzzer passivo
    mioServo.write(0); // il servo resta fermo/chiuso

  } else if (distanza >= 2 && distanza <= 10) {
    // --- ACCESSO AUTORIZZATO ---
    if (statoPrecedente != 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Accesso");
      lcd.setCursor(0, 1);
      lcd.print("autorizzato");
      statoPrecedente = 1;
    }
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledRosso, LOW);
    noTone(buzzerPin); // spegne il tono del buzzer passivo
    mioServo.write(90); // apre di 90 gradi

  } else {
    // --- NESSUN OGGETTO RILEVATO / FUORI RANGE ---
    if (statoPrecedente != 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("In attesa...");
      statoPrecedente = 0;
    }
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledRosso, LOW);
    noTone(buzzerPin); // spegne il tono del buzzer passivo
    mioServo.write(0);
  }

  delay(200); // piccola pausa per stabilizzare le letture
}
