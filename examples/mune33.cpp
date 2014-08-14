/*
 * Sketch for a strummed, fretted instrument with a fixed number of
 * strings.  See also mune32 and mune23 for an earlier version.
 *
 * TODO:
 *
 * - Code up the intro to TNVM using this instrument.  See also
 *   https://www.facebook.com/photo.php?v=10201180856748581
 *   (Thank you Bradley...)
 *
 * - per-string output busses and/or per-string panning.  It can sound
 *   grand when the strings are panned across the stero space.
 *
 * - per-string de-tuning
 *
 * - Cross-fades.  I currently use simple cropping to transition from
 *   one sound to the next, but (predictably) this causes clicks.  I 
 *   need SpliceSP, a SampleProcessor with a source, start time and end
 *   time and fade time.  It fades in at start time and fades out and
 *   end time.  
 *
 * - Mixing many streams.  I currently use AddSP to mix an arbitrary
 *   number of streams together.  This works, but if it becomes bogged
 *   down on a more complex mix, enhance AddSP (or create MixSP) that
 *   keeps sources sorted by start time and only process sources that
 *   are actively contributing to the output.
 */
#include "mu.h"

#include "add_sp.h"
#include "constant_sp.h"
#include "crop_sp.h"
#include "delay_sp.h"
#include "file_read_sp.h"
#include "multi_source_sp.h"
#include "multiply_sp.h"
#include "null_sp.h"
#include "rt_player.h"
#include "single_source_sp.h"
#include "tick_utils.h"

#include <map>

// ================================================================
// class SpliceSP : public mu::SingleSourceSP<SpliceSP> {
  // for now, delgate to mu::CropSP
// };

// ================================================================
// class MixSP : public mu::MultiSourceSP<MixSP> {
  // for now, delegate to mu::AddSP
// };

// ================================================================
// SPSet maps an integer pitch number to a SampleProcessor that
// produces that pitch.  Its subclass (FileReadSPSet) maps from a
// pitch number to a sound file.

class SPSet {
public:
  SPSet( void ) {
  }
  ~SPSet( void ) {
  }
  virtual mu::SampleProcessor *findSP(int pitch) = 0;

protected:
  mu::NullSP null_sp_;
};                              // class SPSet

class FileReadSPSet : public SPSet {
public:
  mu::SampleProcessor *findSP(int pitch) {
    if (cache_.find(pitch) == cache_.end()) {
      std::string file_name = makeFileName(pitch);
      mu::FileReadSP *frs = new mu::FileReadSP();
      frs->fileName(file_name).doNormalize(true);
      cache_[pitch] = frs;
    }
    return cache_[pitch];
  }

  std::string getDirectoryName( void ) { return directory_name_; }
  FileReadSPSet& setDirectoryName(std::string directory_name) { 
    directory_name_ = directory_name; 
    return *this;
  }

protected:
  std::string makeFileName(int pitch) {
    std::stringstream ss;
    ss << directory_name_ << pitch << ".wav";
    return ss.str();
  }
  std::string directory_name_;
  std::map<int, mu::FileReadSP *> cache_;
};                              // class FileReadSPSet


// ================================================================
class FrettedString {
public:
  static const int kHammer = 1000;
  static const int kDamped = -1;
  static const int kSustain = -2;

  FrettedString( int open_pitch,  mu::AddSP *output, FileReadSPSet *sp_set ) {
    open_pitch_ = open_pitch;
    output_ = output;
    sp_set_ = sp_set;
    curr_sp_ = NULL;
  }

  FrettedString& pluck(mu::Tick start, int fret) {
    if (fret == kSustain) {
      return *this;
    } else if (fret == kDamped) {
      return emitDamped(start);
    } else if (fret >= kHammer) {
      return emitHammer(start, findSP(fret - kHammer));
    } else {
      return emitPluck(start, findSP(fret));
    }
  }

protected:

  // emit the previously started note
  FrettedString &emitDamped(mu::Tick start) {
    emit(start);
    curr_sp_ = NULL;
    return *this;
  }

