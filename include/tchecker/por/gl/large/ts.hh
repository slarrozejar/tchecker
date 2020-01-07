/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_GL_LARGE_TS_HH
#define TCHECKER_POR_GL_LARGE_TS_HH

#include <cassert>
#include <limits>
#include <tuple>
#include <type_traits>

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

#include "tchecker/basictypes.hh"
#include "tchecker/flat_system/vedge.hh"
#include "tchecker/por/state.hh"
#include "tchecker/por/synchronizable.hh"
#include "tchecker/system/static_analysis.hh"
#include "tchecker/ts/ts.hh"

/*!
 \file ts.hh
 \brief Transition system with large POR for global/local systems
 */

namespace tchecker {
  
  namespace por {
    
    namespace gl {
      
      namespace large {
        
        namespace details {
          
          constexpr tchecker::process_id_t const global
          = std::numeric_limits<tchecker::process_id_t>::max();         /*!< Rank value for global transition */
          
          
          
          
          /*!
           \class outgoing_iterator_t
           \brief Outgoing iterator for global-local transition system with largePOR
           \tparam OUTGOING_ITERATOR : type of outgoing iterator for underlying transition system
           */
          template <class OUTGOING_ITERATOR>
          class outgoing_iterator_t {
          public:
            /*!
             \brief Constructor
             \param outgoing_it : outgoing iterator in underlying transition system
             \param active_pid : identifier of active process
             \note this iterator filters outgoing_it by only allowing local transitions from process with identifier >= active_pid, and all global
             transitions
             */
            outgoing_iterator_t(OUTGOING_ITERATOR const & outgoing_it, tchecker::process_id_t active_pid)
            : _outgoing_it(outgoing_it), _active_pid(active_pid), _vedge_pid(0)
            {
              skip();
            }
            
            /*!
             \brief Copy constructor
             */
            outgoing_iterator_t
            (tchecker::por::gl::large::details::outgoing_iterator_t<OUTGOING_ITERATOR> const &) = default;
            
            /*!
             \brief Move constructor
             */
            outgoing_iterator_t
            (tchecker::por::gl::large::details::outgoing_iterator_t<OUTGOING_ITERATOR> &&) = default;
            
            /*!
             \brief Destructor
             */
            ~outgoing_iterator_t() = default;
            
            /*!
             \brief Assignment operator
             */
            tchecker::por::gl::large::details::outgoing_iterator_t<OUTGOING_ITERATOR> &
            operator= (tchecker::por::gl::large::details::outgoing_iterator_t<OUTGOING_ITERATOR> const &)
            = default;
            
            /*!
             \brief Move-assignment operator
             */
            tchecker::por::gl::large::details::outgoing_iterator_t<OUTGOING_ITERATOR> &
            operator= (tchecker::por::gl::large::details::outgoing_iterator_t<OUTGOING_ITERATOR> &&) = default;
            
            /*!
             \brief Equality predicate
             \param it : iterator
             \return true iff this and it are equal
             */
            inline bool
            operator==
            (tchecker::por::gl::large::details::outgoing_iterator_t<OUTGOING_ITERATOR> const & it) const
            {
              return ((_outgoing_it == it._outgoing_it) &&
                      (_active_pid == it._active_pid) &&
                      (_vedge_pid == it._vedge_pid));
            }
            
            /*!
             \brief Disequality predicate
             \param it : iterator
             \return true iff this and it are different
             */
            inline bool
            operator!=
            (tchecker::por::gl::large::details::outgoing_iterator_t<OUTGOING_ITERATOR> const & it) const
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
            inline tchecker::por::gl::large::details::outgoing_iterator_t<OUTGOING_ITERATOR> & operator++ ()
            {
              assert(! at_end());
              ++ _outgoing_it;
              skip();
              return *this;
            }
            
