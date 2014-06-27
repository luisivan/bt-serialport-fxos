Notes
===

Bluetooth
---

Initiator -> Acceptor

For purposes of mapping the Serial Port profile to the 
conventional
serial port architecture, both DevA and DevB can be either 
a Data Circuit Endpoint
(DCE) or a Data Terminal Endpoint (DTE). (The RFCOMM 
protocol is designed to be
independent of DTE-DCE or DTE-DTE relationships.)

This profile requires support for one-slot packets only. 
This means that this profile
ensures that data rates up to 128 kbps can be used. 
Support for higher rates is optional.

Only one connection at a time is dealt with in this 
profile. Consequently, only point-to-
point configurations are considered. However, this should 
not be construed as imposing
any limitation on concurrence; multiple executions of this 
profile should be able to run
concurrently in the same device. This also includes taking 
on the two different roles (as
DevA and DevB) concurrently.

Bond: lets devices connect to each other without 
re-pairing

This profile is built upon the Generic Access Profile.


Gecko
---
