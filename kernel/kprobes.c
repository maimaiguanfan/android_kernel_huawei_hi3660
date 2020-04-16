/*
 *  Kernel Probes (KProbes)
 *  kernel/kprobes.c
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) IBM Corporation, 2002, 2004
 *
 * 2002-Oct	Created by Vamsi Krishna S <vamsi_krishna@in.ibm.com> Kernel
 *		Probes initial implementation (includes suggestions from
 *		Rusty Russell).
 * 2004-Aug	Updated by Prasanna S Panchamukhi <prasanna@in.ibm.com> with
 *		hlists and exceptions notifier as suggested by Andi Kleen.
 * 2004-July	Suparna Bhattacharya <suparna@in.ibm.com> added jumper probes
 *		interface to access function arguments.
 * 2004-Sep	Prasanna S Panchamukhi <prasanna@in.ibm.com> Changed Kprobes
 *		exceptions notifier to be first on the priority list.
 * 2005-May	Hien Nguyen <hien@us.ibm.com>, Jim Keniston
 *		<jkenisto@us.ibm.com> and Prasanna S Panchamukhi
 *		<prasanna@in.ibm.com> added function-return probes.
 */
#include <linux/kprobes.h>
#include <linux/hash.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/stddef.h>
#include <linux/export.h>
#include <linux/moduleloader.h>
#include <linux/kallsyms.h>
#include <linux/freezer.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include <linux/sysctl.h>
#include <linux/kdebug.h>
#include <linux/memory.h>
#include <linux/ftrace.h>
#include <linux/cpu.h>
#include <linux/jump_label.h>

#include <asm/sections.h>
#include <asm/cacheflush.h>
#include <asm/errno.h>
#include <asm/uaccess.h>

#define KPROBE_HASH_BITS 6
#define KPROBE_TABLE_SIZE (1 << KPROBE_HASH_BITS)


/*
 * Some oddball architectures like 64bit powerpc have function descriptors
 * so this must be overridable.
 */
#ifndef kprobe_lookup_name
#define kprobe_lookup_name(name, addr) \
	addr = ((kprobe_opcode_t *)(kallsyms_lookup_name(name)))
#endif

static int kprobes_initialized;
static struct hlist_head kprobe_table[KPROBE_TABLE_SIZE];
static struct hlist_head kretprobe_inst_table[KPROBE_TABLE_SIZE];

/* NOTE: change this value only with kprobe_mutex held */
static bool kprobes_all_disarmed;

/* This protects kprobe_table and optimizing_list */
static DEFINE_MUTEX(kprobe_mutex);
static DEFINE_PER_CPU(struct kprobe *, kprobe_instance) = NULL;
static struct {
	raw_spinlock_t lock ____cacheline_aligned_in_smp;
} kretprobe_table_locks[KPROBE_TABLE_SIZE];

static raw_spinlock_t *kretprobe_table_lock_ptr(unsigned long hash)
{
	return &(kretprobe_table_locks[hash].lock);
}

/* Blacklist -- list of struct kprobe_blacklist_entry */
static LIST_HEAD(kprobe_blacklist);

#ifdef __ARCH_WANT_KPROBES_INSN_SLOT
/*
 * kprobe->ainsn.insn points to the copy of the instruction to be
 * single-stepped. x86_64, POWER4 and above have no-exec support and
 * stepping on the instruction on a vmalloced/kmalloced/data page
 * is a recipe for disaster
 */
struct kprobe_insn_page {
	struct list_head list;
	kprobe_opcode_t *insns;		/* Page of instruction slots */
	struct kprobe_insn_cache *cache;
	int nused;
	int ngarbage;
	char slot_used[];
};

#define KPROBE_INSN_PAGE_SIZE(slots)			\
	(offsetof(struct kprobe_insn_page, slot_used) +	\
	 (sizeof(char) * (slots)))

static int slots_per_page(struct kprobe_insn_cache *c)
{
	return PAGE_SIZE/(c->insn_size * sizeof(kprobe_opcode_t));
}

enum kprobe_slot_state {
	SLOT_CLEAN = 0,
	SLOT_DIRTY = 1,
	SLOT_USED = 2,
};

static void *alloc_insn_page(void)
{
	return module_alloc(PAGE_SIZE);
}

void __weak free_insn_page(void *page)
{
	module_memfree(page);
}

struct kprobe_insn_cache kprobe_insn_slots = {
	.mutex = __MUTEX_INITIALIZER(kprobe_insn_slots.mutex),
	.alloc = alloc_insn_page,
	.free = free_insn_page,
	.pages = LIST_HEAD_INIT(kprobe_insn_slots.pages),
	.insn_size = MAX_INSN_SIZE,
	.nr_garbage = 0,
};
static int collect_garbage_slots(struct kprobe_insn_cache *c);

/**
 * __get_insn_slot() - Find a slot on an executable page for an instruction.
 * We allocate an executable page if there's no room on existing ones.
 */
kprobe_opcode_t *__get_insn_slot(struct kprobe_insn_cache *c)
{
	struct kprobe_insn_page *kip;
	kprobe_opcode_t *slot = NULL;

	mutex_lock(&c->mutex);
 retry:
	list_for_each_entry(kip, &c->pages, list) {
		if (kip->nused < slots_per_page(c)) {
			int i;
			for (i = 0; i < slots_per_page(c); i++) {
				if (kip->slot_used[i] == SLOT_CLEAN) {
					kip->slot_used[i] = SLOT_USED;
					kip->nused++;
					slot = kip->insns + (i * c->insn_size);
					goto out;
				}
			}
			/* kip->nused is broken. Fix it. */
			kip->nused = slots_per_page(c);
			WARN_ON(1);
		}
	}

	/* If there are any garbage slots, collect it and try again. */
	if (c->nr_garbage && collect_garbage_slots(c) == 0)
		goto retry;

	/* All out of space.  Need to allocate a new page. */
	kip = kmalloc(KPROBE_INSN_PAGE_SIZE(slots_per_page(c)), GFP_KERNEL);
	if (!kip)
		goto out;

	/*
	 * Use module_alloc so this page is within +/- 2GB of where the
	 * kernel image and loaded module images reside. This is required
	 * so x86_64 can correctly handle the %rip-relative fixups.
	 */
	kip->insns = c->alloc();
	if (!kip->insns) {
		kfree(kip);
		goto out;
	}
	INIT_LIST_HEAD(&kip->list);
	memset(kip->slot_used, SLOT_CLEAN, slots_per_page(c));
	kip->slot_used[0] = SLOT_USED;
	kip->nused = 1;
	kip->ngarbage = 0;
	kip->cache = c;
	list_add(&kip->list, &c->pages);
	slot = kip->insns;
out:
	mutex_unlock(&c->mutex);
	return slot;
}

/* Return 1 if all garbages are collected, otherwise 0. */
static int collect_one_slot(struct kprobe_insn_page *kip, int idx)
{
	kip->slot_used[idx] = SLOT_CLEAN;
	kip->nused--;
	if (kip->nused == 0) {
		/*
		 * Page is no longer in use.  Free it unless
		 * it's the last one.  We keep the last one
		 * so as not to have to set it up again the
		 * next time somebody inserts a probe.
		 */
		if (!list_is_singular(&kip->list)) {
			list_del(&kip->list);
			kip->cache->free(kip->insns);
			kfree(kip);
		}
		return 1;
	}
	return 0;
}

static int collect_garbage_slots(struct kprobe_insn_cache *c)
{
	struct kprobe_insn_page *kip, *next;

	/* Ensure no-one is interrupted on the garbages */
	synchronize_sched();

	list_for_each_entry_safe(kip, next, &c->pages, list) {
		int i;
		if (kip->ngarbage == 0)
			continue;
		kip->ngarbage = 0;	/* we will collect all garbages */
		for (i = 0; i < slots_per_page(c); i++) {
			if (kip->slot_used[i] == SLOT_DIRTY &&
			    collect_one_slot(kip, i))
				break;
		}
	}
	c->nr_garbage = 0;
	return 0;
}

void __free_insn_slot(struct kprobe_insn_cache *c,
		      kprobe_opcode_t *slot, int dirty)
{
	struct kprobe_insn_page *kip;

	mutex_lock(&c->mutex);
	list_for_each_entry(kip, &c->pages, list) {
		long idx = ((long)slot - (long)kip->insns) /
				(c->insn_size * sizeof(kprobe_opcode_t));
		if (idx >= 0 && idx < slots_per_page(c)) {
			WARN_ON(kip->slot_used[idx] != SLOT_USED);
			if (dirty) {
				kip->slot_used[idx] = SLOT_DIRTY;
				kip->ngarbage++;
				if (++c->nr_garbage > slots_per_page(c))
					collect_garbage_slots(c);
			} else
				collect_one_slot(kip, idx);
			goto out;
		}
	}
	/* Could not free this slot. */
	WARN_ON(1);
out:
	mutex_unlock(&c->mutex);
}

#ifdef CONFIG_OPTPROBES
/* For optimized_kprobe buffer */
struct kprobe_insn_cache kprobe_optinsn_slots = {
	.mutex = __MUTEX_INITIALIZER(kprobe_optinsn_slots.mutex),
	.alloc = alloc_insn_page,
	.free = free_insn_page,
	.pages = LIST_HEAD_INIT(kprobe_optinsn_slots.pages),
	/* .insn_size is initialized later */
	.nr_garbage = 0,
};
#endif
#endif

/* We have preemption disabled.. so it is safe to use __ versions */
static inline void set_kprobe_instance(struct kprobe *kp)
{
	__this_cpu_write(kprobe_instance, kp);
}

static inline void reset_kprobe_instance(void)
{
	__this_cpu_write(kprobe_instance, NULL);
}

/*
 * This routine is called either:
 * 	- under the kprobe_mutex - during kprobe_[un]register()
 * 				OR
 * 	- with preemption disabled - from arch/xxx/kernel/kprobes.c
 */
struct kprobe *get_kprobe(void *addr)
{
	struct hlist_head *head;
	struct kprobe *p;

	head = &kprobe_table[hash_ptr(addr, KPROBE_HASH_BITS)];
	hlist_for_each_entry_rcu(p, head, hlist) {
		if (p->addr == addr)
			return p;
	}

	return NULL;
}
NOKPROBE_SYMBOL(get_kprobe);

static int aggr_pre_handler(struct kprobe *p, struct pt_regs *regs);

/* Return true if the kprobe is an aggregator */
static inline int kprobe_aggrprobe(struct kprobe *p)
{
	return p->pre_handler == aggr_pre_handler;
}

/* Return true(!0) if the kprobe is unused */
static inline int kprobe_unused(struct kprobe *p)
{
	return kprobe_aggrprobe(p) && kprobe_disabled(p) &&
	       list_empty(&p->list);
}

/*
 * Keep all fields in the kprobe consistent
 */
static inline void copy_kprobe(struct kprobe *ap, struct kprobe *p)
{
	memcpy(&p->opcode, &ap->opcode, sizeof(kprobe_opcode_t));
	memcpy(&p->ainsn, &ap->ainsn, sizeof(struct arch_specific_insn));
}

#ifdef CONFIG_OPTPROBES
/* NOTE: change this value only with kprobe_mutex held */
static bool kprobes_allow_optimization;

/*
 * Call all pre_handler on the list, but ignores its return value.
 * This must be called from arch-dep optimized caller.
 */
void opt_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	struct kprobe *kp;

	list_for_each_entry_rcu(kp, &p->list, list) {
		if (kp->pre_handler && likely(!kprobe_disabled(kp))) {
			set_kprobe_instance(kp);
			kp->pre_handler(kp, regs);
		}
		reset_kprobe_instance();
	}
}
NOKPROBE_SYMBOL(opt_pre_handler);

/* Free optimized instructions and optimized_kprobe */
static void free_aggr_kprobe(struct kprobe *p)
{
	struct optimized_kprobe *op;

	op = container_of(p, struct optimized_kprobe, kp);
	arch_remove_optimized_kprobe(op);
	arch_remove_kprobe(p);
	kfree(op);
}

/* Return true(!0) if the kprobe is ready for optimization. */
static inline int kprobe_optready(struct kprobe *p)
{
	struct optimized_kprobe *op;

	if (kprobe_aggrprobe(p)) {
		op = container_of(p, struct optimized_kprobe, kp);
		return arch_prepared_optinsn(&op->optinsn);
	}

	return 0;
}

/* Return true(!0) if the kprobe is disarmed. Note: p must be on hash list */
static inline int kprobe_disarmed(struct kprobe *p)
{
	struct optimized_kprobe *op;

	/* If kprobe is not aggr/opt probe, just return kprobe is disabled */
	if (!kprobe_aggrprobe(p))
		return kprobe_disabled(p);

	op = container_of(p, struct optimized_kprobe, kp);

	return kprobe_disabled(p) && list_empty(&op->list);
}

/* Return true(!0) if the probe is queued on (un)optimizing lists */
static int kprobe_queued(struct kprobe *p)
{
	struct optimized_kprobe *op;

	if (kprobe_aggrprobe(p)) {
		op = container_of(p, struct optimized_kprobe, kp);
		if (!list_empty(&op->list))
			return 1;
	}
	return 0;
}

/*
 * Return an optimized kprobe whose optimizing code replaces
 * instructions including addr (exclude breakpoint).
 */
static struct kprobe *get_optimized_kprobe(unsigned long addr)
{
	int i;
	struct kprobe *p = NULL;
	struct optimized_kprobe *op;

	/* Don't check i == 0, since that is a breakpoint case. */
	for (i = 1; !p && i < MAX_OPTIMIZED_LENGTH; i++)
		p = get_kprobe((void *)(addr - i));

	if (p && kprobe_optready(p)) {
		op = container_of(p, struct optimized_kprobe, kp);
		if (arch_within_optimized_kprobe(op, addr))
			return p;
	}

	return NULL;
}

