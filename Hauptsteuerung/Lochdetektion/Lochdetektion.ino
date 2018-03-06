const int entprellZyklen = 200;
int entprellZaehler[3][6] = {0};
uint8_t Sensorregister[] = {0,0,0};


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

void setup()
{
  starteAbtastTimer(4);
  initialisiereSensorpins();
  Serial.begin(9600);
}

void loop()
{
  pruefeLoecher();
  
  while( ! (TIFR3 & (1<<ICF4)) );
  TIFR3 = 1<<ICF4;
}
