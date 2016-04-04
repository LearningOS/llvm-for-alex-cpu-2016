#include "AlexFrameLowering.h"
using namespace llvm;

AlexFrameLowering::AlexFrameLowering(const AlexSubtarget *sti)
        : TargetFrameLowering(
            StackGrowsDown, AlexStackAlignment, 0, AlexStackAlignment),
            subtarget(sti) {
}