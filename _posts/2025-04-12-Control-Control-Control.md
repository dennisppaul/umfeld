---
layout: post
title:  "Control Control Control"
date:   2025-04-12 10:00:00 +0100
---

![2025-04-12-Control-Control-Control](/assets/2025-04-12-Control-Control-Control.gif){: style="width:100%;" }

*Umfeld* can now handle gamepads. just enable them in `settings()` and implement the callbacks:

```c++
void settings() {  
    enable_gamepads();  
}  
  
void gamepad_axis(const int id, const int axis, const float value) {  
    // ...  
}  
  
void gamepad_button(const int id, const int button, const bool down) {  
    // ...  
}
```

( note, renamed methods to all lower case on 2025-04-14 )

i have tested this with *Nintendo Joy-Cons* and an *Microsoft XBox One* controller on macOS. note, that controllers need to be connected as bluetooth devices in the OS first. 

also, this was almost too easy. SDL3 does A LOT OF nice things in the background already. e.g it handles very seemless removing and adding controllers. also, i have really just implemented the very basic basics. there is a lot more that can be done with the gamepads ( e.g rumble, LEDs, querry properties ). however, i left a *window* open into SDL3, if one chooses to access the underlying infrastructure directly.