/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_OUTPUT_HH
#define TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_OUTPUT_HH

#include "tchecker/async_zg/details/output.hh"
#include "tchecker/async_zg/bounded_spread/details/state.hh"

/*!
 \file output.hh
 \brief Outputters for asynchronous zone graphs with bounded spread
 */

namespace tchecker {
  
  namespace async_zg {
    
    namespace bounded_spread {
      
      namespace details {
        
        /*!
         \class state_outputter_t
         \brief Outputter for states
         */
        class state_outputter_t : public tchecker::async_zg::details::state_outputter_t {
        public:
          using tchecker::async_zg::details::state_outputter_t::state_outputter_t;
        };
        
        
        
        
        /*!
         \class transition_ouputter_t
         \brief Transition outputter
         */
        class transition_outputter_t : public tchecker::async_zg::details::transition_outputter_t {
        public:
          using tchecker::async_zg::details::transition_outputter_t::transition_outputter_t;
        };
        
      } // end of namespace details
      
    } // end of namespace bounded_spread
    
  } // end of namespace async_zg
  
} // end of namespace tchecker

#endif // TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_OUTPUT_HH


