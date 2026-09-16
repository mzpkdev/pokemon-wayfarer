// Capture real GBA frames and verify title input with the repo's patched SkyEmu.
import { spawn, execFileSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { copyFile, mkdir, mkdtemp, readFile, writeFile } from 'node:fs/promises';
import { createServer } from 'node:net';
import { tmpdir } from 'node:os';
import { resolve, join } from 'node:path';
import { parseArgs } from 'node:util';

const { values } = parseArgs({ options: {
  emulator: { type: 'string' }, rom: { type: 'string' }, elf: { type: 'string' },
  output: { type: 'string' },
} });
for (const key of ['emulator', 'rom', 'elf', 'output'])
  if (!values[key]) throw new Error(`Required: --${key}`);
const output = resolve(values.output);
await mkdir(output, { recursive: true });
const temporary = await mkdtemp(join(tmpdir(), 'wayfarer-title-capture-'));
const rom = join(temporary, 'title.gba');
await copyFile(resolve(values.rom), rom);
const symbols = new Map(execFileSync('arm-none-eabi-nm', [resolve(values.elf)], { encoding: 'utf8', maxBuffer: 32 * 1024 * 1024 })
  .split('\n').flatMap(line => {
    const match = line.match(/^([0-9a-f]+)\s+\w\s+(\S+)$/i);
    return match ? [[match[2], parseInt(match[1], 16)]] : [];
  }));
function symbol(name) {
  if (!symbols.has(name)) {
    const ltoName = [...symbols.keys()].find(candidate => candidate === name || candidate.startsWith(`${name}.lto_priv.`));
    if (ltoName) return symbols.get(ltoName);
  }
  if (!symbols.has(name)) throw new Error(`ELF lacks ${name}`);
  return symbols.get(name);
}
const port = await new Promise((done, reject) => {
  const server = createServer();
  server.on('error', reject);
  server.listen(0, '127.0.0.1', () => {
    const selected = server.address().port;
    server.close(() => done(selected));
  });
});
const base = `http://127.0.0.1:${port}`;
const child = spawn('xvfb-run', ['--auto-servernum', resolve(values.emulator), 'http_server', String(port), rom], {
  detached: true, env: { ...process.env, XDG_DATA_HOME: temporary }, stdio: 'ignore',
});
let launchError;
child.on('error', error => { launchError = error; });
async function request(path) {
  const response = await fetch(`${base}/${path}`, { signal: AbortSignal.timeout(30000) });
  if (!response.ok) throw new Error(`${path}: HTTP ${response.status}`);
  return response;
}
async function command(path) {
  const result = (await (await request(path)).text()).replaceAll('\0', '');
  if (result !== 'ok') throw new Error(`${path}: ${result}`);
}
async function read(address, size) {
  const query = new URLSearchParams();
  for (let i = 0; i < size; i++) query.append('addr', (address + i).toString(16));
  const hex = (await (await request(`read_byte?${query}`)).text()).replaceAll('\0', '');
  if (!new RegExp(`^[a-f0-9]{${size * 2}}$`, 'i').test(hex)) throw new Error('Invalid memory response');
  return Buffer.from(hex, 'hex');
}
const step = frames => command(`step?frames=${frames}`);
async function taskData(name) {
  if (![...symbols.keys()].some(candidate => candidate === name || candidate.startsWith(`${name}.lto_priv.`))) return null;
  const expected = symbol(name) & ~1;
  const tasks = Buffer.alloc(640);
  for (let offset = 0; offset < 640; offset += 160)
    (await read(symbol('gTasks') + offset, 160)).copy(tasks, offset);
  for (let offset = 0; offset < 640; offset += 40)
    if (tasks[offset + 4] && (tasks.readUInt32LE(offset) & ~1) === expected)
      return Array.from({ length: 16 }, (_, index) => tasks.readInt16LE(offset + 8 + index * 2));
  return null;
}
async function hasTask(name) {
  return (await taskData(name)) !== null;
}
async function taskSlot(name) {
  const expected = symbol(name) & ~1;
  const tasks = Buffer.alloc(640);
  for (let offset = 0; offset < 640; offset += 160)
    (await read(symbol('gTasks') + offset, 160)).copy(tasks, offset);
  for (let offset = 0; offset < 640; offset += 40)
    if (tasks[offset + 4] && (tasks.readUInt32LE(offset) & ~1) === expected) return offset / 40;
  throw new Error(`Task not active: ${name}`);
}
async function passingState(slot) {
  const data = await read(symbol('gTasks') + slot * 40 + 8, 16);
  return { wait: data.readInt16LE(0), species: data.readInt16LE(2),
    spriteId: data.readInt16LE(4), bicycleId: data.readInt16LE(6),
    torchicState: data.readInt16LE(10), direction: data.readInt16LE(14) };
}
async function spriteX(spriteId) {
  return (await read(symbol('gSprites') + spriteId * 68 + 0x20, 2)).readInt16LE();
}
async function waitTask(name, limit = 12000, interval = 10) {
  for (let frames = 0; frames < limit; frames += interval) {
    if (await hasTask(name)) return;
    await step(interval);
  }
  throw new Error(`Did not reach ${name} within ${limit} frames`);
}
async function press(button) {
  await command(`input?${button}=1`);
  await step(2);
  await command(`input?${button}=0`);
  await step(2);
}
const captures = [];
async function capture(name) {
  const bytes = Buffer.from(await (await request('screen')).arrayBuffer());
  if (bytes.subarray(0, 8).toString('hex') !== '89504e470d0a1a0a') throw new Error('Not a PNG');
  if (bytes.readUInt32BE(16) !== 240 || bytes.readUInt32BE(20) !== 160) throw new Error('Not a native GBA frame');
  await writeFile(join(output, `${name}.png`), bytes);
  captures.push({ name, sha256: createHash('sha256').update(bytes).digest('hex'),
    displayRegisters: (await read(0x04000000, 16)).toString('hex'),
    introFrameCounter: symbols.has('gIntroFrameCounter')
      ? (await read(symbol('gIntroFrameCounter'), 4)).readInt32LE() : null });
}
async function reload() {
  await command(`load_rom?${new URLSearchParams({ path: rom, pause: '1' })}`);
  await command('input?A=0&B=0&Down=0&L=0&Left=0&R=0&Right=0&Select=0&Start=0&Up=0');
}
try {
  for (let attempt = 0; ; attempt++) {
    if (launchError) throw launchError;
    try { if ((await (await request('ping')).text()).replaceAll('\0', '') === 'pong') break; } catch {}
    if (attempt >= 120) throw new Error('SkyEmu did not start');
    await new Promise(done => setTimeout(done, 250));
  }
  await reload();
  await waitTask('Task_Scene1_WaterDrops');
  await step(150);
  await capture('game-freak');
  await waitTask('Task_Scene1_PanUp');
  await step(320);
  await capture('mountain-pan');
  await waitTask('Task_WayfarerTitleReveal');
  await capture('mountain-hold');
  await waitTask('Task_WayfarerTitleFinishReveal', 300, 1);
  await capture('title-logo-rise-start');
  await step(6);
  await capture('title-logo-rise-mid');
  await step(6);
  await capture('title-wayfarer-rise');
  if (await hasTask('Task_WayfarerTitleInput')) throw new Error('Title input appeared before wordmarks settled');
  await waitTask('Task_WayfarerTitleInput', 300, 1);
  await step(4);
  await capture('title-overlays');
  if (((await read(0x04000000, 2)).readUInt16LE() & 0x6000) !== 0x6000)
    throw new Error('Logo shine did not enable both windows');
  await step(18);
  await capture('title-shine-left');
  await step(12);
  await capture('title-shine-center');
  await step(12);
  await capture('title-shine-right');
  await step(32);
  await capture('title-shine-finished');
  const shineRegisters = {
    dispcnt: (await read(0x04000000, 2)).readUInt16LE(),
    bldcnt: (await read(0x04000050, 2)).readUInt16LE(),
  };
  // SkyEmu exposes BLDY as write-only (reads return 0xffff), so check the
  // window and blend controller and inspect the post-sweep frame instead.
  if (shineRegisters.dispcnt & 0x6000 || shineRegisters.bldcnt !== 0)
    throw new Error(`Logo shine did not restore display and blend state: ${JSON.stringify(shineRegisters)}`);
  await step(16);
  await capture('title-blink');
  const passSlot = await taskSlot('Task_WayfarerPokemonPass');
  const seenPairs = new Set();
  const passOrder = [];
  const passDirections = [];
  const waits = [];
  const transitionChoices = new Set();
  const waitChoices = new Set();
  let previousSpecies = null;
  let wasLoaded = false;
  const torchicFallDirections = new Set();
  const torchicGetUpDirections = new Set();
  let previousX = null;
  for (let elapsed = 0; elapsed < 100000 &&
       (seenPairs.size < 8 || torchicFallDirections.size < 2 || torchicGetUpDirections.size < 2 || passOrder.length < 10 ||
        transitionChoices.size < 2 || waitChoices.size < 2); elapsed += 10) {
    const state = await passingState(passSlot);
    const loaded = state.spriteId < 64;
    const x = loaded ? await spriteX(state.spriteId) : null;
    if (state.direction !== -1 && state.direction !== 1)
      throw new Error(`Invalid passing direction: ${state.direction}`);
    if (loaded && !wasLoaded) {
      if (previousSpecies !== null) {
        const transition = (state.species - previousSpecies + 4) % 4;
        if (transition === 0) throw new Error('Passing character repeated immediately');
        transitionChoices.add(transition);
      }
      previousSpecies = state.species;
      passOrder.push(state.species);
      passDirections.push(state.direction);
    }
    if (loaded && wasLoaded && (x - previousX) * state.direction < 0)
      throw new Error(`Pass ${state.species} moved against its direction: ${previousX} → ${x}`);
    if (loaded && state.species === 2 &&
        (state.bicycleId >= 64 || await spriteX(state.bicycleId) !== x))
      throw new Error('Bicycle and rider did not stay aligned');
    if (!loaded && wasLoaded) {
      // Polling can happen up to nine frames after the wait was assigned.
      if (state.wait < 890 || state.wait > 1500)
        throw new Error(`Passing interval outside 15–25 seconds: ${state.wait}`);
      if (state.bicycleId < 64) throw new Error('Bicycle sprite leaked after rider exit');
      waits.push(state.wait);
      waitChoices.add(state.wait);
    }
    const pair = `${state.species}:${state.direction}`;
    if (loaded && !seenPairs.has(pair)) {
      if (x >= 40 && x <= 180) {
        const names = ['volbeat', 'torchic', 'bicyclist', 'manectric'];
        await capture(`title-${names[state.species]}-${state.direction > 0 ? 'right' : 'left'}`);
        seenPairs.add(pair);
        if (state.species === 0) {
          await step(8);
          await capture(`title-volbeat-${state.direction > 0 ? 'right' : 'left'}-zig-8`);
          await step(8);
          await capture(`title-volbeat-${state.direction > 0 ? 'right' : 'left'}-zig-16`);
        }
      }
    }
    if (loaded && state.species === 1 && state.torchicState === 2 && !torchicFallDirections.has(state.direction)) {
      await capture(`title-torchic-${state.direction > 0 ? 'right' : 'left'}-fall`);
      torchicFallDirections.add(state.direction);
    }
    if (loaded && state.species === 1 && state.torchicState === 3 && !torchicGetUpDirections.has(state.direction)) {
      await capture(`title-torchic-${state.direction > 0 ? 'right' : 'left'}-get-up`);
      torchicGetUpDirections.add(state.direction);
    }
    wasLoaded = loaded;
    previousX = x;
    await step(10);
  }
  if (seenPairs.size !== 8 || torchicFallDirections.size !== 2 || torchicGetUpDirections.size !== 2 || passOrder.length < 10 ||
      transitionChoices.size < 2 || waitChoices.size < 2)
    throw new Error(`Incomplete random pass coverage: pairs=${[...seenPairs]}, order=${passOrder}, waits=${waits}, trip=${[...torchicFallDirections]}/${[...torchicGetUpDirections]}`);
  await capture('title-held');
  await press('Start');
  await step(180);
  if (await hasTask('Task_WayfarerTitleInput')) throw new Error('Start did not leave title');
  await capture('after-start');

  for (const [label, delay] of [['early', 90], ['mid', 480], ['late', 800]]) {
    await reload();
    await waitTask('Task_Scene1_WaterDrops');
    if (delay) await step(delay);
    await press('Start');
    await waitTask('Task_WayfarerTitleInput', 300);
    await step(4);
    await capture(`title-skipped-${label}`);
    if (label === 'early') {
      await press('Start');
      if ((await read(0x04000000, 2)).readUInt16LE() & 0x6000 ||
          (await read(0x04000050, 2)).readUInt16LE() !== 0)
        throw new Error('Leaving during logo shine kept window or blend state enabled');
    }
  }
  // Remain longer than the title loop's audible opening. It may restart music,
  // but it must remain in the held title task and never reload the cinematic.
  await step(7200);
  if (!(await hasTask('Task_WayfarerTitleInput'))) throw new Error('Idle title left the held composition');
  if (await hasTask('Task_Scene2_Load')) throw new Error('Bike scene was reached');
  await capture('title-long-idle');
  await writeFile(join(output, 'capture.json'), JSON.stringify({
    rom: resolve(values.rom), elf: resolve(values.elf),
    romSha256: createHash('sha256').update(await readFile(rom)).digest('hex'), captures,
    passOrder, passDirections, observedWaitFrames: waits,
    verified: [
      'Game Freak sequence', 'Scene 1 mountain pan and hold', 'staggered wordmark rise followed by one-off logo shine, held overlays, all four passers in both directions, Torchic trip/recovery from both sides, random non-repeating order and 15–25-second gaps, and blink',
      'Start exits title', 'early/mid/late skips reach held title', 'long idle stays held',
      'bike scene task was not reached',
    ],
  }, null, 2) + '\n');
  console.log(`Captured and verified title screen in ${output}`);
} finally {
  if (child.pid) {
    try { process.kill(-child.pid, 'SIGTERM'); } catch (error) { if (error.code !== 'ESRCH') throw error; }
  }
}