/* Optimization staging list, protected by kprobe_mutex */
static LIST_HEAD(optimizing_list);
static LIST_HEAD(unoptimizing_list);
static LIST_HEAD(freeing_list);

static void kprobe_optimizer(struct work_struct *work);
static DECLARE_DELAYED_WORK(optimizing_work, kprobe_optimizer);
#define OPTIMIZE_DELAY 5

/*
 * Optimize (replace a breakpoint with a jump) kprobes listed on
 * optimizing_list.
 */
static void do_optimize_kprobes(void)
{
	/* Optimization never be done when disarmed */
	if (kprobes_all_disarmed || !kprobes_allow_optimization ||
	    list_empty(&optimizing_list))
		return;

	/*
	 * The optimization/unoptimization refers online_cpus via
	 * stop_machine() and cpu-hotplug modifies online_cpus.
	 * And same time, text_mutex will be held in cpu-hotplug and here.
	 * This combination can cause a deadlock (cpu-hotplug try to lock
	 * text_mutex but stop_machine can not be done because online_cpus
	 * has been changed)
	 * To avoid this deadlock, we need to call get_online_cpus()
	 * for preventing cpu-hotplug outside of text_mutex locking.
	 */
	get_online_cpus();
	mutex_lock(&text_mutex);
	arch_optimize_kprobes(&optimizing_list);
	mutex_unlock(&text_mutex);
	put_online_cpus();
}

/*
 * Unoptimize (replace a jump with a breakpoint and remove the breakpoint
 * if need) kprobes listed on unoptimizing_list.
 */
static void do_unoptimize_kprobes(void)
{
	struct optimized_kprobe *op, *tmp;

	/* Unoptimization must be done anytime */
	if (list_empty(&unoptimizing_list))
		return;

	/* Ditto to do_optimize_kprobes */
	get_online_cpus();
	mutex_lock(&text_mutex);
	arch_unoptimize_kprobes(&unoptimizing_list, &freeing_list);
	/* Loop free_list for disarming */
	list_for_each_entry_safe(op, tmp, &freeing_list, list) {
		/* Disarm probes if marked disabled */
		if (kprobe_disabled(&op->kp))
			arch_disarm_kprobe(&op->kp);
		if (kprobe_unused(&op->kp)) {
			/*
			 * Remove unused probes from hash list. After waiting
			 * for synchronization, these probes are reclaimed.
			 * (reclaiming is done by do_free_cleaned_kprobes.)
			 */
			hlist_del_rcu(&op->kp.hlist);
		} else
			list_del_init(&op->list);
	}
	mutex_unlock(&text_mutex);
	put_online_cpus();
}

/* Reclaim all kprobes on the free_list */
static void do_free_cleaned_kprobes(void)
{
	struct optimized_kprobe *op, *tmp;

	list_for_each_entry_safe(op, tmp, &freeing_list, list) {
		BUG_ON(!kprobe_unused(&op->kp));
		list_del_init(&op->list);
		free_aggr_kprobe(&op->kp);
	}
}

/* Start optimizer after OPTIMIZE_DELAY passed */
static void kick_kprobe_optimizer(void)
{
	schedule_delayed_work(&optimizing_work, OPTIMIZE_DELAY);
}

/* Kprobe jump optimizer */
static void kprobe_optimizer(struct work_struct *work)
{
	mutex_lock(&kprobe_mutex);
	/* Lock modules while optimizing kprobes */
	mutex_lock(&module_mutex);

	/*
	 * Step 1: Unoptimize kprobes and collect cleaned (unused and disarmed)
	 * kprobes before waiting for quiesence period.
	 */
	do_unoptimize_kprobes();

	/*
	 * Step 2: Wait for quiesence period to ensure all running interrupts
	 * are done. Because optprobe may modify multiple instructions
	 * there is a chance that Nth instruction is interrupted. In that
	 * case, running interrupt can return to 2nd-Nth byte of jump
	 * instruction. This wait is for avoiding it.
	 */
	synchronize_sched();

	/* Step 3: Optimize kprobes after quiesence period */
	do_optimize_kprobes();

	/* Step 4: Free cleaned kprobes after quiesence period */
	do_free_cleaned_kprobes();

	mutex_unlock(&module_mutex);
	mutex_unlock(&kprobe_mutex);

	/* Step 5: Kick optimizer again if needed */
	if (!list_empty(&optimizing_list) || !list_empty(&unoptimizing_list))
		kick_kprobe_optimizer();
}

/* Wait for completing optimization and unoptimization */
void wait_for_kprobe_optimizer(void)
{
	mutex_lock(&kprobe_mutex);

	while (!list_empty(&optimizing_list) || !list_empty(&unoptimizing_list)) {
		mutex_unlock(&kprobe_mutex);

		/* this will also make optimizing_work execute immmediately */
		flush_delayed_work(&optimizing_work);
		/* @optimizing_work might not have been queued yet, relax */
		cpu_relax();

		mutex_lock(&kprobe_mutex);
	}

	mutex_unlock(&kprobe_mutex);
}

/* Optimize kprobe if p is ready to be optimized */
static void optimize_kprobe(struct kprobe *p)
{
	struct optimized_kprobe *op;

	/* Check if the kprobe is disabled or not ready for optimization. */
	if (!kprobe_optready(p) || !kprobes_allow_optimization ||
	    (kprobe_disabled(p) || kprobes_all_disarmed))
		return;

	/* Both of break_handler and post_handler are not supported. */
	if (p->break_handler || p->post_handler)
		return;

	op = container_of(p, struct optimized_kprobe, kp);

	/* Check there is no other kprobes at the optimized instructions */
	if (arch_check_optimized_kprobe(op) < 0)
		return;

	/* Check if it is already optimized. */
	if (op->kp.flags & KPROBE_FLAG_OPTIMIZED)
		return;
	op->kp.flags |= KPROBE_FLAG_OPTIMIZED;

	if (!list_empty(&op->list))
		/* This is under unoptimizing. Just dequeue the probe */
		list_del_init(&op->list);
	else {
		list_add(&op->list, &optimizing_list);
		kick_kprobe_optimizer();
	}
}

/* Short cut to direct unoptimizing */
static void force_unoptimize_kprobe(struct optimized_kprobe *op)
{
	get_online_cpus();
	arch_unoptimize_kprobe(op);
	put_online_cpus();
	if (kprobe_disabled(&op->kp))
		arch_disarm_kprobe(&op->kp);
}

/* Unoptimize a kprobe if p is optimized */
static void unoptimize_kprobe(struct kprobe *p, bool force)
{
	struct optimized_kprobe *op;

	if (!kprobe_aggrprobe(p) || kprobe_disarmed(p))
		return; /* This is not an optprobe nor optimized */

	op = container_of(p, struct optimized_kprobe, kp);
	if (!kprobe_optimized(p)) {
		/* Unoptimized or unoptimizing case */
		if (force && !list_empty(&op->list)) {
			/*
			 * Only if this is unoptimizing kprobe and forced,
			 * forcibly unoptimize it. (No need to unoptimize
			 * unoptimized kprobe again :)
			 */
			list_del_init(&op->list);
			force_unoptimize_kprobe(op);
		}
		return;
	}

	op->kp.flags &= ~KPROBE_FLAG_OPTIMIZED;
	if (!list_empty(&op->list)) {
		/* Dequeue from the optimization queue */
		list_del_init(&op->list);
		return;
	}
	/* Optimized kprobe case */
	if (force)
		/* Forcibly update the code: this is a special case */
		force_unoptimize_kprobe(op);
	else {
		list_add(&op->list, &unoptimizing_list);
		kick_kprobe_optimizer();
	}
}

