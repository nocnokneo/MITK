![MITK Logo][logo]

The [Medical Imaging Interaction Toolkit][mitk] (MITK) is a free open-source software
system for development of interactive medical image processing software. MITK
combines the [Insight Toolkit][itk] (ITK) and the [Visualization Toolkit][vtk] (VTK) with an application framework.

See the [MITK homepage][mitk] for details.

Supported Platforms
-----------------------------

MITK is a cross-platform C++ toolkit and officially supports:

 - Windows
 - MacOS X
 - Linux

For details, please read the [Supported Platforms][platforms] page.

License
-----------

Copyright (c) [German Cancer Research Center][dkfz].

MITK is available as free open-source software under a [BSD-style license][license].

Download
---------

The MITK source code and binaries for the *mitkWorkbench* application are released regularly according to the [MITK release cycle][release-cycle]. See the [Download][download] page for a list of releases.

The official MITK source code is available in the [MITK Git repository][gitweb]. The Git clone command is

    git clone http://git.mitk.org/MITK.git

Active development takes place in the MITK master branch and its usage is advised for advanced users only.

How to Contribute
--------------

Contributions of all kind are happily accepted. However, to make the contribution process as smooth as possible, please read the [How to contribute to MITK][contribute] page if you plan to contribute to MITK.


Build Instructions
------------------------

MITK uses [CMake][cmake] to configure a build tree. The following is a crash course about cloning, configuring, and building MITK on a Linux/Unix system:

    git clone http://git.mitk.org/MITK.git
    mkdir MITK-build
    cd MITK-build
    cmake ../MITK
    make -j4

Read the comprehensive [Build Instructions][build] page for details.

Useful Links
------------------

 - [Homepage][mitk]
 - [Download][download]
 - [Mailing List][mailinglist]
 - [Bug Tracker][bugs]


[logo]: https://github.com/MITK/MITK/raw/master/mitk.png
[mitk]: http://mitk.org
[itk]: http://www.itk.org
[vtk]: http://www.vtk.org
[platforms]: http://docs.mitk.org/nightly-qt4/SupportedPlatformsPage.html
[dkfz]: http://www.dkfz.de
[license]: http://mitk.org/git/?p=MITK.git;a=blob_plain;f=LICENSE.txt
[release-cycle]: http://mitk.org/MitkReleaseCycle
[download]: http://mitk.org/Download
[gitweb]: http://mitk.org/git/?p=MITK.git
[contribute]: http://mitk.org/How_to_contribute
[cmake]: http://www.cmake.org
[build]: http://docs.mitk.org/nightly-qt4/BuildInstructionsPage.html
[mailinglist]: http://mitk.org/Mailinglist
[bugs]: http://bugs.mitk.org/
