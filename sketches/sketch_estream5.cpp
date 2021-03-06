/*
 * side-effect free streams.  Uses lambda to create deferred evaluation.
 */

#include <functional>
#include <string>

#define NULL_ESTREAM nullptr
#define IS_NULL_ESTREAM(s) ((s) == NULL_ESTREAM)
#define DEFER(form, T) [=]()->EStream<T>*{return (form);}
#define FORCE(deferred_form) (deferred_form())
#define CREATE_STREAM(first, rest, T) \
  new EStream<T>((first), DEFER(rest, T))

template <typename T>class EStream {
public:
  EStream(T first, std::function<EStream<T> *()> rest) :
    first_(first), rest_(rest) {}

  T first() { 
    return first_; 
  }

  EStream<T> *rest() { 
    return IS_NULL_ESTREAM(rest_) ? NULL_ESTREAM : FORCE(rest_); 
  }

protected:
  T first_;
  std::function<EStream<T> *()> rest_;
};

// Specialize streams below, arranged alphabetically

template <typename T>
EStream<T> *AppendES(EStream<T> *s0, EStream<T> *s1) {
  if (IS_NULL_ESTREAM(s0)) {
    return s1;
  } else if (IS_NULL_ESTREAM(s1)) {
    return s0;
  } else {
    return CREATE_STREAM(s0->first(),
                         AppendES(s0->rest(), s1),
                         T);
  }
}

template <typename T>
EStream<T> *ConstantES(T value) {
  return CREATE_STREAM(value, ConstantES(value), T);
}

template <typename T>
EStream<T> *CycleES(EStream<T> *s) {
  return CycleESAux(s, s);
}

template <typename T>
EStream<T> *CycleESAux(EStream<T> *s, EStream<T> *s0) {
  if (IS_NULL_ESTREAM(s)) {
    return CREATE_STREAM(s0->first(),
                         CycleESAux(s0->rest(), s0), T);
  } else {
    return CREATE_STREAM(s->first(),
                         CycleESAux(s->rest(), s0), T);
  }
}

template <typename T>
EStream <T> *DropES(EStream<T> *s0, int n) {
  if (n == 0) {
    return s0;
  } else {
    return DropES(s0->rest(), n-1);
  }
}

template <typename T>
EStream<T> *FilterES(EStream<T> *s, bool (*predicate)(T)) {
  if (IS_NULL_ESTREAM(s)) {
    return NULL_ESTREAM;
  } else if (predicate(s->first())) {
    return CREATE_STREAM(s->first(),
                         FilterES(s->rest(), predicate),
                         T);
  } else {
    return FilterES(s->rest(), predicate);
  }
}

template <typename T>
EStream<T> *InterleaveES(EStream<T> *s0, EStream<T> *s1) {
  if (IS_NULL_ESTREAM(s0)) {
    return s1;
  } else if (IS_NULL_ESTREAM(s1)) {
    return s0;
  } else {
    return CREATE_STREAM(s0->first(),
                         InterleaveES(s1, s0->rest()),
                         T);
  }
}

// apply lambda or function to each element of stream
template <typename T, typename MapFn>
EStream<T> *MapES(EStream<T> *s, MapFn mapfn) {
  if (IS_NULL_ESTREAM(s)) {
    return NULL_ESTREAM;
  } else {
    return CREATE_STREAM(mapfn(s->first()),
                         MapES(s->rest(), mapfn),
                         T);
  }
}

template <typename T, typename PredFn>
EStream<T> *MergeES(EStream<T> *s0, EStream<T> *s1, PredFn predicate) {
  if (IS_NULL_ESTREAM(s0)) {
    return s1;
  } else if (IS_NULL_ESTREAM(s1)) {
    return s0;
  } else if (predicate(s0->first(), s1->first())) {
    return CREATE_STREAM(s0->first(),
                         MergeES(s0->rest(), s1, predicate),
                         T);
  } else {
    return CREATE_STREAM(s1->first(),
                         MergeES(s0, s1->rest(), predicate),
                         T);
  }
}

template <typename T>
EStream<T> *NullES() {
  return NULL_ESTREAM;
}

