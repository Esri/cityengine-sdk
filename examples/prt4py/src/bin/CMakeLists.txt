set_source_files_properties( ${PRT4PY_SOURCE_DIR}/src/prt4py.pyx PROPERTIES CYTHON_IS_CXX TRUE )

cython_add_standalone_executable(prt
  MAIN_MODULE prt.py
  ${PRT4PY_SOURCE_DIR}/src/prt4py.pyx
  ${PRT4PY_SOURCE_DIR}/src/SimpleWrapper.cpp
  prt.py
)
target_link_libraries(prt ${PRT_LIBRARY})
