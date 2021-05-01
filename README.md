They don't make light guns that work with LCD screens, so I'm making my own using an Arduino pro micro, a gyroscope, an accelerometer, an encoder, a mini-joystick, some geometry and physics math.

So far it supports primary firing, secondary firing, scope aim, reload, character leaning (by tilting the gun), adjustable sensitivity on each axis, and movement.

The code is complete and should work for anyone. Take careful note of the pin assignments and adjust accordingly.

Added mini-joystick support for movement and zoom (aiming down sights). I feel like it would be limiting if you have to use a secondary controller/keyboard for movement, but it's your call if you want to use it.

All options are just that, optional. If you don't add a joystick or encoder or buttons they'll just be ignored. No need to disable them in the code.

This code can easily be used as a simple hover-mouse if you wish, with zero required code changes.

Assignments:
[Trigger] : Left mouse
[Alt] : Middle mouse
[Reload] : Presses 'r' key
[Joystick button] : Right mouse
[Encoder button] :
(press) Switch between X and Y gyro sensitivity adjustment
(hold) Hold cursor - used to adjust gun aim
[Encoder + Alt] : Re-calibrate gyro

[Encoder] : increase/decrease gyro sensitivity on a per-axis basis (press encoder button to switch axes)
[Joystick] : WASD keys
[Left/Right tilt] : Presses PgUp/PgDn keys (designed to be used for leaning in games which support it)


My code is on Github: https://github.com/Domush/Arduino-Arcade-PC-light-gun

For non-commercial use ONLY.
If you wish to use this code for commercial purposes, you must contact me for permission.
