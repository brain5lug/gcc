// { dg-do compile }

#include <coroutine>

struct Promise;

struct Task
{
  using promise_type = Promise;
};

struct Promise
{
  std::suspend_never initial_suspend () { return {}; }
  std::suspend_never final_suspend () noexcept { return {}; }
  void unhandled_exception () {}
  Task get_return_object () { return Task {}; }
  void return_value (bool) {}
};

struct Awaitable
{
  /* __attribute__((noinline)) apparently prevents the compiler from seeing
     that co_await can be a no-op.  */
  bool __attribute__((noinline)) await_ready () { return true; }
  void await_suspend (std::coroutine_handle<>) {}
  bool await_resume () { return true; }
};

/* A type whose alignment requirement makes the coroutine frame
   over-aligned.  */
struct HighlyAligned
{
  alignas (64) char storage[16];
};

/* The parameter copy is stored in the coroutine frame, so passing an
   over-aligned type by value also makes the frame over-aligned.  */
Task
__attribute__((noinline))
Coroutine (HighlyAligned a) // { dg-warning "coroutine frame 'Coroutine\\.Frame' requires extended alignment of 64 bytes, but its allocation function only guarantees \[0-9]+-byte alignment" }
{
  bool result = co_await Awaitable {};
  static_cast<void> (a);
  co_return result;
}

// { dg-message "field 'a' of type 'HighlyAligned' requires 64-byte alignment, forcing over-alignment of the coroutine frame" "" { target *-*-* } 0 }
