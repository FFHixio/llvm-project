// RUN: %clang_cc1 -fexperimental-strict-floating-point -DEXCEPT=1 -fcxx-exceptions -triple x86_64-linux-gnu -emit-llvm -o - %s | FileCheck -check-prefix=CHECK-NS %s
// RUN: %clang_cc1 -fexperimental-strict-floating-point -triple x86_64-linux-gnu -emit-llvm -o - %s | FileCheck %s -check-prefix=CHECK-DEFAULT
// RUN: %clang_cc1 -fexperimental-strict-floating-point -DFENV_ON=1 -triple x86_64-linux-gnu -emit-llvm -o - %s | FileCheck -check-prefix=CHECK-FENV %s
// RUN: %clang_cc1 -fexperimental-strict-floating-point -DNF128 -triple %itanium_abi_triple -O3 -emit-llvm -o - %s | FileCheck -check-prefix=CHECK-O3 %s
// RUN: %clang_cc1 -fexperimental-strict-floating-point -triple x86_64-linux-gnu -emit-llvm -o - %s -ffp-eval-method=source | FileCheck %s -check-prefix=CHECK-SOURCE
// RUN: %clang_cc1 -fexperimental-strict-floating-point -triple x86_64-linux-gnu -emit-llvm -o - %s -ffp-eval-method=double | FileCheck %s -check-prefix=CHECK-DOUBLE
// RUN: %clang_cc1 -fexperimental-strict-floating-point -triple x86_64-linux-gnu -emit-llvm -o - %s -ffp-eval-method=extended -mlong-double-80 | FileCheck %s -check-prefix=CHECK-EXTENDED
// RUN: %clang_cc1 -fexperimental-strict-floating-point -triple i386-linux-gnu -emit-llvm -o - %s -ffp-eval-method=source | FileCheck %s -check-prefix=CHECK-SOURCE
// RUN: %clang_cc1 -fexperimental-strict-floating-point -triple i386-linux-gnu -emit-llvm -o - %s -ffp-eval-method=double | FileCheck %s -check-prefix=CHECK-DOUBLE
// RUN: %clang_cc1 -fexperimental-strict-floating-point -triple i386-linux-gnu -emit-llvm -o - %s -ffp-eval-method=extended -mlong-double-80 | FileCheck %s -check-prefix=CHECK-EXTENDED
// RUN: %clang_cc1 -triple powerpc-unknown-aix -DNF128 -emit-llvm -o - %s | FileCheck %s -check-prefix=CHECK-AIX

// Verify float_control(precise, off) enables fast math flags on fp operations.
float fp_precise_1(float a, float b, float c) {
// CHECK-O3: _Z12fp_precise_1fff
// CHECK-O3: %[[M:.+]] = fmul fast float{{.*}}
// CHECK-O3: fadd fast float %[[M]], %c
#pragma float_control(precise, off)
  return a * b + c;
}

// Is float_control state cleared on exiting compound statements?
float fp_precise_2(float a, float b, float c) {
  // CHECK-O3: _Z12fp_precise_2fff
  // CHECK-O3: %[[M:.+]] = fmul float{{.*}}
  // CHECK-O3: fadd float %[[M]], %c
  {
#pragma float_control(precise, off)
  }
  return a * b + c;
}

// Does float_control survive template instantiation?
class Foo {};
Foo operator+(Foo, Foo);

template <typename T>
T template_muladd(T a, T b, T c) {
#pragma float_control(precise, off)
  return a * b + c;
}

float fp_precise_3(float a, float b, float c) {
  // CHECK-O3: _Z12fp_precise_3fff
  // CHECK-O3: %[[M:.+]] = fmul fast float{{.*}}
  // CHECK-O3: fadd fast float %[[M]], %c
  return template_muladd<float>(a, b, c);
}

template <typename T>
class fp_precise_4 {
  float method(float a, float b, float c) {
#pragma float_control(precise, off)
    return a * b + c;
  }
};

template class fp_precise_4<int>;
// CHECK-O3: _ZN12fp_precise_4IiE6methodEfff
// CHECK-O3: %[[M:.+]] = fmul fast float{{.*}}
// CHECK-O3: fadd fast float %[[M]], %c

