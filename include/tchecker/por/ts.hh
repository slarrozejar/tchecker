/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_TS_HH
#define TCHECKER_POR_TS_HH

#include <cassert>
#include <tuple>
#include <type_traits>

#include <boost/dynamic_bitset.hpp>

#include "tchecker/basictypes.hh"
#include "tchecker/flat_system/vedge.hh"
#include "tchecker/por/state.hh"
#include "tchecker/ts/ts.hh"

/*!
 \file ts.hh
 \brief Transition system with POR
 */

namespace tchecker {
  
  namespace por {
    
    namespace details {
      
      /*!
       \class global_edge_map_t
       \brief Tells for each location in a system whether it has an outgoing global edge
       \note THIS SHOULD BE STORED IN A MODEL IN ORDER TO BE SAFE WRT UPDATABLE MODELS
       */
      class global_edge_map_t {
      public:
        template <class SYSTEM>
        global_edge_map_t(SYSTEM const & system)
        : _map(system.locations_count(), 0)
        {
          auto edges = system.edges();
          for (auto const * edge : edges)
            if (! system.asynchronous(edge->pid(), edge->event_id()))
              _map[edge->src()->id()] = 1;
        }
        
        /*!
         \brief Copy constructor
         */
        global_edge_map_t(tchecker::por::details::global_edge_map_t const &) = default;
        
        /*!
         \brief Move constructor
         */
        global_edge_map_t(tchecker::por::details::global_edge_map_t &&) = default;
        
        /*!
         \brief Destructor
         */
        ~global_edge_map_t() = default;
        
        /*!
         \brief Assignment operator
         */
        tchecker::por::details::global_edge_map_t &
        operator=(tchecker::por::details::global_edge_map_t const &) = delete;
        
        /*!
         \brief Move-assignment operator
         */
        tchecker::por::details::global_edge_map_t &
        operator=(tchecker::por::details::global_edge_map_t &&) = delete;
        
        /*!
         \brief Accessor
         \param s : state
         \tparam STATE : type of state s, should inherit from tchecker::por::state_t
         \pre s.active_pid() != tchecker::por::all_processes_active (checked by assertion)
         \return first process with identifier greater than or equal to s.active_pid() that has no global action from s if any,
         tchecker::por::all_processes_active otherwise
         */
        template <class STATE>
        tchecker::process_id_t turn(STATE const & s) const
        {
          assert( ! s.all_active() );

          tchecker::process_id_t active_pid = s.active_pid();
          auto const & vloc = s.vloc();
          for ( ; active_pid < vloc.size(); ++active_pid)
            if (! _map[vloc[active_pid]->id()])
              return active_pid;
          return tchecker::por::all_processes_active;
        }
      private:
        boost::dynamic_bitset<> _map;            /*!< Map : location ID -> has global outgoing edge */
      };
      
      
      
      
      /*!
       \class gl_outgoing_iterator_t
       \brief Outgoing iterator for global-local transition system with POR
       \tparam OUTGOING_ITERATOR : type of outgoing iterator for underlying transition system
       */
      template <class OUTGOING_ITERATOR>
      class gl_outgoing_iterator_t {
      public:
        /*!
         \brief Constructor
         \param outgoing_it : outgoing iterator in underlying transition system
         \param active_pid : identifier of active process
         \param processes_count : number of processes
         \note this iterator filters outgoing_it by only allowing local transitions from process with identifier active_pid if active_pid !=
         tchecker::por::all_processes_active,
         and by allowing all transitions (local and global) when active_pid is equal to tchecker::por::all_processes_active
         */
        gl_outgoing_iterator_t(OUTGOING_ITERATOR const & outgoing_it,
                               tchecker::process_id_t active_pid,
                               tchecker::process_id_t processes_count)
        : _outgoing_it(outgoing_it),
        _active_pid(active_pid),
        _vedge_active_pid(0),
        _processes_count(processes_count)
        {
          skip();
        }
        
        /*!
         \brief Copy constructor
         */
        gl_outgoing_iterator_t(tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> const &)
        = default;
        
        /*!
         \brief Move constructor
         */
        gl_outgoing_iterator_t(tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> &&) = default;
        
        /*!
         \brief Destructor
         */
        ~gl_outgoing_iterator_t() = default;
        
        /*!
         \brief Assignment operator
         */
        tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> &
        operator= (tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> const &) = default;
        
        /*!
         \brief Move-assignment operator
         */
        tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> &
        operator= (tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> &&) = default;
        
        /*!
         \brief Equality predicate
         \param it : iterator
         \return true iff this and it are equal
         */
        inline bool
        operator== (tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> const & it) const
        {
          return ((_outgoing_it == it._outgoing_it) &&
                  (_active_pid == it._active_pid) &&
                  (_vedge_active_pid == it._vedge_active_pid) &&
                  (_processes_count == it._processes_count));
        }
        
