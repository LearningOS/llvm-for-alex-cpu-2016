; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mcpu=bdver1 | FileCheck %s
; Verify that for the architectures that are known to have poor latency
; double precision shift instructions we generate alternative sequence 
; of instructions with lower latencies instead of shrd instruction.

;uint64_t rshift1(uint64_t a, uint64_t b)
;{
;    return (a >> 1) | (b << 63);
;}

; CHECK:             rshift1:
; CHECK:             shrq    {{.*}}
; CHECK-NEXT:        shlq    $63, {{.*}}
; CHECK-NEXT:        leaq    ({{.*}},{{.*}}), {{.*}}

define i64 @rshift1(i64 %a, i64 %b) nounwind readnone uwtable {
  %1 = lshr i64 %a, 1
  %2 = shl i64 %b, 63
  %3 = or i64 %2, %1
  ret i64 %3
}

;uint64_t rshift2(uint64_t a, uint64_t b)
;{
;    return (a >> 2) | (b << 62);
;}

; CHECK:             rshift2:
; CHECK:             shrq    $2, {{.*}}
; CHECK-NEXT:        shlq    $62, {{.*}}
; CHECK-NEXT:        leaq    ({{.*}},{{.*}}), {{.*}}


define i64 @rshift2(i64 %a, i64 %b) nounwind readnone uwtable {
  %1 = lshr i64 %a, 2
  %2 = shl i64 %b, 62
  %3 = or i64 %2, %1
  ret i64 %3
}

;uint64_t rshift7(uint64_t a, uint64_t b)
;{
;    return (a >> 7) | (b << 57);
;}

; CHECK:             rshift7:
; CHECK:             shrq    $7, {{.*}}
; CHECK-NEXT:        shlq    $57, {{.*}}
; CHECK-NEXT:        leaq    ({{.*}},{{.*}}), {{.*}}


define i64 @rshift7(i64 %a, i64 %b) nounwind readnone uwtable {
  %1 = lshr i64 %a, 7
  %2 = shl i64 %b, 57
  %3 = or i64 %2, %1
  ret i64 %3
}

;uint64_t rshift63(uint64_t a, uint64_t b)
;{
;    return (a >> 63) | (b << 1);
;}

; CHECK-LABEL:       rshift63:
; CHECK:             shrq    $63, %rdi
; CHECK-NEXT:        leaq    (%rdi,%rsi,2), %rax

define i64 @rshift63(i64 %a, i64 %b) nounwind readnone uwtable {
  %1 = lshr i64 %a, 63
  %2 = shl i64 %b, 1
  %3 = or i64 %2, %1
  ret i64 %3
}
