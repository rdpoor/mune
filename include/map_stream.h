/*
 * MapStream lets the user examine and (optionally) modify individual
 * frames generated by a source stream.  After fetching the data from
 * the source, MapStream calls a user-supplied method with the current
 * tick, stkFrames object, index into the stkFrames object and a
 * reference to the Player object.
 *
 * The user-supplied method may modify samples in place.  It can do
 * other things as well, such as print information about individual
 * frames and/or halt playback under certain conditions.
 */
#ifndef MU_MAP_STREAM_H
#define MU_MAP_STREAM_H

#include "mu.h"
#include "stream.h"

namespace mu {

  typedef void (*MapStreamCallback)( stk::StkFrames& buffer, 
                                     Tick tick, 
                                     Player& player,
                                     Tick frame_index );
  
  class MapStream : public Stream {
  public:

    MapStream();
    ~MapStream( void );

    MapStream& step(stk::StkFrames& buffer, 
                          Tick tick,
                          Player &player);
    Tick frameCount();

    Stream *getSource() const;
    MapStream& setSource(Stream *source);

    MapStreamCallback getCallback() const;
    MapStream& setCallback(MapStreamCallback callback);

  protected:
    Stream *source_;
    MapStreamCallback callback_;

  };                            // class MapStream

  inline MapStream::MapStream() {
    TRACE("MapStream::MapStream()\n");
  }

  inline Stream *MapStream::getSource() const { return source_; }
  inline MapStream& MapStream::setSource(Stream *source) {
    TRACE("MapStream::source()\n");
    source_ = source; return *this;
  }
  
  inline MapStreamCallback MapStream::getCallback() const { return callback_; }
  inline MapStream& MapStream::setCallback(MapStreamCallback callback) {
    TRACE("MapStream::setCallback()\n");
    callback_ = callback; return *this;
  }

}                               // namespace mu

#endif
