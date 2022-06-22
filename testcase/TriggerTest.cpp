#include <iostream>
using std::cout, std::endl;

#include "Trigger.h"

void printStatus(const Trigger &trigger) {
    cout << "The trigger is " << (trigger.isTriggered() ? "" : "not ") << "triggered." << endl;
    cout << "The color of the trigger is (" << trigger.getColor().r << ", " << trigger.getColor().g << ", " << trigger.getColor().b << ")" << endl;
}

int main() {
    cout << "Testing Trigger" << endl;
    
    Trigger t1 {3};
    cout << "Creating a trigger with threshold 3" << endl;
    printStatus(t1);
    for (int i {1}; i <= 5; i++) {
        cout << "The " << i << "-th try to trigger:" << endl;
        t1.trigger();
        printStatus(t1);
    }

    Trigger t2 {0};
    cout << "Creating a trigger with threshold 0" << endl;
    printStatus(t2);
    for (int i {1}; i <= 5; i++) {
        cout << "The " << i << "-th try to trigger:" << endl;
        t2.trigger();
        printStatus(t2);
    }
}