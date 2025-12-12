#ifndef __OBSERVER_H__
#define __OBSERVER_H__

/** @class Observer
 *  @brief The Observer class is part of the Observer design pattern.
 *         It defines an interface for objects that should be notified of changes in a subject.
 *         Classes that implement this interface can be registered with a subject to receive updates.
 *         The update method is called by the subject when there is new data available.
 */
template <typename T>
class Observer
{
public:
	virtual void update(const T& data) = 0;
	virtual ~Observer() = default;
};

#endif  // __OBSERVER_H__