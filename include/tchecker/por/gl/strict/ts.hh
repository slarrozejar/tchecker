/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_GL_STRICT_TS_HH
#define TCHECKER_POR_GL_STRICT_TS_HH

#include <cassert>
#include <limits>
#include <tuple>
#include <type_traits>

#include <boost/dynamic_bitset.hpp>

#include "tchecker/basictypes.hh"
#include "tchecker/flat_system/vedge.hh"
#include "tchecker/por/state.hh"
#include "tchecker/system/static_analysis.hh"
#include "tchecker/ts/ts.hh"

/*!
 \file ts.hh
 \brief Transition system with strict POR for global/local systems
 */

namespace tchecker {
  
  namespace por {
    
    namespace gl {
      
      namespace strict {
        
        namespace details {
          
        
          constexpr tchecker::process_id_t const all_processes_active
          = std::numeric_limits<tchecker::process_id_t>::max();         /*!< Rank value for all processes active */
          
          
          
          
          /*!
           \class outgoing_iterator_t
           \brief Outgoing iterator for global-local transition system with strict POR
           \tparam OUTGOING_ITERATOR : type of outgoing iterator for underlying transition system
           */
          template <class OUTGOING_ITERATOR>
          class outgoing_iterator_t {
          public:
            /*!
             \brief Constructor
             \param outgoing_it : outgoing iterator in underlying transition system
             \param active_pid : identifier of active process
             \note this iterator filters outgoing_it by only allowing local transitions from process with identifier active_pid if active_pid !=
             tchecker::por::gl::strict::details::all_processes_active,
             and by allowing all transitions (local and global) when active_pid is equal to tchecker::por::gl::strict::details::all_processes_active
             */
            outgoing_iterator_t(OUTGOING_ITERATOR const & outgoing_it, tchecker::process_id_t active_pid)
            : _outgoing_it(outgoing_it), _active_pid(active_pid)
            {
              skip();
            }
            
            /*!
             \brief Copy constructor
             */
            outgoing_iterator_t
            (tchecker::por::gl::strict::details::outgoing_iterator_t<OUTGOING_ITERATOR> const &) = default;
            
            /*!
             \brief Move constructor
             */
            outgoing_iterator_t
            (tchecker::por::gl::strict::details::outgoing_iterator_t<OUTGOING_ITERATOR> &&) = default;
            
            /*!
             \brief Destructor
             */
            ~outgoing_iterator_t() = default;
            
            /*!
             \brief Assignment operator
             */
            tchecker::por::gl::strict::details::outgoing_iterator_t<OUTGOING_ITERATOR> &
            operator= (tchecker::por::gl::strict::details::outgoing_iterator_t<OUTGOING_ITERATOR> const &)
            = default;
            
            /*!
             \brief Move-assignment operator
             */
            tchecker::por::gl::strict::details::outgoing_iterator_t<OUTGOING_ITERATOR> &
            operator= (tchecker::por::gl::strict::details::outgoing_iterator_t<OUTGOING_ITERATOR> &&) = default;
            
            /*!
             \brief Equality predicate
             \param it : iterator
             \return true iff this and it are equal
             */
            inline bool
            operator==
            (tchecker::por::gl::strict::details::outgoing_iterator_t<OUTGOING_ITERATOR> const & it) const
            {
              return ((_outgoing_it == it._outgoing_it) &&
                      (_active_pid == it._active_pid));
            }
            
            /*!
             \brief Disequality predicate
             \param it : iterator
             \return true iff this and it are different
             */
            inline bool
            operator!=
            (tchecker::por::gl::strict::details::outgoing_iterator_t<OUTGOING_ITERATOR> const & it) const
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
            inline tchecker::por::gl::strict::details::outgoing_iterator_t<OUTGOING_ITERATOR> & operator++ ()
            {
              assert(! at_end());
              ++ _outgoing_it;
              skip();
              return *this;
            }
            
            /*!
             \brief Value type (vedge)
             */
            using value_type_t = typename std::iterator_traits<OUTGOING_ITERATOR>::value_type;
            
            /*!
             \brief Dereference operator
             \pre this is not past-the-end
             \return data pointed by this
             */
            inline value_type_t operator* ()
            {
              assert(! at_end());
              return *_outgoing_it;
            }
          private:
            /*!
             \brief Skip transitions which are not allowed
             \post _outgoing_it points to the next allowed transition: either from process _active_pid when
             _active_pid != tchecker::por::gl::strict::details::all_processes_active,
             or from any process when _active_pid is equal to tchecker::por::gl::strict::details::all_processes_active.
             */
            void skip()
            {
              if (_active_pid == tchecker::por::gl::strict::details::all_processes_active)
                return;
              
              while (! _outgoing_it.at_end()) {
                if (vedge_pid(*_outgoing_it) == _active_pid)
                  break;
                ++ _outgoing_it;
              }
            }
            
            
            /*!
             \brief Compute  process invovled in a vedge
             \param vedge : a vedge
             \pre system is global/local, i.e. vedge involves either one process (local) or all the processes (global)
             \return the identifier of active process in vedge if vedge is local,
             otherwise returns tchecker::por::gl::strict::details::all_processes_active if vedge is global
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
              return tchecker::por::gl::strict::details::all_processes_active;
            }
            
            OUTGOING_ITERATOR _outgoing_it;           /*!< Underlying outgoing iterator */
            tchecker::process_id_t _active_pid;       /*!< Identifier of active process */
          };
          
        } // end of namespace details
        
      } // end of namespace strict
      
    } // end of namespace gl
    
  } // end of namespace por
  
} // end of namespace tchecker


