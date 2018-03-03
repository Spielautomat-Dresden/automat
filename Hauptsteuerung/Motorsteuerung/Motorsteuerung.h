#ifndef Motorsteuerung_h
#define Motorsteuerung_h

#include "Arduino.h"
#include "Motorsteuerung.cpp"


class Motorsteuerung
{
public:
    Motorsteuerung(int schrittPeriode);
    int startePWM_Pin6(int periodendauer_ms);
    int startePWMcounter(void);
    void addiereSchritte(int anzahlSchritte);
private:
    const int pinSchritt;
    const int pinsEnable[] = {44,45,46};
// Periodendauer der Schrittmotoren bei Vorwaertsgang in ms
const int schrittPeriode = 0;


#endif
