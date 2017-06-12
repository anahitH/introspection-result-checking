#include <stdio.h>

void display() {
}

void calc() {

}

void notify() {
}

int readKey(char c) {
    printf("runing readKey\n");
    if (c == '^') {
        return 42;
    } else if (c == '*') {
        return 43;
    }
    return -1;
}

void encrypt() {
    readKey('a');
}

void decrypt() {
    readKey('n');
}

void loadUI() {
  display();
}

void operation1() {
  calc();
  notify();
}

void operation2() {
  encrypt();
  decrypt();
}

int main() {
//    for (unsigned i = 0; i < 10000; ++i) {
        loadUI();
        operation1();
        operation2();
//    }
}
