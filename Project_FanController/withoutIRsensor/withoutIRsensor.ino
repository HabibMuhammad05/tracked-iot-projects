#define RLY_S1 D1
#define RLY_S2 D2
#define RLY_S3 D3
#define RLY_SWING D4

#define BUZZER D5

#define BTN_POWER D6
#define BTN_SPEED D7
#define BTN_SWING D8
#define BTN_RHYTHM D0
#define BTN_TIMER A0

bool power=false;
bool swing=false;
bool rhythm=false;

int speed=1;
int baseSpeed=1;

uint8_t timerHours=0;
unsigned long timerStart=0;

unsigned long lastRhythm=0;

void beep()
{
  digitalWrite(BUZZER,HIGH);
  delay(40);
  digitalWrite(BUZZER,LOW);
}

void allSpeedOff()
{
  digitalWrite(RLY_S1,LOW);
  digitalWrite(RLY_S2,LOW);
  digitalWrite(RLY_S3,LOW);
}

void setSpeed(int s)
{
  allSpeedOff();

  if(!power) return;

  switch(s)
  {
    case 1: digitalWrite(RLY_S1,HIGH); break;
    case 2: digitalWrite(RLY_S2,HIGH); break;
    case 3: digitalWrite(RLY_S3,HIGH); break;
  }
}

void togglePower()
{
  power=!power;
  beep();

  if(power)
    setSpeed(speed);
  else
    allSpeedOff();
}

void changeSpeed()
{
  if(!power) return;

  speed++;
  if(speed>3) speed=1;

  baseSpeed=speed;

  beep();
  setSpeed(speed);
}

void toggleSwing()
{
  swing=!swing;
  digitalWrite(RLY_SWING,swing);
  beep();
}

void toggleRhythm()
{
  rhythm=!rhythm;

  if(rhythm)
    baseSpeed=speed;

  beep();
}

void changeTimer()
{
  timerHours++;

  if(timerHours>15)
    timerHours=0;

  timerStart=millis();

  beep();
}

void setup()
{
  pinMode(RLY_S1,OUTPUT);
  pinMode(RLY_S2,OUTPUT);
  pinMode(RLY_S3,OUTPUT);
  pinMode(RLY_SWING,OUTPUT);

  pinMode(BUZZER,OUTPUT);

  pinMode(BTN_POWER,INPUT_PULLUP);
  pinMode(BTN_SPEED,INPUT_PULLUP);
  pinMode(BTN_SWING,INPUT_PULLUP);
  pinMode(BTN_RHYTHM,INPUT_PULLUP);
  pinMode(BTN_TIMER,INPUT_PULLUP);

  allSpeedOff();
}

void loop()
{
  static bool lp,ls,lsw,lr,lt;

  bool p=digitalRead(BTN_POWER);
  bool s=digitalRead(BTN_SPEED);
  bool sw=digitalRead(BTN_SWING);
  bool r=digitalRead(BTN_RHYTHM);
  bool t=digitalRead(BTN_TIMER);

  if(p==LOW && lp==HIGH) togglePower();
  if(s==LOW && ls==HIGH) changeSpeed();
  if(sw==LOW && lsw==HIGH) toggleSwing();
  if(r==LOW && lr==HIGH) toggleRhythm();
  if(t==LOW && lt==HIGH) changeTimer();

  lp=p;
  ls=s;
  lsw=sw;
  lr=r;
  lt=t;

  // TIMER
  if(timerHours>0 && power)
  {
    unsigned long duration=(unsigned long)timerHours*3600000;

    if(millis()-timerStart>duration)
    {
      power=false;
      allSpeedOff();
      timerHours=0;
    }
  }

  // RHYTHM MODE
  if(rhythm && power)
  {
    if(millis()-lastRhythm>5000)
    {
      int r=random(0,baseSpeed+1);

      if(r==0)
        allSpeedOff();
      else
        setSpeed(r);

      lastRhythm=millis();
    }
  }
}
