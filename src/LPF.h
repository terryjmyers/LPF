/*
LPF - Low Pass Filter Library
	Adapted from code retrieved from http://www.schwietering.com/jayduino/filtuino



	https://github.com/terryjmyers/LPF.git
    v1.1 - Condensed the multiple functions into one LPF function with a Stregth input
    v1.0 - First release
*/
#ifndef LPF_H
#define LPF_H


/*=========================================================================*/


class LPF
{
	public:
		LPF(uint8_t Strenth = 1) { //set the LPF up initially with alpha = 0.5 the lowest setting if no value is passed in
			setStrength(Strenth);	
		}

		float step(float x) {
			if (_Strength==0) return x; //return if filter is disabled
			//If the LPF hasn't been initiated once, set the internal variables up so that it reports the current value immediatly
				if (_Strength != _StrengthREM) init(x); //_StrengthREM is set to -1 on instantiation so the first time this is run it will trigger an init
			//Calculate the LPF algorithm
				v[0] = v[1];
				v[1] = (_c1 * x) + (_c2 * v[0]);
			//remember for the next step
				_StrengthREM = _Strength; 
			return (v[0] + v[1]);
		}

		//Saturate the internal data so that it the filter reports the value immediatly
			void init(float x) {
				v[0] = x / 2.0;
				v[1] = v[0];
			}

		//0=disable, 1=0.05alpha, 2=0.01alpha, 3=0.00 alpha, 4=0.0001alpha
		void setStrength(uint8_t s) {
			_Strength = s; //set the internal Strength variable
			switch (_Strength)
			{
			case 0://disable filter, return immediatly
				return;
			case 1: //alpha = 0.05
				/*
				Low pass butterworth filter order=1 alpha1=0.05
				63% after 4 steps
				95% after 10 steps
				99.5% after 17 steps
				99.99% after 29 steps
				*/
				_c1 = 1.367287359973195227e-1;
				_c2 = 0.72654252800536101020;
				break;
			case 2: //alpha = 0.01
				/*
				Low pass butterworth filter order=1 alpha1=0.01
				63% after 16 steps
				95% after 48 steps
				99.5% after 85 steps
				99.99% after 147 steps
				*/
				_c1 = 3.046874709125380054e-2;
				_c2 = 0.93906250581749239892;
				break;
			case 3: //alpha = 0.001
				/*
				Low pass butterworth filter order=1 alpha1=0.001
				63% after 159 steps
				95% after 478 steps
				99.5% after 844 steps
				99.99% after 1100 steps
				*/
				_c1 = 3.131764229192701265e-3;
				_c2 = 0.99373647154161459660;
				break;
			case 4: //alpha = 0.0001
				/*
				Low pass butterworth filter order=1 alpha1=0.0001
				63% after 1583 steps
				95% after 4768 steps
				99.5% after 8433 steps
				99.99% after 14659 steps
				*/
				_c1 = 3.140606106404320030e-4;
				_c2 = 0.99937187877871913599;
				break;
			default: //Set default the same as alpha = 0.05 above
				_c1 = 1.367287359973195227e-1;
				_c2 = 0.72654252800536101020;
				break;
			}
		}
		uint8_t getStrength(void) { return _Strength; }

	private:
		float v[2]; //storage values
		float _c1;	//coefficient 1
		float _c2;//coefficient 2
		uint8_t _Strength;
		uint8_t _StrengthREM=-1; //Remember what Strength was the last time Step was run
};
#endif