/* Cancel unoptimizing for reusing */
674
675
676
677
678
679
680
681
682
683
684
685
686
687
688
689
690
691
692
693
694
695
696
697
698
699
700
701
702
703
704
705
706
707
708
709
710
711
712
713
714
715
716
717
718
719
720
721
722
723
724
725
726
727
728
729
730
731
732
733
734
735
736
737
738
739
740
741
742
743
744
745
746
747
748
749
750
751
752
753
754
755
756
757
758
759
760
761
762
763
764
765
766
767
768
769
770
771
772
773
774
775
776
777
778
779
780
781
782
783
784
785
786
787
788
789
790
791
792
793
794
795
796
797
798
799
800
801
802
803
804
805
806
807
808
809
810
811
812
813
814
815
816
817
818
819
820
821
822
823
824
825
826
827
828
829
830
831
832
833
834
835
836
837
838
839
840
841
842
843
844
845
846
847
848
849
850
851
852
853
854
855
856
857
858
859
860
861
862
863
864
865
866
867
868
869
870
871
872
873
874
875
876
877
878
879
880
881
882
883
884
885
886
887
888
889
890
891
892
893
894
895
896
897
898
899
900
901
902
903
904
905
906
907
908
909
910
911
912
913
914
915
916
917
918
919
920
921
922
923
924
925
926
927
928
929
930
931
932
933
934
935
936
937
938
939
940
941
942
943
944
945
946
947
948
949
950
951
952
953
954
955
956
957
958
959
960
961
962
963
964
965
966
967
968
969
970
971
972
973
974
975
976
977
978
979
980
981
982
983
984
985
986
987
988
989
990
991
992
993
994
995
996
997
998
999
1000
1001
1002
1003
1004
1005
1006
1007
1008
1009
1010
1011
1012
1013
1014
1015
1016
1017
1018
1019
1020
1021
1022
1023
1024
1025
1026
1027
1028
1029
1030
1031
1032
1033
1034
1035
1036
1037
1038
1039
1040
1041
1042
1043
1044
1045
1046
1047
1048
1049
1050
1051
1052
1053
1054
1055
1056
1057
1058
1059
1060
1061
1062
1063
1064
1065
1066
1067
1068
1069
1070
1071
1072
1073
1074
1075
1076
1077
1078
1079
1080
1081
1082
1083
1084
1085
1086
1087
1088
1089
1090
1091
1092
1093
1094
1095
1096
1097
1098
1099
1100
1101
1102
1103
1104
1105
1106
1107
1108
1109
1110
1111
1112
1113
1114
1115
1116
1117
1118
1119
1120
1121
1122
1123
1124
1125
1126
1127
1128
1129
1130
1131
1132
1133
1134
1135
1136
1137
1138
1139
1140
1141
1142
1143
1144
1145
1146
1147
1148
1149
1150
1151
1152
1153
1154
1155
1156
1157
1158
1159
1160
1161
1162
1163
1164
1165
1166
1167
1168
1169
1170
1171
1172
1173
1174
1175
1176
1177
1178
1179
1180
1181
1182
1183
1184
1185
1186
1187
1188
1189
1190
1191
1192
1193
1194
1195
1196
1197
1198
1199
1200
1201
1202
1203
1204
1205
1206
1207
1208
1209
1210
1211
1212
1213
1214
1215
1216
1217
1218
1219
1220
1221
1222
1223
1224
1225
1226
1227
1228
1229
1230
1231
1232
1233
1234
1235
1236
1237
1238
1239
1240
1241
1242
1243
1244
1245
1246
1247
1248
1249
1250
1251
1252
1253
1254
1255
1256
1257
1258
1259
1260
1261
1262
1263
1264
1265
1266
1267
1268
1269
1270
1271
1272
1273
1274
1275
1276
1277
1278
1279
1280
1281
1282
1283
1284
1285
1286
1287
1288
1289
1290
1291
1292
1293
1294
1295
1296
1297
1298
1299
1300
1301
1302
1303
1304
1305
1306
1307
1308
1309
1310
1311
1312
1313
1314
1315
1316
1317
1318
1319
1320
1321
1322
1323
1324
1325
1326
1327
1328
1329
1330
1331
1332
1333
1334
1335
1336
1337
1338
1339
1340
1341
1342
1343
1344
1345
1346
1347
1348
1349
1350
1351
1352
1353
1354
1355
1356
1357
1358
1359
1360
1361
1362
1363
1364
1365
1366
1367
1368
1369
1370
1371
1372
1373
1374
1375
1376
1377
1378
1379
1380
1381
1382
1383
1384
1385
1386
1387
1388
1389
1390
1391
1392
1393
1394
1395
1396
1397
1398
1399
1400
1401
1402
1403
1404
1405
1406
1407
1408
1409
1410
1411
1412
1413
1414
1415
1416
1417
1418
1419
1420
1421
1422
1423
1424
1425
1426
1427
1428
1429
1430
1431
1432
1433
1434
1435
1436
1437
1438
1439
1440
1441
1442
1443
1444
1445
1446
1447
1448
1449
1450
1451
1452
1453
1454
1455
1456
1457
1458
1459
1460
1461
1462
1463
1464
1465
1466
1467
1468
1469
1470
1471
1472
1473
1474
1475
1476
1477
1478
1479
1480
1481
1482
1483
1484
1485
1486
1487
1488
1489
1490
1491
1492
1493
1494
1495
1496
1497
1498
1499
1500
1501
1502
1503
1504
1505
1506
1507
1508
1509
1510
1511
1512
1513
1514
1515
1516
1517
1518
1519
1520
1521
1522
1523
1524
1525
1526
1527
1528
1529
1530
1531
1532
1533
1534
1535
1536
1537
1538
1539
1540
1541
1542
1543
1544
1545
1546
1547
1548
1549
1550
1551
1552
1553
1554
1555
1556
1557
1558
1559
1560
1561
1562
1563
1564
1565
1566
1567
1568
1569
1570
1571
1572
1573
1574
1575
1576
1577
1578
1579
1580
1581
1582
1583
1584
1585
1586
1587
1588
1589
1590
1591
1592
1593
1594
1595
1596
1597
1598
1599
1600
1601
1602
1603
1604
1605
1606
1607
1608
1609
1610
1611
1612
1613
1614
1615
1616
1617
1618
1619
1620
1621
1622
1623
1624
1625
1626
1627
1628
1629
1630
1631
1632
1633
1634
1635
1636
1637
1638
1639
1640
1641
1642
1643
1644
1645
1646
1647
1648
1649
1650
1651
1652
1653
1654
1655
1656
1657
1658
1659
1660
1661
1662
1663
1664
1665
1666
1667
1668
1669
1670
1671
1672
1673
1674
1675
1676
1677
1678
1679
1680
1681
1682
1683
1684
1685
1686
1687
1688
1689
1690
1691
1692
1693
1694
1695
1696
1697
1698
1699
1700
1701
1702
1703
1704
1705
1706
1707
1708
1709
1710
1711
1712
1713
1714
1715
1716
1717
1718
1719
1720
1721
1722
1723
1724
1725
1726
1727
1728
1729
1730
1731
1732
1733
1734
1735
1736
1737
1738
1739
1740
1741
1742
1743
1744
1745
1746
1747
1748
1749
1750
1751
1752
1753
1754
1755
1756
1757
1758
1759
1760
1761
1762
1763
1764
1765
1766
1767
1768
1769
1770
1771
1772
1773
1774
1775
1776
1777
1778
1779
1780
1781
1782
1783
1784
1785
1786
1787
1788
1789
1790
1791
1792
1793
1794
1795
1796
1797
1798
1799
1800
1801
1802
1803
1804
1805
1806
1807
1808
1809
1810
1811
1812
1813
1814
1815
1816
1817
1818
1819
1820
1821
1822
1823
1824
1825
1826
1827
1828
1829
1830
1831
1832
1833
1834
1835
1836
1837
1838
1839
1840
1841
1842
1843
1844
1845
1846
1847
1848
1849
1850
1851
1852
1853
1854
1855
1856
1857
1858
1859
1860
1861
1862
1863
1864
1865
1866
1867
1868
1869
1870
1871
1872
1873
1874
1875
1876
1877
1878
1879
1880
1881
1882
1883
1884
1885
1886
1887
1888
1889
1890
1891
1892
1893
1894
1895
1896
1897
1898
1899
1900
1901
1902
1903
1904
1905
1906
1907
1908
1909
1910
1911
1912
1913
1914
1915
1916
1917
1918
1919
1920
1921
1922
1923
1924
1925
1926
1927
1928
1929
1930
1931
1932
1933
1934
1935
1936
1937
1938
1939
1940
1941
1942
1943
1944
1945
1946
1947
1948
1949
1950
1951
1952
1953
1954
1955
1956
1957
1958
1959
1960
1961
1962
1963
1964
1965
1966
1967
1968
1969
1970
1971
1972
1973
1974
1975
1976
1977
1978
1979
1980
1981
1982
1983
1984
1985
1986
1987
1988
1989
1990
1991
1992
1993
1994
1995
1996
1997
1998
1999
2000
2001
2002
2003
2004
2005
2006
2007
2008
2009
2010
2011
2012
2013
2014
2015
2016
2017
2018
2019
2020
2021
2022
2023
2024
2025
2026
2027
2028
2029
2030
2031
2032
2033
2034
2035
2036
2037
2038
2039
2040
2041
2042
2043
2044
2045
2046
2047
2048
2049
2050
2051
2052
2053
2054
2055
2056
2057
2058
2059
2060
2061
2062
2063
2064
2065
2066
2067
2068
2069
2070
2071
2072
2073
2074
2075
2076
2077
2078
2079
2080
2081
2082
2083
2084
2085
2086
2087
2088
2089
2090
2091
2092
2093
2094
2095
2096
2097
2098
2099
2100
2101
2102
2103
2104
2105
2106
2107
2108
2109
2110
2111
2112
2113
2114
2115
2116
2117
2118
2119
2120
2121
2122
2123
2124
2125
2126
2127
2128
2129
2130
2131
2132
2133
2134
2135
2136
2137
2138
2139
2140
2141
2142
2143
2144
2145
2146
2147
2148
2149
2150
2151
2152
2153
2154
2155
2156
2157
2158
2159
2160
2161
2162
2163
2164
2165
2166
2167
2168
2169
2170
2171
2172
2173
2174
2175
2176
2177
2178
2179
2180
2181
2182
2183
2184
2185
2186
2187
2188
2189
2190
2191
2192
2193
2194
2195
2196
2197
2198
2199
2200
2201
2202
2203
2204
2205
2206
2207
2208
2209
2210
2211
2212
2213
2214
2215
2216
2217
2218
2219
2220
2221
2222
2223
2224
2225
2226
2227
2228
2229
2230
2231
2232
2233
2234
2235
2236
2237
2238
2239
2240
2241
2242
2243
2244
2245
2246
2247
2248
2249
2250
2251
2252
2253
2254
2255
2256
2257
2258
2259
2260
2261
2262
2263
2264
2265
2266
2267
2268
2269
2270
2271
2272
2273
2274
2275
2276
2277
2278
2279
2280
2281
2282
2283
2284
2285
2286
2287
2288
2289
2290
2291
2292
2293
2294
2295
2296
2297
2298
2299
2300
2301
2302
2303
2304
2305
2306
2307
2308
2309
2310
2311
2312
2313
2314
2315
2316
2317
2318
2319
2320
2321
2322
2323
2324
2325
2326
2327
2328
2329
2330
2331
2332
2333
2334
2335
2336
2337
2338
2339
2340
2341
2342
2343
2344
2345
2346
2347
2348
2349
2350
2351
2352
2353
2354
2355
2356
2357
2358
2359
2360
2361
2362
2363
2364
2365
2366
2367
2368
2369
2370
2371
2372
2373
2374
2375
2376
2377
2378
2379
2380
2381
2382
2383
2384
2385
2386
2387
2388
2389
2390
2391
2392
2393
2394
2395
2396
2397
2398
2399
2400
2401
2402
2403
2404
2405
2406
2407
2408
2409
2410
2411
2412
2413
2414
2415
2416
2417
2418
2419
2420
2421
2422
2423
2424
2425
2426
2427
2428
2429
2430
2431
2432
2433
2434
2435
2436
2437
2438
2439
2440
2441
2442
2443
2444
2445
2446
2447
2448
2449
2450
2451
2452
2453
2454
2455
2456
2457
2458
2459
2460
2461
2462
2463
2464
2465
2466
2467
2468
2469
2470
2471
2472
2473
2474
2475
2476
2477
2478
2479
2480
2481
2482
2483
2484
2485
2486
2487
2488
2489
2490
/*
 *  Kernel Probes (KProbes)
 *  kernel/kprobes.c
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) IBM Corporation, 2002, 2004
 *
 * 2002-Oct	Created by Vamsi Krishna S <vamsi_krishna@in.ibm.com> Kernel
 *		Probes initial implementation (includes suggestions from
 *		Rusty Russell).
 * 2004-Aug	Updated by Prasanna S Panchamukhi <prasanna@in.ibm.com> with
 *		hlists and exceptions notifier as suggested by Andi Kleen.
 * 2004-July	Suparna Bhattacharya <suparna@in.ibm.com> added jumper probes
 *		interface to access function arguments.
 * 2004-Sep	Prasanna S Panchamukhi <prasanna@in.ibm.com> Changed Kprobes
 *		exceptions notifier to be first on the priority list.
 * 2005-May	Hien Nguyen <hien@us.ibm.com>, Jim Keniston
 *		<jkenisto@us.ibm.com> and Prasanna S Panchamukhi
 *		<prasanna@in.ibm.com> added function-return probes.
 */
#include <linux/kprobes.h>
#include <linux/hash.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/stddef.h>
#include <linux/export.h>
#include <linux/moduleloader.h>
#include <linux/kallsyms.h>
#include <linux/freezer.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include <linux/sysctl.h>
#include <linux/kdebug.h>
#include <linux/memory.h>
#include <linux/ftrace.h>
#include <linux/cpu.h>
#include <linux/jump_label.h>

#include <asm/sections.h>
#include <asm/cacheflush.h>
#include <asm/errno.h>
#include <asm/uaccess.h>

#define KPROBE_HASH_BITS 6
#define KPROBE_TABLE_SIZE (1 << KPROBE_HASH_BITS)


/*
 * Some oddball architectures like 64bit powerpc have function descriptors
 * so this must be overridable.
 */
#ifndef kprobe_lookup_name
#define kprobe_lookup_name(name, addr) \
	addr = ((kprobe_opcode_t *)(kallsyms_lookup_name(name)))
#endif

static int kprobes_initialized;
static struct hlist_head kprobe_table[KPROBE_TABLE_SIZE];
static struct hlist_head kretprobe_inst_table[KPROBE_TABLE_SIZE];

/* NOTE: change this value only with kprobe_mutex held */
static bool kprobes_all_disarmed;

/* This protects kprobe_table and optimizing_list */
static DEFINE_MUTEX(kprobe_mutex);
static DEFINE_PER_CPU(struct kprobe *, kprobe_instance) = NULL;
static struct {
	raw_spinlock_t lock ____cacheline_aligned_in_smp;
} kretprobe_table_locks[KPROBE_TABLE_SIZE];

static raw_spinlock_t *kretprobe_table_lock_ptr(unsigned long hash)
{
	return &(kretprobe_table_locks[hash].lock);
}

/* Blacklist -- list of struct kprobe_blacklist_entry */
static LIST_HEAD(kprobe_blacklist);

#ifdef __ARCH_WANT_KPROBES_INSN_SLOT
/*
 * kprobe->ainsn.insn points to the copy of the instruction to be
 * single-stepped. x86_64, POWER4 and above have no-exec support and
 * stepping on the instruction on a vmalloced/kmalloced/data page
 * is a recipe for disaster
 */
struct kprobe_insn_page {
	struct list_head list;
	kprobe_opcode_t *insns;		/* Page of instruction slots */
	struct kprobe_insn_cache *cache;
	int nused;
	int ngarbage;
	char slot_used[];
};

#define KPROBE_INSN_PAGE_SIZE(slots)			\
	(offsetof(struct kprobe_insn_page, slot_used) +	\
	 (sizeof(char) * (slots)))

static int slots_per_page(struct kprobe_insn_cache *c)
{
	return PAGE_SIZE/(c->insn_size * sizeof(kprobe_opcode_t));
}

enum kprobe_slot_state {
	SLOT_CLEAN = 0,
	SLOT_DIRTY = 1,
	SLOT_USED = 2,
};

static void *alloc_insn_page(void)
{
	return module_alloc(PAGE_SIZE);
}

void __weak free_insn_page(void *page)
{
	module_memfree(page);
}

struct kprobe_insn_cache kprobe_insn_slots = {
	.mutex = __MUTEX_INITIALIZER(kprobe_insn_slots.mutex),
	.alloc = alloc_insn_page,
	.free = free_insn_page,
	.pages = LIST_HEAD_INIT(kprobe_insn_slots.pages),
	.insn_size = MAX_INSN_SIZE,
	.nr_garbage = 0,
};
static int collect_garbage_slots(struct kprobe_insn_cache *c);

/**
 * __get_insn_slot() - Find a slot on an executable page for an instruction.
 * We allocate an executable page if there's no room on existing ones.
 */
kprobe_opcode_t *__get_insn_slot(struct kprobe_insn_cache *c)
{
	struct kprobe_insn_page *kip;
	kprobe_opcode_t *slot = NULL;

	mutex_lock(&c->mutex);
 retry:
	list_for_each_entry(kip, &c->pages, list) {
		if (kip->nused < slots_per_page(c)) {
			int i;
			for (i = 0; i < slots_per_page(c); i++) {
				if (kip->slot_used[i] == SLOT_CLEAN) {
					kip->slot_used[i] = SLOT_USED;
					kip->nused++;
					slot = kip->insns + (i * c->insn_size);
					goto out;
				}
			}
			/* kip->nused is broken. Fix it. */
			kip->nused = slots_per_page(c);
			WARN_ON(1);
		}
	}

	/* If there are any garbage slots, collect it and try again. */
	if (c->nr_garbage && collect_garbage_slots(c) == 0)
		goto retry;

	/* All out of space.  Need to allocate a new page. */
	kip = kmalloc(KPROBE_INSN_PAGE_SIZE(slots_per_page(c)), GFP_KERNEL);
	if (!kip)
		goto out;

	/*
	 * Use module_alloc so this page is within +/- 2GB of where the
	 * kernel image and loaded module images reside. This is required
	 * so x86_64 can correctly handle the %rip-relative fixups.
	 */
	kip->insns = c->alloc();
	if (!kip->insns) {
		kfree(kip);
		goto out;
	}
	INIT_LIST_HEAD(&kip->list);
	memset(kip->slot_used, SLOT_CLEAN, slots_per_page(c));
	kip->slot_used[0] = SLOT_USED;
	kip->nused = 1;
	kip->ngarbage = 0;
	kip->cache = c;
	list_add(&kip->list, &c->pages);
	slot = kip->insns;
out:
	mutex_unlock(&c->mutex);
	return slot;
}

/* Return 1 if all garbages are collected, otherwise 0. */
static int collect_one_slot(struct kprobe_insn_page *kip, int idx)
{
	kip->slot_used[idx] = SLOT_CLEAN;
	kip->nused--;
	if (kip->nused == 0) {
		/*
		 * Page is no longer in use.  Free it unless
		 * it's the last one.  We keep the last one
		 * so as not to have to set it up again the
		 * next time somebody inserts a probe.
		 */
		if (!list_is_singular(&kip->list)) {
			list_del(&kip->list);
			kip->cache->free(kip->insns);
			kfree(kip);
		}
		return 1;
	}
	return 0;
}

static int collect_garbage_slots(struct kprobe_insn_cache *c)
{
	struct kprobe_insn_page *kip, *next;

	/* Ensure no-one is interrupted on the garbages */
	synchronize_sched();

	list_for_each_entry_safe(kip, next, &c->pages, list) {
		int i;
		if (kip->ngarbage == 0)
			continue;
		kip->ngarbage = 0;	/* we will collect all garbages */
		for (i = 0; i < slots_per_page(c); i++) {
			if (kip->slot_used[i] == SLOT_DIRTY &&
			    collect_one_slot(kip, i))
				break;
		}
	}
	c->nr_garbage = 0;
	return 0;
}

