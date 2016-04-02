
#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUSUBTARGET_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUSUBTARGET_H

#include "V9CpuFrameLowering.h"
#include "V9CpuISelLowering.h"
#include "V9CpuRegisterInfo.h"
#include "V9CpuInstrInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/MC/MCInstrItineraries.h"
#include "llvm/Target/TargetSubtargetInfo.h"
//#include "llvm/Target/TargetSelectionDAGInfo.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "V9CpuGenSubtargetInfo.inc"

//@1
namespace llvm {
    class StringRef;

    class V9CpuTargetMachine;

    class V9CpuSubtarget : public V9CpuGenSubtargetInfo {
        virtual void anchor();

    protected:
        enum V9CpuArchEnum {
            V9CpuI
        };

        // V9Cpu architecture version
        V9CpuArchEnum V9CpuArchVersion;

        InstrItineraryData InstrItins;


        const V9CpuTargetMachine &TM;

        Triple TargetTriple;

        //TargetSelectionDAGInfo TSInfo;

        std::unique_ptr<const V9CpuInstrInfo> InstrInfo;
        std::unique_ptr<const V9CpuFrameLowering> FrameLowering;
        std::unique_ptr<const V9CpuTargetLowering> TLInfo;

    public:
        const V9CpuABIInfo &getABI() const;
        unsigned stackAlignment() const { return 8; }
        /// This constructor initializes the data members to match that
        /// of the specified triple.
        V9CpuSubtarget(const Triple &TT, const std::string &CPU, const std::string &FS,
                      bool little, const V9CpuTargetMachine &_TM);

//        const TargetSelectionDAGInfo *getSelectionDAGInfo() const override {
//            return &TSInfo;
//        }
        const V9CpuInstrInfo *getInstrInfo() const override { return InstrInfo.get(); }
//        const TargetFrameLowering *getFrameLowering() const override {
//            return FrameLowering.get();
//        }
        const V9CpuRegisterInfo *getRegisterInfo() const override {
            return &InstrInfo->getRegisterInfo();
        }
        const V9CpuTargetLowering *getTargetLowering() const override {
            return TLInfo.get();
        }
        const InstrItineraryData *getInstrItineraryData() const override {
            return &InstrItins;
        }
    };
} // End llvm namespace


#endif