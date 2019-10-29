# Maya scripts & plugins
Simple scripts and utility plugins. CMake system was adopted from [Chad Vernon - cgcmake](https://github.com/chadmv/cgcmake)

Tested with Visual Studio 16 2019 (x64) with v140 platform toolset on Autodesk Maya 2018.6.

## Utah Teapot primitive
<p align="center"><img src="teapot/teapot.png" width="600" /></p>
<p>C++ plugin to create NURBS and polygon geometry primitive from the Utah Teapot dataset</p>

## Torus Knot primitive
<p align="center"><img src="torusknot/parametric_equ_lsfit.png" width="600" /></p>
<p align="center"><img src="torusknot/tk_nurbs_poly.png" width="600" /></p>
<p>Approximating (least-squares fit) the parametric equation of (p,q)-torus knots with a series of Bézier curves joined end to end</p>

## Gerstner Wave
<p align="center"><img src="gerstner_wave/wave_anim.gif" width="480" /></p>
Performs deformation of the input plane and creates trochoidal / Gerstner waves

## Geom Info
Debug utility commands for querying curve / polygon / NURBS geometry data

## Issues:
* Torus Knot - extruding a profile along the torus knot curve results in a NURBS surface that pinches at the start-end of the curve
* Torus Knot - poly torus knot utility generates surface that exhibits polygon flow distortion (TNB frame)