        /*!
         \brief Disequality predicate
         \param it : iterator
         \return true iff this and it are different
         */
        inline bool
        operator!= (tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> const & it) const
        {
          return ! (*this == it);
        }
        
        /*!
         \brief Accessor
         \return true if this is past-the-end, false otherwise
         */
        inline bool at_end() const
        {
          return _outgoing_it.at_end();
        }
        
        /*!
         \brief Increment operator
         \pre this is not past-the-end
         \post this points to the next outgoing transition if any, or past-the-end otherwise
         \return this after increment
         */
        inline tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> & operator++ ()
        {
          assert(! at_end());
          ++ _outgoing_it;
          skip();
          return *this;
        }
        
        /*!
         \brief Value type (edge active pid, edge)
         \note The edge active pid is set to tchecker::por::all_processes_active on global edges, otherwise it is set to  the active process
         id for local transitions
         */
        using value_type_t
        = std::tuple<tchecker::process_id_t, typename std::iterator_traits<OUTGOING_ITERATOR>::value_type>;
        
        /*!
         \brief Dereference operator
         \pre this is not past-the-end
         \return data pointed by this
         */
        inline value_type_t operator* ()
        {
          assert(! at_end());
          return std::make_tuple(_vedge_active_pid, *_outgoing_it);
        }
      private:
        /*!
         \brief Skip transitions which are not allowed
         \post _outgoing_it points to the next allowed transition: either a transition from process _active_pid when
         _active_pid != tchecker::por::all_processes_active, ot from any process when _active_pid is equal to tchecker::por::all_processes_active.
         _vedge_active_pid has been set to the process identifier in vedge if local, and it has been set to tchecker::por::all_processes_active if
         the vedge is global
         \throw std::runtime_error : if the model is not local/gobal
         */
        void skip()
        {
          while (! _outgoing_it.at_end()) {
            _vedge_active_pid = vedge_active_pid(*_outgoing_it);
            
            if ((_active_pid == tchecker::por::all_processes_active) ||
                (_active_pid == _vedge_active_pid))
              break;
              
            ++ _outgoing_it;
          }
        }
      
        
        /*!
         \brief Compute size and active process of a vedge
         \param vedge : a vedge
         \pre vedge is either local (involves 1 process) or global (involves all processes)
         \return the identifier of active process in vedge if vedge is local, otherwise returns tchecker::por::all_processes_active
         if vedge is global
         \throw std::runtime_error : if vedge is neither local nor global
         */
        template <class VEDGE>
        tchecker::process_id_t vedge_active_pid(VEDGE const & vedge) const
        {
          // Compute vedge size and edge rank
          tchecker::process_id_t vedge_active_pid = tchecker::por::all_processes_active;
          std::size_t vedge_size = 0;
          for (auto it = vedge.begin(); it != vedge.end(); ++it) {
            if (vedge_active_pid == tchecker::por::all_processes_active)
              vedge_active_pid = (*it)->pid();
            ++vedge_size;
          }

          // Check global/local system
          if ((vedge_size != 1) && (vedge_size != _processes_count))
            throw std::runtime_error("System is not local/global");
          
          return (vedge_size == 1 ? vedge_active_pid : tchecker::por::all_processes_active);
        }
                
        OUTGOING_ITERATOR _outgoing_it;           /*!< Underlying iterator */
        tchecker::process_id_t _active_pid;       /*!< Identifier of active process */
        tchecker::process_id_t _vedge_active_pid; /*!< identifier of active process on edge */
        tchecker::process_id_t _processes_count;  /*!< Number of processes */
      };
      
    } // end of namespace details
    
  } // end of namespace por
  
} // end of namespace tchecker




/*!
 \brief Iterator traits for tchecker::por::details::gl_outgoing_iterator_t
 */
template <class OUTGOING_ITERATOR>
struct std::iterator_traits<tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR>> {
  using difference_type = nullptr_t;
  
  using value_type
  = typename tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR>::value_type_t;
  
  using pointer = value_type *;
  
  using reference = value_type &;
  
  using iterator_category = std::forward_iterator_tag;
};
    

    
  
namespace tchecker {
    
  namespace por {
  
