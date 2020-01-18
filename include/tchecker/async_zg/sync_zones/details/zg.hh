/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_ZG_HH
#define TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_ZG_HH

#include "tchecker/async_zg/details/zg.hh"
#include "tchecker/basictypes.hh"
#include "tchecker/dbm/offset_dbm.hh"
#include "tchecker/variables/clocks.hh"

/*!
 \file zg.hh
 \brief Asynchronous zone graph with sync zones (details)
 */

namespace tchecker {
  
  namespace async_zg {
    
    namespace sync_zones {
      
      namespace details {
        
        /*!
         \class zg_t
         \brief Asynchronous zone graph (details)
         \tparam TA : type of timed automaton, should inherit from tchecker::ta::details::ta_t
         \tparam ASYNC_ZONE_SEMANTICS : type of asynchronous zone semantics, should implement tchecker::zone::semantics_t
         */
        template <class TA, class ASYNC_ZONE_SEMANTICS>
        class zg_t : protected tchecker::async_zg::details::zg_t<TA, ASYNC_ZONE_SEMANTICS> {
        public:
          /*!
           \brief Type of model
           */
          using model_t = typename tchecker::async_zg::details::zg_t<TA, ASYNC_ZONE_SEMANTICS>::model_t;
          
          /*!
           \brief Type of tuple of locations
           */
          using vloc_t = typename tchecker::async_zg::details::zg_t<TA, ASYNC_ZONE_SEMANTICS>::vloc_t;
          
          /*!
           \brief Type of valuation of bounded integer variables
           */
          using intvars_valuation_t
          = typename tchecker::async_zg::details::zg_t<TA, ASYNC_ZONE_SEMANTICS>::intvars_valuation_t;
          
          /*!
           \brief Type of offset zones
           */
          using offset_zone_t
          = typename tchecker::async_zg::details::zg_t<TA, ASYNC_ZONE_SEMANTICS>::offset_zone_t;
          
          /*!
           \brief Type of synchronized zones
           */
          using sync_zone_t = typename ASYNC_ZONE_SEMANTICS::sync_zone_t;
          
          
          /*!
           \brief Constructor
           \tparam ASYNC_MODEL : type of model, should derive from tchecker::async_zg::sync_zones::details::model_t
           \param model : a model
           \note this keeps a pointer to the refmap in model
           */
          template <class ASYNC_MODEL>
          explicit zg_t(ASYNC_MODEL & model)
          : tchecker::async_zg::details::zg_t<TA, ASYNC_ZONE_SEMANTICS>(model)
          {}
          
          /*!
           \brief Copy constructor
           */
          zg_t(tchecker::async_zg::sync_zones::details::zg_t<TA, ASYNC_ZONE_SEMANTICS> const &) = default;
          
          /*!
           \brief Move constructor
           */
          zg_t(tchecker::async_zg::sync_zones::details::zg_t<TA, ASYNC_ZONE_SEMANTICS> &&) = default;
          
          /*!
           \brief Destructor
           */
          ~zg_t() = default;
          
          /*!
           \brief Assignment operator
           \param zg : a zone graph
           \post this is a copy of zg
           \return this after assignment
           */
          tchecker::async_zg::sync_zones::details::zg_t<TA, ASYNC_ZONE_SEMANTICS> &
          operator= (tchecker::async_zg::sync_zones::details::zg_t<TA, ASYNC_ZONE_SEMANTICS> const & zg)
          = default;
          
          /*!
           \brief Move-assignment operator
           \param zg : a zone graph
           \post zg has been moved to this
           \return this after assignment
           */
          tchecker::async_zg::sync_zones::details::zg_t<TA, ASYNC_ZONE_SEMANTICS> &
          operator= (tchecker::async_zg::sync_zones::details::zg_t<TA, ASYNC_ZONE_SEMANTICS> && zg) = default;
          
          /*!
           \brief Type of iterator over initial states
           */
          using initial_iterator_t
          = typename tchecker::async_zg::details::zg_t<TA, ASYNC_ZONE_SEMANTICS>::initial_iterator_t;
          
          /*!
           \brief Accessor
           \return iterator over initial states
           */
          inline tchecker::range_t<initial_iterator_t> initial() const
          {
            return tchecker::async_zg::details::zg_t<TA, ASYNC_ZONE_SEMANTICS>::initial();
          }
          
          /*!
           \brief Dereference type for iterator over initial states
           */
          using initial_iterator_value_t
          = typename tchecker::async_zg::details::zg_t<TA, ASYNC_ZONE_SEMANTICS>::initial_iterator_value_t;
        
