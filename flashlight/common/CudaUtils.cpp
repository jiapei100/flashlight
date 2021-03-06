/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "flashlight/common/CudaUtils.h"

#include <af/device.h>

namespace fl {
namespace cuda {

cudaStream_t getActiveStream() {
  auto af_id = af::getDevice();
  return afcu::getStream(af_id);
}

} // namespace cuda
} // namespace fl
