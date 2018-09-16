#include <Bounce2.h>

#define maxLEDs 5
#define minIntervalMS 1000
#define plusIntervalMS 500

typedef struct myTimer_t {
	uint32_t oldTime;
	uint32_t newTime;
	uint32_t intervalMS;
};

typedef struct state_t {
	uint32_t interval;
	uint32_t decInterval;
	byte molesWhacked;
	bool missed;
	bool start;
	bool led[maxLEDs];
};

const byte butPin[maxLEDs] = {2, 3, 4, 5, 6};
const byte ledPin[maxLEDs] = {9, 10, 11, 12, 13};

Bounce button[maxLEDs] = {
	Bounce(butPin[0], maxLEDs),
	Bounce(butPin[1], maxLEDs),
	Bounce(butPin[2], maxLEDs),
	Bounce(butPin[3], maxLEDs),
	Bounce(butPin[4], maxLEDs)
};

void setup() {
	for(byte i = 0; i < maxLEDs; i++) {
		pinMode(ledPin[i], OUTPUT);
		pinMode(butPin[i], INPUT_PULLUP);
		button[i].attach(butPin[i]);
	}
	randomSeed(analogRead(0));
	Serial.begin(9600);
}

void loop() {
	//myTimer_t timer;
	state_t state;
	while(true) {
		delayInput();
		reset(state);
		Serial.println("Select speed of new game.");
		Serial.println("Very slow to very fast with button left to right.");
		setGameSpeed(state);
		playGame(state);
		Serial.println("GAME OVER!");
	}
}

void reset(state_t &s) {
	s.missed = false;
	s.start = false;
	s.decInterval = 100;
	s.molesWhacked = 0;
	LEDsOff(s);
}

void setGameSpeed(state_t &s) {
	myTimer_t sweepTimer;
	setTimer(sweepTimer, 75);
	byte strSize = 35;
	char strOut[strSize];

	while(!s.start) {
		for(byte i = 0; i < maxLEDs; i++) {
			if(getButton(i)) {
				s.interval = minIntervalMS + (i * plusIntervalMS);
				s.start = true;
			}
			if(getTimer(sweepTimer)) {
				s.led[i] = flipLED(i, s.led[i]);
			}
		}
	}
	LEDsOff(s);
}

void playGame(state_t &state) {
	myTimer_t gameTimer;
	byte strSize = 36;
	char strOut[strSize];
	byte mole;
	bool ok;
	
	countDown(state);
	// start rounds
	while(!state.missed) {
		mole = byte(random(0, 5));
		state.led[mole] = flipLED(mole, state.led[mole]);
		setTimer(gameTimer, state.interval);
		ok = false;
		while(!getTimer(gameTimer)) {
			for(byte i = 0; i < maxLEDs; i++) {
				if(getButton(i)) {
					if(i == mole) {
						state.molesWhacked++;
						state.interval -= state.decInterval;
						state.led[mole] = flipLED(mole, state.led[mole]);
						snprintf(strOut, strSize, "Moles Hit: %3d || New interval %4d", state.molesWhacked, state.interval);
						Serial.println(strOut);
						ok = true;
						break;
					} else {
						state.missed = true;
					}
				}
			}
		}
		if(!ok) {
			state.missed = true;
		}
	}
}

void countDown(state_t &s) {
	myTimer_t countDownTimer;
	setTimer(countDownTimer, 500);
	byte flashes = 6;
	bool count = true;
	
	while(count) {
		if(getTimer(countDownTimer)) {
			flashes--;
			for(byte i = 0; i < maxLEDs; i++) {
				s.led[i] = flipLED(i, s.led[i]);
			}
		}
		if(flashes == 0) {
			count = false;
		} 
	}
}

void delayInput() {
	myTimer_t delayTimer;
	setTimer(delayTimer, 1000);
	while(!getTimer(delayTimer)) {
		flipLED(random(0, 5), random(0,1));
	}
}

void setTimer(myTimer_t &s, uint32_t freq) {
	s.oldTime = millis();
	s.newTime = s.oldTime;
	s.intervalMS = freq;
}

bool getTimer(myTimer_t &t){
	t.newTime = millis();
	if(t.newTime > (t.oldTime + t.intervalMS)){
		t.oldTime = t.newTime;
		return true;
	} else {
		return false;
	}
}

bool getButton(byte b) { 
	if(button[b].update() && button[b].read() == LOW){ 
		return true;
	} else {
		return false;
	}
}

bool flipLED(byte b, bool state) {
	digitalWrite(ledPin[b], !state);
	return !state;
}

void LEDsOff(state_t &s) {
	for(byte i = 0; i < maxLEDs; i++) {
		s.led[i] = false;
		digitalWrite(ledPin[i], s.led[i]);
	}
}