/*
 * pixeldmxparamsconst.h
 */

#ifndef PIXELDMXPARAMSCONST_H_
#define PIXELDMXPARAMSCONST_H_

struct PixelDmxParamsConst {
#if defined (CONFIG_DMXNODE_PIXEL_MAX_PORTS)
	static inline const char START_UNI_PORT[CONFIG_DMXNODE_PIXEL_MAX_PORTS][20] = {
			"start_uni_port_1",
# if CONFIG_DMXNODE_PIXEL_MAX_PORTS > 2
			"start_uni_port_2",
			"start_uni_port_3",
			"start_uni_port_4",
			"start_uni_port_5",
			"start_uni_port_6",
			"start_uni_port_7",
			"start_uni_port_8",
# endif
# if CONFIG_DMXNODE_PIXEL_MAX_PORTS == 16
			"start_uni_port_9",
			"start_uni_port_10",
			"start_uni_port_11",
			"start_uni_port_12",
			"start_uni_port_13",
			"start_uni_port_14",
			"start_uni_port_15",
			"start_uni_port_16"
# endif
	};
#endif
};

#endif /* PIXELDMXPARAMSCONST_H_ */
