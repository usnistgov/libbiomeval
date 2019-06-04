Biometric Evaluation Framework
==============================
Biometric Evaluation Framework is a set of C++ classes, error codes, and design
patterns used to create a common environment to provide logging, data
management, error handling, and other functionality that is needed for many
applications used in the testing of biometric software.

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
If all requirements have been met, building is as simple as:
```bash
make
```

### Build Configuration
The makefile understands several variables:

 * **`CC`**: C compiler;
 * **`CXX`**: C++ compiler;
 * **`MPICXX`**: OpenMPI compiler;
 * **`EXTRA_CXXFLAGS`**: Additional compile-time flags to **`$(CC)`**;
 * **`EXTRA_CXXFLAGS`**: Additional compile-time flags to **`$(CXX)`**;
 * **`EXTRA_LDFLAGS`**: Additional link-time flags to **`$(CXX)`**;
 * **`32`**: Force 32-bit compilation when set to `1`;
 * **`64`**: Force 64-bit compilation when set to `1`.

Requirements
------------
 * A C++ 2011 compiler:
	* `g++` >= 4.7
	* `clang++` >= 3.1
	* `icpc` >= 15.0

 * A supported operating system:
	* RHEL/CentOS 7.x
	* macOS >= 10.9
	* Cygwin 1.7.x

 * System packages (depending on desired modules, see below).

Other operating systems and compilers are likely to work as expected, but have
not been thoroughly tested.

