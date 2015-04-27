#include <TimerOne.h>
#include <Math.h>

//int pwCurvePot=A0;
int freqPot = A1;
int durationPot = A2;
int freqCurvePot = A0;
int tempoPot = A3;
int ledPin = 3;
int buttonPin = 5;
int triggerPin = 8;
int clockPulse = 0;
int tempoValue;
int potMap;
int clockDivMult;
unsigned long timeoutTimer = 0;
unsigned long pulseTime = 0;
unsigned long previousPulse = 0;
unsigned long currentPulse = 0;

bool buttonState = 0;
bool lastButtonState = 0;
bool trigState = 0;
bool lastTrigState = 0;
bool playing;
long lowFreq = 30000; //lowest frequency in microseconds 30,000 = 33.3 Hz

int out = 9;
float pulseWidth = 512;
float duration = 100;
float frequency = 20;
float freqCurve;
float pwCurve;

bool isHit = false;

float beginTime;
float endTime;
float now;
float duty;
float period;

boolean keyOn = false;

void setup()
{
	Timer1.initialize();
	//Serial.begin(115200); 
	pinMode(ledPin, OUTPUT);
	pinMode(buttonPin, INPUT);
	pinMode(triggerPin, INPUT);
	playing = false;
}


bool checkTrigger()
{
	//isHit=true;

	buttonState = digitalRead(buttonPin);

	if ((buttonState == LOW) && (lastButtonState == HIGH)) {
		isHit = true;
		lastButtonState = LOW;
	}

	// check if the pushbutton is pressed.
	// if it is, the buttonState is HIGH:
	if ((buttonState == HIGH) && (lastButtonState == LOW)) {
		//isHit = false;
		lastButtonState = HIGH;
	}
	else {
		//isHit=0;
		//lastButtonState=0;
	}

	trigState = digitalRead(triggerPin);



	tempoValue = analogRead(tempoPot);

	if ((trigState == HIGH) && (lastTrigState == LOW)) {
		//clockPulse++;
		currentPulse = millis();

		if ((currentPulse - previousPulse) > 100)		//	Reset clock pulse if clock stops to start on the first clock
		{
			clockPulse = 0;

		}
		previousPulse = currentPulse;

		if (clockPulse > 95) { clockPulse = 0; } //	Clocks start at zero.


		potMap = map(tempoValue, 0, 1023, 0, 10);
		if (potMap == 5) { clockDivMult = 12; } //	8th note -- default setting
		if (potMap == 4) { clockDivMult = 6; } // 
		if (potMap == 3) { clockDivMult = 4; } // 
		if (potMap == 2) { clockDivMult = 3; } // 
		if (potMap == 1) { clockDivMult = 2; } // 
		if (potMap == 0) { clockDivMult = 1; } // 
		if (potMap == 6) { clockDivMult = 24; } // 
		if (potMap == 7) { clockDivMult = 32; } // 
		if (potMap == 8) { clockDivMult = 48; } // 
		if (potMap == 9) { clockDivMult = 60; } // 
		if (potMap == 10) { clockDivMult = 96; } // 

		if ((clockPulse % clockDivMult == 0))	// Trigger on every 8th pulse for eighth note sequence.&& (lastClock != clockPulse
		{
			isHit = true;
		}

		lastTrigState = HIGH;
		clockPulse++;
	}

	if ((trigState == LOW) && (lastTrigState == HIGH)) {
		//isHit = false;
		lastTrigState = LOW;
	}

	if (isHit) { return true; }
}






