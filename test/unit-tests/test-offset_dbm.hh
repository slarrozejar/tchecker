/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#include "tchecker/clockbounds/clockbounds.hh"
#include "tchecker/dbm/offset_dbm.hh"

/* Tests are provided for functions over offset DBMs. We do not test functions
 * that are only call the corresponding function over DBMs.
 */

#define DBM(i,j)              dbm[(i)*dim+(j)]
#define DBM2(i,j)             dbm2[(i)*dim+(j)]
#define OFFSET_DBM(i,j)       offset_dbm[(i)*offset_dim+(j)]
#define OFFSET_DBM2(i,j)      offset_dbm2[(i)*offset_dim+(j)]

TEST_CASE( "translation clocks <-> offset variables", "[offset_dbm]") {
  
  tchecker::clock_id_t const dim = 7;
  tchecker::clock_id_t const refcount = 3;
  tchecker::clock_id_t const offset_dim = dim - 1 + refcount;
  tchecker::clock_id_t const refmap[] = {0, 1, 2, 0, 0, 1, 1, 1, 2};
  
  SECTION( "clock -> offset variable" ) {
    for (tchecker::clock_id_t i = 1; i < dim; ++i)
      REQUIRE(tchecker::offset_dbm::offset_id(i, refcount) == refcount + i - 1);
  }
  
  SECTION( "clock -> reference clock" ) {
    for (tchecker::clock_id_t i = 1; i < dim; ++i)
      REQUIRE(tchecker::offset_dbm::reference_id(i, refcount, refmap) == refmap[refcount + i -1]);
  }
  
  SECTION( "reference/offset variable -> clock" ) {
    for (tchecker::clock_id_t i = 0; i < refcount; ++i)
      REQUIRE(tchecker::offset_dbm::clock_id(i, refcount) == tchecker::zero_clock_id);
    for (tchecker::clock_id_t i = refcount; i < offset_dim; ++i)
      REQUIRE(tchecker::offset_dbm::clock_id(i, refcount) == i - refcount + 1);
  }
  
}



TEST_CASE( "is_positive, structural test for offset DBMs", "[offset_dbm]" ) {
  
  tchecker::clock_id_t const dim = 5;
  tchecker::clock_id_t const refcount = 2;
  tchecker::clock_id_t const offset_dim = dim - 1 + refcount;
  tchecker::clock_id_t const refmap[] = {0, 1, 0, 0, 1, 1};
  
  SECTION( "is_positive, on offset DBM with offset_dim = refcount (i.e. no clock)" ) {
    tchecker::dbm::db_t offset_dbm[refcount * refcount]
    = {tchecker::dbm::LE_ZERO, tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO};
    
    REQUIRE(tchecker::offset_dbm::is_positive(offset_dbm, refcount, refcount, refmap));
  }
  
  SECTION( "is_positive, on positive offset DBM, offset_dim > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    for (tchecker::clock_id_t i = 0; i < offset_dim; ++i) {
      for (tchecker::clock_id_t j = 0; j < offset_dim; ++j)
        OFFSET_DBM(i,j) = tchecker::dbm::LT_INFINITY;
      OFFSET_DBM(i,i) = tchecker::dbm::LE_ZERO;
      if (i >= refcount)
        OFFSET_DBM(refmap[i], i) = tchecker::dbm::db(tchecker::dbm::LE,
                                                     (tchecker::integer_t)refcount - (tchecker::integer_t)i);
    }
    
    REQUIRE(tchecker::offset_dbm::is_positive(offset_dbm, offset_dim, refcount, refmap));
  }
  
  SECTION( "is_positive, on non-positive offset DBM, offset_dim > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    for (tchecker::clock_id_t i = 0; i < offset_dim; ++i) {
      for (tchecker::clock_id_t j = 0; j < offset_dim; ++j)
        OFFSET_DBM(i,j) = tchecker::dbm::LT_INFINITY;
      OFFSET_DBM(i,i) = tchecker::dbm::LE_ZERO;
      if (i >= refcount)
        OFFSET_DBM(refmap[i], i) = tchecker::dbm::db(tchecker::dbm::LE, (tchecker::integer_t)i - (tchecker::integer_t)refcount);
    }
    
    REQUIRE_FALSE(tchecker::offset_dbm::is_positive(offset_dbm, offset_dim, refcount, refmap));
  }
}