Installing
----------
A pre-compiled version of Biometric Evaluation framework for RHEL/CentOS 7 is
available from the
[releases](https://github.com/usnistgov/libbiomeval/releases)
page on GitHub. These packages have been signed with our
[public key](
https://github.com/usnistgov/libbiomeval/blob/master/beframework_signing_key.asc).

```bash
# Import the public key
rpm --import beframework_signing_key.asc

# Verify the signature
rpm --checksig libbiomeval-10.0-1.x86_64.rpm
libbiomeval-10.0-1.x86_64.rpm: rsa sha1 (md5) pgp md5 OK

# Install
rpm --install libbiomeval-10.0-1.x86_64.rpm
```

A version compiled from source can be installed via the top-level makefile.
```bash
make install
```

System Packages
---------------
Some modules require system packages that may not be installed by default on
all operating systems. Package names are listed below for RHEL/CentOS, macOS
(via [MacPorts](https://www.macports.org)), and Ubuntu. Other operating systems
may use similarly-named packages.

### CORE
| Name         | RHEL/CentOS     | MacPorts                     | Ubuntu       |
|:------------:|:---------------:|:----------------------------:|:------------:|
| pkg-config   | `pkgconfig`     | `pkgconfig`                  | `pkg-config` |
| OpenSSL      | `openssl-devel` | n/a (uses macOS CommonCrypto)| `libssl-dev` |

### DEVICE

| Name      | RHEL/CentOS       | MacPorts                         | Ubuntu   |
|:---------:|:-----------------:|:--------------------------------:|:--------:|
| PCSC Lite | `pcsc-lite-devel` | n/a (requires [Command Line Tools](https://developer.apple.com/library/archive/technotes/tn2339/_index.html)) | `libpcsclite-dev` |

### IMAGE
| Name         | RHEL/CentOS           | MacPorts   | Ubuntu             |
|:------------:|:---------------------:|:----------:|:------------------:|
| OpenJPEG 2.x | [`openjpeg2-devel`](https://dl.fedoraproject.org/pub/epel/7/x86_64/Packages/o/) | `openjpeg` | `libopenjp2-7-dev` |
| libjpeg      | `libjpeg-turbo-devel` | `jpeg`     | `libjpeg-dev`      |
| libpng       | `libpng-devel`        | `libpng`   | `libpng-dev`       |
| libtiff      | `libtiff-devel`       | `tiff`     | `libtiff-dev`      |
| Zlib         | `zlib-devel`          | `zlib`     | `zlib1g-dev`       |

### IO
| Name         | RHEL/CentOS  | MacPorts | Ubuntu       |
|:------------:|:------------:|:--------:|:-------------|
| Zlib         | `zlib-devel` | `zlib`   | `zlib1g-dev` |

### MPIBASE, MPIDISTRIBUTOR, MPIRECEIVER
| Name         | RHEL/CentOS     | MacPorts  | Ubuntu           |
|:------------:|:---------------:|:---------:|:----------------:|
| Open MPI     | `openmpi-devel` | `openmpi` | `libopenmpi-dev` |

**Note:**

 * You need to modify the definition of `MPICXX` in [`common.mk`][1] if `mpicxx`
   is not in your `PATH`:

    | OS          | Default `mpicxx` location       |
    |:-----------:| :------------------------------:|
    | MacPorts    | `/opt/local/bin/mpicxx`         |
    | RHEL/CentOS | `/usr/lib64/openmpi/bin/mpicxx` |
    | Ubuntu      | `/usr/bin/mpicxx`               |

 * With MacPorts, you may need to select a different MPI group if you have more
   than one installed:
   ```bash
   sudo port select mpi openmpi-mp-fortran
   ```

### RECORDSTORE
| Name         | RHEL/CentOS    | MacPorts  | Ubuntu           |
|:------------:|:--------------:|:---------:|:----------------:|
| Berkeley DB  | `libdb-devel`  | `db44`    | `libdb-dev`      |
| SQLite 3     | `sqlite-devel` | `sqlite3` | `libsqlite3-dev` |
| Zlib         | `zlib-devel`   | `zlib`    | `zlib1g-dev`     |

### SYSTEM
| Name                       | RHEL/CentOS   | MacPorts | Ubuntu         |
|:--------------------------:|:-------------:|:--------:|:--------------:|
| Portable Hardware Locality | `hwloc-devel` | `hwloc`  | `libhwloc-dev` |

### VIDEO
| Name                        | RHEL/CentOS | MacPorts       | Ubuntu          |
|:---------------------------:|:-----------:|:--------------:|:---------------:|
| [ffmpeg](http://ffmpeg.org) | Build from source, and install to `/usr/local` | `ffmpeg-devel` | `libavcodec-dev`, `libavformat-dev`, `libswscale-dev` |


#### NIST Biometric Image Software (NBIS)
[NBIS](https://www.nist.gov/services-resources/software/nist-biometric-image-software-nbis) is supported under current versions
of RHEL/CentOS, Ubuntu, and macOS. The Framework repository contains a subset
of NBIS that is built from the top-level makefile. However, if there is a need
to use the _official_ NBIS, then the makefile in `common/src/libbiomeval` can
be changed to use that NBIS build. Biometric Evaluation Framework will look for NBIS
to be installed at `/usr/local/nbis`. To build NBIS,
[download the source](https://www.nist.gov/itl/iad/image-group/products-and-services/image-group-open-source-server-nigos),
and follow this basic build procedure:

```
./setup.sh /usr/local/nbis [--without-X11]
make config it
sudo make install
```

---

As Seen In...
-------------
NIST is committed to using Biometric Evaluation Framework in their biometric
evaluations, including:

 * [FpVTE 2012](https://www.nist.gov/itl/iad/image-group/fpvte-2012);
 * [MINEX III](https://www.nist.gov/itl/iad/image-group/minutiae-interoperability-exchange-minex-iii);
 * [IREX III](https://www.nist.gov/itl/iad/image-group/irex-iii-homepage);
 * [FaCE](https://www.nist.gov/programs-projects/face-challenges);
 * [FIVE](https://www.nist.gov/programs-projects/face-video-evaluation-five);
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
 * compile without warning under macOS and RHEL/CentOS 7.x;
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
[our manuscript](http://ieeexplore.ieee.org/document/7358800/):

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

[1]: https://github.com/usnistgov/libbiomeval/blob/master/common.mk#L47
