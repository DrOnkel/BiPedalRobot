


void ResetMotorz(void);  // This subroutine Reset the Motorz



int speed; // Speed for Power Width Modulatiщт
unsigned char Foot, OldPINC, Counter, i, k, OldA4, StTimer, RfCounter, NoRF;
int Pos0, Pos1, Target0, Target1, Round0, Round1, inp, Step2Go, Stage  ;

String inputString = "";         // a string to hold incoming data 2 Controle via USB
boolean stringComplete = false;  // whether the string is complete



void setup() {  // This code runs once at the beginning



  pinMode(2, INPUT); // INA2
  pinMode(4, OUTPUT); // INA2
  pinMode(5, OUTPUT); // PWM1

  pinMode(6, OUTPUT); // PWM2
  pinMode(7, OUTPUT); // INA2
  pinMode(8, OUTPUT);  // INB1
  pinMode(9, OUTPUT);  //INB2


  pinMode(10, INPUT_PULLUP); // GO FOOT 0
  pinMode(11, INPUT_PULLUP); // GO FOOT 1

  pinMode(12, INPUT_PULLUP); // MOTOR 0 ZERO
  pinMode(13, INPUT_PULLUP); // MOTOR 1 ZERO


  pinMode(A0, INPUT_PULLUP); // Motor 0
  pinMode(A1, INPUT_PULLUP); // Motor 0

  pinMode(A2, INPUT_PULLUP); // MOTOR 1
  pinMode(A3, INPUT_PULLUP); // MOTOR 1


  pinMode(A4, INPUT_PULLUP); // STEP2Make


  // Initialase the USART module as USB too
  Serial.begin(9600);

  PCMSK1 = 0x05; // Pin change mask pins C0 C1 C2 ENABLE any change
  PCIFR = 0x02; //PCINT 14-8 Flag   clear
  PCICR = 0x02; //Pins C0 - C5 interruptions enable

  Pos0 = 800; // Set zero Foot 0
  Pos1 = 450; // Set zero Foot 1

}


void loop() {
  speed = analogRead (A5) / 4; // Read the data from Analog Input A5 and make byte (0-255)

  if ((PINC & 0x10) == 0x00) // if Button A4 pressed make one  more step
  {
    if (OldA4 == 0x10 && StTimer == 0) // Bounce Defence
      Step2Go++;


    StTimer = 200;

  }
  OldA4 = PINC & 0x10;
  if (StTimer)
    StTimer--;


  if ((PIND & 0x04) == 0x04) // Remote RF input
  {
    if (NoRF == 0x00 && RfCounter == 0) // Bounce Defence
      Step2Go++;


    RfCounter = 50;

  }
  NoRF = PIND & 0x04;
  if (RfCounter)
    RfCounter--;








  if (Step2Go) // make a step
  {

    switch (Stage) // Stage of a step
    {
      case 0: // First Foot Begin
        if (Target1 == 0  && Target0 == 0)
        {
          Target0 = 1100 - Pos0;
          Stage = 1;
        }
        break;

      case 1:// Second Foot Up
        if (Target0 == 0)
        {
          Target1 = 785 - Pos1;
          Stage = 2;

        }

        break;
      case 2:  // First Foot Step End
        if (Target1 == 0)
        {
          Target0 = 1050;
          Stage = 3;
        }

        break;

      case 3: // Second Foot Step Begin
        if (Target0 == 0)
        {
          Target1 = 235;
          Stage = 4;

        }

        break;


      case 4:  // First Foot Up
        if (Target1 == 0)
        {
          Target0 = 350;
          Stage = 5;
        }

        break;


      case 5: // Second Foot Step End
        if (Target0 == 0)
        {
          Target1 = 1130;
          Stage = 6;

        }

        break;
      case 6: // The init - first up, second step End
        if (Target1 == 0)
        {
          Step2Go--;
          Stage = 0;
        }

        break;

    }

  }


  if (Target0 > 0) // Go Forward  First Foot
  {
    digitalWrite ( 7, LOW);
    digitalWrite ( 8, HIGH);
    analogWrite (5, speed);

  }

  if (Target1 > 0) // Go Forward  Second Foot
  {

    digitalWrite ( 4, HIGH);
    digitalWrite ( 9, LOW);
    analogWrite (6, speed);
  }


  if (digitalRead(10) == 0) { // Button first foot go
    digitalWrite ( 7, LOW);
    digitalWrite ( 8, HIGH);
    analogWrite (5, speed);
    digitalWrite ( 4, LOW);
    digitalWrite ( 9, LOW);
    analogWrite (6, speed);
  }

  else if (digitalRead(11) == 0) { // Button second foot go
    digitalWrite ( 7, LOW);
    digitalWrite ( 8, LOW);
    analogWrite (5, speed);
    digitalWrite ( 4, HIGH);
    digitalWrite ( 9, LOW);
    analogWrite (6, speed);
  }
  else  {

    if (Target0 == 0)           // stop if done first foot
    {
      digitalWrite ( 7, LOW);
      digitalWrite ( 8, LOW);
      analogWrite (5, speed);
    }
    if (Target1 == 0) // stop if done second foot
    {
      digitalWrite ( 4, LOW);
      digitalWrite ( 9, LOW);
      analogWrite (6, speed);
    }
  }



  if (Counter++ > 10) // print the motor encoder and targets data
  {
    Counter = 0;

    Serial.print("Pos0= \t");
    Serial.print( Pos0);

    Serial.print(" \tTarget0=  \t");
    Serial.print( Target0);



    Serial.print(" \tPos1= \t  ");
    Serial.print( Pos1);

    Serial.print(" \tTarget1= \t");
    Serial.print( Target1);


    Serial.print(" \tspeed= \t ");

    Serial.print(speed);
    Serial.print(" \tSteep2Go= \t ");

    Serial.print (Step2Go);
    Serial.print(" \tStage= \t ");

    Serial.println (Stage);
  }

  // print the string when a newline arrives:
  if (stringComplete  ) {

    if (  inputString.length() == 6) // if we get command
    {
      inp = int(inputString[1] - 48) * 1000 + int(inputString[2] - 48) * 100 + int(inputString[3] - 48) * 10 + int(inputString[4] - 48);


      if (inputString[0] == '0')
        Target0 = inp;
      else if (inputString[0] == '1')
        Target1 = inp;
      else
        Step2Go = inp;





    }

    inputString = "";
    stringComplete = false;
  }


  if ( (PINB & 0x10) == 0) // Limit switch first foot
    Pos0 = 0;

  if ((PINB & 0x20) == 0) // Limit switch first foot
    Pos1 = 0;


  delay(10);  // pause to think
}

