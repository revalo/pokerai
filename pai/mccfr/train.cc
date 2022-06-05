#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "external_sampling_mccfr.h"
#include "game/liars_dice.h"
#include "infotable.h"

ABSL_FLAG(bool, big, false, "Include 'advanced' options in the menu listing");

using namespace std;

void main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);
  bool big_menu = absl::GetFlag(FLAGS_big);
  cout << "big: " << big_menu << endl;
}
