The Audio Board and the Controller Board are communicating via a simple serial protocol. 
To decouple both galvanically, an optocoupler is used which limits the baudrate to 1200 Baud.

Byte 0: 1 for left channel, 2 for right channel, 3 for both
Byte 1: the command
Byte 2 and 3: an int16 parameter
Byte 4 and 5: second int16 parameter

Command, Parameter 1, Parameter 2, explanation, Default
V, Volume 0..250, not used, Setting the Volume to 0..250, Default 20
I, Input 1 or 2, not used, Setting the Input on 1 (wireless transmission) or 2 (sampling input), Default 2
R, next Radiochannel 1
S, next Radiochannel 2
L, Low Pass
H, High Pass
P, first parametric filter
Q, second parametric filter
