#ifndef __MESSAGE_QUEUE_H_
#define __MESSAGE_QUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>

/** @class MessageQueue
 *  @brief A thread-safe message queue for passing messages between threads.
 */
template <typename T>
class MessageQueue
{
public:
  MessageQueue() : m_stopped(false) {}

  /** @brief Push a message onto the queue.
   *  This function adds a message to the end of the queue and notifies one waiting
   *  thread (if any) that a new message is available.
   *  @param message The message to be added to the queue.
   */
  void push(const T& message)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(message);
    m_condition.notify_one();
  }

  /** @brief Pop a message from the queue.
   *  This function removes and returns the front message from the queue. If the queue is empty,
   *  it will block until a message is available.
   *  @return The message at the front of the queue.
   */
  std::optional<T> pop()
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait(lock, [this]
                     { return m_stopped || !m_queue.empty(); });

    if (m_stopped && m_queue.empty())
    {
      return std::nullopt;
    }

    T message = m_queue.front();
    m_queue.pop();
    return message;
  }

  std::optional<T> try_pop()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_queue.empty())
    {
      T message = m_queue.front();
      m_queue.pop();
      return message;
    }
    return std::nullopt;
  }

  /** @brief Check if the queue is empty.
   *  This function checks whether the queue contains any messages.
   *  @return True if the queue is empty, false otherwise.
   */
  bool empty() const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
  }

  /** @brief Stop the queue.
   *  This function sets the stopped flag to true and notifies all waiting threads.
   *  After calling this function, no new messages can be pushed onto the queue.
   */
  void stop()
  {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_stopped = true;
    }

    m_condition.notify_all();
  }

private:
  std::queue<T> m_queue;
  mutable std::mutex m_mutex;
  std::condition_variable m_condition;
  std::atomic<bool> m_stopped;
};

#endif  // __MESSAGE_QUEUE_H_