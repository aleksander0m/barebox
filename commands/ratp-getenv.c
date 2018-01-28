/*
 * Copyright (c) 2018 Sascha Hauer <s.hauer@pengutronix.de>, Pengutronix
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
 * RATP getenv
 */

#include <common.h>
#include <ratp_bb.h>
#include <malloc.h>
#include <environment.h>

static void *xmemdup_add_zero(const void *buf, int len)
{
	void *ret;

	ret = xzalloc(len + 1);
	*(uint8_t *)(ret + len) = 0;
	memcpy(ret, buf, len);

	return ret;
}

static int ratp_cmd_getenv(const struct ratp_bb *req, int req_len,
			   struct ratp_bb **rsp, int *rsp_len)
{
	int dlen = req_len - sizeof (struct ratp_bb);
	char *varname;
	const char *value;

	varname = xmemdup_add_zero (req->data, dlen);
	value = getenv (varname);
	free (varname);

	dlen = strlen (value);

	*rsp_len = sizeof(struct ratp_bb) + dlen;
	*rsp = xzalloc(*rsp_len);
	(*rsp)->type = cpu_to_be16(BB_RATP_TYPE_GETENV);
	(*rsp)->flags = cpu_to_be16(BB_RATP_FLAG_RESPONSE);
	memcpy ((*rsp)->data, value, dlen);
	return 0;
}

BAREBOX_RATP_CMD_START(GETENV)
	.cmd = ratp_cmd_getenv
BAREBOX_RATP_CMD_END
