#pragma once

namespace krc {

// helper for the deeply unhelpful signature of makecontext, enabling only int arguments to the target function

void from_ptr(void *p, int &p1, int &ptr_as_intsp2);

void *to_ptr(int p1, int p2);

}
