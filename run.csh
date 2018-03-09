yosys -p "proc; memory; pmuxtree; to_coreir" -m /Users/dillon/CppWorkspace/VerilogToCoreIR/to_coreir.so picorv32.v

coreir -i picorv32.json -o risc5Only.json -p rungenerators,flatten,removeconstduplicates,cullzexts,clockifyinterface,cullgraph --load_libs rtlil

make clean
make -j
./testbench
