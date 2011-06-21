### Buildall  
Chris Pilkington  
Copyright (C) 2010-present  
<http://chris.iluo.net/> 

### License

This application is free software; you can redistribute it and/or  
modify it under the terms of the GNU General Public  
License as published by the Free Software Foundation; either  
version 2 of the License, or (at your option) any later version. 

This library is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  
General Public License for more details. 

You should have received a copy of the GNU General Public  
License along with this library; if not, write to the Free Software  
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA  


### Getting a copy of the project on Linux

Prerequisites:  
yum install git  
OR  
sudo apt-get install git-core  


Directory structure:  
source/  
library/  
buildall/  


Pull [my library][1] which this project uses:  
git clone git@github.com:pilkch/library.git  
OR  
git clone https://github.com/pilkch/git@github.com:pilkch/library.git  


Pull this project:  
git clone git@github.com:pilkch/buildall.git  
OR  
git clone https://github.com/pilkch/git@github.com:pilkch/buildall.git  


### Building on Linux

Prerequisites:  
yum install cmake  
OR  
sudo apt-get install cmake  


The project can be built like so:  
cd buildall  
cmake .  
make  
### build.xml

TODO: Document the format of build.xml  


### Running on Linux

Running buildall in Build Mode:  
./buildall -build  


### Scheduling on Linux

Gnome Schedule can help you set up buildall to run at a scheduled time. I run it at 3am so that I can just get the results in the morning.  
You can install gnome-schedule like so:  
sudo apt-get install gnome-schedule  
And run it:  
gnome-schedule  
This would be your command line:  
<path to buildall> -build  
This will run the buildall as your user. After each run results.xml will be written to your home directory. 

### Credit

Buildall was created by me, Christopher Pilkington.   
For the parts that are not mine, I try to keep an up to date list of any third party libraries that I use.   
I only use libraries that are license under either the GPL, LGPL or similar and am eternally grateful for the high quality, ease of use and generosity of the open source community. 

MD5 RFC 1321 compliant MD5 implementation  
Copyright (C) 2001-2003 Christophe Devine </body> </html>

 [1]: https://github.com/pilkch/library/
