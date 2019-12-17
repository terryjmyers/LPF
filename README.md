# LPF Low Pass Filter Library

A First Order Low Pass Filter library  

Current Version = 1.3  
Version Date = 12/17/2019  

**How to use**  
1. Add constructor and input time constant: i.e. LPF AnalogValueLPF(10);  
2. Every time you get a new Analog Value to filter (i.e. analogRead(A0)), call the Step method and pass it in.  
The Step method will return the filtered ananalog value:: i.e. PostFilteredAnalogValue = AnalogValueLPF.step(analogRead(A0))
Returns NAN if there is a problem.  Check GetErrorMessage for human readable message as to the error:  
i.e. 
```
if (AnalogValueLPF.step(PreFilteredAnalogValue) == NAN) {
  Serial.println(AnalogValueLPF.GetErrorMessage());
}
```
3. Any time you want, directly set the time constant in units of seconds: i.e. AnalogValueLPF.RC = 1;
4. Set the time constant based on cuttoff frequency (fc) by having the Setfc method calculate the RC and set it for you
5. All internal variables are doubles for maximum precision.  For maximum speed I recommend converting your pre filtered value first and returning it into a double:  
i.e.
```
double PreFilteredAnalogValue = double(analogRead(A0));
double PostFilteredAnalogValue = AnalogValueLPF.step(PreFilteredAnalogValue); //Use the 'PostFilteredAnalogValue' elsewhere in your code by scaling it or whatever
if (PostFilteredAnalogValue == NAN) {
  //Do something here...something went wrong
  Serial.println(AnalogValueLPF.GetErrorMessage());
}
```

**Simple Usage (Note this program hasn't actually been tested, but you get the idea):**
```
#include <LPF.h>
LPF AnalogValueLPF(10);			//Create a LPF will a time constant of 10seconds
int PostFilteredRawAnalogValue;	//The final filtered value to use in your program, ready for scaling
Setup {
  Serial.Begin(115200);
}
loop {
  int PreFilteredRawAnalogValue = analogRead(A0);
  PostFilteredRawAnalogValue = AnalogValueLPF.step(PreFilteredAnalogValue);		//Perform Low Pass Filter
  Serial.print(F(" Pre= ")); Serial.println(PreFilteredAnalogValue);
  Serial.print(F(" Post= ")); Serial.println(PostFilteredAnalogValue);
  Serial.print();
  //....Rest of program here, such as scaling PostFilteredRawAnalogValue to useable units
  delay(10); //Just to prevent spamming of Serial port
}
```
**Suggested Usage (Note this program hasn't actually been tested, but you get the idea):**
```
#include <LPF.h>
LPF AnalogValueLPF(10);			//Create a LPF will a time constant of 10seconds
double PostFilteredRawAnalogValue;	//The final filtered value used in your program
double ScaledValue;
Setup {
  Serial.Begin(115200);
}
loop {

  //Library internal variables are all 'double', you will save some time in method calls if you convert to double right away and return to a double
  //Get Raw analog value and convert it to a double right away (the library is just going to do this anyway)
  double PreFilteredAnalogValue = double(analogRead(A0));

  //Perform Low Pass Filter
  PostFilteredAnalogValue = AnalogValueLPF.step(PreFilteredAnalogValue);

  //Error Checking
  if (PostFilteredAnalogValue == NAN) {//Something Went Wrong, handle it, you don't want the rest of your code working with a bad number
    //Do something to handle that its a bad number, such as go to scale high.
    ScaledValue = 100.0
    Serial.println(AnalogValueLPF.GetErrorMessage());
  } 
  else {
    //PostFilteredAnalogValue is good!  Scale it or something.  Example below scales a 12bit analog between 0-100.0
    ScaledValue = PostFilteredAnalogValue /  4096.0 * 100.0  
  }

  Serial.print(F(" Pre= ")); Serial.println(PreFilteredAnalogValue);
  Serial.print(F(" Post= ")); Serial.println(PostFilteredAnalogValue);
  Serial.print();

  //....Rest of program here

  delay(10); //Just to prevent spamming of Serial port
}
```
ChangeLog  
 v1.3 - Modified to dynamically calculate the clock cycles per us dynamically based on F_CPU.  Moved assembly code for getting clock cycles into main step method to avoid an additional call.  Added Additional overloads for direct use of analogRead and other ints  
v1.2 - Modified to dynamic assignment of LPF coeffients based on a cutoff frequency or time constant. adapated from: https://www.quora.com/Whats-the-C-coding-for-a-low-pass-filter  
v1.1 - Condensed the multiple functions into one LPF function with a Stregth input  
v1.0 - First release.  Adapted from code retrieved from http://www.schwietering.com/jayduino/filtuino  
