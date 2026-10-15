// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stddef.h>
void mcp_init(void);
void mcp_stop_input(void);
void mcp_network_lost(void);
void mcp_open_setup(void);
void mcp_close_setup(void);
bool mcp_setup_pending(char *code, size_t size);
void mcp_confirm_setup(void);
bool mcp_reset_identity(void);
bool mcp_ready(void);
bool mcp_http_ready(void);
int mcp_http_error(void);
bool mcp_https_ready(void);
typedef enum {
    MCP_PAIR_IDLE,
    MCP_PAIR_PENDING,
    MCP_PAIR_WORKING,
    MCP_PAIR_READY,
    MCP_PAIR_FAILED,
    MCP_PAIR_EXPIRED
} mcp_pair_state;
mcp_pair_state mcp_pair_snapshot(char *code, size_t code_size, char *fingerprint,
                                 size_t fingerprint_size);
