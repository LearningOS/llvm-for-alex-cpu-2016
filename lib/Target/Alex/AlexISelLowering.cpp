#include "AlexTargetMachine.h"
#include "AlexISelLowering.h"
#include "AlexISelDAGToDAG.h"
#include "AlexRegisterInfo.h"
#include "AlexMachineFunction.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "Alex-lower"
#include "AlexGenCallingConv.inc"
#include "AlexTargetObjectFile.h"

const char *AlexTargetLowering::getTargetNodeName(unsigned Opcode) const {
    switch (Opcode) {
        case AlexISD::JmpLink:           return "AlexISD::JmpLink";
        case AlexISD::TailCall:          return "AlexISD::TailCall";
        case AlexISD::Hi:                return "AlexISD::Hi";
        case AlexISD::Lo:                return "AlexISD::Lo";
        case AlexISD::GPRel:             return "AlexISD::GPRel";
        case AlexISD::Ret:               return "AlexISD::Ret";
        case AlexISD::EH_RETURN:         return "AlexISD::EH_RETURN";
        case AlexISD::DivRem:            return "AlexISD::DivRem";
        case AlexISD::DivRemU:           return "AlexISD::DivRemU";
        case AlexISD::Wrapper:           return "AlexISD::Wrapper";
        default:                         return NULL;
    }
}

AlexTargetLowering::AlexTargetLowering(const AlexTargetMachine *targetMachine,
                                         const AlexSubtarget *subtarget,
                                       const AlexRegisterInfo* registerInfo)
        : TargetLowering(*targetMachine), subtarget(subtarget) {
    // disable dag nodes here
    setOperationAction(ISD::BR_CC, MVT::i32, Expand);
    //setOperationAction(ISD::SETCC, MVT::i32, Expand);
    //setOperationAction(ISD::SELECT_CC, MVT::i32, Expand);
    setOperationAction(ISD::GlobalAddress,      MVT::i32,   Custom);
    addRegisterClass(MVT::i32, &Alex::Int32RegsRegClass);
    computeRegisterProperties(registerInfo);
}
SDValue AlexTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const
{
    switch (Op.getOpcode())
    {
      //  case ISD::BRCOND:             return lowerBRCOND(Op, DAG);
        case ISD::GlobalAddress:      return lowerGlobalAddress(Op, DAG);
      //  case ISD::BlockAddress:       return lowerBlockAddress(Op, DAG);
       // case ISD::JumpTable:          return lowerJumpTable(Op, DAG);
       // case ISD::SELECT:             return lowerSELECT(Op, DAG);
    }
    return SDValue();
}

SDValue AlexTargetLowering::getGlobalReg(SelectionDAG &DAG, EVT Ty) const {
    AlexFunctionInfo *FI = DAG.getMachineFunction().getInfo<AlexFunctionInfo>();
    return DAG.getRegister(Alex::PC, Ty);
}

//@getTargetNode(GlobalAddressSDNode
SDValue AlexTargetLowering::getTargetNode(GlobalAddressSDNode *N, EVT Ty,
                                          SelectionDAG &DAG,
                                          unsigned Flag) const {
    return DAG.getTargetGlobalAddress(N->getGlobal(), SDLoc(N), Ty, 0);
}

//@getTargetNode(ExternalSymbolSDNode
SDValue AlexTargetLowering::getTargetNode(ExternalSymbolSDNode *N, EVT Ty,
                                          SelectionDAG &DAG,
                                          unsigned Flag) const {
    return DAG.getTargetExternalSymbol(N->getSymbol(), Ty);
}

