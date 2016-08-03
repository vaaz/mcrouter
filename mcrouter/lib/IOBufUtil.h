/*
 *  Copyright (c) 2016, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include <memory>
#include <string>

#include <folly/io/IOBuf.h>

namespace folly {
class IOBuf;
template <class T> class Range;
using StringPiece = Range<const char*>;
} // folly

namespace facebook { namespace memcache {

folly::StringPiece getRange(const std::unique_ptr<folly::IOBuf>& buf);
folly::StringPiece getRange(const folly::IOBuf& buf);

folly::StringPiece coalesceAndGetRange(std::unique_ptr<folly::IOBuf>& buf);
folly::StringPiece coalesceAndGetRange(folly::IOBuf& buf);

bool hasSameMemoryRegion(const folly::IOBuf& buf,
                         folly::StringPiece range);

bool hasSameMemoryRegion(const folly::IOBuf& a, const folly::IOBuf& b);

void copyInto(char* raw, const folly::IOBuf& buf);

template <typename InputIterator>
folly::IOBuf concatAll(InputIterator begin, InputIterator end) {
  folly::IOBuf out;
  if (begin == end) {
    return out;
  }

  (*begin)->cloneInto(out);
  ++begin;
  while (begin != end) {
    out.prependChain(std::move((*begin)->clone()));
    ++begin;
  }

  return out;
}

/**
 * Given a coalesced IOBuf and a range of bytes [begin, begin + size) inside it,
 * clones into out IOBuf so that cloned.data() == begin and
 * cloned.length() == size.
 * @return false If the range is invalid.
 */
inline bool cloneInto(folly::IOBuf& out, const folly::IOBuf& source,
                      const uint8_t* begin, size_t size) {
  if (!(begin >= source.data() &&
        begin + size <= source.data() + source.length())) {
    return false;
  }
  source.cloneOneInto(out);
  out.trimStart(begin - out.data());
  out.trimEnd(out.length() - size);
  return true;
}

/**
 * Given a coalesced IOBuf and a range of bytes [begin, begin + size) inside it,
 * copies range as a string str such that str[i] == *(char*)(begin+i)
 * for i = 0..size-1.
 *
 * Required that size > 0
 *
 * @return  On success, a string copy of input byte range.
 *          On failure, empty string.
 */
inline std::string copyAsString(const folly::IOBuf& source,
                                const uint8_t* begin,
                                size_t size) {
  assert(size > 0);

  std::string ret;

  if (!(begin >= source.data() &&
        begin + size <= source.data() + source.length())) {
    return ret;
  }

  ret.reserve(size);
  for (size_t i = 0; i < size; ++i) {
    ret.push_back(*reinterpret_cast<const char*>(begin + i));
  }
  return ret;
}

/**
 * If there is only one iovec then just creating it using IOBuf constructor.
 * Otherwise using coalesceSlow.
 */
folly::IOBuf
coalesceIovecs(const struct iovec* iov, size_t iovcnt, size_t destCapacity);

}} // facebook::memcache
