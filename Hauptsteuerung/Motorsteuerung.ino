// Step-Pin (mit allen Motortreibern verbunden) mit PWM Signal belegt
const int pinSchritt = 6;
// Enable-Pins (jeweils mit Motortreibern verbunden)
const int pinsEnable[] = {44,45,46};
// Periodendauer der Schrittmotoren bei Vorwaertsgang in ms
const int schrittPeriode = 0;
// Pins der Sensoren. Reihenfolge beachten
const int pinsTische[3][6] = {{41,39,37,35,33,31}, {52,50,48,52,50,42},{53,51,49,47,45,43}};
// Register der Enable-Pins
const volatile uint16_t enableRegister = OCR5A;

/*
Startet ein PWM Signal mit einer gegebenen Periodendauer an Pin 6.
Möglich sind Werte zwischen 1 und 3355 ms.
*/
int startePWM_Pin6(int periodendauer_ms)
{
  TCCR4A = (1<<COM4A1); // Pin 6 verbinden, non-inverting
  TCCR4A |= (1<<WGM41); // Fast PWM Teil 1 (WGM4 = 14, letzte beiden Bits in TCCR4A)
  TCCR4B = (1<<WGM42) | (1<<WGM43); // Fast PWM Teil 2 (erste beiden Bits)
  TCCR4B |= (0b101<<CS40); //Prescaler 1024
  int prescaler = 1024;
  ICR4 = periodendauer_ms*((F_CPU)/prescaler)/1000; // Input Capture Register
  OCR4A = ICR4/2; // Output Compare Register
  return 1;
}

/*
Startet Timer 5 als Counter des PWM Signals an Pin 6. Dafür wird der Timer als externe Clocksource genutzt
Pin 6 muss dafür mit Pin 47 verbunden sein
*/
int startePWMcounter(void)
{
  TCCR5A = (1<<COM5A1) | (1<<COM5A0) | (1<<COM5B1) | (1<<COM5B0) | (1<<COM5C1) | (1<<COM5C0); // Pins 46, 45 und 44 als Enable-Pins verbinden, non-inverting
  TCCR5A &= ~ ((1<<WGM41) | (1<<WGM40)); // Fast PWM Teil 1 (WGM4 = 14, letzte beiden Bits in TCCR4A)
  TCCR5B = (0b110<<CS50); // External clock source (falling edge)
  ICR5 = 0xFFFF; // Input Capture Register maximal
  OCR5A = 0x0003; // Output Compare Register
  OCR5B = 0x0000;
  OCR5C = 0x0000;
  TCNT5 = 0x0000;
  TIFR5 = (1<<OCF5A) | (1<<OCF5B) | (1<<OCF5C); // Compare Match Flags 1 setzen (nötig für ersten Aufruf von addiereSchritte()
  //TCCR5C |= (1<<FOC5A) | (1<<FOC5B) | (1<<FOC5C); // Setzt Enable-Pins HIGH -> deaktivieren der Motoren
  Serial.println(OCR5A);
  return 1;
}

/*
Funktion lässt den Motor des entsprechenen Tischs die vorgegebene Anzahl an Schritten laufen
indem das Register OCR5 des PWMcounters (Timer 5) neu beschrieben wird.
*/
int addiereSchritte(int tisch, int schrittAnzahl)
{         
  switch(tisch)
  {
    case 0: Serial.println("Addiere Schritte");
            Serial.println((1<<OCF5A));
            Serial.println(TIFR5);
            Serial.println((TIFR5 & (1<<OCF5A)));
            if(TIFR5 > OCR5A)
            {
              // Motoren bisher deaktiviert
              Serial.println("Hallo");
              TCCR5C |= (1<<FOC5A);
              OCR5A = TCNT5 + schrittAnzahl;
              Serial.println(TCCR5C);
            }
            else
              // Motoren laufen schon
              OCR5A += schrittAnzahl;
            Serial.println(OCR5A);
    break;
    case 1: if(TIFR5 & (0<<OCF5B))
              OCR5B = TCNT5 + schrittAnzahl;
            else
              OCR5B += schrittAnzahl;
    break;
    case 2: if(TIFR5 & (0<<OCF5C))
              OCR5C = TCNT5 + schrittAnzahl;
            else
              OCR5C += schrittAnzahl;
    break;
  }
  return 1;
}

void setup()
{
  Serial.begin(9600);
  /* Festlegen der Pin-Modi */
  pinMode(pinSchritt,OUTPUT);
  for(int pin = 0; pin <= 2; pin++)
  {
    pinMode(pinsEnable[pin],OUTPUT);

    for(int loch = 0; loch <=5; loch++)
    {
      pinMode(pinsTische[pin][loch],INPUT);
    }
  }
  
  /* Einstellen des MotorsteuerungsPWM-Signals */
  startePWM_Pin6(1000);
  
  startePWMcounter();
}
void loop()
{
//  Serial.print("Zaehler: ");Serial.println(TCNT5);
//  Serial.print("Compare: ");Serial.println(OCR5A);
  delay(5000);
  TCCR5C = 0;
  /*
  while(1)
  {
    
    //Ein
    delay(5000);
    //Aus
    Serial.println(TIFR5);
    TIFR5 = 3;
    OCR5A = 3;
    Serial.println(TIFR5);
    TCCR5A = 0x00;
    pinMode(46,OUTPUT);
    digitalWrite(46,LOW);
    //TCCR5C &= ~(1<<FOC5A);
    delay(4000);
    
    TCCR5A = 0xF0;

    
    delay(1000000);
  }
  */
}
