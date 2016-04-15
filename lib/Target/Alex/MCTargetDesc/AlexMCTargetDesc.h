
#ifndef LLVM_LIB_TARGET_ALEX_MCTARGETDESC_ALEXMCTARGETDESC_H
#define LLVM_LIB_TARGET_ALEX_MCTARGETDESC_ALEXMCTARGETDESC_H

//#include "AlexConfig.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {
    class MCAsmBackend;
    class MCCodeEmitter;
    class MCContext;
    class MCInstrInfo;
    class MCObjectWriter;
    class MCRegisterInfo;
    class MCSubtargetInfo;
    class StringRef;
    class Target;
    class Triple;
    class raw_ostream;
    class raw_pwrite_stream;

    extern Target TheAlexTarget;
    extern Target TheAlexelTarget;

    MCCodeEmitter *createAlexMCCodeEmitterEL(const MCInstrInfo &MCII,
                                             const MCRegisterInfo &MRI,
                                             MCContext &Context);

    MCAsmBackend *createAlexAsmBackendEL32(const Target &T,
                                           const MCRegisterInfo &MRI,
                                           const Triple &TT, StringRef CPU);

    MCObjectWriter *createAlexELFObjectWriter(raw_pwrite_stream &OS,
                                              uint8_t OSABI,
                                              bool IsLittleEndian);
} // End llvm namespace

// Defines symbolic names for Alex registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "AlexGenRegisterInfo.inc"

// Defines symbolic names for the Alex instructions.
#define GET_INSTRINFO_ENUM
#include "AlexGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "AlexGenSubtargetInfo.inc"

#endif