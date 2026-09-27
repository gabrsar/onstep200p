#include "../config/LxDate.h"
#include <cassert>
int main() {
  LxDate d;
  assert(parseLxDate("09/25/2026",d) && d.year==2026 && d.month==9 && d.day==25);
  assert(parseLxDate("09/25/26",d) && d.year==2026);
  assert(parseLxDate("02/29/2000",d));
  assert(!parseLxDate("02/29/2100",d));
  assert(!parseLxDate("02/30/2026",d));
  assert(!parseLxDate("09/25/2026x",d));
  assert(!parseLxDate("00/25/2026",d));
}
