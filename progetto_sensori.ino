/*
  Progetto Elegoo UNO R3
  - Sensore di umidità (DHT11) su pin digitale 2
  - Sensore ad ultrasuoni (HC-SR04) su pin 4 (Trig) e 5 (Echo)
  - Display LCD 16x2 su pin 7,8,9,10,11,12 (RS,E,D4,D5,D6,D7)

  Logica:
  - Se l'umidità è alta -> mostra "Ambiente umido"
  - Se qualcosa è troppo vicino -> mostra "Allontanarsi"
  - Se nessuna delle due condizioni si verifica -> mostra solo i valori misurati
  - Se entrambe le condizioni si verificano, i due messaggi vengono alternati
*/

#include <LiquidCrystal.h>
#include <DHT.h>

// ---------- Configurazione pin ----------
#define DHTPIN 2
#define DHTTYPE DHT11

#define TRIG_PIN 4
#define ECHO_PIN 5

// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
DHT dht(DHTPIN, DHTTYPE);

// ---------- Soglie (modificabili) ----------
const float SOGLIA_UMIDITA = 70.0;   // % oltre la quale l'ambiente è "umido"
const float SOGLIA_DISTANZA = 10.0;  // cm sotto la quale c'è qualcosa troppo vicino

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  dht.begin();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.setCursor(0, 0);
  lcd.print("Avvio sistema...");
  delay(1500);
  lcd.clear();
}

// Funzione per leggere la distanza dal sensore ad ultrasuoni
float leggiDistanza() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long durata = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
  if (durata == 0) {
    return -1; // nessuna lettura valida (fuori portata)
  }
  float distanza = durata * 0.034 / 2; // conversione in cm
  return distanza;
}

void loop() {
  float umidita = dht.readHumidity();
  float distanza = leggiDistanza();

  bool umiditaAlta = (!isnan(umidita)) && (umidita > SOGLIA_UMIDITA);
  bool troppoVicino = (distanza > 0) && (distanza < SOGLIA_DISTANZA);

  // --- Riga 1: valori misurati ---
  lcd.clear();
  lcd.setCursor(0, 0);
  if (isnan(umidita)) {
    lcd.print("Um:Err ");
  } else {
    lcd.print("Um:");
    lcd.print(umidita, 0);
    lcd.print("% ");
  }

  if (distanza < 0) {
    lcd.print("Di:--- ");
  } else {
    lcd.print("Di:");
    lcd.print(distanza, 0);
    lcd.print("cm");
  }

  // --- Riga 2: messaggi di stato ---
  lcd.setCursor(0, 1);
  if (umiditaAlta && troppoVicino) {
    // alterna i due messaggi ogni 1.5 secondi
    lcd.print("Ambiente umido");
    Serial.println("Ambiente umido / Allontanarsi");
    delay(1500);
    lcd.setCursor(0, 1);
    lcd.print("Allontanarsi   ");
    delay(1500);
  } else if (umiditaAlta) {
    lcd.print("Ambiente umido  ");
    Serial.println("Ambiente umido");
    delay(1500);
  } else if (troppoVicino) {
    lcd.print("Allontanarsi    ");
    Serial.println("Allontanarsi");
    delay(1500);
  } else {
    lcd.print("Tutto OK        ");
    Serial.println("Nessuna condizione critica");
    delay(1500);
  }
}
