#include "organization.cpp"

// http://www.parashift.com/c%2B%2B-faq-lite/templates.html#faq-35.13
//
// Template implementations screw up the linker; this is a workaround.

template int32_t Memory<1024>::load(uint32_t index);
template void Memory<1024>::store(uint32_t index, int32_t param);
