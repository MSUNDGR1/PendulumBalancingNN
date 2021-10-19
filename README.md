# PendulumBalancingNN
**Using reinforcement learning to balance an inverted pendulum on a linear gantry.**

This project was far more of a learning experience than it was a productive endeavour. I designed a number of 3D printed components to build a linear gantry. The linear gantry was driven by a nema23 stepper motor and the carriage mounted a rotary encoder for the pendulum angle.

An arduino was used to control the stepper motor and to read the encoder signals. The arduino was also connected to a computer through USB, and communication with the reinforcement learning algorithm was done through this link.

On the computer side, a python script was written to communicate with the arduino and relay commands, and a python script was written to actually run the reinforcement learning algorithm on pytorch.
