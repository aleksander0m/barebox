/*
 * Copyright (c) 2011 Sascha Hauer <s.hauer@pengutronix.de>, Pengutronix
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*
 * Memory Functions
 *
 * Copied from FADS ROM, Dan Malek (dmalek@jlc.net)
 */

#include <common.h>
#include <command.h>
#include <ratp_bb.h>
#include <init.h>
#include <driver.h>
#include <malloc.h>
#include <errno.h>
#include <fs.h>
#include <libfile.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/stat.h>
#include <xfuncs.h>

extern char *mem_rw_buf;

static int common_mem_md(const char *filename,
			 loff_t start,
			 loff_t size,
			 int swab,
			 int mode,
			 uint8_t *output)
{
	int r, now, t;
	int ret = 0;
	int fd;
	void *map;

	fd = open_and_lseek(filename, mode | O_RDONLY, start);
	if (fd < 0)
		return 1;

	map = memmap(fd, PROT_READ);
	if (map != (void *)-1) {
		if (output)
			memcpy(output, (uint8_t *)(map + start), size);
		 else
			ret = memory_display(map + start, start, size,
					     mode >> O_RWSIZE_SHIFT, swab);
		goto out;
	}

	t = 0;
	do {
		now = min(size, (loff_t)RW_BUF_SIZE);
		r = read(fd, mem_rw_buf, now);
		if (r < 0) {
			perror("read");
			goto out;
		}
		if (!r)
			goto out;

		if (output)
			memcpy(output + t, (uint8_t *)(mem_rw_buf), r);
		else if ((ret = memory_display(mem_rw_buf, start + t, r,
					       mode >> O_RWSIZE_SHIFT, swab)))
			goto out;

		size  -= r;
		t     += r;
	} while (size);

out:
	close(fd);

	return ret ? 1 : 0;
}

static int do_mem_md(int argc, char *argv[])
{
	loff_t	start = 0, size = 0x100;
	char *filename = "/dev/mem";
	int mode = O_RWSIZE_4;
	int swab = 0;

	if (argc < 2)
		return COMMAND_ERROR_USAGE;

	if (mem_parse_options(argc, argv, "bwlqs:x", &mode, &filename, NULL,
			&swab) < 0)
		return 1;

	if (optind < argc) {
		if (parse_area_spec(argv[optind], &start, &size)) {
			printf("could not parse: %s\n", argv[optind]);
			return 1;
		}
		if (size == ~0)
			size = 0x100;
	}

	return common_mem_md(filename, start, size, swab, mode, NULL);
}


BAREBOX_CMD_HELP_START(md)
BAREBOX_CMD_HELP_TEXT("Display (hex dump) a memory region.")
BAREBOX_CMD_HELP_TEXT("")
BAREBOX_CMD_HELP_TEXT("Options:")
BAREBOX_CMD_HELP_OPT ("-b",  "byte access")
BAREBOX_CMD_HELP_OPT ("-w",  "word access (16 bit)")
BAREBOX_CMD_HELP_OPT ("-l",  "long access (32 bit)")
BAREBOX_CMD_HELP_OPT ("-q",  "quad access (64 bit)")
BAREBOX_CMD_HELP_OPT ("-s FILE",  "display file (default /dev/mem)")
BAREBOX_CMD_HELP_OPT ("-x",       "swap bytes at output")
BAREBOX_CMD_HELP_TEXT("")
BAREBOX_CMD_HELP_TEXT("Memory regions can be specified in two different forms: START+SIZE")
BAREBOX_CMD_HELP_TEXT("or START-END, If START is omitted it defaults to 0x100")
BAREBOX_CMD_HELP_TEXT("Sizes can be specified as decimal, or if prefixed with 0x as hexadecimal.")
BAREBOX_CMD_HELP_TEXT("An optional suffix of k, M or G is for kbytes, Megabytes or Gigabytes.")
BAREBOX_CMD_HELP_END


BAREBOX_CMD_START(md)
	.cmd		= do_mem_md,
	BAREBOX_CMD_DESC("memory display")
	BAREBOX_CMD_OPTS("[-bwlsx] REGION")
	BAREBOX_CMD_GROUP(CMD_GRP_MEM)
	BAREBOX_CMD_HELP(cmd_md_help)
BAREBOX_CMD_END

/* RATP command */

struct ratp_bb_md_request {
	struct ratp_bb header;
	uint16_t start;
	uint16_t size;
	uint16_t path_size;
	uint8_t  path[];
} __attribute__((packed));

struct ratp_bb_md_response {
	struct ratp_bb header;
	uint16_t data_size;
	uint8_t  data[];
} __attribute__((packed));

static void *xmemdup_add_zero(const void *buf, int len)
{
	void *ret;

	ret = xzalloc(len + 1);
	*(uint8_t *)(ret + len) = 0;
	memcpy(ret, buf, len);

	return ret;
}

static int ratp_cmd_md(const struct ratp_bb *req, int req_len,
		       struct ratp_bb **rsp, int *rsp_len)
{
	struct ratp_bb_md_request *md_req = (struct ratp_bb_md_request *)req;
	struct ratp_bb_md_response *md_rsp;
	int md_rsp_len;
	uint16_t start, size, path_size;
	char *path;
	int ret;

	if (req_len < sizeof(*md_req)) {
		printf("ratp md ignored: size mismatch (%d < %zu)\n",
		       req_len, sizeof (*md_req));
		return 1;
	}

	path_size = be16_to_cpu(md_req->path_size);
	if (req_len < (sizeof(*md_req) + path_size)) {
		printf("ratp md ignored: size mismatch (%d < %zu): path not fully given\n",
		       req_len, sizeof(*md_req) + path_size);
		return 1;
	}

	start = be16_to_cpu (md_req->start);
	size = be16_to_cpu (md_req->size);

	if (!path_size) {
		printf("ratp md ignored: no filepath given\n");
		return 1;
	}

	path = xmemdup_add_zero(md_req->path, path_size);
	if (!path)
		return 1;

	md_rsp_len = sizeof(*md_rsp) + size;
	md_rsp = xzalloc(md_rsp_len);
	md_rsp->header.type = cpu_to_be16(BB_RATP_TYPE_MD);
	md_rsp->header.flags = cpu_to_be16(BB_RATP_FLAG_RESPONSE);
	md_rsp->data_size = cpu_to_be16(size);

	ret = common_mem_md(path, start, size, 0, O_RWSIZE_1, md_rsp->data);
	if (ret) {
		free (md_rsp);
		*rsp = NULL;
		*rsp_len = 0;
	} else {
		*rsp = (struct ratp_bb *)md_rsp;
		*rsp_len = md_rsp_len;
	}

	free (path);
	return ret;
}

BAREBOX_RATP_CMD_START(MD)
	.cmd = ratp_cmd_md
BAREBOX_RATP_CMD_END
