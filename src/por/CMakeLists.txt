# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

set(POR_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/cs/state.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/gl/state.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/por1/state.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/por2/state.cc
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/cs/builder.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/cs/output.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/cs/state.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/cs/ts.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/gl/builder.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/gl/output.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/gl/state.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/gl/ts.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/por1/builder.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/por1/output.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/por1/state.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/por1/ts.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/por2/builder.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/por2/output.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/por2/state.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/por2/ts.hh
  ${TCHECKER_INCLUDE_DIR}/tchecker/por/synchronizable.hh
PARENT_SCOPE)