template<class NodeTy>
SDValue AlexTargetLowering::getAddrGlobal(NodeTy *N, EVT Ty, SelectionDAG &DAG,
                      unsigned Flag, SDValue Chain,
                      const MachinePointerInfo &PtrInfo) const {
    SDLoc DL(N);
    SDValue Tgt = DAG.getNode(AlexISD::Wrapper, DL, Ty, getGlobalReg(DAG, Ty),
                              getTargetNode(N, Ty, DAG, Flag));
    return DAG.getLoad(Ty, DL, Chain, Tgt, PtrInfo, false, false, false, 0);
}
SDValue AlexTargetLowering::lowerGlobalAddress(SDValue Op,
                                               SelectionDAG &DAG) const {
    //@lowerGlobalAddress }
    SDLoc DL(Op);
    const AlexTargetObjectFile *TLOF =
            static_cast<const AlexTargetObjectFile *>(getTargetMachine().getObjFileLowering());
    //@lga 1 {
    EVT Ty = Op.getValueType();
    GlobalAddressSDNode *N = cast<GlobalAddressSDNode>(Op);
    const GlobalValue *GV = N->getGlobal();
    //@lga 1 }

    /*if (getTargetMachine().getRelocationModel() != Reloc::PIC_) {
        //@ %gp_rel relocation
        if (TLOF->IsGlobalInSmallSection(GV, getTargetMachine())) {
            SDValue GA = DAG.getTargetGlobalAddress(GV, DL, MVT::i32, 0,
                                                    AlexII::MO_GPREL);
            SDValue GPRelNode = DAG.getNode(AlexISD::GPRel, DL,
                                            DAG.getVTList(MVT::i32), GA);
            SDValue GPReg = DAG.getRegister(Alex::GP, MVT::i32);
            return DAG.getNode(ISD::ADD, DL, MVT::i32, GPReg, GPRelNode);
        }

        //@ %hi/%lo relocation
        return getAddrNonPIC(N, Ty, DAG);
    }*/

    //if (GV->hasInternalLinkage() || (GV->hasLocalLinkage() && !isa<Function>(GV)))
    //    return getAddrLocal(N, Ty, DAG);

    //@large section
    //if (!TLOF->IsGlobalInSmallSection(GV, getTargetMachine()))
    //    return getAddrGlobalLargeGOT(N, Ty, DAG, AlexII::MO_GOT_HI16,
    //                                 AlexII::MO_GOT_LO16, DAG.getEntryNode(),
    //                                 MachinePointerInfo::getGOT());
    MachineFunction &MF = DAG.getMachineFunction();
    return getAddrGlobal(N, Ty, DAG, 0, DAG.getEntryNode(), MachinePointerInfo::getGOT(MF));
}

static unsigned
addLiveIn(MachineFunction &MF, unsigned PReg, const TargetRegisterClass *RC)
{
    unsigned VReg = MF.getRegInfo().createVirtualRegister(RC);
    MF.getRegInfo().addLiveIn(PReg, VReg);
    return VReg;
}
/*
void AlexTargetLowering::copyByValRegs(SDValue Chain, SDLoc DL, std::vector<SDValue> &OutChains,
              SelectionDAG &DAG, const ISD::ArgFlagsTy &Flags,
              SmallVectorImpl<SDValue> &InVals, const Argument *FuncArg,
              const AlexCC &CC, const ByValArgInfo &ByVal) const {
    MachineFunction &MF = DAG.getMachineFunction();
    MachineFrameInfo *MFI = MF.getFrameInfo();
    unsigned RegAreaSize = ByVal.NumRegs * CC.regSize();
    unsigned FrameObjSize = std::max(Flags.getByValSize(), RegAreaSize);
    int FrameObjOffset;

    const ArrayRef<MCPhysReg> ByValArgRegs = CC.intArgRegs();

    if (RegAreaSize)
        FrameObjOffset = (int)CC.reservedArgArea() -
                         (int)((CC.numIntArgRegs() - ByVal.FirstIdx) * CC.regSize());
    else
        FrameObjOffset = ByVal.Address;

    // Create frame object.
    EVT PtrTy = getPointerTy(DAG.getDataLayout());
    int FI = MFI->CreateFixedObject(FrameObjSize, FrameObjOffset, true);
    SDValue FIN = DAG.getFrameIndex(FI, PtrTy);
    InVals.push_back(FIN);

    if (!ByVal.NumRegs)
        return;

    // Copy arg registers.
    MVT RegTy = MVT::getIntegerVT(CC.regSize() * 8);
    const TargetRegisterClass *RC = getRegClassFor(RegTy);

    for (unsigned I = 0; I < ByVal.NumRegs; ++I) {
        unsigned ArgReg = ByValArgRegs[ByVal.FirstIdx + I];
        unsigned VReg = addLiveIn(MF, ArgReg, RC);
        unsigned Offset = I * CC.regSize();
        SDValue StorePtr = DAG.getNode(ISD::ADD, DL, PtrTy, FIN,
                                       DAG.getConstant(Offset, DL, PtrTy));
        SDValue Store = DAG.getStore(Chain, DL, DAG.getRegister(VReg, RegTy),
                                     StorePtr, MachinePointerInfo(FuncArg, Offset),
                                     false, false, 0);
        OutChains.push_back(Store);
    }
}*/