TEST_CASE( "is_universal_positive, structural test for offset DBMs", "[offset_dbm]" ) {
  
  tchecker::clock_id_t const dim = 6;
  tchecker::clock_id_t const refcount = 2;
  tchecker::clock_id_t const offset_dim = dim - 1 + refcount;
  tchecker::clock_id_t const refmap[] = {0, 1, 0, 0, 1, 1, 1};
  
  SECTION( "is_universal_positive, on offset DBM with offset_dim = refcount (i.e. no clock)" ) {
    tchecker::dbm::db_t offset_dbm[refcount * refcount]
    = {tchecker::dbm::LE_ZERO, tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO};
    
    REQUIRE(tchecker::offset_dbm::is_universal_positive(offset_dbm, refcount, refcount, refmap));
  }
  
  SECTION( "is_universal_positive, on positive offset DBM, offset_dim > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    for (tchecker::clock_id_t i = 0; i < offset_dim; ++i) {
      for (tchecker::clock_id_t j = 0; j < offset_dim; ++j)
        OFFSET_DBM(i,j) = tchecker::dbm::LT_INFINITY;
      OFFSET_DBM(i,i) = tchecker::dbm::LE_ZERO;
    }
    for (tchecker::clock_id_t i = refcount; i < offset_dim; ++i)
      OFFSET_DBM(refmap[i], i) = tchecker::dbm::LE_ZERO;
    
    REQUIRE(tchecker::offset_dbm::is_universal_positive(offset_dbm, offset_dim, refcount, refmap));
  }
  
  SECTION( "is_universal_positive, on non-positive offset DBM, offset_dim > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    for (tchecker::clock_id_t i = 0; i < offset_dim; ++i) {
      for (tchecker::clock_id_t j = 0; j < offset_dim; ++j)
        OFFSET_DBM(i,j) = tchecker::dbm::LT_INFINITY;
      OFFSET_DBM(i,i) = tchecker::dbm::LE_ZERO;
      if (i >= refcount)
        OFFSET_DBM(refmap[i], i) = tchecker::dbm::db(tchecker::dbm::LE, i - refcount);
    }
    
    REQUIRE_FALSE(tchecker::offset_dbm::is_universal_positive(offset_dbm, offset_dim, refcount, refmap));
  }
  
  SECTION( "is_universal_positive, on non-universal offset DBM, offset_dim > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    for (tchecker::clock_id_t i = 0; i < offset_dim; ++i) {
      for (tchecker::clock_id_t j = 0; j < offset_dim; ++j)
        OFFSET_DBM(i,j) = tchecker::dbm::LT_INFINITY;
      OFFSET_DBM(i,i) = tchecker::dbm::LE_ZERO;
      if (i >= refcount)
        OFFSET_DBM(refmap[i], i) = tchecker::dbm::db(tchecker::dbm::LE,
                                                     (tchecker::integer_t)refcount-(tchecker::integer_t)i);
    }
    OFFSET_DBM(refcount, refcount + 1) = tchecker::dbm::db(tchecker::dbm::LT, 4);
    
    REQUIRE_FALSE(tchecker::offset_dbm::is_universal_positive(offset_dbm, offset_dim, refcount, refmap));
  }
}



TEST_CASE( "is_spread_bounded, structural test for offset DBMs", "[offset_dbm]" ) {

  tchecker::clock_id_t const dim = 3;
  tchecker::clock_id_t const refcount = 3;
  tchecker::clock_id_t const offset_dim = dim - 1 + refcount;
  tchecker::clock_id_t const refmap[] = {0, 1, 2, 0, 1};
  
  tchecker::clock_id_t const r0 = 0;
  tchecker::clock_id_t const r1 = 1;
  tchecker::clock_id_t const r2 = 2;
  
  tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
  tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
  OFFSET_DBM(r0,r1) = tchecker::dbm::LE_ZERO;
  OFFSET_DBM(r0,r2) = tchecker::dbm::db(tchecker::dbm::LE, -1);
  OFFSET_DBM(r1,r0) = tchecker::dbm::db(tchecker::dbm::LE, 1);
  OFFSET_DBM(r1,r2) = tchecker::dbm::db(tchecker::dbm::LE, -1);
  OFFSET_DBM(r2,r0) = tchecker::dbm::db(tchecker::dbm::LE, 3);
  OFFSET_DBM(r2,r1) = tchecker::dbm::db(tchecker::dbm::LE, 3);
  
  auto tight = tchecker::offset_dbm::tighten(offset_dbm, offset_dim);
  REQUIRE(tight == tchecker::dbm::NON_EMPTY);
  
  SECTION( "spread bounded offset DBM" ) {
    tchecker::integer_t const spread = 3;
    REQUIRE(tchecker::offset_dbm::is_spread_bounded(offset_dbm, offset_dim, refcount, spread));
  }
  
  SECTION( "offset DBM that is not spread bounded" ) {
    tchecker::integer_t const spread = 2;
    REQUIRE_FALSE(tchecker::offset_dbm::is_spread_bounded(offset_dbm, offset_dim, refcount, spread));
  }
}



TEST_CASE( "is_synchronized, structural test for offset DBMs", "[offset_dbm]" ) {
  
  tchecker::clock_id_t const dim = 5;
  tchecker::clock_id_t const refcount = 2;
  tchecker::clock_id_t const offset_dim = dim - 1 + refcount;
  
  SECTION( "the synchronized offset DBM is synchronized, dimension refcount" ) {
    tchecker::dbm::db_t offset_dbm[refcount * refcount]
    = {tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO};
    
    REQUIRE(tchecker::offset_dbm::is_synchronized(offset_dbm, refcount, refcount));
  }
  
  SECTION( "a non-synchronized offset DBM is not synchronized, dimension refcount" ) {
    tchecker::dbm::db_t offset_dbm[refcount * refcount]
    = {tchecker::dbm::LE_ZERO, tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO};
    
    REQUIRE_FALSE(tchecker::offset_dbm::is_synchronized(offset_dbm, refcount, refcount));
  }
  
  SECTION( "a synchronized offset DBM is synchronized, dimension > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    for (tchecker::clock_id_t i = 0; i < offset_dim; ++i) {
      for (tchecker::clock_id_t j = 0; j < offset_dim; ++j)
        OFFSET_DBM(i, j) = ((i < refcount) && (j < refcount)
                            ? tchecker::dbm::LE_ZERO
                            : tchecker::dbm::LT_INFINITY);
      OFFSET_DBM(i, i) = tchecker::dbm::LE_ZERO;
    }
    
    REQUIRE(tchecker::offset_dbm::is_synchronized(offset_dbm, offset_dim, refcount));
  }
  
  SECTION( "a non-synchronized offset DBM is not synchronized, dimension > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    for (tchecker::clock_id_t i = 0; i < offset_dim; ++i) {
      for (tchecker::clock_id_t j = 0; j < offset_dim; ++j)
        OFFSET_DBM(i, j) = tchecker::dbm::LT_INFINITY;
      OFFSET_DBM(i, i) = tchecker::dbm::LE_ZERO;
    }
    
    REQUIRE_FALSE(tchecker::offset_dbm::is_synchronized(offset_dbm, offset_dim, refcount));
  }
  
}



