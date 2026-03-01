
// === BUTTON LIBRARY VARIABLES ===

// Tracks the current debounced state of the button (0 = released, 1 = pressed)
uint8_t buttonState = 0;

// Records the time (in ms) when the button was pressed
uint32_t buttonOn = 0;

// Stores the duration (in ms) of the last button press
uint32_t buttonTime = 0;


// === INITIALIZATION FUNCTION ===

void Button_Init()
{
  // Configure the button pin as an input
  pinMode(PIN_BUTTON, INPUT);
}


// === MAIN BUTTON LOOP FUNCTION ===
// Call this function repeatedly (e.g., in loop()).
// Returns:
// 0 = no new event
// 1 = short press detected (between 10 and 500 ms)
// 2 = long press detected (over 500 ms)

int Button_Loop()
{
  uint8_t buttonpressed = 0; // 0 = nothing, 1 = short, 2 = long

  // Read current button state (1 = pressed, 0 = released)
  uint8_t button = digitalRead(PIN_BUTTON); 

  // --- Detect Button Press (rising edge) ---
  if ((buttonState == 0) && (button == 1))
  {
    // Button was just pressed
    buttonState = 1;           // Mark as pressed
    buttonOn = millis();       // Save the press timestamp
  } 

  // --- Detect Button Release (falling edge) ---
  if ((buttonState == 1) && (button == 0))
  {
    // Button was just released
    buttonState = 0;             // Mark as released
    buttonTime = millis() - buttonOn; // Calculate press duration

    // --- Classify the button press ---
    if (buttonTime > 500)
    {
      buttonpressed = 2; // Long press
    }

    if ((buttonTime > 10) && (buttonTime < 500))
    {
      buttonpressed = 1; // Short press
    }
    // (button presses shorter than 10 ms are ignored as likely noise)
  } 

  // Return the detected press type (or 0 if no event)
  return buttonpressed;  
}
