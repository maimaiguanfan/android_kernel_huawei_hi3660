#include "lz4accel.h"
#include <asm/cputype.h>

int _lz4_decompress_asm(uint8_t **dst_ptr, uint8_t *dst_begin,
			uint8_t *dst_end, const uint8_t **src_ptr,
			const uint8_t *src_end);

int _lz4_decompress_asm_noprfm(uint8_t **dst_ptr, uint8_t *dst_begin,
			       uint8_t *dst_end, const uint8_t **src_ptr,
			       const uint8_t *src_end);


int lz4_decompress_asm_select(uint8_t **dst_ptr, uint8_t *dst_begin,
			      uint8_t *dst_end, const uint8_t **src_ptr,
			      const uint8_t *src_end) {
	const unsigned i = smp_processor_id();

	switch(read_cpuid_part_number()) {
	case ARM_CPU_PART_CORTEX_A53:
		lz4_decompress_asm_fn[i] = _lz4_decompress_asm_noprfm;
		return _lz4_decompress_asm_noprfm(dst_ptr, dst_begin, dst_end,
						  src_ptr, src_end);
	}
	lz4_decompress_asm_fn[i] = _lz4_decompress_asm;
	return _lz4_decompress_asm(dst_ptr, dst_begin, dst_end,
				   src_ptr, src_end);
}

int (*lz4_decompress_asm_fn[NR_CPUS])(uint8_t **dst_ptr, uint8_t *dst_begin,
	uint8_t *dst_end, const uint8_t **src_ptr, const uint8_t *src_end)
__read_mostly = {
	[0 ... NR_CPUS-1]  = lz4_decompress_asm_select,
};

