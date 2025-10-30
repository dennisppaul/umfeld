---
layout: home
---

{% include_relative /assets/abstract.md %}

<!-- ![umfeld-logotype](/assets/umfeld-logotype.png){: style="width:50%;" } -->

## Quickstart

### 1. Prepare

- on *macOS*: no preparation needed
- on *Linux*: open a console and run `sudo apt install -y git curl`  
- on *Windows ( MSYS2 UCRT64 )*: install [MSYS2](https://www.msys2.org/) and open the `MSYS2 UCRT64` console

### 2. Install Dependencies + Download *Umfeld*

open a console, navigate to the desired installation folder, and run the following script:

```sh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/dennisppaul/umfeld/main/install.sh)"
```

this script installs all required packages ( on *macOS*, *Linux* ( including *Raspberry Pi OS* ), and *Windows ( MSYS2 UCRT64 )* ) and downloads *Umfeld* with examples and external libraries into the current directory.

⚠️ IMPORTANT ⚠️ on *Windows ( MSYS2 UCRT64 )*, the console might close when running the script above. this is expected behavior ( part of the self-updating process ). just run the script again. it should work fine the second time.

### 3. Build + Run an Example

run an example ( e.g. `minimal` ) to see if *Umfeld* works. open a console, navigate to the example folder and run the CMake build commands:

```sh
cd umfeld-examples/Basics/minimal
cmake -B build
cmake --build build
./build/minimal
```

if everything works, head over to [Umfeld](https://github.com/dennisppaul/umfeld).
