#include "V9CpuMCAsmInfo.h"
#include "llvm/ADT/Triple.h"

using namespace llvm;

void V9CpuMCAsmInfo::anchor() { }

V9CpuMCAsmInfo::V9CpuMCAsmInfo(const Triple &TheTriple) {
    if ((TheTriple.getArch() == Triple::v9cpu))
        IsLittleEndian = false; // the default of IsLittleEndian is true

    AlignmentIsInBytes          = false;
    Data16bitsDirective         = "\t.2byte\t";
    Data32bitsDirective         = "\t.4byte\t";
    Data64bitsDirective         = "\t.8byte\t";
    PrivateGlobalPrefix         = "$";
// PrivateLabelPrefix: display $BB for the labels of basic block
    PrivateLabelPrefix          = "$";
    CommentString               = "#";
    ZeroDirective               = "\t.space\t";
    GPRel32Directive            = "\t.gpword\t";
    GPRel64Directive            = "\t.gpdword\t";
    WeakRefDirective            = "\t.weak\t";
    UseAssignmentForEHBegin = true;

    SupportsDebugInformation = true;
    ExceptionsType = ExceptionHandling::DwarfCFI;
    DwarfRegNumForCFI = true;
}
