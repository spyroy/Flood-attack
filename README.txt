To run this program open terminal and type - "sudo ./Q4 -t [target's ip] -p [target's port] -r"
where -t -p -r are optional but need to be in this order, for example - "sudo ./Q4 -t 127.0.0.2 -p 444"
is ok, but - "sudo ./Q4 -p 444 -t 127.0.0.2" is not ok.
when -t or -r not specified the default ip address is 127.0.0.1 and the default port is 443.
'-r' option is to change from tcp flood to udp flood.

We tested the progaram by running it from ubuntu, and the other person checked the target's pc on wireshark. we managed to see all RST packets or the udp packets that was sent. 

Matan Greenberg wrote the send_tcp function and Roi Mash wrote the send_udp function.

![](https://github.com/spyroy/Security-Protocols-Assignment/blob/main/tcp%20capture.png)
