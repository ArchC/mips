/**
 * @file      mips_syscall.cpp
 * @author    Sandro Rigo
 *            Marcus Bartholomeu
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
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "mips_syscall.H"

// 'using namespace' statement to allow access to all
// mips-specific datatypes
using namespace mips_parms;
unsigned procNumber = 0;

void mips_syscall::get_buffer(int argn, unsigned char* buf, unsigned int size)
{
  unsigned int addr = RB[4+argn];
  for (unsigned int i = 0; i<size; i++, addr++) {
    buf[i] = DATA_PORT->read_byte(addr);
  }
}

void mips_syscall::guest2hostmemcpy(unsigned char *dst, uint32_t src,
                                    unsigned int size) {
  for (unsigned int i = 0; i < size; i++) {
    dst[i] = DATA_PORT->read_byte(src++);
  }
}

void mips_syscall::set_buffer(int argn, unsigned char* buf, unsigned int size)
{
  unsigned int addr = RB[4+argn];

  for (unsigned int i = 0; i<size; i++, addr++) {
    DATA_PORT->write_byte(addr, buf[i]);
  }
}

void mips_syscall::host2guestmemcpy(uint32_t dst, unsigned char *src,
                                    unsigned int size) {
  for (unsigned int i = 0; i < size; i++) {
    DATA_PORT->write_byte(dst++, src[i]);
  }
}

void mips_syscall::set_buffer_noinvert(int argn, unsigned char* buf, unsigned int size)
{
  unsigned int addr = RB[4+argn];

  for (unsigned int i = 0; i<size; i+=4, addr+=4) {
    DATA_PORT->write(addr, *(unsigned int *) &buf[i]);
    //printf("\nDATA_PORT_no[%d]=%d", addr, buf[i]);
  }
}

int mips_syscall::get_int(int argn)
{
  return RB[4+argn];
}

void mips_syscall::set_int(int argn, int val)
{
  RB[2+argn] = val;
}

void mips_syscall::return_from_syscall()
{
  ac_pc = RB[31];
  npc = ac_pc + 4;
}

void mips_syscall::set_prog_args(int argc, char **argv)
{
  // ac_argstr holds argument strings to be stored into guest memory
  char ac_argstr[512];
  // guest_stack represents pre-allocated values into the guest stack
  // passing information from kernel to userland (program args, etc)
  //
  // Memory diagram:
  //   higher addresses  .. auxv
  //                        envp pointers
  //                        argv pointers
  //   lower addresses   .. argc
  uint32_t guest_stack_size = argc + 6;
  uint32_t *guest_stack = (uint32_t *) malloc(sizeof(uint32_t) *
                                              guest_stack_size);
  uint32_t i = 0, j = 0;
  uint32_t strtable = AC_RAM_END - 512 - procNumber * 64 * 1024;
  guest_stack[i++] = argc;
  for (uint32_t k = 0; k < argc; ++k) {
    uint32_t len = strlen(argv[k]) + 1;
    guest_stack[i++] = strtable + j;
    if (j + len > 512) {
      fprintf(stderr, "Fatal: argv strings bigger than 512 bytes.\n");
      exit(EXIT_FAILURE);
    }
    memcpy(&ac_argstr[j], argv[k], len);
    j += len;
  }
  // Set argv end
  guest_stack[i++] = 0;
  // Set envp
  guest_stack[i++] = 0;
  // Set auxv
  guest_stack[i++] = 6; // AT_PAGESZ -
                    // see http://articles.manugarg.com/aboutelfauxiliaryvectors
  guest_stack[i++] = 4096; // page size
  guest_stack[i++] = 0; // AT_NULL

  RB[4] = strtable;
  set_buffer(0, (unsigned char*) ac_argstr, 512);   //$25 = $29(sp) - 4 (set_buffer adds 4)

  RB[4] = strtable - guest_stack_size * 4;
  set_buffer_noinvert(0, (unsigned char*) guest_stack, guest_stack_size * 4);

  RB[29] = strtable - guest_stack_size * 4;

  // FIXME: Necessary?
  RB[4] = argc;
  RB[5] = strtable - guest_stack_size * 4 + 4;

  procNumber ++;
}

void mips_syscall::set_pc(unsigned val) {
  ac_pc.write(val);
  npc = ac_pc + 4;
}

void mips_syscall::set_return(unsigned val) {
  RB.write(31, val);
}

unsigned mips_syscall::get_return() {
  return (unsigned) RB.read(31);
}

bool mips_syscall::is_mmap_anonymous(uint32_t flags) {
  return flags & 0x800;
}

uint32_t mips_syscall::convert_open_flags(uint32_t flags) {
  uint32_t dst = 0;
  dst |= (flags & 00000)? O_RDONLY : 0;
  dst |= (flags & 00001)? O_WRONLY : 0;
  dst |= (flags & 00002)? O_RDWR   : 0;
  dst |= (flags & 001000)? O_CREAT  : 0;
  dst |= (flags & 004000)? O_EXCL   : 0;
  dst |= (flags & 0100000)? O_NOCTTY   : 0;
  dst |= (flags & 02000)? O_TRUNC    : 0;
  dst |= (flags & 00010)? O_APPEND   : 0;
  dst |= (flags & 040000)? O_NONBLOCK : 0;
  dst |= (flags & 020000)? O_SYNC  : 0;
  // We don't know the mapping of these:
  //    dst |= (flags & 020000)? O_ASYNC   : 0;
  //    dst |= (flags & 0100000)? O_LARGEFILE  : 0;
  //    dst |= (flags & 0200000)? O_DIRECTORY  : 0;
  //    dst |= (flags & 0400000)? O_NOFOLLOW   : 0;
  //    dst |= (flags & 02000000)? O_CLOEXEC   : 0;
  //    dst |= (flags & 040000)? O_DIRECT      : 0;
  //    dst |= (flags & 01000000)? O_NOATIME   : 0;
  //    dst |= (flags & 010000000)? O_PATH     : 0;
  //    dst |= (flags & 010000)? O_DSYNC       : 0;
  return dst;
}


// MIPS syscalls mapping for process simulators
//
// These constants were extracted from:
//   linux-3.10.14/arch/mips/include/uapi/asm/unistd.h
//
// We extracted syscalls numbers for the o32 abi

#define MIPS__NR_Linux			4000
#define MIPS__NR_syscall			(MIPS__NR_Linux +	0)
#define MIPS__NR_exit			(MIPS__NR_Linux +	1)
#define MIPS__NR_fork			(MIPS__NR_Linux +	2)
#define MIPS__NR_read			(MIPS__NR_Linux +	3)
#define MIPS__NR_write			(MIPS__NR_Linux +	4)
#define MIPS__NR_open			(MIPS__NR_Linux +	5)
#define MIPS__NR_close			(MIPS__NR_Linux +	6)
#define MIPS__NR_waitpid			(MIPS__NR_Linux +	7)
#define MIPS__NR_creat			(MIPS__NR_Linux +	8)
#define MIPS__NR_link			(MIPS__NR_Linux +	9)
#define MIPS__NR_unlink			(MIPS__NR_Linux +  10)
#define MIPS__NR_execve			(MIPS__NR_Linux +  11)
#define MIPS__NR_chdir			(MIPS__NR_Linux +  12)
#define MIPS__NR_time			(MIPS__NR_Linux +  13)
#define MIPS__NR_mknod			(MIPS__NR_Linux +  14)
#define MIPS__NR_chmod			(MIPS__NR_Linux +  15)
#define MIPS__NR_lchown			(MIPS__NR_Linux +  16)
#define MIPS__NR_break			(MIPS__NR_Linux +  17)
#define MIPS__NR_unused18			(MIPS__NR_Linux +  18)
#define MIPS__NR_lseek			(MIPS__NR_Linux +  19)
#define MIPS__NR_getpid			(MIPS__NR_Linux +  20)
#define MIPS__NR_mount			(MIPS__NR_Linux +  21)
#define MIPS__NR_umount			(MIPS__NR_Linux +  22)
#define MIPS__NR_setuid			(MIPS__NR_Linux +  23)
#define MIPS__NR_getuid			(MIPS__NR_Linux +  24)
#define MIPS__NR_stime			(MIPS__NR_Linux +  25)
#define MIPS__NR_ptrace			(MIPS__NR_Linux +  26)
#define MIPS__NR_alarm			(MIPS__NR_Linux +  27)
#define MIPS__NR_unused28			(MIPS__NR_Linux +  28)
#define MIPS__NR_pause			(MIPS__NR_Linux +  29)
#define MIPS__NR_utime			(MIPS__NR_Linux +  30)
#define MIPS__NR_stty			(MIPS__NR_Linux +  31)
#define MIPS__NR_gtty			(MIPS__NR_Linux +  32)
#define MIPS__NR_access			(MIPS__NR_Linux +  33)
#define MIPS__NR_nice			(MIPS__NR_Linux +  34)
#define MIPS__NR_ftime			(MIPS__NR_Linux +  35)
#define MIPS__NR_sync			(MIPS__NR_Linux +  36)
#define MIPS__NR_kill			(MIPS__NR_Linux +  37)
#define MIPS__NR_rename			(MIPS__NR_Linux +  38)
#define MIPS__NR_mkdir			(MIPS__NR_Linux +  39)
#define MIPS__NR_rmdir			(MIPS__NR_Linux +  40)
#define MIPS__NR_dup			(MIPS__NR_Linux +  41)
#define MIPS__NR_pipe			(MIPS__NR_Linux +  42)
#define MIPS__NR_times			(MIPS__NR_Linux +  43)
#define MIPS__NR_prof			(MIPS__NR_Linux +  44)
#define MIPS__NR_brk			(MIPS__NR_Linux +  45)
#define MIPS__NR_setgid			(MIPS__NR_Linux +  46)
#define MIPS__NR_getgid			(MIPS__NR_Linux +  47)
#define MIPS__NR_signal			(MIPS__NR_Linux +  48)
#define MIPS__NR_geteuid			(MIPS__NR_Linux +  49)
#define MIPS__NR_getegid			(MIPS__NR_Linux +  50)
#define MIPS__NR_acct			(MIPS__NR_Linux +  51)
#define MIPS__NR_umount2			(MIPS__NR_Linux +  52)
#define MIPS__NR_lock			(MIPS__NR_Linux +  53)
#define MIPS__NR_ioctl			(MIPS__NR_Linux +  54)
#define MIPS__NR_fcntl			(MIPS__NR_Linux +  55)
#define MIPS__NR_mpx			(MIPS__NR_Linux +  56)
#define MIPS__NR_setpgid			(MIPS__NR_Linux +  57)
#define MIPS__NR_ulimit			(MIPS__NR_Linux +  58)
#define MIPS__NR_unused59			(MIPS__NR_Linux +  59)
#define MIPS__NR_umask			(MIPS__NR_Linux +  60)
#define MIPS__NR_chroot			(MIPS__NR_Linux +  61)
#define MIPS__NR_ustat			(MIPS__NR_Linux +  62)
#define MIPS__NR_dup2			(MIPS__NR_Linux +  63)
#define MIPS__NR_getppid			(MIPS__NR_Linux +  64)
#define MIPS__NR_getpgrp			(MIPS__NR_Linux +  65)
#define MIPS__NR_setsid			(MIPS__NR_Linux +  66)
#define MIPS__NR_sigaction			(MIPS__NR_Linux +  67)
#define MIPS__NR_sgetmask			(MIPS__NR_Linux +  68)
#define MIPS__NR_ssetmask			(MIPS__NR_Linux +  69)
#define MIPS__NR_setreuid			(MIPS__NR_Linux +  70)
#define MIPS__NR_setregid			(MIPS__NR_Linux +  71)
#define MIPS__NR_sigsuspend			(MIPS__NR_Linux +  72)
#define MIPS__NR_sigpending			(MIPS__NR_Linux +  73)
#define MIPS__NR_sethostname		(MIPS__NR_Linux +  74)
#define MIPS__NR_setrlimit			(MIPS__NR_Linux +  75)
#define MIPS__NR_getrlimit			(MIPS__NR_Linux +  76)
#define MIPS__NR_getrusage			(MIPS__NR_Linux +  77)
#define MIPS__NR_gettimeofday		(MIPS__NR_Linux +  78)
#define MIPS__NR_settimeofday		(MIPS__NR_Linux +  79)
#define MIPS__NR_getgroups			(MIPS__NR_Linux +  80)
#define MIPS__NR_setgroups			(MIPS__NR_Linux +  81)
#define MIPS__NR_reserved82			(MIPS__NR_Linux +  82)
#define MIPS__NR_symlink			(MIPS__NR_Linux +  83)
#define MIPS__NR_unused84			(MIPS__NR_Linux +  84)
#define MIPS__NR_readlink			(MIPS__NR_Linux +  85)
#define MIPS__NR_uselib			(MIPS__NR_Linux +  86)
#define MIPS__NR_swapon			(MIPS__NR_Linux +  87)
#define MIPS__NR_reboot			(MIPS__NR_Linux +  88)
#define MIPS__NR_readdir			(MIPS__NR_Linux +  89)
#define MIPS__NR_mmap			(MIPS__NR_Linux +  90)
#define MIPS__NR_munmap			(MIPS__NR_Linux +  91)
#define MIPS__NR_truncate			(MIPS__NR_Linux +  92)
#define MIPS__NR_ftruncate			(MIPS__NR_Linux +  93)
#define MIPS__NR_fchmod			(MIPS__NR_Linux +  94)
#define MIPS__NR_fchown			(MIPS__NR_Linux +  95)
#define MIPS__NR_getpriority		(MIPS__NR_Linux +  96)
#define MIPS__NR_setpriority		(MIPS__NR_Linux +  97)
#define MIPS__NR_profil			(MIPS__NR_Linux +  98)
#define MIPS__NR_statfs			(MIPS__NR_Linux +  99)
#define MIPS__NR_fstatfs			(MIPS__NR_Linux + 100)
#define MIPS__NR_ioperm			(MIPS__NR_Linux + 101)
#define MIPS__NR_socketcall			(MIPS__NR_Linux + 102)
#define MIPS__NR_syslog			(MIPS__NR_Linux + 103)
#define MIPS__NR_setitimer			(MIPS__NR_Linux + 104)
#define MIPS__NR_getitimer			(MIPS__NR_Linux + 105)
#define MIPS__NR_stat			(MIPS__NR_Linux + 106)
#define MIPS__NR_lstat			(MIPS__NR_Linux + 107)
#define MIPS__NR_fstat			(MIPS__NR_Linux + 108)
#define MIPS__NR_unused109			(MIPS__NR_Linux + 109)
#define MIPS__NR_iopl			(MIPS__NR_Linux + 110)
#define MIPS__NR_vhangup			(MIPS__NR_Linux + 111)
#define MIPS__NR_idle			(MIPS__NR_Linux + 112)
#define MIPS__NR_vm86			(MIPS__NR_Linux + 113)
#define MIPS__NR_wait4			(MIPS__NR_Linux + 114)
#define MIPS__NR_swapoff			(MIPS__NR_Linux + 115)
#define MIPS__NR_sysinfo			(MIPS__NR_Linux + 116)
#define MIPS__NR_ipc			(MIPS__NR_Linux + 117)
#define MIPS__NR_fsync			(MIPS__NR_Linux + 118)
#define MIPS__NR_sigreturn			(MIPS__NR_Linux + 119)
#define MIPS__NR_clone			(MIPS__NR_Linux + 120)
#define MIPS__NR_setdomainname		(MIPS__NR_Linux + 121)
#define MIPS__NR_uname			(MIPS__NR_Linux + 122)
#define MIPS__NR_modify_ldt			(MIPS__NR_Linux + 123)
#define MIPS__NR_adjtimex			(MIPS__NR_Linux + 124)
#define MIPS__NR_mprotect			(MIPS__NR_Linux + 125)
#define MIPS__NR_sigprocmask		(MIPS__NR_Linux + 126)
#define MIPS__NR_create_module		(MIPS__NR_Linux + 127)
#define MIPS__NR_init_module		(MIPS__NR_Linux + 128)
#define MIPS__NR_delete_module		(MIPS__NR_Linux + 129)
#define MIPS__NR_get_kernel_syms		(MIPS__NR_Linux + 130)
#define MIPS__NR_quotactl			(MIPS__NR_Linux + 131)
#define MIPS__NR_getpgid			(MIPS__NR_Linux + 132)
#define MIPS__NR_fchdir			(MIPS__NR_Linux + 133)
#define MIPS__NR_bdflush			(MIPS__NR_Linux + 134)
#define MIPS__NR_sysfs			(MIPS__NR_Linux + 135)
#define MIPS__NR_personality		(MIPS__NR_Linux + 136)
#define MIPS__NR_afs_syscall		(MIPS__NR_Linux + 137) /* Syscall for Andrew File System */
#define MIPS__NR_setfsuid			(MIPS__NR_Linux + 138)
#define MIPS__NR_setfsgid			(MIPS__NR_Linux + 139)
#define MIPS__NR__llseek			(MIPS__NR_Linux + 140)
#define MIPS__NR_getdents			(MIPS__NR_Linux + 141)
#define MIPS__NR__newselect			(MIPS__NR_Linux + 142)
#define MIPS__NR_flock			(MIPS__NR_Linux + 143)
#define MIPS__NR_msync			(MIPS__NR_Linux + 144)
#define MIPS__NR_readv			(MIPS__NR_Linux + 145)
#define MIPS__NR_writev			(MIPS__NR_Linux + 146)
#define MIPS__NR_cacheflush			(MIPS__NR_Linux + 147)
#define MIPS__NR_cachectl			(MIPS__NR_Linux + 148)
#define MIPS__NR_sysmips			(MIPS__NR_Linux + 149)
#define MIPS__NR_unused150			(MIPS__NR_Linux + 150)
#define MIPS__NR_getsid			(MIPS__NR_Linux + 151)
#define MIPS__NR_fdatasync			(MIPS__NR_Linux + 152)
#define MIPS__NR__sysctl			(MIPS__NR_Linux + 153)
#define MIPS__NR_mlock			(MIPS__NR_Linux + 154)
#define MIPS__NR_munlock			(MIPS__NR_Linux + 155)
#define MIPS__NR_mlockall			(MIPS__NR_Linux + 156)
#define MIPS__NR_munlockall			(MIPS__NR_Linux + 157)
#define MIPS__NR_sched_setparam		(MIPS__NR_Linux + 158)
#define MIPS__NR_sched_getparam		(MIPS__NR_Linux + 159)
#define MIPS__NR_sched_setscheduler		(MIPS__NR_Linux + 160)
#define MIPS__NR_sched_getscheduler		(MIPS__NR_Linux + 161)
#define MIPS__NR_sched_yield		(MIPS__NR_Linux + 162)
#define MIPS__NR_sched_get_priority_max	(MIPS__NR_Linux + 163)
#define MIPS__NR_sched_get_priority_min	(MIPS__NR_Linux + 164)
#define MIPS__NR_sched_rr_get_interval	(MIPS__NR_Linux + 165)
#define MIPS__NR_nanosleep			(MIPS__NR_Linux + 166)
#define MIPS__NR_mremap			(MIPS__NR_Linux + 167)
#define MIPS__NR_accept			(MIPS__NR_Linux + 168)
#define MIPS__NR_bind			(MIPS__NR_Linux + 169)
#define MIPS__NR_connect			(MIPS__NR_Linux + 170)
#define MIPS__NR_getpeername		(MIPS__NR_Linux + 171)
#define MIPS__NR_getsockname		(MIPS__NR_Linux + 172)
#define MIPS__NR_getsockopt			(MIPS__NR_Linux + 173)
#define MIPS__NR_listen			(MIPS__NR_Linux + 174)
#define MIPS__NR_recv			(MIPS__NR_Linux + 175)
#define MIPS__NR_recvfrom			(MIPS__NR_Linux + 176)
#define MIPS__NR_recvmsg			(MIPS__NR_Linux + 177)
#define MIPS__NR_send			(MIPS__NR_Linux + 178)
#define MIPS__NR_sendmsg			(MIPS__NR_Linux + 179)
#define MIPS__NR_sendto			(MIPS__NR_Linux + 180)
#define MIPS__NR_setsockopt			(MIPS__NR_Linux + 181)
#define MIPS__NR_shutdown			(MIPS__NR_Linux + 182)
#define MIPS__NR_socket			(MIPS__NR_Linux + 183)
#define MIPS__NR_socketpair			(MIPS__NR_Linux + 184)
#define MIPS__NR_setresuid			(MIPS__NR_Linux + 185)
#define MIPS__NR_getresuid			(MIPS__NR_Linux + 186)
#define MIPS__NR_query_module		(MIPS__NR_Linux + 187)
#define MIPS__NR_poll			(MIPS__NR_Linux + 188)
#define MIPS__NR_nfsservctl			(MIPS__NR_Linux + 189)
#define MIPS__NR_setresgid			(MIPS__NR_Linux + 190)
#define MIPS__NR_getresgid			(MIPS__NR_Linux + 191)
#define MIPS__NR_prctl			(MIPS__NR_Linux + 192)
#define MIPS__NR_rt_sigreturn		(MIPS__NR_Linux + 193)
#define MIPS__NR_rt_sigaction		(MIPS__NR_Linux + 194)
#define MIPS__NR_rt_sigprocmask		(MIPS__NR_Linux + 195)
#define MIPS__NR_rt_sigpending		(MIPS__NR_Linux + 196)
#define MIPS__NR_rt_sigtimedwait		(MIPS__NR_Linux + 197)
#define MIPS__NR_rt_sigqueueinfo		(MIPS__NR_Linux + 198)
#define MIPS__NR_rt_sigsuspend		(MIPS__NR_Linux + 199)
#define MIPS__NR_pread64			(MIPS__NR_Linux + 200)
#define MIPS__NR_pwrite64			(MIPS__NR_Linux + 201)
#define MIPS__NR_chown			(MIPS__NR_Linux + 202)
#define MIPS__NR_getcwd			(MIPS__NR_Linux + 203)
#define MIPS__NR_capget			(MIPS__NR_Linux + 204)
#define MIPS__NR_capset			(MIPS__NR_Linux + 205)
#define MIPS__NR_sigaltstack		(MIPS__NR_Linux + 206)
#define MIPS__NR_sendfile			(MIPS__NR_Linux + 207)
#define MIPS__NR_getpmsg			(MIPS__NR_Linux + 208)
#define MIPS__NR_putpmsg			(MIPS__NR_Linux + 209)
#define MIPS__NR_mmap2			(MIPS__NR_Linux + 210)
#define MIPS__NR_truncate64			(MIPS__NR_Linux + 211)
#define MIPS__NR_ftruncate64		(MIPS__NR_Linux + 212)
#define MIPS__NR_stat64			(MIPS__NR_Linux + 213)
#define MIPS__NR_lstat64			(MIPS__NR_Linux + 214)
#define MIPS__NR_fstat64			(MIPS__NR_Linux + 215)
#define MIPS__NR_pivot_root			(MIPS__NR_Linux + 216)
#define MIPS__NR_mincore			(MIPS__NR_Linux + 217)
#define MIPS__NR_madvise			(MIPS__NR_Linux + 218)
#define MIPS__NR_getdents64			(MIPS__NR_Linux + 219)
#define MIPS__NR_fcntl64			(MIPS__NR_Linux + 220)
#define MIPS__NR_reserved221		(MIPS__NR_Linux + 221)
#define MIPS__NR_gettid			(MIPS__NR_Linux + 222)
#define MIPS__NR_readahead			(MIPS__NR_Linux + 223)
#define MIPS__NR_setxattr			(MIPS__NR_Linux + 224)
#define MIPS__NR_lsetxattr			(MIPS__NR_Linux + 225)
#define MIPS__NR_fsetxattr			(MIPS__NR_Linux + 226)
#define MIPS__NR_getxattr			(MIPS__NR_Linux + 227)
#define MIPS__NR_lgetxattr			(MIPS__NR_Linux + 228)
#define MIPS__NR_fgetxattr			(MIPS__NR_Linux + 229)
#define MIPS__NR_listxattr			(MIPS__NR_Linux + 230)
#define MIPS__NR_llistxattr			(MIPS__NR_Linux + 231)
#define MIPS__NR_flistxattr			(MIPS__NR_Linux + 232)
#define MIPS__NR_removexattr		(MIPS__NR_Linux + 233)
#define MIPS__NR_lremovexattr		(MIPS__NR_Linux + 234)
#define MIPS__NR_fremovexattr		(MIPS__NR_Linux + 235)
#define MIPS__NR_tkill			(MIPS__NR_Linux + 236)
#define MIPS__NR_sendfile64			(MIPS__NR_Linux + 237)
#define MIPS__NR_futex			(MIPS__NR_Linux + 238)
#define MIPS__NR_sched_setaffinity		(MIPS__NR_Linux + 239)
#define MIPS__NR_sched_getaffinity		(MIPS__NR_Linux + 240)
#define MIPS__NR_io_setup			(MIPS__NR_Linux + 241)
#define MIPS__NR_io_destroy			(MIPS__NR_Linux + 242)
#define MIPS__NR_io_getevents		(MIPS__NR_Linux + 243)
#define MIPS__NR_io_submit			(MIPS__NR_Linux + 244)
#define MIPS__NR_io_cancel			(MIPS__NR_Linux + 245)
#define MIPS__NR_exit_group			(MIPS__NR_Linux + 246)
#define MIPS__NR_lookup_dcookie		(MIPS__NR_Linux + 247)
#define MIPS__NR_epoll_create		(MIPS__NR_Linux + 248)
#define MIPS__NR_epoll_ctl			(MIPS__NR_Linux + 249)
#define MIPS__NR_epoll_wait			(MIPS__NR_Linux + 250)
#define MIPS__NR_remap_file_pages		(MIPS__NR_Linux + 251)
#define MIPS__NR_set_tid_address		(MIPS__NR_Linux + 252)
#define MIPS__NR_restart_syscall		(MIPS__NR_Linux + 253)
#define MIPS__NR_fadvise64			(MIPS__NR_Linux + 254)
#define MIPS__NR_statfs64			(MIPS__NR_Linux + 255)
#define MIPS__NR_fstatfs64			(MIPS__NR_Linux + 256)
#define MIPS__NR_timer_create		(MIPS__NR_Linux + 257)
#define MIPS__NR_timer_settime		(MIPS__NR_Linux + 258)
#define MIPS__NR_timer_gettime		(MIPS__NR_Linux + 259)
#define MIPS__NR_timer_getoverrun		(MIPS__NR_Linux + 260)
#define MIPS__NR_timer_delete		(MIPS__NR_Linux + 261)
#define MIPS__NR_clock_settime		(MIPS__NR_Linux + 262)
#define MIPS__NR_clock_gettime		(MIPS__NR_Linux + 263)
#define MIPS__NR_clock_getres		(MIPS__NR_Linux + 264)
#define MIPS__NR_clock_nanosleep		(MIPS__NR_Linux + 265)
#define MIPS__NR_tgkill			(MIPS__NR_Linux + 266)
#define MIPS__NR_utimes			(MIPS__NR_Linux + 267)
#define MIPS__NR_mbind			(MIPS__NR_Linux + 268)
#define MIPS__NR_get_mempolicy		(MIPS__NR_Linux + 269)
#define MIPS__NR_set_mempolicy		(MIPS__NR_Linux + 270)
#define MIPS__NR_mq_open			(MIPS__NR_Linux + 271)
#define MIPS__NR_mq_unlink			(MIPS__NR_Linux + 272)
#define MIPS__NR_mq_timedsend		(MIPS__NR_Linux + 273)
#define MIPS__NR_mq_timedreceive		(MIPS__NR_Linux + 274)
#define MIPS__NR_mq_notify			(MIPS__NR_Linux + 275)
#define MIPS__NR_mq_getsetattr		(MIPS__NR_Linux + 276)
#define MIPS__NR_vserver			(MIPS__NR_Linux + 277)
#define MIPS__NR_waitid			(MIPS__NR_Linux + 278)
/* #define MIPS__NR_sys_setaltroot		(MIPS__NR_Linux + 279) */
#define MIPS__NR_add_key			(MIPS__NR_Linux + 280)
#define MIPS__NR_request_key		(MIPS__NR_Linux + 281)
#define MIPS__NR_keyctl			(MIPS__NR_Linux + 282)
#define MIPS__NR_set_thread_area		(MIPS__NR_Linux + 283)
#define MIPS__NR_inotify_init		(MIPS__NR_Linux + 284)
#define MIPS__NR_inotify_add_watch		(MIPS__NR_Linux + 285)
#define MIPS__NR_inotify_rm_watch		(MIPS__NR_Linux + 286)
#define MIPS__NR_migrate_pages		(MIPS__NR_Linux + 287)
#define MIPS__NR_openat			(MIPS__NR_Linux + 288)
#define MIPS__NR_mkdirat			(MIPS__NR_Linux + 289)
#define MIPS__NR_mknodat			(MIPS__NR_Linux + 290)
#define MIPS__NR_fchownat			(MIPS__NR_Linux + 291)
#define MIPS__NR_futimesat			(MIPS__NR_Linux + 292)
#define MIPS__NR_fstatat64			(MIPS__NR_Linux + 293)
#define MIPS__NR_unlinkat			(MIPS__NR_Linux + 294)
#define MIPS__NR_renameat			(MIPS__NR_Linux + 295)
#define MIPS__NR_linkat			(MIPS__NR_Linux + 296)
#define MIPS__NR_symlinkat			(MIPS__NR_Linux + 297)
#define MIPS__NR_readlinkat			(MIPS__NR_Linux + 298)
#define MIPS__NR_fchmodat			(MIPS__NR_Linux + 299)
#define MIPS__NR_faccessat			(MIPS__NR_Linux + 300)
#define MIPS__NR_pselect6			(MIPS__NR_Linux + 301)
#define MIPS__NR_ppoll			(MIPS__NR_Linux + 302)
#define MIPS__NR_unshare			(MIPS__NR_Linux + 303)
#define MIPS__NR_splice			(MIPS__NR_Linux + 304)
#define MIPS__NR_sync_file_range		(MIPS__NR_Linux + 305)
#define MIPS__NR_tee			(MIPS__NR_Linux + 306)
#define MIPS__NR_vmsplice			(MIPS__NR_Linux + 307)
#define MIPS__NR_move_pages			(MIPS__NR_Linux + 308)
#define MIPS__NR_set_robust_list		(MIPS__NR_Linux + 309)
#define MIPS__NR_get_robust_list		(MIPS__NR_Linux + 310)
#define MIPS__NR_kexec_load			(MIPS__NR_Linux + 311)
#define MIPS__NR_getcpu			(MIPS__NR_Linux + 312)
#define MIPS__NR_epoll_pwait		(MIPS__NR_Linux + 313)
#define MIPS__NR_ioprio_set			(MIPS__NR_Linux + 314)
#define MIPS__NR_ioprio_get			(MIPS__NR_Linux + 315)
#define MIPS__NR_utimensat			(MIPS__NR_Linux + 316)
#define MIPS__NR_signalfd			(MIPS__NR_Linux + 317)
#define MIPS__NR_timerfd			(MIPS__NR_Linux + 318)
#define MIPS__NR_eventfd			(MIPS__NR_Linux + 319)
#define MIPS__NR_fallocate			(MIPS__NR_Linux + 320)
#define MIPS__NR_timerfd_create		(MIPS__NR_Linux + 321)
#define MIPS__NR_timerfd_gettime		(MIPS__NR_Linux + 322)
#define MIPS__NR_timerfd_settime		(MIPS__NR_Linux + 323)
#define MIPS__NR_signalfd4			(MIPS__NR_Linux + 324)
#define MIPS__NR_eventfd2			(MIPS__NR_Linux + 325)
#define MIPS__NR_epoll_create1		(MIPS__NR_Linux + 326)
#define MIPS__NR_dup3			(MIPS__NR_Linux + 327)
#define MIPS__NR_pipe2			(MIPS__NR_Linux + 328)
#define MIPS__NR_inotify_init1		(MIPS__NR_Linux + 329)
#define MIPS__NR_preadv			(MIPS__NR_Linux + 330)
#define MIPS__NR_pwritev			(MIPS__NR_Linux + 331)
#define MIPS__NR_rt_tgsigqueueinfo		(MIPS__NR_Linux + 332)
#define MIPS__NR_perf_event_open		(MIPS__NR_Linux + 333)
#define MIPS__NR_accept4			(MIPS__NR_Linux + 334)
#define MIPS__NR_recvmmsg			(MIPS__NR_Linux + 335)
#define MIPS__NR_fanotify_init		(MIPS__NR_Linux + 336)
#define MIPS__NR_fanotify_mark		(MIPS__NR_Linux + 337)
#define MIPS__NR_prlimit64			(MIPS__NR_Linux + 338)
#define MIPS__NR_name_to_handle_at		(MIPS__NR_Linux + 339)
#define MIPS__NR_open_by_handle_at		(MIPS__NR_Linux + 340)
#define MIPS__NR_clock_adjtime		(MIPS__NR_Linux + 341)
#define MIPS__NR_syncfs			(MIPS__NR_Linux + 342)
#define MIPS__NR_sendmmsg			(MIPS__NR_Linux + 343)
#define MIPS__NR_setns			(MIPS__NR_Linux + 344)
#define MIPS__NR_process_vm_readv		(MIPS__NR_Linux + 345)
#define MIPS__NR_process_vm_writev		(MIPS__NR_Linux + 346)
#define MIPS__NR_kcmp			(MIPS__NR_Linux + 347)
#define MIPS__NR_finit_module		(MIPS__NR_Linux + 348)

