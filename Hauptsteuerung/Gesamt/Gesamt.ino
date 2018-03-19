/*-------PIN-Definitionen-----------------------------------------------------------------*/

// Step-Pin (mit allen Motortreibern verbunden) mit PWM Signal belegt
const int pinSchritt = 6;
// Enable-Pins (jeweils mit Motortreibern verbunden)
const int pinsEnable[] = {44,45,46};
// Periodendauer der Schrittmotoren bei Vorwaertsgang in ms
const int schrittPeriode = 1000;
// Periodendauer der Sensorabfrageperiode
const int abtastPeriode = 4;
const int pinRichtung = 11;
const int pinEndlage[] = {5,10,2};


const int entprellZyklen = 200;
int entprellZaehler[3][6] = {0};
uint8_t Sensorregister[] = {0,0,0};
const int schrittMaximum = 20; // Maximale Schrittanzahl bis Tischende
int schrittKonto[3] = {schrittMaximum};
const int lochBonus[] = {3,3,3,6,6,10};
boolean spielBeendet = true;
int siegerTisch = 3;


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
          setzeSchritte(tisch, loch);
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

void setzeSchritte(int tisch, int loch)
{
  int schrittDifferenz = schrittMaximum - schrittKonto[tisch];
  
  if(schrittDifferenz > lochBonus[loch])
  {
    // Normalfall
    addiereSchritte(lochBonus[loch], tisch);
    schrittKonto[tisch] += lochBonus[loch];
  }
  else
  {
    // Dieser Tisch ist Sieger
    addiereSchritte(schrittDifferenz, tisch);
    siegerTisch = tisch;
    spielBeendet = true;
    schrittKonto[tisch] += schrittDifferenz;
  }
  
  Serial.println(schrittKonto[tisch]);
}
    

void addiereSchritte(int anzahlSchritte, int tisch)
{
  Serial.println("addiere Schritte");Serial.println(anzahlSchritte);Serial.println(tisch);
  // TODO: tisch beachten!!!
  // Beachten Tischreihenfolge "umgekehrt": Tisch 0 an Channel C
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

void rueckfahrt()
{
  Serial.println("Rückwärtsfahrt");
  digitalWrite(pinRichtung, HIGH);
  
  for(int tisch; tisch <= 2; tisch ++)
  {
    addiereSchritte(schrittKonto[tisch]+50, tisch);
  }
  while((TIFR5 & 0x0E) != 0x0E)
  {
    for(int tisch; tisch <= 2; tisch ++)
    {
      if(digitalRead(pinEndlage[tisch]) == 0)
      {
        Serial.println("Tisch ist angekommen");
        // Figur dieses Tisches ist an Endlage angekommen
        TCCR5C |= (0x80 >> tisch); // Force output compare
      }
    }
  }
  digitalWrite(pinRichtung, LOW);
}

void setup()
{
  startePWM_Pin6(schrittPeriode);
  startePWMcounter();
  starteAbtastTimer(abtastPeriode);
  
  initialisiereSensorpins();
  pinMode(pinSchritt,OUTPUT);
  pinMode(pinRichtung,OUTPUT);
  for(int tisch = 0; tisch <= 2; tisch++)
  {
    pinMode(pinsEnable[tisch],OUTPUT);
    pinMode(pinEndlage[tisch], INPUT_PULLUP);
  }
  
  
  Serial.begin(9600);
}


void loop()
{
  //Rueckweg
  rueckfahrt();
  
  spielBeendet = false;
  for(int tisch; tisch <= 2; tisch ++)
  {
    schrittKonto[tisch] = 0;
  }
  
  
  //Hinweg
  startePWMcounter();
  while(! spielBeendet)
  {
    pruefeLoecher();
    while( ! (TIFR3 & (1<<ICF4)) );
    TIFR3 = 1<<ICF4;
  }
  Serial.print("Sieger: ");Serial.println(siegerTisch);
  while(! (TIFR5 & (0x2 << siegerTisch))){} // Warte solange bis Sieger angekommen ist
  Serial.println(TIFR5);
  TCCR5C |= 0xE0; // Motoren deaktivieren  
}