SDValue AlexTargetLowering::LowerFormalArguments(SDValue chain, CallingConv::ID CallConv, bool IsVarArg,
                                                 const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc dl, SelectionDAG &dag,
                                                 SmallVectorImpl<SDValue> &InVals) const {
    MachineFunction &MF = dag.getMachineFunction();
    MachineFrameInfo *MFI = MF.getFrameInfo();
    AlexFunctionInfo *AlexFI = MF.getInfo<AlexFunctionInfo>();

    AlexFI->setVarArgsFrameIndex(0);

    // Assign locations to all of the incoming arguments.
    SmallVector<CCValAssign, 16> ArgLocs;
    CCState CCInfo(CallConv, IsVarArg, dag.getMachineFunction(),
                   ArgLocs, *dag.getContext());
    AlexCC AlexCCInfo(CallConv, false,
                      CCInfo);
    AlexFI->setFormalArgInfo(CCInfo.getNextStackOffset(),
                             AlexCCInfo.hasByValArg());

    Function::const_arg_iterator FuncArg =
            dag.getMachineFunction().getFunction()->arg_begin();
    bool UseSoftFloat = false;

    AlexCCInfo.analyzeFormalArguments(Ins, UseSoftFloat, FuncArg);

    // Used with vargs to acumulate store chains.
    std::vector<SDValue> OutChains;

    unsigned CurArgIdx = 0;
    AlexCC::byval_iterator ByValArg = AlexCCInfo.byval_begin();

    //@2 {
    for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
        //@2 }
        CCValAssign &VA = ArgLocs[i];
        std::advance(FuncArg, Ins[i].OrigArgIndex - CurArgIdx);
        CurArgIdx = Ins[i].OrigArgIndex;
        EVT ValVT = VA.getValVT();
        ISD::ArgFlagsTy Flags = Ins[i].Flags;
        bool IsRegLoc = VA.isRegLoc();

        //@byval pass {
        if (Flags.isByVal()) {
            assert(false);
            assert(Flags.getByValSize() &&
                   "ByVal args of size 0 should have been ignored by front-end.");
            assert(ByValArg != AlexCCInfo.byval_end());
            //copyByValRegs(chain, dl, OutChains, dag, Flags, InVals, &*FuncArg,
            //              AlexCCInfo, *ByValArg);
            ++ByValArg;
            continue;
        }
        //@byval pass }
        // Arguments stored on registers
        // sanity check
        assert(VA.isMemLoc());

        // The stack pointer offset is relative to the caller stack frame.
        int FI = MFI->CreateFixedObject(ValVT.getSizeInBits()/8,
                                        VA.getLocMemOffset(), true);

        // Create load nodes to retrieve arguments from the stack
        SDValue FIN = dag.getFrameIndex(FI, getPointerTy(dag.getDataLayout()));
        SDValue Load = dag.getLoad(ValVT, dl, chain, FIN,
                                   MachinePointerInfo::getFixedStack(MF, FI, 0),
                                   false, false, false, 0);
        InVals.push_back(Load);
        OutChains.push_back(Load.getValue(1));
    }

    // All stores are grouped in one node to allow the matching between
    // the size of Ins and InVals. This only happens when on varg functions
    if (!OutChains.empty()) {
        OutChains.push_back(chain);
        chain = dag.getNode(ISD::TokenFactor, dl, MVT::Other, OutChains);
    }
    return chain;
}
void AlexTargetLowering::AlexCC::handleByValArg(unsigned ValNo, MVT ValVT,
                                                MVT LocVT,
                                                CCValAssign::LocInfo LocInfo,
                                                ISD::ArgFlagsTy ArgFlags) {
    assert(ArgFlags.getByValSize() && "Byval argument's size shouldn't be 0.");

    struct ByValArgInfo ByVal;
    unsigned RegSize = regSize();
    unsigned ByValSize = ArgFlags.getByValSize();//RoundUpToAlignment(, RegSize);
    unsigned Align = std::min(std::max(ArgFlags.getByValAlign(), RegSize),
                              RegSize * 2);

    // Allocate space on caller's stack.
    ByVal.Address = CCInfo.AllocateStack(ByValSize - RegSize * ByVal.NumRegs,
                                         Align);
    CCInfo.addLoc(CCValAssign::getMem(ValNo, ValVT, ByVal.Address, LocVT,
                                      LocInfo));
    ByValArgs.push_back(ByVal);
}

