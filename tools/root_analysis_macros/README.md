---
# SPDX-FileCopyrightText: 2017-2025 CERN and the Allpix Squared authors
# SPDX-License-Identifier: CC-BY-4.0
title: "ROOT Analysis & Helper Macros"
---

Collection of macros demonstrating how to analyze data generated by the framework. Currently contains a C++ macro to convert the TTree of objects to a tree containing standard data written by the framework. This is useful for analysis and comparisons with other frameworks.
A simple example of how to read the output objects TTree using a Python macro is also included.


## Comparison tree

Reads all required trees from the given file and binds their content to the objects defined by the framework. Then creates an output tree and binds every branch to a simple arithmetic type. Continues to loop over all events in the tree and converting the stored data from the various trees to the output tree. The final output tree contains branches for the cluster sizes, aspect ratios, accumulated charge per event, the track position from the Monte Carlo truth and the reconstructed track obtained from a center of gravity calculation using the charge values without additional corrections.

To construct a comparison tree using this macro, follow these steps:

* Open root with the data file attached like `root -l /path/to/data.root`
* Load the current library of objects with `.L path/to/libAllpixObjects.so`
* Build the macro with `.L path/to/constructComparisonTree.C++`
* Open a new file with `auto file = new TFile("output.root", "RECREATE")`
* Run the macro with `auto tree = constructComparisonTree(_file0, "name_of_dut")`
* Write the tree with `tree->Write()`

## Analysis example

Analysis example demonstrating how to read data from ROOT TTrees, access attributes and access object history. The macro for this reads TTrees of `PixelHit` and `MCParticle` objects from an Allpix Squared data file created using the `ROOTObjectWriter`. Iterating over individual events, the position of every `PixelHit` is compared to the center of gravity position of all `MCParticles` and then only to those that are retrieved from the history of the `PixelHit`. Produces graphs for a 2D hitmap, the mentioned residuals and the signal spectrum. As this macro does not perform a clustering, it is only a starting point for a data analysis.

Usage:

* Open root with the data file attached like `root -l /path/to/data.root`
* Load the current library of objects with `.L path/to/libAllpixObjects.so`
* Build the macro with `.L path/to/analysisExample.C++`
* Run the macro with `analysisExample(_file0, "name_of_detector")`

## Eta correction of residuals example

Analysis example demonstrating how to perform an "eta correction" for rectangular pixels. Builds on from `Analysis example` above. This macro performs clustering using the method implemented in the `DetectorHistogrammer` module, and loops over all events twice; once to generate and fit the eta function (a fifth-order polynomial is used in this case), and once to apply the correction given by the fit. Produces graph of residuals before and after application of the correction. The pixel size used should be edited to match the simulated pixel size in the data to be analysed.

Usage:

* Open root with the data file attached like `root -l /path/to/data.root`
* Load the current library of objects with `.L path/to/libAllpixObjects.so`
* Load the Cluster object definition from the `DetectorHistogrammer` module with `.L path/to/libAllpixModuleDetectorHistogrammer.so`
* Build the macro with `.L path/to/etaCorrectionResiduals.C++`
* Run the macro with `etaCorrectionResiduals(_file0, "name_of_detector")`

## Remake project

Simple macro to show the possibility to recreate source files for legacy objects stored in ROOT data files from older versions of the framework. Can be used if the corresponding dynamic library for that particular version is not accessible anymore. It is however not possible to recreate methods of the objects and it is therefore not easily possible to reconstruct the stored history.

To recreate the project source files, the following commands should be executed:

* Open root with the data file attached like `root -l /path/to/data.root`
* Build the macro with `.L path/to/remakeProject.C++`
* Recreate the source files using `remakeProject(_file0, "output_dir")`

## Recover Configuration Files

This macro allows to recover the full configuration of a simulation from a data file written by the ROOTObjectWriter module. It retrieves the stored key-value pairs and writes them into new files, including the framework and module configuration, the detector setup and the individual detector models with possibly overwritten parameters.

The simulation configuration can be recreated using the following command:

```shell
root -x 'recoverConfiguration.C("path/to/output/data.root",
                                "configuration.conf")'
```

Here, the first argument is the input data file produced by the ROOTObjectWriter, while the second argument is the output file name and path for the framework configuration. The detector setup and model files will be named as defined in the main configuration and are placed in the same folder.

## Display Monte Carlo hits (Python)

Simple macro that reads the required trees to plot Monte Carlo hits in pixel versus the pixel charge. Loops over all events of the root file.
A few relevant histograms are displayed at the end of the event loop.
Requires PyROOT, numpy, matplotlib. To execute the script, run:

```shell
python3 display_mc_hits.py -l path/to/libAllpixObjects.so -f path/to/data.root -d <detector_name>
```
