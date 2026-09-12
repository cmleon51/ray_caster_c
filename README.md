# C RAY CASTER

A simple ray caster that tries to draw everything through the CPU even on modern displays by utilizing multithreading.


## Screenshots

<img width="3425" height="1377" alt="screenshot_2026-09-12_09-19-51" src="https://github.com/user-attachments/assets/d7f5e218-79cd-47e1-87e3-06e45e3fc5ed" />
<img width="3425" height="1377" alt="screenshot_2026-09-12_09-20-25" src="https://github.com/user-attachments/assets/c9f80c07-5fb4-4e03-bbd5-2dcac31908af" />


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