            /*!
             \brief Value type: pair (vedge pid, vedge)
             \note The edge active pid is set to tchecker::por::gl::large::details::global on global edges,
             otherwise it is set to the active process id for local edges
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
              return std::make_tuple(_vedge_pid, *_outgoing_it);
            }
          private:
            /*!
             \brief Skip transitions which are not allowed
             \post _outgoing_it points to the next transitions which is either global, or from a process >= _active_pid, if any
             Points past-the-end otherwise. All transitions are allowed when _active_pid == tchecker::pro::gl__large::details::global
             */
            void skip()
            {
              while (! _outgoing_it.at_end()) {
                _vedge_pid = vedge_pid(*_outgoing_it);
                if ((_active_pid == tchecker::por::gl::large::details::global) ||
                    (_vedge_pid >= _active_pid))
                  break;
                ++ _outgoing_it;
              }
            }
            
            
            /*!
             \brief Compute  process involved in a vedge
             \param vedge : a vedge
             \pre system is global/local, i.e. vedge involves either one process (local) or all the processes (global)
             \return the identifier of active process in vedge if vedge is local,
             otherwise returns tchecker::por::gl::large::details::global if vedge is global
             \throw std::runtime_error : if vedge is empty
             */
            template <class VEDGE>
            tchecker::process_id_t vedge_pid(VEDGE const & vedge) const
            {
              auto it = vedge.begin(), end = vedge.end();
              
              if (it == end) // empty vedge
                throw std::runtime_error("Empty vedge");
              
              tchecker::process_id_t first_pid = (*it)->pid();
              
              ++it;
              if (it == end) // local vedge
                return first_pid;
              
              // global vedge
              return tchecker::por::gl::large::details::global;
            }
            
            OUTGOING_ITERATOR _outgoing_it;     /*!< Underlying outgoing iterator */
            tchecker::process_id_t _active_pid; /*!< Identifier of active process in source node */
            tchecker::process_id_t _vedge_pid;  /*!< Identifier of active process in current vedfe */
          };
          
        } // end of namespace details
        
      } // end of namespace large
      
    } // end of namespace gl
    
  } // end of namespace por
  
} // end of namespace tchecker


/*!
 \brief Iterator traits for tchecker::por::gl::large::details::outgoing_iterator_t
 */
template <class OUTGOING_ITERATOR>
struct std::iterator_traits<tchecker::por::gl::large::details::outgoing_iterator_t<OUTGOING_ITERATOR>> {
  using difference_type = nullptr_t;
  
  using value_type
  = typename tchecker::por::gl::large::details::outgoing_iterator_t<OUTGOING_ITERATOR>::value_type_t;
  
  using pointer = value_type *;
  
  using reference = value_type &;
  
  using iterator_category = std::forward_iterator_tag;
};




namespace tchecker {
  
  namespace por {
    
    namespace gl {
      
      namespace large {
        
        namespace details {
          
          /*!
           \brief Short name for TS instance
           */
          template <class TS, class STATE>
          using ts_instance_t = tchecker::ts::ts_t
          <
          STATE,
          typename TS::transition_t,
          typename TS::initial_iterator_t,
          tchecker::por::gl::large::details::outgoing_iterator_t<typename TS::outgoing_edges_iterator_t>,
          typename TS::initial_iterator_value_t,
          typename std::iterator_traits<tchecker::por::gl::large::details::outgoing_iterator_t<typename TS::outgoing_edges_iterator_t>>::value_type
          >;
          
        } // end of namespace details
        
        
        
        
        /*!
         \class ts_t
         \brief Transition system with large POR for global/local systems
         \tparam TS : type of underlying transition system, should implement tchecker::ts::ts_t
         \tparam STATE : type of states, should inherit from TS::state_t, and from tchecker::por::state_t
         \note ts_t<TS> implements a large round-robin POR on top of TS: each node has a rank. From a given node,
         all global transitions, and all local transitions from processes >= from the node's rank are allowed.
         TS is supposed to be global/local: every transition involves either one process (local) or all the processes (global).
         Transitions that lead to a node with rank r with a process < r that has no gobal edge are disabled (optimisation).
         \note This implementation is only correct for reachability of a global transition. Due to the optimisation above, some
         Mazukiewicz traces not leading to a global action, may be missing.
         */
        template <class TS, class STATE>
        class ts_t final : public tchecker::por::gl::large::details::ts_instance_t<TS, STATE> {
          