void __free_insn_slot(struct kprobe_insn_cache *c,
		      kprobe_opcode_t *slot, int dirty)
{
	struct kprobe_insn_page *kip;

	mutex_lock(&c->mutex);
	list_for_each_entry(kip, &c->pages, list) {
		long idx = ((long)slot - (long)kip->insns) /
				(c->insn_size * sizeof(kprobe_opcode_t));
		if (idx >= 0 && idx < slots_per_page(c)) {
			WARN_ON(kip->slot_used[idx] != SLOT_USED);
			if (dirty) {
				kip->slot_used[idx] = SLOT_DIRTY;
				kip->ngarbage++;
				if (++c->nr_garbage > slots_per_page(c))
					collect_garbage_slots(c);
			} else
				collect_one_slot(kip, idx);
			goto out;
		}
	}
	/* Could not free this slot. */
	WARN_ON(1);
out:
	mutex_unlock(&c->mutex);
}

#ifdef CONFIG_OPTPROBES
/* For optimized_kprobe buffer */
struct kprobe_insn_cache kprobe_optinsn_slots = {
	.mutex = __MUTEX_INITIALIZER(kprobe_optinsn_slots.mutex),
	.alloc = alloc_insn_page,
	.free = free_insn_page,
	.pages = LIST_HEAD_INIT(kprobe_optinsn_slots.pages),
	/* .insn_size is initialized later */
	.nr_garbage = 0,
};
#endif
#endif

/* We have preemption disabled.. so it is safe to use __ versions */
static inline void set_kprobe_instance(struct kprobe *kp)
{
	__this_cpu_write(kprobe_instance, kp);
}

static inline void reset_kprobe_instance(void)
{
	__this_cpu_write(kprobe_instance, NULL);
}

/*
 * This routine is called either:
 * 	- under the kprobe_mutex - during kprobe_[un]register()
 * 				OR
 * 	- with preemption disabled - from arch/xxx/kernel/kprobes.c
 */
struct kprobe *get_kprobe(void *addr)
{
	struct hlist_head *head;
	struct kprobe *p;

	head = &kprobe_table[hash_ptr(addr, KPROBE_HASH_BITS)];
	hlist_for_each_entry_rcu(p, head, hlist) {
		if (p->addr == addr)
			return p;
	}

	return NULL;
}
NOKPROBE_SYMBOL(get_kprobe);

static int aggr_pre_handler(struct kprobe *p, struct pt_regs *regs);

/* Return true if the kprobe is an aggregator */
static inline int kprobe_aggrprobe(struct kprobe *p)
{
	return p->pre_handler == aggr_pre_handler;
}

/* Return true(!0) if the kprobe is unused */
static inline int kprobe_unused(struct kprobe *p)
{
	return kprobe_aggrprobe(p) && kprobe_disabled(p) &&
	       list_empty(&p->list);
}

/*
 * Keep all fields in the kprobe consistent
 */
static inline void copy_kprobe(struct kprobe *ap, struct kprobe *p)
{
	memcpy(&p->opcode, &ap->opcode, sizeof(kprobe_opcode_t));
	memcpy(&p->ainsn, &ap->ainsn, sizeof(struct arch_specific_insn));
}

#ifdef CONFIG_OPTPROBES
/* NOTE: change this value only with kprobe_mutex held */
static bool kprobes_allow_optimization;

/*
 * Call all pre_handler on the list, but ignores its return value.
 * This must be called from arch-dep optimized caller.
 */
void opt_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	struct kprobe *kp;

	list_for_each_entry_rcu(kp, &p->list, list) {
		if (kp->pre_handler && likely(!kprobe_disabled(kp))) {
			set_kprobe_instance(kp);
			kp->pre_handler(kp, regs);
		}
		reset_kprobe_instance();
	}
}
NOKPROBE_SYMBOL(opt_pre_handler);

/* Free optimized instructions and optimized_kprobe */
static void free_aggr_kprobe(struct kprobe *p)
{
	struct optimized_kprobe *op;

	op = container_of(p, struct optimized_kprobe, kp);
	arch_remove_optimized_kprobe(op);
	arch_remove_kprobe(p);
	kfree(op);
}

/* Return true(!0) if the kprobe is ready for optimization. */
static inline int kprobe_optready(struct kprobe *p)
{
	struct optimized_kprobe *op;

	if (kprobe_aggrprobe(p)) {
		op = container_of(p, struct optimized_kprobe, kp);
		return arch_prepared_optinsn(&op->optinsn);
	}

	return 0;
}

/* Return true(!0) if the kprobe is disarmed. Note: p must be on hash list */
static inline int kprobe_disarmed(struct kprobe *p)
{
	struct optimized_kprobe *op;

	/* If kprobe is not aggr/opt probe, just return kprobe is disabled */
	if (!kprobe_aggrprobe(p))
		return kprobe_disabled(p);

	op = container_of(p, struct optimized_kprobe, kp);

	return kprobe_disabled(p) && list_empty(&op->list);
}

/* Return true(!0) if the probe is queued on (un)optimizing lists */
static int kprobe_queued(struct kprobe *p)
{
	struct optimized_kprobe *op;

	if (kprobe_aggrprobe(p)) {
		op = container_of(p, struct optimized_kprobe, kp);
		if (!list_empty(&op->list))
			return 1;
	}
	return 0;
}

/*
 * Return an optimized kprobe whose optimizing code replaces
 * instructions including addr (exclude breakpoint).
 */
static struct kprobe *get_optimized_kprobe(unsigned long addr)
{
	int i;
	struct kprobe *p = NULL;
	struct optimized_kprobe *op;

	/* Don't check i == 0, since that is a breakpoint case. */
	for (i = 1; !p && i < MAX_OPTIMIZED_LENGTH; i++)
		p = get_kprobe((void *)(addr - i));

	if (p && kprobe_optready(p)) {
		op = container_of(p, struct optimized_kprobe, kp);
		if (arch_within_optimized_kprobe(op, addr))
			return p;
	}

	return NULL;
}

/* Optimization staging list, protected by kprobe_mutex */
static LIST_HEAD(optimizing_list);
static LIST_HEAD(unoptimizing_list);
static LIST_HEAD(freeing_list);

static void kprobe_optimizer(struct work_struct *work);
static DECLARE_DELAYED_WORK(optimizing_work, kprobe_optimizer);
#define OPTIMIZE_DELAY 5

/*
 * Optimize (replace a breakpoint with a jump) kprobes listed on
 * optimizing_list.
 */
static void do_optimize_kprobes(void)
{
	/* Optimization never be done when disarmed */
	if (kprobes_all_disarmed || !kprobes_allow_optimization ||
	    list_empty(&optimizing_list))
		return;

	/*
	 * The optimization/unoptimization refers online_cpus via
	 * stop_machine() and cpu-hotplug modifies online_cpus.
	 * And same time, text_mutex will be held in cpu-hotplug and here.
	 * This combination can cause a deadlock (cpu-hotplug try to lock
	 * text_mutex but stop_machine can not be done because online_cpus
	 * has been changed)
	 * To avoid this deadlock, we need to call get_online_cpus()
	 * for preventing cpu-hotplug outside of text_mutex locking.
	 */
	get_online_cpus();
	mutex_lock(&text_mutex);
	arch_optimize_kprobes(&optimizing_list);
	mutex_unlock(&text_mutex);
	put_online_cpus();
}

/*
 * Unoptimize (replace a jump with a breakpoint and remove the breakpoint
 * if need) kprobes listed on unoptimizing_list.
 */
static void do_unoptimize_kprobes(void)
{
	struct optimized_kprobe *op, *tmp;

	/* Unoptimization must be done anytime */
	if (list_empty(&unoptimizing_list))
		return;

	/* Ditto to do_optimize_kprobes */
	get_online_cpus();
	mutex_lock(&text_mutex);
	arch_unoptimize_kprobes(&unoptimizing_list, &freeing_list);
	/* Loop free_list for disarming */
	list_for_each_entry_safe(op, tmp, &freeing_list, list) {
		/* Disarm probes if marked disabled */
		if (kprobe_disabled(&op->kp))
			arch_disarm_kprobe(&op->kp);
		if (kprobe_unused(&op->kp)) {
			/*
			 * Remove unused probes from hash list. After waiting
			 * for synchronization, these probes are reclaimed.
			 * (reclaiming is done by do_free_cleaned_kprobes.)
			 */
			hlist_del_rcu(&op->kp.hlist);
		} else
			list_del_init(&op->list);
	}
	mutex_unlock(&text_mutex);
	put_online_cpus();
}

/* Reclaim all kprobes on the free_list */
static void do_free_cleaned_kprobes(void)
{
	struct optimized_kprobe *op, *tmp;

	list_for_each_entry_safe(op, tmp, &freeing_list, list) {
		list_del_init(&op->list);
		if (WARN_ON_ONCE(!kprobe_unused(&op->kp))) {
			/*
			 * This must not happen, but if there is a kprobe
			 * still in use, keep it on kprobes hash list.
			 */
			continue;
		}
		free_aggr_kprobe(&op->kp);
	}
}

/* Start optimizer after OPTIMIZE_DELAY passed */
static void kick_kprobe_optimizer(void)
{
	schedule_delayed_work(&optimizing_work, OPTIMIZE_DELAY);
}

/* Kprobe jump optimizer */
static void kprobe_optimizer(struct work_struct *work)
{
	mutex_lock(&kprobe_mutex);
	/* Lock modules while optimizing kprobes */
	mutex_lock(&module_mutex);

	/*
	 * Step 1: Unoptimize kprobes and collect cleaned (unused and disarmed)
	 * kprobes before waiting for quiesence period.
	 */
	do_unoptimize_kprobes();

	/*
	 * Step 2: Wait for quiesence period to ensure all running interrupts
	 * are done. Because optprobe may modify multiple instructions
	 * there is a chance that Nth instruction is interrupted. In that
	 * case, running interrupt can return to 2nd-Nth byte of jump
	 * instruction. This wait is for avoiding it.
	 */
	synchronize_sched();

	/* Step 3: Optimize kprobes after quiesence period */
	do_optimize_kprobes();

	/* Step 4: Free cleaned kprobes after quiesence period */
	do_free_cleaned_kprobes();

	mutex_unlock(&module_mutex);
	mutex_unlock(&kprobe_mutex);

	/* Step 5: Kick optimizer again if needed */
	if (!list_empty(&optimizing_list) || !list_empty(&unoptimizing_list))
		kick_kprobe_optimizer();
}

/* Wait for completing optimization and unoptimization */
void wait_for_kprobe_optimizer(void)
{
	mutex_lock(&kprobe_mutex);

	while (!list_empty(&optimizing_list) || !list_empty(&unoptimizing_list)) {
		mutex_unlock(&kprobe_mutex);

		/* this will also make optimizing_work execute immmediately */
		flush_delayed_work(&optimizing_work);
		/* @optimizing_work might not have been queued yet, relax */
		cpu_relax();

		mutex_lock(&kprobe_mutex);
	}

	mutex_unlock(&kprobe_mutex);
}

/* Optimize kprobe if p is ready to be optimized */
static void optimize_kprobe(struct kprobe *p)
{
	struct optimized_kprobe *op;

	/* Check if the kprobe is disabled or not ready for optimization. */
	if (!kprobe_optready(p) || !kprobes_allow_optimization ||
	    (kprobe_disabled(p) || kprobes_all_disarmed))
		return;

	/* Both of break_handler and post_handler are not supported. */
	if (p->break_handler || p->post_handler)
		return;

	op = container_of(p, struct optimized_kprobe, kp);

	/* Check there is no other kprobes at the optimized instructions */
	if (arch_check_optimized_kprobe(op) < 0)
		return;

	/* Check if it is already optimized. */
	if (op->kp.flags & KPROBE_FLAG_OPTIMIZED)
		return;
	op->kp.flags |= KPROBE_FLAG_OPTIMIZED;

	if (!list_empty(&op->list))
		/* This is under unoptimizing. Just dequeue the probe */
		list_del_init(&op->list);
	else {
		list_add(&op->list, &optimizing_list);
		kick_kprobe_optimizer();
	}
}

/* Short cut to direct unoptimizing */
static void force_unoptimize_kprobe(struct optimized_kprobe *op)
{
	get_online_cpus();
	arch_unoptimize_kprobe(op);
	put_online_cpus();
	if (kprobe_disabled(&op->kp))
		arch_disarm_kprobe(&op->kp);
}

/* Unoptimize a kprobe if p is optimized */
static void unoptimize_kprobe(struct kprobe *p, bool force)
{
	struct optimized_kprobe *op;

	if (!kprobe_aggrprobe(p) || kprobe_disarmed(p))
		return; /* This is not an optprobe nor optimized */

	op = container_of(p, struct optimized_kprobe, kp);
	if (!kprobe_optimized(p)) {
		/* Unoptimized or unoptimizing case */
		if (force && !list_empty(&op->list)) {
			/*
			 * Only if this is unoptimizing kprobe and forced,
			 * forcibly unoptimize it. (No need to unoptimize
			 * unoptimized kprobe again :)
			 */
			list_del_init(&op->list);
			force_unoptimize_kprobe(op);
		}
		return;
	}

	op->kp.flags &= ~KPROBE_FLAG_OPTIMIZED;
	if (!list_empty(&op->list)) {
		/* Dequeue from the optimization queue */
		list_del_init(&op->list);
		return;
	}
	/* Optimized kprobe case */
	if (force)
		/* Forcibly update the code: this is a special case */
		force_unoptimize_kprobe(op);
	else {
		list_add(&op->list, &unoptimizing_list);
		kick_kprobe_optimizer();
	}
}

