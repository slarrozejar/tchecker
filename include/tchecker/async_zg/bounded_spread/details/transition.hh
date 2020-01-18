/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_TRANSITION_HH
#define TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_TRANSITION_HH

#include "tchecker/async_zg/details/transition.hh"

/*!
 \file transition.hh
 \brief Transition of asynchronous zone graphs with bounded spread (details)
 */

namespace tchecker {
  
  namespace async_zg {
    
    namespace bounded_spread {
      
      namespace details {
        
        /*!
         \class transition_t
         \brief Transition of asynchronous zone graphs with bounded spread
         */
        class transition_t : public tchecker::async_zg::details::transition_t {
        public:
          using tchecker::async_zg::details::transition_t::transition_t;
        };
        
      } // end of namespace details
      
    } // end of namespace bounded_spread
    
  } // end of namespace async_zg
  
} // end of namespace tchecker

#endif // TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_TRANSITION_HH


