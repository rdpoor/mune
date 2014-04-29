/*
 * Test FileReader
 */
#include "mu.h"
#include "rt_player.h"
#include "file_reader.h"

int main() {
  mu::RtPlayer player;      // allocate a real-time audio playback
  mu::FileReader fr;
  fr.fileName("/Users/r/Projects/Musics/TNVM/sources/Chords/A.wav").doNormalize(true);

  player.setSource(&fr);    // connect output of generator to player.
  player.start();            // start streaming (should hear output)
  sleep(5);                     // ...for five seconds
  player.stop();             // then stop.
  return 0;
}
