/*
  ================================================================
  Copyright (C) 2014 Robert D. Poor
  
  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:
  
  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  ================================================================
*/
#include "add_rs.h"

namespace mu {

  AddRS::AddRS() {
    buffer_.resize(stk::RT_BUFFER_SIZE, 2);
  }

  AddRS::~AddRS() {
  }

  bool AddRS::render(stk::StkFrames& frames, MuTick base_tick, MuTick start_tick, MuTick end_tick) {

    bool any_modified = false;

    for (int i=sources_.size()-1; i>=0; i--) {
      RenderStream *source = sources_.at(i);
      buffer_.resize(frames.frames(), frames.channels());
      // render source into temp buffer and sum into frames
      if (source->render(buffer_, base_tick, start_tick, end_tick)) {
        any_modified = true;
        for (int tick=start_tick; tick<end_tick; tick++){
          for (int ch=buffer_.channels()-1; ch>=0; ch--) {
            frames(frame_index(base_tick, tick), ch) += buffer_(frame_index(base_tick, tick), ch);
          } // for ch
        }   // for tick
      }     // if (source->)
    }       // for i
    return any_modified;
  }

}
