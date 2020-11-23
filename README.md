# Raw socket SCPI tools

This is a small collection of CLI tools using standard
Berkeley sockets to communicate with laboratory equipment
using SCPI. These tools only support a small subset of
SCPI and the device commands - and do not provide any device
family abstraction or device abstraction layer like libscpi.
These tools have been developed to play around with SCPI,
test how commands work and solve some specific scriptable
problems.

# License

These utilities are open source and licensed under a 4 clause
BSD license. See [LICENSE.md](./LICENSE.md)

# Compiling

Use gnu make. Depending on your system this means either

```
gmake
```

or

```
make
```

# Supported devices

* Oscilloscopes
   * RIGOL MSO5072
* Signal generators
   * SIGLENT SSG3021X

# Supported operations

* RIGOL MSO5072
   * ```rigolmso5000_idn``` queries the identity string and serial number
   * ```rigolmso5000_querywaveform``` queries the waveform from a selected
     channel into an ASCII data file using the currently selected settings
* SIGLENT SSG3021X
   * ```siglentssg3021x_idn``` queries the identity string and serial number
   * ```siglentSSG3021x_rfonoff``` enables or disables the RF output
