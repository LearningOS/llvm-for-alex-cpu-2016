int fuck;

struct Shit {
  int _;
  int __;
  int ___;
  int a;
  int b;
  int c;
};
struct Shit smallShit;

typedef int (*fp)();

int test_arith_logic_branch(int a, int b) {
  int c = 100;
  if (a <= b) {
    c += b;
  }
  else {
    c -= a;
  }
  for (int i = 0; i < 100; ++i) {
    c += i* c / b % a;
  }
  return a + b + c;
}

int test_shl_shr(int a, int b) {
  return (a << b) + (b >> a);
}

int test_access_global() {
  return fuck;
}

int test_recur(int x) {
  if (x <= 0) return 0;
  return test_recur(x-1) + 1;
}

int test_cmp_assign(int a, int b) {
  if (a == b) {
    return a > b;
  }
  else if (a > b) {
    return a < b;
  }
  else {
    return a == b;
  }
}

int test_get_addr(int x) {
  return (int) &x;
}


int test_array(int k) {
  int ar[100];
  for (int i = 0; i < k; ++i)
    ar[i] = k * i;
  return ar[k / 2];
}


int test_struct();
int test_function_pointer(int a) {
  fp fp1 = (fp)1234;
  fp1 = (fp)((int)fp1 + a);
  fp fp2 = (fp)test_struct;
  return fp1() + fp2();
}

int test_32bit_int() {
  return 0x80000001;
}

int test_struct() {
  smallShit.b = 3;
  return smallShit.a;
};

int test_param_struct(struct Shit shit) {
  return shit.a;
}
void test_param_struct1() {
  test_param_struct(smallShit);
}

struct Shit test_return_struct() {
  return smallShit;
}
int test_return_shit1() {
  int a = test_return_struct().a;
  int m = test_return_struct().b;
  return a + m;
}

int test_quest(int a, int b) {
  return a > b ? a : b;
}

int test_switch(int a, int *ary) {
  switch(a) {
    case 1:
      return ary[0];
    case 2:
      ary[1] += 1; break;
    case 3:
      ary[2] += 1;
    case 4:
      return ary[3];
    default:
      return 0;
  }
  return -1;
}

int main() {
  return 0;
}
void _start() {
}
