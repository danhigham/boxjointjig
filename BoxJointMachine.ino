#include <EncoderButton.h>
// EncoderButton eb1(51, 49, 53);
EncoderButton eb1(50, 48, 52);

#include <Encoder.h>

#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
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

#define R_CLK_PIN  48
#define R_DATA_PIN 50

uint16_t version = MCUFRIEND_KBV_H_;

void testFunction() {
  Serial.println("foo");
}

void testFunction2() {
  Serial.println("bar");
}

typedef void (*menuFunction)();

typedef struct menuItem {
  String caption;
  menuFunction function;
  menuItem *children[10];
};

struct menuItem topLevel;
menuItem *currentMenuItem;
menuItem *previousMenuItem;
int currentMenuSelectionIdx;

void getChildMenuCount(int* count) {
  for (int i=0; i < 10; i++) {
    if (currentMenuItem->children[i] == NULL) break;
    *count = i+1;
  } 
}

void getTextDimensions(String text, uint16_t* w, uint16_t* h) {
  int16_t  x1, y1;
  tft.getTextBounds(text, 0, 0, &x1, &y1, w, h);
}

void dummyHandler(){};

void drawScreen(bool drawTitle) {

  uint16_t text_w, text_h;
  uint16_t title_w, title_h;

  tft.setTextSize(2);
  getTextDimensions("AAAA", &text_w, &text_h);

  tft.setTextSize(3);
  getTextDimensions(currentMenuItem->caption, &title_w, &title_h);

  int width = tft.width();
  int height = tft.height();

  // tft.drawRect(0, 0, width, height, WHITE);
  tft.drawRect(SCREEN_MARGIN, SCREEN_MARGIN, width - (SCREEN_MARGIN * 2), height - (SCREEN_MARGIN * 2), WHITE);
  
  if (drawTitle) {
    tft.setTextSize(3);
    tft.setTextColor(WHITE);

    uint16_t offset = (width / 2) - (title_w / 2);
    tft.setCursor(offset, 5);
    tft.setTextColor(BLUE);
    tft.print(previousMenuItem->caption);
    tft.setCursor(offset, 5);
    tft.setTextColor(WHITE);
    tft.print(currentMenuItem->caption);
  }

  tft.setTextSize(2);
  
  for (int i=0; i < 10; i++) {
    if (currentMenuItem->children[i] != NULL) {
      tft.fillRect(SCREEN_MARGIN + 1, SCREEN_MARGIN + ((text_h + VERTICAL_TEXT_SPACING) * i) + 1, 
        width - (SCREEN_MARGIN * 2) - 2, text_h + VERTICAL_TEXT_SPACING - 2, BLUE);
      if (i == currentMenuSelectionIdx) {
        tft.fillRect(SCREEN_MARGIN + 1, SCREEN_MARGIN + ((text_h + VERTICAL_TEXT_SPACING) * i) + 1, 
          width - (SCREEN_MARGIN * 2) - 2, text_h + VERTICAL_TEXT_SPACING - 2, WHITE);
        tft.setTextColor(BLACK);
      } else {
        tft.setTextColor(WHITE);
      }
      tft.setCursor(TEXT_SCREEN_MARGIN, TEXT_SCREEN_MARGIN + ((text_h + VERTICAL_TEXT_SPACING) * i));
      tft.print(currentMenuItem->children[i]->caption);
    }
  }
}

void onEb1Clicked(EncoderButton& eb) {
  Serial.print("eb1 clickCount: ");
  Serial.println(eb.clickCount());
}

void onEb1Encoder(EncoderButton& eb) {
  int oldIndex = currentMenuSelectionIdx;
  int childCount;
  int inc = eb.increment();
  
  getChildMenuCount(&childCount);
  Serial.print("Child count:");
  Serial.println(childCount);
  
  currentMenuSelectionIdx = currentMenuSelectionIdx + inc; 
  
  if (currentMenuSelectionIdx < 0) currentMenuSelectionIdx = 0;
  if (currentMenuSelectionIdx >= childCount-1) currentMenuSelectionIdx = childCount-1;

  Serial.print("INC:");
  Serial.println(eb.increment());

  Serial.print("INDEX:");
  Serial.println(currentMenuSelectionIdx);

  if (currentMenuSelectionIdx != oldIndex) {
    drawScreen(false);
  }  
  // Serial.println("************");
  // Serial.print("eb1 incremented by: ");
  // Serial.println(eb.increment());
  // Serial.print("eb1 position is: ");
  // Serial.println(eb.position());
}

void setup() {
  pinMode(48, INPUT_PULLUP);
  pinMode(50, INPUT_PULLUP);
  uint16_t ID = tft.readID();

  tft.begin(ID);
  tft.setRotation(1);

  int width = tft.width();
  int height = tft.height();
  tft.fillScreen(BLUE);
  
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("hello...\n");

  topLevel = {"*** Box Jointer ***", testFunction, {
    new menuItem{"Settings", testFunction2},
    new menuItem{"Programs", testFunction},
    new menuItem{"Control", testFunction},
    new menuItem{"Run", testFunction},
  }};
  
  currentMenuItem = &topLevel;
  previousMenuItem = &topLevel;
  currentMenuSelectionIdx = 0;

  drawScreen(true);

  //Link the event(s) to your function
  eb1.setClickHandler(onEb1Clicked);
  eb1.setEncoderHandler(onEb1Encoder);
}

void loop() {
  eb1.update();
  delay(10);
}