void AlexTargetLowering::AlexCC::analyzeFormalArguments(const SmallVectorImpl<ISD::InputArg> &Args,
                       bool IsSoftFloat, Function::const_arg_iterator FuncArg) {
    unsigned NumArgs = Args.size();
    llvm::CCAssignFn *FixedFn = CC_Alex;
    unsigned CurArgIdx = 0;

    for (unsigned I = 0; I != NumArgs; ++I) {
        MVT ArgVT = Args[I].VT;
        ISD::ArgFlagsTy ArgFlags = Args[I].Flags;
        std::advance(FuncArg, Args[I].OrigArgIndex - CurArgIdx);
        CurArgIdx = Args[I].OrigArgIndex;

        if (ArgFlags.isByVal()) {
            handleByValArg(I, ArgVT, ArgVT, CCValAssign::Full, ArgFlags);
            continue;
        }

        MVT RegVT = getRegVT(ArgVT, FuncArg->getType(), nullptr, IsSoftFloat);

        if (!FixedFn(I, ArgVT, RegVT, CCValAssign::Full, ArgFlags, CCInfo))
            continue;

        llvm_unreachable(nullptr);
    }
}


template<typename Ty>
void AlexTargetLowering::AlexCC::
analyzeReturn(const SmallVectorImpl<Ty> &RetVals, bool IsSoftFloat,
              const SDNode *CallNode, const Type *RetTy) const {
    CCAssignFn *Fn;

    Fn = RetCC_Alex;

    for (unsigned I = 0, E = RetVals.size(); I < E; ++I) {
        MVT VT = RetVals[I].VT;
        ISD::ArgFlagsTy Flags = RetVals[I].Flags;
        MVT RegVT = this->getRegVT(VT, RetTy, CallNode, IsSoftFloat);

        if (Fn(I, VT, RegVT, CCValAssign::Full, Flags, this->CCInfo)) {
#ifndef NDEBUG
            dbgs() << "Call result #" << I << " has unhandled type "
            << EVT(VT).getEVTString() << '\n';
#endif
            llvm_unreachable(nullptr);
        }
    }
}
AlexTargetLowering::AlexCC::AlexCC(
        CallingConv::ID CC, bool IsO32_, CCState &Info,
        AlexCC::SpecialCallingConvType SpecialCallingConv_)
        : CCInfo(Info), CallConv(CC), IsO32(IsO32_) {
    // Pre-allocate reserved argument area.
    CCInfo.AllocateStack(reservedArgArea(), 1);
}
void AlexTargetLowering::AlexCC::
analyzeCallResult(const SmallVectorImpl<ISD::InputArg> &Ins, bool IsSoftFloat,
                  const SDNode *CallNode, const Type *RetTy) const {
    analyzeReturn(Ins, IsSoftFloat, CallNode, RetTy);
}

