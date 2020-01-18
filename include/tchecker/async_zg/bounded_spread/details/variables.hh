/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_VARIABLES_HH
#define TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_VARIABLES_HH

#include "tchecker/async_zg/details/variables.hh"

/*!
 \file variables.hh
 \brief Variables for asynchronous zone graph model with bounded spread
 */

namespace tchecker {
  
  namespace async_zg {
    
    namespace bounded_spread {
      
      namespace details {
        
        /*!
         \class variables_t
         \brief Model variables for asynchronous zone graphs with bounded spread
         */
        class variables_t : public tchecker::async_zg::details::variables_t {
        public:
          using tchecker::async_zg::details::variables_t::variables_t;
        };
        
      } // end of namespace details
      
    } // end of namespace bounded_spread
    
  } // end of namespace async_zg
  
} // end of namespace tchecker

#endif // TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_VARIABLES_HH




