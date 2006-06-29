/**
 * @file      mips1_gdb_funcs.cpp
 * @author    Sandro Rigo
 *            Marcus Bartholomeu
 *            Alexandro Baldassin (acasm information)
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:50:52 -0300
 * 
 * @brief     The ArchC MIPS-I functional model.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include "mips1.H"

// 'using namespace' statement to allow access to all
// mips1-specific datatypes
using namespace mips1_parms;

int mips1::nRegs(void) {
   return 73;
}


ac_word mips1::reg_read( int reg ) {
  /* general purpose registers */
  if ( ( reg >= 0 ) && ( reg < 32 ) )
    return RB.read( reg );
  else {
    if ( ( reg >= 33 ) && ( reg < 35 ) )
      return RB.read( reg - 1 );
    else
      /* pc */
      if ( reg == 37 )
        return ac_pc;
  }

  return 0;
}


void mips1::reg_write( int reg, ac_word value ) {
  /* general purpose registers */
  if ( ( reg >= 0 ) && ( reg < 32 ) )
    RB.write( reg, value );
  else
    {
      /* lo, hi */
      if ( ( reg >= 33 ) && ( reg < 35 ) )
        RB.write( reg - 1, value );
      else
        /* pc */
        if ( reg == 37 )
          ac_pc = value;
    }
}


unsigned char mips1::mem_read( unsigned int address ) {
  return IM->read_byte( address );
}


void mips1::mem_write( unsigned int address, unsigned char byte ) {
  IM->write_byte( address, byte );
}
