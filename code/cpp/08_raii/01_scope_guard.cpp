/* SPDX-FileCopyrightText: 2026 HUIMY */
/* SPDX-License-Identifier: MIT */
/* LAB 08-01  raii/01_scope_guard.cpp
 * Drills: a class that owns, the move that transfers ownership, a custom
 *         deleter, and a guard object that releases on every exit path.
 * Build:  cmake --build build --target cpp_08_raii
 */
#include "craft_assert.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <utility>

/* Owns an array; copy is deleted, so there is exactly one owner at a time. */
class Buffer {
public:
  explicit Buffer(std::size_t n) : data_(new int[n]), size_(n) {
    for (std::size_t i = 0; i < n; ++i) {
      data_[i] = static_cast<int>(i);
    }
  }
  ~Buffer() {
    delete[] data_;
    std::cout << "Buffer destroyed\n";
  }
  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;

  /* Move transfers the pointer and leaves the source destructible. */
  Buffer(Buffer &&other) noexcept : data_(other.data_), size_(other.size_) {
    other.data_ = nullptr;
    other.size_ = 0;
    std::cout << "Buffer moved\n";
  }
  Buffer &operator=(Buffer &&other) noexcept {
    if (this != &other) {
      delete[] data_;
      data_ = other.data_;
      size_ = other.size_;
      other.data_ = nullptr;
      other.size_ = 0;
    }
    return *this;
  }

  int at(std::size_t i) const { return data_[i]; }
  std::size_t size() const { return size_; }

private:
  int *data_;
  std::size_t size_;
};

/* A guard that runs one function on scope exit. The destructor is the only
 * place that has to be correct. */
class ScopeGuard {
public:
  explicit ScopeGuard(void (*fn)(void *), void *ctx) : fn_(fn), ctx_(ctx) {}
  ~ScopeGuard() {
    if (fn_ != nullptr) {
      fn_(ctx_);
    }
  }
  ScopeGuard(const ScopeGuard &) = delete;
  ScopeGuard &operator=(const ScopeGuard &) = delete;
  void dismiss() { fn_ = nullptr; }

private:
  void (*fn_)(void *);
  void *ctx_;
};

static void mark(void *ctx) { *static_cast<int *>(ctx) = 1; }

struct FileCloser {
  void operator()(std::FILE *fp) const {
    if (fp != nullptr) {
      std::fclose(fp);
      std::cout << "FILE closed by unique_ptr\n";
    }
  }
};

static void risky(int *flag) {
  ScopeGuard guard(mark, flag);
  Buffer buf(4);
  (void)buf;
  /* A throw here would still run ~ScopeGuard and ~Buffer: that is the whole
   * claim of RAII, and the reason a raw new would leak. */
}

int main() {
  {
    std::unique_ptr<Buffer> owned = std::make_unique<Buffer>(4);
    CRAFT_ASSERT(owned->size() == 4);
    CRAFT_ASSERT(owned->at(3) == 3);

    std::unique_ptr<Buffer> taken = std::move(owned);
    CRAFT_ASSERT(owned == nullptr); // moved-from is valid and null
    CRAFT_ASSERT(taken->at(2) == 2);
  } // both destroyed here, in reverse order

  int flag = 0;
  risky(&flag);
  CRAFT_ASSERT(flag == 1);

  /* A C resource through a smart pointer with a custom deleter: the empty
   * deleter struct costs nothing, so the pointer is still one word. */
  std::unique_ptr<std::FILE, FileCloser> file(std::fopen("craft_raii.tmp", "wb"));
  CRAFT_ASSERT(file != nullptr);
  std::fputs("x", file.get());

  std::cout << "sizeof(unique_ptr<FILE>)=" << sizeof(file) << '\n';
  std::remove("craft_raii.tmp");
  return 0;
}
