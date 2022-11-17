typedef void (*menuFunction)();

struct menuItem {
  String caption;
  menuItem *children[10];
  menuFunction function;
};