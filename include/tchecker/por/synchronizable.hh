/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_SYNCHRONIZABLE_HH
#define TCHECKER_POR_SYNCHRONIZABLE_HH

#include <set>

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

#include "tchecker/basictypes.hh"
#include "tchecker/system/static_analysis.hh"

/*!
 \file synchronizable.hh
 \brief Synchronization check for POR transition systems
 */

namespace tchecker {
  
  namespace por {
    
    /*!
     \brief Check if a tuple of locations can lead to a global action
     \param vloc : tuple of locations
     \param rank : rank
     \param lns : location next sync
     \return true if all the processes can reach a common global action from vloc, false otherwise
     \note processes < rank shall have a common global action that is also reachable from vloc for all other processes
     */
    template <class VLOC>
    bool synchronizable_global(VLOC const & vloc,
                               tchecker::process_id_t rank,
                               tchecker::location_next_syncs_t const & lns)
    {
      tchecker::process_id_t const vloc_size = vloc.size();
      tchecker::sync_id_t const next_syncs_size = lns.next_sync_size();
      
      boost::dynamic_bitset<> next_syncs(next_syncs_size);
      next_syncs.set();
      
      for (tchecker::process_id_t pid = 0; pid < vloc_size; ++pid) {
        enum tchecker::location_next_syncs_t::next_type_t next_type
        = (pid < rank
           ? tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION
           : tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
        
        next_syncs &= lns.next_syncs(vloc[pid]->id(), next_type);
                                       
        if (next_syncs.none())
          return false;
      }
      
      return true;
    }
    
    
    
    /*!
     \brief Check if a tuple of locations can lead to a communication for a given process
     \param vloc : tuple of locations
     \param pid : process identifier
     \param server_pid : server process identifier
     \param lns : location next syncs
     \return true if server process server_pid can do a sync action that reachable for process pid from vloc, false otherwise
     */
    template <class VLOC>
    bool synchronizable_server(VLOC const & vloc,
                               tchecker::process_id_t pid,
                               tchecker::process_id_t server_pid,
                               tchecker::location_next_syncs_t const & lns)
    {
      boost::dynamic_bitset<> next_syncs = lns.next_syncs(vloc[server_pid]->id(),
                                                          tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
      next_syncs &= lns.next_syncs(vloc[pid]->id(), tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
      
      return next_syncs.any();
    }
    
    
    /*!
     \brief Check if a tuple of locations can lead to a communication for a given group of processes
     \param vloc : tuple of locations
     \param group : set of processes
     \param server_pid : server process identifier
     \param lns : location next syncs with server
     \return true if server process server_pid can do a sync action from vloc that reachable form vloc  for some process in group, false otherwise
     */
    template <class VLOC>
    bool synchronizable_group_server(VLOC const & vloc,
                                     std::set<tchecker::process_id_t> const & group,
                                     tchecker::process_id_t server_pid,
                                     tchecker::location_next_syncs_t const & lns)
    {
      boost::dynamic_bitset<> server_syncs = lns.next_syncs(vloc[server_pid]->id(),
                                                            tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
      for (tchecker::process_id_t pid : group) {
        boost::dynamic_bitset<> next_syncs = lns.next_syncs(vloc[pid]->id(),
                                                            tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
        next_syncs &= server_syncs;
        if (next_syncs.any())
          return true;
      }
      return false;
    }
    
  } // end of namespace por
  
} // end of namespace tchecker

#endif // TCHECKER_POR_SYNCHRONIZABLE_HH

