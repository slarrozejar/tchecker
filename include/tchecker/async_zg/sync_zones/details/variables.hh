/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_VARIABLES_HH
#define TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_VARIABLES_HH

#include "tchecker/async_zg/details/variables.hh"

/*!
 \file variables.hh
 \brief Variables for asynchronous zone graph model with sync zones
 */

namespace tchecker {
  
  namespace async_zg {
    
    namespace sync_zones {
      
      namespace details {
        
        /*!
         \class variables_t
         \brief Model variables for asynchronous zone graphs with sync zones
         */
        class variables_t : public tchecker::async_zg::details::variables_t {
        public:
          using tchecker::async_zg::details::variables_t::variables_t;
        };
        
      } // end of namespace details
      
    } // end of namespace sync_zones
    
  } // end of namespace async_zg
  
} // end of namespace tchecker

#endif // TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_VARIABLES_HH