/* Cancel unoptimizing for reusing */
static int reuse_unused_kprobe(struct kprobe *ap)
{
	struct optimized_kprobe *op;

	BUG_ON(!kprobe_unused(ap));
	/*
	 * Unused kprobe MUST be on the way of delayed unoptimizing (means
	 * there is still a relative jump) and disabled.
	 */
	op = container_of(ap, struct optimized_kprobe, kp);
	if (unlikely(list_empty(&op->list)))
		printk(KERN_WARNING "Warning: found a stray unused "
			"aggrprobe@%p\n", ap->addr);
	/* Enable the probe again */
	ap->flags &= ~KPROBE_FLAG_DISABLED;
	/* Optimize it again (remove from op->list) */
	if (!kprobe_optready(ap))
		return -EINVAL;

	optimize_kprobe(ap);
	return 0;
}

/* Remove optimized instructions */
static void kill_optimized_kprobe(struct kprobe *p)
{
	struct optimized_kprobe *op;

	op = container_of(p, struct optimized_kprobe, kp);
	if (!list_empty(&op->list))
		/* Dequeue from the (un)optimization queue */
		list_del_init(&op->list);
	op->kp.flags &= ~KPROBE_FLAG_OPTIMIZED;

	if (kprobe_unused(p)) {
		/* Enqueue if it is unused */
		list_add(&op->list, &freeing_list);
		/*
		 * Remove unused probes from the hash list. After waiting
		 * for synchronization, this probe is reclaimed.
		 * (reclaiming is done by do_free_cleaned_kprobes().)
		 */
		hlist_del_rcu(&op->kp.hlist);
	}

	/* Don't touch the code, because it is already freed. */
	arch_remove_optimized_kprobe(op);
}

/* Try to prepare optimized instructions */
static void prepare_optimized_kprobe(struct kprobe *p)
{
	struct optimized_kprobe *op;

	op = container_of(p, struct optimized_kprobe, kp);
	arch_prepare_optimized_kprobe(op, p);
}

/* Allocate new optimized_kprobe and try to prepare optimized instructions */
static struct kprobe *alloc_aggr_kprobe(struct kprobe *p)
{
	struct optimized_kprobe *op;

	op = kzalloc(sizeof(struct optimized_kprobe), GFP_KERNEL);
	if (!op)
		return NULL;

	INIT_LIST_HEAD(&op->list);
	op->kp.addr = p->addr;
	arch_prepare_optimized_kprobe(op, p);

	return &op->kp;
}

static void init_aggr_kprobe(struct kprobe *ap, struct kprobe *p);

/*
 * Prepare an optimized_kprobe and optimize it
 * NOTE: p must be a normal registered kprobe
 */
static void try_to_optimize_kprobe(struct kprobe *p)
{
	struct kprobe *ap;
	struct optimized_kprobe *op;

	/* Impossible to optimize ftrace-based kprobe */
	if (kprobe_ftrace(p))
		return;

	/* For preparing optimization, jump_label_text_reserved() is called */
	jump_label_lock();
	mutex_lock(&text_mutex);

	ap = alloc_aggr_kprobe(p);
	if (!ap)
		goto out;

	op = container_of(ap, struct optimized_kprobe, kp);
	if (!arch_prepared_optinsn(&op->optinsn)) {
		/* If failed to setup optimizing, fallback to kprobe */
		arch_remove_optimized_kprobe(op);
		kfree(op);
		goto out;
	}

	init_aggr_kprobe(ap, p);
	optimize_kprobe(ap);	/* This just kicks optimizer thread */

out:
	mutex_unlock(&text_mutex);
	jump_label_unlock();
}

#ifdef CONFIG_SYSCTL
static void optimize_all_kprobes(void)
{
	struct hlist_head *head;
	struct kprobe *p;
	unsigned int i;

	mutex_lock(&kprobe_mutex);
	/* If optimization is already allowed, just return */
	if (kprobes_allow_optimization)
		goto out;

	kprobes_allow_optimization = true;
	for (i = 0; i < KPROBE_TABLE_SIZE; i++) {
		head = &kprobe_table[i];
		hlist_for_each_entry_rcu(p, head, hlist)
			if (!kprobe_disabled(p))
				optimize_kprobe(p);
	}
	printk(KERN_INFO "Kprobes globally optimized\n");
out:
	mutex_unlock(&kprobe_mutex);
}

static void unoptimize_all_kprobes(void)
{
	struct hlist_head *head;
	struct kprobe *p;
	unsigned int i;

	mutex_lock(&kprobe_mutex);
	/* If optimization is already prohibited, just return */
	if (!kprobes_allow_optimization) {
		mutex_unlock(&kprobe_mutex);
		return;
	}

	kprobes_allow_optimization = false;
	for (i = 0; i < KPROBE_TABLE_SIZE; i++) {
		head = &kprobe_table[i];
		hlist_for_each_entry_rcu(p, head, hlist) {
			if (!kprobe_disabled(p))
				unoptimize_kprobe(p, false);
		}
	}
	mutex_unlock(&kprobe_mutex);

	/* Wait for unoptimizing completion */
	wait_for_kprobe_optimizer();
	printk(KERN_INFO "Kprobes globally unoptimized\n");
}

static DEFINE_MUTEX(kprobe_sysctl_mutex);
int sysctl_kprobes_optimization;
int proc_kprobes_optimization_handler(struct ctl_table *table, int write,
				      void __user *buffer, size_t *length,
				      loff_t *ppos)
{
	int ret;

	mutex_lock(&kprobe_sysctl_mutex);
	sysctl_kprobes_optimization = kprobes_allow_optimization ? 1 : 0;
	ret = proc_dointvec_minmax(table, write, buffer, length, ppos);

	if (sysctl_kprobes_optimization)
		optimize_all_kprobes();
	else
		unoptimize_all_kprobes();
	mutex_unlock(&kprobe_sysctl_mutex);

	return ret;
}
#endif /* CONFIG_SYSCTL */

/* Put a breakpoint for a probe. Must be called with text_mutex locked */
static void __arm_kprobe(struct kprobe *p)
{
	struct kprobe *_p;

	/* Check collision with other optimized kprobes */
	_p = get_optimized_kprobe((unsigned long)p->addr);
	if (unlikely(_p))
		/* Fallback to unoptimized kprobe */
		unoptimize_kprobe(_p, true);

	arch_arm_kprobe(p);
	optimize_kprobe(p);	/* Try to optimize (add kprobe to a list) */
}

/* Remove the breakpoint of a probe. Must be called with text_mutex locked */
static void __disarm_kprobe(struct kprobe *p, bool reopt)
{
	struct kprobe *_p;

	/* Try to unoptimize */
	unoptimize_kprobe(p, kprobes_all_disarmed);

	if (!kprobe_queued(p)) {
		arch_disarm_kprobe(p);
		/* If another kprobe was blocked, optimize it. */
		_p = get_optimized_kprobe((unsigned long)p->addr);
		if (unlikely(_p) && reopt)
			optimize_kprobe(_p);
	}
	/* TODO: reoptimize others after unoptimized this probe */
}

#else /* !CONFIG_OPTPROBES */

#define optimize_kprobe(p)			do {} while (0)
#define unoptimize_kprobe(p, f)			do {} while (0)
#define kill_optimized_kprobe(p)		do {} while (0)
#define prepare_optimized_kprobe(p)		do {} while (0)
#define try_to_optimize_kprobe(p)		do {} while (0)
#define __arm_kprobe(p)				arch_arm_kprobe(p)
#define __disarm_kprobe(p, o)			arch_disarm_kprobe(p)
#define kprobe_disarmed(p)			kprobe_disabled(p)
#define wait_for_kprobe_optimizer()		do {} while (0)

/* There should be no unused kprobes can be reused without optimization */
static void reuse_unused_kprobe(struct kprobe *ap)
{
	printk(KERN_ERR "Error: There should be no unused kprobe here.\n");
	BUG_ON(kprobe_unused(ap));
}

static void free_aggr_kprobe(struct kprobe *p)
{
	arch_remove_kprobe(p);
	kfree(p);
}

static struct kprobe *alloc_aggr_kprobe(struct kprobe *p)
{
	return kzalloc(sizeof(struct kprobe), GFP_KERNEL);
}
#endif /* CONFIG_OPTPROBES */

#ifdef CONFIG_KPROBES_ON_FTRACE
static struct ftrace_ops kprobe_ftrace_ops __read_mostly = {
	.func = kprobe_ftrace_handler,
	.flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_IPMODIFY,
};
static int kprobe_ftrace_enabled;

/* Must ensure p->addr is really on ftrace */
static int prepare_kprobe(struct kprobe *p)
{
	if (!kprobe_ftrace(p))
		return arch_prepare_kprobe(p);

	return arch_prepare_kprobe_ftrace(p);
}

/* Caller must lock kprobe_mutex */
static void arm_kprobe_ftrace(struct kprobe *p)
{
	int ret;

	ret = ftrace_set_filter_ip(&kprobe_ftrace_ops,
				   (unsigned long)p->addr, 0, 0);
	WARN(ret < 0, "Failed to arm kprobe-ftrace at %p (%d)\n", p->addr, ret);
	kprobe_ftrace_enabled++;
	if (kprobe_ftrace_enabled == 1) {
		ret = register_ftrace_function(&kprobe_ftrace_ops);
		WARN(ret < 0, "Failed to init kprobe-ftrace (%d)\n", ret);
	}
}

/* Caller must lock kprobe_mutex */
static void disarm_kprobe_ftrace(struct kprobe *p)
{
	int ret;

	kprobe_ftrace_enabled--;
	if (kprobe_ftrace_enabled == 0) {
		ret = unregister_ftrace_function(&kprobe_ftrace_ops);
		WARN(ret < 0, "Failed to init kprobe-ftrace (%d)\n", ret);
	}
	ret = ftrace_set_filter_ip(&kprobe_ftrace_ops,
			   (unsigned long)p->addr, 1, 0);
	WARN(ret < 0, "Failed to disarm kprobe-ftrace at %p (%d)\n", p->addr, ret);
}
#else	/* !CONFIG_KPROBES_ON_FTRACE */
#define prepare_kprobe(p)	arch_prepare_kprobe(p)
#define arm_kprobe_ftrace(p)	do {} while (0)
#define disarm_kprobe_ftrace(p)	do {} while (0)
#endif

/* Arm a kprobe with text_mutex */
static void arm_kprobe(struct kprobe *kp)
{
	if (unlikely(kprobe_ftrace(kp))) {
		arm_kprobe_ftrace(kp);
		return;
	}
	/*
	 * Here, since __arm_kprobe() doesn't use stop_machine(),
	 * this doesn't cause deadlock on text_mutex. So, we don't
	 * need get_online_cpus().
	 */
	mutex_lock(&text_mutex);
	__arm_kprobe(kp);
	mutex_unlock(&text_mutex);
}

/* Disarm a kprobe with text_mutex */
static void disarm_kprobe(struct kprobe *kp, bool reopt)
{
	if (unlikely(kprobe_ftrace(kp))) {
		disarm_kprobe_ftrace(kp);
		return;
	}
	/* Ditto */
	mutex_lock(&text_mutex);
	__disarm_kprobe(kp, reopt);
	mutex_unlock(&text_mutex);
}

/*
 * Aggregate handlers for multiple kprobes support - these handlers
 * take care of invoking the individual kprobe handlers on p->list
 */
static int aggr_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	struct kprobe *kp;

	list_for_each_entry_rcu(kp, &p->list, list) {
		if (kp->pre_handler && likely(!kprobe_disabled(kp))) {
			set_kprobe_instance(kp);
			if (kp->pre_handler(kp, regs))
				return 1;
		}
		reset_kprobe_instance();
	}
	return 0;
}
NOKPROBE_SYMBOL(aggr_pre_handler);

static void aggr_post_handler(struct kprobe *p, struct pt_regs *regs,
			      unsigned long flags)
{
	struct kprobe *kp;

	list_for_each_entry_rcu(kp, &p->list, list) {
		if (kp->post_handler && likely(!kprobe_disabled(kp))) {
			set_kprobe_instance(kp);
			kp->post_handler(kp, regs, flags);
			reset_kprobe_instance();
		}
	}
}
NOKPROBE_SYMBOL(aggr_post_handler);

static int aggr_fault_handler(struct kprobe *p, struct pt_regs *regs,
			      int trapnr)
{
	struct kprobe *cur = __this_cpu_read(kprobe_instance);

	/*
	 * if we faulted "during" the execution of a user specified
	 * probe handler, invoke just that probe's fault handler
	 */
	if (cur && cur->fault_handler) {
		if (cur->fault_handler(cur, regs, trapnr))
			return 1;
	}
	return 0;
}
NOKPROBE_SYMBOL(aggr_fault_handler);

static int aggr_break_handler(struct kprobe *p, struct pt_regs *regs)
{
	struct kprobe *cur = __this_cpu_read(kprobe_instance);
	int ret = 0;

	if (cur && cur->break_handler) {
		if (cur->break_handler(cur, regs))
			ret = 1;
	}
	reset_kprobe_instance();
	return ret;
}
NOKPROBE_SYMBOL(aggr_break_handler);

/* Walks the list and increments nmissed count for multiprobe case */
void kprobes_inc_nmissed_count(struct kprobe *p)
{
	struct kprobe *kp;
	if (!kprobe_aggrprobe(p)) {
		p->nmissed++;
	} else {
		list_for_each_entry_rcu(kp, &p->list, list)
			kp->nmissed++;
	}
	return;
}
NOKPROBE_SYMBOL(kprobes_inc_nmissed_count);

void recycle_rp_inst(struct kretprobe_instance *ri,
		     struct hlist_head *head)
{
	struct kretprobe *rp = ri->rp;

	/* remove rp inst off the rprobe_inst_table */
	hlist_del(&ri->hlist);
	INIT_HLIST_NODE(&ri->hlist);
	if (likely(rp)) {
		raw_spin_lock(&rp->lock);
		hlist_add_head(&ri->hlist, &rp->free_instances);
		raw_spin_unlock(&rp->lock);
	} else
		/* Unregistering */
		hlist_add_head(&ri->hlist, head);
}
NOKPROBE_SYMBOL(recycle_rp_inst);

