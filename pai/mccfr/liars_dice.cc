#include <iostream>
#include <thread>

using namespace std;

void testThread() { std::cout << "I am a seperate thread..."; }

void main() {
  cout << "Hello, World!" << endl;
  thread newThread(testThread);
  newThread.join();
}
