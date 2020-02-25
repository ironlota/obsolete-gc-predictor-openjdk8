#ifndef SHARE_VM_UTILITIES_UCARE_HPP
#define SHARE_VM_UTILITIES_UCARE_HPP

#include "gc_implementation/shared/gcId.hpp"

#include "memory/allocation.hpp"
#include "memory/iterator.hpp"
#include "memory/universe.hpp"

#include "utilities/globalDefinitions.hpp"


// struct ObjectCounterPhase {
//   const char* phase;
//   size_t live_object_counts;
//   size_t dead_object_counts;
// };

// struct GCObjectMap {
//   int gc_id;
//   BasicHashtable<ObjectCounterPhase>* phases;
// };

// @rayandrew
// create new helper classes to contain all data for UCARE Research
class Ucare : AllStatic {
  friend class Universe;
  friend jint  universe_init();


  private:
    class HeapIterationClosure : public ObjectClosure {
      private:
        BoolObjectClosure* _live_filter;
        size_t _total_object_counts;
        size_t _dead_object_counts;
        size_t _live_object_counts;
      
      private:
        bool is_live_object(oop obj) {
          return _live_filter != NULL && _live_filter->do_object_b(obj);
        }
      
      public:
        HeapIterationClosure(BoolObjectClosure* filter) : _live_filter(filter), _total_object_counts(0), _dead_object_counts(0), _live_object_counts(0) {}
        virtual void do_object(oop obj);
        size_t get_total_object_counts() {
          return _total_object_counts;
        }
        size_t get_dead_object_counts() {
          return _dead_object_counts;
        }
        size_t get_live_object_counts() {
          return _live_object_counts;
        }
    };
    static jint initialize();
  
  public:
    Ucare() { ShouldNotReachHere(); }
    ~Ucare() { ShouldNotReachHere(); }

    static void count_objects(BoolObjectClosure* filter, const GCId& gc_id, const char* phase = "");
    static void count_all_objects(const GCId& gc_id, const char* phase = "");
};

#endif // SHARE_VM_UTILITIES_UCARE_HPP