          static_assert(std::is_base_of<tchecker::por::state_t, STATE>::value,
                        "STATE should derive from tchecker::por::state_t");
          
          static_assert(std::is_base_of<typename TS::state_t, STATE>::value,
                        "STATE should derive from TS::state_t");
          
          using por_ts_t = tchecker::por::gl::large::details::ts_instance_t<TS, STATE>;
          
        public:
          /*!
           \brief Constructor
           \param args : arguments to a constructor of TS
           \throw std::invalid_argument : if TS is not global/local
           */
          template <class ... ARGS>
          ts_t(ARGS && ... args)
          : _ts(args...),
          _location_next_syncs(tchecker::location_next_syncs(_ts.model().system()))
          {
            if (! tchecker::global_local(_ts.model().system()))
              throw std::invalid_argument("System is not global/local");
          }
          
          /*!
           \brief Copy constructor
           */
          ts_t(tchecker::por::gl::large::ts_t<TS, STATE> const &) = default;
          
          /*!
           \brief Move constructor
           */
          ts_t(tchecker::por::gl::large::ts_t<TS, STATE> &&) = default;
          
          /*!
           \brief Destructor
           */
          virtual ~ts_t() = default;
          
          /*!
           \brief Assignment operator
           */
          tchecker::por::gl::large::ts_t<TS, STATE> &
          operator= (tchecker::por::gl::large::ts_t<TS, STATE> const &) = default;
          
          /*!
           \brief Move-assignment oeprator
           */
          tchecker::por::gl::large::ts_t<TS, STATE> &
          operator= (tchecker::por::gl::large::ts_t<TS, STATE> &&) = default;
          
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
          virtual enum tchecker::state_status_t initialize(STATE & s,
                                                           typename TS::transition_t & t,
                                                           typename TS::initial_iterator_value_t const & v)
          {
            enum tchecker::state_status_t status = _ts.initialize(s, t, v);
            if (status != tchecker::STATE_OK)
              return status;
            
            s.rank(0);
            
            if (! synchronizable(s))
              return tchecker::STATE_POR_REMOVED;
            
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
            return tchecker::make_range(outgoing_edges_iterator_t(ts_outgoing_edges.begin(), s.rank()),
                                        outgoing_edges_iterator_t(ts_outgoing_edges.end(), s.rank()));
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
            tchecker::process_id_t vedge_pid = std::get<0>(v);
            typename TS::outgoing_edges_iterator_value_t const & vedge = std::get<1>(v);
            
            enum tchecker::state_status_t status = _ts.next(s, t, vedge);
            if (status != tchecker::STATE_OK)
              return status;
            
            s.rank(vedge_pid);
            
            if (! synchronizable(s))
              return tchecker::STATE_POR_REMOVED;
            
            return tchecker::STATE_OK;
          }
        private:
          /*!
           \brief Checks can lead to a global action
           \param s : state
           \return true if all processes can reach a common global action from s, false otherwise
           */
          bool synchronizable(STATE const & s) const
          {
            tchecker::process_id_t rank = (s.rank() == tchecker::por::gl::large::details::global ? 0 : s.rank());
            return tchecker::por::synchronizable(s.vloc(), rank, _location_next_syncs);
          }
          
          TS _ts;                                                      /*!< Underlying transition system */
          tchecker::location_next_syncs_t const _location_next_syncs;  /*!< Next synchronisations */
        };
        
        
        
        
        /*!
         \brief Covering check
         \param s1 : state
         \param s2 : state
         \return true if s1 can be covered by s1, false othewise
         */
        bool cover_leq(tchecker::por::state_t const & s1, tchecker::por::state_t const & s2);
              
      } // end of namespace large
      
    } // end of namespace gl
    
  } // end of namespace por
  
} // end of namespace tchecker

#endif // TCHECKER_POR_GL_LARGE_TS_HH
