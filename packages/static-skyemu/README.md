# @wayfarer/static-skyemu

Internal package that downloads and exposes the pinned SkyEmu v5 Linux x64
binary used by the E2E suite.

```sh
pnpm --filter @wayfarer/static-skyemu run setup
```

The setup script checks the official release archive against its published
SHA-256 before extracting `vendor/SkyEmu`. Import `skyEmuBinary` to get that
path, or set `STATIC_SKYEMU_DIR` to store the binary elsewhere.

The package needs Linux x64 and `unzip`. Other platforms can provide their own
emulator through the E2E suite's `SKYEMU_BIN` variable.
