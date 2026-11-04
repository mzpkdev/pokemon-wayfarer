# Gameplay clips

Short in-game clips the video shows in a GBA frame beside the map, one folder
per clip named as `insert.clip` in `src/timeline.ts`:

    clips/<clip>/manifest.json   { "fps": 60, "frames": ["0001.png", "0002.png", ...] }
    clips/<clip>/0001.png        240x160 screenshots

Until a clip's folder exists the video shows a placeholder frame with the clip's
name, so the storyboard can be timed and rendered before any footage is made.

The plan is to record them from the E2E ROM with the `e2e/` harness: SkyEmu's
HTTP server steps the game one frame at a time and returns a PNG of each
(`e2e/src/harness/skyemu/client.ts`), so a playbook that boots a fresh game,
answers Oak and walks to the S.S. Aqua's gangway gives the same clip every time.
