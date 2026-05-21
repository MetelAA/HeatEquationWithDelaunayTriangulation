#include <iostream>

int main() {
    std::cout << "Hello, World!" << std::endl;


    int a = 5;
    int& b = a;

    int& c = b;
    c = 5;
    return 0;
}
