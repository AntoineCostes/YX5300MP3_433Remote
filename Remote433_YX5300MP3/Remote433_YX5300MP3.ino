#include <MD_YX5300.h>
#include <HardwareSerial.h>

#define SERIAL_DEBUG 1
#include "MP3Player.h"

#include <EasyButton.h>
#define BUTTON_A 2
#define BUTTON_B 4 
#define BUTTON_C 3
#define BUTTON_D 5
#define BUTTON_ANY 6

EasyButton btnA(BUTTON_A);
EasyButton btnB(BUTTON_B);
EasyButton btnC(BUTTON_C);
EasyButton btnD(BUTTON_D);
EasyButton btnANY(BUTTON_ANY);

bool volumeChanged;

void pressedA()
{
  if(SERIAL_DEBUG) Serial.println("A previous");
  if (playing) stop();
  else playPrevious();
}

void pressedB()
{
  if(SERIAL_DEBUG) Serial.println("B next");
  if (playing) stop();
  else playNext();
}

void pressedC()
{
  if(SERIAL_DEBUG) Serial.println("C: vol down");
}

void pressedD()
{
  if(SERIAL_DEBUG) Serial.println("D: vol up");
}

void pressedANY()
{
  if(SERIAL_DEBUG) Serial.println("ANY");
}

void setup()
{
  if(SERIAL_DEBUG) Serial.begin(115200);
  if(SERIAL_DEBUG) Serial.println("start");

  initMP3();
  
  for (int i = 2; i < 7; i++) pinMode(i, INPUT);

  btnA.begin();
  btnB.begin();
  btnC.begin();
  btnD.begin();
  btnANY.begin();
  
  btnA.onPressed(pressedA);
  btnB.onPressed(pressedB);
  btnC.onPressed(pressedC);
  btnD.onPressed(pressedD);
  btnANY.onPressed(pressedANY);
}

void updateVolume()
{
  if (digitalRead(BUTTON_C))
  {
    targetVolume -= 0.01f;
    targetVolume = max(0.1f, targetVolume);
    volumeChanged = true;
  }
  else if (digitalRead(BUTTON_D))
  {
    targetVolume += 0.01f;
    targetVolume = min(1.0f, targetVolume);
    volumeChanged = true;
  }
  if (volumeChanged && !digitalRead(BUTTON_C) && !digitalRead(BUTTON_D))
  {
    setVolume(targetVolume);
    volumeChanged = false;
  }
}

void loop()
{
  updateMP3();

  btnA.read();
  btnB.read();
  btnC.read(); // not used
  btnD.read(); // not used
  btnANY.read(); // not used

  updateVolume();
  
  delay(10);
}
