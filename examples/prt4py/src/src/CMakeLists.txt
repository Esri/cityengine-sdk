set_source_files_properties(
  ${PRT4PY_SOURCE_DIR}/src/prt4py.pyx
  PROPERTIES CYTHON_IS_CXX TRUE
)

cython_add_module(prt4py prt4py.pyx SimpleWrapper.cpp)
target_link_libraries(prt4py ${PRT_LIBRARY})
