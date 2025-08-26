### Dullahan OpenGL Example Application

Cross platform example for illustration and standalone of Dullahan features. Renders output to an OpenGL 2.1 quad and allows interaction using the mouse.

* Cross platform
* Run then open Help -> About for instructions

#### Known issues:
* Developed on Windows but should work on macOS and Linux without any major changes - coming soon I hope.
* GLFW doesn't appear to expose the native OS keyboard/window events that Dullahan requires so no keyboard input for the moment.
* Code could use an optimization pass - glReadPixels is used to determine mouse move/click position and it's notoriously slow - should probably intersect a ray from mouse cursor into scene.
* The requisite unique nature of the CEF cache folder means that each time this runs, a new cache folder is created and left behind. This can get quite large - should consider pruning at startup
