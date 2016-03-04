// @file      mips_isa.cpp
// @brief     The ArchC MIPS functional model.

#include  "mips_isa.H"
#include  "mips_isa_init.cpp"
#include  "mips_bhv_macros.H"

//If you want debug information for this model, uncomment next line
//#define DEBUG_MODEL
#include "ac_debug_model.H"

//!User defined macros to reference registers.
#define Ra 31
#define Sp 29

// 'using namespace' statement to allow access to all
// mips-specific datatypes
using namespace mips_parms;

static int processors_started = 0;
#define DEFAULT_STACK_SIZE (256*1024)

// Generic instruction behavior method.
void ac_behavior( instruction )
{
   dbg_printf("----- PC=%#x ----- %lld\n", (int) ac_pc, ac_instr_counter);
#ifndef NO_NEED_PC_UPDATE
  ac_pc = npc;
  npc = ac_pc + 4;
#endif
};

//  Instruction Format behavior methods.
void ac_behavior( Type_R ){}
void ac_behavior( Type_I ){}
void ac_behavior( Type_J ){}
void ac_behavior( Type_S ){}

// Behavior called before starting simulation
void ac_behavior(begin)
{
  dbg_printf("@@@ begin behavior @@@\n");
  npc = ac_pc + 4;

  for (int regNum = 0; regNum < 32; regNum ++)
    RB[regNum] = 0;
  hi = 0;
  lo = 0;
}

// Behavior called after finishing simulation
void ac_behavior(end)
{
  dbg_printf("@@@ end behavior @@@\n");
}

