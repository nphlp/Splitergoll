# Splitergoll keyboard

Welcome to the Splitergoll Project: a column staggered split keyboard working with the ZMK Firmware.

- `split` : A split keyboard, one part for each hand
- `ergo` : An ergonomic keyboard with a column staggered layout
- `ll` : Low latency, requires a dongle to improve battery life and Bluetooth latency

In this repository, you will find:

- The `electronic PCB design` files created with KiCad
- The `3D printable case design` files designed using Fusion 360
- The complete [ZMK firmware](https://github.com/zmkfirmware/zmk) configuration for the **Splitergoll**.

<div>
    <img src="/Images/IMG_7786.webp"/>
    <div style="display: flex; flex-direction: row; justify-content: space-between;">
        <img src="/Images/IMG_7666.webp" width="49.5%"/>
        <img src="/Images/IMG_7669.webp" width="49.5%"/> 
    </div>
    <div style="display: flex; flex-direction: row; justify-content: space-between;">
        <img src="/Images/IMG_7670.webp" width="49.5%"/>
        <img src="/Images/IMG_7671.webp" width="49.5%"/> 
    </div>
    <img src="/Images/IMG_7981.webp"/>
</div>

## Licenses and Inspirations

### Bluetooth wireless keyboards

Huge thanks to these guys for their beautiful documentations about ZMK Wireless keyboards:

[KLOR Documentation](https://github.com/GEIGEIGEIST/KLOR) an INCREDIBLE documentation by GEIGEIGEIST:

- Polydactyl keyboard layout
- PCB design files
- Electronic component references
- Build guide

[Aenyhm's ZMK config](https://github.com/Aenyhm/zmk-config) for:

- French key layout correspondences (Q → A, W → Z...)
- System key correspondences (Windows, MacOS...)
- Layer system for "Windows" and "MacOS" compatibility

[Eigateck's ZMK config](https://github.com/eigatech/zmk-config) for:
- His incredible [Youtube channel](https://www.youtube.com/@EIGAtech)
- The nice documentation of its keyboards

Other great resources:

- [temper](https://github.com/raeedcho/temper) by raeedcho
- [nine-nano](https://github.com/bsag/nine-nano) by bsag
- [urchin](https://github.com/duckyb/urchin) by duckyb
- [LambBT](https://github.com/johnlamb/LambBT) by johnlamb
- [Rolio46Keyboard](https://github.com/MickiusMousius/Rolio46Keyboard) by MickiusMousius
- [SofleChocWireless](https://github.com/db-ok/SofleChocWireless) by db-ok
- [KLOTZ](https://github.com/GEIGEIGEIST/KLOTZ) by GEIGEIGEIST
- [pica40](https://github.com/zzeneg/pica40) by zzeneg
- [karn-keyboard](https://github.com/robcmills/karn-keyboard) by robcmills
- [SofleKeyboard](https://github.com/josefadamcik/SofleKeyboard) by josefadamcik

### Dongle wireless keyboards

Incredible resources about ZMK keyboards dongles:

- [Aroum's Dongle Keyboard](https://github.com/aroum/zmk-enki42-dongle) based on a `SuperMini nRF52840` as a dongle.
- [Charlesmst's Dongle Keyboard](https://github.com/charlesmst/zmk-enki42-dongle) based on a `Makerdiary nRF52840 MDK Dongle` as a dongle.
- [RasmusKoit's Dongle Keyboard](https://github.com/RasmusKoit/pipar) based on a `Seeed Studio Xiao nRF52840` as a dongle.

Eigateck's ZMK config:

- [Totem Dongle](https://github.com/eigatech/zmk-config/tree/totem-dongle)
- [Corne Dongle](https://github.com/eigatech/zmk-config/tree/corne-dongle)
- [Charybdis Dongle](https://github.com/eigatech/zmk-config/tree/charybdis-dongle)

Documentation about the dongle keyboards:

- [SliceMK Docs](https://docs.slicemk.com/firmware/zmk/wireless/dongle/): a ZMK fork which inspired Aroum's Dongle Keyboard.
- [PR#2401 Dongle Keyboard](https://github.com/zmkfirmware/zmk/pull/2401): a proposition for a dongle keyboard documentation [see preview](https://deploy-preview-2401--zmk.netlify.app/docs/features/dongle).

Hardware resources about the dongle keyboards:

- [Seeed Studio Xiao BLE](https://github.com/zmkfirmware/zmk/blob/main/app/boards/arm/seeeduino_xiao_ble/seeeduino_xiao_ble.zmk.yml)
- [Nice!Nano V2](https://github.com/zmkfirmware/zmk/blob/main/app/boards/arm/nice_nano/nice_nano_v2.yaml) (or SuperMini nRF52840) for each keyboard halve.
- [Makerdiary nRF52840 Dongle](https://github.com/zmkfirmware/zephyr/blob/v3.5.0%2Bzmk-fixes/boards/arm/nrf52840_mdk_usb_dongle/nrf52840_mdk_usb_dongle.yaml)
- [Nordic nRF52840 Dongle](https://github.com/zmkfirmware/zmk/blob/main/app/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.zmk.yml) as a dongle.

## Git Branches

Classique version:
- [main](https://github.com/nansphilip/Splitergoll/tree/main) → Bluetooth wireless keyboard
- [dongle](https://github.com/nansphilip/Splitergoll/tree/dongle) → Dongle wireless keyboard

Minimalist version (to learn ZMK):
- [main (minimalist)](https://github.com/nansphilip/Splitergoll/tree/main-minimal) → Minimalist bluetooth wireless keyboard
- [dongle (minimalist)](https://github.com/nansphilip/Splitergoll/tree/dongle-minimal) → Minimalist dongle wireless keyboard

## Todo

Works:
- [x] Global configuration: Bluetooth and USB
- [x] Switches and encoders pin connections
- [x] Battery level and sleep mode
- [x] Layer inheritance and conditions
- [x] Accents, symbols, movements
- [x] Gaming compatibility
- [x] Release default shift morph behavior
- [x] Dongle mode (needs improvements...)
- [x] French layout (make sure you are using `Français (variante)` for Linux)
- [x] Windows compatibility
- [x] Linux compatibility (needs improvements...)

Testing:
- [ ] MacOS compatibility (work in progress...)

Incoming:
- [ ] Switch Bluetooth device
- [ ] Flying macro (record a macro, replay it, and repeat it)
- [ ] Sleep mode ?
- [ ] Bluetooth power ?
- [ ] Battery reporting ?
- [ ] Debouncing ?

## Improvements

- Hotswapable switches for an easier maintenance
- Led indicators for connection status (bluetooth, usb...) and battery level
- Keycap legends for each keys for a professional look

## Layout

This keyboard is compatible with `Windows` (default) and `MacOS` (soon).
It is optimized for `desktop`, `programming`, `gaming`, and `multimedia`.

Some shortcuts and modifiers may be different:

- `CTRL` key is replaced by the `command` key
- `ALT` key is replaced by the `option` key
- `WINDOWS` key is replaced by the `control` key

This keyboard works with 4 layers:

- `DEFAULT`: letters and most used punctuation
- `PRIMARY`: text editor movements, desktop management, and all programming symbols
- `SECONDARY`: accented letters, calculation symbols, and numbers
- `TERTIARY`: miscellaneous shortcuts, macros, etc.

During your reading, you will find these symbols:

- `+` means [tap dance](https://zmk.dev/docs/behaviors/tap-dance)
- `→` means [hold tap](https://zmk.dev/docs/behaviors/hold-tap)
- `or` means [shift morph tap](https://zmk.dev/docs/behaviors/mod-morph)

### Default layer

> Philosophy

The default layer contains all letters and the most frequent punctuation symbols. This main layer can be shifted to access uppercase letters and shift-morphed symbols.

The modifiers `SHIFT` key and `NUMPAD` layer can be locked (toggle → press modifier) by tap dance patterns (follow the `Lock usage` section below).
You can easily unlock them (toggle → release modifier) by a single tap on the respective keys.

> Shortcuts

#### Basic usage

- `SHIFT` to shift layer
- `L1` to layer 1
- `L2` to layer 2

#### Combo usage

- `L1` + `L2` to layer 3

#### Alternative usage

- `L1` + `L1` to layer 2
- `L2` + `L2` to layer 1

#### Lock usage

- `SHIFT` + `SHIFT` to LOCK shift layer
- `SHIFT` to UNLOCK shift layer

- `L1` + `L1` + `L1` to LOCK numpad layer
- `L1` to UNLOCK numpad layer

```keymap
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃    ESC    ┃     A     ┃     Z     ┃     E     ┃     R     ┃     T     ┃   ┃     Y     ┃     U     ┃     I     ┃     O     ┃     P     ┃   BSPACE  ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃    TAB    ┃     Q     ┃     S     ┃     D     ┃     F     ┃     G     ┃   ┃     H     ┃     J     ┃     K     ┃     L     ┃     M     ┃    DEL    ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃     W     ┃     X     ┃     C     ┃     V     ┃     B     ┃   ┃     N     ┃  , or ?   ┃  . or !   ┃  ; or /   ┃  : or \   ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│ VOL+  │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │ LEFT  │
│ VOL-  │ ┃ Play→Mute ┃ ┃   CTRL    ┃    ALT    ┃     L2    ┃   SPACE   ┃   ┃SHIFT+LOCK ┃     L1    ┃   ENTER   ┃    WIN    ┃ ┃ To Linux  ┃ │ RIGHT │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

### Primary layer

> Philosophy

The first layer is optimized for programming: directional arrows, word jump shortcuts, home and end line, and every symbol.

> Shortcuts

- `L1` + `SHIFT²` + `←, ↑, ↓ or →` to select text
- `L1` + `ALT` + `↑ or ↓` to move up/down a row
- `L1` + `ALT` + `SHIFT` + `↑ or ↓` to duplicate a row
- `L1` + `ALT` + `CTRL` + `↑ or ↓` to create multiline cursor

```keymap
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃           ┃Del desktop┃   HOME    ┃Show desktp┃    END    ┃New desktop┃   ┃     "     ┃     (     ┃     )     ┃     [     ┃     ]     ┃           ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃           ┃Left desktp┃     ←     ┃     ↑     ┃     →     ┃Right deskp┃   ┃     '     ┃     {     ┃     }     ┃     <     ┃     >     ┃           ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃           ┃  CTRL ←   ┃     ↓     ┃   CTRL →  ┃           ┃   ┃     `     ┃     #     ┃     &     ┃     |     ┃     @     ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│ LEFT  │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │ UP    │
│ RIGHT │ ┃           ┃ ┃           ┃           ┃           ┃   SHIFT²  ┃   ┃   SHIFT   ┃     L1    ┃           ┃           ┃ ┃ "Windows" ┃ │ DOWN  │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

### Secondary layer

> Philosophy

The second layer contains all French accents on the left side, and a numpad with calculation symbols on the right side.

Accented letters are positioned by frequency and sorted by letter and accent.
You have to single, double, or triple click to reach even keys.

> Shortcuts

Common use case:

- `L2` + `CTRL` + `+` to zoom in
- `L2` + `CTRL` + `-` to zoom out
- `L2` + `SPACE` to space, useful with numpad

Left hand on keyboard, and right hand on mouse:

- `L2` + `CTRL` + `/` to comment/uncomment (add it to VSCode settings)
- `L2` + `ENTER` to return
- `L2` + `BSPACE` to backspace
- `L2` + `DEL` to delete

```keymap
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃   BSPACE* ┃   œ + Œ   ┃ ° + ² + ³ ┃     _     ┃     -     ┃     +     ┃   ┃     =     ┃     7     ┃     8     ┃     9     ┃   € + £   ┃           ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃    DEL*   ┃   ô + Ô   ┃   é + É   ┃   ê + Ê   ┃   è + ë   ┃     *     ┃   ┃     0     ┃     4     ┃     5     ┃     6     ┃     $     ┃           ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃   î + ï   ┃ ù + û + μ ┃   ç + Ç   ┃ à + â + À ┃     /     ┃   ┃     .     ┃     1     ┃     2     ┃     3     ┃     %     ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│ LEFT  │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │ UP    │
│ RIGHT │ ┃           ┃ ┃           ┃           ┃     L2    ┃   ENTER   ┃   ┃   SPACE   ┃           ┃           ┃           ┃ ┃           ┃ │ DOWN  │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

### Tertiary layer

> Philosophy

The third layer is used for complex shortcuts, arrow characters, and still contains empty keys that are waiting to find a utility.

```keymap
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃    F1     ┃    F2     ┃    F3     ┃    F4     ┃    F5     ┃    F6     ┃   ┃    F7     ┃    F8     ┃    F9     ┃    F10    ┃    F11    ┃    F12    ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃   Reset   ┃           ┃     ←     ┃     ↑     ┃     →     ┃Rec Screen ┃   ┃           ┃           ┃           ┃           ┃           ┃ ALT + F4  ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃           ┃           ┃     ↓     ┃           ┃Print Scree┃   ┃           ┃           ┃           ┃           ┃           ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│ UP    │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │ LEFT  │
│ DOWN  │ ┃ Bootloader┃ ┃   CTRL    ┃    ALT    ┃     L3    ┃           ┃   ┃   SHIFT   ┃     L3    ┃           ┃   To L8   ┃ ┃Del BT link┃ │ RIGHT │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

### Numpad layer

> Philosophy

The numpad layer is useful for editing a calculation sheet, conception entering values, etc...
This layer is a hybrid between the main layer and the secondary layer: you have every number and calculation symbols, but you have also arrows, and essentials keys for the common shortcuts (copy, paste, undo, redo, etc).

```keymap
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃    ESC    ┃     A     ┃     Z     ┃     ↑     ┃     -     ┃     +     ┃   ┃     =     ┃     7     ┃     8     ┃     9     ┃   € + £   ┃   BSPACE  ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃    TAB    ┃     Y     ┃     ←     ┃     ↓     ┃     →     ┃     *     ┃   ┃     0     ┃     4     ┃     5     ┃     6     ┃     $     ┃    DEL    ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃           ┃     X     ┃     C     ┃     V     ┃     /     ┃   ┃     .     ┃     1     ┃     2     ┃     3     ┃     %     ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│ LEFT  │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │ UP    │
│ RIGHT │ ┃           ┃ ┃   CTRL    ┃           ┃   BSPACE  ┃    DEL    ┃   ┃   SPACE   ┃           ┃   ENTER   ┃    WIN    ┃ ┃           ┃ │ DOWN  │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

### Gaming layer

> Philosophy

The gaming layer must be designed to prevent accidental clicks: wrong clicks, unwanted shortcuts, etc.
However, it should still be comprehensive enough to facilitate easy communication in gaming chats.

The `ESC` key is too close to the other frequently used keys. The `ESC` key needs to be held for 200ms to be pressed.

The `CTRL`, `ALT`, and `L3` keys are replaced with non-modifier keys.

```keymap
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃ (Tab)→ESC ┃     A     ┃     Z     ┃     E     ┃     R     ┃     T     ┃   ┃     Y     ┃     U     ┃     I     ┃     O     ┃     P     ┃   BSPACE  ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃    Tab    ┃     Q     ┃     S     ┃     D     ┃     F     ┃     G     ┃   ┃     H     ┃     J     ┃     K     ┃     L     ┃     M     ┃    DEL    ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃     W     ┃     X     ┃     C     ┃     V     ┃     B     ┃   ┃     N     ┃  , or ?   ┃  . or !   ┃  ; or /   ┃  : or \   ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│ VOL+  │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │ VOL+  │
│ VOL-  │ ┃   To L0   ┃ ┃     +     ┃     -     ┃     '     ┃   SPACE   ┃   ┃   SHIFT   ┃     L9    ┃   ENTER   ┃    WIN    ┃ ┃           ┃ │ VOL-  │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

### Gaming layer 2

> NVidia Shortcuts

- `NV_Scrn` to take screenshot
- `NV_FPS` to show performance panel
- `NV_Swth` to toggle performance panel content
- `NV_Strt` to start recording
- `NV_Save` to save recording
- `NV_Dsh` to show dashboard

```keymap
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃           ┃           ┃           ┃           ┃           ┃           ┃   ┃     "     ┃NV ScrenSht┃           ┃           ┃           ┃           ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃           ┃           ┃           ┃           ┃           ┃           ┃   ┃     '     ┃NV Save Rec┃NV StartRec┃           ┃           ┃           ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃           ┃           ┃           ┃           ┃           ┃         `     ┃NV Dashbrd ┃  NV FPS   ┃ NV Rotate ┃           ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│ VOL+  │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │ VOL+  │
│ VOL-  │ ┃           ┃ ┃   CTRL    ┃    ALT    ┃           ┃           ┃   ┃           ┃    L10    ┃           ┃           ┃ ┃           ┃ │ VOL-  │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```
