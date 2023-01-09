#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

#include <AceButton.h>
#include <FlexyStepper.h>
#include "boxjointmachine.h"

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define DARK_BLUE 0x085d
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY    0x8410

#define SCREEN_MARGIN 32
#define TEXT_SCREEN_MARGIN 40
#define VERTICAL_TEXT_SPACING 10

#define SETTING_TXT_SIZE 10
#define TITLE_TXT_SIZE 3
#define MENU_ITEM_TXT_SIZE 2

#define BG_COLOR DARK_BLUE

#define CLK 48
#define DATA 50
#define BUTTON 52

#define MOTOR_STEP_PIN 51
#define MOTOR_DIRECTION_PIN 53

#define PORTRAIT  0
#define LANDSCAPE 1

#define ORIENTATION  LANDSCAPE

FlexyStepper stepper;

void testFunction() {
  Serial.println("foo");
}

using namespace ace_button;
AceButton button(BUTTON);
void handleEvent(AceButton*, uint8_t, uint8_t);

uint16_t version = MCUFRIEND_KBV_H_;
int lcd_height, lcd_width;
uint16_t text_w, text_h;
uint16_t title_w, title_h;

float p_kerfSize = 3.175;
float p_toothSize = 5;
float p_jogSize = .5;

float currentPosition = 0;

menuItem topLevel;
menuItem *currentMenuItem, *previousMenuItem;
int previousMenuSelectionIdx, currentMenuSelectionIdx;

int buttonState;            // the current reading from the input pin
int lastButtonState = LOW;  // the previous reading from the input pin

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void getChildMenuCount(int* count, menuItem* targetMenuItem) {
  *count = 0;
  for (int i=0; i < 10; i++) {
    if (targetMenuItem->children[i] == NULL) break;
    *count = i+1;
  } 
}

void getTextDimensions(String text, uint16_t* w, uint16_t* h) {
  int16_t  x1, y1;
  tft.getTextBounds(text, 0, 0, &x1, &y1, w, h);
}

void eraseMenuItems(menuItem *targetMenuItem) {
  drawTitle(targetMenuItem->caption, BG_COLOR);

  tft.setTextSize(MENU_ITEM_TXT_SIZE);
  
  for (int i=0; i < 10; i++) {
    if (targetMenuItem->children[i] != NULL) {
      if (i == currentMenuSelectionIdx) {
        tft.fillRect(SCREEN_MARGIN + 1, SCREEN_MARGIN + ((text_h + VERTICAL_TEXT_SPACING) * i) + 1, 
          lcd_width - (SCREEN_MARGIN * 2) - 2, text_h + VERTICAL_TEXT_SPACING - 2, BG_COLOR);
        continue;
      }
      tft.setCursor(TEXT_SCREEN_MARGIN, TEXT_SCREEN_MARGIN + ((text_h + VERTICAL_TEXT_SPACING) * i));
      tft.print(targetMenuItem->children[i]->caption);
    }
  }    
}

void erasePrevious() {
  eraseMenuItems(previousMenuItem);
}

void eraseCurrent() {
  eraseMenuItems(currentMenuItem);
}

void drawScreen(bool addTitle=false) {

  if (addTitle) drawTitle(currentMenuItem->caption, WHITE);

  tft.setTextSize(MENU_ITEM_TXT_SIZE);
  
  for (int i=0; i < 10; i++) {
    if (currentMenuItem->children[i] != NULL) {
      if (i == previousMenuSelectionIdx) tft.fillRect(SCREEN_MARGIN + 1, SCREEN_MARGIN + ((text_h + VERTICAL_TEXT_SPACING) * i) + 1, 
        lcd_width - (SCREEN_MARGIN * 2) - 2, text_h + VERTICAL_TEXT_SPACING - 2, BG_COLOR);
      if (i == currentMenuSelectionIdx) {
        tft.fillRect(SCREEN_MARGIN + 1, SCREEN_MARGIN + ((text_h + VERTICAL_TEXT_SPACING) * i) + 1, 
          lcd_width - (SCREEN_MARGIN * 2) - 2, text_h + VERTICAL_TEXT_SPACING - 2, WHITE);
        tft.setTextColor(BG_COLOR);
      } else {
        tft.setTextColor(WHITE);
      }
      tft.setCursor(TEXT_SCREEN_MARGIN, TEXT_SCREEN_MARGIN + ((text_h + VERTICAL_TEXT_SPACING) * i));
      tft.print(currentMenuItem->children[i]->caption);
    }
  }
}