void kretprobe_hash_lock(struct task_struct *tsk,
			 struct hlist_head **head, unsigned long *flags)
__acquires(hlist_lock)
{
	unsigned long hash = hash_ptr(tsk, KPROBE_HASH_BITS);
	raw_spinlock_t *hlist_lock;

	*head = &kretprobe_inst_table[hash];
	hlist_lock = kretprobe_table_lock_ptr(hash);
	raw_spin_lock_irqsave(hlist_lock, *flags);
}
NOKPROBE_SYMBOL(kretprobe_hash_lock);

static void kretprobe_table_lock(unsigned long hash,
				 unsigned long *flags)
__acquires(hlist_lock)
{
	raw_spinlock_t *hlist_lock = kretprobe_table_lock_ptr(hash);
	raw_spin_lock_irqsave(hlist_lock, *flags);
}
NOKPROBE_SYMBOL(kretprobe_table_lock);

void kretprobe_hash_unlock(struct task_struct *tsk,
			   unsigned long *flags)
__releases(hlist_lock)
{
	unsigned long hash = hash_ptr(tsk, KPROBE_HASH_BITS);
	raw_spinlock_t *hlist_lock;

	hlist_lock = kretprobe_table_lock_ptr(hash);
	raw_spin_unlock_irqrestore(hlist_lock, *flags);
}
NOKPROBE_SYMBOL(kretprobe_hash_unlock);

static void kretprobe_table_unlock(unsigned long hash,
				   unsigned long *flags)
__releases(hlist_lock)
{
	raw_spinlock_t *hlist_lock = kretprobe_table_lock_ptr(hash);
	raw_spin_unlock_irqrestore(hlist_lock, *flags);
}
NOKPROBE_SYMBOL(kretprobe_table_unlock);

/*
 * This function is called from finish_task_switch when task tk becomes dead,
 * so that we can recycle any function-return probe instances associated
 * with this task. These left over instances represent probed functions
 * that have been called but will never return.
 */
void kprobe_flush_task(struct task_struct *tk)
{
	struct kretprobe_instance *ri;
	struct hlist_head *head, empty_rp;
	struct hlist_node *tmp;
	unsigned long hash, flags = 0;

	if (unlikely(!kprobes_initialized))
		/* Early boot.  kretprobe_table_locks not yet initialized. */
		return;

	INIT_HLIST_HEAD(&empty_rp);
	hash = hash_ptr(tk, KPROBE_HASH_BITS);
	head = &kretprobe_inst_table[hash];
	kretprobe_table_lock(hash, &flags);
	hlist_for_each_entry_safe(ri, tmp, head, hlist) {
		if (ri->task == tk)
			recycle_rp_inst(ri, &empty_rp);
	}
	kretprobe_table_unlock(hash, &flags);
	hlist_for_each_entry_safe(ri, tmp, &empty_rp, hlist) {
		hlist_del(&ri->hlist);
		kfree(ri);
	}
}
NOKPROBE_SYMBOL(kprobe_flush_task);

static inline void free_rp_inst(struct kretprobe *rp)
{
	struct kretprobe_instance *ri;
	struct hlist_node *next;

	hlist_for_each_entry_safe(ri, next, &rp->free_instances, hlist) {
		hlist_del(&ri->hlist);
		kfree(ri);
	}
}

static void cleanup_rp_inst(struct kretprobe *rp)
{
	unsigned long flags, hash;
	struct kretprobe_instance *ri;
	struct hlist_node *next;
	struct hlist_head *head;

	/* No race here */
	for (hash = 0; hash < KPROBE_TABLE_SIZE; hash++) {
		kretprobe_table_lock(hash, &flags);
		head = &kretprobe_inst_table[hash];
		hlist_for_each_entry_safe(ri, next, head, hlist) {
			if (ri->rp == rp)
				ri->rp = NULL;
		}
		kretprobe_table_unlock(hash, &flags);
	}
	free_rp_inst(rp);
}
NOKPROBE_SYMBOL(cleanup_rp_inst);

/*
* Add the new probe to ap->list. Fail if this is the
* second jprobe at the address - two jprobes can't coexist
*/
static int add_new_kprobe(struct kprobe *ap, struct kprobe *p)
{
	BUG_ON(kprobe_gone(ap) || kprobe_gone(p));

	if (p->break_handler || p->post_handler)
		unoptimize_kprobe(ap, true);	/* Fall back to normal kprobe */

	if (p->break_handler) {
		if (ap->break_handler)
			return -EEXIST;
		list_add_tail_rcu(&p->list, &ap->list);
		ap->break_handler = aggr_break_handler;
	} else
		list_add_rcu(&p->list, &ap->list);
	if (p->post_handler && !ap->post_handler)
		ap->post_handler = aggr_post_handler;

	return 0;
}

/*
 * Fill in the required fields of the "manager kprobe". Replace the
 * earlier kprobe in the hlist with the manager kprobe
 */
static void init_aggr_kprobe(struct kprobe *ap, struct kprobe *p)
{
	/* Copy p's insn slot to ap */
	copy_kprobe(p, ap);
	flush_insn_slot(ap);
	ap->addr = p->addr;
	ap->flags = p->flags & ~KPROBE_FLAG_OPTIMIZED;
	ap->pre_handler = aggr_pre_handler;
	ap->fault_handler = aggr_fault_handler;
	/* We don't care the kprobe which has gone. */
	if (p->post_handler && !kprobe_gone(p))
		ap->post_handler = aggr_post_handler;
	if (p->break_handler && !kprobe_gone(p))
		ap->break_handler = aggr_break_handler;

	INIT_LIST_HEAD(&ap->list);
	INIT_HLIST_NODE(&ap->hlist);

	list_add_rcu(&p->list, &ap->list);
	hlist_replace_rcu(&p->hlist, &ap->hlist);
}

/*
 * This is the second or subsequent kprobe at the address - handle
 * the intricacies
 */
static int register_aggr_kprobe(struct kprobe *orig_p, struct kprobe *p)
{
	int ret = 0;
	struct kprobe *ap = orig_p;

	/* For preparing optimization, jump_label_text_reserved() is called */
	jump_label_lock();
	/*
	 * Get online CPUs to avoid text_mutex deadlock.with stop machine,
	 * which is invoked by unoptimize_kprobe() in add_new_kprobe()
	 */
	get_online_cpus();
	mutex_lock(&text_mutex);

	if (!kprobe_aggrprobe(orig_p)) {
		/* If orig_p is not an aggr_kprobe, create new aggr_kprobe. */
		ap = alloc_aggr_kprobe(orig_p);
		if (!ap) {
			ret = -ENOMEM;
			goto out;
		}
		init_aggr_kprobe(ap, orig_p);
	} else if (kprobe_unused(ap))
		/* This probe is going to die. Rescue it */
		reuse_unused_kprobe(ap);

	if (kprobe_gone(ap)) {
		/*
		 * Attempting to insert new probe at the same location that
		 * had a probe in the module vaddr area which already
		 * freed. So, the instruction slot has already been
		 * released. We need a new slot for the new probe.
		 */
		ret = arch_prepare_kprobe(ap);
		if (ret)
			/*
			 * Even if fail to allocate new slot, don't need to
			 * free aggr_probe. It will be used next time, or
			 * freed by unregister_kprobe.
			 */
			goto out;

		/* Prepare optimized instructions if possible. */
		prepare_optimized_kprobe(ap);

		/*
		 * Clear gone flag to prevent allocating new slot again, and
		 * set disabled flag because it is not armed yet.
		 */
		ap->flags = (ap->flags & ~KPROBE_FLAG_GONE)
			    | KPROBE_FLAG_DISABLED;
	}

	/* Copy ap's insn slot to p */
	copy_kprobe(ap, p);
	ret = add_new_kprobe(ap, p);

out:
	mutex_unlock(&text_mutex);
	put_online_cpus();
	jump_label_unlock();

	if (ret == 0 && kprobe_disabled(ap) && !kprobe_disabled(p)) {
		ap->flags &= ~KPROBE_FLAG_DISABLED;
		if (!kprobes_all_disarmed)
			/* Arm the breakpoint again. */
			arm_kprobe(ap);
	}
	return ret;
}

bool __weak arch_within_kprobe_blacklist(unsigned long addr)
{
	/* The __kprobes marked functions and entry code must not be probed */
	return addr >= (unsigned long)__kprobes_text_start &&
	       addr < (unsigned long)__kprobes_text_end;
}

bool within_kprobe_blacklist(unsigned long addr)
{
	struct kprobe_blacklist_entry *ent;

	if (arch_within_kprobe_blacklist(addr))
		return true;
	/*
	 * If there exists a kprobe_blacklist, verify and
	 * fail any probe registration in the prohibited area
	 */
	list_for_each_entry(ent, &kprobe_blacklist, list) {
		if (addr >= ent->start_addr && addr < ent->end_addr)
			return true;
	}

	return false;
}

/*
 * If we have a symbol_name argument, look it up and add the offset field
 * to it. This way, we can specify a relative address to a symbol.
 * This returns encoded errors if it fails to look up symbol or invalid
 * combination of parameters.
 */
static kprobe_opcode_t *kprobe_addr(struct kprobe *p)
{
	kprobe_opcode_t *addr = p->addr;

	if ((p->symbol_name && p->addr) ||
	    (!p->symbol_name && !p->addr))
		goto invalid;

	if (p->symbol_name) {
		kprobe_lookup_name(p->symbol_name, addr);
		if (!addr)
			return ERR_PTR(-ENOENT);
	}

	addr = (kprobe_opcode_t *)(((char *)addr) + p->offset);
	if (addr)
		return addr;

invalid:
	return ERR_PTR(-EINVAL);
}

/* Check passed kprobe is valid and return kprobe in kprobe_table. */
static struct kprobe *__get_valid_kprobe(struct kprobe *p)
{
	struct kprobe *ap, *list_p;

	ap = get_kprobe(p->addr);
	if (unlikely(!ap))
		return NULL;

	if (p != ap) {
		list_for_each_entry_rcu(list_p, &ap->list, list)
			if (list_p == p)
			/* kprobe p is a valid probe */
				goto valid;
		return NULL;
	}
valid:
	return ap;
}

/* Return error if the kprobe is being re-registered */
static inline int check_kprobe_rereg(struct kprobe *p)
{
	int ret = 0;

	mutex_lock(&kprobe_mutex);
	if (__get_valid_kprobe(p))
		ret = -EINVAL;
	mutex_unlock(&kprobe_mutex);

	return ret;
}

int __weak arch_check_ftrace_location(struct kprobe *p)
{
	unsigned long ftrace_addr;

	ftrace_addr = ftrace_location((unsigned long)p->addr);
	if (ftrace_addr) {
#ifdef CONFIG_KPROBES_ON_FTRACE
		/* Given address is not on the instruction boundary */
		if ((unsigned long)p->addr != ftrace_addr)
			return -EILSEQ;
		p->flags |= KPROBE_FLAG_FTRACE;
#else	/* !CONFIG_KPROBES_ON_FTRACE */
		return -EINVAL;
#endif
	}
	return 0;
}

static int check_kprobe_address_safe(struct kprobe *p,
				     struct module **probed_mod)
{
	int ret;

	ret = arch_check_ftrace_location(p);
	if (ret)
		return ret;
	jump_label_lock();
	preempt_disable();

	/* Ensure it is not in reserved area nor out of text */
	if (!kernel_text_address((unsigned long) p->addr) ||
	    within_kprobe_blacklist((unsigned long) p->addr) ||
	    jump_label_text_reserved(p->addr, p->addr)) {
		ret = -EINVAL;
		goto out;
	}

	/* Check if are we probing a module */
	*probed_mod = __module_text_address((unsigned long) p->addr);
	if (*probed_mod) {
		/*
		 * We must hold a refcount of the probed module while updating
		 * its code to prohibit unexpected unloading.
		 */
		if (unlikely(!try_module_get(*probed_mod))) {
			ret = -ENOENT;
			goto out;
		}

		/*
		 * If the module freed .init.text, we couldn't insert
		 * kprobes in there.
		 */
		if (within_module_init((unsigned long)p->addr, *probed_mod) &&
		    (*probed_mod)->state != MODULE_STATE_COMING) {
			module_put(*probed_mod);
			*probed_mod = NULL;
			ret = -ENOENT;
		}
	}
out:
	preempt_enable();
	jump_label_unlock();

	return ret;
}

int register_kprobe(struct kprobe *p)
{
	int ret;
	struct kprobe *old_p;
	struct module *probed_mod;
	kprobe_opcode_t *addr;

	/* Adjust probe address from symbol */
	addr = kprobe_addr(p);
	if (IS_ERR(addr))
		return PTR_ERR(addr);
	p->addr = addr;

	ret = check_kprobe_rereg(p);
	if (ret)
		return ret;

	/* User can pass only KPROBE_FLAG_DISABLED to register_kprobe */
	p->flags &= KPROBE_FLAG_DISABLED;
	p->nmissed = 0;
	INIT_LIST_HEAD(&p->list);

	ret = check_kprobe_address_safe(p, &probed_mod);
	if (ret)
		return ret;

	mutex_lock(&kprobe_mutex);

	old_p = get_kprobe(p->addr);
	if (old_p) {
		/* Since this may unoptimize old_p, locking text_mutex. */
		ret = register_aggr_kprobe(old_p, p);
		goto out;
	}

	mutex_lock(&text_mutex);	/* Avoiding text modification */
	ret = prepare_kprobe(p);
	mutex_unlock(&text_mutex);
	if (ret)
		goto out;

	INIT_HLIST_NODE(&p->hlist);
	hlist_add_head_rcu(&p->hlist,
		       &kprobe_table[hash_ptr(p->addr, KPROBE_HASH_BITS)]);

	if (!kprobes_all_disarmed && !kprobe_disabled(p))
		arm_kprobe(p);

	/* Try to optimize kprobe */
	try_to_optimize_kprobe(p);

out:
	mutex_unlock(&kprobe_mutex);

	if (probed_mod)
		module_put(probed_mod);

	return ret;
}
EXPORT_SYMBOL_GPL(register_kprobe);

