/*-------PIN-Definitionen-----------------------------------------------------------------*/

// Step-Pin (mit allen Motortreibern verbunden) mit PWM Signal belegt
const int pinSchritt = 6;
// Enable-Pins (jeweils mit Motortreibern verbunden)
const int pinsEnable[] = {44,45,46};
// Periodendauer der Schrittmotoren bei Vorwaertsgang in ms
const int schrittPeriode = 0;
// Pins der Sensoren. Reihenfolge beachten
const int pinsTische[3][6] = {{41,39,37,35,33,31}, {52,50,48,52,50,42},{53,51,49,47,45,43}};


const int entprellZyklen = 200;
int entprellZaehler[3][6] = {0};
uint8_t Sensorregister[] = {0,0,0};


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

/*
Startet Timer 3. Dieser dient dem Einstellen einer konstanten Abtastrate.
*/
void starteAbtastTimer(int abtastPeriode_ms)
{
  TCCR3A = 0x0000; // Keine Pins verbunden
  TCCR3B = (1 << WGM33) | (1 << WGM32); // CTC Mode mit ICR3 als Top
  TCCR3B |= (1 << CS30); // Prescaler 1 -> ohne Prescaler
  int prescaler = 1;
  ICR3 = abtastPeriode_ms*((F_CPU)/prescaler)/1000; // Input Capture Register
}

void initialisiereSensorpins(void)
{
  DDRA &= 0xC0; // Pins als Input
  DDRF &= 0xC0;
  DDRK &= 0xC0;
  PORTA &= 0xC0;
  PORTF &= 0xC0;
  PORTK &= 0xC0;
  
  PORTA = 0xFF; //TODO !!!!!!!!!!!!! NUR ZUM TESTEN: PULLUP AKTIVIERT; später Zeile löschen !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  PORTF = 0xFF;
  PORTK = 0xFF;
}

void pruefeLoecher(void)
{
  Sensorregister[0] = (~PINA) & 0x3F; // eventuell an Sensorlogik anpassen
  Sensorregister[1] = (~PINF) & 0x3F;
  Sensorregister[2] = (~PINK) & 0x3F;
  
  for(int tisch = 0; tisch <= 2; tisch++)
  {
    for(int loch = 0; loch <=5; loch++)
    {
      if(Sensorregister[tisch] & (1 << loch))
      {
        if(entprellZaehler[tisch][loch] <= 0)
        {
          // Loch erstmalig detektiert
          entprellZaehler[tisch][loch] = entprellZyklen;
          Serial.print(loch);Serial.print(" "); 
          Serial.println(tisch);
        }
      }
      else if(entprellZaehler[tisch][loch] > 0)
      {
        // Loch wurde in den letzten Entprellzyklen getroffen
        entprellZaehler[tisch][loch] -=1;
      }      
    }
  }
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
  starteAbtastTimer(4);
  
  initialisiereSensorpins();
  pinMode(pinSchritt,OUTPUT);
  for(int pin = 0; pin <= 2; pin++)
  {
    pinMode(pinsEnable[pin],OUTPUT);

    for(int loch = 0; loch <=5; loch++)
    {
      pinMode(pinsTische[pin][loch],INPUT);
    }
  }  
  
  //Serial.begin(9600);
}

void loop(()
{
  //Rueckweg
  
  //Hinweg
  
  while()
  {
    pruefeLoecher();
    while( ! (TIFR3 & (1<<ICF4)) );
    TIFR3 = 1<<ICF4;
  }
