/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_SYSTEM_STATIC_ANALYSIS_HH
#define TCHECKER_SYSTEM_STATIC_ANALYSIS_HH

#include <vector>

#include <boost/container/flat_set.hpp>
#include <boost/dynamic_bitset.hpp>

#include "tchecker/basictypes.hh"
#include "tchecker/system/synchronization.hh"
#include "tchecker/system/system.hh"

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
   \class location_sync_flag_t
   \brief Indicates for each location in a system whether it has an outgoing sychronized event
   */
  class location_sync_flag_t {
  public:
    /*!
     \brief Constructor
     \param locations_count : number of locations
     \post All flags are unset
     */
    location_sync_flag_t(tchecker::loc_id_t locations_count);
    
    /*!
     \brief Copy constructor
     */
    location_sync_flag_t(tchecker::location_sync_flag_t const &) = default;
    
    /*!
     \brief Move constructor
     */
    location_sync_flag_t(tchecker::location_sync_flag_t &&) = default;
    
    /*!
     \brief Destructor
     */
    ~location_sync_flag_t() = default;
    
    /*!
     \brief Assignment operator (deleted)
     */
    tchecker::location_sync_flag_t & operator= (tchecker::location_sync_flag_t const &) = delete;
    
    /*!
     \brief Move-assignment operator (deleted)
     */
    tchecker::location_sync_flag_t & operator= (tchecker::location_sync_flag_t &&) = delete;
    
    /*!
     \brief Set synchronisation
     \param loc_id : location ID
     \pre loc_id < number of locations (checked by assertion)
     \post The sync flag for location loc_id has been set
     */
    void sync(tchecker::loc_id_t loc_id);
    
    /*!
     \brief Accessor
     \param loc_id : location ID
     \pre loc_id < number of locations (checked by assertion)
     \return true if loc_id has an outgoing synchronized event, false otherwise
     */
    bool has_synchronized_event(tchecker::loc_id_t loc_id) const;
  private:
    boost::dynamic_bitset<> _flags; /*!< Flags: whether locations have an outgoing synchronized event */
  };
    
  
  
  /*!
   \brief Compute locations synchronization flags
   \tparam LOC : type of locations
   \tparam EDGE : type of edges
   \param system : a system of processes
   \return flags that indicates for each location whether it has an outgoing synchronized event or not
   */
  template <class LOC, class EDGE>
  tchecker::location_sync_flag_t location_synchronisation_flags(tchecker::system_t<LOC, EDGE> const & system)
  {
    tchecker::location_sync_flag_t flags(system.locations_count());
    for (auto const * edge : system.edges())
      if (! system.asynchronous(edge->pid(), edge->event_id()))
        flags.sync(edge->src()->id());
    return flags;
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
  
} // end of namespace tchecker

#endif // TCHECKER_SYSTEM_STATIC_ANALYSIS_HH
