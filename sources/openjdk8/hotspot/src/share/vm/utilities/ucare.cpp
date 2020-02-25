#include "precompiled.hpp"

#include "memory/iterator.hpp"
#include "memory/resourceArea.hpp"

#include "runtime/timer.hpp"

#include "utilities/globalDefinitions.hpp"
#include "utilities/hashtable.hpp"
#include "utilities/ostream.hpp"
#include "utilities/ucare.hpp"

jint Ucare::initialize() {
  return JNI_OK;
}

class UcareAlwaysTrueClosure: public BoolObjectClosure {
public:
  bool do_object_b(oop p) { return true; }
};

static UcareAlwaysTrueClosure always_true;

void Ucare::HeapIterationClosure::do_object(oop obj) {
  _total_object_counts++;
  if (is_live_object(obj)) {
    _live_object_counts++;
  } else {
    _dead_object_counts++;
  }
}

void Ucare::count_objects(BoolObjectClosure* filter, const GCId& gc_id, const char* phase) {
  ResourceMark rm;
  
  ucarelog_or_tty->print_cr("starting object counter for gc_id: %u, phase : %s", gc_id.id(), phase);
  elapsedTimer time_execution;
  time_execution.reset();
  TraceTime t("process_objects_counting", &time_execution);
  HeapIterationClosure hic(filter);
  Universe::heap()->object_iterate(&hic);
  ucarelog_or_tty->stamp(PrintGCTimeStamps);
  ucarelog_or_tty->print_cr("object counter [ gc_id: %u, phase %s, elapsed_time: %zu ms, counter: { dead: %zu, live: %zu, total: %zu } ]", gc_id.id(), phase, time_execution.milliseconds(), hic.get_dead_object_counts(), hic.get_live_object_counts(), hic.get_total_object_counts());
}

void Ucare::count_all_objects(const GCId& gc_id, const char* phase) {
  count_objects(&always_true, gc_id, phase);
}
