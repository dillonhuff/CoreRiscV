#include "coreir.h"
#include "coreir/libs/rtlil.h"
#include "coreir/passes/transform/deletedeadinstances.h"
#include "coreir/passes/transform/unpackconnections.h"
#include "coreir/passes/transform/packconnections.h"

using namespace CoreIR;
using namespace std;

void processTop(const std::string& fileName,
                const std::string& topModName) {
  Context* c = newContext();

  CoreIRLoadLibrary_rtlil(c);

  Module* topMod = nullptr;

  if (!loadFromFile(c, fileName, &topMod)) {
    cout << "Could not Load from json!!" << endl;
    c->die();
  }

  topMod = c->getGlobal()->getModule(topModName);
  c->setTop(topMod);

  assert(topMod->hasDef());

  c->runPasses({"rungenerators",
        "removeconstduplicates",
        "flatten",
        "removeconstduplicates",
        "packconnections"});

  cout << "Flattened core" << endl;
  cout << "# of instances in " topMod->getName() << " = " << topMod->getDef()->getInstances().size() << endl;

  if (!saveToFile(c->getGlobal(), "risc5Processed.json", topMod)) {
    cout << "Could not save to json!!" << endl;
    c->die();
  }

  cout << "Saved to file" << endl;

  SimulatorState state(topMod);

  cout << "Got simulator state for top module" << endl;
  
  deleteContext(c);
}

int main() {
  string fileName = "__DOLLAR__paramod__DOLLAR__4d2dfdcc1db1a7362453fb449ccdda75bb1b39f9__BACKSLASH__picorv32.json";
  string topMod = "__DOLLAR__paramod__DOLLAR__4d2dfdcc1db1a7362453fb449ccdda75bb1b39f9__BACKSLASH__picorv32";
  processTop(fileName, topMod);
}
