/*
  ================================================================
  Copyright (C) 2014-2015 Robert D. Poor
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  ================================================================
*/
#include "multi_source_stream.h"

namespace mu {
  
  MultiSourceStream::MultiSourceStream() {
  }

  MultiSourceStream::~MultiSourceStream() {
    // printf("~MultiSourceStream()\n");
    for (int i=sources_.size()-1; i>=0; --i) {
      MuStream *source = sources_.at(i);
      if (source != NULL) delete source;
    }
  }

#if 0
  MultiSourceStream *MultiSourceStream::clone() {
    MultiSourceStream *c = new MultiSourceStream();
    for (int i=sources_.size()-1; i>=0; --i) {
      MuStream *source = sources_.at(i);
      c->add_source(source ? source->clone() : source);
    }
  }
#endif

  void MultiSourceStream::inspect_aux(int level, std::stringstream *ss) {
    MuStream::inspect_aux(level, ss);
    inspect_indent(level, ss);
    *ss << "sources()" << std::endl;
    for (int i=sources_.size()-1; i>=0; --i) {
      sources().at(i)->inspect_aux(level+1, ss);
    }
  }

}                               // namespace mu
