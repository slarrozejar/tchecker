/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_CS_TS_HH
#define TCHECKER_POR_CS_TS_HH

#include <limits>
#include <set>
#include <type_traits>

#include "tchecker/basictypes.hh"
#include "tchecker/por/state.hh"
#include "tchecker/por/synchronizable.hh"
#include "tchecker/por/ts.hh"
#include "tchecker/system/static_analysis.hh"

/*!
 \file ts.hh
 \brief Transition system with POR for client/server systems
 */

namespace tchecker {
  
  namespace por {
    
    namespace cs {
      
      /*! Rank value of client-server communications */
      constexpr tchecker::process_id_t const communication = std::numeric_limits<tchecker::process_id_t>::max();
      
      
      
      
      /*!
       \brief Compute  processes involved in a vedge
       \param vedge : a vedge
       \return the identifiers of the processes involved in vedge
       */
      template <class VEDGE>
      std::set<tchecker::process_id_t> vedge_pids(VEDGE const & vedge)
      {
        std::set<tchecker::process_id_t> pids;
        
        auto end = vedge.end();
        for (auto it = vedge.begin(); it != end; ++it)
          pids.insert((*it)->pid());
        
        return pids;
      }
      
      
      
      /*!
       \class source_set_t
       \brief Source set for client/server systems
       \tparam STATE : type of state, should derive from tchecker::por::state_t
       */
      template <class STATE>
      class source_set_t {
        
        static_assert(std::is_base_of<tchecker::por::state_t, STATE>::value,
                      "STATE should derive from tchecker::por::state_t");
        
      public:
        /*!
         \brief Membership predicate
         \tparam VEDGE : type of vedge
         \param s : state
         \param vedge : a vedge
         \return true if vedge shoud be included in the source set of s, false otherwise
         \note vedge is in source_set(s) if either a communication has just happened (s.rank() == tchecker::por::cs::communication)
         or if vedge is a local or communication action of current active process (s.rank() belongs to tchecker::por::cs::vedge_pids(vedge))
         */
        template <class VEDGE>
        bool operator() (STATE const & s, VEDGE const & vedge)
        {
          return (s.rank() == tchecker::por::cs::communication) ||
          (tchecker::por::cs::vedge_pids(vedge).count(s.rank()) >= 1);
        }
      };
      
      
      
      
      /*!
       \brief Transition system with partial-order reduction for client/server systems
       \tparam TS : type of underlying transition system, should implement tchecker::ts::ts_t
       \tparam STATE : type of states, should inherit from TS::state_t, and from tchecker::por::state_t
       \tparam SOURCE_SET : source set, should provide membership operator
       bool operator() (STATE const &, tchecker::vedge_iterator_t const &)
       \note ts_t<TS> implements partial-order reduction on top of TS, using client/server source sets
       */
      template <class TS, class STATE>
      class ts_t final : public tchecker::por::ts_t<TS, STATE, tchecker::por::cs::source_set_t<STATE>> {
        using base_ts_t = tchecker::por::ts_t<TS, STATE, tchecker::por::cs::source_set_t<STATE>>;
      public:
        /*!
         \brief Constructor
         \param model : a model
         \throw std::invalid_argument : if model is not client/server
         \note TS should have a constructor TS(MODEL &)
         */
        template <class MODEL>
        ts_t(MODEL & model)
        : base_ts_t(model),
        _location_next_syncs(tchecker::location_next_syncs(model.system()))
        {
          if (! tchecker::client_server(model.system(), _server_pid))
            throw std::invalid_argument("System is not client/server");
        }
        
        /*!
         \brief Copy constructor
         */
        ts_t(tchecker::por::cs::ts_t<TS, STATE> const &) = default;
        
        /*!
         \brief Copy constructor
         */
        ts_t(tchecker::por::cs::ts_t<TS, STATE> &&) = default;
        
        /*!
         \brief Destructor
         */
        ~ts_t() = default;
        
        /*!
         \brief Assignment operator
         */
        tchecker::por::cs::ts_t<TS, STATE> & operator= (tchecker::por::cs::ts_t<TS, STATE> const &) = default;
        
        /*!
         \brief Move-assignment operator
         */
        tchecker::por::cs::ts_t<TS, STATE> & operator= (tchecker::por::cs::ts_t<TS, STATE> &&) = default;
        

        /*!
         \brief Initialize state
         \param s : state
         \param t : transition
         \param v : initial state valuation
         \post state s and transtion t have been initialized from v
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
          
          s.rank(tchecker::por::cs::communication);
          
          if (! synchronizable(s))
            return tchecker::STATE_POR_DISABLED;
          
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
          
          std::set<tchecker::process_id_t> vedge_pids = tchecker::por::cs::vedge_pids(v);
          if (vedge_pids.size() == 2)
            s.rank(tchecker::por::cs::communication);
          else
            s.rank(* vedge_pids.begin());
          
          if (! synchronizable(s))
            return tchecker::STATE_POR_DISABLED;
          
          return tchecker::STATE_OK;
        }
      private:
        /*!
         \brief Checks if a state can reach a communication
         \param s : state
         \return true if a communication is reachable, false otherwise
         \note state with rank == tchecker::por::cs::communication can trivially reach a communication. Other states, where only
         process s.rank() is allowed to do local actions, can reach a communication action if there is a communication that is
         feasible by the server process and reachable (through local actions) for process s.rank()
         */
        bool synchronizable(STATE const & s) const
        {
          return (s.rank() == tchecker::por::cs::communication) ||
          tchecker::por::synchronizable_server(s.vloc(), s.rank(), _server_pid, _location_next_syncs);
        }
        
        tchecker::location_next_syncs_t _location_next_syncs;  /*!< Next synchronisations */
        tchecker::process_id_t _server_pid;                    /*!< Identifier of server process */
      };
    
 
 
      
      /*!
       \brief Covering check
       \param s1 : state
       \param s2 : state
       \return true if s1 can be covered by s1, false othewise
       */
      bool cover_leq(tchecker::por::state_t const & s1, tchecker::por::state_t const & s2);
      
      
    } // end of namespace cs
    
  } // end of namespace por
  
} // end of namespace tchecker

#endif // TCHECKER_POR_CS_TS_HH
