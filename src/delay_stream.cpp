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

#include "delay_stream.h"
#include "mu_utils.h"

namespace mu {

  DelayStream::DelayStream( void ) 
    : delay_(0) {
  }

  DelayStream::~DelayStream( void ) {
    // printf("~DelayStream()\n");
  }

  DelayStream *DelayStream::clone( void ) {
    DelayStream *c = new DelayStream();
    // TODO: Should be defined in SingleSourceStream
    c->set_source(source() ? source()->clone() : NULL);
    c->set_delay(delay());
    return c;
  }

  bool DelayStream::render(MuTick buffer_start, MuBuffer *buffer) {
    // can source be NULL in a well-formed stream?  If not, perhaps
    // this should assert-fail?
    //
    // I recommend you get https://code.google.com/p/google-glog/ and
    // use CHECK macros for this (if the above is correct).
    if (source_ == NULL) {
      return false;
    } else {
      return source_->render(buffer_start - delay_, buffer);
    }
  }

  void DelayStream::inspect_aux(int level, std::stringstream *ss) {
    MuStream::inspect_aux(level, ss);
    inspect_indent(level, ss); 
    *ss << "delay() = " << delay() << std::endl;
    SingleSourceStream::inspect_aux(level, ss);
  }
  

}                               // namespace mu

