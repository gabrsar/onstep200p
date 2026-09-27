# PanelJoystick

VRx→GPIO5, VRy→GPIO4, SW→GPIO6. The physical stick is rotated90°: VRx is
vertical and VRy horizontal. Power it from3.3V. The plugin owns filtering,
gestures, three-circle calibration, persistent dead zones and manual motion.

Switch presses request stop immediately. Manual motion remains unverified
with motors; read [hardware](../../docs/HARDWARE.md) before commissioning.
The full gesture/calibration flow is in [controls](../../docs/CONTROLS.md).
