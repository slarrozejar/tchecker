/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_SYSTEM_STATIC_ANALYSIS_HH
#define TCHECKER_SYSTEM_STATIC_ANALYSIS_HH

#include <limits>
#include <vector>

#include <boost/container/flat_set.hpp>
#include <boost/dynamic_bitset.hpp>

#include "tchecker/basictypes.hh"
#include "tchecker/system/synchronization.hh"
#include "tchecker/system/system.hh"
#include "tchecker/system/edge.hh"

/*!
 \file static_analysis.hh
 \brief Static analysis on systems of processes
 */

namespace tchecker {
  /*!
   \class process_events_map_t
   \brief Type of map : process ID -> event IDs
   */
  class process_events_map_t {
  public:
    /*!
     \brief Constructor
     \param proc_count : number of processes
     */
    process_events_map_t(tchecker::process_id_t proc_count);
    
    /*!
     \brief Copy-constructor
     */
    process_events_map_t(tchecker::process_events_map_t const &) = default;
    
    /*!
     \brief Move-constructor
     */
    process_events_map_t(tchecker::process_events_map_t &&) = default;
    
    /*!
     \brief Destructor
     */
    ~process_events_map_t() = default;
    
    /*!
     \brief Assignment operator
     */
    tchecker::process_events_map_t & operator= (tchecker::process_events_map_t const &) = default;
    
    /*!
     \brief Move-assignment operator
     */
    tchecker::process_events_map_t & operator= (tchecker::process_events_map_t &&) = default;
    
    /*!
     \brief Insertion
     \param pid : a process ID
     \param event_id : an event ID
     \pre pid < proc_count (checked by assertion)
     \post pid |-> event_id has been added to the map
     */
    void insert(tchecker::process_id_t pid, tchecker::event_id_t event_id);
    
    /*!
     \brief Lookup
     \param pid : a process ID
     \param event_id : an event ID     
     \return true if this maps pid to (a set of events containing) event_id, false otherwise
     */
    bool contains(tchecker::process_id_t pid, tchecker::event_id_t event_id);
  private:
    std::vector<boost::container::flat_set<tchecker::event_id_t>> _map;  /*!< multi-map : process ID -> event IDs */
  };
  
  
  /*!
   \brief Compute weakly synchronized events
   \param range : a range of synchronization vectors
   \param proc_count : number of processes
   \pre all the process IDs in synchronization vectors in range are less than proc_count
   \return a map from process IDs to the set of weakly synchronized event IDs in each process, w.r.t. synchronization vectors
   in range
   */
  tchecker::process_events_map_t
  weakly_synchronized_events(tchecker::range_t<tchecker::const_sync_iterator_t> const & range,
                             tchecker::process_id_t proc_count);
  
  
  /*!
   \brief Compute weakly synchronized events
   \tparam LOC : type of locations
   \tparam EDGE : type of edges
   \param system : a system of processes
   \return a map from process IDs to the set of weakly synchronized event IDs in each process of system
   */
  template <class LOC, class EDGE>
  tchecker::process_events_map_t weakly_synchronized_events(tchecker::system_t<LOC, EDGE> const & system)
  {
    return tchecker::weakly_synchronized_events(system.synchronizations(), system.processes_count());
  }

  /*!
   \class location_next_syncs_t
   \brief Indicates, for each location, the next synchronisations that can be activated either in the location itself (see NEXT_SYNC_LOCATION)
   or through a (potentially empty) sequence of asynchronous transitions from
   the location (see NEXT_SYNC_REACHABLE)
   or through any path (see ALL_SYNC_REACHABLE)
   */
  class location_next_syncs_t {
  public:
    /*!
     \brief Constructor
     \param locations_count : number of locations
     \param syncs_count : number of synchronisation vectors
     */
    location_next_syncs_t(tchecker::loc_id_t locations_count, tchecker::sync_id_t syncs_count);
    
    /*!
     \brief Copy constructor
     */
    location_next_syncs_t(tchecker::location_next_syncs_t const &) = default;
    
    /*!
     \brief Move constructor
     */
    location_next_syncs_t(tchecker::location_next_syncs_t &&) = default;
    
    /*!
     \brief Destructor
     */
    ~location_next_syncs_t() = default;
    
    /*!
     \brief Assignment operator (deleted)
     */
    tchecker::location_next_syncs_t & operator= (tchecker::location_next_syncs_t const &) = delete;
    
