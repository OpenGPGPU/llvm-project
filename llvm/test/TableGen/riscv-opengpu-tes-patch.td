// RUN: llvm-tblgen -print-records -I %p/../../include -I %p/../../lib/Target/RISCV %s | FileCheck %s --check-prefix=RECORD
// RUN: llvm-tblgen -gen-asm-matcher -I %p/../../include -I %p/../../lib/Target/RISCV %s | FileCheck %s --check-prefix=MATCH
// RUN: llvm-tblgen -gen-emitter -I %p/../../include -I %p/../../lib/Target/RISCV %s | FileCheck %s --check-prefix=EMIT
// RUN: llvm-tblgen -gen-disassembler -I %p/../../include -I %p/../../lib/Target/RISCV %s | FileCheck %s --check-prefix=DECODE
// The small target includes actual production types/operands/instruction. It
// does not replace or mock the encoding, register classes, predicates or mask.
// This tests generators; it is not full-RISCV integration or MC roundtrip.
include "llvm/Target/Target.td"
include "RISCVFeatures.td"
include "RISCVSystemOperands.td"
include "RISCVRegisterInfo.td"
include "RISCVBareSymbolOperand.td"
include "RISCVInstrFormats.td"
include "RISCVVectorMaskOperand.td"
include "RISCVInstrFormatsXOpengpu.td"
include "RISCVInstrInfoXOpengpuTES.td"

def PatchInstrInfo : InstrInfo { let guessInstructionProperties = false; }
def PatchParser : AsmParser {
  let ShouldEmitMatchRegisterAltName = true;
  let AllowDuplicateRegisterNames = true;
  let ReportMultipleNearMisses = true;
  let PrioritizeFeatureInMultiMismatchFallback = true;
  let EmitTokenDiagnosticTypes = true;
}
def Patch : Target { let InstructionSet = PatchInstrInfo; let AssemblyParsers = [PatchParser]; }

// RECORD-LABEL: def XOPENGPU_VTES_PATCH_READ {
// RECORD-NEXT: field bits<32> Inst = { 1, 0, 0, 0, 0, 1, vm, vs2{4}, vs2{3}, vs2{2}, vs2{1}, vs2{0}, vs1{4}, vs1{3}, vs1{2}, vs1{1}, vs1{0}, 0, 0, 0, vd{4}, vd{3}, vd{2}, vd{1}, vd{0}, 0, 1, 0, 1, 0, 1, 1 };
// RECORD: list<Predicate> Predicates = [HasVendorXOpengpu];
// RECORD: dag OutOperandList = (outs VR:$vd);
// RECORD: dag InOperandList = (ins VR:$vs1, VR:$vs2, VMaskOp:$vm);
// RECORD: string AsmString = "opengpu.vtes.patch.read
// RECORD: list<dag> Pattern = [];
// RECORD: list<Register> Uses = [VL, VTYPE];
// RECORD: bit mayLoad = 1;
// RECORD: bit mayStore = 0;
// RECORD: bit isNotDuplicable = 1;
// RECORD: bit isConvergent = 1;
// RECORD: bit hasSideEffects = 1;
// MATCH: /* opengpu.vtes.patch.read */, RISCV::XOPENGPU_VTES_PATCH_READ,
// MATCH-SAME: AMFBS_HasVendorXOpengpu, { MCK_VR, MCK_VR, MCK_VR, MCK_RVVMaskRegOpOperand }
// EMIT: UINT64_C(2214592555),{{.*}}// XOPENGPU_VTES_PATCH_READ
// EMIT: case RISCV::XOPENGPU_VTES_PATCH_READ:
// EMIT: MI.getOperand(0)
// EMIT-NEXT: Value |= (op & 0x1f) << 7;
// EMIT: getVMaskReg(MI, 3, Fixups, STI);
// EMIT-NEXT: Value |= (op & 0x1) << 25;
// EMIT: MI.getOperand(2)
// EMIT-NEXT: Value |= (op & 0x1f) << 20;
// EMIT: MI.getOperand(1)
// EMIT-NEXT: Value |= (op & 0x1f) << 15;
// DECODE: OPC_CheckPredicate, 0,
// DECODE-NEXT: OPC_CheckField, 26, 6, 33,
// DECODE-NEXT: OPC_CheckField, 12, 3, 0,
// DECODE-NEXT: OPC_CheckField, 0, 7, 43,
// DECODE-NEXT: OPC_Decode,{{.*}}decode to XOPENGPU_VTES_PATCH_READ
// DECODE: fieldFromInstruction(insn, 7, 5);
// DECODE: DecodeSimpleRegisterClass<RISCV::V0, 32>
// DECODE: fieldFromInstruction(insn, 15, 5);
// DECODE: DecodeSimpleRegisterClass<RISCV::V0, 32>
// DECODE: fieldFromInstruction(insn, 20, 5);
// DECODE: DecodeSimpleRegisterClass<RISCV::V0, 32>
// DECODE: fieldFromInstruction(insn, 25, 1);
// DECODE: decodeVMaskReg(MI, tmp, Address, Decoder)
// DECODE: return FB[Patch::FeatureVendorXOpengpu];
