# One craft_lab() call per source file, in pipeline order.
# Stage directories mirror docs/bio/: 01_fasta .. 09_parallel.
#
# Sample structures are not stored in the repository. data/README.md lists the
# public accession numbers and the download command for each.

# 01  FASTA and Sequence Input
craft_lab(bio_01_fasta_c    01_fasta/fasta_c.c)
craft_lab(bio_01_fasta_cpp  01_fasta/fasta_cpp.cpp)

# 02  Sequence Encoding
craft_lab(bio_02_gc         02_encoding/gc_kmer.c)
craft_lab(bio_02_twobit     02_encoding/two_bit.c)

# 03  Numerics for Scoring
craft_lab(bio_03_kahan      03_numerics/kahan.c)
craft_lab(bio_03_lse        03_numerics/lse.c)

# 04  Alignment
craft_lab(bio_04_nw         04_alignment/nw.c)

# 05  PDB Format
craft_lab(bio_05_pdb        05_pdb_parse/pdb_dump.c)

# 06  Geometry
craft_lab(bio_06_dihedral   06_geometry/dihedral.c)
craft_lab(bio_06_rmsd       06_geometry/rmsd.c)

# 07  Contacts
craft_lab(bio_07_contacts   07_contacts/contact_map.c)

# 08  Structure Model
craft_lab(bio_08_model      08_structure_model/model.cpp)

# 09  Parallel Scoring
# OpenMP is optional: without it the source falls back to a serial loop and the
# lab still builds, runs, and prints its reference value.
craft_lab(bio_09_parallel   09_parallel/omp_scores.c)
find_package(OpenMP QUIET)
if(OpenMP_C_FOUND)
  target_link_libraries(bio_09_parallel PRIVATE OpenMP::OpenMP_C)
  message(STATUS "bio_09_parallel: OpenMP enabled")
else()
  message(STATUS "bio_09_parallel: OpenMP not found, serial fallback")
endif()
