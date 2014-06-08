/*
 * SplayStream gives independent control of the pitch and the
 * duration of a given periodic waveform.  It takes three stream inputs:
 * - sampleSource: the sample to be played back.
 * - thetaSource: a stream that produces theta as a function of time.
 * - zSource: a stream that produces z as a function of time (0..1).
 *
 * In addition, SplayStream has one instance variable:
 * - period: the period (in samples) of sampleSource.
 *
 * How it works: thetaSource specifies the instantaneous phase of
 * the sampleSource to be read out.  It is taken modulo 2*PI to get
 * the phase.  phiSource specifies which period to play.
 *
 * For example, assume the original sample has a period of P samples.
 *
 * If thetaSource increases by 2 PI / P on each tick, the sample will
 * play back at the original frequency.  If phiSource increases by 1/P
 * on each tick, the sample will play back at the original rate.
 */
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

#ifndef MU_SPLAY_STREAM_H
#define MU_SPLAY_STREAM_H

#include "mu.h"
#include "stream.h"

namespace mu {

  class SplayStream : public Stream {
  public:

    // static const stk::StkFloat kDefaultPeriod = 100.0;
    // static const stk::StkFloat kDefaultNPeriods = 100;

    SplayStream( void );
    ~SplayStream( void );

    std::string getClassName() { return "SplayStream"; }
    virtual void inspectAux(std::stringstream& ss, int level);

    SplayStream& step(stk::StkFrames& buffer, Tick tick, Player &player);

    double getPeriod( void ) { return period_; }
    SplayStream& setPeriod( double period ) { period_ = period; return *this; }

    Tick getNPeriods( void ) { return n_periods_; }
    SplayStream& setNPeriods( Tick n_periods ) { n_periods_ = n_periods; return *this; }

    Tick getNFrames( void ) { return n_periods_ * period_; }

    Stream *getSampleSource( void ) { return sample_source_; }
    SplayStream& setSampleSource(Stream *source) { sample_source_ = source; return *this; }

    Stream *getThetaSource( void ) { return theta_source_; }
    SplayStream& setThetaSource(Stream *source) { theta_source_ = source; return *this; }

    Stream *getPhiSource( void ) { return phi_source_; }
    SplayStream& setPhiSource(Stream *source) { phi_source_ = source; return *this; }

  protected:
    double period_;
    Tick n_periods_;
    Stream *sample_source_;
    Stream *theta_source_;
    Stream *phi_source_;
    stk::StkFrames sample_buffer_;
    stk::StkFrames theta_buffer_;
    stk::StkFrames phi_buffer_;

  };                            // class SplayStream
  
}                               // namespace mu

#endif

// Local Variables:
// mode: c++
// End: