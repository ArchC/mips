/**
 * @file      mips_isa_helper.H
 * @author
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Feb 2016
 *
 * @brief     This is a helper header file included directly
 *            in the generated mips_isa.H file. Here we put
 *            data structures and functions used throughout
 *            the mips_isa.cpp (mips instruction behaviors).
 *
 * @attention Copyright (C) 2002-2016 --- The ArchC Team
 *
 */

static inline int32_t sign_extend(int32_t word, int word_length) {
  const int32_t m = 1U << (word_length - 1);
  int32_t x = (word) & ((1ULL << word_length) - 1);
  return ((x ^ m) - m);
}

inline double load_double(uint32_t index) {
  return (double) (RBF[index] + ((uint64_t) RBF[index + 1]) << 32);
}

inline float load_float(uint32_t index) {
  return (float) RBF[index];
}
