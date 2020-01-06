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

TEST_CASE( "next syncs on asynchronous system", "[next_syncs]") {
  std::string file =
  "system:asynchronous \n\
  event:a \n\
  event:b \n\
  event:c \n\
  event:d \n\
  \n\
  process:P1 \n\
  location:P1:A{initial:} \n\
  location:P1:B \n\
  location:P1:C \n\
  edge:P1:A:B:a \n\
  edge:P1:B:A:c \n\
  edge:P1:B:C:b \n\
  edge:P1:C:A:d \n\
  \n\
  process:P2 \n\
  location:P2:A{initial:} \n\
  location:P2:B \n\
  location:P2:C \n\
  edge:P2:A:B:a \n\
  edge:P2:B:A:c \n\
  edge:P2:B:C:b \n\
  edge:P2:C:A:d \n\
  \n";
  
  tchecker::log_t log(&std::cerr);
  tchecker::parsing::system_declaration_t const * sysdecl = tchecker::test::parse(file, log);
  
  REQUIRE(sysdecl != nullptr);
  
  tchecker::fsm::model_t model(*sysdecl, log);
  tchecker::fsm::system_t const & system = model.system();
  tchecker::location_next_syncs_t const lns = tchecker::location_next_syncs(system);
  
  tchecker::loc_id_t A1 = system.location("P1", "A")->id();
  tchecker::loc_id_t B1 = system.location("P1", "B")->id();
  tchecker::loc_id_t C1 = system.location("P1", "C")->id();
  
  boost::dynamic_bitset<> const & A1_next_syncs_location
  = lns.next_syncs(A1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
  REQUIRE(A1_next_syncs_location.none());

  boost::dynamic_bitset<> const & B1_next_syncs_location
  = lns.next_syncs(B1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
  REQUIRE(B1_next_syncs_location.none());
  
  boost::dynamic_bitset<> const & C1_next_syncs_location
  = lns.next_syncs(C1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
  REQUIRE(C1_next_syncs_location.none());
  
  boost::dynamic_bitset<> const & A1_next_syncs_reachable
  = lns.next_syncs(A1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
  REQUIRE(A1_next_syncs_reachable.none());

  boost::dynamic_bitset<> const & B1_next_syncs_reachable
  = lns.next_syncs(B1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
  REQUIRE(B1_next_syncs_reachable.none());
  
  boost::dynamic_bitset<> const & C1_next_syncs_reachable
  = lns.next_syncs(C1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
  REQUIRE(C1_next_syncs_reachable.none());
}



TEST_CASE( "next syncs on ABCD example", "[next_syncs]" ) {
  std::string file =
  "system:abcd \n\
  event:a \n\
  event:b \n\
  event:c \n\
  event:d \n\
  \n\
  process:P1 \n\
  location:P1:A{initial:} \n\
  location:P1:B \n\
  location:P1:C \n\
  edge:P1:A:B:a \n\
  edge:P1:B:A:c \n\
  edge:P1:B:C:b \n\
  edge:P1:C:A:d \n\
  \n\
  process:P2 \n\
  location:P2:A{initial:} \n\
  location:P2:B \n\
  location:P2:C \n\
  edge:P2:A:B:a \n\
  edge:P2:B:A:c \n\
  edge:P2:B:C:b \n\
  edge:P2:C:A:d \n\
  \n\
  sync:P1@c:P2@c \n\
  sync:P1@d:P2@d \n\
  \n";
  
  tchecker::log_t log(&std::cerr);
  tchecker::parsing::system_declaration_t const * sysdecl = tchecker::test::parse(file, log);
  
  REQUIRE(sysdecl != nullptr);
  
  tchecker::fsm::model_t model(*sysdecl, log);
  tchecker::fsm::system_t const & system = model.system();
  tchecker::location_next_syncs_t const lns = tchecker::location_next_syncs(system);
  
  tchecker::sync_id_t c_sync, d_sync;
  
  tchecker::process_id_t const P1 = system.processes().key("P1");
  tchecker::event_id_t const c = system.events().key("c");
  tchecker::event_id_t const d = system.events().key("d");
  
  for (tchecker::synchronization_t const & sync : system.synchronizations())
    if (sync.synchronizes(P1, c))
      c_sync = sync.id();
    else if (sync.synchronizes(P1, d))
      d_sync = sync.id();

  SECTION( "A has no location next sync" ) {
    tchecker::loc_id_t A1 = system.location("P1", "A")->id();
    
    boost::dynamic_bitset<> const & A1_next_syncs
    = lns.next_syncs(A1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(A1_next_syncs.none());
  }
  
  SECTION( "B has location next sync {c}" ) {
    tchecker::loc_id_t B1 = system.location("P1", "B")->id();
    
    boost::dynamic_bitset<> const & B1_next_syncs
    = lns.next_syncs(B1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(B1_next_syncs[c_sync] == true);
    REQUIRE(B1_next_syncs[d_sync] == false);
  }
  
  SECTION( "C has location next sync {d}" ) {
    tchecker::loc_id_t C1 = system.location("P1", "C")->id();
    
    boost::dynamic_bitset<> const & C1_next_syncs
    = lns.next_syncs(C1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(C1_next_syncs[c_sync] == false);
    REQUIRE(C1_next_syncs[d_sync] == true);
  }
  
  SECTION( "A has reachable next syncs {c,d}" ) {
    tchecker::loc_id_t A1 = system.location("P1", "A")->id();
    
    boost::dynamic_bitset<> const & A1_next_syncs
    = lns.next_syncs(A1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(A1_next_syncs[c_sync] == true);
    REQUIRE(A1_next_syncs[d_sync] == true);
  }
  
  SECTION( "B has reachable next syncs {c,d}" ) {
    tchecker::loc_id_t B1 = system.location("P1", "B")->id();
    
    boost::dynamic_bitset<> const & B1_next_syncs
    = lns.next_syncs(B1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(B1_next_syncs[c_sync] == true);
    REQUIRE(B1_next_syncs[d_sync] == true);
  }
  
  SECTION( "C has reachable next sync {d}" ) {
    tchecker::loc_id_t C1 = system.location("P1", "C")->id();
    
    boost::dynamic_bitset<> const & C1_next_syncs
    = lns.next_syncs(C1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(C1_next_syncs[c_sync] == false);
    REQUIRE(C1_next_syncs[d_sync] == true);
  }
  
  delete sysdecl;
}



TEST_CASE( "next syncs on ABCDE example", "[next_syncs]" ) {
  std::string file =
  "system:abcde \n\
  event:a \n\
  event:b \n\
  event:c \n\
  event:d \n\
  event:e \n\
  \n\
  process:P1 \n\
  location:P1:A{initial:} \n\
  location:P1:B \n\
  location:P1:C \n\
  location:P1:D \n\
  edge:P1:A:B:a \n\
  edge:P1:B:A:c \n\
  edge:P1:B:C:b \n\
  edge:P1:C:D:d \n\
  edge:P1:D:A:e \n\
  \n\
  process:P2 \n\
  location:P2:A{initial:} \n\
  location:P2:B \n\
  location:P2:C \n\
  edge:P2:A:B:a \n\
  edge:P2:A:C:e \n\
  edge:P2:B:A:c \n\
  edge:P2:B:C:b \n\
  edge:P2:C:A:d \n\
  \n\
  sync:P1@c:P2@c \n\
  sync:P1@d:P2@d \n\
  sync:P1@e:P2@e \n\
  \n";
  
  tchecker::log_t log(&std::cerr);
  tchecker::parsing::system_declaration_t const * sysdecl = tchecker::test::parse(file, log);
  
  REQUIRE(sysdecl != nullptr);
  
  tchecker::ta::model_t model(*sysdecl, log);
  tchecker::ta::system_t const & system = model.system();
  tchecker::location_next_syncs_t const lns = tchecker::location_next_syncs(system);
  
  tchecker::sync_id_t c_sync, d_sync, e_sync;
  
  tchecker::process_id_t const P1 = system.processes().key("P1");
  tchecker::event_id_t const c = system.events().key("c");
  tchecker::event_id_t const d = system.events().key("d");
  tchecker::event_id_t const e = system.events().key("e");
  
  for (tchecker::synchronization_t const & sync : system.synchronizations())
    if (sync.synchronizes(P1, c))
      c_sync = sync.id();
    else if (sync.synchronizes(P1, d))
      d_sync = sync.id();
    else if (sync.synchronizes(P1, e))
      e_sync = sync.id();

  SECTION( "P1.A has no location next sync" ) {
    tchecker::loc_id_t A1 = system.location("P1", "A")->id();
    
    boost::dynamic_bitset<> const & A1_next_syncs
    = lns.next_syncs(A1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(A1_next_syncs.none());
  }
  
  SECTION( "P1.B has location next sync {c}" ) {
    tchecker::loc_id_t B1 = system.location("P1", "B")->id();
    
    boost::dynamic_bitset<> const & B1_next_syncs
    = lns.next_syncs(B1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(B1_next_syncs[c_sync] == true);
    REQUIRE(B1_next_syncs[d_sync] == false);
    REQUIRE(B1_next_syncs[e_sync] == false);
  }
  
  SECTION( "P1.C has location next sync {d}" ) {
    tchecker::loc_id_t C1 = system.location("P1", "C")->id();
    
    boost::dynamic_bitset<> const & C1_next_syncs
    = lns.next_syncs(C1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(C1_next_syncs[c_sync] == false);
    REQUIRE(C1_next_syncs[d_sync] == true);
    REQUIRE(C1_next_syncs[e_sync] == false);
  }
  
  SECTION( "P1.D has location next sync {e}" ) {
    tchecker::loc_id_t D1 = system.location("P1", "D")->id();
    
    boost::dynamic_bitset<> const & D1_next_syncs
    = lns.next_syncs(D1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(D1_next_syncs[c_sync] == false);
    REQUIRE(D1_next_syncs[d_sync] == false);
    REQUIRE(D1_next_syncs[e_sync] == true);
  }
  
  SECTION( "P1.A has reachable next syncs {c,d}" ) {
    tchecker::loc_id_t A1 = system.location("P1", "A")->id();
    
    boost::dynamic_bitset<> const & A1_next_syncs
    = lns.next_syncs(A1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(A1_next_syncs[c_sync] == true);
    REQUIRE(A1_next_syncs[d_sync] == true);
    REQUIRE(A1_next_syncs[e_sync] == false);
  }
  
  SECTION( "P1.B has reachable next syncs {c,d}" ) {
    tchecker::loc_id_t B1 = system.location("P1", "B")->id();
    
    boost::dynamic_bitset<> const & B1_next_syncs
    = lns.next_syncs(B1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(B1_next_syncs[c_sync] == true);
    REQUIRE(B1_next_syncs[d_sync] == true);
    REQUIRE(B1_next_syncs[e_sync] == false);
  }
  
  SECTION( "P1.C has reachable next sync {d}" ) {
    tchecker::loc_id_t C1 = system.location("P1", "C")->id();
    
    boost::dynamic_bitset<> const & C1_next_syncs
    = lns.next_syncs(C1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(C1_next_syncs[c_sync] == false);
    REQUIRE(C1_next_syncs[d_sync] == true);
    REQUIRE(C1_next_syncs[e_sync] == false);
  }
  
  SECTION( "P1.D has reachable next sync {e}" ) {
    tchecker::loc_id_t D1 = system.location("P1", "D")->id();
    
    boost::dynamic_bitset<> const & D1_next_syncs
    = lns.next_syncs(D1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(D1_next_syncs[c_sync] == false);
    REQUIRE(D1_next_syncs[d_sync] == false);
    REQUIRE(D1_next_syncs[e_sync] == true);
  }
  
  SECTION( "P2.A has location next sync {e}" ) {
    tchecker::loc_id_t A2 = system.location("P2", "A")->id();
    
    boost::dynamic_bitset<> const & A2_next_syncs
    = lns.next_syncs(A2, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(A2_next_syncs[c_sync] == false);
    REQUIRE(A2_next_syncs[d_sync] == false);
    REQUIRE(A2_next_syncs[e_sync] == true);
  }
  
  SECTION( "P2.B has location next sync {c}" ) {
    tchecker::loc_id_t B2 = system.location("P2", "B")->id();
    
    boost::dynamic_bitset<> const & B2_next_syncs
    = lns.next_syncs(B2, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(B2_next_syncs[c_sync] == true);
    REQUIRE(B2_next_syncs[d_sync] == false);
    REQUIRE(B2_next_syncs[e_sync] == false);
  }
  
  SECTION( "P2.C has location next sync {d}" ) {
    tchecker::loc_id_t C2 = system.location("P2", "C")->id();
    
    boost::dynamic_bitset<> const & C2_next_syncs
    = lns.next_syncs(C2, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(C2_next_syncs[c_sync] == false);
    REQUIRE(C2_next_syncs[d_sync] == true);
    REQUIRE(C2_next_syncs[e_sync] == false);
  }
  
  SECTION( "P2.A has reachable next syncs {c,d,e}" ) {
    tchecker::loc_id_t A2 = system.location("P2", "A")->id();
    
    boost::dynamic_bitset<> const & A2_next_syncs
    = lns.next_syncs(A2, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(A2_next_syncs[c_sync] == true);
    REQUIRE(A2_next_syncs[d_sync] == true);
    REQUIRE(A2_next_syncs[e_sync] == true);
  }
  
  SECTION( "P2.B has reachable next syncs {c,d}" ) {
    tchecker::loc_id_t B2 = system.location("P2", "B")->id();
    
    boost::dynamic_bitset<> const & B2_next_syncs
    = lns.next_syncs(B2, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(B2_next_syncs[c_sync] == true);
    REQUIRE(B2_next_syncs[d_sync] == true);
    REQUIRE(B2_next_syncs[e_sync] == false);
  }
  
  SECTION( "P2.C has reachable next sync {d}" ) {
    tchecker::loc_id_t C2 = system.location("P2", "C")->id();
    
    boost::dynamic_bitset<> const & C2_next_syncs
    = lns.next_syncs(C2, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(C2_next_syncs[c_sync] == false);
    REQUIRE(C2_next_syncs[d_sync] == true);
    REQUIRE(C2_next_syncs[e_sync] == false);
  }
  
  delete sysdecl;
}



TEST_CASE( "next syncs on example with propagation", "[next_syncs]" ) {
  std::string file =
  "system:abcd \n\
  event:a \n\
  event:b \n\
  event:c \n\
  event:d \n\
  event:e \n\
  event:f \n\
  event:g \n\
  \n\
  process:P1 \n\
  location:P1:A{initial:} \n\
  location:P1:B \n\
  location:P1:C \n\
  location:P1:D \n\
  location:P1:E \n\
  location:P1:F \n\
  edge:P1:A:B:a \n\
  edge:P1:A:C:b \n\
  edge:P1:A:F:g \n\
  edge:P1:B:D:c \n\
  edge:P1:C:D:d \n\
  edge:P1:D:E:e \n\
  edge:P1:E:A:f \n\
  edge:P1:F:A:a \n\
  \n\
  process:P2 \n\
  \n\
  sync:P1@d:P2@d \n\
  sync:P1@f:P2@f \n\
  sync:P1@g:P2@g \n\
  \n";
  
  tchecker::log_t log(&std::cerr);
  tchecker::parsing::system_declaration_t const * sysdecl = tchecker::test::parse(file, log);
  
  REQUIRE(sysdecl != nullptr);
  
  tchecker::ta::model_t model(*sysdecl, log);
  tchecker::ta::system_t const & system = model.system();
  tchecker::location_next_syncs_t const lns = tchecker::location_next_syncs(system);
  
  tchecker::sync_id_t d_sync, f_sync, g_sync;
  
  tchecker::process_id_t const P1 = system.processes().key("P1");
  tchecker::event_id_t const d = system.events().key("d");
  tchecker::event_id_t const f = system.events().key("f");
  tchecker::event_id_t const g = system.events().key("g");
  
  for (tchecker::synchronization_t const & sync : system.synchronizations())
    if (sync.synchronizes(P1, d))
      d_sync = sync.id();
    else if (sync.synchronizes(P1, f))
      f_sync = sync.id();
    else if (sync.synchronizes(P1, g))
      g_sync = sync.id();

  SECTION( "P1.A has location next sync {g}" ) {
    tchecker::loc_id_t A1 = system.location("P1", "A")->id();
    
    boost::dynamic_bitset<> const & A1_next_syncs
    = lns.next_syncs(A1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(A1_next_syncs[d_sync] == false);
    REQUIRE(A1_next_syncs[f_sync] == false);
    REQUIRE(A1_next_syncs[g_sync] == true);
  }
  
  SECTION( "P1.B has no location next sync" ) {
    tchecker::loc_id_t B1 = system.location("P1", "B")->id();
    
    boost::dynamic_bitset<> const & B1_next_syncs
    = lns.next_syncs(B1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(B1_next_syncs.none());
  }
  
  SECTION( "P1.C has location next sync {d}" ) {
    tchecker::loc_id_t C1 = system.location("P1", "C")->id();
    
    boost::dynamic_bitset<> const & C1_next_syncs
    = lns.next_syncs(C1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(C1_next_syncs[d_sync] == true);
    REQUIRE(C1_next_syncs[f_sync] == false);
    REQUIRE(C1_next_syncs[g_sync] == false);
  }
  
  SECTION( "P1.D has no location next sync" ) {
    tchecker::loc_id_t D1 = system.location("P1", "D")->id();
    
    boost::dynamic_bitset<> const & D1_next_syncs
    = lns.next_syncs(D1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(D1_next_syncs.none());
  }
  
  SECTION( "P1.E has location next sync {f}" ) {
    tchecker::loc_id_t E1 = system.location("P1", "E")->id();
    
    boost::dynamic_bitset<> const & E1_next_syncs
    = lns.next_syncs(E1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(E1_next_syncs[d_sync] == false);
    REQUIRE(E1_next_syncs[f_sync] == true);
    REQUIRE(E1_next_syncs[g_sync] == false);
  }
  
  SECTION( "P1.F has no location next sync" ) {
    tchecker::loc_id_t F1 = system.location("P1", "F")->id();
    
    boost::dynamic_bitset<> const & F1_next_syncs
    = lns.next_syncs(F1, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    REQUIRE(F1_next_syncs.none());
  }
  
  SECTION( "P1.A has reachable next syncs {d,f,g}" ) {
    tchecker::loc_id_t A1 = system.location("P1", "A")->id();
    
    boost::dynamic_bitset<> const & A1_next_syncs
    = lns.next_syncs(A1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(A1_next_syncs[d_sync] == true);
    REQUIRE(A1_next_syncs[f_sync] == true);
    REQUIRE(A1_next_syncs[g_sync] == true);
  }
  
  SECTION( "P1.B has reachable next syncs {f}" ) {
    tchecker::loc_id_t B1 = system.location("P1", "B")->id();
    
    boost::dynamic_bitset<> const & B1_next_syncs
    = lns.next_syncs(B1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(B1_next_syncs[d_sync] == false);
    REQUIRE(B1_next_syncs[f_sync] == true);
    REQUIRE(B1_next_syncs[g_sync] == false);
  }
  
  SECTION( "P1.C has reachable next sync {d}" ) {
    tchecker::loc_id_t C1 = system.location("P1", "C")->id();
    
    boost::dynamic_bitset<> const & C1_next_syncs
    = lns.next_syncs(C1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(C1_next_syncs[d_sync] == true);
    REQUIRE(C1_next_syncs[f_sync] == false);
    REQUIRE(C1_next_syncs[g_sync] == false);
  }
  
  SECTION( "P1.D has reachable next sync {f}" ) {
    tchecker::loc_id_t D1 = system.location("P1", "D")->id();
    
    boost::dynamic_bitset<> const & D1_next_syncs
    = lns.next_syncs(D1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(D1_next_syncs[d_sync] == false);
    REQUIRE(D1_next_syncs[f_sync] == true);
    REQUIRE(D1_next_syncs[g_sync] == false);
  }
  
  SECTION( "P1.E has reachable next sync {f}" ) {
    tchecker::loc_id_t E1 = system.location("P1", "E")->id();
    
    boost::dynamic_bitset<> const & E1_next_syncs
    = lns.next_syncs(E1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(E1_next_syncs[d_sync] == false);
    REQUIRE(E1_next_syncs[f_sync] == true);
    REQUIRE(E1_next_syncs[g_sync] == false);
  }
  
  SECTION( "P1.F has reachable next sync {d,f,g}" ) {
    tchecker::loc_id_t F1 = system.location("P1", "F")->id();
    
    boost::dynamic_bitset<> const & F1_next_syncs
    = lns.next_syncs(F1, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
    
    REQUIRE(F1_next_syncs[d_sync] == true);
    REQUIRE(F1_next_syncs[f_sync] == true);
    REQUIRE(F1_next_syncs[g_sync] == true);
  }
  
  delete sysdecl;
}
