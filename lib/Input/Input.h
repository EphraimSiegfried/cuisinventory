#pragma once

#include <Constants.h>
#include <SparkFun_Qwiic_Button.h>
#include <Debug.h>

extern QwiicButton greenButton1;
extern QwiicButton greenButton2;
extern QwiicButton redButton;

typedef enum Button { GREEN_BUTTON1 = 0, GREEN_BUTTON2, RED_BUTTON } Button;
typedef enum Press { SHORT_PRESS = 0, LONG_PRESS } Press;

bool input(Button btn, Press press);
