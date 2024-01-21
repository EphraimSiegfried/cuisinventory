#include <Input.h>

QwiicButton greenButton1;
QwiicButton greenButton2;
QwiicButton redButton;

QwiicButton* btns[] = {&greenButton1, &greenButton2, &redButton};

bool input(Button btn, Press press) {
    uint32_t start = btns[btn]->timeSinceLastPress();
    btns[btn]->popPressedQueue();  // ms since button down
    uint32_t end = btns[btn]->timeSinceLastClick();
    btns[btn]->popClickedQueue();  // ms since button up
                                   // LOG("start" + String(start));
                                   // LOG("end" + String(end));
    if (start >= PRESS_AGE_THRESHOLD) {
        return false;  // disregard too old presses
    }
    if (press == LONG_PRESS) {
        return (start - end >= LONG_PRESS_DURATION);
    }
    return true;
    return false;
}
