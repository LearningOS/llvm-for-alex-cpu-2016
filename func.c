extern int fuck;

struct Shit {
    int a;
    int b;
};
struct Shit smallShit;

int func(int a, int b) {
  int c =100;
  if (a <= b) {
    c += b;
  }
  else {
    c += a;
  }
  for (int i = 0; i < 100; ++i) {
    c += i*c;
  }
  return a + b +c;
}

int shl_shr(int a, int b) {
  return (a << b) + (b >> a);
}

int access_global() {
  return fuck;
}

int test_struct() {
  return smallShit.a;
};

int sum(int x) {
    return sum(x-1) + 1;
}