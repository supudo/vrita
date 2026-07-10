# DMG core: optimization / duplication / correctness findings

Findings-only document from a broad pass over the CPU, APU, and surrounding subsystems
(mmu/ppu/timer/joypad/interrupt/cartridge). No code was changed as part of this document.

## 1. High-priority correctness bugs — ALL FIXED

All items below have been applied. One deviated from its original suggestion after
verification: `memorySize` turned out to be read by the debugger UI (`emulators.cpp`), so
it was kept and synced after ROM resize instead of removed.

- **Pulse DAC-off doesn't immediately disable CH1/CH2.** `apu_registers.cpp` NR12/NR22 write
  handlers set `state.dacEnabled` but never clear `state.enabled` when the DAC turns off.
  Wave (NR30) and noise (NR42) both correctly do
  `if (!state.dacEnabled) state.enabled = false;` — pulse channels are missing this.

- **Register read-masking missing for write-only bits.** NR41 correctly returns
  `noise.NR41 | 0xFF` (open-bus read). The analogous registers don't get the same treatment:
  - NR11 / NR21 should OR `0x3F` (length bits are write-only).
  - NR13 / NR23 / NR31 / NR33 should read back as `0xFF` (fully write-only).
  Looks like an oversight rather than intentional — could cause test-ROM (e.g. blargg sound
  tests) register-read failures.

- **`envelope.volumeCurrent` sync inconsistency.** Pulse channels (NR12/NR22 write handlers)
  sync `volumeCurrent = volumeInitial` on every register write. Noise (NR42) only syncs it at
  trigger time. Pick one behavior deliberately — right now it's an unexplained asymmetry.

- **OAM DMA (`$FF46`) is entirely unimplemented.** No 160-byte transfer logic exists anywhere
  in `mmu.cpp`, `ppu.cpp`, or `dmg.cpp` — a write to `$FF46` just stores a byte and does
  nothing. `renderSprites` reads OAM directly from `mmu.memory[0xFE00...]`, but nothing ever
  copies WRAM/ROM data there via DMA, which is how virtually every commercial game refreshes
  sprite data every frame. Likely breaks sprite rendering in real games.

- **Timer duplicate address/interrupt constants.** `timer.hpp` declares member constants
  (`addressDIV`/`addressTIMA`/`addressTMA`/`addressTAC`/`addressTIMER_INTERRUPT`);
  `timer.cpp` re-declares an anonymous-namespace set with the same values
  (`DIV_ADDR`/`TIMA_ADDR`/`TMA_ADDR`/`TAC_ADDR`/`TIMER_INTERRUPT`). They currently agree, but
  it's two sources of truth for the same constants. `addressTIMER_INTERRUPT` and its
  duplicate `TIMER_INTERRUPT` are both unused — actual interrupt firing uses
  `INTERRUPT_TIMER` from `interrupt.hpp` instead.

- **`add(uint16_t*, int8_t)` latent bug.** In `cpu.cpp`, this function's carry/half-carry
  computation is hardcoded against `Registers.SP` rather than the generic `destination`
  parameter it's given. It works today only because its single call site
  (`cpu_normal.cpp`, opcode `0xE8 ADD SP,n`) always passes `&Registers.SP` — but the
  signature is misleading and a latent bug if it's ever called with a different destination.

- **`DMG_CPU` constructor has dead parameters.** `halted` and `cycles` are accepted but never
  used anywhere in the member-initializer list or body. The one call site
  (`dmg.cpp`) always passes `false, 0`. Actual halt state is tracked via `mmu.isHalted`.

- **Mislabeled debug logs in `cpu_normal.cpp` default case.** Two log lines print "DIV" and
  "cycles" in their format strings but both just print the opcode value — neither the actual
  DIV register nor actual elapsed cycles are logged. Misleading if anyone relies on this
  output.

- **Copy-paste typo** at `cpu_normal.cpp:466` (`LD L, L`): `logCall(true, "break LD L, L")` —
  the string should describe the opcode, not contain the literal word "break".

- **Dead commented-out code** in `cpu_normal.cpp` (~line 772-775): a commented-out
  `case 0xCB:` block with a note that it's "handled in cpu" — adds no value, just clutter.

- **`firstRAMWrite` never resets.** In `mmu.cpp`, this is a function-local `static bool`
  inside `write8`, so it's process-lifetime state — the "first external RAM write"
  diagnostic will never log again after a second ROM load in the same session.

- **`memorySize` is stale/dead.** Set once in `clearMemory()`, but `dmg.cpp` resizes
  `managerMMU->memory` directly on ROM load without updating it, and nothing else in the
  codebase reads it. Either remove it or keep it consistent.

- **Joypad's `setDpadState`/`setButtonsState` are a dormant version of the same "shadow
  state" bug class as the Timer/APU MMU-routing bugs.** `buttonsVal` is the real source of
  truth, updated by `setButton()`/`handleKey()`, which then derives `stateDPad`/
  `stateButtons` from it. But these two setters write `stateDPad`/`stateButtons` directly,
  bypassing `buttonsVal` — if they're ever wired up (e.g. gamepad support), the next
  keyboard event would silently recompute and wipe out that external input. Currently
  unreferenced anywhere, so it's dead code today, but a footgun if reactivated later.

