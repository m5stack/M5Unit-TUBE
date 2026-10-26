# M5Unit - TUBE

## Overview

Library for UnitTUBE using [M5UnitUnified](https://github.com/m5stack/M5UnitUnified).  
M5UnitUnified is a library for unified handling of various M5 units products.

### SKU:U130
Unit Tube Pressure is a positive and negative pressure gauge that supports a wide measurement range of -100 ~ 200kPa. When in use, it is connected via a tube, with the other end connected to the gas measurement environment. The sensor maps the range -100 ~ 200kPa to output a voltage of 0.1 ~ 3.1V. It comes with a fully enclosed protective shell, effectively ensuring the stability of the sensor, making it very suitable for applications such as gas pressure detection in industrial equipment.

## Related Link

- [Unit Tube Pressure - Document & Datasheet](https://docs.m5stack.com/en/unit/tube_pressure)

## Required Libraries

- [M5UnitUnified](https://github.com/m5stack/M5UnitUnified)
- [M5Utility](https://github.com/m5stack/M5Utility)
- [M5HAL](https://github.com/m5stack/M5HAL)

## License

- [M5Unit-TUBE - MIT](LICENSE)

## Examples
See also [examples/UnitUnified](examples/UnitUnified)

- [UnitTubePressure/PlotToSerial](examples/UnitUnified/UnitTubePressure/PlotToSerial): Outputs the pressure, voltage and raw value of each periodic measurement for a serial plotter
- [UnitTubePressure/ViaPbHub](examples/UnitUnified/UnitTubePressure/ViaPbHub): The same through PbHub (ch:3).
  Requires [M5Unit-HUB](https://github.com/m5stack/M5Unit-HUB).

### For ESP-IDF settings

> **NOTE:** The ESP-IDF native build (`idf.py`) targets ESP-IDF **5.1 or later** (5.x and 6.x).

Each example covers a single unit, so the examples need no Kconfig choice. Build them directly:

```sh
cd examples/UnitUnified/UnitTubePressure/PlotToSerial
idf.py set-target esp32s3               # or esp32 / esp32c5 / esp32c6 / esp32h2 / esp32p4 / ...
idf.py build flash monitor
```

## Support via [PbHub](https://docs.m5stack.com/en/unit/pbhub_1.1)

|Unit|Support|
|---|---|
|UnitTubePressure|OK|

See also [M5Unit-HUB](https://github.com/m5stack/M5Unit-HUB)

## Doxygen document
[GitHub Pages](https://m5stack.github.io/M5Unit-TUBE/)

If you want to generate documents on your local machine, execute the following command

```
bash docs/doxy.sh
```

It will output it under docs/html  
If you want to output Git commit hashes to html, do it for the git cloned folder.

### Required
- [Doxygen](https://www.doxygen.nl/)
- [Git](https://git-scm.com/) (Output commit hash to html)
