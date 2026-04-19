## Quick Start

> [!NOTE]
> Building requires a Linux environment.
>
> On Windows, use [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) or a Linux VM/server such as [Ubuntu Server](https://ubuntu.com/aws).

## Custom build

Follow these steps to build the project from source.

1. Install the required submodules and external tools.

```bash
cd Tools
git clone https://github.com/MokhaLeee/FE-CLib-Mokha.git
git clone https://github.com/MokhaLeee/EventAssembler.git -b mokha-fix
git clone https://github.com/StanHash/FE-PyTools.git --recursive
git clone https://github.com/MokhaLeee/check_patch.git
```

1. Install system and Python dependencies.

```bash
sudo apt-get -y install binutils-arm-none-eabi ctags \
	gcc-arm-none-eabi build-essential cmake re2c ghc \
	cabal-install libghc-vector-dev libghc-juicypixels-dev \
	python3-pip pkg-config libpng* moreutils perl

pip install pyelftools PyInstaller tmx six Pillow

cabal update

# install wine
sudo apt-get -y wine
wget https://mirrors.tuna.tsinghua.edu.cn/winehq/wine/wine-mono/9.4.0/wine-mono-9.4.0-x86.msi
wine msiexec /i wine-mono-9.4.0-x86.msi
```

2. Install DevkitPro.

```bash
wget https://apt.devkitpro.org/install-devkitpro-pacman
chmod +x ./install-devkitpro-pacman
sudo ./install-devkitpro-pacman
sudo dkp-pacman -S gba-dev

# Export vars
echo "export DEVKITPRO=/opt/devkitpro" >> ~/.bashrc
echo "export DEVKITARM=\${DEVKITPRO}/devkitARM" >> ~/.bashrc
echo "export DEVKITPPC=\${DEVKITPRO}/devkitPPC" >> ~/.bashrc
echo "export PATH=\${DEVKITPRO}/tools/bin:\$PATH" >> ~/.bashrc
source ~/.bashrc
```

3. Build Event Assembler tools.

```bash
cp Tools/scripts/build_ea_wo_core.sh Tools/EventAssembler/
cd Tools/EventAssembler
./build_ea_wo_core.sh
```

4. Install the repository hook.

```bash
cp Tools/scripts/pre-commit .git/hooks/
```

5. Place a clean Fire Emblem: The Sacred Stones ROM in the repository root as `fe8.gba`.

6. Build the project.

```bash
make
```

> [!NOTE]
> If GCC reports an error, update C-Lib and retry. See [discussion #115](https://github.com/MokhaLeee/fe8u-cskillsys-kernel/discussions/115).

## Build Outputs

Successful builds produce the following files.

| Name | Description |
| :--- | :---------- |
| fe8-kernel-dev.gba | Built ROM |
| fe8-kernel-dev.sym | Debug symbols for NO$GBA |
| fe8-kernel-dev.ref.s | Lyn reference output |
| fe8-kernel-dev.ref.event | Event Assembler reference output |
