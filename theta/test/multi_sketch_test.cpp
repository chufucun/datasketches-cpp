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

#include <fstream>
#include <sstream>

#include <catch.hpp>
#include <theta_sketch.hpp>
#include <cpc_sketch.hpp>

namespace datasketches {

#ifdef TEST_BINARY_INPUT_PATH
const std::string inputPath = TEST_BINARY_INPUT_PATH;
#else
const std::string inputPath = "test/";
#endif

TEST_CASE("theta sketch: empty", "[theta_sketch]") {
  update_theta_sketch update_sketch = update_theta_sketch::builder().build();
  REQUIRE(update_sketch.is_empty());
  REQUIRE_FALSE(update_sketch.is_estimation_mode());
  REQUIRE(update_sketch.get_theta() == 1.0);
  REQUIRE(update_sketch.get_estimate() == 0.0);
  REQUIRE(update_sketch.get_lower_bound(1) == 0.0);
  REQUIRE(update_sketch.get_upper_bound(1) == 0.0);

  compact_theta_sketch compact_sketch = update_sketch.compact();
  REQUIRE(compact_sketch.is_empty());
  REQUIRE_FALSE(compact_sketch.is_estimation_mode());
  REQUIRE(compact_sketch.get_theta() == 1.0);
  REQUIRE(compact_sketch.get_estimate() == 0.0);
  REQUIRE(compact_sketch.get_lower_bound(1) == 0.0);
  REQUIRE(compact_sketch.get_upper_bound(1) == 0.0);
}

} /* namespace datasketches */