    /*!
     \brief Move-assignment operator (deleted)
     */
    tchecker::location_next_syncs_t & operator= (tchecker::location_next_syncs_t &&) = delete;
    
    /*!
     \brief Type of next synchronisation
     */
    enum next_type_t {
      NEXT_SYNC_LOCATION,  /*!< In the location */
      NEXT_SYNC_REACHABLE, /*!< Reachable from location (through asynchronous transitions) */
      ALL_SYNC_REACHABLE,  /*!< Reachable from localtion */
      NEXT_SYNC_END
    };
    
    /*!
     \brief Add synchronisation
     \param sync_id : synchronisation identifier
     \param loc_id : location identifier
     \param next_type : type of next synchonisations
     \post (loc_id, next_type) !-> sync_id has been added to the map
     */
    void add_next_sync(tchecker::sync_id_t sync_id, tchecker::loc_id_t loc_id, enum next_type_t next_type);
    
    /*!
     \brief Accessor
     \param loc_id : location identifier
     \param next_type : type of next synchronisations
     \return set of next synchronisations of type next_type for location loc_id
     */
    boost::dynamic_bitset<> const & next_syncs(tchecker::loc_id_t loc_id, enum next_type_t next_type) const;
    
    /*!
     \brief Accessor
     \param loc_id : location identifier
     \param next_type : type of next synchronisations
     \return set of next synchronisations of type next_type for location loc_id
     */
    boost::dynamic_bitset<> & next_syncs(tchecker::loc_id_t loc_id, enum next_type_t next_type);
    
    /*!
     \brief Accessor
     \return size of boost::dynamic_bitset<> returned by next_syncs()
     */
    std::size_t next_sync_size() const;
  private:
    std::vector<boost::dynamic_bitset<>> _next_syncs_map[NEXT_SYNC_END]; /*!< Map : (location ID, next sync type) -> sync IDs */
    std::size_t _syncs_count;                                            /*!< Number of synchronizations */
  };
  
  
  