/*!
 \brief Iterator traits for tchecker::por::details::gl_outgoing_iterator_t
 */
template <class OUTGOING_ITERATOR>
struct std::iterator_traits<tchecker::por::gl::strict::details::outgoing_iterator_t<OUTGOING_ITERATOR>> {
  using difference_type = nullptr_t;
  
  using value_type
  = typename tchecker::por::gl::strict::details::outgoing_iterator_t<OUTGOING_ITERATOR>::value_type_t;
  
  using pointer = value_type *;
  
  using reference = value_type &;
  
  using iterator_category = std::forward_iterator_tag;
};




namespace tchecker {
  
  namespace por {
    
    namespace gl {
      
      namespace strict {
        
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
          tchecker::por::gl::strict::details::outgoing_iterator_t<typename TS::outgoing_edges_iterator_t>,
          typename TS::initial_iterator_value_t,
          typename std::iterator_traits<tchecker::por::gl::strict::details::outgoing_iterator_t<typename TS::outgoing_edges_iterator_t>>::value_type
          >;
          
        } // end of namespace details
        
        
        
        
        /*!
         \class ts_t
         \brief Transition system with strict POR for global/local systems
         \tparam TS : type of underlying transition system, should implement tchecker::ts::ts_t
         \tparam STATE : type of states, should inherit from TS::state_t, and from tchecker::por::state_t
         \note ts_t<TS> implements a strict round-robin POR on top of TS: either one process can move, or all processes can move.
         TS is supposed to be global/local: every transition involves either one process (local) or all the processes (global)
         */
        template <class TS, class STATE>
        class ts_t final : public tchecker::por::gl::strict::details::ts_instance_t<TS, STATE> {
          
          static_assert(std::is_base_of<tchecker::por::state_t, STATE>::value,
                        "STATE should derive from tchecker::por::state_t");
          
          static_assert(std::is_base_of<typename TS::state_t, STATE>::value,
                        "STATE should derive from TS::state_t");
          
          using por_ts_t = tchecker::por::gl::strict::details::ts_instance_t<TS, STATE>;
          
        public:
          /*!
           \brief Constructor
           \param args : arguments to a constructor of TS
           \throw std::invalid_argument : if TS is not global/local
           */
          template <class ... ARGS>
          ts_t(ARGS && ... args)
          : _ts(args...),
          _location_sync_flag(tchecker::location_synchronisation_flags(_ts.model().system()))
          {
            if (! tchecker::global_local(_ts.model().system()))
              throw std::invalid_argument("System is not global/local");
          }
          
          /*!
           \brief Copy constructor
           */
          ts_t(tchecker::por::gl::strict::ts_t<TS, STATE> const &) = default;
          
          /*!
           \brief Move constructor
           */
          ts_t(tchecker::por::gl::strict::ts_t<TS, STATE> &&) = default;
          
          /*!
           \brief Destructor
           */
          virtual ~ts_t() = default;
          
          /*!
           \brief Assignment operator
           */
          tchecker::por::gl::strict::ts_t<TS, STATE> &
          operator= (tchecker::por::gl::strict::ts_t<TS, STATE> const &) = default;
          
          /*!
           \brief Move-assignment oeprator
           */
          tchecker::por::gl::strict::ts_t<TS, STATE> &
          operator= (tchecker::por::gl::strict::ts_t<TS, STATE> &&) = default;
          
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
            set_active_pid(s);
            
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
            enum tchecker::state_status_t status = _ts.next(s, t, v);
            if (status != tchecker::STATE_OK)
              return status;
            
            set_active_pid(s);
            return tchecker::STATE_OK;
          }
        private:
          /*!
           \brief Set the active pid in state
           \param s : state
           \post s.rank() == tchecker::por::gl::strict::details::all_processes_active if all location in s have an outgoing
           synchronized event,
           otherwise, s.rank() is the PID of the active process
           */
          void set_active_pid(STATE & s) const
          {
            tchecker::process_id_t active_pid = s.rank();
            if (active_pid == tchecker::por::gl::strict::details::all_processes_active)
              active_pid = 0;
            
            auto const & vloc = s.vloc();
            for ( ; active_pid < vloc.size(); ++active_pid)
              if (! _location_sync_flag.has_synchronized_event(vloc[active_pid]->id()))
                break;

            if (active_pid < vloc.size())
              s.rank(active_pid);
            else
              s.rank(tchecker::por::gl::strict::details::all_processes_active);
          }
          
          TS _ts;                                                   /*!< Underlying transition system */
          tchecker::location_sync_flag_t const _location_sync_flag; /*!< Synchronized event flag for locations */
        };
        
        
        
        
        /*!
         \brief Covering check
         \param s1 : state
         \param s2 : state
         \return true if s1 can be covered by s1, false othewise
         */
        bool cover_leq(tchecker::por::state_t const & s1, tchecker::por::state_t const & s2);
        
      } // end of namespace strict
      
    } // end of namespace gl
        
  } // end of namespace por
  
} // end of namespace tchecker
    
#endif // TCHECKER_POR_GL_STRICT_TS_HH
