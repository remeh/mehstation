#pragma once

#include "system/app.h"
#include "view/screen.h"

struct ExecutableListData;

void meh_exec_selection_prepare(App* app, Screen* screen, struct ExecutableListData* data);
void meh_exec_selection_destroy(Screen* screen, struct ExecutableListData* data);
void meh_exec_selection_render(App* app, Screen* screen, struct ExecutableListData* data);
