# One craft_lab() call per source file, in chapter order.
# Stage directories mirror the chapters under docs/c/, so a reader can move
# from page to source without guessing a path.
#
# Target name convention: c_<stage>_<short> — short enough to type, unique
# enough that a ctest failure names one file.

# 01  Objects in Memory
craft_lab(c_01_sizeof   01_objects_in_memory/01_sizeof_table.c)
craft_lab(c_01_endian   01_objects_in_memory/02_endianness_probe.c)
craft_lab(c_01_align    01_objects_in_memory/03_alignment_probe.c)

# 02  Stack Frames
craft_lab(c_02_frames   02_stack_frames/01_frame_addresses.c)
craft_lab(c_02_lifetime 02_stack_frames/02_lifetime_probe.c)
craft_lab(c_02_depth    02_stack_frames/03_recursion_depth.c)

# 03  Pointers
craft_lab(c_03_decay    03_pointers/01_array_decay.c)
craft_lab(c_03_swap     03_pointers/02_out_param_swap.c)
craft_lab(c_03_basics   03_pointers/03_pointer_basics.c)

# 04  Arrays and Layout
craft_lab(c_04_rowmajor 04_arrays_layout/01_row_major_walk.c)
craft_lab(c_04_bounds   04_arrays_layout/02_bounds_probe.c)
craft_lab(c_04_transpose 04_arrays_layout/03_transpose_inplace.c)

# 05  Strings
craft_lab(c_05_strlen   05_strings/01_strlen_reimpl.c)
craft_lab(c_05_input    05_strings/02_safe_input.c)
craft_lab(c_05_words    05_strings/03_word_count.c)

# 06  Structs and Padding
craft_lab(c_06_layout   06_structs_padding/01_layout_table.c)
craft_lab(c_06_union    06_structs_padding/02_union_typing.c)
craft_lab(c_06_bits     06_structs_padding/03_bitfield_bounds.c)

# 07  The Heap
craft_lab(c_07_growth   07_heap/01_growth_trace.c)
craft_lab(c_07_double   07_heap/02_double_free.c)
craft_lab(c_07_leak     07_heap/03_leak_hunt.c)

# 08  Qualifiers and Aliasing
craft_lab(c_08_alias    08_qualifiers/01_alias_violation.c)

# 09  Undefined Behaviour
craft_lab(c_09_overflow 09_undefined_behavior/01_signed_overflow.c)
craft_lab(c_09_sweep    09_undefined_behavior/02_ub_sweep.c)

# 10  Bit-Level Programming
craft_lab(c_10_mask     10_bitops/01_mask_fields.c)
craft_lab(c_10_popcount 10_bitops/02_popcount.c)
craft_lab(c_10_twobit   10_bitops/03_two_bit_code.c)

# 11  Function Pointers
craft_lab(c_11_qsort    11_callbacks/01_qsort_words.c)
craft_lab(c_11_dispatch 11_callbacks/02_dispatch_table.c)

# 12  Files and Streams
craft_lab(c_12_lines    12_files_streams/01_line_reader.c)
craft_lab(c_12_binary   12_files_streams/02_binary_roundtrip.c)

# 13  Translation Units
craft_lab(c_13_static   13_translation_units/01_static_scope.c)

# 14  Sorting and Cost
craft_lab(c_14_sort     14_sorting/01_sort_bench.c)

# 15  Linked Structures
craft_lab(c_15_list     15_linked_structures/01_list_reverse.c)
craft_lab(c_15_ring     15_linked_structures/02_ring_buffer.c)

# 16  Error Handling
craft_lab(c_16_errno    16_error_handling/01_errno_chain.c)
craft_lab(c_16_cleanup  16_error_handling/02_cleanup_goto.c)
