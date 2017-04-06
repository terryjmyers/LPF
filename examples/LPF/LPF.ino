
/*
LPF
Low Pass Filter
*/

#include <LPF.h>
float Unstable_Signal = 0.0; //create a numebr for random data

//define some low pass filters with different strengths.  Review the LPF.h for more information
//0=disable, 1=0.05alpha, 2=0.01alpha, 3=0.00 alpha, 4=0.0001alpha
LPF LowPF05(1);
LPF LowPF01(2);
LPF LowPF001(3);
LPF LowPF0001(4);


void setup(void) 
{
  //Setup Serial 
  //In case the user uses the default setting in Arduino, give them a message to change BAUD rate
    Serial.begin(9600);
    Serial.println(F("Set Serial baud rate to 250 000"));
    Serial.flush ();   // wait for send buffer to empty
    delay (2);    // let last character be sent
    Serial.end ();      // close serial
    Serial.begin(250000);
    Serial.println();

    //Initialze the REALLY slow filter to the average of the signal random values.  You will see the LPF0001 already "at the average"
    LowPF0001.init(50.0);
}

void loop(void) {

  Unstable_Signal=random(0,100); //create a random number between 0 and 100, which would have an average of 50
Serial.print("Unstable Signal = ");  Serial.print(Unstable_Signal);
 Serial.print("   LPF05=");    Serial.print(LowPF05.step(Unstable_Signal));
Serial.print("   LPF01=");    Serial.print(LowPF01.step(Unstable_Signal));
Serial.print("   LPF001=");   Serial.print(LowPF001.step(Unstable_Signal));
Serial.print("   LPF0001=");  Serial.print(LowPF0001.step(Unstable_Signal));
Serial.println();
delay(10);

}