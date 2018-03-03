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
  OCR5A = 0x0000; // Output Compare Register
  TCCR5C |= (1<<FOC5A) | (1<<FOC5B) | (1<<FOC5C);
  TCNT5 = 0x0000;
}

void addiereSchritte(int anzahlSchritte)
{
  OCR5A = anzahlSchritte;
  // Motor aktivieren bzw Licht anschalten
  TCCR5A &= ~ (1 << COM5A0); // Clear on compare match
  TCCR5C |= (1<<FOC5A); // Force output compare
  TCCR5A |= (1<<COM5A1) | (1<<COM5A0); // Set on compare match
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
  delay(1000);
  addiereSchritte(5);
  delay(10000);
}
