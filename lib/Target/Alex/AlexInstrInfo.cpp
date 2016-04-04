#include "AlexInstrInfo.h"

using namespace llvm;

class AlexSubtarget;

AlexInstrInfo::AlexInstrInfo(const AlexSubtarget *subtarget)
        :subtarget(subtarget) {

}