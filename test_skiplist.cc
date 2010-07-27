#include <limits.h>
#include <stdint.h>
#include <map>
#include "skiplist.hpp"


int main(void){
  lockfree::skiplist<uint32_t,uint32_t> sl(0,INT_MAX);
  assert(sl.find(3) == sl.end());
  sl.insert(std::make_pair(3,4));
  assert(sl.find(3)->second == 4);
  return 0;
}
