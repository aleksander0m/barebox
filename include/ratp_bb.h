#ifndef __RATP_BB_H
#define __RATP_BB_H

#include <linux/stringify.h>

#define BB_RATP_TYPE_CONSOLE		1
#define BB_RATP_TYPE_PING		2
#define BB_RATP_TYPE_GETENV		3
#define BB_RATP_TYPE_FS			4
#define BB_RATP_TYPE_RESET		5

#define BB_RATP_FLAG_NONE		0
#define BB_RATP_FLAG_RESPONSE		(1 << 0) /* Packet is a response */
#define BB_RATP_FLAG_INDICATION		(1 << 1) /* Packet is an indication */

struct ratp_bb {
	uint16_t type;
	uint16_t flags;
	uint8_t data[];
};

struct ratp_bb_pkt {
	unsigned int len;
	uint8_t data[];
};

int  barebox_ratp(struct console_device *cdev);
void barebox_ratp_command_run(void);
int  barebox_ratp_fs_call(struct ratp_bb_pkt *tx, struct ratp_bb_pkt **rx);
int  barebox_ratp_fs_mount(const char *path);

/*
 * RATP commands definition
 */

struct ratp_command {
	struct list_head  list;
	uint16_t          id;
	int		(*cmd)(const struct ratp_bb *req,
			       int req_len,
			       struct ratp_bb **rsp,
			       int *rsp_len);
}
#ifdef __x86_64__
/* This is required because the linker will put symbols on a 64 bit alignment */
__attribute__((aligned(64)))
#endif
;

#define BAREBOX_RATP_CMD_START(_name)							\
extern const struct ratp_command __barebox_cmd_##_name;					\
const struct ratp_command __barebox_cmd_##_name						\
	__attribute__ ((unused,section (".barebox_ratp_cmd_" __stringify(_name)))) = {	\
	.id		= BB_RATP_TYPE_##_name,

#define BAREBOX_RATP_CMD_END								\
};

int register_ratp_command(struct ratp_command *cmd);

#endif /* __RATP_BB_H */
