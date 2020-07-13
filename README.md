# astro-enhance
Image processing and manipulation geared toward astrophotography

---

## Utilities
There are several distinct utilities: `coadd`, `stacker`, `startrails`, 
`subtract`, `advanced_coadd` (**`advanced_coadd` is currently 
broken**) and `test`. 
  - `coadd` does basic coadding of image stacks either by average or 
  by median - it does not perform any alignment.
  - `stacker` attempts to perform feature-based image alignment with
  coadding of the aligned images. The alignment methodology is not 
  robust and needs improvement. Aligning based on the star field is 
  desirable and intended as a future improvement.
  - `startrails` performs a selective 'brighten-only' additive 
  operation on a series of images which attempts to coadd everything
  but the stars in order to digitally create star trails. New star 
  detection algorithm `[align_stars()]` has not been implemented
  in this routine but will be in the future.
  - `subtract` does simple subtraction of a given image from a stack
  of images with the option to add a scalar multiplication factor to
  the subtraction frame. Designed for subtracting dark frames but needs
  improvement.
  - `test` is simply for testing new code.
  - `advanced_coadd` is intended to perform selective coadding of only 
  the region encompassed by the star field (i.e. ignoring the foreground)
  but it is currently broken.
  
## Building
Any of the above routines can be built by simply specifiying 
the routine name to `make`, i.e.

```
make <routine>
```

note that a routine must be specified or nothing will be built.
