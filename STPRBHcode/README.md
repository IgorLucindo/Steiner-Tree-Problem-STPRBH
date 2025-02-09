viennaNodehopper                {#mainpage}
================

This is the documentation for the program **viennaNodehopper**, which is the implementation of the branch-and-cut algorithm  
for the *Steiner tree problem with revenues, budget and hop-constraints* described in

> *A Node-Based Layered Graph Approach for the Steiner Tree Problem with Revenues, Budget and Hop-Constraints*

by Markus Sinnl and Ivana Ljubic

## Installation Instructions ##

Note that this program is intended to be run under Linux/has only be tested under Linux.

1. Unzip the package to a folder of your choosing
2. Ensure that you have installed the following prerequisites:
  * OGDF, which can be obtained from http://www.ogdf.net/doku.php 
    OGDF is licensed under the GNU General Public License, see https://www.gnu.org/copyleft/gpl.html for details.
  * CPLEX, which can be obtained from http://www-01.ibm.com/software/commerce/optimization/cplex-optimizer/ 
    Note that CPLEX is a commercial software, however, there exists an academic version.
  * boost, which is available from http://www.boost.org/ or using the package managers of your Linux distribution.
    Note that we use the following boost libraries: *boost_program_options* *boost_filesystem* *boost_system* *boost_timer* *boost_thread* *boost_chrono*
    boost is licensed under the boost license, see http://www.boost.org/users/license.html
3. Configure the **Makefile** with approbriate paths for the prerequisites. If you want to link the external libraries statically, look at the
   *ifeq($USER,msinnl)* block, if you want to link them dynamically, look ath the *ifeq($USER,markus)* block. Replace the name with your username, or just delete the *ifeq* *endif*.
   The paths to change are under *OGDFDIR*, *CPLEXDIR*, *BOOST_PATHINC*, *BOOST_PATHLIB*, you can also set the compiler with *CXX* (it has been tested with g++>=4.8). 
4. Type **make** to compile the code. Grab some coffee, it will take a few minutes ;)
5. (Optional) Type **make test** to start a run on an easy instance to verify the compilation worked
6. (Optional) Type **make doc** to make documentation using doxygen. The documentation will be in the folder *doc*
   Open index.html in the folder *doc/html* or run **make** in *doc/latex* to create a pdf

## Verifying the Results Produced in Above Paper ##

1. Download the instances from http://dimacs11.cs.princeton.edu/instances/STPRBH-RANDOM.zip
2. Create a folder *instances* in the same folder, where *code* also resides and put the instances in this folder
3. Create a folder *results* in the *STPRBHcode* folder
4. Execute the script **runAll.sh** (this will sequentially start the runs for every instance in the folder *instances* with settings as used in the paper)
5. The results are collected in single-files with ending *.stat* in the folder *results*. They are in csv-format, you can, e.g., use **cat** to combine them all in
   a single file to view. The *runtime* is in the second column, *solution value* in the fourth, and *UB* in the fifth.
   For more details about the format, please see the method *STPRBHSolver::setStatistics()* in the file *mipsolver/STPRBHSolver.cpp*.

To run a single-instance with the settings used in the paper, execute **runSingle.sh INSTANCE**.

## Trying Some Other Options ##

The program allows to turn on/off various separation routines, the heuristic, and some more settings.  
Please run **STPRBH --help** for details on the possible settings.
See also runSingle.sh, where most of the available options are set (note that the values set in the script are also default in the code).

## Contact ##

If there are any problems or unclear instructions, please do not hesitate to contact me under markus.sinnl@univie.ac.at or *markus.sinnl* (Skype)

## License ##

The code is distributed under the GNU General Public License, see https://www.gnu.org/copyleft/gpl.html for details.
Please cite our paper, if you use our code.
