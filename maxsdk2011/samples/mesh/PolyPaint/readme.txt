These files are used commonly by both EditablePoly and EditPoly.

They were written by Michaelson Britt, and modified some by Steve Anderson.

-steve.anderson, 6/18/04

Removed this cpp file from Edit Poly Modifier. Instead the Modifier links against the
editable poly library (EPoly.lib / EPoly.dlo). This was done to 
1. prevent the cpp file from getting compiled twice.
2. prevent memory leaks.
3. simply the design.
4. provide clear ownership of the Manager classes that are in it.

-chris.johnson, 11/20/07