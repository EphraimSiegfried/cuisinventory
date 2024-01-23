#include <Input.h>

QwiicButton greenButton1;
QwiicButton greenButton2;
QwiicButton redButton;

QwiicButton* btns[] = {&greenButton1, &greenButton2, &redButton};

bool input(Button btn, Press press) {
    if (btns[btn]->hasBeenClicked()) {
        btns[btn]->clearEventBits();
        return true;
    }
    // while (!btns[btn]->isClickedQueueEmpty() &&
    // !btns[btn]->isPressedQueueEmpty()) {
    //     uint32_t start = btns[btn]->popPressedQueue();  // ms since button
    //     down uint32_t end = btns[btn]->popClickedQueue();    // ms since
    //     button up
    //     // if (start >= PRESS_AGE_THRESHOLD)
    //     //     continue;  // disregard too old presses
    //     if (press == LONG_PRESS) return (start - end >= LONG_PRESS_DURATION);
    //     return true;
    // }
    // return false;
    return false;
}