void AlexTargetLowering::AlexCC::
analyzeReturn(const SmallVectorImpl<ISD::OutputArg> &Outs, bool IsSoftFloat,
              const Type *RetTy) const {
    analyzeReturn(Outs, IsSoftFloat, nullptr, RetTy);
}

SDValue AlexTargetLowering::LowerReturn(SDValue chain, CallingConv::ID CallConv, bool isVarArg,
                                        const SmallVectorImpl<ISD::OutputArg> &outs,
                                        const SmallVectorImpl<SDValue> &outVals,
                                        SDLoc dl,
                                        SelectionDAG &dag) const {
    SmallVector<CCValAssign, 16> RVLocs;
    MachineFunction &MF = dag.getMachineFunction();

    // CCState - Info about the registers and stack slot.
    CCState CCInfo(CallConv, isVarArg, MF, RVLocs,
                   *dag.getContext());
    AlexCC AlexCCInfo(CallConv, true,
                        CCInfo);

    // Analyze return values.
    AlexCCInfo.analyzeReturn(outs, false,
                              MF.getFunction()->getReturnType());

    SDValue Flag;
    SmallVector<SDValue, 4> RetOps(1, chain);

    // Copy the result values into the output registers.
    for (unsigned i = 0; i != RVLocs.size(); ++i) {
        SDValue Val = outVals[i];
        CCValAssign &VA = RVLocs[i];
        assert(VA.isRegLoc() && "Can only return in registers!");

        if (RVLocs[i].getValVT() != RVLocs[i].getLocVT())
            Val = dag.getNode(ISD::BITCAST, dl, RVLocs[i].getLocVT(), Val);

        chain = dag.getCopyToReg(chain, dl, VA.getLocReg(), Val, Flag);

        // Guarantee that all emitted copies are stuck together with flags.
        Flag = chain.getValue(1);
        RetOps.push_back(dag.getRegister(VA.getLocReg(), VA.getLocVT()));
    }

//@Ordinary struct type: 2 {
    // The Alex ABIs for returning structs by value requires that we copy
    // the sret argument into $v0 for the return. We saved the argument into
    // a virtual register in the entry block, so now we copy the value out
    // and into $v0.
    if (MF.getFunction()->hasStructRetAttr()) {
        AlexFunctionInfo *AlexFI = MF.getInfo<AlexFunctionInfo>();
        unsigned Reg = AlexFI->getSRetReturnReg();

        if (!Reg)
            llvm_unreachable("sret virtual register not created in the entry block");
        SDValue Val =
                dag.getCopyFromReg(chain, dl, Reg, getPointerTy(dag.getDataLayout()));
        unsigned RETVAL = Alex::R0;

        chain = dag.getCopyToReg(chain, dl, RETVAL, Val, Flag);
        Flag = chain.getValue(1);
        RetOps.push_back(dag.getRegister(RETVAL, getPointerTy(dag.getDataLayout())));
    }
//@Ordinary struct type: 2 }

    RetOps[0] = chain;  // Update chain.

    // Add the flag if we have it.
    if (Flag.getNode())
        RetOps.push_back(Flag);

    // Return on Alex is always a "ret $lr"
    return dag.getNode(AlexISD::Ret, dl, MVT::Other, RetOps);
}
