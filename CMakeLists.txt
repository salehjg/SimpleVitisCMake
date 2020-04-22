cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)


set(BinaryContainersName "binarycontainer1" CACHE STRING "Binary Container's Name")
set(IpCachePath "/home/saleh/01_workspace/00_vitis/ip_cache" CACHE STRING "IP Cache Path(with enough space)")
set(Profiling OFF CACHE BOOL "Profiling ON/OFF")





PROJECT(SimpleVitisCMake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/hlslib/cmake)
find_package(Vitis REQUIRED)

add_executable(MyHostExecutable src/host.cpp src/xcl2.cpp)
include_directories(${Vitis_INCLUDE_DIRS} inc/)
target_link_libraries(MyHostExecutable ${Vitis_LIBRARIES}) 






# ====================================================================================================
# Global vars and files...

# Create bash scripts for compiling 
file(WRITE "${CMAKE_BINARY_DIR}/Compile_Hw_Batch.sh" "find . -name \"*.xo\" -type f -delete\n")
file(WRITE "${CMAKE_BINARY_DIR}/Compile_HwEmu_Batch.sh" "find . -name \"*.xo\" -type f -delete\n")
file(WRITE "${CMAKE_BINARY_DIR}/Compile_SwEmu_Batch.sh" "find . -name \"*.xo\" -type f -delete\n")

# Create common-config.ini
file(WRITE "${CMAKE_BINARY_DIR}/common-config.ini" "platform=$ENV{AWS_PLATFORM}\nsave-temps=1\n")

set(hw_link_command "")
set(hwemu_link_command "")
set(swemu_link_command "")
set(hw_link "")
set(hwemu_link "")
set(swemu_link "")

include(ProcessorCount)
ProcessorCount(_CpuCount)
if(NOT ${_CpuCount} EQUAL 0)
    set(CpuCount ${_CpuCount})
else()
    set(CpuCount 4)
endif()
message("Detected CPU Core Count: ${CpuCount}")


# ====================================================================================================
# Functions...

function(CreateLinkIni
    nkTagsForKernels)
    # Create <binary container>-link.ini
    file(WRITE "${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini" "debug=1\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini" "messageDb=${CMAKE_BINARY_DIR}/${BinaryContainersName}.mdb\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini" "temp_dir=${CMAKE_BINARY_DIR}/${BinaryContainersName}.build\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini" "report_dir=${CMAKE_BINARY_DIR}/${BinaryContainersName}.build/reports\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini" "log_dir=${CMAKE_BINARY_DIR}/${BinaryContainersName}.build/logs\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini" "remote_ip_cache=${IpCachePath}\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini" "[advanced]\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini" "misc=solution_name=link\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini" "[connectivity]\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini" "${${nkTagsForKernels}}\n")
    #file(APPEND "${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini" "nk=vadd_pipelined:1:vadd_pipelined_1\n")
endfunction()