// Check file-scoped float_control
#pragma float_control(push)
#pragma float_control(precise, off)
float fp_precise_5(float a, float b, float c) {
  // CHECK-O3: _Z12fp_precise_5fff
  // CHECK-O3: %[[M:.+]] = fmul fast float{{.*}}
  // CHECK-O3: fadd fast float %[[M]], %c
  return a * b + c;
}
#pragma float_control(pop)

float fff(float x, float y) {
// CHECK-LABEL: define{{.*}} float @_Z3fffff{{.*}}
// CHECK: entry
#pragma float_control(except, on)
  float z;
  z = z * z;
  //CHECK: llvm.experimental.constrained.fmul{{.*}}
  {
    z = x * y;
    //CHECK: llvm.experimental.constrained.fmul{{.*}}
  }
  {
// This pragma has no effect since if there are any fp intrin in the
// function then all the operations need to be fp intrin
#pragma float_control(except, off)
    z = z + x * y;
    //CHECK: llvm.experimental.constrained.fmul{{.*}}
  }
  z = z * z;
  //CHECK: llvm.experimental.constrained.fmul{{.*}}
  return z;
}
float check_precise(float x, float y) {
  // CHECK-LABEL: define{{.*}} float @_Z13check_preciseff{{.*}}
  float z;
  {
#pragma float_control(precise, on)
    z = x * y + z;
    //CHECK: llvm.fmuladd{{.*}}
  }
  {
#pragma float_control(precise, off)
    z = x * y + z;
    //CHECK: fmul fast float
    //CHECK: fadd fast float
  }
  return z;
}

float fma_test2(float a, float b, float c) {
// CHECK-LABEL define{{.*}} float @_Z9fma_test2fff{{.*}}
#pragma float_control(precise, off)
  float x = a * b + c;
  //CHECK: fmuladd
  return x;
}

float fma_test1(float a, float b, float c) {
// CHECK-LABEL define{{.*}} float @_Z9fma_test1fff{{.*}}
#pragma float_control(precise, on)
  float x = a * b + c;
  //CHECK: fmuladd
  return x;
}

#pragma float_control(push)
#pragma float_control(precise, on)
struct Distance {};
Distance operator+(Distance, Distance);

template <class T>
T add(T lhs, T rhs) {
#pragma float_control(except, on)
  return lhs + rhs;
}
#pragma float_control(pop)

float test_OperatorCall() {
  return add(1.0f, 2.0f);
  //CHECK: llvm.experimental.constrained.fadd{{.*}}fpexcept.strict
}
// CHECK-LABEL define{{.*}} float  {{.*}}test_OperatorCall{{.*}}

#if FENV_ON
#pragma STDC FENV_ACCESS ON
#endif
// CHECK-LABEL: define {{.*}}callt{{.*}}

void callt() {
  volatile float z;
  z = z * z;
//CHECK-FENV: llvm.experimental.constrained.fmul{{.*}}
}

#if EXCEPT
namespace ns {
// Check that pragma float_control can appear in namespace.
#pragma float_control(except, on, push)
float exc_on(double x, float zero) {
// CHECK-NS: define {{.*}}exc_on{{.*}}
  {} try {
    x = 1.0 / zero; /* division by zero, the result unused */
//CHECK-NS: llvm.experimental.constrained.fdiv{{.*}}
  } catch (...) {}
  return zero;
}
}

// Check pragma is still effective after namespace closes
float exc_still_on(double x, float zero) {
// CHECK-NS: define {{.*}}exc_still_on{{.*}}
  {} try {
    x = 1.0 / zero; /* division by zero, the result unused */
//CHECK-NS: llvm.experimental.constrained.fdiv{{.*}}
  } catch (...) {}
  return zero;
}

#pragma float_control(pop)
float exc_off(double x, float zero) {
// CHECK-NS: define {{.*}}exc_off{{.*}}
  {} try {
    x = 1.0 / zero; /* division by zero, the result unused */
//CHECK-NS: fdiv double
  } catch (...) {}
  return zero;
}

