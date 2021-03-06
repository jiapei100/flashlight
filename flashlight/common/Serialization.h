/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
/**
 * @file common/Serialization.h
 *
 * Serialization support using the Cereal library. Provides serialization
 * functions for `af::array` and `af::dim4` and convenience utilities.
 */

#pragma once

#include <fstream>
#include <iostream>
#include <type_traits>

#include <cereal/access.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>

namespace fl {

/**
 * Save (serialize) the specified args to a binary file (via Cereal).
 * @param filepath the file path to save to
 * @param args the objects to save (e.g. shared_ptr to Module)
 */
template <typename... Args>
void save(const std::string& filepath, const Args&... args);

/**
 * Save (serialize) the specified args to a binary file (via Cereal).
 * @param ostr output stream
 * @param args the objects to save (e.g. shared_ptr to Module)
 */
template <typename... Args>
void save(std::ostream& ostr, const Args&... args);

/**
 * Load (deserialize) the specified args from a binary file (via Cereal).
 * @param filepath the file path to load from
 * @param args the objects to load (expects default-constructed)
 */
template <typename... Args>
void load(const std::string& filepath, Args&... args);

/**
 * Load (deserialize) the specified args from a binary file (via Cereal).
 * @param istr input stream
 * @param args the objects to load (expects default-constructed)
 */
template <typename... Args>
void load(std::istream& istr, Args&... args);
} // namespace fl

/**
 * Note the following guidelines for serialization:
 * - By default you should use save/load pairs and explicit versioning.
 *   The provided macros encourage this usage.
 * - Saving an object must not mutate it. `save()` being `const` helps, but
 *   be careful about `Variable` which is a `shared_ptr` to non-const.
 * - Loading an object must provide the basic exception guarantee. After an
 *   exception, the object must be safe to destroy and no leaks can occur.
 * - For simplicity, `load()` may assume that the initial state of the
 *   object is default-constructed. Conversely, one must only call `load()`
 *   on a default-constructed object.
 *
 */

/**
 * Convenience macro for specifying a simple serialization method for Cereal.
 * Serializes the specified arguments, usually class members. Should be placed
 * in the `private` section of a class. For polymorphic classes, use
 * `FL_SAVE_LOAD_WITH_BASE` instead.
 *
 * Supports the common case when one adds fields to a class, which should be
 * conditionally loaded for newer file versions. See `fl::versioned()`.
 */
#define FL_SAVE_LOAD(...)                                   \
  friend class cereal::access;                              \
  template <class Archive>                                  \
  void save(Archive& ar, const uint32_t version) const {    \
    ::fl::detail::applyArchive(ar, version, ##__VA_ARGS__); \
  }                                                         \
  template <class Archive>                                  \
  void load(Archive& ar, const uint32_t version) {          \
    ::fl::detail::applyArchive(ar, version, ##__VA_ARGS__); \
  }

/**
 * Like `FL_SAVE_LOAD`, but also serializes the base class, which must
 * be specified as the first argument.
 *
 * NB: You do not need to use `CEREAL_REGISTER_POLYMORPHIC_RELATION` if you are
 * using this macro. However you will still need `CEREAL_REGISTER_TYPE`.
 */
#define FL_SAVE_LOAD_WITH_BASE(Base, ...) \
  FL_SAVE_LOAD(cereal::base_class<Base>(this), ##__VA_ARGS__)

/**
 * Declaration-only. Intended to reduce clutter in class definitions.
 * This macro should be placed in the `private` section of a class.
 * The method must be defined later (outside the class).
 * Do not use this macro if you want your class to be unversioned.
 */
#define FL_SAVE_LOAD_DECLARE()                          \
  friend class cereal::access;                          \
  template <class Archive>                              \
  void save(Archive& ar, const uint32_t version) const; \
  template <class Archive>                              \
  void load(Archive& ar, const uint32_t version);

namespace fl {
namespace detail {

template <typename T>
struct Versioned;

struct AfArraySerializeProxy;

} // namespace detail

/**
 * Serialize an expression iff the version is in a certain range.
 * Only intended to wrap arguments of FL_SAVE_LOAD* macros.
 *
 * Example: if we have field `x` in version 0, and add field `y` in version 1,
 * we might write: `FL_SAVE_LOAD(x, fl::versioned(y, 1))`.
 */
template <typename T>
detail::Versioned<T>
versioned(T&& t, uint32_t minVersion, uint32_t maxVersion = UINT32_MAX);

} // namespace fl

namespace cereal {

template <class Archive>
void serialize(Archive& ar, af::dim4& dims);

template <class Archive>
void save(
    Archive& ar,
    const fl::detail::AfArraySerializeProxy& proxy,
    const uint32_t /* version */);

template <class Archive>
void load(Archive& ar, af::array& arr, const uint32_t /* version */);

} // namespace cereal

#include "flashlight/common/Serialization-inl.h"
