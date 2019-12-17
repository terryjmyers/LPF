
/*
LPF
Low Pass Filter Example

*/


#include <LPF.h>
LPF AnalogValueLPF(1);			//Create a LPF will a time constant of 1second
double PostFilteredRawAnalogValue;	//The final filtered value used in your program
double ScaledValue;
Setup {
	Serial.Begin(115200);
}
loop {

	//Library internal variables are all 'double', you will save some time in method calls if you convert to double right away and return to a double
	//Get Raw analog value and convert it to a double right away (the library is just going to do this anyway)
	double PreFilteredAnalogValue = double(random(0, 100));

	//Perform Low Pass Filter
	PostFilteredAnalogValue = AnalogValueLPF.Step(PreFilteredAnalogValue);

	//Error Checking
	if (PostFilteredAnalogValue == NAN) {//Something Went Wrong, handle it, you don't want the rest of your code working with a bad number
		//Do something to handle that its a bad number, such as go to scale high.
		ScaledValue = 100.0;
		Serial.println(AnalogValueLPF.GetErrorMessage());
	} 
	else {
		//PostFilteredAnalogValue is good!  Scale it or something.  Example below scales a 12bit analog between 0-100.0
		ScaledValue = PostFilteredAnalogValue / 4096.0 * 100.0;
	}

	Serial.print(F(" Pre= ")); Serial.println(PreFilteredAnalogValue);
	Serial.print(F(" Post= ")); Serial.println(PostFilteredAnalogValue);
	Serial.println();

	//....Rest of program here

	delay(10); //Just to prevent spamming of Serial port
}