TEST_CASE ("is_am_le offset DBMs, inclusion in universal zone", "[offset_dbm]" ) {

  tchecker::clock_id_t const dim = 3;
  tchecker::clock_id_t const refcount = 2;
  tchecker::clock_id_t const offset_dim = dim - 1 + refcount;
  tchecker::clock_id_t const refmap[] = {0, 1, 0, 1};
  
  tchecker::clockbounds::map_t * M = tchecker::clockbounds::allocate_map(dim);
  (*M)[0] = 0;
  (*M)[1] = 1;
  (*M)[2] = 1;
  
  tchecker::dbm::db_t universal_Z[offset_dim * offset_dim];
  tchecker::offset_dbm::universal(universal_Z, offset_dim);
  
  SECTION( "Universal zone is included into itself" ) {
    REQUIRE( tchecker::offset_dbm::is_am_le(universal_Z, universal_Z, offset_dim, refcount, refmap, M->ptr()));
  }
  
  SECTION( "Non-empty, non-universal zone is included into universal zone" ) {
    tchecker::dbm::db_t le_1 = tchecker::dbm::db(tchecker::dbm::LE, 1);
    tchecker::dbm::db_t le_m1 = tchecker::dbm::db(tchecker::dbm::LE, -1);
    
    tchecker::dbm::db_t Z1[offset_dim * offset_dim] =
    { tchecker::dbm::LE_ZERO, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, le_m1,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, le_1, tchecker::dbm::LE_ZERO,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, le_m1,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, le_1, tchecker::dbm::LE_ZERO
    };
    
    REQUIRE( tchecker::offset_dbm::is_am_le(Z1, universal_Z, offset_dim, refcount, refmap, M->ptr()));
  }
  
  tchecker::clockbounds::deallocate_map(M);
}



TEST_CASE ("is_am_le offset DBMs, inclusion of the universal zone", "[offset_dbm]" ) {

  tchecker::clock_id_t const dim = 3;
  tchecker::clock_id_t const refcount = 2;
  tchecker::clock_id_t const offset_dim = dim - 1 + refcount;
  tchecker::clock_id_t const refmap[] = {0, 1, 0, 1};
  
  tchecker::clockbounds::map_t * M = tchecker::clockbounds::allocate_map(dim);
  (*M)[0] = 0;
  
  tchecker::dbm::db_t universal_Z[offset_dim * offset_dim];
  tchecker::offset_dbm::universal(universal_Z, offset_dim);
  
  tchecker::dbm::db_t le_1 = tchecker::dbm::db(tchecker::dbm::LE, 1);
  tchecker::dbm::db_t le_m1 = tchecker::dbm::db(tchecker::dbm::LE, -1);
  
  tchecker::dbm::db_t Z1[offset_dim * offset_dim] =
  { tchecker::dbm::LE_ZERO, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, le_m1,
    tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, le_1, tchecker::dbm::LE_ZERO,
    tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, le_m1,
    tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, le_1, tchecker::dbm::LE_ZERO
  };
  
  SECTION( "Universal zone is not contained in Z1 with clock bounds 1,1" ) {
    (*M)[1] = 1;
    (*M)[2] = 1;
    REQUIRE( ! tchecker::offset_dbm::is_am_le(universal_Z, Z1, offset_dim, refcount, refmap, M->ptr()));
  }
  
  SECTION( "Universal zone is contained in Z1 with clock bounds -inf,-inf" ) {
    (*M)[1] = tchecker::clockbounds::NO_BOUND;
    (*M)[2] = tchecker::clockbounds::NO_BOUND;
    REQUIRE( tchecker::offset_dbm::is_am_le(universal_Z, Z1, offset_dim, refcount, refmap, M->ptr()));
  }
  
  tchecker::clockbounds::deallocate_map(M);
}



TEST_CASE ("is_am_le offset DBMs, non trivial zones, non trivial clock bounds", "[offset_dbm]" ) {
  
  tchecker::clock_id_t const dim = 3;
  tchecker::clock_id_t const refcount = 2;
  tchecker::clock_id_t const offset_dim = dim - 1 + refcount;
  tchecker::clock_id_t const refmap[] = {0, 1, 0, 1};
  
  tchecker::clockbounds::map_t * M = tchecker::clockbounds::allocate_map(dim);
  (*M)[0] = 0;
  (*M)[1] = 1;
  (*M)[2] = 1;
  
  tchecker::dbm::db_t le_1 = tchecker::dbm::db(tchecker::dbm::LE, 1);
  tchecker::dbm::db_t le_m1 = tchecker::dbm::db(tchecker::dbm::LE, -1);
  
  SECTION( "First case" ) {
    tchecker::dbm::db_t Z1[offset_dim * offset_dim] =
    { tchecker::dbm::LE_ZERO, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, le_m1,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, le_1, tchecker::dbm::LE_ZERO,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, le_m1,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, le_1, tchecker::dbm::LE_ZERO
    };
    
    tchecker::dbm::db_t Z2[offset_dim * offset_dim] =
    { tchecker::dbm::LE_ZERO, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO
    };
    
    REQUIRE( ! tchecker::offset_dbm::is_am_le(Z1, Z2, offset_dim, refcount, refmap, M->ptr()) );
  }
  
  SECTION( "Second case" ) {
    tchecker::dbm::db_t Z1[offset_dim * offset_dim] =
    { tchecker::dbm::LE_ZERO, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, le_1,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, le_m1, tchecker::dbm::LE_ZERO,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, le_1,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, le_m1, tchecker::dbm::LE_ZERO
    };
    
    tchecker::dbm::db_t Z2[offset_dim * offset_dim] =
    { tchecker::dbm::LE_ZERO, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO,
      tchecker::dbm::LT_INFINITY, tchecker::dbm::LT_INFINITY, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO
    };
    
    REQUIRE( ! tchecker::offset_dbm::is_am_le(Z1, Z2, offset_dim, refcount, refmap, M->ptr()) );
  }
  
  tchecker::clockbounds::deallocate_map(M);
}



