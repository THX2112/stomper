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

// Pins:
//int pwCurvePot=A0;
int freqPot = A1;
int durationPot = A2;
int freqCurvePot = A0;
int tempoPot = A3;
int ledPin = 3;
int buttonPin = 5;
int triggerPin = 8;
int clockPulse = 0;
int out = 9;

int tempoValue;
int potMap;
int clockDivMult;
int bounceTimer = 0;
int lastBounceTime = 0;

unsigned long timeoutTimer = 0;	//	microseconds
unsigned long previousPulse = 0;	//	microseconds
unsigned long currentPulse = 0;	//	microseconds
unsigned long periodStartTime = 0;	//	microseconds
unsigned long periodEndTime = 0;	//	microseconds
unsigned long periodPW; // pulsewidth size in microseconds
float startFreq = 0;
float endFreq = 1000000;

bool buttonState = 0;
bool lastButtonState = 0;
bool trigState = 0;
bool lastTrigState = 0;


unsigned long pulseWidth = 512;	// used later
float duration;	//	milliseconds
float frequency;	//	wavelength in microseconds
float freqCurve;		//0-1
unsigned long periodNow = 0;

bool isHit = false;

float beginTime;
float now;


// Testing Variables
int PTLoopNumber = 0;
unsigned long PTTimerResult = 0;
unsigned long PTTimer = 0;
unsigned long PTOldTimer = 0;

///////////////////////////////////////////////////////////////////////////////
//
//	Setup pins. Remember to tie unused pins high.
//

void setup()
{
	//Serial.begin(115200);	//	For testing.
	pinMode(ledPin, OUTPUT);
	pinMode(out, OUTPUT);
	pinMode(buttonPin, INPUT);
	pinMode(triggerPin, INPUT);
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

	buttonState = digitalRead(buttonPin); // Note that in this HW version button is externally tied high. This changes in future revs.

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
	//	Then check trigger.
	//

	trigState = digitalRead(triggerPin);
	tempoValue = analogRead(tempoPot);

	if ((trigState == HIGH) && (lastTrigState == LOW))
	{
		currentPulse = millis();

		if ((currentPulse - previousPulse) > 100)		//	Reset clock pulse if clock stops to start on the first clock
		{
			clockPulse = 0;
		}

		previousPulse = currentPulse;

		if (clockPulse > 95)	//	Clocks start at zero.
		{
			clockPulse = 0;
		}

		//
		//	Set division
		//

		potMap = map(tempoValue, 0, 1023, 0, 10);

		if (potMap == 5) { clockDivMult = 12; } //	8th note -- default setting
		if (potMap == 4) { clockDivMult = 6; }	//	16th notes
		if (potMap == 3) { clockDivMult = 4; }	//	24	fast
		if (potMap == 2) { clockDivMult = 3; }	//	32	fucking fast
		if (potMap == 1) { clockDivMult = 2; }	//	64	something is going to break
		if (potMap == 0) { clockDivMult = 1; }	//	96	every 24ppqm clock tick (may cause tiny black hole formations as time collapses upon itself.)
		if (potMap == 6) { clockDivMult = 24; } //	4	quarter
		if (potMap == 7) { clockDivMult = 32; } //	3	third
		if (potMap == 8) { clockDivMult = 48; } //	2	half
		if (potMap == 9) { clockDivMult = 64; } //	1.5	
		if (potMap == 10) { clockDivMult = 96; }//	1	Every bar.

		//
		//	Check if it's time to send a hit
		//

		if ((clockPulse % clockDivMult == 0))
		{
			isHit = true;
		}

		lastTrigState = HIGH;
		clockPulse++;
	}

	if ((trigState == LOW) && (lastTrigState == HIGH))
	{
		lastTrigState = LOW;
	}

	//if (isHit)
	//{
	//	return true;
	//}
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

	isHit = false;	//	Reset.
	//duration = map(analogRead(durationPot), 0, 1023, 50, 1000);		//	Milliseconds
	//duration = analogRead(durationPot) + 50;		//	Milliseconds
	duration = (.3 * 1000) * 1000; // seconds to milliseconds to microseconds
	//startFreq = (analogRead(freqPot) * 960) + 16667; // need to keep frequency as period length (in micros) to avoid using floats. 2048
	endFreq = 100; //Hz
	beginTime = micros();
	now = 0;
	periodStartTime = micros();

	//
	//	Loop until the end of DURATION. This needs to loop really fast. 
	//	Unfortunately it uses floats which severely impinges on performance
	//	resulting in an aliasing effect at high frequencies. However this was the
	//	best variant tried. Will keep trying...
	//

	while (now < duration)
	{

		//
		//	Calculate the duration of each individual wavelength, taking into
		//	consideration curvature -- which is why floats need to be used here.
		//

		//PTTimer = micros();	//	Timing test.
		startFreq = (analogRead(freqPot) * 2) + 10; // Hz
		freqCurve = ((analogRead(freqCurvePot) / 1024.0) * 1.0) + 0.3; // make range between -1 and +1
		//freqCurve = 1;
		pulseWidth = 512; // Replace is analog reading later...

		// MAGIC
		frequency = (((now / duration) * (freqCurve * 10)) * ((1 / endFreq) * 1000000)) + ((1 / startFreq) * 1000000);

		periodNow = micros();
		periodEndTime = periodStartTime + (frequency); // frequency/2 for 1/2 wave?
		//periodPW = frequency / 2;
		//
		//	Toggle the output to create a square wave.
		//

		if (periodNow >= periodEndTime)
		{
			digitalWrite(out, !digitalRead(out));	//	Toggle output (half frequency).
			periodStartTime = micros();				//	Register time waveform was made.
		}

		//
		//	Flash LED at start of hit.
		//

		if (now <= 100000)
		{
			analogWrite(ledPin, 254);
		}
		else
		{
			analogWrite(ledPin, 0);
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

		now = (micros() - beginTime);	//	Register time for next run through.

		//Timing Tests:
		/*PTLoopNumber = PTLoopNumber + 1;
		PTOldTimer = micros();
		PTTimerResult = PTTimerResult + (PTOldTimer - PTTimer);*/

	}

	//
	//	Clean up at end of hit. Diagnostic code here.
	//

	analogWrite(ledPin, 0);
	digitalWrite(out, LOW);

	//Timing Tests:
	/*Serial.print("Number: ");
	Serial.print(PTLoopNumber);
	Serial.print(" Average micros: ");
	Serial.println((PTTimerResult / PTLoopNumber));
	PTTimer = 0;
	PTTimerResult = 0;
	PTLoopNumber = 0;*/

}

