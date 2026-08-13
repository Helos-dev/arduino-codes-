/*
  Progetto Elegoo UNO R3 (senza libreria DHT.h)
  - Sensore di umidita (DHT11) su pin digitale 2  -> letto manualmente (bit-banging)
  - Sensore ad ultrasuoni (HC-SR04) su pin 4 (Trig) e 5 (Echo)
  - Display LCD 16x2 su pin 7,8,9,10,11,12 (RS,E,D4,D5,D6,D7)

  Logica:
  - Se l'umidita e alta -> mostra "Ambiente umido"
  - Se qualcosa e troppo vicino -> mostra "Allontanarsi"
  - Se nessuna delle due condizioni si verifica -> mostra solo i valori misurati
  - Se entrambe le condizioni si verificano, i due messaggi vengono alternati

  NOTA: serve solo la libreria LiquidCrystal (inclusa di default nell'IDE Arduino)
*/

#include <LiquidCrystal.h>

// ---------- Configurazione pin ----------
#define DHT_PIN 2

#define TRIG_PIN 4
#define ECHO_PIN 5

// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// ---------- Soglie (modificabili) ----------
const float SOGLIA_UMIDITA = 70.0;   // % oltre la quale l'ambiente e "umido"
const float SOGLIA_DISTANZA = 10.0;  // cm sotto la quale c'e qualcosa troppo vicino

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.setCursor(0, 0);
  lcd.print("Avvio sistema...");
  delay(1500);
  lcd.clear();
}

// ---------- Lettura manuale DHT11 ----------
// Ritorna true se la lettura ha successo, salva umidita e temperatura nei puntatori
bool leggiDHT11(float &umidita, float &temperatura) {
  byte dati[5] = {0, 0, 0, 0, 0};

  // 1) Segnale di start: pin basso per almeno 18ms
  pinMode(DHT_PIN, OUTPUT);
  digitalWrite(DHT_PIN, LOW);
  delay(18);
  digitalWrite(DHT_PIN, HIGH);
  delayMicroseconds(30);
  pinMode(DHT_PIN, INPUT_PULLUP);

  // 2) Attendo risposta del sensore (basso ~80us, poi alto ~80us)
  unsigned long timeout = micros();
  while (digitalRead(DHT_PIN) == HIGH) {
    if (micros() - timeout > 100) return false; // nessuna risposta
  }
  timeout = micros();
  while (digitalRead(DHT_PIN) == LOW) {
    if (micros() - timeout > 100) return false;
  }
  timeout = micros();
  while (digitalRead(DHT_PIN) == HIGH) {
    if (micros() - timeout > 100) return false;
  }

  // 3) Lettura dei 40 bit (5 byte)
  for (int i = 0; i < 40; i++) {
    // ogni bit inizia con ~50us bassi
    timeout = micros();
    while (digitalRead(DHT_PIN) == LOW) {
      if (micros() - timeout > 100) return false;
    }

    // la durata del livello alto determina 0 o 1
    unsigned long inizioAlto = micros();
    while (digitalRead(DHT_PIN) == HIGH) {
      if (micros() - inizioAlto > 200) return false;
    }
    unsigned long durataAlto = micros() - inizioAlto;

    byte indiceByte = i / 8;
    dati[indiceByte] <<= 1;
    if (durataAlto > 40) { // ~26-28us = 0, ~70us = 1 -> soglia a 40us
      dati[indiceByte] |= 1;
    }
  }

  // 4) Controllo checksum
  byte checksum = dati[0] + dati[1] + dati[2] + dati[3];
  if (checksum != dati[4]) {
    return false; // dato corrotto
  }

  // DHT11: dati[0] = umidita intera, dati[2] = temperatura intera
  umidita = dati[0];
  temperatura = dati[2];
  return true;
}

// ---------- Lettura distanza HC-SR04 ----------
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
  float umidita = 0, temperatura = 0;
  bool letturaOk = leggiDHT11(umidita, temperatura);
  float distanza = leggiDistanza();

  bool umiditaAlta = letturaOk && (umidita > SOGLIA_UMIDITA);
  bool troppoVicino = (distanza > 0) && (distanza < SOGLIA_DISTANZA);

  lcd.clear();

  // --- Riga 1: umidita e temperatura ---
  lcd.setCursor(0, 0);
  if (!letturaOk) {
    lcd.print("U:Err T:Err");
  } else {
    lcd.print("U:");
    lcd.print(umidita, 0);
    lcd.print("% T:");
    lcd.print(temperatura, 0);
    lcd.print("C");
  }

  // --- Riga 2: distanza (+ eventuali messaggi di stato) ---
  lcd.setCursor(0, 1);
  if (distanza < 0) {
    lcd.print("D:---cm");
  } else {
    lcd.print("D:");
    lcd.print(distanza, 0);
    lcd.print("cm");
  }

  Serial.print("Umidita: "); Serial.print(umidita);
  Serial.print("% Temp: "); Serial.print(temperatura);
  Serial.print("C Distanza: "); Serial.print(distanza);
  Serial.println("cm");

  if (umiditaAlta || troppoVicino) {
    delay(1500); // mostra prima i valori, poi il messaggio

    if (umiditaAlta) {
      lcd.setCursor(0, 1);
      lcd.print("Ambiente umido  ");
      Serial.println("Ambiente umido");
      delay(1500);
    }

    if (troppoVicino) {
      lcd.setCursor(0, 1);
      lcd.print("Allontanarsi    ");
      Serial.println("Allontanarsi");
      delay(1500);
    }
  } else {
    delay(1500);
  }
}