TEST_CASE( "universal_positive offset DBMs", "[offset_dbm]" ) {
  
  tchecker::clock_id_t const dim = 7;
  tchecker::clock_id_t const refcount = 3;
  tchecker::clock_id_t const offset_dim = dim - 1 + refcount;
  tchecker::clock_id_t const refmap[] = {0, 1, 2, 0, 0, 1, 1, 1, 2};
  
  SECTION( "universal positive offset DBM, dimension refcount" ) {
    tchecker::dbm::db_t offset_dbm[refcount * refcount];
    tchecker::offset_dbm::universal_positive(offset_dbm, refcount, refcount, refmap);
    REQUIRE(tchecker::offset_dbm::is_universal_positive(offset_dbm, refcount, refcount, refmap));
  }
  
  SECTION( "universal positive offset DBM, dimension > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    REQUIRE(tchecker::offset_dbm::is_universal_positive(offset_dbm, offset_dim, refcount, refmap));
  }
  
}



TEST_CASE( "bound spread offset DBMs", "[offset_dbm]" ) {

  tchecker::clock_id_t const dim = 3;
  tchecker::clock_id_t const refcount = 3;
  tchecker::clock_id_t const offset_dim = dim - 1 + refcount;
  tchecker::clock_id_t const refmap[] = {0, 1, 2, 0, 1};
  tchecker::clock_id_t const r0 = 0;
  tchecker::clock_id_t const r1 = 1;
  tchecker::clock_id_t const r2 = 2;
  tchecker::clock_id_t const x = 3;
  tchecker::clock_id_t const y = 4;
 
  SECTION( "bound spread of universal DBM, dimension refcount" ) {
    tchecker::dbm::db_t offset_dbm[refcount * refcount];
    tchecker::offset_dbm::universal_positive(offset_dbm, refcount, refcount, refmap);
    
    tchecker::integer_t const spread = 2;
    auto res = tchecker::offset_dbm::bound_spread(offset_dbm, refcount, refcount, spread);
    REQUIRE(res == tchecker::dbm::NON_EMPTY);
    REQUIRE(tchecker::offset_dbm::is_spread_bounded(offset_dbm, refcount, refcount, spread));
    
    tchecker::dbm::db_t le_spread = tchecker::dbm::db(tchecker::dbm::LE, spread);
    tchecker::dbm::db_t expected[] =
    { tchecker::dbm::LE_ZERO, le_spread, le_spread,
      le_spread, tchecker::dbm::LE_ZERO, le_spread,
      le_spread, le_spread, tchecker::dbm::LE_ZERO
    };
    
    for (tchecker::clock_id_t r = 0; r < refcount * refcount; ++r)
      REQUIRE(offset_dbm[r] == expected[r]);
  }
  
  SECTION( "bound spread of non-universal DBM, dimension refcount" ) {
    tchecker::dbm::db_t offset_dbm[refcount * refcount];
    tchecker::offset_dbm::universal_positive(offset_dbm, refcount, refcount, refmap);
    offset_dbm[r0*refcount+r1] = tchecker::dbm::db(tchecker::dbm::LE, -1);
    offset_dbm[r2*refcount+r1] = tchecker::dbm::db(tchecker::dbm::LE, 3);
    
    tchecker::integer_t const spread = 1;
    auto res = tchecker::offset_dbm::bound_spread(offset_dbm, refcount, refcount, spread);
    REQUIRE(res == tchecker::dbm::NON_EMPTY);
    REQUIRE(tchecker::offset_dbm::is_spread_bounded(offset_dbm, refcount, refcount, spread));
    
    tchecker::dbm::db_t le_spread = tchecker::dbm::db(tchecker::dbm::LE, spread);
    tchecker::dbm::db_t expected[] =
    { tchecker::dbm::LE_ZERO, tchecker::dbm::db(tchecker::dbm::LE, -1), tchecker::dbm::LE_ZERO,
      le_spread, tchecker::dbm::LE_ZERO, le_spread,
      le_spread, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO
    };
    
    for (tchecker::clock_id_t r = 0; r < refcount * refcount; ++r)
      REQUIRE(offset_dbm[r] == expected[r]);
  }
  
  SECTION( "bound spread makes DBM empty, dimension refcount" ) {
    tchecker::dbm::db_t offset_dbm[refcount * refcount];
    tchecker::offset_dbm::universal_positive(offset_dbm, refcount, refcount, refmap);
    offset_dbm[r0*refcount+r1] = tchecker::dbm::db(tchecker::dbm::LE, -2);
    offset_dbm[r2*refcount+r1] = tchecker::dbm::db(tchecker::dbm::LE, 2);
    
    tchecker::integer_t const spread = 1;
    auto res = tchecker::offset_dbm::bound_spread(offset_dbm, refcount, refcount, spread);
    REQUIRE(res == tchecker::dbm::EMPTY);
  }
  
  SECTION( "bound spread of a DBM, dimension > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    OFFSET_DBM(x,r1) = tchecker::dbm::db(tchecker::dbm::LE, 3);
    OFFSET_DBM(r0,x) = tchecker::dbm::db(tchecker::dbm::LE, 1);
    OFFSET_DBM(x,y) = tchecker::dbm::LE_ZERO;
    OFFSET_DBM(y,x) = tchecker::dbm::LE_ZERO;
    auto tighten = tchecker::offset_dbm::tighten(offset_dbm, offset_dim);
    REQUIRE(tighten == tchecker::dbm::NON_EMPTY);
    
    tchecker::integer_t const spread = 1;
    auto res = tchecker::offset_dbm::bound_spread(offset_dbm, offset_dim, refcount, spread);
    REQUIRE(res == tchecker::dbm::NON_EMPTY);
    REQUIRE(tchecker::offset_dbm::is_spread_bounded(offset_dbm, offset_dim, refcount, spread));
    
    tchecker::dbm::db_t le_1 = tchecker::dbm::db(tchecker::dbm::LE, 1);
    tchecker::dbm::db_t le_3 = tchecker::dbm::db(tchecker::dbm::LE, 3);
    tchecker::dbm::db_t le_4 = tchecker::dbm::db(tchecker::dbm::LE, 4);
    tchecker::dbm::db_t expected[] =
    { tchecker::dbm::LE_ZERO, le_1, le_1, le_1, le_1,
      le_1, tchecker::dbm::LE_ZERO, le_1, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO,
      le_1, le_1, tchecker::dbm::LE_ZERO, le_1, le_1,
      le_4, le_3, le_4, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO,
      le_4, le_3, le_4, tchecker::dbm::LE_ZERO, tchecker::dbm::LE_ZERO
    };
    
    for (tchecker::clock_id_t r = 0; r < offset_dim * offset_dim; ++r)
      REQUIRE(offset_dbm[r] == expected[r]);
  }
  
  SECTION( "bound spread makes DBM empty, dimension > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    OFFSET_DBM(y,r1) = tchecker::dbm::db(tchecker::dbm::LE, 1);
    OFFSET_DBM(x,y) = tchecker::dbm::db(tchecker::dbm::LT, -7);
    auto tighten = tchecker::offset_dbm::tighten(offset_dbm, offset_dim);
    REQUIRE(tighten == tchecker::dbm::NON_EMPTY);
    
    tchecker::integer_t const spread = 4;
    auto res = tchecker::offset_dbm::bound_spread(offset_dbm, offset_dim, refcount, spread);
    REQUIRE(res == tchecker::dbm::EMPTY);
  }
}



