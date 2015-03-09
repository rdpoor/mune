/* 
   ================================================================
   Copyright (C) 2014-2015 Robert D. Poor
  
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.  

   ================================================================ 
*/

#include "mu_scheduler.h"
#include <algorithm>

namespace mu {

  bool deferredEventComparison(MuDeferredEvent *de0, MuDeferredEvent *de1) {
    return de0->time() > de1->time();
  }

  MuScheduler::~MuScheduler() {
    // printf("~MuScheduler()\n");
  }

  void MuScheduler::schedule_event(MuTick time, DeferredAction action) {
    MuDeferredEvent *deferred_event = new MuDeferredEvent();
    deferred_event->set_time(time);
    deferred_event->set_action(action);
    std::vector<MuDeferredEvent *>::iterator low;

    mutex_.lock();
    low = std::lower_bound(queue_.begin(), 
			   queue_.end(), 
			   deferred_event, 
			   deferredEventComparison);
    queue_.insert(low, deferred_event);
    mutex_.unlock();
  }
    

}                               // namespace mu
