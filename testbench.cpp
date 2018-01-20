#include "coreir.h"
#include "coreir/libs/rtlil.h"
#include "coreir/passes/transform/deletedeadinstances.h"
#include "coreir/passes/transform/unpackconnections.h"
#include "coreir/passes/transform/packconnections.h"

using namespace CoreIR;
using namespace std;

std::vector<char> hexToBytes(const std::string& hex) {
  std::vector<char> bytes;

  for (unsigned int i = 0; i < hex.length(); i += 2) {
    std::string byteString = hex.substr(i, 2);
    char byte = (char) strtol(byteString.c_str(), NULL, 16);
    bytes.push_back(byte);
  }

  return bytes;
}

BitVector hexStringToBitVector(const std::string& str) {
  vector<char> addrBytes = hexToBytes(str);

  assert(addrBytes.size() == 4);

  reverse(addrBytes);

  BitVector configAddr(32, 0);

  int offset = 0;
  for (auto byte : addrBytes) {
    BitVec tmp(8, byte);
    for (uint i = 0; i < (uint) tmp.bitLength(); i++) {
      configAddr.set(offset, tmp.get(i));
      offset++;
    }
  }

  assert(offset == 32);

  return configAddr;
}

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
        "cullzexts",
        "clockifyinterface",
        "deletedeadinstances",
        "packconnections"});

  foldConstants(topMod);

  cout << "Flattened core" << endl;
  cout << "# of instances in " << topMod->getName() << " = " << topMod->getDef()->getInstances().size() << endl;

  if (!saveToFile(c->getGlobal(), "risc5Processed.json", topMod)) {
    cout << "Could not save to json!!" << endl;
    c->die();
  }

  cout << "Saved to file" << endl;

  SimulatorState state(topMod);
  state.setMainClock("self.clk");

  cout << "Got simulator state for top module" << endl;

  state.setClock("self.clk", 0, 1);

  state.setClock("self.pcpi_wr", 0, 0);

  state.setValue("self.irq", BitVec(32, 0));
  state.setValue("self.mem_rdata", BitVec(32, 0));
  state.setValue("self.mem_ready", BitVec(1, 0));

  state.setValue("self.pcpi_rd", BitVec(32, 0));
  state.setValue("self.pcpi_ready", BitVec(1, 0));
  state.setValue("self.pcpi_wait", BitVec(1, 0));
  state.setValue("self.resetn", BitVec(1, 0));
  
  cout << "Executing core" << endl;

  state.execute();
  state.execute();
  
  deleteContext(c);
}

void simulateState(const std::string& fileName,
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

  SimulatorState state(topMod);
  state.setMainClock("self.clk");

  cout << "Got simulator state for top module" << endl;

  state.setClock("self.clk", 0, 1);

  state.setClock("self.pcpi_wr", 0, 0);

  state.setValue("self.irq", BitVec(32, 0));
  state.setValue("self.mem_rdata", BitVec(32, 0));
  state.setValue("self.mem_ready", BitVec(1, 0));

  state.setValue("self.pcpi_rd", BitVec(32, 0));
  state.setValue("self.pcpi_ready", BitVec(1, 0));
  state.setValue("self.pcpi_wait", BitVec(1, 0));
  state.setValue("self.resetn", BitVec(1, 0));
  
  cout << "Executing core" << endl;

  state.execute();
  state.execute();
  
  deleteContext(c);

}

int main() {
  string fileName = "picorv32.json";//"__DOLLAR__paramod__DOLLAR__4d2dfdcc1db1a7362453fb449ccdda75bb1b39f9__BACKSLASH__picorv32.json";
  string topMod = "picorv32"; //"__DOLLAR__paramod__DOLLAR__4d2dfdcc1db1a7362453fb449ccdda75bb1b39f9__BACKSLASH__picorv32";
  //processTop(fileName, topMod);
  simulateState("risc5Processed.json", topMod);
}