TEST_CASE( "synchronize offset DBMs", "[offset_dbm]" ) {
  
  tchecker::clock_id_t const dim = 7;
  tchecker::clock_id_t const refcount = 3;
  tchecker::clock_id_t const offset_dim = dim - 1 + refcount;
  tchecker::clock_id_t const refmap[] = {0, 1, 2, 0, 0, 1, 1, 1, 2};
  
  SECTION( "synchronize offset DBM, dimension refcount" ) {
    tchecker::dbm::db_t offset_dbm[refcount * refcount];
    tchecker::offset_dbm::universal_positive(offset_dbm, refcount, refcount, refmap);
    auto res = tchecker::offset_dbm::synchronize(offset_dbm, refcount, refcount);
    REQUIRE(res == tchecker::dbm::NON_EMPTY);
    REQUIRE(tchecker::offset_dbm::is_synchronized(offset_dbm, refcount, refcount));
  }
  
  SECTION( "synchronize offset DBM yields non-empty synchronized DBM, dimension > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    auto res = tchecker::offset_dbm::synchronize(offset_dbm, offset_dim, refcount);
    REQUIRE(res == tchecker::dbm::NON_EMPTY);
    REQUIRE(tchecker::offset_dbm::is_synchronized(offset_dbm, offset_dim, refcount));
  }
  
  SECTION( "synchronize offset DBM yields empty DBM, dimension > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    OFFSET_DBM(3,0) = tchecker::dbm::LE_ZERO;
    OFFSET_DBM(5,3) = tchecker::dbm::db(tchecker::dbm::LE, -1);
    OFFSET_DBM(1,5) = tchecker::dbm::LE_ZERO;
    tchecker::offset_dbm::tighten(offset_dbm, offset_dim);
    
    auto res = tchecker::offset_dbm::synchronize(offset_dbm, offset_dim, refcount);
    REQUIRE(res == tchecker::dbm::EMPTY);
  }
  
}



