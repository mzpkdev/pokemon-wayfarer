Ordinary mGBA GUI compatibility probe, 2026-09-30

Binary: Ubuntu noble mgba-qt 0.10.2+dfsg-1.1build3, unmodified distribution package; --version reports 0.10.2 ((unknown)). All packages downloaded with apt download and extracted with dpkg-deb -x into ./root. No system installation or repository modifications. See manifest.json for package list and binary/ROM SHA256.

ROM: isolated copy of ../baseline-artifacts/pokemon-wayfarer-e2e-poc.gba (phase1 presence PoC, not phase2 battle experiment). Saves: copied ../fixture-valid-save-long/player0.sav to phase1.sav, player1.sav to phase1-peer.sav. Original files untouched.

Commands and environment: probe.py launches Xvfb :88 -screen 0 1280x800x24, then ./root/usr/games/mgba-qt ./phase1.gba with env.json plus QT_QPA_PLATFORM=xcb. LD_LIBRARY_PATH and QT plugin paths point into extracted root; XDG configuration/data isolated here; SDL_AUDIODRIVER=dummy. xdotool performs ordinary GUI keyboard/mouse actions. xwd captures X server pixels; probe.py converts those to PNG without changing pixels. No emulator memory injection, scripting bridge, or custom core runner was used.

Verified observations:
- GUI booted the ROM intro (gui-first.png).
- Native File > New multiplayer window opened a second window; loading phase1-peer.gba produced titles 'mGBA - WAYFARER - Player 1 of 2 (45.4 fps) - 0.10.2' and 'mGBA - WAYFARER - Player 2 of 2 - 0.10.2'.
- First core reached Continue with copied save. Second core remained white; its native F12 screenshot phase1-peer-0.png was also white. No second core FPS appeared. GUI Pause was unchecked (gui-emulation-menu.png); toggling pause did not establish gameplay.
- Shutting down only second core via Emulation > Shutdown let first continue normally into New Bark Town (gui-solo-continue.png). Directional keyboard input moved the player (gui-solo-moved.png).
- BWFE RTC override was NOT added; first GUI showed the ordinary dry-battery notice, dismissible by A. This probe does not validate RTC correctness.

LIMIT: ordinary single-player GUI boot/Continue/input verified, native two-window multiplayer setup opened, but native GUI multiplayer presence was NOT established. The second-core white screen is an unresolved observation, not proof of a ROM defect or emulator incompatibility. No phase2 battle, network-latency, phone emulator, or real hardware test. Probe stopped after bounded attempt; processes terminated. Native libmgba 0.10.5 harness results are separate evidence, not GUI evidence.

Control notes: default keyboard A=x, B=z, L=a, R=s, START=Return, SELECT=BackSpace. Hold game keys ~0.3s rather than xdotool's short key pulse. New multiplayer window has no default shortcut and was selected through File menu.
