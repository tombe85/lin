cmd_/home/kernel/lin/github/lin/PF/Miguel/cbuffer.o := gcc -Wp,-MD,/home/kernel/lin/github/lin/PF/Miguel/.cbuffer.o.d  -nostdinc -isystem /usr/lib/gcc/x86_64-linux-gnu/4.7/include -I/home/kernel/kitkat-src/kernel/arch/x86/include -Iarch/x86/include/generated  -I/home/kernel/kitkat-src/kernel/include -Iinclude -I/home/kernel/kitkat-src/kernel/arch/x86/include/uapi -Iarch/x86/include/generated/uapi -I/home/kernel/kitkat-src/kernel/include/uapi -Iinclude/generated/uapi -include /home/kernel/kitkat-src/kernel/include/linux/kconfig.h   -I/home/kernel/lin/github/lin/PF/Miguel -D__KERNEL__ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -Wno-maybe-uninitialized -m32 -msoft-float -mregparm=3 -freg-struct-return -mno-mmx -mno-sse -fno-pic -mpreferred-stack-boundary=2 -march=i686 -mtune=pentium3 -mtune=generic -Wa,-mtune=generic32 -ffreestanding -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -DCONFIG_AS_CFI_SECTIONS=1 -DCONFIG_AS_AVX=1 -DCONFIG_AS_AVX2=1 -pipe -Wno-sign-compare -fno-asynchronous-unwind-tables -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -Wframe-larger-than=1024 -fno-stack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -fno-var-tracking-assignments -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -DCC_HAVE_ASM_GOTO  -DMODULE  -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(cbuffer)"  -D"KBUILD_MODNAME=KBUILD_STR(fifomultimod)" -c -o /home/kernel/lin/github/lin/PF/Miguel/.tmp_cbuffer.o /home/kernel/lin/github/lin/PF/Miguel/cbuffer.c

source_/home/kernel/lin/github/lin/PF/Miguel/cbuffer.o := /home/kernel/lin/github/lin/PF/Miguel/cbuffer.c

