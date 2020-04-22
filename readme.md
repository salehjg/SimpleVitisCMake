/opt/Xilinx/Vitis/2019.2/bin/v++ 
--target hw --compile -I"../src" 
--config common-config.ini 
--config vector_addition-vadd-compile.ini 
-o"vector_addition.build/vadd.xo" 
"../src/vector_addition_BAD.cpp"



/opt/Xilinx/Vitis/2019.2/bin/v++ 
--target hw --compile 
-I"../src" 
--config common-config.ini 
--config vector_addition-vadd_pipelined-compile.ini 
-o"vector_addition.build/vadd_pipelined.xo" 
"../src/vector_addition.cpp"



/opt/Xilinx/Vitis/2019.2/bin/v++ 
--target hw --link 
--vivado.prop run.impl_1.STEPS.ROUTE_DESIGN.TCL.PRE=/home/saleh/01_workspace/00_vitis/sampleVitis01/slr_report.tcl 
--config common-config.ini 
--config vector_addition-link.ini 
-o"vector_addition.xclbin" 
vector_addition.build/vadd.xo 
vector_addition.build/vadd_pipelined.xo

/opt/Xilinx/Vitis/2019.2/bin/v++ 
--target hw --link 
--vivado.prop run.impl_1.STEPS.ROUTE_DESIGN.TCL.PRE=/home/saleh/01_workspace/00_vitis/sampleVitis01/slr_report.tcl 
--config common-config.ini 
--config vector_addition-link.ini 
-ovector_addition.xclbin 
vector_addition.build/vadd.xo 
vector_addition.build/vadd_pipelined.xo
 --interactive impl

