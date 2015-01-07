#ifndef MU_TIMED_QUEUE_H
#define MU_TIMED_QUEUE_H

#include <vector>
#include <stdlib.h>
#include "timed_event.h"
#include <mutex>

namespace mu {

  class TimedQueue {
  public:
    TimedQueue() {
    }
    ~TimedQueue() {
    }

    TimedEvent *peekNext() {
      TimedEvent *te = NULL;
      mutex_.lock();
      if (v_.size() > 0) {
        te = v_.back();
      }
      mutex_.unlock();
      return te;
    }

    TimedEvent *getNext() {
      TimedEvent *te = NULL;
      mutex_.lock();
      if (v_.size() > 0) { 
        te = v_.back();
        v_.pop_back();
      }
      mutex_.unlock();
      return te;
    }
    
    void enqueue(TimedEvent *event);

    size_t size() {
      return v_.size();
    }

    void clear() {
      mutex_.lock();
      v_.clear();
      mutex_.unlock();
    }

  protected:
    std::vector<TimedEvent *> v_;
    std::mutex mutex_;
  };



} // namespace mu

#endif

// Local Variables:
// mode: c++
// End:
