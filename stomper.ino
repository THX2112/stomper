/***
*       _____ _______ ____  __  __ _____  ______ _____
*      / ____|__   __/ __ \|  \/  |  __ \|  ____|  __ \
*     | (___    | | | |  | | \  / | |__) | |__  | |__) |
*      \___ \   | | | |  | | |\/| |  ___/|  __| |  _  /
*      ____) |  | | | |__| | |  | | |    | |____| | \ \
*     |_____/   |_|  \____/|_|  |_|_|    |______|_|  \_\
*
*						***THX2112***
*
*					http://syinsi.com
*
*/



#include <digitalWriteFast.h>


// Pins:
int freqPot = A0;
int freqCurvePot = A1;
int freqEndPot = A2;
int durationPot = A3;
int tempoPot = A4;
int ledPin = 3;
int gatePin = 4;
int buttonPin = 5;
int triggerPin = 8;
int clockPin = 7;
int out = 9;

//	Variables:

int tempoValue;
int potMap;
int clockDivMult;
int bounceTimer = 0;
int lastBounceTime = 0;
int clockPulse = 7;
boolean done;

boolean period = false;
unsigned long timeoutTimer = 0;		//	microseconds
unsigned long previousPulse = 0;	//	microseconds
unsigned long currentPulse = 0;		//	microseconds
unsigned long periodStartTime = 0;	//	microseconds
unsigned long periodEndTime = 0;	//	microseconds
unsigned long periodPW;				//	pulsewidth size in microseconds
float startFreq = 0;
float endFreq = 1000000;

unsigned long previousEndTime;

bool buttonState = 0;
bool lastButtonState = 0;
bool trigState = 0;
bool lastTrigState = 0;
bool clockState = 0;
bool lastClockState = 0;
bool outState;

unsigned long pulseWidth = 512;		//	used later
float duration;						//	microseconds
float frequency;					//	wavelength in microseconds
float freqCurve;					//	0-1
unsigned long periodNow = 0;

bool isHit = false;
bool ledLit = false;
bool justHit = false;

float beginTime;
float now;


///////////////////////////////////////////////////////////////////////////////
//
//	Setup pins.
//

void setup()
{
	pinModeFast(ledPin, OUTPUT);
	pinModeFast(out, OUTPUT);
	pinModeFast(buttonPin, INPUT_PULLUP);
	pinModeFast(triggerPin, INPUT);
	pinModeFast(clockPin, INPUT);
	pinModeFast(gatePin, OUTPUT);

	//	Unused pins tied high
	pinMode(2, INPUT_PULLUP);
	pinMode(6, INPUT_PULLUP);
	pinMode(10, INPUT_PULLUP);
	pinMode(11, INPUT_PULLUP);
	pinMode(12, INPUT_PULLUP);
	pinMode(13, INPUT_PULLUP);

	//	Flash LED
	digitalWriteFast(ledPin, HIGH);
	delay(20);
	digitalWriteFast(ledPin, LOW);
}

///////////////////////////////////////////////////////////////////////////////
//
//	This is it.
//

