/*
LPF - Low Pass Filter Library
https://github.com/terryjmyers/LPF.git
Author =			GitHub.com/TerryJMyers
Current Version =	1.3
Version Date =		12/17/2019

A First Order Low Pass Filter library

	How to use
		Add constructor and input time constant: i.e. LPF AnalogValueLPF(10);
		Every time you get a new Analog Value to filter (i.e. analogRead(A0)), call the Step method and pass it in.
			The Step method will return the filtered ananalog value:: i.e. PostFilteredAnalogValue = AnalogValueLPF.step(analogRead(A0))
			Returns NAN if there is a problem.  Check GetErrorMessage for human readable message as to the error: 
				i.e. 
				if (AnalogValueLPF.step(PreFilteredAnalogValue) == NAN) {
					Serial.println(AnalogValueLPF.GetErrorMessage());
				}
		Any time you want, directly set the time constant in units of seconds: i.e. AnalogValueLPF.RC = 1;
		Set the time constant based on cuttoff frequency (fc) by having the Setfc method calculate the RC and set it for you
		All internal variables are doubles for maximum precision.  For maximum speed I recommend converting your pre filtered value first and returning it into a double:
			i.e.
				double PreFilteredAnalogValue = double(analogRead(A0));
				double PostFilteredAnalogValue = AnalogValueLPF.step(PreFilteredAnalogValue); //Use the 'PostFilteredAnalogValue' elsewhere in your code by scaling it or whatever
				if (PostFilteredAnalogValue == NAN) {
					//Do something here...something went wrong
					Serial.println(AnalogValueLPF.GetErrorMessage());
				}

	Simple Usage (Note this program hasn't actually been tested, but you get the idea):
		#include <LPF.h>
		LPF AnalogValueLPF(1);			//Create a LPF will a time constant of 1second
		int PostFilteredRawAnalogValue;	//The final filtered value to use in your program, ready for scaling
		Setup {
		  Serial.Begin(115200);
		}
		loop {
			int PreFilteredRawAnalogValue = analogRead(A0);
			PostFilteredRawAnalogValue = AnalogValueLPF.Step(PreFilteredAnalogValue);		//Perform Low Pass Filter
			Serial.print(F(" Pre= ")); Serial.println(PreFilteredAnalogValue);
			Serial.print(F(" Post= ")); Serial.println(PostFilteredAnalogValue);
			Serial.print();
			//....Rest of program here, such as scaling PostFilteredRawAnalogValue to useable units
			delay(10); //Just to prevent spamming of Serial port
		}

	Suggested Usage (Note this program hasn't actually been tested, but you get the idea):
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




	
	v1.3 - Modified to dynamically calculate the clock cycles per us dynamically based on F_CPU
			Moved assembly code for getting clock cycles into main step method to avoid an additional call
			Added Additional overloads for direct use of analogRead and other ints
	v1.2 - Modified to dynamic assignment of LPF coeffients based on a cutoff frequency or time constant
			adapated from: https://www.quora.com/Whats-the-C-coding-for-a-low-pass-filter
	v1.1 - Condensed the multiple functions into one LPF function with a Stregth input
	v1.0 - First release
			Adapted from code retrieved from http://www.schwietering.com/jayduino/filtuino

*/
#ifndef LPF_H
#define LPF_H


/*=========================================================================*/


class LPF
{
	public:
		//Constructor Overloads
		LPF(double _RC) { RC = _RC; _SecondsPerClockCycle = double(1) / double(F_CPU); }
		LPF(float _RC) { RC = double(_RC); _SecondsPerClockCycle = double(1) / double(F_CPU); }
		LPF(int _RC) { RC = double(_RC); _SecondsPerClockCycle = double(1) / double(F_CPU); }
		LPF(void) {	RC = 1.0;	_SecondsPerClockCycle = double(1) / double(F_CPU); }

		//Set the Time Constant in units of seconds at any time
		double RC = 0.0;


		// Calculate the Time Consant from a cutoff frequency
		void Setfc(double CutoffFrequency) { RC = 1.0 / (CutoffFrequency * 2.0 * PI); } 
		void Setfc(float CutoffFrequency) { Setfc(double(CutoffFrequency)); }
		void Setfc(int CutoffFrequency) { Setfc(double(CutoffFrequency)); }

		//Call to re-initialize the Filter
		void init(double _PreFilteredAnalog) {_PreviousPreFilteredAnalog = _PreFilteredAnalog; }
		void init(float _PreFilteredAnalog) { init(double(_PreFilteredAnalog)); }