          /*!
           \brief Initialize state
           \param vloc : tuple of locations
           \param intvars_val : valuation of integer variables
           \param offset_zone : an offset zone
           \param sync_zone : a synchronized zone
           \param initial_range : range of locations
           \param invariant : a tchecker::clock_constraint_t container
           \pre sync_zone has dimension corresponding to number of (non-offset) clocks. See initialize() for other parameters
           \post sync_zone is the set of synchronized valuations in offset_zone. See initialize() for other parameters
           \return STATE_EMPTY_ZONE if sync_zone is empty. See initialize() for other return values
           */
          enum tchecker::state_status_t initialize(vloc_t & vloc,
                                                   intvars_valuation_t & intvars_val,
                                                   offset_zone_t & offset_zone,
                                                   sync_zone_t & sync_zone,
                                                   initial_iterator_value_t const & initial_range,
                                                   tchecker::clock_constraint_container_t & invariant)
          {
            auto status = this->_ta.initialize(vloc, intvars_val, initial_range, invariant);
            if (status != tchecker::STATE_OK)
              return status;
            this->translate_invariant(invariant);
            tchecker::ta::delay_allowed(vloc, this->_src_delay_allowed);
            return this->_async_zone_semantics.initialize(offset_zone, sync_zone, this->_src_delay_allowed,
                                                          this->_offset_src_invariant, vloc);
          }
          
          /*!
           \brief Type of iterator over outgoing edges
           */
          using outgoing_edges_iterator_t
          = typename tchecker::async_zg::details::zg_t<TA, ASYNC_ZONE_SEMANTICS>::outgoing_edges_iterator_t;
          
          /*!
           \brief Accessor
           \param vloc : tuple of locations
           \return range of outgoing synchronized and asynchronous edges from vloc
           */
          inline tchecker::range_t<outgoing_edges_iterator_t> outgoing_edges(vloc_t const & vloc) const
          {
            return tchecker::async_zg::details::zg_t<TA, ASYNC_ZONE_SEMANTICS>::outgoing_edges(vloc);
          }
          
          /*!
           \brief Dereference type for iterator over outgoing edges
           */
          using outgoing_edges_iterator_value_t
          = typename tchecker::async_zg::details::zg_t<TA, ASYNC_ZONE_SEMANTICS>::outgoing_edges_iterator_value_t;
          
          /*!
           \brief Compute next state
           \param vloc : tuple of locations
           \param intvars_val : valuation of integer variables
           \param offset_zone : an offset zone
           \param sync_zone : a synchronized zone
           \param vedge : range of synchronized edges
           \param src_invariant : a tchecker::clock_constraint_t container
           \param guard : a tchecker::clock_constraint_t container
           \param clkreset : a tchecker::clock_reset_t container
           \param tgt_invariant : a tchecker::clock_constraint_t container
           \pre sync_zone has dimension corresponding to number of (non-offset) clocks. See next() for other parameters
           \post sync_zone is the set of synchronized valuations in offset_zone. See next() for other parameters
           \return STATE_EMPTY_ZONE if sync_zone is empty. See next() for other return values
           */
          enum tchecker::state_status_t next(vloc_t & vloc,
                                             intvars_valuation_t & intvars_val,
                                             offset_zone_t & offset_zone,
                                             sync_zone_t & sync_zone,
                                             outgoing_edges_iterator_value_t const & vedge,
                                             tchecker::clock_constraint_container_t & src_invariant,
                                             tchecker::clock_constraint_container_t & guard,
                                             tchecker::clock_reset_container_t & clkreset,
                                             tchecker::clock_constraint_container_t & tgt_invariant)
          {
            tchecker::ta::delay_allowed(vloc, this->_src_delay_allowed);
            auto status = this->_ta.next(vloc, intvars_val, vedge, src_invariant, guard, clkreset,
                                         tgt_invariant);
            if (status != tchecker::STATE_OK)
              return status;
            tchecker::ta::delay_allowed(vloc, this->_tgt_delay_allowed);
            this->translate_guard_reset_invariants(src_invariant, guard, clkreset, tgt_invariant);
            this->reference_clock_synchronization(vedge, this->_offset_guard);
            return this->_async_zone_semantics.next(offset_zone,
                                                    sync_zone,
                                                    this->_src_delay_allowed,
                                                    this->_offset_src_invariant,
                                                    this->_offset_guard,
                                                    this->_offset_clkreset,
                                                    this->_tgt_delay_allowed,
                                                    this->_offset_tgt_invariant,
                                                    vloc);
            
          }
          
          /*!
           \brief Accessor
           \return Underlying model
           */
          inline constexpr model_t const & model() const
          {
            return tchecker::async_zg::details::zg_t<TA, ASYNC_ZONE_SEMANTICS>::model();
          }
        };
        
      } // end of namespace details
      
    } // end of namespace sync_zones
    
  } // end of namespace async_zg
  
} // end of namespace tchecker

#endif // TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_ZG_HH

