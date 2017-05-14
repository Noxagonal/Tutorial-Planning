Planning source code for my YouTube Vulkan tutorials: https://www.youtube.com/playlist?list=PLUXvZMiAqNbK8jd7s52BIDtCbZnKNGp0P


Windows only, sorry... Contact me if you want to make a branch for other OSes.
Visual Studio Community 2015 only, you can update if you want but you'll have to check the paths.


You'll need at least 2 extra libraries for this project to compile:

Freeimage 3.17.0 or newer DLL libraries:
- http://freeimage.sourceforge.net/download.html (Download "FreeImage DLL")
- unpack in %VK_SDK_PATH%/../ (For example "C:/VulkanSDK/")
- copy 32 or 64 bit dll file to next to the solution file of this project. (For example 64 bit dll: "C:/VulkanSDK/FreeImage/Dist/x64/FreeImage.dll" to <wherever you put this solution file>.

GLM 0.9.8.4 or newer
- http://glm.g-truc.net/0.9.8/index.html
- unpack in %VK_SDK_PATH%/../ (For example "C:/VulkanSDK/")

(Suggested, Enabled by default but optional) GLFW 3.2.1 or newer
- http://www.glfw.org/download.html (Download pre-compiled binaries)
- unpack in %VK_SDK_PATH%/../ (For example "C:/VulkanSDK/")


This code is provided in hopes it'll be useful for people studying Vulkan, no licence.
Copy, share, redistribute, modify and use however you wish for whatever project you wish.