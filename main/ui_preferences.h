// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "ui.h"
ui_language ui_language_load(void);
bool ui_language_save(ui_language language);
input_mode_t ui_input_load(void);
bool ui_input_save(input_mode_t mode);