    /*!
     \class gl_ts_t
     \brief Transition system with global-local POR
     \tparam TS : type of underlying transition system, should implement tchecker::ts::ts_t
     \tparam STATE : type of states, should inherit from TS::state_t, and from tchecker::por::state_t
     \note gl_ts_t<TS> implements a POR on top of TS. TS is supposed to be global/local: every transition involves either
     one process (local) or all the processes (global)
     */
    template <class TS, class STATE>
    class gl_ts_t
    : public tchecker::ts::ts_t
    <
    STATE,
    typename TS::transition_t,
    typename TS::initial_iterator_t,
    tchecker::por::details::gl_outgoing_iterator_t<typename TS::outgoing_edges_iterator_t>,
    typename TS::initial_iterator_value_t,
    typename std::iterator_traits<tchecker::por::details::gl_outgoing_iterator_t<typename TS::outgoing_edges_iterator_t>>::value_type
    >
    {
      
      static_assert(std::is_base_of<tchecker::por::state_t, STATE>::value,
                    "STATE should derive from tchecker::por::state_t");
      
      static_assert(std::is_base_of<typename TS::state_t, STATE>::value,
                    "STATE should derive from TS::state_t");
      
      using por_ts_t
      = tchecker::ts::ts_t
      <
      STATE,
      typename TS::transition_t,
      typename TS::initial_iterator_t,
      tchecker::por::details::gl_outgoing_iterator_t<typename TS::outgoing_edges_iterator_t>,
      typename TS::initial_iterator_value_t,
      typename std::iterator_traits<tchecker::por::details::gl_outgoing_iterator_t<typename TS::outgoing_edges_iterator_t>>::value_type
      >;
      
    public:
      /*!
       \brief Constructor
       \param args : arguments to a constructor of TS
       */
      template <class ... ARGS>
      gl_ts_t(ARGS && ... args)
      : _ts(args...),
      _processes_count(_ts.model().system().processes_count()),
      _global_edge_map(_ts.model().system())
      {}
      
      /*!
       \brief Copy constructor
       */
      gl_ts_t(tchecker::por::gl_ts_t<TS, STATE> const &) = default;
      
      /*!
       \brief Move constructor
       */
      gl_ts_t(tchecker::por::gl_ts_t<TS, STATE> &&) = default;
      
      /*!
       \brief Destructor
       */
      virtual ~gl_ts_t() = default;
      
      /*!
       \brief Assignment operator
       */
      tchecker::por::gl_ts_t<TS, STATE> & operator= (tchecker::por::gl_ts_t<TS, STATE> const &) = default;
      
      /*!
       \brief Move-assignment oeprator
       */
      tchecker::por::gl_ts_t<TS, STATE> & operator= (tchecker::por::gl_ts_t<TS, STATE> &&) = default;
      
      /*!
       \brief Accessor
       \return Initial state valuations
       */
      inline virtual tchecker::range_t<typename TS::initial_iterator_t> initial()
      {
        return _ts.initial();
      }
      
      /*!
       \brief Initialize state
       \param s : state
       \param t : transition
       \param v : initial state valuation
       \post state s and transtion t have been initialized from v
       \return status of state s after initialization
       \note t represents an initial transition to s
       */
      inline virtual enum tchecker::state_status_t initialize(STATE & s,
                                                              typename TS::transition_t & t,
                                                              typename TS::initial_iterator_value_t const & v)
      {
        enum tchecker::state_status_t status = _ts.initialize(s, t, v);
        if (status != tchecker::STATE_OK)
          return status;
        
        // Compute active process
        s.active_pid(0);
        s.active_pid(_global_edge_map.turn(s));
        
        return tchecker::STATE_OK;
      }
      
      /*!
       \brief Type of outgoing edges iterator
       */
      using outgoing_edges_iterator_t = typename por_ts_t::outgoing_edges_iterator_t;
      
      /*!
       \brief Type of outgoing edges itreator value
       */
      using outgoing_edges_iterator_value_t = typename por_ts_t::outgoing_edges_iterator_value_t;
      
      /*!
       \brief Accessor
       \param s : state
       \return outgoing edges from state s
       */
      virtual tchecker::range_t<outgoing_edges_iterator_t> outgoing_edges(STATE const & s)
      {
        tchecker::range_t<typename TS::outgoing_edges_iterator_t> ts_outgoing_edges = _ts.outgoing_edges(s);
        return tchecker::make_range
        (outgoing_edges_iterator_t(ts_outgoing_edges.begin(), s.active_pid(), _processes_count),
         outgoing_edges_iterator_t(ts_outgoing_edges.end(), s.active_pid(), _processes_count));
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
                                                 outgoing_edges_iterator_value_t const & v)
      {
        tchecker::process_id_t vedge_active_pid = std::get<0>(v);
        typename TS::outgoing_edges_iterator_value_t const & vedge = std::get<1>(v);

        assert(s.all_active() || (vedge_active_pid == s.active_pid()));
        
        enum tchecker::state_status_t status = _ts.next(s, t, vedge);
        if (status != tchecker::STATE_OK)
          return status;
        
        if (s.active_pid() == tchecker::por::all_processes_active)
          s.active_pid(0);
        s.active_pid(_global_edge_map.turn(s));
        return tchecker::STATE_OK;
      }
    private:
      TS _ts;                                                     /*!< Underlying transition system */
      tchecker::process_id_t _processes_count;                    /*!< Number of processes */
      tchecker::por::details::global_edge_map_t _global_edge_map; /*!< Map : location ID -> has global outgoing edge */
    };
    
  } // end of namespace por
  
} // end of namespace tchecker

#endif // TCHECKER_POR_TS_HH
