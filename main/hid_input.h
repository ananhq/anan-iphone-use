// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "actions.h"
#include "cJSON.h"

bool hid_input_is_tool(const char *name);
bool hid_input_parse(const char *name, const cJSON *args, action *out);
const char *hid_input_unit(action_kind kind);
unsigned hid_input_total(const action *a);
