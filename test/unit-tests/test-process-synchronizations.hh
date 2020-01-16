/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#include <string>

#include "tchecker/fsm/fsm.hh"
#include "tchecker/parsing/declaration.hh"
#include "tchecker/system/static_analysis.hh"
#include "tchecker/ta/ta.hh"
#include "tchecker/utils/log.hh"

#include "utils.hh"

TEST_CASE( "process synchronizations on asynchronous system", "[process_synchronizations]") {
  std::string file =
  "system:asynchronous \n\
  process:P1 \n\
  process:P2 \n\
  \n";
  
  tchecker::log_t log(&std::cerr);
  tchecker::parsing::system_declaration_t const * sysdecl = tchecker::test::parse(file, log);
  
  REQUIRE(sysdecl != nullptr);
  
  tchecker::fsm::model_t model(*sysdecl, log);
  tchecker::fsm::system_t const & system = model.system();
  tchecker::process_synchronizations_t const psyncs = tchecker::process_synchronizations(system);

  auto psyncs_range = psyncs.process_synchronizations();
  REQUIRE(psyncs_range.begin() == psyncs_range.end());
}


TEST_CASE( "process synchronizations on global/local system", "[process_synchronizations]") {
  std::string file =
  "system:asynchronous \n\
  event:a1 \n\
  event:a2 \n\
  event:a3 \n\
  event:g \n\
  process:P1 \n\
  process:P2 \n\
  process:P3 \n\
  sync:P1@g:P2@g:P3@g \n\
  \n";
  
  tchecker::log_t log(&std::cerr);
  tchecker::parsing::system_declaration_t const * sysdecl = tchecker::test::parse(file, log);
  
  REQUIRE(sysdecl != nullptr);
  
  tchecker::fsm::model_t model(*sysdecl, log);
  tchecker::fsm::system_t const & system = model.system();
  
  tchecker::process_id_t const P1 = system.processes().key("P1");
  tchecker::process_id_t const P2 = system.processes().key("P2");
  tchecker::process_id_t const P3 = system.processes().key("P3");
    
  tchecker::process_synchronizations_t const psyncs = tchecker::process_synchronizations(system);
  auto psyncs_range = psyncs.process_synchronizations();

  std::set<tchecker::process_id_t> P123{P1, P2, P3};
  
  auto it = psyncs_range.begin(), end = psyncs_range.end();
  REQUIRE(it != end);

  auto mismatch = std::mismatch(P123.begin(), P123.end(), (*it).begin(), (*it).end());
  REQUIRE(mismatch == std::make_pair(P123.end(), (*it).end()));
  
  ++it;
  REQUIRE(it == end);
}


TEST_CASE( "process synchronizations on extended global/local system", "[process_synchronizations]") {
  std::string file =
  "system:asynchronous \n\
  event:a \n\
  event:b \n\
  event:c \n\
  event:g \n\
  process:P1 \n\
  process:P2 \n\
  process:P3 \n\
  process:P4 \n\
  sync:P1@a:P2@a \n\
  sync:P3@b:P4@b \n\
  sync:P3@c:P4@c \n\
  sync:P1@g:P2@g:P3@g:P4@g \n\
  \n";
  
  tchecker::log_t log(&std::cerr);
  tchecker::parsing::system_declaration_t const * sysdecl = tchecker::test::parse(file, log);
  
  REQUIRE(sysdecl != nullptr);
  
  tchecker::fsm::model_t model(*sysdecl, log);
  tchecker::fsm::system_t const & system = model.system();
  
  tchecker::process_id_t const P1 = system.processes().key("P1");
  tchecker::process_id_t const P2 = system.processes().key("P2");
  tchecker::process_id_t const P3 = system.processes().key("P3");
  tchecker::process_id_t const P4 = system.processes().key("P4");
    
  tchecker::process_synchronizations_t const psyncs = tchecker::process_synchronizations(system);
  auto psyncs_range = psyncs.process_synchronizations();

  std::set<tchecker::process_id_t> P12{P1, P2};
  std::set<tchecker::process_id_t> P34{P3, P4};
  std::set<tchecker::process_id_t> P1234{P1, P2, P3, P4};

  bool match12 = false, match34 = false, match1234 = false;
  std::size_t count = 0;
  for (auto it = psyncs_range.begin() ; it != psyncs_range.end(); ++it) {
    ++count;
    match12 = match12 || (std::mismatch(P12.begin(), P12.end(), (*it).begin(), (*it).end())
                          == std::make_pair(P12.end(), (*it).end()));
    match34 = match34 || (std::mismatch(P34.begin(), P34.end(), (*it).begin(), (*it).end())
                          == std::make_pair(P34.end(), (*it).end()));
    match1234 = match1234 || (std::mismatch(P1234.begin(), P1234.end(), (*it).begin(), (*it).end())
                              == std::make_pair(P1234.end(), (*it).end()));
  }
  
  REQUIRE(count == 3);
  REQUIRE(match12);
  REQUIRE(match34);
  REQUIRE(match1234);
}


