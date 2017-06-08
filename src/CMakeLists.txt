add_library(result-checking MODULE
    FunctionDominanceTree.cpp
    SensitiveFunctionMarkPass.cpp
    PassRegistration.cpp
    GuardsNetwork.cpp
    GuardsInserter.cpp
    DotPrinter.cpp
)

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(result-checking PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(result-checking PROPERTIES
    COMPILE_FLAGS "-fno-rtti -g"
)