// Individual instructions emulation code
void ac_behavior( lb )
{
  char byte;
  dbg_printf("lb r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  byte = DATA_PORT->read_byte(RB[rs]+ imm);
  RB[rt] = (ac_Sword)byte ;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( lbu )
{
  unsigned char byte;
  dbg_printf("lbu r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  byte = DATA_PORT->read_byte(RB[rs]+ imm);
  RB[rt] = byte ;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( lh )
{
  short int half;
  dbg_printf("lh r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  half = DATA_PORT->read_half(RB[rs]+ imm);
  RB[rt] = (ac_Sword)half ;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( lhu )
{
  unsigned short int  half;
  half = DATA_PORT->read_half(RB[rs]+ imm);
  RB[rt] = half ;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( lw )
{
  dbg_printf("lw r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  RB[rt] = DATA_PORT->read(RB[rs]+ imm);
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( ldc1 )
{
  dbg_printf("ldc1 %%f%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  RBF[rt + 1] = DATA_PORT->read(RB[rs]+ imm);
  RBF[rt] = DATA_PORT->read(RB[rs]+ imm + 4);
  double temp = load_double(rt);
  dbg_printf("Result = %lf\n", temp);
}

void ac_behavior( lwc1 )
{
  dbg_printf("lwc1 %%f%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  RBF[rt] = DATA_PORT->read(RB[rs]+ imm);
  dbg_printf("Result = %f\n", (float)RBF[rt]);
}

void ac_behavior( ldxc1 )
{
  dbg_printf("ldxc1 %%f%d, %%%d(%%%d)\n", shamt, rt, rs);
  RBF[shamt + 1] = DATA_PORT->read(RB[rt]+ RB[rs]);
  RBF[shamt] = DATA_PORT->read(RB[rt]+ RB[rs] + 4);
  double temp = load_double(shamt);
  dbg_printf("Result = %lf\n", temp);
}

void ac_behavior( lwxc1 )
{
  dbg_printf("lwxc1 %%f%d, %%%d(%%%d)\n", shamt, rt, rs);
  RBF[shamt] = DATA_PORT->read(RB[rt]+ RB[rs]);
  float temp = load_float(shamt);
  dbg_printf("Result = %f\n", temp);
}

void ac_behavior( lwl )
{
  dbg_printf("lwl r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (addr & 0x3) * 8;
  data = DATA_PORT->read(addr & 0xFFFFFFFC);
  data <<= offset;
  data |= RB[rt] & ((1<<offset)-1);
  RB[rt] = data;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( lwr )
{
  dbg_printf("lwr r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (3 - (addr & 0x3)) * 8;
  data = DATA_PORT->read(addr & 0xFFFFFFFC);
  data >>= offset;
  data |= RB[rt] & (0xFFFFFFFF << (32-offset));
  RB[rt] = data;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( sb )
{
  unsigned char byte;
  dbg_printf("sb r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  byte = RB[rt] & 0xFF;
  DATA_PORT->write_byte(RB[rs] + imm, byte);
  dbg_printf("Result = %#x\n", (int) byte);
}

void ac_behavior( sh )
{
  unsigned short int half;
  dbg_printf("sh r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  half = RB[rt] & 0xFFFF;
  DATA_PORT->write_half(RB[rs] + imm, half);
  dbg_printf("Result = %#x\n", (int) half);
}

void ac_behavior( sw )
{
  dbg_printf("sw r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  DATA_PORT->write(RB[rs] + imm, RB[rt]);
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( absd )
{
  dbg_printf("abs.d %%f%d, %%f%d\n", shamt, rd);
  double res = fabs(load_double(rd));
  save_double(res, shamt);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior( abss )
{
  dbg_printf("abs.s %%f%d, %%f%d\n", shamt, rd);
  float res = fabsf(load_float(rd));
  save_float(res, shamt);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior( addd )
{
  dbg_printf("add.d %%f%d, %%f%d, %%f%d\n", shamt, rd, rt);
  double res = load_double(rd) + load_double(rt);
  save_double(res, shamt);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior( adds )
{
  dbg_printf("add.s %%f%d, %%f%d, %%f%d\n", shamt, rd, rt);
  float res = load_float(rd) + load_float(rt);
  save_float(res, shamt);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior( ceqd )
{
  dbg_printf("c.eq.d %%f%d, %%f%d\n", rd, rt);
  double a = load_double(rd);
  double b = load_double(rt);
  cc = a == b ? (custom_isnan(a) || custom_isnan(b) ?  0 : 1) : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior( ceqs )
{
  dbg_printf("c.eq.s %%f%d, %%f%d\n", rd, rt);
  float a = load_float(rd);
  float b = load_float(rt);
  cc = a == b ? (custom_isnanf(a) || custom_isnanf(b) ?  0 : 1) : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior( coled )
{
  dbg_printf("c.ole.d %%f%d, %%f%d\n", rd, rt);
  double a = load_double(rd);
  double b = load_double(rt);
  cc = a <= b ? (custom_isnan(a) || custom_isnan(b) ?  0 : 1) : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior( coles )
{
  dbg_printf("c.ole.s %%f%d, %%f%d\n", rd, rt);
  float a = load_float(rd);
  float b = load_float(rt);
  cc = a <= b ? (custom_isnanf(a) || custom_isnanf(b) ?  0 : 1) : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior( coltd )
{
  dbg_printf("c.olt.d %%f%d, %%f%d\n", rd, rt);
  double a = load_double(rd);
  double b = load_double(rt);
  cc = a < b ? (custom_isnan(a) || custom_isnan(b) ?  0 : 1) : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior( colts )
{
  dbg_printf("c.olt.s %%f%d, %%f%d\n", rd, rt);
  float a = load_float(rd);
  float b = load_float(rt);
  cc = a < b ? (custom_isnanf(a) || custom_isnanf(b) ?  0 : 1) : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior( cueqd )
{
  dbg_printf("c.ueq.d %%f%d, %%f%d\n", rd, rt);
  cc = (load_double(rd) == load_double(rt)) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior( cueqs )
{
  dbg_printf("c.ueq.s %%f%d, %%f%d\n", rd, rt);
  cc = (load_float(rd) == load_float(rt)) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior( culed )
{
  dbg_printf("c.ule.d %%f%d, %%f%d\n", rd, rt);
  cc = (load_double(rd) <= load_double(rt)) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior( cules )
{
  dbg_printf("c.ule.s %%f%d, %%f%d\n", rd, rt);
  cc = (load_float(rd) <= load_float(rt)) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior( cultd )
{
  dbg_printf("c.ult.d %%f%d, %%f%d\n", rd, rt);
  cc = (load_double(rd) < load_double(rt)) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior( cults )
{
  dbg_printf("c.ult.s %%f%d, %%f%d\n", rd, rt);
  cc = (load_float(rd) < load_float(rt)) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior( cund )
{
  dbg_printf("c.un.d %%f%d, %%f%d\n", rd, rt);
  cc = (custom_isnan(load_double(rd)) || custom_isnan(load_double(rt))) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior( cuns )
{
  dbg_printf("c.un.s %%f%d, %%f%d\n", rd, rt);
  cc = (custom_isnanf(load_float(rd)) || custom_isnanf(load_float(rt))) ? 1 : 0;
  dbg_printf("Result = %d\n", cc.read());
}

void ac_behavior( cvtsd )
{
  dbg_printf("cvt.s.d %%f%d, %%f%d\n", shamt, rd);
  float temp = (float) load_double(rd);
  save_float(temp, shamt);
  dbg_printf("Result = %f\n", temp);
}

void ac_behavior( cvtds )
{
  dbg_printf("cvt.d.s %%f%d, %%f%d\n", shamt, rd);
  double temp = (double) load_float(rd);
  save_double(temp, shamt);
  dbg_printf("Result = %lf\n", temp);
}

void ac_behavior( cvtdw )
{
  dbg_printf("cvt.d.w %%f%d, %%f%d\n", shamt, rd);
  double temp = (double) (int) RBF[rd];
  save_double(temp, shamt);
  dbg_printf("Result = %lf\n", temp);
}

void ac_behavior( cvtsw )
{
  dbg_printf("cvt.s.w %%f%d, %%f%d\n", shamt, rd);
  float temp = (float) (int) RBF[rd];
  save_float(temp, shamt);
  dbg_printf("Result = %f\n", temp);
}

void ac_behavior( divd )
{
  dbg_printf("div.d %%f%d, %%f%d, %%f%d\n", shamt, rd, rt);
  double res = load_double(rd) / load_double(rt);
  save_double(res, shamt);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior( divs )
{
  dbg_printf("div.s %%f%d, %%f%d, %%f%d\n", shamt, rd, rt);
  float res = load_float(rd) / load_float(rt);
  save_float(res, shamt);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior( mfc1 )
{
  dbg_printf("mfc1 %%%d, %%f%d\n", rt, rd);
  RB[rt] = RBF[rd];
  dbg_printf("Result = 0x%X\n", RB[rt]);
}

void ac_behavior( movd )
{
  dbg_printf("mov.d %%f%d, %%f%d\n", shamt, rd);
  double res = load_double(rd);
  save_double(res, shamt);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior( movs )
{
  dbg_printf("mov.s %%f%d, %%f%d\n", shamt, rd);
  float res = load_float(rd);
  save_float(res, shamt);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior( muld )
{
  dbg_printf("mul.d %%f%d, %%f%d, %%f%d\n", shamt, rd, rt);
  double res = load_double(rd) * load_double(rt);
  save_double(res, shamt);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior( muls )
{
  dbg_printf("mul.s %%f%d, %%f%d, %%f%d\n", shamt, rd, rt);
  float res = load_float(rd) * load_float(rt);
  save_float(res, shamt);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior( mtc1 )
{
  dbg_printf("mtc1 %%%d, %%f%d\n", rt, rd);
  RBF[rd] = RB[rt];
}

void ac_behavior( negd )
{
  dbg_printf("neg.d %%f%d, %%f%d\n", shamt, rd);
  double res = - load_double(rt);
  save_double(res, shamt);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior( negs )
{
  dbg_printf("neg.s %%f%d, %%f%d\n", shamt, rd);
  float res = - load_float(rt);
  save_float(res, shamt);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior( subd )
{
  dbg_printf("sub.d %%f%d, %%f%d, %%f%d\n", shamt, rd, rt);
  double res = load_double(rd) - load_double(rt);
  save_double(res, shamt);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior( subs )
{
  dbg_printf("sub.s %%f%d, %%f%d, %%f%d\n", shamt, rd, rt);
  float res = load_float(rd) - load_float(rt);
  save_float(res, shamt);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior( truncwd )
{
  dbg_printf("trunc.w.d %%f%d, %%f%d\n", shamt, rd);
  RBF[shamt] = (int32_t) load_double(rd);
  dbg_printf("Result = %d\n", RBF[shamt]);
}

void ac_behavior( truncws )
{
  dbg_printf("trunc.w.s %%f%d, %%f%d\n", shamt, rd);
  RBF[shamt] = (int32_t) load_float(rd);
  dbg_printf("Result = %d\n", RBF[shamt]);
}

void ac_behavior( sqrtd )
{
  dbg_printf("sqrt.d %%f%d, %%f%d\n", shamt, rd);
  double res = sqrt(load_double(rd));
  save_double(res, shamt);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior( sqrts )
{
  dbg_printf("sqrt.s %%f%d, %%f%d\n", shamt, rd);
  float res = sqrtf(load_float(rd));
  save_float(res, shamt);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior( bc1f )
{
  dbg_printf("bc1f %d\n", imm & 0xFFFF);
  if(cc == 0) {
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }
}

void ac_behavior( bc1fl )
{
  dbg_printf("bc1fl %d\n", imm & 0xFFFF);
  if(cc == 0) {
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }
}

void ac_behavior( bc1t )
{
  dbg_printf("bc1t %d\n", imm & 0xFFFF);
  if(cc == 1) {
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }
}

void ac_behavior( bc1tl )
{
  dbg_printf("bc1tl %d\n", imm & 0xFFFF);
  if(cc == 1) {
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }
}

void ac_behavior( sdc1 )
{
  dbg_printf("sdc1 %%f%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  DATA_PORT->write(RB[rs] + imm + 4, RBF[rt]);
  DATA_PORT->write(RB[rs] + imm, RBF[rt + 1]);
}

void ac_behavior( swc1 )
{
  dbg_printf("swc1 %%f%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  DATA_PORT->write(RB[rs] + imm, RBF[rt]);
}

void ac_behavior( sdxc1 )
{
  dbg_printf("sdxc1 %%f%d, %%%d(%%%d)\n", rd, rt, rs);
  DATA_PORT->write(RB[rt] + RB[rs] + 4, RBF[rd]);
  DATA_PORT->write(RB[rt] + RB[rs], RBF[rd + 1]);
}

void ac_behavior( swxc1 )
{
  dbg_printf("swxc1 %%f%d, %%%d(%%%d)\n", rd, rt, rs);
  DATA_PORT->write(RB[rt] + RB[rs], RBF[rd]);
}

void ac_behavior( mfhc1 )
{
  dbg_printf("mfhc1 %%%d, %%f%d\n", rt, rd);
  uint64_t temp;
  double input = load_double(rd);
  memcpy(&temp, &input, sizeof(uint64_t));
  RB[rt] = temp >> 32;
  dbg_printf("Result = 0x%X\n", RB[rt]);
}

void ac_behavior( mthc1 )
{
  dbg_printf("mthc1 %%%d, %%f%d\n", rt, rd);
  double temp = load_double(rd);
  uint64_t to_int;
  memcpy(&to_int, &temp, sizeof(uint64_t));
  to_int = (to_int & 0xFFFFFFFFULL) + (((uint64_t)RB[rt]) << 32);
  memcpy(&temp, &to_int, sizeof(uint64_t));
  save_double(temp, rd);
}

void ac_behavior( swl )
{
  dbg_printf("swl r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (addr & 0x3) * 8;
  data = RB[rt];
  data >>= offset;
  data |= DATA_PORT->read(addr & 0xFFFFFFFC) & (0xFFFFFFFF << (32-offset));
  DATA_PORT->write(addr & 0xFFFFFFFC, data);
  dbg_printf("Result = %#x\n", data);
}

void ac_behavior( swr )
{
  dbg_printf("swr r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (3 - (addr & 0x3)) * 8;
  data = RB[rt];
  data <<= offset;
  data |= DATA_PORT->read(addr & 0xFFFFFFFC) & ((1<<offset)-1);
  DATA_PORT->write(addr & 0xFFFFFFFC, data);
  dbg_printf("Result = %#x\n", data);
}

void ac_behavior( addi )
{
  dbg_printf("addi r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] + imm;
  dbg_printf("Result = %#x\n", RB[rt]);
  //Test overflow
  if ( ((RB[rs] & 0x80000000) == (imm & 0x80000000)) &&
       ((imm & 0x80000000) != (RB[rt] & 0x80000000)) ) {
    fprintf(stderr, "EXCEPTION(addi): integer overflow.\n"); exit(EXIT_FAILURE);
  }
}

void ac_behavior( addiu )
{
  dbg_printf("addiu r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] + imm;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( slti )
{
  dbg_printf("slti r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  // Set the RD if RS< IMM
  if( (ac_Sword) RB[rs] < (ac_Sword) imm )
    RB[rt] = 1;
  // Else reset RD
  else
    RB[rt] = 0;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( sltiu )
{
  dbg_printf("sltiu r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  // Set the RD if RS< IMM
  if( (ac_Uword) RB[rs] < (ac_Uword) imm )
    RB[rt] = 1;
  // Else reset RD
  else
    RB[rt] = 0;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( andi )
{
  dbg_printf("andi r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] & (imm & 0xFFFF) ;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( ori )
{
  dbg_printf("ori r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] | (imm & 0xFFFF) ;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( xori )
{
  dbg_printf("xori r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] ^ (imm & 0xFFFF) ;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( lui )
{
  dbg_printf("lui r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  // Load a constant in the upper 16 bits of a register
  // To achieve the desired behaviour, the constant was shifted 16 bits left
  // and moved to the target register ( rt )
  RB[rt] = imm << 16;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( add )
{
  dbg_printf("add r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] + RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
  //Test overflow
  if ( ((RB[rs] & 0x80000000) == (RB[rd] & 0x80000000)) &&
       ((RB[rd] & 0x80000000) != (RB[rt] & 0x80000000)) ) {
    fprintf(stderr, "EXCEPTION(add): integer overflow.\n"); exit(EXIT_FAILURE);
  }
}

void ac_behavior( addu )
{
  dbg_printf("addu r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] + RB[rt];
  //cout << "  RS: " << (unsigned int)RB[rs] << " RT: " << (unsigned int)RB[rt] << endl;
  //cout << "  Result =  " <<  (unsigned int)RB[rd] <<endl;
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( sub )
{
  dbg_printf("sub r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] - RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
  //TODO: test integer overflow exception for sub
}

void ac_behavior( subu )
{
  dbg_printf("subu r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] - RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( slt )
{
  dbg_printf("slt r%d, r%d, r%d\n", rd, rs, rt);
  // Set the RD if RS< RT
  if( (ac_Sword) RB[rs] < (ac_Sword) RB[rt] )
    RB[rd] = 1;
  // Else reset RD
  else
    RB[rd] = 0;
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( sltu )
{
  dbg_printf("sltu r%d, r%d, r%d\n", rd, rs, rt);
  // Set the RD if RS < RT
  if( RB[rs] < RB[rt] )
    RB[rd] = 1;
  // Else reset RD
  else
    RB[rd] = 0;
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( instr_and )
{
  dbg_printf("instr_and r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] & RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( instr_or )
{
  dbg_printf("instr_or r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] | RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( instr_xor )
{
  dbg_printf("instr_xor r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] ^ RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( instr_nor )
{
  dbg_printf("nor r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = ~(RB[rs] | RB[rt]);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( nop )
{
  dbg_printf("nop\n");
}

void ac_behavior( sll )
{
  dbg_printf("sll r%d, r%d, %d\n", rd, rt, shamt);
  RB[rd] = RB[rt] << shamt;
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( srl )
{
  dbg_printf("srl r%d, r%d, %d\n", rd, rt, shamt);
  RB[rd] = RB[rt] >> shamt;
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( sra )
{
  dbg_printf("sra r%d, r%d, %d\n", rd, rt, shamt);
  RB[rd] = (ac_Sword) RB[rt] >> shamt;
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( sllv )
{
  dbg_printf("sllv r%d, r%d, r%d\n", rd, rt, rs);
  RB[rd] = RB[rt] << (RB[rs] & 0x1F);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( srlv )
{
  dbg_printf("srlv r%d, r%d, r%d\n", rd, rt, rs);
  RB[rd] = RB[rt] >> (RB[rs] & 0x1F);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( srav )
{
  dbg_printf("srav r%d, r%d, r%d\n", rd, rt, rs);
  RB[rd] = (ac_Sword) RB[rt] >> (RB[rs] & 0x1F);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( mult )
{
  dbg_printf("mult r%d, r%d\n", rs, rt);

  long long result;
  int half_result;

  result = (ac_Sword) RB[rs];
  result *= (ac_Sword) RB[rt];

  half_result = (result & 0xFFFFFFFF);
  // Register LO receives 32 less significant bits
  lo = half_result;

  half_result = ((result >> 32) & 0xFFFFFFFF);
  // Register HI receives 32 most significant bits
  hi = half_result ;

  dbg_printf("Result = %#llx\n", result);
}

void ac_behavior( multu )
{
  dbg_printf("multu r%d, r%d\n", rs, rt);

  unsigned long long result;
  unsigned int half_result;

  result  = RB[rs];
  result *= RB[rt];

  half_result = (result & 0xFFFFFFFF);
  // Register LO receives 32 less significant bits
  lo = half_result;

  half_result = ((result>>32) & 0xFFFFFFFF);
  // Register HI receives 32 most significant bits
  hi = half_result ;

  dbg_printf("Result = %#llx\n", result);
}

void ac_behavior( div )
{
  dbg_printf("div r%d, r%d\n", rs, rt);
  // Register LO receives quotient
  lo = (ac_Sword) RB[rs] / (ac_Sword) RB[rt];
  // Register HI receives remainder
  hi = (ac_Sword) RB[rs] % (ac_Sword) RB[rt];
}

void ac_behavior( divu )
{
  dbg_printf("divu r%d, r%d\n", rs, rt);
  // Register LO receives quotient
  lo = RB[rs] / RB[rt];
  // Register HI receives remainder
  hi = RB[rs] % RB[rt];
}

void ac_behavior( mfhi )
{
  dbg_printf("mfhi r%d\n", rd);
  RB[rd] = hi;
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( mthi )
{
  dbg_printf("mthi r%d\n", rs);
  hi = RB[rs];
  dbg_printf("Result = %#x\n", (unsigned int) hi);
}

void ac_behavior( mflo )
{
  dbg_printf("mflo r%d\n", rd);
  RB[rd] = lo;
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( mtlo )
{
  dbg_printf("mtlo r%d\n", rs);
  lo = RB[rs];
  dbg_printf("Result = %#x\n", (unsigned int) lo);
}

void ac_behavior( j )
{
  dbg_printf("j %d\n", addr);
  addr = addr << 2;
#ifndef NO_NEED_PC_UPDATE
  npc =  (ac_pc & 0xF0000000) | addr;
#endif
  dbg_printf("Target = %#x\n", (ac_pc & 0xF0000000) | addr );
}

void ac_behavior( jal )
{
  dbg_printf("jal %d\n", addr);
  // Save the value of PC + 8 (return address) in $ra ($31) and
  // jump to the address given by PC(31...28)||(addr<<2)
  // It must also flush the instructions that were loaded into the pipeline
  RB[Ra] = ac_pc+4; //ac_pc is pc+4, we need pc+8

  addr = addr << 2;
#ifndef NO_NEED_PC_UPDATE
  npc = (ac_pc & 0xF0000000) | addr;
#endif

  dbg_printf("Target = %#x\n", (ac_pc & 0xF0000000) | addr );
  dbg_printf("Return = %#x\n", ac_pc+4);
}

void ac_behavior( jr )
{
  dbg_printf("jr r%d\n", rs);
  // Jump to the address stored on the register reg[RS]
  // It must also flush the instructions that were loaded into the pipeline
#ifndef NO_NEED_PC_UPDATE
  npc = RB[rs], 1;
#endif
  dbg_printf("Target = %#x\n", RB[rs]);
}

void ac_behavior( jalr )
{
  dbg_printf("jalr r%d, r%d\n", rd, rs);
  // Save the value of PC + 8(return address) in rd and
  // jump to the address given by [rs]

#ifndef NO_NEED_PC_UPDATE
  npc = RB[rs], 1;
#endif
  dbg_printf("Target = %#x\n", RB[rs]);

  if( rd == 0 )  //If rd is not defined use default
    rd = Ra;
  RB[rd] = ac_pc+4;
  dbg_printf("Return = %#x\n", ac_pc+4);
}

void ac_behavior( beq )
{
  dbg_printf("beq r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  if( RB[rs] == RB[rt] ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }
}

void ac_behavior( bne )
{
  dbg_printf("bne r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  if( RB[rs] != RB[rt] ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }
}

void ac_behavior( blez )
{
  dbg_printf("blez r%d, %d\n", rs, imm & 0xFFFF);
  if( (RB[rs] == 0 ) || (RB[rs]&0x80000000 ) ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2), 1;
#endif
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }
}

void ac_behavior( bgtz )
{
  dbg_printf("bgtz r%d, %d\n", rs, imm & 0xFFFF);
  if( !(RB[rs] & 0x80000000) && (RB[rs]!=0) ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }
}

void ac_behavior( bltz )
{
  dbg_printf("bltz r%d, %d\n", rs, imm & 0xFFFF);
  if( RB[rs] & 0x80000000 ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }
}

void ac_behavior( bgez )
{
  dbg_printf("bgez r%d, %d\n", rs, imm & 0xFFFF);
  if( !(RB[rs] & 0x80000000) ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }
}

void ac_behavior( bltzal )
{
  dbg_printf("bltzal r%d, %d\n", rs, imm & 0xFFFF);
  RB[Ra] = ac_pc+4; //ac_pc is pc+4, we need pc+8
  if( RB[rs] & 0x80000000 ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }
  dbg_printf("Return = %#x\n", ac_pc+4);
}

void ac_behavior( bgezal )
{
  dbg_printf("bgezal r%d, %d\n", rs, imm & 0xFFFF);
  RB[Ra] = ac_pc+4; //ac_pc is pc+4, we need pc+8
  if( !(RB[rs] & 0x80000000) ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }
  dbg_printf("Return = %#x\n", ac_pc+4);
}

void ac_behavior( sys_call )
{
  if (code == 0) {
    // o32 abi, expected syscall number is in $v0 ($2)
    uint32_t sysnum = RB[2];
    dbg_printf("Syscall number: 0x%X\t(%d)\n", sysnum, sysnum);
    if (syscall.process_syscall(sysnum) == -1) {
      fprintf(stderr, "Warning: Unimplemented syscall.\n");
      fprintf(stderr, "\tCaller address: 0x%X\n\tSyscall number: 0x%X\t%d\n",
              (unsigned int)ac_pc, sysnum, sysnum);
      RB[2] = -1;
    }
    // Sets a3 to 1 or 0 for error/success
    if ((int)RB[2] < 0)
      RB[7] = 1;
    else
      RB[7] = 0;
  } else {
    dbg_printf("Syscall number: 0x%X\t(%d)\n", code, code);
    if (syscall.process_syscall(code) == -1) {
      fprintf(stderr, "Warning: Unimplemented syscall.\n");
      fprintf(stderr, "\tCaller address: 0x%X\n\tSyscall number: 0x%X\t%d\n",
              (unsigned int)ac_pc, code, code);
      RB[2] = -1;
    }
    if ((int)RB[2] < 0)
      RB[7] = 1;
    else
      RB[7] = 0;

  }
  dbg_printf("Result = %#x\n", RB[2]);
}

void ac_behavior( instr_break )
{
  fprintf(stderr, "instr_break behavior not implemented.\n"); 
  exit(EXIT_FAILURE);
}

void ac_behavior( seb )
{
  dbg_printf("seb r%d, r%d\n", rd, rt);
  RB[rd] = sign_extend(RB[rt], 8);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( seh )
{
  dbg_printf("seh r%d, r%d\n", rd, rt);
  RB[rd] = sign_extend(RB[rt], 16);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( ext )
{
  dbg_printf("ext r%d, r%d, %d, %d\n", rt, rs, shamt, rd);
  uint32_t lsb = shamt;
  uint32_t size = rd + 1;
  RB[rt] = (RB[rs] << (32 - size - lsb)) >> (32 - size);
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( rotr )
{
  dbg_printf("rotr r%d, r%d, %d\n", rd, rt, shamt);
  RB[rd] = rotate_right(RB[rt], shamt);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( rotrv )
{
  dbg_printf("rotrv r%d, r%d, r%d\n", rd, rt, rs);
  RB[rd] = rotate_right(RB[rt], RB[rs]);
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( clz )
{
  dbg_printf("clz %%%d, %%%d\n", rd, rs);
  uint32_t x = RB[rs];
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x = ffs(x + 1);
  if (x != 0) {
    x = 32 - x + 1;
  }
  RB[rd] = x;
  dbg_printf("Result = %#x\n", x);
}

void ac_behavior( mul )
{
  dbg_printf("mul %%%d, %%%d, %%%d\n", rd, rs, rt);
  RB[rd] = RB[rs] * RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( ll )
{
  dbg_printf("ll r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  RB[rt] = DATA_PORT->read(RB[rs]+ imm);
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( sc )
{
  dbg_printf("sc r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  DATA_PORT->write(RB[rs] + imm, RB[rt]);
  RB[rt] = 1;
  dbg_printf("Result = %#x\n", RB[rt]);
}

void ac_behavior( sync )
{
  dbg_printf("sync\n");
}

void ac_behavior( teq )
{
  dbg_printf("teq %%%d, %%%d\n", rs, rt);
  if (RB[rs] == RB[rt]) {
    fprintf(stderr, "Trap generated at PC=0x%X\n", (uint32_t) ac_pc);
    exit(EXIT_FAILURE);
  }
}

void ac_behavior( movz )
{
  dbg_printf("movz r%d, r%d, r%d\n", rd, rs, rt);
  if (RB[rt] == 0)
    RB[rd] = RB[rs];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( movn )
{
  dbg_printf("movn r%d, r%d, r%d\n", rd, rs, rt);
  if (RB[rt] != 0)
    RB[rd] = RB[rs];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( movzd )
{
  dbg_printf("movz.d %%f%d, %%f%d, %%%d\n", shamt, rd, rt);
  if (RB[rt] != 0) {
    RBF[shamt] = RBF[rd];
    RBF[shamt + 1] = RBF[rd + 1];
  }
}

void ac_behavior( movzs )
{
  dbg_printf("movz.s %%f%d, %%f%d, %%%d\n", shamt, rd, rt);
  if (RB[rt] != 0) {
    RBF[shamt] = RBF[rd];
  }
}

void ac_behavior( movnd )
{
  dbg_printf("movn.d %%f%d, %%f%d, %%%d\n", shamt, rd, rt);
  if (RB[rt] != 0) {
    RBF[shamt] = RBF[rd];
    RBF[shamt + 1] = RBF[rd + 1];
  }
}

void ac_behavior( movns )
{
  dbg_printf("movn.s %%f%d, %%f%d, %%%d\n", shamt, rd, rt);
  if (RB[rt] != 0) {
    RBF[shamt] = RBF[rd];
  }
}

void ac_behavior( movf )
{
  dbg_printf("movf r%d, r%d, %%fcc0\n", rd, rs);
  if (cc == 0)
    RB[rd] = RB[rs];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( movt )
{
  dbg_printf("movt r%d, r%d, %%fcc0\n", rd, rs);
  if (cc != 0)
    RB[rd] = RB[rs];
  dbg_printf("Result = %#x\n", RB[rd]);
}

void ac_behavior( movtd )
{
  dbg_printf("movt.d %%f%d, %%f%d, %%fcc0\n", shamt, rd);
  if (cc != 0) {
    RBF[shamt] = RBF[rd];
    RBF[shamt + 1] = RBF[rd + 1];
  }
}

void ac_behavior( movts )
{
  dbg_printf("movt.s %%f%d, %%f%d, %%fcc0\n", shamt, rd);
  if (cc != 0) {
    RBF[shamt] = RBF[rd];
  }
}

void ac_behavior( movfd )
{
  dbg_printf("movf.d %%f%d, %%f%d, %%fcc0\n", shamt, rd);
  if (cc == 0) {
    RBF[shamt] = RBF[rd];
    RBF[shamt + 1] = RBF[rd + 1];
  }
}

void ac_behavior( movfs )
{
  dbg_printf("movf.s %%f%d, %%f%d, %%fcc0\n", shamt, rd);
  if (cc == 0) {
    RBF[shamt] = RBF[rd];
  }
}

void ac_behavior( maddd )
{
  dbg_printf("madd.d %%f%d, %%f%d, %%f%d, %%f%d\n", shamt, rs, rd, rt);
  double res = load_double(rd) * load_double(rt) + load_double(rs);
  save_double(res, shamt);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior( madds )
{
  dbg_printf("madd.s %%f%d, %%f%d, %%f%d, %%f%d\n", shamt, rs, rd, rt);
  float res = load_float(rd) * load_float(rt) + load_float(rs);
  save_float(res, shamt);
  dbg_printf("Result = %f\n", res);
}

void ac_behavior( msubd )
{
  dbg_printf("msub.d %%f%d, %%f%d, %%f%d, %%f%d\n", shamt, rs, rd, rt);
  double res = load_double(rd) * load_double(rt) - load_double(rs);
  save_double(res, shamt);
  dbg_printf("Result = %lf\n", res);
}

void ac_behavior( msubs )
{
  dbg_printf("msub.s %%f%d, %%f%d, %%f%d, %%f%d\n", shamt, rs, rd, rt);
  float res = load_float(rd) * load_float(rt) - load_float(rs);
  save_float(res, shamt);
  dbg_printf("Result = %f\n", res);
}
