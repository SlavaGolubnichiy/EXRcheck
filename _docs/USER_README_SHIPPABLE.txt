How to use:
If you want to view .exr content in app windows (console) => 
	just drag-and-drop .exr file over EXRcheck_App.exe file.

If you want to save and view .exr content in a text file =>
	1. open command prompt at the same directory of EXRcheck_App.exe
		on Windows: 
			open folder storing EXRcheck_App.exe -> 
			-> in the box where path is shown, type in "cmd" (without quotes) -> cmd.exe opens
	2. in opened cmd.exe, type in the following command:
		EXRcheck_App.exe filepath\filename.exr > outputDestination.txt
		, where:
			a) filepath.exr - is a file path to .exr file you want to analyze. 
				If you put .exr file next to EXRcheck_App.exe: you can type only filename.exr (myFile.exr).
			b) outputDestination.txt - name of the text file where EXRcheck_App.exe's output will be saved.
				The result outputDestination.txt is saved in the same folder with EXRcheck_App.exe.
	3. After typing the command, press 'Enter' to run it -> press 'Enter' again to let EXRcheck_App.exe finish.
	4. Done! 
		The results of EXRcheck_App.exe analyzed your filepath\filename.exr are saved in outputDestination.txt.
		