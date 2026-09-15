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
async function hasTask(name) {
  const expected = symbol(name) & ~1;
  const tasks = Buffer.alloc(640);
  for (let offset = 0; offset < 640; offset += 160)
    (await read(symbol('gTasks') + offset, 160)).copy(tasks, offset);
  for (let offset = 0; offset < 640; offset += 40)
    if (tasks[offset + 4] && (tasks.readUInt32LE(offset) & ~1) === expected) return true;
  return false;
}
async function waitTask(name, limit = 12000) {
  for (let frames = 0; frames < limit; frames += 10) {
    if (await hasTask(name)) return;
    await step(10);
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
    displayRegisters: (await read(0x04000000, 16)).toString('hex') });
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
  await waitTask('Task_TitleScreenPhase1');
  await capture('intro-logo');
  await waitTask('Task_TitleScreenPhase2');
  await capture('intro-banner');
  await waitTask('Task_TitleScreenPhase3');
  await step(4);
  await capture('title');
  await step(16);
  await capture('title-blink');
  await step(120);
  await capture('title-held');
  await press('Start');
  await step(180);
  if (await hasTask('Task_TitleScreenPhase3')) throw new Error('Start did not leave title');
  await capture('after-start');
  await reload();
  await waitTask('Task_TitleScreenPhase1');
  // The task is allocated before the initial palette fade finishes and input runs.
  await step(90);
  await press('Start');
  await waitTask('Task_TitleScreenPhase3', 300);
  await step(20);
  await capture('title-skipped');
  await writeFile(join(output, 'capture.json'), JSON.stringify({
    rom: resolve(values.rom), elf: resolve(values.elf),
    romSha256: createHash('sha256').update(await readFile(rom)).digest('hex'), captures,
    verified: ['natural intro', 'settled title and blink', 'Start exits title', 'skipped intro reaches title'],
  }, null, 2) + '\n');
  console.log(`Captured and verified title screen in ${output}`);
} finally {
  if (child.pid) {
    try { process.kill(-child.pid, 'SIGTERM'); } catch (error) { if (error.code !== 'ESRCH') throw error; }
  }
}
