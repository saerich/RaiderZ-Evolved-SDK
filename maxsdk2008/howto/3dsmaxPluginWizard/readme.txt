
Archive Contents
----------------------------
3ds max Plugin Wizard Functionality and Template files.
----------------------------


Outline
----------------------------
The 3ds max Plugin Wizard allows you to create plugin projects for 3ds max 8
through a wizard interface in the Visual Studio 8 IDE.
----------------------------


Installing
----------------------------

1. Open the 3dsmaxPluginWizard.vsz file (in the 3dsmaxPluginWizard directory root) 
   in a text editor and edit the ABSOLUTE PATH parameter to reflect the new location of the 
   3dsmaxPluginWizard root directory. Do not add a backslash after the directory name.

	Param="ABSOLUTE_PATH = [Absolute Path Location of 3dsmaxPluginWizard Root Directory]"

2. Copy the following files from the 3dsmaxPluginWizard root to the 'VC\vcprojects'
   directory under your Visual Studio install (e.g. C:\Program Files\Microsoft Visual Studio 8\VC\vcprojects):
	
	3dsmaxPluginWizard.ico
	3dsmaxPluginWizard.vsdir
	3dsmaxPluginWizard.vsz

3. At this point the 3ds max Plugin Wizard project should appear under File menu:New:Projects:Visual C++ Projects
   in Visual Studio.
	
----------------------------


Usage
----------------------------
The wizard will guide you through the steps involved in setting up your 3ds max 7/8 plugin project.
----------------------------


Feedback
----------------------------
Please send any and all feedback to sparks.tools@autodesk.com.
----------------------------




