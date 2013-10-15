RTStruct2Vtk
============

Small code to build 3D images from radiotherapy structure sets (RTSTRUCT). 

## 1. Requirements
[Cmake](http://cmake.org) is required to build the code.

Two libraries are required

- [VTK](http://www.vtk.org) that can be installed on a Mac with
 
        brew install vtk

- [GDCM](http://gdcm.sourceforge.net/wiki/index.php/Main_Page) for converting RTStruct to PolyData.


## 2. Installation
The code is built using:

    git clone git@github.com:osaut/RTStruct2Vtk.git
    cd RTStruct2Vtk
    cmake .
    make

## 3. Usage
The code is called with a RTStruct file in argument

    ./rtsr.out RTStruct.dcm

it should create `VTI` volumes file that [Paraview](http://www.paraview.org) can read and that [VTK](http://www.vtk.org) can convert to other useful file formats. A volume is created for each delineation found in the RTStruct file.
