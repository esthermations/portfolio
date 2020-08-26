# portfolio
Demonstrations of my work!

## RV32Sim
RV32Sim is a simulator of the RISC-V 32-bit architecture written for a Computer Architecture class in 2019. It's a broadly object-oriented program with plenty of low-level nitty-gritty -- bitmasks, unions, all that.

This assignment was delivered in a test-driven way: we were provided with a battery of tests that our simulator must pass, and our grade was directly based on how many tests we passed. Part of these tests were time deadlines given to each operation. Some tests were specifically geared towards this, for example performing an extreme number of memory accesses in a set time. I'm pleased to say mine passed 100% of the tests, so I got full marks on the assignment.

To facilitate an easier test-driven workflow, I wrote a Python script, Run_Tests.py, which provided a very user-friendly command-line interface for running the tests. It would print [PASS], [FAIL], or some other options depending on my program's output versus what was expected. It also has various options for displaying details of the test, outputs, expected outputs, etc. I was quite pleased with this script and it really sped up the development process.

## OpenGL-Game
This was a 3D game written for my Computer Graphics course, in 2017. I'm less proud of the code than the RV32Sim from 2019, but it's quite functional and readable and demonstrates knowledge of concepts important to 3D graphics and game engines. There's plenty of use of GLM for vector maths, mostly for 3D physics and handling the projections in the renderer, and also code hooked up to an open-source Wavefront OBJ parser which turns it all into usable 3D model formats.

The shaders are fairly straightforward but are used to procedurally modify the rendered terrain, putting a road down the middle and bumpy grass either side. The fragment shader colours these segments and draws lines down the middle of the road. A simple Phong lighting system is implemented.
