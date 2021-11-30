/*
   Button to Serial

   If seed button is pressed, play seed, record selection, and send seed selection over serial
   If interpolate button is pressed, send all seed selections over serial
   Make sure button is debounced
*/

// buttons
#define BUTTON_A 4
#define BUTTON_B 5
#define BUTTON_C 6
#define BUTTON_D 7
#define BUTTON_E 8
#define BUTTON_F 9
#define BUTTON_G 10
#define BUTTON_H 11
#define BUTTON_INTERPOLATE 2

// status LEDs
#define LED_A 12
#define LED_B 13
#define LED_C A0
#define LED_D A1
#define LED_E A2
#define LED_F A3
#define LED_G A4
#define LED_H A5
#define LED_INTERPOLATE 3

// state variables
// the current state of the output pin
int selectionStateInterpolate = HIGH;
int selectionStateA = HIGH;
int selectionStateB = HIGH;
int selectionStateC = HIGH;

// TODO: when these are plugged in, initialize to HIGH
int selectionStateD = LOW;
int selectionStateE = LOW;
int selectionStateF = LOW;
int selectionStateG = LOW;
int selectionStateH = LOW;

// the current reading from the input pin
int buttonStateA = LOW;
int buttonStateB = LOW;
int buttonStateC = LOW;
int buttonStateD = LOW;
int buttonStateE = LOW;
int buttonStateF = LOW;
int buttonStateG = LOW;
int buttonStateH = LOW;
int buttonStateInterpolate = LOW;

// the previous reading from the input pin
int lastButtonStateA = LOW;
int lastButtonStateB = LOW;
int lastButtonStateC = LOW;
int lastButtonStateD = LOW;
int lastButtonStateE = LOW;
int lastButtonStateF = LOW;
int lastButtonStateG = LOW;
int lastButtonStateH = LOW;
int lastButtonStateInterpolate = LOW;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_A, INPUT);
  pinMode(BUTTON_B, INPUT);
  pinMode(BUTTON_C, INPUT);
  pinMode(BUTTON_D, INPUT);
  pinMode(BUTTON_E, INPUT);
  pinMode(BUTTON_F, INPUT);
  pinMode(BUTTON_G, INPUT);
  pinMode(BUTTON_H, INPUT);
  pinMode(BUTTON_INTERPOLATE, INPUT);

  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_C, OUTPUT);
  pinMode(LED_D, OUTPUT);
  pinMode(LED_E, OUTPUT);
  pinMode(LED_F, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_H, OUTPUT);
  pinMode(LED_INTERPOLATE, OUTPUT);

  // set initial LED state
  digitalWrite(LED_A, selectionStateA);
  digitalWrite(LED_B, selectionStateB);
  digitalWrite(LED_C, selectionStateC);
  digitalWrite(LED_D, selectionStateD);
  digitalWrite(LED_E, selectionStateE);
  digitalWrite(LED_F, selectionStateF);
  digitalWrite(LED_G, selectionStateG);
  digitalWrite(LED_H, selectionStateH);
  digitalWrite(LED_INTERPOLATE, selectionStateInterpolate);

  debounced_seed_selection(BUTTON_A, LED_A, selectionStateA, buttonStateA, lastButtonStateA, 'a');
  debounced_seed_selection(BUTTON_B, LED_B, selectionStateB, buttonStateB, lastButtonStateB, 'b');
  debounced_seed_selection(BUTTON_C, LED_C, selectionStateC, buttonStateC, lastButtonStateC, 'c');
  delay(1000);
}

void debounced_seed_selection(int buttonPin, int ledPin, int &selectionState, int &buttonState, int &lastButtonState, char c) {
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle selection state if the new button state is HIGH
      if (buttonState == HIGH) {
        selectionState = !selectionState;

        // only send request if new selection state is HIGH
        if (selectionState == HIGH) {
          Serial.print("0 ");
          Serial.println(c);
          // wait for song to end
          // delay(4000);
        }
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;

  // set status led
  digitalWrite(ledPin, selectionState);
}


void debounced_interpolation_selection(int buttonPin, int ledPin, int &selectionState, int &buttonState, int &lastButtonState) {
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle selection state if the new button state is HIGH
      if (buttonState == HIGH) {

        // set status led
        digitalWrite(ledPin, HIGH);

        // create request message
        String request = String("1 ");
        if (selectionStateA == HIGH) {
          request += String("a ");
        }
        if (selectionStateB == HIGH) {
          request += String("b ");
        }
        if (selectionStateC == HIGH) {
          request += String("c ");
        }
        if (selectionStateD == HIGH) {
          request += String("d ");
        }
        if (selectionStateE == HIGH) {
          request += String("e ");
        }
        if (selectionStateF == HIGH) {
          request += String("f ");
        }
        if (selectionStateG == HIGH) {
          request += String("g ");
        }
        if (selectionStateH == HIGH) {
          request += String("h ");
        }

        if (request.length() >= 6) {
          // is valid interpolation request: has at least two selected seeds
          request += String("\n");
          Serial.print(request);
          // wait for song to end
          //delay(50000);
          delay(1000);
        }

        // set status led
        digitalWrite(ledPin, LOW);
      }
    }
  }
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}


void loop() {
  /* debounce button press
    set status LED
    send selection over serial
  */
  debounced_seed_selection(BUTTON_A, LED_A, selectionStateA, buttonStateA, lastButtonStateA, 'a');
  debounced_seed_selection(BUTTON_B, LED_B, selectionStateB, buttonStateB, lastButtonStateB, 'b');
  debounced_seed_selection(BUTTON_C, LED_C, selectionStateC, buttonStateC, lastButtonStateC, 'c');
  debounced_seed_selection(BUTTON_D, LED_D, selectionStateD, buttonStateD, lastButtonStateD, 'd');
  debounced_seed_selection(BUTTON_E, LED_E, selectionStateE, buttonStateE, lastButtonStateE, 'e');
  debounced_seed_selection(BUTTON_F, LED_F, selectionStateF, buttonStateF, lastButtonStateF, 'f');
  debounced_seed_selection(BUTTON_G, LED_G, selectionStateG, buttonStateG, lastButtonStateG, 'g');
  debounced_seed_selection(BUTTON_H, LED_H, selectionStateH, buttonStateH, lastButtonStateH, 'h');
  debounced_interpolation_selection(BUTTON_INTERPOLATE, LED_INTERPOLATE, selectionStateInterpolate, buttonStateInterpolate, lastButtonStateInterpolate);
}
