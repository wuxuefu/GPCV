cmd_/home/wu/GPCV2159/openplatform/platform/gplus/cvr_turnkey_demo0/board_config.o := arm-none-linux-gnueabi-gcc -Wp,-MD,/home/wu/GPCV2159/openplatform/platform/gplus/cvr_turnkey_demo0/.board_config.o.d  -nostdinc -isystem /usr/local/arm/4.3.2/bin/../lib/gcc/arm-none-linux-gnueabi/4.3.2/include -Iinclude  -I/home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include -include include/linux/autoconf.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-gpl32900b/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -marm -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -D__LINUX_ARM_ARCH__=6 -march=armv6 -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -Uarm -fno-stack-protector -fomit-frame-pointer -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow  -DMODULE -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(board_config)"  -D"KBUILD_MODNAME=KBUILD_STR(board_config)"  -c -o /home/wu/GPCV2159/openplatform/platform/gplus/cvr_turnkey_demo0/.tmp_board_config.o /home/wu/GPCV2159/openplatform/platform/gplus/cvr_turnkey_demo0/board_config.c

deps_/home/wu/GPCV2159/openplatform/platform/gplus/cvr_turnkey_demo0/board_config.o := \
  /home/wu/GPCV2159/openplatform/platform/gplus/cvr_turnkey_demo0/board_config.c \
    $(wildcard include/config/internal/adc.h) \
    $(wildcard include/config/screen/rotate.h) \
    $(wildcard include/config/internal/rtc.h) \
    $(wildcard include/config/usb/phy0/en.h) \
    $(wildcard include/config/usb/phy1/sel.h) \
    $(wildcard include/config/usb/host/highspeed/mode.h) \
    $(wildcard include/config/usb/host/hub/config.h) \
    $(wildcard include/config/usb/phy0/voltage/up.h) \
    $(wildcard include/config/usb/phy1/voltage/up.h) \
    $(wildcard include/config/mainstorage.h) \
    $(wildcard include/config/gp/fast/boot.h) \
  include/linux/init.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  include/linux/module.h \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/kallsyms.h) \
    $(wildcard include/config/tracepoints.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/event/tracing.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/constructors.h) \
    $(wildcard include/config/sysfs.h) \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/stddef.h \
  include/linux/poison.h \
  include/linux/prefetch.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/posix_types.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/processor.h \
    $(wildcard include/config/mmu.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/arm/thumb.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/hwcap.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/cache.h \
    $(wildcard include/config/arm/l1/cache/shift.h) \
    $(wildcard include/config/aeabi.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/system.h \
    $(wildcard include/config/cpu/xsc3.h) \
    $(wildcard include/config/cpu/fa526.h) \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/sa110.h) \
    $(wildcard include/config/cpu/32v6k.h) \
  include/linux/linkage.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/linkage.h \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
    $(wildcard include/config/x86.h) \
  include/linux/typecheck.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/irqflags.h \
  include/asm-generic/cmpxchg-local.h \
  include/linux/stat.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/stat.h \
  include/linux/time.h \
    $(wildcard include/config/arch/uses/gettimeoffset.h) \
  include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/numa.h) \
  /usr/local/arm/4.3.2/bin/../lib/gcc/arm-none-linux-gnueabi/4.3.2/include/stdarg.h \
  include/linux/bitops.h \
    $(wildcard include/config/generic/find/first/bit.h) \
    $(wildcard include/config/generic/find/last/bit.h) \
    $(wildcard include/config/generic/find/next/bit.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/bitops.h \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/lock.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/ratelimit.h \
  include/linux/param.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/param.h \
    $(wildcard include/config/hz.h) \
  include/linux/dynamic_debug.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/swab.h \
  include/linux/byteorder/generic.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  include/linux/seqlock.h \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  include/linux/thread_info.h \
    $(wildcard include/config/compat.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/thread_info.h \
    $(wildcard include/config/arm/thumbee.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/fpstate.h \
    $(wildcard include/config/vfpv3.h) \
    $(wildcard include/config/iwmmxt.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/domain.h \
    $(wildcard include/config/io/36.h) \
  include/linux/stringify.h \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  include/linux/spinlock_types_up.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
    $(wildcard include/config/generic/hardirqs.h) \
  include/linux/spinlock_up.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/atomic.h \
  include/asm-generic/atomic-long.h \
  include/linux/spinlock_api_up.h \
  include/linux/math64.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/div64.h \
  include/linux/kmod.h \
  include/linux/gfp.h \
    $(wildcard include/config/kmemcheck.h) \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/zone/dma.h) \
    $(wildcard include/config/zone/dma32.h) \
    $(wildcard include/config/debug/vm.h) \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/sparsemem.h) \
    $(wildcard include/config/arch/populates/node/map.h) \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/cgroup/mem/res/ctlr.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
    $(wildcard include/config/nodes/span/other/nodes.h) \
    $(wildcard include/config/holes/in/zone.h) \
    $(wildcard include/config/arch/has/holes/memorymodel.h) \
  include/linux/wait.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/current.h \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  include/linux/nodemask.h \
  include/linux/bitmap.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/string.h \
  include/linux/pageblock-flags.h \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/hugetlb/page/size/variable.h) \
  include/linux/bounds.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/page.h \
    $(wildcard include/config/cpu/copy/v3.h) \
    $(wildcard include/config/cpu/copy/v4wt.h) \
    $(wildcard include/config/cpu/copy/v4wb.h) \
    $(wildcard include/config/cpu/copy/feroceon.h) \
    $(wildcard include/config/cpu/copy/fa.h) \
    $(wildcard include/config/cpu/xscale.h) \
    $(wildcard include/config/cpu/copy/v6.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/glue.h \
    $(wildcard include/config/cpu/arm610.h) \
    $(wildcard include/config/cpu/arm710.h) \
    $(wildcard include/config/cpu/abrt/lv4t.h) \
    $(wildcard include/config/cpu/abrt/ev4.h) \
    $(wildcard include/config/cpu/abrt/ev4t.h) \
    $(wildcard include/config/cpu/abrt/ev5tj.h) \
    $(wildcard include/config/cpu/abrt/ev5t.h) \
    $(wildcard include/config/cpu/abrt/ev6.h) \
    $(wildcard include/config/cpu/abrt/ev7.h) \
    $(wildcard include/config/cpu/pabrt/legacy.h) \
    $(wildcard include/config/cpu/pabrt/v6.h) \
    $(wildcard include/config/cpu/pabrt/v7.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/memory.h \
    $(wildcard include/config/page/offset.h) \
    $(wildcard include/config/thumb2/kernel.h) \
    $(wildcard include/config/dram/size.h) \
    $(wildcard include/config/dram/base.h) \
  include/linux/const.h \
  arch/arm/mach-gpl32900b/include/mach/memory.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/sizes.h \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/sparsemem/vmemmap.h) \
  include/asm-generic/getorder.h \
  include/linux/memory_hotplug.h \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
    $(wildcard include/config/memory/hotremove.h) \
  include/linux/notifier.h \
  include/linux/errno.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/mutex.h \
    $(wildcard include/config/debug/mutexes.h) \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/linux/rwsem-spinlock.h \
  include/linux/srcu.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
    $(wildcard include/config/sched/mc.h) \
  include/linux/cpumask.h \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
    $(wildcard include/config/disable/obsolete/cpumask/functions.h) \
  include/linux/smp.h \
    $(wildcard include/config/use/generic/smp/helpers.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/topology.h \
  include/asm-generic/topology.h \
  include/linux/mmdebug.h \
    $(wildcard include/config/debug/virtual.h) \
  include/linux/elf.h \
  include/linux/elf-em.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/elf.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/user.h \
  include/linux/kobject.h \
  include/linux/sysfs.h \
  include/linux/kref.h \
  include/linux/moduleparam.h \
    $(wildcard include/config/alpha.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/ppc64.h) \
  include/linux/tracepoint.h \
  include/linux/rcupdate.h \
    $(wildcard include/config/tree/preempt/rcu.h) \
    $(wildcard include/config/tree/rcu.h) \
  include/linux/completion.h \
  include/linux/rcutree.h \
    $(wildcard include/config/no/hz.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/local.h \
  include/asm-generic/local.h \
  include/linux/percpu.h \
    $(wildcard include/config/have/legacy/per/cpu/area.h) \
    $(wildcard include/config/need/per/cpu/embed/first/chunk.h) \
    $(wildcard include/config/need/per/cpu/page/first/chunk.h) \
    $(wildcard include/config/debug/kmemleak.h) \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  include/linux/slab.h \
    $(wildcard include/config/slab/debug.h) \
    $(wildcard include/config/debug/objects.h) \
    $(wildcard include/config/slub.h) \
    $(wildcard include/config/slob.h) \
    $(wildcard include/config/debug/slab.h) \
  include/linux/slub_def.h \
    $(wildcard include/config/slub/stats.h) \
    $(wildcard include/config/slub/debug.h) \
    $(wildcard include/config/kmemtrace.h) \
  include/linux/workqueue.h \
  include/linux/timer.h \
    $(wildcard include/config/timer/stats.h) \
    $(wildcard include/config/debug/objects/timers.h) \
  include/linux/ktime.h \
    $(wildcard include/config/ktime/scalar.h) \
  include/linux/jiffies.h \
  include/linux/timex.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/timex.h \
  arch/arm/mach-gpl32900b/include/mach/timex.h \
  include/linux/debugobjects.h \
    $(wildcard include/config/debug/objects/free.h) \
  include/linux/kmemtrace.h \
  include/trace/events/kmem.h \
  include/trace/define_trace.h \
  include/linux/kmemleak.h \
  include/linux/pfn.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/percpu.h \
  include/asm-generic/percpu.h \
  include/linux/percpu-defs.h \
    $(wildcard include/config/debug/force/weak/per/cpu.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/module.h \
    $(wildcard include/config/arm/unwind.h) \
  include/trace/events/module.h \
  include/linux/fs.h \
    $(wildcard include/config/dnotify.h) \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/fsnotify.h) \
    $(wildcard include/config/inotify.h) \
    $(wildcard include/config/security.h) \
    $(wildcard include/config/fs/posix/acl.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/debug/writecount.h) \
    $(wildcard include/config/file/locking.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/block.h) \
    $(wildcard include/config/fs/xip.h) \
    $(wildcard include/config/migration.h) \
  include/linux/limits.h \
  include/linux/ioctl.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  include/linux/kdev_t.h \
  include/linux/dcache.h \
  include/linux/rculist.h \
  include/linux/path.h \
  include/linux/radix-tree.h \
  include/linux/prio_tree.h \
  include/linux/pid.h \
  include/linux/capability.h \
    $(wildcard include/config/security/file/capabilities.h) \
  include/linux/semaphore.h \
  include/linux/fiemap.h \
  include/linux/quota.h \
  include/linux/dqblk_xfs.h \
  include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h \
  include/linux/dqblk_qtree.h \
  include/linux/nfs_fs_i.h \
  include/linux/nfs.h \
  include/linux/sunrpc/msg_prot.h \
  include/linux/inet.h \
  include/linux/fcntl.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/fcntl.h \
  include/asm-generic/fcntl.h \
  include/linux/err.h \
  arch/arm/mach-gpl32900b/include/mach/cdev.h \
  include/linux/miscdevice.h \
  include/linux/major.h \
  include/linux/cdev.h \
  include/linux/input.h \
  include/linux/device.h \
    $(wildcard include/config/debug/devres.h) \
    $(wildcard include/config/devtmpfs.h) \
  include/linux/ioport.h \
  include/linux/klist.h \
  include/linux/pm.h \
    $(wildcard include/config/pm/speedy.h) \
    $(wildcard include/config/pm/sleep.h) \
    $(wildcard include/config/pm/runtime.h) \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/device.h \
    $(wildcard include/config/dmabounce.h) \
  include/linux/pm_wakeup.h \
    $(wildcard include/config/pm.h) \
  include/linux/mod_devicetable.h \
  arch/arm/mach-gpl32900b/include/mach/gp_board.h \
  arch/arm/mach-gpl32900b/include/mach/typedef.h \
  arch/arm/mach-gpl32900b/include/mach/gp_display.h \
  include/linux/clk.h \
  arch/arm/mach-gpl32900b/include/mach/gp_display_func.h \
  arch/arm/mach-gpl32900b/include/mach/general.h \
  arch/arm/mach-gpl32900b/include/mach/common.h \
  /home/wu/GPCV2159/openplatform/platform/gplus/cvr_turnkey_demo0/sysconfig.h \
    $(wildcard include/config/.h) \
    $(wildcard include/config/arch.h) \
    $(wildcard include/config/arch/gpl32900b.h) \
    $(wildcard include/config/audio.h) \
    $(wildcard include/config/audiofile.h) \
    $(wildcard include/config/audiomixer.h) \
    $(wildcard include/config/a/sensor.h) \
    $(wildcard include/config/a/sensor/device.h) \
    $(wildcard include/config/btplay.h) \
    $(wildcard include/config/ceva.h) \
    $(wildcard include/config/cf.h) \
    $(wildcard include/config/chunkmem/size.h) \
    $(wildcard include/config/codec/image.h) \
    $(wildcard include/config/codec/video.h) \
    $(wildcard include/config/csi0.h) \
    $(wildcard include/config/csi1.h) \
    $(wildcard include/config/csi2.h) \
    $(wildcard include/config/dc2dc.h) \
    $(wildcard include/config/defconfig/file.h) \
    $(wildcard include/config/file.h) \
    $(wildcard include/config/disp0.h) \
    $(wildcard include/config/disp0/fb.h) \
    $(wildcard include/config/disp0/hdmi.h) \
    $(wildcard include/config/disp0/panel.h) \
    $(wildcard include/config/disp0/tvout.h) \
    $(wildcard include/config/disp1.h) \
    $(wildcard include/config/disp1/fb.h) \
    $(wildcard include/config/disp1/panel.h) \
    $(wildcard include/config/disp2.h) \
    $(wildcard include/config/disp2/fb.h) \
    $(wildcard include/config/emmc/nand.h) \
    $(wildcard include/config/esound.h) \
    $(wildcard include/config/exfat.h) \
    $(wildcard include/config/expat.h) \
    $(wildcard include/config/flash.h) \
    $(wildcard include/config/freetype2.h) \
    $(wildcard include/config/graphic/2d.h) \
    $(wildcard include/config/g/sensor.h) \
    $(wildcard include/config/g/sensor/device.h) \
    $(wildcard include/config/host.h) \
    $(wildcard include/config/jpeg.h) \
    $(wildcard include/config/kernel/cmdline.h) \
    $(wildcard include/config/key.h) \
    $(wildcard include/config/keyboard.h) \
    $(wildcard include/config/keyboard/device.h) \
    $(wildcard include/config/libfuse.h) \
    $(wildcard include/config/libid3tag.h) \
    $(wildcard include/config/libz.h) \
    $(wildcard include/config/lib/audio.h) \
    $(wildcard include/config/lib/fd.h) \
    $(wildcard include/config/lib/mcp.h) \
    $(wildcard include/config/lib/viddec.h) \
    $(wildcard include/config/lib/vidstream.h) \
    $(wildcard include/config/lua.h) \
    $(wildcard include/config/lzo.h) \
    $(wildcard include/config/main/filesystem.h) \
    $(wildcard include/config/mem/size.h) \
    $(wildcard include/config/microwindows.h) \
    $(wildcard include/config/module/allin/rootfs.h) \
    $(wildcard include/config/ms.h) \
    $(wildcard include/config/nand.h) \
    $(wildcard include/config/on2.h) \
    $(wildcard include/config/openvg.h) \
    $(wildcard include/config/platform.h) \
    $(wildcard include/config/platform/dir.h) \
    $(wildcard include/config/png.h) \
    $(wildcard include/config/ppu.h) \
    $(wildcard include/config/ppu/tv.h) \
    $(wildcard include/config/product.h) \
    $(wildcard include/config/project.h) \
    $(wildcard include/config/project/dir.h) \
    $(wildcard include/config/ps2mouse.h) \
    $(wildcard include/config/ps2mouse/device.h) \
    $(wildcard include/config/pulseaudio.h) \
    $(wildcard include/config/qt.h) \
    $(wildcard include/config/resample.h) \
    $(wildcard include/config/rm/uboot.h) \
    $(wildcard include/config/sd.h) \
    $(wildcard include/config/sdio.h) \
    $(wildcard include/config/sdl.h) \
    $(wildcard include/config/sdl/gfx.h) \
    $(wildcard include/config/sdl/image.h) \
    $(wildcard include/config/sdl/ttf.h) \
    $(wildcard include/config/sensor0.h) \
    $(wildcard include/config/sensor0/port/sel.h) \
    $(wildcard include/config/sensor1.h) \
    $(wildcard include/config/sensor1/port/sel.h) \
    $(wildcard include/config/sensor2.h) \
    $(wildcard include/config/sensor2/port/sel.h) \
    $(wildcard include/config/sensor/driver/num.h) \
    $(wildcard include/config/sflash.h) \
    $(wildcard include/config/showlogo.h) \
    $(wildcard include/config/simulator.h) \
    $(wildcard include/config/spu.h) \
    $(wildcard include/config/target.h) \
    $(wildcard include/config/tiff.h) \
    $(wildcard include/config/toolchain/sel.h) \
    $(wildcard include/config/touch.h) \
    $(wildcard include/config/touchpad.h) \
    $(wildcard include/config/touchpad/device.h) \
    $(wildcard include/config/touch/panel.h) \
    $(wildcard include/config/touch/panel/device.h) \
    $(wildcard include/config/ts.h) \
    $(wildcard include/config/usb.h) \
    $(wildcard include/config/usb/host.h) \
    $(wildcard include/config/usb/host/storage.h) \
    $(wildcard include/config/usb/slave.h) \
    $(wildcard include/config/usb/slave/msdc.h) \
    $(wildcard include/config/usb/wifi.h) \
    $(wildcard include/config/xd.h) \
  /home/wu/GPCV2159/openplatform/platform/gplus/cvr_turnkey_demo0/platform.h \
  arch/arm/mach-gpl32900b/include/mach/gp_pwm.h \
  arch/arm/mach-gpl32900b/include/mach/gp_gpio.h \
  include/linux/delay.h \
  /home/wu/GPCV2159/openplatform/sdk/os/kernel-2.6.32/arch/arm/include/asm/delay.h \
  arch/arm/mach-gpl32900b/include/mach/gp_adc.h \
  arch/arm/mach-gpl32900b/include/mach/gp_usb.h \
  arch/arm/mach-gpl32900b/include/mach/hal/hal_gpio.h \
  arch/arm/mach-gpl32900b/include/mach/hal/hal_common.h \
  arch/arm/mach-gpl32900b/include/mach/diag.h \
  arch/arm/mach-gpl32900b/include/mach/hal/hal_usb.h \
  arch/arm/mach-gpl32900b/include/mach/hal/hal_clock.h \
  arch/arm/mach-gpl32900b/include/mach/clk/clk-private.h \
    $(wildcard include/config/common/clk.h) \
    $(wildcard include/config/common/clk/debug.h) \
  arch/arm/mach-gpl32900b/include/mach/clk/clk-provider.h \
  arch/arm/mach-gpl32900b/include/mach/clk/clk.h \
    $(wildcard include/config/have/clk/prepare.h) \
    $(wildcard include/config/have/clk.h) \
  arch/arm/mach-gpl32900b/include/mach/gp_sd.h \
  arch/arm/mach-gpl32900b/include/mach/gp_apbdma0.h \
  arch/arm/mach-gpl32900b/include/mach/hal/hal_apbdma0.h \
  arch/arm/mach-gpl32900b/include/mach/app_dev.h \
  /home/wu/GPCV2159/openplatform/platform/gplus/cvr_turnkey_demo0/../../../sdk/include/dbgs.h \

/home/wu/GPCV2159/openplatform/platform/gplus/cvr_turnkey_demo0/board_config.o: $(deps_/home/wu/GPCV2159/openplatform/platform/gplus/cvr_turnkey_demo0/board_config.o)

$(deps_/home/wu/GPCV2159/openplatform/platform/gplus/cvr_turnkey_demo0/board_config.o):