void selectItem() {
  // switch menu items
  int childCount;
  getChildMenuCount(&childCount, currentMenuItem->children[currentMenuSelectionIdx]);
  if (childCount > 0) {
    previousMenuItem = currentMenuItem;
    erasePrevious();
    // switch menu items out
    currentMenuItem = currentMenuItem->children[currentMenuSelectionIdx];
    currentMenuSelectionIdx = 0;
    drawScreen(true);
  } else if (currentMenuItem->function != NULL) currentMenuItem->children[currentMenuSelectionIdx]->function();
}

void setKerfWidth() {
  setValue(&p_kerfSize, 0.025);
}

void setToothSize() {
  setValue(&p_toothSize, 0.5);
}

void setJogLength() {
  setValue(&p_jogSize, 0.1);
}

void setHome() {
  stepper.setCurrentPositionInMillimeters(0);
  currentPosition = 0;
}

void goHome() {
  stepper.moveToPositionInMillimeters(0);
  currentPosition = 0;
}
void jogPosition() {
  uint16_t setting_title_w, setting_title_h;
  eraseCurrent();

  drawTitle(currentMenuItem->children[currentMenuSelectionIdx]->caption, WHITE);

  // draw screen for value
  drawSettingValue(&currentPosition, 2, WHITE);

  int8_t val;

  while (true) {
    if( val=read_rotary() ) {
      drawSettingValue(&currentPosition, 2, BG_COLOR);
      if (val > 0) {
        currentPosition += p_jogSize;
      } else {
        currentPosition -= p_jogSize;
      }
      drawSettingValue(&currentPosition, 2, WHITE);

      stepper.moveToPositionInMillimeters(currentPosition);
    }

    // check for button push to move back to previous menu
    if (button.isPressedRaw()) {
      drawSettingValue(&currentPosition, 2, BG_COLOR);
      drawTitle(currentMenuItem->children[currentMenuSelectionIdx]->caption, BG_COLOR);

      drawScreen(true);
      break;
    }
  }
}

void moveTo() {
  uint16_t setting_title_w, setting_title_h;
  eraseCurrent();

  drawTitle(currentMenuItem->children[currentMenuSelectionIdx]->caption, WHITE);

  // draw screen for value
  drawSettingValue(&currentPosition, 2, WHITE);

  int8_t val;

  while (true) {
    if( val=read_rotary() ) {
      drawSettingValue(&currentPosition, 2, BG_COLOR);
      if (val > 0) {
        currentPosition += 1;
      } else {
        currentPosition -= 1;
      }
      drawSettingValue(&currentPosition, 2, WHITE);
    }

    // check for button push to move back to previous menu
    if (button.isPressedRaw()) {
      stepper.moveToPositionInMillimeters(currentPosition);

      drawSettingValue(&currentPosition, 2, BG_COLOR);
      drawTitle(currentMenuItem->children[currentMenuSelectionIdx]->caption, BG_COLOR);

      drawScreen(true);
      break;
    }
  }
}


void setValue(float *settingValue, float increment) {
  uint16_t setting_title_w, setting_title_h;
  eraseCurrent();

  drawTitle(currentMenuItem->children[currentMenuSelectionIdx]->caption, WHITE);
  
  // draw screen for setting
  drawSettingValue(settingValue, 2, WHITE);

  int8_t val;

  while (true) {
    if( val=read_rotary() ) {
      drawSettingValue(settingValue, 2, BG_COLOR);
      if (val > 0) {
        *settingValue += increment;
      } else {
        *settingValue -= increment;
      }
      drawSettingValue(settingValue, 2, WHITE);
    }

    // check for button push to move back to previous menu
    if (button.isPressedRaw()) {
      drawSettingValue(settingValue, 2, BG_COLOR);
      drawTitle(currentMenuItem->children[currentMenuSelectionIdx]->caption, BG_COLOR);

      drawScreen(true);
      break;
    }
  }
}

void drawTitle(String text, uint16_t color) {
  uint16_t setting_title_w, setting_title_h;

  tft.setTextSize(TITLE_TXT_SIZE);
  getTextDimensions(text, &setting_title_w, &setting_title_h);

  // draw title
  uint16_t offset = (lcd_width / 2) - (setting_title_w / 2);
  tft.setCursor(offset, 5);  
  tft.setTextColor(color);
  tft.print(text);
}

