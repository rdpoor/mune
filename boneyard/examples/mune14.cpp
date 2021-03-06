/*
 * Experiment: when Ticks counter decreases, randomly pick
 * a new sound sample to play back.
 */
#include "multi_source_sp.h"
#include "file_read_sp.h"
#include "loop_sp.h"
#include "mu.h"
#include "rt_player.h"
#include <unistd.h>

#define SOUND_DIR "/Users/r/Projects/Musics/TNVM/sources/ThumpsAndScratches/"

/*
 * Maintain a collection of input streams.  Every time the tick counter
 * backs up, select a different input stream as the current stream.
 */
class ResetSP : public mu::MultiSourceSP<ResetSP> {
public:
  
  ResetSP() : prev_tick_ (mu::TickUtils::indefinite()) { }
  ~ResetSP( void ) { }
  void step(stk::StkFrames& buffer, mu::Tick tick, bool is_new_event) {
    if (sources_.size() == 0) {
      fprintf(stderr,"A");
      zeroBuffer(buffer);
    } else {
      fprintf(stderr,"B");
      if (tick < prev_tick_) { reset(); }
      prev_tick_ = tick;
      current_sp_->step(buffer, tick, true);
    }
  }
  mu::Tick getStart(){ return current_sp_->getStart(); }
  mu::Tick getEnd(){ return current_sp_->getEnd(); }

protected:
  ResetSP& reset();

  mu::Tick prev_tick_;
  mu::SampleProcessor *current_sp_;

};                            // class ResetSP

ResetSP& ResetSP::reset() {
  int index = std::rand() % sources_.size();
  fprintf(stderr, "reset: index=%d, size=%ld\n", index, sources_.size());
  current_sp_ = sources_.at(index);
  return *this;
}

mu::FileReadSP *getSound(std::string file_name) {
  return &(new mu::FileReadSP())->fileName(file_name).doNormalize(true);
}

int main() {
  ResetSP reset_sp;
  mu::LoopSP loop_sp;
  mu::RtPlayer player;

  reset_sp.addSource(getSound(SOUND_DIR "s00.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s01.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s02.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s03.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s04.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s05.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s06.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s07.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s08.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s09.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s10.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s11.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s12.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s13.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s14.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s15.wav"));
  reset_sp.addSource(getSound(SOUND_DIR "s16.wav"));

  loop_sp.setSource(&reset_sp).setLoopDuration(44100/3);
  player.setSource(&loop_sp);
  player.start();
  sleep(30);
  player.stop();

  return 0;
}
