/**
 * @file init_rules_test.cpp
 * @author Marcus Edel
 *
 * Tests for the various weight initialize methods.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#include <mlpack/core.hpp>
#include <mlpack/core/math/random.hpp>

#include <mlpack/methods/ann/init_rules/kathirvalavakumar_subavathi_init.hpp>
#include <mlpack/methods/ann/init_rules/nguyen_widrow_init.hpp>
#include <mlpack/methods/ann/init_rules/oivs_init.hpp>
#include <mlpack/methods/ann/init_rules/orthogonal_init.hpp>
#include <mlpack/methods/ann/init_rules/random_init.hpp>
#include <mlpack/methods/ann/init_rules/zero_init.hpp>
#include <mlpack/methods/ann/init_rules/gaussian_init.hpp>


#include <boost/test/unit_test.hpp>
#include "test_tools.hpp"

using namespace mlpack;
using namespace mlpack::ann;

BOOST_AUTO_TEST_SUITE(InitRulesTest);

// Test the RandomInitialization class with a constant value.
BOOST_AUTO_TEST_CASE(ConstantInitTest)
{
  arma::mat weights;
  RandomInitialization constantInit(1, 1);
  constantInit.Initialize(weights, 100, 100);

  bool b = arma::all(arma::vectorise(weights) == 1);
  BOOST_REQUIRE_EQUAL(b, 1);
}

// Test the OrthogonalInitialization class.
BOOST_AUTO_TEST_CASE(OrthogonalInitTest)
{
  arma::mat weights;
  OrthogonalInitialization orthogonalInit;
  orthogonalInit.Initialize(weights, 100, 200);

  arma::mat orthogonalWeights = arma::eye<arma::mat>(100, 100);
  weights *= weights.t();

  for (size_t i = 0; i < weights.n_rows; i++)
    for (size_t j = 0; j < weights.n_cols; j++)
      BOOST_REQUIRE_SMALL(weights.at(i, j) - orthogonalWeights.at(i, j), 1e-3);

  orthogonalInit.Initialize(weights, 200, 100);
  weights = weights.t() * weights;

  for (size_t i = 0; i < weights.n_rows; i++)
    for (size_t j = 0; j < weights.n_cols; j++)
      BOOST_REQUIRE_SMALL(weights.at(i, j) - orthogonalWeights.at(i, j), 1e-3);
}

// Test the OrthogonalInitialization class with a non default gain.
BOOST_AUTO_TEST_CASE(OrthogonalInitGainTest)
{
  arma::mat weights;

  const double gain = 2;
  OrthogonalInitialization orthogonalInit(gain);
  orthogonalInit.Initialize(weights, 100, 200);

  arma::mat orthogonalWeights = arma::eye<arma::mat>(100, 100);
  orthogonalWeights *= (gain * gain);
  weights *= weights.t();

  for (size_t i = 0; i < weights.n_rows; i++)
    for (size_t j = 0; j < weights.n_cols; j++)
      BOOST_REQUIRE_SMALL(weights.at(i, j) - orthogonalWeights.at(i, j), 1e-3);
}

// Test the ZeroInitialization class. If you think about it, it's kind of
// ridiculous to test the zero init rule. But at least we make sure it
// builds without any problems.
BOOST_AUTO_TEST_CASE(ZeroInitTest)
{
  arma::mat weights;
  ZeroInitialization zeroInit;
  zeroInit.Initialize(weights, 100, 100);

  bool b = arma::all(arma::vectorise(weights) == 0);
  BOOST_REQUIRE_EQUAL(b, 1);
}

// Test the KathirvalavakumarSubavathiInitialization class.
BOOST_AUTO_TEST_CASE(KathirvalavakumarSubavathiInitTest)
{
  arma::mat data = arma::randu<arma::mat>(100, 1);

  arma::mat weights;
  arma::cube weights3d;

  KathirvalavakumarSubavathiInitialization kathirvalavakumarSubavathiInit(
      data, 1.5);

  kathirvalavakumarSubavathiInit.Initialize(weights, 100, 100);
  kathirvalavakumarSubavathiInit.Initialize(weights3d, 100, 100, 2);

  BOOST_REQUIRE_EQUAL(weights.n_rows, 100);
  BOOST_REQUIRE_EQUAL(weights.n_cols, 100);

  BOOST_REQUIRE_EQUAL(weights3d.n_rows, 100);
  BOOST_REQUIRE_EQUAL(weights3d.n_cols, 100);
  BOOST_REQUIRE_EQUAL(weights3d.n_slices, 2);
}

// Test the NguyenWidrowInitialization class.
BOOST_AUTO_TEST_CASE(NguyenWidrowInitTest)
{
  arma::mat weights;
  arma::cube weights3d;

  NguyenWidrowInitialization nguyenWidrowInit;

  nguyenWidrowInit.Initialize(weights, 100, 100);
  nguyenWidrowInit.Initialize(weights3d, 100, 100, 2);

  BOOST_REQUIRE_EQUAL(weights.n_rows, 100);
  BOOST_REQUIRE_EQUAL(weights.n_cols, 100);

  BOOST_REQUIRE_EQUAL(weights3d.n_rows, 100);
  BOOST_REQUIRE_EQUAL(weights3d.n_cols, 100);
  BOOST_REQUIRE_EQUAL(weights3d.n_slices, 2);
}

// Test the OivsInitialization class.
BOOST_AUTO_TEST_CASE(OivsInitTest)
{
  arma::mat weights;
  arma::cube weights3d;

  OivsInitialization<> oivsInit;

  oivsInit.Initialize(weights, 100, 100);
  oivsInit.Initialize(weights3d, 100, 100, 2);

  BOOST_REQUIRE_EQUAL(weights.n_rows, 100);
  BOOST_REQUIRE_EQUAL(weights.n_cols, 100);

  BOOST_REQUIRE_EQUAL(weights3d.n_rows, 100);
  BOOST_REQUIRE_EQUAL(weights3d.n_cols, 100);
  BOOST_REQUIRE_EQUAL(weights3d.n_slices, 2);
}

// Test the GaussianInitialization class.
BOOST_AUTO_TEST_CASE(GaussianInitTest)
{
  const size_t rows = 7;
  const size_t cols = 8;
  const size_t slices = 2;

  arma::mat weights;
  arma::cube weights3d;

  GaussianInitialization t(0, 0.2);

  t.Initialize(weights, rows, cols);
  t.Initialize(weights3d, rows, cols, slices);

  BOOST_REQUIRE_EQUAL(weights.n_rows, rows);
  BOOST_REQUIRE_EQUAL(weights.n_cols, cols);

  BOOST_REQUIRE_EQUAL(weights3d.n_rows, rows);
  BOOST_REQUIRE_EQUAL(weights3d.n_cols, cols);
  BOOST_REQUIRE_EQUAL(weights3d.n_slices, slices);
}

BOOST_AUTO_TEST_SUITE_END();
