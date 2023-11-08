# Questinfo-command
questinfo command for TrinityCore 3.3.5

## How to install  
**1.** Go to the **TrinityCore\src\server\scripts\Custom** folder and copy the script named **cs_questinfo_custom.cpp** into it<br/>

***

**2.** Add **cs_questinfo_custom.cpp** to \src\server\scripts\Custom\ **custom_script_loader.cpp**<br/>

![alt_tag](https://i.imgur.com/jfpbVAK.png)<br/>

***

**3.** Сopy the contents of the **Language.h.txt** file and add it to the end of the **Language.h** file, it is located **src\server\game\Miscellaneous**<br/>

![alt_tag](https://i.imgur.com/YGzZMiR.png)<br/>

***

**4.** Use CMake (configure and generate)<br/>

***

**5.** Execute **language.sql** to your database.
Make sure that the numbers under which LANGs will be added are unoccupied(world -> trinity_string)<br/>

***

**6.** Open TrinityCore.sln and Build the solution. Click on **Build** and then **Build Solution** or press (Ctrl+Shift+B)<br/>

***

*Optional* <br/>

**7.1** If you want the command to output information about **creaturestarter/ender** and **gostarter/ender**, then you must copy the contents of the **ObjectMgr.h.txt** file and paste this into the **ObjectMgr.h** file as shown in the figure below<br/>

![alt_tag](https://i.imgur.com/CFWad6z.png)<br/>

Then you can uncomment the lines shown below<br/>

![alt_tag](https://i.imgur.com/hO6NOnx.png)<br/>

***

**7.2.** If you want the command to output a list of status and a list of quest flags, then you must copy the contents of the enuminfo_QuestDef.cpp.txt file and add it to the end of the enuminfo_QuestDef.cpp file as shown in the figure below.
(check to see if you already have this code in your core)

![alt_tag](https://i.imgur.com/2fF0yg4.png)<br/>

After that you can uncomment the lines shown below

![alt_tag](https://i.imgur.com/3Ppc4b2.png)<br/>

***

**7.3** Open the TrinityCore.sln file again and build the solution. Click on **Build** and then **Build Solution**<br/>

***
## How to use  