## 2. Dedup / unification opportunities — ALL FIXED

- **APU trigger boilerplate.** The "DAC-off disables channel" check, length-counter
  reload-on-zero, and envelope reset-on-trigger are duplicated near-identically across
  `triggerPulse`/`triggerWave`/`triggerNoise`. Same shape of opportunity already exploited
  once for `clockEnvelope`/`clockLength` via templates.

- **APU frequency-timer reload formula** is computed inline twice each for pulse and wave
  (once at trigger time, once at timer-expiry time) instead of factored into a shared helper
  the way noise's `noiseTimerReload()` already is.

- **`isFlagSet()` vs `getFlag()`** in `cpu.hpp` — functionally identical, just two names,
  with 29 call sites split across them for no reason.

- **`rl()`/`rlc()` and `rr()`/`rrc()`** in `cpu.cpp` — identical bodies except carry-in
  source (carry flag vs the value's own top/bottom bit). Each pair could collapse into one
  parameterized function.

- **`cp()` vs `cp_n()`** in `cpu.cpp` — confirmed functionally identical duplicate (same
  Z/C/H/N results, computed via different intermediate steps).

- **`add(uint16_t*, int8_t)` vs `ldhl()`** in `cpu.cpp` — same SP+signed-immediate algorithm;
  could unify into one function (also resolves the latent bug noted above).

- **Read-modify-write `(HL)` triad** (`read8` → op → `write8`) repeats ~24 times in
  `cpu_extended.cpp` (RLC/RRC/RL/RR/SLA/SRA/SWAP/SRL/RES/SET on `(HL)`) plus twice more in
  `cpu_normal.cpp` (`INC (HL)`/`DEC (HL)`) — candidate for a small `modifyHL(fn)` helper.

- **PPU `renderBackground()`/`renderWindow()`** share near-identical tile-index lookup,
  signed/unsigned tile addressing, bitplane fetch, and colorId extraction — candidate for a
  shared helper to prevent the two paths from drifting apart under future fixes.

## 3. Dead code — ALL FIXED (both fields removed entirely, confirmed zero readers)

- **`ChannelState::justTriggered`** (`apu_structs.hpp`) — written for wave/noise triggers,
  never read anywhere in the codebase. Also inconsistently *not* set by `triggerPulse`.

- **`PulseChannel::sweepCalculatedNegate`** (`apu_structs.hpp`) — written in `triggerPulse`,
  never read anywhere.

## 4. Bigger, optional rewrites (not recommended to do now — shown as options only)

- **`cpu_extended.cpp`'s entire 256-case CB-prefixed switch** (RLC/RRC/RL/RR/SLA/SRA/SWAP/SRL
  × 8 registers, plus BIT/RES/SET × 8 bits × 8 registers) is mechanically derivable from the
  opcode bits (`op = opcode >> 6`, `sub/bit = (opcode >> 3) & 7`, `reg = opcode & 7`, plus a
  register-pointer table). Could collapse ~870 lines to a fraction of that — at the cost of
  rewriting code that is now correct and has been validated against real games.

- **`cpu_normal.cpp`'s `LD r,r'` block** (opcodes `0x40`-`0x7F` minus `0x76`, ~63 cases,
  several of which are no-op self-assignments with empty bodies) is the same kind of
  mechanical table-driven dispatch candidate.

## 5. Minor / lower-priority perf notes — ALL FIXED

- **Diagnostic `logger.log(...)` calls** removed entirely from `apu_registers.cpp` now that
  the bug hunt is done. The APU no longer needs a `Logger` dependency at all — removed the
  `Logger&` constructor parameter and member from `DMG_APU` too.

- **`mixSample()`** now skips `channelOutput(i)` entirely for any channel routed to neither
  left nor right (`if (!enabledLeft[i] && !enabledRight[i]) continue;`).

- **Resampling accumulator** (`AudioOutput::sampleAccumulator`) converted from `double` to
  `uint32_t` fixed-point — `sampleRate`/`cpuClock` are both exact integers, so this is a
  strictly equivalent, faster per-T-cycle add+compare.

- **Cartridge/MBC raw pointer**: added a comment at the capture site (`cartridge.cpp`
  `loadROM()`) documenting the call-order invariant it depends on, so a future reorder of
  `DMG::clear()`/`loadROM()` doesn't silently invalidate it.

- **Cartridge/MBC holds a raw pointer into `mmu.memory.data()`** captured at ROM-load time.
  Safe under the current call order in `dmg.cpp` (`clearResources()` always runs before a new
  `DMG_MBC` is constructed), but fragile if that order ever changes — worth a defensive
  comment or an index-based redesign instead of a raw pointer.

---

**Explicitly out of scope for this pass** (per explicit decision): serial port/interrupt
implementation, Echo RAM ($E000-$FDFF) mirroring.
