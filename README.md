Biometric Evaluation Framework <img src="doc/doxygen/assets/nist_itl_two_color.svg" align="right" alt="NIST Information Technology Laboratory" style="width:250px;" />
==============================

[![Build Library and Tests](https://github.com/usnistgov/libbiomeval/actions/workflows/build.yml/badge.svg)](https://github.com/usnistgov/libbiomeval/actions/workflows/build.yml)

The NIST Image Group's [Biometric Evaluation Framework][1] is a set of C++
classes, error codes, and design patterns used to create a common environment to
provide logging, data management, error handling, and other functionality that
is needed for many applications used in the testing of biometric software.

The goals of Biometric Evaluation Framework include:

 * reduce the amount of I/O error handling implemented by applications;
 * provide standard interfaces for data management and logging;
 * remove the need for applications to handle low-level events from the
   operating system;
 * provide time tracking and constraints for blocks of code;
 * reduce potential for memory errors and corruption;
 * simplify the use of parallel processing.

How to Build
------------
Once all requirements have been met, use CMake to build:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Documentation
-------------
All header files are well documented. A web browsable version of the API
documentation is
[available on our website](https://pages.nist.gov/libbiomeval/docs/).

Requirements
------------
We currently test this software using **g++ 13.3** under **Ubuntu 24.04**. Other
operating systems and compilers are likely to work as expected, but have not
been thoroughly tested. Additionally, not all parts of `libbiomeval` will be
enabled to build on all platforms.

You will need to install several [system packages](#system-packages) to enable
certain features of Biometric Evaluation Framework.

Installing
----------
We occasionally post pre-compiled version of Biometric Evaluation framework on
the [releases](https://github.com/usnistgov/libbiomeval/releases)
page on GitHub. These packages have been signed with our
[public key][2].

System Packages
---------------
### Quick Install Commands

The following package manager commands may be useful to install all packages
needed to support all features of Biometric Evaluation Framework (though you
still may need to download and install sources for other non-system packages on
some operating systems---read all of [Module Requirements](#module-requirements)
for complete details).

 * macOS:
   - MacPorts:
     ```sh
     port install openjpeg jpeg libpng tiff zlib openmpi db62 sqlite3 hwloc ffmpeg
     ```

   - Homebrew:
     ```sh
     brew install openjpeg jpeg-turbo libpng libtiff zlib open-mpi berkeley-db sqlite hwloc ffmpeg
     ```
 * Ubuntu 20-24:
   ```sh
   apt install libpcsclite-dev libssl-dev libopenjp2-7-dev libjpeg-dev libpng-dev libtiff-dev zlib1g-dev libopenmpi-dev libdb++-dev libsqlite3-dev libhwloc-dev libavcodec-dev libavformat-dev libswscale-dev
   ```
 * Ubuntu 26:
   ```sh
   apt install libpcsclite-dev libssl-dev libopenjp2-7-dev libjpeg-dev libpng-dev libtiff-dev zlib1g-dev libopenmpi-dev libdb5.3++-dev libsqlite3-dev libhwloc-dev libavcodec-dev libavformat-dev libswscale-dev
   ```
 * Windows:
   ```dos
   vcpkg install openssl openjpeg libjpeg-turbo libpng tiff zlib msmpi berkeleydb sqlite3 zlib hwloc ffmpeg pkgconf
   ```

### Module Requirements

Some modules require system packages that may not be installed by default on
all operating systems. Package names are listed below for macOS
(via [MacPorts](https://www.macports.org) and [Homebrew](https://brew.sh),
Ubuntu, and Windows (via [`vcpkg`](https://github.com/Microsoft/vcpkg)).
Other operating systems may use similarly-named packages.

**Note:**

 * When using `vcpkg`, you **must** provide CMake with the path to your `vcpkg`
   toolchain:
   ```dos
   cmake .. -DCMAKE_TOOLCHAIN_FILE=%vcpkg_root%\scripts\buildsystems\vcpkg.cmake
   ```



#### CORE
| Name         | MacPorts/Homebrew            | Ubuntu       | vcpkg     |
|:------------:|:----------------------------:|:------------:|:---------:|
| OpenSSL      | n/a (uses macOS CommonCrypto)| `libssl-dev` | `openssl` |

#### DEVICE

| Name      | MacPorts/Homebrew                | Ubuntu   | vcpkg |
|:---------:|:--------------------------------:|:--------:|:-----:|
| PCSC Lite | n/a (requires [Command Line Tools](https://developer.apple.com/library/archive/technotes/tn2339/_index.html)) | `libpcsclite-dev` | *Unsupported* |

#### IMAGE
| Name         | MacPorts   | Homebrew               | Ubuntu             | vcpkg          |
|:------------:|:----------:|:----------------------:|:------------------:|:--------------:|
| OpenJPEG 2.x | `openjpeg` | `openjpeg`             | `libopenjp2-7-dev` | `openjpeg`     |
| libjpeg      | `jpeg`     | `jpeg` or `jpeg-turbo` | `libjpeg-dev`      | `libjpeg-turbo`|
| libpng       | `libpng`   | `libpng`               | `libpng-dev`       | `libpng`       |
| libtiff      | `tiff`     | `libtiff`              | `libtiff-dev`      | `tiff`         |
| Zlib         | `zlib`     | `zlib`                 | `zlib1g-dev`       | `zlib`         |

**Note:**

 * Under CentOS, `openjpeg2-devel` is part of the EPEL repository. It can be
   enabled by installing the package `epel-release`.

#### IO
| Name         | MacPorts | Homebrew  |Ubuntu       | vcpkg  |
|:------------:|:--------:|:---------:|:-----------:|:------:|
| Zlib         | `zlib`   | `zlib`    |`zlib1g-dev` | `zlib` |

#### MPIBASE, MPIDISTRIBUTOR, MPIRECEIVER
| Name         | MacPorts  | Homebrew   | Ubuntu           | vcpkg   |
|:------------:|:---------:|:----------:|:----------------:|:-------:|
| Open MPI     | `openmpi` | `open-mpi` | `libopenmpi-dev` | `msmpi` |

**Note:**

 * With MacPorts, you may need to select a different MPI group if you have more
   than one installed:
   ```bash
   sudo port select mpi openmpi-mp-fortran
   ```

 * With `msmpi` and vcpkg, you must first install the
  [Microsoft MPI redistributable package](https://docs.microsoft.com/en-us/message-passing-interface/microsoft-mpi).

#### RECORDSTORE
| Name         | MacPorts  | Homebrew      | Ubuntu           | vcpkg        |
|:------------:|:---------:|:-------------:|:----------------:|:------------:|
| Berkeley DB  | `db62`    | `berkeley-db` | `libdb++-dev`   | `berkeleydb` |
| SQLite 3     | `sqlite3` | `sqlite`      | `libsqlite3-dev` | `sqlite3`    |
| Zlib         | `zlib`    | `zlib`        | `zlib1g-dev`     | `zlib`       |

**Note:**

 * Under Ubuntu 26, the package name for Berkeley DB is `libdb5.3++-dev`

#### SYSTEM
| Name                       | MacPorts | Homebrew | Ubuntu         | vcpkg   |
|:--------------------------:|:--------:|:--------:|:--------------:|:-------:|
| Portable Hardware Locality | `hwloc`  | `hwloc`  | `libhwloc-dev` | `hwloc` |

#### VIDEO
| Name                        | MacPorts       | Homebrew | Ubuntu          | vcpkg |
|:---------------------------:|:--------------:|:--------:|:---------------:|:------|
| [ffmpeg](http://ffmpeg.org) | `ffmpeg` | `ffmpeg` | `libavcodec-dev`, `libavformat-dev`, `libswscale-dev` | `ffmpeg` |

##### NIST Biometric Image Software (NBIS)
The Framework repository contains a subset of [NBIS](https://www.nist.gov/services-resources/software/nist-biometric-image-software-nbis) that is built automatically.

Options
-------
The CMake build supports the following options:

| CMake Option            | Default | Description                                          | Notes |
|:------------------------|:-------:|:-----------------------------------------------------|:------|
| `BUILD_BIOMEVAL_32BIT`  | `OFF`   | Compile 32-bit on 64-bit host OS                     ||
| `BUILD_BIOMEVAL_TESTS`  | `OFF`   | Build test programs                                  ||
| `BUILD_FOR_WASM` | `OFF` | Disable components currently not supported under WebAssembly | Defaults to `ON` when the [Emscripten](https://emscripten.org) toolchain is detected |
| `BUILD_SHARED_LIBS`     | `OFF`    | Build shared library (i.e., `.so`, `.dll`, `.dylib`) | When `OFF`, a static library (i.e., `.a`, `.lib`) is built instead |
| `STATIC_CRT` | `OFF` | Statically link the universal C runtime library | `MSVC` only |
| `WASM_EXCEPTIONS` | `ON` | When compiled to WebAssembly, use WebAssembly exceptions instead of JavaScript exceptions | Only available when `BUILD_FOR_WASM` is `YES` and the [Emscripten](https://emscripten.org) toolchain is detected |
| `WITH_FFMPEG` | `ON` | Build sources that require [FFMPEG](https://ffmpeg.org) ||
| `WITH_HWLOC` | `ON` | Build sources that require [libhwloc](https://www.open-mpi.org/projects/hwloc/) |
| `WITH_MPI` | `ON` | Build sources that require [OpenMPI](https://www.open-mpi.org/) |
| `WITH_PCSC` | `ON` | Build sources that require [PCSC](https://pcsclite.apdu.fr) |

### A Note about WebAssembly

A minimal version of the library can be compiled to WebAssembly and is supported. Compilation has only been tested using the [Emscripten](https://emscripten.org) toolchain. When this toolchain is detected (i.e., when `emcmake` is used to configure, rather than `cmake`), the above options for WebAssembly are automatically enabled.

---

As Seen In...
-------------
NIST is committed to using Biometric Evaluation Framework in their biometric
evaluations, including:

 * [ELFT](https://www.nist.gov/itl/iad/btg/evaluation-latent-friction-ridge-technology);
 * [FRTE/FATE](https://www.nist.gov/programs-projects/face-technology-evaluations-frtefate);
 * [IREX 10](https://www.nist.gov/programs-projects/iris-exchange-irex-10-ongoing-evaluation-iris-recognition);
 * [MINEX III](https://www.nist.gov/itl/iad/btg/minutiae-interoperability-exchange-minex-iii);
 * [PFT III](https://www.nist.gov/itl/iad/btg/proprietary-fingerprint-template-pft-iii);
 * [SlapSeg III](https://www.nist.gov/itl/iad/btg/slap-fingerprint-segmentation-evaluation-iii);
 * ...and more.

Communication
-------------
If you found a bug and can provide steps to reliably reproduce it, or if you
have a feature request, please
[open an issue](https://github.com/usnistgov/libbiomeval/issues). Other
questions may be addressed to the
[project maintainers](mailto:beframework@nist.gov).

Pull Requests
-------------
Thanks for your interest in submitting code to Biometric Evaluation Framework.
In order to maintain our project goals, pull requests must:

 * adhere to the existing coding style;
 * use Framework types consistently wherever possible;
 * compile without warning;
 * only make use of POSIX APIs;
 * be in the public domain.

Pull requests may be subject to additional rules as imposed by the National
Institute of Standards and Technology. *Please contact the maintainers*
**before** starting work on or submitting a pull request.

Credits
-------
Biometric Evaluation Framework is primarily maintained by Wayne Salamon and
Gregory Fiumara, featuring code from several NIST contributors. This work has
been sponsored by the National Institute of Standards and Technology, the
Department of Homeland Security, and the Federal Bureau of Investigation.

Citing
------
If you use Biometric Evaluation Framework in the course of your work, please
consider linking back to
[our website](https://www.nist.gov/services-resources/software/biometric-evaluation-framework) or citing
[our manuscript](https://doi.org/10.1109/BTAS.2015.7358800):

Fiumara, G.; Salamon, W.; Watson, C, "Towards Repeatable, Reproducible, and
Efficient Biometric Technology Evaluations," in Biometrics: Theory,
Applications and Systems (BTAS), 2015 IEEE 7th International Conference on,
Sept. 8 2015-Sept 11 2015.

### BibTeX
```latex
@INPROCEEDINGS{7358800,
	author={Gregory Fiumara and Wayne Salamon and Craig Watson},
	title={{Towards Repeatable, Reproducible, and Efficient Biometric
	Technology Evaluations}},
	booktitle={Biometrics Theory, Applications and Systems (BTAS), 2015 IEEE
	7th International Conference on},
	year={2015},
	pages={1-8},
	doi={10.1109/BTAS.2015.7358800},
	month={Sept}
}
```

License
-------
Biometric Evaluation Framework is released in the public domain. See the
[LICENSE](https://github.com/usnistgov/libbiomeval/blob/master/LICENSE.md)
for details.

[1]: https://www.nist.gov/services-resources/software/biometric-evaluation-framework
[2]: https://github.com/usnistgov/libbiomeval/blob/master/beframework_signing_key.asc
