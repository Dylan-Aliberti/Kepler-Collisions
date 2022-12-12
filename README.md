# Kepler-Collisions
Code used for testing the analytic propagator. This algorithm was described by Dr. P.M. Visser, and in my bachelor thesis an implementation of this algorithm was made in order to test it. The thesis can be found here: http://resolver.tudelft.nl/uuid:a5764b08-9dca-4398-9309-fb99fe3a271d

The main code is written in C++, where the simulations were done.
The data from the simulations was then processed in a Jupyter Notebook (which runs on Python), to make plots and/or calculate certain statistics.
The C++ part also has a graphical interface using OpenGL, allowing you to "fly" in the space of the simulation and observing everything. This was mainly used for debugging, but it can help you get an idea of what's going on.

The algorithm simulates an N-body Keplerian system. It maintains a list of all collision possibilities, and jumps from collision to collision. The idea is that for a lot of systems in astronomy, for example the Solar System, collisions are rare, thus making this approach a fast method.

This repository contains all code used for this project, including the libraries.
This code was developed in Microsoft Visual Studio. If you are downloading and starting it in Visual Studio, you will probably have to change the directories in project settings. To get the libraries working in Visual Studio, do the following:
- At the top of the window, go to Project -> properties
- Go to VC++ Directories
- Edit the following two paths: <br />
  Include directories: <baseFolder>/libraries/include <br />
  Library directories: <baseFolder>/libraries/lib
- Go to Linker -> Input <br />
Add the following two additional dependencies: <br />
glfw3.lib <br />
opengl32.lib