TEST_CASE( "process synchronizations on client/server system", "[process_synchronizations]") {
  std::string file =
  "system:asynchronous \n\
  event:a \n\
  event:b \n\
  event:c \n\
  event:d \n\
  process:P1 \n\
  process:P2 \n\
  process:P3 \n\
  process:P4 \n\
  process:S \n\
  sync:P1@a:S@a \n\
  sync:P2@b:S@b \n\
  sync:P3@c:S@c \n\
  sync:P4@d:S@d \n\
  \n";
  
  tchecker::log_t log(&std::cerr);
  tchecker::parsing::system_declaration_t const * sysdecl = tchecker::test::parse(file, log);
  
  REQUIRE(sysdecl != nullptr);
  
  tchecker::fsm::model_t model(*sysdecl, log);
  tchecker::fsm::system_t const & system = model.system();
  
  tchecker::process_id_t const P1 = system.processes().key("P1");
  tchecker::process_id_t const P2 = system.processes().key("P2");
  tchecker::process_id_t const P3 = system.processes().key("P3");
  tchecker::process_id_t const P4 = system.processes().key("P4");
  tchecker::process_id_t const S = system.processes().key("S");
    
  tchecker::process_synchronizations_t const psyncs = tchecker::process_synchronizations(system);
  auto psyncs_range = psyncs.process_synchronizations();

  std::set<tchecker::process_id_t> P1S{P1, S};
  std::set<tchecker::process_id_t> P2S{P2, S};
  std::set<tchecker::process_id_t> P3S{P3, S};
  std::set<tchecker::process_id_t> P4S{P4, S};

  bool match1S = false, match2S = false, match3S = false, match4S = false;
  std::size_t count = 0;
  for (auto it = psyncs_range.begin() ; it != psyncs_range.end(); ++it) {
    ++count;
    match1S = match1S || (std::mismatch(P1S.begin(), P1S.end(), (*it).begin(), (*it).end())
                          == std::make_pair(P1S.end(), (*it).end()));
    match2S = match2S || (std::mismatch(P2S.begin(), P2S.end(), (*it).begin(), (*it).end())
                          == std::make_pair(P2S.end(), (*it).end()));
    match3S = match3S || (std::mismatch(P3S.begin(), P3S.end(), (*it).begin(), (*it).end())
                         == std::make_pair(P3S.end(), (*it).end()));
    match4S = match4S || (std::mismatch(P4S.begin(), P4S.end(), (*it).begin(), (*it).end())
                          == std::make_pair(P4S.end(), (*it).end()));
  }
  
  REQUIRE(count == 4);
  REQUIRE(match1S);
  REQUIRE(match2S);
  REQUIRE(match3S);
  REQUIRE(match4S);
}

