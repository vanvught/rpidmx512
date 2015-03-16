
#ifndef MONITOR_DEBUG_H_
#define MONITOR_DEBUG_H_

#define MONITOR_LINE_LABEL		5
#define MONITOR_LINE_INFO		6
#define MONITOR_LINE_RDM_DATA	11
#define MONITOR_LINE_STATUS		23

extern void monitor_debug_line(uint8_t, const char *, ...);

#endif /* MONITOR_DEBUG_H_ */
