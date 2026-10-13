// SPDX-License-Identifier: Apache-2.0
#pragma once
#define EMPTY_SCHEMA "{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}"
#define POINT_PROP "{\"type\":\"number\",\"minimum\":0,\"maximum\":1}"
static const char mcp_tools_json[] =
    "{\"tools\":["
    "{\"name\":\"device_status\",\"description\":\"Read actual USB, mode, control time and input "
    "state. No screen capture.\",\"inputSchema\":" EMPTY_SCHEMA
    ",\"annotations\":{\"readOnlyHint\":true,\"idempotentHint\":true}},"
    "{\"name\":\"tap\",\"description\":\"Tap normalized portrait coordinates 0..1 via iPhone USB "
    "HID. Requires device MCP mode and physical authorization. Completion describes HID only; "
    "observe the phone separately. Never retry an unknown "
    "outcome.\",\"inputSchema\":{\"type\":\"object\",\"properties\":{\"x\":" POINT_PROP
    ",\"y\":" POINT_PROP "},\"required\":[\"x\",\"y\"],\"additionalProperties\":false}},"
    "{\"name\":\"drag\",\"description\":\"Drag from x,y to toX,toY (0..1), durationMs 100..2000 "
    "via USB HID. May have partial effects on interruption. Observe separately; do not "
    "replay.\",\"inputSchema\":{\"type\":\"object\",\"properties\":{\"x\":" POINT_PROP
    ",\"y\":" POINT_PROP ",\"toX\":" POINT_PROP ",\"toY\":" POINT_PROP
    ",\"durationMs\":{\"type\":\"integer\",\"minimum\":100,\"maximum\":2000}},\"required\":[\"x\","
    "\"y\",\"toX\",\"toY\",\"durationMs\"],\"additionalProperties\":false}},"
    "{\"name\":\"key\",\"description\":\"One US-layout HID key press/release. keycode 4..231; "
    "modifiers bit mask 0..255. Requires physical authorization. No application-success "
    "guarantee.\",\"inputSchema\":{\"type\":\"object\",\"properties\":{\"keycode\":{\"type\":"
    "\"integer\",\"minimum\":4,\"maximum\":231},\"modifiers\":{\"type\":\"integer\",\"minimum\":0,"
    "\"maximum\":255}},\"required\":[\"keycode\",\"modifiers\"],\"additionalProperties\":false}},"
    "{\"name\":\"type_ascii\",\"description\":\"Type 1..120 ASCII bytes using US HID keyboard "
    "layout, printable characters plus TAB/LF. No Unicode or IME. Interrupted typing may be "
    "partial; observe separately and never replay "
    "automatically.\",\"inputSchema\":{\"type\":\"object\",\"properties\":{\"text\":{\"type\":"
    "\"string\",\"minLength\":1,\"maxLength\":120}},\"required\":[\"text\"],"
    "\"additionalProperties\":false}},"
    "{\"name\":\"stop\",\"description\":\"Revoke physical MCP authorization and interrupt shared "
    "HID input. Does not undo previous effects. Device button confirmation is required before "
    "further MCP input.\",\"inputSchema\":" EMPTY_SCHEMA
    ",\"annotations\":{\"idempotentHint\":true}}]}";
