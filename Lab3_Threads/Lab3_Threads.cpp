#include "ThreadController.h"
#include <iostream>

int main() {
    int size, count;
    std::cout << "Enter array size: ";
    std::cin >> size;
    std::cout << "Enter marker threads count: ";
    std::cin >> count;

    ThreadController controller(size, count);
    controller.Run();

    return 0;
}