void loop()
{

	checkTrigger();

	if (isHit)
	{
		hitIt();
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	Time stuff. See if trigger is being hit...
//

void checkTrigger()
{
	//
	//	First check if button is being pressed, and debounce. Could be done better.
	//

	buttonState = digitalReadFast(buttonPin);

	if ((buttonState == HIGH) && (lastButtonState == LOW))
	{
		lastButtonState = HIGH;
		bounceTimer = millis() - lastBounceTime;
	}

	if ((buttonState == LOW) && (lastButtonState == HIGH))
	{
		lastButtonState = LOW;

		if (bounceTimer >= 50)
		{
			isHit = true;
			lastBounceTime = millis();
		}
		else {
			lastBounceTime = millis();
		}
	}

	//
	//	Then check clock.
	//

	clockState = digitalReadFast(clockPin);

	if ((clockState == HIGH) && (lastClockState == LOW))
	{
		currentPulse = millis();
		lastClockState = HIGH;

		if ((currentPulse - previousPulse) > 5000)	//	Reset clock pulse if clock stops to start on the first clock
		{
			clockPulse = 0;
		}

		previousPulse = currentPulse;

		if (clockPulse > 96)						//	Clocks start at one.
		{
			clockPulse = 1;
		}

		//
		//	Set division. Half of range for 24ppqn clock, other half for trigger division
		//

		tempoValue = analogRead(tempoPot);

		potMap = map(tempoValue, 0, 1023, 11, 0);	// Reverse response of pot and map to X values

		if (potMap == 11) { clockDivMult = 96; }	//	1	Every bar @24ppqn
		if (potMap == 10) { clockDivMult = 48; }	//	2	half	@24ppqn
		if (potMap == 9) { clockDivMult = 24; }		//	4	quarter @24ppqn
		if (potMap == 8) { clockDivMult = 16; }		//	6	@24ppqn
		if (potMap == 7) { clockDivMult = 12; }		//	8th notes  @24ppqn
		if (potMap == 6) { clockDivMult = 8; }		//	/8
		if (potMap == 5) { clockDivMult = 6; }		//	/6
		if (potMap == 4) { clockDivMult = 5; }		//	/5
		if (potMap == 3) { clockDivMult = 4; }		//	/4
		if (potMap == 2) { clockDivMult = 3; }		//	/3
		if (potMap == 1) { clockDivMult = 2; }		//	/2
		if (potMap == 0) { clockDivMult = 1; }		//	No Division

		//
		//	Check if it's time to send a hit
		//

		if ((clockPulse % clockDivMult == 0))
		{
			isHit = true;
			justHit = true;
			clockPulse = 0;							// Restart clock if this is a new hit or timing changed.
		}

		clockPulse++;
	}

	if ((clockState == LOW) && (lastClockState == HIGH))
	{
		lastClockState = LOW;
	}

	//
	//	Now check trigger.
	//

	trigState = digitalReadFast(triggerPin);

	if ((trigState == HIGH) && (lastTrigState == LOW))
	{
		isHit = true;
		justHit = true;
		lastTrigState = HIGH;
		clockPulse = 0;
	}

	if ((trigState == LOW) && (lastTrigState == HIGH))
	{
		lastTrigState = LOW;
	}

}

///////////////////////////////////////////////////////////////////////////////
//
//	Audio stuff. Send the audio pulse.
//

void hitIt()
{
	//
	//	Start of the sound.
	//

	digitalWriteFast(gatePin, HIGH);
	digitalWriteFast(out, HIGH);									//	Start hit immediately -- catch up with calculations later.
	outState = HIGH;
	periodStartTime = micros();

	beginTime = micros();
	now = 0;
	periodStartTime = micros();
	period = LOW;
	duration = (analogRead(durationPot) * 2000.0) + 100000.0;		//	Microseconds
	isHit = false;
	unsigned long halfFreq;

	//
	//	Calculate the duration of each individual wavelength, taking into consideration curvature.
	//	Can be put inside the next loop for faster response, but increases audio artifacts.
	//

	duration = (analogRead(durationPot) * 1000.0) + 100000.0;		//	Microseconds
	startFreq = (analogRead(freqPot) * 2) + 200.0;					//	200 to 2246 Hz
	freqCurve = ((analogRead(freqCurvePot) / 1023.0)) + .2;			//	make range between 0 and +1 and adda bit for movement. 0 stands still, minus moves backwards.
	endFreq = (analogRead(freqEndPot) / 4.0) + 20.0;				//	20 to 276 Hz

	//
	//	Loop until the end of DURATION set by pot. This needs to loop really fast.
	//	Could be improved by replacing float math with fixed math.
	//

	while (now < duration)
	{
		//	When I wrote this, only God and I understood what I was doing.
		//	Now, God only knows.

		frequency = (((now / duration) * (freqCurve * 10.0)) * ((1.0 / endFreq) * 1000000.0)) + ((1.0 / startFreq) * 1000000.0); //period length in micros

		periodNow = micros();
		halfFreq = frequency / 2;
		periodEndTime = periodStartTime + halfFreq;

		//
		//	Toggle the output to create a square wave.
		//

		if ((periodNow >= periodEndTime))
		{
			if (outState == LOW)
			{
				if (now + halfFreq <= duration)			//	Prevent new period from starting if a new clock is going to be sent.
				{
					digitalWriteFast(out, HIGH);
					outState = HIGH;
				}
				else
				{
					//	do nothing
				}
			}

			else if (outState == HIGH)
			{
				digitalWriteFast(out, LOW);
				outState = LOW;
			}

			periodStartTime = micros();					//	Register time waveform was made.
		}

		//
		//	Flash LED for 100ms at start of hit.
		//

		if (now <= 100000)	//
		{
			digitalWriteFast(ledPin, HIGH);
		}
		else
		{
			digitalWriteFast(ledPin, LOW);
		}

		//
		//	Break out of loop if there's another hit.
		//

		checkTrigger();

		if (isHit)
		{
			break;
		}

		//
		//	End of fast wavelength loop -- a good place for timing tests.
		//

		now = (micros() - beginTime);					//	Register time for next run through.

	}

	//
	//	Clean up at end of hit. Diagnostic code here.
	//

	digitalWriteFast(gatePin, LOW);
	digitalWriteFast(ledPin, LOW);
	digitalWriteFast(out, LOW);
	period = LOW;

}

