#CH9102，设置别名为wheeltec_mic
echo  'KERNEL=="ttyACM*", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="55d4",MODE:="0777", GROUP:="dialout", SYMLINK+="wheeltec_mic"' >>/etc/udev/rules.d/wheeltec_mic.rules

service udev reload
sleep 2
service udev restart


# guest@guest:~/offline_chat$ cat /etc/udev/rules.d/wheeltec_mic.rules 
# KERNEL=="tty*", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="55d4",MODE:="0777", GROUP:="dialout", SYMLINK+="wheeltec_mic"
# KERNEL=="tty*", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523",MODE:="0777", GROUP:="dialout", SYMLINK+="wheeltec_mic"
# KERNEL=="tty*", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="8091",MODE:="0777", GROUP:="dialout", SYMLINK+="wheeltec_mic"

