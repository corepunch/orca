#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <source/ios_scroll_policy.h>

int main(void)
{
  const double normal = 0.998;
  const double longer = ORCA_IOS_ScrollRetention(normal, 1.5);
  assert(longer > normal && longer < 1);
  // Compare projected free travel for several release speeds, both directions.
  for (int velocity = -6000; velocity <= 6000; velocity += 500) {
    double original = velocity / 1000.0 * normal / (1 - normal);
    double extended = velocity / 1000.0 * longer / (1 - longer);
    assert(fabs(extended - original * 1.5) < 1e-6);
  }
  assert(ORCA_IOS_ScrollRetention(normal, 1) == normal);
  assert(ORCA_IOS_ScrollRetention(normal, 0) == normal);
  assert(ORCA_IOS_ScrollOffset(-420) == 420);
  assert(ORCA_IOS_ScrollOffset(32) == -32); // rubber-band overdrag is preserved
  assert(ORCA_IOS_ScrollExtent(1800, 600) == 1200);
  assert(ORCA_IOS_ScrollExtent(300, 600) == 0);
  assert(ORCA_IOS_ScrollExtent(NAN, 600) == 0);
  puts("PASS: native scroll free distance is 150% without changing release velocity");
  return 0;
}
