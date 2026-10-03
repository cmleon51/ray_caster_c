# C RAY CASTER

A simple ray caster that tries to draw everything through the CPU even on modern displays by utilizing multithreading.


## Screenshots

<img width="3440" height="1440" alt="screenshot_2026-10-03_17-41-52" src="https://github.com/user-attachments/assets/a8189fea-51b4-4f29-9ed9-ff8c17fb0f65" />
<img width="3440" height="1440" alt="screenshot_2026-10-03_17-41-59" src="https://github.com/user-attachments/assets/c16811a3-67f2-4b44-888f-cac1298a939d" />
<img width="3440" height="1440" alt="screenshot_2026-09-27_18-51-09" src="https://github.com/user-attachments/assets/f336e673-8cba-4c72-82a2-22da9c2c1032" />


## TODO

Since this program still has a long way to go this is the roadmap for now:
- [X] implement ceiling and ground rendering
- [X] implement sprite rendering
- [X] dynamic lights
- [X] static lights
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
