STREET MAYHEM 2.0 — PSP SOURCE
==============================

This is a corrected graphics-initialization revision of the original prototype.
It uses the conventional PSP GU framebuffer layout:
- Draw buffer: VRAM offset 0
- Display buffer: VRAM offset 0x88000
- Depth buffer: VRAM offset 0x110000
- 512-pixel buffer stride

GAME FEATURES
- Procedural 3D city
- Player car and third-person chase camera
- Eight moving traffic/rival bots
- Simple chase and roaming behavior
- Collision scoring
- Title screen and pause state
- PSP / PPSSPP controls

CONTROLS
D-pad UP: accelerate
D-pad DOWN: brake / reverse
D-pad LEFT/RIGHT: steer
SELECT: pause
X or START: start / resume
CIRCLE: return to title while paused

BUILD
Requires PSPSDK:
  make

Copy generated EBOOT.PBP to:
  /PSP/GAME/STREETMAYHEM/EBOOT.PBP

IMPORTANT
This ZIP contains source code, not a prebuilt EBOOT.PBP. It has not been
compiled or executed in a PSP/PPSSPP runtime here, so treat it as a source
revision to test. It is an original arcade-driving prototype, not the
Payback 2 game or its assets.
