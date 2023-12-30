#include <Input.h>

QwiicButton greenButton1;
QwiicButton greenButton2;
QwiicButton redButton;

QwiicButton* btns[] = {&greenButton1, &greenButton2, &redButton};

bool input(Button btn, Press press) {
    while (!btns[btn]->isClickedQueueEmpty()) {
        uint32_t start = btns[btn]->popPressedQueue();
        uint32_t end = btns[btn]->popClickedQueue();
        if (start >= PRESS_AGE_THRESHOLD)
            continue;  // disregard too old presses
        if (press == LONG_PRESS) return (start - end >= LONG_PRESS_DURATION);
        return true;
    }
    return false;
}
