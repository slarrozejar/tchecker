/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#include "tchecker/dbm/offset_dbm.hh"
#include "tchecker/utils/ordering.hh"

#define DBM(i,j)         dbm[(i)*dim+(j)]
#define OFFSET_DBM(i,j)  offset_dbm[(i)*offset_dim+(j)]
#define OFFSET_DBM1(i,j) offset_dbm1[(i)*offset_dim+(j)]
#define OFFSET_DBM2(i,j) offset_dbm2[(i)*offset_dim+(j)]

namespace tchecker {
  
  namespace offset_dbm {
    
    void universal(tchecker::dbm::db_t * offset_dbm, tchecker::clock_id_t offset_dim)
    {
      assert(offset_dbm != nullptr);
      assert(offset_dim >= 1);
      tchecker::dbm::universal(offset_dbm, offset_dim);
    }
    
    
    void empty(tchecker::dbm::db_t * offset_dbm, tchecker::clock_id_t offset_dim)
    {
      assert(offset_dbm != nullptr);
      assert(offset_dim >= 1);
      tchecker::dbm::empty(offset_dbm, offset_dim);
    }
    
    
    void universal_positive(tchecker::dbm::db_t * offset_dbm, tchecker::clock_id_t offset_dim,
                            tchecker::clock_id_t refcount, tchecker::clock_id_t const * refmap)
    {
      assert(offset_dbm != nullptr);
      assert(refcount >= 1);
      assert(refcount <= offset_dim);
      assert(refmap != nullptr);
      
      tchecker::offset_dbm::universal(offset_dbm, offset_dim);
      // clocks are non-negative: x>=0 <=> X>=RX <=> RX-X<=0
      for (tchecker::clock_id_t i = refcount; i < offset_dim; ++i)
        OFFSET_DBM(refmap[i], i) = tchecker::dbm::LE_ZERO;
    }
    
    
    void zero(tchecker::dbm::db_t * offset_dbm, tchecker::clock_id_t offset_dim)
    {
      assert(offset_dbm != nullptr);
      assert(offset_dim >= 1);
      tchecker::dbm::zero(offset_dbm, offset_dim);
    }
    
    
    bool is_empty_0(tchecker::dbm::db_t const * offset_dbm, tchecker::clock_id_t offset_dim)
    {
      assert(offset_dbm != nullptr);
      assert(offset_dim >= 1);
      return tchecker::dbm::is_empty_0(offset_dbm, offset_dim);
    }
    
    
    bool is_universal(tchecker::dbm::db_t const * offset_dbm, tchecker::clock_id_t offset_dim)
    {
      assert(offset_dbm != nullptr);
      assert(offset_dim >= 1);
      return tchecker::dbm::is_universal(offset_dbm, offset_dim);
    }
    
    
    bool is_positive(tchecker::dbm::db_t const * offset_dbm, tchecker::clock_id_t offset_dim,
                     tchecker::clock_id_t refcount, tchecker::clock_id_t const * refmap)
    {
      assert(offset_dbm != nullptr);
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
      assert(offset_dim >= 1);
      assert(refmap != nullptr);
      
      // RX-X are less-or-equal to <=0
      for (tchecker::clock_id_t i = refcount; i < offset_dim; ++i)
        if (OFFSET_DBM(refmap[i], i) > tchecker::dbm::LE_ZERO)
          return false;
      return true;
    }
    
    
    bool is_universal_positive(tchecker::dbm::db_t const * offset_dbm, tchecker::clock_id_t offset_dim,
                               tchecker::clock_id_t refcount, tchecker::clock_id_t const * refmap)
    {
      assert(offset_dbm != nullptr);
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
      assert(1 <= refcount);
      assert(refcount <= offset_dim);
      assert(refmap != nullptr);
      
      // <inf everywhere, except <=0 on the diagonal and for RX-X
      for (tchecker::clock_id_t i = 0; i < offset_dim; ++i) {
        for (tchecker::clock_id_t j = 0; j < offset_dim; ++j) {
          tchecker::clock_id_t rj = refmap[j];
          tchecker::dbm::db_t expected = ((i == j) || (i == rj)
                                          ? tchecker::dbm::LE_ZERO
                                          : tchecker::dbm::LT_INFINITY);
          if (OFFSET_DBM(i, j) != expected)
            return false;
        }
      }
      return true;
    }
    
    
    bool is_tight(tchecker::dbm::db_t const * offset_dbm, tchecker::clock_id_t offset_dim)
    {
      assert(offset_dbm != nullptr);
      assert(offset_dim >= 1);
      return tchecker::dbm::is_tight(offset_dbm, offset_dim);
    }
    
    
    bool is_spread_bounded(tchecker::dbm::db_t const * offset_dbm, tchecker::clock_id_t offset_dim,
                           tchecker::clock_id_t refcount, tchecker::integer_t spread)
    {
      assert(offset_dbm != nullptr);
      assert(tchecker::dbm::is_consistent(offset_dbm, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
      assert(1 <= refcount);
      assert(refcount <= offset_dim);
      assert(0 <= spread);
      
      tchecker::dbm::db_t le_spread = tchecker::dbm::db(tchecker::dbm::LE, spread);
      for (tchecker::clock_id_t r1 = 0; r1 < refcount; ++r1)
        for (tchecker::clock_id_t r2 = 0; r2 < refcount; ++r2)
          if (OFFSET_DBM(r1, r2) > le_spread)
            return false;
      return true;
    }
    
    
    bool is_synchronized(tchecker::dbm::db_t const * offset_dbm, tchecker::clock_id_t offset_dim,
                         tchecker::clock_id_t refcount)
    {
      assert(offset_dbm != nullptr);
      assert(tchecker::dbm::is_consistent(offset_dbm, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
      assert(1 <= refcount);
      assert(refcount <= offset_dim);
      
      return tchecker::offset_dbm::is_spread_bounded(offset_dbm, offset_dim, refcount, 0);
    }
    
    
    bool is_equal(tchecker::dbm::db_t const * offset_dbm1, tchecker::dbm::db_t const * offset_dbm2,
                  tchecker::clock_id_t offset_dim)
    {
      assert(offset_dbm1 != nullptr);
      assert(offset_dbm2 != nullptr);
      assert(tchecker::offset_dbm::is_tight(offset_dbm1, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm2, offset_dim));
      assert(tchecker::dbm::is_consistent(offset_dbm1, offset_dim));
      assert(tchecker::dbm::is_consistent(offset_dbm2, offset_dim));
      assert(offset_dim >= 1);
      return tchecker::dbm::is_equal(offset_dbm1, offset_dbm2, offset_dim);
    }
    
    
    bool is_le(tchecker::dbm::db_t const * offset_dbm1, tchecker::dbm::db_t const * offset_dbm2,
               tchecker::clock_id_t offset_dim)
    {
      assert(offset_dbm1 != nullptr);
      assert(offset_dbm2 != nullptr);
      assert(tchecker::offset_dbm::is_tight(offset_dbm1, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm2, offset_dim));
      assert(tchecker::dbm::is_consistent(offset_dbm1, offset_dim));
      assert(tchecker::dbm::is_consistent(offset_dbm2, offset_dim));
      assert(offset_dim >= 1);
      return tchecker::dbm::is_le(offset_dbm1, offset_dbm2, offset_dim);
    }
    
    
    bool is_am_le(tchecker::dbm::db_t const * offset_dbm1, tchecker::dbm::db_t const * offset_dbm2,
                  tchecker::clock_id_t offset_dim, tchecker::clock_id_t refcount,
                  tchecker::clock_id_t const * refmap, tchecker::integer_t const * m)
    {
      assert(offset_dbm1 != nullptr);
      assert(offset_dbm2 != nullptr);
      assert(tchecker::offset_dbm::is_tight(offset_dbm1, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm2, offset_dim));
      assert(tchecker::dbm::is_consistent(offset_dbm1, offset_dim));
      assert(tchecker::dbm::is_consistent(offset_dbm2, offset_dim));
      assert(offset_dim >= 1);
      assert(m[0] == 0);
      
      for (tchecker::clock_id_t r1 = 0; r1 < refcount; ++r1)
        for (tchecker::clock_id_t r2 = 0; r2 < refcount; ++r2)
          if (OFFSET_DBM1(r1,r2) > OFFSET_DBM2(r1,r2))
            return false;
      
      for (tchecker::clock_id_t x = refcount; x < offset_dim; ++x) {
        tchecker::integer_t const Mx = m[x-refcount+1];
        if (Mx == -tchecker::dbm::INF_VALUE)
          continue;
        
        tchecker::dbm::db_t const lt_minus_Mx = tchecker::dbm::db(tchecker::dbm::LT, -Mx);
        
        for (tchecker::clock_id_t y = refcount; y < offset_dim; ++y) {
          if (x == y)
            continue;
          
          tchecker::integer_t const My = m[y-refcount+1];
          if (My == -tchecker::dbm::INF_VALUE)
            continue;
          
          tchecker::clock_id_t const rx = refmap[x];
          tchecker::clock_id_t const ry = refmap[y];
          
          // IMPLEMENTATION NOTE: we use negation of OFFSET_DBM1 and OFFSET_DBM2 since the test
          // is designed for offset zones with increasing time, whereas this implementation uses
          // decreasing time
          /* The checks reads as:
               offset_dbm1(y,r(y)) >= (<=,-m(y)) &&
               offset_dbm2(y,x) < offset_dbm1(y,x) &&
               offset_dbm2(y,x) + (<,-m(x)) < offset_dbm1(y,r(x))
           plus, we need negation of offset DBMs entries as observed above
           */
          if ((-OFFSET_DBM1(y,ry) >= tchecker::dbm::db(tchecker::dbm::LE, -My)) &&
              (-OFFSET_DBM2(y,x) < -OFFSET_DBM1(y,x)) &&
              (tchecker::dbm::sum(-OFFSET_DBM2(y,x), lt_minus_Mx) < -OFFSET_DBM1(y, rx)))
            return false;
        }
      }
      
      return true;
    }
    
    
    std::size_t hash(tchecker::dbm::db_t const * offset_dbm, tchecker::clock_id_t offset_dim)
    {
      assert(offset_dbm != nullptr);
      assert(offset_dim >= 1);
      return tchecker::dbm::hash(offset_dbm, offset_dim);
    }
    
    
    enum tchecker::dbm::status_t
    constrain(tchecker::dbm::db_t * offset_dbm, tchecker::clock_id_t offset_dim, tchecker::clock_id_t x,
              tchecker::clock_id_t y, tchecker::dbm::comparator_t cmp, tchecker::integer_t value)
    {
      assert(offset_dbm != nullptr);
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
      assert(offset_dim >= 1);
      assert(x < offset_dim);
      assert(y < offset_dim);
      assert(x != y);
      return tchecker::dbm::constrain(offset_dbm, offset_dim, x, y, cmp, value);
    }
    
    
    enum tchecker::dbm::status_t bound_spread(tchecker::dbm::db_t * offset_dbm, tchecker::clock_id_t offset_dim,
                                              tchecker::clock_id_t refcount, tchecker::integer_t spread)
    {
      assert(offset_dbm != nullptr);
      assert(tchecker::dbm::is_consistent(offset_dbm, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
      assert(1 <= refcount);
      assert(refcount <= offset_dim);
      assert(0 <= spread);
      
      tchecker::dbm::db_t le_spread = tchecker::dbm::db(tchecker::dbm::LE, spread);
      
      for (tchecker::clock_id_t r1 = 0; r1 < refcount; ++r1)
        for (tchecker::clock_id_t r2 = 0; r2 < refcount; ++r2)
          OFFSET_DBM(r1, r2) = tchecker::dbm::min(OFFSET_DBM(r1, r2), le_spread);

      // Optimized tightening: Floyd-Warshall algorithm w.r.t. reference clocks
      for (tchecker::clock_id_t r = 0; r < refcount; ++r) {
        for (tchecker::clock_id_t x = 0; x < offset_dim; ++x) {
          if ((x == r) || (OFFSET_DBM(x,r) == tchecker::dbm::LT_INFINITY))
            continue;   // optimization
          
          for (tchecker::clock_id_t y = 0; y < offset_dim; ++y) {
            if (y == r)
              continue;  // optimization
            
            OFFSET_DBM(x,y) = tchecker::dbm::min(tchecker::dbm::sum(OFFSET_DBM(x,r), OFFSET_DBM(r,y)),
                                                 OFFSET_DBM(x,y));
          }
          
          if (OFFSET_DBM(x,x) < tchecker::dbm::LE_ZERO) {
            OFFSET_DBM(0,0) = tchecker::dbm::LT_ZERO;
            return tchecker::dbm::EMPTY;
          }
        }
      }
      
      assert(tchecker::dbm::is_consistent(offset_dbm, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
      
      return tchecker::dbm::NON_EMPTY;
    }
    
    
    enum tchecker::dbm::status_t synchronize(tchecker::dbm::db_t * offset_dbm, tchecker::clock_id_t offset_dim,
                                             tchecker::clock_id_t refcount)
    {
      assert(offset_dbm != nullptr);
      assert(tchecker::dbm::is_consistent(offset_dbm, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
      assert(1 <= refcount);
      assert(refcount <= offset_dim);
      
      return tchecker::offset_dbm::bound_spread(offset_dbm, offset_dim, refcount, 0);
    }
    
    
    void reset_to_refclock(tchecker::dbm::db_t * offset_dbm, tchecker::clock_id_t offset_dim,
                           tchecker::clock_id_t x, tchecker::clock_id_t refcount,
                           tchecker::clock_id_t const * refmap)
    {
      assert(offset_dbm != nullptr);
      assert(tchecker::dbm::is_consistent(offset_dbm, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
      assert(offset_dim >= 1);
      assert(x < offset_dim);
      
      if (refmap[x] == x)
        return;
      
      // x is identified to rx w.r.t. all clocks z
      for (tchecker::clock_id_t z = 0; z < offset_dim; ++z) {
        OFFSET_DBM(x,z) = OFFSET_DBM(refmap[x],z);
        OFFSET_DBM(z,x) = OFFSET_DBM(z,refmap[x]);
      }
      OFFSET_DBM(x,x) = tchecker::dbm::LE_ZERO; // cheaper than testing in loop
      
      assert(tchecker::dbm::is_consistent(offset_dbm, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
    }
    
    
    void asynchronous_open_up(tchecker::dbm::db_t * offset_dbm, tchecker::clock_id_t offset_dim,
                              tchecker::clock_id_t refcount)
    {
      assert(offset_dbm != nullptr);
      assert(tchecker::dbm::is_consistent(offset_dbm, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
      assert(1 <= refcount);
      assert(refcount <= offset_dim);
      
      // X - R < inf for all X and R (including X=R')
      for (tchecker::clock_id_t r = 0; r < refcount; ++r) {
        for (tchecker::clock_id_t i = 0; i < offset_dim; ++i)
          OFFSET_DBM(i,r) = tchecker::dbm::LT_INFINITY;
        OFFSET_DBM(r,r) = tchecker::dbm::LE_ZERO;
      }
      
      assert(tchecker::dbm::is_consistent(offset_dbm, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
    }
    
    
    void asynchronous_open_up(tchecker::dbm::db_t * offset_dbm, tchecker::clock_id_t offset_dim,
                              tchecker::clock_id_t refcount, boost::dynamic_bitset<> const & delay_allowed)
    {
      assert(offset_dbm != nullptr);
      assert(tchecker::dbm::is_consistent(offset_dbm, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
      assert(1 <= refcount);
      assert(refcount <= offset_dim);
      assert(refcount == delay_allowed.size());
      
      // X - R < inf for all X and R (including X=R')
      for (tchecker::clock_id_t r = 0; r < refcount; ++r) {
        if (! delay_allowed[r])
          continue;
        
        for (tchecker::clock_id_t i = 0; i < offset_dim; ++i)
          OFFSET_DBM(i,r) = tchecker::dbm::LT_INFINITY;
        OFFSET_DBM(r,r) = tchecker::dbm::LE_ZERO;
      }
      
      assert(tchecker::dbm::is_consistent(offset_dbm, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
    }
    
    
    enum tchecker::dbm::status_t tighten(tchecker::dbm::db_t * offset_dbm, tchecker::clock_id_t offset_dim)
    {
      assert(offset_dbm != nullptr);
      assert(offset_dim >= 1);
      return tchecker::dbm::tighten(offset_dbm, offset_dim);
    }
    
    
    void to_dbm(tchecker::dbm::db_t const * offset_dbm, tchecker::clock_id_t offset_dim,
                tchecker::clock_id_t refcount, tchecker::clock_id_t const * refmap,
                tchecker::dbm::db_t * dbm, tchecker::clock_id_t dim)
    {
      assert(offset_dbm != nullptr);
      assert(tchecker::dbm::is_consistent(offset_dbm, offset_dim));
      assert(tchecker::offset_dbm::is_tight(offset_dbm, offset_dim));
      assert(tchecker::offset_dbm::is_synchronized(offset_dbm, offset_dim, refcount) );
      assert(1 <= refcount);
      assert(refcount <= offset_dim);
      assert(refmap != nullptr);
      assert(dbm != nullptr);
      assert(dim == offset_dim - refcount + 1);
      
      for (tchecker::clock_id_t i = 1; i < dim; ++i) {
        auto reference_i = tchecker::offset_dbm::reference_id(i, refcount, refmap);
        auto offset_i = tchecker::offset_dbm::offset_id(i, refcount);
        DBM(0,i) = OFFSET_DBM(reference_i, offset_i);
        DBM(i,0) = OFFSET_DBM(offset_i, reference_i);
        for (tchecker::clock_id_t j = i; j < dim; ++j) {
          auto offset_j = tchecker::offset_dbm::offset_id(j, refcount);
          DBM(i,j) = OFFSET_DBM(offset_i, offset_j);
          if (i != j)
            DBM(j,i) = OFFSET_DBM(offset_j, offset_i);
        }
      }
      DBM(0,0) = tchecker::dbm::LE_ZERO;
      
      assert(tchecker::dbm::is_tight(dbm, dim));
    }
    
    
    std::ostream & output_matrix(std::ostream & os, tchecker::dbm::db_t const * offset_dbm,
                                 tchecker::clock_id_t offset_dim)
    {
      return tchecker::dbm::output_matrix(os, offset_dbm, offset_dim);
    }
    
    
    std::ostream & output(std::ostream & os, tchecker::dbm::db_t const * offset_dbm,
                          tchecker::clock_id_t offset_dim,
                          std::function<std::string(tchecker::clock_id_t)> clock_name)
    {
      bool first = true;
      
      os << "(";

      for (tchecker::clock_id_t i = 0; i < offset_dim; ++i) {
        for (tchecker::clock_id_t j = i+1; j < offset_dim; ++j) {
          tchecker::dbm::db_t cij = OFFSET_DBM(i,j), cji = OFFSET_DBM(j,i);
          // vi == vj + k
          if (tchecker::dbm::sum(cij, cji) == tchecker::dbm::LE_ZERO) {
            if (! first)
              os << " & ";
            first = false;
            
            os << clock_name(i) << "=" << clock_name(j);
            tchecker::integer_t vij = tchecker::dbm::value(cij);
            if (vij > 0)
              os << "+" << tchecker::dbm::value(cij);
            else if (vij < 0)
              os << "-" << - tchecker::dbm::value(cij);
          }
          // k1 <= xi - xj <= k2
          else if ((cij != tchecker::dbm::LT_INFINITY) || (cji != tchecker::dbm::LT_INFINITY)) {
            if (! first)
              os << " & ";
            first = false;
            
            if (cji != tchecker::dbm::LT_INFINITY)
              os << - tchecker::dbm::value(cji) << tchecker::dbm::comparator_str(cji);
            
            os << clock_name(i) << "-" << clock_name(j);
            
            if (cij != tchecker::dbm::LT_INFINITY)
              os << tchecker::dbm::comparator_str(cij) << tchecker::dbm::value(cij);
          }
        }
      }
      
      os << ")";
      
      return os;
    }


    int lexical_cmp(tchecker::dbm::db_t const * offset_dbm1, tchecker::clock_id_t offset_dim1,
                    tchecker::dbm::db_t const * offset_dbm2, tchecker::clock_id_t offset_dim2)
    {
      assert(offset_dbm1 != nullptr);
      assert(offset_dbm2 != nullptr);
      assert(offset_dim1 >= 1);
      assert(offset_dim2 >= 1);
      return tchecker::lexical_cmp(offset_dbm1, offset_dbm1 + offset_dim1 * offset_dim1,
                                   offset_dbm2, offset_dbm2 + offset_dim2 * offset_dim2,
                                   tchecker::dbm::db_cmp);
    }
    
  } // end of namespace offset_dbm
  
} // end of namespace tchecker