template <typename T>
EStream<T> *TakeES(EStream<T> *s, int n) {
  if ((IS_NULL_ESTREAM(s)) || (n <= 0)) {
    return NULL_ESTREAM;
  } else {
    return CREATE_STREAM(s->first(),
                         TakeES(s->rest(), n-1),
                         T);
  }
}

// TODO: predicate could return a bool that stops processing.
template <typename T>void ForEachES(EStream<T> *s, void (*predicate)(T)) {
  if (!IS_NULL_ESTREAM(s)) {
    predicate(s->first());
    ForEachES(s->rest(), predicate);
  }
}

template <typename T>T ReferenceES(EStream<T> *s, int n) {
  if (n == 0) {
    return s->first();
  } else {
    return ReferenceES(s->rest(), n-1);
  }
}

// ================================================================
// specialized streams

EStream<int> *IntegersES(int starting) {
  return CREATE_STREAM(starting,
                       IntegersES(starting + 1),
                       int);
}

// see https://mitpress.mit.edu/sicp/full-text/book/book-Z-H-24.html#%_sec_3.5.2
// WORK IN PROGRESS
// template <typename T>
// EStream *SieveES(EStream<T> *s, bool (*predicate)(T, T)) {
//   // TODO: convert predicate into a lambda
//   // predicate 
//   return CREATE_STREAM(s->first(),
//                        SieveES(FilterES(s->rest(), predicate)));
// }

// ================================================================  
// ================================================================  

// Test out the various streams...
void test(EStream<int> *s,
          std::string name,
          std::string expected,
          int max = -1) {
  printf("=== %s expect %s\n", name.c_str(), expected.c_str());
  printf("    %*c got    ", (int)name.size(), ' ');
  for (int i=max; (i != 0) && (!IS_NULL_ESTREAM(s)); i--, s = s->rest()) {
    printf("%d ", s->first());
  }
  printf("\n");
}

int dbl(int v) { return 2 * v; }

bool sort_ints(int v0, int v1) { return v0 < v1; }

int main( void ) {

  EStream<int> *append = AppendES(TakeES(IntegersES(10), 3),
                                  TakeES(IntegersES(20), 3));
  test(append, "AppendES", "10 11 12 20 21 22");

  EStream<int> *constant = ConstantES(42);
  test(constant, "ConstantES", "42 42 42 42 42", 5);

  EStream<int> *cycle = CycleES(TakeES(IntegersES(0), 3));
  test(cycle, "CycleES", "0 1 2 0 1 2 0 1 2 0 1 2", 12);

  EStream<int> *drop = DropES(IntegersES(0), 4);
  test(drop, "DropES", "4 5 6 7 8", 5);

  EStream<int> *ints = IntegersES(22);
  test(ints, "IntegersES", "22 23 24 25 26", 5);

  EStream<int> *interleave = InterleaveES(TakeES(IntegersES(10), 3),
                                         TakeES(IntegersES(20), 3));
  test(interleave, "InterleaveES", "10 20 11 21 12 22");
    
  EStream<int> *take = TakeES(IntegersES(0), 5);
  test(take, "TakeES", "0 1 2 3 4");

  // map using a function pointer
  EStream<int> *map = MapES(TakeES(IntegersES(10), 3), dbl);
  test(map, "MapES (with fn pointer)", "20 22 24");

  // map using a lambda
  EStream<int> *map2 = MapES(TakeES(IntegersES(30), 3), 
                             [=](int v)->int { return 2 * v; });
  test(map2, "MapES (with lambda)", "60 62 64");

  // merge using function pointer
  EStream<int> *merge1 = MergeES(MapES(TakeES(IntegersES(1), 5),
                                       [=](int v) { return 4 * v; }),
                                 MapES(TakeES(IntegersES(1), 5),
                                       [=](int v) { return 5 * v; }),
                                 sort_ints);
  test(merge1, "MergeES (with fn pointer)", "4 5 8 10 12 15 16 20 20 25");

  // merge using lambda
  EStream<int> *merge2 = MergeES(MapES(TakeES(IntegersES(1), 5),
                                       [=](int v) { return 4 * v; }),
                                 MapES(TakeES(IntegersES(1), 5),
                                       [=](int v) { return 5 * v; }),
                                 [=](int v0, int v1) { return v0 < v1; });
  test(merge2, "MergeES (with lambda)", "4 5 8 10 12 15 16 20 20 25");


  return 0;
}
