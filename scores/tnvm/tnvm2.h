#ifndef TNVM2_H
#define TNVM2_H

#define FRAMES_PER_SECOND 44100
#define SECONDS_PER_MINUTE 60.0

#define BEATS_PER_MINUTE 130.0
#define BEATS_PER_MEASURE 4

#define BEAT_TO_SECOND(__b__) (__b__ * SECONDS_PER_MINUTE / BEATS_PER_MINUTE)
#define BEAT_TO_FRAME(__b__) (BEAT_TO_SECOND(__b__) * FRAMES_PER_SECOND)
#define MEASURE_TO_BEAT(__m__) (__m__ * BEATS_PER_MEASURE)
#define MEASURE_TO_FRAME(__m__) (BEAT_TO_FRAME(MEASURE_TO_BEAT(__m__)))

#endif