  // emit the previously started note, start a new note
  // that has the same attack time as the previous note,
  // but cropped to start at the current start time.
  FrettedString &emitHammer(mu::Tick start, mu::SampleProcessor *sp) {
    if (curr_sp_ == NULL) {
      // special case: hammer on from no note is same as plucking
      return emitPluck(start, sp);
    }
    emit(start);
    hammer_time_ = start;
    curr_sp_ = sp;
    return *this;
  }

  // emit previously started note and start a new note.
  FrettedString &emitPluck(mu::Tick start, mu::SampleProcessor *sp) {
    emit(start);
    attack_time_ = hammer_time_ = start;
    curr_sp_ = sp;
    return *this;
  }

  FrettedString &emit(mu::Tick start) {
    if (curr_sp_ != NULL) {
      mu::CropSP *cropSP = &(new mu::CropSP)->
        setSource(curr_sp_).
        setStart(hammer_time_ - attack_time_).
        setEnd(start - attack_time_);
      mu::DelaySP *delaySP = &(new mu::DelaySP)->
        setSource(cropSP).
        setDelay(attack_time_);
      output_->addSource(delaySP);
    }
    return *this;
  }
  
  mu::SampleProcessor *findSP(int fret) {
    mu::SampleProcessor *sp = sp_set_->findSP(open_pitch_ + fret);
    return (sp == NULL) ? &null_sp_ : sp;
  }
  
  int open_pitch_;
  FileReadSPSet *sp_set_;
  mu::AddSP *output_;
  mu::NullSP null_sp_;
  mu::SampleProcessor *curr_sp_; // sample source
mu::Tick attack_time_;         // time of attack
mu::Tick hammer_time_;           // time of hammer on / hammer off
};

// ================================================================
class FrettedInstrument {

public:

  FrettedInstrument( void ) {
  }
  ~FrettedInstrument( void ) {
  }

  FileReadSPSet *getSPSet() { return stream_set_; }
  FrettedInstrument& setSPSet( FileReadSPSet *stream_set ) { stream_set_ = stream_set; return *this; }

  mu::AddSP *getOutput() {
    return &output_;
  }

  FrettedInstrument& addString(int open_pitch) {
    strings_.push_back(new FrettedString(open_pitch, getOutput(), getSPSet()));
    return *this;
  }

  /*
   * Generate a strum.  If rate is positive, it's a downstroke.  If
   * negative, it's an upstroke.
   *
   * Frets specifies the fret number (including 0 for open), and
   * recognizes the following special values:
   *
   *  negative pitch -- change pitch without re-attack
   *  FrettedInstrument::kDamped -- string does not play
   *  FrettedInstrument::kSustain -- prev note allowed to ring.
   *
   * nb: \c frets must contain exactly one item per string.
   */
  FrettedInstrument& strum(mu::Tick start, int frets[], mu::Tick rate) {
    if (rate < 0) {
      start = start + (1-strings_.size()) * rate;
    }
    for (long int i=0; i<strings_.size();  i++) {
      FrettedString *s = strings_.at(i);
      s->pluck(start + (i * rate), frets[i]);
    }
    return *this;
  }

  FrettedInstrument& damp(mu::Tick end) {
    for (long int i=0; i<strings_.size(); i++) {
      FrettedString *s = strings_.at(i);
      s->pluck(end, FrettedString::kDamped);
    }
    return *this;
  }

protected:

  std::vector<FrettedString *> strings_;
  mu::AddSP output_;
  FileReadSPSet *stream_set_;
};

