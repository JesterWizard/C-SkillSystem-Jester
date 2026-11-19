## Quick Start

> [!NOTE]
> Building requires a Linux environment.
>
> On Windows, use [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) or a Linux VM/server such as [Ubuntu Server](https://ubuntu.com/aws).

## Custom build

Follow these steps to build the project from source.

1. Install system and Python dependencies.

```bash
sudo apt-get -y install git make build-essential python3 python3-pip moreutils perl

pip install Pillow six numpy
```

2. Install DevkitPro (it provides the ARM compiler and `grit`).

```bash
wget https://apt.devkitpro.org/install-devkitpro-pacman
chmod +x ./install-devkitpro-pacman
sudo ./install-devkitpro-pacman
sudo dkp-pacman -S gba-dev

# Export vars
echo "export DEVKITPRO=/opt/devkitpro" >> ~/.bashrc
echo "export DEVKITARM=\${DEVKITPRO}/devkitARM" >> ~/.bashrc
echo "export PATH=\${DEVKITPRO}/tools/bin:\$PATH" >> ~/.bashrc
source ~/.bashrc
```

3. Clone the repository and place a clean Fire Emblem: The Sacred Stones (U) ROM in its root as `fe8.gba`
   (SHA-1 `c25b145e37456171ada4b0d440bf88a19f4d509f`).

```bash
git clone https://github.com/JesterWizard/C-SkillSystem-Jester.git
cd C-SkillSystem-Jester
```

4. Run the setup script.

```bash
./Tools/setup.sh
```

It downloads nothing. It marks the committed `.lyn.event` files as up to date, then checks the committed tools, your
devkitARM, the ROM and the Python packages. It is safe to run again.

> [!IMPORTANT]
> Every tool the build uses is committed to this repository: EventAssembler, FE-CLib-Mokha (with its FE8U symbol
> files in `Tools/FE-CLib-Mokha/reference`), FE-PyTools and check_patch. Do not clone or update them from upstream:
> newer upstream versions rename symbols and fail the build with errors such as
> `Undefined identifier: ItemEffectiveness_088ADEC2` and
> `Trying to write data to area protected in file HPBarPalettes.lyn.event`. If you see those errors, restore the
> committed files (`git checkout -- Tools '*.lyn.event'`) and run `./Tools/setup.sh` again.

5. (Optional) Install the repository hook.

```bash
cp Tools/scripts/pre-commit .git/hooks/
```

6. Build the project.

```bash
make
```

### Optional tools

- **Rebuilding Event Assembler tools.** Only needed if you change their sources in `Tools/EventAssembler/.Sources`.
  It needs `cmake`, `ghc` and `cabal-install`. Run `Tools/scripts/build_ea_wo_core.sh` after copying it into
  `Tools/EventAssembler/`.
- **Windows Event Assembler** (`CONFIG_EA_WIN=1` in `configs.mk`). This needs `wine` and `wine-mono`. The default
  build uses the Linux `ColorzCore` and does not need them.

## Build Outputs

Successful builds produce the following files.

| Name | Description |
| :--- | :---------- |
| fe8-kernel-dev.gba | Built ROM |
| fe8-kernel-dev.sym | Debug symbols for NO$GBA |
| fe8-kernel-dev.ref.s | Lyn reference output |
| fe8-kernel-dev.ref.event | Event Assembler reference output |
