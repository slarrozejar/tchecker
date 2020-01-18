/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_ALLOCATORS_HH
#define TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_ALLOCATORS_HH

#include "tchecker/async_zg/details/allocators.hh"

/*!
 \file allocators.hh
 \brief Allocators for asynchronous zone graphs with sync zones (details)
 */

namespace tchecker {
  
  namespace async_zg {
    
    namespace sync_zones {
      
      namespace details {
        
        /*!
         \class state_pool_allocator_t
         \brief Pool allocator for asynchronous zone graph states
         \tparam STATE : type of state, should inherit from tchecker::async_zg::details::state_t
         \tparam VLOC : type of tuple of locations
         \tparam INTVARS_VAL : type of integer variables valuations
         \tparam OFFSET_ZONE : type of offset zones
         \tparam SYNC_ZONE : type of synchronized zones
         */
        template
        <class STATE,
        class VLOC=typename STATE::vloc_t,
        class INTVARS_VAL=typename STATE::intvars_valuation_t,
        class OFFSET_ZONE=typename STATE::offset_zone_t,
        class SYNC_ZONE=typename STATE::sync_zone_t
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
           \brief Type of allocated synchronized zones
           */
          using sync_zone_t = SYNC_ZONE;
          
          /*!
           \brief Type of allocated objects (i.e. states)
           */
          using t = state_t;
          
          /*!
           \brief Type of pointer to allocated objects (pointer to state)
           */
          using ptr_t = tchecker::intrusive_shared_ptr_t<state_t>;
          
          /*!
           \brief Constructor
           \param state_alloc_nb : number of states allocated in one block
           \param vloc_alloc_nb : number of tuple of locations allocated in one block
           \param vloc_capacity : capacity of allocated tuples of locations
           \param intvars_val_alloc_nb : number of integer variables valuations allocated in one block
           \param intvars_val_capacity : capacity of allocated integer variables valuations
           \param zone_alloc_nb : number of offset/sync zones allocated in one block
           \param offset_zone_dimension : dimension of allocated offset zones
           \param sync_zone_dimension : dimension of allocated synchronized zones
           */
          state_pool_allocator_t(std::size_t state_alloc_nb,
                                 std::size_t vloc_alloc_nb,
                                 std::size_t vloc_capacity,
                                 std::size_t intvars_val_alloc_nb,
                                 std::size_t intvars_val_capacity,
                                 std::size_t zone_alloc_nb,
                                 tchecker::clock_id_t offset_zone_dimension,
                                 tchecker::clock_id_t sync_zone_dimension)
          : tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL>
          (state_alloc_nb,
           vloc_alloc_nb, vloc_capacity,
           intvars_val_alloc_nb, intvars_val_capacity,
           zone_alloc_nb, offset_zone_dimension
           ),
          _sync_zone_dimension(sync_zone_dimension),
          _sync_zone_pool(zone_alloc_nb,
                          tchecker::allocation_size_t<SYNC_ZONE>::alloc_size(_sync_zone_dimension))
          {}
          
          /*!
           \brief Copy constructor (deleted)
           */
          state_pool_allocator_t
          (tchecker::async_zg::sync_zones::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL,
           OFFSET_ZONE, SYNC_ZONE> const &) = delete;
          
          /*!
           \brief Move constructor (deleted)
           */
          state_pool_allocator_t(tchecker::async_zg::sync_zones::details::state_pool_allocator_t<STATE, VLOC,
                                 INTVARS_VAL, OFFSET_ZONE, SYNC_ZONE> &&) = delete;
          
          /*!
           \brief Destructor
           */
          ~state_pool_allocator_t()
          {
            destruct_all();
          }
          
          /*!
           \brief Assignment operator (deleted)
           */
          tchecker::async_zg::sync_zones::details::state_pool_allocator_t
          <STATE, VLOC, INTVARS_VAL, OFFSET_ZONE, SYNC_ZONE> &
          operator=
          (tchecker::async_zg::sync_zones::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL,
           OFFSET_ZONE, SYNC_ZONE> const &) = delete;
          
          /*!
           \brief Move-assignment operator (deleted)
           */
          tchecker::async_zg::sync_zones::details::state_pool_allocator_t
          <STATE, VLOC, INTVARS_VAL, OFFSET_ZONE, SYNC_ZONE> &
          operator=
          (tchecker::async_zg::sync_zones::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL,
           OFFSET_ZONE, SYNC_ZONE> &&) = delete;
          
