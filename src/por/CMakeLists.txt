# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

set(POR_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/state.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/cs/state.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/gl/ts.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/pure_local/state.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/pure_local/ts.cc
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/cs/builder.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/cs/output.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/cs/state.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/cs/ts.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/gl/ts.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/pure_local/output.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/pure_local/state.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/pure_local/ts.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/output.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/state.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/synchronizable.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/ts.hh
PARENT_SCOPE)
