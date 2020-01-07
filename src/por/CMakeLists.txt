# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

set(POR_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/state.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/gl/large/ts.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/gl/strict/ts.cc
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/gl/large/ts.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/gl/strict/ts.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/output.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/state.hh
    ${TCHECKER_INCLUDE_DIR}/tchecker/por/synchronizable.hh
PARENT_SCOPE)