// ================================================================
int main() {
  mu::RtPlayer player;
  FileReadSPSet file_read_sp_set;
  FrettedInstrument fretted_instrument;

  file_read_sp_set.setDirectoryName("/Users/r/Projects/Mu/SoundSets/A/");
  fretted_instrument.setSPSet(&file_read_sp_set);

  // 12 string open D tuning
  fretted_instrument.addString(38);    // D2
  fretted_instrument.addString(38+24); // D3
  fretted_instrument.addString(45);    // A2
  fretted_instrument.addString(45+24); // A3
  fretted_instrument.addString(50);    // D3
  fretted_instrument.addString(50+24); // D4
  fretted_instrument.addString(54);    // F#3
  fretted_instrument.addString(54+24); // F#4
  fretted_instrument.addString(57);    // A3
  fretted_instrument.addString(57+24); // A4
  fretted_instrument.addString(62);    // D4
  fretted_instrument.addString(62+24); // D5
  
#define T_STROKE (44100 * 0.015)
#define TQ (44100 * 0.8)
#define TE (TQ/2)
#define TS (TQ/4)
  mu::Tick t = 100;

  fretted_instrument.strum(t, (int []){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, T_STROKE); // downstroke
  t += TS;
  fretted_instrument.strum(t, (int []){0, 0, 2, 2, 0, 0, 1, 1, 0, 0, 0, 0}, -T_STROKE); // upstroke
  t += TE;
  fretted_instrument.strum(t, (int []){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, T_STROKE); // downstroke
  t += 4 * TQ;
  fretted_instrument.damp(t); // final stop

  t += TQ;
  fretted_instrument.strum(t, (int []){0, 0, 0, 0, 0, 0}, T_STROKE); // downstroke
  t += TS;
  fretted_instrument.strum(t, (int []) { // hammer on
      FrettedString::kSustain, 
        FrettedString::kSustain, 
        FrettedString::kHammer + 2, 
        FrettedString::kHammer + 2, 
        FrettedString::kSustain, 
        FrettedString::kSustain, 
        FrettedString::kHammer + 1, 
        FrettedString::kHammer + 1, 
        FrettedString::kSustain, 
        FrettedString::kSustain, 
        FrettedString::kSustain,
        FrettedString::kSustain
        }, 0);
  t += TE;
  fretted_instrument.strum(t, (int []) { // hammer off
      FrettedString::kSustain, 
        FrettedString::kSustain, 
        FrettedString::kHammer + 0, 
        FrettedString::kHammer + 0, 
        FrettedString::kSustain, 
        FrettedString::kSustain, 
        FrettedString::kHammer + 0, 
        FrettedString::kHammer + 0, 
        FrettedString::kSustain, 
        FrettedString::kSustain, 
        FrettedString::kSustain,
        FrettedString::kSustain
        }, 0);
  t += 4 * TQ;
  fretted_instrument.damp(t); // final stop

  t += TQ;
  fretted_instrument.strum(t, (int []){0, 0, 0, 0, 0, 0}, T_STROKE); // downstroke
  t += TS;
  fretted_instrument.strum(t, (int []) { // hammer on
      FrettedString::kSustain, 
        FrettedString::kSustain, 
        FrettedString::kHammer + 2, 
        FrettedString::kHammer + 2, 
        FrettedString::kSustain, 
        FrettedString::kSustain, 
        FrettedString::kHammer + 1, 
        FrettedString::kHammer + 1, 
        FrettedString::kSustain, 
        FrettedString::kSustain, 
        FrettedString::kSustain,
        FrettedString::kSustain
        }, 0);
  t += TE;
  fretted_instrument.strum(t, (int []) { // hammer off
      FrettedString::kSustain, 
        FrettedString::kSustain, 
        FrettedString::kHammer + 0, 
        FrettedString::kHammer + 0, 
        FrettedString::kSustain, 
        FrettedString::kSustain, 
        FrettedString::kHammer + 0, 
        FrettedString::kHammer + 0, 
        FrettedString::kSustain, 
        FrettedString::kSustain, 
        FrettedString::kSustain,
        FrettedString::kSustain
        }, 0);
  t += 4 * TQ;
  fretted_instrument.damp(t); // final stop

  mu::MultiplySP multiply_sp;
  mu::ConstantSP constant_sp;

  // add gain control
  constant_sp.setValue(0.25);
  multiply_sp.addSource(fretted_instrument.getOutput());
  multiply_sp.addSource(&constant_sp);

  player.setSource(&multiply_sp);
  player.start();
  fprintf(stderr, "Type [return] to quit:"); getchar();
  player.stop();

  // std::cout << fretted_instrument.getOutput()->inspect();


}
