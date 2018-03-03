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

void setup()
{
  starteAbtastTimer(4);
  Serial.begin(9600);
}

void loop()
{
  
  
  
  while( ! (TIFR3 & (1<<ICF4)) );
  TIFR3 = 1<<ICF4;
}