  /*!
   \brief Compute locations next synchronizations
   \tparam LOC : type of locations
   \tparam EDGE : type of edges
   \param system : a system of processes
   \return Map that indicates for each location which synchronisation can be activated in the location, and which synchronisations
   are reachable through a (potentially empty) sequence of asynchronous transitions.
   */
  template <class LOC, class EDGE>
  tchecker::location_next_syncs_t location_next_syncs(tchecker::system_t<LOC, EDGE> const & system)
  {
    tchecker::location_next_syncs_t map(system.locations_count(), system.synchronizations_count());
    
    // Location next syncs
    for (auto const * edge : system.edges())
      for (auto const & sync : system.synchronizations())
        if (sync.synchronizes(edge->pid(), edge->event_id()))
          map.add_next_sync(sync.id(), edge->src()->id(), tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    // Copy to reachable next syncs
    tchecker::loc_id_t const locations_count = system.locations_count();
    for (tchecker::loc_id_t loc_id = 0; loc_id < locations_count; ++loc_id)
      map.next_syncs(loc_id, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE)
      = map.next_syncs(loc_id, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    // Propagate reachable next syncs along asynchronous edges
    bool fixed_point = true;
    do {
      fixed_point = true;
      for (auto const * edge : system.edges())
        if (system.asynchronous(edge->pid(), edge->event_id())) {
          boost::dynamic_bitset<> const & tgt_syncs
          = map.next_syncs(edge->tgt()->id(), tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
          boost::dynamic_bitset<> & src_syncs
          = map.next_syncs(edge->src()->id(), tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
          if (! tgt_syncs.is_subset_of(src_syncs)) {
            fixed_point = false;
            src_syncs |= tgt_syncs;
          }
        }
    }
    while ( ! fixed_point );

    // Copy to all reachable syncs
    for (tchecker::loc_id_t loc_id = 0; loc_id < locations_count; ++loc_id)
      map.next_syncs(loc_id, tchecker::location_next_syncs_t::ALL_SYNC_REACHABLE)
      = map.next_syncs(loc_id, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
      
    // Propagate reachable syncs along edges
    do {
      fixed_point = true;
      for (auto const * edge : system.edges()) {
        boost::dynamic_bitset<> const & tgt_syncs
        = map.next_syncs(edge->tgt()->id(), tchecker::location_next_syncs_t::ALL_SYNC_REACHABLE);
        boost::dynamic_bitset<> & src_syncs
        = map.next_syncs(edge->src()->id(), tchecker::location_next_syncs_t::ALL_SYNC_REACHABLE);
        if (! tgt_syncs.is_subset_of(src_syncs)) {
          fixed_point = false;
          src_syncs |= tgt_syncs;
        }
      }
    }
    while ( ! fixed_point );
        
    return map;
  }
  
  
  
  
  /*!
   \brief Compute locations next global synchronizations
   \tparam LOC : type of locations
   \tparam EDGE : type of edges
   \param system : a system of processes
   \return Map that indicates for each location which global synchronisation can be activated in the location, and which global synchronisations
   are reachable through a (potentially empty) sequence of asynchronous or partially synchronized transitions.
   */
  template <class LOC, class EDGE>
  tchecker::location_next_syncs_t location_next_global_syncs(tchecker::system_t<LOC, EDGE> const & system)
  {
    tchecker::process_id_t const processes_count = system.processes_count();
    
    tchecker::location_next_syncs_t map(system.locations_count(), system.synchronizations_count());
    
    // Location next global syncs
    for (auto const * edge : system.edges())
      for (auto const & sync : system.synchronizations())
        if (sync.synchronizes(edge->pid(), edge->event_id()) && (sync.size() == processes_count))
          map.add_next_sync(sync.id(), edge->src()->id(), tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    // Copy to reachable next global syncs
    tchecker::loc_id_t const locations_count = system.locations_count();
    for (tchecker::loc_id_t loc_id = 0; loc_id < locations_count; ++loc_id)
      map.next_syncs(loc_id, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE)
      = map.next_syncs(loc_id, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    // Checks if a process event (pid, event_id) only appears in global synchronizations
    auto only_globally_sync = [&] (tchecker::process_id_t pid, tchecker::event_id_t event_id) -> bool {
      if (system.asynchronous(pid, event_id))
        return false;
      for (auto const & sync : system.synchronizations())
        if (sync.synchronizes(pid, event_id) && (sync.size() < processes_count))
          return false;
      return true;
    };
    
    // Propagate reachable next global syncs along edges that do not only appear on global synchronisations
    bool fixed_point = true;
    do {
      fixed_point = true;
      for (auto const * edge : system.edges())
        if (! only_globally_sync(edge->pid(), edge->event_id())) {
          boost::dynamic_bitset<> const & tgt_syncs
          = map.next_syncs(edge->tgt()->id(), tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
          boost::dynamic_bitset<> & src_syncs
          = map.next_syncs(edge->src()->id(), tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
          if (! tgt_syncs.is_subset_of(src_syncs)) {
            fixed_point = false;
            src_syncs |= tgt_syncs;
          }
        }
    }
    while ( ! fixed_point );

    // Copy to all reachable global syncs
    for (tchecker::loc_id_t loc_id = 0; loc_id < locations_count; ++loc_id)
      map.next_syncs(loc_id, tchecker::location_next_syncs_t::ALL_SYNC_REACHABLE)
      = map.next_syncs(loc_id, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    // Propagate all reachable syncs over edges
    do {
      fixed_point = true;
      for (auto const * edge : system.edges()) {
        boost::dynamic_bitset<> const & tgt_syncs
        = map.next_syncs(edge->tgt()->id(), tchecker::location_next_syncs_t::ALL_SYNC_REACHABLE);
        boost::dynamic_bitset<> & src_syncs
        = map.next_syncs(edge->src()->id(), tchecker::location_next_syncs_t::ALL_SYNC_REACHABLE);
        if (! tgt_syncs.is_subset_of(src_syncs)) {
          fixed_point = false;
          src_syncs |= tgt_syncs;
        }
      }
    }
    while ( ! fixed_point );

    return map;
  }
  
  
  
  
  /*!
   \brief Checks if a system is global/local
   \tparam LOC : type of locations
   \tparam EDGE : type of edges
   \param system : a system of processes
   \return true if every edge in system involves either a single process or all the processes, false otherwise
   */
  template <class LOC, class EDGE>
  bool global_local(tchecker::system_t<LOC, EDGE> const & system)
  {
    std::size_t const processes_count = system.processes_count();
    for (tchecker::synchronization_t const & sync : system.synchronizations()) {
      std::size_t size = 0;
      for (tchecker::sync_constraint_t const & constr : sync.synchronization_constraints()) {
        if (constr.strength() != tchecker::SYNC_STRONG)
          return false;
        ++ size;
      }
      if (size != processes_count)
        return false;
    }
    return true;
  }
  
  
  
  
  /*!
   \brief Compute locations next server synchronizations
   \tparam LOC : type of locations
   \tparam EDGE : type of edges
   \param system : a system of processes
   \param server : pid of server process
   \return Map that indicates for each location which server synchronisation can be activated in the location, and which server synchronisations
   are reachable through a (potentially empty) sequence of asynchronous or partially synchronized transitions (not involving the server).
   */
  template <class LOC, class EDGE>
  tchecker::location_next_syncs_t location_next_server_syncs(tchecker::system_t<LOC, EDGE> const & system,
                                                             tchecker::process_id_t server)
  {
    // REMOVE
    /*tchecker::process_id_t const processes_count = system.processes_count();*/
    
    tchecker::location_next_syncs_t map(system.locations_count(), system.synchronizations_count());
    
    // Location next server syncs
    for (auto const & sync : system.synchronizations()) {
      if (! sync.synchronizes(server))
        continue;
      
      for (auto const * edge : system.edges())
        if (sync.synchronizes(edge->pid(), edge->event_id()))
          map.add_next_sync(sync.id(), edge->src()->id(), tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    }
    
    // Copy to reachable next server syncs
    tchecker::loc_id_t const locations_count = system.locations_count();
    for (tchecker::loc_id_t loc_id = 0; loc_id < locations_count; ++loc_id)
      map.next_syncs(loc_id, tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE)
      = map.next_syncs(loc_id, tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
    
    // Checks if a process event (pid, event_id) only appears in server synchronizations
    auto only_server_sync = [&] (tchecker::process_id_t pid, tchecker::event_id_t event_id) -> bool {
      if (system.asynchronous(pid, event_id))
        return false;
      for (auto const & sync : system.synchronizations())
        if (sync.synchronizes(pid, event_id) && ! sync.synchronizes(server))
          return false;
      return true;
    };
    
    // Propagate reachable next server syncs along edges that do not only appear on server synchronisations
    bool fixed_point = true;
    do {
      fixed_point = true;
      for (auto const * edge : system.edges())
        if (! only_server_sync(edge->pid(), edge->event_id())) {
          boost::dynamic_bitset<> const & tgt_syncs
          = map.next_syncs(edge->tgt()->id(), tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
          boost::dynamic_bitset<> & src_syncs
          = map.next_syncs(edge->src()->id(), tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
          if (! tgt_syncs.is_subset_of(src_syncs)) {
            fixed_point = false;
            src_syncs |= tgt_syncs;
          }
        }
    }
    while ( ! fixed_point );
        
    return map;
  }
  
  
  
  
  /*!
   \brief Checks if s system is client/server
   \tparam LOC : type of locations
   \tparam EDGE : type of edges
   \param system : a system of processes
   \param server_pid : process ID of server
   \return true if every edge in system involves either a single process or two processes, and one of them if server_pid,
   false otherwise
   */
  template <class LOC, class EDGE>
  bool client_server(tchecker::system_t<LOC, EDGE> const & system, tchecker::process_id_t server_pid)
  {
    for (tchecker::synchronization_t const & sync : system.synchronizations()) {
      std::size_t size = 0;
      bool server_sync = false;
      
      for (tchecker::sync_constraint_t const & constr : sync.synchronization_constraints()) {
        if (constr.strength() != tchecker::SYNC_STRONG)
          return false;
        ++size;
        server_sync = server_sync || (constr.pid() == server_pid);
      }
      
      if (size != 2 || !server_sync)
        return false;
    }
    
    return true;
  }
  
  
  
  
  /*!
   \brief Compute process groups for extended client/server POR
   \tparam LOC : type of locations
   \tparam EDGE : type of edges
   \param system : a system of processes
   \param server_pid : process identifier of the server process
   \return A map : process ID -> group ID such that for any p and q distinct from server_pid, group_id(p) == group_id(q)
   iff (p, q) belongs to the transitive closure of the synchronization relation induced by synchronization vectors in systems
   */
  template <class LOC, class EDGE>
  std::vector<tchecker::process_id_t> client_server_groups(tchecker::system_t<LOC, EDGE> const & system,
                                                           tchecker::process_id_t server_pid)
  {
    tchecker::process_id_t processes_count = system.processes_count();
    std::vector<tchecker::process_id_t> group_id(processes_count, 0);
    
    // Initialize group_id as identity map
    for (tchecker::process_id_t i = 0; i < processes_count; ++i)
      group_id[i] = i;
    
    // Processes that synchronize belong to the same group (identified by smallest pid in the group)
    for (tchecker::synchronization_t const & sync : system.synchronizations()) {
      tchecker::process_id_t smallest_pid = std::numeric_limits<tchecker::process_id_t>::max();
      for (tchecker::sync_constraint_t const & constr : sync.synchronization_constraints())
        if (constr.pid() != server_pid)
          smallest_pid = std::min(smallest_pid, group_id[constr.pid()]);
      
      if (smallest_pid == std::numeric_limits<tchecker::process_id_t>::max())
        continue;
      
      for (tchecker::sync_constraint_t const & constr : sync.synchronization_constraints())
        if (constr.pid() != server_pid)
          group_id[constr.pid()] = smallest_pid;
    }
    
    return group_id;
  }




  /*!
  \class pure_local_map_t
  \brief Tells if a location is pure local or not. A location is pure local if
  all its outgoing edges are asynchronous
  */
  class pure_local_map_t {
  public:
    /*!
    \brief Constructor
    \param loc_count : number of locations
    \param status : pure local status of locations
    */
    pure_local_map_t(tchecker::loc_id_t loc_count, bool status = false);

    /*!
    \brief Copy constructor
    */
    pure_local_map_t(tchecker::pure_local_map_t const &) = default;

    /*!
    \brief Move constructor
    */
    pure_local_map_t(tchecker::pure_local_map_t &&) = default;

    /*!
    \brief Destructor
    */
    ~pure_local_map_t() = default;

    /*!
    \brief Assignment operator
    */
    tchecker::pure_local_map_t & operator= (tchecker::pure_local_map_t const &) = default;
    
    /*!
    \brief More-assignment operator
    */
    tchecker::pure_local_map_t & operator= (tchecker::pure_local_map_t &&) = default;
    
    /*!
    \brief Accessor
    \param id : location ID
    \pre id is a valid location ID (checked by assertion)
    \return true if id is pure local, false otherwise
    */
    bool is_pure_local(tchecker::loc_id_t id) const;

    /*!
    \brief Set location pure local or non pure local
    \param id : location ID
    \param status : pure local status
    \pre id is a valid location ID (checked by assertion)
    \post location id is pure local if status == true, otherwise location id is not pure local
    */
    void set_pure_local(tchecker::loc_id_t id, bool status = true);
  private:
    boost::dynamic_bitset<> _map;  /*!< pure local map : location ID -> bool */
  };


  /*!
  \brief Compute pure local and non pure local locations in a system
  \tparam LOC : type of locations
  \tparam EDGE : type of edges
  \param system : a system of processes
  \return A map : location ID -> bool that tells if a location in system is pure
  local or not (deadlock locations are NOT pure local)
  */
  template <class LOC, class EDGE>
  pure_local_map_t pure_local_map(tchecker::system_t<LOC, EDGE> const & system)
  {
    pure_local_map_t m(system.locations_count(), true);
    // Set all locations with a sync edge non pure local
    for (EDGE const * edge : system.edges())
    {
      if (! system.asynchronous(edge->pid(), edge->event_id()))
        m.set_pure_local(edge->src()->id(), false);
    }
    // Set all deadlock locations non pure local
    for (LOC const * loc : system.locations())
    {
      auto edges = loc->outgoing_edges();
      if (edges.begin() == edges.end())
        m.set_pure_local(loc->id(), false);
    }
    return m;
  }

  /*!
  \brief Compute pure local and non pure local locations in a system
  \tparam LOC : type of locations
  \tparam EDGE : type of edges
  \param system : a system of processes
  \param group_id : Map : process ID -> group ID
  \return A map : location ID -> bool that tells if a location in system is pure
  local or not (deadlock locations are NOT pure local)
  */
  template <class LOC, class EDGE>
  pure_local_map_t pure_local_map(tchecker::system_t<LOC, EDGE> const & system, 
                                  std::vector<tchecker::process_id_t> group_id)
  {
    pure_local_map_t m(system.locations_count(), true);
    // Set all locations with a sync edge outside its group non pure local
    for (EDGE const * edge : system.edges())
    {
      if (! system.asynchronous_groups(edge->pid(), edge->event_id(), group_id))
        m.set_pure_local(edge->src()->id(), false);
    }
    // Set all deadlock locations non pure local
    for (LOC const * loc : system.locations())
    {
      auto edges = loc->outgoing_edges();
      if (edges.begin() == edges.end())
        m.set_pure_local(loc->id(), false);
    }
    return m;
  }

  /*!
  \class pure_sync_map_t
  \brief Tells if a location is pure sync or not. A location is pure sync if
  all its outgoing edges are synchronous
  */
  class pure_sync_map_t {
  public:
    /*!
    \brief Constructor
    \param loc_count : number of locations
    \param status : pure sync status of locations
    */
    pure_sync_map_t(tchecker::loc_id_t loc_count, bool status = false);

    /*!
    \brief Copy constructor
    */
    pure_sync_map_t(tchecker::pure_sync_map_t const &) = default;

    /*!
    \brief Move constructor
    */
    pure_sync_map_t(tchecker::pure_sync_map_t &&) = default;

    /*!
    \brief Destructor
    */
    ~pure_sync_map_t() = default;

    /*!
    \brief Assignment operator
    */
    tchecker::pure_sync_map_t & operator= (tchecker::pure_sync_map_t const &) = default;
    
    /*!
    \brief More-assignment operator
    */
    tchecker::pure_sync_map_t & operator= (tchecker::pure_sync_map_t &&) = default;
    
    /*!
    \brief Accessor
    \param id : location ID
    \pre id is a valid location ID (checked by assertion)
    \return true if id is pure sync, false otherwise
    */
    bool is_pure_sync(tchecker::loc_id_t id) const;

    /*!
    \brief Set location pure sync or non pure sync
    \param id : location ID
    \param status : pure sync status
    \pre id is a valid location ID (checked by assertion)
    \post location id is pure sync if status == true, otherwise location id is not pure sync
    */
    void set_pure_sync(tchecker::loc_id_t id, bool status = true);
  private:
    boost::dynamic_bitset<> _map;  /*!< pure sync map : location ID -> bool */
  };


  /*!
  \brief Compute pure sync and non pure sync locations in a system
  \tparam LOC : type of locations
  \tparam EDGE : type of edges
  \param system : a system of processes
  \return A map : location ID -> bool that tells if a location in system is pure
  sync or not (deadlock locations are NOT pure sync)
  */
  template <class LOC, class EDGE>
  pure_sync_map_t pure_sync_map(tchecker::system_t<LOC, EDGE> const & system)
  {
    pure_sync_map_t m(system.locations_count(), true);
    // Set all locations with a sync edge non pure sync
    for (EDGE const * edge : system.edges())
    {
      if (system.asynchronous(edge->pid(), edge->event_id()))
        m.set_pure_sync(edge->src()->id(), false);
    }
    // Set all deadlock locations non pure sync
    for (LOC const * loc : system.locations())
    {
      auto edges = loc->outgoing_edges();
      if (edges.begin() == edges.end())
        m.set_pure_sync(loc->id(), false);
    }
    return m;
  }

  /*!
  \class mixed_map_t
  \brief Tells if a location is pure sync or not. A location is pure sync if
  all its outgoing edges are synchronous
  */
  class mixed_map_t {
  public:
    /*!
    \brief Constructor
    \param loc_count : number of locations
    \param status : pure sync status of locations
    */
    mixed_map_t(tchecker::loc_id_t loc_count, bool status = false);

    /*!
    \brief Copy constructor
    */
    mixed_map_t(tchecker::mixed_map_t const &) = default;

    /*!
    \brief Move constructor
    */
    mixed_map_t(tchecker::mixed_map_t &&) = default;

    /*!
    \brief Destructor
    */
    ~mixed_map_t() = default;

    /*!
    \brief Assignment operator
    */
    tchecker::mixed_map_t & operator= (tchecker::mixed_map_t const &) = default;
    
    /*!
    \brief More-assignment operator
    */
    tchecker::mixed_map_t & operator= (tchecker::mixed_map_t &&) = default;
    
    /*!
    \brief Accessor
    \param id : location ID
    \pre id is a valid location ID (checked by assertion)
    \return true if id is mixed, false otherwise
    */
    bool is_mixed(tchecker::loc_id_t id) const;

    /*!
    \brief Set location mixed or non mixed
    \param id : location ID
    \param status : mixed status
    \pre id is a valid location ID (checked by assertion)
    \post location id is mixed if status == true, otherwise location id is not mixed
    */
    void set_mixed(tchecker::loc_id_t id, bool status = true);
  private:
    boost::dynamic_bitset<> _map;  /*!< pure sync map : location ID -> bool */
  };

  /*!
  \brief Compute mixed and non mixed locations in a system
  \tparam LOC : type of locations
  \tparam EDGE : type of edges
  \param system : a system of processes
  \return A map : location ID -> bool that tells if a location in system is mixed
  or not (deadlock locations are NOT mixed)
  */
  template <class LOC, class EDGE>
  mixed_map_t mixed_map(tchecker::system_t<LOC, EDGE> const & system)
  {
    mixed_map_t m(system.locations_count(), false);
    // Set all locations with both outgoing sync and local actions mixed
    for (LOC const * loc : system.locations())
    {
      bool has_local = false;
      bool has_sync = false;
      for (EDGE const * edge : loc->outgoing_edges())
      {
        if (system.asynchronous(edge->pid(), edge->event_id()))
          has_local = true;
        else 
          has_sync = true;
      }
      if (has_sync && has_local)
        m.set_mixed(loc->id(), true);
    }
    return m;
  }

   /*!
  \class synchronization_map_t
  \brief Tells if a location has an event satisfying some property.
  */
  class event_map_t {
  public:
    /*!
    \brief Constructor
    \param loc_count : number of locations
    \param status : pure local status of locations
    */
    event_map_t(tchecker::loc_id_t loc_count, bool status = false);

    /*!
    \brief Copy constructor
    */
    event_map_t(tchecker::event_map_t const &) = default;

    /*!
    \brief Move constructor
    */
    event_map_t(tchecker::event_map_t &&) = default;

    /*!
    \brief Destructor
    */
    ~event_map_t() = default;

    /*!
    \brief Assignment operator
    */
    tchecker::event_map_t & operator= (tchecker::event_map_t const &) = default;
    
    /*!
    \brief More-assignment operator
    */
    tchecker::event_map_t & operator= (tchecker::event_map_t &&) = default;
    
    /*!
    \brief Accessor
    \param id : location ID
    \pre id is a valid location ID (checked by assertion)
    \return true if id has an event satisfying some property, false otherwise
    */
    bool has_event(tchecker::loc_id_t id) const;

    /*!
    \brief Set location has synchronization or not
    \param id : location ID
    \param status : pure local status
    \pre id is a valid location ID (checked by assertion)
    \post location id has an event satisfying some property if status == true, otherwise location
    id does not have an event satisfying some property
    */
    void set_event(tchecker::loc_id_t id, bool status = true);
  private:
    boost::dynamic_bitset<> _map;  /*!< pure local map : location ID -> bool */
  };

  /*!
  \brief Compute pure local and non pure local locations in a system
  \tparam LOC : type of locations
  \tparam EDGE : type of edges
  \param system : a system of processes
  \return A map : location ID -> bool that tells if a location in system has an 
  outgoing synchronization or not 
  */
  template <class LOC, class EDGE>
  event_map_t synchronization_map(tchecker::system_t<LOC, EDGE> const & system)
  {
    event_map_t m(system.locations_count(), false);
    // Set all locations with a sync edge 
    for (EDGE const * edge : system.edges())
    {
      if (!system.asynchronous(edge->pid(), edge->event_id()))
        m.set_event(edge->src()->id(), true);
    }
    return m;
  }

    /*!
  \brief Compute pure local and non pure local locations in a system
  \tparam LOC : type of locations
  \tparam EDGE : type of edges
  \param system : a system of processes
  \return A map : location ID -> bool that tells if a location in system has a 
  outgoing local action or not 
  */
  template <class LOC, class EDGE>
  event_map_t local_map(tchecker::system_t<LOC, EDGE> const & system)
  {
    event_map_t m(system.locations_count(), false);
    // Set all locations with a local edge 
    for (EDGE const * edge : system.edges())
    {
      if (system.asynchronous(edge->pid(), edge->event_id()))
        m.set_event(edge->src()->id(), true);
    }
    return m;
  }
} // end of namespace tchecker

#endif // TCHECKER_SYSTEM_STATIC_ANALYSIS_HH
