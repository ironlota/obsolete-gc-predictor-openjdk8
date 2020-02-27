#include "precompiled.hpp"

#include "utilities/ucare.hpp"
#include "gc_implementation/shared/ucare.gc.inline.hpp"

#if INCLUDE_ALL_GCS

// --------------------------------------------------
// PSParallelCompact
// --------------------------------------------------
void Ucare::MarkAndPushClosure::do_oop(oop* p) {
  mark_and_push(this, _compaction_manager, p);
}

void Ucare::MarkAndPushClosure::do_oop(narrowOop* p) {
  mark_and_push(this, _compaction_manager, p);
}

void Ucare::FollowKlassClosure::do_klass(Klass* klass) {
  klass->oops_do(_mark_and_push_closure);
}

#endif