TEST_CASE( "reset to reference clocks on offset DBMs", "[offset_dbm]" ) {
  
  tchecker::clock_id_t const dim = 6;
  tchecker::clock_id_t const refcount = 3;
  tchecker::clock_id_t const offset_dim = dim - 1 + refcount;
  tchecker::clock_id_t const refmap[] = {0, 1, 2, 0, 1, 1, 2, 2};
  
  SECTION( "reset 1 clock to reference clock on universal positive offset DBM" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    
    tchecker::clock_id_t const x = 3;
    tchecker::offset_dbm::reset_to_refclock(offset_dbm, offset_dim, x, refcount, refmap);
    
    tchecker::dbm::db_t offset_dbm2[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm2, offset_dim, refcount, refmap);
    OFFSET_DBM2(x, refmap[x]) = tchecker::dbm::LE_ZERO;
    
    REQUIRE(tchecker::dbm::is_equal(offset_dbm, offset_dbm2, offset_dim));
  }
  
  SECTION( "reset 2 clocks to reference clock on universal positive offset DBM" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    
    tchecker::clock_id_t const x = 3;
    tchecker::offset_dbm::reset_to_refclock(offset_dbm, offset_dim, x, refcount, refmap);
    tchecker::clock_id_t const y = 4;
    tchecker::offset_dbm::reset_to_refclock(offset_dbm, offset_dim, y, refcount, refmap);
    tchecker::clock_id_t const z = 5; // same reference clock as y
    
    tchecker::dbm::db_t offset_dbm2[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm2, offset_dim, refcount, refmap);
    OFFSET_DBM2(x, refmap[x]) = tchecker::dbm::LE_ZERO;
    OFFSET_DBM2(y, refmap[y]) = tchecker::dbm::LE_ZERO;
    OFFSET_DBM2(y, z) = tchecker::dbm::LE_ZERO;   // tightening
    
    REQUIRE(tchecker::dbm::is_equal(offset_dbm, offset_dbm2, offset_dim));
  }
  
  SECTION( "reset to reference clock on offset DBM" ) {
    tchecker::clock_id_t const x = 6;
    tchecker::clock_id_t const y = 7;  // same reference clock as x
    
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    OFFSET_DBM(refmap[x], x) = tchecker::dbm::db(tchecker::dbm::LT, -1);
    OFFSET_DBM(x, refmap[x]) = tchecker::dbm::db(tchecker::dbm::LE, 5);
    tchecker::offset_dbm::tighten(offset_dbm, offset_dim);
    
    tchecker::offset_dbm::reset_to_refclock(offset_dbm, offset_dim, x, refcount, refmap);
    
    tchecker::dbm::db_t offset_dbm2[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm2, offset_dim, refcount, refmap);
    OFFSET_DBM2(x, refmap[x]) = tchecker::dbm::LE_ZERO;
    OFFSET_DBM2(x, y) = tchecker::dbm::LE_ZERO;  // tightening
    
    REQUIRE(tchecker::dbm::is_equal(offset_dbm, offset_dbm2, offset_dim));
  }
  
  SECTION( "reset to reference clock on synchronized offset DBM" ) {
    tchecker::clock_id_t const x = 4;
    
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    tchecker::offset_dbm::synchronize(offset_dbm, offset_dim, refcount);
    OFFSET_DBM(refmap[x], x) = tchecker::dbm::db(tchecker::dbm::LT, -1);
    OFFSET_DBM(x, refmap[x]) = tchecker::dbm::db(tchecker::dbm::LE, 5);
    tchecker::offset_dbm::tighten(offset_dbm, offset_dim);
    
    tchecker::offset_dbm::reset_to_refclock(offset_dbm, offset_dim, x, refcount, refmap);
    
    tchecker::dbm::db_t offset_dbm2[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm2, offset_dim, refcount, refmap);
    tchecker::offset_dbm::synchronize(offset_dbm2, offset_dim, refcount);
    for (tchecker::clock_id_t j = 0; j < offset_dim; ++j)
      OFFSET_DBM2(x, j) = tchecker::dbm::LE_ZERO;   // reset of x + tightening
    
    REQUIRE(tchecker::dbm::is_equal(offset_dbm, offset_dbm2, offset_dim));
  }
  
}