		/*
		Step Method
			Every time you get a new Analog Value to filter (i.e. analogRead(A0)), call the Step method and pass it in.
			The Step method will return the filtered ananalog value:: i.e. PostFilteredAnalogValue = AnalogValueLPF.step(analogRead(A0))
			Returns NAN if there is a problem.  Check GetErrorMessage for human readable message as to the error: i.e. 
				if (AnalogValueLPF.step(SomePreFilteredAnalogValue) == NAN) {
					Serial.println(AnalogValueLPF.GetErrorMessage());
				}
			Additional Overloads for method are provided below
		*/
		double Step(double _PreFilteredAnalog) {

			//If this is the first call to this routine don't actually do anything but return the value you just passed in	
			if (_FirstRun == false) {
				_FirstRun = true; //Remember that you've called this routine previously

				//Create Assembly code that gets the clock cycles the fastest way possible
				#ifdef ESP32
					__asm__ __volatile__("esync; rsr %0,ccount":"=a" (_ClockCyclesPrevious));
				#else
					#ifdef ESP8266
						__asm__ __volatile__("rsr %0,ccount":"=a"(_ClockCyclesPrevious));
					#else
						#error LPF.h ERROR: Library Depends on assembly level programming to retrieve clock cycles as quick as possible so that the Step function can be called every scan.  Only ESP8266 and ESP32 is implemented
					#endif
				#endif
				init(_PreFilteredAnalog); //Remember what the previous value was
				return _PreFilteredAnalog; //return what you've passed in.
			}

			uint32_t ClockCycles;
			//Create Assembly code that gets the clock cycles the fastest way possible
			#ifdef ESP32
					__asm__ __volatile__("esync; rsr %0,ccount":"=a" (ClockCycles));
				#else
					#ifdef ESP8266
						__asm__ __volatile__("rsr %0,ccount":"=a"(ClockCycles));
					#else
						#error LPF.h ERROR: Library Depends on assembly level programming to retrieve clock cycles as quick as possible so that the Step function can be called every scan.  Only ESP8266 and ESP32 is implemented
					#endif
				#endif
			double _dt = double(ClockCycles - _ClockCyclesPrevious) * _SecondsPerClockCycle; //Calculate


			double _alpha;
			if (RC >= 0.0) {
				if (_dt > 0.0) {					
					_ErrorCode = 0;
					_alpha = _dt / (RC + _dt);
					_ClockCyclesPrevious = ClockCycles; //Remember the time captured above
					_PreviousPreFilteredAnalog += _alpha * (_PreFilteredAnalog - _PreviousPreFilteredAnalog);
					/*
					//Just some debug code used during development
					Serial.println("-------------------");
					Serial.print(F("RC = ")); Serial.println(RC,13);
					Serial.print(F("RC >= 0.0 = ")); Serial.println(RC >= 0.0);
					Serial.print(F("_dt = ")); Serial.println(_dt,13);
					Serial.print(F("_dt > 0.0 = ")); Serial.println(_dt > 0.0);
					Serial.print(F("RC + _dt = ")); Serial.println(RC + _dt,13);
					Serial.print(F("_alpha = _dt / (RC + _dt) = ")); Serial.println(_alpha,13);
					Serial.print(F("_alpha = ")); Serial.println(_alpha,13);
					Serial.print(F("_PreFilteredAnalog = ")); Serial.println(_PreFilteredAnalog,13);
					Serial.print(F("_PreviousPreFilteredAnalog = ")); Serial.println(_PreviousPreFilteredAnalog,13);
					Serial.print(F("_PreviousPreFilteredAnalog += _alpha * (_PreFilteredAnalog - _PreviousPreFilteredAnalog = ")); Serial.println(_PreviousPreFilteredAnalog,13);
					Serial.println("-------------------");
					*/
					return _PreviousPreFilteredAnalog;
				}
				else {
					_ErrorCode = 2;
					return NAN;
				}
			}
			else {
				_ErrorCode = 1;
				return NAN;				
			}
		}
		
		//Additional Overloads
		float Step(float _PreFilteredAnalog)	{	double temp = double(_PreFilteredAnalog);	return float(Step(temp));	};
		int Step(int _PreFilteredAnalog)		{	double temp = double(_PreFilteredAnalog);	return int(Step(temp));		};

		//Get a human Readable Error Code.  Do this if Step returns NAN.
		String GetErrorMessage(void) {
			switch (_ErrorCode)
			{
			case 0:
				return F("No Error");
			case 1:
				return F("Time Constant (LPF.RC) must be > 0");
			case 2:
				return F("Time Between Step Calls must be > 0");
			default:
				break;
			}
		}

private:
	double		_SecondsPerClockCycle;
	bool		_FirstRun = 0;
	double		_PreviousPreFilteredAnalog;
	uint32_t	_ClockCyclesPrevious = 0;
	uint8_t		_ErrorCode = 0;
};
#endif