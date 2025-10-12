---
layout: post
title:  "ASCIINEMA Shows How to Run an Umfeld Application"
date:   2025-05-28 10:00:00 +0100
---

<link rel="stylesheet" type="text/css" href="/assets/asciinema-player.css" />
<div class="asciinema-container"><div id="asciinema-cast"></div></div>
<script src="/assets/asciinema-player.min.js"></script>
<script>
  AsciinemaPlayer.create('/assets/umfeld-example--minimal.cast', document.getElementById('asciinema-cast'));
</script>

i was playing around with ASCIINEMA and this recording demonstrates how to compile and run an *Umfeld* application from CLI:

```sh
cd umfeld-examples
ls
cd Basics
ls
cd minimal
ls
cmake -B build
cmake --build build --parallel
./build/minimal
echo "done ;)"
exit
```
