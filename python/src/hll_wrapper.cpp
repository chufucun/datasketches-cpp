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

#include "hll.hpp"
#include <boost/python.hpp>
#include <memory>

namespace bpy = boost::python;

namespace datasketches {
namespace python {

HllSketch<> HllSketch_deserialize(bpy::object obj) {
  PyObject* skBytes = obj.ptr();
  if (!PyBytes_Check(skBytes)) {
    PyErr_SetString(PyExc_TypeError, "Attmpted to deserialize non-bytes object");
    bpy::throw_error_already_set();
  }
  
  size_t len = PyBytes_GET_SIZE(skBytes);
  char* sketchImg = PyBytes_AS_STRING(skBytes);
  HllSketch<> sk = HllSketch<>::deserialize(sketchImg, len);
  return sk;
}

bpy::object HllSketch_serializeCompact(const HllSketch<>& sk) {
  std::pair<byte_ptr_with_deleter, const size_t> serResult = sk.serializeCompact();
  PyObject* sketchBytes = PyBytes_FromStringAndSize((char*)serResult.first.get(), serResult.second);
  return bpy::object{bpy::handle<>(sketchBytes)};
}


bpy::object HllSketch_serializeUpdatable(const HllSketch<>& sk) {
  // TODO: can we just releast the smart pointer?
  std::pair<byte_ptr_with_deleter, const size_t> serResult = sk.serializeUpdatable();
  PyObject* sketchBytes = PyBytes_FromStringAndSize((char*)serResult.first.get(), serResult.second);
  return bpy::object{bpy::handle<>(sketchBytes)};
}

std::string HllSketch_toString(const HllSketch<>& sk,
                               bool summary = true,
                               bool detail = false,
                               bool auxDetail = false,
                               bool all = false) {
  return sk.to_string(summary, detail, auxDetail, all);
}

std::string HllSketch_toStringDefault(const HllSketch<>& sk) {
  return HllSketch_toString(sk);
}

HllUnion<> HllUnion_deserialize(bpy::object obj) {
  PyObject* skBytes = obj.ptr();
  if (!PyBytes_Check(skBytes)) {
    PyErr_SetString(PyExc_TypeError, "Attmpted to deserialize non-bytes object");
    bpy::throw_error_already_set();
  }
  
  size_t len = PyBytes_GET_SIZE(skBytes);
  char* sketchImg = PyBytes_AS_STRING(skBytes);
  HllUnion<> u = HllUnion<>::deserialize(sketchImg, len);
  return u;
}

bpy::object HllUnion_serializeCompact(const HllUnion<>& u) {
  std::pair<byte_ptr_with_deleter, const size_t> serResult = u.serializeCompact();
  PyObject* unionBytes = PyBytes_FromStringAndSize((char*)serResult.first.get(), serResult.second);
  return bpy::object{bpy::handle<>(unionBytes)};
}

bpy::object HllUnion_serializeUpdatable(const HllUnion<>& u) {
  std::pair<byte_ptr_with_deleter, const size_t> serResult = u.serializeUpdatable();
  PyObject* unionBytes = PyBytes_FromStringAndSize((char*)serResult.first.get(), serResult.second);
  return bpy::object{bpy::handle<>(unionBytes)};
}

std::string HllUnion_toString(const HllUnion<>& u,
                              bool summary = true,
                              bool detail = false,
                              bool auxDetail = false,
                              bool all = false) {
  return u.to_string(summary, detail, auxDetail, all);
}

std::string HllUnion_toStringDefault(const HllUnion<>& u) {
  return HllUnion_toString(u);
}

HllSketch<> HllUnion_getResult(const HllUnion<>& u,
                                TgtHllType tgtHllType = HLL_4) {
  return std::move(u.getResult(tgtHllType));
}

}
}

namespace dspy = datasketches::python;

BOOST_PYTHON_FUNCTION_OVERLOADS(HllSketchToStringOverloads, dspy::HllSketch_toString, 1, 5);

BOOST_PYTHON_FUNCTION_OVERLOADS(HllUnionToStringOverloads, dspy::HllUnion_toString, 1, 5);
BOOST_PYTHON_FUNCTION_OVERLOADS(HllUnionGetResultOverloads, dspy::HllUnion_getResult, 1, 2);

