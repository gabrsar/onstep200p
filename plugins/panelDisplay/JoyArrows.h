#pragma once
#include <stdint.h>
#include <stdlib.h>

// 9x9 glyph table: right and northeast, thin/medium/filled. Rotations give
// → ↗ ↑ ↖ ← ↙ ↓ ↘ without assuming UTF-8 support in the ASCII text renderer.
inline void joyArrow(int x,int y,uint16_t rows[9],int deadZone=450) {
  for (int i=0;i<9;++i) rows[i]=0;
  const int ax=abs(x),ay=abs(y),major=ax>ay?ax:ay,minor=ax>ay?ay:ax;
  if (major<=deadZone) { rows[3]=rows[5]=0x38; rows[4]=0x28; return; }
  static const uint16_t glyphs[2][3][9]={
    {{0,0x20,0x40,0x80,0x1fe,0x80,0x40,0x20,0},
     {0,0x20,0x60,0xfe,0x1fe,0xfe,0x60,0x20,0},
     {0x10,0x30,0x70,0xff,0x1ff,0xff,0x70,0x30,0x10}},
    {{0x1f0,0x180,0x140,0x120,0x110,0x8,0x4,0x2,0},
     {0x1f0,0x1e0,0x1e0,0x170,0x138,0x1c,0xe,0x6,0},
     {0x1f0,0x1f0,0x1f0,0x1f8,0x17c,0x3e,0x1e,0xe,0x4}}
  };
  const bool diagonal=minor*2>=major;
  const int level=major>=1400?2:major>=700?1:0;
  const int turns=diagonal?(y>0?(x>0?0:1):(x<0?2:3)):
    (ax>=ay?(x>0?0:2):(y>0?1:3));
  for (int row=0;row<9;++row) for (int col=0;col<9;++col)
    if (glyphs[diagonal][level][row] & (1u<<col)) {
      int px=col,py=row;
      for (int i=0;i<turns;++i) { const int old=px; px=py; py=8-old; }
      rows[py]|=1u<<px;
    }
}