deps_/home/kernel/lin/github/lin/PF/Miguel/cbuffer.o := \
  /home/kernel/lin/github/lin/PF/Miguel/cbuffer.h \
  /home/kernel/kitkat-src/kernel/include/linux/vmalloc.h \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/smp.h) \
  /home/kernel/kitkat-src/kernel/include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  /home/kernel/kitkat-src/kernel/include/linux/typecheck.h \
  /home/kernel/kitkat-src/kernel/include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/context/tracking.h) \
    $(wildcard include/config/preempt/count.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  /home/kernel/kitkat-src/kernel/include/linux/thread_info.h \
    $(wildcard include/config/compat.h) \
    $(wildcard include/config/debug/stack/usage.h) \
  /home/kernel/kitkat-src/kernel/include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/kernel/kitkat-src/kernel/include/uapi/linux/types.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/uapi/asm/types.h \
  /home/kernel/kitkat-src/kernel/include/uapi/asm-generic/types.h \
  /home/kernel/kitkat-src/kernel/include/asm-generic/int-ll64.h \
  /home/kernel/kitkat-src/kernel/include/uapi/asm-generic/int-ll64.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/uapi/asm/bitsperlong.h \
  /home/kernel/kitkat-src/kernel/include/asm-generic/bitsperlong.h \
  /home/kernel/kitkat-src/kernel/include/uapi/asm-generic/bitsperlong.h \
  /home/kernel/kitkat-src/kernel/include/uapi/linux/posix_types.h \
  /home/kernel/kitkat-src/kernel/include/linux/stddef.h \
  /home/kernel/kitkat-src/kernel/include/uapi/linux/stddef.h \
  /home/kernel/kitkat-src/kernel/include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
    $(wildcard include/config/kprobes.h) \
  /home/kernel/kitkat-src/kernel/include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  /home/kernel/kitkat-src/kernel/include/linux/compiler-gcc4.h \
    $(wildcard include/config/arch/use/builtin/bswap.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/posix_types.h \
    $(wildcard include/config/x86/32.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/uapi/asm/posix_types_32.h \
  /home/kernel/kitkat-src/kernel/include/uapi/asm-generic/posix_types.h \
  /home/kernel/kitkat-src/kernel/include/linux/bug.h \
    $(wildcard include/config/generic/bug.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  /home/kernel/kitkat-src/kernel/include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  /home/kernel/kitkat-src/kernel/include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/atomic/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  /usr/lib/gcc/x86_64-linux-gnu/4.7/include/stdarg.h \
  /home/kernel/kitkat-src/kernel/include/linux/linkage.h \
  /home/kernel/kitkat-src/kernel/include/linux/stringify.h \
  /home/kernel/kitkat-src/kernel/include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/linkage.h \
    $(wildcard include/config/x86/64.h) \
    $(wildcard include/config/x86/alignment/16.h) \
  /home/kernel/kitkat-src/kernel/include/linux/bitops.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/bitops.h \
    $(wildcard include/config/x86/cmov.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/alternative.h \
    $(wildcard include/config/paravirt.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/asm.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/cpufeature.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/required-features.h \
    $(wildcard include/config/x86/minimum/cpu/family.h) \
    $(wildcard include/config/math/emulation.h) \
    $(wildcard include/config/x86/pae.h) \
    $(wildcard include/config/x86/cmpxchg64.h) \
    $(wildcard include/config/x86/use/3dnow.h) \
    $(wildcard include/config/x86/p6/nop.h) \
    $(wildcard include/config/matom.h) \
  /home/kernel/kitkat-src/kernel/include/asm-generic/bitops/fls64.h \
  /home/kernel/kitkat-src/kernel/include/asm-generic/bitops/find.h \
    $(wildcard include/config/generic/find/first/bit.h) \
  /home/kernel/kitkat-src/kernel/include/asm-generic/bitops/sched.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/arch_hweight.h \
  /home/kernel/kitkat-src/kernel/include/asm-generic/bitops/const_hweight.h \
  /home/kernel/kitkat-src/kernel/include/asm-generic/bitops/le.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/uapi/asm/byteorder.h \
  /home/kernel/kitkat-src/kernel/include/linux/byteorder/little_endian.h \
  /home/kernel/kitkat-src/kernel/include/uapi/linux/byteorder/little_endian.h \
  /home/kernel/kitkat-src/kernel/include/linux/swab.h \
  /home/kernel/kitkat-src/kernel/include/uapi/linux/swab.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/uapi/asm/swab.h \
  /home/kernel/kitkat-src/kernel/include/linux/byteorder/generic.h \
  /home/kernel/kitkat-src/kernel/include/asm-generic/bitops/ext2-atomic-setbit.h \
  /home/kernel/kitkat-src/kernel/include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  /home/kernel/kitkat-src/kernel/include/linux/printk.h \
    $(wildcard include/config/early/printk.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
  /home/kernel/kitkat-src/kernel/include/linux/init.h \
    $(wildcard include/config/broken/rodata.h) \
  /home/kernel/kitkat-src/kernel/include/linux/kern_levels.h \
  /home/kernel/kitkat-src/kernel/include/linux/dynamic_debug.h \
  /home/kernel/kitkat-src/kernel/include/uapi/linux/kernel.h \
  /home/kernel/kitkat-src/kernel/include/uapi/linux/sysinfo.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/div64.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/thread_info.h \
    $(wildcard include/config/ia32/emulation.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/page.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/page_types.h \
  /home/kernel/kitkat-src/kernel/include/uapi/linux/const.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/page_32_types.h \
    $(wildcard include/config/highmem4g.h) \
    $(wildcard include/config/highmem64g.h) \
    $(wildcard include/config/page/offset.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/page_32.h \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/debug/virtual.h) \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/x86/3dnow.h) \
  /home/kernel/kitkat-src/kernel/include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  /home/kernel/kitkat-src/kernel/include/uapi/linux/string.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/string.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/string_32.h \
    $(wildcard include/config/kmemcheck.h) \
  /home/kernel/kitkat-src/kernel/include/linux/range.h \
  /home/kernel/kitkat-src/kernel/include/asm-generic/memory_model.h \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/sparsemem/vmemmap.h) \
    $(wildcard include/config/sparsemem.h) \
  /home/kernel/kitkat-src/kernel/include/asm-generic/getorder.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/processor.h \
    $(wildcard include/config/x86/vsmp.h) \
    $(wildcard include/config/cc/stackprotector.h) \
    $(wildcard include/config/m486.h) \
    $(wildcard include/config/x86/debugctlmsr.h) \
    $(wildcard include/config/xen.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/processor-flags.h \
    $(wildcard include/config/vm86.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/uapi/asm/processor-flags.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/vm86.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/ptrace.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/segment.h \
    $(wildcard include/config/x86/32/lazy/gs.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/uapi/asm/ptrace.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/uapi/asm/ptrace-abi.h \
  /home/kernel/kitkat-src/kernel/include/asm-generic/ptrace.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/uapi/asm/vm86.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/math_emu.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/sigcontext.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/uapi/asm/sigcontext.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/current.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/percpu.h \
    $(wildcard include/config/x86/64/smp.h) \
  /home/kernel/kitkat-src/kernel/include/asm-generic/percpu.h \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  /home/kernel/kitkat-src/kernel/include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  /home/kernel/kitkat-src/kernel/include/linux/percpu-defs.h \
    $(wildcard include/config/debug/force/weak/per/cpu.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/pgtable_types.h \
    $(wildcard include/config/compat/vdso.h) \
    $(wildcard include/config/proc/fs.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/pgtable_32_types.h \
    $(wildcard include/config/highmem.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/pgtable-2level_types.h \
  /home/kernel/kitkat-src/kernel/include/asm-generic/pgtable-nopud.h \
  /home/kernel/kitkat-src/kernel/include/asm-generic/pgtable-nopmd.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/msr.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/uapi/asm/msr.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/uapi/asm/msr-index.h \
  /home/kernel/kitkat-src/kernel/include/uapi/linux/ioctl.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/uapi/asm/ioctl.h \
  /home/kernel/kitkat-src/kernel/include/asm-generic/ioctl.h \
  /home/kernel/kitkat-src/kernel/include/uapi/asm-generic/ioctl.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/uapi/asm/errno.h \
  /home/kernel/kitkat-src/kernel/include/uapi/asm-generic/errno.h \
  /home/kernel/kitkat-src/kernel/include/uapi/asm-generic/errno-base.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/cpumask.h \
  /home/kernel/kitkat-src/kernel/include/linux/cpumask.h \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
    $(wildcard include/config/disable/obsolete/cpumask/functions.h) \
  /home/kernel/kitkat-src/kernel/include/linux/bitmap.h \
  /home/kernel/kitkat-src/kernel/include/linux/errno.h \
  /home/kernel/kitkat-src/kernel/include/uapi/linux/errno.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/desc_defs.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/nops.h \
    $(wildcard include/config/mk7.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/special_insns.h \
  /home/kernel/kitkat-src/kernel/include/linux/personality.h \
  /home/kernel/kitkat-src/kernel/include/uapi/linux/personality.h \
  /home/kernel/kitkat-src/kernel/include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/cache.h \
    $(wildcard include/config/x86/l1/cache/shift.h) \
    $(wildcard include/config/x86/internode/cache/shift.h) \
  /home/kernel/kitkat-src/kernel/include/linux/math64.h \
  /home/kernel/kitkat-src/kernel/include/linux/err.h \
  /home/kernel/kitkat-src/kernel/include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/irqflags.h \
  /home/kernel/kitkat-src/kernel/include/linux/atomic.h \
    $(wildcard include/config/arch/has/atomic/or.h) \
    $(wildcard include/config/generic/atomic64.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/atomic.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/cmpxchg.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/cmpxchg_32.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/atomic64_32.h \
  /home/kernel/kitkat-src/kernel/include/asm-generic/atomic-long.h \
  /home/kernel/kitkat-src/kernel/include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  /home/kernel/kitkat-src/kernel/include/linux/poison.h \
    $(wildcard include/config/illegal/pointer/value.h) \
  /home/kernel/kitkat-src/kernel/include/linux/bottom_half.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/barrier.h \
    $(wildcard include/config/x86/ppro/fence.h) \
    $(wildcard include/config/x86/oostore.h) \
  /home/kernel/kitkat-src/kernel/include/linux/spinlock_types.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/spinlock_types.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/rwlock.h \
  /home/kernel/kitkat-src/kernel/include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
    $(wildcard include/config/prove/rcu.h) \
  /home/kernel/kitkat-src/kernel/include/linux/rwlock_types.h \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/spinlock.h \
    $(wildcard include/config/paravirt/spinlocks.h) \
  /home/kernel/kitkat-src/kernel/arch/x86/include/asm/paravirt.h \
  /home/kernel/kitkat-src/kernel/include/linux/rwlock.h \
  /home/kernel/kitkat-src/kernel/include/linux/spinlock_api_smp.h \
    $(wildcard include/config/inline/spin/lock.h) \
    $(wildcard include/config/inline/spin/lock/bh.h) \
    $(wildcard include/config/inline/spin/lock/irq.h) \
    $(wildcard include/config/inline/spin/lock/irqsave.h) \
    $(wildcard include/config/inline/spin/trylock.h) \
    $(wildcard include/config/inline/spin/trylock/bh.h) \
    $(wildcard include/config/uninline/spin/unlock.h) \
    $(wildcard include/config/inline/spin/unlock/bh.h) \
    $(wildcard include/config/inline/spin/unlock/irq.h) \
    $(wildcard include/config/inline/spin/unlock/irqrestore.h) \
  /home/kernel/kitkat-src/kernel/include/linux/rwlock_api_smp.h \
    $(wildcard include/config/inline/read/lock.h) \
    $(wildcard include/config/inline/write/lock.h) \
    $(wildcard include/config/inline/read/lock/bh.h) \
    $(wildcard include/config/inline/write/lock/bh.h) \
    $(wildcard include/config/inline/read/lock/irq.h) \
    $(wildcard include/config/inline/write/lock/irq.h) \
    $(wildcard include/config/inline/read/lock/irqsave.h) \
    $(wildcard include/config/inline/write/lock/irqsave.h) \
    $(wildcard include/config/inline/read/trylock.h) \
    $(wildcard include/config/inline/write/trylock.h) \
    $(wildcard include/config/inline/read/unlock.h) \
    $(wildcard include/config/inline/write/unlock.h) \
    $(wildcard include/config/inline/read/unlock/bh.h) \
    $(wildcard include/config/inline/write/unlock/bh.h) \
    $(wildcard include/config/inline/read/unlock/irq.h) \
    $(wildcard include/config/inline/write/unlock/irq.h) \
    $(wildcard include/config/inline/read/unlock/irqrestore.h) \
    $(wildcard include/config/inline/write/unlock/irqrestore.h) \
  /home/kernel/kitkat-src/kernel/include/linux/rbtree.h \

/home/kernel/lin/github/lin/PF/Miguel/cbuffer.o: $(deps_/home/kernel/lin/github/lin/PF/Miguel/cbuffer.o)

$(deps_/home/kernel/lin/github/lin/PF/Miguel/cbuffer.o):
