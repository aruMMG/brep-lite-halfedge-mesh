#include <iostream>
#include <vector>

int main() {
    std::vector<std::string> msg {"Hello", "C++", "on", "Ubuntu", "without", "sudo!"};
    for (const std::string& word : msg) {
        std::cout << word << " ";
    }
    std::cout << std::endl;
    return 0;
}