void hitIt()
{
	isHit = false;
	playing = true;

	//Serial.println ("HIT"); 

	duration = map(analogRead(durationPot), 0, 1023, 50, 1000);

	beginTime = millis();
	endTime = (beginTime + duration) - millis();
	now = 1;
	while (now < endTime){                                       // Loop until end of duration, 


		now = (millis() - beginTime) + 1;                                        // milliseconds since start of loop

		freqCurve = (((float((analogRead(freqCurvePot)) - 512) / 512)) * 10); // make range between -1 and +1
		//pwCurve=  (((float((analogRead  (pwCurvePot))-512)/512))*10);

		duration = map(analogRead(durationPot), 0, 1023, 50, 1000);      // Final duration 100 to 1000 ms milliseconds

		frequency = map(analogRead(freqPot), 0, 1023, 1000, lowFreq);         // Final frequency 100000 to 1000 us microseconds -- lowest to highest 
		//frequency=fscale(0,1023,100000,10000,analogRead(freqPot),freqCurve);
		pulseWidth = fscale(0, 1023, 512, 0, 256, pwCurve);
		pulseWidth = 256;
		//period=(frequency/(endTime/now));                                 // calculate frequency at current time 
		period = ((fscale(1000, lowFreq, lowFreq, lowFreq, frequency, freqCurve)) / (endTime / now)) + frequency;

		duty = (512 - pulseWidth / (endTime / now));

		//      Serial.print ("  Duration: ");Serial.print (duration);
		//      Serial.print ("  frequency: ");  Serial.print(frequency);
		//      Serial.print ("  freqCurve: ");  Serial.print(freqCurve);
		//      Serial.print("  endTime: ");Serial.print (endTime);  
		//      Serial.print ("  period: ");Serial.print(period);
		//      Serial.print ("  duty: ");Serial.print(duty);    
		//      Serial.print("  now: ");Serial.println (now);  



		Timer1.pwm(out, duty, period);                         // send square wave at proper pulse width and frequency.

		analogWrite(ledPin, (duty / 4) + 512);                   // set BPM indictor LED

		//if (checkTrigger()) { exit; }
		checkTrigger();
		if (isHit){ break; }
	}

	Timer1.pwm(out, 0, 0);                                         // at end of note shut off
	analogWrite(ledPin, 0);
	playing = false;
	//if (isHit){ hitIt; } // play immediately if interrupted.
	//delay(1000);        
	// wait one second (for testing)

}

void loop()
{
	if (isHit){ hitIt; }

	checkTrigger();

	if (isHit) {

		if (playing){
			/*Timer1.pwm(out, 0, 0);
			analogWrite(ledPin, 0);
			playing = false;


			duration = map(analogRead(durationPot), 0, 1023, 50, 1000);

			beginTime = millis();
			endTime = (beginTime + duration) - millis();
			now = 1;*/
		}

		hitIt();
	}

}


float fscale(float originalMin, float originalMax, float newBegin, float
	newEnd, float inputValue, float curve){

	float OriginalRange = 0;
	float NewRange = 0;
	float zeroRefCurVal = 0;
	float normalizedCurVal = 0;
	float rangedValue = 0;
	boolean invFlag = 0;


	// condition curve parameter
	// limit range

	if (curve > 10) curve = 10;
	if (curve < -10) curve = -10;

	curve = (curve * -.1); // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output 
	curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function

	/*
	Serial.println(curve * 100, DEC);   // multply by 100 to preserve resolution
	Serial.println();
	*/

	// Check for out of range inputValues
	if (inputValue < originalMin) {
		inputValue = originalMin;
	}
	if (inputValue > originalMax) {
		inputValue = originalMax;
	}

	// Zero Refference the values
	OriginalRange = originalMax - originalMin;

	if (newEnd > newBegin){
		NewRange = newEnd - newBegin;
	}
	else
	{
		NewRange = newBegin - newEnd;
		invFlag = 1;
	}

	zeroRefCurVal = inputValue - originalMin;
	normalizedCurVal = zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float

	/*
	Serial.print(OriginalRange, DEC);
	Serial.print("   ");
	Serial.print(NewRange, DEC);
	Serial.print("   ");
	Serial.println(zeroRefCurVal, DEC);
	Serial.println();
	*/

	// Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine 
	if (originalMin > originalMax) {
		return 0;
	}

	if (invFlag == 0){
		rangedValue = (pow(normalizedCurVal, curve) * NewRange) + newBegin;

	}
	else     // invert the ranges
	{
		rangedValue = newBegin - (pow(normalizedCurVal, curve) * NewRange);
	}

	return rangedValue;
}

