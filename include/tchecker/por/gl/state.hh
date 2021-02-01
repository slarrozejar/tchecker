/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_GL_STATE_HH
#define TCHECKER_POR_GL_STATE_HH

#include <limits>

#if BOOST_VERSION <= 106600
# include <boost/functional/hash.hpp>
#else
# include <boost/container_hash/hash.hpp>
#endif

#include "tchecker/basictypes.hh"
#include "tchecker/utils/allocation_size.hh"

/*!
 \file state.hh
 \brief States for global-local POR
 */

namespace tchecker {
  
  namespace por {

    namespace gl {

      /*! Rank value of global transitions */
      constexpr tchecker::process_id_t const global = 
      std::numeric_limits<tchecker::process_id_t>::max();
    
    
      /*!
      \class state_t
      \brief State for global-local POR
      */
      class state_t {
      public:
        /*!
        \brief Constructor
        \param rank : state rank
        \post this state's rank has been set to rank
        */
        state_t(tchecker::process_id_t rank = 0);
      
        /*!
        \brief Accessor
        \return this state's rank
        */
        tchecker::process_id_t por_rank() const;
      
        /*!
        \brief Set state rank
        \param rank : a rank
        \post this state's rank has been set to rank
        */
        void por_rank(tchecker::process_id_t rank);

        /*!
        \brief Equality predicate
        \param s : state
        \return true if this and s have the same rank, false otherwise
        */
        bool operator== (tchecker::por::gl::state_t const & s) const;
      
        /*!
        \brief Disequality predicate
        \param s : state
        \return true if this and s do not have the same rank, false otherwise
        */
        bool operator!= (tchecker::por::gl::state_t const & s) const;
      private:
        tchecker::process_id_t _por_rank;   /*!< Rank */
      };
    
    
      /*!
      \brief Hash
      \param s : state
      \return hash value for s
      */
      std::size_t hash_value(tchecker::por::gl::state_t const & s);
    
    
      /*!
      \brief Lexical ordering
      \param s1 : state
      \param s2 : state
      \return <0 if s1 has smaller rank than s2, 0 if same rank, >0 otherwise
      */
      int lexical_cmp(tchecker::por::gl::state_t const & s1, 
                      tchecker::por::gl::state_t const & s2);
    
    
      /*!
      \brief Make state a global-local POR state
      \tparam STATE : type of state
      \note make_state_t<STATE> derives both from STATE and from tchecker::por::gl::state_t
      */
      template <class STATE>
      class make_state_t : public STATE, public tchecker::por::gl::state_t {
      public:
        using STATE::STATE;
      
        /*!
        \brief Constructor
        \param s : a state
        \param args : arguments to a constructor STATE::STATE(s, args...)
        */
        template <class ... ARGS>
        make_state_t(tchecker::por::gl::make_state_t<STATE> const & s, 
                     ARGS && ... args)
        : STATE(s, args...), tchecker::por::gl::state_t(s)
        {}
      };
    
    
      /*!
      \brief Equality predicate
      \param s1 : state
      \param s2 : state
      \return true if s1 and s2 are equal w.r.t. both STATE and tchecker::por::gl::state_t, false otherwise
      */
      template <class STATE>
      bool operator== (tchecker::por::gl::make_state_t<STATE> const & s1,
                       tchecker::por::gl::make_state_t<STATE> const & s2)
      {
        return
        ((static_cast<tchecker::por::gl::state_t const &>(s1) == 
          static_cast<tchecker::por::gl::state_t const &>(s2))
        &&
         (static_cast<STATE const &>(s1) == static_cast<STATE const &>(s2))
        );
      }
    
    
      /*!
      \brief Disequality predicate
      \param s1 : state
      \param s2 : state
      \return false if s1 and s2 are equal w.r.t. both STATE and tchecker::por::cs::state_t, true otherwise
      */
      template <class STATE>
      bool operator!= (tchecker::por::gl::make_state_t<STATE> const & s1,
                       tchecker::por::gl::make_state_t<STATE> const & s2)
      {
        return ! (s1 == s2);
      }
    
    
      /*!
      \brief Hash
      \param s : state
      \return hash code for s
      */
      template <class STATE>
      std::size_t hash_value(tchecker::por::gl::make_state_t<STATE> const & s)
      {
        std::size_t h = tchecker::por::gl::hash_value
        (static_cast<tchecker::por::gl::state_t const &>(s));
        boost::hash_combine(h, hash_value(static_cast<STATE const &>(s)));
        return h;
      }
    
    
      /*!
      \brief Lexical ordering
      \param s1 : state
      \param s2 : state
      \return <0 if s1 is smaller than s2, 0 if s1 and s2 are equal, >0
      otherwise, w.r.t. lexical ordering over both STATE and 
      tchecker::por::cs::state_t
      */
      template <class STATE>
      int lexical_cmp(tchecker::por::gl::make_state_t<STATE> const & s1, 
                      tchecker::por::gl::make_state_t<STATE> const & s2)
      {
        int cmp = lexical_cmp(static_cast<STATE const &>(s1), 
                              static_cast<STATE const &>(s2));
        if (cmp != 0)
          return cmp;
        return tchecker::por::gl::lexical_cmp
        (static_cast<tchecker::por::gl::state_t const &>(s1),
         static_cast<tchecker::por::gl::state_t const &>(s2));
      }


      /*!
       \brief Covering check
       \param s1 : state
       \param s2 : state
       \return true if s1 can be covered by s2, false othewise
       */
      bool cover_leq(tchecker::por::gl::state_t const & s1, 
                     tchecker::por::gl::state_t const & s2);

    } // end of namespace gl
    
  } // end of namespace por


  /*!
   \brief Specialization of class allocation_size_t for 
   tchecker::por::gl::state_t
   */
  template <class STATE>
  class allocation_size_t<tchecker::por::gl::make_state_t<STATE>> {
  public:
    template <class ... ARGS>
    static constexpr std::size_t alloc_size(ARGS && ... args)
    {
      return sizeof(tchecker::por::gl::make_state_t<STATE>);
    }
  };
  
} // end of namespace tchecker

#endif // TCHECKER_POR_GL_STATE_HH