void export_hll()
{
  using namespace datasketches;

  bpy::enum_<TgtHllType>("TgtHllType")
    .value("HLL_4", HLL_4)
    .value("HLL_6", HLL_6)
    .value("HLL_8", HLL_8)
    ;

  bpy::class_<HllSketch<>, boost::noncopyable>("HllSketch", bpy::init<int>())
    .def(bpy::init<int, TgtHllType>())
    .def(bpy::init<int, TgtHllType, bool>())
    //.def("deserialize", &dspy::HllSketch_deserialize, bpy::return_value_policy<bpy::manage_new_object>())
    .def("deserialize", &dspy::HllSketch_deserialize)
    .staticmethod("deserialize")
    .def("serializeCompact", &dspy::HllSketch_serializeCompact)
    .def("serializeUpdatable", &dspy::HllSketch_serializeUpdatable)
    .def("__str__", &dspy::HllSketch_toStringDefault)
    .add_property("lgConfigK", &HllSketch<>::getLgConfigK)
    .add_property("tgtHllType", &HllSketch<>::getTgtHllType)
    .def("toString", &dspy::HllSketch_toString, HllSketchToStringOverloads())
    .def("getEstimate", &HllSketch<>::getEstimate)
    .def("getCompositeEstimate", &HllSketch<>::getCompositeEstimate)
    .def("getLowerBound", &HllSketch<>::getLowerBound)
    .def("getUpperBound", &HllSketch<>::getUpperBound)
    .def("isCompact", &HllSketch<>::isCompact)
    .def("isEmpty", &HllSketch<>::isEmpty)
    .def("getUpdatableSerializationBytes", &HllSketch<>::getUpdatableSerializationBytes)
    .def("getCompactSerializationBytes", &HllSketch<>::getCompactSerializationBytes)
    .def("reset", &HllSketch<>::reset)
    .def<void (HllSketch<>::*)(uint64_t)>("update", &HllSketch<>::update)
    .def<void (HllSketch<>::*)(int64_t)>("update", &HllSketch<>::update)
    .def<void (HllSketch<>::*)(double)>("update", &HllSketch<>::update)
    .def<void (HllSketch<>::*)(const std::string&)>("update", &HllSketch<>::update)
    .def("getMaxUpdatableSerializationBytes", &HllSketch<>::getMaxUpdatableSerializationBytes)
    .staticmethod("getMaxUpdatableSerializationBytes")
    .def("getRelErr", &HllSketch<>::getRelErr)
    .staticmethod("getRelErr")
    ;

  bpy::class_<HllUnion<>, boost::noncopyable>("HllUnion", bpy::init<int>())
    //.def("deserialize", &dspy::HllUnion_deserialize, bpy::return_value_policy<bpy::manage_new_object>())
    .def("deserialize", &dspy::HllUnion_deserialize)
    .staticmethod("deserialize")
    .def("serializeCompact", &dspy::HllUnion_serializeCompact)
    .def("serializeUpdatable", &dspy::HllUnion_serializeUpdatable)
    .def("__str__", &dspy::HllUnion_toStringDefault)
    .add_property("lgConfigK", &HllUnion<>::getLgConfigK)
    .add_property("tgtHllType", &HllUnion<>::getTgtHllType)
    .def("toString", &dspy::HllUnion_toString, HllUnionToStringOverloads())
    .def("getEstimate", &HllUnion<>::getEstimate)
    .def("getCompositeEstimate", &HllUnion<>::getCompositeEstimate)
    .def("getLowerBound", &HllUnion<>::getLowerBound)
    .def("getUpperBound", &HllUnion<>::getUpperBound)
    .def("isCompact", &HllUnion<>::isCompact)
    .def("isEmpty", &HllUnion<>::isEmpty)
    .def("getUpdatableSerializationBytes", &HllUnion<>::getUpdatableSerializationBytes)
    .def("getCompactSerializationBytes", &HllUnion<>::getCompactSerializationBytes)
    .def("reset", &HllUnion<>::reset)
    //.def("getResult", &dspy::HllUnion_getResult, HllUnionGetResultOverloads()[bpy::return_value_policy<bpy::manage_new_object>()])
    .def("getResult", &dspy::HllUnion_getResult, HllUnionGetResultOverloads())
    .def<void (HllUnion<>::*)(const HllSketch<>&)>("update", &HllUnion<>::update)
    .def<void (HllUnion<>::*)(uint64_t)>("update", &HllUnion<>::update)
    .def<void (HllUnion<>::*)(int64_t)>("update", &HllUnion<>::update)
    .def<void (HllUnion<>::*)(double)>("update", &HllUnion<>::update)
    .def<void (HllUnion<>::*)(const std::string&)>("update", &HllUnion<>::update)
    //.def<void (HllUnion::*)(const void*, size_t)>("update", &HllUnion::update)
    .def("getMaxSerializationBytes", &HllUnion<>::getMaxSerializationBytes)
    .staticmethod("getMaxSerializationBytes")
    .def("getRelErr", &HllUnion<>::getRelErr)
    .staticmethod("getRelErr")
    ;
}