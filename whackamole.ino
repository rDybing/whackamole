#include <Bounce2.h>
#include <LiquidCrystal.h>

#define maxLEDs 4
#define minIntervalMS 1000
#define plusIntervalMS 500
#define decIntervalMS 25

typedef struct myTimer_t {
	uint32_t oldTime;
	uint32_t newTime;
	uint32_t intervalMS;
};

typedef struct state_t {
	uint32_t interval;
	byte molesWhacked;
	bool missed;
	bool start;
	bool led[maxLEDs];
};

const byte rs = 3, en = 2, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
const byte butPin[maxLEDs] = {6, 7, 8, 9};
const byte ledPin[maxLEDs] = {10, 11, 12, 13};

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

Bounce button[maxLEDs] = {
	Bounce(butPin[0], maxLEDs),
	Bounce(butPin[1], maxLEDs),
	Bounce(butPin[2], maxLEDs),
	Bounce(butPin[3], maxLEDs)
};

void setup() {
	for(byte i = 0; i < maxLEDs; i++) {
		pinMode(ledPin[i], OUTPUT);
		pinMode(butPin[i], INPUT_PULLUP);
		button[i].attach(butPin[i]);
	}
	randomSeed(analogRead(0));
	lcd.begin(16, 2);
}

void loop() {
	state_t state;
	while(true) {
		reset(state);
		setGameSpeed(state);
		playGame(state);
		delayInput();
	}
}

void reset(state_t &s) {
	s.missed = false;
	s.start = false;
	s.molesWhacked = 0;
	LEDsOff(s);
	lcdMenu();
}

void setGameSpeed(state_t &s) {
	myTimer_t sweepTimer;
	setTimer(sweepTimer, 75);

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
	lcdGameStart(s.interval);
}

void playGame(state_t &state) {
	myTimer_t gameTimer;
	byte mole;
	bool ok;
	
	countDown(state);
	lcdGameConst();
	// start rounds
	while(!state.missed) {
		mole = byte(random(0, maxLEDs));
		state.led[mole] = flipLED(mole, state.led[mole]);
		setTimer(gameTimer, state.interval);
		ok = false;
		while(!getTimer(gameTimer)) {
			for(byte i = 0; i < maxLEDs; i++) {
				if(getButton(i)) {
					if(i == mole) {
						state.molesWhacked++;
						state.interval -= decIntervalMS;
						state.led[mole] = flipLED(mole, state.led[mole]);
						lcdGameState(state.molesWhacked, state.interval);
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
	lcdGameOver(state.molesWhacked, state.interval);
}

void lcdMenu() {
	lcd.clear();
	lcd.print("Set Game Speed:");
	lcd.setCursor(0,1);
	lcd.print("Button 1 -> 4");
}

void lcdGameStart(uint32_t interval) {
	byte strSize = 16;
	char strInterval[strSize];
	snprintf(strInterval, strSize, "Interval: %4d", interval);
	lcd.clear();
	lcd.print("Game Starting!");
	lcd.setCursor(0,1);
	lcd.print(strInterval);
}

void lcdGameConst() {
	lcd.clear();
	lcd.print("Moles Hit:");
	lcd.setCursor(0,1);
	lcd.print("Interval :");
}

void lcdGameState(byte moles, uint32_t interval) {
	byte strSize = 5;
	char strMoles[strSize];
	char strInterval[strSize];
	snprintf(strMoles, strSize, "%3d", moles);
	snprintf(strInterval, strSize, "%4d", interval);
	lcd.setCursor(12,0);
	lcd.print(strMoles);
	lcd.setCursor(11,1);
	lcd.print(strInterval);
}

void lcdGameOver(byte moles, uint32_t interval) {
	byte strSize = 16;
	char strScore[strSize];
	snprintf(strScore, strSize, "M: %3d-I: %4d", moles, interval);
	lcd.clear();
	lcd.print("GAME OVER!");
	lcd.setCursor(0,1);
	lcd.print(strScore);
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
	setTimer(delayTimer, 5000);
	while(!getTimer(delayTimer)) {
		flipLED(random(0, maxLEDs), random(0,1));
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