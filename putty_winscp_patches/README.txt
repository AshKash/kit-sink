I was wondering if any one is interested in integrating winscp more tightly with putty? 
i have a proof of concept at: 
http://www.cs.rutgers.edu/~ashwink/putty 

This is very much proof of concept: 

1. displays a winscp menu item 
2. when you click on it, does CreateProcess(winscp.exe) 
3. the correct directory is opened 

problems: 
1. winscp path is hardcoded (c:\program files\winscp3\winscp3.exe) 
2. does not load up the proper profile: 
basically execs: winscp username@host:path 
This will hence not work if you have a firewall 
configured 

cybernytrix at y a h o o dot com 

~ashwin 
