/*
 *  Copyright (c) 2017-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include <folly/dynamic.h>

#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/routes/AllMajorityRoute.h"
#include "mcrouter/lib/routes/NullRoute.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeAllMajorityRoute(
    std::vector<typename RouterInfo::RouteHandlePtr> rh) {
  if (rh.empty()) {
    return createNullRoute<typename RouterInfo::RouteHandleIf>();
  }

  if (rh.size() == 1) {
    return std::move(rh[0]);
  }

  return makeRouteHandle<typename RouterInfo::RouteHandleIf, AllMajorityRoute>(
      std::move(rh));
}

} // detail

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeAllMajorityRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  std::vector<typename RouterInfo::RouteHandlePtr> children;
  if (json.isObject()) {
    if (auto jchildren = json.get_ptr("children")) {
      children = factory.createList(*jchildren);
    }
  } else {
    children = factory.createList(json);
  }
  return detail::makeAllMajorityRoute<RouterInfo>(std::move(children));
};
} // mcrouter
} // memcache
} // facebook
