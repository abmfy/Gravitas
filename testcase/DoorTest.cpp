#include <iostream>
using std::cout, std::endl;

#include "Door.h"

Door door;

void printStatus() {
    cout << "Now door is " << (door.isOpen() ? "" : "not ") << "opened." << endl;
}

void printColor() {
    cout << "The color of door is now (" << door.getColor().r << ", " << door.getColor().g << ", " << door.getColor().b << ")" << endl;
}

int main() {
    cout << "Testing Door" << endl;
    printStatus();
    printColor();
    cout << "Open the door!" << endl;
    door.open();
    printStatus();
    printColor();
    cout << "Open the door again." << endl;
    door.open();
    printStatus();
    printColor();
}