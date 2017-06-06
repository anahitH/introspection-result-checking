#include <stdio.h>

#define PROTECTED __attribute__((annotate("protected")))

void display() {

}

void calc() {

}

void notify() {
}

void encrypt(int i);

PROTECTED
void readKey() {
  printf("Super Secret\n");
  static int i = 0;
  encrypt(++i);
}

void encrypt(int i) {
  if (i < 4) {
      readKey();
  }
}

void decrypt() {
  readKey();
}

void loadUI() {
  display();
}

void operation1() {
  calc();
  notify();
}

void operation2() {
  encrypt(0);
  decrypt();
}

int main() {
//    for (unsigned i = 0; i < 10000; ++i) {
        loadUI();
        operation1();
        operation2();
//    }
}
