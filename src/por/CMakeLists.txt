# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

set(POR_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/state.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/gl/ts.cc
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/gl/ts.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/output.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/state.hh
    ${TCHECKER_INCLUDE_DIR}/tchecker/por/synchronizable.hh
PARENT_SCOPE)