ISR(PCINT1_vect)  // External interruption for Encoder service
{


  if ( (PINC & 0x01 ) == 0x00  && (OldPINC & 0x01) == 0x01)
  {
    if (PINC & 0x02 )
      Pos0--;
    else
    {
      Pos0++;

      if (Target0)
        Target0--;

    }
  }


  if ( (PINC & 0x04 ) == 0x00  && (OldPINC & 0x04) == 0x04)
  {
    if (PINC & 0x08 )
    {
      Pos1++;

      if (Target1)
        Target1--;

    }

    else
      Pos1--;

  }



  OldPINC = PINC;

}


void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {


      stringComplete = true;


    }
  }



}



void ResetMotorz(void)
{
  int k0 = 1, k1 = 1;
  speed = analogRead (A5) / 4;

  Pos1 = 100;
  Pos0 = 100;
  for (i = 0; i < 16   && (k0 > 0 || k1 > 0)  ; i++)
  {
    Target0 = 100;
    Target1 = 100;
    while  (  (Target0 > 0  || Target1 > 0 )  && (k1 > 0 || k0 > 0)  ) {
      if (Target0 > 0)
      {
        digitalWrite ( 7, LOW);
        digitalWrite ( 8, HIGH);
        analogWrite (5, speed);

      }
      else
      {
        digitalWrite ( 7, LOW);
        digitalWrite ( 8, LOW);
        analogWrite (5, speed);

      }


      if (Target1 > 0)
      {
        digitalWrite ( 4, HIGH);
        digitalWrite ( 9, LOW);
        analogWrite (6, speed);

      }
      else
      {
        digitalWrite ( 4, LOW);
        digitalWrite ( 9, LOW);
        analogWrite (6, speed);

      }



      if ( (PINB & 0x10) == 0)
      {
        Pos0 = 0;

        k0 = 0;
      }

      if ((PINB & 0x20) == 0)
      {
        Pos1 = 0;

        k1 = 0;
      }
      if (0)
      {
        Serial.print(i);
        Serial.print("   \n");

        Serial.print(k0);
        Serial.print("   \n");

        Serial.print(k1);
        Serial.print("   \n");

        Serial.print(Target0);
        Serial.print("   \n");
        Serial.println(Target1);
      }


    }


    digitalWrite ( 7, LOW);
    digitalWrite ( 8, LOW);
    analogWrite (5, speed);

    digitalWrite ( 4, LOW);
    digitalWrite ( 9, LOW);
    analogWrite (6, speed);
    delay(300);
    Serial.println("hre stop");
  }

  delay(700);



  if (Pos0 > 800 || Pos1 > 450)
  {

    Target0 = 1700 + 800 - Pos0;
    Target1 = 1708 + 450 - Pos1 + 66;

  }
  else

  {

    Target0 = 800 - Pos0;
    Target1 = 450 - Pos1 + 66;

  }

  Serial.print(Pos0);
  Serial.print("\t   ");
  Serial.print(Target0);
  Serial.print("\t   ");

  Serial.print(Pos1);
  Serial.print("\t   ");
  Serial.print(Target1);
  Serial.println("\t   ");



}
