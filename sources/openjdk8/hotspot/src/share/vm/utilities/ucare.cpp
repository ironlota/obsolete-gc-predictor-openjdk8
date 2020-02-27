#include "precompiled.hpp"

#include "gc_implementation/shared/gcId.hpp"
#include "memory/allocation.hpp"
#include "memory/iterator.hpp"
#include "memory/resourceArea.hpp"
#include "memory/universe.hpp"
#include "runtime/timer.hpp"
#include "utilities/globalDefinitions.hpp"
#include "utilities/ostream.hpp"
#include "utilities/ucare.hpp"

int Ucare::RootTypeMixin::NUM_OF_ROOTS = 10;

jint Ucare::initialize() {
  return JNI_OK;
}

class UcareAlwaysTrueClosure: public BoolObjectClosure {
public:
  bool do_object_b(oop p) { return true; }
};

static UcareAlwaysTrueClosure always_true;

// mixins
//// ObjectCounterMixin
void Ucare::ObjectCounterMixin::set_dead_object_counts(size_t dead) {
  _dead = dead;
}

void Ucare::ObjectCounterMixin::inc_dead_object_counts() {
  _dead++;
}

void Ucare::ObjectCounterMixin::dec_dead_object_counts() {
  _dead--;
}

void Ucare::ObjectCounterMixin::set_live_object_counts(size_t live) {
  _live = live;
}

void Ucare::ObjectCounterMixin::inc_live_object_counts() {
  _live++;
}

void Ucare::ObjectCounterMixin::dec_live_object_counts() {
  _live--;
}

void Ucare::ObjectCounterMixin::set_total_object_counts(size_t total) {
  _total = total;
}

void Ucare::ObjectCounterMixin::inc_total_object_counts() {
  _total++;
}

void Ucare::ObjectCounterMixin::dec_total_object_counts() {
  _total--;
}

void Ucare::ObjectCounterMixin::add_counter(const ObjectCounterMixin* object_counter) {
  _dead += object_counter->get_dead_object_counts();
  _live += object_counter->get_live_object_counts();
  _total += object_counter->get_total_object_counts();
}

void Ucare::ObjectCounterMixin::remove_counter(const ObjectCounterMixin* object_counter) {
  _dead -= object_counter->get_dead_object_counts();
  _live -= object_counter->get_live_object_counts();
  _total -= object_counter->get_total_object_counts();
}

size_t Ucare::ObjectCounterMixin::get_dead_object_counts() const {
  return _dead;
}

size_t Ucare::ObjectCounterMixin::get_live_object_counts() const {
  return _live;
}

size_t Ucare::ObjectCounterMixin::get_total_object_counts() const {
  return _total;
}

//// TraceTimeMixin

Ucare::TraceTimeMixin::TraceTimeMixin(elapsedTimer* accumulator, bool active) {
  _active = active;
  if (_active) {
    _accumulator = accumulator;
    _t.start();
  }
}

Ucare::TraceTimeMixin::~TraceTimeMixin() {
  if (_active) {
    _t.stop();
    if (_accumulator != NULL) _accumulator->add(_t);
  }
}

//// RootTypeMixin

const char* Ucare::RootTypeMixin::get_identifier(RootType type, const char* name) {
  stringStream ss;
  ss.print("%s::%s", name, get_root_type_as_string(type));
  return ss.as_string();
}

const char* Ucare::RootTypeMixin::get_root_type_as_string(RootType type) {
  switch (type) {
    case universe:
      return "universe";
    case jni_handles:
      return "jni_handles";
    case threads:
      return "threads";
    case object_synchronizer:
      return "object_synchronizer";
    case flat_profiler:
      return "flat_profiler";
    case system_dictionary:
      return "system_dictionary";
    case class_loader_data:
      return "class_loader_data";      
    case management:
      return "management";
    case jvmti:
      return "jvmti";
    case code_cache:
      return "code_cache";
    default:
      return "unknown";
  }
}


// closures

bool Ucare::HeapIterationClosure::is_object_live(oop obj) {
  return _live_filter != NULL && _live_filter->do_object_b(obj);
}

void Ucare::HeapIterationClosure::do_object(oop obj) {
  inc_total_object_counts();
  if (is_object_live(obj)) {
    inc_live_object_counts();
  } else {
    inc_dead_object_counts();
  }
}

//// TraceAndCountRootOopClosure
Ucare::TraceAndCountRootOopClosure::~TraceAndCountRootOopClosure() {
  if (_verbose) {
    print_info();
  }
}

void Ucare::TraceAndCountRootOopClosure::print_info() {
  // no need to check verbose here
  // this call is intended by the caller

  // ucarelog_or_tty->stamp(PrintGCTimeStamps);
  ucarelog_or_tty->print_cr("%s: { root_type: \"%s\", gc_id: %u, elapsed_time: { value: %3.7f, unit: \"ms\" }, objects: { dead: %zu, live: %zu, total: %zu } }", _id, RootTypeMixin::get_root_type_as_string(_root_type), _gc_id.id(), _t.seconds() * 1000.0, get_dead_object_counts(), get_live_object_counts(), get_total_object_counts());
}

//// TraceAndCountRootOopClosureContainer
Ucare::TraceAndCountRootOopClosureContainer::~TraceAndCountRootOopClosureContainer() {
  ucarelog_or_tty->stamp(PrintGCTimeStamps);
  ucarelog_or_tty->print_cr("TraceAndCountRootOopClosureContainer: { context: \"%s\", gc_id: %u, elapsed_time: { value: %3.7f, unit: \"ms\" }, roots_walk_count: %zu, objects: { dead: %zu, live: %zu, total: %zu } }", _context, _gc_id.id(), _t.seconds() * 1000.0, _added_count, get_dead_object_counts(), get_live_object_counts(), get_total_object_counts());
}

void Ucare::TraceAndCountRootOopClosureContainer::add_counter(const TraceAndCountOopClosure* oop_closure) {
  _added_count++;
  add_counter(oop_closure);
}

void Ucare::count_objects(BoolObjectClosure* filter, const GCId& gc_id, const char* phase) {
  ResourceMark rm;

  HeapIterationClosure hic(filter);
  elapsedTimer time_execution;
  
  {
    TraceTime t("process_objects_counting", &time_execution);
    Universe::heap()->object_iterate(&hic);
  }
  
  ucarelog_or_tty->stamp(PrintGCTimeStamps);
  ucarelog_or_tty->print_cr("object_counter: { gc_id: %u, phase: \"%s\", elapsed_time: { value: %3.7f, unit: \"ms\" }, counter: { dead: %zu, live: %zu, total: %zu } }", gc_id.id(), phase, time_execution.seconds() * 1000.0, hic.get_dead_object_counts(), hic.get_live_object_counts(), hic.get_total_object_counts());
}

void Ucare::count_all_objects(const GCId& gc_id, const char* phase) {
  count_objects(&always_true, gc_id, phase);
}
