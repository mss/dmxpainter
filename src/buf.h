#pragma once

extern char gg_buf_gs[512];
extern char gg_buf_dc[3];

void buf_init(void);

void buf_next(void);
void buf_do(void);
