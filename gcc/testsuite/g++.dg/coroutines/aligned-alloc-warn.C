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

/* The coroutine frame must hold HighlyAligned across the co_await, so the
   frame type requires extended alignment (512 bits) while the frame is
   allocated by a plain operator new call.  */
Task
Coroutine () // { dg-warning "coroutine frame 'Coroutine\\.Frame' requires extended alignment of 64 bytes, but its allocation function only guarantees \[0-9]+-byte alignment" }
{
  auto a = HighlyAligned {};
  bool result = co_await Awaitable {};
  static_cast<void> (a);
  co_return result;
}

// { dg-message "field 'a_1_2' of type 'HighlyAligned' requires 64-byte alignment, forcing over-alignment of the coroutine frame" "" { target *-*-* } 0 }
