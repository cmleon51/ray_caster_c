# C RAY CASTER

A simple ray caster that tries to draw everything through the CPU even on modern displays by utilizing multithreading.


## Screenshots

<img width="3425" height="1377" alt="screenshot_2026-09-13_17-21-04" src="https://github.com/user-attachments/assets/222894cb-f77d-443b-addc-7886efa4c1c2" />
<img width="3425" height="1377" alt="screenshot_2026-09-13_17-21-18" src="https://github.com/user-attachments/assets/f9132a5f-3595-4af2-84e6-102c557d614d" />




## TODO

Since this program still has a long way to go this is the roadmap for now:
- [X] implement ceiling and ground rendering
- [ ] implement sprite rendering
- [ ] dynamic lights
- [ ] cpu only rendering with platform-agnostic compilation

`this list will change in the future`

## Dependencies

Depends on the compilation target.
For now the available targets are the following:
  - SDL3 => needs SDL3

## Build

Without specifying a `BACKEND` the compilation will select from a list of targets and pick the first working solution:
```
$ make
$ ./ray_caster
```

When specified `BACKEND` will compile for that target directly if the necessary Dependencies are available:
```
$ BACKEND=sdl3 make
$ ./ray_caster
```

## AI Usage
AI has been used in the development of this project.
Precisely AI has been used for the following:
  - understanding the math needed for the ray-casting algorithm
  - monitoring the performance bottlenecks of the application
  - Write the `Makefile` needed to compile the project