# Generates and appends commands for compiling and linking the given kernel.
function(CreateKernel 
    ContainersName
    Path
    TopFunctionsName  
    IsProfiled
    SpTag
    OutNkTagsAppended)
    
    # Creating compile config file for this kernel
    file(WRITE "${CMAKE_BINARY_DIR}/${ContainersName}-${TopFunctionsName}-compile.ini" "kernel=${TopFunctionsName}\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${ContainersName}-${TopFunctionsName}-compile.ini" "debug=1\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${ContainersName}-${TopFunctionsName}-compile.ini" "messageDb=${CMAKE_BINARY_DIR}/${ContainersName}.build/${TopFunctionsName}.mdb\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${ContainersName}-${TopFunctionsName}-compile.ini" "temp_dir=${CMAKE_BINARY_DIR}/${ContainersName}.build\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${ContainersName}-${TopFunctionsName}-compile.ini" "report_dir=${CMAKE_BINARY_DIR}/${ContainersName}.build/reports\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${ContainersName}-${TopFunctionsName}-compile.ini" "log_dir=${CMAKE_BINARY_DIR}/${ContainersName}.build/logs\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${ContainersName}-${TopFunctionsName}-compile.ini" "[advanced]\n")
    file(APPEND "${CMAKE_BINARY_DIR}/${ContainersName}-${TopFunctionsName}-compile.ini" "misc=solution_name=${TopFunctionsName}\n")
    set(NkTag "nk=${TopFunctionsName}:1:${TopFunctionsName}_1")
    set(${OutNkTagsAppended} "${${OutNkTagsAppended}}\n${NkTag}" PARENT_SCOPE)

    set(hw_compile 
        ${Vitis_COMPILER} 
        --target hw
        --compile
        --jobs ${CpuCount}
        -I\"${CMAKE_SOURCE_DIR}/inc\" 
        --config ${CMAKE_BINARY_DIR}/common-config.ini 
        --config ${CMAKE_BINARY_DIR}/${ContainersName}-${TopFunctionsName}-compile.ini 
        -o\"${CMAKE_BINARY_DIR}/${ContainersName}.build/${TopFunctionsName}_hw.xo\" 
        \"${Path}\"
    )

    set(hwemu_compile 
        ${Vitis_COMPILER} 
        --target hw_emu
        --compile
        --jobs ${CpuCount}
        -I\"${CMAKE_SOURCE_DIR}/inc\" 
        --config ${CMAKE_BINARY_DIR}/common-config.ini 
        --config ${CMAKE_BINARY_DIR}/${ContainersName}-${TopFunctionsName}-compile.ini 
        -o\"${CMAKE_BINARY_DIR}/${ContainersName}.build/${TopFunctionsName}_hw_emu.xo\" 
        \"${Path}\"
    )

    set(swemu_compile 
        ${Vitis_COMPILER} 
        --target sw_emu
        --compile
        --jobs ${CpuCount}
        -I\"${CMAKE_SOURCE_DIR}/inc\" 
        --config ${CMAKE_BINARY_DIR}/common-config.ini 
        --config ${CMAKE_BINARY_DIR}/${ContainersName}-${TopFunctionsName}-compile.ini 
        -o\"${CMAKE_BINARY_DIR}/${ContainersName}.build/${TopFunctionsName}_sw_emu.xo\" 
        \"${Path}\"
    )

    string (REPLACE ";" " " hw_compile "${hw_compile}")
    string (REPLACE ";" " " hwemu_compile "${hwemu_compile}")
    string (REPLACE ";" " " swemu_compile "${swemu_compile}")

    file(APPEND "${CMAKE_BINARY_DIR}/Compile_Hw_Batch.sh" "${hw_compile}\n")
    file(APPEND "${CMAKE_BINARY_DIR}/Compile_HwEmu_Batch.sh" "${hwemu_compile}\n")
    file(APPEND "${CMAKE_BINARY_DIR}/Compile_SwEmu_Batch.sh" "${swemu_compile}\n")

    set(hw_link "${hw_link} ${CMAKE_BINARY_DIR}/${ContainersName}.build/${TopFunctionsName}_hw.xo" PARENT_SCOPE) 
    set(hwemu_link "${hwemu_link} ${CMAKE_BINARY_DIR}/${ContainersName}.build/${TopFunctionsName}_hw_emu.xo" PARENT_SCOPE) 
    set(swemu_link "${swemu_link} ${CMAKE_BINARY_DIR}/${ContainersName}.build/${TopFunctionsName}_sw_emu.xo" PARENT_SCOPE) 

    if(${IsProfiled})
        set(hw_link_command "${hw_link_command} --profile_kernel data:${TopFunctionsName}:all:all")
    endif()
    set(hw_link_command "${hw_link_command} ${SpTag}")

    # Identical commands for hw-emu link process
    set(hwemu_link_command "${hw_link_command}")
    set(swemu_link_command "${hw_link_command}")

    # Set link commands globally
    set(hw_link_command "${hw_link_command}" PARENT_SCOPE)
    set(hwemu_link_command "${hwemu_link_command}" PARENT_SCOPE)
    set(swemu_link_command "${swemu_link_command}" PARENT_SCOPE)