          /*!
           \brief Construct state
           \param args : arguments to a constructor of STATE beyond tuple of locations, integer variables valuation and zone
           \return a new instance of STATE constructed from a newly allocated vloc, a newly allocated valuation of integer
           variables, a newly allocated zone, and args
           */
          template <class ... ARGS>
          tchecker::intrusive_shared_ptr_t<STATE> construct(ARGS && ... args)
          {
            return tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL,
            OFFSET_ZONE>::construct(args..., _sync_zone_pool.construct(_sync_zone_dimension));
          }
          
          /*!
           \brief Construct state
           \param state : a state
           \param args : arguments to a constructor of STATE beyond tuple of locations, integer variables valuation and zone
           \return a new instance of STATE constructed from state, a copy of the zone in state, and args
           */
          template <class ... ARGS>
          tchecker::intrusive_shared_ptr_t<STATE> construct_from_state(STATE const & state, ARGS && ... args)
          {
            return tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL,
            OFFSET_ZONE>::construct_from_state(state, args..., _sync_zone_pool.construct(state.sync_zone()));
          }
          
          /*!
           \brief Destruct state
           \param p : pointer to state
           \pre p has been constructed by this allocator.
           p is not nullptr
           \post the state pointed by p has been destructed if its reference counter is 1 (i.e. p is the only
           reference to the state), and p points to nullptr. Does nothing otherwise.
           The tuple of locations, integer variables valuation, offset zone and synchronized zone have been destructed
           (if the state was the only one pointing to them)
           \return true if the state has been destructed, false otherwise
           */
          bool destruct(tchecker::intrusive_shared_ptr_t<STATE> & p)
          {
            if (p.ptr() == nullptr)
              return false;
            
            auto sync_zone_ptr = p->sync_zone_ptr();
            
            if ( ! tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL,
                OFFSET_ZONE>::destruct(p) )
              return false;
            
            _sync_zone_pool.destruct(sync_zone_ptr);
            
            return true;
          }
          
          /*!
           \brief Collect unused states
           \post Unused states, unused tuples of locations, unused integer variables valuations and unused zones have been
           collected
           */
          void collect()
          {
            tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL,
            OFFSET_ZONE>::collect();
            _sync_zone_pool.collect();
          }
          
          /*!
           \brief Destruct all allocated states
           \post All allocated states, tuples of locations, integer variables valuations, and zones have been destructed
           */
          void destruct_all()
          {
            tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL,
            OFFSET_ZONE>::destruct_all();
            _sync_zone_pool.destruct_all();
          }
          
          /*!
           \brief Free all allocated memory
           \post All allocated states, tuples of locations, integer variables valuations, and zones have been freed.
           No destructor has been called
           */
          void free_all()
          {
            tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL,
            OFFSET_ZONE>::free_all();
            _sync_zone_pool.free_all();
          }
          
          /*!
           \brief Accessor
           \return Memory used by this state allocator
           */
          std::size_t memsize() const
          {
            return tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL,
            OFFSET_ZONE>::memsize()
            + _sync_zone_pool.memsize();
          }
          
          /*!
           \brief Enroll on garbage collector
           \param gc : garbage collector
           \post The state, vloc, integer variables valuations, and zone pool allocators have been enrolled on gc
           \note this should be enrolled on at most one GC
           */
          void enroll(tchecker::gc_t & gc)
          {
            tchecker::async_zg::details::state_pool_allocator_t<STATE, VLOC, INTVARS_VAL,
            OFFSET_ZONE>::enroll(gc);
            _sync_zone_pool.enroll(gc);
          }
        protected:
          tchecker::clock_id_t _sync_zone_dimension;        /*!< Dimension of allocated synchronized zones */
          tchecker::pool_t<SYNC_ZONE> _sync_zone_pool;      /*!< Pool of synchronized zones */
        };
        
        
        
        
        /*!
         \class transition_singleton_allocator_t
         \brief Singleton allocator for transitions
         \tparam TRANSITION : type of transition, should derive from tchecker::async_zg::details::transition_t
         */
        template <class TRANSITION>
        class transition_singleton_allocator_t : public tchecker::async_zg::details::transition_singleton_allocator_t<TRANSITION> {
        public:
          using tchecker::async_zg::details::transition_singleton_allocator_t<TRANSITION>
          ::transition_singleton_allocator_t;
        };
        
      } // end of namespace details
      
    } // end of namespace sync_zones
    
  } // end of namespace async_zg
  
} // end of namespace tchecker

#endif // TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_ALLOCATORS_HH

