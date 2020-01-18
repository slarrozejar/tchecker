/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_ALLOCATORS_HH
#define TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_ALLOCATORS_HH

#include "tchecker/async_zg/details/allocators.hh"

/*!
 \file allocators.hh
 \brief Allocators for asynchronous zone graphs with bounded spread (details)
 */

namespace tchecker {
  
  namespace async_zg {
    
    namespace bounded_spread {
      
      namespace details {
        
        /*!
         \class state_pool_allocator_t
         \brief Pool allocator for bounded-spread asynchronous zone graph states
         \tparam STATE : type of state, should inherit from tchecker::async_zg::details::state_t
         \tparam VLOC : type of tuple of locations
         \tparam INTVARS_VAL : type of integer variables valuations
         \tparam OFFSET_ZONE : type of offset zones
         */
        template
        <class STATE,
        class VLOC=typename STATE::vloc_t,
        class INTVARS_VAL=typename STATE::intvars_valuation_t,
        class OFFSET_ZONE=typename STATE::offset_zone_t
        >
        class state_pool_allocator_t
        : protected tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL, OFFSET_ZONE> {
        public:
          /*!
           \brief Type of allocated states
           */
          using state_t
          = typename tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL,
          OFFSET_ZONE>::state_t;
          
          /*!
           \brief Type of allocated tuples of locations
           */
          using vloc_t
          = typename tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL,
          OFFSET_ZONE>::vloc_t;
          
          /*!
           \brief Type of allocated integer variables valuations
           */
          using intvars_valuation_t
          = typename tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC,
          INTVARS_VAL, OFFSET_ZONE>::intvars_valuation_t;
          
          /*!
           \brief Type of allocated offset zones
           */
          using offset_zone_t
          = typename tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC,
          INTVARS_VAL, OFFSET_ZONE>::offset_zone_t;
          
          /*!
           \brief Type of allocated objects (i.e. states)
           */
          using t = state_t;
          
          /*!
           \brief Type of pointer to allocated objects (pointer to state)
           */
          using ptr_t = tchecker::intrusive_shared_ptr_t<state_t>;
          
          
          using tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL, OFFSET_ZONE>
          ::state_pool_allocator_t;
        };
        
        
        
        
        /*!
         \class transition_singleton_allocator_t
         \brief Singleton allocator for transitions
         \tparam TRANSITION : type of transition, should derive from tchecker::async_zg::details::transition_t
         */
        template <class TRANSITION>
        class transition_singleton_allocator_t
        : public tchecker::async_zg::details::transition_singleton_allocator_t<TRANSITION> {
        public:
          using tchecker::async_zg::details::transition_singleton_allocator_t<TRANSITION>
          ::transition_singleton_allocator_t;
        };
        
      } // end of namespace details
      
    } // end of namespace bounded_spread
    
  } // end of namespace async_zg
  
} // end of namespace tchecker

#endif // TCHECKER_ASYNC_ZG_bounded_spread_DETAILS_ALLOCATORS_HH