TEST_CASE( "asynchronous_open_up on offset DBMs", "[offset_dbm]" ) {
  
  tchecker::clock_id_t const dim = 5;
  tchecker::clock_id_t const refcount = 3;
  tchecker::clock_id_t const offset_dim = dim - 1 + refcount;
  tchecker::clock_id_t const refmap[] = {0, 1, 2, 0, 1, 2, 2};
  
  SECTION( "asynchronous_open_up on universal positive offset DBM, dimension refcount" ) {
    tchecker::dbm::db_t offset_dbm[refcount * refcount];
    tchecker::offset_dbm::universal_positive(offset_dbm, refcount, refcount, refmap);
    
    tchecker::offset_dbm::asynchronous_open_up(offset_dbm, refcount, refcount);
    
    tchecker::dbm::db_t offset_dbm2[refcount * refcount];
    tchecker::offset_dbm::universal_positive(offset_dbm2, refcount, refcount, refmap);
    
    REQUIRE(tchecker::offset_dbm::is_equal(offset_dbm, offset_dbm2, refcount));
  }
  
  SECTION( "asynchronous_open_up on universal positive offset DBM, dimension > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    
    tchecker::offset_dbm::asynchronous_open_up(offset_dbm, offset_dim, refcount);
    
    tchecker::dbm::db_t offset_dbm2[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm2, offset_dim, refcount, refmap);
    
    REQUIRE(tchecker::offset_dbm::is_equal(offset_dbm, offset_dbm2, offset_dim));
  }
  
  SECTION( "asynchronous_open_up on synchronized universal positive offset DBM, dimension refcount" ) {
    tchecker::dbm::db_t offset_dbm[refcount * refcount];
    tchecker::offset_dbm::universal_positive(offset_dbm, refcount, refcount, refmap);
    tchecker::offset_dbm::synchronize(offset_dbm, refcount, refcount);
    
    tchecker::offset_dbm::asynchronous_open_up(offset_dbm, refcount, refcount);
    
    tchecker::dbm::db_t offset_dbm2[refcount * refcount];
    tchecker::offset_dbm::universal_positive(offset_dbm2, refcount, refcount, refmap);
    
    REQUIRE(tchecker::offset_dbm::is_equal(offset_dbm, offset_dbm2, refcount));
  }
  
  SECTION( "asynchronous_open_up on synchronized universal positive offset DBM, dimension > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    tchecker::offset_dbm::synchronize(offset_dbm, offset_dim, refcount);
    
    tchecker::offset_dbm::asynchronous_open_up(offset_dbm, offset_dim, refcount);
    
    tchecker::dbm::db_t offset_dbm2[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm2, offset_dim, refcount, refmap);
    for (tchecker::clock_id_t r = 0; r < refcount; ++r)
      for (tchecker::clock_id_t i = refcount; i < offset_dim; ++i)
        OFFSET_DBM2(r,i) = tchecker::dbm::LE_ZERO;
    
    REQUIRE(tchecker::offset_dbm::is_equal(offset_dbm, offset_dbm2, offset_dim));
  }
  
  SECTION( "asynchronous_open_up on offset DBM, dimension refcount" ) {
    tchecker::dbm::db_t offset_dbm[refcount * refcount];
    offset_dbm[0*refcount+0] = tchecker::dbm::LE_ZERO;
    offset_dbm[0*refcount+1] = tchecker::dbm::LE_ZERO;
    offset_dbm[0*refcount+2] = tchecker::dbm::db(tchecker::dbm::LT, -1);
    offset_dbm[1*refcount+0] = tchecker::dbm::LE_ZERO;
    offset_dbm[1*refcount+1] = tchecker::dbm::LE_ZERO;
    offset_dbm[1*refcount+2] = tchecker::dbm::db(tchecker::dbm::LT, -1);
    offset_dbm[2*refcount+0] = tchecker::dbm::LT_INFINITY;
    offset_dbm[2*refcount+1] = tchecker::dbm::LT_INFINITY;
    offset_dbm[2*refcount+2] = tchecker::dbm::LE_ZERO;
    
    tchecker::offset_dbm::asynchronous_open_up(offset_dbm, refcount, refcount);
    
    tchecker::dbm::db_t offset_dbm2[refcount * refcount];
    tchecker::offset_dbm::universal_positive(offset_dbm2, refcount, refcount, refmap);
    
    REQUIRE(tchecker::offset_dbm::is_equal(offset_dbm, offset_dbm2, refcount));
  }
  
  SECTION( "asynchronous_open_up on offset DBM, dimension > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    OFFSET_DBM(0, 1) = tchecker::dbm::LE_ZERO;
    OFFSET_DBM(1, 0) = tchecker::dbm::LE_ZERO;
    OFFSET_DBM(1, 4) = tchecker::dbm::db(tchecker::dbm::LE, -1);
    OFFSET_DBM(2, 1) = tchecker::dbm::db(tchecker::dbm::LE, 1);
    OFFSET_DBM(4, 1) = tchecker::dbm::db(tchecker::dbm::LE, 2);
    OFFSET_DBM(4, 3) = tchecker::dbm::db(tchecker::dbm::LE, 8);
    OFFSET_DBM(5, 6) = tchecker::dbm::db(tchecker::dbm::LE, -1);
    OFFSET_DBM(6, 5) = tchecker::dbm::db(tchecker::dbm::LE, 3);
    tchecker::offset_dbm::tighten(offset_dbm, offset_dim);
    
    tchecker::dbm::db_t offset_dbm2[offset_dim * offset_dim];
    std::memcpy(offset_dbm2, offset_dbm, offset_dim * offset_dim * sizeof(*offset_dbm2));
    for (tchecker::clock_id_t r = 0; r < refcount; ++r)
      for (tchecker::clock_id_t i = 0; i < offset_dim; ++i)
        OFFSET_DBM2(i,r) = (i == r ? tchecker::dbm::LE_ZERO : tchecker::dbm::LT_INFINITY);
    
    tchecker::offset_dbm::asynchronous_open_up(offset_dbm, offset_dim, refcount);
    
    REQUIRE(tchecker::offset_dbm::is_equal(offset_dbm, offset_dbm2, offset_dim));
  }
  
  SECTION( "asynchronous_open_up on offset DBM, patial delay allowed" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    OFFSET_DBM(0, 1) = tchecker::dbm::LE_ZERO;
    OFFSET_DBM(0, 3) = tchecker::dbm::LE_ZERO;
    OFFSET_DBM(1, 0) = tchecker::dbm::LE_ZERO;
    OFFSET_DBM(1, 4) = tchecker::dbm::db(tchecker::dbm::LE, 1);
    OFFSET_DBM(2, 1) = tchecker::dbm::db(tchecker::dbm::LE, 1);
    OFFSET_DBM(2, 5) = tchecker::dbm::db(tchecker::dbm::LE, 2);
    OFFSET_DBM(4, 1) = tchecker::dbm::db(tchecker::dbm::LE, 2);
    OFFSET_DBM(4, 3) = tchecker::dbm::db(tchecker::dbm::LE, 8);
    OFFSET_DBM(5, 6) = tchecker::dbm::db(tchecker::dbm::LE, -1);
    OFFSET_DBM(6, 5) = tchecker::dbm::db(tchecker::dbm::LE, 3);
    tchecker::offset_dbm::tighten(offset_dbm, offset_dim);
    
    boost::dynamic_bitset<> delay_allowed(refcount, 0);
    delay_allowed[1] = 1;
    
    tchecker::dbm::db_t offset_dbm2[offset_dim * offset_dim];
    std::memcpy(offset_dbm2, offset_dbm, offset_dim * offset_dim * sizeof(*offset_dbm2));
    tchecker::clock_id_t r = 1;
    for (tchecker::clock_id_t i = 0; i < offset_dim; ++i)
      OFFSET_DBM2(i,r) = (i == r ? tchecker::dbm::LE_ZERO : tchecker::dbm::LT_INFINITY);
    
    tchecker::offset_dbm::asynchronous_open_up(offset_dbm, offset_dim, refcount, delay_allowed);
    
    REQUIRE(tchecker::offset_dbm::is_equal(offset_dbm, offset_dbm2, offset_dim));
  }
  
}



