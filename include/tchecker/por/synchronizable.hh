/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_SYNCHRONIZABLE_HH
#define TCHECKER_POR_SYNCHRONIZABLE_HH

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
     \pre rank < vloc.size() (checked by assertion)
     \return true if all the processes can reach a common global action from vloc (processes below rank shall be able to do this
     global action from their current location), false otherwise
     */
    template <class VLOC>
    bool synchronizable(VLOC const & vloc,
                        tchecker::process_id_t rank,
                        tchecker::location_next_syncs_t const & lns)
    {
      tchecker::process_id_t const vloc_size = vloc.size();
      tchecker::sync_id_t const next_syncs_size = lns.next_sync_size();
      
      boost::dynamic_bitset<> next_syncs(next_syncs_size);
      next_syncs.set();
      
      for (tchecker::process_id_t pid = 0; pid < vloc_size; ++pid) {
        if (pid < rank)
          next_syncs &= lns.next_syncs(vloc[pid]->id(), tchecker::location_next_syncs_t::NEXT_SYNC_LOCATION);
        else
          next_syncs &= lns.next_syncs(vloc[pid]->id(), tchecker::location_next_syncs_t::NEXT_SYNC_REACHABLE);
                                       
        if (next_syncs.none())
          return false;
      }
      
      return true;
    }
    
  } // end of namespace por
  
} // end of namespace tchecker

#endif // TCHECKER_POR_SYNCHRONIZABLE_HH