/* Check if all probes on the aggrprobe are disabled */
static int aggr_kprobe_disabled(struct kprobe *ap)
{
	struct kprobe *kp;

	list_for_each_entry_rcu(kp, &ap->list, list)
		if (!kprobe_disabled(kp))
			/*
			 * There is an active probe on the list.
			 * We can't disable this ap.
			 */
			return 0;

	return 1;
}

/* Disable one kprobe: Make sure called under kprobe_mutex is locked */
static struct kprobe *__disable_kprobe(struct kprobe *p)
{
	struct kprobe *orig_p;

	/* Get an original kprobe for return */
	orig_p = __get_valid_kprobe(p);
	if (unlikely(orig_p == NULL))
		return NULL;

	if (!kprobe_disabled(p)) {
		/* Disable probe if it is a child probe */
		if (p != orig_p)
			p->flags |= KPROBE_FLAG_DISABLED;

		/* Try to disarm and disable this/parent probe */
		if (p == orig_p || aggr_kprobe_disabled(orig_p)) {
			/*
			 * If kprobes_all_disarmed is set, orig_p
			 * should have already been disarmed, so
			 * skip unneed disarming process.
			 */
			if (!kprobes_all_disarmed)
				disarm_kprobe(orig_p, true);
			orig_p->flags |= KPROBE_FLAG_DISABLED;
		}
	}

	return orig_p;
}

/*
 * Unregister a kprobe without a scheduler synchronization.
 */
static int __unregister_kprobe_top(struct kprobe *p)
{
	struct kprobe *ap, *list_p;

	/* Disable kprobe. This will disarm it if needed. */
	ap = __disable_kprobe(p);
	if (ap == NULL)
		return -EINVAL;

	if (ap == p)
		/*
		 * This probe is an independent(and non-optimized) kprobe
		 * (not an aggrprobe). Remove from the hash list.
		 */
		goto disarmed;

	/* Following process expects this probe is an aggrprobe */
	WARN_ON(!kprobe_aggrprobe(ap));

	if (list_is_singular(&ap->list) && kprobe_disarmed(ap))
		/*
		 * !disarmed could be happen if the probe is under delayed
		 * unoptimizing.
		 */
		goto disarmed;
	else {
		/* If disabling probe has special handlers, update aggrprobe */
		if (p->break_handler && !kprobe_gone(p))
			ap->break_handler = NULL;
		if (p->post_handler && !kprobe_gone(p)) {
			list_for_each_entry_rcu(list_p, &ap->list, list) {
				if ((list_p != p) && (list_p->post_handler))
					goto noclean;
			}
			ap->post_handler = NULL;
		}
noclean:
		/*
		 * Remove from the aggrprobe: this path will do nothing in
		 * __unregister_kprobe_bottom().
		 */
		list_del_rcu(&p->list);
		if (!kprobe_disabled(ap) && !kprobes_all_disarmed)
			/*
			 * Try to optimize this probe again, because post
			 * handler may have been changed.
			 */
			optimize_kprobe(ap);
	}
	return 0;

disarmed:
	BUG_ON(!kprobe_disarmed(ap));
	hlist_del_rcu(&ap->hlist);
	return 0;
}

static void __unregister_kprobe_bottom(struct kprobe *p)
{
	struct kprobe *ap;

	if (list_empty(&p->list))
		/* This is an independent kprobe */
		arch_remove_kprobe(p);
	else if (list_is_singular(&p->list)) {
		/* This is the last child of an aggrprobe */
		ap = list_entry(p->list.next, struct kprobe, list);
		list_del(&p->list);
		free_aggr_kprobe(ap);
	}
	/* Otherwise, do nothing. */
}

int register_kprobes(struct kprobe **kps, int num)
{
	int i, ret = 0;

	if (num <= 0)
		return -EINVAL;
	for (i = 0; i < num; i++) {
		ret = register_kprobe(kps[i]);
		if (ret < 0) {
			if (i > 0)
				unregister_kprobes(kps, i);
			break;
		}
	}
	return ret;
}
EXPORT_SYMBOL_GPL(register_kprobes);

void unregister_kprobe(struct kprobe *p)
{
	unregister_kprobes(&p, 1);
}
EXPORT_SYMBOL_GPL(unregister_kprobe);

void unregister_kprobes(struct kprobe **kps, int num)
{
	int i;

	if (num <= 0)
		return;
	mutex_lock(&kprobe_mutex);
	for (i = 0; i < num; i++)
		if (__unregister_kprobe_top(kps[i]) < 0)
			kps[i]->addr = NULL;
	mutex_unlock(&kprobe_mutex);

	synchronize_sched();
	for (i = 0; i < num; i++)
		if (kps[i]->addr)
			__unregister_kprobe_bottom(kps[i]);
}
EXPORT_SYMBOL_GPL(unregister_kprobes);

static struct notifier_block kprobe_exceptions_nb = {
	.notifier_call = kprobe_exceptions_notify,
	.priority = 0x7fffffff /* we need to be notified first */
};

unsigned long __weak arch_deref_entry_point(void *entry)
{
	return (unsigned long)entry;
}

int register_jprobes(struct jprobe **jps, int num)
{
	struct jprobe *jp;
	int ret = 0, i;

	if (num <= 0)
		return -EINVAL;
	for (i = 0; i < num; i++) {
		unsigned long addr, offset;
		jp = jps[i];
		addr = arch_deref_entry_point(jp->entry);

		/* Verify probepoint is a function entry point */
		if (kallsyms_lookup_size_offset(addr, NULL, &offset) &&
		    offset == 0) {
			jp->kp.pre_handler = setjmp_pre_handler;
			jp->kp.break_handler = longjmp_break_handler;
			ret = register_kprobe(&jp->kp);
		} else
			ret = -EINVAL;

		if (ret < 0) {
			if (i > 0)
				unregister_jprobes(jps, i);
			break;
		}
	}
	return ret;
}
EXPORT_SYMBOL_GPL(register_jprobes);

int register_jprobe(struct jprobe *jp)
{
	return register_jprobes(&jp, 1);
}
EXPORT_SYMBOL_GPL(register_jprobe);

void unregister_jprobe(struct jprobe *jp)
{
	unregister_jprobes(&jp, 1);
}
EXPORT_SYMBOL_GPL(unregister_jprobe);

void unregister_jprobes(struct jprobe **jps, int num)
{
	int i;

	if (num <= 0)
		return;
	mutex_lock(&kprobe_mutex);
	for (i = 0; i < num; i++)
		if (__unregister_kprobe_top(&jps[i]->kp) < 0)
			jps[i]->kp.addr = NULL;
	mutex_unlock(&kprobe_mutex);

	synchronize_sched();
	for (i = 0; i < num; i++) {
		if (jps[i]->kp.addr)
			__unregister_kprobe_bottom(&jps[i]->kp);
	}
}
EXPORT_SYMBOL_GPL(unregister_jprobes);

#ifdef CONFIG_KRETPROBES
/*
 * This kprobe pre_handler is registered with every kretprobe. When probe
 * hits it will set up the return probe.
 */
static int pre_handler_kretprobe(struct kprobe *p, struct pt_regs *regs)
{
	struct kretprobe *rp = container_of(p, struct kretprobe, kp);
	unsigned long hash, flags = 0;
	struct kretprobe_instance *ri;

	/*
	 * To avoid deadlocks, prohibit return probing in NMI contexts,
	 * just skip the probe and increase the (inexact) 'nmissed'
	 * statistical counter, so that the user is informed that
	 * something happened:
	 */
	if (unlikely(in_nmi())) {
		rp->nmissed++;
		return 0;
	}

	/* TODO: consider to only swap the RA after the last pre_handler fired */
	hash = hash_ptr(current, KPROBE_HASH_BITS);
	raw_spin_lock_irqsave(&rp->lock, flags);
	if (!hlist_empty(&rp->free_instances)) {
		ri = hlist_entry(rp->free_instances.first,
				struct kretprobe_instance, hlist);
		hlist_del(&ri->hlist);
		raw_spin_unlock_irqrestore(&rp->lock, flags);

		ri->rp = rp;
		ri->task = current;

		if (rp->entry_handler && rp->entry_handler(ri, regs)) {
			raw_spin_lock_irqsave(&rp->lock, flags);
			hlist_add_head(&ri->hlist, &rp->free_instances);
			raw_spin_unlock_irqrestore(&rp->lock, flags);
			return 0;
		}

		arch_prepare_kretprobe(ri, regs);

		/* XXX(hch): why is there no hlist_move_head? */
		INIT_HLIST_NODE(&ri->hlist);
		kretprobe_table_lock(hash, &flags);
		hlist_add_head(&ri->hlist, &kretprobe_inst_table[hash]);
		kretprobe_table_unlock(hash, &flags);
	} else {
		rp->nmissed++;
		raw_spin_unlock_irqrestore(&rp->lock, flags);
	}
	return 0;
}
NOKPROBE_SYMBOL(pre_handler_kretprobe);

int register_kretprobe(struct kretprobe *rp)
{
	int ret = 0;
	struct kretprobe_instance *inst;
	int i;
	void *addr;

	if (kretprobe_blacklist_size) {
		addr = kprobe_addr(&rp->kp);
		if (IS_ERR(addr))
			return PTR_ERR(addr);

		for (i = 0; kretprobe_blacklist[i].name != NULL; i++) {
			if (kretprobe_blacklist[i].addr == addr)
				return -EINVAL;
		}
	}

	rp->kp.pre_handler = pre_handler_kretprobe;
	rp->kp.post_handler = NULL;
	rp->kp.fault_handler = NULL;
	rp->kp.break_handler = NULL;

	/* Pre-allocate memory for max kretprobe instances */
	if (rp->maxactive <= 0) {
#ifdef CONFIG_PREEMPT
		rp->maxactive = max_t(unsigned int, 10, 2*num_possible_cpus());
#else
		rp->maxactive = num_possible_cpus();
#endif
	}
	raw_spin_lock_init(&rp->lock);
	INIT_HLIST_HEAD(&rp->free_instances);
	for (i = 0; i < rp->maxactive; i++) {
		inst = kmalloc(sizeof(struct kretprobe_instance) +
			       rp->data_size, GFP_KERNEL);
		if (inst == NULL) {
			free_rp_inst(rp);
			return -ENOMEM;
		}
		INIT_HLIST_NODE(&inst->hlist);
		hlist_add_head(&inst->hlist, &rp->free_instances);
	}

	rp->nmissed = 0;
	/* Establish function entry probe point */
	ret = register_kprobe(&rp->kp);
	if (ret != 0)
		free_rp_inst(rp);
	return ret;
}
EXPORT_SYMBOL_GPL(register_kretprobe);

int register_kretprobes(struct kretprobe **rps, int num)
{
	int ret = 0, i;

	if (num <= 0)
		return -EINVAL;
	for (i = 0; i < num; i++) {
		ret = register_kretprobe(rps[i]);
		if (ret < 0) {
			if (i > 0)
				unregister_kretprobes(rps, i);
			break;
		}
	}
	return ret;
}
EXPORT_SYMBOL_GPL(register_kretprobes);

void unregister_kretprobe(struct kretprobe *rp)
{
	unregister_kretprobes(&rp, 1);
}
EXPORT_SYMBOL_GPL(unregister_kretprobe);

void unregister_kretprobes(struct kretprobe **rps, int num)
{
	int i;

	if (num <= 0)
		return;
	mutex_lock(&kprobe_mutex);
	for (i = 0; i < num; i++)
		if (__unregister_kprobe_top(&rps[i]->kp) < 0)
			rps[i]->kp.addr = NULL;
	mutex_unlock(&kprobe_mutex);

	synchronize_sched();
	for (i = 0; i < num; i++) {
		if (rps[i]->kp.addr) {
			__unregister_kprobe_bottom(&rps[i]->kp);
			cleanup_rp_inst(rps[i]);
		}
	}
}
EXPORT_SYMBOL_GPL(unregister_kretprobes);

#else /* CONFIG_KRETPROBES */
int register_kretprobe(struct kretprobe *rp)
{
	return -ENOSYS;
}
EXPORT_SYMBOL_GPL(register_kretprobe);

int register_kretprobes(struct kretprobe **rps, int num)
{
	return -ENOSYS;
}
EXPORT_SYMBOL_GPL(register_kretprobes);

void unregister_kretprobe(struct kretprobe *rp)
{
}
EXPORT_SYMBOL_GPL(unregister_kretprobe);

void unregister_kretprobes(struct kretprobe **rps, int num)
{
}
EXPORT_SYMBOL_GPL(unregister_kretprobes);

static int pre_handler_kretprobe(struct kprobe *p, struct pt_regs *regs)
{
	return 0;
}
NOKPROBE_SYMBOL(pre_handler_kretprobe);

#endif /* CONFIG_KRETPROBES */

/* Set the kprobe gone and remove its instruction buffer. */
static void kill_kprobe(struct kprobe *p)
{
	struct kprobe *kp;

	p->flags |= KPROBE_FLAG_GONE;
	if (kprobe_aggrprobe(p)) {
		/*
		 * If this is an aggr_kprobe, we have to list all the
		 * chained probes and mark them GONE.
		 */
		list_for_each_entry_rcu(kp, &p->list, list)
			kp->flags |= KPROBE_FLAG_GONE;
		p->post_handler = NULL;
		p->break_handler = NULL;
		kill_optimized_kprobe(p);
	}
	/*
	 * Here, we can remove insn_slot safely, because no thread calls
	 * the original probed function (which will be freed soon) any more.
	 */
	arch_remove_kprobe(p);
}