TEST_CASE( "to_dbm on offset DBMs", "[offset_dbm]" ) {
  
  tchecker::clock_id_t const dim = 5;
  tchecker::clock_id_t const refcount = 3;
  tchecker::clock_id_t const offset_dim = dim - 1 + refcount;
  tchecker::clock_id_t const refmap[] = {0, 1, 2, 0, 0, 1, 2};
  
  SECTION( "synchronized universal positive offset DBM, dimension refcount" ) {
    tchecker::dbm::db_t offset_dbm[refcount * refcount];
    tchecker::offset_dbm::universal_positive(offset_dbm, refcount, refcount, refmap);
    tchecker::offset_dbm::synchronize(offset_dbm, refcount, refcount);
    
    tchecker::clock_id_t const dim = 1;
    tchecker::dbm::db_t dbm[dim * dim];
    
    tchecker::offset_dbm::to_dbm(offset_dbm, refcount, refcount, refmap, dbm, dim);
    
    REQUIRE(tchecker::dbm::is_universal_positive(dbm, dim));
  }
  
  SECTION( "synchronized universal positive offset DBM, dimension > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    tchecker::offset_dbm::synchronize(offset_dbm, offset_dim, refcount);
    
    tchecker::dbm::db_t dbm[dim * dim];
    
    tchecker::offset_dbm::to_dbm(offset_dbm, offset_dim, refcount, refmap, dbm, dim);
    
    REQUIRE(tchecker::dbm::is_universal_positive(dbm, dim));
  }
  
  SECTION( "synchronized offset DBM, dimension > refcount" ) {
    tchecker::dbm::db_t offset_dbm[offset_dim * offset_dim];
    tchecker::offset_dbm::universal_positive(offset_dbm, offset_dim, refcount, refmap);
    OFFSET_DBM(1, 5) = tchecker::dbm::db(tchecker::dbm::LT, -2);
    OFFSET_DBM(2, 6) = tchecker::dbm::db(tchecker::dbm::LE, -1);
    OFFSET_DBM(3, 4) = tchecker::dbm::db(tchecker::dbm::LT, -1);
    OFFSET_DBM(4, 3) = tchecker::dbm::db(tchecker::dbm::LE, 2);
    OFFSET_DBM(6, 5) = tchecker::dbm::db(tchecker::dbm::LE, 7);
    tchecker::offset_dbm::tighten(offset_dbm, offset_dim);
    tchecker::offset_dbm::synchronize(offset_dbm, offset_dim, refcount);
    
    tchecker::dbm::db_t dbm[dim * dim];
    tchecker::offset_dbm::to_dbm(offset_dbm, offset_dim, refcount, refmap, dbm, dim);
    
    tchecker::dbm::db_t dbm2[dim * dim];
    DBM2(0, 0) = tchecker::dbm::LE_ZERO;
    DBM2(0, 1) = tchecker::dbm::LE_ZERO;
    DBM2(0, 2) = tchecker::dbm::db(tchecker::dbm::LT, -1);
    DBM2(0, 3) = tchecker::dbm::db(tchecker::dbm::LT, -2);
    DBM2(0, 4) = tchecker::dbm::db(tchecker::dbm::LE, -1);
    DBM2(1, 0) = tchecker::dbm::LT_INFINITY;
    DBM2(1, 1) = tchecker::dbm::LE_ZERO;
    DBM2(1, 2) = tchecker::dbm::db(tchecker::dbm::LT, -1);
    DBM2(1, 3) = tchecker::dbm::LT_INFINITY;
    DBM2(1, 4) = tchecker::dbm::LT_INFINITY;
    DBM2(2, 0) = tchecker::dbm::LT_INFINITY;
    DBM2(2, 1) = tchecker::dbm::db(tchecker::dbm::LE, 2);
    DBM2(2, 2) = tchecker::dbm::LE_ZERO;
    DBM2(2, 3) = tchecker::dbm::LT_INFINITY;
    DBM2(2, 4) = tchecker::dbm::LT_INFINITY;
    DBM2(3, 0) = tchecker::dbm::LT_INFINITY;
    DBM2(3, 1) = tchecker::dbm::LT_INFINITY;
    DBM2(3, 2) = tchecker::dbm::LT_INFINITY;
    DBM2(3, 3) = tchecker::dbm::LE_ZERO;
    DBM2(3, 4) = tchecker::dbm::LT_INFINITY;
    DBM2(4, 0) = tchecker::dbm::LT_INFINITY;
    DBM2(4, 1) = tchecker::dbm::LT_INFINITY;
    DBM2(4, 2) = tchecker::dbm::LT_INFINITY;
    DBM2(4, 3) = tchecker::dbm::db(tchecker::dbm::LE, 7);
    DBM2(4, 4) = tchecker::dbm::LE_ZERO;
    
    REQUIRE(tchecker::dbm::is_equal(dbm, dbm2, dim));
  }
  
}