void drawSettingValue(float *settingValue, int precision, uint16_t color) {

  int setting_value_w, setting_value_h;
  char s[10];
  tft.setTextSize(SETTING_TXT_SIZE);
  tft.setTextColor(color);
  dtostrf(*settingValue, 4, precision, s);

  String sValue = String(s);
  getTextDimensions(sValue, &setting_value_w, &setting_value_h);
  
  uint16_t offset_x = (lcd_width / 2) - (setting_value_w / 2);
  uint16_t offset_y = (lcd_height / 2) - (setting_value_h / 2);

  tft.setCursor(offset_x, offset_y);  
  tft.print(sValue);

}

void goBack() {
  menuItem *temp = previousMenuItem;
  previousMenuItem = currentMenuItem;
  erasePrevious();
  currentMenuItem = temp;
  currentMenuSelectionIdx = 0;
  drawScreen(true);
}

void setup() {
  pinMode(CLK, INPUT);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DATA, INPUT);
  pinMode(DATA, INPUT_PULLUP);
  pinMode(BUTTON, INPUT_PULLUP);

  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
  stepper.setStepsPerMillimeter(205);
  stepper.setSpeedInMillimetersPerSecond(70.0);
  stepper.setAccelerationInMillimetersPerSecondPerSecond(150.0);

  uint16_t ID = tft.readID();

  tft.begin(ID);
  tft.setRotation(ORIENTATION);

  lcd_width = tft.width();
  lcd_height = tft.height();
  tft.fillScreen(BG_COLOR);
  
  // put your setup code here, to run once:
  Serial.begin(9600);

  button.setEventHandler(handleEvent);
  topLevel = {"Box Jointer", {
    new menuItem{"Settings", {
      new menuItem{"<- Go Back", {}, goBack},
      new menuItem{"Kerf Width", {}, setKerfWidth},
      new menuItem{"Tooth Size", {}, setToothSize},
      new menuItem{"Jog Length", {}, setJogLength},
      // new menuItem{"Setting Three", {}, testFunction},
      // new menuItem{"Setting Four", {}, testFunction},
      // new menuItem{"Setting Five", {}, testFunction},
      // new menuItem{"Setting Six", {}, testFunction},
    }, testFunction},
    new menuItem{"Programs", {}, testFunction},
    new menuItem{"Control", {
      new menuItem{"<- Go Back", {}, goBack},
      new menuItem{"Go Home", {}, goHome},
      new menuItem{"Set Home", {}, setHome},
      new menuItem{"Jog", {}, jogPosition},
      new menuItem{"Move", {}, moveTo},
    }, testFunction},
    new menuItem{"Run", {}, testFunction},
  }};
  
  currentMenuItem = &topLevel;
  previousMenuItem = &topLevel;
  currentMenuSelectionIdx = 0;

  tft.setTextSize(MENU_ITEM_TXT_SIZE);
  getTextDimensions("AAAA", &text_w, &text_h);

  tft.drawRect(SCREEN_MARGIN, SCREEN_MARGIN, lcd_width - (SCREEN_MARGIN * 2), lcd_height - (SCREEN_MARGIN * 2), WHITE);

  drawScreen(true);
}

static uint8_t prevNextCode = 0;
static uint16_t store=0;

// A vald CW or  CCW move returns 1, invalid returns 0.
int8_t read_rotary() {
  static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

  prevNextCode <<= 2;
  if (digitalRead(DATA)) prevNextCode |= 0x02;
  if (digitalRead(CLK)) prevNextCode |= 0x01;
  prevNextCode &= 0x0f;

   // If valid then store as 16 bit data.
   if  (rot_enc_table[prevNextCode] ) {
      store <<= 4;
      store |= prevNextCode;
      if ((store&0xff)==0x2b) return -1;
      if ((store&0xff)==0x17) return 1;
   }
   return 0;
}

void loop() {
  static int8_t val;
  if( val=read_rotary() ) {
    int oldIndex = currentMenuSelectionIdx;

    int childCount;
    getChildMenuCount(&childCount, currentMenuItem);
    currentMenuSelectionIdx += val;

    if (currentMenuSelectionIdx < 0) currentMenuSelectionIdx = 0;
    if (currentMenuSelectionIdx >= childCount-1) currentMenuSelectionIdx = childCount-1;
    
    if (oldIndex != currentMenuSelectionIdx) {
      previousMenuSelectionIdx = oldIndex;
      drawScreen();
    }
  }

  button.check();
}

void handleEvent(AceButton* /* button */, uint8_t eventType,
    uint8_t /* buttonState */) {
  switch (eventType) {
    case AceButton::kEventPressed:
      break;
    case AceButton::kEventReleased:
      selectItem();
      break;
  }
}
