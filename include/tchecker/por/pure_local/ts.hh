/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_PURE_LOCAL_TS_HH
#define TCHECKER_POR_PURE_LOCAL_TS_HH

#include <algorithm>
#include <cstring>
#include <limits>
#include <type_traits>

#include "tchecker/basictypes.hh"
#include "tchecker/dbm/offset_dbm.hh"
#include "tchecker/flat_system/vedge.hh"
#include "tchecker/por/pure_local/state.hh"
#include "tchecker/por/ts.hh"
#include "tchecker/system/static_analysis.hh"

/*!
 \file ts.hh
 \brief Transition system with POR with priority to first pure-local process. A
 process is pure local if its current state only has local transitions
 */

namespace tchecker {
  
  namespace por {
    
    namespace pure_local {
      
      /*! pure-local pid value when no pure local process */
      constexpr tchecker::process_id_t const no_pure_local = std::numeric_limits<tchecker::process_id_t>::max();

      /*!
       \brief Transition system with POR with priority to first pure-local
       process
       \tparam TS : type of underlying transition system, should implement tchecker::ts::ts_t
       \tparam STATE : type of states, should inherit from TS::state_t, and from tchecker::por::pure_local::state_t
       \note ts_t<TS> implements partial-order reduction on top of TS, using global/local source sets
       */
      template <class TS, class STATE>
      class ts_t final : public tchecker::por::ts_t<TS, STATE> {
        using base_ts_t = tchecker::por::ts_t<TS, STATE>;

        static_assert(std::is_base_of<tchecker::por::pure_local::state_t, STATE>::value, "");

      public:
        /*!
         \brief Constructor
         \param model : a model
         \note TS should have a constructor TS(MODEL &)
         */
        template <class MODEL, class ... ARGS>
        ts_t(MODEL & model, ARGS && ... args)
        : base_ts_t(model, args...),
        pure_local_map(tchecker::pure_local_map(model.system()))
        {}
        
        /*!
         \brief Copy constructor
         */
        ts_t(tchecker::por::pure_local::ts_t<TS, STATE> const &) = default;
        
        /*!
         \brief Copy constructor
         */
        ts_t(tchecker::por::pure_local::ts_t<TS, STATE> &&) = default;
        
        /*!
         \brief Destructor
         */
        ~ts_t() = default;

        /*!
         \brief Assignment operator
         */
        tchecker::por::pure_local::ts_t<TS, STATE> & operator= (tchecker::por::pure_local::ts_t<TS, STATE> const &) = default;
        
        /*!
         \brief Move-assignment operator
         */
        tchecker::por::pure_local::ts_t<TS, STATE> & operator= (tchecker::por::pure_local::ts_t<TS, STATE> &&) = default;
        

        /*!
         \brief Initialize state
         \param s : state
         \param t : transition
         \param v : initial state valuation
         \post state s and transition t have been initialized from v
         \return status of state s after initialization
         \note t represents an initial transition to s
         */
        virtual enum tchecker::state_status_t initialize(STATE & s,
                                                         typename TS::transition_t & t,
                                                         typename TS::initial_iterator_value_t const & v)
        {
          enum tchecker::state_status_t status = base_ts_t::initialize(s, t, v);
          if (status != tchecker::STATE_OK)
            return status;
          
          s.pl_pid(first_pure_local_process(s));          
          return tchecker::STATE_OK;
        }
        
        
        /*!
         \brief Next state computation
         \param s : state
         \param t : transition
         \param v : outgoing edge value
         \post s and t have been updated from v
         \return status of state s after update
         */
        virtual enum tchecker::state_status_t next(STATE & s,
                                                   typename TS::transition_t & t,
                                                   typename TS::outgoing_edges_iterator_value_t const & v)
        {
          enum tchecker::state_status_t status = base_ts_t::next(s, t, v);
          if (status != tchecker::STATE_OK)
            return status;
         
          s.pl_pid(first_pure_local_process(s));
          return tchecker::STATE_OK;
        }

        /*!
        \brief Check if a state has a synchronizable zone
        \param s : a state
        \return true if the zone in s is synchronizable, false otherwise
        */
        inline bool synchronizable_zone(STATE const & s) const
        {
          return base_ts_t::synchronizable_zone(s);
        }
      private:
        /*!
         \brief Source set selection
         \param s : a state
         \param v : a vedge
         \return true if v is in the source set of state s, false otherwise
         \note called by base_ts_t::next
        */
        virtual bool in_source_set(STATE const & s, typename TS::outgoing_edges_iterator_value_t const & v)
        {
          if (s.pl_pid() == tchecker::por::pure_local::no_pure_local)
            return true;
          // Check if 1st process in v is the pure local process in state s
          auto it = v.begin(), end = v.end();
          if (it == end) // empty vedge
            throw std::runtime_error("Empty vedge");
          return (*it)->pid() == s.pl_pid();
        }

        /*!
        \brief Compute first process ID which is pure local in state s
        \param s : a state
        \return ID of first process that is pure local in s if any,
        tchecker::por::pure_local::no_pure_local otherwise
        */
        tchecker::process_id_t first_pure_local_process(STATE const & s) const
        {
          for (auto const * loc : s.vloc())
            if (pure_local_map.is_pure_local(loc->id()))
              return loc->pid();
          return tchecker::por::pure_local::no_pure_local;
        }

        tchecker::pure_local_map_t pure_local_map; /*!< Map : location ID -> pure local status */
      };
    
 
 
      
      /*!
       \brief Covering check
       \param s1 : state
       \param s2 : state
       \return true if s1 can be covered by s2, false othewise
       */
      bool cover_leq(tchecker::por::pure_local::state_t const & s1, tchecker::por::pure_local::state_t const & s2);
      
      
    } // end of namespace pure_local
    
  } // end of namespace por
  
} // end of namespace tchecker

#endif // TCHECKER_POR_PURE_LOCAL_TS_HH
