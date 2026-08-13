/*
  Elegoo Super Starter Kit
  ------------------------
  Joystick -> muove un microservo
  Sul servo e' montato un sensore ad ultrasuoni (HC-SR04)
  La distanza misurata viene mostrata su un LCD 16x2
  Se un oggetto e' entro 10 cm, il display avvisa "Allontanarsi!"

  COLLEGAMENTI:
  - Joystick:  SW -> D2   |  X -> A0   |  Y -> A1
  - Servo:     segnale -> D6
  - HC-SR04:   ECHO -> D3 |  TRIG -> D5
  - LCD 16x2:  RS->7  EN->8  D4->9  D5->10  D6->11  D7->12
*/

#include <Servo.h>
#include <LiquidCrystal.h>

// ---------- Pin ----------
const int PIN_JOY_SW  = 2;
const int PIN_JOY_X   = A0;
const int PIN_JOY_Y   = A1;

const int PIN_SERVO   = 6;

const int PIN_ECHO    = 3;
const int PIN_TRIG    = 5;

const int PIN_LCD_RS  = 7;
const int PIN_LCD_EN  = 8;
const int PIN_LCD_D4  = 9;
const int PIN_LCD_D5  = 10;
const int PIN_LCD_D6  = 11;
const int PIN_LCD_D7  = 12;

// ---------- Oggetti ----------
Servo microServo;
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

// ---------- Parametri ----------
const int SOGLIA_CM   = 10;     // distanza di allarme
const int DEADZONE    = 60;     // zona morta joystick (centro ~512)
int angoloServo       = 90;     // posizione iniziale del servo

unsigned long ultimoAggiornamentoLCD = 0;
const unsigned long INTERVALLO_LCD = 200; // ms

void setup() {
  Serial.begin(9600);

  pinMode(PIN_JOY_SW, INPUT_PULLUP);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_TRIG, OUTPUT);

  microServo.attach(PIN_SERVO);
  microServo.write(angoloServo);

  lcd.begin(16, 2);
  lcd.print("Distanza:");
}

// Legge il joystick e muove il servo di conseguenza
void aggiornaServoDaJoystick() {
  int valX = analogRead(PIN_JOY_X); // 0-1023, centro ~512

  // Se il joystick e' spostato oltre la deadzone, muovi il servo
  if (valX > 512 + DEADZONE) {
    angoloServo += 2;
  } else if (valX < 512 - DEADZONE) {
    angoloServo -= 2;
  }

  angoloServo = constrain(angoloServo, 0, 180);
  microServo.write(angoloServo);
}

// Misura la distanza in cm con l'HC-SR04
long misuraDistanzaCM() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // timeout 25ms (~4m) per evitare blocchi se non torna eco
  long durata = pulseIn(PIN_ECHO, HIGH, 25000);
  if (durata == 0) return -1; // nessuna lettura valida

  long distanza = durata * 0.0343 / 2; // velocita' suono / 2 (andata e ritorno)
  return distanza;
}

void aggiornaLCD(long distanza) {
  lcd.setCursor(0, 0);
  lcd.print("Distanza:       ");
  lcd.setCursor(10, 0);
  if (distanza < 0) {
    lcd.print("---");
  } else {
    lcd.print(distanza);
    lcd.print("cm ");
  }

  lcd.setCursor(0, 1);
  if (distanza > 0 && distanza <= SOGLIA_CM) {
    lcd.print("Allontanarsi!   ");
  } else {
    lcd.print("                "); // pulisce la riga
  }
}

void loop() {
  aggiornaServoDaJoystick();

  long distanza = misuraDistanzaCM();

  // Aggiorna il display ogni INTERVALLO_LCD ms (evita sfarfallio)
  if (millis() - ultimoAggiornamentoLCD >= INTERVALLO_LCD) {
    aggiornaLCD(distanza);
    ultimoAggiornamentoLCD = millis();
  }

  // Debug su Serial Monitor (opzionale)
  Serial.print("Angolo servo: ");
  Serial.print(angoloServo);
  Serial.print("  Distanza: ");
  Serial.println(distanza);

  delay(20);
}