endfunction()






# ====================================================================================================
# Defining Kernels...

# Assembling kernels and required config files...  
set(GlobalNkTags "")

CreateKernel(
    "binary01"
    "${CMAKE_SOURCE_DIR}/src/kernels/mykernel1.cpp"
    "vadd_pipelined"
    ${Profiling}
    ""
    GlobalNkTags)

CreateKernel(
    "binary01"
    "${CMAKE_SOURCE_DIR}/src/kernels/mykernel2.cpp"
    "vadd"
    ${Profiling}
    ""
    GlobalNkTags)

CreateLinkIni(GlobalNkTags)

add_custom_target(compile_hw COMMAND sh Compile_Hw_Batch.sh) 
add_custom_target(compile_hwemu COMMAND sh Compile_HwEmu_Batch.sh) 
add_custom_target(compile_swemu COMMAND sh Compile_SwEmu_Batch.sh) 


set( _link_hw_
    ${Vitis_COMPILER}
    --target hw --link
    --jobs ${CpuCount}
    --config ${CMAKE_BINARY_DIR}/common-config.ini
    --config ${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini
    -o\"${CMAKE_BINARY_DIR}/${BinaryContainersName}_hw.xclbin\"
    ${hw_link_command}
    ${hw_link}
    )
string (REPLACE ";" " " _link_hw_ "${_link_hw_}")
file(WRITE "${CMAKE_BINARY_DIR}/Link_Hw_Batch.sh" "${_link_hw_}")

set( _link_hwemu_
    ${Vitis_COMPILER}
    --target hw_emu --link
    --jobs ${CpuCount}
    --config ${CMAKE_BINARY_DIR}/common-config.ini
    --config ${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini
    -o\"${CMAKE_BINARY_DIR}/${BinaryContainersName}_hw_emu.xclbin\"
    ${hwemu_link_command}
    ${hwemu_link}
    )
string (REPLACE ";" " " _link_hwemu_ "${_link_hwemu_}")
file(WRITE "${CMAKE_BINARY_DIR}/Link_HwEmu_Batch.sh" "${_link_hwemu_}")

set( _link_swemu_
    ${Vitis_COMPILER}
    --target sw_emu --link
    --jobs ${CpuCount}
    --config ${CMAKE_BINARY_DIR}/common-config.ini
    --config ${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini
    -o\"${CMAKE_BINARY_DIR}/${BinaryContainersName}_sw_emu.xclbin\"
    ${swemu_link_command}
    ${swemu_link}
    )
string (REPLACE ";" " " _link_swemu_ "${_link_swemu_}")
file(WRITE "${CMAKE_BINARY_DIR}/Link_SwEmu_Batch.sh" "${_link_swemu_}")

set( _link_interactive_imp_
    ${Vitis_COMPILER}
    --target hw --link
    --jobs ${CpuCount}
    --config ${CMAKE_BINARY_DIR}/common-config.ini
    --config ${CMAKE_BINARY_DIR}/${BinaryContainersName}-link.ini
    -o\"${CMAKE_BINARY_DIR}/${BinaryContainersName}_hw.xclbin\"
    ${hw_link_command}
    ${hw_link}
    --interactive impl
    )
string (REPLACE ";" " " _link_interactive_imp_ "${_link_interactive_imp_}")
file(WRITE "${CMAKE_BINARY_DIR}/Interactrive_Hw_Batch.sh" "${_link_interactive_imp_}")

add_custom_target(link_hw COMMAND sh Link_Hw_Batch.sh) 
add_custom_target(link_hwemu COMMAND sh Link_HwEmu_Batch.sh ) 
add_custom_target(link_swemu COMMAND sh Link_SwEmu_Batch.sh ) 
add_custom_target(interactive_hw COMMAND sh Interactrive_Hw_Batch.sh) 

