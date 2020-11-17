/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <catch.hpp>

#include <req_sketch.hpp>

#include <limits>

namespace datasketches {

#ifdef TEST_BINARY_INPUT_PATH
const std::string inputPath = TEST_BINARY_INPUT_PATH;
#else
const std::string inputPath = "test/";
#endif

TEST_CASE("req sketch: empty", "[req_sketch]") {
  req_sketch<float, true> sketch(100);
  REQUIRE(sketch.is_empty());
  REQUIRE_FALSE(sketch.is_estimation_mode());
  REQUIRE(sketch.get_n() == 0);
  REQUIRE(sketch.get_num_retained() == 0);
  REQUIRE(std::isnan(sketch.get_rank(0)));
  REQUIRE(std::isnan(sketch.get_rank(std::numeric_limits<float>::infinity())));
  REQUIRE(std::isnan(sketch.get_min_value()));
  REQUIRE(std::isnan(sketch.get_max_value()));
  REQUIRE(std::isnan(sketch.get_quantile(0)));
  REQUIRE(std::isnan(sketch.get_quantile(0.5)));
  REQUIRE(std::isnan(sketch.get_quantile(1)));
}

TEST_CASE("req sketch: single value", "[req_sketch]") {
  req_sketch<float, true> sketch(100);
  sketch.update(1);
  REQUIRE_FALSE(sketch.is_empty());
  REQUIRE_FALSE(sketch.is_estimation_mode());
  REQUIRE(sketch.get_n() == 1);
  REQUIRE(sketch.get_num_retained() == 1);
  REQUIRE(sketch.get_rank(1) == 0);
  REQUIRE(sketch.get_rank<true>(1) == 1);
  REQUIRE(sketch.get_rank(1.1) == 1);
  REQUIRE(sketch.get_rank(std::numeric_limits<float>::infinity()) == 1);
  REQUIRE(sketch.get_quantile(0) == 1);
  REQUIRE(sketch.get_quantile(0.5) == 1);
  REQUIRE(sketch.get_quantile(1) == 1);
}

TEST_CASE("req sketch: repeated values", "[req_sketch]") {
  req_sketch<float, true> sketch(100);
  sketch.update(1);
  sketch.update(1);
  sketch.update(1);
  sketch.update(2);
  sketch.update(2);
  sketch.update(2);
  REQUIRE_FALSE(sketch.is_empty());
  REQUIRE_FALSE(sketch.is_estimation_mode());
  REQUIRE(sketch.get_n() == 6);
  REQUIRE(sketch.get_num_retained() == 6);
  REQUIRE(sketch.get_rank(1) == 0);
  REQUIRE(sketch.get_rank<true>(1) == 0.5);
  REQUIRE(sketch.get_rank(2) == 0.5);
  REQUIRE(sketch.get_rank<true>(2) == 1);
}

TEST_CASE("req sketch: exact mode", "[req_sketch]") {
  req_sketch<float, true> sketch(100);
  for (size_t i = 0; i < 100; ++i) sketch.update(i);
  REQUIRE_FALSE(sketch.is_empty());
  REQUIRE_FALSE(sketch.is_estimation_mode());
  REQUIRE(sketch.get_n() == 100);
  REQUIRE(sketch.get_num_retained() == 100);

  // like KLL
  REQUIRE(sketch.get_rank(0) == 0);
  REQUIRE(sketch.get_rank(1) == 0.01);
  REQUIRE(sketch.get_rank(50) == 0.5);
  REQUIRE(sketch.get_rank(98) == 0.98);
  REQUIRE(sketch.get_rank(99) == 0.99);

  // inclusive
  REQUIRE(sketch.get_rank<true>(0) == 0.01);
  REQUIRE(sketch.get_rank<true>(1) == 0.02);
  REQUIRE(sketch.get_rank<true>(49) == 0.5);
  REQUIRE(sketch.get_rank<true>(98) == 0.99);
  REQUIRE(sketch.get_rank<true>(99) == 1);

  // like KLL
  REQUIRE(sketch.get_quantile(0) == 0);
  REQUIRE(sketch.get_quantile(0.01) == 1);
  REQUIRE(sketch.get_quantile(0.5) == 50);
  REQUIRE(sketch.get_quantile(.99) == 99);
  REQUIRE(sketch.get_quantile(1) == 99);

  // inclusive
  REQUIRE(sketch.get_quantile<true>(0) == 0);
  REQUIRE(sketch.get_quantile<true>(0.01) == 0);
  REQUIRE(sketch.get_quantile<true>(0.5) == 49);
  REQUIRE(sketch.get_quantile<true>(0.99) == 98);
  REQUIRE(sketch.get_quantile<true>(1) == 99);
}

TEST_CASE("req sketch: estimation mode", "[req_sketch]") {
  req_sketch<float, true> sketch(100);
  const size_t n = 100000;
  for (size_t i = 0; i < n; ++i) sketch.update(i);
  REQUIRE_FALSE(sketch.is_empty());
  REQUIRE(sketch.is_estimation_mode());
  REQUIRE(sketch.get_n() == n);
//  std::cout << sketch.to_string(true);
  REQUIRE(sketch.get_num_retained() < n);
  REQUIRE(sketch.get_rank(0) == 0);
  REQUIRE(sketch.get_rank(n) == 1);
  REQUIRE(sketch.get_rank(n / 2) == Approx(0.5).margin(0.01));
  REQUIRE(sketch.get_rank(n - 1) == Approx(1).margin(0.01));
  REQUIRE(sketch.get_min_value() == 0);
  REQUIRE(sketch.get_max_value() == n - 1);
}

TEST_CASE("req sketch: stream serialize-deserialize empty", "[req_sketch]") {
  req_sketch<float, true> sketch(100);

  std::stringstream s(std::ios::in | std::ios::out | std::ios::binary);
  sketch.serialize(s);
  auto sketch2 = req_sketch<float, true>::deserialize(s);
  REQUIRE(s.tellg() == s.tellp());
  REQUIRE(sketch2.is_empty() == sketch.is_empty());
  REQUIRE(sketch2.is_estimation_mode() == sketch.is_estimation_mode());
  REQUIRE(sketch2.get_num_retained() == sketch.get_num_retained());
  REQUIRE(sketch2.get_n() == sketch.get_n());
  REQUIRE(std::isnan(sketch2.get_min_value()));
  REQUIRE(std::isnan(sketch2.get_max_value()));
}

TEST_CASE("req sketch: byte serialize-deserialize empty", "[req_sketch]") {
  req_sketch<float, true> sketch(100);

  auto bytes = sketch.serialize();
  REQUIRE(bytes.size() == sketch.get_serialized_size_bytes());
  auto sketch2 = req_sketch<float, true>::deserialize(bytes.data(), bytes.size());
  REQUIRE(bytes.size() == sketch2.get_serialized_size_bytes());
  REQUIRE(sketch2.is_empty() == sketch.is_empty());
  REQUIRE(sketch2.is_estimation_mode() == sketch.is_estimation_mode());
  REQUIRE(sketch2.get_num_retained() == sketch.get_num_retained());
  REQUIRE(sketch2.get_n() == sketch.get_n());
  REQUIRE(std::isnan(sketch2.get_min_value()));
  REQUIRE(std::isnan(sketch2.get_max_value()));
}

TEST_CASE("req sketch: stream serialize-deserialize single item", "[req_sketch]") {
  req_sketch<float, true> sketch(100);
  sketch.update(1);

  std::stringstream s(std::ios::in | std::ios::out | std::ios::binary);
  sketch.serialize(s);
  auto sketch2 = req_sketch<float, true>::deserialize(s);
  REQUIRE(s.tellg() == s.tellp());
  REQUIRE(sketch2.is_empty() == sketch.is_empty());
  REQUIRE(sketch2.is_estimation_mode() == sketch.is_estimation_mode());
  REQUIRE(sketch2.get_num_retained() == sketch.get_num_retained());
  REQUIRE(sketch2.get_n() == sketch.get_n());
  REQUIRE(sketch2.get_min_value() == sketch.get_min_value());
  REQUIRE(sketch2.get_max_value() == sketch.get_max_value());
}

TEST_CASE("req sketch: byte serialize-deserialize single item", "[req_sketch]") {
  req_sketch<float, true> sketch(100);
  sketch.update(1);

  auto bytes = sketch.serialize();
  REQUIRE(bytes.size() == sketch.get_serialized_size_bytes());
  auto sketch2 = req_sketch<float, true>::deserialize(bytes.data(), bytes.size());
  REQUIRE(bytes.size() == sketch2.get_serialized_size_bytes());
  REQUIRE(sketch2.is_empty() == sketch.is_empty());
  REQUIRE(sketch2.is_estimation_mode() == sketch.is_estimation_mode());
  REQUIRE(sketch2.get_num_retained() == sketch.get_num_retained());
  REQUIRE(sketch2.get_n() == sketch.get_n());
  REQUIRE(sketch2.get_min_value() == sketch.get_min_value());
  REQUIRE(sketch2.get_max_value() == sketch.get_max_value());
}

TEST_CASE("req sketch: stream serialize-deserialize exact mode", "[req_sketch]") {
  req_sketch<float, true> sketch(100);
  const size_t n = 50;
  for (size_t i = 0; i < n; ++i) sketch.update(i);
  REQUIRE_FALSE(sketch.is_estimation_mode());

  std::stringstream s(std::ios::in | std::ios::out | std::ios::binary);
  sketch.serialize(s);
  auto sketch2 = req_sketch<float, true>::deserialize(s);
  REQUIRE(s.tellg() == s.tellp());
  REQUIRE(sketch2.is_empty() == sketch.is_empty());
  REQUIRE(sketch2.is_estimation_mode() == sketch.is_estimation_mode());
  REQUIRE(sketch2.get_num_retained() == sketch.get_num_retained());
  REQUIRE(sketch2.get_n() == sketch.get_n());
  REQUIRE(sketch2.get_min_value() == sketch.get_min_value());
  REQUIRE(sketch2.get_max_value() == sketch.get_max_value());
}

TEST_CASE("req sketch: byte serialize-deserialize exact mode", "[req_sketch]") {
  req_sketch<float, true> sketch(100);
  const size_t n = 50;
  for (size_t i = 0; i < n; ++i) sketch.update(i);
  REQUIRE_FALSE(sketch.is_estimation_mode());

  auto bytes = sketch.serialize();
  REQUIRE(bytes.size() == sketch.get_serialized_size_bytes());
  auto sketch2 = req_sketch<float, true>::deserialize(bytes.data(), bytes.size());
  REQUIRE(bytes.size() == sketch2.get_serialized_size_bytes());
  REQUIRE(sketch2.is_empty() == sketch.is_empty());
  REQUIRE(sketch2.is_estimation_mode() == sketch.is_estimation_mode());
  REQUIRE(sketch2.get_num_retained() == sketch.get_num_retained());
  REQUIRE(sketch2.get_n() == sketch.get_n());
  REQUIRE(sketch2.get_min_value() == sketch.get_min_value());
  REQUIRE(sketch2.get_max_value() == sketch.get_max_value());
}

TEST_CASE("req sketch: stream serialize-deserialize estimation mode", "[req_sketch]") {
  req_sketch<float, true> sketch(100);
  const size_t n = 100000;
  for (size_t i = 0; i < n; ++i) sketch.update(i);
  REQUIRE(sketch.is_estimation_mode());

  std::stringstream s(std::ios::in | std::ios::out | std::ios::binary);
  sketch.serialize(s);
  auto sketch2 = req_sketch<float, true>::deserialize(s);
  REQUIRE(s.tellg() == s.tellp());
  REQUIRE(sketch2.is_empty() == sketch.is_empty());
  REQUIRE(sketch2.is_estimation_mode() == sketch.is_estimation_mode());
  REQUIRE(sketch2.get_num_retained() == sketch.get_num_retained());
  REQUIRE(sketch2.get_n() == sketch.get_n());
  REQUIRE(sketch2.get_min_value() == sketch.get_min_value());
  REQUIRE(sketch2.get_max_value() == sketch.get_max_value());
}

TEST_CASE("req sketch: byte serialize-deserialize estimation mode", "[req_sketch]") {
  req_sketch<float, true> sketch(100);
  const size_t n = 100000;
  for (size_t i = 0; i < n; ++i) sketch.update(i);
  REQUIRE(sketch.is_estimation_mode());

  auto bytes = sketch.serialize();
  REQUIRE(bytes.size() == sketch.get_serialized_size_bytes());
  auto sketch2 = req_sketch<float, true>::deserialize(bytes.data(), bytes.size());
  REQUIRE(bytes.size() == sketch2.get_serialized_size_bytes());
  REQUIRE(sketch2.is_empty() == sketch.is_empty());
  REQUIRE(sketch2.is_estimation_mode() == sketch.is_estimation_mode());
  REQUIRE(sketch2.get_num_retained() == sketch.get_num_retained());
  REQUIRE(sketch2.get_n() == sketch.get_n());
  REQUIRE(sketch2.get_min_value() == sketch.get_min_value());
  REQUIRE(sketch2.get_max_value() == sketch.get_max_value());
}

TEST_CASE("req sketch: merge", "[req_sketch]") {
  req_sketch<float, true> sketch1(100);
  for (size_t i = 0; i < 1000; ++i) sketch1.update(i);

  req_sketch<float, true> sketch2(100);
  for (size_t i = 1000; i < 2000; ++i) sketch2.update(i);

  sketch1.merge(sketch2);
  //std::cout << sketch1.to_string(true, true);
  REQUIRE(sketch1.get_min_value() == 0);
  REQUIRE(sketch1.get_max_value() == 1999);
  //REQUIRE(sketch1.get_quantile(0.5) == 1000);
  REQUIRE(sketch1.get_rank(1000) == Approx(0.5).margin(0.01));
}

} /* namespace datasketches */
