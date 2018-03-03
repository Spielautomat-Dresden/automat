// Step-Pin (mit allen Motortreibern verbunden) mit PWM Signal belegt
const int pinSchritt = 6;
// Enable-Pins (jeweils mit Motortreibern verbunden)
const int pinsEnable[] = {44,45,46};
// Periodendauer der Schrittmotoren bei Vorwaertsgang in ms
const int schrittPeriode = 0;
// Pins der Sensoren. Reihenfolge beachten
const int pinsTische[3][6] = {{41,39,37,35,33,31}, {52,50,48,52,50,42},{53,51,49,47,45,43}};

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
  OCR5A = 0x0001; // Output Compare Register
  OCR5B = 0x0001;
  OCR5C = 0x0001;
  TCNT5 = 0x0000; // Timer Counter Register: Zähler auf Null setzen
}

void addiereSchritte(int anzahlSchritte)
{
  int aktuellerCounterWert; 
  if(TIFR5 & (1<<OCF5A))
  {
    // Compare Match Flag ist high -> ~Enable High -> Motor steht
    aktuellerCounterWert = TCNT5;
    OCR5A = anzahlSchritte + aktuellerCounterWert;
    // Motor aktivieren
    TCCR5A &= ~ (1 << COM5A0); // Clear on compare match
    TCCR5C |= (1<<FOC5A); // Force output compare
    TCCR5A |= (1<<COM5A1) | (1<<COM5A0); // Set on compare match
    TIFR5 |= (1<<OCF5A); //Output Compare Match Flag
  }
  else
  {
    // Compare Match Flag ist low -> ~Enable Loq -> Motor läuft bereits
    OCR5A += anzahlSchritte;
  }
}

void setup()
{
  startePWM_Pin6(1000);
  startePWMcounter();
  pinMode(pinSchritt,OUTPUT);
  for(int pin = 0; pin <= 2; pin++)
  {
    pinMode(pinsEnable[pin],OUTPUT);

    for(int loch = 0; loch <=5; loch++)
    {
      pinMode(pinsTische[pin][loch],INPUT);
    }
  }  
}
void loop()
{
  delay(3000);
  addiereSchritte(1);
  delay(2000);
  addiereSchritte(2);
  addiereSchritte(1);
  delay(5000);
}