namespace fc_template_namespace {
#pragma float_control(except, on, push)
template <class T>
T exc_on(double x, T zero) {
// CHECK-NS: define {{.*}}fc_template_namespace{{.*}}
  {} try {
    x = 1.0 / zero; /* division by zero, the result unused */
//CHECK-NS: llvm.experimental.constrained.fdiv{{.*}}
  } catch (...) {}
  return zero;
}
}

#pragma float_control(pop)
float xx(double x, float z) {
  return fc_template_namespace::exc_on<float>(x, z);
}
#endif // EXCEPT

float try_lam(float x, unsigned n) {
// CHECK: define {{.*}}try_lam{{.*}}class.anon{{.*}}
  float result;
  auto t =
        // Lambda expression begins
        [](float a, float b) {
#pragma float_control( except, on)
            return a * b;
//CHECK: llvm.experimental.constrained.fmul{{.*}}fpexcept.strict
        } // end of lambda expression
  (1.0f,2.0f);
  result = x + t;
  return result;
}

float mySub(float x, float y) {
  // CHECK: define {{.*}}float {{.*}}mySub{{.*}}
  // CHECK-NS: fsub float
  // CHECK-SOURCE: fsub float
  // CHECK-DOUBLE: fpext float
  // CHECK-DOUBLE: fpext float
  // CHECK-DOUBLE: fsub double
  // CHECK-DOUBLE: fptrunc double {{.*}} to float
  // CHECK-EXTENDED: fpext float
  // CHECK-EXTENDED: fpext float
  // CHECK-EXTENDED: fsub double
  // CHECK-EXTENDED: fptrunc double {{.*}} to float
  return x - y;
}

float mySubSource(float x, float y) {
// CHECK: define {{.*}}float {{.*}}mySubSource{{.*}}
#pragma float_control(source)
  return x - y;
  // CHECK: fsub float
}

float mySubExtended(float x, float y) {
// CHECK: define {{.*}}float {{.*}}mySubExtended{{.*}}
#pragma float_control(extended)
  return x - y;
  // CHECK: fpext float
  // CHECK: fpext float
  // CHECK: fsub x86_fp80
  // CHECK: fptrunc x86_fp80 {{.*}} to float
}

float mySubDouble(float x, float y) {
// CHECK: define {{.*}}float {{.*}}mySubDouble{{.*}}
#pragma float_control(double)
  return x - y;
  // CHECK: fpext float
  // CHECK: fpext float
  // CHECK: fsub double
  // CHECK: fptrunc double {{.*}} to float
}

#ifndef NF128
__float128 mySub128(__float128 x, __float128 y) {
  // CHECK: define {{.*}}mySub128{{.*}}
  // Expect no fpext since fp128 is already widest
  // CHECK: load fp128
  // CHECK-NEXT: load fp128
  // CHECK-NEXT: fsub fp128
  // CHECK-NEXT: ret fp128
  return x - y;
}
#endif

void mySubfp16(__fp16 *res, __fp16 *x, __fp16 *y) {
  // CHECK: define {{.*}}mySubfp16{{.*}}
  *res = *x - *y;
  // CHECK: load half
  // CHECK-NEXT: load half
  // CHECK-NEXT: fpext half{{.*}}
  // CHECK-NEXT: load half
  // CHECK-NEXT: load half
  // CHECK-NS: fpext half{{.*}} to float
  // CHECK-DEFAULT: fpext half{{.*}} to float
  // CHECK-DOUBLE: fpext half{{.*}} to double
  // CHECK-EXTENDED: fpext half{{.*}} to x86_fp80
  // CHECK-NEXT: fsub
  // CHECK-NEXT: fptrunc {{.*}}to half
  // CHECK-NS: fptrunc float {{.*}} to half
  // CHECK-DOUBLE: fptrunc double {{.*}} to half
  // CHECK-EXTENDED: fptrunc x86_fp80 {{.*}} to half
}

int getFEM() {
  // CHECK: define {{.*}}getFEM{{.*}}
  return __FLT_EVAL_METHOD__;
  // CHECK-NS: ret {{.*}} 0
  // CHECK-AIX: ret {{.*}} 1
  // CHECK-SOURCE: ret {{.*}} 0
  // CHECK-DOUBLE: ret {{.*}} 1
  // CHECK-EXTENDED: ret {{.*}} 2
}
