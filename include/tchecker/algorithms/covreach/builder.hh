/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ALGORITHMS_COVREACH_BUILDER_HH
#define TCHECKER_ALGORITHMS_COVREACH_BUILDER_HH

#include <tuple>
#include <vector>

#include "tchecker/basictypes.hh"
#include "tchecker/ts/builder.hh"

/*!
 \file builder.hh
 \brief Transition system builder for covering reachability algorithm
 */

namespace tchecker {
  
  namespace covreach {
    
    /*!
    \class states_builder_t
    \brief Computation of initial and next states
    \tparam STATE_PTR : type of pointer to states
    */
    template <class STATE_PTR>
    class states_builder_t {
    public:
      /*!
      \brief Destructor
      */
      virtual ~states_builder_t() = default;

      /*!
      \brief Computes initial states
      \param v : states container
      \post initial states have been pushed back in v
      */
      virtual void initial(std::vector<STATE_PTR> & v) = 0;

      /*!
      \brief Computes next states
      \param s : a state
      \param v : states container
      \post successor states of s have been pushed back in v
      */
      virtual void next(STATE_PTR & s, std::vector<STATE_PTR> & v) = 0;
    };


    /*!
     \class full_states_builder_t
     \brief States builder for covering reachability algorithm that returns all
     initial states, and all successor states
     \tparam TS : type of transition system (see tchecker::ts::builder_ok_t)
     \tparam ALLOCATOR : type of allocator (see tchecker::ts::builder_ok_t)
     */
    template <class TS, class ALLOCATOR>
    class full_states_builder_t final
    : private tchecker::ts::builder_ok_t<TS, ALLOCATOR>,
    public tchecker::covreach::states_builder_t<typename tchecker::ts::builder_ok_t<TS, ALLOCATOR>::state_ptr_t> {
    public:
      /*!
       \brief Type of pointers to state
       */
      using state_ptr_t = typename tchecker::ts::builder_ok_t<TS, ALLOCATOR>::state_ptr_t;
      
      /*!
       \brief Type of pointers to transition
       */
      using transition_ptr_t = typename tchecker::ts::builder_ok_t<TS, ALLOCATOR>::transition_ptr_t;
      
      /*!
      \brief Type of underlying builder
      */
      using builder_t = tchecker::ts::builder_ok_t<TS, ALLOCATOR>;
      
      /*!
       \brief Constructors
       */
      using tchecker::ts::builder_ok_t<TS, ALLOCATOR>::builder_ok_t;

      /*!
      \brief Destructor
      */
      virtual ~full_states_builder_t() = default;

      /*!
      \brief Computes initial states
      \param v : states container
      \post all initial states have been pushed back in v
      */
      virtual void initial(std::vector<state_ptr_t> & v)
      {
        state_ptr_t state{nullptr};
        transition_ptr_t transition{nullptr};
        
        auto initial_range = builder_t::initial();
        for (auto it = initial_range.begin(); ! it.at_end(); ++it) {
          std::tie(state, transition) = *it;
          assert(state != state_ptr_t{nullptr});
          v.push_back(state);
        }
      }

      /*!
      \brief Computes next states
      \param s : a state
      \param v : states container
      \post all successor states of s have been pushed back in v
      */
      virtual void next(state_ptr_t & s, std::vector<state_ptr_t> & v)
      {
        state_ptr_t next_state{nullptr};
        transition_ptr_t transition{nullptr};
        
        auto outgoing_range = builder_t::outgoing(s);
        for (auto it = outgoing_range.begin(); ! it.at_end(); ++it) {
          std::tie(next_state, transition) = *it;
          assert(next_state != state_ptr_t{nullptr});
          v.push_back(next_state);
        }
      }
    };
    
  } // end of namespace covreach
  
} // end of namespace tchecker

#endif // TCHECKER_ALGORITHMS_COVREACH_BUILDER_HH
