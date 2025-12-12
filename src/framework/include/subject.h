#ifndef __SUBJECT_H__
#define __SUBJECT_H__

#include <memory>
#include <vector>
#include <mutex>
#include <algorithm>

#include "observer.h"

/** @class Subject
 *  @brief The Subject class is part of the Observer design pattern.
 *         It maintains a list of observers and notifies them of changes.
 *         Observers can be attached or detached from the subject.
 */
template <typename T>
class Subject
{
public:
  /** @brief Attaches an observer to the subject.
   *  @param observer A shared pointer to the observer to be attached.
   */
  void attach(std::shared_ptr<Observer<T>> observer)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_observers.push_back(observer);
  }

  /** @brief Detaches an observer from the subject.
   *  @param observer A shared pointer to the observer to be detached.
   */
  void detach(std::shared_ptr<Observer<T>> observer)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    // Remove the observer from the list of observers
    // using a weak pointer to avoid dangling references
    m_observers.erase(std::remove_if(m_observers.begin(), m_observers.end(),
                                      [&observer](const std::weak_ptr<Observer<T>>& weak_observer) {
                                        return weak_observer.lock() == observer;
                                      }), m_observers.end());
  }

  /** @brief Notifies all attached observers with the provided data.
   *  @param data The data to be sent to the observers.
   */
  void notify(const T& data)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& weak_observer : m_observers)
    {
      // Check if the observer is still valid
      // and call its update method if it is
      if (auto observer = weak_observer.lock())
      {
        observer->update(data);
      }
    }
  }

private:
	std::vector<std::weak_ptr<Observer<T>>> m_observers;
  std::mutex m_mutex;
};

#endif  // __SUBJECT_H__