unit8_t Sensorregister[] = {0,0,0};

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
  
  PORTF = 0xFF; //TODO !!!!!!!!!!!!! NUR ZUM TESTEN !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void pruefeLoecher(void)
{
  Sensorregister[0] = PINA
  Sensorregister[1] = PINF
  Sensorregister[1] = PINK
  if( ! Sensorregister[2]) // TODO Eventuell Sensorlogik anpassen
  {
    // Mindestens ein Loch in Channel F getroffen
    
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