/*
 * Offset of the last Linux o32 flavoured syscall
 */
#define MIPS__NR_Linux_syscalls		348


int *mips_syscall::get_syscall_table() {
  static int syscall_table[] = {
    MIPS__NR_restart_syscall,
    MIPS__NR_exit,
    MIPS__NR_fork,
    MIPS__NR_read,
    MIPS__NR_write,
    MIPS__NR_open,
    MIPS__NR_close,
    MIPS__NR_creat,
    MIPS__NR_time,
    MIPS__NR_lseek,
    MIPS__NR_getpid,
    MIPS__NR_access,
    MIPS__NR_kill,
    MIPS__NR_dup,
    MIPS__NR_times,
    MIPS__NR_brk,
    MIPS__NR_mmap,
    MIPS__NR_munmap,
    MIPS__NR_stat,
    MIPS__NR_lstat,
    MIPS__NR_fstat,
    MIPS__NR_uname,
    MIPS__NR__llseek,
    MIPS__NR_readv,
    MIPS__NR_writev,
    MIPS__NR_mmap2,
    MIPS__NR_stat64,
    MIPS__NR_lstat64,
    MIPS__NR_fstat64,
    MIPS__NR_getuid,
    MIPS__NR_getgid,
    MIPS__NR_geteuid,
    MIPS__NR_getegid,
    MIPS__NR_fcntl64,
    MIPS__NR_exit_group,
    MIPS__NR_socketcall,
    MIPS__NR_gettimeofday,
    MIPS__NR_settimeofday,
    MIPS__NR_clock_gettime
  };
  return syscall_table;
}
