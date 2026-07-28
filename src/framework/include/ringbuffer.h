#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include <array>
#include <atomic>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <type_traits>

#include "logger.h"

namespace miniaudioengine::framework
{

constexpr size_t BUFFER_SIZE = 32 * 1024;

/** @enum eDirection
 *  @brief Read Audio/MIDI stream as input or output
 */
enum class eDirection : unsigned int
{
  Input,
  Output
};

/** @class RingBuffer
 *  @brief A lock-free Ring Buffer implementation for audio streaming.
 *  The data structure is single-producer, single-consumer (SPSC). Exactly one
 *  thread may call the producer methods (write/try_push/space/set_producer_finished)
 *  and exactly one other thread may call the consumer methods (read/try_pop/available).
 *  @tparam T The type of elements stored in the ring buffer. Must be trivially copyable.
 *  @tparam Size The number of element slots. Must be a power of two.
 */
template <typename T, size_t Size>
class RingBuffer
{
  static_assert(Size > 1, "Size must be greater than one");
  static_assert((Size & (Size - 1)) == 0, "Size must be a power of two");
  static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

public:
  RingBuffer() = default;
  ~RingBuffer() = default;

  /** @brief Writes up to `count` items into the buffer.
   *  Producer side. Never blocks; writes as many items as there is space for.
   *  @param src Source array to copy from.
   *  @param count Number of items to write.
   *  @return The number of items actually written, which may be less than `count`.
   */
  size_t write(const T *src, size_t count)
  {
    const size_t write_index = m_write_index.load(std::memory_order_relaxed);
    const size_t read_index = m_read_index.load(std::memory_order_acquire);

    const size_t writable = std::min(count, free_space(write_index, read_index));
    if (writable == 0)
    {
      return 0;
    }

    // The writable region may wrap around the end of the array, so copy it in two parts.
    const size_t first_part = std::min(writable, Size - write_index);
    std::memcpy(&m_buffer[write_index], src, first_part * sizeof(T));
    if (writable > first_part)
    {
      std::memcpy(&m_buffer[0], src + first_part, (writable - first_part) * sizeof(T));
    }

    m_write_index.store((write_index + writable) & (Size - 1), std::memory_order_release);
    return writable;
  }

  /** @brief Reads up to `count` items out of the buffer.
   *  Consumer side. Never blocks; reads as many items as are available.
   *  @param dst Destination array to copy into.
   *  @param count Number of items to read.
   *  @return The number of items actually read, which may be less than `count`.
   */
  size_t read(T *dst, size_t count)
  {
    const size_t read_index = m_read_index.load(std::memory_order_relaxed);
    const size_t write_index = m_write_index.load(std::memory_order_acquire);

    const size_t readable = std::min(count, used_space(write_index, read_index));
    if (readable == 0)
    {
      return 0;
    }

    // The readable region may wrap around the end of the array, so copy it in two parts.
    const size_t first_part = std::min(readable, Size - read_index);
    std::memcpy(dst, &m_buffer[read_index], first_part * sizeof(T));
    if (readable > first_part)
    {
      std::memcpy(dst + first_part, &m_buffer[0], (readable - first_part) * sizeof(T));
    }

    m_read_index.store((read_index + readable) & (Size - 1), std::memory_order_release);
    return readable;
  }

  /** @brief Attempts to push a single item into the ring buffer.
   *  @param item The item to be pushed into the buffer.
   *  @return false if the buffer is full. True otherwise.
   */
  bool try_push(const T &item)
  {
    return write(&item, 1) == 1;
  }

  /** @brief Attempts to pop a single item from the ring buffer.
   *  @param item Reference to store the popped item.
   *  @return false if the buffer is empty. True otherwise.
   */
  bool try_pop(T &item)
  {
    return read(&item, 1) == 1;
  }

  /** @brief Returns the number of items currently readable.
   *  Safe to call from the consumer thread. From the producer thread the result
   *  is a lower bound, since the consumer may drain further at any moment.
   */
  size_t available() const
  {
    return used_space(m_write_index.load(std::memory_order_acquire),
                      m_read_index.load(std::memory_order_acquire));
  }

  /** @brief Returns the number of items that can currently be written.
   *  Safe to call from the producer thread. From the consumer thread the result
   *  is a lower bound, since the producer may fill further at any moment.
   */
  size_t space() const
  {
    return free_space(m_write_index.load(std::memory_order_acquire),
                      m_read_index.load(std::memory_order_acquire));
  }

  /** @brief Returns the maximum number of items the buffer can hold.
   *  @note One slot is reserved to distinguish full from empty.
   */
  size_t capacity() const
  {
    return Size - 1;
  }

  /** @brief Clears the ring buffer, resetting it to an empty state.
   *  @note Not thread-safe. Only call when neither producer nor consumer is running.
   */
  void clear()
  {
    m_write_index.store(0, std::memory_order_relaxed);
    m_read_index.store(0, std::memory_order_relaxed);
    m_producer_finished.store(false, std::memory_order_release);
  }

  /** @brief Marks the end of the stream. Set by the producer once it has no more data.
   *  The consumer treats "producer finished AND available() == 0" as end of stream.
   */
  void set_producer_finished(bool finished = true)
  {
    m_producer_finished.store(finished, std::memory_order_release);
  }

  /** @brief Returns true if the producer has signalled that it has no more data. */
  bool is_producer_finished() const
  {
    return m_producer_finished.load(std::memory_order_acquire);
  }

private:
  static size_t used_space(size_t write_index, size_t read_index)
  {
    return (write_index - read_index) & (Size - 1);
  }

  static size_t free_space(size_t write_index, size_t read_index)
  {
    // One slot is reserved so that write_index == read_index unambiguously means empty.
    return Size - 1 - used_space(write_index, read_index);
  }

  std::array<T, Size> m_buffer{};

  std::atomic<size_t> m_write_index{0}; // Producer index
  std::atomic<size_t> m_read_index{0};  // Consumer index

  std::atomic<bool> m_producer_finished{false};
};

} // namespace miniaudioengine::framework

#endif // __RINGBUFFER_H__
