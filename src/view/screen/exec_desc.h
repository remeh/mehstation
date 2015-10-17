#pragma once

#include "system/app.h"
#include "view/screen.h"

struct ExecutableListData;

void meh_exec_desc_create_widgets(App* app, Screen* screen);
void meh_exec_desc_destroy(Screen* screen);
void meh_exec_desc_adapt_view(App* app, Screen* screen);
void meh_exec_desc_update(Screen* screen);
void meh_exec_desc_render(App* app,Screen* screen);
