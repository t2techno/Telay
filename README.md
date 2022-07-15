# Telay
A multi-featured plugin based around delays. Can do a normal delay, flanger, and chorus.

To run: 
1. Clone the code
2. Download Projucer: https://juce.com/get-juce
3. Open Telay.jucer
4. In side window, go to Exporters tab
5. Click Debug
6. Enable "Enable Plugin Copy Step"
7. In the next field down, add your DAW's vst3 location. Ex: C:/vst3
8. Export to your selected IDE

If Visual Studios-Windows (That's what I use. Version 17.2.5 at time of writing)

9. Right click on Telay_VST3 solution in Solution Explorer
10. Set as Startup Project
11. Right click on Telay_VST3 solution in Solution Explorer again
12. Properties
13. In Left window, under Configuration Properties, click on Debugging
14. Right window, click on arrow at right of top row to change Command
15. Browse and add your DAW of choice (Who's vst3 folder you used in step 7)</br>
    Ex: C:\ProgramData\Ableton\Live 11 Standard\Program\Ableton Live 11 Standard.exe
16. Run the project and your DAW of choice should open automatically and you should find the vst in your normal plugin location

*Note*: If you don't see the vst in your DAW, some starting troubleshooting steps: 
1. Try to manually refresh/rescan your vst folder
2. Check The vst location in File Explorer to verify the file was copied properly
3. Confirm you entered the correct field in Projucer. 


To do's:
1. Add labels
2. Improve interpolation method
3. Make Prettier
