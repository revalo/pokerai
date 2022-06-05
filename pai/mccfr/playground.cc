#include <iostream>
#include <thread>

using namespace std;

void testThread() { std::cout << "I am a seperate thread..."; }

void main() {
  int playerIndex = 4;
  int maxPlayers = 3;
  cout << "Coerced player index: "
       << (maxPlayers + (playerIndex % maxPlayers)) % maxPlayers << endl;
  return;
  cout << "Hello, World!" << endl;
  thread newThread(testThread);
  newThread.join();
}
