#include <vector>
typedef void (*menuFunction)();

struct menuItem {
  String caption;
  menuItem *children[10];
  menuFunction function;
  menuItem *parent;
};

struct pattern {
  String caption;
  std::vector<float> steps;
};