/* Disable one kprobe */
int disable_kprobe(struct kprobe *kp)
{
	int ret = 0;

	mutex_lock(&kprobe_mutex);

	/* Disable this kprobe */
	if (__disable_kprobe(kp) == NULL)
		ret = -EINVAL;

	mutex_unlock(&kprobe_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(disable_kprobe);

/* Enable one kprobe */
int enable_kprobe(struct kprobe *kp)
{
	int ret = 0;
	struct kprobe *p;

	mutex_lock(&kprobe_mutex);

	/* Check whether specified probe is valid. */
	p = __get_valid_kprobe(kp);
	if (unlikely(p == NULL)) {
		ret = -EINVAL;
		goto out;
	}

	if (kprobe_gone(kp)) {
		/* This kprobe has gone, we couldn't enable it. */
		ret = -EINVAL;
		goto out;
	}

	if (p != kp)
		kp->flags &= ~KPROBE_FLAG_DISABLED;

	if (!kprobes_all_disarmed && kprobe_disabled(p)) {
		p->flags &= ~KPROBE_FLAG_DISABLED;
		arm_kprobe(p);
	}
out:
	mutex_unlock(&kprobe_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(enable_kprobe);

void dump_kprobe(struct kprobe *kp)
{
	printk(KERN_WARNING "Dumping kprobe:\n");
	printk(KERN_WARNING "Name: %s\nAddress: %p\nOffset: %x\n",
	       kp->symbol_name, kp->addr, kp->offset);
}
NOKPROBE_SYMBOL(dump_kprobe);

/*
 * Lookup and populate the kprobe_blacklist.
 *
 * Unlike the kretprobe blacklist, we'll need to determine
 * the range of addresses that belong to the said functions,
 * since a kprobe need not necessarily be at the beginning
 * of a function.
 */
static int __init populate_kprobe_blacklist(unsigned long *start,
					     unsigned long *end)
{
	unsigned long *iter;
	struct kprobe_blacklist_entry *ent;
	unsigned long entry, offset = 0, size = 0;

	for (iter = start; iter < end; iter++) {
		entry = arch_deref_entry_point((void *)*iter);

		if (!kernel_text_address(entry) ||
		    !kallsyms_lookup_size_offset(entry, &size, &offset)) {
			pr_err("Failed to find blacklist at %p\n",
				(void *)entry);
			continue;
		}

		ent = kmalloc(sizeof(*ent), GFP_KERNEL);
		if (!ent)
			return -ENOMEM;
		ent->start_addr = entry;
		ent->end_addr = entry + size;
		INIT_LIST_HEAD(&ent->list);
		list_add_tail(&ent->list, &kprobe_blacklist);
	}
	return 0;
}

/* Module notifier call back, checking kprobes on the module */
static int kprobes_module_callback(struct notifier_block *nb,
				   unsigned long val, void *data)
{
	struct module *mod = data;
	struct hlist_head *head;
	struct kprobe *p;
	unsigned int i;
	int checkcore = (val == MODULE_STATE_GOING);

	if (val != MODULE_STATE_GOING && val != MODULE_STATE_LIVE)
		return NOTIFY_DONE;

	/*
	 * When MODULE_STATE_GOING was notified, both of module .text and
	 * .init.text sections would be freed. When MODULE_STATE_LIVE was
	 * notified, only .init.text section would be freed. We need to
	 * disable kprobes which have been inserted in the sections.
	 */
	mutex_lock(&kprobe_mutex);
	for (i = 0; i < KPROBE_TABLE_SIZE; i++) {
		head = &kprobe_table[i];
		hlist_for_each_entry_rcu(p, head, hlist)
			if (within_module_init((unsigned long)p->addr, mod) ||
			    (checkcore &&
			     within_module_core((unsigned long)p->addr, mod))) {
				/*
				 * The vaddr this probe is installed will soon
				 * be vfreed buy not synced to disk. Hence,
				 * disarming the breakpoint isn't needed.
				 */
				kill_kprobe(p);
			}
	}
	mutex_unlock(&kprobe_mutex);
	return NOTIFY_DONE;
}

static struct notifier_block kprobe_module_nb = {
	.notifier_call = kprobes_module_callback,
	.priority = 0
};

/* Markers of _kprobe_blacklist section */
extern unsigned long __start_kprobe_blacklist[];
extern unsigned long __stop_kprobe_blacklist[];

static int __init init_kprobes(void)
{
	int i, err = 0;

	/* FIXME allocate the probe table, currently defined statically */
	/* initialize all list heads */
	for (i = 0; i < KPROBE_TABLE_SIZE; i++) {
		INIT_HLIST_HEAD(&kprobe_table[i]);
		INIT_HLIST_HEAD(&kretprobe_inst_table[i]);
		raw_spin_lock_init(&(kretprobe_table_locks[i].lock));
	}

	err = populate_kprobe_blacklist(__start_kprobe_blacklist,
					__stop_kprobe_blacklist);
	if (err) {
		pr_err("kprobes: failed to populate blacklist: %d\n", err);
		pr_err("Please take care of using kprobes.\n");
	}

	if (kretprobe_blacklist_size) {
		/* lookup the function address from its name */
		for (i = 0; kretprobe_blacklist[i].name != NULL; i++) {
			kprobe_lookup_name(kretprobe_blacklist[i].name,
					   kretprobe_blacklist[i].addr);
			if (!kretprobe_blacklist[i].addr)
				printk("kretprobe: lookup failed: %s\n",
				       kretprobe_blacklist[i].name);
		}
	}

#if defined(CONFIG_OPTPROBES)
#if defined(__ARCH_WANT_KPROBES_INSN_SLOT)
	/* Init kprobe_optinsn_slots */
	kprobe_optinsn_slots.insn_size = MAX_OPTINSN_SIZE;
#endif
	/* By default, kprobes can be optimized */
	kprobes_allow_optimization = true;
#endif

	/* By default, kprobes are armed */
	kprobes_all_disarmed = false;

	err = arch_init_kprobes();
	if (!err)
		err = register_die_notifier(&kprobe_exceptions_nb);
	if (!err)
		err = register_module_notifier(&kprobe_module_nb);

	kprobes_initialized = (err == 0);

	if (!err)
		init_test_probes();
	return err;
}

#ifdef CONFIG_DEBUG_FS
static void report_probe(struct seq_file *pi, struct kprobe *p,
		const char *sym, int offset, char *modname, struct kprobe *pp)
{
	char *kprobe_type;

	if (p->pre_handler == pre_handler_kretprobe)
		kprobe_type = "r";
	else if (p->pre_handler == setjmp_pre_handler)
		kprobe_type = "j";
	else
		kprobe_type = "k";

	if (sym)
		seq_printf(pi, "%p  %s  %s+0x%x  %s ",
			p->addr, kprobe_type, sym, offset,
			(modname ? modname : " "));
	else
		seq_printf(pi, "%p  %s  %p ",
			p->addr, kprobe_type, p->addr);

	if (!pp)
		pp = p;
	seq_printf(pi, "%s%s%s%s\n",
		(kprobe_gone(p) ? "[GONE]" : ""),
		((kprobe_disabled(p) && !kprobe_gone(p)) ?  "[DISABLED]" : ""),
		(kprobe_optimized(pp) ? "[OPTIMIZED]" : ""),
		(kprobe_ftrace(pp) ? "[FTRACE]" : ""));
}

static void *kprobe_seq_start(struct seq_file *f, loff_t *pos)
{
	return (*pos < KPROBE_TABLE_SIZE) ? pos : NULL;
}

static void *kprobe_seq_next(struct seq_file *f, void *v, loff_t *pos)
{
	(*pos)++;
	if (*pos >= KPROBE_TABLE_SIZE)
		return NULL;
	return pos;
}

static void kprobe_seq_stop(struct seq_file *f, void *v)
{
	/* Nothing to do */
}

static int show_kprobe_addr(struct seq_file *pi, void *v)
{
	struct hlist_head *head;
	struct kprobe *p, *kp;
	const char *sym = NULL;
	unsigned int i = *(loff_t *) v;
	unsigned long offset = 0;
	char *modname, namebuf[KSYM_NAME_LEN];

	head = &kprobe_table[i];
	preempt_disable();
	hlist_for_each_entry_rcu(p, head, hlist) {
		sym = kallsyms_lookup((unsigned long)p->addr, NULL,
					&offset, &modname, namebuf);
		if (kprobe_aggrprobe(p)) {
			list_for_each_entry_rcu(kp, &p->list, list)
				report_probe(pi, kp, sym, offset, modname, p);
		} else
			report_probe(pi, p, sym, offset, modname, NULL);
	}
	preempt_enable();
	return 0;
}

static const struct seq_operations kprobes_seq_ops = {
	.start = kprobe_seq_start,
	.next  = kprobe_seq_next,
	.stop  = kprobe_seq_stop,
	.show  = show_kprobe_addr
};

static int kprobes_open(struct inode *inode, struct file *filp)
{
	return seq_open(filp, &kprobes_seq_ops);
}

static const struct file_operations debugfs_kprobes_operations = {
	.open           = kprobes_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = seq_release,
};

/* kprobes/blacklist -- shows which functions can not be probed */
static void *kprobe_blacklist_seq_start(struct seq_file *m, loff_t *pos)
{
	return seq_list_start(&kprobe_blacklist, *pos);
}

static void *kprobe_blacklist_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	return seq_list_next(v, &kprobe_blacklist, pos);
}

static int kprobe_blacklist_seq_show(struct seq_file *m, void *v)
{
	struct kprobe_blacklist_entry *ent =
		list_entry(v, struct kprobe_blacklist_entry, list);

	seq_printf(m, "0x%p-0x%p\t%ps\n", (void *)ent->start_addr,
		   (void *)ent->end_addr, (void *)ent->start_addr);
	return 0;
}

static const struct seq_operations kprobe_blacklist_seq_ops = {
	.start = kprobe_blacklist_seq_start,
	.next  = kprobe_blacklist_seq_next,
	.stop  = kprobe_seq_stop,	/* Reuse void function */
	.show  = kprobe_blacklist_seq_show,
};

static int kprobe_blacklist_open(struct inode *inode, struct file *filp)
{
	return seq_open(filp, &kprobe_blacklist_seq_ops);
}

static const struct file_operations debugfs_kprobe_blacklist_ops = {
	.open           = kprobe_blacklist_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = seq_release,
};

static void arm_all_kprobes(void)
{
	struct hlist_head *head;
	struct kprobe *p;
	unsigned int i;

	mutex_lock(&kprobe_mutex);

	/* If kprobes are armed, just return */
	if (!kprobes_all_disarmed)
		goto already_enabled;

	/*
	 * optimize_kprobe() called by arm_kprobe() checks
	 * kprobes_all_disarmed, so set kprobes_all_disarmed before
	 * arm_kprobe.
	 */
	kprobes_all_disarmed = false;
	/* Arming kprobes doesn't optimize kprobe itself */
	for (i = 0; i < KPROBE_TABLE_SIZE; i++) {
		head = &kprobe_table[i];
		hlist_for_each_entry_rcu(p, head, hlist)
			if (!kprobe_disabled(p))
				arm_kprobe(p);
	}

	printk(KERN_INFO "Kprobes globally enabled\n");

already_enabled:
	mutex_unlock(&kprobe_mutex);
	return;
}

static void disarm_all_kprobes(void)
{
	struct hlist_head *head;
	struct kprobe *p;
	unsigned int i;

	mutex_lock(&kprobe_mutex);

	/* If kprobes are already disarmed, just return */
	if (kprobes_all_disarmed) {
		mutex_unlock(&kprobe_mutex);
		return;
	}

	kprobes_all_disarmed = true;
	printk(KERN_INFO "Kprobes globally disabled\n");

	for (i = 0; i < KPROBE_TABLE_SIZE; i++) {
		head = &kprobe_table[i];
		hlist_for_each_entry_rcu(p, head, hlist) {
			if (!arch_trampoline_kprobe(p) && !kprobe_disabled(p))
				disarm_kprobe(p, false);
		}
	}
	mutex_unlock(&kprobe_mutex);

	/* Wait for disarming all kprobes by optimizer */
	wait_for_kprobe_optimizer();
}

/*
 * XXX: The debugfs bool file interface doesn't allow for callbacks
 * when the bool state is switched. We can reuse that facility when
 * available
 */
static ssize_t read_enabled_file_bool(struct file *file,
	       char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[3];

	if (!kprobes_all_disarmed)
		buf[0] = '1';
	else
		buf[0] = '0';
	buf[1] = '\n';
	buf[2] = 0x00;
	return simple_read_from_buffer(user_buf, count, ppos, buf, 2);
}

static ssize_t write_enabled_file_bool(struct file *file,
	       const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[32];
	size_t buf_size;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	buf[buf_size] = '\0';
	switch (buf[0]) {
	case 'y':
	case 'Y':
	case '1':
		arm_all_kprobes();
		break;
	case 'n':
	case 'N':
	case '0':
		disarm_all_kprobes();
		break;
	default:
		return -EINVAL;
	}

	return count;
}

static const struct file_operations fops_kp = {
	.read =         read_enabled_file_bool,
	.write =        write_enabled_file_bool,
	.llseek =	default_llseek,
};

static int __init debugfs_kprobe_init(void)
{
	struct dentry *dir, *file;
	unsigned int value = 1;

	dir = debugfs_create_dir("kprobes", NULL);
	if (!dir)
		return -ENOMEM;

	file = debugfs_create_file("list", 0400, dir, NULL,
				&debugfs_kprobes_operations);
	if (!file)
		goto error;

	file = debugfs_create_file("enabled", 0600, dir,
					&value, &fops_kp);
	if (!file)
		goto error;

	file = debugfs_create_file("blacklist", 0400, dir, NULL,
				&debugfs_kprobe_blacklist_ops);
	if (!file)
		goto error;

	return 0;

error:
	debugfs_remove(dir);
	return -ENOMEM;
}

late_initcall(debugfs_kprobe_init);
#endif /* CONFIG_DEBUG_FS */

module_init(init_kprobes);

/* defined in arch/.../kernel/kprobes.c */
EXPORT_SYMBOL_GPL(jprobe_return);
