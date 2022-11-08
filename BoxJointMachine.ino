
#include <EncoderButton.h>
EncoderButton eb1(51, 49, 53);

#include <UTFTGLUE.h>              //use GLUE class and constructor
UTFTGLUE myGLCD(0,A2,A1,A3,A4,A0); //all dummy args

/**
 * A function to handle the 'clicked' event
 * Can be called anything but requires EncoderButton& 
 * as its only parameter.
 * I tend to prefix with 'on' and suffix with the 
 * event type.
 */
void onEb1Clicked(EncoderButton& eb) {
  Serial.print("eb1 clickCount: ");
  Serial.println(eb.clickCount());
}

/**
 * A function to handle the 'encoder' event
 */
void onEb1Encoder(EncoderButton& eb) {
  Serial.print("eb1 incremented by: ");
  Serial.println(eb.increment());
  Serial.print("eb1 position is: ");
  Serial.println(eb.position());
}

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


menuItem *currentMenuItem;
int currentMenuSelectionIdx;

void drawScreen() {
  myGLCD.clrScr();

  myGLCD.setColor(255, 0, 0);
  myGLCD.fillRect(0, 0, 479, 20);

  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(255, 0, 0);

  myGLCD.setTextSize(100);

  myGLCD.print(currentMenuItem->caption, CENTER, 3);
  myGLCD.setBackColor(0, 0, 255);

  for (int i=0; i < 10; i++) {
    if (currentMenuItem->children[i] != NULL) {
      currentMenuItem->children[i]->function();
      myGLCD.print(currentMenuItem->children[i]->caption, LEFT, 30*(i+1));
    }
  }

  
  // delay(500);
}

void setup() {

  randomSeed(analogRead(0));
  
  // Setup the LCD
  myGLCD.InitLCD();
  myGLCD.setFont(BigFont);

  // put your setup code here, to run once:
  Serial.begin(9600);

  struct menuItem topLevel = {"*** Box Jointer ***", testFunction, {
    new menuItem{"hello2", testFunction2},
    new menuItem{"hello3", testFunction},
    new menuItem{"hello4", testFunction}    
  }};
  
  currentMenuItem = &topLevel;
  currentMenuSelectionIdx = 0;

  drawScreen();

  //Link the event(s) to your function
  eb1.setClickHandler(onEb1Clicked);
  eb1.setEncoderHandler(onEb1Encoder);
}



void loop() {
  // drawScreen();
  eb1.update();
}
