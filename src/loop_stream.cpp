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

#include "loop_stream.h"
#include "mu_utils.h"
#include <cmath>

namespace mu {

  LoopStream::LoopStream() :
    interval_(44100),
    source_start_(0), 
    source_end_(44100) {
  }
  
  LoopStream::~LoopStream() {
    // printf("~LoopStream()\n");
  }

  LoopStream *LoopStream::clone() {
    LoopStream *c = new LoopStream();
    c->set_interval(interval());
    c->set_source_start(source_start());
    c->set_source_end(source_end());
    // TODO: can I delegate this to SingleSourceStream?
    c->set_source(source() ? source()->clone() : NULL);
    return c;
  }

  bool LoopStream::render(MuTick buffer_start, MuBuffer *buffer) {
    if (source_ == NULL) { return false; }

    MuTick buffer_end = buffer_start + buffer->frames();

    // The loop stream function is defined as:
    //
    //   loop(source, t) = SUM[i=-inf to +inf] { source(t - i * interval) }
    // where
    //   source_start <= (t - i * interval) < source_end
    //
    // We can't iterate from -inf to +inf, so we depend upon source_start_ and
    // source_end_ to limit the iteration.  The first order of business is to
    // find the minimum and maximum values for i that produce non-zero samples.

    // bmin = buffer_start (inclusive) - first t in buffer
    // bmax = buffer_end (exclusive) - last t in buffer
    // smin = source_start (inclusive) - first t in source
    // smax = source_end (exclusive) - last t in source

    // The source is delayed by i * interval.  

    // imin:
    //   t - imin * interval < smax
    //   t - smax < imin * interval
    //   imin > (t - smax) / interval
    //   imin = floor((t - smax) / interval) + 1
    // imax:
    //   t - imax * interval >= smin
    //   t - smin >= imax * interval
    //   imax <= (t - smin) / interval
    //   imax = floor((t - smin) / interval)

    double bmin = buffer_start;
    double bmax = buffer_end;
    double smin = source_start_;
    double smax = source_end_;

    int imin = floor((bmin - smax) / (double)interval_) + 1;
    int imax = floor((bmax - smin) / (double)interval_);

    // looping doesn't start before tick=0
    imin = std::max(imin, 0);

    if (imin > imax) { 
      // printf("Z: imin=%3d, imax = %3d\n", imin, imax);
      return false; 
    }

    bool any_written = false;
    for (int i=imin; i<=imax; ++i) {
      MuTick delay = i * interval_;
      if (render_segment(buffer, buffer_start, buffer_end, delay)) {
        any_written = true;
      }
    }
    
    return any_written;
  }

  // Add source stream delayed by `delay` frames into buffer.  Returns
  // true if any frames were availble from the source.
  bool LoopStream::render_segment(MuBuffer *buffer, 
                                  MuTick buffer_start, 
                                  MuTick buffer_end,
                                  MuTick delay) {
    // Add source[N - delay] into buffer[N], but:
    // source extent is limited to source_start_ to source_end_
    // buffer extent is limited to buffer_start to buffer_end.
    // Calculate the extent of N:

    //  printf("A: bs=%3ld, be=%3ld, ss=%3ld, se=%3ld, dly=%3ld\n",
    //        buffer_start, buffer_end, source_start_, source_end_, delay);
    MuTick s1 = std::max(source_start_ + delay, buffer_start);
    MuTick e1 = std::min(source_end_ + delay, buffer_end);
    if (s1 >= e1) { 
      // printf("B: s1 = %3ld, e1 = %3ld\n", s1, e1);
      return false; 
    }

    // printf("processing %3ld samples:\n", e1-s1);
    // printf("  src start = %3ld\n", s1 - delay);
    // printf("  dst start = %3ld\n", s1);

    tmp_buffer_.resize(e1 - s1, buffer->channels());
    MuUtils::zero_buffer(&tmp_buffer_);
    if (source_->render(s1 - delay, &tmp_buffer_) == false) { return false; }

    // tmp_buffer_ now contains samples from the source to be added into buffer.
    // tmp_buffer[0] contains the sample for (output) time s1.
    for (MuTick frame=s1; frame<e1; frame++) {
      for (int channel=buffer->channels()-1; channel >= 0; channel--) {
        (*buffer)(frame-buffer_start, channel) += tmp_buffer_(frame - s1, channel);
      }
    }

    return true;
  }

  void LoopStream::inspect_aux(int level, std::stringstream *ss) {
    MuStream::inspect_aux(level, ss);
    inspect_indent(level, ss);
    *ss << "interval() = " << interval() << std::endl;
    inspect_indent(level, ss);
    *ss << "source_start() = " << source_start() << std::endl;
    inspect_indent(level, ss);
    *ss << "source_end() = " << source_end() << std::endl;
    inspect_indent(level, ss); 
    SingleSourceStream::inspect_aux(level, ss);
  }

}                               // namespace mu
