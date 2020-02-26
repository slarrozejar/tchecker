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
#include "tchecker/dbm/offset_dbm.hh"
#include "tchecker/flat_system/vedge.hh"
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
      
      
#define PARTIAL_SYNC_ALLOWED
      
      
      /*!
       \brief Transition system with partial-order reduction for client/server systems
       \tparam TS : type of underlying transition system, should implement tchecker::ts::ts_t
       \tparam STATE : type of states, should inherit from TS::state_t, and from tchecker::por::state_t
       \tparam SOURCE_SET : source set, should provide membership operator
       bool operator() (STATE const &, tchecker::vedge_iterator_t const &)
       \note ts_t<TS> implements partial-order reduction on top of TS, using client/server source sets
       */
      template <class TS, class STATE>
      class ts_t final : public tchecker::por::ts_t<TS, STATE> {
        using base_ts_t = tchecker::por::ts_t<TS, STATE>;
      public:
        /*!
         \brief Constructor
         \param model : a model
         \param server : name of server process
         \param args : arguments to a constructor of tchecker::por::ts_t
         \throw std::invalid_argument : if model is not client/server
         \note TS should have a constructor TS(MODEL &)
         */
        template <class MODEL, class ... ARGS>
        ts_t(MODEL & model, std::string const & server, ARGS && ... args)
        : base_ts_t(model, args...),
        _location_next_syncs(tchecker::location_next_syncs(model.system())),
        _refcount(model.flattened_offset_clock_variables().refcount()),
        _offset_dim(model.flattened_offset_clock_variables().flattened_size())
        {
          try {
            _server_pid = model.system().processes().key(server);
          }
          catch (...) {
            throw std::invalid_argument("Unknown server process");
          }
#ifdef PARTIAL_SYNC_ALLOWED
          _group_id = tchecker::client_server_groups(model.system(), _server_pid);
          assert(_refcount == model.system().processes_count());
#else
          if (! tchecker::client_server(model.system(), _server_pid))
            throw std::invalid_argument("System is not client/server");
#endif // PARTIAL_SYNC_ALLOWED
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
          
#ifdef PARTIAL_SYNC_ALLOWED
          // Synchronize reference clocks from the same group
          if (synchronize_groups(s) != tchecker::STATE_OK)
            return tchecker::STATE_EMPTY_ZONE;
#endif // PARTIAL_SYNC_ALLOWED
          
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
          
#ifdef PARTIAL_SYNC_ALLOWED
          // Synchronize reference clocks from the same group
          if (synchronize_groups(s) != tchecker::STATE_OK)
            return tchecker::STATE_EMPTY_ZONE;
#endif // PARTIAL_SYNC_ALLOWED
          
          std::set<tchecker::process_id_t> vedge_pids = tchecker::vedge_pids(v);
#ifdef PARTIAL_SYNC_ALLOWED
          tchecker::process_id_t rank = tchecker::por::cs::communication;
          for (tchecker::process_id_t pid : vedge_pids)
            if (pid == _server_pid) {
              rank = tchecker::por::cs::communication;
              break;
            }
            else
              rank = _group_id[pid];
          s.rank(rank);
#else
          if (vedge_pids.size() == 2)
            s.rank(tchecker::por::cs::communication);
          else
            s.rank(* vedge_pids.begin());
#endif // PARTIAL_SYNC_ALLOWED
          
          if (! synchronizable(s))
            return tchecker::STATE_POR_DISABLED;
          
          return tchecker::STATE_OK;
        }
      private:
        /*!
         \brief Source set selection
         \param s : a state
         \param v : a vedge
         \return true if v is in the source set of state s, false otherwise
         \note called by base_ts_t::next
         \note vedge is in source_set(s) if either a communication has just happened (s.rank() == tchecker::por::cs::communication)
         or if vedge is a local or communication action of current active process (s.rank() belongs to tchecker::por::cs::vedge_pids(vedge))
         */
        virtual bool in_source_set(STATE const & s, typename TS::outgoing_edges_iterator_value_t const & v)
        {
#ifdef PARTIAL_SYNC_ALLOWED
          if (s.rank() == tchecker::por::cs::communication)
            return true;
          for (tchecker::process_id_t pid : tchecker::vedge_pids(v))
            if (pid != _server_pid)
              return (_group_id[pid] == s.rank());
          return false;
#else
          return (s.rank() == tchecker::por::cs::communication) || (tchecker::vedge_pids(v).count(s.rank()) >= 1);
#endif // PARTIAL_SYNC_ALLOWED
        }
        
        /*!
         \brief Synchronise reference clocks that belong to the same group
         \param s : a state
         \post the reference clocks that belong to the same group have been synchronised in the zone of s
         \return tchecker::STATE_OK if the resulting zone is not empty,  tchecker::STATE_EMPTY_ZONE otherwise
         */
        enum tchecker::state_status_t synchronize_groups(STATE & s) const
        {
          tchecker::dbm::db_t * offset_dbm = s.offset_zone_ptr()->dbm();
          for (tchecker::clock_id_t r = 0; r < _refcount; ++r) {
            if (r == _group_id[r])
              continue;
            
            auto status = tchecker::offset_dbm::constrain(offset_dbm, _offset_dim, r, _group_id[r],
                                                          tchecker::dbm::LE, 0);
            if (status == tchecker::dbm::EMPTY)
              return tchecker::STATE_EMPTY_ZONE;
            
            status = tchecker::offset_dbm::constrain(offset_dbm, _offset_dim, _group_id[r], r,
                                                     tchecker::dbm::LE, 0);
            if (status == tchecker::dbm::EMPTY)
              return tchecker::STATE_EMPTY_ZONE;
          }
          
          return tchecker::STATE_OK;
        }
        
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
#ifdef PARTIAL_SYNC_ALLOWED
        std::vector<tchecker::process_id_t> _group_id;         /*!< Map : process ID -> group ID */
        tchecker::clock_id_t _refcount;                        /*!< Number of reference clocks */
        tchecker::clock_id_t _offset_dim;                      /*!< Dimension of offset DBMs */
#endif // PARTIAL_SYNC_ALLOWED
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
