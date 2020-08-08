UMBRA OCCLUSION BOOSTER README FOR WINDOWS


(C) 2009-2010 Umbra Software Ltd. 
All Rights Reserved.

This file consists of unpublished, proprietary information of
Umbra Software, and is considered Confidential for the purposes 
of non-disclosure agreement. Disclosure outside the terms
outlined in signed agreement may result in irrepairable harm to
Umbra Software and legal action against the party in breach.



Directory structure
---------------------------------

\bin		DLLs and compiled sample binaries.
\build		Visual Studio solution files for compiling binaries and samples.
\interface	Umbra OB header files.
\lib		Umbra OB libraries and .pdb files.
\samples	Sample sources.


Naming conventions
---------------------------------

umbra{32|64}.lib		linker lib for release build (umbra{32|64}.dll)
umbra{32|64}_eval.lib	linker lib for evaluation build (umbra{32|64}_eval.dll)
umbra{32|64}_d.lib		linker lib for debug build (umbra{32|64}_d.dll)
umbra{32|64}_p.lib		linker lib for profile build (umbra{32|64}_p.dll)
umbra{32|64}_s.lib		static release lib
umbra{32|64}_sd.lib		static debug lib
umbra{32|64}_sp.lib		static profile lib


Known issues
---------------------------------

- The performance in the OpenGL sample on certain AMD video cards is not optimal
- 64 bit build uses more memory than 32 bit build
- Due to the memory optimization in 2.2.0 release, some corner cases have slightly worse
  performance when compared to 2.1.2