/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_TRANSITION_HH
#define TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_TRANSITION_HH

#include "tchecker/async_zg/details/transition.hh"

/*!
 \file transition.hh
 \brief Transition of asynchronous zone graphs with sync zones (details)
 */

namespace tchecker {
  
  namespace async_zg {
    
    namespace sync_zones {
      
      namespace details {
        
        /*!
         \class transition_t
         \brief Transition of asynchronous zone graphs with sync zones
         */
        class transition_t : public tchecker::async_zg::details::transition_t {
        public:
          using tchecker::async_zg::details::transition_t::transition_t;
        };
        
      } // end of namespace details
      
    } // end of namespace sync_zones
    
  } // end of namespace async_zg
  
} // end of namespace tchecker

#endif // TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_TRANSITION_HH


