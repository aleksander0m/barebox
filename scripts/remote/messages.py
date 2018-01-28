#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from __future__ import absolute_import, division, print_function

import struct

class BBFlag(object):
    none = 0
    response = 1 << 0
    indication = 1 << 1


class BBType(object):
    console = 1
    ping = 2
    getenv = 3
    fs = 8
    fs_return = 9


class BBPacket(object):
    def __init__(self, p_type=0, p_flags=0, payload="", raw=None):
        self.p_type = p_type
        self.p_flags = p_flags
        if raw is not None:
            self.unpack(raw)
        else:
            self.payload = payload

    def __repr__(self):
        return "BBPacket(%i, %i)" % (self.p_type, self.p_flags)

    def _unpack_payload(self, data):
        self.payload = data

    def _pack_payload(self):
        return self.payload

    def unpack(self, data):
        self.p_type, self.p_flags = struct.unpack("!HH", data[:4])
        self._unpack_payload(data[4:])

    def pack(self):
        return struct.pack("!HH", self.p_type, self.p_flags) + \
            self._pack_payload()


class BBPacketConsoleRequest(BBPacket):
    def __init__(self, raw=None, cmd=None):
        self.cmd = cmd
        super(BBPacketConsoleRequest, self).__init__(BBType.console,
                                                     raw=raw)

    def __repr__(self):
        return "BBPacketConsoleRequest(cmd=%r)" % self.cmd

    def _unpack_payload(self, payload):
        self.cmd = payload

    def _pack_payload(self):
        return self.cmd


class BBPacketConsoleResponse(BBPacket):
    def __init__(self, raw=None, exit_code=None):
        self.exit_code = exit_code
        super(BBPacketConsoleResponse, self).__init__(BBType.console,
                                                      BBFlag.response,
                                                      raw=raw)

    def __repr__(self):
        return "BBPacketConsoleResponse(exit_code=%i)" % self.exit_code

    def _unpack_payload(self, data):
        self.exit_code, = struct.unpack("!L", data[:4])

    def _pack_payload(self):
        return struct.pack("!L", self.exit_code)


class BBPacketConsoleIndication(BBPacket):
    def __init__(self, raw=None, text=None):
        self.text = text
        super(BBPacketConsoleIndication, self).__init__(BBType.console,
                                                        BBFlag.indication,
                                                        raw=raw)

    def __repr__(self):
        return "BBPacketConsoleIndication(text=%r)" % self.text

    def _unpack_payload(self, payload):
        self.text = payload

    def _pack_payload(self):
        return self.text


class BBPacketPingRequest(BBPacket):
    def __init__(self, raw=None):
        super(BBPacketPingRequest, self).__init__(BBType.ping,
                                                  raw=raw)

    def __repr__(self):
        return "BBPacketPingRequest()"


class BBPacketPingResponse(BBPacket):
    def __init__(self, raw=None):
        super(BBPacketPingResponse, self).__init__(BBType.ping,
                                                   BBFlag.response,
                                                   raw=raw)

    def __repr__(self):
        return "BBPacketPingResponse()"


class BBPacketGetenvRequest(BBPacket):
    def __init__(self, raw=None, varname=None):
        self.varname = varname
        super(BBPacketGetenvRequest, self).__init__(BBType.getenv,
                                                    raw=raw)

    def __repr__(self):
        return "BBPacketGetenvRequest(varname=%r)" % self.varname

    def _unpack_payload(self, payload):
        self.varname = payload

    def _pack_payload(self):
        return self.varname


class BBPacketGetenvResponse(BBPacket):
    def __init__(self, raw=None, text=None):
        self.text = text
        super(BBPacketGetenvResponse, self).__init__(BBType.getenv,
                                                     BBFlag.response,
                                                     raw=raw)

    def __repr__(self):
        return "BBPacketGetenvResponse(varvalue=%s)" % self.text

    def _unpack_payload(self, payload):
        self.text = payload

    def _pack_payload(self):
        return self.text


class BBPacketFS(BBPacket):
    def __init__(self, raw=None, payload=None):
        super(BBPacketFS, self).__init__(BBType.fs, payload=payload, raw=raw)

    def __repr__(self):
        return "BBPacketFS(payload=%r)" % self.payload


class BBPacketFSReturn(BBPacket):
    def __init__(self, raw=None, payload=None):
        super(BBPacketFSReturn, self).__init__(BBType.fs_return, payload=payload, raw=raw)

    def __repr__(self):
        return "BBPacketFSReturn(payload=%r)" % self.payload
