# One craft_lab() call per source file, in chapter order.
# Stage directories mirror docs/cpp/: 01_bridge .. 13_capstone.
#
# Target name convention: cpp_<stage>_<short>, matching the Build line in each
# source file's header comment.
#
# C++ labs must compile without warnings under -Wall -Wextra -Wconversion. A
# warning is a failing grade, not a remark: the acceptance script checks it.

# 01  From C to C++
craft_lab(cpp_01_bridge      01_bridge/01_ref_vs_ptr.cpp)

# 02  Classes
craft_lab(cpp_02_classes     02_classes/01_interface_split.cpp)

# 03  Construction and Lifetime
craft_lab(cpp_03_ctor        03_lifetime/01_ctor_order.cpp)

# 04  Inheritance
craft_lab(cpp_04_inherit     04_inheritance/01_layout_inherit.cpp)

# 05  Polymorphism
craft_lab(cpp_05_vtable      05_polymorphism/01_vtable_probe.cpp)
craft_lab(cpp_05_slice       05_polymorphism/02_slicing.cpp)

# 06  Operators
craft_lab(cpp_06_fraction    06_operators/01_fraction_ops.cpp)

# 07  Templates
craft_lab(cpp_07_templates   07_templates/01_template_diag.cpp)

# 08  RAII and Smart Pointers
craft_lab(cpp_08_raii        08_raii/01_scope_guard.cpp)

# 09  STL Containers
craft_lab(cpp_09_containers  09_containers/01_invalidation.cpp)

# 10  STL Algorithms
craft_lab(cpp_10_algorithms  10_algorithms/01_lambda_pipeline.cpp)

# 11  Modern C++
craft_lab(cpp_11_modern      11_modern/01_auto_and_structured.cpp)

# 12  Exceptions
craft_lab(cpp_12_exceptions  12_exceptions/01_stack_unwind.cpp)

# 13  Capstone: Sequence Toolkit
craft_lab(cpp_13_capstone    13_capstone/sequence_toolkit.cpp)
